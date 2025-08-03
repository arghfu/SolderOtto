#pragma once
#include <stdint.h>

struct wave_control {
    uint16_t ton;
    uint16_t tperiod;
    uint16_t count;
};

int wave_control_init();