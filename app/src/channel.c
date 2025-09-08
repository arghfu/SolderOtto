#include "channel.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(channel);

// Debounce delay in milliseconds
#define DEBOUNCE_DELAY_MS 20

struct debounce_ctx
{
    struct k_work_delayable dwork;
    struct gpio_dt_spec* gpio;
};

static struct debounce_ctx tip_ctx;
static struct debounce_ctx stand_ctx;

static struct gpio_callback tip_cb_data;
static struct gpio_callback stand_cb_data;

// Store last pin states
static int last_tip_state = -1;
static int last_stand_state = -1;

static void tip_debounce_handler(struct k_work* work);
static void stand_debounce_handler(struct k_work* work);

static void tip_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins);

static void stand_callback(const struct device* dev,
                           struct gpio_callback* cb, uint32_t pins);

int channel_init(struct channel* self)
{
    self->state = CHANNEL_DISCONNECTED;
    self->type = CHANNEL_TYPE_NONE;
    self->enabled = false;

    self->adc_dev = DEVICE_DT_GET(DT_ALIAS(adc_1));

    self->load_switches[0] = (struct gpio_dt_spec)GPIO_DT_SPEC_GET(DT_NODELABEL(load0), gpios);
    self->load_switches[1] = (struct gpio_dt_spec)GPIO_DT_SPEC_GET(DT_NODELABEL(load1), gpios);

    self->tip = (struct channel_tip){
        .buffer = {0, 0},
        .sequence = {
            .buffer = self->tip.buffer,
            .buffer_size = sizeof(self->tip.buffer),
            .resolution = 12,
            .oversampling = 5,
        },
        .adc_cfg = {
            ADC_CHANNEL_CFG_DT(DT_CHILD(DT_ALIAS(adc_1), channel_8)),
            ADC_CHANNEL_CFG_DT(DT_CHILD(DT_ALIAS(adc_1), channel_9))
        },
        .select = {
            {GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel_1a), gpios),
             GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel_2a), gpios)
            },
            {GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel_1b), gpios),
             GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel_2b), gpios)
            },
        },
        .config = MEASURE_CONFIG_TIP,
    };

    self->tip.sequence.channels = 0;
    for (size_t i = 0U; i < CHANNEL_TIPS_CNT; i++)
    {
        self->tip.sequence.channels |= BIT(self->tip.adc_cfg[i].channel_id);
        int err = adc_channel_setup(self->adc_dev, &self->tip.adc_cfg[i]);

        if (err < 0)
        {
            printf("Could not setup channel #%d (%d)\n", i, err);
            return 0;
        }

        self->filter[i] = (moving_average_t){0};
        moving_average_init(&self->filter[i], 200);

        self->pid[i] = (struct pid){0};
        pid_init(&self->pid[i], 0, 0, 0, -5000, 5000, -500, 500);
        pid_set_time_function(&self->pid[i], k_cycle_get_64);
        pid_set_setpoint(&self->pid[i], 350);

        gpio_pin_configure_dt(&self->tip.select[i].a,GPIO_OUTPUT_LOW);
        gpio_pin_configure_dt(&self->tip.select[i].b, GPIO_OUTPUT_LOW);

        gpio_pin_configure_dt(&self->load_switches[i], GPIO_OUTPUT_LOW);
    }

    self->tip_change = (struct gpio_dt_spec)GPIO_DT_SPEC_GET(DT_ALIAS(ch0_tip), gpios);
    self->stand = (struct gpio_dt_spec)GPIO_DT_SPEC_GET(DT_ALIAS(ch0_stand), gpios);

    gpio_pin_configure_dt(&self->tip_change, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&self->tip_change, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&tip_cb_data, tip_callback, BIT(self->tip_change.pin));
    gpio_add_callback_dt(&self->tip_change, &tip_cb_data);

    gpio_pin_configure_dt(&self->stand, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&self->stand, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&stand_cb_data, stand_callback, BIT(self->stand.pin));
    gpio_add_callback_dt(&self->stand, &stand_cb_data);

    // Initialize debounce work items
    k_work_init_delayable(&tip_ctx.dwork, tip_debounce_handler);
    tip_ctx.gpio = &self->tip_change;
    k_work_init_delayable(&stand_ctx.dwork, stand_debounce_handler);
    stand_ctx.gpio = &self->stand;

    self->id = (struct adc_dt_spec)ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_handle);
    return 0;
}

