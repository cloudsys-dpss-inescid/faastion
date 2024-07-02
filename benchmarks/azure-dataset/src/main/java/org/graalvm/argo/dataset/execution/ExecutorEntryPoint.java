package org.graalvm.argo.dataset.execution;

import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Option;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;

public class ExecutorEntryPoint {

    public static void main(String[] args) {
        Options options = prepareOptions();
        try {
            CommandLine cmd = new DefaultParser().parse(options, args);
            String inputFilePath = cmd.getOptionValue("input");
            String functionCodeFilePath = cmd.getOptionValue("functionCode");
            String functionLanguage = cmd.getOptionValue("functionLanguage");
            String functionEntryPoint = cmd.getOptionValue("functionEntryPoint");
            String functionMemory = cmd.getOptionValue("functionMemory");
            String functionRuntime = cmd.getOptionValue("functionRuntime");
            String invocationCollocation = cmd.getOptionValue("invocationCollocation");
            String functionIsolation = cmd.getOptionValue("functionIsolation");
            String gvSandbox = cmd.getOptionValue("gvSandbox");
            boolean debug = cmd.hasOption("debug");
            byte[] functionCode = Files.readAllBytes(Paths.get(functionCodeFilePath));
            String lambdaManagerAddress = cmd.getOptionValue("lambdaManagerAddress", "localhost:30009");
            ExecutorConfiguration config = new ExecutorConfiguration(functionCode, functionLanguage, functionEntryPoint, functionMemory, functionRuntime, invocationCollocation, functionIsolation, gvSandbox, debug, lambdaManagerAddress);
            InvocationTraceExecutor executor = new InvocationTraceExecutor(config);
            executor.execute(inputFilePath);
        } catch (ParseException e) {
            System.out.println(e.getMessage());
            new HelpFormatter().printHelp("utility-name", options);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static Options prepareOptions() {
        Options options = new Options();
        Option input = new Option("i", "input", true, "Input invocation trace file path.");
        input.setRequired(true);
        options.addOption(input);
        Option functionCode = new Option("fc", "functionCode", true, "Function code file path.");
        functionCode.setRequired(true);
        options.addOption(functionCode);
        Option functionLanguage = new Option("fl", "functionLanguage", true, "Function language.");
        functionLanguage.setRequired(true);
        options.addOption(functionLanguage);
        Option functionEntryPoint = new Option("fep", "functionEntryPoint", true, "Function entry point.");
        functionEntryPoint.setRequired(true);
        options.addOption(functionEntryPoint);
        Option functionMemory = new Option("fm", "functionMemory", true, "Function memory.");
        functionMemory.setRequired(true);
        options.addOption(functionMemory);
        Option functionRuntime = new Option("fr", "functionRuntime", true, "Function runtime.");
        functionRuntime.setRequired(true);
        options.addOption(functionRuntime);
        Option invocationCollocation = new Option("ic", "invocationCollocation", true, "Collocation of invocations in a single worker.");
        invocationCollocation.setRequired(true);
        options.addOption(invocationCollocation);
        Option functionIsolation = new Option("fi", "functionIsolation", true, "Isolation of functions across several workers.");
        functionIsolation.setRequired(true);
        options.addOption(functionIsolation);
        Option gvSandbox = new Option("gvs", "gvSandbox", true, "Sandbox to be used in Graalvisor.");
        gvSandbox.setRequired(false);
        options.addOption(gvSandbox);
        Option debug = new Option("d", "debug", false, "Just print requests instead of sending them.");
        debug.setRequired(false);
        options.addOption(debug);
        Option lambdaManagerAddress = new Option("lm", "lambdaManagerAddress", true, "Full address of the lambda manager.");
        lambdaManagerAddress.setRequired(false);
        options.addOption(lambdaManagerAddress);
        return options;
    }

}
