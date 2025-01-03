/*
 * Copyright (c) 2020 Hubert Miś
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief FT8XX host commands enumeration
 */

#ifndef ZEPHYR_DRIVERS_DISPLAY_BT81X_BT81X_HOST_COMMANDS_H_
#define ZEPHYR_DRIVERS_DISPLAY_BT81X_BT81X_HOST_COMMANDS_H_

#ifdef __cplusplus
extern "C" {
#endif

enum ft800_command_t {
	ACTIVE = 0,
	STANDBY = 0x41,
	SLEEP   = 0x42,
	PWRDOWN = 0x50,

	CLKINT	= 0x48,
	CLKEXT  = 0x44,
	CLK48M  = 0x62,
	CLKSEL  = 0x61,

	CORERST = 0x68
};

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_DISPLAY_BT81X_BT81X_HOST_COMMANDS_H_ */
