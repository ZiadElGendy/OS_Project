#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#pragma region definitions
#define ANSI_COLOR_RED			"\x1b[31m"
#define ANSI_COLOR_GREEN		"\x1b[32m"
#define ANSI_COLOR_YELLOW		"\x1b[33m"
#define ANSI_COLOR_BLUE			"\x1b[34m"
#define ANSI_COLOR_MAGENTA		"\x1b[35m"
#define ANSI_COLOR_CYAN			"\x1b[36m"
#define ANSI_COLOR_BLACK		"\x1b[30m"
#define ANSI_BACKGROUND_RED		"\x1b[41m"
#define ANSI_BACKGROUND_GREEN	"\x1b[42m"
#define ANSI_BACKGROUND_BLUE	"\x1b[44m"
#define ANSI_BACKGROUND_WHITE	"\x1b[47m"
#define ANSI_COLOR_RESET		"\x1b[0m"

#define MAX_LINES 256
#define MAX_LINE_LENGTH 64
#define MAX_FILE_NAME 64
#define QUEUE_CAPACITY 32
#pragma endregion

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

void clearQueue(struct Queue* queue)
{
	// Reset front, rear, and size to make the queue empty
	queue->front = 0;
	queue->rear = queue->capacity - 1;
	queue->size = 0;
}
#pragma endregion

#pragma region priority queue implementation
struct PriorityQueue
{
	int idx;
	int* pqVal;
	int* pqPriority;
};

void initPriorityQueue(struct PriorityQueue* pQueue)
{
	pQueue->idx = -1;
	pQueue->pqVal = (int*)malloc(QUEUE_CAPACITY * sizeof(int));
	pQueue->pqPriority = (int*)malloc(QUEUE_CAPACITY * sizeof(int));
}

int isPriorityQueueEmpty(struct PriorityQueue* pQueue)
{
	return pQueue->idx == -1;
}

int isPriorityQueueFull(struct PriorityQueue* pQueue)
{
	return pQueue->idx == QUEUE_CAPACITY - 1;
}

void enqueuePriorityQueue(struct PriorityQueue* pQueue, int val, int priority)
{
	if (isPriorityQueueFull(pQueue))
	{
		perror("Priority queue is full");
		return;
	}

	pQueue->idx++;
	pQueue->pqVal[pQueue->idx] = val;
	pQueue->pqPriority[pQueue->idx] = priority;
}

int dequeuePriorityQueue(struct PriorityQueue* pQueue)
{
	if (isPriorityQueueEmpty(pQueue))
	{
		perror("Priority queue is empty");
		return -1;
	}

	int minPriority = pQueue->pqPriority[0];
	int minIdx = 0;
	for (int i = 1; i <= pQueue->idx; i++)
	{
		if (pQueue->pqPriority[i] < minPriority)
		{
			minPriority = pQueue->pqPriority[i];
			minIdx = i;
		}
	}

	int minVal = pQueue->pqVal[minIdx];
	for (int i = minIdx; i < pQueue->idx; i++)
	{
		pQueue->pqVal[i] = pQueue->pqVal[i + 1];
		pQueue->pqPriority[i] = pQueue->pqPriority[i + 1];
	}
	pQueue->idx--;

	return minVal;
}
#pragma endregion

#pragma region structs and globals
struct BinarySemaphore {
	bool value;
	int owner;
};

void initBinarySemaphore(struct BinarySemaphore* semaphore)
{
	semaphore->value = true;
	semaphore->owner = -1;
}

struct BinarySemaphore inputBSemaphore;
struct BinarySemaphore outputBSemaphore;
struct BinarySemaphore fileBSemaphore;

struct Queue priority1Queue;
struct Queue priority2Queue;
struct Queue priority3Queue;
struct Queue priority4Queue;
struct Queue blockedQueue;
struct PriorityQueue inputQueue;
struct PriorityQueue outputQueue;
struct PriorityQueue fileQueue;
struct Queue arrivalQueue;

int arrivalClockCycle = 0;
int arrivalProgram = 1;

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

#pragma endregion

