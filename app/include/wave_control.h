#pragma once
#include <stdint.h>

#define WAVE_CTRL_TASK_PRIORITY 0
#define WAVE_CTRL_TASK_STACK_SIZE 1024

#define TC_COMPENSATION_X2_T210 (4.223931712905644e-06)
#define TC_COMPENSATION_X1_T210 0.31863796444354214
#define TC_COMPENSATION_X0_T210 20.968033870812942

#define TC_COMPENSATION_X2_T245 (-4.735112838956741e-07)
#define TC_COMPENSATION_X1_T245 0.11936452029674384
#define TC_COMPENSATION_X0_T245 23.777399955382318

typedef struct wave_control {
    uint16_t ton;
    uint16_t tperiod;
    uint16_t count;
} wave_control_t;

int wave_control_init();