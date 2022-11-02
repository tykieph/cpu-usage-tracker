#include "Logger.h"

#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

/********************************************************************************/
#define LOGGER_SAVE_PERIOD 50 // save every 50 output lines

static const char *fileName = "CUT_debug.log";
static const char *directory = "../tmp";
static FILE *mFile = NULL;
static unsigned int mCount = 0;

static pthread_mutex_t mtxLog;

/********************************************************************************/
static void curr_time(char *buff);
static int open_log_file(const char *path, const char *flags);
static void save_log_file(void);
static int init(const char *flags);
/********************************************************************************/
int logger_init()
{
    pthread_mutex_init(&mtxLog, NULL);
    return init("w");
}
/********************************************************************************/
int logger_stop()
{
    fclose(mFile);
    mFile = NULL;

    return 0;
}
/********************************************************************************/
int logger_logMsg(char *from, char *message)
{
    pthread_mutex_lock(&mtxLog);
    if (mFile)
    {
        if ((++mCount) >= LOGGER_SAVE_PERIOD)
        {
            save_log_file();
            mCount = 0;
        }

        char currTime[16]; // hh:mm:ss
        curr_time(currTime);

        fprintf(mFile, "%s [%16.16s] %.64s\n", currTime, from, message);
    }
    pthread_mutex_unlock(&mtxLog);

    return 0;
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
int open_log_file(const char *path, const char *flags)
{
    mFile = fopen(path, flags);

    if (mFile == NULL)
    {
        fprintf(stderr, "Logger error: cannot open %s/%s file\n", directory, fileName);
        return 1;
    }  

    return 0;
}
/********************************************************************************/
void save_log_file()
{
    logger_stop();
    init("a+");
}
/********************************************************************************/
int init(const char *flags)
{
    if (mFile != NULL)
        return 1;

    struct stat st = {0};
    if (stat(directory, &st) == -1)
    {
        mkdir(directory, 0700);
    }

    char filePath[256];
    sprintf(filePath, "%s/%s", directory, fileName);
    
    return open_log_file(filePath, flags);
}
/********************************************************************************/