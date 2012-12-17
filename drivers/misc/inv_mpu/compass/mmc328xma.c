/*
	$License:
	Copyright (C) 2011 InvenSense Corporation, All Rights Reserved.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	$
 */

/**
 *  @addtogroup COMPASSDL
 *
 *  @{
 *      @file   mmc328x.c
 *      @brief  Magnetometer setup and handling methods for Honeywell
 *              mmc328x compass.
 */

/* -------------------------------------------------------------------------- */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include "mpu-dev.h"

#include <log.h>
#include <linux/mpu.h>
#include "mlsl.h"
#include "mldl_cfg.h"
#undef MPL_LOG_TAG
#define MPL_LOG_TAG "MPL-compass"

/* --------------------- */
/* -    Variables.     - */
/* --------------------- */

static int reset_int = 1000;
static int read_count = 1;

#define MMC328XMA_REG_CTL0            (0x07)
#define MMC328XMA_REG_ST              (0x06)
#define MMC328XMA_REG_X_LSB           (0x00)

#define MMC328XMA_VAL_TM_READY        (0x01)
#define MMC328XMA_VAL_RM_MAG          (0x20)
#define MMC328XMA_VAL_RRM_MAG         (0x40)
#define MMC328XMA_CNTL_MODE_WAKE_UP   (0x01)

/* -------------------------------------------------------------------------- */
static int mmc328x_suspend(void *mlsl_handle,
			   struct ext_slave_descr *slave,
			   struct ext_slave_platform_data *pdata)
{
	int result = INV_SUCCESS;

	return result;
}

static int mmc328x_resume(void *mlsl_handle,
			  struct ext_slave_descr *slave,
			  struct ext_slave_platform_data *pdata)
{
	int result;
	result = inv_serial_single_write(mlsl_handle,
			pdata->address,
			MMC328XMA_REG_CTL0,
			MMC328XMA_VAL_RM_MAG);
	result = inv_serial_single_write(mlsl_handle,
			pdata->address,
			MMC328XMA_REG_CTL0,
			MMC328XMA_CNTL_MODE_WAKE_UP);
	msleep(10);

	read_count = 1;
	return INV_SUCCESS;
}

static int mmc328x_read(void *mlsl_handle,
			struct ext_slave_descr *slave,
			struct ext_slave_platform_data *pdata,
			unsigned char *data)
{
	int result;
	unsigned char status = 0;
	int md_times = 0;
	if (read_count > 1000)
		read_count = 1;


	do {
		result = inv_serial_read(mlsl_handle,
				pdata->address,
				MMC328XMA_REG_ST,
				1, &status);
		if (result) {
			LOG_RESULT_LOCATION(result);
			return result;
		}

		md_times++;
		if (md_times > 3) {
			printk(KERN_INFO "mmc328x tried 3 times, failed");
			return INV_ERROR_COMPASS_DATA_NOT_READY;
		}

		msleep(1);
	} while ((status & MMC328XMA_VAL_TM_READY) != MMC328XMA_VAL_TM_READY);

	result = inv_serial_read(mlsl_handle, pdata->address,
					MMC328XMA_REG_X_LSB,
				6, (unsigned char *)data);
	if (result) {
		LOG_RESULT_LOCATION(result);
		return result;
	}
	{
		short tmp[3];
		unsigned char tmpdata[6];
		int ii;
		for (ii = 0; ii < 6; ii++)
			tmpdata[ii] = data[ii];

		for (ii = 0; ii < 3; ii++) {
			tmp[ii] = (short)((tmpdata[2 * ii + 1] << 8)
						+ tmpdata[2 * ii]);
			tmp[ii] = tmp[ii] - 4096;
			tmp[ii] = tmp[ii] * 16;
		}

		for (ii = 0; ii < 3; ii++) {
			data[2 * ii] = (unsigned char)(tmp[ii] >> 8);
			data[2 * ii + 1] = (unsigned char)(tmp[ii]);
		}
	}
	read_count++;

	if (read_count % reset_int == 0) {
		result = inv_serial_single_write(mlsl_handle,
					pdata->address,
					MMC328XMA_REG_CTL0,
					MMC328XMA_VAL_RM_MAG);
		if (result) {
			LOG_RESULT_LOCATION(result);
			return result;
		}
		msleep(50);
		result = inv_serial_single_write(mlsl_handle,
					pdata->address,
					MMC328XMA_REG_CTL0,
					MMC328XMA_VAL_RRM_MAG);
		if (result) {
			LOG_RESULT_LOCATION(result);
			return result;
		}
		msleep(100);
	}

	result = inv_serial_single_write(mlsl_handle,
			pdata->address,
			MMC328XMA_REG_CTL0,
			MMC328XMA_CNTL_MODE_WAKE_UP);
	if (result) {
		LOG_RESULT_LOCATION(result);
		return result;
	}

	return INV_SUCCESS;
}
static struct ext_slave_descr mmc328x_descr = {
	.init             = NULL,
	.exit             = NULL,
	.suspend          = mmc328x_suspend,
	.resume           = mmc328x_resume,
	.read             = mmc328x_read,
	.config           = NULL,
	.get_config       = NULL,
	.name             = "mmc328x",
	.type             = EXT_SLAVE_TYPE_COMPASS,
	.id               = COMPASS_ID_MMC328X,
	.read_reg         = 0x00,
	.read_len         = 6,
	.endian           = EXT_SLAVE_BIG_ENDIAN,
	.range            = {400, 0},
	.trigger          = NULL,
};

