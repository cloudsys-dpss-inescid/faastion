#define _GNU_SOURCE

#include <jni.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "memory_map.h"
#include "pkru_sandbox.h"
#include "hash_table.h"

#include "org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface.h"

#define PIPE_READ_END  0
#define PIPE_WRITE_END 1

// TLS variable addressable via offset from FS
static __thread pid_t cached_tid = 0;

pid_t __get_cached_tid() {
    return cached_tid;
}

void __set_cached_tid(pid_t tid) {
    cached_tid = tid;
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

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_ginit(JNIEnv *env, jobject thisObj) {
    setbuf(stdout, NULL);

    if (pkru_sandbox_init(__get_cached_tid, __set_cached_tid)) {
        fprintf(stderr, "failed to initialize pthread sandboxes\n");
        cleanup_and_exit();
    }
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
    } else {
        // Close the unnecessary pipe ends.
        close(childPipeFDptr[PIPE_WRITE_END]);
        close(parentPipeFDptr[PIPE_READ_END]);
    }
    return pid;
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeIsolateSandbox(JNIEnv *env, jobject thisObj, jboolean lazyIsolation) {
    
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createNativeRuntimeSandbox(JNIEnv *env, jobject thisObj, jboolean lazyIsolation) {
    
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_createIsolateFunction(JNIEnv *env, jobject thisObj) {
    IsolateFunction *function;
    pthread_t thread;

    function = create_pkru_sandbox();
    set_cached_pkru_sandbox(function);
    hash_table_insert(proc_tbl, gettid(), function);
    
    pthread_create(&thread, NULL, jvm_monitor, (void *)function);
    
    function->notif_fd = install_jvm_filter();
    pthread_detach(thread);
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_destroyIsolateFunction(JNIEnv *env, jobject thisObj) {
    hash_table_remove(proc_tbl, gettid(), NULL);
}

JNIEXPORT jboolean JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_resetActiveWaitingCount(JNIEnv *env, jobject thisObj, int active_waiting_threshold) {
    if (get_active_waiting_count() > active_waiting_threshold) {
        reset_active_waiting_count();
        return 1;
    } else {
        return 0;
    }
}

JNIEXPORT int JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_getDomainUsage(JNIEnv *env, jobject thisObj) {
    return get_domain_usage();
}

JNIEXPORT void JNICALL Java_org_graalvm_argo_graalvisor_sandboxing_NativeSandboxInterface_invokeProcessSandbox(JNIEnv *env, jobject thisObj, jstring filenameString) {
    int ret;
    pid_t child_pid;
    const char *filename = (*env)->GetStringUTFChars(env, filenameString, NULL);
    char *argv[] = {
        (char *)filename,
        NULL
    };

    if ((ret = posix_spawn(&child_pid, filename, NULL, NULL, argv, environ)) != 0){
		fprintf(stderr, "posix_spawn failed %d", ret);
		goto out;
	}

    waitpid(child_pid, NULL, 0);
out:
    (*env)->ReleaseStringUTFChars(env, filenameString, filename);
    return;
}