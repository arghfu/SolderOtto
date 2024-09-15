/*
 * Copyright (c) 2020 Hubert Miś
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/byteorder.h>

#include <app/drivers/bt81x/bt81x_common.h>

#include "bt81x_drv.h"

void bt81x_wr8(uint32_t address, uint8_t data)
{
	int err = bt81x_drv_write(address, &data, sizeof(data));
	__ASSERT(err == 0, "Writing BT81x data at 0x%x failed", address);
}

void bt81x_wr16(uint32_t address, uint16_t data)
{
	uint8_t buffer[2];

	*(uint16_t *)buffer = sys_cpu_to_le16(data);
	int err = bt81x_drv_write(address, buffer, sizeof(buffer));
	__ASSERT(err == 0, "Writing BT81x data at 0x%x failed", address);
}

void bt81x_wr32(uint32_t address, uint32_t data)
{
	uint8_t buffer[4];

	*(uint32_t *)buffer = sys_cpu_to_le32(data);
	int err = bt81x_drv_write(address, buffer, sizeof(buffer));
	__ASSERT(err == 0, "Writing BT81x data at 0x%x failed", address);
}

uint8_t bt81x_rd8(uint32_t address)
{
	uint8_t data = 0;

	int err = bt81x_drv_read(address, &data, sizeof(data));
	__ASSERT(err == 0, "Reading BT81x data from 0x%x failed", address);

	return data;
}

uint16_t bt81x_rd16(uint32_t address)
{
	uint8_t buffer[2] = {0};

	int err = bt81x_drv_read(address, buffer, sizeof(buffer));
	__ASSERT(err == 0, "Reading BT81x data from 0x%x failed", address);

	return sys_le16_to_cpu(*(const uint16_t *)buffer);
}

uint32_t bt81x_rd32(uint32_t address)
{
	uint8_t buffer[4] = {0};

	int err = bt81x_drv_read(address, buffer, sizeof(buffer));
	__ASSERT(err == 0, "Reading BT81x data from 0x%x failed", address);

	return sys_le32_to_cpu(*(const uint32_t *)buffer);
}