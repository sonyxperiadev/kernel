#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/bmp18x.h>
#include "board-brooks.h"

#define SENSOR_I2C_BUS_ID 2

static struct bmp18x_platform_data bmp18x_platform_data = {
	.supply_name = "gpldo1_uc",
};

static struct i2c_board_info __initdata bmp18x_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(BMP18X_NAME, 0xEE >> 1),
		.platform_data = &bmp18x_platform_data,
	},
};

void __init kivu_add_barometer(void)
{
	pr_info("Registering BMP180 with I2C bus #%i", SENSOR_I2C_BUS_ID);
	i2c_register_board_info(SENSOR_I2C_BUS_ID,
				bmp18x_i2c_boardinfo,
				ARRAY_SIZE(bmp18x_i2c_boardinfo));
}
