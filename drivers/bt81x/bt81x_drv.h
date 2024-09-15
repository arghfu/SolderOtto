/*
 * Copyright (c) 2020 Hubert Miś
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief FT8XX serial driver API
 */

#ifndef ZEPHYR_DRIVERS_DISPLAY_BT81X_BT81X_DRV_H_
#define ZEPHYR_DRIVERS_DISPLAY_BT81X_BT81X_DRV_H_

#include <stdint.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

int bt81x_drv_init(void);
int bt81x_drv_read(uint32_t address, uint8_t *data, unsigned int length);
int bt81x_drv_write(uint32_t address, const uint8_t *data, unsigned int length);
int bt81x_drv_command(uint8_t command, uint8_t parameter);

extern void bt81x_drv_irq_triggered(const struct device *dev,
		struct gpio_callback *cb, uint32_t pins);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_DISPLAY_BT81X_BT81X_DRV_H_ */
