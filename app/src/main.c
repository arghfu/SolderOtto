#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <app_version.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/flash.h>

#include "debug.h"
#include "display.h"
#include "wave_control.h"
#include "storage.h"
#include "channel.h"

#define APP_LOG_LEVEL_DBG
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define SPI_FLASH_TEST_REGION_OFFSET 0xff000

#define SPI_FLASH_SECTOR_SIZE        4096
#define SPI_FLASH_MULTI_SECTOR_TEST
#define SPI_FLASH_COMPAT st_stm32_qspi_nor

K_THREAD_STACK_DEFINE(wave_control_task_stack, WAVE_CTRL_TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(display_task_stack, DISPLAY_TASK_STACK_SIZE);
K_THREAD_STACK_DEFINE(detection_task_stack, CHANNEL_DETECTION_TASK_STACK_SIZE);

struct k_thread display_thread;
struct k_thread wave_control_thread;
struct k_thread channel_thread;



int main(void)
{
    LOG_INF("Starting Solderotto %s", APP_VERSION_STRING);






    // storage_init();
    // dbg_init();

    // wave_control_init();
    //
    // k_thread_create(&wave_control_thread, wave_control_task_stack,
    //                 K_THREAD_STACK_SIZEOF(wave_control_task_stack),
    //                 wave_control_run, NULL, NULL, NULL,
    //                 WAVE_CTRL_TASK_PRIORITY, 0, K_NO_WAIT);
    //
    // k_thread_name_set(&wave_control_thread, "wave_control");
    //
    // k_thread_create(&display_thread, display_task_stack,
    //                 K_THREAD_STACK_SIZEOF(display_task_stack),
    //                 display_run, NULL, NULL, NULL,
    //                 DISPLAY_TASK_PRIORITY, 0, K_NO_WAIT);
    //
    // k_thread_name_set(&display_thread, "display");



    k_thread_create(&channel_thread, detection_task_stack,
                    K_THREAD_STACK_SIZEOF(detection_task_stack),
                    channel_detection_run, NULL, NULL, NULL,
                    CHANNEL_DETECTION_TASK_PRIORITY, 0, K_NO_WAIT);

    k_thread_name_set(&channel_thread, "channel_detect");
    return 0;
}
