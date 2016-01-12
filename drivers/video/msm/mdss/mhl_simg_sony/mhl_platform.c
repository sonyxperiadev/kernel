/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/regulator/consumer.h>

#include "mhl_platform.h"

struct clk *mhl_clk;

static enum {
	CHIP_PWR_ON,
	CHIP_PWR_OFF
} chip_pwr_state;

/*device name*/
static const char *device_name;

static int mhl_pf_clock_enable(void);
static void mhl_pf_clock_disable(void);

#define MHL_VCC "mhl_vcc"
static struct regulator *vreg_mhl_vcc;

static int mhl_pf_regulator_handler(struct regulator *regulator,
								struct device *dev,
								const char *func_str,
								const char *reg_str,
								int sw)
{
	int rc, enabled;

	if (IS_ERR_OR_NULL(regulator)) {
		rc = regulator ? PTR_ERR(regulator) : -EINVAL;
		pr_err("%s: regulator '%s' invalid",
			func_str ? func_str : "?",
			reg_str ? reg_str : "?");
		goto exit;
	}

	if (sw)
		rc = regulator_enable(regulator);
	else
		rc = regulator_disable(regulator);
	if (rc) {
		enabled = regulator_is_enabled(regulator);
		pr_err("%s: regulator '%s' status is %d",
			func_str ? func_str : "?",
			reg_str ? reg_str : "?",
			enabled);
		if ((!enabled && !sw) || (enabled > 0 && sw))
			rc = 0;
	}
exit:
	return rc;
}

static int mhl_pf_vreg_configure(struct device *dev, int enable)
{
	int rc = 0;
	if (enable) {
		pr_err("%s: enable\n", __func__);

		vreg_mhl_vcc = regulator_get(dev, MHL_VCC);
		if (IS_ERR(vreg_mhl_vcc)) {
			pr_err("%s: get vcc failed\n", __func__);
			rc = -ENODEV;
			goto err_ret;
		}

		rc = mhl_pf_regulator_handler(vreg_mhl_vcc, dev, __func__, MHL_VCC, 1);
		if (rc) {
			pr_err("%s: err_put_vdd\n", __func__);
			goto err_put_vdd;
		}

		rc = regulator_set_optimum_mode(vreg_mhl_vcc, 343000);
		if (rc < 0) {
			pr_err("%s: set vcc mode failed, rc=%d\n", __func__, rc);
			goto err_disable_vcc;
		}

	} else {
		pr_err("%s: disable\n", __func__);
		if (!IS_ERR(vreg_mhl_vcc)) {
			mhl_pf_regulator_handler(vreg_mhl_vcc, dev, __func__, MHL_VCC, 0);
			regulator_put(vreg_mhl_vcc);
		}
	}
	return rc;

err_disable_vcc:
	mhl_pf_regulator_handler(vreg_mhl_vcc, dev, __func__, MHL_VCC, 0);
err_put_vdd:
	regulator_put(vreg_mhl_vcc);
err_ret:
	return rc;
}

int is_interrupt_asserted(void)
{
	int int_gpio = mhl_pf_get_gpio_num_int();
	return gpio_get_value(int_gpio) ? 0 : 1;
}
EXPORT_SYMBOL(is_interrupt_asserted);

const char *mhl_pf_get_device_name(void)
{
	return device_name;
}
EXPORT_SYMBOL(mhl_pf_get_device_name);

bool mhl_pf_is_chip_power_on(void)
{
	if (chip_pwr_state == CHIP_PWR_ON)
		return true;
	else
		return false;
}
EXPORT_SYMBOL(mhl_pf_is_chip_power_on);

void mhl_pf_chip_power_on(void)
{
	int fw_wake_gpio = 0;
	int rst_gpio = 0;
	int mhl_clock_gpio = 0;

	int mhl_vreg_rc = 0;
	struct i2c_client *client;

	pr_debug("%s()\n", __func__);

	/* turn on main power for the MHL chip */
	if (chip_pwr_state == CHIP_PWR_OFF) {
		client = mhl_pf_get_i2c_client();
		mhl_vreg_rc = mhl_pf_vreg_configure(&client->dev, 1);
	}

	/* the state is referred by i2c function.
	 * So the state must be changed at first.
	 * If it is PWR_OFF, all i2c access fails.
	 */
	chip_pwr_state = CHIP_PWR_ON;

	/* enable the clock supply */
	mhl_clock_gpio = mhl_pf_get_gpio_num_mhl_clock();
	gpio_set_value(mhl_clock_gpio, 1);
	mhl_pf_clock_enable();

	/* 20 msec is enough to make the clock stable
	 * udelay might be better for less than 10 msec,
	 * however it blocks process, so avoiding it.
	 * (250 micro sec wait is the spec to make stable clock)
	 */
	msleep(20);

	/* Assert FW_WAKE */
	fw_wake_gpio = mhl_pf_get_gpio_num_fw_wake();
	gpio_set_value(fw_wake_gpio, 1);

	/* Assert RESET */
	/*
	 * probably, the 0 might not be needed since it is already low at probe.
	 * However, just incase, 0 has been set here.
	 */
	rst_gpio = mhl_pf_get_gpio_num_rst();
	gpio_set_value(rst_gpio, 0);
	msleep(20);
	gpio_set_value(rst_gpio, 1);
}
EXPORT_SYMBOL(mhl_pf_chip_power_on);


