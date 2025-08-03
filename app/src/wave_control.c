#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include "wave_control.h"
#include "debug.h"

#include <zephyr/logging/log.h>

#define ZCD_BEGIN GPIO_PIN_RESET
#define ZCD_END GPIO_PIN_SET

LOG_MODULE_REGISTER(wave_control);

static const struct gpio_dt_spec zcd = // NOLINT(*-interfaces-global-init)
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(zcd0), gpios, {0});

static const struct adc_dt_spec adc_tip_a_temp = // NOLINT(*-interfaces-global-init)
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);

static const struct adc_dt_spec adc_tip_b_temp = // NOLINT(*-interfaces-global-init)
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipb);

struct gpio_callback zcd_cb_data;

struct wave_control control = {
    .ton = 1,
    .tperiod = 5,
    .count = 0
};

uint16_t ana_buf;

struct adc_sequence ana_sequence = {
    .buffer = &ana_buf,
    /* buffer size in bytes, not number of samples */
    .buffer_size = sizeof(ana_buf),
};

static void zcd_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins);

static int wave_control_init_tip(struct adc_dt_spec* adc_spec);

// Define message structure
struct zcd_event
{
    int pin_state;
};

// Create message queue
K_MSGQ_DEFINE(zcd_msgq, sizeof(struct zcd_event), 10, 4);

// High priority thread
void zcd_processing_thread(void)
{
    struct zcd_event event;
    uint64_t diff = 0;
    int32_t val_mv = 0;
    bool enable_output = false;

    while (1)
    {
        // Wait for message from ISR
        if (k_msgq_get(&zcd_msgq, &event, K_FOREVER) == 0)
        {
            switch (event.pin_state)
            {
            case ZCD_BEGIN:
                dbg_pin_set(0, GPIO_PIN_SET);
                if (control.count < control.ton)
                {
                    enable_output = true;
                }
                ++control.count;

                if (control.count >= control.tperiod)
                {
                    control.count = 0;
                }

                uint64_t time_begin = k_cycle_get_64();
                adc_read_dt(&adc_tip_a_temp, &ana_sequence);

                val_mv = (int32_t)ana_buf;

                adc_raw_to_millivolts_dt(&adc_tip_a_temp, &val_mv);
                uint64_t time_end = k_cycle_get_64();

                diff = k_cyc_to_us_floor64(time_end - time_begin);
                break;
            case ZCD_END:

                if (enable_output)
                {
                    dbg_pin_set(0, GPIO_PIN_RESET);
                }

                LOG_INF("Analog voltage: %"PRId32" mV    Measurement time: %"PRId64" us", val_mv, diff);

                break;
            default:
                break;
            }


        }
    }
}

// Create high priority thread
K_THREAD_DEFINE(zcd_thread_id, WAVE_CTRL_TASK_STACK_SIZE, zcd_processing_thread,
                NULL, NULL, NULL, WAVE_CTRL_TASK_PRIORITY, 0, 0);


int wave_control_init()
{
    LOG_INF("Initializing...");
    // configure button pin as input
    gpio_pin_configure_dt(&zcd, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&zcd, GPIO_INT_EDGE_BOTH);

    int err = wave_control_init_tip(&adc_tip_a_temp);
    err |= wave_control_init_tip(&adc_tip_b_temp);

    gpio_init_callback(&zcd_cb_data, zcd_callback, BIT(zcd.pin));
    gpio_add_callback_dt(&zcd, &zcd_cb_data);

    /* Configure channel individually prior to sampling. */
    if (!adc_is_ready_dt(&adc_tip_a_temp))
    {
        printk("ADC controller device %s not ready\n", adc_tip_a_temp.dev->name);
        return 0;
    }

    err = adc_channel_setup_dt(&adc_tip_a_temp);
    if (err < 0)
    {
        printk("Could not setup channel Ch0-tip (%d)\n", err);
        return 0;
    }


    (void)adc_sequence_init_dt(&adc_tip_a_temp, &ana_sequence);
    // (void)adc_sequence_init_dt(&adc_tip_b_temp, &ana_sequence);

    return err;
}

// Modified ISR callback
static void zcd_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins)
{
    static uint64_t time_last = 0;
    struct zcd_event event;

    int state = gpio_pin_get_dt(&zcd);
    uint64_t time_now = k_cycle_get_64();
    uint64_t diff = time_now - time_last;
    time_last = time_now;

    event.pin_state = state;

    // Send to thread (non-blocking from ISR)
    k_msgq_put(&zcd_msgq, &event, K_NO_WAIT);
}

static int wave_control_init_tip(struct adc_dt_spec* adc_spec)
{
    int err = adc_is_ready_dt(adc_spec);
    if (err < 0)
    {
        LOG_ERR("ADC controller device %s not ready", adc_spec->dev->name);
        return 0;
    }

    err = adc_channel_setup_dt(adc_spec);
    if (err < 0)
    {
        LOG_ERR("Could not setup channel (%d)", err);
        return 0;
    }

    return 1;
}
