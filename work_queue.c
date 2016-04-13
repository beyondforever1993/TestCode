#include <uv.h>
#include <stdlib.h>
#include <unistd.h>

#define dprintf(fmt, args...) do{fprintf(stderr, "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##args);}while(0)
/*
 *typedef void (*task)(void *data);
 *typedef void (*task_callback)(void *data, int status);
 */

typedef struct
{
    uv_work_t work_req;
    void *data;
} hc_work_queue_struct;

void task(uv_work_t *work_request)
{
    if (!work_request)
    {
        dprintf("Wrong function's parameter: work_request\n");
        return ;
    }
    sleep(1);

    dprintf("This is task. data: %d\n", *(int *)work_request->data);
    (*(int *)work_request->data)++;

    return;
}

void task_callback(uv_work_t *work_request, int status)
{
    dprintf("This is callback. data: %d, status:%d\n", *(int *)work_request->data, status);
}

int hc_work_queue(void *data, uv_work_cb tsk, uv_after_work_cb tsk_cb);

int hc_work_queue(void *data, uv_work_cb tsk, uv_after_work_cb tsk_cb)
{
    int ret = 0, i;
    uv_work_t work_request[20];
    uv_work_t work_req;

    if (!tsk)
    {
        dprintf("Must has a task to enqueue in work queue\n");
        return -1;
    }

#if 1
    work_req.data = data;
    if ((ret = uv_queue_work(uv_default_loop(), &work_req, tsk, tsk_cb)))
    {
        dprintf("ret: %d\n", ret);
        return ret;
    }
#else
    for (i = 0; i < sizeof(work_request)/sizeof(work_request[0]); i++)
    {
        work_request[i].data = data;
        if ((ret = uv_queue_work(uv_default_loop(), &work_request[i], tsk, tsk_cb)))
            return ret;
        /*
         *ret = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
         */
    }
#endif
    /*
     *ret = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
     */

    return ret;
}

int main(int argc, char *argv[])
{
    int data0  = 0;
    int data1  = 1;
    int data2  = 2;
    int data3  = 3;
    int data4  = 4;
    int data5  = 5;
    int data6  = 6;
    int data7  = 7;
    int data8  = 8;
    int data9  = 9;
    int data10 = 10;
    int data11 = 11;
    int data12 = 12;
    int data13 = 13;
    int data14 = 14;
    int data15 = 15;
    int data16 = 16;
    int data17 = 17;
    int data18 = 18;
    int data19 = 19;

    /*
     *dprintf("threadpool size: %s\n", getenv("UV_THREADPOOL_SIZE"));
     *setenv("UV_THREADPOOL_SIZE", "4", 1);
     *dprintf("threadpool size: %s\n", getenv("UV_THREADPOOL_SIZE"));
     */
    hc_work_queue(&data0, task, task_callback);
    hc_work_queue(&data1, task, task_callback);
    hc_work_queue(&data2, task, task_callback);
    hc_work_queue(&data3, task, task_callback);
    hc_work_queue(&data4, task, task_callback);
    hc_work_queue(&data5, task, task_callback);
    hc_work_queue(&data6, task, task_callback);
    hc_work_queue(&data7, task, task_callback);
    hc_work_queue(&data8, task, task_callback);
    hc_work_queue(&data9, task, task_callback);
    hc_work_queue(&data10, task, task_callback);
    hc_work_queue(&data11, task, task_callback);
    hc_work_queue(&data12, task, task_callback);
    hc_work_queue(&data13, task, task_callback);
    hc_work_queue(&data14, task, task_callback);
    hc_work_queue(&data15, task, task_callback);
    hc_work_queue(&data16, task, task_callback);
    hc_work_queue(&data17, task, task_callback);
    hc_work_queue(&data18, task, task_callback);
    hc_work_queue(&data19, task, task_callback);

    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    sleep(30);
    return 0;
}