int channel_detect(struct channel* self)
{
    // channel_read_tip(self);
    // self->tip_data[0].filtered = moving_average_add_value(&self->filter[0], self->tip_data[0].filtered);
    // self->tip_data[1].filtered = moving_average_add_value(&self->filter[1], self->tip_data[1].filtered);
    //
    // channel_set_mesasure(self, TIP_A, MEASURE_CONFIG_DIFF);
    // channel_set_mesasure(self, TIP_B, MEASURE_CONFIG_DIFF);


    return 0;
}

int channel_read_tip(struct channel* self)
{
    adc_read(self->adc_dev, &self->tip.sequence);

    for (size_t i = 0U; i < CHANNEL_TIPS_CNT; i++)
    {

        self->tip_data[i].mv = (int32_t)self->tip.buffer[i];

        adc_raw_to_millivolts(adc_ref_internal(self->adc_dev),
                              self->tip.adc_cfg[0].gain,
                              12, &self->tip_data[i].mv);

        self->tip_data[i].filtered = (int32_t)moving_average_add_value(&self->filter[i],
                                                                       (uint32_t)self->tip_data[i].mv);
    }

    return 0;
}

int channel_is_enabled(struct channel* self)
{
    return self->enabled;
}

int channel_set_load(struct channel* self, enum tip tip, GPIO_PinState state)
{
    gpio_pin_set_dt(&self->load_switches[tip], state);
    return 0;
}

int channel_set_mesasure(struct channel* self, enum tip tip, enum measure_config config)
{
    self->tip.config = config;
    switch (self->tip.config)
    {
    case MEASURE_CONFIG_TIP:
        gpio_pin_set_dt(&self->tip.select[tip].a, GPIO_PIN_RESET);
        gpio_pin_set_dt(&self->tip.select[tip].b, GPIO_PIN_RESET);
        break;
    case MEASURE_CONFIG_EARTH:
        gpio_pin_set_dt(&self->tip.select[tip].a, GPIO_PIN_RESET);
        gpio_pin_set_dt(&self->tip.select[tip].b, GPIO_PIN_SET);
        break;
    case MEASURE_CONFIG_DIFF:
        gpio_pin_set_dt(&self->tip.select[tip].a, GPIO_PIN_SET);
        gpio_pin_set_dt(&self->tip.select[tip].b, GPIO_PIN_RESET);
        break;
    case MEASURE_CONFIG_TIP_INV:
        gpio_pin_set_dt(&self->tip.select[tip].a, GPIO_PIN_SET);
        gpio_pin_set_dt(&self->tip.select[tip].b, GPIO_PIN_SET);
        break;
    }
    return 0;
}


// Callback function for ZCD pin state change
static void tip_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins)
{
    // Cancel any pending work and reschedule
    k_work_cancel_delayable(&tip_ctx.dwork);
    k_work_reschedule(&tip_ctx.dwork, K_MSEC(DEBOUNCE_DELAY_MS));
}

// Callback function for ZCD pin state change
static void stand_callback(const struct device* dev,
                           struct gpio_callback* cb, uint32_t pins)
{
    // Cancel any pending work and reschedule
    k_work_cancel_delayable(&stand_ctx.dwork);
    k_work_reschedule(&stand_ctx.dwork, K_MSEC(DEBOUNCE_DELAY_MS));
}

static void tip_debounce_handler(struct k_work* work)
{
    struct k_work_delayable* dwork = k_work_delayable_from_work(work);
    struct debounce_ctx* ctx = CONTAINER_OF(dwork, struct debounce_ctx, dwork);
    struct gpio_dt_spec* gpio = ctx->gpio;

    int current_state = gpio_pin_get_dt(gpio);

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
    struct k_work_delayable* dwork = k_work_delayable_from_work(work);
    struct debounce_ctx* ctx = CONTAINER_OF(dwork, struct debounce_ctx, dwork);
    struct gpio_dt_spec* gpio = ctx->gpio;

    int current_state = gpio_pin_get_dt(gpio);

    // Only process if state is stable
    if (current_state != last_stand_state)
    {
        last_stand_state = current_state;
        LOG_INF("Stand callback - debounced state: %d", current_state);
        // Add your actual stand processing logic here
    }
}
