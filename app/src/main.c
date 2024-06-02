/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <app_version.h>

uint64_t last_time = 0;

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

static const struct gpio_dt_spec sel2 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel2), gpios);
static const struct gpio_dt_spec sel3 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel3), gpios);

static const struct gpio_dt_spec load_high = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_load_high), gpios);
static const struct gpio_dt_spec load_low = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_load_low), gpios);
static const struct gpio_dt_spec com_high = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_com_high), gpios);
static const struct gpio_dt_spec com_low = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_com_low), gpios);

static const struct gpio_dt_spec handle = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_handle), gpios);
static const struct gpio_dt_spec stand0 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_stand0), gpios);
static const struct gpio_dt_spec stand1 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_stand1), gpios);
static const struct gpio_dt_spec tip = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_tip), gpios);

static const struct gpio_dt_spec zeroCross = GPIO_DT_SPEC_GET(DT_ALIAS(zcd), gpios);

struct gpio_callback zcd_cb_data;

void zcd_callback(const struct device *dev,
	struct gpio_callback *cb, uint32_t pins)
{
	uint64_t now = k_uptime_get();

	printk("%llu\n", now - last_time);

	last_time = now;
}

int main(void)
{
	bool led_state = true;

	if (!gpio_is_ready_dt(&sel2) && ! gpio_is_ready_dt(&sel3))
	{
		return 0;
	}
	// make sure the GPIO device is ready
	if (!gpio_is_ready_dt(&zeroCross))
	{
		return 0;
	}

	// configure the button pin as input
	gpio_pin_configure_dt(&zeroCross, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&zeroCross, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&zcd_cb_data, zcd_callback, BIT(zeroCross.pin));
	gpio_add_callback_dt(&zeroCross, &zcd_cb_data);

	gpio_pin_configure_dt(&handle, GPIO_INPUT);
	gpio_pin_configure_dt(&stand0, GPIO_INPUT);
	gpio_pin_configure_dt(&stand1, GPIO_INPUT | GPIO_PULL_DOWN);
	gpio_pin_configure_dt(&tip, GPIO_INPUT);

	gpio_pin_configure_dt(&sel2, GPIO_OUTPUT_LOW);
	gpio_pin_configure_dt(&sel3, GPIO_OUTPUT_LOW);

	gpio_pin_configure_dt(&load_high, GPIO_OUTPUT_LOW);
	gpio_pin_configure_dt(&load_low, GPIO_OUTPUT_LOW);
	gpio_pin_configure_dt(&com_high, GPIO_OUTPUT_LOW);
	gpio_pin_configure_dt(&com_low, GPIO_OUTPUT_LOW);

	gpio_pin_set_dt(&sel2, GPIO_PIN_RESET);
	gpio_pin_set_dt(&sel3, GPIO_PIN_SET);

	gpio_pin_set_dt(&load_high, GPIO_PIN_RESET);
	gpio_pin_set_dt(&load_low, GPIO_PIN_RESET);

	gpio_pin_set_dt(&com_high, GPIO_PIN_RESET);
	gpio_pin_set_dt(&com_low, GPIO_PIN_RESET);


	printk("Zephyr Example Application %s\n", APP_VERSION_STRING);

	while (1)
	{
		// gpio_pin_toggle_dt(&sel3);

		// ret = gpio_pin_set_dt(&sel3, GPIO_PIN_SET);
		//
		// if (ret < 0) {
		// 	return 0;
		// }

		// T245
		// if(led_state == true)
		// {
		// 	gpio_pin_set_dt(&load_low, GPIO_PIN_SET);
		// 	gpio_pin_set_dt(&com_high, GPIO_PIN_SET);
		// 	k_sleep(K_MSEC(800));
		// }
		// else
		// {
		// 	gpio_pin_set_dt(&com_high, GPIO_PIN_RESET);
		// 	k_sleep(K_MSEC(5000));
		// }

		// T210
		// if(led_state == true)
		// {
		// 	gpio_pin_set_dt(&com_low, GPIO_PIN_SET);
		// 	gpio_pin_set_dt(&load_high, GPIO_PIN_SET);
		// 	k_sleep(K_MSEC(100));
		// }
		// else
		// {
		// 	gpio_pin_set_dt(&load_high, GPIO_PIN_RESET);
		// 	k_sleep(K_MSEC(5000));
		// }

		led_state = !led_state;
		// printk("LED state: %s\n", led_state ? "ON" : "OFF");


		k_sleep(K_MSEC(200));
	}

	return 0;
}

