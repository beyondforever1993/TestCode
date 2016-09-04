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
static char * caller_function[2] = {"main function", "signal handler"};
static int sig_handler_exit = 0;

static void hold_mutex(int index)
{
    printf("enter hold_mutex [caller_function is %s]\n", caller_function[index]);
    pthread_mutex_lock(&mutex);

    while (!sig_handler_exit && index != 1) {
        sleep(5);
    }
    pthread_mutex_unlock(&mutex);
    printf("leave hold_mutex [caller_function is %s]\n", caller_function[index]);
}

static void signal_handler(int signum)
{
    hold_mutex(1);
    sig_handler_exit = 1;
}

int main()
{

    signal(SIGALRM, signal_handler);
    alarm(3);
    hold_mutex(0);

    return 0;
}
