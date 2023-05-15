package pt.ulisboa.tecnico.cnv.javassist.tools;

import java.util.List;

import javassist.CtBehavior;
import javassist.CtClass;
import javassist.CtConstructor;

public class CodeDumper extends AbstractJavassistTool {

    public CodeDumper(List<String> packageNameList, String writeDestination) {
        super(packageNameList, writeDestination);
    }

    @Override
    protected void transform(CtClass clazz) throws Exception {
        super.transform(clazz);
    }

    @Override
    protected void transform(CtConstructor constructor) throws Exception {
        super.transform(constructor);
    }

    @Override
    protected void transform(CtBehavior behavior) throws Exception {
        super.transform(behavior);
    }
}
