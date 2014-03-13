/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
 *
 * File Name         : mmc328xma.c
 * Original File Name    : lsm303dlh_mag_char.c
 * Original Authors  : MH - C&I BU - Application Team
 *               : Carmine Iascone (carmine.iascone@st.com)
 *               : Matteo Dameno (matteo.dameno@st.com)
 * Original Version  : V 0.3
 * Original File Date    : 24/02/2010
 *
 *******************************************************************************
 *
 * Original STMicroelectronics Notice:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
 * PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
 * AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
 * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
 *
 ******************************************************************************/

/******************** PORTIONS (C) COPYRIGHT 2012 Honeywell ********************
 *
 * Modified Author       : Jonny.Long
 * Modified Version  : V 0.1
 * Modified Date     : 9/4/2012
 *
 * Notice:
 * This program contains open source software that is covered by the GNU General
 * Public License version 2 as published by the Free Software Foundation.
 *
 * THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
 * PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
 * AS A RESULT, HONEYWELL SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
 * CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
 * INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH HONEYWELL PARTS
 *
 *******************************************************************************
 *
 * Modified Items:
 * 1. Replacing all references to "lsm303dlh_mag" by "mmc328xma" including:
 *    filename, variable names, function names, case names, comments, and texts.
 * 2. Modified magnetometer "sensitivity" values to match MMC328XMA datasheet.
 *    Renamed constant names with "SENSITIVITY" to "GAIN" and combined XY and Z.
 *    Added definition for "gain" 0.
 * 3. Modified magnetometer "initialization" value for output rate to match
 *    MMC328XMA datasheet (0x20).
 * 4. Swapped Y and Z sensor output data in function "mmc328xma_read_mag_xyz()".
 *
 ******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#include "mmc328xma.h"


#define ABS_STATUS		(ABS_RUDDER)

#define MMC328XMA_MAJOR		101
#define MMC328XMA_MINOR		4

#define MMC328XMA_RRM_INTV	100

/* Output register start address*/
#define MMC328XMA_REG_DATA	0x00
#define MMC328XMA_REG_DS	0x06
#define MMC328XMA_REG_CNTL	0x07
#define MMC328XMA_REG_CHIPID	0x10

#define MMC328XMA_MODE_IOC_TM	0x01
#define MMC328XMA_MODE_IOC_RM	0x20
#define MMC328XMA_MODE_IOC_RRM	0x40

#define MMC328XMA_TEST_ID	0x03

/* mmc328xma magnetometer identification registers */
#define GAIN_0  1280	/* XYZ sensitivity at 0.9G */
#define GAIN_1  1024	/* XYZ sensitivity at 1.2G */
#define GAIN_2  768	/* XYZ sensitivity at 1.9G */
#define GAIN_3  614	/* XYZ sensitivity at 2.5G */
#define GAIN_4  415	/* XYZ sensitivity at 4.0G */
#define GAIN_5  361	/* XYZ sensitivity at 4.6G */
#define GAIN_6  307	/* XYZ sensitivity at 5.5G */
#define GAIN_7  219	/* XYZ sensitivity at 7.9G */

#define ABSMIN_MGA		(-2048)
#define ABSMAX_MGA		(2047)

#define MMC328XMA_AXIS_X_VALID	0x01
#define MMC328XMA_AXIS_Y_VALID	0x02
#define MMC328XMA_AXIS_Z_VALID	0x04
#define MMC328XMA_AXIS_ALL_VALID  \
	(MMC328XMA_AXIS_X_VALID | MMC328XMA_AXIS_Y_VALID | \
	MMC328XMA_AXIS_Z_VALID)

#define MMC328XMA_DEFAULT_DELAY 50
#define MMC328XMA_MAX_DELAY     500
#define MMC328XMA_MIN_DELAY     20


#define delay_to_jiffies(d)	((d) ? msecs_to_jiffies(d) : 1)
#define actual_delay(d)		(jiffies_to_msecs(delay_to_jiffies(d)))

