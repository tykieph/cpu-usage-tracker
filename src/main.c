#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "Reader.h"
#include "Analyzer.h"
#include "Printer.h"


volatile sig_atomic_t stop = 0;

pthread_cond_t condData;
pthread_mutex_t mtxData;
char **data;
size_t rows;

pthread_cond_t condUsage;
pthread_mutex_t mtxUsage;
float *usage;

void *reader_loop(void *arg);
void *analyzer_loop(void *arg);
void *printer_loop(void *arg);

void sig_handler(int signum);
void cleanup(void);


int main(void)
{
    open_proc_stat();


    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    pthread_mutex_init(&mtxData, NULL);
    pthread_cond_init(&condData, NULL);

    pthread_mutex_init(&mtxUsage, NULL);
    pthread_cond_init(&condUsage, NULL);

    pthread_t thd[3];

    enum enumThd
    {
        Reader = 0,
        Analyzer,
        Printer
    };

    for (size_t i = 0; i < 3; i++)
    {
        switch (i)
        {
            case Reader:
                pthread_create(&thd[i], NULL, &reader_loop, NULL);
                break;
            case Analyzer:
                pthread_create(&thd[i], NULL, &analyzer_loop, NULL);
                break;
            case Printer:
                pthread_create(&thd[i], NULL, &printer_loop, NULL);
                break;
        }
    }
    

    for (size_t i = 0; i < 3; i++)
    {
        pthread_join(thd[i], NULL);
        
        printf("Thread %ld has joined successufully.\n", i);
    }

    pthread_mutex_destroy(&mtxData);    
    pthread_mutex_destroy(&mtxUsage);

    cleanup();

    return 0;
}


void *reader_loop(void *arg)
{
    (void)arg; // unused
    
    while (1)
    {
        pthread_mutex_lock(&mtxData);

        if (stop)
        {
            pthread_mutex_unlock(&mtxData);
            pthread_cond_signal(&condData);

            return 0;
        }

        get_buffer_without_header(&data, &rows);
        pthread_mutex_unlock(&mtxData);

        // signal condData condition, that tells Analyzer it can start processing data array
        pthread_cond_signal(&condData);

        usleep(1000 * 1000);
    }

    return 0;
}

void *analyzer_loop(void *arg)
{
    (void)arg; // unused

    while (1)
    {
        pthread_mutex_lock(&mtxData);

        // wait for condData condition 
        pthread_cond_wait(&condData, &mtxData);

        if (stop)
        {
            pthread_mutex_unlock(&mtxData);
            pthread_cond_signal(&condUsage);

            return 0;
        }

        process_data(&data, rows);

        // lock mtxUsage before writing into usage array
        pthread_mutex_lock(&mtxUsage);
        get_cpus_usage(&usage, rows);
        pthread_mutex_unlock(&mtxUsage);

        // signal condUsage condition, that tells Printer it can start reading from usage array
        pthread_cond_signal(&condUsage);

        pthread_mutex_unlock(&mtxData);
    }

    return 0;
}

void *printer_loop(void *arg)
{   
    (void)arg; // unused

    while (1)
    {
        pthread_mutex_lock(&mtxUsage);        
        pthread_cond_wait(&condUsage, &mtxUsage); // wait for Analyzer to send signal that usage array is ready to read from
        
        if (stop)
        {
            pthread_mutex_unlock(&mtxUsage);
            return 0;
        }

        print_cpus_usage(&usage, rows);
        putchar('\n');

        pthread_mutex_unlock(&mtxUsage);
    }

    return 0;
}

void cleanup()
{
    printf("Cleaning up...\n");
    destroy_analyzer();
    destroy_reader();
}

void sig_handler(int signum)
{
    if (signum == SIGINT)
    {
        printf("SIGINT signal received. Closing app...\n");
    }
    else if (signum == SIGTERM)
    {
        printf("SIGTERM signal received. Closing app...\n");   
    }

    stop = 1;
}