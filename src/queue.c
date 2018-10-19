
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
	sem_t get; // Semaphore for 'getting' from the queue
	sem_t put; // Semaphore for 'putting' on the queue
	pthread_mutex_t lock;
	int length;
	void* q[]; // Is an array structure of arbitrary length, holding pointers to an arbitrary type
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
    return queue;
}

void queue_free(Queue *queue) {
	free(queue);
}

void queue_put(Queue *queue, void *item) {
	
	sem_wait(&(queue->put)); // Wait until queue is 'free' to add to
	pthread_mutex_lock(&queue->lock); // Obtain exclusive access to the queue
	int i = 0;
	while (i < queue->length) { // New items are appended to the queue
		if (queue->q[i] == NULL) { // Find a position on the queue that is empty
			queue->q[i] = item;    // and put the item there
			break;
		}
		i++;
	}
	if (i == queue->length) {
		printf("Queue is full.");
	}
	pthread_mutex_unlock(&queue->lock); // Enable other threads to access queue
	sem_post(&(queue->get)); // Post that there is something to 'get' from the queue
}

void *queue_get(Queue *queue) {
	
	sem_wait(&(queue->get)); // Wait for signal that something can be obtained
	pthread_mutex_lock(&queue->lock);
	void* item = queue->q[0]; // Get item first on queue (FIFO)
	for (int i = 1; i < queue->length; i++) {
		queue->q[i-1] = queue->q[i]; // Move all items on the queue forward one
		if (i == queue->length - 1) {
			queue->q[i] = NULL;
		}
	}
	pthread_mutex_unlock(&queue->lock);
	sem_post(&(queue->put)); // Signal that any threads can now add to the queue
	return item;
}

