/*
 * Copyright (c) 2020 Hubert Miś
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT bridgetek_bt81x

#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <app/drivers/bt81x/bt81x.h>
#include <app/drivers/bt81x/bt81x_copro.h>
#include <app/drivers/bt81x/bt81x_common.h>
#include <app/drivers/bt81x/bt81x_dl.h>
#include <app/drivers/bt81x/bt81x_memory.h>

#include "bt81x_drv.h"
#include "bt81x_host_commands.h"

LOG_MODULE_REGISTER(bt81x, CONFIG_DISPLAY_LOG_LEVEL);

#define FT8XX_DLSWAP_FRAME 0x02

#define BT81X_EXPECTED_ID 0x7C

struct bt81x_config {
	uint16_t pclk_freq;
	uint16_t vsize;
	uint16_t voffset;
	uint16_t vcycle;
	uint16_t vsync0;
	uint16_t vsync1;
	uint16_t hsize;
	uint16_t hoffset;
	uint16_t hcycle;
	uint16_t hsync0;
	uint16_t hsync1;
	uint8_t pclk;
	uint8_t pclk_2x	 :1;
	uint8_t dither	 :1;
	uint8_t pclk_pol :1;
	uint8_t cspread  :1;
	uint8_t swizzle  :4;
};

struct bt81x_data {
	const struct bt81x_config *config;
	bt81x_int_callback irq_callback;
};

const static struct bt81x_config bt81x_config = {
	.pclk     = DT_INST_PROP(0, pclk),
	.pclk_pol = DT_INST_PROP(0, pclk_pol),
	.pclk_freq = DT_INST_PROP(0, pclk_freq),
	.pclk_2x = DT_INST_PROP(0, pclk_2x),
	.cspread  = DT_INST_PROP(0, cspread),
	.swizzle  = DT_INST_PROP(0, swizzle),
	.vsize    = DT_INST_PROP(0, vsize),
	.voffset  = DT_INST_PROP(0, voffset),
	.vcycle   = DT_INST_PROP(0, vcycle),
	.vsync0   = DT_INST_PROP(0, vsync0),
	.vsync1   = DT_INST_PROP(0, vsync1),
	.hsize    = DT_INST_PROP(0, hsize),
	.hoffset  = DT_INST_PROP(0, hoffset),
	.hcycle   = DT_INST_PROP(0, hcycle),
	.hsync0   = DT_INST_PROP(0, hsync0),
	.hsync1   = DT_INST_PROP(0, hsync1),
	.dither   = DT_INST_PROP(0, dither),
};

static struct bt81x_data bt81x_data = {
	.config = &bt81x_config,
	.irq_callback = NULL,
};

static void host_command(uint8_t cmd, uint8_t parameter)
{
	int err = bt81x_drv_command(cmd, parameter);
	__ASSERT(err == 0, "Writing FT8xx command failed");
}

static bool verify_chip(void)
{
	uint8_t id = bt81x_rd8(BT81X_REG_ID);

	return (id & 0xff) == BT81X_EXPECTED_ID;
}

static bool verify_state(void)
{
	uint8_t state = bt81x_rd8(BT81X_REG_CPURESET);

	return (state & 0x7) == 0;
}

static int bt81x_init(const struct device *dev)
{
	const struct bt81x_config *config = dev->config;

	int ret = bt81x_drv_init();
	if (ret < 0) {
		LOG_ERR("BT81x driver initialization failed with %d", ret);
		return ret;
	}

	/* Reset display controller */
	host_command(CORERST, 0);
	host_command(CLKEXT, 0);
	host_command(CLKSEL, 0);
	host_command(ACTIVE, 0);
	k_sleep(K_MSEC(50));

	if (!verify_chip()) {
		LOG_ERR("BT81x chip not recognized");
		return -ENODEV;
	}
	k_sleep(K_MSEC(50));

	if (!verify_state()) {
		LOG_ERR("BT81x chip not resetted");
		return -ENODEV;
	}

	bt81x_wr32(BT81X_REG_FREQUENCY, 72000000);


	/* Configure LCD */
	bt81x_wr16(BT81X_REG_HSIZE, config->hsize);
	bt81x_wr16(BT81X_REG_HCYCLE, config->hcycle);
	bt81x_wr16(BT81X_REG_HOFFSET, config->hoffset);
	bt81x_wr16(BT81X_REG_HSYNC0, config->hsync0);
	bt81x_wr16(BT81X_REG_HSYNC1, config->hsync1);
	bt81x_wr16(BT81X_REG_VSIZE, config->vsize);
	bt81x_wr16(BT81X_REG_VCYCLE, config->vcycle);
	bt81x_wr16(BT81X_REG_VOFFSET, config->voffset);
	bt81x_wr16(BT81X_REG_VSYNC0, config->vsync0);
	bt81x_wr16(BT81X_REG_VSYNC1, config->vsync1);

	bt81x_wr8(BT81X_REG_SWIZZLE, config->swizzle);
	bt81x_wr8(BT81X_REG_PCLK_POL, config->pclk_pol);
	bt81x_wr8(BT81X_REG_CSPREAD, config->cspread);

	/* Display initial screen */
	/* Set the initial color */
	bt81x_wr32(BT81X_RAM_DL + 0, FT8XX_CLEAR_COLOR_RGB(0, 0x80, 0));
	/* Clear to the initial color */
	bt81x_wr32(BT81X_RAM_DL + 4, FT8XX_CLEAR(1, 1, 1));
	/* End the display list */
	bt81x_wr32(BT81X_RAM_DL + 8, FT8XX_DISPLAY());
	bt81x_wr32(BT81X_REG_DLSWAP, FT8XX_DLSWAP_FRAME);

	/* Enable backlight */
	bt81x_wr16(BT81X_REG_PWM_HZ, 0x00FA);
	bt81x_wr8(BT81X_REG_PWM_DUTY, 0x10);

	/* Enable LCD */
	bt81x_wr8(BT81X_REG_GPIO, 0x80U);
	bt81x_wr8(BT81X_REG_GPIO_DIR, 0x80);
	bt81x_wr16(BT81X_REG_PCLK_FREQ, config->pclk_freq);
	bt81x_wr8(BT81X_REG_PCLK_2X, config->pclk_2x);
	bt81x_wr8(BT81X_REG_PCLK, config->pclk);

	return 0;
}

