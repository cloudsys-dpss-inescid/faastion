package com.oracle.svm.graalvisor;

import java.io.File;
import java.util.Collections;
import java.util.List;

import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.ObjectHandle;
import org.graalvm.nativeimage.c.CContext;
import org.graalvm.nativeimage.c.function.CFunctionPointer;
import org.graalvm.nativeimage.c.function.InvokeCFunctionPointer;
import org.graalvm.nativeimage.c.struct.CField;
import org.graalvm.nativeimage.c.struct.CStruct;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.word.PointerBase;

@CContext(GraalVisor.GraalVisorAPIDirectives.class)
public class GraalVisor {

    public interface HostReceiveStringFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        ObjectHandle invoke(IsolateThread thread, CCharPointer cString);
    }

    @CStruct("graal_visor_t")
    public interface GraalVisorStruct extends PointerBase {

        @CField("f_host_receive_string")
        HostReceiveStringFunctionPointer getHostReceiveStringFunction();

        @CField("f_host_receive_string")
        void setHostReceiveStringFunction(HostReceiveStringFunctionPointer printFunction);
    }

    static class GraalVisorAPIDirectives implements CContext.Directives {
        @Override
        public List<String> getHeaderFiles() {
            return Collections.singletonList("<graal_visor.h>");
        }

        @Override
        public List<String> getOptions() {
            File headerFiles = new File(System.getProperty("com.oracle.svm.graalvisor.libraryPath"));
            return Collections.singletonList("-I" + headerFiles.getAbsoluteFile());
        }
    }

}
