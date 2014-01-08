/* drivers/misc/hscdtd00x/hscd_bma222e.c
*
* Accelerometer device driver for I2C
*
* Copyright (C) 2011-2012 ALPS ELECTRIC CO., LTD. All Rights Reserved.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/input.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include "alps-input.h"
#define I2C_RETRIES                5
#define ON                1
#define OFF                0
#define I2C_RETRY_DELAY	5

/* Register Name for BMA222E */
#define BMA222E_XOUT		0x02
#define BMA222E_YOUT		0x04
#define BMA222E_ZOUT		0x06
#define BMA222E_TEMP		0x08
#define BMA222E_REG0B		0x0B
#define BMA222E_REG0A		0x0A
#define BMA222E_REG0F		0X0F
#define BMA222E_REG10		0x10
#define BMA222E_REG11		0x11
#define BMA222E_REG14		0x14
#define SOFT_RESEET			0xB6
#define RUNNING_INPUT_EVENT 1
#define STOP_INPUT_EVENT	0
#define SUSPEND_MODE_RETURN 1
static int g_flgptr;
/* Bandwidth */
#define BANDWIDTH_07_81		0x08
#define BANDWIDTH_15_63		0x09
#define BANDWIDTH_31_25		0x0A
#define BANDWIDTH_62_50		0x0B
#define BANDWIDTH_125		0x0C
#define BANDWIDTH_250		0x0D

#define BMA222E_ACC_LSB__POS           4
#define BMA222E_ACC_LSB__LEN           0
#define BMA222E_ACC_LSB__MSK           0xF0

#define BMA222E_ACC_MSB__POS           0
#define BMA222E_ACC_MSB__LEN           8
#define BMA222E_ACC_MSB__MSK           0xFF

#define BMA222E_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)
/** \endcond */

#define CALIBRATION_FILE_PATH	"/data/misc/sensor/calibration_data"
#define CALIBRATION_DATA_AMOUNT	100

#define BMA222E_DRIVER_NAME "bma222e"
#define I2C_BMA222E_ADDR	(0x18)
#define BMA222E_CHIP_ID		0xF8
#define WHO_AM_I		0x00

#define I2C_BUS_NUMBER	6

struct acc_data {
	int x;
	int y;
	int z;
};

struct bma222e_data *bma222e_data;

static void bma222e_early_suspend(struct early_suspend *handler);
static void bma222e_early_resume(struct early_suspend *handler);
#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend bma222e_early_suspend_handler = {
	.suspend = bma222e_early_suspend,
	.resume = bma222e_early_resume,
};
#endif

struct acc_data caldata;
struct i2c_client *this_client;

static struct i2c_driver bma222e_driver;

static atomic_t g_flgena;
static atomic_t delay;

static int bma222e_i2c_writem(u8 *txdata, int length)
{
	int err;
	int tries = 0;
	struct i2c_msg msg[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = length,
		 .buf = txdata,
		 },
	};

	do {
		err = i2c_transfer(this_client->adapter, msg, 1);
	} while ((err != 1) && (++tries < I2C_RETRIES));

	if (err != 1) {
		dev_err(&this_client->adapter->dev,
			"write transfer error [%d]\n", err);
		err = -EIO;
	} else
		err = 0;

	return err;

}
static int bma222e_i2c_readm(u8 *rxdata, int length)
{
	int err;
	int tries = 0;
	struct i2c_msg msgs[] = {
		{
		 .addr = this_client->addr,
		 .flags = 0,
		 .len = 1,
		 .buf = rxdata,
		 },
		{
		 .addr = this_client->addr,
		 .flags = I2C_M_RD,
		 .len = length,
		 .buf = rxdata,
		 },
	};

	do {
		err = i2c_transfer(this_client->adapter, msgs, 2);
	} while ((err != 2) && (++tries < I2C_RETRIES));

	if (err != 2) {
		dev_err(&this_client->adapter->dev, "read transfer error\n");
		err = -EIO;
	} else
		err = 0;

	return err;
}

