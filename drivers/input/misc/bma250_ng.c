/* drivers/input/misc/bma250_ng.c
 *
 * Bosh BMA 250. Digital, triaxial acceleration sensor.
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Marcus Bauer <marcus.bauer@sonymobile.com>
 *         Tadashi Kubo <tadashi.kubo@sonymobile.com>
 *         Takashi Shiina <takashi.shiina@sonymobile.com>
 *         Hisakazu Furuie <hisakazu.x.furuie@sonymobile.com>
 *         Chikaharu Gonnokami <Chikaharu.X.Gonnokami@sonymobile.com>
 *         Stefan Karlsson <stefan4.karlsson@sonymobile.com>
 *
 * NOTE: This file has been created by Sony Ericsson Mobile Communications AB.
 *       This file has been modified by Sony Mobile Communications AB.
 *       This file contains code from: bma150.c
 *       The orginal bma150.c header is included below:
 *
 * Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * Protocol driver for Bosch BMA250 accelerometer
 *
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/bma250_ng.h>
#include <linux/bma250_ng_common.h>
#include <linux/delay.h>
#include <linux/mutex.h>

#define BMA250_NAME                      "bma250"
#define BMA250_VENDORID                  0x0001

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("bma250");

/* bma250 register restrictions */
#define BMA250_LAST_REG 0x3F
#define BMA250_NA (1 << 8)
#define BMA250_RD (2 << 8)
#define BMA250_WR (4 << 8)
#define BMA250_RW (8 << 8)
static uint16_t bma250_mask[] = {
	0xFF | BMA250_RD, /* 00 */
	0xFF | BMA250_RD, /* 01 */
	0xC1 | BMA250_RD, /* 02 */
	0xFF | BMA250_RD, /* 03 */
	0xC1 | BMA250_RD, /* 04 */
	0xFF | BMA250_RD, /* 05 */
	0xC1 | BMA250_RD, /* 06 */
	0xFF | BMA250_RD, /* 07 */
	0xFF | BMA250_RD, /* 08 */
	0xF7 | BMA250_RD, /* 09 */
	0x80 | BMA250_RD, /* 0A */
	0xFF | BMA250_RD, /* 0B */
	0xFF | BMA250_RD, /* 0C */
	0x00 | BMA250_NA, /* 0D */
	0x00 | BMA250_NA, /* 0E */
	0x0F | BMA250_RW, /* 0F */
	0x1F | BMA250_RW, /* 10 */
	0xDE | BMA250_RW, /* 11 */
	0x00 | BMA250_NA, /* 12 */
	0xC0 | BMA250_RW, /* 13 */
	0xFF | BMA250_RW, /* 14 */
	0x00 | BMA250_NA, /* 15 */
	0xF7 | BMA250_RW, /* 16 */
	0x1F | BMA250_RW, /* 17 */
	0x00 | BMA250_NA, /* 18 */
	0xF7 | BMA250_RW, /* 19 */
	0x81 | BMA250_RW, /* 1A */
	0xF7 | BMA250_RW, /* 1B */
	0x00 | BMA250_NA, /* 1C */
	0x00 | BMA250_NA, /* 1D */
	0x37 | BMA250_RW, /* 1E */
	0x00 | BMA250_NA, /* 1F */
	0x0F | BMA250_RW, /* 20 */
	0x8F | BMA250_RW, /* 21 */
	0xFF | BMA250_RW, /* 22 */
	0xFF | BMA250_RW, /* 23 */
	0xc7 | BMA250_RW, /* 24 */
	0xFF | BMA250_RW, /* 25 */
	0xFF | BMA250_RW, /* 26 */
	0x03 | BMA250_RW, /* 27 */
	0xFF | BMA250_RW, /* 28 */
	0x00 | BMA250_NA, /* 29 */
	0xC7 | BMA250_RW, /* 2A */
	0xDF | BMA250_RW, /* 2B */
	0x7F | BMA250_RW, /* 2C */
	0x3F | BMA250_RW, /* 2D */
	0x3F | BMA250_RW, /* 2E */
	0x30 | BMA250_RW, /* 2F */
	0x00 | BMA250_NA, /* 30 */
	0x01 | BMA250_RD, /* 31 */
	0x00 | BMA250_NA, /* 32 */
	0x0F | BMA250_RW, /* 33 */
	0x03 | BMA250_RW, /* 34 */
	0x00 | BMA250_NA, /* 35 */
	0xF3 | BMA250_RW, /* 36 */
	0x7F | BMA250_RW, /* 37 */
	0xFF | BMA250_RW, /* 38 */
	0xFF | BMA250_RW, /* 39 */
	0xFF | BMA250_RW, /* 3A */
	0xFF | BMA250_RW, /* 3B */
	0xFF | BMA250_RW, /* 3C */
	0xFF | BMA250_RW, /* 3D */
	0x00 | BMA250_NA, /* 3E */
	0x00 | BMA250_NA, /* 3F */
};

#define BMA250_STARTUP_TIME    4
#define BMA250_READDATA_RETRY  5
#define BMA250_BYPASS_STABEL   5
#define BMA250_I2C_WRITE_RETRY 5

