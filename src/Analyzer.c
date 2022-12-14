#include "Analyzer.h"

/********************************************************************************/
typedef struct
{
    ulong user;     // normal processes executing in user mode
    ulong nice;     // niced processes executing in user mode
    ulong system;   // processes executing in user mode
    ulong idle;     // twiddling thumbs
    ulong iowait;   // waiting for IO to complete
    ulong irq;      // servicing interrupts
    ulong softirq;  // servicing softirqs
    ulong steal;    // counts the ticks spent executing other virtual hosts
    ulong guest;    // counts the time spent running a virtual CPU for guest operating systems under the control of the Linux kernel
    ulong guestnice;// time spent running a niced guest (virtual CPU for guest operating systems under the control of the Linux kernel)
    float usage;    // core usage   
} CpuStat;


static CpuStat *mCpusStat;
static float *mUsage;
/********************************************************************************/
static ulong sum_cpustat_idle(const CpuStat *cpu);
static ulong sum_cpustat_non_idle(const CpuStat *cpu);
static void process_line(const char *line, CpuStat *cpu);
/********************************************************************************/
void process_line(const char *line, CpuStat *cpu)
{
    ulong prevIdle      = sum_cpustat_idle(cpu);
    ulong prevNonIdle   = sum_cpustat_non_idle(cpu);
    ulong prevTotal     = prevIdle + prevNonIdle;

    sscanf(line, 
        "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %*s", 
        &cpu->user, &cpu->nice, &cpu->system, &cpu->idle, &cpu->iowait, 
        &cpu->irq, &cpu->softirq, &cpu->steal, &cpu->guest, &cpu->guestnice);    

    ulong currIdle      = sum_cpustat_idle(cpu);
    ulong currNonIdle   = sum_cpustat_non_idle(cpu);
    ulong currTotal     = currIdle + currNonIdle;
    
    ulong totalDiff = currTotal - prevTotal;
    ulong idleDiff  = currIdle - prevIdle;

    cpu->usage = (float)(totalDiff - idleDiff) / (float)totalDiff;
}
/********************************************************************************/
void process_data(char ***data, const size_t rows)
{
    // alloc array of CpuStat structs
    if (mCpusStat == NULL) 
        mCpusStat = malloc(rows * sizeof(CpuStat));

    // alloc array of floats -> % cpu usage
    if (mUsage == NULL)
        mUsage = malloc(rows * sizeof(float));

    for (size_t i = 0; i < rows; i++)
    {
        process_line((*data)[i], &mCpusStat[i]);
    }
}
/********************************************************************************/
void get_cpus_usage(float **data, const size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        mUsage[i] = (&mCpusStat[i])->usage;
    }

    *data = mUsage;
}
/********************************************************************************/
ulong sum_cpustat_idle(const CpuStat *cpu)
{
    return cpu->idle + cpu->iowait;
}
/********************************************************************************/
ulong sum_cpustat_non_idle(const CpuStat *cpu)
{
    return 
        cpu->user + cpu->nice + cpu->system + cpu->irq + 
        cpu->softirq + cpu->steal + cpu->guest + cpu->guestnice;
}
/********************************************************************************/
void destroy_analyzer()
{
    free(mCpusStat);
    free(mUsage);
}
/********************************************************************************/

/********************************************************************************/

