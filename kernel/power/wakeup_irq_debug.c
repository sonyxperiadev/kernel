/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
/*
 * Copyright 2020 Sony Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/suspend.h>
#include <linux/platform_device.h>
#include <linux/percpu.h>
#include <linux/slab.h>
#include <linux/irq.h>

#define MODULE_NAME "wakeup_debug"

static unsigned int wakeup_irq_stat(unsigned int irq)
{
	struct irq_desc *desc;
	unsigned int sum = 0;
	int cpu;

	desc = irq_to_desc(irq);
	if (desc && desc->kstat_irqs) {
		for_each_possible_cpu(cpu)
			sum += *per_cpu_ptr(desc->kstat_irqs, cpu);
	}

	return sum;
}

struct irq_info {
	struct list_head list;
	unsigned int wakeup_count;
	unsigned int irq_count;
	char irq_name[128];
	int irq;
};

struct wakeup_irq {
	struct list_head head;
	bool enabled;
};

static int update_irq_table(struct wakeup_irq *p, int irq)
{
	struct irq_info *i;

	list_for_each_entry(i, &p->head, list) {
		if (i->irq == irq) {
			i->irq_count = wakeup_irq_stat(i->irq);
			return 1;
		}
	}

	return 0;
}

static void add_irq_to_table(struct wakeup_irq *p, int irq)
{
	struct irq_info *info;
	struct irq_desc *desc;

	/*
	 * TODO: add here a filter for root IRQs
	 */
	info = kzalloc(sizeof(*info), GFP_KERNEL);
	info->irq_count = wakeup_irq_stat(irq);
	info->irq = irq;

	desc = irq_to_desc(irq);

	/*
	 * IRQ can be disabled and freed, thus
	 * we save its name because of history
	 */
	if (desc && desc->action && desc->action->name)
		(void) scnprintf(
			info->irq_name, sizeof(info->irq_name),
			"%s", desc->action->name);

	list_add_tail(&info->list, &p->head);
}

static int wakeup_irq_suspend(struct device *dev)
{
	struct irq_desc *desc;
	struct wakeup_irq *p;
	int irq, rv;

	/* get private data */
	p = dev_get_drvdata(dev);
	if (!p->enabled)
		goto leave;

	/*
	 * we want to add IRQs to list by reverse way,
	 * therefore all potential triggered IRQ (nested)
	 * will be in the beginning of the list
	 */
	for_each_irq_desc_reverse(irq, desc) {
		raw_spin_lock_irq(&desc->lock);
		rv = irqd_is_wakeup_set(&desc->irq_data);
		raw_spin_unlock_irq(&desc->lock);

		if (rv == 0)
			continue;

		rv = update_irq_table(p, irq);
		if (rv)
			continue;

		add_irq_to_table(p, irq);
		dev_dbg(dev, "IRQ %d was added to the list\n", irq);
	}

leave:
	return 0;
}

static int wakeup_irq_resume(struct device *dev)
{
	struct irq_info *i;
	struct wakeup_irq *p;
	struct irq_desc *desc;
	unsigned long flags;
	int diff;

	/* get private data */
	p = dev_get_drvdata(dev);
	if (!p->enabled)
		goto leave;

	/*
	 * go through all registered IRQs in our list
	 */
	list_for_each_entry(i, &p->head, list) {
		desc = irq_to_desc(i->irq);
		if (desc) {
			raw_spin_lock_irqsave(&desc->lock, flags);
			diff = wakeup_irq_stat(i->irq) - i->irq_count;
			raw_spin_unlock_irqrestore(&desc->lock, flags);

			if (diff > 0) {
				i->wakeup_count++;

				dev_dbg(dev, "%d, wake-up count: %d, active: %d (%s)\n",
					i->irq, i->wakeup_count,
					irqd_is_wakeup_set(&desc->irq_data),
					desc->action && desc->action->name ?
					desc->action->name : "");

				dev_info(dev, "wakeup caused by IRQ %d, %s\n",
					i->irq, desc->action && desc->action->name ?
					desc->action->name : "");
				update_irq_table(p, i->irq);
			}
		}
	}

leave:
	return 0;
}

static ssize_t irq_stat_show(struct device *dev,
			  struct device_attribute *attr,
			  char *buf)
{
	struct wakeup_irq *p;
	struct irq_info *i;
	int len = 0;

	p = dev_get_drvdata(dev);

	/* build header of the output */
	len += scnprintf(buf + len, PAGE_SIZE - len,
		"IRQ\tCount\tName\n");

	list_for_each_entry(i, &p->head, list) {
		if (i->wakeup_count) {
			len += scnprintf(buf + len, PAGE_SIZE - len,
				"%d\t%d\t%s\n",
				i->irq, i->wakeup_count, i->irq_name);
			/*
			 * Ensure we do not write more than PAGE_SIZE
			 * bytes of data including null terminator and
			 * add '\n' symbol to the buffer if it's full
			 */
			if (len >= PAGE_SIZE - 1) {
				buf[PAGE_SIZE - 2] = '\n';
				dev_err(dev, "%s:%d: buffer is full\n",
					__func__, __LINE__);
				break;
			}
		}
	}

	return len;
}