#define BMA250_CALIB_MODE_START         0
#define BMA250_CALIB_MODE_END           1

#define BMA250_CALIB_GET_OFFSET_MAX     6
#define BMA250_CALIB_GET_REG_MAX        3
#define BMA250_CALIB_GET_READ_RETRY     3

#define BMA250_CALIB_GET_INTERVAL      35
#define BMA250_CALIB_SET_INTERVAL     100
#define BMA250_CALIB_SET_EEPROM        20

/*
 * Data returned from accelerometer.
 * Temp is in units of 0.5 degrees C
 */
struct bma250_accel_data {
	short            accel_x;
	short            accel_y;
	short            accel_z;
	int              temp;
};

struct driver_data {
	struct input_dev            *ip_dev;
	struct i2c_client           *ic_dev;
	unsigned char                shift;
	struct delayed_work          work_data;
	unsigned int                 rate;
	unsigned long                delay_jiffies;
	unsigned int                 range;
	unsigned int                 bw_sel;
	struct list_head             next_dd;
	struct dentry               *dbfs_root;
	struct dentry               *dbfs_regs;
	struct bma250_platform_data *pdata;
	bool                         power;
	bool                         recovery_power;
};

struct mutex                      bma250_power_lock;
static struct mutex               bma250_dd_lock;
static struct list_head           dd_list;

static int slave_hw;
static int bma250_config(struct driver_data *dd, int bw, int range);
static int bma250_power_down(struct driver_data *dd);
static int bma250_power_up(struct driver_data *dd);


struct driver_data *bma250_ic_get_data(struct i2c_client *ic_dev)
{
	return i2c_get_clientdata(ic_dev);
}

void bma250_ic_set_data(struct i2c_client *ic_dev, struct driver_data *data)
{
	i2c_set_clientdata(ic_dev, data);
}

int bma250_ic_read(struct i2c_client *ic_dev, u8 reg, u8 *buf, int len)
{
	int rc;
	rc = i2c_smbus_read_i2c_block_data(ic_dev, reg, len, buf);

	if (rc > 0)
		return 0;
	return rc;
}

int bma250_ic_write(struct i2c_client *ic_dev, u8 reg, u8 val)
{
	int rc, i;

	if (reg > BMA250_LAST_REG) {
		printk(KERN_ERR "%s: reg 0x%.02X, out of range.\n",
			__func__, reg);
		return -EINVAL;
	}
	if (bma250_mask[reg] & BMA250_NA) {
		printk(KERN_ERR "%s: reg 0x%.02X, reserved.\n",
			__func__, reg);
		return -EINVAL;
	}
	if (bma250_mask[reg] & BMA250_RD) {
		printk(KERN_ERR "%s: reg 0x%.02X, read only.\n",
			__func__, reg);
		return -EINVAL;

	}
	if (val & ~bma250_mask[reg]) {
		printk(KERN_ERR "%s: reg 0x%.02X val 0x%.02X, out of bounds.\n",
			__func__, reg, val);
		return -EINVAL;
	}

	for (i = 0; i < BMA250_I2C_WRITE_RETRY; i++) {
		rc = i2c_smbus_write_byte_data(ic_dev, reg, val);
		if (rc == 0)
			return rc;
	}
	printk(KERN_ERR "%s: error code = %d.\n", __func__, rc);
	return rc;
}

/* 10bit to 16 bit will give a 6 bit offset, shift
   it down to represent 256 lsb/g for alla ranges */
static inline u8 bma250_range2shift(u8 range)
{
	switch (range) {
	case BMA250_RANGE_16G:
		return 3;
		break;
	case BMA250_RANGE_8G:
		return 4;
		break;
	case BMA250_RANGE_4G:
		return 5;
		break;
	default:
	case BMA250_RANGE_2G:
		return 6;
		break;
	}
}

static inline int bma250_bw_handler(struct driver_data *dd)
{
	if (dd->rate > 100)
		dd->bw_sel = BMA250_BW_7_81HZ;
	else if (dd->rate > 50)
		dd->bw_sel = BMA250_BW_15_63HZ;
	else
		dd->bw_sel = BMA250_BW_31_25HZ;

	return bma250_ic_write(dd->ic_dev, BMA250_BW_SEL_REG, dd->bw_sel);
}

static inline int bma250_range_handler(struct driver_data *dd)
{
	int rc = 0;
	u8 threshold, duration = 0;

	if (dd->range == BMA250_RANGE_16G)
		threshold = 2;
	else if (dd->range == BMA250_RANGE_8G)
		threshold = 3;
	else if (dd->range == BMA250_RANGE_4G)
		threshold = 4;
	else
		threshold = 5;

	rc = bma250_ic_write(dd->ic_dev, BMA250_RANGE_REG, dd->range);
	if (rc)
		goto range_error;

	/* threshold definition for the slope int, g-range dependant */
	rc = bma250_ic_write(dd->ic_dev, BMA250_SLOPE_THR, threshold);
	if (rc)
		goto range_error;

	/* number of samples (n + 1) to be evaluted for slope int */
	rc = bma250_ic_write(dd->ic_dev, BMA250_SLOPE_DUR, duration);
	if (rc)
		goto range_error;

	dd->shift = bma250_range2shift(dd->range);
	return rc;

range_error:
	dev_err(&dd->ip_dev->dev,
		"%s: device failed, error %d\n", __func__, rc);
	return rc;
}

