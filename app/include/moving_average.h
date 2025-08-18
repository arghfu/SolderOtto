#pragma once

#include <stdint.h>

#define MOVING_AVERAGE_MAX_LENGTH 200

typedef struct moving_average
{
    float sum;
    float history[MOVING_AVERAGE_MAX_LENGTH];
    uint32_t window_length;
    float *act_value;
} moving_average_t;

void moving_average_init(moving_average_t *ma, uint32_t window_length);
float moving_average_add_value(moving_average_t *ma, float value);