/* arch/arm/mach-msm/board-sony_shinano-nfc.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/qpnp/pin.h>
#include <mach/board-nfc.h>

#define WAIT_HW_CONFIG_ENABLED	100
#define WAIT_GPIO_TOGGLE	10

int board_nfc_parse_dt(struct device *dev,
			 struct pn547_i2c_platform_data *pdata)
{
	struct device_node *np = dev->of_node;
	int ret = 0;

	pdata->dynamic_config = of_property_read_bool(np, "dynamic_config");
	if (pdata->dynamic_config) {
		ret = of_get_named_gpio(np, "configure_gpio", 0);
		if (ret < 0) {
			dev_err(dev, "failed to get \"configure_gpio\"\n");
			goto err;
		}
		pdata->configure_gpio = ret;

		ret = of_get_named_gpio(np, "configure_mpp", 0);
		if (ret < 0) {
			dev_err(dev, "failed to get \"configure_mpp\"\n");
			goto err;
		}
		pdata->configure_mpp = ret;
	}

err:
	return ret;
}

int board_nfc_hw_lag_check(struct i2c_client *d,
			struct pn547_i2c_platform_data *pdata)
{
	struct qpnp_pin_cfg mpp_din_config = {
		.mode = QPNP_PIN_MODE_DIG_IN,
		.invert = QPNP_PIN_INVERT_DISABLE,
		.vin_sel = QPNP_PIN_VIN0,
		.src_sel = QPNP_PIN_SEL_FUNC_CONSTANT,
		.master_en = QPNP_PIN_MASTER_ENABLE,
	};
	struct qpnp_pin_cfg mpp_dout_config = {
		.mode = QPNP_PIN_MODE_DIG_OUT,
		.output_type = QPNP_PIN_OUT_BUF_CMOS,
		.invert = QPNP_PIN_INVERT_DISABLE,
		.vin_sel = QPNP_PIN_VIN2,
		.out_strength = QPNP_PIN_OUT_STRENGTH_HIGH,
		.src_sel = QPNP_PIN_SEL_FUNC_CONSTANT,
		.master_en = QPNP_PIN_MASTER_ENABLE,
	};

	int src_val, new_val, ret = 0;
	dev_dbg(&d->dev, "%s: hw lag check start\n", __func__);

	ret = gpio_request(pdata->configure_mpp, "pn547_mpp");
	if (ret) {
		dev_err(&d->dev, "%s: failed gpio_request(mpp) %d\n",
			__func__, ret);
		goto err_gpio_request_mpp;
	}
	ret = gpio_request(pdata->configure_gpio, "pn547_gpio");
	if (ret) {
		dev_err(&d->dev, "%s: failed gpio_request(gpio) %d\n",
			__func__, ret);
		goto err_gpio_request_gpio;
	}

	/* change mpp config */
	ret = qpnp_pin_config(pdata->configure_mpp, &mpp_din_config);
	if (ret) {
		dev_err(&d->dev, "%s: failed config mpp(din) %d\n",
			__func__, ret);
		goto err_pin_config_din;
	}

	/* wait for switching mpp config */
	msleep(WAIT_HW_CONFIG_ENABLED);

	src_val = gpio_get_value_cansleep(pdata->configure_mpp);
	dev_dbg(&d->dev, "%s: mpp value (src=%d)\n", __func__, src_val);

	/* toggle gpio */
	gpio_set_value_cansleep(pdata->configure_gpio, 1);
	msleep(WAIT_GPIO_TOGGLE);
	gpio_set_value_cansleep(pdata->configure_gpio, 0);
	msleep(WAIT_GPIO_TOGGLE);

	new_val = gpio_get_value_cansleep(pdata->configure_mpp);
	dev_dbg(&d->dev, "%s: mpp value (new=%d)\n", __func__, new_val);

	/* toggle gpio */
	gpio_set_value_cansleep(pdata->configure_gpio, 1);
	msleep(WAIT_GPIO_TOGGLE);
	gpio_set_value_cansleep(pdata->configure_gpio, 0);
	msleep(WAIT_GPIO_TOGGLE);

	if (src_val != new_val) {
		dev_dbg(&d->dev, "%s: no matched device\n", __func__);
		ret = -ENODEV;
	} else {
		dev_dbg(&d->dev, "%s: matched device\n", __func__);
		ret = qpnp_pin_config(pdata->configure_mpp, &mpp_dout_config);
		if (ret)
			dev_err(&d->dev, "%s: failed config mpp(dout) %d\n",
				__func__, ret);
		/* wait for switching mpp config */
		msleep(WAIT_HW_CONFIG_ENABLED);
	}
err_pin_config_din:
	gpio_free(pdata->configure_gpio);
err_gpio_request_gpio:
	gpio_free(pdata->configure_mpp);
err_gpio_request_mpp:
	dev_dbg(&d->dev, "%s: hw lag check end\n", __func__);
	return ret;
}
