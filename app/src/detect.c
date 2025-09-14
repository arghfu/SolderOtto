#include "detect.h"

#include <stdint.h>
#include <zephyr/kernel.h>

#include "channel.h"


void channel_detect_run()
{
    while (1)
    {
        // channel_detect();
        k_sleep(K_MSEC(100));
    }
}
