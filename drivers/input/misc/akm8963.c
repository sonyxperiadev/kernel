/* drivers/input/misc/akm8963.c - AK8963 compass driver
 *
 * Copyright (C) 2012 ASAHI KASEI MICRODEVICES CORPORATION.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Authors: Rikita Yamada <yamada.rj (at) om.asahi-kasei.co.jp>
 *          Takashi Shiina <takashi.shiina (at) sonymobile.com>
 *          Masashi Shimizu <Masashi.X.Shimizu (at) sonymobile.com>
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
#include <linux/akm8963.h>
#include <linux/delay.h>
#include <linux/freezer.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

/* \name AK8963 operation mode
 \anchor AK8963_Mode
 Defines an operation mode of the AK8963.
*/
#define AK8963_MODE_SNG_MEASURE		0x01
#define AK8963_MODE_CNT_NORMAL		0x02
#define AK8963_MODE_CNT_FASTEST		0x06
#define AK8963_MODE_SELF_TEST		0x08
#define AK8963_MODE_FUSE_ACCESS		0x0F
#define AK8963_MODE_POWERDOWN		0x00
#define AK8963_MODE_RESET		0x01
#define AK8963_MODE_14BIT		0x00
#define AK8963_MODE_16BIT		0x10

#define SENSOR_DATA_SIZE	8	/* Rx buffer size */
#define RWBUF_SIZE		16	/* Read/Write buffer size.*/
#define FUSEROM_SIZE		3   /* Read only fuse ROM area size. */

#if SENSOR_DATA_SIZE < 8
#error SENSOR_DATA_SIZE is too small.
#endif

/* \name AK8963 register address
 \anchor AK8963_REG
 Defines a register address of the AK8963.
*/
#define AK8963_REG_WIA		0x00
#define AK8963_REG_INFO		0x01
#define AK8963_REG_ST1		0x02
#define AK8963_REG_HXL		0x03
#define AK8963_REG_HXH		0x04
#define AK8963_REG_HYL		0x05
#define AK8963_REG_HYH		0x06
#define AK8963_REG_HZL		0x07
#define AK8963_REG_HZH		0x08
#define AK8963_REG_ST2		0x09
#define AK8963_REG_CNTL1	0x0A
#define AK8963_REG_CNTL2	0x0B
#define AK8963_REG_ASTC		0x0C
#define AK8963_REG_TS1		0x0D
#define AK8963_REG_TS2		0x0E
#define AK8963_REG_I2CDIS	0x0F

#define AK8963_DEVICE_ID	0x48
#define AK8963_ST1_NORMAL	0x00
#define AK8963_ST1_DATA_READY	0x01

#define UCHAR_MIN	0
#define UCHAR_MAX	255
#define AKM_MAG_MAXVAL  32767
#define AKM_MAG_MINVAL  -32768
#define AKM_MAG_MAXST2  24
#define AKM_MAG_MINST2  0

/* \name AK8963 fuse-rom address
 \anchor AK8963_FUSE
 Defines a read-only address of the fuse ROM of the AK8963.
*/
#define AK8963_FUSE_ASAX	0x10
#define AK8963_FUSE_ASAY	0x11
#define AK8963_FUSE_ASAZ	0x12

#define AKM8963_WAIT_TIME_MS	30
#define AKM8963_MODE_DELAY_US	100
#define AKM8963_POR_DELAY_US	100

#define AK8963_DATA_SIZE (AK8963_REG_ST2 - AK8963_REG_WIA + 1)
#define AK8963_FUSE_SIZE (AK8963_FUSE_ASAZ - AK8963_FUSE_ASAX + 1)
#define AK8963_REGS_SIZE (AK8963_DATA_SIZE + AK8963_FUSE_SIZE)

struct akm8963_data {
	struct i2c_client	*i2c;
	struct input_dev	*input;
	struct device		*class_dev;
	struct delayed_work	work;

	/* This value represents current operation mode */
	/* 0x00: power down mode
	 * 0x01: single measurement mode
	 * 0x02: continuous mode @ 8Hz.
	 * 0x06: continuous mode @ 100Hz.
	 * 0x08: selft-test mode
	 */
	atomic_t	mode;
	atomic_t	mode_rsv;
	/* Software continuous measurement interval in millisecond */
	atomic_t	interval;
	/* Register address to be read */
	atomic_t	reg_addr;
	/* Output data bit */
	atomic_t	obit;

