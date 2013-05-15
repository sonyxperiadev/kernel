/*
 * bmg160.c -  Linux kernel driver for BOSCH bmg160 gyroscope
 *
 * Copyright (c) 2012 Sony Mobile Communications AB
 *
 * Authors: Stefan Karlsson <stefan3.karlsson@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */
#include <linux/bmg160.h>
#include <linux/ctype.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spi.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#define BMG160_VENDORID                  0x0001
#define AUTOSUSPEND_MS                   110
#define INITIAL_RATE_MS                  200
#define MIN_RATE                         1
#define MAX_RATE                         10000

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("bmg160");

struct bmg160_data {
	struct input_dev            *ip_dev;
	struct i2c_client           *ic_dev;
	struct bmg160_platform_data *pdata;
	u8                           chip_id;
	unsigned char                shift;
	atomic_t                     rate;
	struct workqueue_struct	    *workqueue;
	struct delayed_work          work;
	bool                         powered;
};

static int bmg160_ic_read(struct i2c_client *ic_dev, u8 reg, u8 *buf, int len)
{
	int rc;

	rc = i2c_smbus_read_i2c_block_data(ic_dev, reg, len, buf);
	return rc == len ? 0 : rc < 0 ? rc : -ENXIO;
}

static inline int bmg160_ic_write(struct i2c_client *ic_dev, u8 reg, u8 val)
{
	return i2c_smbus_write_byte_data(ic_dev, reg, val);
}

static inline u8 bmg160_range2shift(u8 range)
{
	switch (range) {
	case BMG160_RANGE_2000:
		return 4;
	case BMG160_RANGE_1000:
		return 3;
	case BMG160_RANGE_500:
		return 2;
	case BMG160_RANGE_250:
		return 1;
	case BMG160_RANGE_125:
	default:
		return 0;
	}
}

static inline void bmg160_suspend_sleep(struct bmg160_data *bd)
{
	usleep_range(450, 700);
}

static inline void bmg160_reset_sleep(struct bmg160_data *bd)
{
	msleep(30);
}

static int bmg160_power_down(struct bmg160_data *bd)
{
	int rc = 0;

	if (bd->powered) {
		rc = bmg160_ic_write(bd->ic_dev, BMG160_MODE_CTRL_REG,
					BMG160_MODE_DEEPSUSPEND);
		if (rc)
			dev_err(&bd->ic_dev->dev, "%s: Failed to suspend "
						"chip\n", __func__);
		if (bd->pdata->power_down)
			if (bd->pdata->power_down(&bd->ic_dev->dev))
				dev_err(&bd->ic_dev->dev, "%s: Failed to power "
						"down externally\n", __func__);
		bd->powered = false;
	}
	return rc;
}

static int bmg160_power_up(struct bmg160_data *bd)
{
	int rc;

	if (!bd->powered) {
		if (bd->pdata->power_up) {
			rc = bd->pdata->power_up(&bd->ic_dev->dev);
			bmg160_suspend_sleep(bd);
			if (!rc)
				bd->powered = true;
			else
				dev_err(&bd->ic_dev->dev, "%s: Failed to power"
						" up externally\n", __func__);
		} else {
			bd->powered = true;
		}
	}
	rc = bmg160_ic_write(bd->ic_dev, BMG160_RESET_REG,
						BMG160_RESET);
	if (rc) {
		dev_err(&bd->ic_dev->dev, "%s: Failed to reset chip\n",
							 __func__);
		return rc;
	}
	bmg160_reset_sleep(bd);
	return 0;
}

static int bmg160_read_chip_id(struct bmg160_data *bd)
{
	int rc;

	rc = bmg160_ic_read(bd->ic_dev, BMG160_CHIP_ID_REG,
						&bd->chip_id, 1);
	if (rc)
		dev_err(&bd->ic_dev->dev, "%s: unable to read chip id\n",
								__func__);
	else
		dev_info(&bd->ic_dev->dev, "bmg160: detected chip id %d\n",
							bd->chip_id);
	return rc;
}


static inline int bmg160_report_data(struct bmg160_data *bd)
{
	int                         rc;
	u8                          rx_buf[7];
	short		            *rate = (short *)&rx_buf;
	int                         len = 7;
	int x, y, z;

	rc = bmg160_ic_read(bd->ic_dev, BMG160_X_AXIS_LSB_REG, rx_buf, len);
	if (rc) {
		dev_err(&bd->ic_dev->dev,
			"%s: device failed, error %d\n", __func__, rc);
		return rc;
	}

	x = (short)le16_to_cpu(rate[0]);
	y = (short)le16_to_cpu(rate[1]);
	z = (short)le16_to_cpu(rate[2]);

	input_report_abs(bd->ip_dev, ABS_X, x << bd->shift);
	input_report_abs(bd->ip_dev, ABS_Y, y << bd->shift);
	input_report_abs(bd->ip_dev, ABS_Z, z << bd->shift);
	input_report_abs(bd->ip_dev, ABS_MISC, (signed char)rx_buf[6] + 24*2);
	input_sync(bd->ip_dev);
	return 0;
}