int accsns_get_acceleration_data(int *xyz)
{
	u8 buf[6];
	int err, idx;

	if (this_client == NULL) {
		xyz[0] = xyz[1] = xyz[2] = 0;
		return -ENODEV;
	}

	buf[0] = BMA222E_XOUT;
	err = bma222e_i2c_readm(buf, 6);
	if (err < 0)
		return err;

	for (idx = 0; idx < 3; idx++) {
		xyz[idx] = (int)((s8) buf[2 * idx + 1]);
		xyz[idx] = (xyz[idx] * 4);
	}
	xyz[0] -= (caldata.x << 2);
	xyz[1] -= (caldata.y << 2);
	xyz[2] -= (caldata.z << 2);

	return err;
}

static int bma222e_get_acceleration_rawdata(int *xyz)
{
	u8 buf[6];
	int err, idx;

	buf[0] = BMA222E_XOUT;
	err = bma222e_i2c_readm(buf, 6);
	if (err < 0)
		return err;

	for (idx = 0; idx < 3; idx++)
		xyz[idx] = (int)((s8) buf[2 * idx + 1]);

	return err;
}

void accsns_activate(int flgatm, int flg, int dtime)
{
	u8 buf[2];

	int reg = 0;

	if (flg != 0) {
		flg = 1;
		buf[0] = BMA222E_REG14;
		buf[1] = SOFT_RESEET;
		bma222e_i2c_writem(buf, 2);
		msleep(20);
	}

	buf[0] = BMA222E_REG0F;
	buf[1] = 0x03;		/*g-range +/-2g */
	bma222e_i2c_writem(buf, 2);

	buf[0] = BMA222E_REG10;
	buf[1] = BANDWIDTH_15_63;
	bma222e_i2c_writem(buf, 2);

	bma222e_i2c_readm(buf, 1);
	reg = (int)((s8) buf[0]);

	if (flg == 0) {
		buf[1] = 0x80;	/*sleep */
	} else {
		if (g_flgptr == STOP_INPUT_EVENT)
			buf[1] = 0x80;
		else
			buf[1] = 0x00;
	}
	bma222e_i2c_writem(buf, 2);

	if (flgatm) {
		atomic_set(&g_flgena, flg);
		atomic_set(&delay, dtime);
	}
}
EXPORT_SYMBOL(accsns_activate);

static int accel_open_calibration(void)
{
	int err = 0;
	mm_segment_t old_fs;
	struct file *cal_filp = NULL;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cal_filp)) {
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);

		caldata.x = 0;
		caldata.y = 0;
		caldata.z = 0;

		return err;
	}

	err = cal_filp->f_op->read(cal_filp,
				   (char *)&caldata, 3 * sizeof(int),
				   &cal_filp->f_pos);
	if (err != 3 * sizeof(int))
		err = -EIO;

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	if ((caldata.x == 0xffff) && (caldata.y == 0xffff)
	    && (caldata.z == 0xffff)) {
		caldata.x = 0;
		caldata.y = 0;
		caldata.z = 0;

		return -1;
	}

	pr_info("%s: %d, %d, %d\n", __func__, caldata.x, caldata.y, caldata.z);
	return err;
}

static int accel_do_calibrate(int enable)
{
	int data[3] = { 0, };
	int sum[3] = { 0, };
	int err = 0, cnt;
	struct file *cal_filp = NULL;
	mm_segment_t old_fs;

	if (enable) {
		for (cnt = 0; cnt < CALIBRATION_DATA_AMOUNT; cnt++) {
			err = bma222e_get_acceleration_rawdata(data);
			if (err < 0) {
				pr_err("%s:failed in the %dth loop\n",
				       __func__, cnt);
				return err;
			}

			sum[0] += data[0];
			sum[1] += data[1];
			sum[2] += (data[2] - 64);
		}

		caldata.x = (sum[0] / CALIBRATION_DATA_AMOUNT);
		caldata.y = (sum[1] / CALIBRATION_DATA_AMOUNT);
		caldata.z = (sum[2] / CALIBRATION_DATA_AMOUNT);
	} else {
		caldata.x = 0xffff;
		caldata.y = 0xffff;
		caldata.z = 0xffff;
	}

	pr_info("%s: cal data (%d,%d,%d)\n", __func__,
	       caldata.x, caldata.y, caldata.z);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH,
			     O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (IS_ERR(cal_filp)) {
		pr_err("%s: Can't open calibration file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		return err;
	}

	err = cal_filp->f_op->write(cal_filp,
				    (char *)&caldata, 3 * sizeof(int),
				    &cal_filp->f_pos);
	if (err != 3 * sizeof(int)) {
		pr_err("%s: Can't write the cal data to file\n", __func__);
		err = -EIO;
	}

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	return err;
}

static ssize_t accel_calibration_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	int err;
	int count = 0;

	err = accel_open_calibration();

	if (err < 0)
		pr_err("%s: accel_open_calibration() failed\n", __func__);

	pr_info("%d %d %d %d\n", err, caldata.x, caldata.y, caldata.z);

	count = sprintf(buf, "%d %d %d %d\n",
			err, caldata.x, caldata.y, caldata.z);
	return count;
}

