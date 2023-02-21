#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                          } while (0)

struct arguments {
    int* buffer;
    int pkey;
};

arguments* get_thread_args();
int* mem_alloc();
int key_alloc();
