package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.IllegalClassFormatException;
import java.security.ProtectionDomain;
import java.util.List;

import javassist.ClassPool;
import javassist.CtBehavior;
import javassist.CtClass;
import javassist.bytecode.AccessFlag;

public abstract class AbstractJavassistTool implements ClassFileTransformer {
    private List<String> packageNameList;

    private String writeDestination;

    public AbstractJavassistTool(List<String> packageNameList, String writeDestination) {
        this.packageNameList = packageNameList;
        this.writeDestination = writeDestination;
    }

    protected void transform(CtBehavior behavior) throws Exception {
        printNativeCallCounts();
    }

    public void printNativeCallCounts() {
    }

    protected void transform(CtClass clazz) throws Exception {
        for (CtClass nestedClazz : clazz.getDeclaredClasses()) {
            transform(nestedClazz);
        }
        for (CtBehavior behavior : clazz.getDeclaredBehaviors()) {
            if ((AccessFlag.ABSTRACT & behavior.getModifiers()) == 0) {
                transform(behavior);
            }
        }
    }

    final public byte[] transform(String canonicalClassName) {
        byte[] bytecode = null;
        CtClass cc = null;

        try {
            cc = ClassPool.getDefault().get(canonicalClassName);
            transform(cc);
            cc.writeFile(writeDestination);
            bytecode = cc.toBytecode();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            cc.detach();
        }

        return bytecode;
    }

    @Override
    final public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined, ProtectionDomain protectionDomain, byte[] classfileBuffer) throws IllegalClassFormatException {
        String canonicalClassName = className.replaceAll("/", "\\.");

        for (String packageName : packageNameList) {
            if (canonicalClassName.startsWith(packageName)) {
                return transform(canonicalClassName);
            }
        }
        return classfileBuffer;
    }
}