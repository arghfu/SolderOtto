#include "moving_average.h"

#include <zephyr/sys/__assert.h>

void moving_average_init(moving_average_t* ma, uint32_t window_length)
{
    __ASSERT(window_length <= MOVING_AVERAGE_MAX_LENGTH, "Window length must be less then 200");

    for (int i = 0; i < MOVING_AVERAGE_MAX_LENGTH; ++i)
    {
        ma->history[i] = 0;
    }

    ma->act_value = &ma->history[0];
    ma->window_length = window_length;
    ma->sum = 0;
}

uint32_t moving_average_add_value(moving_average_t* ma, uint32_t value)
{
    ma->sum += value;
    ma->sum -= *ma->act_value;

    *ma->act_value = value;

    if (ma->act_value++ == &ma->history[ma->window_length - 1])
    {
        ma->act_value = &ma->history[0];
    }

    return ma->sum / ma->window_length;
}
