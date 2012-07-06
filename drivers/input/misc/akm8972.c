/* drivers/input/misc/akm8972.c - AK8972 compass driver
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 * Authors: Takashi Shiina <takashi.shiina (at) sonymobile.com>
 *          Tadashi Kubo <tadashi.kubo (at) sonymobile.com>
 *          Aleksej Makarov <Aleksej.Makarov (at) sonymobile.com>
 *          Joachim Holst <joachim.holst (at) sonymobile.com>
 *          Chikaharu Gonnokami <Chikaharu.X.Gonnokami (at) sonymobile.com>
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

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/akm8972.h>

#define AKM8972_BASE_NUM 10

#define AKM8972_WAIT_TIME_MS	30
#define MODE_CHANGE_DELAY 100

#if SENSOR_DATA_SIZE < 8
#error SENSOR_DATA_SIZE is too small.
#endif

#define AK8972_DATA_SIZE (AK8972_REG_ST2 - AK8972_REG_WIA + 1)
#define AK8972_FUSE_SIZE (AK8972_FUSE_ASAZ - AK8972_FUSE_ASAX + 1)
#define AK8972_REGS_SIZE (AK8972_DATA_SIZE + AK8972_FUSE_SIZE)

struct akm8972_data {
	struct i2c_client	*i2c;
	struct input_dev	*input;
	struct device		*class_dev;
	struct class		*compass;
	struct delayed_work	work;

	/* Prevent to start measure while measuring */
	atomic_t	is_busy;
	/* If it is positive value, sequential measurement is running */
	atomic_t	interval;
	/* Register address to be read */
	atomic_t	reg_addr;

	/* A buffer to save FUSE ROM value */
	unsigned char	fuse[FUSEROM_SIZE];
	struct akm8972_platform_data *pdata;
};


static int aki2c_rxdata(struct i2c_client *i2c, unsigned char *rxdata,
						int length)
{
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
	if (i2c_transfer(i2c->adapter, msgs, ARRAY_SIZE(msgs))
							!= ARRAY_SIZE(msgs)) {
		dev_err(&i2c->dev, "%s: transfer failed.\n", __func__);
		return -EIO;
	}

	dev_dbg(&i2c->dev, "RxData: len=%02x, addr=%02x  data=%02x",
		length, rxdata[0], rxdata[1]);
	return 0;
}

static int aki2c_txdata(struct i2c_client *i2c, unsigned char *txdata,
						int length)
{
	struct i2c_msg msg[] = {
		{
			.addr = i2c->addr,
			.flags = 0,
			.len = length,
			.buf = txdata,
		},
	};

	if (i2c_transfer(i2c->adapter, msg, ARRAY_SIZE(msg))
							!= ARRAY_SIZE(msg)) {
		dev_err(&i2c->dev, "%s: transfer failed.", __func__);
		return -EIO;
	}

	dev_dbg(&i2c->dev, "TxData: len=%02x, addr=%02x data=%02x",
		length, txdata[0], txdata[1]);
	return 0;
}

static int akecs_setmode_sngmeasure(struct akm8972_data *akm)
{
	unsigned char buffer[2];
	int err;

	if (atomic_cmpxchg(&akm->is_busy, 0, 1) != 0) {
		dev_err(&akm->i2c->dev, "%s: device is busy\n", __func__);
		return -EBUSY;
	}

	buffer[0] = AK8972_REG_CNTL;
	buffer[1] = AK8972_MODE_SNG_MEASURE;
	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not set to measurement mode.\n", __func__);
		atomic_set(&akm->is_busy, 0);
	}

	return err;
}

static int akecs_setmode_selftest(struct akm8972_data *akm,
				  unsigned char astc)
{
	unsigned char buffer[2];
	int err;

	if (atomic_cmpxchg(&akm->is_busy, 0, 1) != 0) {
		dev_err(&akm->i2c->dev, "%s: device is busy\n", __func__);
		return -EBUSY;
	}

	buffer[0] = AK8972_REG_ASTC;
	buffer[1] = astc;
	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not activate self test.\n", __func__);
		atomic_set(&akm->is_busy, 0);
		return err;
	}

	buffer[0] = AK8972_REG_CNTL;
	buffer[1] = AK8972_MODE_SELF_TEST;
	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not set to self test mode.\n", __func__);
		atomic_set(&akm->is_busy, 0);
	}

	return err;
}


