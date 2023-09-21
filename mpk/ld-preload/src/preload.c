#include "preload.h"

#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define NUM_DOMAINS 16

/* The following is the x86-64-specific BPF boilerplate code for checking
    that the BPF program is running on the right architecture. */

#define X86_64_CHECK_ARCH \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                (offsetof(struct seccomp_data, arch))), \
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0), \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL), \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                (offsetof(struct seccomp_data, nr)))

/* File descriptors */
struct NotifyFileDescriptor notifyFDs[NUM_DOMAINS];

/* Maps */
AppMap appMap;
ThreadMap threadMap;

/* Lazy loading array */
char* appIDs[NUM_DOMAINS];

pthread_mutex_t mutex;

/* dlopen function pointer */
static void * ( * real_dlopen)(const char * , int) = NULL;

/* seccomp system call */
static int
seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(SYS_seccomp, operation, flags, args);
}

int
is_domain_empty(int domain)
{
    return threadMap.buckets[domain]->nthreads == 0;
}

/* extern functions */
void
lock()
{
    pthread_mutex_lock(&mutex);
}

void
unlock()
{
    pthread_mutex_unlock(&mutex);
}

int
find_empty_domain()
{
    for (int domain = 1; domain < NUM_DOMAINS; domain++) {
        if (is_domain_empty(domain)) /* is domain empty */
            return domain;
    }
    return -1; /* Empty Domain not found */
}

void 
set_permissions(const char* id, int protectionFlag, int pkey)
{
    size_t count;
    MemoryRegion* regions = get_regions(appMap, (char*)id, &count);

    for (size_t i = 0; i < count; ++i) {
        if (pkey_mprotect(regions[i].address, regions[i].size, protectionFlag, pkey) == -1) {
            fprintf(stderr, "pkey_mprotect error\n");
            exit(EXIT_FAILURE);
        }
    }
}

void 
insert_app_id(int domain, const char* id)
{
    appIDs[domain] = strdup(id);
}

int
find_app_domain(const char* id)
{
    for (int domain = 0; domain < NUM_DOMAINS; domain++) {
        if (appIDs[domain] != NULL && !strcmp(id, appIDs[domain])) {
            return domain;
        }
    }
    return -1; /* App not found */
}

char *
get_app_id(int domain)
{
    return (appIDs[domain] != NULL) ? appIDs[domain] : "";
}

/* Installs a seccomp filter that blocks all pkey related system calls;
    the filter generates user-space notifications (SECCOMP_RET_USER_NOTIF)
    on all other system calls.

    The function assigns a file descriptor to a specific domain from which 
    the user-space notifications can be fetched. */

void
install_notify_filter(int domain)
{    
    struct sock_filter filter[] = {
        X86_64_CHECK_ARCH,

        /* pkey_*(2) triggers KILL signal */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_mprotect, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_alloc, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_free, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        /* mmap(2), clone3(2) and exit(2) trigger notifications to user-space supervisor */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_mmap, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_clone3, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_exit, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

        /* Every other system call is allowed */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
        .len = ARRAY_SIZE(filter),
        .filter = filter,
    };

    /* Install the filter with the SECCOMP_FILTER_FLAG_NEW_LISTENER flag;
        as a result, seccomp() returns a notification file descriptor. */

    int nfd = seccomp(SECCOMP_SET_MODE_FILTER,
                        SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (nfd == -1)
        err(EXIT_FAILURE, "seccomp-install-notify-filter");

    notifyFDs[domain].fd = nfd;
    signal_semaphore(&notifyFDs[domain]);
}

/* Library loading */
void *
dlopen(const char * input, int flag)
{
    if (real_dlopen == NULL) {
        real_dlopen = (void *(*) (const char *, int)) dlsym(RTLD_NEXT, "dlopen");
    }
    
    if (!input || strchr(input, ':') == NULL) {
        return real_dlopen(input, flag);
    }

    // Parse input
    char pathname[256] = "";
    char libname[256] = "lib";
    char id[256] = "";
    char filename[256] = "";
    
    char * basename = extract_basename(input);
    sscanf(basename, "%[^:]:%s", id, filename);
    strcat(libname, filename);

    size_t size = strlen(input) - strlen(basename);
    strncpy(pathname, input, size);
    strcat(pathname, libname);

    void *handle = real_dlopen(pathname, RTLD_NOW | RTLD_DEEPBIND | RTLD_GLOBAL);

    SEC_DBM("Storing %s addresses and sizes in map...", libname);
    get_memory_regions(&appMap, id, pathname);

    remove(input);
    return handle;
}


/* Check that the notification ID provided by a SECCOMP_IOCTL_NOTIF_RECV
    operation is still valid. It will no longer be valid if the target
    has terminated or is no longer blocked in the system call that
    generated the notification (because it was interrupted by a signal).

    This operation can be used when doing such things as accessing
    /proc/PID files in the target process in order to avoid TOCTOU race
    conditions where the PID that is returned by SECCOMP_IOCTL_NOTIF_RECV
    terminates and is reused by another process. */

static bool
cookie_is_valid(int notifyFd, uint64_t id)
{
    return ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_ID_VALID, &id) == 0;
}

