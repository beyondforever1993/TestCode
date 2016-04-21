/**
* @file Workqueue.c
* @author Karl Hiramoto <karl@hiramoto.org>
* @brief Library for work queues.
*
* Distributed under LGPL see COPYING.LGPL
* Copyright 2009 Karl Hiramoto
*/

// #define DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#ifdef DEBUG
#define DEBUG_MSG(fmt, s...) { printf("%s:%d " fmt, __FUNCTION__,__LINE__, ## s); }
#else
#define DEBUG_MSG(fmt, s...)
#endif
#define ERROR_MSG(fmt, s...) { fprintf(stderr, "ERROR %s:%d " fmt, __FUNCTION__,__LINE__, ## s); }

#define LOCK_MUTEX(mutex)  { DEBUG_MSG("LOCKING " #mutex " get %s:%d\n" , __FUNCTION__,__LINE__); pthread_mutex_lock(mutex); DEBUG_MSG("LOCKED " #mutex " got %s:%d \n" , __FUNCTION__,__LINE__); }

#define TRY_LOCK_MUTEX(mutex, r)  { DEBUG_MSG("TRYLOCK " #mutex " get %s:%d\n" , __FUNCTION__,__LINE__); r = pthread_mutex_trylock(mutex); DEBUG_MSG ("TRYLOCK " #mutex " got %s:%d r=%d \n" , __FUNCTION__,__LINE__,r); }

#define UNLOCK_MUTEX(x)	 { DEBUG_MSG("UNLOCK " #x " %s:%d\n" , __FUNCTION__,__LINE__); pthread_mutex_unlock(x); }

#define bool  int
#define true  (1)
#define false (0)



#include "work_queue.h"


struct wq_dequeue_item
{
    struct workqueue_job *wq_job;
    struct wq_dequeue_item *next;
};

struct double_ended_queue
{
    struct wq_dequeue_item *head;
    struct wq_dequeue_item *tail;
};


/**
* @struct workqueue_job
* @brief Every scheduled or running job has a instance of this struct
* @brief This data is private to the library
*/
struct workqueue_job
{
	unsigned int job_id; /** Job ID 1st job is 1 then goes up */
	workqueue_func_t func; /** Callback function pointer*/
	void * data; /** Data to pass to callback pointer */
};

/**
* @struct workqueue_thread
* @brief Every worker thread has an instance of this struct.
* @brief This data is private to the library.
*/
struct workqueue_thread
{

	pthread_mutex_t mutex; /** used to lock this struct and thread*/
	pthread_mutex_t job_mutex; /** locked while job is running */
	pthread_t thread_id;   /** ID returned by pthread_create() */
	int       thread_num;  /** Application-defined thread # */
	bool      keep_running; /** while true schedule next job */
	struct workqueue_ctx *ctx; /** parent context*/
	struct workqueue_job *job; /** Currently running job or NULL if no job*/
};

/**
* @struct workqueue_ctx
* @brief This the the context of the library. Multiple contexts are permited.
* @brief This data is private to the library.
*/

struct workqueue_ctx
{
	pthread_mutex_t mutex; /** used to lock this struct */
	pthread_cond_t work_ready_cond; /** used to signal waiting threads that new work is ready */
	pthread_mutex_t cond_mutex; /** used to lock condition variable*/
	struct worker_thread_ops *ops; /** worker init cleanup functions */
	int num_worker_threads; /** Number of worker threads this context has */
	int job_count; /** starts at 0 and goes to 2^31 then back to 0 */
	//int queue_size; /** max number of jobs that can be queued */
	int waiting_jobs; /** current number of jobs in queue */
	struct workqueue_thread **thread; /** array of num_worker_threads */
	//struct workqueue_job **queue; /** array of queue_size */
    struct double_ended_queue *queue;
};


