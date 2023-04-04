package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.util.List;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import javassist.CannotCompileException;
import javassist.CtBehavior;
import javassist.CtMethod;
import javassist.CtClass;
import javassist.CtConstructor;
import javassist.Modifier;
import javassist.NotFoundException;
import javassist.expr.ExprEditor;
import javassist.expr.MethodCall;

public class NativeRedirection extends CodeDumper {

    public NativeRedirection(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);
    }

    @Override
    protected void transform(CtBehavior behavior) throws Exception {
        super.transform(behavior); 

        String bName = behavior.getName();
        String cName = behavior.getDeclaringClass().getName();
        
        
        behavior.instrument(new ExprEditor() {
            public void edit(MethodCall m) throws CannotCompileException {
                if (bName.equals("loadLibrary")) {
                    if (cName.equals("java.lang.System")) { System.out.println("APANHEI"); }
                    else if (cName.equals("Java.lang.Runtime")) {}
                    else if (cName.equals("com.sun.jna.Native")) {}
                    else if (cName.equals("org.tensorflow.TensorFlow")) {}
                }
                else if (bName.equals("load")) {
                    if (cName.equals("java.lang.System")) {} 
                    if (cName.equals("org.tensorflow.NativeLibrary")) {} 
                }
                
                
                CtMethod method = null;
                try {
                    method = m.getMethod();
                } catch (NotFoundException e) {}
                
                if (method != null && Modifier.isNative(method.getModifiers())) {
                    String methodName = m.getMethodName();
                        
                    m.replace("{ Object[] args = $args; System.out.println(\"" + methodName + "(\" + java.util.Arrays.toString(args) + \")\"); $_ = $proceed($$); }");
                    
                    //File file = new File("snippet.c");
                    //try (FileWriter writer = new FileWriter(file)) {
                    //    writer.write("Hello, world!");
                    //} catch (IOException e) {
                    //    System.err.println("Failed to write to file: " + e.getMessage());
                    //}
                }
            }
        });
    }
}
