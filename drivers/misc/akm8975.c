/* drivers/misc/akm8975.c - akm8975 compass driver
 *
 * Copyright (C) 2007-2008 HTC Corporation.
 * Author: Hou-Kun Chen <houkun.chen@gmail.com>
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

/*
 * Revised by AKM 2009/04/02
 * Revised by Motorola 2010/05/27
 *
 */

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
#include <linux/completion.h>
#include <linux/freezer.h>
#include <linux/akm8975.h>
#include <linux/earlysuspend.h>

#define AK8975DRV_CALL_DBG 0
#if AK8975DRV_CALL_DBG
#define FUNCDBG(msg)	pr_err("%s:%s\n", __func__, msg);
#else
#define FUNCDBG(msg)
#endif

#define AK8975DRV_DATA_DBG 0
#define MAX_FAILURE_COUNT 10
#define AK8975_MAX_CONVERSION_TIMEOUT	500

struct akm8975_data {
	struct i2c_client *this_client;
	struct akm8975_platform_data *pdata;
	struct input_dev *input_dev;
	struct completion data_ready;
	struct mutex flags_lock;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

/*
* Because misc devices can not carry a pointer from driver register to
* open, we keep this global. This limits the driver to a single instance.
*/
struct akm8975_data *akmd_data;

static DECLARE_WAIT_QUEUE_HEAD(open_wq);

static atomic_t open_flag;

static short mv_flag;

static short akmd_delay;

static ssize_t akm8975_show(struct device *dev, struct device_attribute *attr,
				 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	return sprintf(buf, "%u\n", i2c_smbus_read_byte_data(client,
							     AK8975_REG_CNTL));
}
static ssize_t akm8975_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val;
	strict_strtoul(buf, 10, &val);
	if (val > 0xff)
		return -EINVAL;
	i2c_smbus_write_byte_data(client, AK8975_REG_CNTL, val);
	return count;
}
static DEVICE_ATTR(akm_ms1, S_IWUSR | S_IRUGO, akm8975_show, akm8975_store);


/*
 * Helper function to write to the I2C device's registers.
 */
static int akm8975_i2c_txdata(struct akm8975_data *akm, u8 reg, u8 val)
{
	struct i2c_msg msg;
	u8 w_data[2];
	int ret = 0;
 
	w_data[0] = reg;
	w_data[1] = val;

	msg.addr = akm->this_client->addr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = w_data;

	FUNCDBG("called");

	ret = i2c_transfer(akm->this_client->adapter, &msg, 1);

	if (ret < 0) {
		dev_err(&akm->this_client->dev, "Write to device fails status %x\n", ret);
 	}
 
	return ret;
}
 
/*
 * Helper function to read a contiguous set of the I2C device's registers.
 */
static int akm8975_i2c_rxdata(struct akm8975_data *akm,
			    u8 reg, u8 length, u8 *buffer)
{
	struct i2c_msg msg[2];
	u8 w_data[2];
	int ret;

	w_data[0] = reg;

	msg[0].addr = akm->this_client->addr;
	msg[0].flags = 0;	 
	msg[0].len = 1;
	msg[0].buf = w_data;

	msg[1].addr = akm->this_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = length;
	msg[1].buf = buffer;

	ret = i2c_transfer(akm->this_client->adapter, msg, 2);
	if (ret < 0) {
		dev_err(&akm->this_client->dev, "Read from device fails\n");
		return ret;
	}

	return 0;
}


static int akm8975_ecs_get_data(struct akm8975_data *akm, u8 length, char *buffer)
{
	memset(buffer, 0, length);
	
	return akm8975_i2c_rxdata(akm, AK8975_REG_ST1, length, buffer);
}

static int akm8975_ecs_set_mode(struct akm8975_data *akm, char mode)
{
	/* guarantee some delay between each mode switch
	 */
	msleep(1);
 	return akm8975_i2c_txdata(akm, AK8975_REG_CNTL, mode);
}


static void akm8975_ecs_report_value(struct akm8975_data *akm, int *rbuf)
{
	struct akm8975_data *data = i2c_get_clientdata(akm->this_client);

	FUNCDBG("called");

#if AK8975DRV_DATA_DBG
	pr_info("Magnetic:	 x = %d , y = %d , z = %d \n\n",
				 rbuf[0], rbuf[1], rbuf[2]);
#endif

	mutex_lock(&akm->flags_lock);

	if (mv_flag) {
		input_report_abs(data->input_dev, ABS_HAT0X, rbuf[0]);
		input_report_abs(data->input_dev, ABS_HAT0Y, rbuf[1]);
		input_report_abs(data->input_dev, ABS_BRAKE, rbuf[2]);
	}
	mutex_unlock(&akm->flags_lock);

	input_sync(data->input_dev);
}

