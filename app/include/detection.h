//
// Created by arghfu on 16.09.25.
//

#ifndef APP_DETECTION_H
#define APP_DETECTION_H


#define DETECTION_TASK_PRIORITY 11
#define DETECTION_TASK_STACK_SIZE 1024

int detection_init(void);
void detection_run();

#endif //APP_DETECTION_H