	/* A buffer to save FUSE ROM value */
	unsigned char	fuse[FUSEROM_SIZE];
	struct akm8963_platform_data *pdata;
};


static int aki2c_rxdata(struct i2c_client *i2c, unsigned char *rxdata,
						int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr = i2c->addr,
			.flags = 0,
			.len = 1,
			.buf = rxdata,
		},
		{
			.addr = i2c->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxdata,
		},
	};

	ret = i2c_transfer(i2c->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret < 0) {
		dev_err(&i2c->dev, "%s: transfer failed.\n", __func__);
		return ret;
	} else if (ret != ARRAY_SIZE(msgs)) {
		dev_err(&i2c->dev, "%s: transfer failed(size error).\n",
			__func__);
		return -ENXIO;
	}

	dev_vdbg(&i2c->dev, "RxData: len=%02x, addr=%02x  data=%02x",
		length, rxdata[0], rxdata[1]);
	return 0;
}

static int aki2c_txdata(struct i2c_client *i2c, unsigned char *txdata,
						int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr = i2c->addr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	ret = i2c_transfer(i2c->adapter, msg, ARRAY_SIZE(msg));
	if (ret < 0) {
		dev_err(&i2c->dev, "%s: transfer failed.", __func__);
		return ret;
	} else if (ret != ARRAY_SIZE(msg)) {
		dev_err(&i2c->dev, "%s: transfer failed(size error).",
			__func__);
		return -ENXIO;
	}

	dev_vdbg(&i2c->dev, "TxData: len=%02x, addr=%02x data=%02x",
		length, txdata[0], txdata[1]);
	return 0;
}

static int akecs_set_cntl1(struct akm8963_data *akm,
		unsigned char cntl1)
{
	unsigned char buffer[2];
	unsigned char obit;
	int err;

	obit = atomic_read(&akm->obit);

	buffer[0] = AK8963_REG_CNTL1;
	buffer[1] = cntl1 | (obit << 4);

	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err)
		dev_err(&akm->i2c->dev,
			"%s: Can not set %d to CNTL1.\n", __func__, cntl1);

	return err;
}

static int akecs_setmode_sngmeasure(struct akm8963_data *akm)
{
	if (atomic_cmpxchg(&akm->mode,
				AK8963_MODE_POWERDOWN, AK8963_MODE_SNG_MEASURE)
			!= AK8963_MODE_POWERDOWN) {
		dev_err(&akm->i2c->dev, "%s: device is busy\n", __func__);
		return -EBUSY;
	}

	return akecs_set_cntl1(akm, AK8963_MODE_SNG_MEASURE);
}

static int akecs_setmode_continuous_normal(struct akm8963_data *akm)
{
	if (atomic_cmpxchg(&akm->mode,
				AK8963_MODE_POWERDOWN, AK8963_MODE_CNT_NORMAL)
			!= AK8963_MODE_POWERDOWN) {
		dev_err(&akm->i2c->dev, "%s: device is busy\n", __func__);
		return -EBUSY;
	}

	return akecs_set_cntl1(akm, AK8963_MODE_CNT_NORMAL);
}

static int akecs_setmode_continuous_fastest(struct akm8963_data *akm)
{
	if (atomic_cmpxchg(&akm->mode,
				AK8963_MODE_POWERDOWN, AK8963_MODE_CNT_FASTEST)
			!= AK8963_MODE_POWERDOWN) {
		dev_err(&akm->i2c->dev, "%s: device is busy\n", __func__);
		return -EBUSY;
	}

	return akecs_set_cntl1(akm, AK8963_MODE_CNT_FASTEST);
}

static int akecs_setmode_selftest(struct akm8963_data *akm,
				  unsigned char astc)
{
	unsigned char buffer[2];
	int err;

	if (atomic_cmpxchg(&akm->mode,
				AK8963_MODE_POWERDOWN, AK8963_MODE_SELF_TEST)
			!= AK8963_MODE_POWERDOWN) {
		dev_err(&akm->i2c->dev, "%s: device is busy\n", __func__);
		return -EBUSY;
	}

	buffer[0] = AK8963_REG_ASTC;
	buffer[1] = astc;
	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not activate self test.\n", __func__);
		atomic_set(&akm->mode, AK8963_MODE_POWERDOWN);
		return err;
	}

	return akecs_set_cntl1(akm, AK8963_MODE_SELF_TEST);
}