static ssize_t bma250_rate_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct driver_data *dd = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", dd->rate);
}

static ssize_t bma250_rate_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct driver_data *dd = dev_get_drvdata(dev);
	int rc;
	unsigned long val;

	rc = strict_strtoul(buf, 10, &val);
	if (rc)
		return rc;

	if ((val >= 1) && (val <= 10000)) {
		dd->rate = (unsigned int)val;
		dd->delay_jiffies = msecs_to_jiffies(dd->rate);
		rc = bma250_config(dd, TRUE, FALSE);
		return strnlen(buf, count);
	}
	return -EINVAL;
}

static inline int bma250_calibration_init(struct device *dev)
{
	int rc;
	char bypass = 0;
	struct driver_data *dd = dev_get_drvdata(dev);
	struct bma250_platform_data *pdata = dd->ic_dev->dev.platform_data;

	/* Check whether Gyro had been activated. */
	if (slave_hw)
		pdata->bypass_state(READ_BYPASS_STATE, &bypass);
	if (slave_hw && bypass) {
		rc = -EBUSY;
		goto calib_init_error_end;
	}

	/* Save POWER flag for recovery */
	dd->recovery_power = dd->power;

	if (!(dd->recovery_power)) {
		/* Power ON if accelerometer is suspend state. */
		rc = bma250_power_up(dd);
		if (rc)
			goto calib_init_error;
	} else {
		/* Stop data updating if accelerometer is active state. */
		cancel_delayed_work(&dd->work_data);
	}

	/* Set +/-2g Range Mode and 15.63 Hz Low Pass Filter Mode */
	rc = bma250_ic_write(dd->ic_dev, BMA250_RANGE_REG, BMA250_RANGE_2G);
	if (rc)
		goto calib_init_error;
	rc = bma250_ic_write(dd->ic_dev, BMA250_BW_SEL_REG, BMA250_BW_15_63HZ);
	if (rc)
		goto calib_init_error;
	msleep(BMA250_CALIB_SET_INTERVAL);
	return rc;

calib_init_error:
	dev_err(&dd->ic_dev->dev, "%s: Calibration start failure.(%d)\n",
		__func__, rc);
	rc = -EIO;

calib_init_error_end:
	return rc;
}

static inline int bma250_calibration_end(struct device *dev)
{
	int rc = 0;
	struct driver_data *dd = dev_get_drvdata(dev);

	/* power down */
	bma250_power_down(dd);
	dev_dbg(dev, "%s: -> power down.\n", __func__);

	if (dd->recovery_power && dd->ip_dev->users) {
		/* power up & reset*/
		rc = bma250_power_up(dd);
		if (rc)
			goto calib_end_error;

		/* re-schedule by a delay */
		rc = bma250_config(dd, TRUE, TRUE);
		if (rc)
			goto calib_end_error;
		schedule_delayed_work(&dd->work_data, dd->delay_jiffies);

		dev_dbg(dev, "%s: -> power up.\n", __func__);
	}
	return rc;

calib_end_error:
	dev_err(dev, "%s: Calibration end failure.(%d)\n",
		__func__, rc);
	return -EIO;
}

static ssize_t bma250_calib_set_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int rc = 0;

	if (*buf == BMA250_CALIB_MODE_START)
		rc = bma250_calibration_init(dev);
	else if (*buf == BMA250_CALIB_MODE_END)
		rc = bma250_calibration_end(dev);
	else
		rc = -EINVAL;

	if (rc)
		return rc;
	return count;
}

static ssize_t bma250_calib_offset_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct driver_data *dd = dev_get_drvdata(dev);
	int rc;

	/* read accel_data */
	rc = bma250_ic_read(dd->ic_dev, BMA250_X_AXIS_LSB_REG,
		buf, BMA250_CALIB_GET_OFFSET_MAX);
	if (rc)
		goto calib_get_offset_error;

	dev_dbg(&dd->ic_dev->dev, "%s: offset: %d %d %d %d %d %d\n",
		__func__, *buf, *(buf + 1),
		*(buf + 2), *(buf + 3), *(buf + 4), *(buf + 5));

	return BMA250_CALIB_GET_OFFSET_MAX;

calib_get_offset_error:
	dev_err(&dd->ic_dev->dev, "%s: Data read error from register.(%d)\n",
		__func__, rc);
	return -EIO;
}

