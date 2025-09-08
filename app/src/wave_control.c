#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "moving_average.h"
#include "wave_control.h"

#include "channel.h"
#include "debug.h"

#include "pid.h"

#define ZCD_BEGIN GPIO_PIN_SET
#define ZCD_END GPIO_PIN_RESET

LOG_MODULE_REGISTER(wave_control);

static struct channel solder_channel;

static struct gpio_dt_spec zcd = GPIO_DT_SPEC_GET(DT_ALIAS(zcd), gpios);

struct gpio_callback zcd_cb_data;

static struct wave_control control = {
    .ton = 0,
    .tperiod = 200,
    .count = 0
};

static void wave_control_zcd_callback(const struct device* dev,
                                      struct gpio_callback* cb, uint32_t pins);

// Define message structure
struct zcd_event
{
    int pin_state;
    uint64_t event_time;
};

// Create message queue
K_MSGQ_DEFINE(zcd_msgq, sizeof(struct zcd_event), 10, 4);

void wave_control_run(void* channel, void* p2, void* p3)
{
    struct zcd_event event;

    uint64_t diff = 0;
    bool enable_output = false;

    while (1)
    {
        // Wait for message from ISR
        if (k_msgq_get(&zcd_msgq, &event, K_FOREVER) == 0)
        {
            switch (event.pin_state)
            {
            case ZCD_BEGIN:
                channel_set_load(&solder_channel, TIP_A, GPIO_PIN_RESET);
                dbg_pin_set(0, GPIO_PIN_RESET);
                if (control.count < control.ton)
                {
                    enable_output = true;
                }
                ++control.count;

                if (control.count >= control.tperiod)
                {
                    control.count = 0;
                }

                const uint64_t time_begin = k_cycle_get_64();

                channel_read_tip(&solder_channel);

                const uint64_t time_end = k_cycle_get_64();
                diff = k_cyc_to_us_floor64(time_end - time_begin);
                break;
            case ZCD_END:

                dbg_pin_set(0, GPIO_PIN_SET);
                if (enable_output)
                {
                    channel_set_load(&solder_channel, TIP_A, GPIO_PIN_SET);
                    enable_output = false;
                }

                LOG_INF("Analog voltage_0: %"PRId32" mV", solder_channel.tip_data[0].mv);
                LOG_INF("Analog voltage_0: %"PRId32" mV", solder_channel.tip_data[1].mv);
                LOG_INF("Filtered voltage_0: %"PRId32" mV", solder_channel.tip_data[0].filtered);
                LOG_INF("Filtered voltage_1: %"PRId32" mV", solder_channel.tip_data[1].filtered);
                LOG_INF("Temperature_0: %f degC", solder_channel.tip_data[0].temp);
                LOG_INF("Temperature_1: %f degC", solder_channel.tip_data[1].temp);
                LOG_INF("Measurement time: %"PRId64" us", diff);

                break;
            default:
                break;
            }

        }
    }
}

int wave_control_init()
{
    LOG_INF("Initializing...");
    // configure button pin as input
    gpio_pin_configure_dt(&zcd, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&zcd, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&zcd_cb_data, wave_control_zcd_callback, BIT(zcd.pin));
    gpio_add_callback_dt(&zcd, &zcd_cb_data);


    channel_init(&solder_channel);

    return 0;
}

// Callback function for ZCD pin state change
static void wave_control_zcd_callback(const struct device* dev,
                                      struct gpio_callback* cb, uint32_t pins)
{
    struct zcd_event event;

    event.pin_state = gpio_pin_get_dt(&zcd);;
    event.event_time = k_cycle_get_64();
    // Send to thread (non-blocking from ISR)
    k_msgq_put(&zcd_msgq, &event, K_NO_WAIT);
}
