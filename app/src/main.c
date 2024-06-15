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

// static const struct gpio_dt_spec sel2 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel2), gpios);
// static const struct gpio_dt_spec sel3 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_sel3), gpios);
//
// static const struct gpio_dt_spec load_high = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_load_high), gpios);
// static const struct gpio_dt_spec load_low = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_load_low), gpios);
// static const struct gpio_dt_spec com_high = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_com_high), gpios);
// static const struct gpio_dt_spec com_low = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_com_low), gpios);
//
// static const struct gpio_dt_spec handle = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_handle), gpios);
// static const struct gpio_dt_spec stand0 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_stand0), gpios);
// static const struct gpio_dt_spec stand1 = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_stand1), gpios);
// static const struct gpio_dt_spec tip = GPIO_DT_SPEC_GET(DT_ALIAS(ch0_tip), gpios);
//
// static const struct gpio_dt_spec zeroCross = GPIO_DT_SPEC_GET(DT_ALIAS(zcd), gpios);
//
// struct gpio_callback zcd_cb_data;
//
// void zcd_callback(const struct device *dev,
// 	struct gpio_callback *cb, uint32_t pins)
// {
// 	uint64_t now = k_uptime_get();
//
// 	//TODO do stuff
//
// 	last_time = now;
// }
//
// #if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
// !DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
// #error "No suitable devicetree overlay specified"
// #endif
//
// #define DT_SPEC_AND_COMMA(node_id, prop, idx) \
// ADC_DT_SPEC_GET_BY_IDX(node_id, idx),
//
// /* Data of ADC io-channels specified in devicetree. */
// static const struct adc_dt_spec adc_channels[] = {
// 	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
// 				 DT_SPEC_AND_COMMA)
// };