static ssize_t accel_calibration_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
	int err;
	int64_t enable;

	err = kstrtoll(buf, 10, &enable);
	if (err < 0)
		return err;
	err = accel_do_calibrate((int)enable);
	if (err < 0)
		pr_err("%s: accel_do_calibrate() failed\n", __func__);

	if (!enable) {
		caldata.x = 0;
		caldata.y = 0;
		caldata.z = 0;
	}
	return size;
}

static ssize_t raw_data_read(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	int xyz[3] = { 0, };
	accsns_get_acceleration_data(xyz);
	return snprintf(buf, PAGE_SIZE, "%d,%d,%d\n",
			-(xyz[1] >> 2), (xyz[0] >> 2), (xyz[2] >> 2));

}

static ssize_t raw_data_write(struct device *dev,
			      struct device_attribute *attr, const char *buf,
			      size_t size)
{
	pr_info("raw_data_write is work");
	return size;
}

static DEVICE_ATTR(raw_data, 0644, raw_data_read, raw_data_write);
static DEVICE_ATTR(calibration, 0644,
		   accel_calibration_show, accel_calibration_store);

static struct attribute *bma222e_attributes[] = {
	&dev_attr_raw_data.attr,
	&dev_attr_calibration.attr,
	NULL
};

static struct attribute_group bma222e_attr_grp = {
	.attrs = bma222e_attributes,
};

static int bma222e_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int ret = 0;

	this_client = client;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->adapter->dev, "client not i2c capable\n");
		ret = -ENOMEM;
		goto exit;
	}
	/* read chip id */
	ret = i2c_smbus_read_byte_data(this_client, WHO_AM_I);
	pr_info("%s : device ID = 0x%x, reading ID = 0x%x\n", __func__,
		BMA222E_CHIP_ID, ret);
	atomic_set(&g_flgena, 0);
	atomic_set(&delay, 100);
	ret = sysfs_create_group(&client->dev.kobj, &bma222e_attr_grp);
	if (ret < 0)
		pr_err("bma222e: cannot register attr in sys\n");
#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&bma222e_early_suspend_handler);
#endif
	pr_info("%s: success.\n", __func__);
	return 0;

exit:
	this_client = NULL;
	pr_err("%s: failed!\n", __func__);
	return ret;
}

static int bma222e_suspend(struct i2c_client *client, pm_message_t mesg)
{
	if (atomic_read(&g_flgena))
		accsns_activate(0, 0, atomic_read(&delay));

	return 0;
}

static int bma222e_resume(struct i2c_client *client)
{
	if (atomic_read(&g_flgena))
		accsns_activate(0, 1, atomic_read(&delay));

	return 0;
}

static void bma222e_early_suspend(struct early_suspend *handler)
{

	bma222e_suspend(this_client, PMSG_SUSPEND);
}

static void bma222e_early_resume(struct early_suspend *handler)
{
	bma222e_resume(this_client);
}

static const struct i2c_device_id bma222e_id[] = {
	{BMA222E_DRIVER_NAME, 0},
	{}
};

static struct i2c_driver bma222e_driver = {
	.probe = bma222e_probe,
	.id_table = bma222e_id,
	.driver = {
		   .name = BMA222E_DRIVER_NAME,
		   },
};

static int __init bma222e_init(void)
{
	return i2c_add_driver(&bma222e_driver);
}

static void __exit bma222e_exit(void)
{
	i2c_del_driver(&bma222e_driver);
}

module_init(bma222e_init);
module_exit(bma222e_exit);

MODULE_DESCRIPTION("Alps acc Device");
MODULE_AUTHOR("ALPS");
MODULE_LICENSE("GPL v2");
