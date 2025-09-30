#include "svm-snapshot.h"
#include "cr.h"
#include "list_threads.h"
#include "syscalls.h"
#include "graal_capi.h"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/sched.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <unistd.h>

// Mutex to ensure only one thread minimizes syscalls / restores at a time.
pthread_mutex_t cr_mutex = PTHREAD_MUTEX_INITIALIZER;

struct svm_sandbox_t {
    // Pointer to the abi structure where the function pointers will be stored.
    isolate_abi_t       abi;
    // Pointer to the isolate where the thread runs.
    graal_isolate_t*    isolate;
    // Pointer to thread running application.
    pthread_t          thread;
    // File descriptor of the input stream.
    int                istream[2]; // read 0, write into 1.
    // File descriptor of the output stream
    int                 ostream[2];
    // The seed is used to control which virtual memory ranges the svm instance
    // will used. Each seed value represents a 16TB virtual memory range. When
    // calling restore, the user must make sure there is no restoredsvm instance
    // using the same range.
    unsigned long       seed;
};

struct forked_svm_sandbox_t {
    // PID of the child process;
    int child_pid;
    // File descriptor to read commands.
    int ctl_rfd;
    // File descriptor to write command.
    int ctl_wfd;
    // File descriptor used to read invocation payloads.
    int inv_rfd;
    // File descriptor used to write invocation responses.
    int inv_wfd;
};

typedef struct {
    // ABI to create, invoke, destroy isolates.
    isolate_abi_t abi;
    // Pointer to the isolate.
    graal_isolate_t* isolate;
    // Function arguments.
    const char* fin;
    // Return string from function invocation.
    char* fout;
    // Path to the function library.
    const void* fpath;
    // File descriptor used when installing seccomp.
    int seccomp_fd;
    // Number of parallel threads handling requests.
    int concurrency;
    // Number of requests to perform.
    int requests;
    // Integer used as a boolean to decide if the function has terminated.
    int finished;
    // The seed is used to pass the mspace id.
    unsigned int seed;
} checkpoint_worker_args_t;

typedef struct {
     // ABI to create, invoke, destroy isolates.
    isolate_abi_t* abi;
    // Pointer to the isolate.
    graal_isolate_t* isolate;
    // Number of requests to perform.
    int requests;
    // Function arguments.
    const char* fin;
    // Return string from function invocation.
    char* fout;
} entrypoint_worker_args_t;

void run_serial_entrypoint(isolate_abi_t* abi, graal_isolatethread_t *isolatethread, int requests, const char* fin, char* fout) {
     for (int i = 0; i < requests; i++) {
#ifdef PERF_DEBUG
        struct timeval st, et;
        gettimeofday(&st, NULL);
#endif
        abi->entrypoint(isolatethread, fin, fout, FOUT_LEN);
#ifdef PERF_DEBUG
        gettimeofday(&et, NULL);
        log("entrypoint took %lu us\n", ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec));
#endif
    }
}

void* entrypoint_worker(void* args) {
    entrypoint_worker_args_t* wargs = (entrypoint_worker_args_t*) args;
    graal_isolatethread_t *isolatethread = NULL;
    wargs->abi->graal_attach_thread(wargs->isolate, &isolatethread);
    run_serial_entrypoint(wargs->abi, isolatethread, wargs->requests, wargs->fin, wargs->fout);
    // Note: from manual (https://www.graalvm.org/22.1/reference-manual/native-image/C-API/):
    // 'no code may still be executing in the isolate thread's context.' Since we cannot
    // guarantee that threads may be left behind, it is not safe to detach.
    //wargs->abi->graal_detach_thread(isolatethread);
    return NULL;
}

