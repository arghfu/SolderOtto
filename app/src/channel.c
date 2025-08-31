#include "channel.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(fooooooo);

// Debounce delay in milliseconds
#define DEBOUNCE_DELAY_MS 20

struct gpio_callback tip_cb_data;
struct gpio_callback stand_cb_data;

// Work item for debounced processing
static struct k_work_delayable tip_debounce_work;
static struct k_work_delayable stand_debounce_work;

// Store last pin states
static int last_tip_state = -1;
static int last_stand_state = -1;

static const struct gpio_dt_spec tip_change = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_tip), gpios);
static const struct gpio_dt_spec stand = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_stand), gpios);

static struct channel channel = {0};

static void tip_debounce_handler(struct k_work* work);
static void stand_debounce_handler(struct k_work* work);

static void tip_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins);

static void stand_callback(const struct device* dev,
                           struct gpio_callback* cb, uint32_t pins);


int channel_init()
{
    channel.state = CHANNEL_DISCONNECTED;
    channel.type = CHANNEL_TYPE_NONE;
    channel.enabled = false;

    channel.adc_dev = DEVICE_DT_GET(DT_ALIAS(adc_1));

    channel.tip = (channel_tip_t){
        .buffer = {0, 0},
        .sequence = {
            .buffer = channel.tip.buffer,
            .buffer_size = sizeof(channel.tip.buffer),
            .resolution = 12,
            .oversampling = 5,
        },
        .adc_cfg = {
            ADC_CHANNEL_CFG_DT(DT_CHILD(DT_ALIAS(adc_1), channel_8)),
            ADC_CHANNEL_CFG_DT(DT_CHILD(DT_ALIAS(adc_1), channel_9))
        },
    };

    for (size_t i = 0U; i < CHANNEL_TIPS_CNT; i++)
    {
        channel.tip.sequence.channels |= BIT(channel.tip.adc_cfg[i].channel_id);
        int err = adc_channel_setup(channel.adc_dev, &channel.tip.adc_cfg[i]);

        if (err < 0)
        {
            printf("Could not setup channel #%d (%d)\n", i, err);
            return 0;
        }
    }

    for (int i = 0; i < CHANNEL_TIPS_CNT; i++)
    {
        channel.filter[i] = (moving_average_t){0};
        moving_average_init(&channel.filter[i], 200);

        channel.pid[i] = (pid_t){0};
        pid_init(&channel.pid[i], 0, 0, 0, -5000, 5000, -500, 500);
    }

    gpio_pin_configure_dt(&tip_change, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&tip_change, GPIO_INT_EDGE_BOTH);

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

    return 0;
}

int channel_detect()
{
    float foo[2] = {0, 0};
    if (channel.state == CHANNEL_DISCONNECTED)
    {
        adc_read(channel.adc_dev, &channel.tip.sequence);

        for (size_t i = 0U; i < CHANNEL_TIPS_CNT; i++)
        {

            adc_raw_to_millivolts(adc_ref_internal(channel.adc_dev),
                                  channel.tip.adc_cfg[i].gain,
                                  channel.tip.sequence.resolution,
                                  (int32_t*)channel.tip.buffer[i]);

            foo[i] = moving_average_add_value(&channel.filter[i], channel.tip.buffer[i]);
        }

        // LOG_INF("Raw voltage_0 %"PRId32"", foo[0]);
        // LOG_INF("Raw voltage_1 %"PRId32"", foo[1]);
    }
    return 0;
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
