#include <zephyr/kernel.h>
#include "display.h"

#include <zephyr/logging/log.h>

#include "drivers/bt81x/bt81x.h"
#include "drivers/bt81x/bt81x_copro.h"
#include "drivers/bt81x/bt81x_dl.h"


/* touch tags */
#define TAG_PLUS 1
#define TAG_MINUS 2
#define TAG_BUTTON 3


LOG_MODULE_REGISTER(display, CONFIG_DISPLAY_LOG_LEVEL);

static volatile bool process_touch;

static void touch_irq(void)
{
    process_touch = true;
}

void display_run()
{
    int cnt;
    int val;

    struct bt81x_touch_transform tt = {64890, 542, 868120, 3450, 63153, -243158};

    bt81x_touch_transform_set(&tt);

    /* Get interrupts on touch event */
    bt81x_register_int(touch_irq);

    /* Starting counting */
    val = 0;
    cnt = 0;

    while (1)
    {
        /* Start Display List */
        bt81x_copro_cmd_dlstart();
        bt81x_copro_cmd(FT8XX_CLEAR_COLOR_RGB(0x00, 0x00, 0x00));
        bt81x_copro_cmd(FT8XX_CLEAR(1, 1, 1));

        /* Set color */
        bt81x_copro_cmd(FT8XX_COLOR_RGB(0xf0, 0xf0, 0xf0));

        // /* Display the counter */
        // bt81x_copro_cmd_number(20, 20, 29, FT8XX_OPT_SIGNED, cnt);
        // cnt++;
        //
        // /* Display the hello message */
        // bt81x_copro_cmd_text(20, 70, 30, 0, "Hello,");
        // /* Set Zephyr color */
        // bt81x_copro_cmd(FT8XX_COLOR_RGB(0x78, 0x29, 0xd2));
        // bt81x_copro_cmd_text(20, 105, 28, 0, "Zephyr!");
        //
        // /* Display value set with buttons */
        // bt81x_copro_cmd(FT8XX_COLOR_RGB(0xff, 0xff, 0xff));
        // bt81x_copro_cmd_number(80, 170, 29,
        //                        FT8XX_OPT_SIGNED | FT8XX_OPT_RIGHTX,
        //                        val);
        // bt81x_copro_cmd(FT8XX_TAG(TAG_PLUS));
        // bt81x_copro_cmd_text(90, 160, 31, 0, "+");
        // bt81x_copro_cmd(FT8XX_TAG(TAG_MINUS));
        // bt81x_copro_cmd_text(20, 160, 31, 0, "-");

        bt81x_copro_cmd_button(20, 200, 140, 100, 31, 0, "PRESS!");

        /* Finish Display List */
        bt81x_copro_cmd(FT8XX_DISPLAY());
        /* Display created frame */
        bt81x_copro_cmd_swap();

        if (process_touch)
        {
            int tag = bt81x_get_touch_tag();

            if (tag == TAG_PLUS)
            {
                val++;
            }
            else if (tag == TAG_MINUS)
            {
                val--;
            }

            process_touch = false;
        }
        k_sleep(K_MSEC(100));
    }
}
