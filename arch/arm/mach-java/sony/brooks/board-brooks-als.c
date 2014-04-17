#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/bh1721fvc.h>
#include "board-brooks.h"

#define SENSOR_I2C_BUS_ID 1

#define BH1721FVC_DRV_NAME     "bh1721fvc"
#define BH1721FVC_RESET_N	10

static int bh1721fvc_reset(void)
{
	if (gpio_request(BH1721FVC_RESET_N, "bh1721fvc_reset_n") < 0) {
		pr_err("can't get bh1721fvc reset GPIO\n");
		return -1;
	}
	gpio_set_value(BH1721FVC_RESET_N, 1);

	return 0;
}

static struct bh1721fvc_platform_data bh1721fvc_pdata = {
	.reset = bh1721fvc_reset,
	.output = NULL
};

static struct i2c_board_info bh1721fvc_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(BH1721FVC_DRV_NAME, 0x23),
		.platform_data = &bh1721fvc_pdata,
	},
};

void __init brooks_add_als(void)
{
	pr_info("Registering %s on I2C bus #%i",
		BH1721FVC_DRV_NAME, SENSOR_I2C_BUS_ID);
	i2c_register_board_info(SENSOR_I2C_BUS_ID,
				bh1721fvc_i2c_boardinfo,
				ARRAY_SIZE(bh1721fvc_i2c_boardinfo));
}
