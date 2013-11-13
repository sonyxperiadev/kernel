/*
 * Copyright (C) 2012 MEMSIC, Inc.
 *
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/module.h>
#include <linux/mxc622x.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_platform.h>

#define	G_MAX	16000

#define	FUZZ 32
#define	FLAT 32
#define	I2C_RETRY_DELAY 5
#define	I2C_RETRIES 5
#define	I2C_AUTO_INCREMENT 0x80

/* RESUME STATE INDICES */

#define	RESUME_ENTRIES 20
#define DEVICE_INFO "Memsic, MXC622X"
#define DEVICE_INFO_LEN 32

/* end RESUME STATE INDICES */

#define DEBUG
#define MXC622X_DEBUG

#define	MAX_INTERVAL 50


struct mxc622x_reg {
	const char *name;
	u8 reg;
} mxc622x_regs[] = {
	{"XOUT", MXC622X_REG_XOUT},
	{"YOUT", MXC622X_REG_YOUT},
	{"STATUS", MXC622X_REG_STATUS},
	{"DETECTION", MXC622X_REG_DETECTION},
	{"CHIP_ID", MXC622X_REG_CHIPID},
};


static const struct of_device_id mxc622x_of_match[] = {
	{.compatible = "bcm,mxc622x",},
	{},
};
MODULE_DEVICE_TABLE(of, mxc622x_of_match);

