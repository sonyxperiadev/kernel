/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/input/misc/bma222.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

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
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/bma222.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/brvsens_driver.h>
#include <linux/module.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define BMA222_SW_CALIBRATION 1
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

/* uncomment to enable interrupt based event generation */
/* #define BMA222_ACCL_IRQ_MODE */
#define ACCL_NAME			"bma222_accl"
#define ACCL_VENDORID			0x0001

#define LOW_G_THRES			5
#define LOW_G_DUR			50
#define HIGH_G_THRES			10
#define HIGH_G_DUR			1
#define ANY_MOTION_THRES		1
#define ANY_MOTION_CT			1
#define BMA222_INPUT_DEVICE		1

#ifdef BMA222_CALIBRATION
#define BMA222_CHIP_ID_REG                      0x00
#define BMA222_X_AXIS_REG                       0x03
#define BMA222_Y_AXIS_REG                       0x05
#define BMA222_Z_AXIS_REG                       0x07
#define BMA222_MODE_CTRL_REG                    0x11
#define BMA2XX_EEPROM_CTRL_REG                  0x33
#define BMA2XX_OFFSET_CTRL_REG                  0x36
#define BMA2XX_OFFSET_PARAMS_REG                0x37
#endif

#ifdef BMA222_SW_CALIBRATION
static int bma222_offset[3];
#endif
static const struct of_device_id bma222_accl_of_match[] = {
	{.compatible = "bcm,bma222_accl",},
	{},
}

MODULE_DEVICE_TABLE(of, bma222_accl_of_match);

struct drv_data {
#ifdef BMA222_INPUT_DEVICE
	struct input_dev *ip_dev;
#endif
	struct i2c_client *i2c;
	int irq;
	int bma222_accl_mode;
	char bits_per_transfer;
	struct delayed_work work_data;
	bool config;
	struct list_head next_dd;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend suspend_desc;
#endif
};

struct bma_acc_t {
	short x;
	short y;
	short z;
};

static struct mutex bma222_accl_dd_lock;
static struct mutex bma222_accl_wrk_lock;
static struct list_head dd_list;
#ifndef BMA222_ACCL_IRQ_MODE
static struct timer_list bma_wakeup_timer;
#endif
static atomic_t bma_on;
static atomic_t a_flag;
static struct bma_acc_t newacc;

static struct i2c_client *bma222_accl_client;
#ifdef BMA222_CALIBRATION
static bool fastcali_ret;
#endif
enum bma222_accl_modes {
	SENSOR_DELAY_FASTEST = 0,
	SENSOR_DELAY_GAME = 20,
	SENSOR_DELAY_UI = 60,
	SENSOR_DELAY_NORMAL = 200
};

#ifdef BMA222_CALIBRATION
static int bma222_fast_compensate(const unsigned char target[3]);
static int bma222_inline_calibrate(void);
static int bma222_offset_fast_cali(unsigned char target_x,
				   unsigned char target_y,
				   unsigned char target_z);
#endif

static inline void bma222_accl_i2c_delay(unsigned int msec)
{
	mdelay(msec);
}

/*      i2c write routine for bma222    */
static inline char bma222_accl_i2c_write(unsigned char reg_addr,
					 unsigned char *data, unsigned char len)
{
	s32 dummy;
	if (bma222_accl_client == NULL)
		return -1;

	while (len--) {
		dummy =
		    i2c_smbus_write_byte_data(bma222_accl_client, reg_addr,
					      *data);
		reg_addr++;
		data++;
		if (dummy < 0)
			return -1;
	}
	return 0;
}

/*      i2c read routine for bma222     */
static inline char bma222_accl_i2c_read(unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	if (bma222_accl_client == NULL)
		return -1;

	while (len--) {
		dummy = i2c_smbus_read_byte_data(bma222_accl_client, reg_addr);
		if (dummy < 0)
			return -1;
		*data = dummy & 0x000000ff;
		reg_addr++;
		data++;
	}
	return 0;
}

static int bma222_accl_power_down(struct drv_data *dd)
{
	u8 data;
	int rc;

	data = 0x80;
	rc = i2c_smbus_write_byte_data(dd->i2c, 0x11, data);
	if (rc < 0)
		pr_err("G-Sensor power down failed\n");
	else
		atomic_set(&bma_on, 0);

	return rc;
}

