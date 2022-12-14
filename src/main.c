#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#include "Reader.h"
#include "Analyzer.h"
#include "Printer.h"
#include "Logger.h"

#define SLEEP_INTERVAL_MS 1000
#ifndef USE_LOGGER
    #define USE_LOGGER
#endif

#ifdef USE_LOGGER
    #define LOG(from, msg) logger_postMsg(from, msg)
#else
    #define LOG(from, msg) (void)from
#endif

/********************************************************************************/
static volatile sig_atomic_t stop = 0;
static struct timespec start, finish;

static sem_t semReader;
static sem_t semAnalyzer;

static char **data;
static size_t rows;

static sem_t semUsage;
static sem_t semPrinter;
static float *usage;

#ifdef USE_LOGGER
static pthread_t loggerThd;
#endif
/********************************************************************************/
static void *reader_loop(void *arg);
static void *analyzer_loop(void *arg);
static void *printer_loop(void *arg);

static void sig_handler(int signum);
static void init(void);
static void cleanup(void);
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
                LOG("main", "Created Reader thread");
                break;
            case Analyzer:
                pthread_create(&thd[i], NULL, &analyzer_loop, NULL);
                LOG("main", "Created Analyzer thread");
                break;
            case Printer:
                pthread_create(&thd[i], NULL, &printer_loop, NULL);
                LOG("main", "Created Printer thread");
                break;
        }
    }


    for (size_t i = 0; i < 3; i++)
    {
        pthread_join(thd[i], NULL);
        
        char buff[128];
        sprintf(buff, "Thread %ld has joined successufully.", i);

        LOG("main", buff);
    }

    cleanup();

    return 0;
}
/********************************************************************************/
void init()
{
    #ifdef USE_LOGGER
    if (!logger_init())
    {
        fprintf(stderr, "Logger error: cannot create Logger instance.\n");
    }
    else
    {
        pthread_create(&loggerThd, NULL, &logger_start, NULL);
        LOG("main", "Logger thread created successufully");
    }
    #endif

    LOG("main", "App initialization");

    clock_gettime(CLOCK_MONOTONIC, &start);

    open_proc_stat();

    // handle signals
    // signal(SIGINT, sig_handler);
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
    LOG("main", "App cleanup...");

    // destroy semaphores
    sem_destroy(&semReader);
    sem_destroy(&semAnalyzer);

    sem_destroy(&semUsage);
    sem_destroy(&semPrinter);

    // free memory
    destroy_analyzer();
    destroy_reader();

    // stop logger
    #ifdef USE_LOGGER
    LOG("main", "Stopping logger...");
    logger_stop();
    pthread_join(loggerThd, NULL);
    #endif

    clock_gettime(CLOCK_MONOTONIC, &finish);

    double elapsed = (double)(finish.tv_sec - start.tv_sec);
    elapsed += (double)(finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    // printf("Program execution took: %f seconds.\n", elapsed);    
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
            LOG("Reader", "Interrupt detected. Stopping execution.");
            sem_post(&semReader);
            break;
        }

        LOG("Reader", "Reading /proc/stat file into buffer");
        get_buffer_without_header(&data, &rows);

        // tell Analyzer it can start processing data array
        sem_post(&semReader);

        LOG("Reader", "Sleep");
        usleep(SLEEP_INTERVAL_MS * 1000);        
    }

    LOG("Reader", "Stopped. Waiting to join.");
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
            LOG("Analyzer", "Interrupt detected. Stopping execution.");
            sem_post(&semAnalyzer);
            sem_post(&semUsage);
            break;
        }

        LOG("Analyzer", "Processing data read from /proc/stat file");
        process_data(&data, rows);

        // tell Reader it can start writing into data array
        sem_post(&semAnalyzer);


        // wait for Printer to finish reading usage array
        sem_wait(&semPrinter);
        
        LOG("Analyzer", "Preparing data for Printer");
        get_cpus_usage(&usage, rows);

        // tell Printer it can start reading from usage array
        sem_post(&semUsage);
    }

    LOG("Analyzer", "Stopped. Waiting to join.");
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
            LOG("Printer", "Interrupt detected. Stopping execution.");
            sem_post(&semPrinter);
            break;
        }

        LOG("Printer", "Start printing");
        print_cpus_usage(&usage, rows);
        putchar('\n');
        LOG("Printer", "Printing done");

        // tell Analyzer it can start writing data into usage array
        sem_post(&semPrinter);
    }

    LOG("Printer", "Stopped. Waiting to join.");
    return 0;
}
/********************************************************************************/
void sig_handler(int signum)
{
    if (signum == SIGINT)
    {
        LOG("Signal handler", "SIGINT signal received. Closing app...");
    }
    else if (signum == SIGTERM)
    {
        LOG("Signal handler", "SIGTERM signal received. Closing app...");   
    } 

    stop = 1;
}
/********************************************************************************/

/********************************************************************************/