struct mxc622x_acc_data {
	struct i2c_client *client;
	struct mxc622x_acc_platform_data *pdata;
	struct mutex lock;
	struct delayed_work input_work;
	struct input_dev *input_dev;
	int hw_initialized;
	/* hw_working=-1 means not tested yet */
	int hw_working;
	atomic_t enabled;
	int on_before_suspend;
	u8 resume_state[RESUME_ENTRIES];
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

/*
 * Because misc devices can not carry a pointer from driver register to
 * open, we keep this global.  This limits the driver to a single instance.
 */
struct mxc622x_acc_data *mxc622x_acc_misc_data;
struct i2c_client *mxc622x_i2c_client;

static int mxc622x_acc_i2c_read(struct mxc622x_acc_data *acc,
					u8 *buf, int len)
{
	int err;
	int tries = 0;

	struct i2c_msg msgs[] = {
		{
			.addr = acc->client->addr,
			.flags = acc->client->flags & I2C_M_TEN,
			.len = 1,
			.buf = buf,
		},
		{
			.addr = acc->client->addr,
			.flags = (acc->client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

	do {
		err = i2c_transfer(acc->client->adapter, msgs, 2);
		if (err != 2)
			msleep_interruptible(I2C_RETRY_DELAY);
	} while ((err != 2) && (++tries < I2C_RETRIES));

	if (err != 2) {
		dev_err(&acc->client->dev, "read transfer error\n");
		err = -EIO;
	} else
		err = 0;

	return err;
}

static int mxc622x_acc_i2c_write(struct mxc622x_acc_data *acc, u8 *buf,
				 int len)
{
	int err;
	int tries = 0;
	struct i2c_msg msgs[] = {
		{
			.addr = acc->client->addr,
			.flags = acc->client->flags & I2C_M_TEN,
			.len = len + 1,
			.buf = buf,
		},
	};
	do {
		err = i2c_transfer(acc->client->adapter, msgs, 1);
		if (err != 1)
			msleep_interruptible(I2C_RETRY_DELAY);
	} while ((err != 1) && (++tries < I2C_RETRIES));

	if (err != 1) {
		dev_err(&acc->client->dev, "write transfer error\n");
		err = -EIO;
	} else
		err = 0;
	return err;
}

static int mxc622x_acc_hw_init(struct mxc622x_acc_data *acc)
{
	int err = -1;
	u8 buf[7];

	pr_info("%s: hw init start\n", MXC622X_ACC_DEV_NAME);

	buf[0] = MXC622X_REG_CHIPID;
	err = mxc622x_acc_i2c_read(acc, buf, 1);
	if (err < 0)
		goto error_firstread;
	else
		acc->hw_working = 1;
	if ((buf[0] & 0x3F) != MXC622X_ACC_CHIPID) {
		err = -1;	/* choose the right coded error */
		goto error_unknown_device;
	}

	acc->hw_initialized = 1;
	pr_info("%s: hw init done\n", MXC622X_ACC_DEV_NAME);
	return 0;
error_firstread:
	acc->hw_working = 0;
	dev_warn(&acc->client->dev,
		 "reading MXC622X_REG_CHIPID:is dev available/working?\n");
	goto error1;
error_unknown_device:
	dev_err(&acc->client->dev,
		"device unknown. Expected: 0x%x,"
		"Replies: 0x%x\n", MXC622X_ACC_CHIPID, buf[0]);
error1:
	acc->hw_initialized = 0;
	dev_err(&acc->client->dev, "hw init error 0x%x,0x%x: %d\n", buf[0],
		buf[1], err);
	return err;
}

static void mxc622x_acc_device_power_off(struct mxc622x_acc_data *acc)
{
	int err;
	u8 buf[2] = {MXC622X_REG_CTRL, MXC622X_CTRL_PWRDN};

	err = mxc622x_acc_i2c_write(acc, buf, 1);
	if (err < 0)
		dev_err(&acc->client->dev, "soft power off failed: %d\n", err);
}

static int mxc622x_acc_device_power_on(struct mxc622x_acc_data *acc)
{
	int err = -1;
	u8 buf[2] = { MXC622X_REG_CTRL, MXC622X_CTRL_PWRON };

	err = mxc622x_acc_i2c_write(acc, buf, 1);
	if (err < 0)
		dev_err(&acc->client->dev, "soft power on failed: %d\n", err);

	if (!acc->hw_initialized) {
		err = mxc622x_acc_hw_init(acc);
		if (acc->hw_working == 1 && err < 0) {
			mxc622x_acc_device_power_off(acc);
			return err;
		}
	}

	return 0;
}


static int mxc622x_acc_register_read(struct mxc622x_acc_data *acc, u8 *buf,
				     u8 reg_address)
{

	int err = -1;
	buf[0] = (reg_address);
	err = mxc622x_acc_i2c_read(acc, buf, 1);
	return err;
}

static int mxc622x_acc_get_acceleration_data(struct mxc622x_acc_data *acc,
					     int *xyz)
{
	int err;
	u8 acc_x;
	u8 acc_y;
	/* Data bytes from hardware x, y */
	u8 acc_data[2];
	err = 0;

	acc_data[0] = MXC622X_REG_DATA;
	err = mxc622x_acc_i2c_read(acc, acc_data, 2);

	if (err < 0) {
#ifdef DEBUG
		pr_info("%s I2C read error %d\n", MXC622X_ACC_I2C_NAME,
		       err);
#endif
		return err;
	}
	acc_x = (u8)acc_data[0];
	acc_y = (u8)acc_data[1];
	switch (acc->pdata->orientation) {
	case MXC622X_ORIE_10_01:
		xyz[0] = acc_x;
		xyz[1] = acc_y;
		break;
	case MXC622X_ORIE_01_01:
		xyz[0] = -acc_x;
		xyz[1] = acc_y;
		break;
	case MXC622X_ORIE_01_10:
		xyz[0] = -acc_x;
		xyz[1] = -acc_y;
		break;
	case MXC622X_ORIE_10_10:
		xyz[0] = acc_x;
		xyz[1] = -acc_y;
		break;
	default:
		break;
	}
	xyz[2] = 32;

#ifdef MXC622X_DEBUG
	pr_info("[mxc622x] x = %d, y = %d\n", xyz[0], xyz[1]);
#endif

#ifdef MXC622X_DEBUG

	pr_info("%s read x=%d, y=%d, z=%d\n",
	       MXC622X_ACC_DEV_NAME, xyz[0], xyz[1], xyz[2]);
	pr_info("%s poll interval %d\n", MXC622X_ACC_DEV_NAME,
	       acc->pdata->poll_interval);

#endif
	return err;
}

static void mxc622x_acc_report_values(struct mxc622x_acc_data *acc, int *xyz)
{
	input_report_abs(acc->input_dev, ABS_X, xyz[0]);
	input_report_abs(acc->input_dev, ABS_Y, xyz[1]);
	input_report_abs(acc->input_dev, ABS_Z, xyz[2]);
	input_sync(acc->input_dev);
}

static int mxc622x_acc_enable(struct mxc622x_acc_data *acc)
{
	int err;

	if (!atomic_cmpxchg(&acc->enabled, 0, 1)) {
		err = mxc622x_acc_device_power_on(acc);
		msleep(300);
		if (err < 0) {
			atomic_set(&acc->enabled, 0);
			return err;
		}

		schedule_delayed_work(&acc->input_work,
				      msecs_to_jiffies(acc->pdata->
						       poll_interval));
	}

	return 0;
}

static int mxc622x_acc_disable(struct mxc622x_acc_data *acc)
{
	if (atomic_cmpxchg(&acc->enabled, 1, 0)) {
		cancel_delayed_work_sync(&acc->input_work);
		mxc622x_acc_device_power_off(acc);
	}

	return 0;
}

static int mxc622x_acc_misc_open(struct inode *inode, struct file *file)
{
	int err;
	err = nonseekable_open(inode, file);
	if (err < 0)
		return err;

	file->private_data = mxc622x_acc_misc_data;

	return 0;
}

static long mxc622x_acc_misc_ioctl(struct file *file,
				  unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int err;
	int interval;
	int xyz[3] = { 0 };
	struct mxc622x_acc_data *acc = file->private_data;


	switch (cmd) {
	case MXC622X_ACC_IOCTL_GET_DELAY:
		interval = acc->pdata->poll_interval;
		if (copy_to_user(argp, &interval, sizeof(interval)))
			return -EFAULT;
		break;

	case MXC622X_ACC_IOCTL_SET_DELAY:
		if (copy_from_user(&interval, argp, sizeof(interval)))
			return -EFAULT;
		if (interval < 0 || interval > 1000)
			return -EINVAL;
		if (interval > MAX_INTERVAL)
			interval = MAX_INTERVAL;
		acc->pdata->poll_interval = max(interval,
						acc->pdata->min_interval);
		break;

	case MXC622X_ACC_IOCTL_SET_ENABLE:
		if (copy_from_user(&interval, argp, sizeof(interval)))
			return -EFAULT;
		if (interval > 1)
			return -EINVAL;
		if (interval)
			err = mxc622x_acc_enable(acc);
		else
			err = mxc622x_acc_disable(acc);
		return err;
		break;

	case MXC622X_ACC_IOCTL_GET_ENABLE:
		interval = atomic_read(&acc->enabled);
		if (copy_to_user(argp, &interval, sizeof(interval)))
			return -EINVAL;
		break;
	case MXC622X_ACC_IOCTL_GET_COOR_XYZ:
		err = mxc622x_acc_get_acceleration_data(acc, xyz);
		if (err < 0)
			return err;
		if (copy_to_user(argp, xyz, sizeof(xyz))) {
			pr_err(" %s %d error in copy_to_user\n",
				__func__, __LINE__);
			return -EINVAL;
		}
		break;
	case MXC622X_ACC_IOCTL_GET_CHIP_ID:
		{
			u8 devid = 0;
			u8 devinfo[DEVICE_INFO_LEN] = { 0 };
			err =
			    mxc622x_acc_register_read(acc, &devid,
						      MXC622X_REG_CHIPID);
			if (err < 0) {
				pr_err("%s, error read mxc622x chip id\n",
					__func__);
				return -EAGAIN;
			}
			sprintf(devinfo, "%s, %#x", DEVICE_INFO, devid);

			if (copy_to_user(argp, devinfo, sizeof(devinfo))) {
				pr_err("%s error in copy_to_user\n",
					__func__);
				return -EINVAL;
			}
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static const struct file_operations mxc622x_acc_misc_fops = {
	.owner = THIS_MODULE,
	.open = mxc622x_acc_misc_open,
	.unlocked_ioctl = mxc622x_acc_misc_ioctl,
};

static struct miscdevice mxc622x_acc_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MXC622X_ACC_DEV_NAME,
	.fops = &mxc622x_acc_misc_fops,
};

static void mxc622x_acc_input_work_func(struct work_struct *work)
{
	struct mxc622x_acc_data *acc;

	int xyz[3] = { 0 };
	int err;

	acc = container_of((struct delayed_work *)work,
			   struct mxc622x_acc_data, input_work);

	mutex_lock(&acc->lock);
	err = mxc622x_acc_get_acceleration_data(acc, xyz);
	if (err < 0)
		dev_err(&acc->client->dev, "get_acceleration_data failed\n");
	else
		mxc622x_acc_report_values(acc, xyz);

	schedule_delayed_work(&acc->input_work,
			      msecs_to_jiffies(acc->pdata->poll_interval));
	mutex_unlock(&acc->lock);
}

#ifdef MXC622X_OPEN_ENABLE
int mxc622x_acc_input_open(struct input_dev *input)
{
	struct mxc622x_acc_data *acc = input_get_drvdata(input);

	return mxc622x_acc_enable(acc);
}

void mxc622x_acc_input_close(struct input_dev *dev)
{
	struct mxc622x_acc_data *acc = input_get_drvdata(dev);

	mxc622x_acc_disable(acc);
}
#endif


static int mxc622x_acc_input_init(struct mxc622x_acc_data *acc)
{
	int err;
	INIT_DELAYED_WORK(&acc->input_work,
			mxc622x_acc_input_work_func);

	acc->input_dev = input_allocate_device();
	if (!acc->input_dev) {
		err = -ENOMEM;
		dev_err(&acc->client->dev, "input device allocate failed\n");
		goto err0;
	}
#ifdef MXC622X_ACC_OPEN_ENABLE
	acc->input_dev->open = mxc622x_acc_input_open;
	acc->input_dev->close = mxc622x_acc_input_close;
#endif
	input_set_drvdata(acc->input_dev, acc);
	set_bit(EV_ABS, acc->input_dev->evbit);
	input_set_abs_params(acc->input_dev, ABS_X, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(acc->input_dev, ABS_Y, -G_MAX, G_MAX, FUZZ, FLAT);
	input_set_abs_params(acc->input_dev, ABS_Z, -G_MAX, G_MAX, FUZZ, FLAT);
	acc->input_dev->name = MXC622X_ACC_INPUT_NAME;
	err = input_register_device(acc->input_dev);
	if (err) {
		dev_err(&acc->client->dev,
			"unable to register input polled device %s\n",
			acc->input_dev->name);
		goto err1;
	}

	return 0;

err1:
	input_free_device(acc->input_dev);
err0:
	return err;
}

static void mxc622x_acc_input_cleanup(struct mxc622x_acc_data *acc)
{
	input_unregister_device(acc->input_dev);
	input_free_device(acc->input_dev);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void mxc622x_early_suspend(struct early_suspend *es);
static void mxc622x_early_resume(struct early_suspend *es);
#endif

static ssize_t mxc622x_registers_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	unsigned i, n, reg_count;
	u8 value;
	reg_count = sizeof(mxc622x_regs) / sizeof(mxc622x_regs[0]);
	for (i = 0, n = 0; i < reg_count; i++) {
		value = i2c_smbus_read_byte_data(mxc622x_i2c_client,
			mxc622x_regs[i].reg);
		n += scnprintf(buf + n, PAGE_SIZE - n,
			"%-20s = 0x%02X\n",
			mxc622x_regs[i].name,
			value);
	}
	return n;
}

static ssize_t mxc622x_registers_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned i, reg_count, value;
	int ret;
	char name[30];
	char reg_buf[2];
	if (count >= 30)
		return -EFAULT;
	if (sscanf(buf, "%30s %x", name, &value) != 2) {
		pr_err("input invalid\n");
		return -EFAULT;
	}
	reg_count = sizeof(mxc622x_regs) / sizeof(mxc622x_regs[0]);
	for (i = 0; i < reg_count; i++) {
		if (!strcmp(name, mxc622x_regs[i].name)) {
			reg_buf[0] = mxc622x_regs[i].reg;
			reg_buf[1] = value;
			ret = mxc622x_acc_i2c_write(mxc622x_acc_misc_data,
							reg_buf, 1);
			if (ret) {
				pr_err("Failed to write register %s\n", name);
				return -EFAULT;
			}
			return count;
		}
	}
	pr_err("no such register %s\n", name);
	return -EFAULT;
}

static DEVICE_ATTR(registers, 0644, mxc622x_registers_show,
				mxc622x_registers_store);

static struct attribute *mxc622x_ctrl_attr[] = {
	&dev_attr_registers.attr,
	NULL,
};

static struct attribute_group mxc622x_ctrl_attr_grp = {
	.attrs = mxc622x_ctrl_attr,
};


static int __devinit mxc622x_acc_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{

	struct mxc622x_acc_data *acc;
	struct device_node *np;
	int tempvalue;
	u32 val = 0;
	int err = -1;
	pr_info("mxc622x probe start.\n");
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "client not i2c capable\n");
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}


	acc = kzalloc(sizeof(struct mxc622x_acc_data), GFP_KERNEL);
	if (acc == NULL) {
		err = -ENOMEM;
		dev_err(&client->dev,
			"failed to allocate memory for module data: "
			"%d\n", err);
		goto exit_alloc_data_failed;
	}

	mutex_init(&acc->lock);

	acc->client = client;
	mxc622x_i2c_client = client;
	i2c_set_clientdata(client, acc);

	/* read chip id */
	tempvalue = i2c_smbus_read_word_data(client, MXC622X_REG_CHIPID);

	if ((tempvalue & 0x003F) == MXC622X_ACC_CHIPID) {
		pr_info("%s I2C driver registered!\n",
		       MXC622X_ACC_DEV_NAME);
	}
	acc->pdata = kzalloc(sizeof(struct mxc622x_acc_platform_data),
				GFP_KERNEL);
	if (acc->pdata == NULL) {
		err = -ENOMEM;
		dev_err(&client->dev,
			"failed to allocate memory for pdata: %d\n", err);
		goto exit_kfree_pdata;
	}
	if (client->dev.platform_data)
		acc->pdata = client->dev.platform_data;
	else if (client->dev.of_node) {
		np = mxc622x_i2c_client->dev.of_node;
		err = of_property_read_u32(np, "poll_interval", &val);
		if (err) {
			dev_err(&client->dev,
				"dts config poll_interval missed");
			acc->pdata->poll_interval = 200;
		}
		acc->pdata->poll_interval = val;
		err = of_property_read_u32(np, "min_interval", &val);
		if (err) {
			dev_err(&client->dev, "dts config min_interval missed");
			acc->pdata->min_interval = 10;
		}
		acc->pdata->min_interval = val;
		err = of_property_read_u32(np, "orientation", &val);
		if (err) {
			dev_err(&client->dev, "dts config orientation missed");
			acc->pdata->orientation = MXC622X_ORIE_10_01;
		}
		acc->pdata->orientation = val;
	}
	i2c_set_clientdata(client, acc);

	err = mxc622x_acc_device_power_on(acc);
	if (err < 0) {
		dev_err(&client->dev, "power on failed: %d\n", err);
		goto err_power_off;
	}

	atomic_set(&acc->enabled, 1);

	err = mxc622x_acc_input_init(acc);
	if (err < 0) {
		dev_err(&client->dev, "input init failed\n");
		goto err_power_off;
	}
	mxc622x_acc_misc_data = acc;

	err = misc_register(&mxc622x_acc_misc_device);
	if (err < 0) {
		dev_err(&client->dev,
			"misc MXC622X_ACC_DEV_NAME register failed\n");
		goto err_input_cleanup;
	}

	mxc622x_acc_device_power_off(acc);

	/* As default, do not report information */
	atomic_set(&acc->enabled, 0);

	acc->on_before_suspend = 0;
	err = sysfs_create_group(&mxc622x_i2c_client->dev.kobj,
		&mxc622x_ctrl_attr_grp);
	if (0 != err) {
		goto exit_input_register_device_failed;
		pr_err("sysfs create debug node fail, ret: %d\n", err);
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	acc->early_suspend.suspend = mxc622x_early_suspend;
	acc->early_suspend.resume = mxc622x_early_resume;
	acc->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	register_early_suspend(&acc->early_suspend);
#endif

	dev_info(&client->dev, "%s: probed\n", MXC622X_ACC_DEV_NAME);
	return 0;
exit_input_register_device_failed:
	sysfs_remove_group(&mxc622x_i2c_client->dev.kobj,
					&mxc622x_ctrl_attr_grp);
err_input_cleanup:
	mxc622x_acc_input_cleanup(acc);
err_power_off:
	mxc622x_acc_device_power_off(acc);
exit_kfree_pdata:
	kfree(acc->pdata);
	kfree(acc);
	i2c_set_clientdata(client, NULL);
	mxc622x_acc_misc_data = NULL;
exit_alloc_data_failed:
exit_check_functionality_failed:
	pr_err("%s: Driver Init failed\n", MXC622X_ACC_DEV_NAME);
	return err;
}

static int __devexit mxc622x_acc_remove(struct i2c_client *client)
{
	/* TODO: revisit ordering here once _probe order is finalized */
	struct mxc622x_acc_data *acc = i2c_get_clientdata(client);
	unregister_early_suspend(&acc->early_suspend);
	sysfs_remove_group(&mxc622x_i2c_client->dev.kobj,
				&mxc622x_ctrl_attr_grp);
	misc_deregister(&mxc622x_acc_misc_device);
	mxc622x_acc_input_cleanup(acc);
	mxc622x_acc_device_power_off(acc);
	kfree(acc->pdata);
	kfree(acc);

	return 0;
}

static int mxc622x_acc_resume(struct i2c_client *client)
{
	struct mxc622x_acc_data *acc = i2c_get_clientdata(client);
#ifdef MXC622X_DEBUG
	pr_info("%s.\n", __func__);
#endif

	if (acc != NULL && acc->on_before_suspend) {
		acc->on_before_suspend = 0;
		return mxc622x_acc_enable(acc);
	}

	return 0;
}

static int mxc622x_acc_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct mxc622x_acc_data *acc = i2c_get_clientdata(client);
#ifdef MXC622X_DEBUG
	pr_info("%s.\n", __func__);
#endif
	if (acc != NULL) {
		if (atomic_read(&acc->enabled)) {
			acc->on_before_suspend = 1;
			return mxc622x_acc_disable(acc);
		}
	}
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND

static void mxc622x_early_suspend(struct early_suspend *es)
{
	mxc622x_acc_suspend(mxc622x_i2c_client,
		(pm_message_t) {.event = 0}
				);
}

static void mxc622x_early_resume(struct early_suspend *es)
{
	mxc622x_acc_resume(mxc622x_i2c_client);
}

#endif /* CONFIG_HAS_EARLYSUSPEND */

static struct i2c_device_id mxc622x_acc_id[]
= { {MXC622X_ACC_DEV_NAME, 0}, {}, };

MODULE_DEVICE_TABLE(i2c, mxc622x_acc_id);

static struct i2c_driver mxc622x_acc_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = MXC622X_ACC_I2C_NAME,
		.of_match_table = mxc622x_of_match,
	},
	.probe = mxc622x_acc_probe,
	.remove = __devexit_p(mxc622x_acc_remove),
	.id_table = mxc622x_acc_id,
};


static int __init mxc622x_acc_init(void)
{
	int ret = 0;
	pr_info("mxc622x_acc_init+++\n");
	return i2c_add_driver(&mxc622x_acc_driver);
	return ret;
}

static void __exit mxc622x_acc_exit(void)
{
	pr_info("%s accelerometer driver exit\n",
	       MXC622X_ACC_DEV_NAME);
	i2c_del_driver(&mxc622x_acc_driver);
	return;
}

module_init(mxc622x_acc_init);
module_exit(mxc622x_acc_exit);

MODULE_DESCRIPTION("mxc622x accelerometer misc driver");
MODULE_AUTHOR("Memsic");
MODULE_LICENSE("GPL");
