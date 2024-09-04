#include <stdio.h>
#include <stdlib.h>

/*
 * Note: allocate list ahead of time
 *
 * `set_thread_domain` with dynamic nodes might
 * call mmap which will trigger the monitor
 * resulting in a deadlock
 *
 */ 

typedef struct List {
    int num_threads;
    pid_t *values;
    int max_size;
} List;

int get_size(List *list) {
    return list->num_threads;
}

List *new_list() {
    List *l = (List *)malloc(sizeof(List));
    l->max_size = 20;
    l->values = (pid_t *)calloc(l->max_size, sizeof(pid_t));
    return l;
}

void add_node(List* list, pid_t pid) {
    if (list->num_threads >= list->max_size) {
        // FIXME: implement realloc or just increase the max_size
        fprintf(stderr, "no space left on list\n");
        exit(1);
    }

    for (int i = 0; i < list->max_size; i++) {
        if (!list->values[i]) {
            list->values[i] = pid;
            list->num_threads++;
            return;
        }
    }
}

void remove_node(List *list, pid_t pid) {
    for (int i = 0; i < list->max_size; i++) {
        if (list->values[i] == pid) {
            list->values[i] = 0;
            list->num_threads--;
            return;
        }
    }
}

void print_list(List *list) {
    for (int i = 0; i < list->max_size; i++) {
        if (list->values[i])
            printf("PID: %d\n", list->values[i]);
    }
}

int lookup_node(List *list, pid_t pid) {
    for (int i = 0; i < list->max_size; i++) {
        if (list->values[i] == pid) {
            return 1;
        }
    }
    return 0;
}