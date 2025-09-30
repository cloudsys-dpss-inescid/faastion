package org.faastion.javassist;

import java.io.IOException;
import java.lang.System;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.IntStream;

import javassist.CannotCompileException;
import javassist.ClassPool;
import javassist.CtBehavior;
import javassist.CtMethod;
import javassist.CtClass;
import javassist.CtConstructor;
import javassist.Modifier;
import javassist.NotFoundException;
import javassist.expr.ExprEditor;
import javassist.expr.MethodCall;
import javassist.CtNewMethod;

public class JNITemplateBuilder extends TemplateBuilder {

	private Set<String> createdSnippets;

	private final String loadNativeLib = 
"void loadNativeLibrary(String libName);";

	private class LoadWrapper {

		private final String loadWrapper = 
"public static void loadWrapper(String pathName) {"							+
"	if (pathName == null) {"												+
"		throw new NullPointerException();"									+
"	}"																		+
"	java.io.File file = new java.io.File(pathName);"						+
"	if (file.isFile()) {"													+
"		System.load(\"" + wrapperLib + "\");"								+
"		loadNativeLibrary(pathName);"										+
"		return;"															+
"	}"																		+
"	throw new UnsatisfiedLinkError();"										+
"}";

		private final String loadLibraryWrapper = 
"public static void loadLibraryWrapper(String libName) {"					+
"	if (libName == null) {"													+
"		throw new NullPointerException();"									+
"	}"																		+
"	String libraryPath = System.getProperty(\"java.library.path\");"		+
"	String[] folders = libraryPath.split(\":\");"							+
"	for (int i = 0; i < folders.length; i++) {"								+
"		String pathName = folders[i] + \"/lib\" + libName + \".so\";"		+
"		java.io.File file = new java.io.File(pathName);"					+
"		if (file.isFile()) {"												+
"			System.load(\"" + wrapperLib + "\");"							+
"			loadNativeLibrary(pathName);"									+
"			return;"														+
"		}"																	+
"	}"																		+
"	throw new UnsatisfiedLinkError();"										+
"}";

		private String methodName;

		public LoadWrapper(String methodName) {
			this.methodName = methodName;
		}

		public String toString() {
			switch (methodName) {
			case "loadWrapper":
				return loadWrapper;
			case "loadLibraryWrapper":
				return loadLibraryWrapper;
			default:
				throw new RuntimeException("Invalid load wrapper method");
			}
		}

	}

	private class CallGate {
		private CtClass returnType;
		private String signature;
		private String className;
		private String methodName;
		private String gateName;
		private String gateLib;
		private String[] parameters;

		public CallGate(MethodCall methodCall) {
			CtMethod method = getMethodFromMethodCall(methodCall);			
			this.returnType = returnType(method);
			this.parameters = parameterTypes(method);
			this.methodName = method.getName().replace("_", "_1");
			this.signature = method.getSignature();
			this.className = method.getDeclaringClass().getName().replace("_", "_1");
			// this.gateName = methodName + "callGate";
			this.gateName = methodName;
			this.gateLib = functionID.concat("-").concat("pkru");

			System.out.println("Native method call " + className + "\t" + methodName);
		}

		public CtClass getReturnType() {
			return returnType;
		}

		public String getSignature() {
			return signature;
		}

		public String getMethodName() {
			return methodName;
		}

		public String getGateName() {
			return gateName;
		}

		public String getGateLib() {
			return gateLib;
		}

		public String[] getParameters() {
			return parameters;
		}

		public boolean returnTypeIsVoid() {
			return returnType.getName().equals("void");
		}

		public void createNativeTemplates() {
			String returnJniType = getJniType(returnType.getName());
			String[] jniTypes = Arrays.stream(parameters)
					.map(param -> getJniType(param))
					.toArray(String[]::new);

			String name = className.replace(".", "_");
			createHeader(jniTypes, returnJniType, methodName, name, gateName);
			createSnippet(jniTypes, returnJniType, methodName, name, gateName);
		}

	}

	private String functionID;
	private String templateDir;
	private String nativeLibName;
	private String loaderLib;
	private String wrapperLib;

