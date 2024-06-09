/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/app_version.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/dma.h>
#include <zephyr/drivers/spi.h>

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

	//TODO do stuff

	last_time = now;
}

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
				 DT_SPEC_AND_COMMA)
};

int main(void)
{
	bool led_state = false;
	int err;
	uint32_t count = 0;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}
	LOG_INF("Device Ready");
	LOG_INF("Zephyr Example Application %s", APP_VERSION_STRING);

	// configure button pin as input
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
	gpio_pin_set_dt(&sel3, GPIO_PIN_RESET);

	gpio_pin_set_dt(&load_high, GPIO_PIN_RESET);
	gpio_pin_set_dt(&load_low, GPIO_PIN_RESET);

	gpio_pin_set_dt(&com_high, GPIO_PIN_RESET);
	gpio_pin_set_dt(&com_low, GPIO_PIN_SET);

	while (1)
	{
		// // // T245
		// if(led_state == true)
		// {
		// 	gpio_pin_set_dt(&load_low, GPIO_PIN_SET);
		// 	gpio_pin_set_dt(&com_high, GPIO_PIN_SET);
		// 	k_sleep(K_MSEC(40));
		// }
		// else
		// {
		// 	gpio_pin_set_dt(&com_high, GPIO_PIN_RESET);
		// 	k_sleep(K_MSEC(5000));
		// }

		// T210
		if(led_state == true)
		{
			// gpio_pin_set_dt(&com_low, GPIO_PIN_SET);
			// gpio_pin_set_dt(&load_high, GPIO_PIN_SET);
			k_sleep(K_MSEC(40));
		}
		else
		{
			gpio_pin_set_dt(&load_high, GPIO_PIN_RESET);
			k_sleep(K_MSEC(5000));
		}

		led_state = !led_state;

		// printk("ADC reading[%u]:\n", count++);
		// for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		// 	int32_t val_mv;
		//
		// 	printk("- %s, channel %d: ",
		// 		   adc_channels[i].dev->name,
		// 		   adc_channels[i].channel_id);
		//
		// 	(void)adc_sequence_init_dt(&adc_channels[i], &sequence);
		//
		// 	err = adc_read_dt(&adc_channels[i], &sequence);
		// 	if (err < 0) {
		// 		printk("Could not read (%d)\n", err);
		// 		continue;
		// 	}
		//
		// 	val_mv = (int32_t)buf;
		//
		// 	printk("%"PRId32, val_mv);
		// 	err = adc_raw_to_millivolts_dt(&adc_channels[i],
		// 					   &val_mv);
		// 	/* conversion to mV may not be supported, skip if not */
		// 	if (err < 0) {
		// 		printk(" (value in mV not available)\n");
		// 	} else {
		// 		printk(" = %"PRId32" mV\n", val_mv);
		// 	}
		// }
		//
		// k_sleep(K_MSEC(400));
	}

	return 0;
}

