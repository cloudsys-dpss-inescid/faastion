#ifndef __SHARED_QUEUE_H__
#define __SHARED_QUEUE_H__

#define QUEUE_SIZE 32

struct tfd {
    int nfd; // notification file-descriptor
    int sfd; // signal file-descriptor
};

typedef struct shared_queue_struct {
    struct tfd data[QUEUE_SIZE];
    int in;
    int out;
    int consumers;
    pthread_mutex_t mutex;
} shared_queue;

void put(shared_queue *queue, int nfd, int sfd);
int head(shared_queue *queue, struct tfd *target);
void queue_wait(shared_queue *queue);
void shared_queue_init(shared_queue *queue);

#endif