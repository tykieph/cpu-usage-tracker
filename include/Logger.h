#pragma once

#include <stdio.h>


/* initialize logger */
int logger_init(void);

/* start executing main loop */
void *logger_start(void *arg);

/* tell logger to stop executing main loop */
int logger_stop(void);

/* post log entry onto the queue */
int logger_postMsg(char *from, char *message);
