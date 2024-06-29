#include <jni.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#ifdef LAZY_ISOLATION
#include "lazyisolation.h"
#endif
#ifdef MEM_ISOLATION
#include <time.h>
#include <stdlib.h>
#include "memisolation.h"
#endif
#include "org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface.h"

#define PIPE_READ_END  0
#define PIPE_WRITE_END 1

#ifdef MEM_ISOLATION
int eager_mpk = 0;
#endif

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

JNIEXPORT jboolean JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_isLazyIsolationSupported(JNIEnv *env, jobject thisObj) {
#ifdef LAZY_ISOLATION
    return 1;
#else
    return 0;
#endif
}

JNIEXPORT jboolean JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_isMemIsolationSupported(JNIEnv *env, jobject thisObj) {
#ifdef MEM_ISOLATION
    return 1;
#else
    return 0;
#endif
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_setupMemIsolation(JNIEnv *env, jobject thisObj, jstring functionName) {
    if (eager_mpk) {
        const char *function_name = (*env)->GetStringUTFChars(env, functionName, NULL);
        find_domain_eager(function_name);
        (*env)->ReleaseStringUTFChars(env, functionName, function_name);
    }
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_teardownMemIsolation(JNIEnv *env, jobject thisObj, jstring functionName) {
    if (eager_mpk) {
        const char *function_name = (*env)->GetStringUTFChars(env, functionName, NULL);
        reset_env(function_name, 1);
        (*env)->ReleaseStringUTFChars(env, functionName, function_name);
    }
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_ginit(JNIEnv *env, jobject thisObj) {
    setbuf(stdout, NULL);
#ifdef LAZY_ISOLATION
        initialize_seccomp();
#endif
#ifdef MEM_ISOLATION
        char* mpk_env = getenv("EAGER_MPK");
        if (mpk_env != NULL) {
            eager_mpk = atoi(mpk_env);
        }
        initialize_memory_isolation();
#endif
}

JNIEXPORT int JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeProcessSandbox(JNIEnv *env, jobject thisObj, jintArray childPipeFD, jintArray parentPipeFD, jboolean lazyIsolation) {
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

    // Forking.
    int pid = fork();
    if (pid == 0) {
        // Sanitizing the child process.
        close_parent_fds(childWrite, parentRead);
        reset_parent_signal_handlers();
#ifdef LAZY_ISOLATION
        if (lazyIsolation) {
            install_proc_filter(childPipeFDptr);
        }
#endif
    } else {
        // Close the unnecessary pipe ends.
        close(childPipeFDptr[PIPE_WRITE_END]);
        if (!lazyIsolation) {
            close(parentPipeFDptr[PIPE_READ_END]);
        }
#ifdef LAZY_ISOLATION
        if (lazyIsolation) {
            attach(pid, childPipeFDptr, parentPipeFDptr);
        }
#endif
    }
    return pid;
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeIsolateSandbox(JNIEnv *env, jobject thisObj, jboolean lazyIsolation) {
#ifdef LAZY_ISOLATION
    if (lazyIsolation) {
        install_thread_filter();
    }
#endif
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeRuntimeSandbox(JNIEnv *env, jobject thisObj, jboolean lazyIsolation) {
#ifdef LAZY_ISOLATION
    if (lazyIsolation) {
        install_thread_filter();
    }
#endif
}