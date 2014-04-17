/* ami306_km.c - AMI-Sensor driver
 *
 * Copyright (C) 2011 AICHI STEEL CORPORATION
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
/**
 * @file	ami306_km.c
 * @brief	Linux Kernel Module Interface
 */
/*-------+---------+---------+---------+---------+---------+---------+---------+
 *		Includes depend on platform
 *-------+---------+---------+---------+---------+---------+---------+--------*/
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/kfifo.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/hrtimer.h>
#include "linux/ami306_def.h"
#include "linux/ami306_hw.h"
#include "linux/ami306_cmd.h"
#include "linux/ami_sensor.h"
#include "linux/ami_sensor_pif.h"

#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#ifdef CONFIG_ARCH_KONA
#include <linux/regulator/consumer.h>
#endif

static int mod_debug = 0x0;
module_param(mod_debug, int, 0644);


#define AMI306_EVENT_NUM 64
#define AMI_SENSOR_START_TIME_DELAY 50000000

/*-===========================================================================
 *	Device Driver Platform Interface
 *-=========================================================================-*/
/**
 * suspend execution for microsecond intervals
 *
 * @param usec	microsecond
 */
void AMI_udelay(u32 usec)
{
	udelay(usec);
}

/**
 * suspend execution for millisecond intervals
 *
 * @param usec	millisecond
 */
void AMI_mdelay(u32 msec)
{
	msleep(msec);
}

/**
 * get DRDY value
 *
 * @return DRDY value
 */
#ifdef USE_DRDY_PIN
int AMI_DRDY_Value(void)
{
	return gpio_get_value(AMI_GPIO_DRDY);
}
#endif

/**
 * i2c write
 *
 * @param i2c	i2c handle
 * @param adr	register address
 * @param len	data length
 * @param buf	data buffer
 * @return		result
 */
int AMI_i2c_send(void *i2c, u8 adr, u8 len, u8 *buf)
{
	int res = 0;
	res = i2c_smbus_write_i2c_block_data(i2c, adr, len, buf);
	return (res < 0 ? AMI_COMM_ERR : 0);
}

/**
 * i2c read
 *
 * @param i2c	i2c handle
 * @param adr	register address
 * @param len	data length
 * @param buf	data buffer
 * @return		result
 */
int AMI_i2c_recv(void *i2c, u8 adr, u8 len, u8 *buf)
{
	int res = 0;
	res = i2c_smbus_read_i2c_block_data(i2c, adr, len, buf);
	return (res < 0 ? AMI_COMM_ERR : 0);
}

/*-------+---------+---------+---------+---------+---------+---------+---------+
 *	Trace
 *-------+---------+---------+---------+---------+---------+---------+--------*/
/**
 * Trace
 *
 * @param fmt	trace format
 * @param ...	value
 */
void AMI_LOG(const char *fmt, ...)
{
	va_list argp;
	char buf[256];
	va_start(argp, fmt);
	vsprintf(buf, fmt, argp);
	va_end(argp);
	printk(buf);
	printk("\n");
}

/**
 * Trace for debug
 *
 * @param fmt	trace format
 * @param ...	value
 */
void AMI_DLOG(const char *fmt, ...)
{
	if (mod_debug) {
		va_list argp;
		char buf[256];
		va_start(argp, fmt);
		vsprintf(buf, fmt, argp);
		va_end(argp);
		printk(buf);
		printk("\n");
	}
}

struct ami306_dev_data {
	struct miscdevice dev;
	struct i2c_client *client;
	void *handle;
	struct semaphore mutex;
	int ami_dir;
	int ami_polarity;
	int gpio_intr;
	int gpio_drdy;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
	DECLARE_KFIFO (ebuff, AMI306_EVENT_NUM * sizeof(struct ami_sensor_value) );
#else
	DECLARE_KFIFO (ebuff, struct ami_sensor_value, AMI306_EVENT_NUM);   // more civilized!
#endif
	wait_queue_head_t waitq;
	struct workqueue_struct *wq;
	struct work_struct work;
	int enable_wq;
	ktime_t poll_delay;
	struct hrtimer timer;
#ifdef CONFIG_ARCH_KONA
	struct regulator *regulator;
#endif
};

