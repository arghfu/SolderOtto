/*
 * Copyright (c) 2024 arghfu
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief BT81X public API
 */

#ifndef ZEPHYR_DRIVERS_MISC_BT81X_BT81X_H_
#define ZEPHYR_DRIVERS_MISC_BT81X_BT81X_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BT81x driver public APIs
 * @defgroup bt81x_interface BT81x driver APIs
 * @ingroup misc_interfaces
 * @{
 */

/**
 * @struct bt81x_touch_transform
 * @brief Structure holding touchscreen calibration data
 *
 * The content of this structure is filled by bt81x_calibrate().
 */
struct bt81x_touch_transform {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
	uint32_t e;
	uint32_t f;
};

/**
 * @typedef bt81x_int_callback
 * @brief Callback API to inform API user that BT81X triggered interrupt
 *
 * This callback is called from IRQ context.
 */
typedef void (*bt81x_int_callback)(void);

/**
 * @brief Calibrate touchscreen
 *
 * Run touchscreen calibration procedure that collects three touches from touch
 * screen. Computed calibration result is automatically applied to the
 * touchscreen processing and returned with @p data.
 *
 * The content of @p data may be stored and used after reset in
 * bt81x_touch_transform_set() to avoid calibrating touchscreen after each
 * device reset.
 *
 * @param data Pointer to touchscreen transform structure to populate
 */
void bt81x_calibrate(struct bt81x_touch_transform *data);

/**
 * @brief Set touchscreen calibration data
 *
 * Apply given touchscreen transform data to the touchscreen processing.
 * Data is to be obtained from calibration procedure started with
 * bt81x_calibrate().
 *
 * @param data Pointer to touchscreen transform structure to apply
 */
void bt81x_touch_transform_set(const struct bt81x_touch_transform *data);

/**
 * @brief Get tag of recently touched element
 *
 * @return Tag value 0-255 of recently touched element
 */
int bt81x_get_touch_tag(void);

/**
 * @brief Set callback executed when BT81x triggers interrupt
 *
 * This function configures BT81x to trigger interrupt when touch event changes
 * tag value.
 *
 * When touch event changes tag value, BT81x activates INT line. The line is
 * kept active until bt81x_get_touch_tag() is called. It results in single
 * execution of @p callback until bt81x_get_touch_tag() is called.
 *
 * @param callback Pointer to function called when BT81X triggers interrupt
 */
void bt81x_register_int(bt81x_int_callback callback);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_MISC_BT81X_BT81X_H_ */
