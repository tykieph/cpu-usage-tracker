#include "Reader.h"

typedef struct
{
    char** data;
    size_t rows, cols;  
} Output;

FILE *mFile;
size_t mHardwareConcurrency;
Output mBuffer;
/********************************************************************************/
int read_proc_stat(char ***buffer, size_t rows, size_t cols);
void alloc_buffer(char ***buffer, size_t rows, size_t cols);
void dealloc_buffer(char ***buffer, size_t rows);

/********************************************************************************/
void get_buffer(char ***buffer, size_t *rows)
{
    read_proc_stat(&mBuffer.data, mBuffer.rows, mBuffer.cols);

    *buffer = mBuffer.data;
    *rows = mBuffer.rows;
}
/********************************************************************************/
void alloc_buffer(char ***buffer, size_t rows, size_t cols)
{
    *buffer = (char**)malloc(rows * sizeof(char*));
    for (size_t i = 0; i < rows; i++)
        (*buffer)[i] = (char*)malloc(cols * sizeof(char));    
}
/********************************************************************************/
void dealloc_buffer(char ***buffer, size_t rows)
{
    for (size_t i = 0; i < rows; i++)
        free((*buffer)[i]);
    free(*buffer);    
}
/********************************************************************************/
int open_proc_stat()
{
    mFile = fopen("/proc/stat", "r");

    if (mFile == NULL)
    {
        printf("ERROR: cannot open /proc/stat file\n");
        return 1;
    }

    mBuffer.rows = sysconf(_SC_NPROCESSORS_ONLN) + 1;
    mBuffer.cols = 256;
    alloc_buffer(&mBuffer.data, mBuffer.rows, mBuffer.cols);

    return 0;    
}
/********************************************************************************/
int read_proc_stat(char ***buffer, size_t rows, size_t cols)
{
    fseek(mFile, 0, SEEK_SET);
    fflush(mFile);

    // read first (rows) lines of the /proc/stat file into Buffer
    for (size_t i = 0; i < rows; i++)
    {
        if (fgets((*buffer)[i], cols, mFile) == NULL)
        {
            printf("ERROR: an error occured while reading /proc/stat file\n");
            return 1;
        }
    }

    return 0;
}
/********************************************************************************/
int close_proc_stat()
{
    fclose(mFile);
    dealloc_buffer(&mBuffer.data, mBuffer.rows);

    return 0;
}
/********************************************************************************/