#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "Reader.h"
#include "Analyzer.h"


volatile sig_atomic_t stop = 0;

void *reader_loop()
{
    while (!stop)
    {
        // Reader
        char **data;
        size_t rows;

        get_buffer_without_header(&data, &rows);
        
        // Analyzer
        float *cpusUsage;

        process_data(&data, rows);
        get_cpus_usage(&cpusUsage, rows);

        if (data != NULL && rows > 0)
        {
            for (size_t i = 0; i < rows; i++)
                printf("%lu: %.2f%%\n", i, cpusUsage[i] * 100);
        }

        sleep(1);
        printf("\n\n");
    }

    return 0;
}

void cleanup()
{
    printf("Cleaning up...\n");
    destroy_analyzer();
    destroy_reader();
}

void sigint_handler()
{
    printf("SIGINT signal received. Closing app...\n");
    stop = 1;
}

void sigterm_handler()
{
    printf("SIGTERM signal received. Closing app...\n");    
    stop = 1;
}

int main()
{
    open_proc_stat();

    pthread_t readerThread;
    pthread_create(&readerThread, NULL, &reader_loop, NULL);

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    pthread_join(readerThread, NULL);

    printf("ReaderThread joined.\n");

    cleanup();

    return 0;
}