#pragma region program loading
int setArrivalTimes()
{
	printf("Enter the number of programs to load: \n");
	int numPrograms;
	int ret = scanf("%d", &numPrograms);
	if (ret != 1)
	{
		printf(ANSI_BACKGROUND_RED"Error: Invalid input\n"ANSI_COLOR_RESET);
		return -1;
	}
	arrivalQueue = *createQueue(numPrograms);

	printf("\nNote: The first clock cycle is 0\n");
	for (int i = 1; i <= numPrograms; i++)
	{
		printf("Enter the arrival clock cycle of program %d: \n", i);
		int prevArrivalClockCycle = arrivalClockCycle;
		int ret = scanf("%d", &arrivalClockCycle);
		if (ret != 1)
		{
			printf(ANSI_BACKGROUND_RED"Error: Invalid input\n"ANSI_COLOR_RESET);
			arrivalQueue = *createQueue(0);
			return -1;
		}
		if (arrivalClockCycle < prevArrivalClockCycle)
		{
			printf(ANSI_BACKGROUND_RED"Error: Arrival clock cycle must be greater than the previous arrival clock cycle.\nConsider renaming files to match the desired order.\n"ANSI_COLOR_RESET);
			arrivalQueue = *createQueue(0);
			return -1;
		}
		enqueue(&arrivalQueue, arrivalClockCycle);
	}
	arrivalClockCycle = dequeue(&arrivalQueue);
	return numPrograms;
}

char** readInstructionFile(int programNum)
{
    FILE* file;
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
    sprintf(memory.words[lowerMemoryBound + 3].data, "%d", lowerMemoryBound + 9);

    strcpy(memory.words[lowerMemoryBound + 4].name, "lowerMemoryBound");
    sprintf(memory.words[lowerMemoryBound + 4].data,  "%d", lowerMemoryBound);

    strcpy(memory.words[lowerMemoryBound + 5].name, "upperMemoryBound");
    sprintf(memory.words[lowerMemoryBound + 5].data,  "%d", upperMemoryBound);


    //Load process variables into memory
    strcpy(memory.words[lowerMemoryBound + 6].name,  "a");
    strcpy(memory.words[lowerMemoryBound + 6].data, "0");
                                                                                                
    strcpy(memory.words[lowerMemoryBound + 7].name, "b");
    strcpy(memory.words[lowerMemoryBound + 7].data, "0");
                                                                                                
    strcpy(memory.words[lowerMemoryBound + 8].name, "c");
    strcpy(memory.words[lowerMemoryBound + 8].data, "0");

    //Load instructions into memory
    for (int i = 0; i < numLines; i++)
    {
        sprintf(memory.words[lowerMemoryBound + 9 + i].name, "line %d", i);
        strcpy(memory.words[lowerMemoryBound + 9 + i].data, lines[i]);
    }
}

#pragma endregion

#pragma region program getters and setters
int getLowerMemoryBound(int processId)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			return i;
		}
	}
	return -1;
}

int getUpperMemoryBound(int processId)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			return atoi(memory.words[i + 5].data);
		}
	}
	return -1;
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

int incrementProgramCounter(int processId)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			int programCounter = atoi(memory.words[i + 3].data);
			if (programCounter >= getUpperMemoryBound(processId))
			{
				return -1;
			}	
			sprintf(memory.words[i + 3].data, "%d", programCounter + 1);
			return programCounter + 1;
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

	return memory.words[programCounter].data;
}

int getQuantum(int processId)
{
	int lowerMemoryBound = getLowerMemoryBound(processId);
	if (lowerMemoryBound == -1)
	{
		return -1;
	}

	int programCounter = getProgramCounter(processId);
	if (programCounter == -1)
	{
		return -1;
	}

	return programCounter - lowerMemoryBound - 9;

}

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

void updateProgramPriority(int processId)
{
	int quantum = getQuantum(processId);
	if (quantum == -1)
	{
		perror("Invalid quantum");
		return;
	}

	char newPriority; // Changed to char from char*
	if (quantum < 1)
	{
		newPriority = '1'; // Removed the dereference operator *
	}
	else if (quantum < 3)
	{
		newPriority = '2';
	}
	else if (quantum < 7)
	{
		newPriority = '3';
	}
	else
	{
		newPriority = '4';
	}

	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			strncpy(memory.words[i + 2].data, &newPriority, 1); // Used strncpy to copy a single character
		}
	}
}

