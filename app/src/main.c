/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/app_version.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/spi.h>


LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

// static const struct gpio_dt_spec sel1 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel1), gpios);
// static const struct gpio_dt_spec sel2 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel2), gpios);
// static const struct gpio_dt_spec sel3 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel3), gpios);
//
// static const struct gpio_dt_spec load0 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_load0), gpios);
// static const struct gpio_dt_spec load1 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_load1), gpios);
// static const struct gpio_dt_spec com = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_com), gpios);
//
// static const struct gpio_dt_spec low_load = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_low_load), gpios);
// static const struct gpio_dt_spec low_com = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_low_com), gpios);

static const struct gpio_dt_spec dbg0 = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_0), gpios);

int main(void)
{
	int err;

	// gpio_pin_configure_dt(&dbg0, (GPIO_OUTPUT |  GPIO_OPEN_DRAIN) );

	// gpio_pin_configure_dt(&sel1, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&sel2, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&sel3, GPIO_OUTPUT_LOW);
	//
	// gpio_pin_configure_dt(&load0, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&load1, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&com, GPIO_OUTPUT_LOW);
	//
	// gpio_pin_configure_dt(&low_com, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&low_load, GPIO_OUTPUT_LOW);
	//
	// gpio_pin_set_dt(&low_com, GPIO_PIN_RESET);
	//
	// gpio_pin_set_dt(&sel1, GPIO_PIN_RESET);
	// gpio_pin_set_dt(&sel2, GPIO_PIN_RESET);
	// gpio_pin_set_dt(&sel3, GPIO_PIN_RESET);

	while (1)
	{
		printk("Could not read (%d)\n", err);
		// gpio_pin_set_dt(&sel1, GPIO_PIN_SET);
		k_usleep((100));
		// gpio_pin_set_dt(&sel1, GPIO_PIN_RESET);

	}

	return 0;
}
