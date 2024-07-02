package com.demo_polyglot;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.HostAccess;

public class DemoPolyglot {
 
    @HostAccess.Export
    public void sleep() {
    	System.out.println("Sleeping in Java!");
    }
   
    public static void main(String[] args) {
        Context context = Context.newBuilder().allowAllAccess(true).build();
        context.getBindings("js").putMember("polyHostAccess", new DemoPolyglot());
        context.eval("js", "polyHostAccess.sleep()");
        context.close();
    }
}
