package org.graalvm.argo.dataset;

import org.apache.commons.cli.*;
import org.graalvm.argo.dataset.aot.AOTInvocationTraceSimulator;

import java.util.List;

public class SimulatorEntryPoint {

    /**
     * Minimum number of milliseconds to wait before recalculating aggregated data.
     */
    protected static final int SAMPLE_INTERVAL = 1000;

    public static void main(String[] args) throws Exception {
        Options options = prepareOptions();
        try {
            CommandLine cmd = new DefaultParser().parse(options, args);
            String inputfile = cmd.getOptionValue("input");
            int keepalive = Integer.parseInt(cmd.getOptionValue("keepalive", "600000"));

            InvocationTraceSimulator simulator;
            if (cmd.hasOption("aot")) {
                simulator = new AOTInvocationTraceSimulator();
            } else {
                simulator = new InvocationTraceSimulator();
            }
            List<OutputEntry> output = simulator.simulate(inputfile, keepalive, SAMPLE_INTERVAL);

            for (OutputEntry entry : output) {
                System.out.println(entry);
            }
        } catch (ParseException e) {
            System.err.println(e.getMessage());
            new HelpFormatter().printHelp("utility-name", options);
        }
    }

    private static Options prepareOptions() {
        Options options = new Options();
        Option input = new Option("i", "input", true, "Input invocation trace file path.");
        input.setRequired(true);
        options.addOption(input);
        Option keepalive = new Option("k", "keepalive", true, "Function keep alive time in milliseconds.");
        keepalive.setRequired(false);
        options.addOption(keepalive);
        Option aot = new Option("aot", "aot", false, "Enable AOT optimization.");
        aot.setRequired(false);
        options.addOption(aot);
        return options;
    }
}
