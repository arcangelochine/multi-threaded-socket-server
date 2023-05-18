#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <my_thread.h>
#include <stdlib.h>

typedef struct node
{
    void *data;
    struct node *next;
} Node;

typedef struct
{
    Mutex lock;
    Cond cond;

    size_t length;
    Node *head;
    Node *tail;
} Thread_Safe_Queue;

void TSQInit(Thread_Safe_Queue *queue)
{
    MInit(&queue->lock);
    CInit(&queue->cond);

    queue->length = 0;
    queue->head = queue->tail = NULL;
}

void TSQDestroy(Thread_Safe_Queue *queue)
{
    MDestroy(&queue->lock);
    CDestroy(&queue->cond);

    while (queue->head)
    {
        Node *temp = queue->head;
        queue->head = queue->head->next;
        free(temp);
    }

    queue->tail = NULL;
}

void TSQPush(Thread_Safe_Queue *queue, void *data)
{
    if (queue == NULL)
        return;

    Node *new_node = (Node *)malloc(sizeof(Node));

    if (new_node == NULL)
        return;

    new_node->data = data;
    new_node->next = NULL;

    Lock(&queue->lock);

    if (queue->head == NULL)
        queue->head = new_node;
    else
        queue->tail->next = new_node;

    queue->tail = new_node;

    queue->length++;

    Signal(&queue->cond);
    Unlock(&queue->lock);
}

/**
 * @brief Always waits for a TSQPush
*/
void TSQPop(Thread_Safe_Queue *queue, void **buffer)
{
    if (queue == NULL || buffer == NULL)
        return;

    Lock(&queue->lock);

    while (queue->head == NULL)
        Wait(&queue->cond, &queue->lock);

    Node *temp = queue->head;
    queue->head = queue->head->next;

    if (queue->head == NULL)
        queue->tail = NULL;

    queue->length--;

    Unlock(&queue->lock);

    *buffer = temp->data;
    free(temp);
}

unsigned char TSQIsEmpty(Thread_Safe_Queue *queue)
{
    unsigned char empty = 0;
    Lock(&queue->lock);
    empty = queue->head == NULL;
    Unlock(&queue->lock);
    return empty;
}

/**
 * @warning This function is not thread safe. DO NOT USE without holding a lock.
*/
size_t TSQLength(Thread_Safe_Queue *queue)
{
    return queue->length;
}

#endif