/* If power down mode is set while DRDY is HIGH,
  (i.e. before work que function read out the measurement data)
  DRDY bit is reset to 0, then work que function will fail.*/
static int akecs_setmode_powerdown(struct akm8963_data *akm)
{
	unsigned char buffer[2];
	int err;

	if (0 <= atomic_read(&akm->interval)) {
		cancel_delayed_work_sync(&akm->work);
		atomic_set(&akm->interval, -1);
	}

	buffer[0] = AK8963_REG_CNTL1;
	buffer[1] = AK8963_MODE_POWERDOWN;

	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err)
		dev_err(&akm->i2c->dev,
			"%s: Can not set to powerdown.\n", __func__);

	udelay(AKM8963_MODE_DELAY_US);

	atomic_set(&akm->mode, AK8963_MODE_POWERDOWN);
	return err;
}

/* Reset function is also same behavior as powerdown mode */
static int akecs_setmode_reset(struct akm8963_data *akm)
{
	unsigned char buffer[2];
	int err;

	if (0 <= atomic_read(&akm->interval)) {
		cancel_delayed_work_sync(&akm->work);
		atomic_set(&akm->interval, -1);
	}

	buffer[0] = AK8963_REG_CNTL2;
	buffer[1] = AK8963_MODE_RESET;

	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err)
		dev_err(&akm->i2c->dev,
			"%s: Can not reset the device.\n", __func__);

	/* Wait for bootup the device */
	udelay(AKM8963_POR_DELAY_US);

	atomic_set(&akm->mode, AK8963_MODE_POWERDOWN);
	atomic_set(&akm->mode_rsv, AK8963_MODE_POWERDOWN);
	atomic_set(&akm->interval, -1);
	atomic_set(&akm->reg_addr, 0);
	atomic_set(&akm->obit, 1);
	return err;
}

/* No busy check */
/* This function is called only once in initialization. */
static int akecs_checkdevice(struct akm8963_data *akm)
{
	unsigned char buffer;
	int err;

	buffer = AK8963_REG_WIA;
	err = aki2c_rxdata(akm->i2c, &buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not read WIA.\n", __func__);
		return err;
	}

	if (buffer != AK8963_DEVICE_ID) {
		dev_err(&akm->i2c->dev,
			"%s: The device is not AK8963.\n", __func__);
		return -ENXIO;
	}

	return err;
}

/* No busy check */
/* This function is called only once in initialization. */
static int akecs_read_fuse(struct akm8963_data *akm)
{
	unsigned char buffer[2];
	int err;

	buffer[0] = AK8963_REG_CNTL1;
	buffer[1] = AK8963_MODE_FUSE_ACCESS;
	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not set to fuse access mode.", __func__);
		return err;
	}

	akm->fuse[0] = AK8963_FUSE_ASAX;
	err = aki2c_rxdata(akm->i2c, akm->fuse, FUSEROM_SIZE);
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not read the FUSE-ROM.", __func__);
		return err;
	}

	dev_info(&akm->i2c->dev, "%s: FUSE = 0x%02x, 0x%02x, 0x%02x",
		__func__, akm->fuse[0], akm->fuse[1], akm->fuse[2]);

	return akecs_setmode_powerdown(akm);
}

