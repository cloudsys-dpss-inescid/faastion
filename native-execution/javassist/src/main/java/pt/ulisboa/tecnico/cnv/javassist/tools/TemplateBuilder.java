package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.util.List;
import java.io.File;
import java.io.FileWriter;
import java.io.StringWriter;
import java.io.IOException;

import javassist.CtBehavior;

import org.apache.velocity.Template;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;

public class TemplateBuilder extends AbstractJavassistTool {
    
    // Attributes
    private VelocityEngine engine;
    private VelocityContext context;

    // Methods

    public TemplateBuilder(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);

        engine = new VelocityEngine();
        context = new VelocityContext();
        engine.setProperty("resource.loader", "class");
        engine.setProperty("class.resource.loader.class", "org.apache.velocity.runtime.resource.loader.ClasspathResourceLoader");
        engine.init();
    }

    public void setTemplateVariable(String variable, boolean value) {
        context.put(variable, value);
    }

    public void setTemplateVariable(String variable, String value) {
        context.put(variable, value);
    }

    public void buildTemplate(String vm, String dirName, String fileName) {
        if (dirName == null || fileName == null)
            throw new RuntimeException("Invalid template output: dirName or fileName is null");

        // File file = new File(dirName, fileName);
        StringWriter sw = new StringWriter();
        Template template = engine.getTemplate(vm);
        template.merge(context, sw);
        System.out.println(sw);
        try (FileWriter writer = new FileWriter(dirName + "/" + fileName)) {
            writer.write(sw.toString());
        } catch (IOException e) {
            throw new RuntimeException("Error writing template to file");
        }
    }
}