void run_entrypoint(
            isolate_abi_t* abi,
            graal_isolate_t* isolate,
            graal_isolatethread_t* isolatethread,
            unsigned int concurrency,
            unsigned int requests,
            const char* fin,
            char* fout) {
    if (concurrency == 1) {
        run_serial_entrypoint(abi, isolatethread, requests, fin, fout);
    } else {
        pthread_t* workers = (pthread_t*) malloc(concurrency * sizeof(pthread_t));
        entrypoint_worker_args_t* wargs = (entrypoint_worker_args_t*) malloc(concurrency * sizeof(entrypoint_worker_args_t));
        char* fouts = (char*) malloc(concurrency * sizeof(char) * FOUT_LEN);
        for (int i = 0; i < concurrency; i++) {
            wargs[i].abi = abi;
            wargs[i].isolate = isolate;
            wargs[i].requests = requests;
            wargs[i].fin = fin;
            // When multiple threads are launched, the output is taken from the first.
            wargs[i].fout = i == 0 ? fout : &(fouts[i * FOUT_LEN]);
            pthread_create(&(workers[i]), NULL, entrypoint_worker, &(wargs[i]));
        }
        for (int i = 0; i < concurrency; i++) {
            pthread_join(workers[i], NULL);
        }
        free(workers);
        free(wargs);
        free(fouts);
    }
}

void run_svm_internal(
        const char* fpath,
        unsigned int concurrency,
        unsigned int requests,
        const char* fin,
        char* fout,
        isolate_abi_t* abi,
        graal_isolate_t** isolate) {
    graal_isolatethread_t *isolatethread = NULL;

    // Load function and initialize abi.
    if (load_function(fpath, abi)) {
        err("error: failed to load isolate abi\n");
        return;
    }

    // Create isolate with a no limit (use 256*1024*1024 for 256MB).
    graal_create_isolate_params_t params;
    memset(&params, 0, sizeof(graal_create_isolate_params_t));
    params.version = 1;
    params.reserved_address_space_size = 0;
    if (abi->graal_create_isolate(&params, isolate, &isolatethread) != 0) {
        err("error: failed to create isolate\n");
        return;
    }

    // Run user code.
    run_entrypoint(abi, *isolate, isolatethread, concurrency, requests, fin, fout);

    // Detach thread function isolate and quit. This is safe since all threads are stopped.
    abi->graal_detach_thread(isolatethread);
}

void run_svm(const char* fpath, unsigned int concurrency, unsigned int requests, const char* fin, char* fout) {
    isolate_abi_t abi;
    graal_isolate_t* isolate = NULL;
    run_svm_internal(fpath, concurrency, requests, fin, fout, &abi, &isolate);
}

void* checkpoint_worker(void* args) {
    checkpoint_worker_args_t* wargs = (checkpoint_worker_args_t*) args;

    // Install seccomp filter.
    wargs->seccomp_fd = install_seccomp_filter();
    if (wargs->seccomp_fd < 0) {
        err("error: failed install seccomp filter\n");
        return NULL;
    }

    // Prepare and run function.
    run_svm_internal(wargs->fpath, wargs->concurrency, wargs->requests, wargs->fin, wargs->fout, &wargs->abi, &(wargs->isolate));
    // Mark execution as finished (will alert the seccomp monitor to quit).
    wargs->finished = 1;

    return NULL;
}

