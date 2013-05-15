/*
 * vibrator-lc898300.c -  Linux kernel modules for linear vibrator
 *
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB
 * Copyright (c) 2012 Sony Mobile Communications AB
 *
 * Authors: Ardiana Karppinen <ardiana.karppinen@sonyericsson.com>
 *          Stefan Karlsson <stefan3.karlsson@sonyericsson.com>
 *          Yevgen Pronenko <yevgen.pronenko@sonymobile.com>
 *          Aleksej Makarov <aleksej.makarov@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/vibrator-lc898300.h>

#include "../staging/android/timed_output.h"

#define LC898300_REG_HBPW       0x01
#define LC898300_REG_RESOFRQ    0x02
#define LC898300_REG_STARTUP    0x03
#define LC898300_REG_BRAKE      0x04
#define LC898300_REG_STOPS      0x05
#define LC898300_MIN_ON         8
#define LC898300_OFF_DELAY      10
#define LC898300_BRAKE_TIME     40
#define LC898300_RESUME_DELAY   100

enum vib_state {
	VIB_OFF,
	VIB_TAKE_OFF,
	VIB_ON,
	VIB_LANDING,
};

struct lc898300_data {
	struct i2c_client *client;
	struct device *dev;
	struct lc898300_platform_data *pdata;
	struct timed_output_dev timed_dev;
	struct hrtimer vib_timer;
	struct mutex lock;
	int on_time;
	enum vib_state on;
	bool hw_on;
	atomic_t resuming;
	struct delayed_work resume_work;
	wait_queue_head_t resume_queue;
};

static const struct i2c_device_id lc898300_id[] = {
	{ LC898300_I2C_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, lc898300_id);

static void start_timer(struct lc898300_data *data, int t)
{
	dev_dbg(data->dev, "%s: start timer for %d ms\n", __func__, t);
	hrtimer_start(&data->vib_timer, ktime_set(t / MSEC_PER_SEC,
		(t % MSEC_PER_SEC) * NSEC_PER_MSEC), HRTIMER_MODE_REL);
}

static void forward_timer(struct lc898300_data *data, int t)
{
	dev_dbg(data->dev, "%s: fwd timer for %d ms\n", __func__, t);
	hrtimer_forward_now(&data->vib_timer, ktime_set(t / MSEC_PER_SEC,
		(t % MSEC_PER_SEC) * NSEC_PER_MSEC));
}

static void start_landing(struct lc898300_data *data)
{
	dev_dbg(data->dev, "%s: start braking\n", __func__);
	if (data->pdata->en_gpio_setup(false))
		dev_err(data->dev, "Failed to unset enable pin\n");
}

static void take_off(struct lc898300_data *data)
{
	dev_dbg(data->dev, "%s: taking off\n", __func__);
	if (data->pdata->en_gpio_setup(true))
		dev_err(data->dev, "Failed to set enable\n");
}

static int setup_hw(struct lc898300_data *data, bool enable)
{
	struct lc898300_vib_cmd *vib_cmd_info = data->pdata->vib_cmd_info;
	int rc = data->pdata->rstb_gpio_setup(enable);

	dev_dbg(data->dev, "%s: enable %d\n", __func__, enable);
	if (rc) {
		dev_err(data->dev, "Failed to set reset pin\n");
		return rc;
	}
	data->hw_on = enable;
	if (!enable)
		return 0;

	udelay(200);
	rc = i2c_smbus_write_i2c_block_data(data->client, LC898300_REG_HBPW,
				sizeof(struct lc898300_vib_cmd),
				(void *)&vib_cmd_info->vib_cmd_intensity);
	if (rc) {
		dev_err(data->dev, "Failed to setup vibrator; rc = %d\n", rc);
		data->hw_on = false;
		data->pdata->rstb_gpio_setup(false);
	}
	return rc;
}

static void vib_on(struct lc898300_data *data, int t)
{
	int rc;
	int remain;

	rc = hrtimer_cancel(&data->vib_timer);
	remain = rc ? ktime_to_ms(hrtimer_get_remaining(&data->vib_timer)) : -1;
	dev_dbg(data->dev, "%s: state %d, timer active %d, remaining %d ms\n",
		__func__, data->on, rc, remain);
	if (remain <= 0)
		/* add 1 ms to cmoplete the acion */
		remain = 1;
	switch (data->on) {
	case VIB_OFF:
		dev_dbg(data->dev, "%s: take off now!\n", __func__);
		if (!data->hw_on && setup_hw(data, true))
			break;
		take_off(data);
		data->on = VIB_TAKE_OFF;
		data->on_time = t - LC898300_MIN_ON;
		start_timer(data, LC898300_MIN_ON);
		break;
	case VIB_TAKE_OFF:
		data->on_time = t - remain;
		dev_dbg(data->dev, "%s: is taking off, set time\n", __func__);
		start_timer(data, remain);
		break;
	case VIB_ON:
		dev_dbg(data->dev, "%s: is running, add time\n", __func__);
		start_timer(data, t);
		break;
	case VIB_LANDING:
		data->on_time = t;
		dev_dbg(data->dev, "%s: is landing, add time\n", __func__);
		start_timer(data, remain);
		break;
	}
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	enum hrtimer_restart ret;
	struct lc898300_data *data = container_of(timer, struct lc898300_data,
			vib_timer);
	int t;

	dev_dbg(data->dev, "%s: state %d\n", __func__, data->on);
	switch (data->on) {
	case VIB_TAKE_OFF:
		if (data->on_time > 0) {
			data->on = VIB_ON;
			t = data->on_time;
			data->on_time = 0;
			dev_dbg(data->dev, "%s: full speed now!\n", __func__);
		} else {
			data->on = VIB_LANDING;
			start_landing(data);
			t = LC898300_BRAKE_TIME;
		}
		break;
	case VIB_ON:
		if (data->on_time > 0) {
			t = data->on_time;
			dev_dbg(data->dev, "%s: run for more time\n", __func__);
			data->on_time = 0;
		} else {
			data->on = VIB_LANDING;
			start_landing(data);
			t = LC898300_BRAKE_TIME;
		}
		break;
	case VIB_LANDING:
		if (data->on_time > 0) {
			dev_dbg(data->dev, "%s: landing done," \
					"but take off again\n", __func__);
			take_off(data);
			data->on = VIB_TAKE_OFF;
			t = LC898300_MIN_ON;
			data->on_time -= LC898300_MIN_ON;
		} else {
			data->on = VIB_OFF;
			t = 0;
			dev_dbg(data->dev, "%s: landing done, off\n", __func__);
			(void)setup_hw(data, false);
		}
		break;
	default:
	case VIB_OFF:
		dev_err(data->dev, "%s: shouldn't happen\n", __func__);
		t = 0;
		break;
	}

	if (t > 0) {
		forward_timer(data, t);
		ret = HRTIMER_RESTART;
	} else
		ret = HRTIMER_NORESTART;
	return ret;
}

