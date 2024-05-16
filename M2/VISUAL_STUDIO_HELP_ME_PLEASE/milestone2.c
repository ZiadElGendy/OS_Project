#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINES 256
#define MAX_LINE_LENGTH 64
#define MAX_FILE_NAME 64
#define QUEUE_CAPACITY 32

#pragma region int queue implementation
struct Queue
{
    int front, rear, size;
    unsigned capacity;
    int* array;
};

// function to create a queue of given capacity. It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue. It changes rear and size
void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue. It changes front and size
int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
int front(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
#pragma endregion


bool inputBSemaphore = true;
bool outputBSemaphore = true;
bool fileBSemaphore = true;

struct Queue priority1Queue;
struct Queue priority2Queue;
struct Queue priority3Queue;
struct Queue priority4Queue;
struct Queue blockedQueue;
struct Queue inputQueue;
struct Queue outputQueue;
struct Queue fileQueue;

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
    char data[64];
};

struct memory {
    struct word words[60];
} memory; //global variable

#pragma region program loading
char** readFile(int programNum)
{
    FILE* file;
    char line[MAX_LINE_LENGTH];

    // Open the file
    char fileName[MAX_FILE_NAME];
    sprintf(fileName, "Program_%d.txt", programNum); //Concatenate the filename
    printf("Opening file:%s\n", fileName);
    file = fopen(fileName, "r");

    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    char** lines = NULL;
    int i = 0;

    // Read lines until the end of the file
    while (fgets(line, sizeof(line), file) != NULL)
    {
		line[strcspn(line, "\n")] = 0; //Remove the newline character from the line

		char** tmp = realloc(lines, (i + 1) * sizeof(*lines));
		if (tmp == NULL)
		{
            perror("Memory reallocation error");
            fclose(file);
			free(lines);
			return NULL;
		}
		lines = tmp;

        lines[i] = _strdup(line);
		if (lines[i] == NULL)
		{
			perror("Memory allocation error");
			fclose(file);
			free(lines);
			return NULL;
		}


        i++;
    }
	lines[i] = NULL;

    // Close the file
    fclose(file);

    return lines;
}

void loadProgramIntoMemory(int processId, char** lines)
{
    //Find the first available memory block
    int lowerMemoryBound = 0;

    for (int i = 0; i < 60; i++)
    {
        if (memory.words[i].name[0] == '\0') //If the first character is the null character, then the memory block is available
        {
            lowerMemoryBound = i;
            break;
        }
    }

    int numLines = 0;
    while (lines[numLines] != NULL) {
        numLines++;
    }

    int upperMemoryBound = lowerMemoryBound + 8 + numLines; //5 for PCB, 3 for process variables,and the number of lines in the program
	if (upperMemoryBound > 59)
    {
		perror("Memory overflow");
		//TODO: Compact memory in this case
		return;
	}

    //Load PCB into memory
    strcpy(memory.words[lowerMemoryBound].name, "pid");
    sprintf(memory.words[lowerMemoryBound].data, "%d", processId);

    strcpy(memory.words[lowerMemoryBound + 1].name, "processState");
    strcpy(memory.words[lowerMemoryBound + 1].data, "ready");
                                                       
    strcpy(memory.words[lowerMemoryBound + 2].name, "currentPriority");
    strcpy(memory.words[lowerMemoryBound + 2].data, "1");

    strcpy(memory.words[lowerMemoryBound + 3].name, "programCounter");
    strcpy(memory.words[lowerMemoryBound + 3].data, "0");

    strcpy(memory.words[lowerMemoryBound + 4].name, "lowerMemoryBound");
    sprintf(memory.words[lowerMemoryBound + 4].data,  "%d", lowerMemoryBound);

    strcpy(memory.words[lowerMemoryBound + 5].name, "higherMemoryBound");
    sprintf(memory.words[lowerMemoryBound + 5].data,  "%d", upperMemoryBound);


    //Load process variables into memory
    strcpy(memory.words[lowerMemoryBound + 6].name,  "a");
    strcpy(memory.words[lowerMemoryBound + 6].data, "0");
                                                                                                
    strcpy(memory.words[lowerMemoryBound + 7].name, "b");
    strcpy(memory.words[lowerMemoryBound + 7].data, "0");
                                                                                                
    strcpy(memory.words[lowerMemoryBound + 8].name, "c");
    strcpy(memory.words[lowerMemoryBound + 8].data, "0");

    //Load program into memory
    for (int i = 0; i < numLines; i++)
    {
        sprintf(memory.words[lowerMemoryBound + 9 + i].name, "line %d", i);
        strcpy(memory.words[lowerMemoryBound + 9 + i].data, lines[i]);
    }
}

void printMemoryContents()
{
    for (int i = 0; i < 60; i++)
    {
        printf("Word %d > %s: %s\n", i, memory.words[i].name, memory.words[i].data);
    }
}
#pragma endregion

#pragma region program getters and setters
char* getProgramState(int processId)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			return memory.words[i + 1].data;
		}
	}
	return NULL;
}

void setProgramState(int processId, char* state)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			strcpy(memory.words[i + 1].data, state);
			return;
		}
	}
}

int getProgramPriority(int processId)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			return atoi(memory.words[i + 2].data);
		}
	}
	return -1;
}

void setProgramPriority(int processId, int priority)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			sprintf(memory.words[i + 2].data, "%d", priority);
			return;
		}
	}
}

int getProgramCounter(int processId)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			return atoi(memory.words[i + 3].data);
		}
	}
	return -1;
}

void incrementProgramCounter(int processId)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			int programCounter = atoi(memory.words[i + 3].data);
			sprintf(memory.words[i + 3].data, "%d", programCounter + 1);
			return;
		}
	}
}

char* getCurrentInstruction(int processId)
{
	int programCounter = getProgramCounter(processId);
	if (programCounter == -1)
	{
		return NULL;
	}

	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			return memory.words[i + 9 + programCounter].data;
		}
	}
	return NULL;   
}

char* getVariableValue(int processId, char variableName)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			if (variableName == "a")
            {
				return memory.words[i + 6].data;
			}
            else if (variableName == "b")
            {
				return memory.words[i + 7].data;
			}
            else if (variableName == "c")
            {
				return memory.words[i + 8].data;
			}
		}
	}
	return NULL;
}

char* setVariableValue(int processId, char variableName)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			if (variableName == "a")
			{
				return memory.words[i + 6].data;
			}
			else if (variableName == "b")
			{
				return memory.words[i + 7].data;
			}
			else if (variableName == "c")
			{
				return memory.words[i + 8].data;
			}
		}
	}
	return NULL;
}

#pragma endregion

int main()
{

    priority1Queue = *createQueue(QUEUE_CAPACITY);
    priority2Queue = *createQueue(QUEUE_CAPACITY);
    priority3Queue = *createQueue(QUEUE_CAPACITY);
    priority4Queue = *createQueue(QUEUE_CAPACITY);
    blockedQueue = *createQueue(QUEUE_CAPACITY);

	for (int i = 1; i <= 3; i++)
    {
        char** lines = readFile(i);
        loadProgramIntoMemory(i, lines);
	}
	printMemoryContents();
	printf("Program 2 current instuction:");
	char* pointer = getCurrentInstruction(2);
	puts(pointer);

    Sleep(1000000);
}