#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include "board-kivu.h"

#define AKM_I2C_NAME "akm09911"
#define SENSOR_I2C_BUS_ID 2

static struct i2c_board_info __initdata akm09911_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(AKM_I2C_NAME, 0x0C),
	},
};

void __init brooks_add_compass(void)
{
	pr_info("Registering %s eCompass with I2C bus #%i",
			AKM_I2C_NAME,
			SENSOR_I2C_BUS_ID);
	i2c_register_board_info(SENSOR_I2C_BUS_ID,
				akm09911_i2c_boardinfo,
				ARRAY_SIZE(akm09911_i2c_boardinfo));
}