struct mmc328xma_t {
	short x;	/* Range -7900 to 7900. */
	short y;	/* Range -7900 to 7900. */
	short z;	/* Range -7900 to 7900. */
};

struct mmc328xma_data {
	struct i2c_client *client;
	struct mmc328xma_platform_data *pdata;
	short xy_sensitivity;
	short z_sensitivity;
	atomic_t enable;		/* attribute value */
	atomic_t delay;			/* attribute value */
	atomic_t position;		/* attribute value */
	struct mmc328xma_t last;	/* last measured data */
	int  need_calibrate;
	int  cal_offset[3];
	int magetic_max[3];
	int magetic_min[3];
	int status;
	char range;
	struct mutex enable_mutex;
	struct mutex data_mutex;
	struct input_dev *input;
	struct delayed_work work;
	int user;
};

static unsigned int read_idx;

static struct mmc328xma_data *mag;

static int mmc328xma_do_magnetization(void);



static int __mmc328xma_i2c_rxdata(char *buf, int length)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = mag->client->addr,
			.flags = 0,
			.len = 1,
			.buf = buf,
		},
		{
			.addr = mag->client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = buf,
		},
	};

	ret = i2c_transfer(mag->client->adapter, msgs, 2);
	if (ret < 0) {
		printk(KERN_ERR "mmc328xma_i2c_rxdata: transfer error\n");
		return -EIO;
	} else {
		return ret;
	}
}


static int mmc328xma_i2c_txdata(char *buf, int length)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = mag->client->addr,
			.flags = 0,
			.len = length,
			.buf = buf,
		},
	};

	ret = i2c_transfer(mag->client->adapter, msgs, 1);
	if (ret < 0) {
		printk(KERN_ERR "mmc328xma_i2c_txdata: transfer error\n");
		return -EIO;
	} else {
		return ret;
	}
}

static  int mmc328xma_position_map[][3][3] = {
	{ { 1,  0, 0}, {  0,  1,  0}, { 0,  0,  1} },	/* top / upper-left */
	{ { 0,  1, 0}, { -1,  0,  0}, { 0,  0,  1} },	/* top / upper-right */
	{ {-1,  0, 0}, {  0, -1,  0}, { 0,  0,  1} },	/* top / lower-right */
	{ { 0, -1, 0}, {  1,  0,  0}, { 0,  0,  1} },	/* top / lower-left */

	{ {-1,  0, 0}, {  0,  1,  0}, { 0,  0, -1} },
	{ { 1,  0, 0}, {  0, -1,  0}, { 0,  0, -1} },	/*360 ok*/
	{ {-1,  0, 0}, {  0, -1,  0}, { 0,  0,  1} },
	{ { 1,  0, 0}, {  0, -1,  0}, { 0,  0, -1} },
	{ { 0,  1, 0}, {  1,  0,  0}, { 0,  0, -1} },
};

static int init_calibirate_data(struct mmc328xma_data *mmc328xma)
{
	int i;

	for (i = 0; i < 3; i++)
		mmc328xma->magetic_max[i] = ABSMIN_MGA;

	for (i = 0; i < 3; i++)
		mmc328xma->magetic_min[i] = ABSMAX_MGA;

	mmc328xma->need_calibrate = 1;

	return 0;
}

static int mmc328xma_read_mag_xyz(struct mmc328xma_t *data)
{
	int res;
	unsigned char mag_data[6];
	short hw_d[3] = { 0 };

	res = mmc328xma_do_magnetization();
	if (res < 0)
		printk(KERN_ERR "Do magnetization fail\n");

	mag_data[0] = MMC328XMA_REG_DATA;
	res = __mmc328xma_i2c_rxdata(mag_data, 6);
	if (res < 0)
		printk(KERN_ERR "%s(), %d, read data error!\n",
			__func__, __LINE__);

	hw_d[0] = (short)(((mag_data[1]) << 8) | mag_data[0]);
	hw_d[1] = (short)(((mag_data[3]) << 8) | mag_data[2]);
	hw_d[2] = (short)(((mag_data[5]) << 8) | mag_data[4]);
	hw_d[2] = (short)(8192 - hw_d[2]);

	data->x = hw_d[0];
	data->y = hw_d[1];
	data->z = hw_d[2];

	return res;
}

