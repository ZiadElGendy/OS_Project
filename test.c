#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

void *func(void *arg)
{
    int thread_id = *((int *)arg);

    printf("Thread %d started\n", thread_id);
    for (int i = 0; i < 5; i++)
    {
        printf("Thread %d is running %d\n", thread_id, i + 1);
        for (int j = 0; j < 99999999; j++)
        {
            int i = j + 1;
            i = i - 1;
            i = i * 2;
            i = i / 2;
            i = i + 1;
            i = i - 1;
            i = i * 2;
            i = i % 2;
            int result = j * j;
        }
    }
    printf("Thread %d finished\n", thread_id);

    pthread_exit(NULL);
}

int main()
{
    pthread_t ptid1;
    pthread_t ptid2;
    pthread_t ptid3;
    pthread_t ptid4;
    
    int thread_id1 = 1;
    int thread_id2 = 2;
    int thread_id3 = 3;
    int thread_id4 = 4;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // Set scheduling policy to SCHED_FIFO
    struct sched_param param;
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    param.sched_priority = sched_get_priority_max(SCHED_RR);
    pthread_attr_setschedpolicy(&attr, SCHED_RR);
    pthread_attr_setschedparam(&attr, &param);

    // Set CPU affinity to ensure all threads run on the same CPU
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset); // Set to CPU 0, you can change it according to your system
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);

    // Check if the effective user id is root
    if (geteuid() != 0) {
        printf("Need root privileges to set real-time scheduling policy.\n");
        exit(EXIT_FAILURE);
    }

    pthread_create(&ptid1, &attr, &func, (void *)&thread_id1);
    pthread_create(&ptid2, &attr, &func, (void *)&thread_id2);
    pthread_create(&ptid3, &attr, &func, (void *)&thread_id3);
    pthread_create(&ptid4, &attr, &func, (void *)&thread_id4); 

    pthread_attr_destroy(&attr);

    pthread_exit(NULL);
}
