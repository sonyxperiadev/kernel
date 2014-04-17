#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/lsm303dlh_mag.h>
#include "board-kivu.h"

#define SENSOR_I2C_BUS_ID 2

static struct regulator *mag_regulator;

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

static int power_on_mag(struct device *dev)
{
	int rc;

	dev_dbg(dev, "%s\n", __func__);

	rc = regulator_enable_handler(mag_regulator, __func__);
	return rc;
}

static int power_off_mag(struct device *dev)
{
	int rc;

	dev_dbg(dev, "%s\n", __func__);

	rc = regulator_disable_handler(mag_regulator, __func__);
	return rc;
}

static int power_config_mag(struct device *dev, bool enable)
{
	int rc;

	dev_dbg(dev, "%s enable = %d\n", __func__, enable);

	rc = platform_power_config(dev, enable, &mag_regulator, "gpldo1_uc");

	return rc;
}

static struct lsm303dlh_mag_platform_data lsm303dlh_mag_platform_data = {
	.range = LSM303_RANGE_8200mG,
	.poll_interval_ms = 100,
	.power_on = power_on_mag,
	.power_off = power_off_mag,
	.power_config = power_config_mag,
};

static struct i2c_board_info __initdata lsm303dlh_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO(LSM303DLH_MAG_DEV_NAME, 0x1E),
		.platform_data = &lsm303dlh_mag_platform_data,
	},
};

void __init kivu_add_magnetometer(void)
{
	pr_info("Registering LSM303DLH Magnetometer with I2C bus #%i",
			SENSOR_I2C_BUS_ID);
	i2c_register_board_info(SENSOR_I2C_BUS_ID,
				lsm303dlh_i2c_boardinfo,
				ARRAY_SIZE(lsm303dlh_i2c_boardinfo));
}
