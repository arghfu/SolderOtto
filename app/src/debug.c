#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include "debug.h"


static const struct gpio_dt_spec dbg_pins[] = { // NOLINT(*-interfaces-global-init)
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_2), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_3), gpios),
};

int dbg_init()
{
    int err = 0;
    for (int i = 0; i < ARRAY_SIZE(dbg_pins); i++)
    {
        err = gpio_pin_configure_dt(&dbg_pins[i], GPIO_OUTPUT | GPIO_ACTIVE_LOW | GPIO_OPEN_DRAIN);
        if (err < 0)
        {
            return err;
        }

        gpio_pin_set_dt(&dbg_pins[i], GPIO_PIN_RESET);
    }
    return 0;
}

int dbg_set_pin(int pin, int value)
{
    if (pin < 0 || pin >= ARRAY_SIZE(dbg_pins))
    {
        return -EINVAL;
    }

    return gpio_pin_set_dt(&dbg_pins[pin], value);;
}
