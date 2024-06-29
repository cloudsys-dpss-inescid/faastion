package com.oracle.svm.graalvisor.api;

import static com.oracle.svm.core.posix.headers.Dlfcn.RTLD_NOW;
import static com.oracle.svm.core.posix.headers.Dlfcn.dlclose;
import static com.oracle.svm.graalvisor.GraalVisorImpl.getGraalVisorHostDescriptor;
import static com.oracle.svm.graalvisor.utils.StringUtils.retrieveString;

import java.io.Closeable;
import java.io.FileNotFoundException;
import java.io.IOException;

import org.graalvm.nativeimage.CurrentIsolate;
import org.graalvm.nativeimage.Isolate;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.ObjectHandle;
import org.graalvm.nativeimage.StackValue;
import org.graalvm.nativeimage.c.function.CFunctionPointer;
import org.graalvm.nativeimage.c.function.InvokeCFunctionPointer;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;
import org.graalvm.word.PointerBase;
import org.graalvm.word.WordFactory;

import com.oracle.svm.core.c.function.CEntryPointCreateIsolateParameters;
import com.oracle.svm.core.c.function.CEntryPointNativeFunctions;
import com.oracle.svm.core.posix.PosixUtils;
import com.oracle.svm.graalvisor.GraalVisor;

@SuppressWarnings("unused")
public class GraalVisorAPI implements Closeable {

    /**
     * Pointer to the guest dynamic library (function/application code).
     */
    private final PointerBase dlHandle;

    /**
     * Methods from the native image isolate interface.
     */
    private final CreateIsolateFunctionPointer createIsolateFunctionPointer;
    private final GetIsolateFunctionPointer getIsolateFunctionPointer;
    private final AttachThreadFunctionPointer attachThreadFunctionPointer;
    private final DetachThreadFunctionPointer detachThreadFunctionPointer;
    private final TearDownIsolateFunctionPointer tearDownIsolateFunctionPointer;

    /**
     * Methods implemented by the guestapi (see guestapi.GuestAPI).
     */
    private final GuestReceiveStringFunctionPointer guestReceiveStringFunctionPointer;
    private final InvokeFunctionPointer invokeFunctionPointer;
    private final GuestInstallGraalvisorFunctionPointer guestInstallGraalvisorFunctionPointer;

    /**
     * Load .so into memory, find the function pointers of the exposed guest apis.
     *
     * @param soName file name of dynamically-linked library built from guest application
     * @throws FileNotFoundException No corresponding file found in LD_LIBRARY_PATH
     */
    public GraalVisorAPI(String soName) throws FileNotFoundException {
        dlHandle = PosixUtils.dlopen(soName, RTLD_NOW());

        if (dlHandle.rawValue() == 0) {
            throw new FileNotFoundException(String.format("%s (%s) hasn't been found, please check the file name or your environment variable LD_LIBRARY_PATH.", soName, PosixUtils.dlerror()));
        }

        createIsolateFunctionPointer = PosixUtils.dlsym(dlHandle, "graal_create_isolate");
        getIsolateFunctionPointer = PosixUtils.dlsym(dlHandle, "graal_get_isolate");
        attachThreadFunctionPointer = PosixUtils.dlsym(dlHandle, "graal_attach_thread");
        detachThreadFunctionPointer = PosixUtils.dlsym(dlHandle, "graal_detach_thread");
        tearDownIsolateFunctionPointer = PosixUtils.dlsym(dlHandle, "graal_tear_down_isolate");

        guestInstallGraalvisorFunctionPointer = PosixUtils.dlsym(dlHandle, "guest_install_graalvisor");
        guestReceiveStringFunctionPointer = PosixUtils.dlsym(dlHandle, "guest_receive_string");
        invokeFunctionPointer = PosixUtils.dlsym(dlHandle, "invoke_main_function");
    }

    public IsolateThread createIsolate() {
        CEntryPointNativeFunctions.IsolateThreadPointer isolateThreadPointer = StackValue.get(CEntryPointNativeFunctions.IsolateThreadPointer.class);
        createIsolateFunctionPointer.invoke(WordFactory.nullPointer(), WordFactory.nullPointer(), isolateThreadPointer);
        IsolateThread isolateThread = (IsolateThread) isolateThreadPointer.read();
        guestInstallGraalvisorFunctionPointer.invoke(isolateThread, getGraalVisorHostDescriptor());
        return isolateThread;
    }

    public Isolate getIsolate(IsolateThread isolateThread) {
        Isolate isolate = getIsolateFunctionPointer.invoke(isolateThread);
        return isolate;
    }

    public IsolateThread attachThread(Isolate isolate) {
        CEntryPointNativeFunctions.IsolateThreadPointer isolateThreadPointer = StackValue.get(CEntryPointNativeFunctions.IsolateThreadPointer.class);
        attachThreadFunctionPointer.invoke(isolate, isolateThreadPointer);
        IsolateThread isolateThread = (IsolateThread) isolateThreadPointer.read();
        return isolateThread;
    }

    public int detachThread(IsolateThread isolateThread) {
        return detachThreadFunctionPointer.invoke(isolateThread);
    }

    public void tearDownIsolate(IsolateThread isolateThread) {
        tearDownIsolateFunctionPointer.invoke(isolateThread);
    }

    /**
     * GraalVisor API: invoke the main function inside the className
     */
    public String invokeFunction(IsolateThread guestIsolate, String className, String arguments) {
        ObjectHandle functionHandle, argumentHandle;
        IsolateThread hostIsolate = CurrentIsolate.getCurrentThread();
        try (CTypeConversion.CCharPointerHolder cStringHolder = CTypeConversion.toCString(className)) {
            functionHandle = guestReceiveStringFunctionPointer.invoke(guestIsolate, cStringHolder.get());
        }
        try (CTypeConversion.CCharPointerHolder cStringHolder = CTypeConversion.toCString(arguments)) {
            argumentHandle = guestReceiveStringFunctionPointer.invoke(guestIsolate, cStringHolder.get());
        }
        ObjectHandle resultHandle = invokeFunctionPointer.invoke(guestIsolate, hostIsolate, functionHandle, argumentHandle);
        return retrieveString(resultHandle);
    }

    @Override
    public void close() throws IOException {
        dlclose(dlHandle);
    }

    /**
     * Declarations of the native methods that the host will use to interact with the guest function/application.
     */
    interface CreateIsolateFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        int invoke(CEntryPointCreateIsolateParameters params, CEntryPointNativeFunctions.IsolatePointer isolate, CEntryPointNativeFunctions.IsolateThreadPointer thread);
    }

    interface GetIsolateFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        Isolate invoke(IsolateThread isolateThread);
    }

    interface AttachThreadFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        int invoke(Isolate isolate, CEntryPointNativeFunctions.IsolateThreadPointer thread);
    }

    interface DetachThreadFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        int invoke(IsolateThread isolate);
    }

    interface TearDownIsolateFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        void invoke(IsolateThread thread);
    }

    interface GuestInstallGraalvisorFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        void invoke(IsolateThread guestThread, GraalVisor.GraalVisorStruct graalVisorStructHost);
    }

    interface InvokeFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        ObjectHandle invoke(IsolateThread guestIsolate, IsolateThread hostIsolate, ObjectHandle functionName, ObjectHandle arguments);
    }

    interface GuestReceiveStringFunctionPointer extends CFunctionPointer {
        @InvokeCFunctionPointer
        ObjectHandle invoke(IsolateThread guestThread, CCharPointer cString);
    }

}
