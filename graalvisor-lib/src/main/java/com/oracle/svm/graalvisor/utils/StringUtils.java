package com.oracle.svm.graalvisor.utils;

import org.graalvm.nativeimage.ObjectHandle;
import org.graalvm.nativeimage.ObjectHandles;

public class StringUtils {
    public static String retrieveString(ObjectHandle handle) {
        String res = ObjectHandles.getGlobal().get(handle);
        ObjectHandles.getGlobal().destroy(handle);
        return res;
    }
}