static int bma222_accl_power_up(struct drv_data *dd)
{
	u8 data;
	int rc;

	data = 0x00;
	rc = i2c_smbus_write_byte_data(dd->i2c, 0x11, data);
	if (rc < 0)
		pr_err("G-Sensor power up failed\n");
	else
		atomic_set(&bma_on, 1);

	return rc;
}

#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static int bma222_accl_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct drv_data *dd;
	dd = i2c_get_clientdata(client);

#ifdef BMA222_ACCL_IRQ_MODE
	disable_irq(dd->irq);
#else
	del_timer_sync(&bma_wakeup_timer);
	cancel_delayed_work_sync(&dd->work_data);
#endif
	if (atomic_read(&bma_on))
		bma222_accl_power_down(dd);

	return 0;
}

static int bma222_accl_resume(struct i2c_client *client)
{
	struct drv_data *dd;
	dd = i2c_get_clientdata(client);

#ifdef BMA222_ACCL_IRQ_MODE
	enable_irq(dd->irq);
	if (atomic_read(&a_flag))
		bma222_accl_power_up(dd);
#else
	if (atomic_read(&a_flag))
		mod_timer(&bma_wakeup_timer, jiffies + HZ / 1000);
#endif

	return 0;
}

#else
#define bma222_accl_suspend NULL
#define bma222_accl_resume NULL
#endif /* CONFIG_PM */

#ifdef CONFIG_HAS_EARLYSUSPEND
static void bma222_accl_early_suspend(struct early_suspend *desc)
{
	struct drv_data *dd = container_of(desc, struct drv_data, suspend_desc);
	pm_message_t mesg = {
		.event = PM_EVENT_SUSPEND,
	};
	bma222_accl_suspend(dd->i2c, mesg);
}

