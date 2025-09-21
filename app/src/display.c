#include <zephyr/kernel.h>
#include "display.h"

#include <zephyr/logging/log.h>

#include "comms.h"
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

struct button_data
{
    bool pressed;
    const char* text;
    uint32_t fg_color;
    uint32_t grad_color;
};

void display_run()
{
    struct bt81x_touch_transform tt = {64890, 542, 868120, 3450, 63153, -243158};

    int set_point = 300;
    int temp = 100;
    bt81x_touch_transform_set(&tt);

    /* Get interrupts on touch event */
    bt81x_register_int(touch_irq);

    struct button_data enable_button = {false, "Off", 0x00c040, 0x000000};
    /* Invert display Orientation */

    bt81x_copro_cmd_simple(CMD_SETROTATE, 1);
    while (1)
    {
        struct display_msg msg;
        // Receive message from control thread (non-blocking)
        if (k_msgq_get(&control_msgq, &msg, K_NO_WAIT) == 0)
        {
            temp = (int)msg.temperature;
            set_point = (int)msg.set_point;
        }

        if (process_touch)
        {
            int tag = bt81x_get_touch_tag();

            if (tag == TAG_PLUS)
            {
                set_point++;
            }
            else if (tag == TAG_MINUS)
            {
                set_point--;
            }
            else if (tag == TAG_BUTTON)
            {
                enable_button.pressed = enable_button.pressed ? false : true;
                enable_button.text = enable_button.pressed ? "Off" : "On";
                enable_button.fg_color = enable_button.pressed ? 0x00c040 : 0xb90007;
                enable_button.grad_color = enable_button.pressed ? 0x00FF00: 0xFF0000;
            }

            process_touch = false;
        }
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
        bt81x_copro_cmd_number(120, 20, 31, FT8XX_OPT_SIGNED | FT8XX_OPT_RIGHTX, set_point);
        bt81x_copro_cmd_number(120, 80, 31, FT8XX_OPT_SIGNED | FT8XX_OPT_RIGHTX, temp);

        bt81x_copro_cmd(FT8XX_TAG(TAG_BUTTON));
        bt81x_copro_cmd_simple(CMD_FGCOLOR, enable_button.fg_color);
        bt81x_copro_cmd_simple(CMD_GRADCOLOR, enable_button.grad_color);
        bt81x_copro_cmd_button(20, 160, 140, 80, 31, 0, enable_button.text);


        /* Finish Display List */
        bt81x_copro_cmd(FT8XX_DISPLAY());
        /* Display created frame */
        bt81x_copro_cmd_swap();


        k_sleep(K_MSEC(20));
    }
}