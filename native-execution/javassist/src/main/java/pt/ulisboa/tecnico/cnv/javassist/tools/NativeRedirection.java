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
		writer.write("#define SNI_DBM(...)\n");
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
	String native_args = "NULL, NULL"+ (arguments.length > 0 ? ", " : "") + String.join(", ", arguments);

	String mc = "native_method(" + args + ");\n";
	String nmc = "native_method(" + native_args + ");\n";

	File file = new File(System.getenv("SNIPPETS_DIR"), methodName + ".c");
	try (FileWriter writer = new FileWriter(file)) {

	    writer.write("#include <" + System.getenv("ENV") + ".h>\n");
	    writer.write("#include <unistd.h>\n");
	    writer.write("#include <stdlib.h>\n");
	    writer.write("#include <time.h>\n");
	    writer.write("#include <spawn.h>\n");
	    writer.write("#include <fcntl.h>\n");
	    writer.write("#include <string.h>\n");
	    writer.write("#include <stdio.h>\n");
	    if (jniTypes.length > 0) {
		writer.write("#include \"JNIWrapper.h\"\n");
	    }
	    writer.write("#include \"" + className + ".h\"\n\n");

	    writer.write("// Erim includes\n");
	    writer.write("#include <erim.h>\n");
	    writer.write("#include <common.h>\n\n");
	    writer.write("#define BUFFER_SIZE 1024\n");
	    /*
TODO: TRY TO MAKE THIS WORK!!
writer.write("static __thread " + returnJniType + " (JNICALL *native_method)(JNIEnv *env, jobject obj" + typeArgs + ") = NULL;\n");
	     */

	    writer.write("static __thread char* regular = NULL; // thread regular stack\n");
	    writer.write("static __thread int fd = 0; // seccomp filter fd\n\n");

	    writer.write("/* Function declaration */\n");
	    writer.write("void lazy_proc_isolation(void);");
	    writer.write(returnJniType + " wrapper(JNIEnv *env, jobject obj" + typeArgs + ");\n");
	    writer.write("JNIEXPORT " + returnJniType + " JNICALL Java_" + className + "_" + gateName + "(JNIEnv *env, jobject obj" + typeArgs + ") {\n");
	    writer.write("\t/* Get available domain */\n");
	    writer.write("\tSNI_DBM(\"[s]: Getting available domain...\");\n");
	    writer.write("\tacquire_domain(\"" + System.getenv("BENCHMARK_NAME") + "\", &fd);\n");
	    writer.write("\tif(domain == -1) {\n");
	    writer.write("\t\tlazy_proc_isolation();\n");
	    writer.write("\t\treturn;\n");
	    writer.write("\t\tchar fifo_path[30];\n");  
	    writer.write("\t\tint idx = atomic_fetch_add(&shared_variable, 1);\n");  
	    writer.write("\t\tsnprintf(fifo_path, sizeof(fifo_path), \"/tmp/fifo/fifo_%d\", procIDs[idx % NUM_PROCESSES]);\n");
	    writer.write("\t\tint fd = open(fifo_path, O_WRONLY);\n");
	    writer.write("\t\tif(fd == -1){\n");
	    writer.write("\t\t\tperror(\"Error Opening FIFO\");\n");
	    writer.write("\t\t\texit(EXIT_FAILURE);\n");
	    writer.write("\t\t};\n");
	    writer.write("\t\tchar buffer[BUFFER_SIZE];\n");
	    writer.write("\t\tchar string1[] = \"lib" + System.getenv("BENCHMARK_NAME") +"-jni.so\";\n");
	    writer.write("\t\tchar string2[] = \""+ nativeMethodName + "\";\n");
	    writer.write("\t\tsnprintf(buffer, sizeof(buffer),\"%s,%s\",string1, string2);\n");
	    writer.write("\t\twrite(fd, buffer, strlen(buffer));\n");
	    writer.write("\t\tclose(fd);\n");
	    writer.write("\t\treturn;\n");
	    writer.write("\t}\n\n");

	    writer.write("\t/* Switch to new stack */\n");
	    writer.write("\tSNI_DBM(\"[s]: switching to new stack...\");\n");
	    writer.write("\tERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(domain), regular);\n");

	    if (returnJniType.equals("void")) {
		writer.write("\twrapper(" + args + ");\n");
		writer.write("\tERIM_SWITCH_BACK(regular);\n\n");

		writer.write("\tSNI_DBM(\"[s]: application terminated!\");\n");
		writer.write("\treset_env(\"" + System.getenv("BENCHMARK_NAME") + "\", 0);\n");
	    }
	    else {
		writer.write("\t" + returnJniType + " res = wrapper(" + args + ");");
		writer.write("\tERIM_SWITCH_BACK(regular);\n\n");

		writer.write("\tSNI_DBM(\"[s]: application terminated!\");\n");
		writer.write("\treset_env(\"" + System.getenv("BENCHMARK_NAME") + "\", 0);\n");
		writer.write("\treturn res;\n");
	    }
	    writer.write("}\n\n\n");

	    writer.write(returnJniType + " wrapper(JNIEnv *env, jobject obj" + typeArgs + ") {\n");
	    /*
TODO: TRY TO MAKE THIS WORK!!
writer.write("\tif (native_method == NULL) {\n");
writer.write("\t\tnative_method = dlsym(RTLD_DEFAULT, \"" + nativeMethodName + "\");\n");
writer.write("\t\tif (native_method == NULL) {\n");
writer.write("\t\t\tfprintf(stdout, \"Failed to find the symbol: " + nativeMethodName + "\\n\");\n");
writer.write("\t\t\texit(EXIT_FAILURE);\n");
writer.write("\t\t}\n");
writer.write("\t}\n\n"); 
	     */
	    writer.write("\tvoid (*native_method)(JNIEnv*, jobject" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ") = dlsym(RTLD_DEFAULT, \"" + nativeMethodName + "\");\n");
	    writer.write("\tif (native_method == NULL) {\n");
	    writer.write("\t\tfprintf(stdout, \"Failed to find the symbol: " + nativeMethodName + "\\n\");\n");
	    writer.write("\t\texit(EXIT_FAILURE);\n");
	    writer.write("\t}\n\n");

	    if (jniTypes.length > 0) {
		writer.write("\tif ((*env)->GetStringUTFChars != Faastion_GetStringUTFChars) {\n");
		writer.write("\t\tinit_jni_wrapper(env);\n");
		writer.write("\t}\n\n");
	    }

	    writer.write("\tSNI_DBM(\"[s]: handler's ready, changing domain...\");\n");
	    writer.write("\t__wrpkrumem(ERIM_DOMAIN(domain));\n");
	    if (returnJniType.equals("void")) {
		writer.write("\t" + mc);                
		writer.write("\t__wrpkru(0);\n");
	    }
	    else {
		writer.write("\t" + returnJniType + " res = " + mc);
		writer.write("\t__wrpkru(0);\n");

		writer.write("\treturn res;\n");
	    }
	    writer.write("}\n\n\n");
	    writer.write("void lazy_proc_isolation() {\n");
	    writer.write("\tint ret;\n");
	    writer.write("\tpid_t child_pid;\n");
	    writer.write("\tint pipefd[2];\n");			
	    writer.write("\tchar buffer[1024];\n");
	    writer.write("\tmemset(buffer,'\\0',sizeof(buffer));\n\n");

	    writer.write("\tif(pipe(pipefd) == -1){\n");
	    writer.write("\t\tperror(\"pipe error\");\n");
	    writer.write("\t\texit(EXIT_FAILURE);\n");
	    writer.write("\t}\n");

	    writer.write("\tchar *argv[] = {\"" + System.getenv("ARGO_HOME") + "/graalvisor/build/libs/" + methodName + "-proc" + "\", NULL};\n");
	    writer.write("\tchar **environ = {NULL};\n\n");

	    writer.write("\tposix_spawn_file_actions_t child_fd_actions;\n");			
	    writer.write("\tif((ret = posix_spawn_file_actions_init(&child_fd_actions)) != 0){\n");
	    writer.write("\t\tfprintf(stderr,\"posix_spawn_file_actions_init failed %d\",ret);\n");
	    writer.write("\t}\n\n");

	    writer.write("\tif((ret = posix_spawn_file_actions_addclose(&child_fd_actions,pipefd[0])) != 0){\n");
	    writer.write("\t\tfprintf(stderr,\"posix_spawn_file_actions_addclose failed %d\",ret);\n");
	    writer.write("\t}\n\n");

	    writer.write("\tif((ret = posix_spawn_file_actions_adddup2(&child_fd_actions,pipefd[1], 1)) != 0){\n");
	    writer.write("\t\tfprintf(stderr,\"posix_spawn_file_actions_adddup2 failed %d\",ret);\n");
	    writer.write("\t}\n\n");

	    writer.write("\tif((ret = posix_spawn_file_actions_addclose(&child_fd_actions,pipefd[1])) != 0){\n");
	    writer.write("\t\tfprintf(stderr,\"posix_spawn_file_actions_addclose failed %d\",ret);\n");
	    writer.write("\t}\n\n");

	    writer.write("\tif((ret = posix_spawn(&child_pid, argv[0], &child_fd_actions, NULL,argv, environ)) != 0){\n");
	    writer.write("\t\tfprintf(stderr,\"posix_spawn failed %d\",ret);\n");
	    writer.write("\t\texit(ret);\n");
	    writer.write("\t}\n");
	    writer.write("}\n\n\n");


	    writer.write("int main() {\n");
	    writer.write("\tvoid *open_lib;\n");
	    writer.write("\topen_lib = dlopen(\"" + System.getenv("ARGO_HOME") + "/graalvisor/build/libs/lib" + System.getenv("BENCHMARK_NAME") + "-jni.so\", RTLD_LAZY);\n");
	    writer.write("\tif (!open_lib) {\n");
	    writer.write("\t\tfprintf(stderr, \"Error: %s\", dlerror());\n");
	    writer.write("\t\treturn 1;\n");
	    writer.write("\t}\n");
	    writer.write("\tvoid (*native_method)(JNIEnv*, jobject" + (jniTypes.length > 0 ? ", " : "") + String.join(", ", jniTypes) + ") = dlsym(open_lib, \"" + nativeMethodName + "\");\n");
	    writer.write("\tif (native_method == NULL) {\n");
	    writer.write("\t\t\tfprintf(stdout, \"Failed to find the symbol: " + nativeMethodName + "\\n\");\n");
	    writer.write("\t\t\texit(EXIT_FAILURE);\n");
	    writer.write("\t\t}\n\n");
	    if (returnJniType.equals("void")) {
		writer.write("\t" + nmc);                
	    }
	    else {
		writer.write("\t" + returnJniType + " res = " + nmc);
		writer.write("\t__wrpkru(0);\n\n");

	    }
	    writer.write("\tdlclose(open_lib);\n");
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
