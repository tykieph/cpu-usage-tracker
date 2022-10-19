#pragma once

#include <stdio.h>
#include <stdlib.h>


int open_proc_stat();
int read_proc_stat(char *Buffer, const int n);
int close_proc_stat();