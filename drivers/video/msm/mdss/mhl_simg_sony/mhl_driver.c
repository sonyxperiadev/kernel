/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_driver.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

//#include <linux/kobject.h>
#include <linux/module.h>
//#include <linux/usb/msm_hsusb.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include "mhl_common.h"
#include "mhl_lib_timer.h"
#include "mhl_sii8620_8061_device.h"
#include "mhl_tx_rcp.h"
#include "mhl_tx.h"
#include "mhl_sii8620_bist.h"

static int __init mhl_8620_8061_drv_init(void)
{
	int rc = -1;
	struct i2c_client *client;
	struct mhl_tx_ctrl *mhl_ctrl;

	pr_err("%s: \n", __func__);

	/*
	 * libs should be initizlized first
	 * since there could be used by other module
	 */
	client = mhl_pf_get_i2c_client();
	if (!client) {
		pr_err("%s: FAILED: cannot get i2c_client\n", __func__);
		return rc;
	}

	mhl_ctrl = i2c_get_clientdata(client);

	rc = mhl_lib_timer_init();
	if (rc < 0)
		return rc;

	mhl_device_initialize(mhl_ctrl->pdev);
	mhl_tx_rcp_init(mhl_ctrl->pdev);
	mhl_bist_initilize(mhl_ctrl->pdev);
	rc = mhl_tx_initialize();
	if (rc < 0)
		return rc;

	return 0;
}

static void __exit mhl_8620_8061_drv_exit(void)
{
	struct i2c_client *client;

	pr_info("%s: \n", __func__);

	client = mhl_pf_get_i2c_client();
	if (!client) {
		pr_err("%s: FAILED: cannot get i2c_client\n", __func__);
		return;
	}

	mhl_bist_release();
	mhl_tx_rcp_release();
	mhl_device_release(&client->dev);
	mhl_tx_release();
	/* libs should be release at the end
	 * since there could be used by other sw module */
	mhl_lib_timer_release();
}

module_init(mhl_8620_8061_drv_init);
module_exit(mhl_8620_8061_drv_exit);

MODULE_LICENSE("GPL");