static irqreturn_t akm8963_irq(int irq, void *handle)
{
	struct akm8963_data *akm = handle;
	unsigned char buffer[SENSOR_DATA_SIZE];
	int mode;
	int err;
	s16 val_hx, val_hy, val_hz, val_st2;

	dev_vdbg(&akm->i2c->dev, "%s called\n", __func__);

	buffer[0] = AK8963_REG_ST1;
	err = aki2c_rxdata(akm->i2c, buffer, SENSOR_DATA_SIZE);
	if (err)
		goto work_func_end;

	/* Check ST bit */
	if ((buffer[0] & AK8963_ST1_DATA_READY) != AK8963_ST1_DATA_READY) {
		dev_err(&akm->i2c->dev, "%s: DRDY is not set", __func__);
		goto work_func_end;
	}

	mode = atomic_read(&akm->mode);

	/* If operation is already canceled, don't report values */
	if (mode == AK8963_MODE_POWERDOWN)
		goto work_func_end;

	/* If one shot operation, set to powerdown state */
	if ((AK8963_MODE_SNG_MEASURE == mode) ||
			(AK8963_MODE_SELF_TEST == mode))
		atomic_set(&akm->mode, AK8963_MODE_POWERDOWN);

	/* report axis data: HXL & HXH / HYL & HYH / HZL & HZH */
	val_hx = (s16)(((s16)buffer[2] << 8) | (s16)buffer[1]);
	val_hy = (s16)(((s16)buffer[4] << 8) | (s16)buffer[3]);
	val_hz = (s16)(((s16)buffer[6] << 8) | (s16)buffer[5]);
	val_st2 = (s16)buffer[7];

	/* Limit following values */
	if (val_hx < AKM_MAG_MINVAL)
		val_hx = AKM_MAG_MINVAL;
	if (val_hx > AKM_MAG_MAXVAL)
		val_hx = AKM_MAG_MAXVAL;
	if (val_hy < AKM_MAG_MINVAL)
		val_hy = AKM_MAG_MINVAL;
	if (val_hy > AKM_MAG_MAXVAL)
		val_hy = AKM_MAG_MAXVAL;
	if (val_hz < AKM_MAG_MINVAL)
		val_hz = AKM_MAG_MINVAL;
	if (val_hz > AKM_MAG_MAXVAL)
		val_hz = AKM_MAG_MAXVAL;
	if (val_st2 < AKM_MAG_MINST2)
		val_st2 = AKM_MAG_MINST2;
	if (val_st2 > AKM_MAG_MAXST2)
		val_st2 = AKM_MAG_MAXST2;

	dev_vdbg(&akm->i2c->dev, "hval = %d,%d,%d: ST2 = 0x%02X",
		val_hx, val_hy, val_hz, val_st2);

	input_event(akm->input, EV_MSC, MSC_RX,  val_hx);
	input_event(akm->input, EV_MSC, MSC_RY,  val_hy);
	input_event(akm->input, EV_MSC, MSC_RZ,  val_hz);
	input_event(akm->input, EV_MSC, MSC_ST2, val_st2);
	input_sync(akm->input);

work_func_end:
	return IRQ_HANDLED;
}

static void akm8963_continuous_measure(struct work_struct *work)
{
	struct akm8963_data *akm =
		container_of(work, struct akm8963_data, work.work);
	int interval;

	dev_vdbg(&akm->i2c->dev, "%s called\n", __func__);

	interval = atomic_read(&akm->interval);

	if (0 <= interval) {
		schedule_delayed_work(&akm->work,
				msecs_to_jiffies(interval));

		akecs_setmode_sngmeasure(akm);
	}
}

static int akm8963_device_power_on(struct akm8963_data *akm)
{
	int ret;

	dev_dbg(&akm->i2c->dev, "%s called\n", __func__);

	ret = akm->pdata->hw_config(&akm->i2c->dev, 1);

	return ret;
}

static void akm8963_device_power_off(struct akm8963_data *akm)
{
	dev_dbg(&akm->i2c->dev, "%s called\n", __func__);

	akm->pdata->hw_config(&akm->i2c->dev, 0);
}

static int akm8963_suspend(struct device *dev)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	int	mode;
	int ret = 0;

	dev_dbg(&akm->i2c->dev, "%s called\n", __func__);

	disable_irq(akm->i2c->irq);
	/* Save current status */
	mode = atomic_read(&akm->mode);
	atomic_set(&akm->mode_rsv, mode);

	if (akm->input->users) {
		ret = akecs_setmode_powerdown(akm);
		akm8963_device_power_off(akm);
	}

	return ret;
}