static void init_dequeue(struct double_ended_queue *dequeue);
static int  add_item_at_end(struct double_ended_queue *dequeue, struct workqueue_job *job);
static int  del_item_at_begin(struct double_ended_queue *dequeue, struct wq_dequeue_item**job);
static int  get_dequeue_len(struct double_ended_queue *dequeue);
static int destroy_dequeue(struct double_ended_queue *dequeue);

/* initializes elements of structure */
static void init_dequeue ( struct double_ended_queue *dequeue)
{
    dequeue->head = dequeue->tail = NULL;
}

/* adds item at the end of dequeue */
static int add_item_at_end(struct double_ended_queue *dequeue, struct workqueue_job *job)
{
    struct wq_dequeue_item *item = NULL;
    if (!dequeue || !job) {
        return -EINVAL;
    }

    item = (struct wq_dequeue_item *)calloc(1, sizeof(struct wq_dequeue_item));
    if (!item) {
        return -ENOMEM;
    }

    item->wq_job = job;
    item->next = NULL;

    if(dequeue->head == NULL)
        dequeue->head = item;
    else
        dequeue->tail->next =item;
    dequeue->tail = item;

    return 0;
}

/* deletes item from begining of dequeue */
static int del_item_at_begin(struct double_ended_queue *dequeue, struct wq_dequeue_item**job)
{
    struct wq_dequeue_item *item = dequeue->head;
    if (!job) {
        return -EINVAL;
    }

    if (!item) {
        DEBUG_MSG( "Queue is empty.\n");
        return -EINVAL;
    }
    else {
        item = dequeue->head;
        *job = item;
        dequeue->head = item->next;

        return 0;
    }
}

/* counts the number of items in dequeue */
static int get_dequeue_len(struct double_ended_queue *dequeue)
{
    int cnt = 0;
    struct wq_dequeue_item *item = dequeue->head;

    while (item != NULL) {
        item = item->next;
        cnt++;
    }

    return cnt;
}

/* deletes the queue */
static int destroy_dequeue( struct double_ended_queue *dequeue )
{
    struct wq_dequeue_item *item;
    int cnt = 0;

    if (dequeue->head == NULL) {
        return cnt;
    }

    while (dequeue->head != NULL)
    {
        item = dequeue->head;
        dequeue->head = dequeue->head->next;
        if (item->wq_job) {
            free(item->wq_job);
        }
        free (item);
        cnt++;
    }

    return cnt;
}

// dequeue the next job that needes to run in this thread
static struct wq_dequeue_item*  _workqueue_get_job(struct workqueue_thread *thread)
{
	struct wq_dequeue_item *job_item = NULL;
	struct workqueue_ctx *ctx = thread->ctx;

	if (!thread->keep_running)
		return NULL;

	assert(ctx);
	DEBUG_MSG("thread %d locking ctx\n",thread->thread_num);
	LOCK_MUTEX(&ctx->mutex);
	DEBUG_MSG("thread %d got lock\n",thread->thread_num);
	assert(ctx->queue);

/*
 *    for(i = 0; thread->keep_running && i < ctx->queue_size; i++) {
 *
 *        [> if queue pointer not null there is a job_item in this slot <]
 *        if (ctx->queue[i]) {
 *
 *                job = ctx->queue[i];
 *                ctx->queue[i] = NULL;
 *                DEBUG_MSG("found job %d\n", job->job_id);
 *                ctx->waiting_jobs--;
 *                break; [> found job ready to run <]
 *        }
 *    }
 *
 */

    if (thread->keep_running) {
        if (del_item_at_begin(ctx->queue, &job_item)) {
            DEBUG_MSG("Get job fail.\n");
        }
        DEBUG_MSG("found job %d\n", job_item->wq_job->job_id);
        ctx->waiting_jobs--;
    }
	DEBUG_MSG("thread %d unlocking ctx job_item=%p\n",
			thread->thread_num, job_item)

	UNLOCK_MUTEX(&ctx->mutex);
	return job_item;
}