void* notif_worker(void* args) {
    svm_sandbox_t* svm = (svm_sandbox_t*) args;
    graal_isolatethread_t *isolatethread = NULL;
    char fin[FOUT_LEN];
    char fout[FOUT_LEN];

    // Attach thread to the isolate.
    (svm->abi).graal_attach_thread(svm->isolate, &isolatethread);

    // Handle requests received from the stream.
    for (;;) {
        size_t len;
        int ret;

        // Read function input length.
        ret = read(svm->istream[0], &len, sizeof(size_t));
        if (ret != sizeof(size_t)) {
            err("error: failed to read input len (error %d)\n", errno);
        }

        // Force input size.
        if (len >= FOUT_LEN) {
            err("warning: input over maximum length (len = %d)\n", len);
            len = FOUT_LEN;
        }

        // Read 'len' input bytes.
        if (read(svm->istream[0], fin, len) != len) {
            err("error: failed to read input (error %d)\n", errno);
        }

        // Force null termination.
        fin[len == 0 ? 0 : len - 1] = '\0';

        dbg("[notif_worker tid=%d] received input: %s (len = %d)\n", gettid(), fin, len);

        // Check if we should quit.
        if (!strncmp(fin, "stop", 4)) {
            ret = svm->abi.graal_detach_thread(isolatethread);
            if (ret) {
                err("error: failed to detach thread from isolate (error %d)", ret);
            }
            break;
        } else if (!strncmp(fin, "destroy", 7)) {
            ret = svm->abi.graal_tear_down_isolate(isolatethread);
            if (ret) {
                err("error: failed to destroy isolate (error %d)", ret);
            }
            break;
        }

        // Call the function.
        run_entrypoint(&svm->abi, svm->isolate, isolatethread, 1, 1, fin, fout);

        // Write output to pipe (+1 to count with the null terminator).
        len = strlen(fout) + 1;
        if (len >= FOUT_LEN) {
            err("warning: output over maximum length (len = %d)\n", len);
            len = FOUT_LEN;
        }

        // Force null termination.
        fout[len == 0 ? 0 : len - 1] = '\0';

        dbg("[notif_worker tid=%d] sending output: %s (len = %d)\n", gettid(), fout, len);

        // Write output len in bytes.
        if (write(svm->ostream[1], &len, sizeof(size_t)) != sizeof(size_t)) {
            err("error: failed to write output len (error %d)\n", errno);
        }

        // Write output.
        if (write(svm->ostream[1], fout, len) != len) {
            err("error: failed to write output (error %d)\n", errno);
        }
    }

    // Close all streams and quit.
    close(svm->istream[0]);
    close(svm->istream[1]);
    close(svm->ostream[0]);
    close(svm->ostream[1]);
    return NULL;
}

void invoke_svm_internal(int input_wfd, int output_rfd, const char* fin, char* fout) {
    // Calculate the size of the string, including the null terminator.
    size_t len = strlen(fin) + 1;

    dbg("[invoke_svm_internal] sending input: %s (len = %d)\n", fin, len);

    // Write len and input into pipe.
    if (write(input_wfd, &len, sizeof(size_t)) != sizeof(size_t)) {
        err("error: failed to write input len (error %d)\n", errno);
    }
    if (write(input_wfd, fin, len) != len) {
        err("error: failed to write input (error %d)\n", errno);
    }
    // Early return if fout is null (i.e., no answer is expected.)
    if (fout == NULL) {
        return;
    }
    // Read output len from pipe.
    if(read(output_rfd, &len, sizeof(size_t)) != sizeof(size_t)) {
        err("error: failed to read output len (error %d)\n", errno);
    }

    //  Read output from the pipe.
    if (read(output_rfd, fout, len) != len) {
        err("error: failed to read input (error %d)\n", errno);
    }

    dbg("[invoke_svm_internal] received output: %s (len = %d)\n", fout, len);
}

void invoke_svm(svm_sandbox_t* sandbox, const char* fin, char* fout) {
    invoke_svm_internal(sandbox->istream[1], sandbox->ostream[0], fin, fout);
}

void forked_invoke_svm(forked_svm_sandbox_t* sandbox, const char* fin, char* fout) {
    invoke_svm_internal(sandbox->inv_wfd, sandbox->inv_rfd, fin, fout);
}

void destroy_svm(svm_sandbox_t* sandbox, int reuse_isolate) {
    invoke_svm_internal(sandbox->istream[1], sandbox->ostream[0], reuse_isolate ? "stop" : "destroy", NULL);
}

void forked_destroy_svm(forked_svm_sandbox_t* sandbox, int reuse_isolate) {
    invoke_svm_internal(sandbox->inv_wfd, sandbox->inv_rfd, reuse_isolate ? "stop" : "destroy", NULL);
    close(sandbox->inv_rfd);
    close(sandbox->inv_wfd);
}

