package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.util.List;
import javassist.CannotCompileException;
import javassist.CtBehavior;
import javassist.CtMethod;
import javassist.Modifier;
import javassist.NotFoundException;
import javassist.expr.ExprEditor;
import javassist.expr.MethodCall;

public class MethodExecutionTimer extends AbstractJavassistTool {

    static int totalNativeCalls = 0;
    static int actualNativeCalls = 0;

    public MethodExecutionTimer(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);
    }

    @Override
    protected void transform(CtBehavior behavior) throws Exception {
        super.transform(behavior);

        // Now instrument the method to count native method calls
        behavior.instrument(new ExprEditor() {
            public void edit(MethodCall m) throws CannotCompileException {
                CtMethod calleeMethod;
                int mod = -1;

                try {
                    calleeMethod = m.getMethod();
                    mod = calleeMethod.getModifiers();
                } catch (NotFoundException e) {
                    String methodName = m.getMethodName();
                    String className = m.getClassName();
                    System.out.println(String.format("Warning: unable to check modifier for %s:%s", className, methodName));
                    return;
                }

                // Check if the method is native
                if (mod != -1 && Modifier.isNative(mod)) {
                    totalNativeCalls++;
                    String calleeMethodName = calleeMethod.getName();
                    String calleeClassName = calleeMethod.getDeclaringClass().getName();
                    
                    // Exclude system library calls
                    if (calleeClassName.startsWith("java.") ||
                    calleeClassName.startsWith("javax.") ||
                    calleeClassName.startsWith("jdk.") ||
                    calleeClassName.startsWith("org.") ||
                    calleeClassName.startsWith("sun.") ||
                    calleeClassName.startsWith("com.sun.")) {
                        return;
                    }
                    
                    actualNativeCalls++;
                    // Measure the execution time of the native method
                    String timerName = "timer_" + calleeMethodName;
                    m.replace("{ " +
                        "long " + timerName + " = System.nanoTime(); " +
                        "try { $_ = $proceed($$); } finally { " +
                            "long endTime = System.nanoTime();" +
                            "System.out.println(\"" + timerName + " took \" + " +
                            "(endTime - " + timerName + ") + \" ns\"); " +
                    "}}");
                }
            }
        });
    }

    public void printNativeCallCounts() {
        System.out.println("Actual/total Native Calls: " + actualNativeCalls + "/" + totalNativeCalls);
    }
}