// return number of jobs waiting in queue
int workqueue_get_queue_len(struct workqueue_ctx* ctx)
{
	int ret;
	LOCK_MUTEX(&ctx->mutex);
	ret = ctx->waiting_jobs;
	UNLOCK_MUTEX(&ctx->mutex);
	return ret;
}

static void * _workqueue_job_scheduler(void *data)
{
  	struct workqueue_thread *thread = (struct workqueue_thread *) data;
	struct workqueue_ctx *ctx;
    struct wq_dequeue_item *item = NULL;
	int ret;

	DEBUG_MSG("starting data=%p\n", data);
	assert(thread);
	ctx = thread->ctx;
	DEBUG_MSG("thread %d starting\n",thread->thread_num);

	if (ctx->ops && ctx->ops->worker_constructor) {
		DEBUG_MSG("thread %d calling constructor\n",thread->thread_num);
		ctx->ops->worker_constructor(ctx->ops->data);
	}
	LOCK_MUTEX(&thread->mutex);

	while (thread->keep_running) {

		DEBUG_MSG("thread %d looking for work \n",thread->thread_num);
		item = _workqueue_get_job(thread);
        if (item) {
            thread->job = item->wq_job;
        }

		/* is there a job that needs to be run now */
		if (thread->job) {
			/* there is work to do */
			DEBUG_MSG("launching job %d\n",thread->job->job_id);

			/* keep job_mutex locked while running to test if running */
			LOCK_MUTEX(&thread->job_mutex);
			/* mantain unlocked while running so we can check state.
			  in workqueue_job_running() */
			UNLOCK_MUTEX(&thread->mutex);

			/* launch worker job */
			thread->job->func(thread->job->data);

			DEBUG_MSG("job %d finished\n", thread->job->job_id);
			/* done with job free it */
			free(thread->job);
			thread->job = NULL;
            if (item) {
                free(item);
                item = NULL;
            }

			UNLOCK_MUTEX(&thread->job_mutex);
			LOCK_MUTEX(&thread->mutex);

		} else {
			/* wait until we are signaled that there is new work*/


			/* we should idle */

			UNLOCK_MUTEX(&thread->mutex);

			DEBUG_MSG("thread %d going idle.\n", thread->thread_num);
			// note this wait may be a long time, if the system time changes
			LOCK_MUTEX(&ctx->cond_mutex);
			//ret = pthread_cond_timedwait(&ctx->work_ready_cond, &ctx->cond_mutex, &wait_time);
            //wait util condition is true
			ret = pthread_cond_wait(&ctx->work_ready_cond, &ctx->cond_mutex);
			UNLOCK_MUTEX(&ctx->cond_mutex);

			LOCK_MUTEX(&thread->mutex);

			if (!thread->keep_running) {
			  	DEBUG_MSG("thread %d stopping\n",thread->thread_num);
				break;
			}

			if (ret == EINTR) {
			  	DEBUG_MSG("thread %d idle and be interruptted \n",thread->thread_num);
			  	continue; /* wait again */
			} else if (ret == EINVAL) {
				ERROR_MSG("thread %d pthread_cond_wait EINVAL\n", thread->thread_num);
				usleep(1000); // wait 1000th of time wait
			} else if (ret) {
				ERROR_MSG("thread %d pthread_cond_wait ret =%d\n", thread->thread_num, ret);
			}

		}
	}

	UNLOCK_MUTEX(&thread->mutex);

	if (ctx->ops && ctx->ops->worker_destructor) {
		DEBUG_MSG("thread %d calling destructor\n",thread->thread_num);
		ctx->ops->worker_destructor(ctx->ops->data);
	}
	return NULL;
}

static int _empty_queue(struct workqueue_ctx *ctx)
{
	int count = 0;
	/* free any remaining jobs left in queue */
	if(ctx->queue) {
        count = destroy_dequeue(ctx->queue);
	}
    
	return count;
}

