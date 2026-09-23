// Important to load some headers such as sched.h.
#define _GNU_SOURCE

#include "svm-snapshot.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>

enum EXECUTION_MODE { NORMAL, CHECKPOINT, RESTORE, FRESTORE, MINIMIZE};

// Wether we are checkpointing or restoreing.
enum EXECUTION_MODE CURRENT_MODE = NORMAL;

// Path to function library.
const char* FPATH = NULL;

// Defines the number of function entrypoint invocations per thread.
unsigned int ITERS  = 1;

// Defines the number of concurrent threads to invoke the function entrypoint.
unsigned int CONC = 1;

// The seed is used to control which virtual memory ranges the svm instance will used (see svm-snapshot.h).
unsigned int SEED = 0;

void usage_exit() {
    fprintf(stderr, "Syntax: main <normal|checkpoint|restore|frestore> <path to native image app library> [concurrency [iterations [seed]]]\n");
    fprintf(stderr, "Optional: concurrency in checkpoint mode (defaults to 1).\n");
    fprintf(stderr, "Optional: iterations in checkpoint mode (defaults to 1).\n");
    fprintf(stderr, "Optional: seed (defaults to zero and ignored in normal and restore modes).\n");
    exit(1);
}

void init_args(int argc, char** argv) {
    if (argc < 3) {
        usage_exit();
    }

    // Find current mode.
    switch (argv[1][0])
    {
    case 'n':
        CURRENT_MODE = NORMAL;
        break;
    case 'c':
        CURRENT_MODE = CHECKPOINT;
        break;
    case 'r':
        CURRENT_MODE = RESTORE;
        break;
    case 'f':
        CURRENT_MODE = FRESTORE;
        break;
    case 'm':
        CURRENT_MODE = MINIMIZE;
        break;
    default:
        usage_exit();
    }

    // Find function code.
    FPATH = argv[2];

    // Find optional arguments.
    if (argc >= 4) {
        CONC = atoi(argv[3]);
    }

    if (argc >= 5) {
        ITERS = atoi(argv[4]);
    }

    if (argc >= 6) {
        SEED = atoi(argv[5]);
    }
}

int main(int argc, char** argv) {
    // INput and OUTput for function ran by isolate
    const char* fin = "(null)";
    char  fout[FOUT_LEN];

    // Enable unshare() and access to /proc/sys/kernel/ns_last_pid
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, 21, 0, 0) == -1) {
        perror("prctl CAP_SYS_ADMIN");
        return 1;
    }

    // Lock process privileges
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1) {
        perror("prctl NO_NEW_PRIVS");
        return 1;
    }

    // Initialize arguments.
    init_args(argc, argv);

    // Disable buffering for stdout.
    setvbuf(stdout, NULL, _IONBF, 0);

    if (CURRENT_MODE == RESTORE) {
        svm_sandbox_t* sandbox = restore_svm(FPATH, "metadata.snap", "memory.snap", SEED);
        invoke_svm(sandbox, fin, fout);
    } else if (CURRENT_MODE == FRESTORE) {
        forked_svm_sandbox_t* sandbox = forked_restore_svm(FPATH, "metadata.snap", "memory.snap", SEED);
        forked_invoke_svm(sandbox, fin, fout);
    } else if (CURRENT_MODE == CHECKPOINT) {
        checkpoint_svm(FPATH, "metadata.snap", "memory.snap", SEED, CONC, ITERS, fin, fout);
    } else {
        run_svm(FPATH, CONC, ITERS, fin, fout);
    }

    fprintf(stdout, "function(%s) -> %s\n", fin, fout);

    // Flush any open streammed file.
    fflush(NULL);

    // Exit even if there are unfinished threads running in function code.
    exit(0);
}
