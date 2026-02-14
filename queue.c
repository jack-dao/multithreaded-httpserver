#include "queue.h"
#include <stdlib.h>

void init_queue(queue_t *q) {
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void enqueue(queue_t *q, int *client_socket) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->client_socket = client_socket;
    new_node->next = NULL;

    pthread_mutex_lock(&q->mutex);

    if (q->tail == NULL) {
        q->head = new_node;
    } else {
        q->tail->next = new_node;
    }
    q->tail = new_node;

    pthread_cond_signal(&q->cond);
    
    pthread_mutex_unlock(&q->mutex);
}

int* dequeue(queue_t *q) {
    pthread_mutex_lock(&q->mutex);

    while (q->head == NULL) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    int *client_socket = q->head->client_socket;
    node_t *temp = q->head;
    
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    
    free(temp);
    pthread_mutex_unlock(&q->mutex);
    
    return client_socket;
}