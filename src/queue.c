
#include "queue.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct QueueStruct {
	sem_t get;
	sem_t put;
	pthread_mutex_t lock;
	int next;
	int length;
	void* q[];
} Queue;


Queue *queue_alloc(int size) {
    
    Queue* queue = (Queue*) malloc(sizeof(Queue) + sizeof(void*) * size);
    
    for (int i = 0; i < size; i++) {
		queue->q[i] = NULL;
	}
	queue->length = size;
	sem_init(&(queue->get), 0, 0); // Nothing to get from queue
	sem_init(&(queue->put), 0, 1); // Queue is empty, available for adding to
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	queue->lock = lock;
	queue->next = 0;
    return queue;
}

void queue_free(Queue *queue) {
	
	free(queue);
}

void queue_put(Queue *queue, void *item) {
	
	sem_wait(&(queue->put));
	pthread_mutex_lock(&queue->lock);
	if (queue->next != queue->length) {
		queue->q[queue->next] = item;
		queue->next++;
	} else {
		printf("Queue is full.\n");
	}
	pthread_mutex_unlock(&queue->lock);
	sem_post(&(queue->get));
}

void *queue_get(Queue *queue) {
	
	sem_wait(&(queue->get));
	pthread_mutex_lock(&queue->lock);
	void* item = queue->q[0];
	int i = 1;
	while (i < queue->length && queue->q[i] != NULL) {
		queue->q[i-1] = queue->q[i];
		i++;
	}
	queue->next = i;
	pthread_mutex_unlock(&queue->lock);
	sem_post(&(queue->put));
	return item;
}

