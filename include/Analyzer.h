#pragma once

#include <stdio.h>
#include <stdlib.h>


void process_data(char ***data, const size_t rows);
void get_cpus_usage(float **data, const size_t n);
void destroy_analyzer(void);