void workqueue_destroy(struct workqueue_ctx *ctx)
{
	int i;

	DEBUG_MSG("shutting down ctx=%p\n", ctx);
	LOCK_MUTEX(&ctx->mutex);
	if (ctx->thread) {
		DEBUG_MSG("shutting down %d workers\n", ctx->num_worker_threads);
		for (i = 0; i < ctx->num_worker_threads; i++) {
			if (ctx->thread[i]) {
				ctx->thread[i]->keep_running = false;
			}
		}

		/* send signal to unblock threads */
		pthread_cond_broadcast(&ctx->work_ready_cond);

		/* Unlock incase the worker callback is makeing its own calls to workqueue_* to avoid deadlock */
		UNLOCK_MUTEX(&ctx->mutex);
		for (i = 0; i < ctx->num_worker_threads; i++) {
			if (ctx->thread[i]) {

			  	DEBUG_MSG("joining thread %d\n",ctx->thread[i]->thread_num);
				pthread_join(ctx->thread[i]->thread_id, NULL);

			}
		}
		LOCK_MUTEX(&ctx->mutex);

		/* for each worker thread, clean up it's context */
		for (i = 0; i < ctx->num_worker_threads; i++) {
			if (ctx->thread[i]) {
				free(ctx->thread[i]);
				ctx->thread[i] = NULL;
			}
		}
		free(ctx->thread); /* free pointer list */
	}


	_empty_queue(ctx);
	free(ctx->queue);
	ctx->queue = NULL;

	UNLOCK_MUTEX(&ctx->mutex);

	pthread_mutex_destroy(&ctx->mutex);
	pthread_mutex_destroy(&ctx->cond_mutex);

	free(ctx);

}

static struct workqueue_ctx *
__workqueue_init(struct workqueue_ctx *ctx, unsigned int num_worker_threads)
{
	unsigned int i;
	struct workqueue_thread *thread;
	int ret = 0;

	if (!ctx)
		return NULL;

	ret = pthread_mutex_init(&ctx->mutex, NULL);
	if (ret)
		ERROR_MSG("pthread_mutex_init failed ret=%d\n", ret);

	ret = pthread_mutex_init(&ctx->cond_mutex, NULL);
	if (ret)
		ERROR_MSG("pthread_mutex_init failed ret=%d\n", ret);

	/* Allocate pointers for queue */
	ctx->queue = (struct double_ended_queue*) calloc(1, sizeof(struct double_ended_queue));
	if (!ctx->queue) {
		goto free_ctx;
	}
    init_dequeue(ctx->queue);

	/* Allocate pointers for threads */
	ctx->thread = (struct workqueue_thread **) calloc( num_worker_threads + 1, sizeof(struct workqueue_thread *));
	if (!ctx->thread)
		goto free_queue;

	ret = pthread_cond_init(&ctx->work_ready_cond, NULL);
	if (ret)
		ERROR_MSG("pthread_cond_init failed ret=%d\n", ret);
	ctx->num_worker_threads = num_worker_threads;

	for (i = 0; i < num_worker_threads; i++) {
		ctx->thread[i] = thread = (struct workqueue_thread *) calloc ( 1, sizeof(struct workqueue_thread));
		if (!ctx->thread[i]) {
			goto free_threads;
		}
		thread->thread_num = i;
		thread->ctx = ctx;  /* point to parent */
		thread->keep_running = true;
		pthread_mutex_init(&thread->mutex, NULL);
		if (ret)
			ERROR_MSG("pthread_mutex_init failed ret=%d\n", ret);

		ret = pthread_create(&thread->thread_id, NULL, _workqueue_job_scheduler, thread);

		if (ret)
			ERROR_MSG("pthread_create failed ret=%d\n", ret);

	}

	return ctx;

	/* error cases to clean up for*/
	free_threads:

	for (i = 0; i < num_worker_threads; i++) {
		if (ctx->thread[i]) {
			free(ctx->thread[i]);
		}
	}

	free_queue:
	free(ctx->queue);

	free_ctx:
	free(ctx);
	fprintf(stderr, "Error initializing\n");
	return NULL;
}

