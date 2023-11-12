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
                    String gateName = methodName + "CallGate";

                    if (Modifier.isNative(method.getModifiers()) && !isInternalClass(methodClassName)) {
                        CtClass returnType = method.getReturnType();
                        String returnJniType = getJniType(returnType.getName());

                        String[] params = getParameterTypes(methodSignature);
                        String[] jniTypes = Arrays.stream(params)
                            .map(param -> getJniType(param))
                            .toArray(String[]::new);
                        
                        if (!isCallGateDeclared(clazz, methodSignature, gateName)) {
                            CtConstructor staticInitializer = clazz.makeClassInitializer();        
                            staticInitializer.insertBefore("System.loadLibrary(\"" + methodName + "\");");
                            declareCallGate(clazz, params, returnType, gateName);

                            if (methodClassName.contains(".")) {
                                methodClassName = methodClassName.replace(".", "_");
                            }

                            createHeader(jniTypes, returnJniType, methodClassName, gateName);
                            createSnippet(jniTypes, returnJniType, methodName, methodClassName, gateName);
                        }
                        
                        boolean voidType = returnType.getName().equals("void");
                        m.replace((!voidType ? "$_=" : "") + gateName + "($$);");
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

    public static void createHeader(String[] jniTypes, String returnJniType, String className, String gateName) throws IOException {
        File file = new File(System.getenv("SNIPPETS_DIR"), className + ".h");
        
        if (file.exists()) {
            List<String> lines = Files.readAllLines(file.toPath());

            // Keep only the first lines (removing the last 4 lines)
            lines.subList(Math.max(0, lines.size() - 1), lines.size()).clear();

            // Add new method
            lines.add("JNIEXPORT " + returnJniType + " JNICALL Java_" + className + "_" + gateName + "\n");
            lines.add("\t(JNIEnv *, jclass" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ");\n\n");

            lines.add("#endif\n");

            // Write the modified lines back to the file
            Files.write(file.toPath(), lines);
        }
        else {
            try (FileWriter writer = new FileWriter(file)) {
                writer.write("#include <jni.h>\n\n");

                writer.write("#ifndef _Included_" + className + "\n");
                writer.write("#define _Included_" + className + "\n\n");
                
                writer.write("#ifdef SNI_DBG\n");
                writer.write("#define SNI_DBM(...) \\\n");
                writer.write("\tdo { \\\n");
                writer.write("\tfprintf(stderr, __VA_ARGS__); \\\n");
                writer.write("\tfprintf(stderr, \"\\n\"); \\\n");
                writer.write("\t} while(0)\n");
                writer.write("#else // disable debug\n");
                writer.write("#define SNI_DBG(...)\n");
                writer.write("#endif\n\n");

                writer.write("JNIEXPORT " + returnJniType + " JNICALL Java_" + className + "_" + gateName + "\n");
                writer.write("\t(JNIEnv *, jclass" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ");\n\n");

                writer.write("#endif\n");  
            }
        }
    }

    public static void createSnippet(String[] jniTypes, String returnJniType, String methodName, String className, String gateName) throws IOException {
        // parameters for call gate
        String[] arguments = IntStream.range(0, jniTypes.length)
            .mapToObj(i -> "arg" + i)
            .toArray(String[]::new);   

        String typeArgs = IntStream.range(0, arguments.length)
            .mapToObj(i -> jniTypes[i] + " " + arguments[i])
            .collect(Collectors.joining(", ",  arguments.length > 0 ? ", " : "", ""));

        String nativeMethodName = "Java_" + className + "_" + methodName;
        String args = "env, obj" + (arguments.length > 0 ? ", " : "") + String.join(", ", arguments);
        String mc = "native_method(" + args + ");\n";

        File file = new File(System.getenv("SNIPPETS_DIR"), methodName + ".c");
        try (FileWriter writer = new FileWriter(file)) {
            writer.write("#include <" + System.getenv("ENV") + ".h>\n");
            writer.write("#include <unistd.h>\n");
            writer.write("#include <stdlib.h>\n");
            writer.write("#include \"" + className + ".h\"\n\n");

            writer.write("// Erim includes\n");
            writer.write("#include <erim.h>\n");
            writer.write("#include <common.h>\n\n");


            writer.write("static __thread char* regular = NULL; // thread regular stack\n");
            writer.write("static __thread int hasFilter = 0; // seccomp filter is applied\n\n");
            
            writer.write("/* Function declaration */\n");
            writer.write(returnJniType + " wrapper(int domain, JNIEnv *env, jobject obj" + typeArgs + ");\n\n");

            writer.write("JNIEXPORT " + returnJniType + " JNICALL Java_" + className + "_" + gateName + "(JNIEnv *env, jobject obj" + typeArgs + ") {\n");
            writer.write("\tlock();\n\n");

            writer.write("\t/* Get available domain */\n");
            writer.write("\tSNI_DBM(\"[s]: Getting available domain...\");\n");
            writer.write("\tint domain = find_app_domain(\"" + System.getenv("BENCHMARK_NAME") + "\");\n");
            writer.write("\twhile (domain == -1) {\n");
            writer.write("\t\t//FIXME: active waiting\n");
            writer.write("\t\tsleep(1);\n");
            writer.write("\t\tdomain = find_empty_domain();\n");
            writer.write("\t}\n\n");
            
            writer.write("\t/* Handle native library permissions */\n");
            writer.write("\tSNI_DBM(\"[s]: Handling permissions for domain %d...\", domain);\n");
            writer.write("\tupdate_supervisor_app(domain, \"" + System.getenv("BENCHMARK_NAME") + "\");\n");
            writer.write("\tsignal_perms(domain);\n\n");

            writer.write("\t/* Wait for all permissions to be set */\n");
            writer.write("\twait_set(domain);\n");
            writer.write("\tSNI_DBM(\"[s]: permissions are all set, continuing...\");\n\n");

            writer.write("\tunlock();\n\n");

            writer.write("\t/* Switch to new stack */\n");
            writer.write("\tSNI_DBM(\"[s]: switching to new stack...\");\n");
            writer.write("\tERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(domain), regular);\n");

            if (returnJniType.equals("void")) {
                writer.write("\twrapper(domain, " + args + ");\n");
                writer.write("\tERIM_SWITCH_BACK(regular);\n\n");

                writer.write("\t/* Notify supervisor of the app's completion */\n");
                writer.write("\tSNI_DBM(\"[s]: application terminated!\");\n");
                writer.write("\tupdate_supervisor_status(domain);\n");
            }
            else {
                writer.write("\t" + returnJniType + " res = wrapper(" + args + ");");
                writer.write("\tERIM_SWITCH_BACK(regular);\n\n");

                writer.write("\t/* Notify supervisor of the app's completion */\n");
                writer.write("\tupdate_supervisor_status(domain);\n\n");

                writer.write("\treturn res;\n");
            }
            writer.write("}\n\n\n");
                
            writer.write(returnJniType + " wrapper(int domain, JNIEnv *env, jobject obj" + typeArgs + ") {\n");
            writer.write("\tvoid (*native_method)(JNIEnv*, jobject" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ") = dlsym(RTLD_DEFAULT, \"" + nativeMethodName + "\");\n");
            writer.write("\tif (native_method == NULL) {\n");
            writer.write("\t\tfprintf(stderr, \"Failed to find the symbol: " + nativeMethodName + "\\n\");\n");
            writer.write("\t\texit(EXIT_FAILURE);\n");
            writer.write("\t}\n\n");

            writer.write("\t/* Install seccomp filter */\n");
            writer.write("\tSNI_DBM(\"[s]: installing filter...\");\n");
            writer.write("\tif (!hasFilter) {\n");
            writer.write("\t\tinstall_notify_filter(domain);\n");
            writer.write("\t\thasFilter = 1;\n");
            writer.write("\t}\n");
            writer.write("\tsignal_filter(domain);\n\n");

            writer.write("\twait_handler(domain);\n");
            writer.write("\tSNI_DBM(\"[s]: handler's ready, changing domain...\");\n");
            writer.write("\t__wrpkrumem(ERIM_DOMAIN(domain));\n");
            if (returnJniType.equals("void")) {
                writer.write("\t" + mc);                
                writer.write("\t__wrpkru(0);\n");
            }
            else {
                writer.write("\t" + returnJniType + " res = " + mc);
                writer.write("\t__wrpkru(0);\n\n");

                writer.write("\treturn res;\n");
            }
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

    public static boolean isCallGateDeclared(CtClass clazz, String signature, String gateName) {
        CtMethod[] declaredMethods = clazz.getDeclaredMethods();

        for (CtMethod method : declaredMethods) {
            if (method.getName().equals(gateName) && method.getSignature().equals(signature)) {
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

    public static void declareCallGate(CtClass clazz, String[] parameters, CtClass returnType, String gateName) throws NotFoundException, CannotCompileException {
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

        CtMethod nativeMethod = new CtMethod(returnType, gateName, parameterTypes, clazz);
        nativeMethod.setModifiers(Modifier.PUBLIC | Modifier.STATIC | Modifier.NATIVE);

        clazz.addMethod(nativeMethod);        
    }
}