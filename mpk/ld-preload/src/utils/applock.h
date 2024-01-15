#include <pthread.h>

typedef struct Node {
    char* key;
    pthread_mutex_t mutex;
    struct Node* next;
} Node;

typedef struct {
    Node** table;
    size_t size;
} AppLock;

void init_applock_map(AppLock* map, size_t size);
void insert_applock(AppLock* map, char* key);
void lock_app(AppLock* map, char* key);
void unlock_app(AppLock* map, char* key);
