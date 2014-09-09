#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_data/lm3630a_bl.h>
#include <video/kona_fb.h>
#include "board-brooks.h"

#define BMU_NFC_I2C_BUS_ID 1
#define BMU_HW_EN 24

static struct lm3630a_first_frame_ctrl first_frame_data = {
	.register_notifier = &kona_fb_register_notifier,
	.notifier_event = KONA_FB_EVENT_FIRST_FRAME,
	.backlight_value = 75,
	.ramp_on_off = 0x8,
	.leda_ctrl = true,
	.ledb_ctrl = false,
};

static struct lm3630a_platform_data lm3630a_bmu_platform_data = {
	.leda_init_brt = 128,
	.leda_max_brt = 255,
	.leda_ctrl = LM3630A_LEDA_ENABLE_LINEAR,
	.ledb_ctrl = LM3630A_LEDB_DISABLE,
	.pwm_ctrl = LM3630A_PWM_BANK_A,
	.pwm_filter_strength = 0x03,
	.a_max_curr = 0x14,
	.b_max_curr = B_MAX_CURR_DEFAULT,
	.ramp_on_off = 0x0,
	.ramp_run = 0x0,
	.enable_gpio = BMU_HW_EN,
	.first_frame_ctrl = &first_frame_data,
};

static struct i2c_board_info __initdata lm3630a_i2c_boardinfo[] = {
	{
		/* Backlight */
		I2C_BOARD_INFO(LM3630A_NAME, 0x36),
		.platform_data = &lm3630a_bmu_platform_data,
	},
};

void __init brooks_add_backlight(void)
{
	pr_info("Registering LM3630A BMU with I2C bus #%i\n",
							BMU_NFC_I2C_BUS_ID);

	i2c_register_board_info(BMU_NFC_I2C_BUS_ID,
				lm3630a_i2c_boardinfo,
				ARRAY_SIZE(lm3630a_i2c_boardinfo));
}
