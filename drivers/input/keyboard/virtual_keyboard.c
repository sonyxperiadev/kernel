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
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/input.h>
#if defined(CONFIG_RTC_DRV_KONA)
#include <linux/alarmtimer.h>
#endif

struct virtual_key {
	unsigned key;
	unsigned delay;
	struct input_dev *input;
	struct dentry *root;
#if defined(CONFIG_RTC_DRV_KONA)
	struct alarm alarm;
#endif
};

static struct virtual_key virtual_key;

#if defined(CONFIG_RTC_DRV_KONA)
static enum alarmtimer_restart
virtual_key_alarm_callback(struct alarm *alarm, ktime_t now)
{
	printk(KERN_INFO "vkey_alarm_cb key: %d, delay: %d\n",
			virtual_key.key, virtual_key.delay);

	if (unlikely(virtual_key.key) && likely(virtual_key.input)) {
		ktime_t interval = ktime_set(virtual_key.delay, 0);
		ktime_t next = ktime_add(ktime_get_real(), interval);
		alarm_start(&virtual_key.alarm, next);

		input_report_key(virtual_key.input, virtual_key.key, 1);
		input_sync(virtual_key.input);
		input_report_key(virtual_key.input, virtual_key.key, 0);
		input_sync(virtual_key.input);
	}

	return ALARMTIMER_NORESTART;
}
#endif

static int key_get(void *data, u64 *val)
{
	*val = virtual_key.key;
	return 0;
}

static int key_set(void *data, u64 val)
{
	if (!virtual_key.input)
		return 0;

	virtual_key.key = val;
	__set_bit(virtual_key.key & KEY_MAX,
			virtual_key.input->keybit);

	printk(KERN_INFO "virtual_key: %d\n", virtual_key.key);
	printk(KERN_INFO "delay: %d\n", virtual_key.delay);

#if defined(CONFIG_RTC_DRV_KONA)
	if (virtual_key.key) {
		ktime_t interval = ktime_set(virtual_key.delay, 0);
		ktime_t next = ktime_add(ktime_get_real(), interval);
		alarm_start(&virtual_key.alarm, next);
	}
#endif

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(dbg_key, key_get, key_set, "%llu\n");

static int __init virtual_key_init(void)
{
	int ret = 0;
	virtual_key.key = 0;

	virtual_key.input = input_allocate_device();
	if (!virtual_key.input)
		return -ENOMEM;

	virtual_key.input->name = "virtual_key";
	virtual_key.input->phys = "virtual_key/input0";
	set_bit(EV_KEY, virtual_key.input->evbit);
	__set_bit(KEY_POWER & KEY_MAX,
			virtual_key.input->keybit);

	ret = input_register_device(virtual_key.input);
	if (ret) {
		input_free_device(virtual_key.input);
		return ret;
	}

	virtual_key.root = debugfs_create_dir("virtual_key", 0);
	if (virtual_key.root) {
		debugfs_create_file("key", 0644,
				virtual_key.root, NULL, &dbg_key);
		debugfs_create_u32("delay", 0644,
				virtual_key.root, &virtual_key.delay);
	}

#if defined(CONFIG_RTC_DRV_KONA)
	alarm_init(&virtual_key.alarm, ALARM_REALTIME,
			virtual_key_alarm_callback);
#endif

	return 0;
}

module_init(virtual_key_init);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom Simulator Keypad Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
