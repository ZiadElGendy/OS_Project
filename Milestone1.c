#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void thread_short_function(void *arg)
{
    int thread_id = *((int *)arg);

    printf("Thread %d started\n", thread_id);
    for (int i = 0; i < 10; i++)
    {
        if (i % 2 == 0)
            printf("Thread %d is running\n", thread_id);
    }
    printf("Thread %d finished\n", thread_id);

    pthread_exit(NULL);
}

void thread_medium_function(void *arg)
{
    int thread_id = *((int *)arg);

    printf("Thread %d started\n", thread_id);
    for (int i = 0; i < 100; i++)
    {
        if (i % 25 == 0)
            printf("Thread %d is running\n", thread_id);
    }
    printf("Thread %d finished\n", thread_id);

    pthread_exit(NULL);
}

void thread_long_function(void *arg)
{
    int thread_id = *((int *)arg);

    printf("Thread %d started\n", thread_id);
    for (int i = 0; i < 1000; i++)
    {
        if (i % 250 == 0)
            printf("Thread %d is running\n", thread_id);
    }
    printf("Thread %d finished\n", thread_id);

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[4];
    int i, j;
    pthread_attr_t attr;
    struct sched_param param;
    int policies[3] = {SCHED_FIFO, SCHED_RR, SCHED_OTHER};

    for (j = 0; j < 3; j++)
    {
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, policies[j]);
        param.sched_priority = sched_get_priority_max(policies[j]);
        pthread_attr_setschedparam(&attr, &param);

        for (i = 0; i < 4; i++)
        {
            int *thread_id = malloc(sizeof(int));
            *thread_id = i;
            switch (i)
            {
            case 0:
                pthread_create(&threads[i], &attr, thread_short_function, (void *)thread_id);
                break;
            case 1:
                pthread_create(&threads[i], &attr, thread_medium_function, (void *)thread_id);
                break;
            case 2:
                pthread_create(&threads[i], &attr, thread_short_function, (void *)thread_id);
                break;
            case 3:
                pthread_create(&threads[i], &attr, thread_long_function, (void *)thread_id);
                break;
            }
        }

        for (i = 0; i < 4; i++)
        {
            if (pthread_join(threads[i], NULL) != 0)
            {
                perror("pthread_join");
                exit(EXIT_FAILURE);
            }
        }

        pthread_attr_destroy(&attr);
    }

    return 0;
}