static void bma222_accl_late_resume(struct early_suspend *desc)
{
	struct drv_data *dd = container_of(desc, struct drv_data, suspend_desc);
	bma222_accl_resume(dd->i2c);
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

#ifdef BMA222_ACCL_IRQ_MODE
static irqreturn_t bma222_accl_irq(int irq, void *dev_id)
{
	struct device *dev = dev_id;
	struct drv_data *dd;
	long delay;

	dd = dev_get_drvdata(dev);
	delay = dd->bma222_accl_mode * HZ / 1000;
	if (!delay)		/* check for FAST MODE */
		delay = 1;
	schedule_delayed_work(&dd->work_data, delay);

	return IRQ_HANDLED;
}
#else
void bma_wakeup_timer_func(unsigned long data)
{
	struct drv_data *dd;
	long delay = 0;
	dd = (struct drv_data *)data;
	delay = dd->bma222_accl_mode * HZ / 1000;
	if (delay < 2)
		delay = 2;
	schedule_delayed_work(&dd->work_data, HZ / 1000);
	if (atomic_read(&a_flag))
		mod_timer(&bma_wakeup_timer, jiffies + delay);
}
#endif

static int bma222_accl_open(struct input_dev *dev)
{
	int rc = 0;
#ifdef BMA222_ACCL_IRQ_MODE
	struct drv_data *dd = input_get_drvdata(dev);

	if (!dd->irq)
		return -1;
	/* Timer based implementation */
	/* does not require irq t be enabled */
	rc = request_irq(dd->irq,
			 &bma222_accl_irq, 0, ACCL_NAME, &dd->i2c->dev);
#endif

	return rc;
}

static void bma222_accl_release(struct input_dev *dev)
{
#ifdef BMA222_ACCL_IRQ_MODE
	struct drv_data *dd = input_get_drvdata(dev);

	/* Timer based implementation */
	/* does not require irq t be enabled */
	free_irq(dd->irq, &dd->i2c->dev);
#endif

	return;
}

static int bma222_smbus_read_byte_block(struct i2c_client *client,
					unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int bma222_accl_configuration(void)
{
	u8 data;
	int rc;

	/* set register 0x10 '01011b' bandwidth 62.5HZ */
	data = 0x0b;
	rc = i2c_smbus_write_byte_data(bma222_accl_client, 0x10, data);
	if (rc < 0)
		pr_err("G-Sensor set band width fail\n");

	data = 0x03;
	rc = i2c_smbus_write_byte_data(bma222_accl_client, 0x0f, data);
	if (rc < 0)
		pr_err("G-Sensor set band width fail\n");

	return rc;

}

static int bma222_read_accel_xyz(struct bma_acc_t *acc)
{
	int comres;
	unsigned char data[6];

	comres =
	    bma222_smbus_read_byte_block(bma222_accl_client, BMA222_X_AXIS_REG,
					 data, 6);
	acc->x = (signed char)data[0];
	acc->y = (signed char)data[2];
	acc->z = (signed char)data[4];
	return comres;
}

static void bma222_accl_getdata(struct drv_data *dd)
{
	struct bma_acc_t acc;
	int X = 0;
	int Y = 0;
	int Z = 0;
	struct bma222_accl_platform_data *pdata = pdata =
	    bma222_accl_client->dev.platform_data;
#ifndef BMA222_ACCL_IRQ_MODE
	if (!atomic_read(&bma_on)) {
		bma222_accl_power_up(dd);
		/* BMA222 need 2 to 3 ms delay */
		/* to give valid data after wakeup */
		msleep(2);
	}
#endif
	mutex_lock(&bma222_accl_wrk_lock);
	bma222_read_accel_xyz(&acc);
	mutex_unlock(&bma222_accl_wrk_lock);

	switch (pdata->orientation) {
	case BMA_ORI_NOSWITCH_NOINVERSE:
		X = acc.x;
		Y = acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_XYSWITCH_NOINVERSE:
		X = acc.y;
		Y = acc.x;
		Z = acc.z;
		break;
	case BMA_ORI_NOSWITCH_XINVERSE:
		X = -acc.x;
		Y = acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_NOSWITCH_XYINVERSE:
		X = -acc.x;
		Y = -acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_NOSWITCH_YINVERSE:
		X = acc.x;
		Y = -acc.y;
		Z = acc.z;
		break;
	case BMA_ORI_NOSWITCH_ZINVERSE:
		X = acc.x;
		Y = acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_XYSWITCH_ZINVERSE:
		X = acc.y;
		Y = acc.x;
		Z = -acc.z;
		break;
	case BMA_ORI_NOSWITCH_XZINVERSE:
		X = -acc.x;
		Y = acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_NOSWITCH_XYZINVERSE:
		X = -acc.x;
		Y = -acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_NOSWITCH_YZINVERSE:
		X = acc.x;
		Y = -acc.y;
		Z = -acc.z;
		break;
	case BMA_ORI_XYSWITCH_YZINVERSE:
		X = acc.y;
		Y = -acc.x;
		Z = -acc.z;
		break;
	case BMA_ORI_XYSWITCH_XZINVERSE:
		X = -acc.y;
		Y = acc.x;
		Z = -acc.z;
		break;
	case BMA_ORI_XYSWITCH_XINVERSE:
		X = -acc.y;
		Y = acc.x;
		Z = acc.z;
		break;
	case BMA_ORI_XYSWITCH_XYZINVERSE:
		X = -acc.y;
		Y = -acc.x;
		Z = -acc.z;
	default:
		X = acc.x;
		Y = acc.y;
		Z = acc.z;
		break;
	}

#ifdef BMA222_INPUT_DEVICE
#ifdef BMA222_SW_CALIBRATION
	input_report_rel(dd->ip_dev, REL_X, X-bma222_offset[0]);
	input_report_rel(dd->ip_dev, REL_Y, Y-bma222_offset[1]);
	input_report_rel(dd->ip_dev, REL_Z, Z-bma222_offset[2]);
#else
	input_report_rel(dd->ip_dev, REL_X, X);
	input_report_rel(dd->ip_dev, REL_Y, Y);
	input_report_rel(dd->ip_dev, REL_Z, Z);
#endif
#endif
	input_sync(dd->ip_dev);
	newacc.x = X;
	newacc.y = Y;
	newacc.z = Z;

	return;

}

static void bma222_accl_work_f(struct work_struct *work)
{
	struct delayed_work *dwork =
	    container_of(work, struct delayed_work, work);
	struct drv_data *dd = container_of(dwork, struct drv_data, work_data);

	bma222_accl_getdata(dd);

}

static int bma222_accl_misc_open(struct inode *inode, struct file *file)
{
	int err;
	err = nonseekable_open(inode, file);
	if (err < 0)
		return err;

	file->private_data = inode->i_private;

	return 0;
}

static long bma222_accl_misc_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int delay;
	struct drv_data *dd;
#ifdef BMA222_CALIBRATION
	void __user *data;
	struct bma_acc_t sensor_data;
#endif
	dd = i2c_get_clientdata(bma222_accl_client);

	switch (cmd) {
	case BMA222_ACCL_IOCTL_GET_DELAY:
		delay = dd->bma222_accl_mode;

		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;

	case BMA222_ACCL_IOCTL_SET_DELAY:
		if (copy_from_user(&delay, argp, sizeof(delay)))
			return -EFAULT;
		if (delay < 0 || delay > 200)
			return -EINVAL;
		dd->bma222_accl_mode = delay;
		break;
	case BMA222_ACCL_IOCTL_SET_FLAG:
		if (copy_from_user(&delay, argp, sizeof(delay)))
			return -EFAULT;
		if (delay == 1)
			mod_timer(&bma_wakeup_timer, jiffies + HZ / 1000);
		else if (delay == 0)
			del_timer(&bma_wakeup_timer);
		else
			return -EINVAL;
		atomic_set(&a_flag, delay);
		break;
	case BMA222_ACCL_IOCTL_GET_DATA:
		if (!atomic_read(&a_flag))
			bma222_accl_getdata(dd);
		if (copy_to_user(argp, &newacc, sizeof(newacc)))
			return -EFAULT;
		break;
	}

	return 0;
}

int bma222_read_data(struct drv_data *data, u16 * value)
{
	struct drv_data *dd;
	dd = i2c_get_clientdata(bma222_accl_client);

	if (!atomic_read(&a_flag))
		bma222_accl_getdata(dd);

	*(value + 0) = newacc.x;
	*(value + 1) = newacc.y;
	*(value + 2) = newacc.z;
	return 0;
}

static const struct file_operations bma222_accl_misc_fops = {
	.owner = THIS_MODULE,
	.open = bma222_accl_misc_open,
	.unlocked_ioctl = bma222_accl_misc_ioctl,
};

static struct miscdevice bma222_accl_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bma222_accl",
	.fops = &bma222_accl_misc_fops,
	.mode = 0644,
};