static int akm8963_resume(struct device *dev)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	int mode;
	int interval;
	int ret = 0;

	dev_dbg(&akm->i2c->dev, "%s called\n", __func__);

	if (akm->input->users) {
		ret = akm8963_device_power_on(akm);
		/* Revert previous status */
		/* But, ignore one shot operation */
		mode = atomic_read(&akm->mode_rsv);
		interval = atomic_read(&akm->interval);

		if (AK8963_MODE_CNT_NORMAL == mode)
			akecs_setmode_continuous_normal(akm);
		else if (AK8963_MODE_CNT_FASTEST == mode)
			akecs_setmode_continuous_fastest(akm);
		else if (0 <= interval) {
			schedule_delayed_work(&akm->work,
				msecs_to_jiffies(interval));
		}
	}

	enable_irq(akm->i2c->irq);

	return ret;
}

/*****************************************************************************
 *
 * SysFS attribute functions
 *
 * files :
 *  - interval   [rw] [t] : store measurement interval
 *  - obit       [rw] [t] : set output data bit
 *  - selftest   [rw] [t] : device's self test mode
 *  - single     [w]  [t] : single-shot trigger
 *  - continuous [w]  [t] : continuous measurement
 *  - reset      [w]  [t] : soft reset
 *  - registers  [r]  [b] : get all registers value
 *  - regacc     [rw] [b] : provide direct register access
 *
 * [b] = binary format
 * [t] = text format
 *
 */

/*********** interval (TEXT) ***********/
static ssize_t attr_interval_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	dev_dbg(dev, "%s called\n", __func__);
	return scnprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&akm->interval));
}

static ssize_t attr_interval_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	int mode;
	long interval = 0;

	dev_dbg(dev, "%s called\n", __func__);

	if (0 == count)
		return 0;

	mode = atomic_read(&akm->mode);
	if ((AK8963_MODE_SNG_MEASURE != mode) &&
			(AK8963_MODE_POWERDOWN != mode))
		return -EBUSY;

	if (strict_strtol(buf, 10, &interval))
		return -EINVAL;

	if ((0 <= interval) && (interval < AKM8963_WAIT_TIME_MS))
		interval = AKM8963_WAIT_TIME_MS;

	atomic_set(&akm->interval, interval);

	if (akm->input->users) {
		if (0 <= interval)
			schedule_delayed_work(
				&akm->work,
				msecs_to_jiffies(interval));
		else
			cancel_delayed_work_sync(&akm->work);
	}

	return count;
}

/*********** obit (TEXT) ***********/
static ssize_t attr_obit_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	ssize_t res;

	dev_dbg(dev, "%s called\n", __func__);

	if (atomic_read(&akm->obit))
		res = scnprintf(buf, PAGE_SIZE, "16 bit");
	else
		res = scnprintf(buf, PAGE_SIZE, "14 bit");

	return res;
}

static ssize_t attr_obit_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	int mode;
	long obit = 0;

	dev_dbg(dev, "%s called\n", __func__);

	mode = atomic_read(&akm->mode);

	if ((AK8963_MODE_SNG_MEASURE != mode) &&
			(AK8963_MODE_POWERDOWN != mode))
		return -EBUSY;

	if (strict_strtol(buf, 10, &obit))
		return -EINVAL;

	if (obit)
		atomic_set(&akm->obit, 1);
	else
		atomic_set(&akm->obit, 0);

	return count;
}

/*********** selftest (TEXT) ***********/
static ssize_t attr_selftest_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	dev_dbg(dev, "%s called\n", __func__);
	return scnprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&akm->mode));
}

static ssize_t attr_selftest_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	long astc_value = 0;
	int err;

	dev_dbg(dev, "%s called\n", __func__);

	if (strict_strtol(buf, 10, &astc_value))
		return -EINVAL;

	if ((astc_value < UCHAR_MIN) || (UCHAR_MAX < astc_value))
		return -EINVAL;

	if (0 <= atomic_read(&akm->interval))
			return -EBUSY;

	err = akecs_setmode_selftest(akm, (unsigned char)astc_value);
	if (err)
		return err;

	return count;
}

/*********** single (TEXT) ***********/
static ssize_t attr_single_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	long single_value;
	int err;

	dev_dbg(dev, "%s called\n", __func__);

	if (0 == count)
		return 0;

	if (strict_strtol(buf, 10, &single_value))
		return -EINVAL;

	if (0 == single_value)
		return count;

	if (0 <= atomic_read(&akm->interval))
			return -EBUSY;

	err = akecs_setmode_sngmeasure(akm);
	if (err)
		return err;

	return count;
}

