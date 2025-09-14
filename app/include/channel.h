#ifndef CHANNEL_H
#define CHANNEL_H

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include "pid.h"
#include "moving_average.h"

#define CHANNEL_TIPS_CNT 2
#define CHANNEL_LOAD_CNT 2

enum channel_type
{
    CHANNEL_TYPE_DISCONNECTED = 0,
    CHANNEL_TYPE_T210,
    CHANNEL_TYPE_T245,
    CHANNEL_TYPE_AM120,
    CHANNEL_TYPE_NONE,
};

enum tip
{
    TIP_A = 0,
    TIP_B = 1,
};

enum measure_config
{
    MEASURE_CONFIG_TIP = 0,
    MEASURE_CONFIG_EARTH,
    MEASURE_CONFIG_TIP_INV,
    MEASURE_CONFIG_DIFF,
};

struct tip_data
{
    int32_t mv;
    int32_t filtered;
    float temp;
    struct moving_average filter;
};

struct channel_tip
{
    struct
    {
        struct gpio_dt_spec a;
        struct gpio_dt_spec b;
    } select[CHANNEL_TIPS_CNT];

    enum measure_config config;
    uint16_t buffer[CHANNEL_TIPS_CNT];
    struct adc_sequence sequence;
    struct adc_channel_cfg adc_cfg[CHANNEL_TIPS_CNT];
};

struct channel_load_current
{
    uint16_t buffer[CHANNEL_LOAD_CNT];
    struct adc_sequence sequence;
    struct adc_channel_cfg adc_cfg[CHANNEL_LOAD_CNT];
};

struct channel_handle_id
{
    uint16_t buffer;
    struct adc_sequence sequence;
    struct adc_channel_cfg adc_cfg;
    struct moving_average filter;
};

struct channel_interrupt
{
    struct k_work_delayable dwork;
    struct gpio_callback cb;
    struct gpio_dt_spec pin;
    int last_pin_state;
};

typedef struct channel
{
    bool enabled;

    enum channel_type type;

    const struct device* adc_dev;
    struct channel_interrupt tip_change;
    struct channel_interrupt stand;

    struct channel_handle_id handle_id;
    struct channel_tip tip;
    struct channel_load_current load;

    struct gpio_dt_spec load_switches[CHANNEL_TIPS_CNT];
    struct tip_data tip_data[CHANNEL_TIPS_CNT];
    struct pid pid[CHANNEL_TIPS_CNT];
} solder_channel_t;

int channel_init(struct channel* self);
int channel_detect(struct channel* self);
int channel_read_tip(struct channel* self);
int channel_is_enabled(struct channel* self);
int channel_set_mesasure(struct channel* self, enum tip tip, enum measure_config config);
int channel_set_load(struct channel* self, enum tip tip, GPIO_PinState state);

#endif // CHANNEL_H