static int bmg160_write_settings(struct bmg160_data *bd)
{
	int rc;
	rc = bmg160_ic_write(bd->ic_dev, BMG160_RANGE_REG, bd->pdata->range
							& BMG160_RANGE_MASK);
	if (rc)
		goto error;
	rc = bmg160_ic_write(bd->ic_dev, BMG160_UR_BW_SEL_REG,
						BMG160_UR_BW_100HZ_32HZ);
	if (rc)
		goto error;

	return rc;
error:
	dev_err(&bd->ic_dev->dev, "%s: device failed, error %d\n",
							__func__, rc);
	return rc;
}

static void work_f(struct work_struct *work)
{
	struct bmg160_data *bd =
		container_of(work, struct bmg160_data, work.work);
	int rc;

	queue_delayed_work(bd->workqueue, &bd->work,
			msecs_to_jiffies(atomic_read(&bd->rate)));
	pm_runtime_mark_last_busy(&bd->ic_dev->dev);
	rc = pm_runtime_get_sync(&bd->ic_dev->dev);
	if (rc < 0) {
		dev_err(&bd->ic_dev->dev, "%s: Failed to runtime resume; "
				"rc = %d\n", __func__, rc);
		pm_runtime_set_suspended(&bd->ic_dev->dev);
		pm_runtime_put_noidle(&bd->ic_dev->dev);
		return;
	}

	bmg160_report_data(bd);
	pm_runtime_put_autosuspend(&bd->ic_dev->dev);
}

static ssize_t bmg160_rate_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&bd->rate));
}

static ssize_t bmg160_rate_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int rc = 0;
	struct bmg160_data *bd = dev_get_drvdata(dev);
	unsigned long val;

	rc = strict_strtoul(buf, 10, &val);
	if (rc)
		return rc;
	if (val < MIN_RATE || val > MAX_RATE)
		return -EINVAL;

	atomic_set(&bd->rate, val);
	return count;
}

static ssize_t bmg160_chip_id_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", bd->chip_id);
}

static struct device_attribute attributes[] = {
	__ATTR(bmg160_rate, 0644, bmg160_rate_show, bmg160_rate_store),
	__ATTR(bmg160_chip_id, 0444, bmg160_chip_id_show, NULL),
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

static int bmg160_runtime_resume(struct device *dev)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	int rc;

	rc = bmg160_power_up(bd);
	if (rc) {
		dev_err(dev,
			"%s: failed to power up; error %d\n", __func__, rc);
		return -EAGAIN;
	}
	rc = bmg160_write_settings(bd);
	if (rc)
		return -EAGAIN;
	msleep(50);
	return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int bmg160_runtime_suspend(struct device *dev)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	int rc;

	rc = bmg160_power_down(bd);
	if (rc)
		dev_err(dev,
			"%s: failed to power down; error %d\n", __func__, rc);
	return 0;
}
#endif

static int bmg160_open(struct input_dev *dev)
{
	struct bmg160_data *bd = input_get_drvdata(dev);

	queue_delayed_work(bd->workqueue, &bd->work,
		msecs_to_jiffies(atomic_read(&bd->rate)));
	return 0;
}

static void bmg160_release(struct input_dev *dev)
{
	struct bmg160_data *bd = input_get_drvdata(dev);

	cancel_delayed_work_sync(&bd->work);
}

#ifdef CONFIG_PM_SLEEP
static int bmg160_resume(struct device *dev)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);

	bmg160_runtime_resume(dev);

	if (bd->ip_dev->users)
		queue_delayed_work(bd->workqueue, &bd->work,
			msecs_to_jiffies(atomic_read(&bd->rate)));
	return 0;
}

static int bmg160_suspend(struct device *dev)
{
	struct bmg160_data *bd = dev_get_drvdata(dev);
	int rc;

	cancel_delayed_work_sync(&bd->work);
	rc = bmg160_power_down(bd);
	return 0;
}
#endif

