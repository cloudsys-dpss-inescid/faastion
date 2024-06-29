#include <pthread.h>
#include "shared_queue.h"

int is_empty(shared_queue *queue);
int is_full(shared_queue *queue);

/**
 * Initialize queue with zeros and create locks
*/
void shared_queue_init(shared_queue *queue) {
    pthread_mutex_init(&queue->mutex, NULL);
    queue->in = 0;
    queue->out = 0;
    queue->consumers = 0;
}

int is_full(shared_queue *queue) {
    return queue->in - queue->out == QUEUE_SIZE;
}

int is_empty(shared_queue *queue) {
    return queue->in == queue->out;
}

/**
 * Insert element at the end of the queue
 * @pid: process id of new tracee
*/
void put(shared_queue *queue, int nfd, int sfd) {
    pthread_mutex_lock(&queue->mutex);
    while (is_full(queue)) {
        pthread_mutex_unlock(&queue->mutex);
        pthread_mutex_lock(&queue->mutex);
    }
    queue->data[queue->in % QUEUE_SIZE].nfd = nfd;
    queue->data[queue->in % QUEUE_SIZE].sfd = sfd;
    queue->in += 1;
    queue->consumers += 1;
    pthread_mutex_unlock(&queue->mutex);
}

/**
 * Get next element in queue 
 * @return: On sucess, zero is returned. On error, -1 is returned.
*/
int head(shared_queue *queue, struct tfd *target) {
    pthread_mutex_lock(&queue->mutex);
    if (is_empty(queue)) {
        pthread_mutex_unlock(&queue->mutex);
        return -1;
    }
    target->nfd = queue->data[queue->out % QUEUE_SIZE].nfd;
    target->sfd = queue->data[queue->out % QUEUE_SIZE].sfd;
    queue->out += 1;
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

void queue_wait(shared_queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->consumers == 0) {
        pthread_mutex_unlock(&queue->mutex);
        pthread_mutex_lock(&queue->mutex);
    }
    queue->consumers -= 1;
    pthread_mutex_unlock(&queue->mutex);
}