void mhl_pf_chip_power_off(void)
{
	int fw_wake_gpio = 0;
	int rst_gpio = 0;
	int mhl_vreg_rc = 0;
	int mhl_clock_gpio = 0;
	struct i2c_client *client;

	pr_debug("%s()\n", __func__);

	/* de-assert FW_WAKE */
	fw_wake_gpio = mhl_pf_get_gpio_num_fw_wake();
	gpio_set_value(fw_wake_gpio, 0);

	/* turn off main power for the MHL chip */
	if (chip_pwr_state == CHIP_PWR_ON) {
		client = mhl_pf_get_i2c_client();
		mhl_vreg_rc = mhl_pf_vreg_configure(&client->dev, 0);
	}

	/* de-assert RESET */
	rst_gpio = mhl_pf_get_gpio_num_rst();
	gpio_set_value(rst_gpio, 0);

	/* disable the clock supply */
	mhl_pf_clock_disable();
	mhl_clock_gpio = mhl_pf_get_gpio_num_mhl_clock();
	gpio_set_value(mhl_clock_gpio, 0);

	/* the state is referred by i2c function.
	 * So the state must be changed at the end.
	 */
	chip_pwr_state = CHIP_PWR_OFF;

}
EXPORT_SYMBOL(mhl_pf_chip_power_off);

static int mhl_pf_clock_enable(void)
{
	int rc = -1;
	struct mhl_tx_ctrl *mhl_ctrl;
	struct i2c_client *client;

	pr_debug("%s()\n", __func__);

	client = mhl_pf_get_i2c_client();
	if (!client) {
		pr_err("%s: cannot get i2c_client\n", __func__);
		return -EBUSY;
	}

	mhl_ctrl = i2c_get_clientdata(client);

	if (!mhl_ctrl->pdev)
		return -EBUSY;

	mhl_clk = mhl_pf_get_mhl_clk();
	if (!mhl_clk) {
		mhl_clk = clk_get(mhl_ctrl->pdev, "mhl_clk");
		if (!mhl_clk) {
			pr_err("%s: mhl clk is null\n", __func__);
			return -EBUSY;
		}
	}

	rc = clk_prepare_enable(mhl_clk);
	if (rc) {
		pr_err("%s: invalid clk prepare, rc : %d\n", __func__, rc);
		mhl_clk = NULL;
		return -EBUSY;
	}

	pr_debug("%s:clk is enabled\n", __func__);

	return 0;
}

static void mhl_pf_clock_disable(void)
{
	pr_debug("%s()\n", __func__);
	if (mhl_clk) {
		clk_disable_unprepare(mhl_clk);
		pr_debug("%s:clk is disabled\n", __func__);
		mhl_clk = NULL;
	}
}

int mhl_pf_init(void)
{
	struct mhl_tx_ctrl *mhl_ctrl;
	int rc = -1;
	struct i2c_client *client;

	pr_debug("%s:\n", __func__);

	chip_pwr_state = CHIP_PWR_OFF;

	client = mhl_pf_get_i2c_client();
	if (!client) {
		pr_err("%s: FAILED: cannot get i2c_client\n", __func__);
		goto failed_i2c_get_error;
	}

	mhl_ctrl = devm_kzalloc(&client->dev, sizeof(*mhl_ctrl), GFP_KERNEL);

	if (!mhl_ctrl) {
		pr_err("%s: FAILED: cannot alloc hdmi tx ctrl\n", __func__);
		rc = -ENOMEM;
		goto failed_no_mem;
	}

	mhl_ctrl->i2c_handle = client;
	mhl_ctrl->mhlclass = class_create(THIS_MODULE, "mhl");

	if (IS_ERR(mhl_ctrl->mhlclass)) {
		pr_err("%s:fail class creation\n", __func__);
		rc = PTR_ERR(mhl_ctrl->mhlclass);
		goto failed_error;
	}

	mhl_ctrl->pdev = device_create(mhl_ctrl->mhlclass,
					NULL, 0, NULL, "hdcp_state");
	pr_debug("%s:class name : %s\n", __func__, mhl_ctrl->pdev->class->name);

	i2c_set_clientdata(client, mhl_ctrl);

	mhl_pf_i2c_init(client->adapter);
	device_name = client->dev.driver->name;
	pr_debug("%s:device name : %s\n", __func__, device_name);

	return 0;

failed_error:

failed_no_mem:
	if (mhl_ctrl)
		devm_kfree(&client->dev, mhl_ctrl);

failed_i2c_get_error:

	return rc;
}

void mhl_pf_exit(void)
{
	struct mhl_tx_ctrl *mhl_ctrl;
	struct i2c_client *client;

	pr_debug("%s:\n", __func__);

	client = mhl_pf_get_i2c_client();
	if (!client) {
		pr_err("%s: cannot get i2c_client\n", __func__);
		return;
	}

	mhl_ctrl = i2c_get_clientdata(client);

	/*
	 * MHL driver sw module release
	 */
	/* release clock */
	if (mhl_clk) {
		pr_debug("%s: disable mhl clk\n", __func__);
		clk_disable_unprepare(mhl_clk);
		clk_put(mhl_clk);
	}
	mhl_pf_switch_to_usb();

	/* mhl device class is released */
	class_destroy(mhl_ctrl->mhlclass);

	if (!mhl_ctrl)
		pr_err("%s: i2c get client data failed\n", __func__);
	else
		devm_kfree(&client->dev, mhl_ctrl);
}