static int __devinit bmg160_probe(struct i2c_client *ic_dev,
		const struct i2c_device_id *id)
{
	struct bmg160_data *bd;
	int                 rc;
	struct bmg160_platform_data *pdata = ic_dev->dev.platform_data;

	if (!pdata) {
		rc = -EINVAL;
		goto probe_exit;
	}

	bd = kzalloc(sizeof(struct bmg160_data), GFP_KERNEL);
	if (!bd) {
		rc = -ENOMEM;
		goto probe_exit;
	}

	bd->ic_dev = ic_dev;
	bd->pdata = pdata;
	bd->shift = bmg160_range2shift(bd->pdata->range);
	bd->powered = false;
	INIT_DELAYED_WORK(&bd->work, work_f);
	/* initial configuration */
	atomic_set(&bd->rate, INITIAL_RATE_MS);

	i2c_set_clientdata(ic_dev, bd);

	if (bd->pdata->setup) {
		rc = bd->pdata->setup(&bd->ic_dev->dev);
		if (rc) {
			dev_err(&bd->ic_dev->dev,
				"%s: failed to setup in probe"
				" with error %d\n", __func__, rc);
			goto probe_err_setup;
		}
	}

	pm_runtime_set_active(&bd->ic_dev->dev);
	pm_runtime_enable(&bd->ic_dev->dev);

	rc = bmg160_runtime_resume(&bd->ic_dev->dev);
	if (rc) {
		dev_err(&bd->ic_dev->dev, "%s: failed to power up chip;"
					" rc = %d\n", __func__, rc);
		bmg160_power_down(bd);
		goto probe_err_config;
	}

	rc = bmg160_read_chip_id(bd);
	if (rc) {
		dev_err(&bd->ic_dev->dev, "%s: failed to read chip id;"
					" rc = %d\n", __func__, rc);
		bmg160_power_down(bd);
		goto probe_err_config;
	}

	bd->workqueue = create_singlethread_workqueue(dev_name(
							&bd->ic_dev->dev));
	if (!bd->workqueue) {
		dev_err(&bd->ic_dev->dev, "%s, failed to allocate workqueue\n",
								__func__);
		rc = -ENOMEM;
		goto probe_err_config;
	}

	bd->ip_dev = input_allocate_device();
	if (!bd->ip_dev) {
		rc = -ENOMEM;
		goto probe_err_reg;
	}
	input_set_drvdata(bd->ip_dev, bd);
	bd->ip_dev->open       = bmg160_open;
	bd->ip_dev->close      = bmg160_release;
	bd->ip_dev->name       = BMG160_NAME;
	bd->ip_dev->id.vendor  = BMG160_VENDORID;
	bd->ip_dev->id.product = 1;
	bd->ip_dev->id.version = 1;
	__set_bit(EV_ABS,       bd->ip_dev->evbit);
	__set_bit(ABS_X,        bd->ip_dev->absbit);
	__set_bit(ABS_Y,        bd->ip_dev->absbit);
	__set_bit(ABS_Z,        bd->ip_dev->absbit);
	__set_bit(ABS_MISC,     bd->ip_dev->absbit);

	input_set_abs_params(bd->ip_dev, ABS_X, -1048576, 1048575, 0, 0);
	input_set_abs_params(bd->ip_dev, ABS_Y, -1048576, 1048575, 0, 0);
	input_set_abs_params(bd->ip_dev, ABS_Z, -1048576, 1048575, 0, 0);
	input_set_abs_params(bd->ip_dev, ABS_MISC, -80, 175, 0, 0);

	rc = input_register_device(bd->ip_dev);
	if (rc) {
		input_free_device(bd->ip_dev);
		goto probe_err_reg;
	}

	rc = add_sysfs_interfaces(&bd->ic_dev->dev);
	if (rc)
		goto probe_err_sysfs;

	pm_runtime_set_autosuspend_delay(&bd->ic_dev->dev, AUTOSUSPEND_MS);
	pm_runtime_use_autosuspend(&bd->ic_dev->dev);
	pm_runtime_mark_last_busy(&bd->ic_dev->dev);

	return rc;
probe_err_sysfs:
	input_unregister_device(bd->ip_dev);
probe_err_reg:
	destroy_workqueue(bd->workqueue);
probe_err_config:
	pm_runtime_disable(&bd->ic_dev->dev);
	if (bd->pdata->teardown)
		bd->pdata->teardown(&bd->ic_dev->dev);
probe_err_setup:
	i2c_set_clientdata(ic_dev, NULL);
	kfree(bd);
probe_exit:
	return rc;
}

static int __devexit bmg160_remove(struct i2c_client *ic_dev)
{
	struct bmg160_data *bd;

	bd = i2c_get_clientdata(ic_dev);

	cancel_delayed_work_sync(&bd->work);
	remove_sysfs_interfaces(&bd->ic_dev->dev);
	input_unregister_device(bd->ip_dev);
	destroy_workqueue(bd->workqueue);

	if (!pm_runtime_suspended(&bd->ip_dev->dev))
		bmg160_power_down(bd);
	pm_runtime_disable(&bd->ip_dev->dev);

	if (bd->pdata->teardown)
		bd->pdata->teardown(&ic_dev->dev);
	i2c_set_clientdata(ic_dev, NULL);
	kfree(bd);
	return 0;
}

static const struct i2c_device_id bmg160_i2c_id[] = {
	{BMG160_NAME, 0},
	{}
};

static const struct dev_pm_ops bmg160_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(bmg160_suspend, bmg160_resume)
	SET_RUNTIME_PM_OPS(bmg160_runtime_suspend, bmg160_runtime_resume, NULL)
};

static struct i2c_driver bmg160_driver = {
	.driver = {
		.name  = BMG160_NAME,
		.owner = THIS_MODULE,
		.pm	= &bmg160_pm_ops,
	},
	.probe         = bmg160_probe,
	.remove        = __devexit_p(bmg160_remove),
	.id_table      = bmg160_i2c_id,
};

static int __init bmg160_init(void)
{
	return i2c_add_driver(&bmg160_driver);
}
module_init(bmg160_init);

static void __exit bmg160_exit(void)
{
	i2c_del_driver(&bmg160_driver);
}
module_exit(bmg160_exit);
