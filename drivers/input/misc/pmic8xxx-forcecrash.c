/* drivers/input/misc/pmic8xxx-forcecrash.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications Japan.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/wakelock.h>
#include <linux/mfd/pm8xxx/core.h>
#include <linux/input/pmic8xxx-pwrkey.h>

static struct timer_list forcecrash_timer;
static struct wake_lock wakelock;
static struct device *dev;
static bool forcecrash_on;
static int key_press_irq;

#define FORCE_CRASH_TIMEOUT 10

static void forcecrash_timeout(unsigned long data)
{
	int status = pm8xxx_read_irq_stat(dev->parent, key_press_irq);

	if (forcecrash_on && (status == 1))
		panic("Force crash triggered!!!\n");
	else {
		del_timer(&forcecrash_timer);
		wake_unlock(&wakelock);
	}
}

static ssize_t pmic8xxx_forcecrash_on_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", (unsigned int)forcecrash_on);
}

static ssize_t pmic8xxx_forcecrash_on_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	unsigned long i = 0;
	int rc;

	rc = strict_strtoul(buf, 10, &i);
	if (rc)
		return -EINVAL;

	i = !!i;

	forcecrash_on = i;
	return count;
}

static DEVICE_ATTR(forcecrash_on, S_IRUGO|S_IWUSR, pmic8xxx_forcecrash_on_show,
			pmic8xxx_forcecrash_on_store);

void pmic8xxx_forcecrash_timer_setup(bool key_pressed)
{
	if (!forcecrash_on)
		return;

	if (key_pressed) {
		mod_timer(&forcecrash_timer,
				jiffies + FORCE_CRASH_TIMEOUT * HZ);
		wake_lock(&wakelock);
	} else {
		del_timer(&forcecrash_timer);
		wake_unlock(&wakelock);
	}
}
EXPORT_SYMBOL(pmic8xxx_forcecrash_timer_setup);

int pmic8xxx_forcecrash_init(struct platform_device *pdev)
{
	int ret;
	key_press_irq = platform_get_irq(pdev, 1);
	dev = &pdev->dev;
	init_timer(&forcecrash_timer);
	forcecrash_timer.function = forcecrash_timeout;
	wake_lock_init(&wakelock, WAKE_LOCK_SUSPEND, PM8XXX_PWRKEY_DEV_NAME);
	ret = device_create_file(&pdev->dev, &dev_attr_forcecrash_on);
	return ret;
}
EXPORT_SYMBOL(pmic8xxx_forcecrash_init);

void pmic8xxx_forcecrash_exit(struct platform_device *pdev)
{
	wake_lock_destroy(&wakelock);
	del_timer(&forcecrash_timer);
	device_remove_file(&pdev->dev, &dev_attr_forcecrash_on);
}
EXPORT_SYMBOL(pmic8xxx_forcecrash_exit);