int main(void)
{
	// bool led_state = false;
	// int err;
	// uint32_t count = 0;
	// uint16_t buf;
	// struct adc_sequence sequence = {
	// 	.buffer = &buf,
	// 	/* buffer size in bytes, not number of samples */
	// 	.buffer_size = sizeof(buf),
	// };
	//
	// /* Configure channels individually prior to sampling. */
	// for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
	// 	if (!adc_is_ready_dt(&adc_channels[i])) {
	// 		printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
	// 		return 0;
	// 	}
	//
	// 	err = adc_channel_setup_dt(&adc_channels[i]);
	// 	if (err < 0) {
	// 		printk("Could not setup channel #%d (%d)\n", i, err);
	// 		return 0;
	// 	}
	// }
	// LOG_INF("Device Ready");
	// LOG_INF("Zephyr Example Application %s", APP_VERSION_STRING);
	//
	// // configure button pin as input
	// gpio_pin_configure_dt(&zeroCross, GPIO_INPUT);
	// gpio_pin_interrupt_configure_dt(&zeroCross, GPIO_INT_EDGE_TO_ACTIVE);
	// gpio_init_callback(&zcd_cb_data, zcd_callback, BIT(zeroCross.pin));
	// gpio_add_callback_dt(&zeroCross, &zcd_cb_data);
	//
	// gpio_pin_configure_dt(&handle, GPIO_INPUT);
	// gpio_pin_configure_dt(&stand0, GPIO_INPUT);
	// gpio_pin_configure_dt(&stand1, GPIO_INPUT | GPIO_PULL_DOWN);
	// gpio_pin_configure_dt(&tip, GPIO_INPUT);
	//
	// gpio_pin_configure_dt(&sel2, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&sel3, GPIO_OUTPUT_LOW);
	//
	// gpio_pin_configure_dt(&load_high, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&load_low, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&com_high, GPIO_OUTPUT_LOW);
	// gpio_pin_configure_dt(&com_low, GPIO_OUTPUT_LOW);
	//
	// gpio_pin_set_dt(&sel2, GPIO_PIN_RESET);
	// gpio_pin_set_dt(&sel3, GPIO_PIN_RESET);
	//
	// gpio_pin_set_dt(&load_high, GPIO_PIN_RESET);
	// gpio_pin_set_dt(&load_low, GPIO_PIN_RESET);
	//
	// gpio_pin_set_dt(&com_high, GPIO_PIN_RESET);
	// gpio_pin_set_dt(&com_low, GPIO_PIN_SET);
	//
	// while (1)
	// {
	// 	// // T245
	// 	if(led_state == true)
	// 	{
	// 		// gpio_pin_set_dt(&load_low, GPIO_PIN_SET);
	// 		// gpio_pin_set_dt(&com_high, GPIO_PIN_SET);
	// 		k_sleep(K_MSEC(200));
	// 	}
	// 	else
	// 	{
	// 		gpio_pin_set_dt(&com_high, GPIO_PIN_RESET);
	// 		k_sleep(K_MSEC(5000));
	// 	}
	//
	// 	// T210
	// 	// if(led_state == true)d
	// 	// {
	// 		// gpio_pin_set_dt(&com_low, GPIO_PIN_SET);
	// 		// gpio_pin_set_dt(&load_high, GPIO_PIN_SET);
	// 	// 	k_sleep(K_MSEC(40));
	// 	// }
	// 	// else
	// 	// {
	// 	// 	gpio_pin_set_dt(&load_high, GPIO_PIN_RESET);
	// 	// 	k_sleep(K_MSEC(5000));
	// 	// }
	//
	// 	led_state = !led_state;
	//
	// 	// printk("ADC reading[%u]:\n", count++);s
	// 	// for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {a
	// 	// 	int32_t val_mv;
	// 	//
	// 	// 	printk("- %s, channel %d: ",
	// 	// 		   adc_channels[i].dev->name,
	// 	// 		   adc_channels[i].channel_id);
	// 	//
	// 	// 	(void)adc_sequence_init_dt(&adc_channels[i], &sequence);
	// 	//
	// 	// 	err = adc_read_dt(&adc_channels[i], &sequence);
	// 	// 	if (err < 0) {
	// 	// 		printk("Could not read (%d)\n", err);
	// 	// 		continue;
	// 	// 	}
	// 	//
	// 	// 	val_mv = (int32_t)buf;
	// 	//
	// 	// 	printk("%"PRId32, val_mv);
	// 	// 	err = adc_raw_to_millivolts_dt(&adc_channels[i],
	// 	// 					   &val_mv);
	// 	// 	/* conversion to mV may not be supported, skip if not */
	// 	// 	if (err < 0) {
	// 	// 		printk(" (value in mV not available)\n");
	// 	// 	} else {
	// 	// 		printk(" = %"PRId32" mV\n", val_mv);
	// 	// 	}
	// 	// }
	// 	//
	// 	// k_sleep(K_MSEC(400));
	// }

	return 0;
}