int mmc328xma_set_range(char range)
{
	switch (range) {
	case MMC328XMA_0_9G:
		mag->xy_sensitivity = GAIN_0;
		mag->z_sensitivity = GAIN_0;
		break;
	case MMC328XMA_1_2G:
		mag->xy_sensitivity = GAIN_1;
		mag->z_sensitivity = GAIN_1;
		break;
	case MMC328XMA_1_9G:
		mag->xy_sensitivity = GAIN_2;
		mag->z_sensitivity = GAIN_2;
		break;
	case MMC328XMA_2_5G:
		mag->xy_sensitivity = GAIN_3;
		mag->z_sensitivity = GAIN_3;
		break;
	case MMC328XMA_4_0G:
		mag->xy_sensitivity = GAIN_4;
		mag->z_sensitivity = GAIN_4;
		break;
	case MMC328XMA_4_6G:
		mag->xy_sensitivity = GAIN_5;
		mag->z_sensitivity = GAIN_5;
		break;
	case MMC328XMA_5_5G:
		mag->xy_sensitivity = GAIN_6;
		mag->z_sensitivity = GAIN_6;
		break;
	case MMC328XMA_7_9G:
		mag->xy_sensitivity = GAIN_7;
		mag->z_sensitivity = GAIN_7;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mmc328xma_do_magnetization()
{
	int ret;
	int i = 0;
	char buf[6] = {0};

	if (!(read_idx++ % MMC328XMA_RRM_INTV)) {
		printk(KERN_DEBUG "%s(), line: %d, Do RRM or RM times: %d\n",
			__func__, __LINE__, read_idx);

		buf[0] = MMC328XMA_REG_CNTL;
		buf[1] = MMC328XMA_MODE_IOC_RRM;
		ret = mmc328xma_i2c_txdata(buf, 2);
		if (ret < 0) {
			printk(KERN_ERR "%s(), line; %d, i2c write RRM  %d\n",
				__func__, __LINE__, ret);
			return -2;
		}
		msleep(20);

		buf[0] = MMC328XMA_REG_CNTL;
		buf[1] = MMC328XMA_MODE_IOC_RM;
		ret = mmc328xma_i2c_txdata(buf, 2);
		if (ret < 0) {
			printk(KERN_ERR "%s(), line%d, i2c write RM fail %d\n",
				__func__, __LINE__, ret);
			return -3;
		}
		msleep(20);

		buf[0] = MMC328XMA_REG_CNTL;
		buf[1] = MMC328XMA_MODE_IOC_TM;
		ret = mmc328xma_i2c_txdata(buf, 2);
		if (ret < 0) {
			printk(KERN_ERR "%s(), line %d, i2c write TM fail %d\n",
				__func__, __LINE__, ret);
			return -4;
		}
		msleep(20);

		do {
			buf[0] = MMC328XMA_REG_DS;
			ret = __mmc328xma_i2c_rxdata(buf, 1);
			msleep(20);
		} while (!(buf[0] & 0x01) && (i++ < 10));

		if (i >= 10) {
			printk(KERN_ERR "%s(), %d, read DS error!\n",
				__func__, __LINE__);
			return -5;
		}
	}

	buf[0] = MMC328XMA_REG_CNTL;
	buf[1] = MMC328XMA_MODE_IOC_TM;
	ret = mmc328xma_i2c_txdata(buf, 2);
	if (ret < 0) {
		printk(KERN_ERR "%s(), line; %d, i2c write TM fail! ret = %d\n",
			__func__, __LINE__, ret);
		return -4;
	}
	msleep(20);

	return 0;
}

static int device_init(void)
{
	int res = 0;
	read_idx = 0;

	printk(KERN_INFO "%s(), line: %d\n", __func__, __LINE__);

	res = mmc328xma_do_magnetization();
	if (res)
		return -EFAULT;

	mag->xy_sensitivity = GAIN_6;
	mag->z_sensitivity = GAIN_6;
	msleep(100);

	return res;
}


static int mmc328xma_acc_validate_pdata(struct mmc328xma_data *mag)
{
	if (mag->pdata->axis_map_x > 2 ||
			mag->pdata->axis_map_y > 2 ||
			mag->pdata->axis_map_z > 2) {
		dev_err(&mag->client->dev,
				"invalid axis_map value x:%u y:%u z%u\n",
				mag->pdata->axis_map_x, mag->pdata->axis_map_y,
				mag->pdata->axis_map_z);
		return -EINVAL;
	}

	if (mag->pdata->negate_x > 1 ||
			mag->pdata->negate_y > 1 ||
			mag->pdata->negate_z > 1) {
		dev_err(&mag->client->dev,
				"invalid negate value x:%u y:%u z:%u\n",
				mag->pdata->negate_x, mag->pdata->negate_y,
				mag->pdata->negate_z);
		return -EINVAL;
	}

	return 0;
}


static int mmc328xma_get_enable(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mmc328xma_data *mmc328xma = i2c_get_clientdata(client);

	return atomic_read(&mmc328xma->enable);
}

static void mmc328xma_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mmc328xma_data *mmc328xma = i2c_get_clientdata(client);
	int delay = atomic_read(&mmc328xma->delay);

	mutex_lock(&mmc328xma->enable_mutex);
	if (enable)
		mmc328xma->user++ ;
	else
		mmc328xma->user--;

	if (mmc328xma->user < 0)
		mmc328xma->user = 0;

	if (enable && mmc328xma->user == 1) {
		if (!atomic_cmpxchg(&mmc328xma->enable, 0, 1)) {
			device_init();
			schedule_delayed_work(&mmc328xma->work,
					delay_to_jiffies(delay) + 1);
		}
		atomic_set(&mmc328xma->enable, enable);
	} else if (!enable && mmc328xma->user == 0) {
		if (atomic_cmpxchg(&mmc328xma->enable, 1, 0))
			cancel_delayed_work_sync(&mmc328xma->work);

		atomic_set(&mmc328xma->enable, enable);
	}
	mutex_unlock(&mmc328xma->enable_mutex);
}

static int mmc328xma_get_delay(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mmc328xma_data *mmc328xma = i2c_get_clientdata(client);

	return atomic_read(&mmc328xma->delay);
}

static void mmc328xma_set_delay(struct device *dev, int delay)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mmc328xma_data *mmc328xma = i2c_get_clientdata(client);

	atomic_set(&mmc328xma->delay, delay);

	mutex_lock(&mmc328xma->enable_mutex);

	if (mmc328xma_get_enable(dev)) {
		cancel_delayed_work_sync(&mmc328xma->work);
		schedule_delayed_work(&mmc328xma->work,
				delay_to_jiffies(delay) + 1);
	} else {
		device_init();
	}

	mutex_unlock(&mmc328xma->enable_mutex);
}

