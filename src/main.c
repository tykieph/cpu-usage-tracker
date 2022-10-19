#include <stdio.h>

#include "Reader.h"

int main(int argc, char *argv[])
{
    open_proc_stat();

    const int BuffSize = 256;
    char Buff[BuffSize];

    while (!read_proc_stat(Buff, BuffSize))
        printf("%s", Buff);

    close_proc_stat();

    return 0;
}