static void request_vib_off(struct lc898300_data *data)
{
	int rc;
	int remain;

	rc = hrtimer_cancel(&data->vib_timer);
	remain = rc ? ktime_to_ms(hrtimer_get_remaining(&data->vib_timer)) : -1;

	dev_dbg(data->dev, "%s: state %d, timer active %d, remaining %d ms\n",
		__func__, data->on, rc, remain);

	if (remain <= 0)
		/* add 1 ms to complete the acion */
		remain = 1;

	switch (data->on) {
	case VIB_TAKE_OFF:
		dev_dbg(data->dev, "%s: killed on take-off\n", __func__);
		data->on_time = LC898300_OFF_DELAY - remain;
		start_timer(data, remain);
		break;

	case VIB_ON:
		dev_dbg(data->dev, "%s: killed while active\n", __func__);
		data->on_time = 0;
		start_timer(data, LC898300_OFF_DELAY);
		break;

	case VIB_LANDING:
		dev_dbg(data->dev, "%s: killed while landing\n", __func__);
		data->on_time = 0;
		start_timer(data, remain);
		break;
	case VIB_OFF:
	default:
		dev_dbg(data->dev, "%s: not active\n", __func__);
		break;
	}
}

static void lc898300_vib_enable(struct timed_output_dev *dev, int value)
{
	struct lc898300_data *data = container_of(dev, struct lc898300_data,
					 timed_dev);

	dev_dbg(data->dev, "%s: %d msec\n", __func__, value);

	mutex_lock(&data->lock);
	wait_event(data->resume_queue, !atomic_read(&data->resuming));
	if (value)
		vib_on(data, value);
	else
		request_vib_off(data);
	mutex_unlock(&data->lock);
}

static int lc898300_vib_get_time(struct timed_output_dev *dev)
{
	struct lc898300_data *data = container_of(dev, struct lc898300_data,
							 timed_dev);
	int remain;
	if (hrtimer_active(&data->vib_timer)) {
		ktime_t r = hrtimer_get_remaining(&data->vib_timer);
		remain = (int)ktime_to_ms(r);
		return remain > 0 ? remain : 0;
	} else {
		return 0;
	}
}

static void lc898300_resume_work(struct work_struct *work)
{
	struct lc898300_data *data =
		container_of(work, struct lc898300_data, resume_work.work);

	atomic_set(&data->resuming, 0);
	wake_up(&data->resume_queue);
}

static ssize_t lc898300_intensity_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int val;

	mutex_lock(&data->lock);
	val = data->pdata->vib_cmd_info->vib_cmd_intensity;
	mutex_unlock(&data->lock);

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", val);
}