static ssize_t bma250_calib_image_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct driver_data *dd = dev_get_drvdata(dev);
	int rc;

	/* enable EEPROM */
	rc = bma250_ic_write(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
		BMA250_EEPROM_PROG_MODE);
	if (rc)
		goto calib_offset_reg_error;

	/* get eeprom offset value */
	rc = bma250_ic_read(dd->ic_dev, BMA250_OFFSET_X_AXIS_REG,
		buf, BMA250_CALIB_GET_REG_MAX);
	if (rc)
		goto calib_offset_reg_error;

	/* disable EEPROM */
	rc = bma250_ic_write(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
		BMA250_EEPROM_RDY_MODE);
	if (rc)
		goto calib_offset_reg_error;

	dev_dbg(&dd->ic_dev->dev, "%s:REG:%d:%d:%d\n",
		__func__, *buf, *(buf+1), *(buf+2));
	return BMA250_CALIB_GET_REG_MAX;

calib_offset_reg_error:
	dev_err(&dd->ic_dev->dev, "%s: Offset read/write error from register.\n",
		__func__);
	return -EIO;
}

static ssize_t bma250_calib_image_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct driver_data *dd = dev_get_drvdata(dev);
	int rc;

	dev_dbg(&dd->ic_dev->dev, "%s : Image DEC : %d:%d:%d\n",
		__func__, *buf, *(buf + 1), *(buf + 2));

	/* enable EEPROM */
	rc = bma250_ic_write(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
		BMA250_EEPROM_PROG_MODE);
	if (rc)
		goto calib_set_image_reg_error;

	/* write X axis calibration data to Image (LSB) */
	rc = bma250_ic_write(dd->ic_dev, BMA250_OFFSET_X_AXIS_REG, *buf);
	if (rc)
		goto calib_set_image_reg_error;

	/* write Y axis calibration data to Image (LSB) */
	rc = bma250_ic_write(dd->ic_dev, BMA250_OFFSET_Y_AXIS_REG, *(buf + 1));
	if (rc)
		goto calib_set_image_reg_error;

	/* write Z axis calibration data to Image (LSB) */
	rc = bma250_ic_write(dd->ic_dev, BMA250_OFFSET_Z_AXIS_REG, *(buf + 2));
	if (rc)
		goto calib_set_image_reg_error;

	/* disable EEPROM */
	rc = bma250_ic_write(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
		BMA250_EEPROM_RDY_MODE);
	if (rc)
		goto calib_set_image_reg_error;
	msleep(BMA250_CALIB_SET_EEPROM);
	return BMA250_CALIB_GET_REG_MAX;

calib_set_image_reg_error:
	dev_err(&dd->ic_dev->dev, "%s: Image write error from register.\n",
		__func__);
	return -EIO;
}

static ssize_t bma250_calib_eeprom_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct driver_data *dd = dev_get_drvdata(dev);
	int rc;
	u8 tmp = 0;

	dev_dbg(&dd->ic_dev->dev, "%s : DEC : %d:%d:%d\n",
		__func__, *buf, *(buf + 1), *(buf + 2));

	/* enable EEPROM */
	rc = bma250_ic_write(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
		BMA250_EEPROM_PROG_MODE);
	if (rc)
		goto calib_EEPROM_error;

	/* write X axis calibration data to Image (LSB) */
	rc = bma250_ic_write(dd->ic_dev, BMA250_OFFSET_X_AXIS_REG, *buf);
	if (rc)
		goto calib_EEPROM_error;

	/* write Y axis calibration data to Image (LSB) */
	rc = bma250_ic_write(dd->ic_dev, BMA250_OFFSET_Y_AXIS_REG, *(buf + 1));
	if (rc)
		goto calib_EEPROM_error;

	/* write Z axis calibration data to Image (LSB) */
	rc = bma250_ic_write(dd->ic_dev, BMA250_OFFSET_Z_AXIS_REG, *(buf + 2));
	if (rc)
		goto calib_EEPROM_error;

	/* enable EEPROM write process */
	rc = bma250_ic_write(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
		BMA250_EEPROM_WRITE_MODE);
	if (rc)
		goto calib_EEPROM_error;

	msleep(BMA250_CALIB_SET_EEPROM);

	/* waiting for nvm_rdy bit is set*/
	do {
		msleep(BMA250_CALIB_GET_INTERVAL);
		rc = bma250_ic_read(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
			&tmp, 1);
		if (rc)
			goto calib_EEPROM_error;
	} while (!(tmp & BMA250_EEPROM_NOT_RDY_MODE));

	/* disable data access to EEPROM area */
	rc = bma250_ic_write(dd->ic_dev, BMA250_EEPROM_CTRL_REG,
		BMA250_EEPROM_RDY_MODE);
	if (rc)
		goto calib_EEPROM_error;
	msleep(BMA250_CALIB_SET_EEPROM);
	return BMA250_CALIB_GET_REG_MAX;

calib_EEPROM_error:
	dev_err(&dd->ic_dev->dev, "%s: EEPROM read/write error from register.\n",
		__func__);
	return -EIO;
}

