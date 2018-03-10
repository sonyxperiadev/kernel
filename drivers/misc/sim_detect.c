/* drivers/misc/sim_detect.c
 *
 * Author: Atsushi Iyogi <atsushi.x.iyogi@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/sim_detect.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/cei_hw_id.h>

#define SIM_DETECT_DEV_NAME "sim_detect"

struct sim_detect_event_data {
	const struct sim_detect_gpio_event *event;
	struct timer_list det_timer;
	struct work_struct det_work;
	unsigned int timer_debounce;
	unsigned int irq;
};

struct sim_detect_drvdata {
	struct device *dev;
	struct pinctrl *key_pinctrl;
	struct switch_dev sim_detect;
	struct mutex lock;
	atomic_t detection_in_progress;
	unsigned int n_events;
	unsigned int current_state;
	struct sim_detect_event_data data[0];
};

enum sim_detect_switch_state {
	SWITCH_OFF,
	SWITCH_ON,
};

enum sim_report_state {
	NOTHING_HAPPENED = 0,
	SIM_REMOVED = 1,
	SIM_INSERTED = 2,
};

static int sim_detect_gpio_read(struct sim_detect_drvdata *ddata)
{
	int i;
	int gpio_state = 0;

	for (i = 0; i < ddata->n_events; i++)
		gpio_state +=
			(gpio_get_value_cansleep(ddata->data[i].event->gpio)
			 ^ ddata->data[i].event->active_low ?
			SWITCH_ON : SWITCH_OFF) << i;
	return gpio_state;
}

static void sim_detect_report_switch_event(struct sim_detect_drvdata *ddata)
{
	int new_state = 0;
	int report_state = NOTHING_HAPPENED;

	mutex_lock(&ddata->lock);

	new_state = sim_detect_gpio_read(ddata);
	dev_dbg(ddata->dev, "%s: current value(%d) new value(%d)\n",
		__func__, ddata->current_state, new_state);

	if (new_state == ddata->current_state)
		goto skip_report;

	if (new_state < ddata->current_state)
		report_state = SIM_REMOVED;
	else if (new_state > ddata->current_state)
		report_state = SIM_INSERTED;

	dev_info(ddata->dev, "%s: report (%d)\n", __func__, report_state);
	switch_set_state(&ddata->sim_detect, report_state);
	ddata->current_state = new_state;

skip_report:
	mutex_unlock(&ddata->lock);
}

static int sim_detect_get_devtree(struct device *dev,
				  struct sim_detect_platform_data *pdata)
{
	struct device_node *node, *pp = NULL;
	int i = 0;
	struct sim_detect_gpio_event *events;
	u32 reg;
	int gpio;
	int ret = -ENODEV;
	enum of_gpio_flags flags;
	int index;
	char *phase;

	phase = get_cei_simslot_id();
	index = get_simslot_name_index(phase);

	node = dev->of_node;
	if (node == NULL)
		goto fail;

	memset(pdata, 0, sizeof(*pdata));

	pdata->n_events = 0;
	pp = NULL;

	if (index) {
		while ((pp = of_get_next_child(node, pp)))
			pdata->n_events++;
	} else {
		if ((pp = of_get_child_by_name(node, "sim1_det")))
			pdata->n_events++;
	}

	if (pdata->n_events == 0)
		goto fail;

	events = kzalloc(pdata->n_events * sizeof(*events), GFP_KERNEL);
	if (!events) {
		ret = -ENOMEM;
		goto fail;
	}

	if (index) {
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
	} else {
		if ((pp = of_get_child_by_name(node, "sim1_det"))) {

			if (!of_find_property(pp, "gpios", NULL)) {
				pdata->n_events--;
				dev_warn(dev, "Found button without gpios\n");
			} else {
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
			}
		}
	}
	pdata->events = events;

	return 0;

out_fail:
	kfree(events);
fail:
	return ret;
}

static irqreturn_t sim_detect_isr(int irq, void *data)
{
	struct sim_detect_event_data *edata =
			(struct sim_detect_event_data *)data;
	const struct sim_detect_gpio_event *event = edata->event;
	struct sim_detect_drvdata *ddata = container_of(edata,
						struct sim_detect_drvdata,
						data[event->index]);

	if (edata->timer_debounce)
		mod_timer(&edata->det_timer,
			jiffies + msecs_to_jiffies(edata->timer_debounce));
	else
		schedule_work(&edata->det_work);

	atomic_set(&ddata->detection_in_progress, 1);

	return IRQ_HANDLED;
}

static void sim_detect_det_tmr_func(unsigned long func_data)
{
	struct sim_detect_event_data *edata =
		(struct sim_detect_event_data *)func_data;

	schedule_work(&edata->det_work);
}

static void sim_detect_det_work(struct work_struct *work)
{
	struct sim_detect_event_data *edata =
		container_of(work, struct sim_detect_event_data, det_work);
	const struct sim_detect_gpio_event *event = edata->event;
	struct sim_detect_drvdata *ddata = container_of(edata,
						struct sim_detect_drvdata,
						data[event->index]);

	sim_detect_report_switch_event(ddata);

	atomic_set(&ddata->detection_in_progress, 0);
}

static int sim_detect_pinctrl_configure(struct sim_detect_drvdata *ddata,
							bool active)
{
	struct pinctrl_state *set_state;
	int retval;

	if (active) {
		set_state =
			pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_sim_detect_active");
		if (IS_ERR(set_state)) {
			dev_err(ddata->dev,
				"cannot get ts pinctrl active state\n");
			goto lookup_err;
		}
	} else {
		set_state =
			pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_sim_detect_suspend");
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

static int sim_detect_setup_event(struct platform_device *pdev,
				  struct sim_detect_event_data *edata,
				  const struct sim_detect_gpio_event *event)
{
	const char *desc = event->desc ? event->desc : SIM_DETECT_DEV_NAME;
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

	isr = sim_detect_isr;
	irqflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
#ifdef CONFIG_TRAY_SHARED_INTERRUPT_DETECT
	irqflags |= IRQF_SHARED;
#endif

	INIT_WORK(&edata->det_work, sim_detect_det_work);

	setup_timer(&edata->det_timer,
		    sim_detect_det_tmr_func, (unsigned long)edata);

	error = request_any_context_irq(edata->irq, isr, irqflags, desc, edata);
	if (error < 0) {
		dev_err(dev, "Unable to claim irq %d; error %d\n",
			event->irq, error);
		goto fail;
	}
	enable_irq_wake(edata->irq);

	return 0;

fail:
	gpio_free(event->gpio);
	return error;
}

static void sim_detect_remove_event(struct sim_detect_event_data *edata)
{
	free_irq(edata->irq, edata);
	if (edata->timer_debounce)
		del_timer_sync(&edata->det_timer);
	cancel_work_sync(&edata->det_work);
	if (gpio_is_valid(edata->event->gpio))
		gpio_free(edata->event->gpio);
}

static int sim_detect_set_switch_device(struct sim_detect_drvdata *ddata)
{
	int error = 0;

	ddata->sim_detect.name = SIM_DETECT_DEV_NAME;
	ddata->sim_detect.state = SWITCH_OFF;
	error = switch_dev_register(&ddata->sim_detect);
	if (error) {
		dev_err(ddata->dev, "%s cannot regist lid(%d)\n",
			__func__, error);
	}
	return error;
}

static ssize_t sim_attrs_current_state_read(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	struct sim_detect_drvdata *ddata = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n", ddata->current_state);
}

static struct device_attribute sim_state_attr[] = {
	__ATTR(sim_state, S_IRUGO, sim_attrs_current_state_read, NULL),
};

static int sim_detect_probe(struct platform_device *pdev)
{
	struct sim_detect_platform_data *pdata = pdev->dev.platform_data;
	struct sim_detect_platform_data alt_pdata;
	const struct sim_detect_gpio_event *event;
	struct sim_detect_event_data *edata;
	struct sim_detect_drvdata *ddata;
	int i = 0;
	int error = 0;
	struct pinctrl_state *set_state;

	if (!pdata) {
		error = sim_detect_get_devtree(&pdev->dev, &alt_pdata);
		if (error)
			goto fail;
		pdata = &alt_pdata;
	}

	ddata = kzalloc(sizeof(struct sim_detect_drvdata) +
			pdata->n_events * sizeof(struct sim_detect_event_data),
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
	dev_set_drvdata(ddata->dev, ddata);

	if (IS_ERR(ddata->key_pinctrl)) {
		if (PTR_ERR(ddata->key_pinctrl) == -EPROBE_DEFER)
			goto fail_pinctrl;
		pr_debug("Target does not use pinctrl\n");
		ddata->key_pinctrl = NULL;
	}

	if (ddata->key_pinctrl) {
		error = sim_detect_pinctrl_configure(ddata, true);
		if (error) {
			dev_err(ddata->dev,
				"cannot set ts pinctrl active state\n");
			goto fail_pinctrl;
		}
	}

	error = sim_detect_set_switch_device(ddata);
	if (error) {
		dev_err(ddata->dev, "%s cannot set switch dev(%d)\n",
			__func__, error);
		goto fail_set_switch;
	}

	error = device_create_file(ddata->dev, sim_state_attr);
	if (error) {
		dev_err(ddata->dev, "%s: create_file failed %d\n",
			__func__, error);
		goto fail_device_create_file;
	}

	for (i = 0; i < pdata->n_events; i++) {
		event = &pdata->events[i];
		edata = &ddata->data[i];

		error = sim_detect_setup_event(pdev, edata, event);
		if (error) {
			dev_err(ddata->dev, "%s cannot set event error(%d)\n",
				__func__, error);
			goto fail_setup_event;
		}
	}
	ddata->current_state = sim_detect_gpio_read(ddata);

	dev_warn(ddata->dev, "sim_detect driver was successful.\n");
	return 0;

fail_setup_event:
	device_remove_file(ddata->dev, sim_state_attr);
fail_device_create_file:
	switch_dev_unregister(&ddata->sim_detect);
fail_set_switch:
	if (ddata->key_pinctrl) {
		set_state =
		pinctrl_lookup_state(ddata->key_pinctrl,
						"tlmm_sim_detect_suspend");
		if (IS_ERR(set_state))
			dev_err(ddata->dev, "cannot get pinctrl sleep state\n");
		else
			pinctrl_select_state(ddata->key_pinctrl, set_state);
	}
	while (--i >= 0)
		sim_detect_remove_event(&ddata->data[i]);
	platform_set_drvdata(pdev, NULL);
fail_pinctrl:
	mutex_destroy(&ddata->lock);
	kzfree(ddata);
fail:
	return error;
}

static int sim_detect_remove(struct platform_device *pdev)
{
	int i;
	struct sim_detect_drvdata *ddata = platform_get_drvdata(pdev);

	device_remove_file(ddata->dev, sim_state_attr);
	switch_dev_unregister(&ddata->sim_detect);
	mutex_destroy(&ddata->lock);
	for (i = 0; i < ddata->n_events; i++)
		sim_detect_remove_event(&ddata->data[i]);
	if (!pdev->dev.platform_data)
		kfree(ddata->data[0].event);
	kzfree(ddata);

	return 0;
}

static int sim_detect_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct sim_detect_drvdata *ddata = platform_get_drvdata(pdev);
	int ret = 0;

	if (atomic_read(&ddata->detection_in_progress)) {
		dev_dbg(&pdev->dev, "detection in progress. (%s)\n",
			__func__);
		ret = -EAGAIN;
		goto out;
	}

	if (ddata->key_pinctrl) {
		ret = sim_detect_pinctrl_configure(ddata, false);
		if (ret)
			dev_err(&pdev->dev, "failed to put the pin\n");
	}

out:
	return ret;
}

static int sim_detect_resume(struct platform_device *pdev)
{
	struct sim_detect_drvdata *ddata = platform_get_drvdata(pdev);
	int ret = 0;

	if (ddata->key_pinctrl) {
		ret = sim_detect_pinctrl_configure(ddata, true);
		if (ret)
			dev_err(&pdev->dev, "failed to put the pin\n");
	}

	return ret;
}

static struct of_device_id sim_detect_match_table[] = {
	{	.compatible = "sim-detect",
	},
	{}
};

static struct platform_driver sim_detect_driver = {
	.probe = sim_detect_probe,
	.remove = sim_detect_remove,
	.suspend = sim_detect_suspend,
	.resume = sim_detect_resume,
	.driver = {
		.name = SIM_DETECT_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sim_detect_match_table,
	},
};

static int __init sim_detect_init(void)
{
	return platform_driver_register(&sim_detect_driver);
}

static void __exit sim_detect_exit(void)
{
	platform_driver_unregister(&sim_detect_driver);
}

late_initcall(sim_detect_init);
module_exit(sim_detect_exit);

MODULE_LICENSE("GPLv2");
