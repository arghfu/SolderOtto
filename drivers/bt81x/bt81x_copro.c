/*
 * Copyright (c) 2020 Hubert Miś
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <string.h>

#include <app/drivers/bt81x/bt81x_copro.h>
#include <app/drivers/bt81x/bt81x_common.h>
#include <app/drivers/bt81x/bt81x_memory.h>

#include "bt81x_drv.h"

#define FT800_RAM_CMD_SIZE 4096UL

enum {
	CMD_DLSTART   = 0xffffff00,
	CMD_SWAP      = 0xffffff01,
	CMD_TEXT      = 0xffffff0c,
	CMD_NUMBER    = 0xffffff2e,
	CMD_CALIBRATE = 0xffffff15,
} ft8xx_cmd;

static uint16_t reg_cmd_read;
static uint16_t reg_cmd_write;

static uint16_t ram_cmd_fullness(void)
{
	return (reg_cmd_write - reg_cmd_read) % 4096UL;
}

static uint16_t ram_cmd_freespace(void)
{
	return (FT800_RAM_CMD_SIZE - 4UL) - ram_cmd_fullness();
}

static void refresh_reg_cmd_read(void)
{
	reg_cmd_read = bt81x_rd32(BT81X_REG_CMD_READ);
}

static void flush_reg_cmd_write(void)
{
	bt81x_wr32(BT81X_REG_CMD_WRITE, reg_cmd_write);
}

static void increase_reg_cmd_write(uint16_t value)
{
	reg_cmd_write = (reg_cmd_write + value) % FT800_RAM_CMD_SIZE;
}

void bt81x_copro_cmd(uint32_t cmd)
{
	while (ram_cmd_freespace() < sizeof(cmd)) {
		refresh_reg_cmd_read();
	}

	bt81x_wr32(BT81X_RAM_CMD + reg_cmd_write, cmd);
	increase_reg_cmd_write(sizeof(cmd));

	flush_reg_cmd_write();
}

void bt81x_copro_cmd_dlstart(void)
{
	bt81x_copro_cmd(CMD_DLSTART);
}

void bt81x_copro_cmd_swap(void)
{
	bt81x_copro_cmd(CMD_SWAP);
}

void bt81x_copro_cmd_text(int16_t x,
			   int16_t y,
			   int16_t font,
			   uint16_t options,
			   const char *s)
{
	const uint16_t str_bytes = strlen(s) + 1;
	const uint16_t padding_bytes = (4 - (str_bytes % 4)) % 4;
	const uint16_t cmd_size = sizeof(CMD_TEXT) +
				   sizeof(x) +
				   sizeof(y) +
				   sizeof(font) +
				   sizeof(options) +
				   str_bytes +
				   padding_bytes;

	while (ram_cmd_freespace() < cmd_size) {
		refresh_reg_cmd_read();
	}

	bt81x_wr32(BT81X_RAM_CMD + reg_cmd_write, CMD_TEXT);
	increase_reg_cmd_write(sizeof(CMD_TEXT));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, x);
	increase_reg_cmd_write(sizeof(x));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, y);
	increase_reg_cmd_write(sizeof(y));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, font);
	increase_reg_cmd_write(sizeof(font));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, options);
	increase_reg_cmd_write(sizeof(options));

	(void)bt81x_drv_write(BT81X_RAM_CMD + reg_cmd_write, s, str_bytes);
	increase_reg_cmd_write(str_bytes + padding_bytes);

	flush_reg_cmd_write();
}

void bt81x_copro_cmd_number(int16_t x,
			     int16_t y,
			     int16_t font,
			     uint16_t options,
			     int32_t n)
{
	const uint16_t cmd_size = sizeof(CMD_NUMBER) +
				   sizeof(x) +
				   sizeof(y) +
				   sizeof(font) +
				   sizeof(options) +
				   sizeof(n);

	while (ram_cmd_freespace() < cmd_size) {
		refresh_reg_cmd_read();
	}

	bt81x_wr32(BT81X_RAM_CMD + reg_cmd_write, CMD_NUMBER);
	increase_reg_cmd_write(sizeof(CMD_NUMBER));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, x);
	increase_reg_cmd_write(sizeof(x));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, y);
	increase_reg_cmd_write(sizeof(y));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, font);
	increase_reg_cmd_write(sizeof(font));

	bt81x_wr16(BT81X_RAM_CMD + reg_cmd_write, options);
	increase_reg_cmd_write(sizeof(options));

	bt81x_wr32(BT81X_RAM_CMD + reg_cmd_write, n);
	increase_reg_cmd_write(sizeof(n));

	flush_reg_cmd_write();
}

void bt81x_copro_cmd_calibrate(uint32_t *result)
{
	const uint16_t cmd_size = sizeof(CMD_CALIBRATE) + sizeof(uint32_t);
	uint32_t result_address;

	while (ram_cmd_freespace() < cmd_size) {
		refresh_reg_cmd_read();
	}

	bt81x_wr32(BT81X_RAM_CMD + reg_cmd_write, CMD_CALIBRATE);
	increase_reg_cmd_write(sizeof(CMD_CALIBRATE));

	result_address = BT81X_RAM_CMD + reg_cmd_write;
	bt81x_wr32(result_address, 1UL);
	increase_reg_cmd_write(sizeof(uint32_t));

	flush_reg_cmd_write();

	/* Wait until calibration is finished. */
	while (ram_cmd_fullness() > 0) {
		refresh_reg_cmd_read();
	}

	*result = bt81x_rd32(result_address);
}