/* Allocate buffers for the seccomp user-space notification request and
    response structures. It is the caller's responsibility to free the
    buffers returned via 'req' and 'resp'. */

static void
alloc_seccomp_notif_buffers(struct seccomp_notif **req,
                        struct seccomp_notif_resp **resp,
                        struct seccomp_notif_sizes *sizes,
                        int domain)
{
    size_t  resp_size;

    /* Discover the sizes of the structures that are used to receive
        notifications and send notification responses, and allocate
        buffers of those sizes. */

    if (seccomp(SECCOMP_GET_NOTIF_SIZES, 0, sizes) == -1)
        err(EXIT_FAILURE, "[S%d]: seccomp-SECCOMP_GET_NOTIF_SIZES", domain);

    *req = malloc(sizes->seccomp_notif);
    if (*req == NULL)
        err(EXIT_FAILURE, "[S%d]: malloc-seccomp_notif", domain);

    /* When allocating the response buffer, we must allow for the fact
        that the user-space binary may have been built with user-space
        headers where 'struct seccomp_notif_resp' is bigger than the
        response buffer expected by the (older) kernel. Therefore, we
        allocate a buffer that is the maximum of the two sizes. This
        ensures that if the supervisor places bytes into the response
        structure that are past the response size that the kernel expects,
        then the supervisor is not touching an invalid memory location. */

    resp_size = sizes->seccomp_notif_resp;
    if (sizeof(struct seccomp_notif_resp) > resp_size)
        resp_size = sizeof(struct seccomp_notif_resp);

    *resp = malloc(resp_size);
    if (*resp == NULL)
        err(EXIT_FAILURE, "[S%d]: malloc-seccomp_notif_resp", domain);

}

static void
handle_mmap(struct seccomp_notif *req, struct seccomp_notif_resp *resp, int domain) 
{
    SEC_DBM("\t----mmap syscall----");

    void *mapped_mem = mmap((void *)req->data.args[0], req->data.args[1],
                     req->data.args[2], req->data.args[3],
                     req->data.args[4], req->data.args[5]);

    if (mapped_mem == MAP_FAILED) {
        /* If mmap() failed in the supervisor, pass the error
            back to the target */

        resp->error = -errno;
        SEC_DBM("\t[S%d]: failure! (errno = %d; %s)\n", domain, errno,
                strerror(errno));
    }
    else {
        if (pkey_mprotect(mapped_mem, req->data.args[1], req->data.args[2], 1) == -1) {
            resp->error = 1;            /* random value different than 0 */
            perror("pkey_mprotect");
            return;
        }

        resp->error = 0;                /* "Success" */
        resp->val = (__s64)mapped_mem;  /* return value of mmap() in target */

        SEC_DBM("\t[S%d]: success! spoofed return = %p; spoofed val = %lld\n",
                domain, mapped_mem, resp->val);
    }
}

