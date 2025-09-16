#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "channel.h"
#include "storage.h"

LOG_MODULE_REGISTER(channel, CONFIG_APP_LOG_LEVEL);

BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(channel0), handle_io_channels) == 1, "tip-io-channels must have 2 elements");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(channel0), tip_io_channels) == 2, "tip-io-channels must have 2 elements");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(channel0), load_switch_gpios) == 2, "tip-io-channels must have 2 elements");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(channel0), tip_select_a_gpios) == 2, "tip-io-channels must have 2 elements");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(channel0), tip_select_b_gpios) == 2, "tip-io-channels must have 2 elements");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(channel0), tip_change_gpios) == 1, "tip-io-channels must have 2 elements");
BUILD_ASSERT(DT_PROP_LEN(DT_NODELABEL(channel0), stand_idle_gpios) == 1, "tip-io-channels must have 2 elements");

// Debounce delay in milliseconds
#define DEBOUNCE_DELAY_MS 20

const char* channel_type_str(enum channel_type t);

static void tip_debounce_handler(struct k_work* work);
static void stand_debounce_handler(struct k_work* work);

static void tip_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins);

static void stand_callback(const struct device* dev,
                           struct gpio_callback* cb, uint32_t pins);

int channel_init(struct channel* self)
{
    self->active_tip_cnt = 0;
    self->type = CHANNEL_TYPE_DISCONNECTED;

    self->adc_dev = DEVICE_DT_GET(DT_PHANDLE(DT_NODELABEL(channel0), adc));

    self->load_switches[0] = (struct gpio_dt_spec)GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(channel0), load_switch_gpios, 0);
    self->load_switches[1] = (struct gpio_dt_spec)GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(channel0), load_switch_gpios, 1);

    self->tip = (struct channel_tip)
    {
        .buffer = {0, 0},
        .sequence = {
            .buffer = self->tip.buffer,
            .buffer_size = sizeof(self->tip.buffer),
            .resolution = 12,
            .oversampling = 5,
            .channels = 0,
        },
        // TODO change the way getting the adc channels
        .adc_cfg = {
            ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc1), channel_8)),
            ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc1), channel_9))
        },
        .select = {
            {
                GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(channel0), tip_select_a_gpios, 0),
                GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(channel0), tip_select_b_gpios, 0)
            },
            {
                GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(channel0), tip_select_a_gpios, 1),
                GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(channel0), tip_select_b_gpios, 1)
            },
        },
        .config = MEASURE_CONFIG_TIP,
    };

    for (size_t i = 0U; i < CHANNEL_TIPS_CNT; i++)
    {
        self->tip.sequence.channels |= BIT(self->tip.adc_cfg[i].channel_id);
        int err = adc_channel_setup(self->adc_dev, &self->tip.adc_cfg[i]);

        if (err < 0)
        {
            printf("Could not setup channel #%d (%d)\n", i, err);
            return 0;
        }

        self->tip_data[i] = (struct tip_data){0};
        moving_average_init(&self->tip_data[i].filter, 200);

        self->pid[i] = (struct pid){0};
        pid_init(&self->pid[i], 0, 0, 0, 0, 5000, -500, 500);
        pid_set_time_function(&self->pid[i], k_cycle_get_64);
        pid_set_setpoint(&self->pid[i], 350);

        gpio_pin_configure_dt(&self->tip.select[i].a,GPIO_OUTPUT_LOW);
        gpio_pin_configure_dt(&self->tip.select[i].b, GPIO_OUTPUT_LOW);

        gpio_pin_configure_dt(&self->load_switches[i], GPIO_OUTPUT_LOW);
    }

    self->tip_change = (struct channel_interrupt){
        .pin = (struct gpio_dt_spec)GPIO_DT_SPEC_GET(DT_NODELABEL(channel0), tip_change_gpios),
        .last_pin_state = -1,
    };

    gpio_pin_configure_dt(&self->tip_change.pin, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&self->tip_change.pin, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&self->tip_change.cb, tip_callback, BIT(self->tip_change.pin.pin));
    gpio_add_callback_dt(&self->tip_change.pin, &self->tip_change.cb);

    self->stand = (struct channel_interrupt)
    {
        .pin = (struct gpio_dt_spec)GPIO_DT_SPEC_GET(DT_NODELABEL(channel0), stand_idle_gpios),
        .last_pin_state = -1,
    };

    gpio_pin_configure_dt(&self->stand.pin, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&self->stand.pin, GPIO_INT_EDGE_BOTH);

    gpio_init_callback(&self->stand.cb, stand_callback, BIT(self->stand.pin.pin));
    gpio_add_callback_dt(&self->stand.pin, &self->stand.cb);

    // Initialize debounce work items
    k_work_init_delayable(&self->tip_change.dwork, tip_debounce_handler);
    k_work_init_delayable(&self->stand.dwork, stand_debounce_handler);

    self->handle_id = (struct channel_handle_id){
        .buffer = 0,
        .sequence =
        {
            .buffer = &self->handle_id.buffer,
            .buffer_size = sizeof(self->handle_id.buffer),
            .resolution = 12,
        },
        .adc_cfg = ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc1), channel_1)),
        .filter = {0},
    };

    moving_average_init(&self->handle_id.filter, 100);
    self->handle_id.sequence.channels = BIT(self->handle_id.adc_cfg.channel_id);
    adc_channel_setup(self->adc_dev, &self->handle_id.adc_cfg);

    return 0;
}