/* If power down mode is set while DRDY is HIGH,
  (i.e. before work que function read out the measurement data)
  DRDY bit is reset to 0, then work que function will fail.*/
static int akecs_setmode_powerdown(struct akm8972_data *akm)
{
	unsigned char buffer[2];
	int err;

	buffer[0] = AK8972_REG_CNTL;
	buffer[1] = AK8972_MODE_POWERDOWN;
	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not set to powerdown.\n", __func__);
		return err;
	}

	udelay(MODE_CHANGE_DELAY);

	atomic_set(&akm->is_busy, 0);
	return err;
}

static int akecs_checkdevice(struct akm8972_data *akm)
{
	unsigned char buffer;
	int err;

	buffer = AK8972_REG_WIA;
	err = aki2c_rxdata(akm->i2c, &buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not read WIA.\n", __func__);
		return err;
	}

	if (buffer != AK8972_DEVICE_ID) {
		dev_err(&akm->i2c->dev,
			"%s: The device is not AK8972.\n", __func__);
		return -ENXIO;
	}

	return err;
}

static int akecs_read_fuse(struct akm8972_data *akm)
{
	unsigned char buffer[2];
	int err;

	memset(akm->fuse, 0, sizeof(akm->fuse));

	buffer[0] = AK8972_REG_CNTL;
	buffer[1] = AK8972_MODE_FUSE_ACCESS;
	err = aki2c_txdata(akm->i2c, buffer, sizeof(buffer));
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not set to fuse access mode.", __func__);
		return err;
	}

	akm->fuse[0] = AK8972_FUSE_ASAX;
	err = aki2c_rxdata(akm->i2c, akm->fuse, FUSEROM_SIZE);
	if (err) {
		dev_err(&akm->i2c->dev,
			"%s: Can not read the FUSE-ROM.", __func__);
		return err;
	}

	dev_dbg(&akm->i2c->dev, "%s: FUSE = 0x%02x, 0x%02x, 0x%02x",
		__func__, akm->fuse[0], akm->fuse[1], akm->fuse[2]);

	return akecs_setmode_powerdown(akm);
}

static irqreturn_t akm8972_irq(int irq, void *handle)
{
	struct akm8972_data *akm = handle;
	unsigned char buffer[SENSOR_DATA_SIZE];
	int err;
	s16 val_hx, val_hy, val_hz, val_st2;

	dev_vdbg(&akm->i2c->dev, "%s called\n", __func__);

	buffer[0] = AK8972_REG_ST1;
	err = aki2c_rxdata(akm->i2c, buffer, SENSOR_DATA_SIZE);
	if (err)
		goto work_func_end;

	/* Check ST bit */
	if ((buffer[0] & AK8972_ST1_DATA_READY) != AK8972_ST1_DATA_READY) {
		dev_err(&akm->i2c->dev, "%s: DRDY is not set", __func__);
		goto work_func_end;
	}

	atomic_set(&akm->is_busy, 0);

	/* report axis data: HXL & HXH / HYL & HYH / HZL & HZH */
	val_hx = (s16)(((s16)buffer[2] << 8) | (s16)buffer[1]);
	val_hy = (s16)(((s16)buffer[4] << 8) | (s16)buffer[3]);
	val_hz = (s16)(((s16)buffer[6] << 8) | (s16)buffer[5]);
	val_st2 = (s16)buffer[7];

	dev_dbg(&akm->i2c->dev, "hval = %d,%d,%d: ST2 = 0x%02X",
		val_hx, val_hy, val_hz, val_st2);

	input_report_abs(akm->input, ABS_RX, val_hx);
	input_report_abs(akm->input, ABS_RY, val_hy);
	input_report_abs(akm->input, ABS_RZ, val_hz);
	input_report_abs(akm->input, ABS_RUDDER, val_st2);
	input_sync(akm->input);

work_func_end:
	return IRQ_HANDLED;
}

