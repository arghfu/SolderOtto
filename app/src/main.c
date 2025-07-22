// #include <zephyr/drivers/gpio.h>
// #include <zephyr/logging/log.h>
// #include <app_version.h>
// #include <zephyr/drivers/adc.h>
// #include <zephyr/drivers/flash.h>
//
// LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);
//
// #define BLINK_PERIOD_MS_STEP 100U
// #define BLINK_PERIOD_MS_MAX  1000U
//
// static const struct gpio_dt_spec load0_switch =
//     GPIO_DT_SPEC_GET_OR(DT_NODELABEL(load_0), gpios, {0});
//
// static const struct gpio_dt_spec load1_switch =
//     GPIO_DT_SPEC_GET_OR(DT_NODELABEL(load_1), gpios, {0});
//
// static const struct gpio_dt_spec zcd =
//     GPIO_DT_SPEC_GET_OR(DT_NODELABEL(zcd0), gpios, {0});
//
// static const struct adc_dt_spec adc_t_ambient =
//     ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), t_amb);
//
// static const struct adc_dt_spec adc_v_analog =
//     ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), v_ana);
//
// static const struct adc_dt_spec adc_handle =
//     ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);
//
// static const struct adc_dt_spec adc_leak =
//     ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipb);
//
// static const struct adc_dt_spec adc_load =
//     ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);
//
// static const struct adc_dt_spec adc_tip_a_temp =
//     ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);
//
// static const struct adc_dt_spec adc_tip_b_temp =
//     ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipb);
//
// static const struct gpio_dt_spec dbg0_pin = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_0), gpios);
// static const struct gpio_dt_spec dbg1_pin = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_1), gpios);
// static const struct gpio_dt_spec dbg2_pin = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_2), gpios);
// static const struct gpio_dt_spec dbg3_pin = GPIO_DT_SPEC_GET(DT_ALIAS(dbg_3), gpios);
//
//
// struct gpio_callback zcd_cb_data;
//
// uint16_t ana_buf;
//
// struct half_wave_control {
//     uint16_t ton;
//     uint16_t tperiod;
//     uint16_t count;
// };
//
// struct half_wave_control control = {
//     .ton = 1,
//     .tperiod = 100,
//     .count = 0
// };
//
// struct adc_sequence ana_sequence = {
//     .buffer = &ana_buf,
//     /* buffer size in bytes, not number of samples */
//     .buffer_size = sizeof(ana_buf),
// };
//
// void zcd_callback(const struct device* dev,
//                   struct gpio_callback* cb, uint32_t pins)
// {
//     static uint64_t time_last = 0;
//     int state = gpio_pin_get_dt(&zcd);
//     uint64_t time_now = k_cycle_get_64();
//
//     uint64_t diff = time_now - time_last;
//     time_last = time_now;
//
//     if (state == GPIO_PIN_RESET)
//     {
//
//         if (control.count < control.ton)
//         {
//             gpio_pin_set_dt(&dbg0_pin, GPIO_PIN_SET);
//         }
//         else
//         {
//             gpio_pin_set_dt(&dbg0_pin, GPIO_PIN_RESET);
//         }
//         ++control.count;
//     }
//     else
//     {
//         gpio_pin_set_dt(&dbg0_pin, GPIO_PIN_RESET);
//     }
//
//
//     if (control.count >= control.tperiod)
//     {
//         control.count = 0;
//     }
//
//     diff = k_cyc_to_us_floor64(diff);
//
//     LOG_DBG("half wave time: %"PRId64" us", diff);
// }
//
//
// void measure_v_ana(struct k_work* work)
// {
//     int32_t val_mv;
//
//     int err = adc_read_dt(&adc_v_analog, &ana_sequence);
//     if (err < 0)
//     {
//         LOG_ERR("Could not read (%d)", err);
//         return;
//     }
//
//     val_mv = (int32_t)ana_buf;
//
//     err = adc_raw_to_millivolts_dt(&adc_v_analog, &val_mv);
//     if (err < 0)
//     {
//         LOG_WRN("Conversion to mV not available");
//     }
//     else
//     {
//         LOG_INF("Analog voltage: %"PRId32" mV", val_mv);
//     }
// }
//
// void sample_timer(struct k_timer* timer)
// {
//     static uint64_t time_last;
//     LOG_INF("measurement timer expired, starting work");
//
//     uint64_t time_now = k_cycle_get_64();
//     uint64_t diff = k_cyc_to_us_floor64(time_now - time_last);
//
//     LOG_INF("measurement time: %"PRId64" us", diff);
//
//     time_last = time_now;
// }
//
//
// K_WORK_DEFINE(measure_v_ana_work, measure_v_ana);
//
// K_TIMER_DEFINE(measure_v_ana_timer, sample_timer, NULL);
//
//
// int main(void)
// {
//     LOG_INF("Device Ready");
//     LOG_INF("Starting Solderotto %s", APP_VERSION_STRING);
//
//     // configure button pin as input
//     gpio_pin_configure_dt(&zcd, GPIO_INPUT);
//     gpio_pin_interrupt_configure_dt(&zcd, GPIO_INT_EDGE_BOTH);
//     gpio_init_callback(&zcd_cb_data, zcd_callback, BIT(zcd.pin));
//     gpio_add_callback_dt(&zcd, &zcd_cb_data);
//
//     gpio_pin_configure_dt(&dbg0_pin, GPIO_OUTPUT |	GPIO_ACTIVE_LOW  | GPIO_OPEN_DRAIN);
//     gpio_pin_set_dt(&dbg0_pin, GPIO_PIN_RESET);
//
//     gpio_pin_configure_dt(&dbg1_pin, GPIO_OUTPUT |	GPIO_ACTIVE_LOW  | GPIO_OPEN_DRAIN);
//     gpio_pin_set_dt(&dbg1_pin, GPIO_PIN_RESET);
//
//     gpio_pin_configure_dt(&dbg2_pin, GPIO_OUTPUT |	GPIO_ACTIVE_LOW  | GPIO_OPEN_DRAIN);
//     gpio_pin_set_dt(&dbg2_pin, GPIO_PIN_RESET);
//
//     gpio_pin_configure_dt(&dbg3_pin, GPIO_OUTPUT |	GPIO_ACTIVE_LOW  | GPIO_OPEN_DRAIN);
//     gpio_pin_set_dt(&dbg3_pin, GPIO_PIN_RESET);
//
//     uint16_t buf;
//
//     struct adc_sequence sequence = {
//         .buffer = &buf,
//         /* buffer size in bytes, not number of samples */
//         .buffer_size = sizeof(buf),
//     };
//
//     /* Configure channel individually prior to sampling. */
//     if (!adc_is_ready_dt(&adc_tip_a_temp))
//     {
//         printk("ADC controller device %s not ready\n", adc_tip_a_temp.dev->name);
//         return 0;
//     }
//
//     int err = adc_channel_setup_dt(&adc_tip_a_temp);
//     if (err < 0)
//     {
//         printk("Could not setup channel Ch0-tip (%d)\n", err);
//         return 0;
//     }
//
//
//     (void)adc_sequence_init_dt(&adc_tip_a_temp, &sequence);
//
//
//     if (!adc_is_ready_dt(&adc_v_analog))
//     {
//         LOG_ERR("ADC controller device %s not ready", adc_v_analog.dev->name);
//         return 0;
//     }
//
//     err = adc_channel_setup_dt(&adc_v_analog);
//     if (err < 0)
//     {
//         LOG_ERR("Could not setup channel (%d)", err);
//         return 0;
//     }
//
//     (void)adc_sequence_init_dt(&adc_v_analog, &ana_sequence);
//
//     LOG_INF("Starting measurement timer");
//     k_timer_start(&measure_v_ana_timer, K_MSEC(0), K_MSEC(5));
//
//     while (1)
//     {
//         int32_t val_mv;
//
//         uint64_t begin = k_cycle_get_64();
//         int err = adc_read_dt(&adc_tip_a_temp, &sequence);
//         uint64_t end = k_cycle_get_64();
//         uint64_t diff = k_cyc_to_us_floor64(end - begin);
//
//         if (err < 0)
//         {
//             printk("Could not read (%d)\n", err);
//         }
//
//
//         LOG_DBG("time to convert: %"PRId64" \n", diff);
//         val_mv = (int32_t)buf;
//
//         err = adc_raw_to_millivolts_dt(&adc_tip_a_temp,
//                                        &val_mv);
//         // /* conversion to mV may not be supported, skip if not */
//         // if (err < 0) {
//         //     printk(" (value in mV not available)\n");
//         // } else {
//         //     printk(" = %"PRId32" mV\n", val_mv);
//         // }
//
//         k_sleep(K_MSEC(20));
//     }
//
//     return 0;
// }
/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <string.h>

