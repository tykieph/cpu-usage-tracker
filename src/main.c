#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "Reader.h"
#include "Analyzer.h"
#include "Printer.h"


volatile sig_atomic_t stop = 0;

void *reader_loop()
{
    while (!stop)
    {
        clock_t start = clock();
        // Reader
        char **data;
        size_t rows;

        get_buffer_without_header(&data, &rows);
        
        // Analyzer
        float *cpusUsage;

        process_data(&data, rows);
        get_cpus_usage(&cpusUsage, rows);

        // Printer
        print_cpus_usage(&cpusUsage, rows);
        putchar('\n');

        clock_t stop = clock();
        float seconds = (float)(stop - start) / (CLOCKS_PER_SEC);

        printf("Loop iteration took: %fms\n", seconds * 1000);

        sleep(1);
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

    // pthread_t readerThread;
    // pthread_create(&readerThread, NULL, &reader_loop, NULL);

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    reader_loop();

    // pthread_join(readerThread, NULL);

    printf("ReaderThread joined.\n");

    cleanup();

    return 0;
}