static ssize_t lc898300_intensity_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int rc;
	unsigned long val;

	rc = kstrtoul(buf, 16, &val);
	if (rc) {
		dev_err(&data->client->dev, "%s(): strtoul failed, result=%d\n",
								__func__, rc);
		return -EINVAL;
	}

	if (val > VIB_CMD_PWM_15_15)
		return -EINVAL;

	mutex_lock(&data->lock);
	data->pdata->vib_cmd_info->vib_cmd_intensity = val;
	mutex_unlock(&data->lock);

	return strnlen(buf, count);
}

static ssize_t lc898300_resonance_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int val;

	mutex_lock(&data->lock);
	val = data->pdata->vib_cmd_info->vib_cmd_resonance;
	mutex_unlock(&data->lock);

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", val);
}

static ssize_t lc898300_resonance_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int rc;
	unsigned long val;

	rc = kstrtoul(buf, 16, &val);
	if (rc) {
		dev_err(&data->client->dev, "%s(): strtoul failed, result=%d\n",
								__func__, rc);
		return -EINVAL;
	}

	if (val > VIB_CMD_FREQ_200)
		return -EINVAL;

	mutex_lock(&data->lock);
	data->pdata->vib_cmd_info->vib_cmd_resonance = val;
	mutex_unlock(&data->lock);

	return strnlen(buf, count);
}

static ssize_t lc898300_startup_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int val;

	mutex_lock(&data->lock);
	val = data->pdata->vib_cmd_info->vib_cmd_startup;
	mutex_unlock(&data->lock);

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", val);
}

static ssize_t lc898300_startup_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int rc;
	unsigned long val;

	rc = kstrtoul(buf, 16, &val);
	if (rc) {
		dev_err(&data->client->dev, "%s(): strtoul failed, result=%d\n",
								__func__, rc);
		return -EINVAL;
	}

	if (val > VIB_CMD_STTIME_7)
		return -EINVAL;

	mutex_lock(&data->lock);
	data->pdata->vib_cmd_info->vib_cmd_startup = val;
	mutex_unlock(&data->lock);

	return strnlen(buf, count);
}

static ssize_t lc898300_brake_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int val;

	mutex_lock(&data->lock);
	val = data->pdata->vib_cmd_info->vib_cmd_brake;
	mutex_unlock(&data->lock);

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", val);
}

static ssize_t lc898300_brake_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int rc;
	unsigned long val;

	rc = kstrtoul(buf, 16, &val);
	if (rc) {
		dev_err(&data->client->dev, "%s(): strtoul failed, result=%d\n",
								__func__, rc);
		return -EINVAL;
	}

	if (val > (VIB_CMD_BRPWR_15_15 | VIB_CMD_BRTIME_3 | VIB_CMD_ATBR))
		return -EINVAL;

	mutex_lock(&data->lock);
	data->pdata->vib_cmd_info->vib_cmd_brake = val;
	mutex_unlock(&data->lock);

	return strnlen(buf, count);
}

static ssize_t lc898300_stops_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int val;

	mutex_lock(&data->lock);
	val = data->pdata->vib_cmd_info->vib_cmd_stops;
	mutex_unlock(&data->lock);

	return scnprintf(buf, PAGE_SIZE, "%hhx\n", val);
}

static ssize_t lc898300_stops_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct lc898300_data *data  = dev_get_drvdata(dev);
	int rc;
	unsigned long val;

	rc = kstrtoul(buf, 16, &val);
	if (rc) {
		dev_err(&data->client->dev, "%s(): strtoul failed, result=%d\n",
								__func__, rc);
		return -EINVAL;
	}

	if (val > (VIB_CMD_ATSNUM_MASK  | VIB_CMD_ATSON) || (val & 0xf) >
							VIB_CMD_ATSNUM_10_10)
		return -EINVAL;

	mutex_lock(&data->lock);
	data->pdata->vib_cmd_info->vib_cmd_stops = val;
	mutex_unlock(&data->lock);

	return strnlen(buf, count);
}

static struct device_attribute attributes[] = {
	__ATTR(lc898300_intensity, S_IRUGO | S_IWUSR,
		lc898300_intensity_show, lc898300_intensity_store),
	__ATTR(lc898300_resonance, S_IRUGO | S_IWUSR,
		lc898300_resonance_show, lc898300_resonance_store),
	__ATTR(lc898300_startup, S_IRUGO | S_IWUSR,
		lc898300_startup_show, lc898300_startup_store),
	__ATTR(lc898300_brake, S_IRUGO | S_IWUSR,
		lc898300_brake_show, lc898300_brake_store),
	__ATTR(lc898300_stops, S_IRUGO | S_IWUSR,
		lc898300_stops_show, lc898300_stops_store),
};

static int add_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto undo;
	return 0;
