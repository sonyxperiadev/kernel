/* drivers/input/misc/bu52031nvx.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Author: Takashi Shiina <takashi.shiina@sonymobile.com>
 *         Tadashi Kubo <tadashi.kubo@sonymobile.com>
 *         Shogo Tanaka <shogo.tanaka@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/bu52031nvx.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/types.h>

#define DETECTION_DELAY 50
#define DETECTION_CYCLES 4

#define DETECT_WORK_DELAY	\
(jiffies + msecs_to_jiffies(DETECTION_DELAY))

struct bu52031nvx_drvdata {
	struct input_dev *input_dev;
	struct bu52031nvx_platform_data *pdata;

	atomic_t detect_cycle;
	atomic_t detection_in_progress;

	int current_state;

	struct timer_list det_timer;
	struct work_struct det_work;
};

static irqreturn_t bu52031nvx_isr(int irq, void *data)
{
	struct bu52031nvx_drvdata *ddata = (struct bu52031nvx_drvdata *)data;

	atomic_set(&ddata->detect_cycle, 0);
	atomic_set(&ddata->detection_in_progress, 1);
	mod_timer(&ddata->det_timer, DETECT_WORK_DELAY);

	return IRQ_HANDLED;
}

static void bu52031nvx_det_tmr_func(unsigned long func_data)
{
	struct bu52031nvx_drvdata *ddata =
		(struct bu52031nvx_drvdata *)func_data;

	schedule_work(&ddata->det_work);
}

static void bu52031nvx_det_work(struct work_struct *work)
{
	struct bu52031nvx_drvdata *ddata =
		container_of(work, struct bu52031nvx_drvdata, det_work);
	struct input_dev *idev = ddata->input_dev;
	int gpio_state = gpio_get_value(ddata->pdata->gpio_num);

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

static int bu52031nvx_open(struct input_dev *idev)
{
	struct bu52031nvx_drvdata *ddata = input_get_drvdata(idev);

	atomic_set(&ddata->detect_cycle, 0);
	atomic_set(&ddata->detection_in_progress, 1);
	mod_timer(&ddata->det_timer, DETECT_WORK_DELAY);

	return 0;
}

static int bu52031nvx_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bu52031nvx_drvdata *ddata;
	struct input_dev *input;
	int error = 0;

	ddata = kzalloc(sizeof(struct bu52031nvx_drvdata), GFP_KERNEL);
	if (!ddata) {
		dev_err(dev, "failed to allocate drvdata (%s)\n", __func__);
		error = -ENOMEM;
		goto fail_kzalloc;
	}

	ddata->pdata = pdev->dev.platform_data;

	error = ddata->pdata->gpio_setup(dev, 1);
	if (error < 0) {
		dev_err(dev, "failed to setup gpio pin (%s)\n", __func__);
		goto fail_gpio_setup;
	}

	input = input_allocate_device();
	if (!input) {
		dev_err(dev, "failed to allocate input_dev (%s)\n", __func__);
		error = -ENOMEM;
		goto fail_input_alloc;
	}

	ddata->input_dev = input;

	init_timer(&ddata->det_timer);
	ddata->det_timer.function = bu52031nvx_det_tmr_func;
	ddata->det_timer.data = (unsigned long)ddata;

	platform_set_drvdata(pdev, ddata);

	INIT_WORK(&ddata->det_work, bu52031nvx_det_work);

	input->name = BU52031NVX_DEV_NAME;
	input->open = bu52031nvx_open;

	input_set_capability(input, EV_SW, SW_LID);
	input_set_drvdata(input, ddata);

	error = input_register_device(input);
	if (error) {
		dev_err(dev, "failed to register device (%s)\n", __func__);
		goto fail_input_register;
	}

	error = request_irq(gpio_to_irq(ddata->pdata->gpio_num),
			bu52031nvx_isr,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			BU52031NVX_DEV_NAME, ddata);

	if (error) {
		dev_err(dev, "failed to request irq (%s)\n", __func__);
		goto fail_request_irq;
	}
	enable_irq_wake(gpio_to_irq(ddata->pdata->gpio_num));

	return 0;

fail_request_irq:
	input_unregister_device(input);
	goto fail_input_alloc;
fail_input_register:
	input_free_device(input);
fail_input_alloc:
	ddata->pdata->gpio_setup(dev, 0);
fail_gpio_setup:
	kfree(ddata);
fail_kzalloc:

	return error;
}

static int bu52031nvx_remove(struct platform_device *pdev)
{
	struct bu52031nvx_drvdata *ddata = platform_get_drvdata(pdev);

	input_unregister_device(ddata->input_dev);

	free_irq(gpio_to_irq(ddata->pdata->gpio_num), ddata);

	ddata->pdata->gpio_setup(&pdev->dev, 0);

	kfree(ddata);

	return 0;
}

static int bu52031nvx_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bu52031nvx_drvdata *ddata = platform_get_drvdata(pdev);

	if (atomic_read(&ddata->detection_in_progress)) {
		dev_dbg(&pdev->dev, "Lid detection in progress. (%s)\n",
			__func__);
		return -EAGAIN;
	}

	return 0;
}

static struct platform_driver bu52031nvx_driver = {
	.probe = bu52031nvx_probe,
	.remove = bu52031nvx_remove,
	.suspend = bu52031nvx_suspend,
	.driver = {
		.name = BU52031NVX_DEV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init bu52031nvx_init(void)
{
	return platform_driver_register(&bu52031nvx_driver);
}

static void __exit bu52031nvx_exit(void)
{
	platform_driver_unregister(&bu52031nvx_driver);
}

module_init(bu52031nvx_init);
module_exit(bu52031nvx_exit);

MODULE_LICENSE("GPL");