svm_sandbox_t* create_sandbox(unsigned long seed, isolate_abi_t abi, graal_isolate_t* isolate) {
    svm_sandbox_t* svm_sandbox = malloc(sizeof(svm_sandbox_t));
    memset(svm_sandbox, 0, sizeof(svm_sandbox_t));
    svm_sandbox->seed = seed;
    svm_sandbox->abi = abi;
    svm_sandbox->isolate = isolate;
    if (pipe(svm_sandbox->istream) != 0) {
        err("error: failed to create input stream pipe (error %d)\n", errno);
        return NULL;
    }
    if (pipe(svm_sandbox->ostream) != 0) {
        err("error: failed to create output stream pipe (error %d)\n", errno);
        return NULL;
    }
    return svm_sandbox;
}

forked_svm_sandbox_t* forked_create_sandbox(int cpid, int ctl_wfd, int ctl_rfd) {
    forked_svm_sandbox_t* clone = (forked_svm_sandbox_t*) malloc(sizeof(forked_svm_sandbox_t));
    clone->child_pid = cpid;
    clone->ctl_wfd = ctl_wfd;
    clone->ctl_rfd = ctl_rfd;

    // Receive streams for function invocation.
    if (read(ctl_rfd, &clone->inv_wfd, sizeof(int)) != sizeof(int)) {
        err("error: failed to receive input stream to parent");
        return NULL;
    }
    if (read(ctl_rfd, &clone->inv_rfd, sizeof(int)) != sizeof(int)) {
        err("error: failed to receive output stream to parent");
        return NULL;
    }

    // Clone file descrptors from the child into the parent.
    int cpidfd = syscall(SYS_pidfd_open, cpid, 0);
    if (cpidfd < 0) {
        err("error: failed to create pid file descriptor for child process");
        return NULL;
    }
    clone->inv_wfd = syscall(SYS_pidfd_getfd, cpidfd, clone->inv_wfd, 0);
    if (clone->inv_wfd < 0) {
        err("error: failed to copy istream from the child process");
        return NULL;
    }
    clone->inv_rfd = syscall(SYS_pidfd_getfd, cpidfd, clone->inv_rfd, 0);
    if (clone->inv_wfd < 0) {
        err("error: failed to copy ostream from the child process");
        return NULL;
    }
    if (close(cpidfd)) {
        err("error: failed to clone child pid file descriptor");
        return NULL;
    }

    log("[forked_create_sandbox] new forked sandbox: cpid = %d, ctl_rfd = %d, ctl_wfd = %d, inv_rfd = %d, inv_wfg = %d\n",
        clone->child_pid, clone->ctl_rfd, clone->ctl_wfd, clone->inv_rfd, clone->inv_wfd);
    return clone;
}

svm_sandbox_t* clone_svm(svm_sandbox_t* sandbox, int reuse_isolate) {
    graal_isolate_t* clone_isolate;

    if (reuse_isolate) {
        clone_isolate = sandbox->isolate;
    } else {
        // Create isolate with no limit (use 256*1024*1024 for 256MB).
        graal_create_isolate_params_t params;
        graal_isolatethread_t *isolatethread = NULL;
        memset(&params, 0, sizeof(graal_create_isolate_params_t));
        params.version = 1;
        params.reserved_address_space_size = 0;
        if (sandbox->abi.graal_create_isolate(&params, &clone_isolate, &isolatethread) != 0) {
            err("error: failed to create isolate for cloned svm\n");
            return NULL;
        }

        // Detach thread function isolate and quit.
        sandbox->abi.graal_detach_thread(isolatethread);
    }

    // Launch thread that will be handling requests for this svm sandbox.
    svm_sandbox_t* clone = create_sandbox(sandbox->seed, sandbox->abi, clone_isolate);
    pthread_create(&clone->thread, NULL, notif_worker, clone);
    return clone;
}