// /*
//  * Copyright (c) 2016 Intel Corporation.
//  *
//  * SPDX-License-Identifier: Apache-2.0
//  */
//
// #include <zephyr/kernel.h>
// #include <zephyr/drivers/flash.h>
// #include <zephyr/device.h>
// #include <zephyr/devicetree.h>
// #include <stdio.h>
// #include <string.h>
//
// #if defined(CONFIG_BOARD_ADAFRUIT_FEATHER_STM32F405)
// #define SPI_FLASH_TEST_REGION_OFFSET 0xf000
// #elif defined(CONFIG_BOARD_ARTY_A7_DESIGNSTART_FPGA_CORTEX_M1) || \
// 	defined(CONFIG_BOARD_ARTY_A7_DESIGNSTART_FPGA_CORTEX_M3)
// /* The FPGA bitstream is stored in the lower 536 sectors of the flash. */
// #define SPI_FLASH_TEST_REGION_OFFSET \
// 	DT_REG_SIZE(DT_NODE_BY_FIXED_PARTITION_LABEL(fpga_bitstream))
// #elif defined(CONFIG_BOARD_NPCX9M6F_EVB) || \
// 	defined(CONFIG_BOARD_NPCX7M6FB_EVB)
// #define SPI_FLASH_TEST_REGION_OFFSET 0x7F000
// #else
// #define SPI_FLASH_TEST_REGION_OFFSET 0xff000
// #endif
// #define SPI_FLASH_SECTOR_SIZE        4096
//
// #if defined(CONFIG_FLASH_STM32_OSPI) || \
// 	defined(CONFIG_FLASH_STM32_QSPI) || \
// 	defined(CONFIG_FLASH_STM32_XSPI)
// #define SPI_FLASH_MULTI_SECTOR_TEST
// #endif
//
// #if DT_HAS_COMPAT_STATUS_OKAY(jedec_spi_nor)
// #define SPI_FLASH_COMPAT jedec_spi_nor
// #elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_qspi_nor)
// #define SPI_FLASH_COMPAT st_stm32_qspi_nor
// #elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_ospi_nor)
// #define SPI_FLASH_COMPAT st_stm32_ospi_nor
// #elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_xspi_nor)
// #define SPI_FLASH_COMPAT st_stm32_xspi_nor
// #elif DT_HAS_COMPAT_STATUS_OKAY(nordic_qspi_nor)
// #define SPI_FLASH_COMPAT nordic_qspi_nor
// #else
// #define SPI_FLASH_COMPAT invalid
// #endif
//
// const uint8_t erased[] = { 0xff, 0xff, 0xff, 0xff };
//
// void single_sector_test(const struct device *flash_dev)
// {
// 	const uint8_t expected[] = { 0x55, 0xaa, 0x66, 0x99 };
// 	const size_t len = sizeof(expected);
// 	uint8_t buf[sizeof(expected)];
// 	int rc;
//
// 	printf("\nPerform test on single sector");
// 	/* Write protection needs to be disabled before each write or
// 	 * erase, since the flash component turns on write protection
// 	 * automatically after completion of write and erase
// 	 * operations.
// 	 */
// 	printf("\nTest 1: Flash erase\n");
//
// 	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
// 	 * SPI_FLASH_SECTOR_SIZE = flash size
// 	 */
// 	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
// 			 SPI_FLASH_SECTOR_SIZE);
// 	if (rc != 0) {
// 		printf("Flash erase failed! %d\n", rc);
// 	} else {
// 		/* Check erased pattern */
// 		memset(buf, 0, len);
// 		rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
// 		if (rc != 0) {
// 			printf("Flash read failed! %d\n", rc);
// 			return;
// 		}
// 		if (memcmp(erased, buf, len) != 0) {
// 			printf("Flash erase failed at offset 0x%x got 0x%x\n",
// 				SPI_FLASH_TEST_REGION_OFFSET, *(uint32_t *)buf);
// 			return;
// 		}
// 		printf("Flash erase succeeded!\n");
// 	}
// 	printf("\nTest 2: Flash write\n");
//
// 	printf("Attempting to write %zu bytes\n", len);
// 	rc = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, expected, len);
// 	if (rc != 0) {
// 		printf("Flash write failed! %d\n", rc);
// 		return;
// 	}
//
// 	memset(buf, 0, len);
// 	rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
// 	if (rc != 0) {
// 		printf("Flash read failed! %d\n", rc);
// 		return;
// 	}
//
// 	if (memcmp(expected, buf, len) == 0) {
// 		printf("Data read matches data written. Good!!\n");
// 	} else {
// 		const uint8_t *wp = expected;
// 		const uint8_t *rp = buf;
// 		const uint8_t *rpe = rp + len;
//
// 		printf("Data read does not match data written!!\n");
// 		while (rp < rpe) {
// 			printf("%08x wrote %02x read %02x %s\n",
// 			       (uint32_t)(SPI_FLASH_TEST_REGION_OFFSET + (rp - buf)),
// 			       *wp, *rp, (*rp == *wp) ? "match" : "MISMATCH");
// 			++rp;
// 			++wp;
// 		}
// 	}
// }
//
// #if defined SPI_FLASH_MULTI_SECTOR_TEST
// void multi_sector_test(const struct device *flash_dev)
// {
// 	const uint8_t expected[] = { 0x55, 0xaa, 0x66, 0x99 };
// 	const size_t len = sizeof(expected);
// 	uint8_t buf[sizeof(expected)];
// 	int rc;
//
// 	printf("\nPerform test on multiple consecutive sectors");
//
// 	/* Write protection needs to be disabled before each write or
// 	 * erase, since the flash component turns on write protection
// 	 * automatically after completion of write and erase
// 	 * operations.
// 	 */
// 	printf("\nTest 1: Flash erase\n");
//
// 	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
// 	 * SPI_FLASH_SECTOR_SIZE = flash size
// 	 * Erase 2 sectors for check for erase of consequtive sectors
// 	 */
// 	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE * 2);
// 	if (rc != 0) {
// 		printf("Flash erase failed! %d\n", rc);
// 	} else {
// 		/* Read the content and check for erased */
// 		memset(buf, 0, len);
// 		size_t offs = SPI_FLASH_TEST_REGION_OFFSET;
//
// 		while (offs < SPI_FLASH_TEST_REGION_OFFSET + 2 * SPI_FLASH_SECTOR_SIZE) {
// 			rc = flash_read(flash_dev, offs, buf, len);
// 			if (rc != 0) {
// 				printf("Flash read failed! %d\n", rc);
// 				return;
// 			}
// 			if (memcmp(erased, buf, len) != 0) {
// 				printf("Flash erase failed at offset 0x%x got 0x%x\n",
// 				offs, *(uint32_t *)buf);
// 				return;
// 			}
// 			offs += SPI_FLASH_SECTOR_SIZE;
// 		}
// 		printf("Flash erase succeeded!\n");
// 	}
//
// 	printf("\nTest 2: Flash write\n");
//
// 	size_t offs = SPI_FLASH_TEST_REGION_OFFSET;
//
// 	while (offs < SPI_FLASH_TEST_REGION_OFFSET + 2 * SPI_FLASH_SECTOR_SIZE) {
// 		printf("Attempting to write %zu bytes at offset 0x%x\n", len, offs);
// 		rc = flash_write(flash_dev, offs, expected, len);
// 		if (rc != 0) {
// 			printf("Flash write failed! %d\n", rc);
// 			return;
// 		}
//
// 		memset(buf, 0, len);
// 		rc = flash_read(flash_dev, offs, buf, len);
// 		if (rc != 0) {
// 			printf("Flash read failed! %d\n", rc);
// 			return;
// 		}
//
// 		if (memcmp(expected, buf, len) == 0) {
// 			printf("Data read matches data written. Good!!\n");
// 		} else {
// 			const uint8_t *wp = expected;
// 			const uint8_t *rp = buf;
// 			const uint8_t *rpe = rp + len;
//
// 			printf("Data read does not match data written!!\n");
// 			while (rp < rpe) {
// 				printf("%08x wrote %02x read %02x %s\n",
// 					(uint32_t)(offs + (rp - buf)),
// 					*wp, *rp, (*rp == *wp) ? "match" : "MISMATCH");
// 				++rp;
// 				++wp;
// 			}
// 		}
// 		offs += SPI_FLASH_SECTOR_SIZE;
// 	}
// }
// #endif
//
// int main(void)
// {
// 	const struct device *flash_dev = DEVICE_DT_GET_ONE(SPI_FLASH_COMPAT);
//
// 	if (!device_is_ready(flash_dev)) {
// 		printk("%s: device not ready.\n", flash_dev->name);
// 		return 0;
// 	}
//
// 	printf("\n%s SPI flash testing\n", flash_dev->name);
// 	printf("==========================\n");
//
// 	single_sector_test(flash_dev);
// #if defined SPI_FLASH_MULTI_SECTOR_TEST
// 	multi_sector_test(flash_dev);
// #endif
// 	return 0;
// }
