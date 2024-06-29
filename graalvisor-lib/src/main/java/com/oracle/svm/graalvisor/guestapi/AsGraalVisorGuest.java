package com.oracle.svm.graalvisor.guestapi;

import static com.oracle.svm.graalvisor.utils.BuildOptionUtils.checkValidity;

import java.util.function.BooleanSupplier;

public class AsGraalVisorGuest implements BooleanSupplier {
    public static final String graalvisorGuestSystemProperty = "GraalVisorGuest";

    @Override
    public boolean getAsBoolean() {
        checkValidity();
        return Boolean.getBoolean(graalvisorGuestSystemProperty);
    }
}
