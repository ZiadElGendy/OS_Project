#include <stdio.h>
#include <stdlib.h>

#define MAX_LINES 1024
#define MAX_LINE_LENGTH 128
#define MAX_FILE_NAME 64

#pragma region int queue implementation
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int *array;
};

// function to create a queue of given capacity. It initializes size of queue as 0
struct Queue *createQueue(unsigned capacity)
{
    struct Queue *queue = (struct Queue *)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int *)malloc(
        queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue *queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue *queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue. It changes rear and size
void enqueue(struct Queue *queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue. It changes front and size
int dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
int front(struct Queue *queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
#pragma endregion

struct Queue priority1Queue;
struct Queue priority2Queue;
struct Queue priority3Queue;
struct Queue priority4Queue;
struct Queue blockedQueue;

struct PCB {
    int pid;
    char processState;
    int currentPriority;
    int programCounter;
    int lowerMemoryBound;
    int higherMemoryBound;
};

struct Process {
    struct PCB pcb;
    int quantum;
};

struct word {
    char name[16];
    char data[16];
};

struct memory {
    struct word words[60];
};

char** readFile(int programNum)
{
    FILE *file;
    char line[MAX_LINE_LENGTH];

    // Open the file
    char fileName[MAX_FILE_NAME];
    sprintf(fileName, "Program_%d.txt", programNum); //Concatenate the filename
    file = fopen(fileName, "r");

    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    // Allocate memory for storing lines
    char **lines = (char **)malloc(MAX_LINES * sizeof(char *));
    if (lines == NULL)
    {
        perror("Memory allocation error");
        fclose(file);
        return NULL;
    }

    int i = 0;
    // Read lines until the end of the file
    while (fgets(line, sizeof(line), file) != NULL && i < MAX_LINES)
    {
        lines[i] = strdup(line); // Allocate memory and copy line
        i++;
    }
    // Close the file
    fclose(file);

    return lines;
}

int main(){
    
    priority1Queue = *createQueue(10);
    priority2Queue = *createQueue(10);
    priority3Queue = *createQueue(10);
    priority4Queue = *createQueue(10);
    blockedQueue = *createQueue(10);

    for (int i = 1; i <= 3; i++)
    {
        char **lines = readFile(i);
        for (int j = 0; j < MAX_LINES; j++)
        {
            if (lines[j] == NULL)
            {
                break;
            }
            printf("%s", lines[j]);
        }
    }
}