package org.graalvm.argo.graalvisor;

import java.io.IOException;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.argo.graalvisor.sandboxing.SandboxHandle;

import com.sun.net.httpserver.HttpExchange;
import org.graalvm.argo.graalvisor.utils.JsonUtils;

/**
 * A runtime proxy that runs requests on Native image-based sandboxes.
 */
public class SubstrateVMProxy extends RuntimeProxy {

    /**
     * A Request object is used as a communication packet between a foreground thread and a background thread.
     */
    static class Request {

        final boolean async;

        final long startTime;

        final String input;

        String output;

        public Request(boolean async, long startTime, String input) {
            this.async = async;
            this.startTime = startTime;
            this.input = input;
        }

        public boolean isAsync() {
            return this.async;
        }

        public long getStartTime() {
            return this.startTime;
        }

        public String getInput() {
            return input;
        }

        public void setOutput(String output) {
            this.output = output;
        }

        public String getOutput() {
            return this.output;
        }
    }

    /**
     * An isolate worker is a thread that retrieves requests from a queue and runs them in its own isolate.
     */
    static class Worker extends Thread {

        private final FunctionPipeline pipeline;

        public Worker(FunctionPipeline pipeline) {
            this.pipeline = pipeline;
        }

        /**
         * Process the request in the given sandbox.
         * @return true if the request was processed without exceptions; false otherwise.
         */
        private boolean processRequest(SandboxHandle shandle, Request req) {
            try {
                // Extending the arguments JSON object to include sandbox-specific tmp directory.
                String arguments = JsonUtils.appendTmpDirectoryKey(req.getInput(), shandle.initSandboxTmpDirectory());
                req.setOutput(shandle.invokeSandbox(arguments));
                return true;
            } catch (Exception e) {
                e.printStackTrace(System.err);
                req.setOutput(getName());
                return false;
            } finally {
                // If the request is async, send reply. Otherwise, notify the frontend thread.
                if (req.async) {
                    sendReply(null, req.getStartTime(), req.getOutput());
                } else {
                    synchronized (req) {
                        req.notify();
                    }
                }
            }
        }

        public void runInternal() throws Exception {
            SandboxHandle shandle = prepareSandbox(pipeline.getFunction());
            Request req = null;
            int numberAttempts = 0;

            try {
                while (pipeline.isOpen()) {
                    req = pipeline.queue.poll();

                    if (req != null) {
                        // Processes a single request in the pipeline.
                        boolean success = processRequest(shandle, req);
                        // Decrement active requests in the pipeline.
                        pipeline.active.getAndDecrement();
                        // Reset the counter since we successfully processed a request, or it failed.
                        numberAttempts = 0;

                        if (!success) {
                            // If the request failed, break from the loop and destroy sandbox.
                            break;
                        }
                    } else {
                        // Sleep for 1 millisecond before trying again
                        Thread.sleep(10);

                        // Check if we have been polling for more than N times. Timeout of 10-seconds.
                        if (numberAttempts++ > 1000) {
                            break;
                        }
                    }
                }
            } catch (InterruptedException e) {
                e.printStackTrace(System.err);
            }

            // Worker is terminating. Decrement worker count.
            pipeline.workers.decrementAndGet();
            // Also destroy the sandbox.
            destroySandbox(pipeline.getFunction(), shandle);
        }

        @Override
        public void run() {
            try {
                runInternal();
            } catch (Exception e) {
                System.err.println(String.format("[thread %s] Error: thread quit unexpectedly", Thread.currentThread().getId()));
                e.printStackTrace(System.err);
            }
        }
    }

    /**
     * A function pipeline contains a queue used to submit requests for a function along with the number of free workers for this function.
     */
    static class FunctionPipeline {

        private final PolyglotFunction function;

        private final ConcurrentLinkedQueue<Request> queue;

        private final AtomicInteger active = new AtomicInteger(0);

        private final AtomicInteger workers = new AtomicInteger(0);

        private AtomicInteger maxWorkers = new AtomicInteger(0);

        private AtomicBoolean open = new AtomicBoolean(true);

        public FunctionPipeline(PolyglotFunction function) {
            this.function = function;
            this.queue = new ConcurrentLinkedQueue<>();
        }

