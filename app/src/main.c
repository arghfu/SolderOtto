#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <app_version.h>



LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define BLINK_PERIOD_MS_STEP 100U
#define BLINK_PERIOD_MS_MAX  1000U

#if !DT_NODE_EXISTS(DT_NODELABEL(load0))
#error "Overlay for power output node not properly defined."
#endif

#if !DT_NODE_EXISTS(DT_NODELABEL(load1))
#error "Overlay for power output node not properly defined."
#endif

#if !DT_NODE_EXISTS(DT_NODELABEL(zcd0))
#error "Overlay for power output node not properly defined."
#endif

static const struct gpio_dt_spec load0_switch =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(load_0), gpios, {0});

static const struct gpio_dt_spec load1_switch =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(load_1), gpios, {0});

static const struct gpio_dt_spec zcd =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(zcd0), gpios, {0});

struct gpio_callback zcd_cb_data;

void zcd_callback(const struct device *dev,
    struct gpio_callback *cb, uint32_t pins)
{
    static uint64_t time_last = 0;
    int state = gpio_pin_get_dt(&zcd);
    uint64_t time_now = k_cycle_get_64();

    uint64_t diff = time_now - time_last;
    time_last = time_now;

    diff = k_cyc_to_us_floor64(diff);

    LOG_DBG("half wave time: %"PRId64" us", diff);
}

int main(void)
{
    LOG_INF("Device Ready");
    LOG_INF("Starting Solderotto %s", APP_VERSION_STRING);

    // configure button pin as input
    gpio_pin_configure_dt(&zcd, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&zcd, GPIO_INT_EDGE_BOTH);
    gpio_init_callback(&zcd_cb_data, zcd_callback, BIT(zcd.pin));
    gpio_add_callback_dt(&zcd, &zcd_cb_data);

    while (1)
    {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
