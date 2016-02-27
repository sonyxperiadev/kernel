/*
 * Authors: Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/ldo_vibrator.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>

enum ldo_vibrator_state {
	LDO_VIBRATOR_OFF,
	LDO_VIBRATOR_ON,
};

static void ldo_vibrator_vib_set(struct ldo_vibrator_data *data, int on)
{
	dev_dbg(data->dev, "%s vibrator set state(%d)\n", __func__, on);
	gpio_set_value(data->gpio, on);
}

static void ldo_vibrator_vib_work(struct work_struct *work)
{
	struct ldo_vibrator_data *data = container_of(work,
				struct ldo_vibrator_data, work);

	dev_dbg(data->dev, "%s vib state(%d)\n", __func__, data->state);
	ldo_vibrator_vib_set(data, data->state);
}

static enum hrtimer_restart ldo_vibrator_vib_timer(struct hrtimer *timer)
{
	struct ldo_vibrator_data *data = container_of(timer,
						      struct ldo_vibrator_data,
						      vib_timer);

	dev_dbg(data->dev, "%s: timer end\n", __func__);
	data->state = LDO_VIBRATOR_OFF;
	schedule_work(&data->work);

	return HRTIMER_NORESTART;
}

static int ldo_vibrator_vib_get_time(struct timed_output_dev *dev)
{
	struct ldo_vibrator_data *data = container_of(dev,
						      struct ldo_vibrator_data,
						      timed_dev);
	int ret = 0;

	if (hrtimer_active(&data->vib_timer)) {
		ktime_t r = hrtimer_get_remaining(&data->vib_timer);
		ret = (int)ktime_to_us(r);
	}
	return ret;
}

static void ldo_vibrator_vib_enable(struct timed_output_dev *dev, int value)
{
	struct ldo_vibrator_data *data = container_of(dev,
						      struct ldo_vibrator_data,
						      timed_dev);

	mutex_lock(&data->lock);
	hrtimer_cancel(&data->vib_timer);

	dev_dbg(data->dev, "%s: timer value(%d)\n", __func__, value);
	if (value == 0) {
		data->state = LDO_VIBRATOR_OFF;
	} else {
		data->state = LDO_VIBRATOR_ON;
		hrtimer_start(&data->vib_timer,
			      ktime_set(value / MSEC_PER_SEC,
					(value % MSEC_PER_SEC) *
					NSEC_PER_MSEC),
					HRTIMER_MODE_REL);
	}
	mutex_unlock(&data->lock);
	schedule_work(&data->work);
}

#ifdef CONFIG_PM
static int ldo_vibrator_suspend(struct device *dev)
{
	struct ldo_vibrator_data *data = dev_get_drvdata(dev);

	hrtimer_cancel(&data->vib_timer);
	cancel_work_sync(&data->work);
	/* turn-off vibrator */
	ldo_vibrator_vib_set(data, 0);

	return 0;
}

#else
#define ldo_vibrator_suspend NULL
#endif

static SIMPLE_DEV_PM_OPS(ldo_vibrator_pm_ops, ldo_vibrator_suspend, NULL);

static int ldo_vibrator_get_gpio_data(struct device *dev, int *gpio_num)
{
	struct device_node *node;
	int gpio;
	enum of_gpio_flags flags;

	node = dev->of_node;
	if (node == NULL)
		goto error;

	gpio = of_get_gpio_flags(node, 0, &flags);
	if (!gpio_is_valid(gpio)) {
		dev_err(dev, "%s: invalid gpio %d\n", __func__, gpio);
		goto error;
	}
	*gpio_num = gpio;

	return 0;
error:
	return -ENODEV;
}

static int ldo_vibrator_probe(struct platform_device *pdev)
{
	struct ldo_vibrator_data *data;
	int alt_gpio;
	int ret;

	ret = ldo_vibrator_get_gpio_data(&pdev->dev, &alt_gpio);
	if (ret)
		goto out;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		ret = -ENOMEM;
		goto out;
	}

	data->gpio = alt_gpio;
	data->dev = &pdev->dev;

	mutex_init(&data->lock);
	INIT_WORK(&data->work, ldo_vibrator_vib_work);

	hrtimer_init(&data->vib_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	data->vib_timer.function = ldo_vibrator_vib_timer;

	data->timed_dev.name = "vibrator";
	data->timed_dev.get_time = ldo_vibrator_vib_get_time;
	data->timed_dev.enable = ldo_vibrator_vib_enable;

	ret = timed_output_dev_register(&data->timed_dev);
	if (ret < 0) {
		dev_err(data->dev,
			"%s: register timed_output device failed\n", __func__);
		goto out;
	}

	dev_set_drvdata(data->dev, data);

	dev_info(data->dev, "%s: success\n", __func__);
out:
	return ret;
}

static int ldo_vibrator_remove(struct platform_device *pdev)
{
	struct ldo_vibrator_data *data = dev_get_drvdata(&pdev->dev);

	hrtimer_cancel(&data->vib_timer);
	cancel_work_sync(&data->work);
	ldo_vibrator_vib_set(data, 0);
	timed_output_dev_unregister(&data->timed_dev);
	mutex_destroy(&data->lock);

	return 0;
}

static const struct of_device_id ldo_vibrator_of_match[] = {
	{ .compatible = "ldo-vibrator", },
	{ }
};
MODULE_DEVICE_TABLE(of, ldo_vibrator_ids);

static struct platform_driver ldo_vibrator_driver = {
	.driver = {
		.name = LDO_VIBRATOR_NAME,
		.owner = THIS_MODULE,
		.pm	= &ldo_vibrator_pm_ops,
		.of_match_table = ldo_vibrator_of_match,
	},
	.probe = ldo_vibrator_probe,
	.remove = ldo_vibrator_remove,
};

static int __init ldo_vibrator_init(void)
{
	return platform_driver_register(&ldo_vibrator_driver);
}

static void __exit ldo_vibrator_exit(void)
{
	platform_driver_unregister(&ldo_vibrator_driver);
}

module_init(ldo_vibrator_init);
module_exit(ldo_vibrator_exit);

MODULE_DESCRIPTION("LDO vibrator driver");
MODULE_AUTHOR("Atsushi Iyogi");
MODULE_LICENSE("GPLV2");
