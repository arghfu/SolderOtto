#ifndef CHANNEL_H
#define CHANNEL_H

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include "pid.h"
#include "moving_average.h"

#define CHANNEL_TIPS_CNT 2
#define CHANNEL_LOAD_CNT 2

typedef enum connection_state
{
    CHANNEL_DISCONNECTED = 0,
    CHANNEL_CONNECTED,
    CHANNEL_UNKNOWN,
} connection_state_t;

typedef enum channel_type
{
    CHANNEL_TYPE_NONE = 0,
    CHANNEL_TYPE_T210,
    CHANNEL_TYPE_T245,
    CHANNEL_TYPE_AM120,
} channel_type_t;

typedef struct tip_data
{
    int32_t mv;
    int32_t filtered;
    float temp;
} tip_data_t;

typedef struct channel_tip
{
    uint16_t buffer[CHANNEL_TIPS_CNT];
    struct adc_sequence sequence;
    struct adc_channel_cfg adc_cfg[CHANNEL_TIPS_CNT];
} channel_tip_t;

typedef struct channel_load
{
    uint16_t buffer[CHANNEL_LOAD_CNT];
    struct adc_sequence sequence;
    struct adc_channel_cfg adc_cfg[CHANNEL_LOAD_CNT];
} channel_load_t;

typedef struct channel
{
    channel_tip_t tip;
    tip_data_t tip_data[CHANNEL_TIPS_CNT];
    channel_load_t load;

    const struct device* adc_dev;

    connection_state_t state;
    channel_type_t type;

    moving_average_t filter[CHANNEL_TIPS_CNT];
    struct pid pid[CHANNEL_TIPS_CNT];

    struct
    struct gpio_dt_spec tip_change;
    struct gpio_dt_spec stand;
    bool enabled;
} solder_channel_t;

int channel_init(struct channel* self, struct adc_channel_cfg* channel_cfg);
int channel_detect(struct channel* self);
int channel_read_tip(struct channel* self);
int channel_is_enabled(struct channel* self);

#endif // CHANNEL_H