forked_svm_sandbox_t* forked_clone_svm(forked_svm_sandbox_t* sandbox, int reuse_isolate) {
    const char* cmd = "clone"; // TODO - this needs to be parametrized w/ reuse.
    size_t len = strlen(cmd) + 1; // +1 to include the null terminator.
    if (write(sandbox->ctl_wfd, &len, sizeof(size_t)) != sizeof(size_t)) {
        err("error: failed to send clone command len to child");
        return NULL;
    }
    if (write(sandbox->ctl_wfd, cmd, len) != len) {
        err("error: failed to send clone command to child");
        return NULL;
    }
    return forked_create_sandbox(sandbox->child_pid, sandbox->ctl_wfd, sandbox->ctl_rfd);
}

svm_sandbox_t* checkpoint_svm(
        const char* fpath,
        const char* meta_snap_path,
        const char* mem_snap_path,
        size_t seed,
        unsigned int concurrency,
        unsigned int requests,
        const char* fin,
        char* fout) {
    // Thread that will run the sandboxed code.
    pthread_t worker;
    // Thread that will accept all syscalls after checkpoint.
    pthread_t allower;

    // Create and initialize the memory mappings list head;
    mapping_t mappings;
    memset(&mappings, 0, sizeof(mapping_t));

    // Create and initialize the children thread list head;
    thread_t threads;
    memset(&threads, 0, sizeof(thread_t));

    // Create and initialize the checkpoint worker arguments struct.
    checkpoint_worker_args_t* wargs = malloc(sizeof(checkpoint_worker_args_t));
    memset(wargs, 0, sizeof(checkpoint_worker_args_t));

    pthread_mutex_lock(&cr_mutex);
    wargs->fin = fin;
    wargs->fout = fout;
    wargs->fpath = fpath;
    wargs->concurrency = concurrency;
    wargs->requests = requests;
    wargs->seed = seed;

    // TODO - this is for the entire process. We should reset this at some point because it will affect other sandboxes and the hydra.
    if (set_next_pid(1000*(seed+1)) == -1) {
        err("error: failed to set next pid");
        return NULL;
    }

    // If in checkpoint mode, open metadata file, wait for seccomp to be ready and handle notifications.
    int meta_snap_fd = move_to_reserved_fd(open(meta_snap_path,  O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR));
    int mem_snap_fd = move_to_reserved_fd(open(mem_snap_path,  O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR));
    if (meta_snap_fd < 0 || mem_snap_fd < 0) {
        err("error: failed to open meta (fd = %d) or memory (fd = %d) snapshot file\n", meta_snap_fd, mem_snap_fd);
    } else {
        meta_snap_fd = move_to_reserved_fd(meta_snap_fd);
        mem_snap_fd = move_to_reserved_fd(mem_snap_fd);
    }

    // Launch worker thread.
    pthread_create(&worker, NULL, checkpoint_worker, wargs);

    // Wait while the thread initilizes and installs the seccomp filter.
    while (!wargs->seccomp_fd) ;

    // Move seccomp fd to a reserved fd.
    wargs->seccomp_fd = move_to_reserved_fd(wargs->seccomp_fd);

    // Keep handling syscall notifications.
    handle_syscalls(seed, wargs->seccomp_fd, &(wargs->finished), meta_snap_fd, &mappings, &threads);

    if (!list_threads_empty(&threads)) {
        // Pause background threads before checkpointing.
        pause_background_threads(&threads);
    }

    // Merge memory mappings.
    list_mappings_merge(&mappings);

    // If in checkpoint mode, checkpoint memory.
    check_proc_maps("before_checkpoint.log", &mappings);
#ifdef DEBUG
    print_list_mappings(&mappings);
#endif
#ifdef PERF
    struct timeval st, et;
    gettimeofday(&st, NULL);
#endif
    checkpoint(meta_snap_fd, mem_snap_fd, &mappings, &threads, &wargs->abi, wargs->isolate);
#ifdef PERF
    gettimeofday(&et, NULL);
    log("checkpoint took %lu us\n", ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec));
