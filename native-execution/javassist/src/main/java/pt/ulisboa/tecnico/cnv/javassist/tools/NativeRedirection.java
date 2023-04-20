package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.util.Arrays;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

import javassist.CannotCompileException;
import javassist.ClassPool;
import javassist.CtBehavior;
import javassist.CtMethod;
import javassist.CtClass;
import javassist.Modifier;
import javassist.NotFoundException;
import javassist.bytecode.BadBytecode;
import javassist.bytecode.SignatureAttribute;
import javassist.expr.ExprEditor;
import javassist.expr.MethodCall;

import java.lang.System;
public class NativeRedirection extends CodeDumper {

    public NativeRedirection(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);
    }

    @Override
    protected void transform(CtBehavior behavior) throws Exception {
        super.transform(behavior); 

        behavior.instrument(new ExprEditor() {
            public void edit(MethodCall m) throws CannotCompileException {
                try {
                    CtMethod method = m.getMethod();
                    String className = m.getClassName();
                    
                    if (Modifier.isNative(method.getModifiers()) && !isInternalClass(className)) {
                        CtClass clazz = behavior.getDeclaringClass();
                        String[] params = getParameterTypes(method.getSignature());
                        String[] jniTypes = Arrays.stream(params)
                            .map(param -> getJniType(param))
                            .toArray(String[]::new);

                        if (!isCallGateDeclared(clazz, method.getSignature()))
                            declareCallGate(clazz, params);

                        createHeader(jniTypes, className);
                        createSnippet(jniTypes, m.getMethodName(), className);
                        //m.replace("{ $_ = nativeCallGate($$); }");
                    }
                } catch (NotFoundException e) { 
                    System.err.println(e.getMessage()); 
                } catch (BadBytecode e) {
                    System.err.println(e.getMessage()); 
                } catch (IOException e) {
                    System.err.println(e.getMessage());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    public static void createHeader(String[] jniTypes, String className) throws IOException {
        File file = new File("gen-snippets", className + ".h");

        try (FileWriter writer = new FileWriter(file)) {
            writer.write("#include <jni.h>\n\n");
            writer.write("#ifndef _Included_" + className + "\n");
            writer.write("#define _Included_" + className + "\n");
            writer.write("#ifdef __cplusplus\n");
            writer.write("extern \"C\" {\n");
            writer.write("#endif\n\n");
            writer.write("JNIEXPORT void JNICALL Java_" + className + "_nativeCallGate\n");
            writer.write("\t(JNIEnv *, jclass" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ")\n\n");
            writer.write("#ifdef __cplusplus\n");
            writer.write("}\n");
            writer.write("#endif\n");
            writer.write("#endif\n");  
        }
    }

    public static void createSnippet(String[] jniTypes, String methodName, String className) throws IOException {
        // parameters for call gate
        String[] args = Arrays.stream(jniTypes)
            .map(i -> generateUniqueId())
            .toArray(String[]::new);   

        String typeArgs = IntStream.range(0, args.length)
            .mapToObj(i -> jniTypes[i] + " " + args[i])
            .collect(Collectors.joining(", ",  args.length > 0 ? ", " : "", ""));
        
        File file = new File("gen-snippets", methodName + ".c");

        try (FileWriter writer = new FileWriter(file)) {
            writer.write("#include <jni.h>\n");
            writer.write("#include \"" + className + ".h\"\n");
            writer.write("#include \"../common/common.h\"\n");
            writer.write("#include \"../erim/erim.h\"\n\n");
            writer.write("JNIEXPORT void JNICALL Java_" + className + "_nativeCallGate(JNIEnv *env, jobject obj" + typeArgs + ") {\n");
            writer.write("\terim_switch_to_untrusted;\n");
            writer.write("\t" + methodName + "(env, obj" + (args.length > 0 ? ", " : "") + String.join(", ", args) + ");\n");
            writer.write("\terim_switch_to_trusted;\n");
            writer.write("}\n");
        }
    }
    
    public static String generateUniqueId() {
        UUID uuid = UUID.randomUUID();
        return "c" + uuid.toString().replaceAll("-", "");
    }

    public static String[] getParameterTypes(String signature) throws Exception {
        SignatureAttribute.MethodSignature methodSignature = SignatureAttribute.toMethodSignature(signature);

        String[] parameterTypes = Arrays.stream(methodSignature.getParameterTypes())
                .map(SignatureAttribute.Type::toString)
                .toArray(String[]::new);

        return parameterTypes;
    }

    public static String getJniType(String parameterType) {
        switch (parameterType) {
            case "boolean":
            case "java.lang.Boolean":
                return "jboolean";
            case "byte":
            case "java.lang.Byte":
                return "jbyte";
            case "char":
            case "java.lang.Character":
                return "jchar";
            case "short":
            case "java.lang.Short":
                return "jshort";
            case "int":
            case "java.lang.Integer":
                return "jint";
            case "long":
            case "java.lang.Long":
                return "jlong";
            case "float":
            case "java.lang.Float":
                return "jfloat";
            case "double":
            case "java.lang.Double":
                return "jdouble";
            case "object[]":
            case "java.lang.Object[]":
                return "jobjectArray";
            case "boolean[]":
                return "jbooleanArray";
            case "byte[]":
                return "jbyteArray";
            case "char[]":
                return "jcharArray";
            case "shot[]":
                return "jshortArray";
            case "int[]":
                return "jintArray";
            case "long[]":
                return "jlongArray";
            case "float[]":
                return "jfloatArray";
            case "double[]":
                return "jdoubleArray";
            case "void":
                return "void";
            case "java.lang.String":
                return "jstring";
            case "java.lang.Class":
                return "jclass";
            case "java.lang.Object":
                return "jobject";
            case "java.lang.Throwable":
                return "jthrowable";
            default:
                throw new IllegalArgumentException("Unsupported parameter type: " + parameterType);
        }
    }

    public static boolean isCallGateDeclared(CtClass clazz, String signature) {
        CtMethod[] declaredMethods = clazz.getDeclaredMethods();

        for (CtMethod method : declaredMethods) {
            if (method.getName().equals("nativeCallGate") && method.getSignature().equals(signature)) {
                return true;
            }
        }

        return false;
    }

    public static boolean isInternalClass(String className) {
        return className.startsWith("com.sun.") ||
               className.startsWith("java.") ||
               className.startsWith("sun.") ||
               className.startsWith("jdk.") ||
               className.startsWith("org.");
    }

    public static void declareCallGate(CtClass clazz, String[] parameters) throws NotFoundException, CannotCompileException {
        ClassPool cp = clazz.getClassPool();

        CtClass[] parameterTypes = Arrays.stream(parameters)
                .map(parameter -> {
                    try {
                        return cp.get(parameter);
                    } catch (NotFoundException e) {
                        return null;
                    }
                })
                .toArray(CtClass[]::new);

        if (Arrays.asList(parameterTypes).contains(null)) {
            throw new NotFoundException("One or more parameter types not found.");
        }

        CtMethod nativeMethod = new CtMethod(CtClass.voidType, "nativeCallGate", parameterTypes, clazz);
        nativeMethod.setModifiers(Modifier.PUBLIC | Modifier.STATIC | Modifier.NATIVE);

        clazz.addMethod(nativeMethod);        
    }
}