struct ami306_dev_data *gpdev;


/*-===========================================================================
 *	Linux Device Driver API
 *-=========================================================================-*/
static int ami_open(struct inode *inode, struct file *file)
{
	struct ami306_dev_data *pdev = container_of(file->private_data, struct ami306_dev_data, dev);
	AMI_DLOG("AMI : Open device pdev(0x%x), gpdev(0x%x)\n", pdev, gpdev);
	return 0;
}

/*---------------------------------------------------------------------------*/
static int ami_release(struct inode *inode, struct file *file)
{
	AMI_DLOG("AMI : Release device");
	return 0;
}

/*---------------------------------------------------------------------------*/
static struct ami_sensor_parameter k_param;
static struct ami_sensor_value k_val;
static u8 k_offset[3];
static s16 k_si[9];
static s16 k_dir[2];
static struct ami_driverinfo k_drv;
static struct ami_register k_reg;
static char *k_mem = NULL;

static void ami_work_func(struct work_struct *work)
{
	struct ami306_dev_data *pdev = container_of(work, struct ami306_dev_data, work);
	struct ami_sensor_value axis;
	int res;

	res = AMI_GetValue(pdev->handle, &axis);
	/* Copy data to the KFIFO */
	if (res == 0) {
		kfifo_in(&pdev->ebuff, &axis, 1);
		wake_up_interruptible(&pdev->waitq);
	if (mod_debug)
            printk("ami_work_func: pass(%d,%d, %d) to HAL\n", axis.mag[0], axis.mag[1], axis.mag[2]);
	}
}

static enum hrtimer_restart ami_timer_func(struct hrtimer *timer)
{
	struct ami306_dev_data *pdev = container_of(timer, struct ami306_dev_data, timer);
	queue_work(pdev->wq, &pdev->work);
	hrtimer_forward_now(&pdev->timer, pdev->poll_delay);
	return HRTIMER_RESTART;
}

static int ami_enable_work(struct ami306_dev_data *pdev, int enable)
{
	if (enable != pdev->enable_wq) {
		if (enable) {
			hrtimer_start(&pdev->timer,
				ktime_set(0, AMI_SENSOR_START_TIME_DELAY), HRTIMER_MODE_REL);
		}
		else {
			hrtimer_cancel(&pdev->timer);
			cancel_work_sync(&pdev->work);
		}
		pdev->enable_wq = enable;
	}
	return 0;
}

static int ami_start(struct ami306_dev_data *pdev)
{
	int res = AMI_StartSensor(pdev->handle);
	if (0 > res) {
		printk("ami_start: failed to start sensor res=%d\n", res);
		return -EFAULT;
	}
	ami_enable_work(pdev, 1);
	return 0;
}

static int ami_stop(struct ami306_dev_data *pdev)
{
	int res;
	ami_enable_work(pdev, 0);
	res = AMI_StopSensor(pdev->handle);
	if (0 > res) {
		printk("ami_stop: failed to stop sensor res=%d\n", res);
		return -EFAULT;
	}
	return 0;
}