/*********** continuous (TEXT) ***********/
static ssize_t attr_continuous_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	long cont_value;
	int err;

	dev_dbg(dev, "%s called\n", __func__);

	if (strict_strtol(buf, 10, &cont_value))
		return -EINVAL;

	if (0 <= atomic_read(&akm->interval))
			return -EBUSY;

	if (cont_value == 1)
		err = akecs_setmode_continuous_fastest(akm);
	else if (cont_value == 0)
		err = akecs_setmode_continuous_normal(akm);
	else if (cont_value < 0)
		err = akecs_setmode_powerdown(akm);
	else
		err = -EINVAL;

	if (err)
		return err;

	return count;
}

/*********** reset (TEXT) ***********/
static ssize_t attr_reset_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct akm8963_data *akm = dev_get_drvdata(dev);
	long reset_value;
	int err;

	dev_dbg(dev, "%s called\n", __func__);

	if (0 == count)
		return 0;

	if (strict_strtol(buf, 10, &reset_value))
		return -EINVAL;

	if (0 == reset_value)
		return count;

	err = akecs_setmode_reset(akm);
	if (err)
		return err;

	return count;
}

/*********** registers (BINARY) ***********/
static ssize_t bin_attr_registers_read(struct file *file,
				       struct kobject *kobj,
				       struct bin_attribute *attr,
				       char *buf, loff_t pos, size_t size)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct akm8963_data *akm = dev_get_drvdata(dev);
	unsigned char reg[AK8963_DATA_SIZE];
	int err;

	dev_dbg(dev, "%s called\n", __func__);

	if (0 == size)
		return 0;

	reg[0] = AK8963_REG_WIA;
	err = aki2c_rxdata(akm->i2c, reg, AK8963_DATA_SIZE);

	if (err)
		return err;

	if (AK8963_DATA_SIZE > size) {
		memcpy(&buf[0], reg, size);
	} else {
		memcpy(&buf[0], reg, AK8963_DATA_SIZE);
		if (AK8963_REGS_SIZE > size) {
			memcpy(&buf[AK8963_DATA_SIZE], akm->fuse,
				   size - AK8963_DATA_SIZE);
		} else {
			memcpy(&buf[AK8963_DATA_SIZE], akm->fuse,
			       AK8963_FUSE_SIZE);
		}
	}

	return (size < AK8963_REGS_SIZE) ? size : AK8963_REGS_SIZE;
}

/*********** regacc (BINARY) ***********/
static ssize_t bin_attr_regacc_read(struct file *file, struct kobject *kobj,
				    struct bin_attribute *attr, char *buf,
				    loff_t pos, size_t size)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct akm8963_data *akm = dev_get_drvdata(dev);
	unsigned char reg;
	int err;

	dev_dbg(dev, "%s called\n", __func__);

	if (size == 0)
		return 0;

	reg = atomic_read(&akm->reg_addr);

	if ((AK8963_FUSE_ASAX <= reg) && (reg <= AK8963_FUSE_ASAZ)) {
		buf[0] = akm->fuse[reg - AK8963_FUSE_ASAX];
	} else {
		err = aki2c_rxdata(akm->i2c, &reg, sizeof(reg));
		if (err)
			return err;

		buf[0] = reg;
	}

	return 1;
}

static ssize_t bin_attr_regacc_write(struct file *file, struct kobject *kobj,
				     struct bin_attribute *attr, char *buf,
				     loff_t pos, size_t size)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct akm8963_data *akm = dev_get_drvdata(dev);

	dev_dbg(dev, "%s called\n", __func__);

	if (size == 0)
		return 0;

	if ((buf[0] < AK8963_REG_WIA) || (AK8963_FUSE_ASAZ < buf[0]))
		return -ERANGE;

	atomic_set(&akm->reg_addr, buf[0]);

	return size;
}

#define __BIN_ATTR(name_, mode_, size_, private_, read_, write_) \
	{ \
		.attr	= { .name = __stringify(name_), .mode = mode_ }, \
		.size	= size_, \
		.private = private_, \
		.read	= read_, \
		.write   = write_, \
	}

