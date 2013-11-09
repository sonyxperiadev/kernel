#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/apds9702.h>
#include "board-kivu.h"

#define SENSOR_I2C_BUS_ID 2

#define APDS9702_PROX_INT_GPIO 81

static int platform_power_config(struct device *dev, bool enable,
			struct regulator **regulator, char *regulator_id)
{
	int rc = 0;
	dev_dbg(dev, "%s\n", __func__);

	if (enable) {
		if (*regulator == NULL) {
			dev_dbg(dev, "%s: get regulator %s\n",
							__func__, regulator_id);
			*regulator = regulator_get(NULL, regulator_id);
			if (IS_ERR(*regulator)) {
				rc = PTR_ERR(*regulator);
				dev_err(dev, "%s: Failed to get regulator %s\n",
							__func__, regulator_id);
				return rc;
			}
		}
		rc = regulator_set_voltage(*regulator, 2800000, 2800000);
		if (rc) {
			dev_err(dev, "%s: unable to set voltage rc = %d!\n",
					__func__, rc);
			goto exit;
		}
	} else {
		goto exit;
	}

	return rc;

exit:
	regulator_put(*regulator);
	*regulator = NULL;
	return rc;
}

static struct regulator *prox_regulator;
static void power_config_prox(struct device *dev, int enable)
{
	int rc;
	bool enable_bool = 0;

	dev_dbg(dev, "%s enable = %d\n", __func__, enable);
	if (enable)
		enable_bool = 1;

	rc = platform_power_config(dev, enable_bool, &prox_regulator,
			"gpldo1_uc");

	dev_dbg(dev, "%s platform_power_config returned %d\n", __func__, rc);
}

static int apds9702_setup(struct device *dev, int request)
{
	int rc;
	if (request) {
		rc = gpio_request(APDS9702_PROX_INT_GPIO, "apds9702_dout");
		if (rc) {
				dev_err(dev, "%s: failed to request gpio %d\n",
					__func__, APDS9702_PROX_INT_GPIO);
			return rc;
		}
		return 0;
	}
	rc = 0;
	gpio_free(APDS9702_PROX_INT_GPIO);
	return rc;

}

static struct apds9702_platform_data apds9702_pdata = {
	.gpio_dout      = APDS9702_PROX_INT_GPIO,
	.is_irq_wakeup  = 1,
	.hw_config      = power_config_prox,
	.gpio_setup     = apds9702_setup,
	.ctl_reg = {
		.trg   = 1,
		.pwr   = 1,
		.burst = 7,
		.frq   = 3,
		.dur   = 2,
		.th    = 15,
		.rfilt = 0,
	},
	.phys_dev_path = "/sys/devices/i2c-2/2-0054"
};
static struct i2c_board_info __initdata apds9702_i2c_boardinfo[] = {
	{
		/* Proximity (0xA8 >>1 = 0x54)*/
		I2C_BOARD_INFO(APDS9702_NAME, 0xA8 >> 1),
		.platform_data = &apds9702_pdata,
	},
};


void __init kivu_add_proximity(void)
{
	pr_info("Registering APDS-9702 with I2C bus #%i", SENSOR_I2C_BUS_ID);
	i2c_register_board_info(SENSOR_I2C_BUS_ID,
				apds9702_i2c_boardinfo,
				ARRAY_SIZE(apds9702_i2c_boardinfo));
}
