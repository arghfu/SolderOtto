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

#include <app/drivers/bt81x/bt81x.h>
#include <app/drivers/bt81x/bt81x_copro.h>
#include <app/drivers/bt81x/bt81x_dl.h>

static const struct gpio_dt_spec dbg0 = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_0), gpios);

/* sleep time in msec */
#define SLEEPTIME  100

/* touch tags */
#define TAG_PLUS 1
#define TAG_MINUS 2

static volatile bool process_touch;

static void touch_irq(void)
{
	process_touch = true;
}

int main(void)
{

	gpio_pin_configure_dt(&dbg0, (GPIO_OUTPUT |  GPIO_OPEN_DRAIN) );
	int cnt;
	int val;
	struct bt81x_touch_transform tt;

	/* To get touch events calibrate the display */
	bt81x_calibrate(&tt);

	/* Get interrupts on touch event */
	bt81x_register_int(touch_irq);

	/* Starting counting */
	val = 0;
	cnt = 0;

	while (1) {
		/* Start Display List */
		bt81x_copro_cmd_dlstart();
		bt81x_copro_cmd(FT8XX_CLEAR_COLOR_RGB(0x00, 0x00, 0x00));
		bt81x_copro_cmd(FT8XX_CLEAR(1, 1, 1));

		/* Set color */
		bt81x_copro_cmd(FT8XX_COLOR_RGB(0xf0, 0xf0, 0xf0));

		/* Display the counter */
		bt81x_copro_cmd_number(20, 20, 29, FT8XX_OPT_SIGNED, cnt);
		cnt++;

		/* Display the hello message */
		bt81x_copro_cmd_text(20, 70, 30, 0, "Hello,");
		/* Set Zephyr color */
		bt81x_copro_cmd(FT8XX_COLOR_RGB(0x78, 0x29, 0xd2));
		bt81x_copro_cmd_text(20, 105, 30, 0, "Zephyr!");

		/* Display value set with buttons */
		bt81x_copro_cmd(FT8XX_COLOR_RGB(0xff, 0xff, 0xff));
		bt81x_copro_cmd_number(80, 170, 29,
					FT8XX_OPT_SIGNED | FT8XX_OPT_RIGHTX,
					val);
		bt81x_copro_cmd(FT8XX_TAG(TAG_PLUS));
		bt81x_copro_cmd_text(90, 160, 31, 0, "+");
		bt81x_copro_cmd(FT8XX_TAG(TAG_MINUS));
		bt81x_copro_cmd_text(20, 160, 31, 0, "-");

		/* Finish Display List */
		bt81x_copro_cmd(FT8XX_DISPLAY());
		/* Display created frame */
		bt81x_copro_cmd_swap();

		if (process_touch) {
			int tag = bt81x_get_touch_tag();

			if (tag == TAG_PLUS) {
				val++;
			} else if (tag == TAG_MINUS) {
				val--;
			}

			process_touch = false;
		}
		gpio_pin_toggle_dt(&dbg0);
		/* Wait a while */
		k_msleep(SLEEPTIME);
	}
}
