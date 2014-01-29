#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_data/lm3630_bl.h>
#include "board-kivu.h"


#define BMU_I2C_BUS_ID 1
#define BMU_HW_EN 24

static struct lm3630_platform_data lm3630_bmu_platform_data = {
	.max_brt_led1 = 255,
	.max_brt_led2 = 255,
	.init_brt_led1 = 128,
	.init_brt_led2 = 128,
	.pwm_ctrl = PWM_CTRL_BANK_B,
	.pwm_active = PWM_ACTIVE_HIGH,
	.bank_a_ctrl = BANK_A_CTRL_LED1,
	.bank_b_ctrl = BANK_B_CTRL_DISABLE,
};


static struct i2c_board_info __initdata lm3630_i2c_boardinfo[] = {
	{
		/* Backlight */
		I2C_BOARD_INFO("lm3630_bl", 0x36),
		.platform_data = &lm3630_bmu_platform_data,
	},
};

void __init kivu_add_backlight(void)
{
	pr_info("Registering LM3630 BMU with I2C bus #%i", BMU_I2C_BUS_ID);
	i2c_register_board_info(BMU_I2C_BUS_ID,
				lm3630_i2c_boardinfo,
				ARRAY_SIZE(lm3630_i2c_boardinfo));
}
