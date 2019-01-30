/*
 *  stmvl53l0_module-i2c.c - Linux kernel modules for STM VL53L0 FlightSense TOF
 *							sensor
 *
 *  Copyright (C) 2016 STMicroelectronics Imaging Division.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/platform_device.h>
/*
 * power specific includes
 */
#include <linux/pwm.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
/*
 * API includes
 */
#include "vl53l0_api.h"
#include "vl53l0_def.h"
#include "vl53l0_platform.h"
#include "stmvl53l0-i2c.h"
#include "stmvl53l0-cci.h"
#include "stmvl53l0.h"
#ifndef CAMERA_CCI

/*
 * Global data
 */
static int stmvl53l0_parse_vdd(struct device *dev, struct i2c_data *data);

/*
 * QCOM specific functions
 */
static int stmvl53l0_parse_vdd(struct device *dev, struct i2c_data *data)
{
	int ret = 0;

	vl53l0_dbgmsg("Enter\n");

	if (dev->of_node) {
		data->vana = regulator_get(dev, "vdd");
		if (IS_ERR(data->vana)) {
			vl53l0_errmsg("Cannot get vdd supply: %ld\n",
						PTR_ERR(data->vana));
			return PTR_ERR(data->vana);
		}

		/* Not mandatory */
		data->avdd = regulator_get(dev, "tof_avdd");
		if (IS_ERR(data->avdd)) {
			vl53l0_errmsg("tof_avdd supply not provided\n");
			ret = 0;
		}
	}
	vl53l0_dbgmsg("End\n");

	return ret;
}

static int stmvl53l0_parse_pins(struct device *dev, struct i2c_data *data)
{
	int ret = 0;

	if (dev->of_node) {
		data->rst_gpio = of_get_named_gpio(dev->of_node,
						"tof-reset-gpio", 0);
	}

	data->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(data->pinctrl)) {
		dev_err(dev, "Failed to get pinctrl handle.\n");
		return -EPROBE_DEFER;
	}

	data->pinstate_act = pinctrl_lookup_state(data->pinctrl,
						"tof_irq_active");
	if (IS_ERR(data->pinstate_act)) {
		dev_err(dev, "Cannot lookup active state\n");
		ret = -EPROBE_DEFER;
		goto err_pinact;
	}

	data->pinstate_slp = pinctrl_lookup_state(data->pinctrl,
						"tof_irq_suspend");
	if (IS_ERR(data->pinstate_act)) {
		dev_err(dev, "Cannot lookup sleep state\n");
		ret = -EPROBE_DEFER;
		goto err_pinslp;
	}

	data->pinctrl_avail = true;

	return ret;

err_pinact:
err_pinslp:
	devm_pinctrl_put(data->pinctrl);

	return ret;
}

static int stmvl53l0_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	int rc = 0;
	struct stmvl53l0_data *vl53l0_data = NULL;
	struct i2c_data *i2c_object = NULL;

	vl53l0_dbgmsg("Enter\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE)) {
		rc = -EIO;
		return rc;
	}

	vl53l0_data = kzalloc(sizeof(struct stmvl53l0_data), GFP_KERNEL);
	if (!vl53l0_data) {
		rc = -ENOMEM;
		return rc;
	}
	if (vl53l0_data) {
		vl53l0_data->client_object =
		    kzalloc(sizeof(struct i2c_data), GFP_KERNEL);
		i2c_object = (struct i2c_data *)vl53l0_data->client_object;
	}
	i2c_object->client = client;

	/* setup bus type */
	vl53l0_data->bus_type = I2C_BUS;

	/* setup regulator */
	rc = stmvl53l0_parse_vdd(&i2c_object->client->dev, i2c_object);
	if (rc)
		goto end;

	/* setup pinctrl and gpios */
	rc = stmvl53l0_parse_pins(&i2c_object->client->dev, i2c_object);
	if (rc)
		goto end;

	/* setup device name */
	vl53l0_data->dev_name = dev_name(&client->dev);

	/* setup device data */
	dev_set_drvdata(&client->dev, vl53l0_data);

	/* setup client data */
	i2c_set_clientdata(client, vl53l0_data);

	/* setup other stuff */
	rc = stmvl53l0_setup(vl53l0_data);

	/* init default value */
	i2c_object->power_up = 0;

	vl53l0_dbgmsg("End\n");
end:
	if (rc < 0) {
		kfree(i2c_object);
		kfree(vl53l0_data);
	}

	return rc;
}

static int stmvl53l0_remove(struct i2c_client *client)
{
	struct stmvl53l0_data *data = i2c_get_clientdata(client);

	vl53l0_dbgmsg("Enter\n");

	/* Power down the device */
	stmvl53l0_power_down_i2c(data->client_object);
	stmvl53l0_cleanup(data);
	kfree(data->client_object);
	kfree(data);
	vl53l0_dbgmsg("End\n");
	return 0;
}