char* getVariableValue(int processId, char variableName)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			if (variableName == 'a')
            {
				return memory.words[i + 6].data;
			}
            else if (variableName == 'b')
            {
				return memory.words[i + 7].data;
			}
            else if (variableName == 'c')
            {
				return memory.words[i + 8].data;
			}
		}
	}
	return NULL;
}

void setVariableValue(int processId, char variableName, char* newData)
{
	for (int i = 0; i < 60; i++)
	{
		if (strcmp(memory.words[i].name, "pid") == 0 && atoi(memory.words[i].data) == processId)
		{
			if (variableName == 'a')
			{
				strcpy(memory.words[i + 6].data, newData);
			}
			else if (variableName == 'b')
			{
				strcpy(memory.words[i + 7].data, newData);
			}
			else if (variableName == 'c')
			{
				strcpy(memory.words[i + 8].data, newData);
			}
		}
	}
}

#pragma endregion

#pragma region program parsing
char** splitString(const char* str, int* numTokens) {
	// Copy the input string to avoid modifying the original
	char* strCopy = strdup(str);
	if (!strCopy) {
		perror("strdup");
		return NULL;
	}

	// Initial allocation for tokens
	int tokensAllocated = 10;
	char** tokens = malloc(tokensAllocated * sizeof(char*));
	if (!tokens) {
		perror("malloc");
		free(strCopy);
		return NULL;
	}

	int tokenCount = 0;
	char* token = strtok(strCopy, " ");
	while (token) {
		if (tokenCount >= tokensAllocated) {
			tokensAllocated *= 2;
			char** temp = realloc(tokens, tokensAllocated * sizeof(char*));
			if (!temp) {
				perror("realloc");
				for (int i = 0; i < tokenCount; i++) {
					free(tokens[i]);
				}
				free(tokens);
				free(strCopy);
				return NULL;
			}
			tokens = temp;
		}
		tokens[tokenCount++] = strdup(token);
		token = strtok(NULL, " ");
	}

	// Shrink the array to the actual number of tokens
	char** result = realloc(tokens, tokenCount * sizeof(char*));
	if (!result && tokenCount > 0) {
		perror("realloc");
		for (int i = 0; i < tokenCount; i++) {
			free(tokens[i]);
		}
		free(tokens);
		free(strCopy);
		return NULL;
	}

	tokens = result;
	*numTokens = tokenCount;

	free(strCopy);
	return tokens;
}

// Function to free the allocated memory for tokens
void freeTokens(char** tokens, int numTokens) {
	for (int i = 0; i < numTokens; i++) {
		free(tokens[i]);
	}
	free(tokens);
}
#pragma endregion

#pragma region program execution

void queueProcess(int pid)
{
	int priority = getProgramPriority(pid);
	switch (priority)
	{
	case 1:
		enqueue(&priority1Queue, pid);
		break;
	case 2:
		enqueue(&priority2Queue, pid);
		break;
	case 3:
		enqueue(&priority3Queue, pid);
		break;
	case 4:
		enqueue(&priority4Queue, pid);
		break;
	default:
		perror("Invalid priority");
		break;
	}
}

int dequeNextProcess()
{
	if (priority1Queue.size > 0)
	{
		return dequeue(&priority1Queue);
	}
	else if (priority2Queue.size > 0)
	{
		return dequeue(&priority2Queue);
	}
	else if (priority3Queue.size > 0)
	{
		return dequeue(&priority3Queue);
	}
	else if (priority4Queue.size > 0)
	{
		return dequeue(&priority4Queue);
	}
	else
	{
		return -1;
	}
}

void updateGeneralBlockedQueue()
{
	clearQueue(&blockedQueue);

	struct PriorityQueue tempQueue;
	initPriorityQueue(&tempQueue);

	for (int i = 0; i <= inputQueue.idx; i++)
	{
		enqueue(&blockedQueue, inputQueue.pqVal[i]);
	}

	for (int i = 0; i <= outputQueue.idx; i++)
	{
		enqueue(&blockedQueue, outputQueue.pqVal[i]);
	}

	for (int i = 0; i <= fileQueue.idx; i++)
	{
		enqueue(&blockedQueue, fileQueue.pqVal[i]);
	}
}

//print value of varx
void print(int pid, char varX) {
	char* value = getVariableValue(pid, varX);
	//char* value = "Hello world!";
	printf("%s\n", value);

	return;
}