static int mmc328xma_get_position(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mmc328xma_data *mmc328xma = i2c_get_clientdata(client);

	return atomic_read(&mmc328xma->position);
}

static void mmc328xma_set_position(struct device *dev, int position)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct mmc328xma_data *mmc328xma = i2c_get_clientdata(client);

	atomic_set(&mmc328xma->position, position);
	init_calibirate_data(mmc328xma);

}

int mmc328xma_data_transter(int *raw, int *data)
{
	long long g;
	int i, j;
	int pos = atomic_read(&mag->position);
	for (i = 0; i < 3; i++) {
		data[i] = 0;
		for (j = 0; j < 3; j++)
			data[i] += raw[j] * mmc328xma_position_map[pos][i][j];
		g = (long long)data[i];
		data[i] = g;
	}

	return 0;
}

static void mmc328xma_work_func(struct work_struct *work)
{
	struct mmc328xma_data *mmc328xma =
		container_of((struct delayed_work *)work,
		struct mmc328xma_data, work);
	struct mmc328xma_t magn;
	int raw[3];
	int status = 0;
	unsigned long delay = delay_to_jiffies(atomic_read(&mmc328xma->delay));

	if (mag->client == NULL)
		return ;

	mmc328xma_read_mag_xyz(&magn);
	raw[0] = magn.x;
	raw[1] = magn.y;
	raw[2] = magn.z;

	input_report_abs(mmc328xma->input, ABS_RX, raw[0]);
	input_report_abs(mmc328xma->input, ABS_RY, raw[1]);
	input_report_abs(mmc328xma->input, ABS_RZ, raw[2]);
	input_report_abs(mmc328xma->input, ABS_STATUS, status);
	input_sync(mmc328xma->input);

	mutex_lock(&mmc328xma->data_mutex);
	mmc328xma->last.x = raw[0];
	mmc328xma->last.y = raw[1];
	mmc328xma->last.z = raw[2];
	mutex_unlock(&mmc328xma->data_mutex);

	schedule_delayed_work(&mmc328xma->work, delay);
}

