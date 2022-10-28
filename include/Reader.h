#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



int open_proc_stat(void);
void get_buffer(char ***buffer, size_t *rows);
void get_buffer_without_header(char ***buffer, size_t *rows);
void destroy_reader(void);
int proc_stat_closed(void);