static struct device_attribute attributes[] = {
	__ATTR(bma250_rate, 0644, bma250_rate_show, bma250_rate_store),
	__ATTR(bma250_calib_set, S_IRUGO | S_IWUSR,
		NULL, bma250_calib_set_store),
	__ATTR(bma250_calib_offset, S_IRUGO | S_IWUSR,
		bma250_calib_offset_show, NULL),
	__ATTR(bma250_calib_image, S_IRUGO | S_IWUSR,
		bma250_calib_image_show, bma250_calib_image_store),
	__ATTR(bma250_calib_eeprom, S_IRUGO | S_IWUSR,
		NULL, bma250_calib_eeprom_store),
};

static int add_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto undo;
	return 0;
undo:
	for (; i >= 0 ; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s: failed to create sysfs interface\n", __func__);
	return -ENODEV;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
}

#if defined(CONFIG_DEBUG_FS)
static int bma250_dbfs_open(struct inode *inode, struct file *fp)
{
	fp->private_data = inode->i_private;
	return 0;
}

static ssize_t bma250_dbfs_write(struct file *fp, const char __user *buf,
				 size_t count, loff_t *f_pos)
{
	u8                          *p;
	u8                          *np;
	u8                          *mbuf;
	int                          rc;
	unsigned int                 val;
	u8                           reg;
	u8                           data;
	struct driver_data          *dd;

	/* format of write data is "A[A] D[D]" eg. "AA DD", "A D" etc
	   where A is address in hex, D is data in hex.
	   Multiple address/data pairs may be separated by spaces.
	*/
	if (count < 3)
		return 0;

	dd = fp->private_data;

	mbuf = kzalloc(count, GFP_KERNEL);
	if (!mbuf) {
		rc = -ENOMEM;
		goto dbfs_write_exit;
	}

	if (copy_from_user(mbuf, buf, count)) {
		rc = -EFAULT;
		goto dbfs_write_exit_copy;
	}

	p = mbuf;

	while (isspace(*p))
		p++;
	do {
		val = simple_strtoul(p, (char **)&np, 16);
		if ((val > BMA250_LAST_REG) || (p == np)) {
			rc = -EINVAL;
			goto dbfs_write_exit_copy;
		}
		while (isspace(*np) && ((np - mbuf) < count))
			np++;
		p = np;
		reg = (u8)val;

		val = simple_strtoul(p, (char **)&np, 16);
		if ((val > 0xFF)  || (p == np)) {
			rc = -EINVAL;
			goto dbfs_write_exit_copy;
		}
		while (isspace(*np) && ((np - mbuf) < count))
			np++;
		p = np;
		data = (u8)val;

		rc = bma250_ic_write(dd->ic_dev, reg, data);

		/* update here to avoid checking g-range at each interrupt */
		if ((!rc) && (reg == BMA250_RANGE_REG))
			dd->shift = bma250_range2shift(data);

	} while (!rc && (np - mbuf) < count);

	if (rc)
		goto dbfs_write_exit;
	kfree(mbuf);

	return count;

dbfs_write_exit_copy:
	kfree(mbuf);
dbfs_write_exit:
	return rc;
}

static ssize_t bma250_dbfs_read(struct file *fp, char __user *buf,
				size_t count, loff_t *f_pos)
{
	u8                           rx;
	u8                           mbuf[8];
	int                          rc;
	int                          copy_size;
	struct driver_data          *dd;

	dd = fp->private_data;
	if ((int)*f_pos > BMA250_LAST_REG) {
		rc = 0;
		goto dbfs_read_exit;
	}

	rc = bma250_ic_read(dd->ic_dev, (u8)*f_pos, &rx, 1);
	if (rc)
		goto dbfs_read_exit;

	snprintf(mbuf, ARRAY_SIZE(mbuf), "%02x %02x\n", (u8)*f_pos, rx);
	copy_size = min(count, strlen(mbuf) + 1);
	if (copy_to_user(buf, mbuf, copy_size))
		return -EFAULT;
	(*f_pos)++;

	return copy_size;
dbfs_read_exit:
	return rc;
}

static const struct file_operations dbfs_fops = {
	.owner    = THIS_MODULE,
	.open     = bma250_dbfs_open,
	.read     = bma250_dbfs_read,
	.write    = bma250_dbfs_write,
};

static void bma250_create_dbfs_entry(struct driver_data *dd)
{
	char buf[16];

	snprintf(buf, sizeof(buf), BMA250_NAME);
	dd->dbfs_root = debugfs_create_dir(buf, NULL);
	if (dd->dbfs_root <= (struct dentry *)NULL) {
		dd->dbfs_root = NULL;
		goto dbfs_err_root;
	}

	dd->dbfs_regs = debugfs_create_file("registers",
					   S_IRUGO | S_IWUGO,
					   dd->dbfs_root, dd,
					   &dbfs_fops);
	if (dd->dbfs_regs <= (struct dentry *)NULL) {
		dd->dbfs_regs = NULL;
		goto dbfs_err_regs;
	}
	return;

dbfs_err_regs:
	debugfs_remove(dd->dbfs_root);
dbfs_err_root:
	return;
}

