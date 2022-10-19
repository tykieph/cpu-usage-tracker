#include "Reader.h"

FILE *File;

int open_proc_stat()
{
    File = fopen("/proc/stat", "r");

    if (File == NULL)
    {
        printf("ERROR: cannot open /proc/stat file\n");
        return 1;
    }

    return 0;    
}

int read_proc_stat(char *Buffer, const int n)
{
    if (fgets(Buffer, n, File) == NULL)
    {
        return 1;
    }    

    return 0;
}

int close_proc_stat()
{
    fclose(File);

    return 0;
}