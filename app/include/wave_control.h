#pragma once
#include <stdint.h>

#define WAVE_CTRL_TASK_PRIORITY 0
#define WAVE_CTRL_TASK_STACK_SIZE 1024

struct wave_control {
    uint16_t ton;
    uint16_t tperiod;
    uint16_t count;
};

int wave_control_init();