struct workqueue_ctx * workqueue_init(
        unsigned int num_worker_threads, struct worker_thread_ops *ops)
{
	struct workqueue_ctx *ctx;

	DEBUG_MSG("Starting thread_size=%d\n", num_worker_threads);

	/* check for invalid args */
	if (!num_worker_threads)
	  	return NULL;

	ctx = (struct workqueue_ctx *) calloc(1, sizeof (struct workqueue_ctx));
	ctx->ops = ops;
	return __workqueue_init(ctx, num_worker_threads);
}


int workqueue_add_work(struct workqueue_ctx* ctx, 
		workqueue_func_t callback_fn, void *data)
{
	int ret;
	struct workqueue_job *job = NULL;

	LOCK_MUTEX(&ctx->mutex);

    job = (struct workqueue_job *) calloc(1, sizeof(struct workqueue_job));
    if (!job) {
        UNLOCK_MUTEX(&ctx->mutex);
        return -ENOMEM;
    }

    job->data = data;
    job->func = callback_fn;
    ret = job->job_id = ctx->job_count++;
    if (ctx->job_count < 0) /* overflow case */
        ctx->job_count = 0;
    //ctx->queue[i] = job;
    if (add_item_at_end(ctx->queue, job) < 0) {
        DEBUG_MSG("Adding job fail\n");
    }
    ctx->waiting_jobs++;

    DEBUG_MSG("Adding job %p id=%d waiting=%d cb=%p data=%p\n",
            job, job->job_id, 
            ctx->waiting_jobs, job->func, job->data );

    //qsort(ctx->queue, ctx->queue_size,
     //       sizeof(struct workqueue_job *), /* size of pointer to sort */
      //      job_compare);
    if (pthread_cond_signal(&ctx->work_ready_cond)) {
        ERROR_MSG("invalid condition\n");
    }

    DEBUG_MSG("unlock mutex\n", NULL);
    UNLOCK_MUTEX(&ctx->mutex);
    return ret;

    /* queues are full */
    //DEBUG_MSG("Queues are full\n", NULL);

    UNLOCK_MUTEX(&ctx->mutex);
    /* no room in queue */
    return -EBUSY;
}

int workqueue_show_status(struct workqueue_ctx* ctx, FILE *fp)
{
	int i;

	LOCK_MUTEX(&ctx->mutex);
	fprintf(fp, "Number of worker threads=%d \n", ctx->num_worker_threads);
	fprintf(fp, "Total jobs added=%d waiting_jobs=%d \n", ctx->job_count, ctx->waiting_jobs);
	fprintf(fp, "\n");
	fprintf(fp, "%3s | %8s \n", "Qi", "JobID");
	fprintf(fp, "---------------------------------\n");
/*
 *    for (i = 0; i < ctx->queue_size; i++) {
 *        if (!ctx->queue[i])
 *            continue; [> unused location <]
 *
 *
 *        fprintf(fp,"%3d | %8d \n", i, ctx->queue[i]->job_id);
 *    }
 */

	UNLOCK_MUTEX(&ctx->mutex);

	fflush(fp);
	return 0;
}

static int _is_job_queued(struct workqueue_ctx* ctx, int job_id)
{
/*
 *    int i;
 *
 *    if(ctx->queue) {
 *        for (i = 0; i < ctx->queue_size; i++) {
 *            if (ctx->queue[i] && ctx->queue[i]->job_id == job_id)
 *                return 1;
 *        }
 *    }
 */

    struct wq_dequeue_item *item = NULL;
    if (ctx->queue) {
    
        item = ctx->queue->head;
        while (item) {
            if (item->wq_job->job_id == job_id) {
                return 1;
            }
            item = item->next;
        }
    }
	return 0;
}

