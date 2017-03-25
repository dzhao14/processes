#include<sys/mman.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

#include "job.h"
#include "queue.h"
#include "factor.h"

static queue* iqueue;
static queue* oqueue;

static int worker_count = 0;
//static pid_t workers[64];
//static pid_t printer;

shq* shared = 0;

void
shared_init() {
	shared = mmap(0, 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	shared->iqueue = make_queue();
	shared->oqueue = make_queue();
	sem_init(&(shared->isem_items), 1, 0);
	sem_init(&(shared->osem_items), 1, 0);
    sem_init(&(shared->isem_space), 1, QUEUE_SIZE);
    sem_init(&(shared->osem_space), 1, QUEUE_SIZE);

}

void
work_off_jobs()
{
    while (1) {
		int rv = sem_wait(&(shared->isem_items));
		assert(rv == 0);
        job jj = queue_get(shared->iqueue);
		rv = sem_post(&(shared->isem_space));
		assert(rv == 0);
        if (jj.number < 0) {
            break;
        }

        factor(jj.number, &(jj.count), &(jj.factors[0]));
		rv = sem_wait(&(shared->osem_space));
		assert(rv == 0);
        queue_put(shared->oqueue, jj);
		rv = sem_post(&(shared->osem_items));
		assert(rv == 0);
    }
}

void
print_results(int64_t count)
{
    int64_t oks = 0;

	while (1) {
		int rv = sem_wait(&(shared->osem_items));
		assert(rv == 0);

        job res = get_result();
		rv = sem_post(&(shared->osem_space));
		
		if (res.number == -1) {
			break;
		}

        printf("%ld: ", res.number);
        int64_t prod = 1;
        for (int64_t jj = 0; jj < res.count; ++jj) {
            int64_t xx = res.factors[jj];
            prod *= xx;
            printf("%ld ", xx);
        }
        printf("\n");

        if (prod == res.number) {
            ++oks;
        }
        else {
            fprintf(stderr, "Warning: bad factorization");
        }
    }

    printf("Factored %ld / %ld numbers.\n", oks, count);
}

void
factor_wait_done()
{
    // FIXME: Why is this here?
}

void
factor_init(int num_procs, int64_t count)
{
    if (iqueue == 0) iqueue = make_queue();
    if (oqueue == 0) oqueue = make_queue();

    // FIXME: Spawn N worker procs and a printing proc.
}

void
factor_cleanup(int kids[], int procs)
{
    job done = make_job(-1);

    for (int ii = 0; ii < procs; ++ii) {
        submit_job(done);
    }

	for (int i = 0; i < procs; i++) {
		int status;
		waitpid(kids[i], &status, 0);
	}

	int rv = sem_wait(&(shared->osem_space));
	assert(rv == 0);
	queue_put(shared->oqueue, done);
	rv = sem_post(&(shared->osem_items));
	assert(rv == 0);
	

    // FIXME: Make sure all the workers are done.
	int status;
	waitpid(kids[procs + 1], &status, 0);


    free_queue(shared->iqueue);
    free_queue(shared->oqueue);
}

void
submit_job(job jj)
{
	int rv = sem_wait(&(shared->isem_space));
	assert(rv == 0);
    queue_put(shared->iqueue, jj);
	rv = sem_post(&(shared->isem_items));
	assert(rv == 0);
}

job
get_result()
{
    return queue_get(shared->oqueue);
}

static
int64_t
isqrt(int64_t xx)
{
    double yy = ceil(sqrt((double)xx));
    return (int64_t) yy;
}

void
factor(int64_t xx, int64_t* size, int64_t* ys)
{
    int jj = 0;

    while (xx % 2 == 0) {
        ys[jj++] = 2;
        xx /= 2;
    }

    for (int64_t ii = 3; ii <= isqrt(xx); ii += 2) {
        int64_t x1 = xx / ii;
        if (x1 * ii == xx) {
            ys[jj++] = ii;
            xx = x1;
            ii = 1;
        }
    }

    ys[jj++] = xx;
    *size = jj;
}