static int ami_cmd(unsigned int cmd, unsigned long arg, struct ami306_dev_data *pdev)
{
	int res = 0;
	void __user *argp = (void __user *)arg;
	int64_t poll_delay;

	switch (cmd) {
	case AMI_IOCTL_START_SENSOR:
		res = ami_start(pdev);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_STOP_SENSOR:
		res = ami_stop(pdev);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_GET_VALUE:
		res = AMI_GetValue(pdev->handle, &k_val);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_val, sizeof k_val))
			return -EFAULT;
		break;
	case AMI_IOCTL_SET_DELAY:
		if (copy_from_user(&poll_delay, argp, sizeof(int64_t))) {
			printk("AMI_IOCTL_SET_DELAY: failed\n");
			return -EFAULT;
		}
        AMI_DLOG("AMI_IOCTL_SET_DELAY set to %lld ns\n", poll_delay);
		pdev->poll_delay = ns_to_ktime(poll_delay);
		break;
	case AMI_IOCTL_SEARCH_OFFSET:
		res = AMI_SearchOffset(pdev->handle);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_WRITE_OFFSET:
		if (copy_from_user(k_offset, argp, sizeof(k_offset))) {
			printk("AMI_IOCTL_WRITE_OFFSET: failed\n");
			return -EFAULT;
		}
		res = AMI_WriteOffset(pdev->handle, k_offset);
		if (0 > res) {
			printk("AMI_WriteOffset failed return res %d\n", res);
			return -EFAULT;
		}
		break;
	case AMI_IOCTL_READ_OFFSET:
		res = AMI_ReadOffset(pdev->handle, k_offset);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, k_offset, sizeof k_offset))
			return -EFAULT;
		break;
	case AMI_IOCTL_SET_SOFTIRON:
		if (copy_from_user(k_si, argp, sizeof(k_si)))
			return -EFAULT;
		res = AMI_SetSoftIron(pdev->handle, k_si);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_GET_SOFTIRON:
		res = AMI_GetSoftIron(pdev->handle, k_si);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, k_si, sizeof k_si))
			return -EFAULT;
		break;
	case AMI_IOCTL_SET_DIR:
		AMI_DLOG("Set Direction");
		if (copy_from_user(k_dir, argp, sizeof(k_dir)))
			return -EFAULT;
		res = AMI_SetDirection(pdev->handle, k_dir[0], k_dir[1]);
		if (0 > res)
			return -EFAULT;
		break;
	case AMI_IOCTL_GET_DIR:
		AMI_DLOG("Get Direction");
		res = AMI_GetDirection(pdev->handle, &k_dir[0], &k_dir[1]);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, k_dir, sizeof k_dir))
			return -EFAULT;
		break;
	case AMI_IOCTL_READ_PARAMS:
		res = AMI_ReadParameter(pdev->handle, &k_param);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_param, sizeof k_param))
			return -EFAULT;
		break;
	case AMI_IOCTL_DRIVERINFO:
		res = AMI_DriverInformation(pdev->handle, &k_drv);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_drv, sizeof k_drv))
			return -EFAULT;
		break;
	case AMI_IOCTL_SELF_TEST:
		AMI_DLOG("Self Test");
		res = AMI_SelfTest(pdev->client);
		if (copy_to_user(argp, &res, sizeof(res)))
			return -EFAULT;
		break;
	case AMI_IOCTL_DBG_READ:
	case AMI_IOCTL_DBG_READ_W:
		if (copy_from_user(&k_reg, argp, sizeof(k_reg)))
			return -EFAULT;
		if (cmd == AMI_IOCTL_DBG_READ)
			AMI_i2c_recv_b(pdev->client,
					k_reg.adr, &k_reg.dat.byte);
		if (cmd == AMI_IOCTL_DBG_READ_W)
			AMI_i2c_recv_w(pdev->client,
					k_reg.adr, &k_reg.dat.word);
		if (copy_to_user(argp, &k_reg, sizeof k_reg))
			return -EFAULT;
		break;
	case AMI_IOCTL_DBG_WRITE:
	case AMI_IOCTL_DBG_WRITE_W:
		if (copy_from_user(&k_reg, argp, sizeof(k_reg)))
			return -EFAULT;
		if (cmd == AMI_IOCTL_DBG_WRITE)
			AMI_i2c_send_b(pdev->client, k_reg.adr, k_reg.dat.byte);
		if (cmd == AMI_IOCTL_DBG_WRITE_W)
			AMI_i2c_send_w(pdev->client, k_reg.adr, k_reg.dat.word);
		break;
	case AMI_IOCTL_GET_RAW:
		res = AMI_GetRawValue(pdev->handle, &k_val);
		if (0 > res)
			return -EFAULT;
		if (copy_to_user(argp, &k_val, sizeof k_val))
			return -EFAULT;
		break;
	default:
		AMI_LOG("%s not supported = 0x%08x", __func__, cmd);
		return -ENOIOCTLCMD;
	}
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static long ami_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int ami_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		     unsigned long arg)
#endif
{
	struct ami306_dev_data *pdev = container_of(file->private_data, struct ami306_dev_data, dev);
	int ret = 0;
	if(pdev->handle == NULL) {
		AMI_LOG("not initialized");
		return -EFAULT;
	}

	if (down_interruptible(&pdev->mutex)) {
		AMI_LOG("down_interruptible");
		return -ERESTARTSYS;
	}
	ret = ami_cmd(cmd, arg, pdev);
	up(&pdev->mutex);
	return ret;
}

