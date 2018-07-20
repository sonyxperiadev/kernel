/* drivers/input/misc/s210st0x.c
 *
 * Author: Atsushi Iyogi <atsushi2.X.iyogi@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/s210st0x.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/switch.h>

#define S210ST0X_DEV_NAME "s210st0x"
#define S210ST0X_SW_LID_NAME "motion"

struct s210st0x_event_data {
	const struct s210st0x_gpio_event *event;
	unsigned int timer_debounce;
	unsigned int irq;
	struct delayed_work input_work;
	struct delayed_work det_work;
};

struct s210st0x_drvdata {
	struct device *dev;
	struct pinctrl *key_pinctrl;
	struct input_dev *input_dev;
	struct mutex lock;
	atomic_t detection_in_progress;
	unsigned int n_events;
	unsigned int current_state;
	unsigned int act_reason;
	bool input_enable;
	struct switch_dev switch_lid;

	struct s210st0x_event_data data[0];
};

enum s210st0x_input_state {
	NOT_DETECTED,
	DETECTED,
};

enum s210st0x_active_reason {
	RE_DETECTION = BIT(0),
};

static void s210st0x_input_report(struct input_dev *idev,
				  const struct s210st0x_gpio_event *event,
				  int gpio_state)
{
	struct s210st0x_drvdata *ddata = input_get_drvdata(idev);

	if (ddata->act_reason & RE_DETECTION)
		switch_set_state(&ddata->switch_lid, gpio_state);
}

static int s210st0x_get_devtree(struct device *dev,
				  struct s210st0x_platform_data *pdata)
{
	struct device_node *node, *pp = NULL;
	int i = 0;
	struct s210st0x_gpio_event *events;
	u32 reg;
	int gpio;
	int ret = -ENODEV;
	enum of_gpio_flags flags;

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
		events[i].index = i;
		events[i].gpio = gpio;
		events[i].active_low = flags & OF_GPIO_ACTIVE_LOW;

		events[i].desc = of_get_property(pp, "label", NULL);

		if (of_property_read_u32(pp, "debounce-interval", &reg) == 0)
			events[i].debounce_interval = reg;
		i++;
	}
	pdata->events = events;

	return 0;

out_fail:
	kfree(events);
fail:
	return ret;
}

static irqreturn_t s210st0x_isr(int irq, void *data)
{
	struct s210st0x_event_data *edata =
			(struct s210st0x_event_data *)data;
	const struct s210st0x_gpio_event *event = edata->event;
	struct s210st0x_drvdata *ddata = container_of(edata,
						struct s210st0x_drvdata,
						data[event->index]);
	pr_debug("%s: detection_in_progress %d\n", __func__,
		atomic_read(&ddata->detection_in_progress));

	if (!atomic_read(&ddata->detection_in_progress)) {
		pm_stay_awake(ddata->dev);
		atomic_set(&ddata->detection_in_progress, 1);
		schedule_delayed_work(&edata->input_work, 0);
	}
	return IRQ_HANDLED;
}

static void s210st0x_input_work(struct work_struct *work)
{
	struct s210st0x_event_data *edata =
		container_of(work, struct s210st0x_event_data, input_work.work);
	const struct s210st0x_gpio_event *event = edata->event;
	struct s210st0x_drvdata *ddata = container_of(edata,
						struct s210st0x_drvdata,
						data[event->index]);
	int gpio_state = !ddata->current_state;

	pr_debug("%s: detection_in_progress %d\n", __func__,
		atomic_read(&ddata->detection_in_progress));

	s210st0x_input_report(ddata->input_dev, event, gpio_state);
	if (edata->timer_debounce) {
		pr_debug("%s: schedule_delayed_work (%d)\n",
				__func__, edata->timer_debounce);
		schedule_delayed_work(&edata->det_work,
			msecs_to_jiffies(edata->timer_debounce));
	} else {
		pr_debug("%s: schedule_delayed_work now\n", __func__);
		schedule_delayed_work(&edata->det_work, 0);
	}
	ddata->current_state = gpio_state;
}

static void s210st0x_det_work(struct work_struct *work)
{
	struct s210st0x_event_data *edata =
		container_of(work, struct s210st0x_event_data, det_work.work);
	const struct s210st0x_gpio_event *event = edata->event;
	struct s210st0x_drvdata *ddata = container_of(edata,
						struct s210st0x_drvdata,
						data[event->index]);

	pr_debug("%s\n", __func__);
	atomic_set(&ddata->detection_in_progress, 0);
	pm_relax(ddata->dev);
}

static int s210st0x_pinctrl_configure(struct s210st0x_drvdata *ddata,
							bool active)
{
	struct pinctrl_state *set_state;
	int retval = 0;

	if (ddata->key_pinctrl == NULL)
		return 0;

	if (active) {
		set_state =
			pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_s210st0x_active");
		if (IS_ERR(set_state)) {
			dev_err(ddata->dev,
				"cannot get ts pinctrl active state\n");
			goto lookup_err;
		}
	} else {
		set_state =
			pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_s210st0x_suspend");
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

static int s210st0x_active_sensor(struct s210st0x_drvdata *ddata,
							unsigned int reason)
{
	int i;

	pr_debug("%s: start act_reason = 0x%02X\n",
			__func__, ddata->act_reason);
	if (ddata->act_reason)
		goto exit;

	s210st0x_pinctrl_configure(ddata, true);

	if (!atomic_read(&ddata->detection_in_progress)) {
		pm_stay_awake(ddata->dev);
		atomic_set(&ddata->detection_in_progress, 1);
		for (i = 0; i < ddata->n_events; i++) {
			if (ddata->data[i].timer_debounce) {
				pr_debug("%s: schedule_delayed_work (%d)\n",
						__func__, ddata->data[i].timer_debounce);
				schedule_delayed_work(&ddata->data[i].det_work,
					msecs_to_jiffies(ddata->data[i].timer_debounce));
			} else {
				pr_debug("%s: schedule_delayed_work now\n", __func__);
				schedule_delayed_work(&ddata->data[i].det_work, 0);
			}
		}
	}

	for (i = 0; i < ddata->n_events; i++) {
		enable_irq(ddata->data[i].irq);
		enable_irq_wake(ddata->data[i].irq);
	}
exit:
	ddata->act_reason |= reason;
	pr_debug("%s: end act_reason = 0x%02X\n",
			__func__, ddata->act_reason);
	return 0;
}

static int s210st0x_suspend_sensor(struct s210st0x_drvdata *ddata,
							unsigned int reason)
{
	int i;

	pr_debug("%s: start act_reason = 0x%02X\n",
			__func__, ddata->act_reason);
	if (!(ddata->act_reason & reason) ||
	    (ddata->act_reason & ~reason))
		goto exit;

	for (i = 0; i < ddata->n_events; i++) {
		disable_irq(ddata->data[i].irq);
		disable_irq_wake(ddata->data[i].irq);
	}
	s210st0x_pinctrl_configure(ddata, false);
exit:
	ddata->act_reason &= ~reason;
	pr_debug("%s: end act_reason = 0x%02X\n",
			__func__, ddata->act_reason);
	return 0;
}

static int s210st0x_open(struct input_dev *idev)
{
	struct s210st0x_drvdata *ddata = input_get_drvdata(idev);
	struct s210st0x_event_data *edata;
	const struct s210st0x_gpio_event *event;
	int i;

	mutex_lock(&ddata->lock);
	for (i = 0; i < ddata->n_events; i++) {
		edata = &ddata->data[i];
		event = edata->event;
	}
	mutex_unlock(&ddata->lock);

	return 0;
}

static int s210st0x_setup_event(struct platform_device *pdev,
				  struct s210st0x_event_data *edata,
				  const struct s210st0x_gpio_event *event)
{
	const char *desc = event->desc ? event->desc : S210ST0X_DEV_NAME;
	struct device *dev = &pdev->dev;
	irq_handler_t isr;
	unsigned long irqflags;
	int irq, error;

	edata->event = event;

	error = gpio_request(event->gpio, desc);
	if (error < 0)
		dev_warn(dev, "Failed to request GPIO %d, error %d\n",
			event->gpio, error);

	error = gpio_direction_input(event->gpio);
	if (error < 0) {
		dev_err(dev,
			"Failed to configure direction for GPIO %d, error %d\n",
			event->gpio, error);
		goto fail;
	}

	edata->timer_debounce = event->debounce_interval;

	irq = gpio_to_irq(event->gpio);
	if (irq < 0) {
		error = irq;
		dev_err(dev,
			"Unable to get irq number for GPIO %d, error %d\n",
			event->gpio, error);
		goto fail;
	}
	edata->irq = irq;

	isr = s210st0x_isr;
	irqflags = IRQF_TRIGGER_FALLING;

	INIT_DELAYED_WORK(&edata->det_work, s210st0x_det_work);
	INIT_DELAYED_WORK(&edata->input_work, s210st0x_input_work);

	error = request_any_context_irq(edata->irq, isr, irqflags, desc, edata);
	if (error < 0) {
		dev_err(dev, "Unable to claim irq %d; error %d\n",
			event->irq, error);
		goto fail;
	}
	disable_irq(edata->irq);
	disable_irq_wake(edata->irq);

	return 0;

fail:
	gpio_free(event->gpio);
	return error;
}

static void s210st0x_remove_event(struct s210st0x_event_data *edata)
{
	free_irq(edata->irq, edata);
	cancel_delayed_work_sync(&edata->det_work);
	if (gpio_is_valid(edata->event->gpio))
		gpio_free(edata->event->gpio);
}

static ssize_t s210st0x_attrs_current_state_read(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	struct s210st0x_drvdata *ddata = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n", ddata->current_state);
}

static ssize_t s210st0x_attrs_current_enable_read(struct device *dev,
						struct device_attribute *attr,
						char *buf)
{
	struct s210st0x_drvdata *ddata = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", ddata->input_enable);
}

static ssize_t s210st0x_attrs_current_enable_write(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct s210st0x_drvdata *ddata = dev_get_drvdata(dev);
	unsigned long state;

	if (kstrtoul(buf, 10, &state))
		return -EINVAL;

	if (ddata->input_enable != !!state) {
		pr_debug("%s change irq state=%lu \n", __func__, state);
		ddata->input_enable = !!state;
		if (state)
			s210st0x_active_sensor(ddata, RE_DETECTION);
		else
			s210st0x_suspend_sensor(ddata, RE_DETECTION);
	}
	return size;
}

static ssize_t s210st0x_attrs_active_write(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t size)
{
	struct s210st0x_drvdata *ddata = dev_get_drvdata(dev);
	unsigned long state;

	if (kstrtoul(buf, 10, &state))
		return -EINVAL;

	if (!!state) {
		pr_debug("%s: KEY_WAKEUP\n", __func__);
		input_report_key(ddata->input_dev, KEY_WAKEUP, 1);
		input_sync(ddata->input_dev);
		input_report_key(ddata->input_dev, KEY_WAKEUP, 0);
		input_sync(ddata->input_dev);
	} else {
		pr_debug("%s: KEY_SLEEP\n", __func__);
		input_report_key(ddata->input_dev, KEY_SLEEP, 1);
		input_sync(ddata->input_dev);
		input_report_key(ddata->input_dev, KEY_SLEEP, 0);
		input_sync(ddata->input_dev);
	}
	return size;
}

static struct device_attribute s210st0x_attr[] = {
	__ATTR(s210st0x_state, S_IRUGO,
		s210st0x_attrs_current_state_read,
		NULL),
	__ATTR(s210st0x_enable, S_IRUSR | S_IWUSR,
		s210st0x_attrs_current_enable_read,
		s210st0x_attrs_current_enable_write),
	__ATTR(s210st0x_active, S_IWUSR,
		NULL,
		s210st0x_attrs_active_write),
};

static int s210st0x_create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(s210st0x_attr); i++) {
		rc = device_create_file(dev, &s210st0x_attr[i]);
		if (rc < 0) {
			dev_err(dev, "%s: create_file failed %d\n",
				__func__, rc);
			goto revert;
		}
	}
	return 0;

revert:
	for (i = i - 1; i >= 0; i--)
		device_remove_file(dev, &s210st0x_attr[i]);
	return rc;
}

static void s210st0x_remove_sysfs_entries(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(s210st0x_attr); i++)
		device_remove_file(dev, &s210st0x_attr[i]);
}

static int s210st0x_set_input_device(struct s210st0x_drvdata *ddata)
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

	ddata->input_dev->name = S210ST0X_DEV_NAME;
	ddata->input_dev->open = s210st0x_open;

	error = input_register_device(ddata->input_dev);
	if (error) {
		dev_err(ddata->dev, "failed to register device (%s)\n",
			__func__);
		goto fail;
	}
	input_set_capability(ddata->input_dev, EV_KEY, KEY_WAKEUP);
	input_set_capability(ddata->input_dev, EV_KEY, KEY_SLEEP);

	goto out;

fail:
	input_unregister_device(ddata->input_dev);
out:
	return error;

}

static int s210st0x_probe(struct platform_device *pdev)
{
	struct s210st0x_platform_data *pdata = pdev->dev.platform_data;
	struct s210st0x_platform_data alt_pdata;
	const struct s210st0x_gpio_event *event;
	struct s210st0x_event_data *edata;
	struct s210st0x_drvdata *ddata;
	int i = 0;
	int error = 0;
	struct pinctrl_state *set_state;

	if (!pdata) {
		error = s210st0x_get_devtree(&pdev->dev, &alt_pdata);
		if (error)
			goto fail;
		pdata = &alt_pdata;
	}

	ddata = kzalloc(sizeof(struct s210st0x_drvdata) +
			pdata->n_events * sizeof(struct s210st0x_event_data),
			GFP_KERNEL);
	if (!ddata) {
		dev_err(&pdev->dev, "failed to allocate drvdata in probe\n");
		error = -ENOMEM;
		goto fail;
	}

	ddata->dev = &pdev->dev;
	ddata->n_events = pdata->n_events;
	ddata->act_reason = 0;
	ddata->input_enable = false;
	ddata->key_pinctrl = devm_pinctrl_get(ddata->dev);
	mutex_init(&ddata->lock);

	platform_set_drvdata(pdev, ddata);
	dev_set_drvdata(ddata->dev, ddata);

	if (IS_ERR(ddata->key_pinctrl)) {
		if (PTR_ERR(ddata->key_pinctrl) == -EPROBE_DEFER)
			goto fail_pinctrl;
		pr_debug("Target does not use pinctrl\n");
		ddata->key_pinctrl = NULL;
	}

	if (ddata->key_pinctrl) {
		error = s210st0x_pinctrl_configure(ddata, false);
		if (error) {
			dev_err(ddata->dev,
				"cannot set ts pinctrl active state\n");
			goto fail_pinctrl;
		}
	}

	error = s210st0x_set_input_device(ddata);
	if (error) {
		dev_err(ddata->dev, "%s cannot set input dev(%d)\n",
			__func__, error);
		goto fail_set_input;
	}

	error = s210st0x_create_sysfs_entries(ddata->dev);
	if (error)
		goto fail_device_create_file;

	for (i = 0; i < pdata->n_events; i++) {
		event = &pdata->events[i];
		edata = &ddata->data[i];

		error = s210st0x_setup_event(pdev, edata, event);
		if (error) {
			dev_err(ddata->dev, "%s cannot set event error(%d)\n",
				__func__, error);
			goto fail_setup_event;
		}
	}
	ddata->switch_lid.name = S210ST0X_SW_LID_NAME;
	ddata->switch_lid.state = NOT_DETECTED;
	error = switch_dev_register(&ddata->switch_lid);
	if (error) {
		dev_err(ddata->dev, "%s cannot regist lid(%d)\n",
			__func__, error);
		goto fail_setup_event;
	}

	ddata->current_state = NOT_DETECTED;

	dev_warn(ddata->dev, "s210st0x driver was successful.\n");
	return 0;

fail_setup_event:
	s210st0x_remove_sysfs_entries(ddata->dev);
fail_device_create_file:
	input_unregister_device(ddata->input_dev);
	input_free_device(ddata->input_dev);
fail_set_input:
	if (ddata->key_pinctrl) {
		set_state =
		pinctrl_lookup_state(ddata->key_pinctrl,
				     "tlmm_s210st0x_suspend");
		if (IS_ERR(set_state))
			dev_err(ddata->dev, "cannot get pinctrl sleep state\n");
		else
			pinctrl_select_state(ddata->key_pinctrl, set_state);
	}
	while (--i >= 0)
		s210st0x_remove_event(&ddata->data[i]);
	platform_set_drvdata(pdev, NULL);
fail_pinctrl:
	mutex_destroy(&ddata->lock);
	kzfree(ddata);
fail:
	return error;
}

static int s210st0x_remove(struct platform_device *pdev)
{
	int i;
	struct s210st0x_drvdata *ddata = platform_get_drvdata(pdev);

	if (ddata->switch_lid.dev)
		switch_dev_unregister(&ddata->switch_lid);
	s210st0x_remove_sysfs_entries(ddata->dev);
	input_unregister_device(ddata->input_dev);
	mutex_destroy(&ddata->lock);
	for (i = 0; i < ddata->n_events; i++)
		s210st0x_remove_event(&ddata->data[i]);
	if (!pdev->dev.platform_data)
		kfree(ddata->data[0].event);
	kzfree(ddata);

	return 0;
}

static int s210st0x_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct s210st0x_drvdata *ddata = platform_get_drvdata(pdev);
	int ret = 0;

	if (atomic_read(&ddata->detection_in_progress)) {
		dev_dbg(&pdev->dev, "detection in progress. (%s)\n",
			__func__);
		ret = -EAGAIN;
		goto out;
	}

	if (ddata->key_pinctrl) {
		ret = s210st0x_pinctrl_configure(ddata, false);
		if (ret)
			dev_err(&pdev->dev, "failed to put the pin\n");
	}

out:
	return ret;
}

static int s210st0x_resume(struct platform_device *pdev)
{
	struct s210st0x_drvdata *ddata = platform_get_drvdata(pdev);
	int ret = 0;

	if (ddata->key_pinctrl) {
		ret = s210st0x_pinctrl_configure(ddata, true);
		if (ret)
			dev_err(&pdev->dev, "failed to put the pin\n");
	}

	return ret;
}

static struct of_device_id s210st0x_match_table[] = {
	{	.compatible = "murata,s210st0x",
	},
	{}
};

static struct platform_driver s210st0x_driver = {
	.probe = s210st0x_probe,
	.remove = s210st0x_remove,
	.suspend = s210st0x_suspend,
	.resume = s210st0x_resume,
	.driver = {
		.name = S210ST0X_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = s210st0x_match_table,
	},
};

static int __init s210st0x_init(void)
{
	return platform_driver_register(&s210st0x_driver);
}

static void __exit s210st0x_exit(void)
{
	platform_driver_unregister(&s210st0x_driver);
}

module_init(s210st0x_init);
module_exit(s210st0x_exit);

MODULE_LICENSE("GPLv2");