//make a variable x, and assign y [ y could be input, int or string]
void assign(int pid, char varX, char* strY) {
	char str[64];
	if (strcmp(strY, "input") == 0) {
		printf("Please enter a value: ");
		scanf("%s", str);
		setVariableValue(pid, varX, str);
	}
	else {
		setVariableValue(pid, varX, strY);
	}
}

void writeFile(char* fileName, char* data) {

	// Open the file in write mode
	FILE* file = fopen(fileName, "w");


	// Write data to the file
	fprintf(file, "%s", data);

	// Check if the file was opened successfully
	if (file == NULL) {
		printf("Error opening file.\n");
		return;
	}

	// Close the file
	fclose(file);

	printf("File %s created successfully.\n", fileName);

}

char* readFile(int pid, char var) {

	char* fileName = getVariableValue(pid, var);
	// Open the file in read mode
	FILE* file = fopen(fileName, "r");

	// Check if the file was opened successfully
	if (file == NULL) {
		printf("Error opening file.\n");
		return "\0";
	}

	// Read and print the contents of the file
	char buffer[64];
	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		//printf("%s\n", buffer);
	}

	// Close the file
	fclose(file);
	return buffer;
}

//print all numbers between x and y
void printFromTo(int pid, char varX, char varY) {
	int x = atoi(getVariableValue(pid, varX));
	int y = atoi(getVariableValue(pid, varY));

	for (int i = x + 1; i < y; i++) {
		printf("%i\n", i);
	}
	return;
}

int semWait(int pid, char* Sem) {
	if (strcmp(Sem, "userInput") == 0) {
		if (!inputBSemaphore.value && inputBSemaphore.owner != pid)
		{
			enqueuePriorityQueue(&inputQueue, pid, getProgramPriority(pid));
			updateGeneralBlockedQueue();
			return -1;
		}
		else
		{
			inputBSemaphore.value = false;
			inputBSemaphore.owner = pid;
			return 0;	
		}
	}
	else if (strcmp(Sem, "userOutput") == 0) {
		if (!outputBSemaphore.value && outputBSemaphore.owner != pid)
		{
			enqueuePriorityQueue(&outputQueue, pid, getProgramPriority(pid));
			updateGeneralBlockedQueue();
			return -1;
		}
		else
		{
			outputBSemaphore.value = false;
			outputBSemaphore.owner = pid;
			return 0;
		}
	}
	else if (strcmp(Sem, "file") == 0)
	{
		if (!fileBSemaphore.value && fileBSemaphore.owner != pid)
		{
			enqueuePriorityQueue(&fileQueue, pid, getProgramPriority(pid));
			updateGeneralBlockedQueue();
			return -1;
		}
		else
		{
			fileBSemaphore.value = false;
			fileBSemaphore.owner = pid;
			return 0;
		}
	}
	else
	{
		perror("Invalid semaphore");
		return -1;
	}
}

void semSignal(char* Sem) {
	int unblockedProcess = -1;
	if (strcmp(Sem, "userInput") == 0) {
		inputBSemaphore.value = true;
		if(!isPriorityQueueEmpty(&inputQueue))
		unblockedProcess = dequeuePriorityQueue(&inputQueue);
	}
	else if (strcmp(Sem, "userOutput") == 0) {
		outputBSemaphore.value = true;
		if (!isPriorityQueueEmpty(&outputQueue))
		unblockedProcess = dequeuePriorityQueue(&outputQueue);
	}
	else
	{
		fileBSemaphore.value = true;
		if (!isPriorityQueueEmpty(&fileQueue))
		unblockedProcess = dequeuePriorityQueue(&fileQueue);
	}
	updateGeneralBlockedQueue();
	if (unblockedProcess != -1)
	queueProcess(unblockedProcess);
}

