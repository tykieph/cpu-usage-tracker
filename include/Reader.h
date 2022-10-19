#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



int open_proc_stat();
void get_buffer(char ***buffer, size_t *rows);
int close_proc_stat();