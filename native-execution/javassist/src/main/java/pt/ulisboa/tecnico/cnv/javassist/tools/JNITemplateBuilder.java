package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.lang.System;

import java.util.Arrays;
import java.util.List;
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


public class JNITemplateBuilder extends TemplateBuilder {

	private class CallGate {
		private CtClass returnType;
		private String signature;
		private String className;
		private String methodName;
		private String gateName;
		private String gateLib;
		private String[] parameters;

		public CallGate(MethodCall methodCall) {
			CtMethod method = method(methodCall);			
			this.returnType = returnType(method);
			this.parameters = parameterTypes(method);
			this.signature = method.getSignature();
			this.className = methodCall.getClassName(); 
			this.methodName = methodCall.getMethodName();
			this.gateName = methodName + "callGate";
			this.gateLib = functionID.concat("-").concat(methodName);
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
			createHeader(jniTypes, returnJniType, name, gateName);
			createSnippet(jniTypes, returnJniType, methodName, name, gateName);
		}

	}

	private String functionID;
	private String templateDir;
	private String nativeLibName;
	private String loaderLib;

	public JNITemplateBuilder(List<String> packageNameList, String writeDestination) {
		super(packageNameList, writeDestination);

		functionID = System.getenv("FUNCTION_ID");
		templateDir = System.getenv("SNIPPETS_DIR");
		nativeLibName = System.getenv("ARGO_HOME")
				.concat("/graalvisor/build/libs/lib")
				.concat(System.getenv("BENCHMARK_NAME"))
				.concat("-jni.so");
		loaderLib = System.getenv("ARGO_HOME")
				.concat("/graalvisor/build/libs/libloader.so");

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
			return 8;
		case "jstring":
			return -1;
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
		case "java.lang.Object":
			return "jobject";
		case "java.lang.Throwable":
			return "jthrowable";
		default:
			throw new IllegalArgumentException("Unsupported parameter type: " + parameterType);
		}
	}

	@Override
	protected void transform(CtBehavior behavior) throws Exception {
		super.transform(behavior);

		if (isInternalClass(behavior.getDeclaringClass().getName())) {
			return;
		}

		behavior.instrument(new ExprEditor() {

			public void edit(MethodCall m) throws CannotCompileException {
				if (m.getClassName().equals("java.lang.System") && m.getMethodName().equals("loadLibrary")) {
					m.replace(";");
				}
				else if (Modifier.isNative(method(m).getModifiers()) && !isInternalClass(m.getClassName())) {
					CallGate callGate = new CallGate(m);
					if (addCallGateMethod(behavior.getDeclaringClass(), callGate)) {
						callGate.createNativeTemplates();
					}
					m.replace((callGate.returnTypeIsVoid() ? "" : "$_=") + callGate.getGateName() + "($$);");
				}
			}

		});
	}

	public void createHeader(String[] jniTypes, String returnJniType, String className, String gateName) {
		System.out.println("Hello from createHeader");

        setTemplateVariable("headerGuard", "_Included_" + className);
		setTemplateVariable("returnType", returnJniType);
		setTemplateVariable("callGate", "Java_" + className + "_" + gateName);
		setTemplateVariable("numArgs", jniTypes.length);
		setTemplateVariable("jniTypes", jniTypes);

		buildTemplate("templates/jni_header.vm", templateDir, className + ".h");
	}

	public void createSnippet(String[] jniTypes, String returnJniType,
			String methodName, String className, String gateName)
	{		
		int[] argSizes = new int[0];
		if (jniTypes.length != 0) {
			argSizes = IntStream.range(0, jniTypes.length)
				.map(i -> getTypeSize(jniTypes[i]))
				.toArray();
		}

		setTemplateVariable("loaderLib", loaderLib);
		setTemplateVariable("functionID", functionID);
		setTemplateVariable("nativeLibName", nativeLibName);
		setTemplateVariable("jniTypes", jniTypes);
		setTemplateVariable("numArgs", jniTypes.length);
		setTemplateVariable("argSizes", argSizes);
		setTemplateVariable("headerFilename", className + ".h");
		setTemplateVariable("returnType", returnJniType);
		setTemplateVariable("callGate", "Java_" + className + "_" + gateName);
		setTemplateVariable("nativeMethod", "Java_" + className + "_" + methodName);

		buildTemplate("templates/jni_callgate.vm", templateDir, methodName + ".c");
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

	public boolean isCallGateDeclared(CtClass clazz, String signature, String gateName) {
		CtMethod[] declaredMethods = clazz.getDeclaredMethods();

		for (CtMethod method : declaredMethods) {
			if (method.getName().equals(gateName) && method.getSignature().equals(signature)) {
				return true;
			}
		}

		return false;
	}

	boolean addCallGateMethod(CtClass clazz, CallGate callGate) {
		if (isCallGateDeclared(clazz, callGate.getSignature(), callGate.getGateName())) {
			return false;
		}

		try {
			CtConstructor staticInitializer = clazz.makeClassInitializer();
			staticInitializer.insertBefore("System.loadLibrary(\"" + callGate.getGateLib() + "\");");
			declareCallGate(clazz, callGate.getParameters(), callGate.getReturnType(), callGate.getGateName());
		} catch (NotFoundException | CannotCompileException e) {
			throw new RuntimeException("Could not declare call gate");
		}
		
		return true;
	}

	public CtMethod method(MethodCall methodCall) {
		CtMethod method;
		try {
			method = methodCall.getMethod();
		} catch (NotFoundException nfe) {
			throw new RuntimeException("Method could not be found");
		}
		return method;
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
				className.startsWith("jdk.") ||
				className.startsWith("org.") ||
				className.startsWith("byte[]");
	}
}
