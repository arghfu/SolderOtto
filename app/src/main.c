#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <app_version.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/flash.h>

#include "wave_control.h"

LOG_MODULE_REGISTER(main);

#define BLINK_PERIOD_MS_STEP 100U
#define BLINK_PERIOD_MS_MAX  1000U

#define SPI_FLASH_TEST_REGION_OFFSET 0xff000

#define SPI_FLASH_SECTOR_SIZE        4096
#define SPI_FLASH_MULTI_SECTOR_TEST
#define SPI_FLASH_COMPAT st_stm32_qspi_nor

static const struct gpio_dt_spec load0_switch =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(load0), gpios, {0});

static const struct gpio_dt_spec load1_switch =
    GPIO_DT_SPEC_GET_OR(DT_NODELABEL(load1), gpios, {0});

static const struct adc_dt_spec adc_handle =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);

static const struct adc_dt_spec adc_leak =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipb);

static const struct adc_dt_spec adc_load =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);

static const struct adc_dt_spec adc_tip_a_temp =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipa);

static const struct adc_dt_spec adc_tip_b_temp =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), ch0_tipb);

static const struct adc_dt_spec adc_t_ambient =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), t_amb);

static const struct adc_dt_spec adc_v_analog =
    ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), v_ana);



static const struct gpio_dt_spec dbg_pins[] = {
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_0), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_1), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_2), gpios),
    GPIO_DT_SPEC_GET(DT_ALIAS(dbg_3), gpios),
};


uint16_t ana_buf;



struct adc_sequence ana_sequence = {
    .buffer = &ana_buf,
    /* buffer size in bytes, not number of samples */
    .buffer_size = sizeof(ana_buf),
};




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
//     // LOG_INF("measurement timer expired, starting work");
//
//     uint64_t time_now = k_cycle_get_64();
//     uint64_t diff = k_cyc_to_us_floor64(time_now - time_last);
//
//     // LOG_INF("measurement time: %"PRId64" us", diff);
//
//     time_last = time_now;
// }


// K_WORK_DEFINE(measure_v_ana_work, measure_v_ana);
//
// K_TIMER_DEFINE(measure_v_ana_timer, sample_timer, NULL);


int main(void)
{
    LOG_INF("Starting Solderotto %s", APP_VERSION_STRING);

    wave_control_init();

    const struct device *flash_dev = DEVICE_DT_GET_ONE(SPI_FLASH_COMPAT);

    if (!device_is_ready(flash_dev)) {
        printk("%s: device not ready.", flash_dev->name);
        return 0;
    }


    // gpio_pin_configure_dt(&load0_switch, GPIO_OUTPUT);
    // gpio_pin_set_dt(&load0_switch, GPIO_PIN_RESET);
    //
    // uint16_t buf;
    //
    // struct adc_sequence sequence = {
    //     .buffer = &buf,
    //     /* buffer size in bytes, not number of samples */
    //     .buffer_size = sizeof(buf),
    // };
    //
    // /* Configure channel individually prior to sampling. */
    // if (!adc_is_ready_dt(&adc_tip_a_temp))
    // {
    //     printk("ADC controller device %s not ready\n", adc_tip_a_temp.dev->name);
    //     return 0;
    // }
    //
    // int err = adc_channel_setup_dt(&adc_tip_a_temp);
    // if (err < 0)
    // {
    //     printk("Could not setup channel Ch0-tip (%d)\n", err);
    //     return 0;
    // }
    //
    //
    // (void)adc_sequence_init_dt(&adc_tip_a_temp, &sequence);


    for (int i = 0; i < ARRAY_SIZE(dbg_pins); i++)
    {
        gpio_pin_configure_dt(&dbg_pins[i], GPIO_OUTPUT |	GPIO_ACTIVE_LOW  | GPIO_OPEN_DRAIN);
        gpio_pin_set_dt(&dbg_pins[i], GPIO_PIN_RESET);
    }

    // (void)adc_sequence_init_dt(&adc_v_analog, &ana_sequence);

    // LOG_INF("Starting measurement timer");
    // k_timer_start(&measure_v_ana_timer, K_MSEC(0), K_MSEC(5));

    while (1)
    {
        // int32_t val_mv;
        //
        // uint64_t begin = k_cycle_get_64();
        // int err = adc_read_dt(&adc_tip_a_temp, &sequence);
        // uint64_t end = k_cycle_get_64();
        // uint64_t diff = k_cyc_to_us_floor64(end - begin);
        //
        // if (err < 0)
        // {
        //     printk("Could not read (%d)\n", err);
        // }


        // LOG_DBG("time to convert: %"PRId64" \n", diff);
        // val_mv = (int32_t)buf;
        //
        // err = adc_raw_to_millivolts_dt(&adc_tip_a_temp,
        //                                &val_mv);
        // /* conversion to mV may not be supported, skip if not */
        // if (err < 0) {
        //     printk(" (value in mV not available)\n");
        // } else {
        //     printk(" = %"PRId32" mV\n", val_mv);
        // }

        k_sleep(K_MSEC(20));
    }

    return 0;
}

