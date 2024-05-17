#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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
char** readInstructionFile(int programNum)
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
			if (programCounter > getUpperMemoryBound(processId))
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
	else if (quantum < 2)
	{
		newPriority = '2';
	}
	else if (quantum < 4)
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

#pragma region program execution

//print value of varx
void print(int pid, char varX){
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
//void assign(int pid, char varX, int intY) {
//	char str[64];
//	sprintf(str, "% d", intY);
//	setVariableValue(pid, varX, str);
//}




void writeFile(char* fileName, char* data){
    
	for (int i = 0; i < 60; i++) {
		if(memory.words[i].name == NULL){
			strcpy(memory.words[i].name, fileName);
			strcpy(memory.words[i].data, data);
			break;
		}
	}
}

char* readFile(char* fileName){

	for (int i = 0; i < 60; i++) {
		if (strcmp(memory.words[i].name, fileName) == 0) {
			return memory.words[i].data;
		}
	}
	return "File does not exist";
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

void semWait(char* Sem){
	if (strcmp(Sem, "userInput") == 0) {
		inputBSemaphore = false;
	}
	else if (strcmp(Sem, "userOutput") == 0){
		outputBSemaphore = false;
	}
	else
	{
		fileBSemaphore = false;
	}
}

void semSignal(char* Sem){
	if (strcmp(Sem, "userInput") == 0) {
		inputBSemaphore = true;
	}
	else if (strcmp(Sem, "userOutput") == 0) {
		outputBSemaphore = true;
	}
	else
	{
		fileBSemaphore = true;
	}
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
		printf("All queues are empty\n");
		return -1;
	}
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

	printf(ANSI_COLOR_RESET "Blocked queue: ");
	printQueue(&blockedQueue);
	printf("\n");
}
#pragma endregion

int main()
{

    priority1Queue = *createQueue(QUEUE_CAPACITY);
    priority2Queue = *createQueue(QUEUE_CAPACITY);
    priority3Queue = *createQueue(QUEUE_CAPACITY);
    priority4Queue = *createQueue(QUEUE_CAPACITY);
    blockedQueue = *createQueue(QUEUE_CAPACITY);

	//Load programs into memory
	for (int i = 1; i <= 3; i++)
    {
        char** lines = readInstructionFile(i);
        loadProgramIntoMemory(i, lines);
		queueProcess(i);
	}

	int currentProcessId;
	int pc;
	//Execute programs
	while (true)
	{
		currentProcessId = dequeNextProcess();
		if (currentProcessId == -1)
		{
			break;
		}

		//printMemoryContents();
		//printQueues();
		//printf("Current running process: %d\n", currentProcessId);

		ProgramState(currentProcessId, "running");
		//execute()
		setProgramState(currentProcessId, "ready");
		pc = incrementProgramCounter(currentProcessId);
		updateProgramPriority(currentProcessId);
		if (pc == -1)
		{
			//Program has finished
			setProgramState(currentProcessId, "terminated");
			//TODO: Free memory
		}
		else
		{
			queueProcess(currentProcessId);
		}
	}
	//writeFile("m", "m");
	printMemoryContents();
	char* newFile = readFile("m");
	//printf("%s\n", newFile);

	
	printf("All programs have finished executing, press any key to exit\n");
	scanf("%s");
}