DEVICE_DT_INST_DEFINE(0, bt81x_init, NULL, &bt81x_data, &bt81x_config,
		      POST_KERNEL, CONFIG_BT81X_INIT_PRIORITY, NULL);

int bt81x_get_touch_tag(void)
{
	/* Read FT800_REG_INT_FLAGS to clear IRQ */
	(void)bt81x_rd8(BT81X_REG_INT_FLAGS);

	return (int)bt81x_rd8(BT81X_REG_TOUCH_TAG);
}

void bt81x_drv_irq_triggered(const struct device *dev, struct gpio_callback *cb,
			      uint32_t pins)
{
	if (bt81x_data.irq_callback != NULL) {
		bt81x_data.irq_callback();
	}
}

void bt81x_register_int(bt81x_int_callback callback)
{
	if (bt81x_data.irq_callback != NULL) {
		return;
	}

	bt81x_data.irq_callback = callback;
	bt81x_wr8(BT81X_REG_INT_MASK, 0x04);
	bt81x_wr8(BT81X_REG_INT_EN, 0x01);
}

void bt81x_calibrate(struct bt81x_touch_transform *data)
{
	uint32_t result = 0;

	do {
		bt81x_copro_cmd_dlstart();
		bt81x_copro_cmd(FT8XX_CLEAR_COLOR_RGB(0x00, 0x00, 0x00));
		bt81x_copro_cmd(FT8XX_CLEAR(1, 1, 1));
		bt81x_copro_cmd_calibrate(&result);
	} while (result == 0);

	data->a = bt81x_rd32(BT81X_REG_TOUCH_TRANSFORM_A);
	data->b = bt81x_rd32(BT81X_REG_TOUCH_TRANSFORM_B);
	data->c = bt81x_rd32(BT81X_REG_TOUCH_TRANSFORM_C);
	data->d = bt81x_rd32(BT81X_REG_TOUCH_TRANSFORM_D);
	data->e = bt81x_rd32(BT81X_REG_TOUCH_TRANSFORM_E);
	data->f = bt81x_rd32(BT81X_REG_TOUCH_TRANSFORM_F);
}

void bt81x_touch_transform_set(const struct bt81x_touch_transform *data)
{
	bt81x_wr32(BT81X_REG_TOUCH_TRANSFORM_A, data->a);
	bt81x_wr32(BT81X_REG_TOUCH_TRANSFORM_B, data->b);
	bt81x_wr32(BT81X_REG_TOUCH_TRANSFORM_C, data->c);
	bt81x_wr32(BT81X_REG_TOUCH_TRANSFORM_D, data->d);
	bt81x_wr32(BT81X_REG_TOUCH_TRANSFORM_E, data->e);
	bt81x_wr32(BT81X_REG_TOUCH_TRANSFORM_F, data->f);
}
