/* /misc/bu520x1nvx/bu520x1nvx.c
 *
 * Copyright (C) 2017 Sony Mobile Communications Inc.
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
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
#include <linux/switch.h>
#endif
#include <linux/types.h>
#include <linux/input/bu520x1nvx.h>

#define BU520X1NVX_DEV_NAME "bu520x1nvx"
#define BU520X1NVX_SW_LID_NAME "lid"
#define BU520X1NVX_SW_KEYDOCK_NAME "keyboard_dock"
#define BU520X1NVX_SWITCH_NUM 2

#ifdef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
#define LID_DEVPATH "DEVPATH=/devices/virtual/switch/lid"
#endif

struct bu520x1nvx_event_data {
	const struct bu520x1nvx_gpio_event *event;
	struct timer_list det_timer;
	struct work_struct det_work;
	unsigned int irq;
};

struct bu520x1nvx_drvdata {
	struct device *dev;
	struct pinctrl *key_pinctrl;
	struct input_dev *input_dev;
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
	struct switch_dev switch_lid;
	struct switch_dev switch_keydock;
#else
	int input_lid_state;
#endif
	struct mutex lock;
	atomic_t detection_in_progress;
	unsigned int n_events;
	struct bu520x1nvx_event_data data[0];
};

enum bu520x1nvx_input_state {
	LID_OPEN,
	LID_CLOSE,
};

enum bu520x1nvx_switch_state {
	SWITCH_OFF,
	SWITCH_ON,
};

static int bu520x1nvx_get_lid_state(const struct bu520x1nvx_gpio_event *event)
{
	return (gpio_get_value_cansleep(event->gpio)
						  ^ event->active_low ?
						  LID_CLOSE : LID_OPEN);
}

static void bu520x1nvx_report_input_event(struct input_dev *idev,
				 const struct bu520x1nvx_gpio_event *event)
{
	int gpio_state = bu520x1nvx_get_lid_state(event);
#ifdef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
	char *input_event[3];
	char buf[32] = {0};
#endif
	dev_dbg(&idev->dev, "%s: value(%d)\n", __func__, gpio_state);
	input_report_switch(idev, SW_LID, gpio_state);
	input_sync(idev);
#ifdef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
	snprintf(buf, sizeof(buf), "SWITCH_STATE=%d", gpio_state);
	input_event[0] = buf;
	input_event[1] = LID_DEVPATH;
	input_event[2] = NULL;
	kobject_uevent_env(&idev->dev.kobj, KOBJ_CHANGE, input_event);
#endif
}

#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
static void bu520x1nvx_report_switch_event(struct switch_dev *switch_dev,
				 const struct bu520x1nvx_gpio_event *event)
{
	int gpio_state = (gpio_get_value_cansleep(event->gpio)
						  ^ event->active_low ?
						  SWITCH_ON : SWITCH_OFF);

	dev_dbg(switch_dev->dev, "%s: value(%d)\n", __func__, gpio_state);
	switch_set_state(switch_dev, gpio_state);
}
#endif

static int bu520x1nvx_get_devtree(struct device *dev,
				  struct bu520x1nvx_platform_data *pdata)
{
	struct device_node *node, *pp = NULL;
	int i = 0;
	struct bu520x1nvx_gpio_event *events;
	u32 reg;
	int gpio;
	int ret = -ENODEV;

	node = dev->of_node;
	if (node == NULL)
		goto fail;

	memset(pdata, 0, sizeof(*pdata));

	pdata->n_events = 0;
	pp = NULL;
	while ((pp = of_get_next_child(node, pp)))
		pdata->n_events++;

	if (pdata->n_events == 0)
		goto fail;

	events = kzalloc(pdata->n_events * sizeof(*events), GFP_KERNEL);
	if (!events) {
		ret = -ENOMEM;
		goto fail;
	}

	while ((pp = of_get_next_child(node, pp))) {
		enum of_gpio_flags flags;

		if (!of_find_property(pp, "gpios", NULL)) {
			pdata->n_events--;
			dev_warn(dev, "Found button without gpios\n");
			continue;
		}

		gpio = of_get_gpio_flags(pp, 0, &flags);
		if (!gpio_is_valid(gpio)) {
			dev_err(dev, "%s: invalid gpio %d\n", __func__, gpio);
			goto out_fail;
		}
		events[i].gpio = gpio;
		events[i].active_low = flags & OF_GPIO_ACTIVE_LOW;

		if (!of_property_read_u32(pp, "lid-pin", &reg))
			events[i].lid_pin = reg;

		events[i].desc = of_get_property(pp, "label", NULL);

		if (of_property_read_u32(pp, "debounce-interval", &reg) == 0)
			events[i].debounce_interval = reg;
		if (of_property_read_u32(pp, "open-debounce-interval",
					 &reg) == 0)
			events[i].open_debounce_interval = reg;
		if (of_property_read_u32(pp, "close-debounce-interval",
					 &reg) == 0)
			events[i].close_debounce_interval = reg;
		i++;
	}
	pdata->events = events;

	return 0;

out_fail:
	kfree(events);
fail:
	return ret;
}

static irqreturn_t bu520x1nvx_isr(int irq, void *data)
{
	struct bu520x1nvx_event_data *edata =
			(struct bu520x1nvx_event_data *)data;
	const struct bu520x1nvx_gpio_event *event = edata->event;
	struct bu520x1nvx_drvdata *ddata = container_of(edata,
						struct bu520x1nvx_drvdata,
						data[!event->lid_pin]);

	unsigned int timer_debounce;

	if (event->lid_pin) {
		if (bu520x1nvx_get_lid_state(event) == LID_OPEN)
			timer_debounce = event->open_debounce_interval;
		else
			timer_debounce = event->close_debounce_interval;
	} else {
		timer_debounce = event->debounce_interval;
	}

	if (timer_debounce)
		mod_timer(&edata->det_timer,
			jiffies + msecs_to_jiffies(timer_debounce));
	else
		schedule_work(&edata->det_work);

	atomic_set(&ddata->detection_in_progress, 1);

	return IRQ_HANDLED;
}

#ifndef CONFIG_HALL_SENSOR_NOT_USE_SETUP_TIMER
static void bu520x1nvx_det_tmr_func(unsigned long func_data)
{
	struct bu520x1nvx_event_data *edata =
		(struct bu520x1nvx_event_data *)func_data;

	schedule_work(&edata->det_work);
}
#else
static void bu520x1nvx_det_tmr_func(struct timer_list *t)
{
	struct bu520x1nvx_event_data *edata =
		from_timer(edata, t, det_timer);

	schedule_work(&edata->det_work);
}
#endif

static void bu520x1nvx_det_work(struct work_struct *work)
{
	struct bu520x1nvx_event_data *edata =
		container_of(work, struct bu520x1nvx_event_data, det_work);
	const struct bu520x1nvx_gpio_event *event = edata->event;
	struct bu520x1nvx_drvdata *ddata = container_of(edata,
						struct bu520x1nvx_drvdata,
						data[!event->lid_pin]);

	if (event->lid_pin) {
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
		bu520x1nvx_report_switch_event(&ddata->switch_lid, event);
#else
		ddata->input_lid_state = bu520x1nvx_get_lid_state(event);
#endif
		bu520x1nvx_report_input_event(ddata->input_dev, event);
	} else {
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
		bu520x1nvx_report_switch_event(&ddata->switch_keydock, event);
#endif
	}

	atomic_set(&ddata->detection_in_progress, 0);
}

static int bu520x1nvx_pinctrl_configure(struct bu520x1nvx_drvdata *ddata,
							bool active)
{
	struct pinctrl_state *set_state;
	int retval;

	if (active) {
		set_state =
			pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_bu520x1nvx_active");
		if (IS_ERR(set_state)) {
			dev_err(ddata->dev,
				"cannot get ts pinctrl active state\n");
			goto lookup_err;
		}
	} else {
		set_state =
			pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_bu520x1nvx_suspend");
		if (IS_ERR(set_state)) {
			dev_err(ddata->dev,
				"cannot get gpiokey pinctrl sleep state\n");
			goto lookup_err;
		}
	}
	retval = pinctrl_select_state(ddata->key_pinctrl, set_state);
	if (retval) {
		dev_err(ddata->dev,
				"cannot set ts pinctrl active state\n");
		goto select_err;
	}

	return 0;

lookup_err:
	return PTR_ERR(set_state);
select_err:
	return retval;
}

static int bu520x1nvx_open(struct input_dev *idev)
{
	struct bu520x1nvx_drvdata *ddata = input_get_drvdata(idev);
	struct bu520x1nvx_event_data *edata;
	const struct bu520x1nvx_gpio_event *event;
	int i;

	mutex_lock(&ddata->lock);
	for (i = 0; i < ddata->n_events; i++) {
		edata = &ddata->data[i];
		event = edata->event;

		if (event->lid_pin)
			bu520x1nvx_report_input_event(ddata->input_dev, event);
	}
	mutex_unlock(&ddata->lock);

	return 0;
}

static int bu520x1nvx_setup_event(struct platform_device *pdev,
				  struct bu520x1nvx_event_data *edata,
				  const struct bu520x1nvx_gpio_event *event)
{
	const char *desc = event->desc ? event->desc : BU520X1NVX_DEV_NAME;
	struct device *dev = &pdev->dev;
	irq_handler_t isr;
	unsigned long irqflags;
	int irq, error;

	edata->event = event;

	error = gpio_request(event->gpio, desc);
	if (error < 0) {
		dev_err(dev, "Failed to request GPIO %d, error %d\n",
			event->gpio, error);
		goto request_fail;
	}

	error = gpio_direction_input(event->gpio);
	if (error < 0) {
		dev_err(dev,
			"Failed to configure direction for GPIO %d, error %d\n",
			event->gpio, error);
		goto fail;
	}

	irq = gpio_to_irq(event->gpio);
	if (irq < 0) {
		error = irq;
		dev_err(dev,
			"Unable to get irq number for GPIO %d, error %d\n",
			event->gpio, error);
		goto fail;
	}
	edata->irq = irq;

	isr = bu520x1nvx_isr;
	irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;

	INIT_WORK(&edata->det_work, bu520x1nvx_det_work);
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SETUP_TIMER
	setup_timer(&edata->det_timer,
		    bu520x1nvx_det_tmr_func, (unsigned long)edata);
#else
	timer_setup(&edata->det_timer,
		    bu520x1nvx_det_tmr_func, 0);
#endif

	error = request_any_context_irq(edata->irq, isr, irqflags, desc, edata);
	if (error < 0) {
		dev_err(dev, "Unable to claim irq %d; error %d\n",
			event->irq, error);
		goto fail;
	}
	enable_irq_wake(edata->irq);

	return 0;

fail:
	if (gpio_is_valid(event->gpio))
		gpio_free(event->gpio);
request_fail:
	return error;
}

static void bu520x1nvx_remove_event(struct bu520x1nvx_event_data *edata)
{
	free_irq(edata->irq, edata);
	del_timer_sync(&edata->det_timer);
	cancel_work_sync(&edata->det_work);
	if (gpio_is_valid(edata->event->gpio))
		gpio_free(edata->event->gpio);
}

static int bu520x1nvx_set_input_device(struct bu520x1nvx_drvdata *ddata)
{
	struct input_dev *input;
	int error = 0;

	input = input_allocate_device();
	if (!input) {
		dev_err(ddata->dev, "failed to allocate input_dev (%s)\n",
			__func__);
		error = -ENOMEM;
		goto out;
	}
	ddata->input_dev = input;

	input_set_drvdata(ddata->input_dev, ddata);

	ddata->input_dev->name = BU520X1NVX_DEV_NAME;
	ddata->input_dev->open = bu520x1nvx_open;

	error = input_register_device(ddata->input_dev);
	if (error) {
		dev_err(ddata->dev, "failed to register device (%s)\n",
			__func__);
		goto fail;
	}
	input_set_capability(ddata->input_dev, EV_SW, SW_LID);

	goto out;

fail:
	input_unregister_device(ddata->input_dev);
out:
	return error;

}

#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
static int bu520x1nvx_set_switch_device(struct bu520x1nvx_drvdata *ddata,
					bool lid_pin)
{
	int error = 0;

	if (lid_pin) {
		ddata->switch_lid.name = BU520X1NVX_SW_LID_NAME;
		ddata->switch_lid.state = SWITCH_OFF;
		error = switch_dev_register(&ddata->switch_lid);
		if (error) {
			dev_err(ddata->dev, "%s cannot regist lid(%d)\n",
				__func__, error);
			goto out;
		}
	} else {
		ddata->switch_keydock.name = BU520X1NVX_SW_KEYDOCK_NAME;
		ddata->switch_keydock.state = SWITCH_OFF;
		error = switch_dev_register(&ddata->switch_keydock);
		if (error) {
			dev_err(ddata->dev, "%s cannot regist keydock(%d)\n",
				__func__, error);
			goto fail_switch;
		}
	}
	goto out;

fail_switch:
	switch_dev_unregister(&ddata->switch_lid);
out:
	return error;
}
#endif

#ifdef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
static ssize_t bu520x1nvx_show_lid_state(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct bu520x1nvx_drvdata *ddata;

	ddata = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", ddata->input_lid_state);
}

static struct device_attribute bu520x1nvx_attrs[] = {
	__ATTR(state, S_IRUGO, bu520x1nvx_show_lid_state, NULL),
};
#endif

static int bu520x1nvx_probe(struct platform_device *pdev)
{
	struct bu520x1nvx_platform_data *pdata = pdev->dev.platform_data;
	struct bu520x1nvx_platform_data alt_pdata;
	const struct bu520x1nvx_gpio_event *event;
	struct bu520x1nvx_event_data *edata;
	struct bu520x1nvx_drvdata *ddata;
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
	struct switch_dev *switch_dev;
#endif
	int i = 0;
	int error = 0;
	struct pinctrl_state *set_state;

	if (!pdata) {
		error = bu520x1nvx_get_devtree(&pdev->dev, &alt_pdata);
		if (error)
			goto fail;
		pdata = &alt_pdata;
	}

	ddata = kzalloc(sizeof(struct bu520x1nvx_drvdata) +
			pdata->n_events * sizeof(struct bu520x1nvx_event_data),
			GFP_KERNEL);
	if (!ddata) {
		dev_err(&pdev->dev, "failed to allocate drvdata in probe\n");
		error = -ENOMEM;
		goto fail;
	}

	ddata->dev = &pdev->dev;
	ddata->n_events = pdata->n_events;
	ddata->key_pinctrl = devm_pinctrl_get(ddata->dev);
	mutex_init(&ddata->lock);

	platform_set_drvdata(pdev, ddata);

	if (IS_ERR(ddata->key_pinctrl)) {
		if (PTR_ERR(ddata->key_pinctrl) == -EPROBE_DEFER)
			goto fail_pinctrl;
		pr_debug("Target does not use pinctrl\n");
		ddata->key_pinctrl = NULL;
	}

	if (ddata->key_pinctrl) {
		error = bu520x1nvx_pinctrl_configure(ddata, true);
		if (error) {
			dev_err(ddata->dev,
				"cannot set ts pinctrl active state\n");
			goto fail_pinctrl;
		}
	}

	for (i = 0; i < pdata->n_events; i++) {
		event = &pdata->events[i];
		edata = &ddata->data[i];

		if (event->lid_pin) {
			error = bu520x1nvx_set_input_device(ddata);
			if (error) {
				dev_err(ddata->dev,
					"%s cannot set input dev(%d)\n",
					__func__, error);
				goto fail_setup_event;
			}
		}
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
		error = bu520x1nvx_set_switch_device(ddata, event->lid_pin);
		if (error) {
			dev_err(ddata->dev, "%s cannot set switch dev(%d)\n",
				__func__, error);
			goto fail_setup_event;
		}
#endif
		error = bu520x1nvx_setup_event(pdev, edata, event);
		if (error) {
			dev_err(ddata->dev, "%s cannot set event error(%d)\n",
				__func__, error);
			goto fail_setup_event;
		}

#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
		if (event->lid_pin)
			switch_dev = &ddata->switch_lid;
		else
			switch_dev = &ddata->switch_keydock;

		bu520x1nvx_report_switch_event(switch_dev, event);
#endif
	}

#ifdef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
	error = device_create_file(ddata->dev, bu520x1nvx_attrs);
	if (error) {
		dev_err(&pdev->dev, "create_file failed\n");
		goto fail_setup_event;
	}
	kobject_uevent(&ddata->dev->kobj, KOBJ_CHANGE);
	ddata->input_lid_state = bu520x1nvx_get_lid_state(event);
	bu520x1nvx_report_input_event(ddata->input_dev, event);
#endif

	return 0;

fail_setup_event:
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
	if (ddata->switch_lid.dev)
		switch_dev_unregister(&ddata->switch_lid);
	if (ddata->switch_keydock.dev)
		switch_dev_unregister(&ddata->switch_keydock);
	if (&ddata->input_dev->dev) {
		input_unregister_device(ddata->input_dev);
		input_free_device(ddata->input_dev);
	}
#else
	input_unregister_device(ddata->input_dev);
	input_free_device(ddata->input_dev);
#endif
	if (ddata->key_pinctrl) {
		set_state =
		pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_bu520x1nvx_suspend");
		if (IS_ERR(set_state))
			dev_err(ddata->dev, "cannot get pinctrl sleep state\n");
		else
			pinctrl_select_state(ddata->key_pinctrl, set_state);
	}
	while (--i >= 0)
		bu520x1nvx_remove_event(&ddata->data[i]);
	platform_set_drvdata(pdev, NULL);
fail_pinctrl:
	mutex_destroy(&ddata->lock);
	kzfree(ddata);
fail:
	return error;
}

static int bu520x1nvx_remove(struct platform_device *pdev)
{
	int i;
	struct bu520x1nvx_drvdata *ddata = platform_get_drvdata(pdev);
#ifndef CONFIG_HALL_SENSOR_NOT_USE_SWITCH
	if (ddata->switch_lid.dev)
		switch_dev_unregister(&ddata->switch_lid);
	if (ddata->switch_keydock.dev)
		switch_dev_unregister(&ddata->switch_keydock);
	if (&ddata->input_dev->dev)
		input_unregister_device(ddata->input_dev);
#else
	input_unregister_device(ddata->input_dev);
#endif
	mutex_destroy(&ddata->lock);
	for (i = 0; i < ddata->n_events; i++)
		bu520x1nvx_remove_event(&ddata->data[i]);
	if (!pdev->dev.platform_data)
		kfree(ddata->data[0].event);
	kzfree(ddata);

	return 0;
}

static int bu520x1nvx_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bu520x1nvx_drvdata *ddata = platform_get_drvdata(pdev);
	int ret = 0;

	if (ddata->key_pinctrl) {
		ret = bu520x1nvx_pinctrl_configure(ddata, false);
		if (ret) {
			dev_err(&pdev->dev, "failed to put the pin\n");
			goto out;
		}
	}

	if (atomic_read(&ddata->detection_in_progress)) {
		dev_dbg(&pdev->dev, "detection in progress. (%s)\n",
			__func__);
		ret = -EAGAIN;
	}

out:
	return ret;
}

static int bu520x1nvx_resume(struct platform_device *pdev)
{
	struct bu520x1nvx_drvdata *ddata = platform_get_drvdata(pdev);
	int ret = 0;

	if (ddata->key_pinctrl) {
		ret = bu520x1nvx_pinctrl_configure(ddata, true);
		if (ret)
			dev_err(&pdev->dev, "failed to put the pin\n");
	}

	return ret;
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
	.resume = bu520x1nvx_resume,
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

MODULE_LICENSE("GPL v2");
