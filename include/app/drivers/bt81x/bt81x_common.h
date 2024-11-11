/*
 * Copyright (c) 2024 arghfu
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief BT81X common functions
 */

#ifndef ZEPHYR_DRIVERS_MISC_BT81X_BT81X_COMMON_H_
#define ZEPHYR_DRIVERS_MISC_BT81X_BT81X_COMMON_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BT81x functions to write and read memory
 * @defgroup bt81x_common BT81x common functions
 * @ingroup bt81x_interface
 * @{
 */

/**
 * @brief Write 1 byte (8 bits) to BT81x memory
 *
 * @param address Memory address to write to
 * @param data Byte to write
 */
void bt81x_wr8(uint32_t address, uint8_t data);

/**
 * @brief Write 2 bytes (16 bits) to BT81x memory
 *
 * @param address Memory address to write to
 * @param data Value to write
 */
void bt81x_wr16(uint32_t address, uint16_t data);

/**
 * @brief Write 4 bytes (32 bits) to BT81x memory
 *
 * @param address Memory address to write to
 * @param data Value to write
 */
void bt81x_wr32(uint32_t address, uint32_t data);

/**
 * @brief Read 1 byte (8 bits) from BT81x memory
 *
 * @param address Memory address to read from
 *
 * @return Value read from memory
 */
uint8_t bt81x_rd8(uint32_t address);

/**
 * @brief Read 2 bytes (16 bits) from BT81x memory
 *
 * @param address Memory address to read from
 *
 * @return Value read from memory
 */
uint16_t bt81x_rd16(uint32_t address);

/**
 * @brief Read 4 bytes (32 bits) from BT81x memory
 *
 * @param address Memory address to read from
 *
 * @return Value read from memory
 */
uint32_t bt81x_rd32(uint32_t address);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_MISC_BT81X_BT81X_COMMON_H_ */
