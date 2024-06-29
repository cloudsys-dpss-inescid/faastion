package com.oracle.svm.graalvisor.utils;

import static com.oracle.svm.graalvisor.api.AsGraalVisorHost.graalvisorHostSystemProperty;
import static com.oracle.svm.graalvisor.guestapi.AsGraalVisorGuest.graalvisorGuestSystemProperty;

public class BuildOptionUtils {
    public static void checkValidity() {
        if (Boolean.getBoolean(graalvisorHostSystemProperty) && Boolean.getBoolean(graalvisorGuestSystemProperty)) {
            throw new RuntimeException("Only one of GraalVisorHost and GraalVisorGuest system property can be defined.");
        }
    }
}
