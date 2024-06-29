package com.oracle.svm.graalvisor.api;

import static com.oracle.svm.graalvisor.utils.BuildOptionUtils.checkValidity;

import java.util.function.BooleanSupplier;

public class AsGraalVisorHost implements BooleanSupplier {
    public static final String graalvisorHostSystemProperty = "GraalVisorHost";

    @Override
    public boolean getAsBoolean() {
        checkValidity();
        return Boolean.getBoolean(graalvisorHostSystemProperty);
    }
}
