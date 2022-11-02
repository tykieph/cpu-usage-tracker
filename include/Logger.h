#pragma once


#include <stdio.h>


int logger_init(void);
int logger_stop(void);

// from maximum length is 16, and message maximum length is 64
int logger_logMsg(char *from, char *message);