#if defined(CONFIG_BOARD_ADAFRUIT_FEATHER_STM32F405)
#define SPI_FLASH_TEST_REGION_OFFSET 0xf000
#elif defined(CONFIG_BOARD_ARTY_A7_DESIGNSTART_FPGA_CORTEX_M1) || \
	defined(CONFIG_BOARD_ARTY_A7_DESIGNSTART_FPGA_CORTEX_M3)
/* The FPGA bitstream is stored in the lower 536 sectors of the flash. */
#define SPI_FLASH_TEST_REGION_OFFSET \
	DT_REG_SIZE(DT_NODE_BY_FIXED_PARTITION_LABEL(fpga_bitstream))
#elif defined(CONFIG_BOARD_NPCX9M6F_EVB) || \
	defined(CONFIG_BOARD_NPCX7M6FB_EVB)
#define SPI_FLASH_TEST_REGION_OFFSET 0x7F000
#elif defined(CONFIG_BOARD_EK_RA8M1) || defined(CONFIG_BOARD_EK_RA8D1)
#define SPI_FLASH_TEST_REGION_OFFSET 0x40000
#else
#define SPI_FLASH_TEST_REGION_OFFSET 0xff000
#endif
#if defined(CONFIG_BOARD_EK_RA8M1) || defined(CONFIG_BOARD_EK_RA8D1)
#define SPI_FLASH_SECTOR_SIZE 262144
#else
#define SPI_FLASH_SECTOR_SIZE        4096
#endif