static
struct ext_slave_descr *mmc328x_get_slave_descr(void)
{
	return &mmc328x_descr;
}

/* -------------------------------------------------------------------------- */
struct mmc328x_mod_private_data {
	struct i2c_client *client;
	struct ext_slave_platform_data *pdata;
};

static unsigned short normal_i2c[] = { I2C_CLIENT_END };

static int mmc328x_mod_probe(struct i2c_client *client,
			   const struct i2c_device_id *devid)
{
	struct ext_slave_platform_data *pdata;
	struct mmc328x_mod_private_data *private_data;
	int result = 0;

	dev_info(&client->adapter->dev, "%s: %s\n", __func__, devid->name);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		result = -ENODEV;
		goto out_no_free;
	}

	pdata = client->dev.platform_data;
	if (!pdata) {
		dev_err(&client->adapter->dev,
			"Missing platform data for slave %s\n", devid->name);
		result = -EFAULT;
		goto out_no_free;
	}

	private_data = kzalloc(sizeof(*private_data), GFP_KERNEL);
	if (!private_data) {
		result = -ENOMEM;
		goto out_no_free;
	}

	i2c_set_clientdata(client, private_data);
	private_data->client = client;
	private_data->pdata = pdata;

	result = inv_mpu_register_slave(THIS_MODULE, client, pdata,
					mmc328x_get_slave_descr);
	if (result) {
		dev_err(&client->adapter->dev,
			"Slave registration failed: %s, %d\n",
			devid->name, result);
		goto out_free_memory;
	}

	return result;

out_free_memory:
	kfree(private_data);
out_no_free:
	dev_err(&client->adapter->dev, "%s failed %d\n", __func__, result);
	return result;

}

static int mmc328x_mod_remove(struct i2c_client *client)
{
	struct mmc328x_mod_private_data *private_data =
		i2c_get_clientdata(client);

	dev_dbg(&client->adapter->dev, "%s\n", __func__);

	inv_mpu_unregister_slave(client, private_data->pdata,
				mmc328x_get_slave_descr);

	kfree(private_data);
	return 0;
}

static const struct i2c_device_id mmc328x_mod_id[] = {
	{ "mmc328x", COMPASS_ID_MMC328X },
	{}
};

MODULE_DEVICE_TABLE(i2c, mmc328x_mod_id);

static struct i2c_driver mmc328x_mod_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = mmc328x_mod_probe,
	.remove = mmc328x_mod_remove,
	.id_table = mmc328x_mod_id,
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "mmc328x_mod",
		   },
	.address_list = normal_i2c,
};

static int __init mmc328x_mod_init(void)
{
	int res = i2c_add_driver(&mmc328x_mod_driver);
	pr_info("%s: Probe name %s\n", __func__, "mmc328x_mod");
	if (res)
		pr_err("%s failed\n", __func__);
	return res;
}

static void __exit mmc328x_mod_exit(void)
{
	pr_info("%s\n", __func__);
	i2c_del_driver(&mmc328x_mod_driver);
}

module_init(mmc328x_mod_init);
module_exit(mmc328x_mod_exit);

MODULE_AUTHOR("Invensense Corporation");
MODULE_DESCRIPTION("Driver to integrate mmc328x sensor with the MPU");
MODULE_LICENSE("GPL");
MODULE_ALIAS("mmc328x_mod");

/**
 *  @}
 */