static void akm8972_continuous_measure(struct work_struct *work)
{
	struct akm8972_data *akm =
		container_of(work, struct akm8972_data, work.work);
	int interval;

	dev_vdbg(&akm->i2c->dev, "%s called\n", __func__);

	interval = atomic_read(&akm->interval);
	if (0 <= interval) {
		schedule_delayed_work(&akm->work,
				msecs_to_jiffies(interval));

		akecs_setmode_sngmeasure(akm);
	}
}

static int akm8972_device_power_on(struct akm8972_data *akm)
{
	int ret;

	akm->pdata->hw_config(1);
	ret = akm->pdata->setup();
	if (ret)
		akm->pdata->hw_config(0);
	return ret;
}

static void akm8972_device_power_off(struct akm8972_data *akm)
{
	akm->pdata->shutdown();
	akm->pdata->hw_config(0);
}

static int akm8972_suspend(struct device *dev)
{
	struct akm8972_data *akm = dev_get_drvdata(dev);
	int ret = 0;

	dev_vdbg(&akm->i2c->dev, "%s called\n", __func__);

	if (akm->input->users) {
		cancel_delayed_work_sync(&akm->work);
		ret = akecs_setmode_powerdown(akm);
		akm8972_device_power_off(akm);
	}

	akm->pdata->power_mode(0);

	return ret;
}

static int akm8972_resume(struct device *dev)
{
	struct akm8972_data *akm = dev_get_drvdata(dev);
	int interval;
	int ret = 0;

	dev_vdbg(&akm->i2c->dev, "%s called\n", __func__);

	akm->pdata->power_mode(1);

	interval = atomic_read(&akm->interval);
	if ((0 <= interval) && (akm->input->users)) {
		ret = akm8972_device_power_on(akm);
		if (!ret) {
			dev_vdbg(&akm->i2c->dev, "%s power on failed.\n",
				__func__);
			schedule_delayed_work(&akm->work,
				msecs_to_jiffies(interval));
		}
	}

	return ret;
}

/***** internal utility functions for SysFS *****/
static int create_device_attributes(struct device *dev,
				    struct device_attribute *attrs)
{
	int i;
	int err = 0;

	for (i = 0 ; NULL != attrs[i].attr.name ; ++i) {
		err = device_create_file(dev, &attrs[i]);
		if (err)
			break;
	}

	if (err) {
		for (; i >= 0 ; --i)
			device_remove_file(dev, &attrs[i]);
	}

	return err;
}

static void remove_device_attributes(struct device *dev,
				     struct device_attribute *attrs)
{
	int i;

	for (i = 0 ; NULL != attrs[i].attr.name ; ++i)
		device_remove_file(dev, &attrs[i]);
}

static int create_device_binary_attributes(struct kobject *kobj,
					   struct bin_attribute *attrs)
{
	int i;
	int err = 0;

	for (i = 0 ; NULL != attrs[i].attr.name ; ++i) {
		err = sysfs_create_bin_file(kobj, &attrs[i]);
		if (err)
			break;
	}

	if (err) {
		for (; i >= 0 ; --i)
			sysfs_remove_bin_file(kobj, &attrs[i]);
	}

	return err;
}

static void remove_device_binary_attributes(struct kobject *kobj,
					    struct bin_attribute *attrs)
{
	int i;

	for (i = 0 ; NULL != attrs[i].attr.name ; ++i)
		sysfs_remove_bin_file(kobj, &attrs[i]);
}

/*****************************************************************************
 *
 * SysFS attribute functions
 *
 * directory : /sys/class/compass/akm8972/
 * files :
 *  - interval  [rw] [t] : store measurement interval
 *  - selftest  [rw] [t] : device's self test mode
 *  - single    [w]  [t] : single-shot trigger
 *  - registers [r]  [b] : get all registers value
 *  - regacc    [rw] [b] : provide direct register access
 *
 * [b] = binary format
 * [t] = text format
 *
 */

/*********** interval (TEXT) ***********/
static ssize_t attr_interval_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct akm8972_data *akm = dev_get_drvdata(dev);
	dev_vdbg(dev, "%s called\n", __func__);
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&akm->interval));
}

