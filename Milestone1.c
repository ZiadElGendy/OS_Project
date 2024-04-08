#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void thread_function(void *arg)
{
    int thread_id = (int)arg;

    printf("Thread %d started\n", thread_id);
    printf("Thread %d executing statement 1\n", thread_id);
    printf("Thread %d executing statement 2\n", thread_id);
    printf("Thread %d finished\n", thread_id);

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[4];
    int i;

    for (i = 0; i < 4; i++)
    {
        pthread_create(&threads[i], NULL, thread_function, (void*)(&threads[i]));
    }

    for (i = 0; i < 4; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
