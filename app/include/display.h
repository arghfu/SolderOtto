#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_TASK_PRIORITY 12
#define DISPLAY_TASK_STACK_SIZE 1024

int display_init(void);

void display_run();

int display_calibrate(void);

#endif // DISPLAY_H