static struct device_attribute akm8963_attributes[] = {
	__ATTR(interval,   0600, attr_interval_show, attr_interval_store),
	__ATTR(obit,       0600, attr_obit_show,     attr_obit_store),
	__ATTR(selftest,   0600, attr_selftest_show, attr_selftest_store),
	__ATTR(single,     0200, NULL,               attr_single_store),
	__ATTR(continuous, 0200, NULL,               attr_continuous_store),
	__ATTR(reset,      0200, NULL,               attr_reset_store),
};

static struct bin_attribute akm8963_bin_attributes[] = {
	__BIN_ATTR(registers, 0400, 13, NULL, bin_attr_registers_read, NULL),
	__BIN_ATTR(regacc, 0600, 1, NULL,
		   bin_attr_regacc_read, bin_attr_regacc_write),
};

static int create_sysfs_interfaces(struct akm8963_data *akm)
{
	int i, j;
	struct device *dev = &akm->input->dev;

	for (i = 0; i < ARRAY_SIZE(akm8963_attributes); i++)
		if (device_create_file(dev, akm8963_attributes + i))
			goto device_create_file_failed;

	for (j = 0; j < ARRAY_SIZE(akm8963_bin_attributes); j++)
		if (sysfs_create_bin_file(&dev->kobj,
			akm8963_bin_attributes + j))
			goto device_create_bin_file_failed;

	return 0;

device_create_bin_file_failed:
	for (--j; j >= 0; j--)
		sysfs_remove_bin_file(&dev->kobj, akm8963_bin_attributes + j);
	dev_err(dev, "%s: failed to create binary sysfs interface\n", __func__);
device_create_file_failed:
	for (--i; i >= 0; i--)
		device_remove_file(dev, akm8963_attributes + i);
	dev_err(dev, "%s: failed to create sysfs interface\n", __func__);
	return -ENODEV;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(akm8963_attributes); i++)
		device_remove_file(dev, akm8963_attributes + i);

	for (i = 0; i < ARRAY_SIZE(akm8963_bin_attributes); i++)
		sysfs_remove_bin_file(&dev->kobj, akm8963_bin_attributes + i);

}

static int akm8963_open(struct input_dev *dev)
{
	struct akm8963_data *akm = input_get_drvdata(dev);
	int ret;
	int interval;

	ret = akm8963_device_power_on(akm);
	if (ret) {
		dev_err(&akm->i2c->dev, "%s: power on failed.\n", __func__);
		goto exit;
	}

	interval = atomic_read(&akm->interval);
	if (0 <= interval)
		schedule_delayed_work(
			&akm->work,
			msecs_to_jiffies(interval));

	enable_irq(akm->i2c->irq);

exit:	return ret;
}

static void akm8963_close(struct input_dev *dev)
{
	struct akm8963_data *akm = input_get_drvdata(dev);

	disable_irq(akm->i2c->irq);
	akecs_setmode_powerdown(akm);
	akm8963_device_power_off(akm);
}

