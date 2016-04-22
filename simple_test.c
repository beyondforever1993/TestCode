/**
* @file simple test example
* @author Karl Hiramoto <karl@hiramoto.org>
* @brief simple example showing how to enqueue worker jobs
*
* Example code is Distributed under Dual BSD / LGPL licence
* Copyright 2009 Karl Hiramoto
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "work_queue.h"


struct prg_ctx
{
	struct workqueue_ctx *ctx;
	int counter;
};


static void callback_func(void *data)
{
	struct prg_ctx *prg = (struct prg_ctx *) data;
	int ret;

	prg->counter++;
	printf("counter=%d\n", prg->counter);
	if (prg->counter % 2)
		sleep(1);

	if ((prg->counter % 4) == 0) {
		printf("reschedule this job\n");
		ret = workqueue_add_work(prg->ctx, callback_func, prg);

		if (ret >= 0) {
			printf("Added job %d \n", ret);
			workqueue_show_status(prg->ctx, stdout);
		} else {
		  	printf("Error adding job err=%d\n", ret);
		}
	}
}

int main(int argc, char *argv[]) {
	struct prg_ctx prg;
	int i;
	int num_jobs=6;
	int ret;
	printf("starting\n");
	prg.counter = 0;
	prg.ctx = workqueue_init(1, NULL);

	for (i = 0; i < num_jobs; i++) {
		ret = workqueue_add_work(prg.ctx, callback_func, &prg);

		if (ret >= 0) {
			printf("Added job %d \n", ret);
		} else {
		  	printf("Error adding job err=%d\n", ret);
		}
	}
	workqueue_show_status(prg.ctx, stdout);

	for (i = 0; i < num_jobs; i++) {
		printf("job %d is queued=%d running=%d queued_or_running=%d\n", i,
			workqueue_job_queued(prg.ctx, i),
			workqueue_job_running(prg.ctx, i),
			workqueue_job_queued_or_running(prg.ctx, i));
	}

	for (i = 0; i < num_jobs/2; i++) {
		ret = workqueue_add_work(prg.ctx, callback_func, &prg);

		if (ret >= 0) {
			printf("Added job %d \n", ret);
		} else {
		  	printf("Error adding job err=%d\n", ret);
		}
	}
	workqueue_show_status(prg.ctx, stdout);

	for (i = 0; i < num_jobs/2; i++) {
		ret = workqueue_add_work(prg.ctx, callback_func, &prg);

		if (ret >= 0) {
			printf("Added job %d \n", ret);
		} else {
		  	printf("Error adding job err=%d\n", ret);
		}
	}
	workqueue_show_status(prg.ctx, stdout);

	for (i = 20; i && (ret = workqueue_get_queue_len(prg.ctx)) > 5; i--) {
	  	printf("waiting for %d jobs \n", ret);
		sleep(1);
	}

	// empty out remaining jobs and wait for running job to finish
	workqueue_empty_wait(prg.ctx);

	workqueue_destroy(prg.ctx);

#ifdef WINDOWS
	system("pause");
#endif
	return 0;
}
