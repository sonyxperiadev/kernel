/* drivers/input/misc/bu520x1nvx.c
 *
 * Copyright (C) 2013-2014 Sony Mobile Communications AB.
 *
 * Author: Takashi Shiina <takashi.shiina@sonymobile.com>
 *         Tadashi Kubo <tadashi.kubo@sonymobile.com>
 *         Shogo Tanaka <shogo.tanaka@sonymobile.com>
 *         Atsushi Iyogi <atsushi.x.iyogi@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/types.h>

#define BU520X1NVX_DEV_NAME "bu520x1nvx"
#define BU520X1NVX_SWITCH_NAME "lid"

#define DETECTION_DELAY 50
#define DETECTION_CYCLES 4

#define DETECT_WORK_DELAY	\
(jiffies + msecs_to_jiffies(DETECTION_DELAY))

struct bu520x1nvx_drvdata {
	struct input_dev *input_dev;
	struct switch_dev switch_dev;
	int gpio_num;

	atomic_t detect_cycle;
	atomic_t detection_in_progress;
	struct mutex lock;

	int current_state;

	struct timer_list det_timer;
	struct work_struct det_work;
};

enum bu520x1nvx_switch_state {
	LID_OPEN,
	LID_CLOSE,
};

static irqreturn_t bu520x1nvx_isr(int irq, void *data)
{
	struct bu520x1nvx_drvdata *ddata = (struct bu520x1nvx_drvdata *)data;

	atomic_set(&ddata->detect_cycle, 0);
	atomic_set(&ddata->detection_in_progress, 1);

	mutex_lock(&ddata->lock);
	switch_set_state(&ddata->switch_dev,
		gpio_get_value(ddata->gpio_num) ? LID_OPEN : LID_CLOSE);
	mutex_unlock(&ddata->lock);

	mod_timer(&ddata->det_timer, DETECT_WORK_DELAY);

	return IRQ_HANDLED;
}

static void bu520x1nvx_det_tmr_func(unsigned long func_data)
{
	struct bu520x1nvx_drvdata *ddata =
		(struct bu520x1nvx_drvdata *)func_data;

	schedule_work(&ddata->det_work);
}

static void bu520x1nvx_det_work(struct work_struct *work)
{
	struct bu520x1nvx_drvdata *ddata =
		container_of(work, struct bu520x1nvx_drvdata, det_work);
	struct input_dev *idev = ddata->input_dev;
	int gpio_state = gpio_get_value(ddata->gpio_num);

	if (ddata->current_state == gpio_state) {
		atomic_inc(&ddata->detect_cycle);
	} else {
		atomic_set(&ddata->detect_cycle, 0);
		ddata->current_state = gpio_state;
	}

	if (DETECTION_CYCLES > atomic_read(&ddata->detect_cycle)) {
		mod_timer(&ddata->det_timer, DETECT_WORK_DELAY);
	} else {
		input_report_switch(idev, SW_LID, gpio_state ? 0 : 1);
		input_sync(idev);
		atomic_set(&ddata->detect_cycle, 0);
		atomic_set(&ddata->detection_in_progress, 0);
	}
}

static int bu520x1nvx_open(struct input_dev *idev)
{
	struct bu520x1nvx_drvdata *ddata = input_get_drvdata(idev);

	atomic_set(&ddata->detect_cycle, 0);
	atomic_set(&ddata->detection_in_progress, 1);
	mod_timer(&ddata->det_timer, DETECT_WORK_DELAY);

	mutex_lock(&ddata->lock);
	switch_set_state(&ddata->switch_dev,
		gpio_get_value(ddata->gpio_num) ? LID_OPEN : LID_CLOSE);
	mutex_unlock(&ddata->lock);

	return 0;
}

static int bu520x1nvx_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bu520x1nvx_drvdata *ddata;
	struct input_dev *input;
	int error = 0;
	int gpio;
	unsigned int flags;
	struct device_node *of_node = pdev->dev.of_node;

	ddata = kzalloc(sizeof(struct bu520x1nvx_drvdata), GFP_KERNEL);
	if (!ddata) {
		dev_err(dev, "failed to allocate drvdata (%s)\n", __func__);
		error = -ENOMEM;
		goto fail_kzalloc;
	}

	gpio = of_get_gpio_flags(of_node, 0, &flags);
	if (!gpio_is_valid(gpio)) {
		dev_err(dev, "%s: invalid gpio %d\n", __func__, gpio);
		error = -EINVAL;
		goto fail_gpio_setup;
	}
	ddata->gpio_num = gpio;

	error = gpio_request(ddata->gpio_num, "BU520X1NVX_IRQ");
	if (error < 0) {
		dev_err(dev, "failed to setup gpio (%s)\n",
			"BU520X1NVX_IRQ");
		error = -EINVAL;
		goto fail_gpio_setup;
	}

	mutex_init(&ddata->lock);

	input = input_allocate_device();
	if (!input) {
		dev_err(dev, "failed to allocate input_dev (%s)\n", __func__);
		error = -ENOMEM;
		goto fail_input_alloc;
	}

	ddata->input_dev = input;

	init_timer(&ddata->det_timer);
	ddata->det_timer.function = bu520x1nvx_det_tmr_func;
	ddata->det_timer.data = (unsigned long)ddata;

	platform_set_drvdata(pdev, ddata);

	INIT_WORK(&ddata->det_work, bu520x1nvx_det_work);

	input->name = BU520X1NVX_DEV_NAME;
	input->open = bu520x1nvx_open;

	input_set_capability(input, EV_SW, SW_LID);
	input_set_drvdata(input, ddata);

	error = input_register_device(input);
	if (error) {
		dev_err(dev, "failed to register device (%s)\n", __func__);
		goto fail_input_register;
	}

	ddata->switch_dev.name = BU520X1NVX_SWITCH_NAME;
	ddata->switch_dev.state = LID_OPEN;

	error = switch_dev_register(&ddata->switch_dev);
	if (0 > error)
		goto fail_switch_reg;

	error = request_threaded_irq(gpio_to_irq(ddata->gpio_num),
			NULL,
			bu520x1nvx_isr,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			BU520X1NVX_DEV_NAME, ddata);

	if (error) {
		dev_err(dev, "failed to request irq (%s)\n", __func__);
		goto fail_request_irq;
	}
	enable_irq_wake(gpio_to_irq(ddata->gpio_num));

	return 0;

fail_request_irq:
	switch_dev_unregister(&ddata->switch_dev);
fail_switch_reg:
	input_unregister_device(input);
	goto fail_input_alloc;
fail_input_register:
	input_free_device(input);
fail_input_alloc:
	mutex_destroy(&ddata->lock);
	gpio_free(ddata->gpio_num);
fail_gpio_setup:
	kzfree(ddata);
fail_kzalloc:

	return error;
}

static int bu520x1nvx_remove(struct platform_device *pdev)
{
	struct bu520x1nvx_drvdata *ddata = platform_get_drvdata(pdev);

	switch_dev_unregister(&ddata->switch_dev);

	input_unregister_device(ddata->input_dev);

	mutex_destroy(&ddata->lock);

	free_irq(gpio_to_irq(ddata->gpio_num), ddata);

	gpio_free(ddata->gpio_num);

	kzfree(ddata);

	return 0;
}

static int bu520x1nvx_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bu520x1nvx_drvdata *ddata = platform_get_drvdata(pdev);

	if (atomic_read(&ddata->detection_in_progress)) {
		dev_dbg(&pdev->dev, "Lid detection in progress. (%s)\n",
			__func__);
		return -EAGAIN;
	}

	return 0;
}

static struct of_device_id bu520x1nvx_match_table[] = {
	{	.compatible = "rohm,bu520x1nvx",
	},
	{}
};

static struct platform_driver bu520x1nvx_driver = {
	.probe = bu520x1nvx_probe,
	.remove = bu520x1nvx_remove,
	.suspend = bu520x1nvx_suspend,
	.driver = {
		.name = BU520X1NVX_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = bu520x1nvx_match_table,
	},
};

static int __init bu520x1nvx_init(void)
{
	return platform_driver_register(&bu520x1nvx_driver);
}

static void __exit bu520x1nvx_exit(void)
{
	platform_driver_unregister(&bu520x1nvx_driver);
}

module_init(bu520x1nvx_init);
module_exit(bu520x1nvx_exit);

MODULE_LICENSE("GPLv2");

