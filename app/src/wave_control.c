#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "moving_average.h"
#include "wave_control.h"
#include "debug.h"

#include "pid.h"

#define ZCD_BEGIN GPIO_PIN_RESET
#define ZCD_END GPIO_PIN_SET

LOG_MODULE_REGISTER(wave_control);

static const struct adc_dt_spec adc_tip_a_temp = // NOLINT(*-interfaces-global-init)
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);


// Debounce delay in milliseconds
#define DEBOUNCE_DELAY_MS 20

static const struct gpio_dt_spec zcd = GPIO_DT_SPEC_GET(DT_ALIAS(zcd), gpios);
static const struct gpio_dt_spec tip_change = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_tip), gpios);
static const struct gpio_dt_spec stand = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_stand), gpios);

static const struct device* adc = DEVICE_DT_GET(DT_ALIAS(adc_1));

static const struct adc_channel_cfg channel_cfgs[] = {
    ADC_CHANNEL_CFG_DT(DT_CHILD(DT_ALIAS(adc_1), channel_8)),
    ADC_CHANNEL_CFG_DT(DT_CHILD(DT_ALIAS(adc_1), channel_9))
};

struct gpio_callback zcd_cb_data;
struct gpio_callback tip_cb_data;
struct gpio_callback stand_cb_data;

struct wave_control control = {
    .ton = 1,
    .tperiod = 5,
    .count = 0
};
#define CHANNEL_COUNT 2
uint16_t ana_buf[CHANNEL_COUNT] = {0, 0};

static struct adc_sequence ana_sequence = {
    .buffer = &ana_buf,
    /* buffer size in bytes, not number of samples */
    .buffer_size = sizeof(ana_buf),
    .resolution = 12,
};

static pid_t pid;
static moving_average_t avg;

static void wave_control_zcd_callback(const struct device* dev,
                                      struct gpio_callback* cb, uint32_t pins);

static void tip_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins);

static void stand_callback(const struct device* dev,
                           struct gpio_callback* cb, uint32_t pins);

// Work item for debounced processing
static struct k_work_delayable tip_debounce_work;
static struct k_work_delayable stand_debounce_work;

// Store last pin states
static int last_tip_state = -1;
static int last_stand_state = -1;

static void tip_debounce_handler(struct k_work* work);
static void stand_debounce_handler(struct k_work* work);

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

    pid_init(&pid, KP_T210, KI_T210, KD_T210, -5000, 5000, -500, 500);
    pid_set_time_function(&pid, k_cycle_get_64);
    float temp = 0.0f;
    float output = 0.0f;
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

                adc_read(adc, &ana_sequence);

                val_mv = (int32_t)ana_buf[0];

                // adc_raw_to_millivolts_dt(&adc_tip_a_temp, &val_mv);

                temp = val_mv * val_mv * TC_COMPENSATION_X2_T210 + val_mv * TC_COMPENSATION_X1_T210 +
                    TC_COMPENSATION_X0_T210;


                temp = moving_average_add_value(&avg, temp);

                output = pid_process(&pid, temp);

                uint64_t time_end = k_cycle_get_64();
                diff = k_cyc_to_us_floor64(time_end - time_begin);
                break;
            case ZCD_END:

                if (enable_output)
                {
                    dbg_pin_set(0, GPIO_PIN_RESET);
                }

                LOG_INF("Analog voltage: %"PRId32" mV", val_mv);
                // LOG_INF("Measurement time: %"PRId64" us", diff);
                // LOG_INF("Analog voltage: %f degC", temp);
                // LOG_INF("Control output: %f", output);

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

    gpio_init_callback(&zcd_cb_data, wave_control_zcd_callback, BIT(zcd.pin));
    gpio_add_callback_dt(&zcd, &zcd_cb_data);


    gpio_pin_configure_dt(&tip_change, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&tip_change, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&tip_cb_data, tip_callback, BIT(tip_change.pin));
    gpio_add_callback_dt(&tip_change, &tip_cb_data);


    gpio_pin_configure_dt(&stand, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&stand, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&stand_cb_data, stand_callback, BIT(stand.pin));
    gpio_add_callback_dt(&stand, &stand_cb_data);


    // Initialize debounce work items
    k_work_init_delayable(&tip_debounce_work, tip_debounce_handler);
    k_work_init_delayable(&stand_debounce_work, stand_debounce_handler);

    for (size_t i = 0U; i < CHANNEL_COUNT; i++)
    {
        ana_sequence.channels |= BIT(channel_cfgs[i].channel_id);
        int err = adc_channel_setup(adc, &channel_cfgs[i]);

        if (err < 0)
        {
            printf("Could not setup channel #%d (%d)\n", i, err);
            return 0;
        }


    }


    moving_average_init(&avg, 200);
    return 0;
}

// Callback function for ZCD pin state change
static void wave_control_zcd_callback(const struct device* dev,
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

// Callback function for ZCD pin state change
static void tip_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins)
{
    // Cancel any pending work and reschedule
    k_work_cancel_delayable(&tip_debounce_work);
    k_work_reschedule(&tip_debounce_work, K_MSEC(DEBOUNCE_DELAY_MS));
}

// Callback function for ZCD pin state change
static void stand_callback(const struct device* dev,
                           struct gpio_callback* cb, uint32_t pins)
{
    // Cancel any pending work and reschedule
    k_work_cancel_delayable(&stand_debounce_work);
    k_work_reschedule(&stand_debounce_work, K_MSEC(DEBOUNCE_DELAY_MS));
}

static void tip_debounce_handler(struct k_work* work)
{
    int current_state = gpio_pin_get_dt(&tip_change);

    // Only process if state is stable
    if (current_state != last_tip_state)
    {
        last_tip_state = current_state;
        LOG_INF("Tip callback - debounced state: %d", current_state);
        // Add your actual tip processing logic here
    }
}

static void stand_debounce_handler(struct k_work* work)
{
    int current_state = gpio_pin_get_dt(&stand);

    // Only process if state is stable
    if (current_state != last_stand_state)
    {
        last_stand_state = current_state;
        LOG_INF("Stand callback - debounced state: %d", current_state);
        // Add your actual stand processing logic here
    }
}
