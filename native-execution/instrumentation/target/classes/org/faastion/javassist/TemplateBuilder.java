package org.faastion.javassist;

import java.io.FileWriter;
import java.io.StringWriter;
import java.io.IOException;

import org.apache.velocity.Template;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;

import javassist.CannotCompileException;
import javassist.CtBehavior;
import javassist.CtClass;
import javassist.bytecode.AccessFlag;

public class TemplateBuilder {
    
    // Attributes
    private VelocityEngine engine;
    private VelocityContext context;

    // Methods

    public TemplateBuilder() {
        engine = new VelocityEngine();
        context = new VelocityContext();
        engine.setProperty("resource.loader", "class");
        engine.setProperty("class.resource.loader.class", "org.apache.velocity.runtime.resource.loader.ClasspathResourceLoader");
        engine.init();
    }

    public void transform(CtBehavior behavior) throws CannotCompileException, IOException {} 

    public void transform(CtClass ctClass) throws CannotCompileException, IOException {
        for (CtBehavior behavior : ctClass.getDeclaredBehaviors()) {
            if ((AccessFlag.ABSTRACT & behavior.getModifiers()) == 0) {
                transform(behavior);
            }
        }
        ctClass.writeFile("output");
        ctClass.detach();
    }

    public void setTemplateVariable(String variable, Object value) {
        context.put(variable, value);
    }

    public void buildTemplate(String vm, String dirName, String fileName) {
        if (dirName == null || fileName == null)
            throw new RuntimeException("Invalid template output: dirName or fileName is null");

        // File file = new File(dirName, fileName);
        StringWriter sw = new StringWriter();
        Template template = engine.getTemplate(vm);
        template.merge(context, sw);
        try (FileWriter writer = new FileWriter(dirName + "/" + fileName)) {
            writer.write(sw.toString());
        } catch (IOException e) {
            throw new RuntimeException("Error writing template to file");
        }
    }
}