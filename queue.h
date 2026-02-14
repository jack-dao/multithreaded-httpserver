#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

typedef struct node {
    int *client_socket;
    struct node* next;
} node_t;

typedef struct {
    node_t *head;
    node_t *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} queue_t;

void init_queue(queue_t *q);
void enqueue(queue_t *q, int *client_socket);
int* dequeue(queue_t *q);

#endif