static ssize_t attr_interval_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct akm8972_data *akm = dev_get_drvdata(dev);
	long interval = 0;

	dev_vdbg(dev, "%s called\n", __func__);

	if (0 == count)
		return 0;

	if (strict_strtol(buf, AKM8972_BASE_NUM, &interval))
		return -EINVAL;

	if ((0 <= interval) && (interval < AKM8972_WAIT_TIME_MS))
		interval = AKM8972_WAIT_TIME_MS;

	atomic_set(&akm->interval, interval);

	if (0 <= interval)
		schedule_delayed_work(
			&akm->work,
			msecs_to_jiffies(interval));
	else
		cancel_delayed_work_sync(&akm->work);

	return count;
}

/*********** selftest (TEXT) ***********/
static ssize_t attr_selftest_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct akm8972_data *akm = dev_get_drvdata(dev);
	dev_vdbg(dev, "%s called\n", __func__);
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&akm->is_busy));
}

static ssize_t attr_selftest_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct akm8972_data *akm = dev_get_drvdata(dev);
	long astc_value = 0;
	int err;

	dev_vdbg(dev, "%s called\n", __func__);

	if (0 == count)
		return 0;

	if (strict_strtol(buf, AKM8972_BASE_NUM, &astc_value))
		return -EINVAL;

	if ((astc_value < UCHAR_MIN) || (UCHAR_MAX < astc_value))
		return -EINVAL;

	if ((0 != atomic_read(&akm->is_busy)) ||
		(0 <= atomic_read(&akm->interval)))
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
	struct akm8972_data *akm = dev_get_drvdata(dev);
	long single_value;
	int err;

	dev_vdbg(dev, "%s called\n", __func__);

	if (0 == count)
		return 0;

	if (strict_strtol(buf, AKM8972_BASE_NUM, &single_value))
		return -EINVAL;

	if (0 == single_value)
		return count;

	if ((0 != atomic_read(&akm->is_busy)) ||
		(0 <= atomic_read(&akm->interval)))
			return -EBUSY;

	err = akecs_setmode_sngmeasure(akm);
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
	struct akm8972_data *akm = dev_get_drvdata(dev);
	unsigned char reg[AK8972_DATA_SIZE];
	int err;

	dev_vdbg(dev, "%s called\n", __func__);

	if (0 == size)
		return 0;

	reg[0] = AK8972_REG_WIA;
	err = aki2c_rxdata(akm->i2c, reg, AK8972_DATA_SIZE);

	if (err)
		return err;

	if (AK8972_DATA_SIZE > size) {
		memcpy(&buf[0], reg, size);
	} else {
		memcpy(&buf[0], reg, AK8972_DATA_SIZE);
		if (AK8972_REGS_SIZE > size) {
			memcpy(&buf[AK8972_DATA_SIZE], akm->fuse,
				   size - AK8972_DATA_SIZE);
		} else {
			memcpy(&buf[AK8972_DATA_SIZE], akm->fuse,
			       AK8972_FUSE_SIZE);
		}
	}

	return (size < AK8972_REGS_SIZE) ? size : AK8972_REGS_SIZE;
}

