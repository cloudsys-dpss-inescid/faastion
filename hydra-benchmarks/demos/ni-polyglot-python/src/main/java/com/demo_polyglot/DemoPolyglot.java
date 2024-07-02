package com.demo_polyglot;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.HostAccess;

public class DemoPolyglot {
 
    public static void main(String[] args) {
        Context context = Context.newBuilder().allowAllAccess(true).build();
        context.eval("python", "print(1 + 1)");
        context.close();
    }
}
