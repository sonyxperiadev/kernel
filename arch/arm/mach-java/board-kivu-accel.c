#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/lsm303dlhc_acc.h>
#include "board-kivu.h"

#define SENSOR_I2C_BUS_ID 2

static struct regulator *acc_regulator;

static int regulator_enable_handler(struct regulator *r, const char *func_str)
{
	int rc, enabled;

	if (IS_ERR_OR_NULL(r)) {
		rc = r ? PTR_ERR(r) : -EINVAL;
		dev_err(NULL, "%s: regulator invalid",
			func_str ? func_str : "?");
		return rc;
	}

	rc = regulator_enable(r);
	if (!rc)
		return rc;

	enabled = regulator_is_enabled(r);
	if (enabled > 0) {
		dev_warn(NULL, "%s: regulator already enabled",
			func_str ? func_str : "?");
		rc = 0;
	} else if (enabled == 0) {
		dev_err(NULL, "%s: regulator still disabled",
			func_str ? func_str : "?");
	} else {
		dev_err(NULL, "%s: regulator status error %d",
			func_str ? func_str : "?", enabled);
	}
	return rc;
}

static int regulator_disable_handler(struct regulator *r, const char *func_str)
{
	int rc, enabled;

	if (IS_ERR_OR_NULL(r)) {
		rc = r ? PTR_ERR(r) : -EINVAL;
		dev_err(NULL, "%s: regulator invalid",
			func_str ? func_str : "?");
		return rc;
	}

	rc = regulator_disable(r);
	if (!rc)
		return rc;

	enabled = regulator_is_enabled(r);
	if (enabled == 0) {
		dev_warn(NULL, "%s: regulator already disabled",
			func_str ? func_str : "?");
		rc = 0;
	} else if (enabled > 0) {
		dev_err(NULL, "%s: regulator still enabled",
			func_str ? func_str : "?");
	} else {
		dev_err(NULL, "%s: regulator status error %d",
			func_str ? func_str : "?", enabled);
	}
	return rc;
}

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

static int power_config_acc(struct device *dev, bool enable)
{
	int rc;

	dev_info(dev, "%s enable = %d\n", __func__, enable);

	rc = platform_power_config(dev, enable, &acc_regulator, "gpldo1_uc");

	return rc;
}

static int power(struct device *dev, enum lsm303dlhc_acc_power_sate pwr_state)
{
	int rc = -ENOSYS;

	dev_info(dev, "%s pwr_state = %d\n", __func__, pwr_state);

	if (pwr_state == LSM303DLHC_STANDBY)
		goto exit;
	else if (pwr_state == LSM303DLHC_PWR_ON)
		rc = regulator_enable_handler(acc_regulator, __func__);
	else if (pwr_state == LSM303DLHC_PWR_OFF)
		rc = regulator_disable_handler(acc_regulator, __func__);

exit:
	return rc;
}
static struct lsm303dlhc_acc_platform_data lsm303dlhc_acc_platform_data = {
	.range = 2,
	.poll_interval_ms = 100,
	.mode = MODE_POLL,
	.irq_pad = 75,
	.power = power,
	.power_config = power_config_acc,
};

static struct i2c_board_info __initdata lsm303dlhc_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(LSM303DLHC_ACC_DEV_NAME, 0x19),
		.platform_data = &lsm303dlhc_acc_platform_data,
	},
};

void __init kivu_add_accel(void)
{
	pr_info("Registering LSM303DLHC with I2C bus #%i", SENSOR_I2C_BUS_ID);
	i2c_register_board_info(SENSOR_I2C_BUS_ID,
				lsm303dlhc_i2c_boardinfo,
				ARRAY_SIZE(lsm303dlhc_i2c_boardinfo));
}
