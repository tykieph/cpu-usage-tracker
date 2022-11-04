#pragma once

#include <pthread.h>
#include <stdlib.h>

#define LOG_ENTRY_MAX 128

typedef struct Node
{
    char *data;
    struct Node *next; 
} Node;

typedef struct 
{
    Node *head;
    Node *tail;

    pthread_mutex_t mtx;
    pthread_cond_t cond;  
    size_t size;  
} Queue;


/* thread safe queue implementation */

Queue *queue_init(void);
void queue_push(Queue *q, char *data, const size_t size);
void queue_wait_pop(Queue *q, char *buffer, const size_t size);
int queue_timedwait_pop(Queue *q, char *buffer, const size_t size, const uint sec);
int queue_empty(Queue *q);