	public JNITemplateBuilder() {
		super();

		createdSnippets = new HashSet<>();

		functionID = System.getenv("FUNCTION_ID");
		templateDir = System.getenv("SNIPPETS_DIR");
		nativeLibName = System.getenv("ARGO_HOME")
				.concat("/graalvisor/build/libs/lib")
				.concat(System.getenv("BENCHMARK_NAME"))
				.concat("-jni.so");
		loaderLib = System.getenv("ARGO_HOME")
				.concat("/graalvisor/build/libs/libloader.so");
		wrapperLib = System.getenv("ARGO_HOME")
				.concat("/graalvisor/build/libs/lib")
				.concat(System.getenv("BENCHMARK_NAME"))
				.concat("-wrapper.so");

		// default variables to escape the preprocessor directives in C
		setTemplateVariable("include", "#include");
        setTemplateVariable("define", "#define");
        setTemplateVariable("ifndef", "#ifndef");
        setTemplateVariable("ifdef", "#ifdef");
        setTemplateVariable("else", "#else");
        setTemplateVariable("endif", "#endif");
	}

	public int getTypeSize(String parameterType) {
		switch(parameterType) {
		case "jboolean":
		case "jbyte":
			return 1;
		case "jchar":
		case "jshort":
			return 2;
		case "jint":
		case "jfloat":
			return 4;
		case "jlong":
		case "jdouble":
		case "jobjectArray":
		case "jbooleanArray":
		case "jbyteArray":
		case "jcharArray":
		case "jshortArray":
		case "jintArray":
		case "jlongArray":
		case "jfloatArray":
		case "jdoubleArray":
		case "jstring":
		case "jclass":
		case "jobject":
		case "jthrowable":
			return 8;
		default:
			throw new IllegalArgumentException("Unsupported parameter type: " + parameterType);
		}
	}

