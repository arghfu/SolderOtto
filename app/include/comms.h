#ifndef COMMS_H
#define COMMS_H
#include <zephyr/kernel.h>

struct display_msg
{
    float set_point;
    float temperature;
};

extern struct k_msgq control_msgq;
#endif // COMMS_H