static int mmc328xma_input_init(struct mmc328xma_data *mmc328xma)
{
	struct input_dev *dev;
	int err;

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	dev->name = "geomagnetic";
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_ABS, ABS_MISC);
	input_set_abs_params(dev, ABS_RX, ABSMIN_MGA, ABSMAX_MGA, 0, 0);
	input_set_abs_params(dev, ABS_RY, ABSMIN_MGA, ABSMAX_MGA, 0, 0);
	input_set_abs_params(dev, ABS_RZ, ABSMIN_MGA, ABSMAX_MGA, 0, 0);
	input_set_capability(dev, EV_ABS, ABS_STATUS);
	input_set_drvdata(dev, mmc328xma);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}
	mmc328xma->input = dev;

	return 0;
}

static void mmc328xma_input_fini(struct mmc328xma_data *mmc328xma)
{
	struct input_dev *dev = mmc328xma->input;

	input_unregister_device(dev);
	input_free_device(dev);
}

static ssize_t mmc328xma_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *data = input_get_drvdata(input_dev);

	return sprintf(buf, "%d\n", mmc328xma_get_enable(&data->client->dev));
}

static ssize_t mmc328xma_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long enable;
	int ret = 0;
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *data = input_get_drvdata(input_dev);

	ret = kstrtoul(buf, 10, &enable);
	if (ret < 0) {
		printk(KERN_ERR "get enable fail\n");
		return -EINVAL;
	}

	if (!dev)
		return count;

	if ((enable == 0) || (enable == 1))
		mmc328xma_set_enable(&data->client->dev, enable);

	return count;
}

static ssize_t mmc328xma_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *data = input_get_drvdata(input_dev);

	return sprintf(buf, "%d\n", mmc328xma_get_delay(&data->client->dev));
}

static ssize_t mmc328xma_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long delay;
	int ret = 0;
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *data = input_get_drvdata(input_dev);

	ret = kstrtoul(buf, 10, &delay);
	if (ret < 0) {
		printk(KERN_ERR "get enable fail\n");
		return -EINVAL;
	}

	printk(KERN_ERR "delay:%lu\n", delay);
	delay >>= 20;
	if (delay > MMC328XMA_MAX_DELAY)
		delay = MMC328XMA_MAX_DELAY;
	if (delay < MMC328XMA_MIN_DELAY)
		delay = MMC328XMA_MIN_DELAY;

	mmc328xma_set_delay(&data->client->dev, delay);

	return count;
}

static ssize_t mmc328xma_position_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *data = input_get_drvdata(input_dev);
	return sprintf(buf, "%d\n", mmc328xma_get_position(&data->client->dev));
}