static void bma250_remove_dbfs_entry(struct driver_data *dd)
{
	if (dd->dbfs_regs)
		debugfs_remove(dd->dbfs_regs);
	if (dd->dbfs_root)
		debugfs_remove(dd->dbfs_root);
}
#else
static void bma250_create_dbfs_entry(struct driver_data *dd) { }

static void bma250_remove_dbfs_entry(struct driver_data *dd) { }
#endif

static int bma250_power_down(struct driver_data *dd)
{
	int                 rc = 0;
	char                bypass = 0;
	struct bma250_platform_data *pdata = dd->ic_dev->dev.platform_data;

	mutex_lock(&bma250_power_lock);

	if (slave_hw) {
		pdata->vote_sleep_status(BMA250_SLAVE1, BMA250_SLEEP);
		pdata->bypass_state(READ_BYPASS_STATE, &bypass);
		if (!bypass) {
			if (pdata->check_sleep_status() == BMA250_SLEEP) {
				rc = bma250_ic_write(dd->ic_dev,
					BMA250_MODE_CTRL_REG,
					BMA250_MODE_SUSPEND);
				cancel_delayed_work(&dd->work_data);
			}
		}
	} else {
		rc = bma250_ic_write(dd->ic_dev, BMA250_MODE_CTRL_REG,
				BMA250_MODE_SUSPEND);
		cancel_delayed_work(&dd->work_data);
	}
	(void)dd->pdata->hw_config(&dd->ic_dev->dev, 0);
	dd->power = false;
	mutex_unlock(&bma250_power_lock);
	return rc;
}

static int bma250_power_up(struct driver_data *dd)
{
	int                 rc = 0;
	char                bypass = 0;
	struct bma250_platform_data *pdata = dd->ic_dev->dev.platform_data;

	mutex_lock(&bma250_power_lock);
	rc = dd->pdata->hw_config(&dd->ic_dev->dev, 1);
	if (rc)
		goto hw_setup_error;

	if (slave_hw) {
		pdata->bypass_state(READ_BYPASS_STATE, &bypass);
		if (!bypass) {
			if (pdata->check_sleep_status() == BMA250_SLEEP) {
				rc = bma250_ic_write(dd->ic_dev,
					BMA250_RESET_REG, BMA250_RESET);
				if (rc)
					goto power_up_error;
				msleep(BMA250_STARTUP_TIME);
				rc = bma250_ic_write(dd->ic_dev,
					BMA250_MODE_CTRL_REG,
					BMA250_MODE_NOSLEEP);
				if (rc)
					goto power_up_error;
			}
		}
		pdata->vote_sleep_status(BMA250_SLAVE1, BMA250_AWAKE);
	} else {
		rc = bma250_ic_write(dd->ic_dev, BMA250_RESET_REG,
			BMA250_RESET);
		if (rc)
			goto power_up_error;
		msleep(BMA250_STARTUP_TIME);
		rc = bma250_ic_write(dd->ic_dev, BMA250_MODE_CTRL_REG,
				BMA250_MODE_NOSLEEP);
		if (rc)
			goto power_up_error;
	}

	dd->power = true;
	mutex_unlock(&bma250_power_lock);
	return rc;

power_up_error:
	printk(KERN_ERR "%s: Force power doen due to not read bypass status.\n",
		__func__);
	(void) dd->pdata->hw_config(&dd->ic_dev->dev, 0);
hw_setup_error:
	printk(KERN_ERR "%s: Force power down.(bypass check error)\n",
		__func__);
	dd->power = false;
	mutex_unlock(&bma250_power_lock);
	return rc;
}

static int __devinit bma250_hwid(struct driver_data *dd)
{
	int rc;
	u8 rx_buf[2];

	rc = bma250_ic_read(dd->ic_dev, BMA250_CHIP_ID_REG, rx_buf, 2);
	if (rc)
		goto config_exit;

	if ((rx_buf[0] == 0x00) || (rx_buf[1] == 0x00)) {
		printk(KERN_ERR "bma250: device not found.\n");
		rc = -ENODEV;
		goto config_exit;
	}
	printk(KERN_INFO "bma250: detected chip id %d, rev 0x%X\n",
				rx_buf[0] & 0x07, rx_buf[1]);

config_exit:
	return rc;
}

static int bma250_config(struct driver_data *dd, int bw, int range)
{
	int                 rc = 0;
	char                bypass = 0;
	struct bma250_platform_data *pdata = dd->ic_dev->dev.platform_data;

	if (slave_hw)
		pdata->bypass_state(READ_BYPASS_STATE, &bypass);

	if (!slave_hw || !bypass) {
		FIFO_ACCESS_MUTEX_LOCK();
		if (bw) {
			rc = bma250_bw_handler(dd);
			if (rc)
				goto config_error;
		}
		if (range) {
			rc = bma250_range_handler(dd);
			if (rc)
				goto config_error;
		}
		FIFO_ACCESS_MUTEX_UNLOCK();
	}
	return rc;

config_error:
	FIFO_ACCESS_MUTEX_UNLOCK();
	printk(KERN_ERR "%s: Configuration error.\n", __func__);
	return rc;
}

