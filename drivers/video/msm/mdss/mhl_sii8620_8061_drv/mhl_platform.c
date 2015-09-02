/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_platform.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/err.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include "mhl_platform.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_common.h"
#include "si_8620_regs.h"
#include "mhl_tx.h"
#include "mhl_lib_timer.h"
#include "mhl_cbus_control.h"

static struct clk *mhl_clk;

static enum {
	CHIP_PWR_ON,
	CHIP_PWR_OFF
} chip_pwr_state;

/*device name*/
static const char *device_name;

/*device*/
static struct device *pdev;

static int mhl_pf_clock_enable(void);
static void mhl_pf_clock_disable(void);

int is_interrupt_asserted(void)
{
	int int_gpio = mhl_pf_get_gpio_num_int();
	return gpio_get_value(int_gpio) ? 0 : 1;
}

const char *mhl_pf_get_device_name(void)
{
	return device_name;
}

/*
 * Return a value indicating how upstream HPD is
 * implemented on this platform.
 */
hpd_control_mode platform_get_hpd_control_mode(void)
{
	return HPD_CTRL_PUSH_PULL;
}

bool mhl_pf_is_chip_power_on(void)
{
	if (chip_pwr_state == CHIP_PWR_ON)
		return true;
	else
		return false;
}

void mhl_pf_chip_power_on(void)
{
	int pwr_gpio = 0;
	int fw_wake_gpio = 0;
	int rst_gpio = 0;

	pr_debug("%s()\n", __func__);

	/* the state is referred by i2c function.
	 * So the state must be changed at first.
	 * If it is PWR_OFF, all i2c access fails.
	 */
	chip_pwr_state = CHIP_PWR_ON;

	/* enable the clock supply */
	mhl_pf_clock_enable();

	/* 20 msec is enough to make the clock stable
	 * udelay might be better for less than 10 msec,
	 * however it blocks process, so avoiding it.
	 * (250 micro sec wait is the spec to make stable clock)
	 */
	msleep(20);

	/* turn on main power for the MHL chip */
	pwr_gpio = mhl_pf_get_gpio_num_pwr();
	gpio_set_value(pwr_gpio, 1);

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

	/* Active Power Control, then go into D2 mode */
	mhl_pf_write_reg(REG_PAGE_0_DPD, 0xFE);

	/* NOTE : following power ctrl is not enough to access i2c. */
	/* mhl_pf_write_reg(REG_PAGE_0_DPD, 0x10); */


}


void mhl_pf_chip_power_off(void)
{
	int pwr_gpio = 0;
	int fw_wake_gpio = 0;
	int rst_gpio = 0;

	pr_debug("%s()\n", __func__);

	/* device goes into low power mode */
	mhl_pf_write_reg(REG_PAGE_0_DPD, 0x10);

	/* de-assert FW_WAKE */
	fw_wake_gpio = mhl_pf_get_gpio_num_fw_wake();
	gpio_set_value(fw_wake_gpio, 0);

	/* turn off main power for the MHL chip */
	pwr_gpio = mhl_pf_get_gpio_num_pwr();
	gpio_set_value(pwr_gpio, 0);

	/* de-assert RESET */
	rst_gpio = mhl_pf_get_gpio_num_rst();
	gpio_set_value(rst_gpio, 0);

	/* disable the clock supply */
	mhl_pf_clock_disable();

	/* the state is referred by i2c function.
	 * So the state must be changed at the end.
	 */
	chip_pwr_state = CHIP_PWR_OFF;

}

static int mhl_pf_clock_enable(void)
{
	int rc = -1;

	pr_debug("%s()\n", __func__);


	if (!pdev)
		return -EBUSY;

	mhl_clk = mhl_pf_get_mhl_clk();
	if (!mhl_clk) {
		mhl_clk = clk_get(pdev, "");
		if (!mhl_clk) {
			pr_err("%s: mhl clk is null\n", __func__);
			return -EBUSY;
		}
	}

	rc = clk_prepare(mhl_clk);
	if (rc) {
		pr_err("%s: invalid clk prepare, rc : %d\n", __func__, rc);
		return -EBUSY;
	}

	clk_enable(mhl_clk);
	pr_debug("%s:clk is enabled\n", __func__);

	return 0;
}

static void mhl_pf_clock_disable(void)
{
	pr_debug("%s()\n", __func__);
	if (mhl_clk) {
		clk_disable_unprepare(mhl_clk);
		pr_debug("%s:clk is disabled\n", __func__);
	}
}

static int __init mhl_pf_init(void)
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

	pdev = &client->dev;
	if (!pdev) {
		pr_err("%s: FAILED: cannot get pdev\n", __func__);
		goto failed_error;
	}

	pdev->class = class_create(THIS_MODULE, "mhl");
	if (IS_ERR(pdev->class)) {
		pr_err("%s:fail class creation\n", __func__);
		rc = PTR_ERR(pdev->class);
		goto failed_error;
	}
	pr_debug("%s:class name : %s\n", __func__, pdev->class->name);
	i2c_set_clientdata(client, mhl_ctrl);

	mhl_pf_i2c_init(client->adapter);
	device_name = client->dev.driver->name;
	pr_debug("%s:device name : %s\n", __func__, device_name);

	/*
	 * libs should be initizlized first
	 * since there could be used by other module
	 */
	rc = mhl_lib_timer_init();
	if (rc < 0)
		goto failed_error;

	mhl_device_initialize(&client->dev);
	mhl_tx_rcp_init(pdev);
	rc = mhl_tx_initialize();
	if (rc < 0)
		goto failed_error;

	return 0;

failed_error:

failed_no_mem:
	if (mhl_ctrl)
		devm_kfree(&client->dev, mhl_ctrl);

failed_i2c_get_error:

	return rc;
}

static void __exit mhl_pf_exit(void)
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
	mhl_tx_rcp_release();
	mhl_pf_switch_to_usb();
	mhl_device_release(&client->dev);
	mhl_tx_release();
	/* libs should be release at the end
	 * since there could be used by other sw module */
	mhl_lib_timer_release();

	/* mhl device class is released */
	class_destroy(client->dev.class);

	if (!mhl_ctrl)
		pr_err("%s: i2c get client data failed\n", __func__);
	else
		devm_kfree(&client->dev, mhl_ctrl);
}

module_init(mhl_pf_init);
module_exit(mhl_pf_exit);
MODULE_LICENSE("GPL");
