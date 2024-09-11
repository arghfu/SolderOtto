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

static const struct gpio_dt_spec dbg0 = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_0), gpios);

int main(void)
{
	int err;

	gpio_pin_configure_dt(&dbg0, (GPIO_OUTPUT |  GPIO_OPEN_DRAIN) );

	printk("Pin config failed:");
	while (1)
	{
		gpio_pin_toggle_dt(&dbg0);
		k_msleep(100);
	}
}