static void 
handle_clone3(struct seccomp_notif_resp *resp, int domain)
{
    SEC_DBM("\t---clone3 syscall---");

    insert_thread(&threadMap, domain);
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
}

static void 
handle_exit(struct seccomp_notif_resp *resp, int domain)
{
    SEC_DBM("\t----exit syscall----");

    remove_thread(&threadMap, domain);
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
}


/* Handle notifications that arrive via the SECCOMP_RET_USER_NOTIF file
    descriptor, 'notifyFd'. */

static void
handle_notifications(int notifyFd, int domain)
{
    struct seccomp_notif        *req;
    struct seccomp_notif_resp   *resp;
    struct seccomp_notif_sizes  sizes;

    alloc_seccomp_notif_buffers(&req, &resp, &sizes, domain);

    insert_thread(&threadMap, domain);

    /* Loop handling notifications */

    for (;;) {

        /* Wait for next notification, returning info in '*req' */

        memset(req, 0, sizes.seccomp_notif);
        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            if (errno == EINTR)
                continue;
            err(EXIT_FAILURE, "\t[S%d]: ioctl-SECCOMP_IOCTL_NOTIF_RECV", domain);
        }

        SEC_DBM("\t[S%d]: received notifaction id [%lld], from tid: %d, syscall nr: %d\n", 
                domain, req->id, req->pid, req->data.nr);

        if (!cookie_is_valid(notifyFd, req->id)) {
            perror("ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
            continue;
        }

        /* Prepopulate some fields of the response */

        resp->id = req->id;     /* Response includes notification ID */
        resp->flags = 0;
        resp->val = 0;

        // Handle specific syscalls
        switch(req->data.nr) {
            case __NR_mmap:
                handle_mmap(req, resp, domain);
                break;
            case __NR_clone3:
                handle_clone3(resp, domain);
                break;
            case __NR_exit:
                handle_exit(resp, domain);
                break;
            default:
                break;
        }

        /* Send a response to the notification */

        SEC_DBM("\t[S%d]: sending response "
                "(flags = %#x; val = %lld; error = %d)",
                domain, resp->flags, resp->val, resp->error);

        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            if (errno == ENOENT)
                SEC_DBM("\t[S%d]: response failed with ENOENT; "
                        "perhaps target process's syscall was "
                        "interrupted by a signal?\n", domain);
            else
                perror("ioctl-SECCOMP_IOCTL_NOTIF_SEND");
        }
        SEC_DBM("\t--------------------\n");

        if (is_domain_empty(domain))
            break;
    }

    free(req);
    free(resp);
    SEC_DBM("\t[S%d]: terminating **********\n", domain);
}

/* Implementation of the supervisor thread:

    (1) obtains the notification file descriptor
    (2) handles notifications that arrive on that file descriptor. */

static void *
supervisor(void *arg)
{   
    int* domain = (int*)arg;

    loop:
        SEC_DBM("\t[S%d]: waiting for semaphore...", *domain);
        wait_semaphore(&notifyFDs[*domain]);
        SEC_DBM("\t[S%d]: received signal", *domain);

        handle_notifications(notifyFDs[*domain].fd, *domain);

    goto loop;

    return NULL;
}

static void 
__attribute__((constructor)) init(void)
{
    init_app_map(&appMap);
    init_thread_map(&threadMap);

    init_app_array(appIDs);
    init_notify_array(notifyFDs);

    pthread_mutex_init(&mutex, NULL);

    if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK, NUM_DOMAINS)) {
        exit(EXIT_FAILURE);
    }

    pthread_t workers[NUM_DOMAINS];

    for (int i = 0; i < NUM_DOMAINS; i++) {
        int *domain = (int *)malloc(sizeof(int));
        *domain = i;
        pthread_create(&workers[i], NULL, supervisor, domain);
    }
}