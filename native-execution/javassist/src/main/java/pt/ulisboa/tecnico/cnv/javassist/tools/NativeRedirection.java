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
import javassist.CtConstructor;
import javassist.Modifier;
import javassist.NotFoundException;
import javassist.bytecode.BadBytecode;
import javassist.bytecode.SignatureAttribute;
import javassist.expr.ExprEditor;
import javassist.expr.MethodCall;

import java.lang.System;
import java.nio.file.Files;
public class NativeRedirection extends CodeDumper {

    private final static String application_id = generateUniqueId();

    public NativeRedirection(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);
    }

    @Override
    protected void transform(CtBehavior behavior) throws Exception {
        super.transform(behavior); 

        behavior.instrument(new ExprEditor() {
            public void edit(MethodCall m) throws CannotCompileException {
                try {
                    CtClass clazz = behavior.getDeclaringClass();
                    CtMethod method = m.getMethod();
                    String methodClassName = m.getClassName();
                    String methodSignature = method.getSignature();
                    String methodName = m.getMethodName();

                    if (isLoadLibrary(clazz, methodName, methodSignature)) {
                        m.replace("{ $1 = \"" + application_id + ":\" + $1; java.io.File file = new java.io.File(\"lib\" + $1 + \".so\"); file.createNewFile(); $proceed($$); }");

                        //m.replace("{ $1 = \"" + application_id + ":\" + $1; $proceed($$); }");
                    }
                    else if (Modifier.isNative(method.getModifiers()) && !isInternalClass(methodClassName)) {
                        CtClass returnType = method.getReturnType();
                        String returnJniType = getJniType(returnType.getName());

                        String[] params = getParameterTypes(methodSignature);
                        String[] jniTypes = Arrays.stream(params)
                            .map(param -> getJniType(param))
                            .toArray(String[]::new);
                        
                        if (!isCallGateDeclared(clazz, methodSignature)) {
                            CtConstructor staticInitializer = clazz.makeClassInitializer();        
                            staticInitializer.insertBefore("System.loadLibrary(\"" + methodName + "\");");
                            declareCallGate(clazz, params, returnType);
                            createHeader(jniTypes, returnJniType, methodClassName);
                            createSnippet(jniTypes, returnJniType, methodName, methodClassName);
                        }
                        
                        boolean voidType = returnType.getName().equals("void");
                        m.replace((!voidType ? "$_=" : "") + "nativeCallGate($$);");
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

    public static boolean isLoadLibrary(CtClass clazz, String name, String signature) throws NotFoundException {
        ClassPool cp = clazz.getClassPool();
        CtClass systemClass = cp.get("java.lang.System");
        CtMethod loadLibMethod =  systemClass.getDeclaredMethod("loadLibrary");
        String loadLibSignature = loadLibMethod.getSignature();

        return name.equals("loadLibrary") && signature.equals(loadLibSignature);
    }

    public static void createHeader(String[] jniTypes, String returnJniType, String className) throws IOException {
        File file = new File("snippets", className + ".h");
        
        if (file.exists()) {
            List<String> lines = Files.readAllLines(file.toPath());

            // Keep only the first lines (removing the last 4 lines)
            lines.subList(Math.max(0, lines.size() - 4), lines.size()).clear();

            // Add new method
            lines.add("JNIEXPORT " + returnJniType + " JNICALL Java_" + className + "_nativeCallGate\n");
            lines.add("\t(JNIEnv *, jclass" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ");\n\n");
            lines.add("#ifdef __cplusplus\n");
            lines.add("}\n");
            lines.add("#endif\n");
            lines.add("#endif\n");  

            // Write the modified lines back to the file
            Files.write(file.toPath(), lines);
        }
        else {
            try (FileWriter writer = new FileWriter(file)) {
                writer.write("#include <jni.h>\n\n");
                writer.write("#ifndef _Included_" + className + "\n");
                writer.write("#define _Included_" + className + "\n");
                writer.write("#ifdef __cplusplus\n");
                writer.write("extern \"C\" {\n");
                writer.write("#endif\n\n");
                writer.write("JNIEXPORT " + returnJniType + " JNICALL Java_" + className + "_nativeCallGate\n");
                writer.write("\t(JNIEnv *, jclass" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ");\n\n");
                writer.write("#ifdef __cplusplus\n");
                writer.write("}\n");
                writer.write("#endif\n");
                writer.write("#endif\n");  
            }
        }
    }

    public static void createSnippet(String[] jniTypes, String returnJniType, String methodName, String className) throws IOException {
        System.out.println("Creating Snippet for " + methodName + " from class " + className + "...");

        // parameters for call gate
        String[] args = Arrays.stream(jniTypes)
            .map(i -> generateUniqueId())
            .toArray(String[]::new);   

        String typeArgs = IntStream.range(0, args.length)
            .mapToObj(i -> jniTypes[i] + " " + args[i])
            .collect(Collectors.joining(", ",  args.length > 0 ? ", " : "", ""));

        String nativeMethodName = "Java_" + className + "_" + methodName;
        String arguments = "env, obj" + (args.length > 0 ? ", " : "") + String.join(", ", args);
        String mc = "nativeMethod(" + arguments + ");\n";

        File file = new File("snippets", methodName + ".c");
        try (FileWriter writer = new FileWriter(file)) {
            writer.write("#define _GNU_SOURCE\n");
            writer.write("#include <stdio.h>\n");
            writer.write("#include <preload.h>\n");
            writer.write("#include \"" + className + ".h\"\n\n");

            writer.write("static __thread char* regular = NULL;\n\n");
            writer.write(returnJniType + " wrapper(int domain, JNIEnv *env, jobject obj" + typeArgs + ");\n\n");
            
            writer.write("JNIEXPORT " + returnJniType + " JNICALL Java_" + className + "_nativeCallGate(JNIEnv *env, jobject obj" + typeArgs + ") {\n");
            writer.write("\t// Get available domain\n");
            writer.write("\tpthread_mutex_lock(&mutex);\n");
            writer.write("\tint domain = findApp(\"lib" + application_id + "\");\n");
            writer.write("\twhile (domain == -1) {\n");
            writer.write("\t\t//FIXME: active waiting\n");
            writer.write("\t\tsleep(1);\n");
            writer.write("\t\tdomain = findEmptyDomain();\n");
            writer.write("\t}\n\n");
            writer.write("\tinsertThreadInMap(domain);\n\n");
            writer.write("\t#ifndef EAGER_LOAD\n");
            writer.write("\tchar* app = getApp(domain);\n");
            writer.write("\tif (strcmp(app, \"lib" + application_id + "\")) {\n");
            writer.write("\t\tsetAppPermissions(app, PROT_NONE, domain);\n");
            writer.write("\t\tinsertApp(domain, \"lib" + application_id + "\");\n");
            writer.write("\t\tsetAppPermissions(\"lib" + application_id + "\", PROT_READ|PROT_WRITE|PROT_EXEC, domain);\n\n");
            writer.write("\t}\n");
            writer.write("\t#endif\n\n");
            writer.write("\tpthread_mutex_unlock(&mutex);\n");

            writer.write("\t// Switch to new stack\n");
            writer.write("\tERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(domain), regular);\n");

            if (returnJniType.equals("void")) {
                writer.write("\twrapper(domain, " + arguments + ");\n");
                writer.write("\tERIM_SWITCH_BACK(regular);\n");
                writer.write("}\n\n");
                
                writer.write(returnJniType + " wrapper(int domain, JNIEnv *env, jobject obj" + typeArgs + ") {\n");
                writer.write("\tvoid (*nativeMethod)(JNIEnv*, jobject" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ") = dlsym(RTLD_DEFAULT, \"" + nativeMethodName + "\");\n");
                writer.write("\tif (nativeMethod == NULL) {\n");
                writer.write("\t\tfprintf(stderr, \"Failed to find the symbol: " + nativeMethodName + "\\n\");\n");
                writer.write("\t\texit(EXIT_FAILURE);\n");
                writer.write("\t}\n\n");
                
                writer.write("\t#ifndef EAGER_LOAD\n");
                writer.write("\tsetAppPermissions(\"lib" + application_id + "\", PROT_READ|PROT_WRITE|PROT_EXEC, domain);\n\n");
                writer.write("\t#endif\n");

                writer.write("\t__wrpkru(ERIM_DOMAIN(domain));\n");
                writer.write("\t" + mc);
                writer.write("void joinThreads(domain);\n\n");
                
                writer.write("\t// Undo previous changes\n");
                writer.write("\t__wrpkru(ERIM_DOMAIN(0));\n"); //FIXME: This should be above joinThreads?
                writer.write("\t#ifdef EAGER_LOAD\n");
                writer.write("\tsetAppPermissions(\"lib" + application_id + "\", PROT_NONE, domain);\n");
                writer.write("\t#endif\n");
                writer.write("}\n");
            }
            else {
                writer.write("\t" + returnJniType + " res = wrapper(" + arguments + ");");
                writer.write("\tERIM_SWITCH_BACK(regular);\n");
                writer.write("\treturn res;\n");
                writer.write("}\n\n");
                
                writer.write(returnJniType + " wrapper(int domain, JNIEnv *env, jobject obj" + typeArgs + ") {\n");
                writer.write("\tvoid (*nativeMethod)(JNIEnv*, jobject" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ") = dlsym(RTLD_DEFAULT, \"" + nativeMethodName + "\");\n");
                writer.write("\tif (nativeMethod == NULL) {\n");
                writer.write("\t\tfprintf(stderr, \"Failed to find the symbol: " + nativeMethodName + "\\n\");\n");
                writer.write("\t\texit(EXIT_FAILURE);\n");
                writer.write("\t}\n\n");
                
                writer.write("\t#ifndef EAGER_LOAD\n");
                writer.write("\tsetAppPermissions(\"lib" + application_id + "\", PROT_READ|PROT_WRITE|PROT_EXEC, domain);\n\n");
                writer.write("\t#endif\n");
                
                writer.write("\t__wrpkru(ERIM_DOMAIN(domain));\n");
                writer.write("\t" + returnJniType + " res = " + mc);
                writer.write("void joinThreads(domain);\n\n");

                writer.write("\t// Undo previous changes\n");
                writer.write("\t__wrpkru(ERIM_DOMAIN(0));\n"); //FIXME: This should be above joinThreads?
                writer.write("\t#ifdef EAGER_LOAD\n");
                writer.write("\tsetAppPermissions(\"lib" + application_id + "\", PROT_NONE, domain);\n");
                writer.write("\t#endif\n");
                writer.write("\treturn res;\n");
                writer.write("}\n");
            }
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

    public static void declareCallGate(CtClass clazz, String[] parameters, CtClass returnType) throws NotFoundException, CannotCompileException {
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

        CtMethod nativeMethod = new CtMethod(returnType, "nativeCallGate", parameterTypes, clazz);
        nativeMethod.setModifiers(Modifier.PUBLIC | Modifier.STATIC | Modifier.NATIVE);

        clazz.addMethod(nativeMethod);        
    }
}