int executeInstruction(int processId)
{
	int numTokens;
	char* instruction = getCurrentInstruction(processId);
	char** tokens = splitString(instruction, &numTokens);

	if (strcmp(tokens[0], "print") == 0)
	{
		if (!outputBSemaphore.value && outputBSemaphore.owner != processId) {
			enqueuePriorityQueue(&outputQueue, processId, getProgramPriority(processId));
			updateGeneralBlockedQueue();
			return -1;
		}
		else {
			print(processId, tokens[1][0]);
			return 0;
		}
	}

	else if (strcmp(tokens[0], "assign") == 0)
	{
		if (strcmp(tokens[2], "input") == 0)
		{
			if (!inputBSemaphore.value && inputBSemaphore.owner != processId)
			{
				enqueuePriorityQueue(&inputQueue, processId, getProgramPriority(processId));
				updateGeneralBlockedQueue();
				return -1;
			}
			else
			{
				assign(processId, tokens[1][0], tokens[2]);
				return 0;
			}
		}
		else if(strcmp(tokens[2], "readFile") == 0)
		{
			if (!fileBSemaphore.value && fileBSemaphore.owner != processId)
			{
				enqueuePriorityQueue(&fileQueue, processId, getProgramPriority(processId));
				updateGeneralBlockedQueue();
				return -1;
			}
			else
			{
				assign(processId, tokens[1][0], readFile(processId,tokens[3][0]));
				return 0;
			}
		}
		else
		{
			assign(processId, tokens[1][0], tokens[2]);
			return 0;
		}
	}

	else if (strcmp(tokens[0], "writeFile") == 0)
	{
		if (!fileBSemaphore.value && fileBSemaphore.owner != processId)
		{
			enqueuePriorityQueue(&fileQueue, processId, getProgramPriority(processId));
			updateGeneralBlockedQueue();
			return -1;
		}
		else
		{
			writeFile(tokens[1], tokens[2]);
			return 0;
		}
	}

	else if (strcmp(tokens[0], "readFile") == 0)
	{
		if (!fileBSemaphore.value && fileBSemaphore.owner != processId)
		{
			enqueuePriorityQueue(&fileQueue, processId, getProgramPriority(processId));
			updateGeneralBlockedQueue();
			return -1;
		}
		else
		{
			readFile(processId,tokens[1][0]);
			return 0;
		}
	}

	else if (strcmp(tokens[0], "printFromTo") == 0)
	{
		if (!outputBSemaphore.value && outputBSemaphore.owner != processId)
		{
			enqueuePriorityQueue(&outputQueue, processId, getProgramPriority(processId));
			updateGeneralBlockedQueue();
			return -1;
		}
		else
		{
			printFromTo(processId, tokens[1][0], tokens[2][0]);
			return 0;
		}
	}

	else if (strcmp(tokens[0], "semWait") == 0)
	{
		return semWait(processId, tokens[1]);
	}

	else if (strcmp(tokens[0], "semSignal") == 0)
	{
		semSignal(tokens[1]);
		return 0;
	}

	else
	{
		perror("Invalid instruction");
	}

	return 0;
}
#pragma endregion

#pragma region printing
void printMemoryContents() {
	for (int i = 0; i < 60; i++) {

		char* name_color = ANSI_COLOR_CYAN;
		char* data_color = ANSI_COLOR_GREEN;

		if (strcmp(memory.words[i].name, "pid") == 0) {
			name_color = ANSI_COLOR_RED;
			data_color = ANSI_COLOR_RED;
		}

		// Print the memory contents with the appropriate colors
		printf("Word %-2d > %s%-20s" ANSI_COLOR_RESET ": %s%s" ANSI_COLOR_RESET "\n", i, name_color, memory.words[i].name, data_color, memory.words[i].data);
	}
}

void printQueue(struct Queue* queue) {
	int i;
	for (i = queue->front; i != (queue->rear + 1) % queue->capacity; i = (i + 1) % queue->capacity) {
		printf("%d ", queue->array[i]);
	}
}

void printPriorityQueue(struct PriorityQueue* pQueue)
{
	struct PriorityQueue tempQueue;
	initPriorityQueue(&tempQueue); 

	for (int i = 0; i <= pQueue->idx; i++)
	{
		enqueuePriorityQueue(&tempQueue, pQueue->pqVal[i], pQueue->pqPriority[i]);
	}

	while (!isPriorityQueueEmpty(&tempQueue))
	{
		int val = dequeuePriorityQueue(&tempQueue);
		printf("%d ", val);
	}
}

