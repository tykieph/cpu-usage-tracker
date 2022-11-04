#include "SafeQueue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/********************************************************************************/

/********************************************************************************/
/* typical (non concurrent safe) queue implementation */
static Queue *init(void);
static int enqueue(Queue *q, char *data);
static char *dequeue(Queue *q);
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
void queue_push(Queue *q, char *data)
{
    pthread_mutex_lock(&q->mtx);

    enqueue(q, data);

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mtx);
}
/********************************************************************************/
void queue_wait_pop(Queue *q, char **buffer)
{
    pthread_mutex_lock(&q->mtx);

    if (is_empty(q))
    {
        pthread_cond_wait(&q->cond, &q->mtx);
    }

    char *tmp = dequeue(q);

    *buffer = malloc(sizeof(char) * LOG_ENTRY_MAX);
    strncpy(*buffer, tmp, LOG_ENTRY_MAX);

    pthread_mutex_unlock(&q->mtx);
}
/********************************************************************************/
int queue_timedwait_pop(Queue *q, char **buffer, unsigned int sec)
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

    char *tmp = dequeue(q);

    *buffer = malloc(sizeof(char) * LOG_ENTRY_MAX);
    strncpy(*buffer, tmp, LOG_ENTRY_MAX);

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
int enqueue(Queue *q, char *data)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
        return 0;

    newNode->data = malloc(sizeof(char) * LOG_ENTRY_MAX);
    strncpy(newNode->data, data, LOG_ENTRY_MAX);

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
char *dequeue(Queue *q)
{
    if (q->head == NULL)
    {
        return "";
    }

    Node *oldHead = q->head;
    q->head = oldHead->next;

    if (q->head == NULL)
    {
        q->tail = NULL;
    }

    char *data = malloc(sizeof(char) * LOG_ENTRY_MAX);
    strncpy(data, oldHead->data, LOG_ENTRY_MAX);

    free(oldHead->data);    
    free(oldHead);

    q->size -= 1;

    return data;
}
/********************************************************************************/
int is_empty(Queue *q)
{
    return q->head == NULL;
}
/********************************************************************************/

/********************************************************************************/
