#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <semaphore.h>

#include "job.h"

#define QUEUE_SIZE 16
#define atomic _Atomic

typedef struct queue {
    job jobs[QUEUE_SIZE];
    atomic unsigned int qii; // Input index.
    atomic unsigned int qjj; // Output index.
} queue;

typedef struct shq {
    queue* iqueue;
    queue* oqueue;
    sem_t isem_items;
    sem_t isem_space;
    sem_t osem_items;
    sem_t osem_space;
} shq;

queue* make_queue();
void   free_queue(queue* qq);

void queue_put(queue* qq, job jj);
job  queue_get(queue* qq);

#endif