static ssize_t mmc328xma_position_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long position;
	int ret = 0;
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *data = input_get_drvdata(input_dev);

	ret = kstrtoul(buf, 10, &position);
	if (ret < 0) {
		printk(KERN_ERR "fail to get position, argument error\n");
		return -EINVAL;
	}
	if ((position >= 0) && (position <= 7))
		mmc328xma_set_position(&data->client->dev, position);

	return count;
}
static ssize_t mmc328xma_offset_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);
	int *poffset;

	mutex_lock(&mmc328xma->data_mutex);
	poffset = mmc328xma->cal_offset;
	mutex_unlock(&mmc328xma->data_mutex);

	return sprintf(buf, "%d %d %d\n", poffset[0], poffset[1], poffset[2]);
}

static ssize_t mmc328xma_offset_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);
	int *poffset;

	mutex_lock(&mmc328xma->data_mutex);
	poffset = mmc328xma->cal_offset;
	sscanf(buf, "%d %d %d\n", &(poffset[0]), &(poffset[1]), &(poffset[2]));
	printk(KERN_DEBUG "mmc328xma store offset value %d,%d,%d\n",
		mmc328xma->cal_offset[0], mmc328xma->cal_offset[1],
		mmc328xma->cal_offset[2]);
	mutex_unlock(&mmc328xma->data_mutex);
	return count;
}
static ssize_t mmc328xma_calibrate_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);

	mutex_lock(&mmc328xma->data_mutex);
	init_calibirate_data(mmc328xma);
	mutex_unlock(&mmc328xma->data_mutex);
	return count;
}
static ssize_t mmc328xma_wake_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input_dev = to_input_dev(dev);

	static atomic_t serial = ATOMIC_INIT(0);

	input_report_abs(input_dev, ABS_MISC, atomic_inc_return(&serial));

	return count;
}

static ssize_t mmc328xma_data_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);
	struct mmc328xma_t maga;

	mutex_lock(&mmc328xma->data_mutex);
	maga = mmc328xma->last;
	mutex_unlock(&mmc328xma->data_mutex);

	return sprintf(buf, "%d %d %d\n", maga.x, maga.y, maga.z);
}

static ssize_t mmc328xma_direction_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);
	int postion;
	int ret;
	mutex_lock(&mmc328xma->data_mutex);
	postion = mmc328xma_get_position(&mmc328xma->client->dev);
	ret = sprintf(buf, "%d %d %d %d %d %d %d %d %d\n"
			, mmc328xma_position_map[5][0][0]
			, mmc328xma_position_map[5][0][1]
			, mmc328xma_position_map[5][0][2]
			, mmc328xma_position_map[5][1][0]
			, mmc328xma_position_map[5][1][1]
			, mmc328xma_position_map[5][1][2]
			, mmc328xma_position_map[5][2][0]
			, mmc328xma_position_map[5][2][1]
			, mmc328xma_position_map[5][2][2]
			);
	mutex_unlock(&mmc328xma->data_mutex);

	return ret;
}

static ssize_t mmc328xma_direction_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);
	int postion;

	mutex_lock(&mmc328xma->data_mutex);
	postion = mmc328xma_get_position(&mmc328xma->client->dev);
	sscanf(buf, "%d %d %d %d %d %d %d %d %d\n",
		&mmc328xma_position_map[5][0][0],
		&mmc328xma_position_map[5][0][1],
		&mmc328xma_position_map[5][0][2],
		&mmc328xma_position_map[5][1][0],
		&mmc328xma_position_map[5][1][1],
		&mmc328xma_position_map[5][1][2],
		&mmc328xma_position_map[5][2][0],
		&mmc328xma_position_map[5][2][1],
		&mmc328xma_position_map[5][2][2]);
	init_calibirate_data(mmc328xma);
	mutex_unlock(&mmc328xma->data_mutex);

	return count;
}

static ssize_t mmc328xma_range_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int range;

	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);

	mutex_lock(&mmc328xma->data_mutex);
	range = mmc328xma->range;
	mutex_unlock(&mmc328xma->data_mutex);

	return sprintf(buf, "%d\n", range >> 5);
}