static ssize_t enable_show(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct wakeup_irq *p;

	p = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", p->enabled);
}

static ssize_t enable_store(struct device *dev,
			 struct device_attribute *attr,
			 const char *buf, size_t count)
{
	struct wakeup_irq *p;
	int value, ret;

	p = dev_get_drvdata(dev);

	ret = kstrtoint(buf, 10, &value);
	if (ret) {
		dev_err(dev, "kstrtoint failed: %d\n", ret);
		goto error;
	}

	if (value == 1)
		p->enabled = true;
	else if (value == 0)
		p->enabled = false;
	else
		goto error;

	return count;

error:
	return -EINVAL;
}

static const struct dev_pm_ops wakeup_irq_pm_ops = {
	.suspend_noirq = wakeup_irq_suspend,
	.resume_early = wakeup_irq_resume,
};

static struct platform_device wakeup_irq_device = {
	.name = MODULE_NAME,
	.id = 0,
};

static struct platform_driver wakeup_irq_driver = {
	.driver = {
		.name = MODULE_NAME,
		.owner = THIS_MODULE,
		.pm = &wakeup_irq_pm_ops,
	},
};

static struct device_attribute wakeup_irq_attrs[] = {
	__ATTR(wakeup_irq_stat,
		S_IRUSR | S_IRGRP | S_IROTH, irq_stat_show, NULL),
	__ATTR(enable,
		S_IRUSR | S_IWUSR, enable_show, enable_store),
};

static int create_sysfs_entries(struct device *dev)
{
	int i, rc;

	for (i = 0; i < ARRAY_SIZE(wakeup_irq_attrs); i++) {
		rc = device_create_file(dev, &wakeup_irq_attrs[i]);
		if (rc < 0)
			goto revert;
	}

	return 0;

revert:
	for (; --i >= 0;)
		device_remove_file(dev, &wakeup_irq_attrs[i]);

	return rc;
}

static int wakeup_irq_debug_init(void)
{
	struct wakeup_irq *p;
	int rv = -1;
	p = kzalloc(sizeof(struct wakeup_irq), GFP_KERNEL);
	if (p == NULL)
		goto out;

	INIT_LIST_HEAD(&p->head);

	/*
	 * we need it for PM works correctly
	 */
	rv = platform_device_register(&wakeup_irq_device);
	if (rv) {
		pr_err("%s: wakeup_irq_device register failed %d\n",
			   __func__, rv);
		goto free_mem;
	}

	rv = platform_driver_register(&wakeup_irq_driver);
	if (rv) {
		pr_err("%s: wakeup_irq_driver register failed %d\n",
			   __func__, rv);
		goto unregister_device;
	}

	rv = create_sysfs_entries(&wakeup_irq_device.dev);
	if (rv < 0) {
		pr_err("%s: create_sysfs_entries failed %d\n",
			   __func__, rv);
		goto unregister_driver;
	}

	/*
	 * save private data
	 */
	platform_set_drvdata(&wakeup_irq_device, p);

	/*
	 * by default it's disabled
	 */
	p->enabled = false;
	return 0;

unregister_driver:
	platform_driver_unregister(&wakeup_irq_driver);
unregister_device:
	platform_device_unregister(&wakeup_irq_device);
free_mem:
	kfree_sensitive(p);
out:
	printk("wakeup_irq_debug_init return error\n");
	return rv;
}

static void wakeup_irq_debug_exit(void)
{
	struct irq_info *info, *tmp_info;
	struct wakeup_irq *p;
	int i;

	p = dev_get_drvdata(&wakeup_irq_device.dev);

	for (i = 0; i < ARRAY_SIZE(wakeup_irq_attrs); i++)
		device_remove_file(&wakeup_irq_device.dev,
			&wakeup_irq_attrs[i]);

	list_for_each_entry_safe(info, tmp_info, &p->head, list) {
		list_del(&info->list);
		kfree_sensitive(info);
	}

	platform_driver_unregister(&wakeup_irq_driver);
	platform_device_unregister(&wakeup_irq_device);
	kfree_sensitive(p);
}

module_init(wakeup_irq_debug_init);
module_exit(wakeup_irq_debug_exit);

MODULE_DESCRIPTION("Wakeup IRQ Debug Module");
MODULE_AUTHOR("Uladzislau Rezki");
MODULE_LICENSE("GPL v2");