undo:
	for (--i; i >= 0; i--)
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

static int __devinit lc898300_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct lc898300_data *data;
	int rc;

	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE_DATA))
		return -EIO;

	if (!client->dev.platform_data)
		return -EIO;

	data = kzalloc(sizeof(struct lc898300_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->client = client;
	data->dev    = &client->dev;
	data->pdata  = client->dev.platform_data;
	data->on = false;
	mutex_init(&data->lock);
	INIT_DELAYED_WORK(&data->resume_work, lc898300_resume_work);
	init_waitqueue_head(&data->resume_queue);
	atomic_set(&data->resuming, 1);

	i2c_set_clientdata(client, data);

	rc = data->pdata->gpio_allocate(data->dev);
	if (rc)
		goto error;
	rc = data->pdata->power_config(data->dev, true);
	if (rc)
		goto error_gpio_release;
	rc = data->pdata->power_enable(data->dev, true);
	if (rc)
		goto error_power_release;
	hrtimer_init(&data->vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->vib_timer.function = vibrator_timer_func;

	data->timed_dev.name = data->pdata->name;
	data->timed_dev.get_time = lc898300_vib_get_time;
	data->timed_dev.enable = lc898300_vib_enable;

	rc = timed_output_dev_register(&data->timed_dev);
	if (rc < 0)
		goto error_power_release;

	rc = add_sysfs_interfaces(&client->dev);
	if (rc < 0)
		goto error_add_sysfs;

	schedule_delayed_work(&data->resume_work,
		msecs_to_jiffies(LC898300_RESUME_DELAY));

	return rc;
error_add_sysfs:
	timed_output_dev_unregister(&data->timed_dev);
error_power_release:
	data->pdata->power_config(data->dev, false);
error_gpio_release:
	data->pdata->gpio_release(data->dev);
error:
	i2c_set_clientdata(client, NULL);
	kfree(data);
	return rc;
}

static int __devexit lc898300_remove(struct i2c_client *client)
{
	struct lc898300_data *data = i2c_get_clientdata(client);
	struct lc898300_platform_data *pdata = data->pdata;

	remove_sysfs_interfaces(&client->dev);
	timed_output_dev_unregister(&data->timed_dev);
	cancel_delayed_work_sync(&data->resume_work);
	hrtimer_cancel(&data->vib_timer);
	data->pdata->en_gpio_setup(false);
	data->pdata->rstb_gpio_setup(false);
	pdata->power_enable(data->dev, false);
	pdata->gpio_release(data->dev);
	kfree(data);
	i2c_set_clientdata(client, NULL);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int lc898300_pm_resume(struct device *dev)
{
	struct lc898300_data *data = dev_get_drvdata(dev);
	int rc = data->pdata->power_enable(data->dev, true);
	dev_dbg(dev, "%s\n", __func__);
	if (rc)
		dev_err(data->dev, "Failed to power on\n");
	atomic_set(&data->resuming, 1);
	schedule_delayed_work(&data->resume_work,
		msecs_to_jiffies(LC898300_RESUME_DELAY));
	return rc ? -EAGAIN : 0;
}

static int lc898300_pm_suspend(struct device *dev)
{
	struct lc898300_data *data = dev_get_drvdata(dev);
	int rc;

	dev_dbg(dev, "%s\n", __func__);
	if (mutex_trylock(&data->lock) == 0)
		return -EAGAIN;
	cancel_delayed_work_sync(&data->resume_work);
	hrtimer_cancel(&data->vib_timer);
	data->on = VIB_OFF;
	data->pdata->en_gpio_setup(false);
	(void)setup_hw(data, false);
	rc = data->pdata->power_enable(data->dev, false);
	if (rc)
		dev_err(data->dev, "Failed to power off\n");
	mutex_unlock(&data->lock);
	return rc ? -EAGAIN : 0;
}
#endif

static const struct dev_pm_ops lc898300_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(lc898300_pm_suspend, lc898300_pm_resume)
};

static struct i2c_driver lc898300_driver = {
	.driver = {
		.name   = LC898300_I2C_NAME,
		.owner = THIS_MODULE,
		.pm = &lc898300_pm_ops,
	},
	.probe  = lc898300_probe,
	.remove = __devexit_p(lc898300_remove),
	.id_table = lc898300_id,
};

static int __init lc898300_init(void)
{
	return i2c_add_driver(&lc898300_driver);
}

static void __exit lc898300_exit(void)
{
	i2c_del_driver(&lc898300_driver);
}

MODULE_AUTHOR("Ardiana Karppinen <ardiana.karppinen@sonyericsson.com>, " \
"Stefan Karlsson <stefan3.karlsson@sonyericsson.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LC898300 Linear Vibrator");

module_init(lc898300_init);
module_exit(lc898300_exit);