static ssize_t ami_read(struct file *file, char *buf, size_t count, loff_t *ptr)
{
	struct ami306_dev_data *pdev = container_of(file->private_data, struct ami306_dev_data, dev);
	u32 iRead = 0;
	int res;

	wait_event_interruptible(pdev->waitq, (kfifo_len(&(pdev->ebuff))> 0));

	/* copy to user */
	res = kfifo_to_user(&pdev->ebuff, buf, count, &iRead);
	if (res < 0) {
		printk("ami_read failed to copy\n");
		return -EFAULT;
	}

	return iRead;
}

static unsigned int ami_poll(struct file *file, poll_table *wait)
{
	struct ami306_dev_data *pdev = container_of(file->private_data, struct ami306_dev_data, dev);
	unsigned int mask = 0;

	poll_wait(file, &pdev->waitq, wait);
	if (kfifo_len(&pdev->ebuff) > 0) {
		mask = POLLIN | POLLRDNORM;
	}

	return mask;
}

/*----------------------------------------------------------------------------
 *     Sysfs
 *===========================================================================*/
static ssize_t show_ami_polarity(struct device* dev, struct device_attribute* attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);

	if (!pdata)
		return sprintf(buf, "NULL ami306_dev_data\n");

	return sprintf(buf, "%d\n", pdata->ami_polarity);
}

static ssize_t store_ami_polarity(struct device* dev, struct device_attribute* attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);
	int n;

	if (count <= 0)
		return 0;

	sscanf(buf, "%d", &n);
	pdata->ami_polarity = n;

	return count;
}
static DEVICE_ATTR(ami_polarity, 0644, show_ami_polarity, store_ami_polarity);

static ssize_t show_ami_dir(struct device* dev, struct device_attribute* attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);

	if (!pdata)
		return sprintf(buf, "NULL ami306_dev_data\n");

	return sprintf(buf, "%d\n", pdata->ami_dir);
}

static ssize_t store_ami_dir(struct device* dev, struct device_attribute* attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);
	int n;

	if (count <= 0)
		return 0;

	sscanf(buf, "%d", &n);
	pdata->ami_dir = n;

	return count;
}
static DEVICE_ATTR(ami_dir, 0644, show_ami_dir, store_ami_dir);

static ssize_t store_ami_enable(struct device *dev, struct device_attribute*attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);
	int n;

	if (count <= 0)
		return 0;

	sscanf(buf, "%d", &n);
	if (n)
		ami_start(pdata);
	else
		ami_stop(pdata);

	return count;
}
static DEVICE_ATTR(ami_enable, 0644, NULL, store_ami_enable);

static ssize_t show_ami_poll_delay(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);

	if (!pdata)
		return sprintf(buf, "NULL ami306_dev_data\n");

	return sprintf(buf, "%lld ns\n", ktime_to_ns(pdata->poll_delay));
}