/*********** regacc (BINARY) ***********/
static ssize_t bin_attr_regacc_read(struct file *file, struct kobject *kobj,
				    struct bin_attribute *attr, char *buf,
				    loff_t pos, size_t size)
{
	struct device *dev = container_of(kobj, struct device, kobj);
	struct akm8972_data *akm = dev_get_drvdata(dev);
	unsigned char reg;
	int err;

	dev_vdbg(dev, "%s called\n", __func__);

	if (size == 0)
		return 0;

	reg = atomic_read(&akm->reg_addr);

	if ((AK8972_FUSE_ASAX <= reg) && (reg <= AK8972_FUSE_ASAZ)) {
		buf[0] = akm->fuse[reg - AK8972_FUSE_ASAX];
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
	struct akm8972_data *akm = dev_get_drvdata(dev);
	unsigned char addr;

	dev_vdbg(dev, "%s called\n", __func__);

	if (size == 0)
		return 0;

	addr = (unsigned char)buf[0];

	if ((addr < AK8972_REG_WIA) || (AK8972_FUSE_ASAZ < addr))
		return -ERANGE;

	atomic_set(&akm->reg_addr, addr);

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

#define __BIN_ATTR_NULL \
	{ \
		.attr	= { .name = NULL }, \
	}

static struct device_attribute akm8972_attributes[] = {
	__ATTR(interval, 0600, attr_interval_show, attr_interval_store),
	__ATTR(selftest, 0600, attr_selftest_show, attr_selftest_store),
	__ATTR(single,   0200, NULL,               attr_single_store),
	__ATTR_NULL,
};

static struct bin_attribute akm8972_bin_attributes[] = {
	__BIN_ATTR(registers, 0400, 13, NULL, bin_attr_registers_read, NULL),
	__BIN_ATTR(regacc, 0600, 1, NULL,
		   bin_attr_regacc_read, bin_attr_regacc_write),
	__BIN_ATTR_NULL
};

static char const * const compass_class_name = "compass";
static char const * const akm8972_device_name = "akm8972";
static char const * const device_link_name = "i2c";
static const dev_t akm8972_device_dev_t = MKDEV(MISC_MAJOR, 240);

static int create_sysfs_interfaces(struct akm8972_data *akm)
{
	int err;

	if (NULL == akm)
		return -EINVAL;

	akm->compass = class_create(THIS_MODULE, compass_class_name);
	if (IS_ERR(akm->compass)) {
		err = PTR_ERR(akm->compass);
		goto exit_class_create_failed;
	}

	akm->class_dev = device_create(akm->compass, NULL,
				       akm8972_device_dev_t, akm,
				       akm8972_device_name);
	if (IS_ERR(akm->class_dev)) {
		err = PTR_ERR(akm->class_dev);
		goto exit_class_device_create_failed;
	}

	err = sysfs_create_link(&akm->class_dev->kobj, &akm->i2c->dev.kobj,
							device_link_name);
	if (err)
		goto exit_sysfs_create_link_failed;

	err = create_device_attributes(akm->class_dev, akm8972_attributes);
	if (err)
		goto exit_device_attributes_create_failed;

	err = create_device_binary_attributes(&akm->class_dev->kobj,
					      akm8972_bin_attributes);
	if (err)
		goto exit_device_binary_attributes_create_failed;

	return err;

exit_device_binary_attributes_create_failed:
	remove_device_attributes(akm->class_dev, akm8972_attributes);
exit_device_attributes_create_failed:
	sysfs_remove_link(&akm->class_dev->kobj, device_link_name);
exit_sysfs_create_link_failed:
	device_destroy(akm->compass, akm8972_device_dev_t);
exit_class_device_create_failed:
	akm->class_dev = NULL;
	class_destroy(akm->compass);
exit_class_create_failed:
	akm->compass = NULL;
	return err;
}

static void remove_sysfs_interfaces(struct akm8972_data *akm)
{
	if (NULL == akm)
		return;

	if (NULL != akm->class_dev) {
		remove_device_binary_attributes(&akm->class_dev->kobj,
						akm8972_bin_attributes);
		remove_device_attributes(akm->class_dev, akm8972_attributes);
		sysfs_remove_link(&akm->class_dev->kobj, device_link_name);
		akm->class_dev = NULL;
	}
	if (NULL != akm->compass) {
		device_destroy(akm->compass, akm8972_device_dev_t);
		class_destroy(akm->compass);
		akm->compass = NULL;
	}
}

static int akm8972_open(struct input_dev *dev)
{
	struct akm8972_data *akm = input_get_drvdata(dev);
	int ret;
	int interval;

	ret = akm8972_device_power_on(akm);
	if (ret) {
		dev_err(&akm->i2c->dev, "%s: power on failed.\n", __func__);
		goto exit;
	}

	interval = atomic_read(&akm->interval);
	if (0 <= interval)
		schedule_delayed_work(
			&akm->work,
			msecs_to_jiffies(interval));

exit:
	return ret;
}

static void akm8972_close(struct input_dev *dev)
{
	struct akm8972_data *akm = input_get_drvdata(dev);

	cancel_delayed_work_sync(&akm->work);
	akm8972_device_power_off(akm);
}

int akm8972_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct akm8972_data *akm;
	int err = 0;
	struct akm8972_platform_data *pdata = client->dev.platform_data;

	dev_vdbg(&client->dev, "%s called\n", __func__);

	if (!pdata || !pdata->power_mode || !pdata->hw_config ||
			!pdata->setup || !pdata->shutdown) {
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

	akm = kzalloc(sizeof(struct akm8972_data), GFP_KERNEL);
	if (!akm) {
		dev_err(&client->dev,
			"%s: memory allocation failed.", __func__);
		err = -ENOMEM;
		goto err_kzalloc;
	}
	akm->i2c = client;
	akm->pdata = pdata;

	atomic_set(&akm->interval, -1);

	pdata->power_mode(1);
	err = akm8972_device_power_on(akm);
	if (err) {
		dev_err(&client->dev,
			"%s: device power on failed.\n", __func__);
		goto err_power_on;
	}

	err = akecs_checkdevice(akm);
	if (err < 0) {
		akm8972_device_power_off(akm);
		goto err_check_device;
	}

	err = akecs_read_fuse(akm);
	akm8972_device_power_off(akm);
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
	akm->input->open = akm8972_open;
	akm->input->close = akm8972_close;
	akm->input->name = "compass";
	/* HX: 14-bit */
	input_set_abs_params(akm->input, ABS_RX, -8188, 8188, 0, 0);
	/* HY: 14-bit */
	input_set_abs_params(akm->input, ABS_RY, -8188, 8188, 0, 0);
	/* HZ: 14-bit */
	input_set_abs_params(akm->input, ABS_RZ, -8188, 8188, 0, 0);
	/* ST2 */
	input_set_abs_params(akm->input, ABS_RUDDER, 0, 12, 0, 0);

	err = input_register_device(akm->input);
	if (err) {
		input_free_device(akm->input);
		dev_err(&client->dev,
			"%s: Unable to register input device.", __func__);
		goto err_input_register_device;
	}

	INIT_DELAYED_WORK(&akm->work, akm8972_continuous_measure);

	err = request_threaded_irq(client->irq, NULL, akm8972_irq,
				   IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
				   dev_name(&client->dev), akm);
	if (err) {
		input_free_device(akm->input);
		dev_err(&client->dev, "%s: request irq failed.", __func__);
		goto err_request_th_irq;
	}

	err = create_sysfs_interfaces(akm);
	if (err) {
		input_free_device(akm->input);
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
	kfree(akm);
err_kzalloc:
err_i2c_check:
err_pdata:
	return err;
}

static int akm8972_remove(struct i2c_client *client)
{
	struct akm8972_data *akm = i2c_get_clientdata(client);
	dev_vdbg(&client->dev, "%s called\n", __func__);
	remove_sysfs_interfaces(akm);
	input_unregister_device(akm->input);
	free_irq(client->irq, akm);
	akm8972_device_power_off(akm);
	akm->pdata->power_mode(0);
	kfree(akm);
	dev_dbg(&client->dev, "successfully removed.");
	return 0;
}

static const struct i2c_device_id akm8972_id[] = {
	{AKM8972_I2C_NAME, 0 },
	{ }
};

static const struct dev_pm_ops akm8972_pm_ops = {
	.suspend	= akm8972_suspend,
	.resume		= akm8972_resume,
};

static struct i2c_driver akm8972_driver = {
	.probe		= akm8972_probe,
	.remove		= akm8972_remove,
	.id_table	= akm8972_id,
	.driver = {
		.name = AKM8972_I2C_NAME,
		.pm = &akm8972_pm_ops,
	},
};

static int __init akm8972_init(void)
{
	return i2c_add_driver(&akm8972_driver);
}

static void __exit akm8972_exit(void)
{
	i2c_del_driver(&akm8972_driver);
}

module_init(akm8972_init);
module_exit(akm8972_exit);

MODULE_AUTHOR("SEMC");
MODULE_DESCRIPTION("AKM8972 compass driver");
MODULE_LICENSE("GPL");

