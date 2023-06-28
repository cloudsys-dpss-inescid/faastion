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

    public MethodExecutionTimer(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);
    }

    @Override
    protected void transform(CtBehavior behavior) throws Exception {
        super.transform(behavior);

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
                    System.out.println(String.format("Warning unable to check modifier for %s:%s", className, methodName));
                    return;
                }

                if (mod != -1 && Modifier.isNative(mod)) {
                    String callerClassName = m.getEnclosingClass().getName();
                    String callerMethodName = m.where().getName();
                    String calleeMethodName = calleeMethod.getName();
                    String calleeClassName = calleeMethod.getDeclaringClass().getName();
                    if (calleeClassName.startsWith("java.") ||
                            calleeClassName.startsWith("javax.") ||
                            calleeClassName.startsWith("jdk.") ||
                            calleeClassName.startsWith("com.sun.")) {
                        System.out.println(String.format("Ignoring method call %s:%s -> %s:%s", callerClassName, callerMethodName, calleeClassName, calleeMethodName));
                        return;
                    }

                    String timerName = "timer_" + calleeMethodName;
                    m.replace("{ long " + timerName + " = System.nanoTime(); " +
                            "try { $_ = $proceed($$); } finally { " +
                            "long endTime = System.nanoTime();" +
                            "System.out.println(\"" + timerName + " took \" + " +
                            "(endTime - " + timerName + ") + \" ns\"); }}");

                    System.out.println(String.format("Wrapped method call %s:%s -> %s:%s", callerClassName, callerMethodName, calleeClassName, calleeMethodName));
                }
            }
        });
    }
}