static ssize_t store_ami_poll_delay(struct device* dev, struct device_attribute* attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);
	int64_t new_delay;
	int err;

	err = strict_strtoll(buf, 10, &new_delay);
	if (err < 0)
		return err;

	pdata->poll_delay = ns_to_ktime(new_delay);
	return count;
}
static DEVICE_ATTR(ami_poll_delay, 0644, show_ami_poll_delay, store_ami_poll_delay);
static struct attribute *ami_attributes[] ={
	&dev_attr_ami_polarity.attr,
	&dev_attr_ami_dir.attr,
	&dev_attr_ami_enable.attr,
	&dev_attr_ami_poll_delay.attr,
	NULL
};

static const struct attribute_group ami_attr_group =
{
	.attrs = ami_attributes,
};


/*-===========================================================================
 *	Linux Kernel Module
 *-=========================================================================-*/
static struct file_operations ami_fops = {
	.owner = THIS_MODULE,
	.open = ami_open,
	.release = ami_release,
	.read = ami_read,
	.poll = ami_poll,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.unlocked_ioctl = ami_ioctl,
#else
	.ioctl = ami_ioctl,
#endif
};

/*---------------------------------------------------------------------------*/
static int __devinit ami_probe(struct i2c_client *client,
			       const struct i2c_device_id *devid)
{
	int res = 0;
	struct ami306_dev_data *pdev = NULL;
	struct ami306_platform_data *pdata = NULL;

	AMI_LOG("%s %s start", AMI_DRV_NAME, __func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "AMI : adapter can NOT support I2C_FUNC_I2C.");
		return -ENODEV;
	}

	pdev = (struct ami306_dev_data *)
		kzalloc(sizeof(struct ami306_dev_data), GFP_KERNEL);
	if (!pdev) {
		printk(KERN_ERR "AMI: no memory to allocate ami306_dev_data");
		return -ENOMEM;
	}

	if (client->dev.platform_data)
		pdata =
			(struct ami306_platform_data *)
				client->dev.platform_data;

	else if (client->dev.of_node) {
		struct device_node *np = client->dev.of_node;
		const char *prop;
		u32 val;
		pdata = kzalloc(sizeof(struct ami306_platform_data),
				GFP_KERNEL);
		if (!pdata) {
			kfree(pdev);
			return -ENOMEM;
		}

		if (of_property_read_u32(np, "gpio-intr", &val)) {
			res = -EIO;
			goto err_free_pdev;
		}
		pdata->gpio_intr = val;

		if (of_property_read_u32(np, "gpio-drdy", &val)) {
			res = -EIO;
			goto err_free_pdev;
		}
		pdata->gpio_drdy = val;

		if (of_property_read_u32(np, "dir", &val)) {
			res = -EIO;
			goto err_free_pdev;
		}
		pdata->dir = val;

		if (of_property_read_u32(np, "polarity", &val)) {
			res = -EIO;
			goto err_free_pdev;
		}
		pdata->polarity = val;

		if (of_property_read_string(np, "regulator-name", &prop)) {
			printk(KERN_ERR "%s: can't get regulator name from DT!\n",
				__func__);
			goto err_free_pdev;
		}
		pdata->supply_name = prop;
		client->dev.platform_data = pdata;
	} else {
		kfree(pdev);
		return -EINVAL;
	}

		pdev->ami_dir = pdata->dir;
		pdev->ami_polarity = pdata->polarity;
		pdev->gpio_drdy = pdata->gpio_drdy;
		pdev->gpio_intr = pdata->gpio_intr;

	/* Init gpio */
	if (pdev->gpio_drdy) {
		res = gpio_request(pdev->gpio_drdy, "AMI306 DRDY");
		if (res) {
			printk(KERN_ERR "ami_probe: failed to request GPIO\n");
			goto err_free_pdev;
		}
		res = gpio_direction_input(pdev->gpio_drdy);
		if (res) {
			printk(KERN_ERR "ami_probe: failed to set direction for GPIO\n");
			goto err_free_pdev;
		}
	}
	if (pdev->gpio_intr) {
		res = gpio_request(pdev->gpio_intr, "AMI306 INTR");
		if (res) {
			printk(KERN_ERR "ami_probe: failed to request GPIO\n");
			goto err_free_pdev;
		}
		res = gpio_direction_input(pdev->gpio_intr);
		if (res) {
			printk(KERN_ERR "ami_probe: failed to set direction for GPIO\n");
			goto err_free_pdev;
		}
	}
	printk("%s: setup ami_dir:%d, ami_polarity:%d, intr:%d, drdy:%d\n",
		__func__, pdev->ami_dir, pdev->ami_polarity,
		pdev->gpio_intr, pdev->gpio_drdy);

	sema_init(&pdev->mutex, 1);
	pdev->dev.minor = MISC_DYNAMIC_MINOR;
	pdev->dev.name = AMI_DRV_NAME;
	pdev->dev.fops = &ami_fops;
	res = misc_register(&pdev->dev);
	if (res < 0) {
		printk(KERN_ERR "%s: failed to do misc_regsiter return %d\n",
			__func__, res);
		goto err_free_pdev;
	}