static ssize_t mmc328xma_range_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);
	long range;
	int ret = 0;

	mutex_lock(&mmc328xma->data_mutex);
	ret = kstrtol(buf, 10, &range);
	if (ret < 0) {
		printk(KERN_ERR "fail to get range ,argumentt err\n");
		return -EINVAL;
	}

	printk(KERN_DEBUG "mmc328xma store ragne %ld\n", range);
	if (range >= 0 && range <= 7) {
		mmc328xma_set_range(range << 5);
		mag->range = range << 5 ;
	}
	init_calibirate_data(mmc328xma);
	mutex_unlock(&mmc328xma->data_mutex);

	return count;
}

static ssize_t mmc328xma_selftest_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct input_dev *input_dev = to_input_dev(dev);
	struct mmc328xma_data *mmc328xma = input_get_drvdata(input_dev);
	long  range;
	int ret;

	mutex_lock(&mmc328xma->data_mutex);
	ret = kstrtol(buf, 10, &range);
	if (ret < 0) {
		printk(KERN_ERR "fail to get argument\n");
		return -EINVAL;
	}
	printk(KERN_INFO "mmc328xma store ragne %ld\n", range);
	if (range >= 0 && range <= 7) {
		mmc328xma_set_range(range << 5);
		mag->range = range << 5;
	}

	return count;
}

static DEVICE_ATTR(active, 0664,
		mmc328xma_enable_show, mmc328xma_enable_store);
static DEVICE_ATTR(interval, 0664,
		mmc328xma_delay_show, mmc328xma_delay_store);
static DEVICE_ATTR(position, 0664,
		mmc328xma_position_show, mmc328xma_position_store);
static DEVICE_ATTR(offset, 0664,
		mmc328xma_offset_show, mmc328xma_offset_store);
static DEVICE_ATTR(calibrate, 0664,
		NULL, mmc328xma_calibrate_store);
static DEVICE_ATTR(wake, 0664,
		NULL, mmc328xma_wake_store);
static DEVICE_ATTR(data, 0444,
		mmc328xma_data_show, NULL);
static DEVICE_ATTR(direction, 0664,
		mmc328xma_direction_show, mmc328xma_direction_store);
static DEVICE_ATTR(range, 0664,
		mmc328xma_range_show, mmc328xma_range_store);
static DEVICE_ATTR(selftest, 0664,
		NULL, mmc328xma_selftest_store);

static struct attribute *mmc328xma_attributes[] = {
	&dev_attr_active.attr,
	&dev_attr_interval.attr,
	&dev_attr_position.attr,
	&dev_attr_offset.attr,
	&dev_attr_calibrate.attr,
	&dev_attr_wake.attr,
	&dev_attr_data.attr,
	&dev_attr_direction.attr,
	&dev_attr_range.attr,
	&dev_attr_selftest.attr,
	NULL
};

static struct attribute_group mmc328xma_attribute_group = {
	.attrs = mmc328xma_attributes
};

