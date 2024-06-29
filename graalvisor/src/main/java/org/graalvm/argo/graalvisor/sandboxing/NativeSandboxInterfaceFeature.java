package org.graalvm.argo.graalvisor.sandboxing;

import com.oracle.svm.core.jdk.NativeLibrarySupport;
import com.oracle.svm.core.jdk.PlatformNativeLibrarySupport;
import com.oracle.svm.hosted.FeatureImpl;
import com.oracle.svm.hosted.c.NativeLibraries;
import org.graalvm.nativeimage.hosted.Feature;

public class NativeSandboxInterfaceFeature implements Feature {
    @Override
    public void beforeAnalysis(BeforeAnalysisAccess access) {
        // Treat "NativeSandboxInterface" as a built-in library.
        NativeLibrarySupport.singleton().preregisterUninitializedBuiltinLibrary("NativeSandboxInterface");
        // Treat JNI calls in "org.graalvm.argo.graalvisor.sandboxing.NativeSandboxInterface" as calls to built-in library.
        PlatformNativeLibrarySupport.singleton().addBuiltinPkgNativePrefix("org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface");
        NativeLibraries nativeLibraries = ((FeatureImpl.BeforeAnalysisAccessImpl) access).getNativeLibraries();
        // Add "jvm" as a dependency to "NativeSandboxInterface".
        nativeLibraries.addStaticJniLibrary("NativeSandboxInterface");
    }
}