#ifdef BMA222_CALIBRATION
/*calibrate start...................*/
static ssize_t store_fastcali_value(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	unsigned char target[3];
	fastcali_ret = false;
#if 0
	if (sscanf(buf, "%d %d %d", &(target[0]), &(target[1]), &(target[2])) ==
	    3)
		bma222_offset_fast_cali(target[0], target[1], target[2]);
	else
		printk(KERN_ERR "BMA222:invalid format\n");
#endif
	bma222_offset_fast_cali(0, 0, 1);
}

static ssize_t show_fastcali_value(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", fastcali_ret);
}

/*
  offset fast calibration
  target:
  0 => 0g; 1 => +1g; 2 => -1g; 3 => 0g
*/
static int bma222_offset_fast_cali(unsigned char target_x,
				   unsigned char target_y,
				   unsigned char target_z)
{
	unsigned char target[3];
	fastcali_ret = false;
	/* check arguments */
	if ((target_x > 3) || (target_y > 3) || (target_z > 3)) {
		pr_err("bma222_offset_fast_cali para err\n");
		return -1;
	}

	/* trigger offset calibration secondly */
	target[0] = target_x;
	target[1] = target_y;
	target[2] = target_z;
	if (bma222_fast_compensate(target) != 0)
		return -2;

	/* save calibration result to EEPROM finally */
	if (bma222_inline_calibrate() != 0)
		return -3;
	fastcali_ret = true;
	return 0;
}

#define BMA222_REG_POWER_CTL 0x11

static int bma222_set_power_mode(void)
{
	u8 data;
	int rc;
	struct drv_data *dd;
	dd = i2c_get_clientdata(bma222_accl_client);

	data = 0x00;
	rc = i2c_smbus_write_byte_data(dd->i2c, BMA222_REG_POWER_CTL, data);
	if (rc < 0)
		pr_err("bma222 set power mode failed\n");
	else
		atomic_set(&bma_on, 1);

	return rc;
}

static int bma222_set_range(void)
{
	u8 data;
	int rc;

	/* set register 0x10 '01011b' bandwidth 62.5HZ */
	data = 0x03;
	rc = i2c_smbus_write_byte_data(bma222_accl_client, 0xf, data);
	if (rc < 0)
		pr_err("bma222 set range fail\n");

	return rc;

}

static u8 bma222_set_bandwidth(void)
{
	u8 data = 0x0b;
	int rc;

	rc = i2c_smbus_write_byte_data(bma222_accl_client, 0x10, data);
	if (rc < 0)
		pr_err("bma222 set band width fail\n");

	return rc;

}

