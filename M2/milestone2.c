#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
    int val;
    struct node *next;
}node_t;

void enqueue(node_t **head, int val)
{
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node)
        return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
}

int dequeue(node_t **head)
{
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL)
        return -1;

    current = *head;
    while (current->next != NULL)
    {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}

node_t priority1Queue = {0, NULL};
node_t priority2Queue = {0, NULL};
node_t priority3Queue = {0, NULL};
node_t priority4Queue = {0, NULL};
node_t blockedQueue = {0, NULL};

struct PCB {
    int pid;
    char processState;
    int currentPriority;
    int programCounter;
    int lowerMemoryBound;
    int higherMemoryBound;
};

struct word {
    char name[32];
    char data[32];
};

struct memory {
    struct word words[60];
};
