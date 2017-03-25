#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <semaphore.h>
#include "job.h"
#include "factor.h"
#include "queue.h"
extern shq* shared;

int
main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage:\n");
        printf("  ./main procs start count\n");
        return 1;
    }

	shared_init();

    int procs = atoi(argv[1]);
	int kids[procs + 1];
    int64_t start = atol(argv[2]);
    int64_t count = atol(argv[3]);

	for (int i = 0; i < procs; i++) {
		kids[i] = fork();
		if (kids[i] == 0) {
			work_off_jobs();
			exit(1);
		}
	}

	kids[procs + 1] = fork();
	if (kids[procs + 1] == 0) {
		print_results(count);
		exit(1);
	}

    for (int64_t ii = 0; ii < count; ++ii) {
        job jj = make_job(start + ii);
        submit_job(jj);
    }
	
    //factor_wait_done(kids, procs);
    factor_cleanup(kids, procs);

    return 0;
}