#if defined(CONFIG_FLASH_STM32_OSPI) || defined(CONFIG_FLASH_STM32_QSPI) ||                        \
	defined(CONFIG_FLASH_STM32_XSPI) || defined(CONFIG_FLASH_RENESAS_RA_OSPI_B)

#define SPI_FLASH_MULTI_SECTOR_TEST
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(jedec_spi_nor)
#define SPI_FLASH_COMPAT jedec_spi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(jedec_mspi_nor)
#define SPI_FLASH_COMPAT jedec_mspi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_qspi_nor)
#define SPI_FLASH_COMPAT st_stm32_qspi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_ospi_nor)
#define SPI_FLASH_COMPAT st_stm32_ospi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_xspi_nor)
#define SPI_FLASH_COMPAT st_stm32_xspi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_qspi_nor)
#define SPI_FLASH_COMPAT nordic_qspi_nor
#elif DT_HAS_COMPAT_STATUS_OKAY(renesas_ra_ospi_b_nor)
#define SPI_FLASH_COMPAT renesas_ra_ospi_b_nor
#else
#define SPI_FLASH_COMPAT invalid
#endif

#if defined(CONFIG_FLASH_RENESAS_RA_OSPI_B)
const uint8_t erased[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#else
const uint8_t erased[] = { 0xff, 0xff, 0xff, 0xff };
#endif

void single_sector_test(const struct device *flash_dev)
{
#if defined(CONFIG_FLASH_RENESAS_RA_OSPI_B)
	const uint8_t expected[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
#else
	const uint8_t expected[] = { 0x55, 0xaa, 0x66, 0x99 };
#endif
	const size_t len = sizeof(expected);
	uint8_t buf[sizeof(expected)];
	int rc;

	printf("\nPerform test on single sector");
	/* Write protection needs to be disabled before each write or
	 * erase, since the flash component turns on write protection
	 * automatically after completion of write and erase
	 * operations.
	 */
	printf("\nTest 1: Flash erase\n");

	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			 SPI_FLASH_SECTOR_SIZE);
	if (rc != 0) {
		printf("Flash erase failed! %d\n", rc);
	} else {
		/* Check erased pattern */
		memset(buf, 0, len);
		rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
		if (rc != 0) {
			printf("Flash read failed! %d\n", rc);
			return;
		}
		if (memcmp(erased, buf, len) != 0) {
			printf("Flash erase failed at offset 0x%x got 0x%x\n",
				SPI_FLASH_TEST_REGION_OFFSET, *(uint32_t *)buf);
			return;
		}
		printf("Flash erase succeeded!\n");
	}
	printf("\nTest 2: Flash write\n");

	printf("Attempting to write %zu bytes\n", len);
	rc = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, expected, len);
	if (rc != 0) {
		printf("Flash write failed! %d\n", rc);
		return;
	}

	memset(buf, 0, len);
	rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
	if (rc != 0) {
		printf("Flash read failed! %d\n", rc);
		return;
	}

	if (memcmp(expected, buf, len) == 0) {
		printf("Data read matches data written. Good!!\n");
	} else {
		const uint8_t *wp = expected;
		const uint8_t *rp = buf;
		const uint8_t *rpe = rp + len;

		printf("Data read does not match data written!!\n");
		while (rp < rpe) {
			printf("%08x wrote %02x read %02x %s\n",
			       (uint32_t)(SPI_FLASH_TEST_REGION_OFFSET + (rp - buf)),
			       *wp, *rp, (*rp == *wp) ? "match" : "MISMATCH");
			++rp;
			++wp;
		}
	}
}