int akm8963_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct akm8963_data *akm;
	int err = 0;
	struct akm8963_platform_data *pdata = client->dev.platform_data;

	dev_dbg(&client->dev, "%s called\n", __func__);

	if (!pdata || !pdata->hw_config || !pdata->setup || !pdata->shutdown) {
		printk(KERN_ERR "%s: platform data is NULL\n", __func__);
		err = -ENOMEM;
		goto err_pdata;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,
			"%s: check_functionality failed.", __func__);
		err = -ENODEV;
		goto err_i2c_check;
	}

	akm = kzalloc(sizeof(struct akm8963_data), GFP_KERNEL);
	if (!akm) {
		dev_err(&client->dev,
			"%s: memory allocation failed.", __func__);
		err = -ENOMEM;
		goto err_kzalloc;
	}
	akm->i2c = client;
	akm->pdata = pdata;

	atomic_set(&akm->mode, AK8963_MODE_POWERDOWN);
	atomic_set(&akm->mode_rsv, AK8963_MODE_POWERDOWN);
	atomic_set(&akm->interval, -1);
	atomic_set(&akm->reg_addr, 0);
	atomic_set(&akm->obit, 1);

	err = akm->pdata->setup(&akm->i2c->dev);
	if (err) {
		dev_err(&client->dev, "%s: setup failed\n", __func__);
		goto err_setup_failed;
	}

	err = akm8963_device_power_on(akm);
	if (err) {
		dev_err(&client->dev,
			"%s: device power on failed.\n", __func__);
		goto err_power_on;
	}

	err = akecs_checkdevice(akm);
	if (err < 0) {
		akm8963_device_power_off(akm);
		goto err_check_device;
	}

	err = akecs_read_fuse(akm);
	akm8963_device_power_off(akm);
	if (err < 0)
		goto err_read_fuse;

	akm->input = input_allocate_device();
	if (!akm->input) {
		err = -ENOMEM;
		dev_err(&client->dev,
			"%s: Failed to allocate input device.", __func__);
		goto err_input_allocate_device;
	}

	input_set_drvdata(akm->input, akm);

	set_bit(EV_ABS, akm->input->evbit);
	akm->input->open = akm8963_open;
	akm->input->close = akm8963_close;
	akm->input->name = "compass";
	input_set_capability(akm->input, EV_MSC, MSC_RX);
	input_set_capability(akm->input, EV_MSC, MSC_RY);
	input_set_capability(akm->input, EV_MSC, MSC_RZ);
	input_set_capability(akm->input, EV_MSC, MSC_ST2);

	err = input_register_device(akm->input);
	if (err) {
		input_free_device(akm->input);
		dev_err(&client->dev,
			"%s: Unable to register input device.", __func__);
		goto err_input_register_device;
	}

	INIT_DELAYED_WORK(&akm->work, akm8963_continuous_measure);

	err = request_threaded_irq(client->irq, NULL, akm8963_irq,
				   IRQF_TRIGGER_RISING | IRQF_ONESHOT,
				   dev_name(&client->dev), akm);
	if (err) {
		dev_err(&client->dev, "%s: request irq failed.", __func__);
		goto err_request_th_irq;
	}

	disable_irq(client->irq);

	err = create_sysfs_interfaces(akm);
	if (err) {
		dev_err(&client->dev,
			"%s: create sysfs failed.", __func__);
		goto err_create_sysfs_interfaces;
	}

	i2c_set_clientdata(client, akm);

	dev_info(&client->dev, "successfully probed.");
	return 0;

err_create_sysfs_interfaces:
	free_irq(client->irq, akm);
err_request_th_irq:
	input_unregister_device(akm->input);
err_input_register_device:
err_input_allocate_device:
err_read_fuse:
err_check_device:
err_power_on:
err_setup_failed:
	kfree(akm);
err_kzalloc:
err_i2c_check:
err_pdata:
	return err;
}

static int akm8963_remove(struct i2c_client *client)
{
	struct akm8963_data *akm = i2c_get_clientdata(client);
	dev_dbg(&client->dev, "%s called\n", __func__);
	remove_sysfs_interfaces(&akm->input->dev);
	free_irq(client->irq, akm);
	input_unregister_device(akm->input);
	akm8963_device_power_off(akm);
	akm->pdata->shutdown(&akm->i2c->dev);
	kfree(akm);
	dev_info(&client->dev, "successfully removed.");
	return 0;
}

static const struct i2c_device_id akm8963_id[] = {
	{AKM8963_I2C_NAME, 0 },
	{ }
};

static const struct dev_pm_ops akm8963_pm_ops = {
	.suspend	= akm8963_suspend,
	.resume		= akm8963_resume,
};

static struct i2c_driver akm8963_driver = {
	.probe		= akm8963_probe,
	.remove		= akm8963_remove,
	.id_table	= akm8963_id,
	.driver = {
		.name = AKM8963_I2C_NAME,
		.pm = &akm8963_pm_ops,
	},
};

static int __init akm8963_init(void)
{
	return i2c_add_driver(&akm8963_driver);
}

static void __exit akm8963_exit(void)
{
	i2c_del_driver(&akm8963_driver);
}

module_init(akm8963_init);
module_exit(akm8963_exit);

MODULE_AUTHOR("AKM & SOMC");
MODULE_DESCRIPTION("AKM8963 compass driver");
MODULE_LICENSE("GPL");

