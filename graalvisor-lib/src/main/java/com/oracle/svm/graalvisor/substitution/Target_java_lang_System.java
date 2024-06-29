package com.oracle.svm.graalvisor.substitution;

import com.oracle.svm.core.annotate.Substitute;
import com.oracle.svm.core.annotate.TargetClass;

@SuppressWarnings("unused")
@TargetClass(java.lang.System.class)
public final class Target_java_lang_System {

    @Substitute
    public static void exit(int status) {
        throw new RuntimeException("System.exit not allowed in graalvisor isolate");
    }
}
