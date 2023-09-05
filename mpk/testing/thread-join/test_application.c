#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

pthread_t workers[2];

void* hello(void *args) {
    printf("Hello from thread with id %ld!\n", pthread_self());
    return NULL;
}

void create_and_join() {
    for (int i = 0; i < 2; i++) {
        if (pthread_create(&workers[i], NULL, hello, NULL) != 0){
            perror("Can't create thread\n");
            exit(1);
        }
    }

    for(int i = 0; i < 2; i++) {
        if(pthread_join(workers[i], NULL)) {
            perror("Thread can't join\n");
            exit(1);
        }
    }
}

int main() {
    create_and_join();
    
    for(int i = 0; i < 2; i++) {
        if(pthread_join(workers[i], NULL)) {
            printf("Thread with id %ld can't join\n", workers[i]);
        }
    }

    return 0;
}