#if defined(CONFIG_PM)
static int bma250_suspend(struct i2c_client *ic_dev, pm_message_t mesg)
{
	struct driver_data *dd = bma250_ic_get_data(ic_dev);

	if (dd->ip_dev->users)
		bma250_power_down(dd);

	return 0;
}

static int bma250_resume(struct i2c_client *ic_dev)
{
	struct driver_data *dd = bma250_ic_get_data(ic_dev);
	int rc = 0;

	if (dd->ip_dev->users) {
		rc = bma250_power_up(dd);
		if (rc)
			return rc;
		rc = bma250_config(dd, TRUE, TRUE);
		schedule_delayed_work(&dd->work_data, dd->delay_jiffies);
	}
	return rc;
}
#else /* !CONFIG_PM */
#define bma250_suspend NULL
#define bma250_resume NULL
#endif /* CONFIG_PM */

static inline int bma250_report_data(struct driver_data *dd)
{
	int rc = 0;
	u8 rx_buf[8];
	char bypass = 0;
	int retrycount = 0;
	struct bma250_accel_data    data;
	struct bma250_platform_data *pdata = dd->ic_dev->dev.platform_data;

read_data_start:
	if (slave_hw)
		pdata->bypass_state(READ_BYPASS_STATE, &bypass);

	if (!slave_hw || !bypass) {
		/* Bypass mode */
		FIFO_ACCESS_MUTEX_LOCK();
		rc = bma250_ic_read(dd->ic_dev, BMA250_X_AXIS_LSB_REG,
			rx_buf, 7);
		FIFO_ACCESS_MUTEX_UNLOCK();
		if (rc) {
			if (retrycount > BMA250_READDATA_RETRY)
				goto report_error;
			else {
				retrycount++;
				msleep(BMA250_BYPASS_STABEL);
				goto read_data_start;
			}
		}
		/* 10bit signed to 16bit signed */
		data.accel_x = ((rx_buf[1] << 8) | (rx_buf[0] & 0xC0));
		data.accel_y = ((rx_buf[3] << 8) | (rx_buf[2] & 0xC0));
		data.accel_z = ((rx_buf[5] << 8) | (rx_buf[4] & 0xC0));

		/* sensitivty 0.5C, center temprature 24C */
		data.temp = (signed char)rx_buf[6] + 24*2;
	} else {
		/* Master mode */
		FIFO_ACCESS_MUTEX_LOCK();
		rc = pdata->read_axis_data(dd->ic_dev, &rx_buf[0], 6);
		FIFO_ACCESS_MUTEX_UNLOCK();
		if (rc) {
			if (retrycount > BMA250_READDATA_RETRY)
				goto report_error;
			else {
				retrycount++;
				msleep(BMA250_BYPASS_STABEL);
				goto read_data_start;
			}
		}
		/* 10bit signed to 16bit signed */
		data.accel_x = ((rx_buf[1] << 8) | (rx_buf[0] & 0xC0));
		data.accel_y = ((rx_buf[3] << 8) | (rx_buf[2] & 0xC0));
		data.accel_z = ((rx_buf[5] << 8) | (rx_buf[4] & 0xC0));
		/* set defaults value. */
		data.temp = 32*2;
	}

	/* sensitivty 256lsb/g for all g-ranges */
	data.accel_x = data.accel_x >> dd->shift;
	data.accel_y = data.accel_y >> dd->shift;
	data.accel_z = data.accel_z >> dd->shift;

	input_report_abs(dd->ip_dev, ABS_X, data.accel_x);
	input_report_abs(dd->ip_dev, ABS_Y, data.accel_y);
	input_report_abs(dd->ip_dev, ABS_Z, data.accel_z);
	input_report_abs(dd->ip_dev, ABS_MISC, data.temp);
	input_sync(dd->ip_dev);
	return rc;

report_error:
	printk(KERN_ERR "%s: Data read error from register.\n", __func__);
	return rc;
}

static void bma250_work_f(struct work_struct *work)
{
	int                rc;
	struct driver_data *dd = container_of(work, struct driver_data,
						work_data.work);

	if (dd->power) {
		rc = bma250_report_data(dd);
		schedule_delayed_work(&dd->work_data, dd->delay_jiffies);
	}
	return ;
}

static int bma250_open(struct input_dev *dev)
{
	int                 rc = 0;
	struct driver_data *dd = input_get_drvdata(dev);

	rc = bma250_power_up(dd);
	if (rc)
		return rc;
	rc = bma250_config(dd, TRUE, TRUE);
	schedule_delayed_work(&dd->work_data, dd->delay_jiffies);

	return rc;
}

static void bma250_release(struct input_dev *dev)
{
	struct driver_data *dd = input_get_drvdata(dev);

	bma250_power_down(dd);
}

