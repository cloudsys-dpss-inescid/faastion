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

public class JNITemplateBuilder extends TemplateBuilder {

	public JNITemplateBuilder(List<String> packageNameList, String writeDestination) {
		super(packageNameList, writeDestination);

		// default variables to escape the preprocessor directives in C
		setTemplateVariable("include", "#include");
        setTemplateVariable("define", "#define");
        setTemplateVariable("ifndef", "#ifndef");
        setTemplateVariable("ifdef", "#ifdef");
        setTemplateVariable("else", "#else");
        setTemplateVariable("endif", "#endif");
	}

	// FIXME: hard to read switch case, comparing pointer and not string
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

	public void createHeader(String[] jniTypes, String returnJniType, String className, String gateName)
			throws IOException {
		System.out.println("Hello from createHeader");

		String headerGuard = "_Included_" + className;
		String callGateSignature = "Java_" + className + "_" + gateName;
		if (jniTypes.length > 0) {
			callGateSignature += "(JNIEnv *, jclass, " + String.join(", ", jniTypes) + ")";
		} else {
			callGateSignature += "(JNIEnv *, jclass)";
		}

        setTemplateVariable("headerGuard", headerGuard);
		setTemplateVariable("returnType", returnJniType);
		setTemplateVariable("callGateSignature", callGateSignature);

		String dirName = System.getenv("SNIPPETS_DIR");
        String fileName = className + ".h";
		buildTemplate("templates/jni_header.vm", dirName, fileName);
	}

	public void createSnippet(String[] jniTypes, String returnJniType, String methodName, String className,
			String gateName) throws IOException {		
		String params = "JNIEnv *env, jobject obj";
		String paramTypes = "JNIEnv*, jobject";
		String args = "env, obj";
		String literals = "NULL, NULL";
		if (jniTypes.length != 0) {
			String[] extraArguments = IntStream.range(0, jniTypes.length)
					.mapToObj(i -> "arg" + i)
					.toArray(String[]::new);
			params += IntStream.range(0, extraArguments.length)
					.mapToObj(i -> jniTypes[i] + " " + extraArguments[i])
					.collect(Collectors.joining(", ", ", ", ""));
			paramTypes = ", " + String.join(", ", jniTypes);
			args += ", " + String.join(", ", extraArguments);
			literals += ", " + String.join(", ", extraArguments); // FIXME: passing undefined variables (arg0, arg1, arg2...)
		}

		String headerFilename = className + ".h";
		String callGate = "Java_" + className + "_" + gateName;
		String libName = System.getenv("BENCHMARK_NAME");
		String nativeMethodName = "Java_" + className + "_" + methodName;
		String nativeELF = System.getenv("ARGO_HOME") + "/graalvisor/build/libs/" + methodName + "-proc";
		String libPathname = System.getenv("ARGO_HOME") + "/graalvisor/build/libs/lib" + libName + "-jni.so";

		setTemplateVariable("methodReceivesExtraArguments", jniTypes.length > 0);
		setTemplateVariable("headerFilename", headerFilename);
		setTemplateVariable("returnType", returnJniType);
		setTemplateVariable("params", params);
		setTemplateVariable("callGate", callGate);
		setTemplateVariable("args", args);
		setTemplateVariable("paramTypes", paramTypes);
		setTemplateVariable("libName", libName);
		setTemplateVariable("nativeMethod", nativeMethodName);
		setTemplateVariable("nativeELF", nativeELF);
		setTemplateVariable("libPathname", libPathname);
		setTemplateVariable("literals", literals);

		String dirName = System.getenv("SNIPPETS_DIR");
        String fileName = methodName + ".c";
		buildTemplate("templates/jni_callgate.vm", dirName, fileName);
	}

	// FIXME: use Javassist built-in method
	public String[] getParameterTypes(String signature) throws Exception {
		SignatureAttribute.MethodSignature methodSignature = SignatureAttribute.toMethodSignature(signature);

		String[] parameterTypes = Arrays.stream(methodSignature.getParameterTypes())
				.map(SignatureAttribute.Type::toString)
				.toArray(String[]::new);

		return parameterTypes;
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

	// FIXME: signature includes name, but does not include return type
	public boolean isCallGateDeclared(CtClass clazz, String signature, String gateName) {
		CtMethod[] declaredMethods = clazz.getDeclaredMethods();

		for (CtMethod method : declaredMethods) {
			if (method.getName().equals(gateName) && method.getSignature().equals(signature)) {
				return true;
			}
		}

		return false;
	}

	// FIXME: what is meant by internalClass?
	public boolean isInternalClass(String className) {
		return className.startsWith("com.sun.") ||
				className.startsWith("java.") ||
				className.startsWith("sun.") ||
				className.startsWith("jdk.") ||
				className.startsWith("org.");
	}
}
