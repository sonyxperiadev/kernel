/*
 * Copyright 2012 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php, or by writing to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/leds.h>
#include <linux/earlysuspend.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include "leds.h"

#define DEFAULT_DURATION 3000	/*ms*/

struct kpbl_trig_data {
	struct delayed_work del_work;
	struct led_classdev *led_cdev;
	struct list_head list;
	unsigned int duration;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend suspend;
#endif
};

static DEFINE_MUTEX(kpbl_lock);
static LIST_HEAD(kpbl_trigs);
static pregister_cb tp_key_register;
static pregister_cb tp_key_unregister;

static void kpbl_trig_work(struct work_struct *work);
#ifdef CONFIG_HAS_EARLYSUSPEND
static void kpbl_trig_early_suspend(struct early_suspend *h);
static void kpbl_trig_late_resume(struct early_suspend *h);
#endif

static ssize_t kpbl_duration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct kpbl_trig_data *data = led_cdev->trigger_data;

	return sprintf(buf, "%u\n",
			(data) ? data->duration : 0);
}

static ssize_t kpbl_duration_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct kpbl_trig_data *data = led_cdev->trigger_data;

	if (data)
		sscanf(buf, "%u", &data->duration);

	return size;
}

static DEVICE_ATTR(duration, 0644,\
	kpbl_duration_show, kpbl_duration_store);

static void kpbl_trig_activate(struct led_classdev *led_cdev)
{
	struct kpbl_trig_data *data;

	mutex_lock(&kpbl_lock);
	data = kzalloc(sizeof(struct kpbl_trig_data), GFP_KERNEL);
	if (!data) {
		mutex_unlock(&kpbl_lock);
		return;
	}

	data->led_cdev = led_cdev;
	data->duration = DEFAULT_DURATION;
	INIT_DELAYED_WORK(&data->del_work, kpbl_trig_work);

	led_cdev->trigger_data = data;
	device_create_file(led_cdev->dev, &dev_attr_duration);
#ifdef CONFIG_HAS_EARLYSUSPEND
	data->suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	data->suspend.suspend = kpbl_trig_early_suspend;
	data->suspend.resume = kpbl_trig_late_resume;
	register_early_suspend(&data->suspend);
#endif
	list_add_tail(&data->list, &kpbl_trigs);
	mutex_unlock(&kpbl_lock);
}

static void kpbl_trig_deactivate(struct led_classdev *led_cdev)
{
	struct kpbl_trig_data *data = led_cdev->trigger_data;

	if (data) {
		mutex_lock(&kpbl_lock);
		list_del(&data->list);
		led_cdev->trigger_data = NULL;
#ifdef CONFIG_HAS_EARLYSUSPEND
		unregister_early_suspend(&data->suspend);
#endif
		cancel_delayed_work_sync(&data->del_work);
		device_remove_file(led_cdev->dev, &dev_attr_duration);
		kfree(data);
		mutex_unlock(&kpbl_lock);
	}
}

static struct led_trigger kpbl_led_trigger = {
	.name     = "kpbl",
	.activate = kpbl_trig_activate,
	.deactivate = kpbl_trig_deactivate,
};

int kpbl_active(struct notifier_block *n, unsigned long dat, void *val)
{
	struct kpbl_trig_data *data;

	mutex_lock(&kpbl_lock);
	list_for_each_entry(data, &kpbl_trigs, list) {
		led_trigger_event(&kpbl_led_trigger, LED_FULL);
		schedule_delayed_work(&data->del_work,
			msecs_to_jiffies(data->duration));
	}
	mutex_unlock(&kpbl_lock);
	return 0;
}

static struct notifier_block notify = {
	.notifier_call = kpbl_active,
};

static void kpbl_trig_work(struct work_struct *work)
{
	struct kpbl_trig_data *data;

	data = container_of((struct delayed_work *)work,\
		struct kpbl_trig_data, del_work);
	led_set_brightness(data->led_cdev, LED_OFF);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void kpbl_trig_early_suspend(struct early_suspend *h)
{
	struct kpbl_trig_data *data;

	data = container_of(h, struct kpbl_trig_data, suspend);
	cancel_delayed_work_sync(&data->del_work);
	led_set_brightness(data->led_cdev, LED_OFF);
}

static void kpbl_trig_late_resume(struct early_suspend *h)
{
	struct kpbl_trig_data *data;

	data = container_of(h, struct kpbl_trig_data, suspend);
	led_set_brightness(data->led_cdev, LED_FULL);
	schedule_delayed_work(&data->del_work,
				msecs_to_jiffies(data->duration));
}
#endif

void led_kpbl_register(pregister_cb pcallback)
{
	tp_key_register = pcallback;
}
void led_kpbl_unregister(pregister_cb pcallback)
{
	tp_key_unregister = pcallback;
}

static int __init kpbl_trig_init(void)
{
	int ret;

	ret = led_trigger_register(&kpbl_led_trigger);
	if (ret)
		return ret;
	if (tp_key_register)
		return (*tp_key_register)(&notify);
}

static void __exit kpbl_trig_exit(void)
{
	if (tp_key_unregister)
		(*tp_key_unregister)(&notify);
	led_trigger_unregister(&kpbl_led_trigger);
}

module_init(kpbl_trig_init);
module_exit(kpbl_trig_exit);

MODULE_DESCRIPTION("BCM kpbl LED trigger");
MODULE_LICENSE("GPL");
