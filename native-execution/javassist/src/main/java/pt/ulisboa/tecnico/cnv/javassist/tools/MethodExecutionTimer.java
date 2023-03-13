package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.util.List;

import javassist.CannotCompileException;
import javassist.CtBehavior;
import javassist.CtClass;
import javassist.Modifier;
import javassist.NotFoundException;
import javassist.expr.ExprEditor;
import javassist.expr.MethodCall;

public class MethodExecutionTimer extends CodeDumper {

    public MethodExecutionTimer(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);
    }

    @Override
    protected void transform(CtBehavior behavior) throws Exception {
        super.transform(behavior); 

        behavior.instrument(new ExprEditor() {
            public void edit(MethodCall m) throws CannotCompileException {
                int mod = -1;

                try {
                    mod = m.getMethod().getModifiers();
                } catch (NotFoundException e) {}

                if (mod != -1 && Modifier.isNative(mod)) {
                    String methodName = m.getMethodName();
                    //String className = m.getClassName();

                    String timerName = "timer_" + methodName;
                    m.replace("{ long " + timerName + " = System.nanoTime(); " +
                            "try { $_ = $proceed($$); } finally { " +
                            "long endTime = System.nanoTime();" +
                            "System.out.println(\"" + timerName + " took \" + " +
                            "(endTime - " + timerName + ") + \" ns\"); }}");

                    //System.out.println("Wrapped method " + methodName + " in class " + className + " with timer " + timerName);
                }
            }
        });
    }
}