#if defined SPI_FLASH_MULTI_SECTOR_TEST
void multi_sector_test(const struct device *flash_dev)
{
#if defined(CONFIG_FLASH_RENESAS_RA_OSPI_B)
	const uint8_t expected[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
#else
	const uint8_t expected[] = { 0x55, 0xaa, 0x66, 0x99 };
#endif
	const size_t len = sizeof(expected);
	uint8_t buf[sizeof(expected)];
	int rc;

	printf("\nPerform test on multiple consecutive sectors");

	/* Write protection needs to be disabled before each write or
	 * erase, since the flash component turns on write protection
	 * automatically after completion of write and erase
	 * operations.
	 */
	printf("\nTest 1: Flash erase\n");

	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 * Erase 2 sectors for check for erase of consequtive sectors
	 */
	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, SPI_FLASH_SECTOR_SIZE * 2);
	if (rc != 0) {
		printf("Flash erase failed! %d\n", rc);
	} else {
		/* Read the content and check for erased */
		memset(buf, 0, len);
		size_t offs = SPI_FLASH_TEST_REGION_OFFSET;

		while (offs < SPI_FLASH_TEST_REGION_OFFSET + 2 * SPI_FLASH_SECTOR_SIZE) {
			rc = flash_read(flash_dev, offs, buf, len);
			if (rc != 0) {
				printf("Flash read failed! %d\n", rc);
				return;
			}
			if (memcmp(erased, buf, len) != 0) {
				printf("Flash erase failed at offset 0x%x got 0x%x\n",
				offs, *(uint32_t *)buf);
				return;
			}
			offs += SPI_FLASH_SECTOR_SIZE;
		}
		printf("Flash erase succeeded!\n");
	}

	printf("\nTest 2: Flash write\n");

	size_t offs = SPI_FLASH_TEST_REGION_OFFSET;

	while (offs < SPI_FLASH_TEST_REGION_OFFSET + 2 * SPI_FLASH_SECTOR_SIZE) {
		printf("Attempting to write %zu bytes at offset 0x%x\n", len, offs);
		rc = flash_write(flash_dev, offs, expected, len);
		if (rc != 0) {
			printf("Flash write failed! %d\n", rc);
			return;
		}

		memset(buf, 0, len);
		rc = flash_read(flash_dev, offs, buf, len);
		if (rc != 0) {
			printf("Flash read failed! %d\n", rc);
			return;
		}

		if (memcmp(expected, buf, len) == 0) {
			printf("Data read matches data written. Good!!\n");
		} else {
			const uint8_t *wp = expected;
			const uint8_t *rp = buf;
			const uint8_t *rpe = rp + len;

			printf("Data read does not match data written!!\n");
			while (rp < rpe) {
				printf("%08x wrote %02x read %02x %s\n",
					(uint32_t)(offs + (rp - buf)),
					*wp, *rp, (*rp == *wp) ? "match" : "MISMATCH");
				++rp;
				++wp;
			}
		}
		offs += SPI_FLASH_SECTOR_SIZE;
	}
}
#endif

int main(void)
{
	const struct device *flash_dev = DEVICE_DT_GET_ONE(SPI_FLASH_COMPAT);

	if (!device_is_ready(flash_dev)) {
		printk("%s: device not ready.\n", flash_dev->name);
		return 0;
	}

	printf("\n%s SPI flash testing\n", flash_dev->name);
	printf("==========================\n");

	single_sector_test(flash_dev);
#if defined SPI_FLASH_MULTI_SECTOR_TEST
	multi_sector_test(flash_dev);
#endif
	return 0;
}