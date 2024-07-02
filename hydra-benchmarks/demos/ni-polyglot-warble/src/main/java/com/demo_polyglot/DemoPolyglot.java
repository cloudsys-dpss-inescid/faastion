package com.demo_polyglot;

import java.io.File;
import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.Source;

public class DemoPolyglot {
 
	private static void warble(String warbleLoader, String warbleHome) {
		Context context = Context.newBuilder().allowAllAccess(true).option("python.ForceImportSite", "true").build();

        context.eval(Source.newBuilder("python", new File(warbleLoader)).buildLiteral());
        context.eval("python", "main").execute(warbleHome, "['-v', '{PRINT(\"test\")}']").toString();
	}
	
    public static void main(String[] args) {
    	String javaHome = System.getenv("JAVA_HOME");
    	String warbleHome = System.getenv("WARBLE_HOME");
    	String warbleLoader = args[0];

    	if (javaHome == null) {
            System.err.println("JAVA_HOME not found in the environment. Polyglot functionality significantly limited.");
        } else {
            System.setProperty("org.graalvm.language.python.home", javaHome + "/languages/python");
            System.setProperty("org.graalvm.language.llvm.home", javaHome + "/languages/llvm");
            System.setProperty("org.graalvm.language.js.home", javaHome + "/languages/js");
        }

    	if (warbleHome == null) {
            System.err.println("WARBLE_HOME not defined. Exiting...");
            return;
    	}

    	warble(warbleLoader, warbleHome);
    }
}
