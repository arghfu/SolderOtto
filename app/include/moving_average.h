#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include <stdint.h>

#define MOVING_AVERAGE_MAX_LENGTH 200

typedef struct moving_average
{
    uint32_t sum;
    uint32_t history[MOVING_AVERAGE_MAX_LENGTH];
    uint32_t window_length;
    uint32_t *act_value;
} moving_average_t;

void moving_average_init(moving_average_t *ma, uint32_t window_length);
uint32_t moving_average_add_value(moving_average_t *ma, uint32_t value);

#endif // MOVING_AVERAGE_H