int mmc328xma_probe(struct i2c_client *client,
		const struct i2c_device_id *devid)
{
	struct mmc328xma_data *data;
	int err = 0;
	int tempvalue;
	int *pOffset;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit;
	}

	if (!i2c_check_functionality(client->adapter,
		I2C_FUNC_SMBUS_I2C_BLOCK)) {
		err = -ENODEV;
		goto exit;
	}

	data = kzalloc(sizeof(struct mmc328xma_data), GFP_KERNEL);
	if (data == NULL) {
		err = -ENOMEM;
		goto exit;
	}

	i2c_set_clientdata(client, data);
	data->client = client;

	data->pdata = kmalloc(sizeof(*data->pdata), GFP_KERNEL);
	if (data->pdata == NULL) {
		err = -ENOMEM;
		goto exit_kfree;
	}
	memset(data->pdata, 0, sizeof(*data->pdata));
	data->pdata->axis_map_x = 0;
	data->pdata->axis_map_y = 0;
	data->pdata->axis_map_z = 0;
	data->pdata->negate_x = 0;
	data->pdata->negate_y = 0;
	data->pdata->negate_z = 0;
	data->range = MMC328XMA_5_5G;
	pOffset = (int *)client->dev.platform_data;
	if (pOffset) {
		data->cal_offset[0] = pOffset[0];
		data->cal_offset[1] = pOffset[1];
		data->cal_offset[2] = pOffset[2];
		data->need_calibrate = 0;
	} else
		init_calibirate_data(data);

	err = mmc328xma_acc_validate_pdata(data);
	if (err < 0) {
		dev_err(&client->dev, "failed to validate platform data\n");
		goto exit_kfree_pdata;
	}

	mag = data;
	tempvalue = i2c_smbus_read_byte_data(client, MMC328XMA_REG_CHIPID);
	pr_info("mmc328x chip id =0x%x\n", tempvalue&0x00FF);

	mutex_init(&data->enable_mutex);
	mutex_init(&data->data_mutex);
	mmc328xma_set_delay(&client->dev, MMC328XMA_DEFAULT_DELAY);
	INIT_DELAYED_WORK(&data->work, mmc328xma_work_func);
	printk(KERN_INFO "MMC328XMA device created successfully\n");

	err = mmc328xma_input_init(data);
	if (err < 0)
		goto input_init_err;

	err = sysfs_create_group(&data->input->dev.kobj,
			&mmc328xma_attribute_group);
	if (err < 0)
		goto inputdev_exit;

	return 0;

	sysfs_remove_group(&client->dev.kobj, &mmc328xma_attribute_group);
inputdev_exit:
	mmc328xma_input_fini(data);
input_init_err:
exit_kfree_pdata:
	kfree(data->pdata);
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int mmc328xma_remove(struct i2c_client *client)
{
	struct mmc328xma_data *data = i2c_get_clientdata(client);

	printk(KERN_INFO "MMC328XMA driver removing\n");
	sysfs_remove_group(&client->dev.kobj, &mmc328xma_attribute_group);

	mmc328xma_input_fini(data);

	kfree(data->pdata);

	kfree(data);

	mag->client = NULL;

	return 0;
}

static int mmc328xma_suspend(struct i2c_client *client, pm_message_t state)
{
	printk(KERN_DEBUG "mmc328xma_resume\n");

	mmc328xma_set_enable(&client->dev, 0);
	return 0;
}

static int mmc328xma_resume(struct i2c_client *client)
{
	struct mmc328xma_data *mmc328xma = i2c_get_clientdata(client);

	printk(KERN_DEBUG "mmc328xma_resume\n");
	device_init();
	if (mmc328xma->user > 0)
		mmc328xma_set_enable(&client->dev, 1);

	return 0;
}

static const struct i2c_device_id mmc328xma_id[] = {
	{ "mmc328xma", 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, mmc328xma_id);

static const struct of_device_id mmc328x_of_match[] = {
	{.compatible = "bcm,mmc328x",},
	{},
};

MODULE_DEVICE_TABLE(of, mmc328x_of_match);

static struct i2c_driver mmc328xma_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = mmc328xma_probe,
	.remove = mmc328xma_remove,
	.id_table = mmc328xma_id,
	.suspend = mmc328xma_suspend,
	.resume = mmc328xma_resume,
	.driver = {
		.owner = THIS_MODULE,
		.name = "mmc328xma",
		.of_match_table = mmc328x_of_match,
	},
};

static int __init mmc328xma_init(void)
{
	return i2c_add_driver(&mmc328xma_driver);
}

static void __exit mmc328xma_exit(void)
{
	i2c_del_driver(&mmc328xma_driver);

	return;
}

module_init(mmc328xma_init);
module_exit(mmc328xma_exit);

MODULE_DESCRIPTION("mmc328xma magnetometer driver");
MODULE_AUTHOR("Broadcom");
MODULE_LICENSE("GPL");