/*
  offset fast compensation
  target:
  0 => 0g; 1 => +1g; 2 => -1g; 3 => 0g
*/

static int bma222_fast_compensate(const unsigned char target[3])
{
	static const int CALI_TIMEOUT = 100;
	int res = 0, timeout = 0;
	unsigned char databuf;
	fastcali_ret = false;
	/*** set normal mode, make sure that lowpower_en bit is '0' ***/
	res = bma222_set_power_mode();
	if (res != 0) {
		pr_err("bma222: set pw mode fail in calibrate\n");
		return res;
	}

	/*** set +/-2g range ***/
	res = bma222_set_range();
	if (res != 0) {
		pr_err("bma222: set range fail in calibrate\n");
		return res;
	}

	/*** set 1000 Hz bandwidth ***/
	res = bma222_set_bandwidth();
	if (res != 0) {
		pr_err("bma222: set pw mode fail in calibrate\n");
		return res;
	}

	/*** set offset target (x/y/z) ***/
	res = bma222_accl_i2c_read(BMA2XX_OFFSET_PARAMS_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: read params fail in calibrate\n");
		return res;
	}
	/* combine three target value */
	databuf &= 0x81;	/*clean old value */
	databuf |=
	    (((target[0] & 0x03) << 1) | ((target[1] & 0x03) << 3) |
	     ((target[2] & 0x03) << 5));
	res = bma222_accl_i2c_write(BMA2XX_OFFSET_PARAMS_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: write params fail in calibrate\n");
		return res;
	}

	/*** trigger x-axis offset compensation ***/
	res = bma222_accl_i2c_read(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: read x-offset fail in calibrate\n");
		return res;
	}

	databuf &= 0x9F;	/*clean old value */
	databuf |= 0x01 << 5;
	res = bma222_accl_i2c_write(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: write x-offset fail in calibrate\n");
		return res;
	}

	/*** checking status and waiting x-axis offset compensation done ***/
	timeout = 0;
	do {
		mdelay(2);
		bma222_accl_i2c_read(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
		databuf = (databuf >> 4) & 0x01;	/*get cal_rdy bit */
		if (++timeout == CALI_TIMEOUT) {
			pr_err("bma222:check x cal_rdy time out\n");
			return -ETIMEDOUT;
		}
	} while (databuf == 0);

	/*** trigger y-axis offset compensation ***/
	res = bma222_accl_i2c_read(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: read y-offset fail in calibrate\n");
		return res;
	}

	databuf &= 0x9F;	/*clean old value */
	databuf |= 0x02 << 5;
	res = bma222_accl_i2c_write(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: write y-offset fail in calibrate\n");
		return res;
	}

	/*** checking status and waiting y-axis offset compensation done ***/
	timeout = 0;
	do {
		mdelay(2);
		bma222_accl_i2c_read(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
		databuf = (databuf >> 4) & 0x01;	/*get cal_rdy bit */
		if (++timeout == CALI_TIMEOUT) {
			pr_err("bma222:check y cal_rdy time out\n");
			return -ETIMEDOUT;
		}
	} while (databuf == 0);

	/*** trigger z-axis offset compensation ***/
	res = bma222_accl_i2c_read(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: read z-offset fail in calibrate\n");
		return res;
	}

	databuf &= 0x9F;	/*clean old value */
	databuf |= 0x03 << 5;
	res = bma222_accl_i2c_write(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: write z-offset fail in calibrate\n");
		return res;
	}

	/*** checking status and waiting z-axis offset compensation done ***/
	timeout = 0;
	do {
		mdelay(2);
		bma222_accl_i2c_read(BMA2XX_OFFSET_CTRL_REG, &databuf, 1);
		databuf = (databuf >> 4) & 0x01;	/*get cal_rdy bit */
		if (++timeout == CALI_TIMEOUT) {
			pr_err("bma222:check zcal_rdy time out\n");
			return -ETIMEDOUT;
		}
	} while (databuf == 0);

	return 0;
}

  /*---------------------------------------------*/
static int bma222_inline_calibrate(void)
{
	static const int PROG_TIMEOUT = 50;
	int res = 0, timeout = 0;
	unsigned char databuf;

	/*** unlock EEPROM: write '1' to bit (0x33) nvm_prog_mode ***/
	res = bma222_accl_i2c_read(BMA2XX_EEPROM_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: unlock eeprom read fail in calibrate\n");
		return res;
	}

	databuf |= 0x01;
	res = bma222_accl_i2c_write(BMA2XX_EEPROM_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: unlock eeprom write fail in calibrate\n");
		return res;
	}

	/*** need to delay ??? ***/

	/*** trigger the write process: write '1' to bit (0x33) nvm_prog_trigge
	and keep '1' in bit (0x33) nvm_prog_mode ***/
	res = bma222_accl_i2c_read(BMA2XX_EEPROM_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: trigger the wr process[r] fail\n");
		goto __lock_eeprom__;
	}
	databuf |= 0x02;
	res = bma222_accl_i2c_write(BMA2XX_EEPROM_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: trigger the wr process[w] fail\n");
		goto __lock_eeprom__;
	}
	/*check the write status by reading bit (0x33) nvm_rdy
	   while nvm_rdy = '0', the write process is still enduring;
	   if nvm_rdy = '1', then writing is completed */
	do {
		mdelay(2);
		bma222_accl_i2c_read(BMA2XX_EEPROM_CTRL_REG, &databuf, 1);
		databuf = (databuf >> 2) & 0x01;	/*get nvm_rdy bit */
		if (++timeout == PROG_TIMEOUT) {
			res = -ETIMEDOUT;
			pr_err("bma222: check nvm_rdy timeout\n");
			goto __lock_eeprom__;
		}
	} while (databuf == 0);

	/*** lock EEPROM: write '0' to nvm_prog_mode ***/
__lock_eeprom__:
	res = bma222_accl_i2c_read(BMA2XX_EEPROM_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: read nvm_prog_mode fail in calibrate\n");
		return res;
	}
	databuf &= 0xFE;
	res = bma222_accl_i2c_write(BMA2XX_EEPROM_CTRL_REG, &databuf, 1);
	if (res != 0) {
		pr_err("bma222: write nvm_prog_mode fail in calibrate\n");
		return res;

	}
	return res;
}

static DRIVER_ATTR(fastcali, S_IWUSR | S_IRUGO, show_fastcali_value,
		   store_fastcali_value);

/*----------------------------------------------------------------------------*/
static struct attribute *bma222_attr_list[] = {
	&driver_attr_fastcali,
	NULL,
};

static struct attribute_group bma222_attr_grp = {
	.attrs = bma222_attr_list,
};

/*----------------------------------------------------------------------------*/
/*calibrate end*/
#endif

#ifdef BMA222_SW_CALIBRATION

static ssize_t bma222_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct drv_data *dd = i2c_get_clientdata(client);
	return sprintf(buf, "%d\n", dd->bma222_accl_mode);
}

static ssize_t bma222_delay_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct drv_data *dd = i2c_get_clientdata(client);
	error = kstrtoul(buf, 10, &data);
	if (error)
		return error;
	if (data > SENSOR_DELAY_NORMAL)
		data = SENSOR_DELAY_NORMAL;
	dd->bma222_accl_mode = data;
	return count;
}

