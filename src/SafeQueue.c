#include "SafeQueue.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/********************************************************************************/

/********************************************************************************/
/* typical (non concurrent safe) queue implementation */
static Queue *init(void);
static int enqueue(Queue *q, char *data, const size_t size);
static int dequeue(Queue *q, char *data, const size_t size);
static int is_empty(Queue *q);
/********************************************************************************/
Queue *queue_init()
{
    Queue *q = init(); 

    pthread_mutex_init(&q->mtx, NULL);
    pthread_cond_init(&q->cond, NULL);

    return q;
}
/********************************************************************************/
void queue_push(Queue *q, char *data, const size_t size)
{
    pthread_mutex_lock(&q->mtx);

    enqueue(q, data, size);

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mtx);
}
/********************************************************************************/
void queue_wait_pop(Queue *q, char *buffer, const size_t size)
{
    pthread_mutex_lock(&q->mtx);

    if (is_empty(q))
    {
        pthread_cond_wait(&q->cond, &q->mtx);
    }

    dequeue(q, buffer, size);

    pthread_mutex_unlock(&q->mtx);
}
/********************************************************************************/
int queue_timedwait_pop(Queue *q, char *buffer, const size_t size, const uint sec)
{
    pthread_mutex_lock(&q->mtx);

    if (is_empty(q))
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += sec;

        int ret = pthread_cond_timedwait(&q->cond, &q->mtx, &ts);
        if (ret) // error or ETIMEDOUT
        {
            pthread_mutex_unlock(&q->mtx);
            return ret;
        }
    }

    dequeue(q, buffer, size);

    pthread_mutex_unlock(&q->mtx);

    return 0;
}
/********************************************************************************/
int queue_empty(Queue *q)
{
    pthread_mutex_lock(&q->mtx);
    int empty = is_empty(q);
    pthread_mutex_unlock(&q->mtx);

    return empty;
}
/********************************************************************************/
Queue *init()
{
    Queue *q = malloc(sizeof(Queue));

    q->head = NULL;
    q->tail = NULL; 
    q->size = 0;  

    return q; 
}
/********************************************************************************/
int enqueue(Queue *q, char *data, const size_t size)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
        return 0;

    newNode->data = malloc(sizeof(char) * size);
    strncpy(newNode->data, data, size);

    newNode->next = NULL;
    if (q->tail != NULL)
    {
        q->tail->next = newNode;
    }
    q->tail = newNode;

    if (q->head == NULL)
    {
        q->head = newNode;
    }

    q->size += 1;

    return 1;
}
/********************************************************************************/
int dequeue(Queue *q, char *data, const size_t size)
{
    if (q->head == NULL)
    {
        return 0;
    }

    Node *oldHead = q->head;
    q->head = oldHead->next;

    if (q->head == NULL)
    {
        q->tail = NULL;
    }

    strncpy(data, oldHead->data, size);

    free(oldHead->data);    
    free(oldHead);

    q->size -= 1;

    return 1;
}
/********************************************************************************/
int is_empty(Queue *q)
{
    return q->head == NULL;
}
/********************************************************************************/

/********************************************************************************/
