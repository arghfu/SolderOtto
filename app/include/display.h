#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_TASK_PRIORITY 7
#define DISPLAY_TASK_STACK_SIZE 1024


int display_init(void);

int display_run();

int display_calibrate(void);

#endif