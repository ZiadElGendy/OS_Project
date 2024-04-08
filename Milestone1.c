#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Function prototypes for thread functions
void *thread_function(void *arg);

int main() {
    pthread_t threads[4]; // Array to hold thread IDs
    int i;

    // Create threads
    for (i = 0; i < 4; i++) {
        if (pthread_create(&threads[i], NULL, thread_function, (void *)(intptr_t)i) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Join threads
    for (i = 0; i < 4; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

// Thread function
void *thread_function(void *arg) {
    int thread_id = (int)(intptr_t)arg;

    // Print statements
    printf("Thread %d started\n", thread_id);
    // Additional print statements...
    printf("Thread %d executing statement 1\n", thread_id);
    // Additional print statements...
    printf("Thread %d executing statement 2\n", thread_id);
    // Additional print statements...
    printf("Thread %d finished\n", thread_id);

    pthread_exit(NULL);
}