static void akm8975_ecs_close_done(struct akm8975_data *akm)
{
	FUNCDBG("called");
	mutex_lock(&akm->flags_lock);
	mv_flag = 1;
	mutex_unlock(&akm->flags_lock);
}

static int akm_aot_open(struct inode *inode, struct file *file)
{
	int ret = -1;

	FUNCDBG("called");
	if (atomic_cmpxchg(&open_flag, 0, 1) == 0) {
		wake_up(&open_wq);
		ret = 0;
	}

	ret = nonseekable_open(inode, file);
	if (ret)
		return ret;

	file->private_data = akmd_data;

	return ret;
}

static int akm_aot_release(struct inode *inode, struct file *file)
{
	FUNCDBG("called");
	atomic_set(&open_flag, 0);
	wake_up(&open_wq);
	return 0;
}

static int akm_aot_ioctl(struct inode *inode, struct file *file,
	      unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *) arg;
	short flag;
	struct akm8975_data *akm = file->private_data;

 	FUNCDBG("called");

	switch (cmd) {
	case ECS_IOCTL_APP_SET_MVFLAG:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		if (flag < 0 || flag > 1)
			return -EINVAL;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		if (copy_from_user(&flag, argp, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	mutex_lock(&akm->flags_lock);
	switch (cmd) {
	case ECS_IOCTL_APP_SET_MVFLAG:
		mv_flag = flag;
		break;
	case ECS_IOCTL_APP_GET_MVFLAG:
		flag = mv_flag;
		break;
	case ECS_IOCTL_APP_SET_DELAY:
		akmd_delay = flag;
		break;
	case ECS_IOCTL_APP_GET_DELAY:
		flag = akmd_delay;
		break;
	default:
		return -ENOTTY;
	}
	mutex_unlock(&akm->flags_lock);

	switch (cmd) {
	case ECS_IOCTL_APP_GET_MVFLAG:
	case ECS_IOCTL_APP_GET_DELAY:
		if (copy_to_user(argp, &flag, sizeof(flag)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static int akmd_open(struct inode *inode, struct file *file)
{
	int err = 0;

	FUNCDBG("called");
	err = nonseekable_open(inode, file);
	if (err)
		return err;

	file->private_data = akmd_data;
	return 0;
}

static int akmd_release(struct inode *inode, struct file *file)
{
	struct akm8975_data *akm = file->private_data;

	FUNCDBG("called");
	akm8975_ecs_close_done(akm);
	return 0;
}

static int akmd_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		      unsigned long arg)
{
	void __user *argp = (void __user *) arg;

	char rwbuf[16];
	char databuf[RBUFF_SIZE];
	int ret = -1;
	int status;
	int value[3];
	short delay;
	short mode;
	struct akm8975_data *akm = file->private_data;
	
	FUNCDBG("called");

	switch (cmd) {
	case ECS_IOCTL_READ:
	case ECS_IOCTL_WRITE:
		if (copy_from_user(&rwbuf, argp, sizeof(rwbuf)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_MODE:
		if (copy_from_user(&mode, argp, sizeof(mode)))
			return -EFAULT;
		break;
	case ECS_IOCTL_SET_YPR:
		if (copy_from_user(&value, argp, sizeof(value)))
			return -EFAULT;
		break;

	default:
		break;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		if ((rwbuf[0] > AK8975_FUSE_ASAZ) || (rwbuf[1] < 1) ||
			(rwbuf[1] > 3)) {
			pr_err("AKM8975 IOCTL_READ invalid argument: %d, %d\n", 
				(int)rwbuf[0], (int)rwbuf[1]);
			return -EINVAL;
		}

		ret = akm8975_i2c_rxdata(akm, rwbuf[0], rwbuf[1], &rwbuf[2]);
		if (ret < 0)
			return ret;
		break;

	case ECS_IOCTL_WRITE:
		if (rwbuf[0] < 2)
			return -EINVAL;

		ret = akm8975_i2c_txdata(akm, rwbuf[0], rwbuf[1]);
		if (ret < 0)
			return ret;
		break;

	case ECS_IOCTL_SET_MODE:
		ret = akm8975_ecs_set_mode(akm, (char) mode);
		if (ret < 0)
			return ret;

		if (mode == AK8975_MODE_SNG_MEASURE) {
			/* wait for data to become ready */
			ret = wait_for_completion_interruptible_timeout(&akm->data_ready, 
						msecs_to_jiffies(AK8975_MAX_CONVERSION_TIMEOUT));
			if (ret < 0) {
				pr_err("AKM8975 conversion timeout happened\n");
				return -EINVAL;
			}
		}
		break;

	case ECS_IOCTL_GETDATA:
		ret = akm8975_ecs_get_data(akm, RBUFF_SIZE, databuf);
		if (ret < 0)
			return ret;

	case ECS_IOCTL_SET_YPR:
		akm8975_ecs_report_value(akm, value);
		break;

	case ECS_IOCTL_GET_OPEN_STATUS:
		wait_event_interruptible(open_wq,
					 (atomic_read(&open_flag) != 0));
		status = atomic_read(&open_flag);
		break;
	case ECS_IOCTL_GET_CLOSE_STATUS:
		wait_event_interruptible(open_wq,
					 (atomic_read(&open_flag) == 0));
		status = atomic_read(&open_flag);
		break;

	case ECS_IOCTL_GET_DELAY:
		delay = akmd_delay;
		break;

	default:
		FUNCDBG("Unknown cmd\n");
		return -ENOTTY;
	}

	switch (cmd) {
	case ECS_IOCTL_READ:
		if (copy_to_user(argp, &rwbuf[2], rwbuf[1]))
			return -EFAULT;
		break;
	case ECS_IOCTL_GETDATA:
		if (copy_to_user(argp, &databuf, RBUFF_SIZE))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_OPEN_STATUS:
	case ECS_IOCTL_GET_CLOSE_STATUS:
		if (copy_to_user(argp, &status, sizeof(status)))
			return -EFAULT;
		break;
	case ECS_IOCTL_GET_DELAY:
		if (copy_to_user(argp, &delay, sizeof(delay)))
			return -EFAULT;
		break;
	default:
		break;
	}

	return 0;
}

static irqreturn_t akm8975_interrupt(int irq, void *dev_id)
{
	struct akm8975_data *akm = dev_id;
	FUNCDBG("called");

	complete(&akm->data_ready);

	return IRQ_HANDLED;
}

static int akm8975_power_off(struct akm8975_data *akm)
{
#if AK8975DRV_CALL_DBG
	pr_info("%s\n", __func__);
#endif
	if (akm->pdata->power_off)
		akm->pdata->power_off();

	return 0;
}

static int akm8975_power_on(struct akm8975_data *akm)
{
	int err;

#if AK8975DRV_CALL_DBG
	pr_info("%s\n", __func__);
#endif
	if (akm->pdata->power_on) {
		err = akm->pdata->power_on();
		if (err < 0)
			return err;
	}
	return 0;
}

static int akm8975_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct akm8975_data *akm = i2c_get_clientdata(client);

#if AK8975DRV_CALL_DBG
	pr_info("%s\n", __func__);
#endif
	/* TO DO: might need more work after power mgmt
	   is enabled */
	return akm8975_power_off(akm);
}

static int akm8975_resume(struct i2c_client *client)
{
	struct akm8975_data *akm = i2c_get_clientdata(client);

#if AK8975DRV_CALL_DBG
	pr_info("%s\n", __func__);
#endif
	/* TO DO: might need more work after power mgmt
	   is enabled */
	return akm8975_power_on(akm);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void akm8975_early_suspend(struct early_suspend *handler)
{
	struct akm8975_data *akm;
	akm = container_of(handler, struct akm8975_data, early_suspend);

#if AK8975DRV_CALL_DBG
	pr_info("%s\n", __func__);
#endif
	akm8975_suspend(akm->this_client, PMSG_SUSPEND);
}

static void akm8975_early_resume(struct early_suspend *handler)
{
	struct akm8975_data *akm;
	akm = container_of(handler, struct akm8975_data, early_suspend);

#if AK8975DRV_CALL_DBG
	pr_info("%s\n", __func__);
#endif
	akm8975_resume(akm->this_client);
}
#endif


static int akm8975_init_client(struct i2c_client *client)
{
	struct akm8975_data *data;
	int gpio_drdy; 
	int ret;

	data = i2c_get_clientdata(client);

	ret = request_irq(client->irq, akm8975_interrupt, IRQF_TRIGGER_RISING,
				"akm8975", data);
	if (ret < 0) {
		pr_err("akm8975_init_client: request irq failed\n");
		goto err;
	}

	/* set up the Data Ready GPIO pin */
	gpio_drdy = irq_to_gpio(client->irq);
	if (!gpio_drdy) {
		pr_err("akm8975_init_client: no valid GPIO for data ready\n");
		ret = -EINVAL;
		goto err;
	}
	
	ret = gpio_request(gpio_drdy, "akm_8975_drdy");
	if (ret < 0) {
		pr_err("akm8975_init_client: request GPIO %d failed, err %d\n",
			gpio_drdy, ret);
		goto err;
	}
	
	ret = gpio_direction_input(gpio_drdy);
	if (ret < 0) {
		pr_err("akm8975_init_client: set GPIO %d as input failed, err %d\n",
			gpio_drdy, ret);
		gpio_free(gpio_drdy);
		goto err;
	}
	
	init_waitqueue_head(&open_wq);

	mutex_lock(&data->flags_lock);
	mv_flag = 1;
	mutex_unlock(&data->flags_lock);

	return 0;
err:
  return ret;
}

static const struct file_operations akmd_fops = {
	.owner = THIS_MODULE,
	.open = akmd_open,
	.release = akmd_release,
	.ioctl = akmd_ioctl,
};

static const struct file_operations akm_aot_fops = {
	.owner = THIS_MODULE,
	.open = akm_aot_open,
	.release = akm_aot_release,
	.ioctl = akm_aot_ioctl,
};

static struct miscdevice akm_aot_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8975_aot",
	.fops = &akm_aot_fops,
};

static struct miscdevice akmd_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "akm8975_dev",
	.fops = &akmd_fops,
};

int akm8975_probe(struct i2c_client *client,
		  const struct i2c_device_id *devid)
{
	struct akm8975_data *akm;
	int err;
	FUNCDBG("called");

	if (client->dev.platform_data == NULL) {
		dev_err(&client->dev, "platform data is NULL. exiting.\n");
		err = -ENODEV;
		goto exit_platform_data_null;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "platform data is NULL. exiting.\n");
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	akm = kzalloc(sizeof(struct akm8975_data), GFP_KERNEL);
	if (!akm) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	akm->pdata = client->dev.platform_data;

	mutex_init(&akm->flags_lock);
	init_completion(&akm->data_ready);

	i2c_set_clientdata(client, akm);

	err = akm8975_power_on(akm);
	if (err < 0)
		goto exit_power_on_failed;

	akm8975_init_client(client);
	akm->this_client = client;
	akmd_data = akm;

	akm->input_dev = input_allocate_device();
	if (!akm->input_dev) {
		err = -ENOMEM;
		dev_err(&akm->this_client->dev,
			"input device allocate failed\n");
		goto exit_input_dev_alloc_failed;
	}

	set_bit(EV_ABS, akm->input_dev->evbit);

	/* x-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0X, -4096, 4095, 0, 0);
	/* y-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_HAT0Y, -4096, 4095, 0, 0);
	/* z-axis of raw magnetic vector */
	input_set_abs_params(akm->input_dev, ABS_BRAKE, -4096, 4095, 0, 0);

	akm->input_dev->name = "compass";

	err = input_register_device(akm->input_dev);
	if (err) {
		pr_err("akm8975_probe: Unable to register input device: %s\n",
					 akm->input_dev->name);
		goto exit_input_register_device_failed;
	}

	err = misc_register(&akmd_device);
	if (err) {
		pr_err("akm8975_probe: akmd_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = misc_register(&akm_aot_device);
	if (err) {
		pr_err("akm8975_probe: akm_aot_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	err = device_create_file(&client->dev, &dev_attr_akm_ms1);

#ifdef CONFIG_HAS_EARLYSUSPEND
	akm->early_suspend.suspend = akm8975_early_suspend;
	akm->early_suspend.resume = akm8975_early_resume;
	register_early_suspend(&akm->early_suspend);
#endif
	return 0;

exit_misc_device_register_failed:
exit_input_register_device_failed:
	input_free_device(akm->input_dev);
exit_input_dev_alloc_failed:
	akm8975_power_off(akm);
exit_power_on_failed:
	kfree(akm);
exit_alloc_data_failed:
exit_check_functionality_failed:
exit_platform_data_null:
	return err;
}

static int __devexit akm8975_remove(struct i2c_client *client)
{
	struct akm8975_data *akm = i2c_get_clientdata(client);
	FUNCDBG("called");

	gpio_free(irq_to_gpio(client->irq));	
	free_irq(client->irq, NULL);
	input_unregister_device(akm->input_dev);
	misc_deregister(&akmd_device);
	misc_deregister(&akm_aot_device);
	akm8975_power_off(akm);
	kfree(akm);
	return 0;
}

static const struct i2c_device_id akm8975_id[] = {
	{ "akm8975", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, akm8975_id);

static struct i2c_driver akm8975_driver = {
	.probe = akm8975_probe,
	.remove = akm8975_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.resume = akm8975_resume,
	.suspend = akm8975_suspend,
#endif
	.id_table = akm8975_id,
	.driver = {
		.name = "akm8975",
	},
};

static int __init akm8975_init(void)
{
	pr_info("AK8975 compass driver: init\n");
	FUNCDBG("AK8975 compass driver: init\n");
	return i2c_add_driver(&akm8975_driver);
}

static void __exit akm8975_exit(void)
{
	FUNCDBG("AK8975 compass driver: exit\n");
	i2c_del_driver(&akm8975_driver);
}

module_init(akm8975_init);
module_exit(akm8975_exit);

MODULE_AUTHOR("Hou-Kun Chen <hk_chen@htc.com>");
MODULE_DESCRIPTION("AK8975 compass driver");
MODULE_LICENSE("GPL");
