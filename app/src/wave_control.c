#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include "wave_control.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wave_control);

static const struct gpio_dt_spec zcd =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(zcd0), gpios, {0});


static const struct adc_dt_spec adc_tip_a_temp =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);

static const struct adc_dt_spec adc_tip_b_temp =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipb);

struct gpio_callback zcd_cb_data;

struct wave_control control = {
    .ton = 0,
    .tperiod = 1000,
    .count = 0
};

void zcd_callback(const struct device* dev,
                  struct gpio_callback* cb, uint32_t pins);

int wave_control_init()
{
    LOG_INF("Initializing...");
    // configure button pin as input
    gpio_pin_configure_dt(&zcd, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&zcd, GPIO_INT_EDGE_BOTH);

    int err = adc_is_ready_dt(&adc_v_analog);

    if (err < 0)
    {
        LOG_ERR("ADC controller device %s not ready", adc_v_analog.dev->name);
        return 0;
    }

    err = adc_channel_setup_dt(&adc_v_analog);
    if (err < 0)
    {
        LOG_ERR("Could not setup channel (%d)", err);
        return 0;
    }


    return 0;
}

// int wave_control_add_zcd_callback(gpio_callback_handler_t handler, void *user_data)
// {
//     // gpio_init_callback(&zcd_cb_data, zcd_callback, BIT(zcd.pin));
//     // gpio_add_callback_dt(&zcd, &zcd_cb_data);
// }

void zcd_callback(const struct device* dev,
                  struct gpio_callback* cb, uint32_t pins)
{
    static uint64_t time_last = 0;
    int state = gpio_pin_get_dt(&zcd);
    uint64_t time_now = k_cycle_get_64();

    uint64_t diff = time_now - time_last;
    time_last = time_now;

    if (state == GPIO_PIN_RESET)
    {
        if (control.count < control.ton)
        {
            // gpio_pin_set_dt(&dbg0_pin, GPIO_PIN_SET);
            // gpio_pin_set_dt(&load0_switch, GPIO_PIN_SET);
        }
        else
        {
            // gpio_pin_set_dt(&dbg0_pin, GPIO_PIN_RESET);
            // gpio_pin_set_dt(&load0_switch, GPIO_PIN_RESET);
        }
        ++control.count;
    }
    else
    {
        // gpio_pin_set_dt(&dbg0_pin, GPIO_PIN_RESET);
    }


    if (control.count >= control.tperiod)
    {
        control.count = 0;
    }

    diff = k_cyc_to_us_floor64(diff);

    LOG_DBG("half wave time: %"PRId64" us", diff);
}
