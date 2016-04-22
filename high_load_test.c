/**
* @file high test example
* @author Karl Hiramoto <karl@hiramoto.org>
* @brief high load with lots of threads and lots of jobs
*
* Example code is Distributed under Dual BSD / LGPL licence
* Copyright 2009 Karl Hiramoto
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#include "work_queue.h" 

struct prg_ctx
{
	struct workqueue_ctx *ctx;
	int counter;
};

void callback_func(void *data)
{
	struct prg_ctx *prg = (struct prg_ctx *) data;
	int ret;
	int i;

	for (i = 0; i < 1234567; i++) {
		ret += i;
	}
	if (ret)
		prg->counter++;
}

int main(int argc, char *argv[]) {
	struct prg_ctx prg;
	int i;
	int num_jobs=1024;
	int ret;
	printf("starting\n");
	prg.counter = 0;
	prg.ctx = workqueue_init(32, NULL);

	for (i = 0; i < num_jobs; i++) {
		ret = workqueue_add_work(prg.ctx, callback_func, &prg);

		if (ret >= 0) {
			/* Added job ok */
		} else if (ret == -ENOMEM){
			printf("No memory adding work %d of %d\n", i, num_jobs);
			sleep(1);
		} else {
			printf("Error adding job err=%d\n", ret);
			workqueue_show_status(prg.ctx, stdout);
		}
	}

	for (i = 0; i < num_jobs/2; i++) {
		ret = workqueue_add_work(prg.ctx, callback_func, &prg);

		if (ret >= 0) {
			/* Added job ok */
		} else if (ret == -ENOMEM){
			printf("No memory adding work %d of %d\n", i, num_jobs);
			sleep(1);
		} else {
			printf("Error adding job err=%d\n", ret);
			workqueue_show_status(prg.ctx, stdout);
		}
	}
	workqueue_show_status(prg.ctx, stdout);

	for (i = 0; i < num_jobs/2; i++) {
		ret = workqueue_add_work(prg.ctx, callback_func, &prg);

		if (ret >= 0) {
			/* Added job ok */
		} else if (ret == -ENOMEM){
			printf("No memory adding work %d of %d\n", i, num_jobs);
			sleep(1);
		} else {
			printf("Error adding job err=%d\n", ret);
			workqueue_show_status(prg.ctx, stdout);
		}
	}
	workqueue_show_status(prg.ctx, stdout);

	for (i = 30; i && (ret = workqueue_get_queue_len(prg.ctx)); i--) {
	  	printf("waiting for %d jobs \n", ret);
		sleep(1);
	}
	
	workqueue_destroy(prg.ctx);
	printf("count =%d \n", prg.counter);
	return 0;
}
