#include "Printer.h"

/********************************************************************************/
void print_cpus_usage(float **data, const size_t n)
{
    for (size_t i = 0; i < n; i++)
        printf("| cpu%2lu |", i);
    putchar('\n');

    for (size_t i = 0; i < n; i++)
        printf("|=======|");    
    putchar('\n');

    for (size_t i = 0; i < n; i++)
        printf("|%5.2f%% |", (*data)[i] * 100.0f);
    putchar('\n');

    for (size_t i = 0; i < n; i++)
        printf("=========");    
    putchar('\n');    
}
/********************************************************************************/