static const struct i2c_device_id stmvl53l0_id[] = {
	{STMVL53L0_DRV_NAME, 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, stmvl53l0_id);

static const struct of_device_id st_stmvl53l0_dt_match[] = {
	{.compatible = "st,stmvl53l0",},
	{},
};

static struct i2c_driver stmvl53l0_driver = {
	.driver = {
		   .name = STMVL53L0_DRV_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = st_stmvl53l0_dt_match,
		   },
	.probe = stmvl53l0_probe,
	.remove = stmvl53l0_remove,
	.id_table = stmvl53l0_id,

};

int stmvl53l0_power_up_i2c(void *i2c_object, unsigned int *preset_flag)
{
	int ret = 0;
#ifndef STM_TEST
	struct i2c_data *data = (struct i2c_data *)i2c_object;
#endif

	vl53l0_dbgmsg("Enter\n");

	/* actual power on */
#ifndef STM_TEST
 #ifndef CONFIG_INPUT_STMVL53L0_SOMC_PARAMS
	ret = regulator_set_voltage(data->vana, VL53L0_VDD_MIN, VL53L0_VDD_MAX);
	if (ret < 0) {
		vl53l0_errmsg("set_vol(%p) fail %d\n", data->vana, ret);
		return ret;
	}
 #endif /* INPUT_STMVL53L0_SOMC_PARAMS */
	ret = regulator_enable(data->vana);

	usleep_range(2950, 3000);
	if (ret < 0) {
		vl53l0_errmsg("reg enable(%p) failed.rc=%d\n", data->vana, ret);
 #ifndef CONFIG_INPUT_STMVL53L0_SOMC_PARAMS
		return ret;
 #endif
	}

	if (!IS_ERR_OR_NULL(data->avdd)) {
		ret = regulator_enable(data->avdd);
		if (ret < 0) {
			vl53l0_errmsg("Cannot enable AVDD VREG: %d\n", ret);
			return ret;
		}
		usleep_range(2950, 3000);
	}

	if (data->pinctrl_avail)
		pinctrl_select_state(data->pinctrl, data->pinstate_act);

	/* Deassert RST if GPIO is available */
	if (gpio_is_valid(data->rst_gpio)) {
		gpio_set_value(data->rst_gpio, 1);
		usleep_range(1000, 1500);
	}

	data->power_up = 1;
	*preset_flag = 1;
#endif

	vl53l0_dbgmsg("End\n");
	return ret;
}


int stmvl53l0_i2c_power_status(void *i2c_object)
{
	struct i2c_data *data = (struct i2c_data *)i2c_object;

	return data->power_up;
}

int stmvl53l0_power_down_i2c(void *i2c_object)
{
	int ret = 0;
#ifndef STM_TEST
	struct i2c_data *data = (struct i2c_data *)i2c_object;
#endif

	vl53l0_dbgmsg("Enter\n");
#ifndef STM_TEST
	usleep_range(2950, 3000);
	ret = regulator_disable(data->vana);
	if (ret < 0)
		vl53l0_errmsg("reg disable(%p) failed.rc=%d\n",
			      data->vana, ret);

	/* Assert RST if GPIO is available */
	if (gpio_is_valid(data->rst_gpio))
		gpio_set_value(data->rst_gpio, 0);

	if (data->pinctrl_avail)
		pinctrl_select_state(data->pinctrl, data->pinstate_slp);

	if (!IS_ERR_OR_NULL(data->avdd)) {
		ret = regulator_disable(data->avdd);
		if (ret < 0)
			vl53l0_errmsg("Cannot disable AVDD VREG: %d\n", ret);
	}

	data->power_up = 0;
#endif

	vl53l0_dbgmsg("End\n");
	return ret;
}

int stmvl53l0_init_i2c(void)
{
	int ret = 0;

#ifdef STM_TEST
	struct i2c_client *client = NULL;
	struct i2c_adapter *adapter;
	struct i2c_board_info info = {
		.type = "stmvl53l0",
		.addr = STMVL53L0_SLAVE_ADDR,
	};
#endif

	vl53l0_dbgmsg("Enter\n");

	/* register as a i2c client device */
	ret = i2c_add_driver(&stmvl53l0_driver);
	if (ret)
		vl53l0_errmsg("%d erro ret:%d\n", __LINE__, ret);

#ifdef STM_TEST
	if (!ret) {
		adapter = i2c_get_adapter(4);
		if (!adapter)
			ret = -EINVAL;
		else
			client = i2c_new_device(adapter, &info);
		if (!client)
			ret = -EINVAL;
	}
#endif

	vl53l0_dbgmsg("End with rc:%d\n", ret);

	return ret;
}

void stmvl53l0_exit_i2c(void *i2c_object)
{
	vl53l0_dbgmsg("Enter\n");
	i2c_del_driver(&stmvl53l0_driver);

	vl53l0_dbgmsg("End\n");
}

#endif				/* end of NOT CAMERA_CCI */