#endif

    // Minimize our meta_snap_path by removing unnecessary system calls.
    minimize_syscalls(meta_snap_path, meta_snap_path);

    // Join thread after checkpoint (this glibc may free memory right before we checkpoint).
    // Glibc is tricky as it has internal state. We should avoid it at all cost.
    pthread_join(worker, NULL);

    if (!list_threads_empty(&threads)) {
        // Starting monitor thread to allow syscalls from the background threads.
        pthread_create(&allower, NULL, allow_syscalls, &(wargs->seccomp_fd));
        // Resume background threads before checkpointing.
        resume_background_threads(&threads);
    }

    // Close meta and mem fds.
    close(meta_snap_fd);
    close(mem_snap_fd);
    pthread_mutex_unlock(&cr_mutex);

    svm_sandbox_t* svm_sandbox = create_sandbox(seed, wargs->abi, wargs->isolate);
    pthread_create(&svm_sandbox->thread, NULL, notif_worker, svm_sandbox);
    return svm_sandbox;
}

svm_sandbox_t* restore_svm_internal(
        const char* fpath,
        const char* meta_snap_path,
        const char* mem_snap_path,
        size_t seed) {
    isolate_abi_t abi;
    graal_isolate_t* isolate;
    int last_pid;

#ifdef PERF
    struct timeval st, et;
    gettimeofday(&st, NULL);
#endif
    pthread_mutex_lock(&cr_mutex);
    restore(meta_snap_path, mem_snap_path, &abi, &isolate);
    pthread_mutex_unlock(&cr_mutex);
#ifdef PERF
    gettimeofday(&et, NULL);
    log("restore took %lu us\n", ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec));
#endif
#ifdef DEBUG
    check_proc_maps("after_restore.log", NULL);
#endif

    // Get pid that will be assigned to next created thread
    if ((last_pid = get_next_pid()) == -1) {
        err("error: failed to get next pid\n");
        return NULL;
    }
    // After having restored threads, set next pid to first pid of current sandbox
    if (set_next_pid(1000*(seed+1)) == -1) {
        err("error: failed to set next pid\n");
        return NULL;
    }

    // Launch worker thread and wait for it to finish.
    svm_sandbox_t* svm_sandbox = create_sandbox(seed, abi, isolate);
    pthread_create(&svm_sandbox->thread, NULL, notif_worker, svm_sandbox);
    // After launching original application, restore last next_pid for future threads
    if (last_pid != 1) {
        // Threads were restored so we want to change next pid
        if (set_next_pid(last_pid) == -1) {
            err("error: failed to set next pid\n");
            return NULL;
        }
    }
    return svm_sandbox;
}

svm_sandbox_t* restore_svm(
        const char* fpath,
        const char* meta_snap_path,
        const char* mem_snap_path,
        size_t seed) {
    return restore_svm_internal(fpath, meta_snap_path, mem_snap_path, seed);
}

