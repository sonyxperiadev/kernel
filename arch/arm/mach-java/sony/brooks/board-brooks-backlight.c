#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/led-lm3530.h>
#include "board-brooks.h"

#define BMU_NFC_I2C_BUS_ID 1

static struct lm3530_platform_data lm3530_bmu_platform_data = {
	.mode = LM3530_BL_MODE_MANUAL,
	.als_input_mode = LM3530_INPUT_ALS2,
	.max_current = LM3530_FS_CURR_26mA,
	.pwm_pol_hi = true,
	.als_avrg_time = LM3530_ALS_AVRG_TIME_512ms,
	.brt_ramp_law = 1,      /* Linear */
	.brt_ramp_fall = LM3530_RAMP_TIME_1s,
	.brt_ramp_rise = LM3530_RAMP_TIME_1s,
	.als1_resistor_sel = LM3530_ALS_IMPD_Z,
	.als2_resistor_sel = LM3530_ALS_IMPD_Z,
	.als_vmin = 730,        /* mV */
	.als_vmax = 1020,       /* mV */
	.brt_val = 0x7F,        /* Max brightness */
};

static struct i2c_board_info __initdata lm3530_i2c_boardinfo[] = {
	{
		/* Backlight */
		I2C_BOARD_INFO("lm3530-led", 0x36),
		.platform_data = &lm3530_bmu_platform_data,
	},
};

void __init kivu_add_backlight(void)
{
	pr_info("Registering LM3530 BMU with I2C bus #%i", BMU_NFC_I2C_BUS_ID);
	i2c_register_board_info(BMU_NFC_I2C_BUS_ID,
				lm3530_i2c_boardinfo,
				ARRAY_SIZE(lm3530_i2c_boardinfo));
}
