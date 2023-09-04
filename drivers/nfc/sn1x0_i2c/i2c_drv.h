/******************************************************************************
 * Copyright (C) 2015, The Linux Foundation. All rights reserved.
 * Copyright (C) 2019-2021 NXP
 * Copyright 2021 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * NOTE: This file has been modified by Sony Corporation
 * Modifications are licensed under the License.
 ******************************************************************************/
#ifndef _I2C_DRV_H_
#define _I2C_DRV_H_

#include <linux/i2c.h>

/* kept same as dts */
#define NFC_I2C_DRV_STR			"nxp,sn1x0-i2c"
#define NFC_I2C_DRV_NAME		"sn1x0"
#define NFC_I2C_DEV_ID			"pn553"

/* Interface specific parameters */
struct i2c_dev {
	struct i2c_client *client;
	/* IRQ parameters */
	bool irq_enabled;
	spinlock_t irq_enabled_lock;
	/* NFC_IRQ wake-up state */
	bool irq_wake_up;
};

long nfc_i2c_dev_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg);
int nfc_i2c_dev_probe(struct i2c_client *client,
		      const struct i2c_device_id *id);
int nfc_i2c_dev_remove(struct i2c_client *client);
int nfc_i2c_dev_suspend(struct device *device);
int nfc_i2c_dev_resume(struct device *device);

#endif /* _I2C_DRV_H_ */
