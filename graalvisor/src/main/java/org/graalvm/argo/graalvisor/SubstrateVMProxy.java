package org.graalvm.argo.graalvisor;

import java.io.IOException;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.argo.graalvisor.sandboxing.SandboxHandle;

/**
 * A runtime proxy that runs requests on Native image-based sandboxes.
 */
public class SubstrateVMProxy extends RuntimeProxy {

    /**
     * A Request object is used as a communication packet between a foreground thread and a background thread.
     */
    static class Request {

        final String input;

        String output;

        public Request(String input) {
            this.input = input;
        }

        public String getInput() {
            return input;
        }

        public String setOutput(String output) {
            return this.output = output;
        }

        public String getOutput() {
            return output;
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

        private void processRequest(SandboxHandle shandle, Request req) {
            synchronized (req) {
                // Get input from request, invoke function in isolate, fill output.
                try {
                    req.setOutput(shandle.invokeSandbox(req.getInput()));
                } catch (Exception e) {
                    e.printStackTrace();
                }
                // Notify the frontend thread about the result being ready.
                req.notify();
            }
        }

        public void runInternal() throws Exception {
            SandboxHandle shandle = prepareSandbox(pipeline.getFunction());
            Request req = null;

            try {
                while ((req = pipeline.queue.poll(60, TimeUnit.SECONDS)) != null) {
                    processRequest(shandle, req);
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            pipeline.workers.decrementAndGet();
            destroySandbox(pipeline.getFunction(), shandle);
        }

        @Override
        public void run() {
            try {
                runInternal();
            } catch (Exception e) {
                System.err.println(String.format("[thread %s] Error: thread quit unexpectedly", Thread.currentThread().getId()));
                e.printStackTrace();
            }
        }
    }

    /**
     * A function pipeline contains a queue used to submit requests for a function along with the number of free workers for this function.
     */
    static class FunctionPipeline {

        private final PolyglotFunction function;

        private final BlockingQueue<Request> queue;

        private final AtomicInteger workers = new AtomicInteger(0);

        private final AtomicInteger active = new AtomicInteger(0);

        public FunctionPipeline(PolyglotFunction function) {
            this.function = function;
            this.queue = new ArrayBlockingQueue<>(64);
        }

        public String invokeInCachedSandbox(String input) {
            Request req = new Request(input);
            active.getAndIncrement();

            synchronized (this) {
                if (workers.intValue() < active.intValue()) {
                    workers.incrementAndGet();
                    new Worker(this).start();
                }
            }

            synchronized (req) {
                queue.add(req);

                while(req.getOutput() == null) {
                    try {
                        req.wait();
                    } catch (InterruptedException e) {
                        // Ignore
                    }
                }
                active.getAndDecrement();
                return req.getOutput();
            }
        }

        public PolyglotFunction getFunction() {
            return this.function;
        }
    }

    /**
     * A map of queues is used to send requests into worker threads.
     */
    private static ConcurrentMap<String, FunctionPipeline> queues = new ConcurrentHashMap<>();

    public SubstrateVMProxy(int port) throws IOException {
        super(port);
    }

    private static FunctionPipeline getFunctionPipeline(PolyglotFunction function) {
        FunctionPipeline pipeline = queues.get(function.getName());

        if (pipeline == null) {
            FunctionPipeline newpipeline = new FunctionPipeline(function);
            FunctionPipeline oldpipeline = queues.putIfAbsent(function.getName(), newpipeline);
            pipeline = oldpipeline == null ? newpipeline : oldpipeline;
        }

        return pipeline;
    }

    private static SandboxHandle prepareSandbox(PolyglotFunction function) throws Exception {
        long start = System.nanoTime();
        SandboxHandle worker = function.getSandboxProvider().createSandbox();
        long finish = System.nanoTime();
        System.out.println(String.format("[thread %s] New %s sandbox %s in %s us", Thread.currentThread().getId(), function.getSandboxProvider().getName(), worker, (finish - start)/1000));
        return worker;
    }

    private static void destroySandbox(PolyglotFunction function, SandboxHandle shandle) throws Exception {
        System.out.println(String.format("[thread %s] Destroying %s sandbox %s", Thread.currentThread().getId(), function.getSandboxProvider().getName(), shandle));
        function.getSandboxProvider().destroySandbox(shandle);
    }

    @Override
    protected String invoke(PolyglotFunction function, boolean cached, boolean warmup, String arguments) throws Exception {
        String res;

        if (warmup) {
            res = function.getSandboxProvider().warmupProvider(arguments);
        } else if (cached) {
            res = getFunctionPipeline(function).invokeInCachedSandbox(arguments);
        } else {
            SandboxHandle shandle = prepareSandbox(function);
            res = shandle.invokeSandbox(arguments);
            destroySandbox(function, shandle);
        }

        return res;
    }
}