int channel_detect(struct channel* self)
{
    int err = adc_read(self->adc_dev, &self->handle_id.sequence);
    if (err < 0)
    {
        LOG_ERR("Could not read handle id");
        return -1;
    }
    int32_t mv = self->handle_id.buffer;

    adc_raw_to_millivolts(adc_ref_internal(self->adc_dev),
                          self->handle_id.adc_cfg.gain,
                          12, &mv);

    mv = (int32_t)moving_average_add_value(&self->handle_id.filter, (uint32_t)mv);

    if (mv > 280 && mv < 350)
    {
        self->type = CHANNEL_TYPE_T210;
        self->active_tip_cnt = 1;
    }
    else if (mv > 3265)
    {
        self->type = CHANNEL_TYPE_T245;
        self->active_tip_cnt = 1;
    }
    else if (mv > 2780 && mv < 2960)
    {
        self->type = CHANNEL_TYPE_AM120;
        self->active_tip_cnt = 2;
    }
    else
    {
        self->type = CHANNEL_TYPE_DISCONNECTED;
        self->active_tip_cnt = 0;
    }

    LOG_DBG("Tip id voltage: %d", mv);
    LOG_INF("Tip type: %s", channel_type_str(self->type));

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

        self->tip_data[i].filtered = (int32_t)moving_average_add_value(&self->tip_data[i].filter,
                                                                       (uint32_t)self->tip_data[i].mv);
    }

    return 0;
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

static void tip_callback(const struct device* dev,
                         struct gpio_callback* cb, uint32_t pins)
{
    struct channel_interrupt* ctx = CONTAINER_OF(cb, struct channel_interrupt, cb);
    // Cancel any pending work and reschedule
    k_work_cancel_delayable(&ctx->dwork);
    k_work_reschedule(&ctx->dwork, K_MSEC(DEBOUNCE_DELAY_MS));
}

static void stand_callback(const struct device* dev,
                           struct gpio_callback* cb, uint32_t pins)
{
    struct channel_interrupt* ctx = CONTAINER_OF(cb, struct channel_interrupt, cb);
    // Cancel any pending work and reschedule
    k_work_cancel_delayable(&ctx->dwork);
    k_work_reschedule(&ctx->dwork, K_MSEC(DEBOUNCE_DELAY_MS));
}

const char* channel_type_str(enum channel_type t)
{
    switch (t)
    {
    case CHANNEL_TYPE_DISCONNECTED:
        return "DISCONNECTED";
    case CHANNEL_TYPE_T210:
        return "T210";
    case CHANNEL_TYPE_T245:
        return "T245";
    case CHANNEL_TYPE_AM120:
        return "AM120";
    default:
        return "UNKNOWN";
    }
}

static void tip_debounce_handler(struct k_work* work)
{
    struct k_work_delayable* dwork = k_work_delayable_from_work(work);
    struct channel_interrupt* ctx = CONTAINER_OF(dwork, struct channel_interrupt, dwork);
    struct gpio_dt_spec* gpio = &ctx->pin;

    int current_state = gpio_pin_get_dt(gpio);

    // Only process if state is stable
    if (current_state != ctx->last_pin_state)
    {
        ctx->last_pin_state = current_state;
        LOG_INF("Tip callback - debounced state: %d", current_state);
        // Add your actual tip processing logic here
    }
}

static void stand_debounce_handler(struct k_work* work)
{
    struct k_work_delayable* dwork = k_work_delayable_from_work(work);
    struct channel_interrupt* ctx = CONTAINER_OF(dwork, struct channel_interrupt, dwork);
    struct gpio_dt_spec* gpio = &ctx->pin;

    int current_state = gpio_pin_get_dt(gpio);

    // Only process if state is stable
    if (current_state != ctx->last_pin_state)
    {
        ctx->last_pin_state = current_state;
        LOG_INF("Stand callback - debounced state: %d", current_state);
        // Add your actual stand processing logic here
    }
}
