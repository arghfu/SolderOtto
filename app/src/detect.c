#include "detect.h"

#include <stdint.h>
#include <zephyr/kernel.h>

#include "channel.h"


void channel_detect_run()
{
    // channel_init();
    uint64_t foobar = 0;
    while (1)
    {
        // channel_detect();
        k_sleep(K_MSEC(100));
    }
}

// K_THREAD_DEFINE(zcd_thread_id, WAVE_CTRL_TASK_STACK_SIZE, zcd_processing_thread,
//                 NULL, NULL, NULL, WAVE_CTRL_TASK_PRIORITY, 0, 0);