static int __devinit bma250_probe(struct i2c_client *ic_dev,
		const struct i2c_device_id *id)
{
	struct driver_data *dd;
	int                 rc;
	struct bma250_platform_data *pdata = ic_dev->dev.platform_data;

	if (!pdata || !pdata->hw_config)
		return -ENODEV;

	if (pdata->bypass_state && pdata->read_axis_data
		&& pdata->check_sleep_status && pdata->vote_sleep_status)
		slave_hw = TRUE;
	else if (!pdata->bypass_state && !pdata->read_axis_data
		&& !pdata->check_sleep_status && !pdata->vote_sleep_status)
		slave_hw = FALSE;
	else
		return -ENODEV;

	dd = kzalloc(sizeof(struct driver_data), GFP_KERNEL);
	if (!dd) {
		rc = -ENOMEM;
		goto probe_exit;
	}

	mutex_lock(&bma250_dd_lock);
	list_add_tail(&dd->next_dd, &dd_list);
	mutex_unlock(&bma250_dd_lock);
	dd->ic_dev = ic_dev;

	INIT_DELAYED_WORK(&dd->work_data, bma250_work_f);

	dd->pdata = pdata;

	/* initial configuration */
	dd->rate = pdata->rate;
	dd->delay_jiffies = msecs_to_jiffies(dd->rate);
	dd->bw_sel = pdata->reg->bw_sel;
	dd->range = pdata->reg->range;

	rc = bma250_power_up(dd);
	if (rc)
		goto probe_err_cfg;
	rc = bma250_hwid(dd);
	bma250_power_down(dd);
	if (rc)
		goto probe_err_cfg;

	bma250_create_dbfs_entry(dd);
	bma250_ic_set_data(ic_dev, dd);

	dd->ip_dev = input_allocate_device();
	if (!dd->ip_dev) {
		rc = -ENOMEM;
		goto probe_err_reg;
	}
	input_set_drvdata(dd->ip_dev, dd);
	dd->ip_dev->open       = bma250_open;
	dd->ip_dev->close      = bma250_release;
	dd->ip_dev->name       = BMA250_NAME;
	dd->ip_dev->id.vendor  = BMA250_VENDORID;
	dd->ip_dev->id.product = 1;
	dd->ip_dev->id.version = 1;
	__set_bit(EV_ABS,       dd->ip_dev->evbit);
	__set_bit(ABS_X,        dd->ip_dev->absbit);
	__set_bit(ABS_Y,        dd->ip_dev->absbit);
	__set_bit(ABS_Z,        dd->ip_dev->absbit);
	__set_bit(ABS_MISC,     dd->ip_dev->absbit);
	input_set_abs_params(dd->ip_dev, ABS_X, -4096, 4095, 0, 0);
	input_set_abs_params(dd->ip_dev, ABS_Y, -4096, 4095, 0, 0);
	input_set_abs_params(dd->ip_dev, ABS_Z, -4096, 4095, 0, 0);
	input_set_abs_params(dd->ip_dev, ABS_MISC, -80, 175, 0, 0);

	rc = input_register_device(dd->ip_dev);
	if (rc) {
		input_free_device(dd->ip_dev);
		goto probe_err_reg;
	}

	rc = add_sysfs_interfaces(&dd->ip_dev->dev);
	if (rc)
		goto probe_err_sysfs;

	return rc;

probe_err_sysfs:
	input_unregister_device(dd->ip_dev);
probe_err_reg:
	bma250_remove_dbfs_entry(dd);
	bma250_ic_set_data(ic_dev, NULL);
probe_err_cfg:
	mutex_lock(&bma250_dd_lock);
	list_del(&dd->next_dd);
	mutex_unlock(&bma250_dd_lock);
	kfree(dd);
probe_exit:
	return rc;
}

static int __devexit bma250_remove(struct i2c_client *ic_dev)
{
	struct driver_data *dd = bma250_ic_get_data(ic_dev);
	int                 rc;

	rc = bma250_power_down(dd);
	if (rc)
		dev_err(&dd->ip_dev->dev,
			"%s: power down failed with error %d\n",
			__func__, rc);
	remove_sysfs_interfaces(&dd->ip_dev->dev);
	input_unregister_device(dd->ip_dev);
	bma250_remove_dbfs_entry(dd);
	bma250_ic_set_data(ic_dev, NULL);
	mutex_lock(&bma250_dd_lock);
	list_del(&dd->next_dd);
	mutex_unlock(&bma250_dd_lock);
	kfree(dd);
	return 0;
}

static const struct i2c_device_id bma250_i2c_id[] = {
	{BMA250_NAME, 0},
	{}
};

static struct i2c_driver bma250_driver = {
	.driver = {
		.name  = BMA250_NAME,
		.owner = THIS_MODULE,
	},
	.probe         = bma250_probe,
	.remove        = __devexit_p(bma250_remove),
	.suspend       = bma250_suspend,
	.resume        = bma250_resume,
	.id_table      = bma250_i2c_id,
};


static int __init bma250_init(void)
{
	INIT_LIST_HEAD(&dd_list);
	mutex_init(&bma250_dd_lock);
	mutex_init(&bma250_power_lock);
	return i2c_add_driver(&bma250_driver);
}
module_init(bma250_init);

static void __exit bma250_exit(void)
{
	i2c_del_driver(&bma250_driver);
}
module_exit(bma250_exit);
