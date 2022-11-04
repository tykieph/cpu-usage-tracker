#include "Logger.h"

#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "SafeQueue.h"

/********************************************************************************/
#define LOGGER_SAVE_PERIOD 2 // save every (value) sec

static const char *fileName = "CUT_debug.log";
static const char *directory = "../tmp";
static FILE *mFile = NULL;

static pthread_mutex_t mtxSave; // protect file save
static Queue *q;

static unsigned int stop; // this flag tells logger it can stop executing main loop if queue is empty 
static unsigned int done; // this flag tells if logger finished executing main loop

static struct timespec timer;
/********************************************************************************/
/* initialize logger - open log file (create if not present), initialize some vars */
static int init(const char *flags);

/* open log file specified by *path* */
static int open_log_file(const char *path, const char *flags);

/* returns current local time ([hh:mm:ss]) in *buff* */
static void curr_time(char *buff);

/* returns time difference from *start* untill now */
static double time_diff(struct timespec *start);
/********************************************************************************/
int logger_init()
{
    if (mFile != NULL)
        return 0;

    pthread_mutex_init(&mtxSave, NULL);
    q = queue_init();

    clock_gettime(CLOCK_MONOTONIC, &timer);    

    return init("w");
}
/********************************************************************************/
void *logger_start(void *arg)
{
    (void)arg;
    while (!stop || !queue_empty(q))
    {
        char *buff;
        if(queue_timedwait_pop(q, &buff, 1))
        {
            continue;
        }
        // queue_wait_pop(q, &buff);


        // save log entry to the log file
        fprintf(mFile, "%s", buff);

        if (time_diff(&timer) >= LOGGER_SAVE_PERIOD)
        {
            // save log file every at least 2s
            fclose(mFile);
            mFile = NULL;

            init("a+"); // reopen

            clock_gettime(CLOCK_MONOTONIC, &timer);
        }        
    }

    done = 1;
    return arg;
}
/********************************************************************************/
int logger_stop()
{
    stop = 1;
    while (!done);

    fclose(mFile);
    mFile = NULL;

    return 1;
}
/********************************************************************************/
int logger_postMsg(char *from, char *message)
{
    char currTime[16]; // hh:mm:ss
    curr_time(currTime);

    char buff[LOG_ENTRY_MAX];
    snprintf(buff, LOG_ENTRY_MAX, "%s [%16.16s] %.64s\n", currTime, from, message);

    queue_push(q, buff);

    return 1;
}
/********************************************************************************/
int init(const char *flags)
{
    if (mFile != NULL)
        return 0;

    struct stat st = {0};
    if (stat(directory, &st) == -1) // check if directory exists
    {
        mkdir(directory, 0700);
    }

    char filePath[256];
    sprintf(filePath, "%s/%s", directory, fileName); // concatenate directory and file path
    
    stop = 0; 
    done = 0; 

    return open_log_file(filePath, flags);
}
/********************************************************************************/
int open_log_file(const char *path, const char *flags)
{
    mFile = fopen(path, flags);

    if (mFile == NULL)
    {
        fprintf(stderr, "Logger error: cannot open %s/%s file\n", directory, fileName);
        return 0;
    }  

    return 1;
}
/********************************************************************************/
void curr_time(char *buff)
{
    time_t rawTime;
    struct tm *timeInfo;

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    sprintf(buff, "[%02d:%02d:%02d]", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
}
/********************************************************************************/
double time_diff(struct timespec *start)
{
    struct timespec finish;
    clock_gettime(CLOCK_MONOTONIC, &finish);

    double elapsed = (double)(finish.tv_sec - start->tv_sec);
    elapsed += (double)(finish.tv_nsec - start->tv_nsec) / 1000000000.0;

    return elapsed;
}
