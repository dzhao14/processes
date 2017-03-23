#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

#include "queue.h"

// TODO: Make this an interprocess queue.

queue*
make_queue()
{
    int pages = 1 + sizeof(queue) / 4096;
    queue* qq = malloc(pages * 4096); // FIXME: Queue should be shared.
    return qq;
}

void
free_queue(queue* qq)
{
    assert(qq->qii == qq->qjj);
    free(qq);
}

void
queue_put(queue* qq, job msg)
{
    unsigned int ii = atomic_fetch_add(&(qq->qii), 1);
    qq->jobs[ii % QUEUE_SIZE] = msg;
}

job
queue_get(queue* qq)
{
    unsigned int jj = atomic_fetch_add(&(qq->qjj), 1);
    return qq->jobs[jj % QUEUE_SIZE];
}

