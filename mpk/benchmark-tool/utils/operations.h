#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct arguments {
    void* buffer;
    size_t buffer_size;
    int pkey;
};

arguments* get_thread_args(int numberPages);
void* mem_alloc(int numberPages);
int key_alloc();
