#include <uv.h>
#include <stdlib.h>
#include <unistd.h>

#define dprintf(fmt, args...) do{fprintf(stderr, "[%s:%d] "fmt, __FUNCTION__, __LINE__, ##args);}while(0)
#define HC_MAX_THREADPOOL_SIZE 128

typedef struct
{
    void *data;
    uv_work_t work_req; /*Don't touch this field*/
} hc_work_queue_struct;

int hc_work_queue(hc_work_queue_struct *hc_work_req,
                  uv_work_cb tsk, 
                  uv_after_work_cb tsk_cb);
int hc_set_threadpool_size(int size);
int hc_get_threadpool_size(void);
int hc_task_cancel(hc_work_queue_struct *hc_work_req);
int hc_task_run();

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

int hc_set_threadpool_size(int size)
{
    char value[16] = {0};
    snprintf(value, sizeof(value), "%u", size);
    if (size > HC_MAX_THREADPOOL_SIZE && size <= 0)
        return -1;

    setenv("UV_THREADPOOL_SIZE", value, 1);
    return 0;
}

int hc_get_threadpool_size(void)
{
    return atoi(getenv("UV_THREADPOOL_SIZE"));
}

int hc_work_queue(hc_work_queue_struct *hc_work_req,
                  uv_work_cb tsk, 
                  uv_after_work_cb tsk_cb)
{
    if (!hc_work_req)
    {
        dprintf("hc_work_req paramter is null\n");
        return -1;
    }

    if (!tsk)
    {
        dprintf("Must has a task to enqueue in work queue\n");
        return -1;
    }

    hc_work_req->work_req.data = hc_work_req->data;
    if (uv_queue_work(uv_default_loop(), &hc_work_req->work_req, tsk, tsk_cb))
        return -UV_EINVAL;

    return 0;
}

int hc_task_cancel(hc_work_queue_struct *hc_work_req)
{
    if (!hc_work_req)
        return -UV_EINVAL;

    return uv_cancel((uv_req_t *)&hc_work_req->work_req);
}

int hc_task_run()
{
    return uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

int main(int argc, char *argv[])
{
    int i;
    int data[20] = {0, 1, 2, 3, 4, 5,
                    6, 7, 8, 9, 10, 11,
                    12, 13, 14, 15, 16,
                    17, 18, 19};
    hc_work_queue_struct hc_work_task[20];

    hc_set_threadpool_size(10);
    for (i = 0; i < 20; i++)
    {
        hc_work_task[i].data = &data[i];
        hc_work_queue(&hc_work_task[i], task, task_callback);
    }

    hc_task_run();

    return 0;
}
