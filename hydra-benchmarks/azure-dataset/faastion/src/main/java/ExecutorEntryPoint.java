
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
            int numFunctions = Integer.valueOf(cmd.getOptionValue("numFunctions", "5"));
            boolean debug = cmd.hasOption("debug");
            String approach = cmd.getOptionValue("approach");
            String lambdaManagerAddress = cmd.getOptionValue("lambdaManagerAddress", "localhost:8080");
            ExecutorConfiguration config = new ExecutorConfiguration(numFunctions, approach, debug, lambdaManagerAddress);
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
        Option numFunctions = new Option("nf", "numFunctions", true, "Number of functions for executions.");
        numFunctions.setRequired(false);
        options.addOption(numFunctions);
        Option approach = new Option("a", "approach", true, "Approach to use.");
        approach.setRequired(true);
        options.addOption(approach);
        Option debug = new Option("d", "debug", false, "Just print requests instead of sending them.");
        debug.setRequired(false);
        options.addOption(debug);
        Option lambdaManagerAddress = new Option("lm", "lambdaManagerAddress", true, "Full address of the lambda manager.");
        lambdaManagerAddress.setRequired(false);
        options.addOption(lambdaManagerAddress);
        return options;
    }

}