static ssize_t bma222_get_offset(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d, %d, %d\n", bma222_offset[0],
					bma222_offset[1], bma222_offset[2]);
}

static ssize_t bma222_set_offset(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	int x, y, z;
	int err = -EINVAL;
	struct i2c_client *client = to_i2c_client(dev);
	err = sscanf(buf, "%d %d %d", &x, &y, &z);
	if (err != 3) {
		pr_err("invalid parameter number: %d\n", err);
		return err;
	}
	mutex_lock(&bma222_accl_wrk_lock);
	bma222_offset[0] = x;
	bma222_offset[1] = y;
	bma222_offset[2] = z;
	mutex_unlock(&bma222_accl_wrk_lock);
	return count;
}
static DEVICE_ATTR(offset, 00664, bma222_get_offset, bma222_set_offset);
static DEVICE_ATTR(delay, S_IRUGO | S_IWUSR | S_IWGRP ,
			bma222_delay_show, bma222_delay_store);
static struct attribute *bma222_attributes[] = {
	&dev_attr_offset.attr,
	&dev_attr_delay.attr,
	NULL
};

static struct attribute_group bma222_attr_swcal_grp = {
	.attrs = bma222_attributes,
};

#endif

static int bma222_accl_probe(struct i2c_client *client,
				       const struct i2c_device_id *id)
{
	struct drv_data *dd;
	int rc = 0;
	unsigned char tempvalue = 0;
	struct device_node *np;
	struct bma222_accl_platform_data *pdata;
	u32 val = 0;
#ifdef BMA222_CALIBRATION
	fastcali_ret = false;
#endif
#ifdef BMA222_SW_CALIBRATION
	bma222_offset[0] = 0;
	bma222_offset[1] = 0;
	bma222_offset[2] = 0;
#endif

	printk(KERN_INFO "+ %s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "i2c_check_functionality error\n");
		goto probe_exit;
	}
	dd = kzalloc(sizeof(struct drv_data), GFP_KERNEL);
	if (!dd) {
		rc = -ENOMEM;
		goto probe_exit;
	}
	bma222_accl_client = client;

	mutex_lock(&bma222_accl_dd_lock);
	list_add_tail(&dd->next_dd, &dd_list);
	mutex_unlock(&bma222_accl_dd_lock);
	INIT_DELAYED_WORK(&dd->work_data, bma222_accl_work_f);
	dd->i2c = client;

	if (client->dev.platform_data)
		pdata = client->dev.platform_data;

	else if (client->dev.of_node) {

		pdata = kzalloc(sizeof(struct bma222_accl_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			rc = -ENOMEM;
			goto err_kzalloc_pdata;
		}

		bma222_accl_client->dev.platform_data = pdata;
		np = client->dev.of_node;
		rc = of_property_read_u32(np, "gpio-irq-pin", &val);
		if (rc) {
			printk(KERN_ERR
			       "BMA222: gpio-irq-pin doesnot exisit\n");
			goto err_read;
		}
		client->irq = val;
		rc = of_property_read_u32(np, "orientation", &val);
		if (rc) {
			printk(KERN_ERR "BMA222: orientation doesnot exisit\n");
			goto err_read;
		}
		pdata->orientation = val;
	}

	if (tempvalue == BMA222_CHIP_ID)
		printk(KERN_INFO "Bosch Sensortec Device detected!\n"
		       "BMA222 registered I2C driver!\n");

#ifdef BMA222_INPUT_DEVICE
	dd->ip_dev = input_allocate_device();
	if (!dd->ip_dev) {
		rc = -ENOMEM;
		goto probe_err_reg;
	}
	input_set_drvdata(dd->ip_dev, dd);
	dd->irq = client->irq;
	dd->ip_dev->open = bma222_accl_open;
	dd->ip_dev->close = bma222_accl_release;
	dd->ip_dev->name = ACCL_NAME;
	dd->ip_dev->phys = ACCL_NAME;
	dd->ip_dev->id.vendor = ACCL_VENDORID;
	dd->ip_dev->id.product = 1;
	dd->ip_dev->id.version = 1;
	dd->ip_dev->dev.parent = &dd->i2c->dev;
	set_bit(EV_REL, dd->ip_dev->evbit);
	/* 32768 == 1g, range -4g ~ +4g */
	/* acceleration x-axis */
	input_set_capability(dd->ip_dev, EV_REL, REL_X);
	input_set_abs_params(dd->ip_dev, REL_X, -256, 256, 0, 0);
	/* acceleration y-axis */
	input_set_capability(dd->ip_dev, EV_REL, REL_Y);
	input_set_abs_params(dd->ip_dev, REL_Y, -256, 256, 0, 0);
	/* acceleration z-axis */
	input_set_capability(dd->ip_dev, EV_REL, REL_Z);
	input_set_abs_params(dd->ip_dev, REL_Z, -256, 256, 0, 0);

	rc = input_register_device(dd->ip_dev);
	if (rc) {
		printk(KERN_ERR
		       "bma222_accl_probe: input_register_device rc=%d\n", rc);
		goto probe_err_reg_dev;
	}
#endif
	rc = misc_register(&bma222_accl_misc_device);
	if (rc < 0) {
		printk(KERN_ERR "bma222 misc_device register failed\n");
		goto probe_err_reg_misc;
	}

	dd->bma222_accl_mode = 200;	/* NORMAL Mode */
	i2c_set_clientdata(client, dd);

	rc = bma222_accl_configuration();
	if (rc < 0) {
		printk(KERN_ERR
		       "bma222_accl_probe: Error configuring device rc=%d\n",
		       rc);
		goto probe_err_setmode;
	}

	setup_timer(&bma_wakeup_timer, bma_wakeup_timer_func, (long)dd);
	mod_timer(&bma_wakeup_timer, jiffies + HZ / 1000);

#ifdef CONFIG_BRCM_VIRTUAL_SENSOR
	rc = brvsens_register(SENSOR_HANDLE_ACCELEROMETER,
			      "BMA222",
			      (void *)NULL,
			      (PFNACTIVATE) NULL, (PFNREAD) bma222_read_data);
	printk(KERN_INFO "BMA222: brvsens_register rc=%d\n", rc);
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	dd->suspend_desc.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	dd->suspend_desc.suspend = bma222_accl_early_suspend;
	dd->suspend_desc.resume = bma222_accl_late_resume;
	register_early_suspend(&dd->suspend_desc);
#endif
#ifdef BMA222_CALIBRATION
	rc = sysfs_create_group(&client->dev.kobj, &bma222_attr_grp);
	if (rc != 0)
		goto err_create_grp0;
#endif
#ifdef BMA222_SW_CALIBRATION
	rc = sysfs_create_group(&client->dev.kobj, &bma222_attr_swcal_grp);
	if (rc != 0)
		goto err_create_grp1;
#endif
	printk(KERN_INFO "- %s\n", __func__);

	return rc;

#ifdef BMA222_SW_CALIBRATION
	sysfs_remove_group(&client->dev.kobj, &bma222_attr_swcal_grp);
err_create_grp1:
#endif
#ifdef BMA222_CALIBRATION
	sysfs_remove_group(&client->dev.kobj, &bma222_attr_grp);
err_create_grp0:
#endif
	del_timer_sync(&bma_wakeup_timer);
probe_err_setmode:
	misc_deregister(&bma222_accl_misc_device);
probe_err_reg_misc:
#ifdef BMA222_INPUT_DEVICE
	input_unregister_device(dd->ip_dev);
	dd->ip_dev = NULL;
probe_err_reg_dev:
	if (dd->ip_dev)
		input_free_device(dd->ip_dev);

probe_err_reg:
#endif
err_read:
	if (client->dev.of_node)
		kfree(pdata);
err_kzalloc_pdata:
	mutex_lock(&bma222_accl_dd_lock);
	list_del(&dd->next_dd);
	mutex_unlock(&bma222_accl_dd_lock);
	kfree(dd);
probe_exit:
	return rc;
}

static int bma222_accl_remove(struct i2c_client *client)
{
	struct drv_data *dd;
	int rc;

	dd = i2c_get_clientdata(client);

	rc = bma222_accl_power_down(dd);
	if (rc)
		printk(KERN_ERR "%s: power down failed with error %d\n",
		       __func__, rc);
#ifdef BMA222_ACCL_IRQ_MODE
	free_irq(dd->irq, &dd->i2c->dev);
#else
	del_timer(&bma_wakeup_timer);
#endif
#ifdef BMA222_CALIBRATION
	sysfs_remove_group(&bma222_accl_client->dev.kobj, &bma222_attr_grp);
#endif
#ifdef BMA222_SW_CALIBRATION
	sysfs_remove_group(&bma222_accl_client->dev.kobj,
						&bma222_attr_swcal_grp);
#endif

	misc_deregister(&bma222_accl_misc_device);

#ifdef BMA222_INPUT_DEVICE
	input_unregister_device(dd->ip_dev);
#endif

	i2c_set_clientdata(client, NULL);
	mutex_lock(&bma222_accl_dd_lock);
	list_del(&dd->next_dd);
	mutex_unlock(&bma222_accl_dd_lock);
	kfree(dd);

	return 0;
}

static struct i2c_device_id bma222_accl_idtable[] = {
	{"bma222_accl", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bma222_accl_idtable);

static struct i2c_driver bma222_accl_driver = {
	.driver = {
		   .name = ACCL_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = bma222_accl_of_match,
		   },
	.id_table = bma222_accl_idtable,
	.probe = bma222_accl_probe,
	.remove = bma222_accl_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = bma222_accl_suspend,
	.resume = bma222_accl_resume,
#endif
};

static int __init bma222_accl_init(void)
{
	INIT_LIST_HEAD(&dd_list);
	mutex_init(&bma222_accl_dd_lock);
	mutex_init(&bma222_accl_wrk_lock);

	return i2c_add_driver(&bma222_accl_driver);
}

module_init(bma222_accl_init);

static void __exit bma222_accl_exit(void)
{
	i2c_del_driver(&bma222_accl_driver);
}

module_exit(bma222_accl_exit);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("bma222");

MODULE_AUTHOR("yongqiang wang");
MODULE_DESCRIPTION("BMA222 driver");
MODULE_LICENSE("GPL");