        public void invokeInCachedSandbox(Request req) {
            int cActive = active.incrementAndGet();
            int cWorkers = workers.intValue();
            int cMaxWorkers = maxWorkers.intValue();

            // Checks if we need to launch additional sandboxes.
            if (cWorkers < cActive && (cMaxWorkers == 0 || cWorkers < cMaxWorkers)) {
                synchronized (this) {
                    // Repeats the check to verify that we won the race.
                    if (workers.intValue() < active.intValue()) {
                        workers.incrementAndGet();
                        new Worker(this).start();
                    }
                }
            }

            // Adding request to the function pipeline queue.
            queue.add(req);

            // If the request is synchronous, we need to wait for the output to be ready.
            if (!req.async) {
                synchronized (req) {
                    while(req.getOutput() == null) {
                        try {
                            req.wait();
                        } catch (InterruptedException e) {
                            // Ignore
                        }
                    }
                }
            }
        }

        public void setMaxWorkers(int value) {
            this.maxWorkers.set(value);
        }

        public void close() {
            this.open.set(false);
        }

        public void open() {
            this.open.set(true);
        }

        public PolyglotFunction getFunction() {
            return this.function;
        }

        public boolean isOpen() {
            return this.open.get();
        }

        public int getWorkers() {
            return this.workers.get();
        }
    }

    /**
     * A map of queues is used to send requests into worker threads.
     */
    private static ConcurrentMap<String, FunctionPipeline> queues = new ConcurrentHashMap<>();

    public SubstrateVMProxy(int port, String appDir) throws IOException {
        super(port, appDir);
    }

    protected static FunctionPipeline getFunctionPipeline(PolyglotFunction function) {
        FunctionPipeline pipeline = queues.get(function.getName());

        if (pipeline == null) {
            FunctionPipeline newpipeline = new FunctionPipeline(function);
            FunctionPipeline oldpipeline = queues.putIfAbsent(function.getName(), newpipeline);
            pipeline = oldpipeline == null ? newpipeline : oldpipeline;
        }

        return pipeline;
    }

    private static SandboxHandle prepareSandbox(PolyglotFunction function) throws IOException {
        long start = System.nanoTime();
        SandboxHandle worker = function.getSandboxProvider().createSandbox();
        long finish = System.nanoTime();
        System.out.println(String.format("[thread %s] New %s sandbox for function %s (sandbox id = %s) in %s us",
            Thread.currentThread().getId(),
            function.getSandboxProvider().getName(),
            function.getName(),
            worker,
            (finish - start)/1000));
        return worker;
    }

    private static void destroySandbox(PolyglotFunction function, SandboxHandle shandle) throws IOException {
        System.out.println(String.format("[thread %s] Destroying %s sandbox for function %s (sandbox id = %s)",
            Thread.currentThread().getId(),
            function.getSandboxProvider().getName(),
            function.getName(),
            shandle));
        function.getSandboxProvider().destroySandbox(shandle);
    }

    @Override
    protected void asyncInvoke(PolyglotFunction function, long startTime, String arguments) {
        getFunctionPipeline(function).invokeInCachedSandbox(new Request(true, startTime, arguments));
    }

    @Override
    protected void invoke(
            HttpExchange he,
            PolyglotFunction function,
            boolean cached,
            int warmupConc,
            int warmupReqs,
            long startTime,
            String arguments) {
        String output = "(uninitialized output)";
        try {
            if (!function.getSandboxProvider().isWarm()) {
                output = function.getSandboxProvider().warmupProvider(warmupConc, warmupReqs, arguments);
            } else if (cached) {
                Request req = new Request(false, startTime, arguments);
                getFunctionPipeline(function).invokeInCachedSandbox(req);
                output = req.getOutput();
            } else {
                SandboxHandle shandle = prepareSandbox(function);
                // Extending the arguments JSON object to include sandbox-specific tmp directory.
                arguments = JsonUtils.appendTmpDirectoryKey(arguments, shandle.initSandboxTmpDirectory());
                output = shandle.invokeSandbox(arguments);
                destroySandbox(function, shandle);
            }
        } catch (Exception e) {
            e.printStackTrace();
            output = e.getLocalizedMessage();
        } finally {
            sendReply(he, startTime, output);
        }
    }
}