static int _is_job_running(struct workqueue_ctx* ctx, int job_id)
{
	int i;
	int ret = 0;
	int rc;
	for (i = 0; i < ctx->num_worker_threads && !ret; i++) {
		if (ctx->thread[i]) {
			TRY_LOCK_MUTEX(&ctx->thread[i]->mutex, rc);
			if (rc == EBUSY)
				return -EBUSY;
			if (ctx->thread[i]->job && ctx->thread[i]->job->job_id == job_id) {
				ret = 1;
			}
			UNLOCK_MUTEX(&ctx->thread[i]->mutex);
		}
	}

	return ret;
}

int workqueue_job_queued(struct workqueue_ctx* ctx, int job_id)
{
	int ret;

	if (!ctx)
		return -1;

	LOCK_MUTEX(&ctx->mutex);
	ret = _is_job_queued(ctx, job_id);
	UNLOCK_MUTEX(&ctx->mutex);
	return ret;
}

int workqueue_job_running(struct workqueue_ctx* ctx, int job_id)
{
	int ret = 0;

	if (!ctx)
		return -1;


	do {
		LOCK_MUTEX(&ctx->mutex);
		ret = _is_job_running(ctx, job_id);

		UNLOCK_MUTEX(&ctx->mutex);
	} while (ret == -EBUSY);


	return ret;
}

int workqueue_job_queued_or_running(struct workqueue_ctx* ctx, int job_id)
{
	int ret;

	if (!ctx)
		return -1;

	LOCK_MUTEX(&ctx->mutex);
	ret = _is_job_queued(ctx, job_id);
	UNLOCK_MUTEX(&ctx->mutex);

	if (!ret) {
		do {
			LOCK_MUTEX(&ctx->mutex);
			ret = _is_job_running(ctx, job_id);

			UNLOCK_MUTEX(&ctx->mutex);
		} while (ret == -EBUSY);
	}



	return ret;
}

/* private function. NOTE ctx must be locked by caller to avoid race with other dequeue*/
static int _dequeue(struct workqueue_ctx* ctx, int job_id)
{
/*
 *    int i;
 *
 *    if(ctx->queue) {
 *        for (i = 0; i < ctx->queue_size; i++) {
 *            if (ctx->queue[i] && ctx->queue[i]->job_id == job_id) {
 *                free(ctx->queue[i]);
 *                ctx->queue[i] = NULL;
 *                ctx->waiting_jobs--;
 *                return 0;
 *            }
 *        }
 *    }
 */
    struct wq_dequeue_item *item = NULL;
    if (ctx->queue) {
        item = ctx->queue->head;
        while (item) {
            if (item->wq_job->job_id == job_id) {
            
            }
        }
    
    }

	return -ENOENT;
}


int workqueue_dequeue(struct workqueue_ctx* ctx, int job_id)
{
	int ret;
	LOCK_MUTEX(&ctx->mutex);
	ret = _dequeue(ctx, job_id);

	UNLOCK_MUTEX(&ctx->mutex);
	return ret;
}


int workqueue_empty(struct workqueue_ctx *ctx)
{
	int count;
	LOCK_MUTEX(&ctx->mutex);

	count = _empty_queue(ctx);

	UNLOCK_MUTEX(&ctx->mutex);
	return count;
}

int workqueue_empty_wait(struct workqueue_ctx *ctx)
{
	int count;
	int i;
	int num_workers = ctx->num_worker_threads;

	LOCK_MUTEX(&ctx->mutex);
	count = _empty_queue(ctx);
	num_workers = ctx->num_worker_threads;
	UNLOCK_MUTEX(&ctx->mutex);



	for (i = 0; i < num_workers; i++) {
		if (ctx->thread[i]) {

			LOCK_MUTEX(&ctx->thread[i]->mutex);
			LOCK_MUTEX(&ctx->thread[i]->job_mutex);
			if (ctx->thread[i]->job) {
				ERROR_MSG("no job should be running\n");
			}
			UNLOCK_MUTEX(&ctx->thread[i]->job_mutex);
			UNLOCK_MUTEX(&ctx->thread[i]->mutex);

		}
	}

	return count;
}
