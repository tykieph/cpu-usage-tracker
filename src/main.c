#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#include "Reader.h"
#include "Analyzer.h"
#include "Printer.h"

#define SLEEP_INTERVAL_MS 1000

/********************************************************************************/
volatile sig_atomic_t stop = 0;
struct timespec start, finish;

sem_t semReader;
sem_t semAnalyzer;

char **data;
size_t rows;

sem_t semUsage;
sem_t semPrinter;
float *usage;
/********************************************************************************/
void *reader_loop(void *arg);
void *analyzer_loop(void *arg);
void *printer_loop(void *arg);

void sig_handler(int signum);
void init(void);
void cleanup(void);
/********************************************************************************/
int main(void)
{
    init();

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

    cleanup();

    return 0;
}
/********************************************************************************/
void init()
{
    clock_gettime(CLOCK_MONOTONIC, &start);

    open_proc_stat();

    // handle signals
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // init semaphores
    sem_init(&semReader, 0, 0);
    sem_init(&semAnalyzer, 0, 1);

    sem_init(&semUsage, 0, 0);
    sem_init(&semPrinter, 0, 1);
}
/********************************************************************************/
void cleanup()
{
    printf("Cleaning up...\n");

    // destroy semaphores
    sem_destroy(&semReader);
    sem_destroy(&semAnalyzer);

    sem_destroy(&semUsage);
    sem_destroy(&semPrinter);

    // free memory
    destroy_analyzer();
    destroy_reader();

    clock_gettime(CLOCK_MONOTONIC, &finish);

    double elapsed = finish.tv_sec - start.tv_sec;
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Program execution took: %f seconds.\n", elapsed);
}
/********************************************************************************/
void *reader_loop(void *arg)
{
    (void)arg; // unused

    while (1)
    {
        // wait for Analyzer to be done with processing data array
        sem_wait(&semAnalyzer);

        if (stop)
        {
            sem_post(&semReader);
            break;
        }

        get_buffer_without_header(&data, &rows);

        // tell Analyzer it can start processing data array
        sem_post(&semReader);

        usleep(SLEEP_INTERVAL_MS * 1000);        
    }

    return 0;
}
/********************************************************************************/
void *analyzer_loop(void *arg)
{
    (void)arg; // unused

    while (1)
    {
        // wait for Reader to finish writing data into data array
        sem_wait(&semReader);

        if (stop)
        {
            sem_post(&semAnalyzer);
            sem_post(&semUsage);
            break;
        }

        process_data(&data, rows);

        // tell Reader it can start writing into data array
        sem_post(&semAnalyzer);


        // wait for Printer to finish reading usage array
        sem_wait(&semPrinter);
        
        get_cpus_usage(&usage, rows);

        // tell Printer it can start reading from usage array
        sem_post(&semUsage);
    }

    return 0;
}
/********************************************************************************/
void *printer_loop(void *arg)
{   
    (void)arg; // unused

    while (1)
    {
        // wait for Analyzer to finish processing usage array
        sem_wait(&semUsage);    
        
        if (stop)
        {
            break;
        }

        print_cpus_usage(&usage, rows);
        putchar('\n');

        // tell Analyzer it can start writing data into usage array
        sem_post(&semPrinter);
    }

    return 0;
}
/********************************************************************************/
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
/********************************************************************************/

/********************************************************************************/