	public String getJniType(String parameterType) {
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
		case "java.lang.Throwable":
			return "jthrowable";
		case "java.lang.Object":
		default:
			return "jobject";
		}
	}

	@Override
	public void transform(CtBehavior behavior) throws CannotCompileException, IOException {

		if (isInternalClass(behavior.getDeclaringClass().getName())) {
			return;
		}

		behavior.instrument(new ExprEditor() {

			public void edit(MethodCall m) throws CannotCompileException {
				CtMethod method = getMethodFromMethodCall(m);
				if (method == null)
					return;
				String className = method.getDeclaringClass().getName();
				String methodName = method.getName();

				// System.out.println("Instrument method call: " + className + "." + methodName);

				if (isLoadMethod(className, methodName)) {
					CtClass clazz = behavior.getDeclaringClass();
					String wrapperMethodName = methodName + "Wrapper";
					defineLoadWrapper(clazz, wrapperMethodName);
                    m.replace(String.format("%s($1);", wrapperMethodName));
				}
				
				else if (Modifier.isNative(method.getModifiers()) && !isInternalClass(className)) {
					CallGate callGate = new CallGate(m);
					if (addCallGateMethod(behavior.getDeclaringClass(), callGate)) {
						callGate.createNativeTemplates();
					}
					// m.replace((callGate.returnTypeIsVoid() ? "" : "$_=") + callGate.getGateName() + "($$);");
				}
			}

		});
	}

	private void defineLoadWrapper(CtClass clazz, String methodName) throws CannotCompileException {
		// System.out.println("Load library method call");

		String nativeMethodName = "loadNativeLibrary";
		if (!isDeclared(clazz, nativeMethodName, "(Ljava/lang/String;)V")) {
			CtMethod newMethod = CtNewMethod.make(loadNativeLib, clazz);
			newMethod.setModifiers(Modifier.PUBLIC | Modifier.STATIC | Modifier.NATIVE);
			clazz.addMethod(newMethod);
			String className = clazz.getName().replace("_", "_1").replace(".", "_");
			createHeader(new String[] {"jstring"}, "void", nativeMethodName, className, nativeMethodName);
			createLoadNativeLibrarySnippet(nativeMethodName, className);
		}
		
		if (!isDeclared(clazz, methodName, "(Ljava/lang/String;)V")) {
            CtMethod newMethod = CtNewMethod.make(new LoadWrapper(methodName).toString(), clazz);
            clazz.addMethod(newMethod);
        }
	}

	private boolean isLoadMethod(String className, String methodName) {
		return className.equals("java.lang.System") && 
			(methodName.equals("load") || methodName.equals("loadLibrary"));
	}

	private CtMethod getMethodFromMethodCall(MethodCall methodCall) {
		CtMethod method;
		try {
			method = methodCall.getMethod();
		} catch (NotFoundException nfe) {
			// System.out.println("WARNING: Method not found " + methodCall.getClassName() + "." +methodCall.getMethodName());
			return null;
		}
		return method;
	}

	private String getHeaderFilename(String methodName, String className) {
		return className.replace("$", "_") + "_" + methodName + ".h";
	}

	public void createLoadNativeLibrarySnippet(String methodName, String className) {
		String headerFilename = getHeaderFilename(methodName, className);
		setTemplateVariable("headerFilename", headerFilename);
		setTemplateVariable("load_native_library", "Java_" + className.replace("$", "_00024") + "_" + methodName);
		buildTemplate("templates/jni_wrapper_lib.vm", templateDir, className.replace("$", "_") + "_" + methodName + ".c");
	}

	public void createHeader(String[] jniTypes, String returnJniType,
			String methodName, String className, String gateName)
	{
		String headerFilename = getHeaderFilename(methodName, className);

        setTemplateVariable("headerGuard", "_Included_" + className.replace("$", "_"));
		setTemplateVariable("returnType", returnJniType);
		setTemplateVariable("callGate", "Java_" + className.replace("$", "_00024") + "_" + gateName);
		setTemplateVariable("numArgs", jniTypes.length);
		setTemplateVariable("jniTypes", jniTypes);

		buildTemplate("templates/jni_header.vm", templateDir, headerFilename);
	}

	public void createSnippet(String[] jniTypes, String returnJniType,
			String methodName, String className, String gateName)
	{		
		String headerFilename = getHeaderFilename(methodName, className);

		setTemplateVariable("methodName", methodName);
		setTemplateVariable("loaderLib", loaderLib);
		setTemplateVariable("functionID", functionID);
		setTemplateVariable("nativeLibName", nativeLibName);
		setTemplateVariable("jniTypes", jniTypes);
		setTemplateVariable("numArgs", jniTypes.length);
		setTemplateVariable("headerFilename", headerFilename);
		setTemplateVariable("returnType", returnJniType);
		setTemplateVariable("callGate", "Java_" + className.replace("$", "_00024") + "_" + gateName);
		setTemplateVariable("nativeMethod", "Java_" + className.replace("$", "_00024") + "_" + methodName);

		buildTemplate("templates/jni_callgate.vm", templateDir, className.replace("$", "_") + "_" + methodName + ".c");
	}

	public void declareCallGate(CtClass clazz, String[] parameters, CtClass returnType, String gateName)
			throws NotFoundException, CannotCompileException {
		ClassPool cp = clazz.getClassPool();

		CtClass[] parameterTypes = Arrays.stream(parameters)
				.map(parameter -> cp.getOrNull(parameter))
				.toArray(CtClass[]::new);

		if (Arrays.asList(parameterTypes).contains(null)) {
			throw new NotFoundException("One or more parameter types not found.");
		}

		CtMethod nativeMethod = new CtMethod(returnType, gateName, parameterTypes, clazz);
		nativeMethod.setModifiers(Modifier.PUBLIC | Modifier.STATIC | Modifier.NATIVE);

		clazz.addMethod(nativeMethod);
	}

	public boolean isDeclared(CtClass clazz, String methodName, String signature) {
		CtMethod[] declaredMethods = clazz.getDeclaredMethods();

		for (CtMethod method : declaredMethods) {
			if (method.getName().equals(methodName) && method.getSignature().equals(signature)) {
				return true;
			}
		}

		return false;
	}

	boolean addCallGateMethod(CtClass clazz, CallGate callGate) {
		String snippet = clazz.getName() + callGate.getGateName();
		
		if (createdSnippets.contains(snippet)) {
			return false;
		} else {
			createdSnippets.add(snippet);
			return true;
		}
	}

	public CtClass returnType(CtMethod method) {
		CtClass returnType;
		try {
			returnType = method.getReturnType();
		} catch (NotFoundException nfe) {
			throw new RuntimeException("Return type could not be found");
		}
		return returnType;
	}

	public String[] parameterTypes(CtMethod method) {
		String[] parameters;
		try {
			parameters = Arrays.stream(method.getParameterTypes())
					.map(param -> param.getName())
					.toArray(String[]::new);
		} catch (NotFoundException nfe) {
			throw new RuntimeException("Parameter types could not be found");
		}
		return parameters;
	}

	public boolean isInternalClass(String className) {
		return className.startsWith("com.sun.") ||
				className.startsWith("java.") ||
				className.startsWith("sun.") ||
				className.startsWith("jdk.");
	}
}