void printQueues() {
	printf(ANSI_COLOR_RED "Priority 1 queue: ");
	printQueue(&priority1Queue);
	printf("\n");

	printf(ANSI_COLOR_YELLOW "Priority 2 queue: ");
	printQueue(&priority2Queue);
	printf("\n");

	printf(ANSI_COLOR_GREEN "Priority 3 queue: ");
	printQueue(&priority3Queue);
	printf("\n");

	printf(ANSI_COLOR_BLUE "Priority 4 queue: ");
	printQueue(&priority4Queue);
	printf("\n");

	printf(ANSI_COLOR_MAGENTA "General blocked queue: ");
	printQueue(&blockedQueue);
	printf("\n");

	printf(ANSI_COLOR_MAGENTA "Input blocked queue: ");
	printPriorityQueue(&inputQueue);
	printf("\n");

	printf(ANSI_COLOR_MAGENTA "Output blocked queue: ");
	printPriorityQueue(&outputQueue);
	printf("\n");

	printf(ANSI_COLOR_MAGENTA "File blocked queue: ");
	printPriorityQueue(&fileQueue);
	printf(ANSI_COLOR_RESET "\n");
}
#pragma endregion

int main()
{
	int clock = 0;
	bool running = true;
	int currentProcessId;
	int pc = 0;

    priority1Queue = *createQueue(QUEUE_CAPACITY);
    priority2Queue = *createQueue(QUEUE_CAPACITY);
    priority3Queue = *createQueue(QUEUE_CAPACITY);
    priority4Queue = *createQueue(QUEUE_CAPACITY);
    blockedQueue = *createQueue(QUEUE_CAPACITY);
	initPriorityQueue(&inputQueue);
	initPriorityQueue(&outputQueue);
	initPriorityQueue(&fileQueue);
	initBinarySemaphore(&inputBSemaphore);
	initBinarySemaphore(&outputBSemaphore);
	initBinarySemaphore(&fileBSemaphore);

	//Initialize arrival times
	printf("Welcome to the MLFQ scheduler! Please note that programs should be saved as 'Program_N.txt'.\n");
	int numPrograms = setArrivalTimes();
	if (numPrograms == -1)
	{
		printf(ANSI_COLOR_RED"Error setting arrival times, program could not start.");
		Sleep(10000);
		return -1;
	}

	//Main loop
	while (numPrograms > 0 || running)
	{
		if (clock == 14)
			printf("");

		printf("\n%s%s Clock cycle: %d %s \n",ANSI_COLOR_BLACK, ANSI_BACKGROUND_WHITE, clock, ANSI_COLOR_RESET);
		//Check if a program has arrived
		while (clock == arrivalClockCycle)
		{
			//Load program into memory
			printf("%s Program %d has arrived%s \n", ANSI_BACKGROUND_GREEN, arrivalProgram, ANSI_COLOR_RESET);
			char** lines = readInstructionFile(arrivalProgram);
			loadProgramIntoMemory(arrivalProgram, lines);
			queueProcess(arrivalProgram);

			arrivalClockCycle = dequeue(&arrivalQueue);
			arrivalProgram++;
			numPrograms--;
		}

		//Execute programs
		currentProcessId = dequeNextProcess();
		if (currentProcessId == -1)
		{
			running = false;
			printf("%s All queues are currently empty %s\n", ANSI_BACKGROUND_RED, ANSI_COLOR_RESET);
		}
		else
		{
			running = true;
			int priority = getProgramPriority(currentProcessId);
			printf("%s Currently running process: %d (Priority %d) %s\n", ANSI_BACKGROUND_BLUE, currentProcessId, priority, ANSI_COLOR_RESET);
			printf("%s Current instruction: %s%s\n",ANSI_COLOR_BLUE, getCurrentInstruction(currentProcessId), ANSI_COLOR_RESET);

			printQueues();
			printMemoryContents();

			setProgramState(currentProcessId, "running");
			int executed = executeInstruction(currentProcessId);

			if (executed == 0)
			{
				setProgramState(currentProcessId, "ready");

				pc = incrementProgramCounter(currentProcessId);
				if (pc != -1)
				{
					updateProgramPriority(currentProcessId);
					queueProcess(currentProcessId);
				}

				else
				{
					setProgramState(currentProcessId, "terminated");
				}
			}
			else
			{
				setProgramState(currentProcessId, "blocked");
			}
		}
		clock++;
	}
	printf("All programs have finished executing, press any key to exit\n");
	int ret = scanf("%d");
}