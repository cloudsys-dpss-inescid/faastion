#include <jni.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "graal_capi.h"
#ifdef LAZY_ISOLATION
#include "lazyisolation.h"
#endif
#include "network-isolation.h"
#include "svm-snapshot.h"
#include "org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface.h"

#define PIPE_READ_END  0
#define PIPE_WRITE_END 1

#define TRUE  1
#define FALSE 0

int network_isolation_enabled() {
    const char* env_var = getenv("network_isolation");

    if(env_var != NULL && !strcmp("on", env_var)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void close_parent_fds(int childWrite, int parentRead) {
    // TODO - we should try to get a sense for the used file descriptors.
    for (int fd = 3; fd < 1024; fd++) {
        if (fd != childWrite && fd != parentRead) {
            close(fd);
        }
    }
}

void reset_parent_signal_handlers() {
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_initialize(JNIEnv *env, jobject thisObj) {
    setbuf(stdout, NULL);
    // Note: for some reason that we should track down, the first call to dup2 takes 10s of ms.
    // We do it now to avoid further latency later.
    int dummy = dup2(0, 1023);
    if (dummy >= 0) {
        close(dummy);
    }
#ifdef LAZY_ISOLATION
    initialize_seccomp();
#endif
    if (network_isolation_enabled()) {
        initialize_network_isolation();
    }
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_teardown(JNIEnv *env, jobject thisObj) {
    if (network_isolation_enabled()) {
        teardown_network_isolation();
    }
}

JNIEXPORT int JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeProcessSandbox(JNIEnv *env, jobject thisObj, jintArray childPipeFD, jintArray parentPipeFD) {
    int parentRead, parentWrite, childRead, childWrite;

    // Preparing child pipe (where the child writes and the parent reads).
    jint *childPipeFDptr = (*env)->GetIntArrayElements(env, childPipeFD, 0);
    pipe(childPipeFDptr);
    childRead = childPipeFDptr[PIPE_READ_END];
    childWrite = childPipeFDptr[PIPE_WRITE_END];
    (*env)->ReleaseIntArrayElements(env, childPipeFD, childPipeFDptr, 0);

    // Preparing the parent pipe (where the parent writes and the child reads).
    jint *parentPipeFDptr = (*env)->GetIntArrayElements(env, parentPipeFD, 0);
    pipe(parentPipeFDptr);
    parentRead = parentPipeFDptr[PIPE_READ_END];
    parentWrite = parentPipeFDptr[PIPE_WRITE_END];
    (*env)->ReleaseIntArrayElements(env, parentPipeFD, parentPipeFDptr, 0);

    // The parent thread will be inserted in the net namespace. The child process
    // will inherit the parent (thread) namespace.
    if (network_isolation_enabled()) {
        create_network_namespace();
    }

    // Forking.
    int pid = fork();
    if (pid == 0) {
        // Sanitizing the child process.
        close_parent_fds(childWrite, parentRead);
        reset_parent_signal_handlers();
#ifdef LAZY_ISOLATION
        install_proc_filter(childPipeFDptr);
#endif
    } else {
        // Close the unnecessary pipe ends.
        close(childPipeFDptr[PIPE_WRITE_END]);
#ifdef LAZY_ISOLATION
        attach(pid, childPipeFDptr, parentPipeFDptr);
#else
        close(parentPipeFDptr[PIPE_READ_END]);
#endif
    }
    return pid;
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeIsolateSandbox(JNIEnv *env, jobject thisObj) {
#ifdef LAZY_ISOLATION
    install_thread_filter();
#endif
    if (network_isolation_enabled()) {
        create_network_namespace();
    }
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeRuntimeSandbox(JNIEnv *env, jobject thisObj) {
#ifdef LAZY_ISOLATION
    install_thread_filter();
#endif
    if (network_isolation_enabled()) {
        create_network_namespace();
    }
}


JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_teardownNativeProcessSandbox(JNIEnv *env, jobject thisObj) {
    if (network_isolation_enabled()) {
        delete_network_namespace();
    }
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_teardownNativeIsolateSandbox(JNIEnv *env, jobject thisObj) {
    if (network_isolation_enabled()) {
        delete_network_namespace();
    }
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_teardownNativeRuntimeSandbox(JNIEnv *env, jobject thisObj) {
    if (network_isolation_enabled()) {
        delete_network_namespace();
    }
}

JNIEXPORT jstring JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_svmInvoke(
        JNIEnv *env,
        jobject thisObj,
        jobject sandboxHandle,
        jstring fin) {
    const char* fin_str = (*env)->GetStringUTFChars(env, fin, 0);
    char fout[FOUT_LEN];
    jclass cls = (*env)->GetObjectClass(env, sandboxHandle);
    jlong sandbox_handle = (*env)->GetLongField(env, sandboxHandle, (*env)->GetFieldID(env, cls, "sandboxHandle", "J"));
    forked_invoke_svm((forked_svm_sandbox_t*)sandbox_handle, fin_str, fout);
    (*env)->ReleaseStringUTFChars(env, fin, fin_str);
    return (*env)->NewStringUTF(env, fout);
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_svmDestroy(
        JNIEnv *env,
        jobject thisObj,
        jobject sandboxHandle,
        jboolean reuseIsolate) {
    jclass cls = (*env)->GetObjectClass(env, sandboxHandle);
    jlong sandbox_handle = (*env)->GetLongField(env, sandboxHandle, (*env)->GetFieldID(env, cls, "sandboxHandle", "J"));
    forked_destroy_svm((forked_svm_sandbox_t*)sandbox_handle, reuseIsolate);
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_svmClone(
        JNIEnv *env,
        jobject thisObj,
        jobject original,
        jobject clone,
        jboolean reuseIsolate) {
    jclass cls = (*env)->GetObjectClass(env, original);
    jfieldID field = (*env)->GetFieldID(env, cls, "sandboxHandle", "J");
    jlong original_handle = (*env)->GetLongField(env, original, field);
    jlong clone_handle = (jlong) forked_clone_svm((forked_svm_sandbox_t*)original_handle, reuseIsolate);
    (*env)->SetLongField(env, clone, field, clone_handle);
}

JNIEXPORT jstring JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_svmCheckpoint(
        JNIEnv *env,
        jobject thisObj,
        jint svmid,
        jobject sandboxHandle,
        jint concurrency,
        jint requests,
        jstring fin,
        jstring fpath,
        jstring meta_snap_path,
        jstring mem_snap_path) {
    const char* fpath_str = (*env)->GetStringUTFChars(env, fpath, 0);
    const char* meta_snap_path_str = (*env)->GetStringUTFChars(env, meta_snap_path, 0);
    const char* mem_snap_path_str = (*env)->GetStringUTFChars(env, mem_snap_path, 0);
    const char* fin_str = (*env)->GetStringUTFChars(env, fin, 0);
    char fout[FOUT_LEN];
    jlong sandbox_handle = (jlong) checkpoint_svm(fpath_str, meta_snap_path_str, mem_snap_path_str, svmid, concurrency, requests, fin_str, fout);
    jclass cls = (*env)->GetObjectClass(env, sandboxHandle);
    (*env)->SetLongField(env, sandboxHandle, (*env)->GetFieldID(env, cls, "sandboxHandle", "J"), sandbox_handle);
    (*env)->ReleaseStringUTFChars(env, fpath, fpath_str);
    (*env)->ReleaseStringUTFChars(env, meta_snap_path, meta_snap_path_str);
    (*env)->ReleaseStringUTFChars(env, mem_snap_path, mem_snap_path_str);
    (*env)->ReleaseStringUTFChars(env, fin, fin_str);
    return (*env)->NewStringUTF(env, fout);
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_svmRestore(
        JNIEnv *env,
        jobject thisObj,
        jint svmid,
        jobject sandboxHandle,
        jstring fpath,
        jstring meta_snap_path,
        jstring mem_snap_path) {
    const char* fpath_str = (*env)->GetStringUTFChars(env, fpath, 0);
    const char* meta_snap_path_str = (*env)->GetStringUTFChars(env, meta_snap_path, 0);
    const char* mem_snap_path_str = (*env)->GetStringUTFChars(env, mem_snap_path, 0);
    jlong sandbox_handle = (jlong) forked_restore_svm(fpath_str, meta_snap_path_str, mem_snap_path_str, svmid);
    jclass cls = (*env)->GetObjectClass(env, sandboxHandle);
    (*env)->SetLongField(env, sandboxHandle, (*env)->GetFieldID(env, cls, "sandboxHandle", "J"), sandbox_handle);
    (*env)->ReleaseStringUTFChars(env, fpath, fpath_str);
    (*env)->ReleaseStringUTFChars(env, mem_snap_path, mem_snap_path_str);
    (*env)->ReleaseStringUTFChars(env, meta_snap_path, meta_snap_path_str);
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_svmUnload(
        JNIEnv *env,
        jobject thisObj,
        jobject sandboxHandle) {
    jclass cls = (*env)->GetObjectClass(env, sandboxHandle);
    jlong sandbox_handle = (*env)->GetLongField(env, sandboxHandle, (*env)->GetFieldID(env, cls, "sandboxHandle", "J"));
    forked_unload_svm((forked_svm_sandbox_t*)sandbox_handle);
}

typedef struct {
    // dlopen handle that points to the function code.
    void* dlhandle;
    // Struct of pointers to svm abi.
    isolate_abi_t sabi;
} function_abi_t;

JNIEXPORT long JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_loadFunction(
        JNIEnv *env,
        jobject thisObj,
        jstring pathStr) {
    char* derror = NULL;
    function_abi_t* fabi = (function_abi_t*) malloc(sizeof(function_abi_t));

    // Load functin path into C.
    const char* path = (*env)->GetStringUTFChars(env, pathStr, 0);
    // Load function from library.
    fabi->dlhandle = dlopen(path, RTLD_LAZY);
    if (fabi->dlhandle == NULL) {
        fprintf(stderr, "error: failed to load dynamic library: %s\n", dlerror());
        return 0;
    }
    // Release function path string.
    (*env)->ReleaseStringUTFChars(env, pathStr, path);

    // Load function abi.
    fabi->sabi.graal_create_isolate = (int (*)(graal_create_isolate_params_t*, graal_isolate_t**, graal_isolatethread_t**)) dlsym(fabi->dlhandle, "graal_create_isolate");
    if ((derror = dlerror()) != NULL) {
        fprintf(stderr, "error: %s\n", derror);
        return 0;
    }

    fabi->sabi.graal_tear_down_isolate = (int (*)(graal_isolatethread_t*)) dlsym(fabi->dlhandle, "graal_tear_down_isolate");
    if ((derror = dlerror()) != NULL) {
        fprintf(stderr, "error: %s\n", derror);
        return 0;
    }

    fabi->sabi.graal_get_isolate = (graal_isolate_t* (*)(graal_isolatethread_t*)) dlsym(fabi->dlhandle, "graal_get_isolate");
    if ((derror = dlerror()) != NULL) {
        fprintf(stderr, "error: %s\n", derror);
        return 0;
    }

    fabi->sabi.entrypoint = (void (*)(graal_isolatethread_t*, const char*, const char*, unsigned long)) dlsym(fabi->dlhandle, "entrypoint");
    if ((derror = dlerror()) != NULL) {
        fprintf(stderr, "error: %s\n", derror);
        return 0;
    }

    fabi->sabi.graal_detach_thread = (int (*)(graal_isolatethread_t*)) dlsym(fabi->dlhandle, "graal_detach_thread");
    if ((derror = dlerror()) != NULL) {
        fprintf(stderr, "error: %s\n", derror);
        return 0;
    }

    fabi->sabi.graal_attach_thread = (int (*)(graal_isolate_t*, graal_isolatethread_t**)) dlsym(fabi->dlhandle, "graal_attach_thread");
    if ((derror = dlerror()) != NULL) {
        fprintf(stderr, "error: %s\n", derror);
        return 0;
    }
    return (long) fabi;
}

JNIEXPORT long JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createSandbox(
        JNIEnv *env,
        jobject thisObj,
        long fabiPtr) {
    function_abi_t* fabi = (function_abi_t*) fabiPtr;
    graal_create_isolate_params_t params;
    graal_isolate_t* isolate = NULL;
    graal_isolatethread_t* ithread = NULL;

    memset(&params, 0, sizeof(graal_create_isolate_params_t));
    params.version = 1;
    // Note: this is where we may limit the size of a sandbox. E.g. (limit for 1GB):
    //params.reserved_address_space_size = 1*1024*1024*1024;

    if (fabi->sabi.graal_create_isolate(&params, &isolate, &ithread) != 0) {
        fprintf(stderr, "error: failed to create isolate\n");
        return 0;
    }

    return (long) ithread;
}

JNIEXPORT long JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_getSandbox(
        JNIEnv *env,
        jobject thisObj,
        long fabiPtr,
        long ithreadPtr) {
    function_abi_t* fabi = (function_abi_t*) fabiPtr;
    graal_isolatethread_t* ithread = (graal_isolatethread_t*) ithreadPtr;
    return (long) fabi->sabi.graal_get_isolate(ithread);
}

JNIEXPORT long JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_attachThread(
        JNIEnv *env,
        jobject thisObj,
        long fabiPtr,
        long isolatePtr) {
    function_abi_t* fabi = (function_abi_t*) fabiPtr;
    graal_isolate_t* isolate = (graal_isolate_t*) isolatePtr;
    graal_isolatethread_t* ithread = NULL;

    if (fabi->sabi.graal_attach_thread(isolate, &ithread) != 0) {
        fprintf(stderr, "error: failed to attach thread to isolate\n");
        return 0;
    }

    return (long) ithread;
}

JNIEXPORT jstring JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_invokeSandbox(
        JNIEnv *env,
        jobject thisObj,
        long fabiPtr,
        long ithreadPtr,
        jstring argsStr) {
    function_abi_t* fabi = (function_abi_t*) fabiPtr;
    graal_isolatethread_t* ithread = (graal_isolatethread_t*) ithreadPtr;
    char fout[256];
    const char* args = (*env)->GetStringUTFChars(env, argsStr, 0);
    fabi->sabi.entrypoint(ithread, args, fout, 256);
    (*env)->ReleaseStringUTFChars(env, argsStr, args);
    return (*env)->NewStringUTF(env, fout);
}

JNIEXPORT int JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_detachThread(
        JNIEnv *env,
        jobject thisObj,
        long fabiPtr,
        long ithreadPtr) {
    function_abi_t* fabi = (function_abi_t*) fabiPtr;
    graal_isolatethread_t* ithread = (graal_isolatethread_t*) ithreadPtr;

    return fabi->sabi.graal_detach_thread(ithread);
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_destroySandbox(
        JNIEnv *env,
        jobject thisObj,
        long fabiPtr,
        long ithreadPtr) {
    function_abi_t* fabi = (function_abi_t*) fabiPtr;
    graal_isolatethread_t* ithread = (graal_isolatethread_t*) ithreadPtr;
    if (fabi->sabi.graal_tear_down_isolate(ithread) != 0) {
        fprintf(stderr, "error: failed to destroy isolate\n");
    }
}

JNIEXPORT int JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_unloadFunction(
        JNIEnv *env,
        jobject thisObj,
        long fabi) {
    return dlclose(((function_abi_t*)fabi)->dlhandle);
}