#ifdef CONFIG_ARCH_KONA
	pdev->regulator = regulator_get(&client->dev, pdata->supply_name);

	res = IS_ERR_OR_NULL(pdev->regulator);
	if (res) {
		pdev->regulator = NULL;
		printk(KERN_ERR "AMI: %s, can't get vdd regulator!\n",
			__func__);
		res = -EIO;
		goto err_misc_deregister;
	}

	/* make sure that regulator is enabled if device is successfully
		bound */
	res = regulator_enable(pdev->regulator);
	AMI_LOG("AMI: %s, regulator_enable(cam2) returned %d\n",
		__func__, res);
	if (res) {
		printk(KERN_ERR "AMI: %s, regulator_enable(cam2) error=%d\n",
			__func__, res);
		res = -EIO;
		goto err_regulator_enable;
	}
#endif

#ifdef USER_MEMORY
	/* Initialize driver command */
	k_mem = kmalloc(AMI_GetMemSize(), GFP_KERNEL);
	if (k_mem == NULL) {
		printk(KERN_ERR "ami_probe: kmalloc error");
		res = -ENODEV;
		goto err_exit_regulator;
	}

	pdev->handle = AMI_InitDriver(k_mem, client,
				pdev->ami_dir, pdev->ami_polarity);
#else
	pdev->handle = AMI_InitDriver(client,
				pdev->ami_dir, pdev->ami_polarity);
#endif
	if (pdev->handle == NULL) {
		printk(KERN_ERR "AMI : AMI_InitDriver error.");
		res = -ENODEV;
		goto err_free_kmem;
	}
	pdev->client = client;
	i2c_set_clientdata(client, pdev);

	/* initialize the waitq */
	init_waitqueue_head(&pdev->waitq);

	/* initialize the kfifo */
	INIT_KFIFO(pdev->ebuff);

	/* create workqueue */
	pdev->wq = create_singlethread_workqueue("ami306_wq");
	if (!pdev->wq) {
		printk(KERN_ERR "AMI: Failed to create workqueue");
		res = -ENOMEM;
		goto err_free_kmem;
	}

	/* initialize the work struct */
	INIT_WORK(&pdev->work, ami_work_func);
	pdev->enable_wq = 0;

	/* create hrtimer, we don't just do msleep_interruptible as android
	   would pass something in unit of nanoseconds */
	hrtimer_init(&pdev->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pdev->poll_delay = ns_to_ktime(500 * NSEC_PER_MSEC);
	pdev->timer.function = ami_timer_func;

	/* Register sysfs hooks */
	res = sysfs_create_group(&client->dev.kobj, &ami_attr_group);
	if (res) {
		printk(KERN_ERR "AMI: sysfs_create_group failed with error %d",
			res);
		res = -ENODEV;
		goto err_destroy_workqueue;
	}
	AMI_LOG("%s %s end", AMI_DRV_NAME, __func__);
	return res;
err_destroy_workqueue:
	destroy_workqueue(pdev->wq);
err_free_kmem:
#ifdef USER_MEMORY
	if (k_mem) {
		kfree(k_mem);
		k_mem = NULL;
	}
#endif
err_exit_regulator:
#ifdef CONFIG_ARCH_KONA
	if (pdev->regulator)
		regulator_disable(pdev->regulator);
#endif
err_regulator_enable:
#ifdef CONFIG_ARCH_KONA
	if (pdev->regulator)
		regulator_put(pdev->regulator);
#endif
err_misc_deregister:
	misc_deregister(&pdev->dev);
err_free_pdev:
	kfree(pdev);
	if (client->dev.of_node)
		kfree(pdata);
	i2c_set_clientdata(client, NULL);
	return res;
}

