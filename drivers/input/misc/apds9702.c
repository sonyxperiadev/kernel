/* kernel/drivers/input/misc/apds9792.c
 *
 * Copyright (C) 2010 Sony Ericsson Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 *         Takashi Shiina <Takashi.Shiina@sonyericsson.com>
 *         Chikaharu Gonnokami <Chikaharu.X.Gonnokami@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/unistd.h>
#include <linux/apds9702.h>
#include <linux/mutex.h>
#include <linux/irq.h>
#include <linux/ctype.h>
#include <linux/input.h>
#include <linux/delay.h>

#define APDS9702_VENDOR      0x0001

#define APDS9702_THRESH_MAX    15
#define APDS9702_NBURST_MAX    15
#define APDS9702_FREQ_MAX       3
#define APDS9702_RFILT_MAX      3
#define APDS9702_DURATION_MAX   3

#define APDS9702_TRIG_BIT      15
#define APDS9702_PWR_BIT       14
#define APDS9702_NBURST_BIT    10
#define APDS9702_FREQ_BIT       8
#define APDS9702_DURATION_BIT   6
#define APDS9702_THRESH_BIT     2
#define APDS9702_RFILT_BIT      0

#define DOUT_VALUE_IF_DETECTED  0

#define APDS9702_NUM_TRIES      5
#define APDS9702_WAIT_TIME      5

struct apds9702data {
	struct input_dev *input_dev;
	struct i2c_client *client;
	struct mutex lock;
	int interrupt;
	u16 ctl_reg;
	unsigned int active:1;
};

static int apds9702_write_byte(struct i2c_client *i2c_client, u8 reg, u8 val)
{
	s32 rc;
	int n;
	struct apds9702_platform_data *pdata = i2c_client->dev.platform_data;

	for (n = 0; n < APDS9702_NUM_TRIES; n++) {
		rc = i2c_smbus_write_byte_data(i2c_client, reg, val);
		if (rc < 0) {
			dev_err(&i2c_client->dev,
					"i2c_smbus write failed, %d\n", rc);
			pdata->hw_config(&i2c_client->dev, 0);
			msleep(APDS9702_WAIT_TIME);
			pdata->hw_config(&i2c_client->dev, 1);
		} else
			return 0;
	}
	return -EIO;
}

static void apds9702_report(struct apds9702data *data)
{
	struct apds9702_platform_data *pdata = data->client->dev.platform_data;
	int d = gpio_get_value(pdata->gpio_dout);
	dev_dbg(&data->client->dev, "%s: gpio = %d\n", __func__, d);
	input_event(data->input_dev, EV_MSC, MSC_RAW,
		 d == DOUT_VALUE_IF_DETECTED ? 0 : 255);
	input_sync(data->input_dev);
}

static int apds9702_do_sensing(struct apds9702data *data, int enable)
{
	int err;
	struct apds9702_platform_data *pdata = data->client->dev.platform_data;

	dev_dbg(&data->client->dev, "%s: enable = %d\n", __func__, enable);
	if (enable) {
		pdata->hw_config(&data->client->dev, 1);
		err = apds9702_write_byte(data->client,
						data->ctl_reg & 0xFF,
						data->ctl_reg >> 8);
		if (err)
			goto err_exit;
		data->active = 1;
		apds9702_report(data);
		return 0;
	}
	err = apds9702_write_byte(data->client, 0, 0);
	data->active = 0;
err_exit:
	pdata->hw_config(&data->client->dev, 0);
	if (err)
		dev_err(&data->client->dev, "%s. I2C write error = %d\n",
			__func__, err);
	return err;
}

static irqreturn_t apds9702_work(int irq, void *handle)
{
	struct apds9702data *data = handle;

	dev_dbg(&data->client->dev, "%s\n", __func__);
	mutex_lock(&data->lock);
	if (data->active)
		apds9702_report(data);
	mutex_unlock(&data->lock);
	return IRQ_HANDLED;
}

static int apds9702_detect(struct i2c_client *client,
			  struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
	dev_dbg(&client->dev, "%s\n", __func__);
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -ENODEV;
	return 0;
}

static ssize_t attr_threshold_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct apds9702data *data = dev_get_drvdata(dev);
	int th = ((data->ctl_reg & (APDS9702_THRESH_MAX << APDS9702_THRESH_BIT))
		  >> APDS9702_THRESH_BIT);
	return snprintf(buf, PAGE_SIZE, "%d\n", th);
}

static ssize_t attr_threshold_set(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	ssize_t ret;
	unsigned long th;
	struct apds9702data *data = dev_get_drvdata(dev);

	ret = strict_strtoul(buf, 10, &th);
	if (!ret && th <= APDS9702_THRESH_MAX) {
		mutex_lock(&data->lock);
		data->ctl_reg = (data->ctl_reg &
			~(APDS9702_THRESH_MAX << APDS9702_THRESH_BIT)) |
			(th << APDS9702_THRESH_BIT);
		dev_dbg(dev, "%s threshold is %ld\n", __func__, th);
		if (data->active) {
			int err = i2c_smbus_write_byte_data(data->client,
							data->ctl_reg & 0xFF,
							data->ctl_reg >> 8);
			if (err)
				dev_err(dev, "%s: I2C write error = %d\n",
					__func__, err);
		}
		mutex_unlock(&data->lock);
		return size;
	}
	return -EINVAL;
}

static ssize_t attr_nburst_set(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	ssize_t ret;
	unsigned long nb;
	struct apds9702data *data = dev_get_drvdata(dev);
	ret = strict_strtoul(buf, 10, &nb);
	if (!ret && nb <= APDS9702_NBURST_MAX) {
		mutex_lock(&data->lock);
		data->ctl_reg = (data->ctl_reg &
			~(APDS9702_NBURST_MAX << APDS9702_NBURST_BIT)) |
			(nb << APDS9702_NBURST_BIT);
		dev_dbg(dev, "%s nburst is %ld\n", __func__, nb);
		mutex_unlock(&data->lock);
		return size;
	}
	return -EINVAL;
}

static ssize_t attr_freq_set(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	ssize_t ret;
	unsigned long f;
	struct apds9702data *data = dev_get_drvdata(dev);
	ret = strict_strtoul(buf, 10, &f);
	if (!ret && f <= APDS9702_FREQ_MAX) {
		mutex_lock(&data->lock);
		data->ctl_reg = (data->ctl_reg &
			~(APDS9702_FREQ_MAX << APDS9702_FREQ_BIT)) |
			(f << APDS9702_FREQ_BIT);
		dev_dbg(dev, "%s freq is %ld\n", __func__, f);
		mutex_unlock(&data->lock);
		return size;
	}
	return -EINVAL;
}

static ssize_t attr_duration_cycle_set(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	ssize_t ret;
	unsigned long dc;
	struct apds9702data *data = dev_get_drvdata(dev);
	ret = strict_strtoul(buf, 10, &dc);
	if (!ret && dc <= APDS9702_DURATION_MAX) {
		mutex_lock(&data->lock);
		data->ctl_reg = (data->ctl_reg &
			~(APDS9702_DURATION_MAX << APDS9702_DURATION_BIT)) |
			(dc << APDS9702_DURATION_BIT);
		dev_dbg(dev, "%s duration cycle is %ld\n", __func__, dc);
		mutex_unlock(&data->lock);
		return size;
	}
	return -EINVAL;
}

static ssize_t attr_rfilt_set(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	ssize_t ret;
	unsigned long rf;
	struct apds9702data *data = dev_get_drvdata(dev);
	ret = strict_strtoul(buf, 10, &rf);
	if (!ret && rf <= APDS9702_RFILT_MAX) {
		mutex_lock(&data->lock);
		data->ctl_reg = (data->ctl_reg &
			~(APDS9702_RFILT_MAX << APDS9702_RFILT_BIT)) |
			(rf << APDS9702_RFILT_BIT);
		dev_dbg(dev, "%s duration cycle %ld\n", __func__, rf);
		mutex_unlock(&data->lock);
		return size;
	}
	return -EINVAL;
}

static struct device_attribute attributes[] = {
	__ATTR(threshold, 0600, attr_threshold_show, attr_threshold_set),
	__ATTR(nburst, 0200, NULL, attr_nburst_set),
	__ATTR(freq, 0200, NULL, attr_freq_set),
	__ATTR(cycle, 0200, NULL, attr_duration_cycle_set),
	__ATTR(filter, 0200, NULL, attr_rfilt_set),
};

static int create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;
error:
	for (; i >= 0 ; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s: Unable to create interface\n", __func__);
	return -ENODEV;
}

static void remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
}

static int apds9702_device_open(struct input_dev *dev)
{
	struct apds9702data *data = input_get_drvdata(dev);
	int rc;
	mutex_lock(&data->lock);
	rc = apds9702_do_sensing(data, 1);
	mutex_unlock(&data->lock);
	if (rc)
		dev_err(&data->client->dev, "%s. Failed to activate device,"
			" err = %d\n",	__func__, rc);
	return rc;
}

static void apds9702_device_close(struct input_dev *dev)
{
	struct apds9702data *data = input_get_drvdata(dev);
	int rc;
	mutex_lock(&data->lock);
	rc = apds9702_do_sensing(data, 0);
	mutex_unlock(&data->lock);
	if (rc)
		dev_err(&data->client->dev, "%s. Failed to deactivate device,"
			" err = %d\n",	__func__, rc);
}

static int apds9702_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	int err;
	struct apds9702_platform_data *pdata = client->dev.platform_data;
	struct apds9702data *data;

	dev_dbg(&client->dev, "%s\n", __func__);
	if (!pdata || !pdata->gpio_setup || !pdata->hw_config) {
		dev_err(&client->dev, "%s: platform data is not complete.\n",
			__func__);
		return -ENODEV;
	}
	err = pdata->gpio_setup(&client->dev, 1);
	if (err) {
		dev_err(&client->dev, "%s: gpio_setup failed\n", __func__);
		goto err_gpio_setup_failed;
	}
	pdata->hw_config(&client->dev, 1);
	err = apds9702_write_byte(client, 0, 0);
	pdata->hw_config(&client->dev, 0);
	if (err) {
		dev_err(&client->dev, "%s: device not responding"
			" error = %d\n", __func__, err);
		err = -ENODEV;
		goto err_not_responding;
	}
	data = kzalloc(sizeof(struct apds9702data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	data->client = client;
	i2c_set_clientdata(client, data);
	mutex_init(&data->lock);
	data->interrupt = gpio_to_irq(pdata->gpio_dout);
	if (data->interrupt < 0) {
		dev_err(&client->dev, "%s: gpio_to_irq failed\n", __func__);
		err = data->interrupt;
		goto err_get_irq_num_failed;
	}

	data->ctl_reg = pdata->ctl_reg.trg << APDS9702_TRIG_BIT |
			pdata->ctl_reg.pwr << APDS9702_PWR_BIT |
			pdata->ctl_reg.burst << APDS9702_NBURST_BIT |
			pdata->ctl_reg.frq << APDS9702_FREQ_BIT |
			pdata->ctl_reg.dur << APDS9702_DURATION_BIT |
			pdata->ctl_reg.th << APDS9702_THRESH_BIT |
			pdata->ctl_reg.rfilt << APDS9702_RFILT_BIT;

	data->input_dev = input_allocate_device();
	if (!data->input_dev) {
		dev_err(&client->dev, "%s: input_allocate_device failed\n",
			__func__);
		goto err_allocate_device;
	}

	input_set_drvdata(data->input_dev, data);

	data->input_dev->open = apds9702_device_open;
	data->input_dev->close = apds9702_device_close;
	data->input_dev->name = APDS9702_NAME;
	data->input_dev->phys = pdata->phys_dev_path;
	data->input_dev->id.vendor = APDS9702_VENDOR;
	data->input_dev->id.product = 1;
	data->input_dev->id.version = 1;
	data->input_dev->id.bustype = BUS_I2C;
	set_bit(EV_MSC, data->input_dev->evbit);
	set_bit(MSC_RAW, data->input_dev->mscbit);

	err = input_register_device(data->input_dev);
	if (err) {
		dev_err(&client->dev, "%s: input_register_device failed!",
			__func__);
		input_free_device(data->input_dev);
		goto err_register_device;
	}

	err = create_sysfs_interfaces(&client->dev);
	if (err)
		goto err_create_interfaces_failed;

	err = request_threaded_irq(data->interrupt, NULL, apds9702_work,
				   IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				   APDS9702_NAME, data);
	if (err < 0) {
		dev_err(&client->dev, "%s: request_irq failed\n", __func__);
		goto err_request_detect_irq;
	}
	err = irq_set_irq_wake(data->interrupt, pdata->is_irq_wakeup);
	if (err) {
		dev_err(&client->dev, "%s: set_irq_wake failed\n", __func__);
		goto err_request_wake_irq;
	}


	printk(KERN_INFO "%s: device create ok. I2C address = %x\n", __func__,
			client->addr);
	return 0;

err_request_wake_irq:
	free_irq(data->interrupt, data);
err_request_detect_irq:
	remove_sysfs_interfaces(&client->dev);
err_create_interfaces_failed:
	input_unregister_device(data->input_dev);
err_register_device:
err_allocate_device:
err_get_irq_num_failed:
	kfree(data);
exit_alloc_data_failed:
err_not_responding:
	pdata->gpio_setup(&client->dev, 0);
err_gpio_setup_failed:
	dev_err(&client->dev, "%s: device create failed.\n", __func__);
	return err;
}

static int apds9702_remove(struct i2c_client *client)
{
	struct apds9702_platform_data *pdata = client->dev.platform_data;
	struct apds9702data *data = i2c_get_clientdata(client);

	remove_sysfs_interfaces(&client->dev);
	free_irq(data->interrupt, data);
	input_unregister_device(data->input_dev);
	input_free_device(data->input_dev);
	input_set_drvdata(data->input_dev, NULL);
	pdata->hw_config(&client->dev, 0);
	pdata->gpio_setup(&client->dev, 0);
	kfree(data);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id apds9702_id[] = {
	{ APDS9702_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, apds9702_id);

static struct i2c_driver apds9702_driver = {
	.probe = apds9702_probe,
	.remove = apds9702_remove,
	.detect = apds9702_detect,
	.class  = I2C_CLASS_HWMON,
	.id_table = apds9702_id,
	.driver	 = {
		.owner = THIS_MODULE,
		.name = APDS9702_NAME,
	},
};

static int __init apds9702_init(void)
{
	return i2c_add_driver(&apds9702_driver);
}

static void __exit apds9702_exit(void)
{
	i2c_del_driver(&apds9702_driver);
}

module_init(apds9702_init);
module_exit(apds9702_exit);

MODULE_AUTHOR("SEMC");
MODULE_DESCRIPTION("APDS9702 proximity sensor driver");
MODULE_LICENSE("GPLv2");