void redirect_child_io() {
    char fpath[256];
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    // Safe close of stdin.
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) {
        err("error: failed to redirect stdin (errno %d)", errno);
        exit(EXIT_FAILURE);
    }
    if (dup2(fd, 0) < 0) {
        err("error: failed to dup2 fd %d into stdin (errno %d)", fd, errno);
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Redirecting stdout and stderr to graalvisor-<pid>.log.
    snprintf(fpath, 256, "graalvisor-%d.log", getpid());
    fd = open(fpath, O_WRONLY | O_CREAT, mode);
    if (fd < 0) {
        err("error: failed to redirect output to %s (errno %d)", fpath, errno);
        exit(EXIT_FAILURE);
    }
    if (dup2(fd, 1) < 0) {
        err("error: failed to dup2 fd %d into stdout (errno %d)", fd, errno);
        exit(EXIT_FAILURE);
    }
    if(dup2(fd, 2) < 0) {
        err("error: failed to dup2 fd %d into stderr (errno %d)", fd, errno);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

forked_svm_sandbox_t* forked_restore_svm(
        const char* fpath,
        const char* meta_snap_path,
        const char* mem_snap_path,
        unsigned long seed) {
    int p2c_pp[2];
    int c2p_pp[2];
    // Control pipe to send commands into the child.
    if (pipe(p2c_pp)) {
        err("error: failed to create parent to child pipe for forked restore");
        return NULL;
    }
    // Control pipe to read command output from the child.
    if (pipe(c2p_pp)) {
        err("error: failed to create child to parent pipe for forked restore");
        return NULL;
    }

    int pid = fork();
    if (pid == 0) {

        // Move pipe fds to the reserved range so that it doesn't clash with any restored fd.
        p2c_pp[0] = move_to_reserved_fd(p2c_pp[0]);
        c2p_pp[1] = move_to_reserved_fd(c2p_pp[1]);

        // Redirect child io to a log file.
        redirect_child_io();


#ifdef PERF
        struct timeval st, et;
        gettimeofday(&st, NULL);
#endif
        signal(SIGTERM, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        for (int i = STDERR_FILENO + 1; i < RESERVED_FDS; i++) {
            if (i != p2c_pp[0] && i !=c2p_pp[1]) {
                close(i);
            }
        }
#ifdef PERF
        gettimeofday(&et, NULL);
        log("preparing child process took %lu us\n", ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec));
#endif

        // If in the child, invoke restore and print output.
        svm_sandbox_t* sandbox = restore_svm_internal(fpath, meta_snap_path, mem_snap_path, seed);

        // Send streams for function invocation.
        if (write(c2p_pp[1], &sandbox->istream[1], sizeof(int)) != sizeof(int)) {
            err("error: failed to send invocation payload write stream to parent");
            exit(EXIT_FAILURE);
        }
        if (write(c2p_pp[1], &sandbox->ostream[0], sizeof(int)) != sizeof(int)) {
            err("error: failed to send invocation response read stream to parent");
            exit(EXIT_FAILURE);
        }

        for (;;) {
            char buffer[FOUT_LEN];
            size_t len;
            // Read len and input into pipe.
            if (read(p2c_pp[0], &len, sizeof(size_t)) != sizeof(size_t)) {
                err("error: failed to read command len (error %d)\n", errno);
                continue;
            }
            if (read(p2c_pp[0], buffer, len) != len) {
                err("error: failed to read command (error %d)\n", errno);
                continue;
            }
            if (strncmp(buffer, "clone", strlen("clone"))) {
                err("error: unknown command from parent: %s\n", buffer);
                continue;
            }

            // Clone sandbox.
            svm_sandbox_t* clone = clone_svm(sandbox, 1); // TODO - allow the parent to decide.

            // Send cloned streams for function invocation.
            if (write(c2p_pp[1], &clone->istream[1], sizeof(int)) != sizeof(int)) {
                err("error: failed to send cloned invocation payload write stream to parent\n");
                continue;
            }
            if (write(c2p_pp[1], &clone->ostream[0], sizeof(int)) != sizeof(int)) {
                err("error: failed to send cloned invocation response read stream to parent\n");
                continue;
            }
        }
    } else {
        // If in the parent, setup the sandbox and invoke.
        return forked_create_sandbox(pid, p2c_pp[1], c2p_pp[0]);
    }
}

void forked_unload_svm(forked_svm_sandbox_t* sandbox) {
    if (close(sandbox->ctl_rfd)) {
        err("error: failed to close control read stream for forked sandbox with pid %d (errno %d)\n", sandbox->child_pid, errno);
    }
    if (close(sandbox->ctl_wfd)) {
        err("error: failed to close control write stream for forked sandbox with pid %d (errno %d)\n", sandbox->child_pid, errno);
    }
    if (kill(sandbox->child_pid, SIGTERM)) {
        err("error: failed to kill forked sandbox with pid %d (errno %d)\n", sandbox->child_pid, errno);
    }
    log("[forked_unload_svm] kill forked sandbox: cpid = %d\n", sandbox->child_pid);
}