/*---------------------------------------------------------------------------*/
static int __devexit ami_remove(struct i2c_client *client)
{
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);
	int ret = 0;

	AMI_LOG("%s %s %s", AMI_DRV_NAME, __func__, "start");

#if defined(CONFIG_PM) && defined(CONFIG_ARCH_KONA)
	if (pdata->regulator) {
		ret = regulator_disable(pdata->regulator);
		AMI_LOG("AMI: %s, called regulator_disable. Status: %d\n",
			__func__, ret);
		if (ret) {
			AMI_LOG("AMI: %s, regulator_disable failed with "
				"status: %d\n", __func__, ret);
			return ret;
		}
		regulator_put(pdata->regulator);
	}
#endif

	sysfs_remove_group(&client->dev.kobj, &ami_attr_group);
	misc_deregister(&pdata->dev);
	kfree(pdata);
	i2c_set_clientdata(client, NULL);
	if (k_mem) {
		kfree(k_mem);
		k_mem = NULL;
	}
	AMI_LOG("%s %s %s", AMI_DRV_NAME, __func__, "end");
	return 0;
}

#if defined(CONFIG_PM) && defined(CONFIG_ARCH_KONA)
static int ami306_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);
	int ret = 0;

	if (pdata->regulator) {
		ret = regulator_disable(pdata->regulator);
		AMI_LOG("AMI: %s, called regulator_disable. Status: %d",
			__func__, ret);
		if (ret)
			AMI_LOG("AMI: %s, regulator_disable failed with "
				"status: %d", __func__, ret);
	}
	return ret;
}

static int ami306_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ami306_dev_data *pdata = i2c_get_clientdata(client);
	int ret = 0;

	if (pdata->regulator) {
		ret = regulator_enable(pdata->regulator);
		AMI_LOG("AMI: %s, called regulator_enable. Status: %d\n",
			__func__, ret);
		if (ret)
			AMI_LOG("AMI: %s, regulator_enable failed with "
				"status: %d\n", __func__, ret);
	}
	return ret;
}

static const struct dev_pm_ops ami306_pm_ops = {
	.suspend	= ami306_suspend,
	.resume		= ami306_resume
};
#endif

/*---------------------------------------------------------------------------*/
static const struct i2c_device_id ami_idtable[] = {
	{AMI_DRV_NAME, 0},
	{}
};

/*---------------------------------------------------------------------------*/
static const struct of_device_id ami_of_match[] = {
	{ .compatible = "bcm,ami_sensor", },
	{},
}

MODULE_DEVICE_TABLE(of, ami_of_match);

static struct i2c_driver ami_i2c_driver = {
	.driver = {
		.name = AMI_DRV_NAME,
#if defined(CONFIG_PM) && defined(CONFIG_ARCH_KONA)
		.pm	= &ami306_pm_ops,
		.of_match_table = ami_of_match,
#endif
	},
	.probe = ami_probe,
	.remove = __devexit_p(ami_remove),
	.id_table = ami_idtable,
};

static int __init ami_init(void)
{
	printk("AMI306 INIT called\n");
	return i2c_add_driver(&ami_i2c_driver);
}

static void __exit ami_exit(void)
{
	return i2c_del_driver(&ami_i2c_driver);
}
module_init(ami_init);
module_exit(ami_exit);

MODULE_AUTHOR("AICHI STEEL");
MODULE_DESCRIPTION("AMI MI sensor");
MODULE_LICENSE("GPL");
