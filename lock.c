#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static const char * const caller[2] = {"main", "signal handler"};
static volatile int signal_handler_exit = 0;

static void hold_mutex(int c)
{
    printf("enter hold_mutex [caller %s]\n", caller[c]);
    pthread_mutex_lock(&mutex);
    printf("11111111111111\n");
    pthread_mutex_lock(&mutex);
    printf("22222222222222\n");
    /* 保证信号函数退出前， main线程始终拥有锁 */
    /*
     *while (!signal_handler_exit && c != 1) {
     *    sleep(5);
     *}
     */
    pthread_mutex_unlock(&mutex);
    printf("leave hold_mutex [caller %s]\n", caller[c]);
}

static void signal_handler(int signum)
{
    hold_mutex(1);
    signal_handler_exit = 1;
}

void test(union sigval arg)
{
    printf("2222222222222222222222\n");
}
int main()
{
    /*
     *signal(SIGALRM, signal_handler);
     */
    /*
     *alarm(3);
     */
    /*
     *hold_mutex(0);
     */

    struct sigevent inotify;
    timer_t time;
    struct itimerspec interval;
    interval.it_value.tv_sec = 3;
    interval.it_value.tv_nsec = 0;
    interval.it_interval.tv_sec = 0;
    interval.it_interval.tv_nsec = 0;
    memset(&inotify, 0, sizeof(struct sigevent));
    inotify.sigev_notify = SIGEV_THREAD;
    inotify.sigev_notify_function = test;

    if (timer_create(CLOCK_REALTIME, &inotify, &time) == 0)
    {
        if (timer_settime(time, 0, &interval, NULL) == 0)
        {
            printf("start time ok\n");
        }
    }
    sleep(10);
    if (timer_delete(time) == 0)
    {
        printf("delete timer \n");
    }
    return 0;
}
