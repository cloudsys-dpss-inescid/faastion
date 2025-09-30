package org.faastion.javassist;

import javassist.CtClass;
import javassist.ClassPool;
import javassist.NotFoundException;

import java.io.File;

import java.util.jar.JarFile;
import java.util.stream.Collectors;
import java.util.Collections;
import java.util.List;
import java.util.Objects;

public class BytecodeTransformer {

    private static CtClass getClassFromJarEntry(ClassPool pool, String name) {
        CtClass ctClass;
        
        if (!name.endsWith(".class") || name.startsWith("META-INF/")) {
            return null;
        }

        try {
            String className = name.replace('/', '.').substring(0, name.length() - 6);
            ctClass = pool.get(className);
        } catch (NotFoundException e) {
            ctClass = null;
        }
        
        return ctClass;
    }

    public static void main(String[] args) throws Exception {
        if (args.length < 1) {
            System.out.println("Syntax: JarOfflineTransformer <jarFile>");
            System.exit(0);
        }

        File file = new File(args[0]);
        JarFile jarFile = new JarFile(file);
        
        ClassPool pool = ClassPool.getDefault();
        pool.insertClassPath(file.getPath());

        List<CtClass> classes = Collections.list(jarFile.entries()).stream()
                .map(entry -> getClassFromJarEntry(pool, entry.getName()))
                .filter(Objects::nonNull)
                .collect(Collectors.toList());

        JNITemplateBuilder templateBuilder = new JNITemplateBuilder();
        for (CtClass ctClass : classes) {
            templateBuilder.transform(ctClass);
        }

        jarFile.close();
    }
}