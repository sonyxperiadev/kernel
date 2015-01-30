/*
 * Suspend / Resume kernel probe
 *
 * Copyright (C) 2014 Sony Mobile Communications AB.
 *
 * Author: Krzysztof Drobinski <krzysztof.drobinski@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/kprobes.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include "idd_jprobe.h"

#define SYMBOL_NAME "dpm_show_time"
#define SYSFS_NAME "idd_jprobe_pm"

static char *pm_verb(int event)
{
	switch (event) {
	case PM_EVENT_SUSPEND:
		return "suspend";
	case PM_EVENT_RESUME:
		return "resume";
	case PM_EVENT_FREEZE:
		return "freeze";
	case PM_EVENT_QUIESCE:
		return "quiesce";
	case PM_EVENT_HIBERNATE:
		return "hibernate";
	case PM_EVENT_THAW:
		return "thaw";
	case PM_EVENT_RESTORE:
		return "restore";
	case PM_EVENT_RECOVER:
		return "recover";
	default:
		return "(unknown PM event)";
	}
}

/* Prototype must be the same as tracked routine has */
static void jroutine(ktime_t starttime, pm_message_t state, char *info)
{
	u64 usecs64;
	ktime_t calltime;

	char buf[MAX_LINE_SIZE];

	calltime = ktime_get();
	usecs64 = ktime_to_ns(ktime_sub(calltime, starttime));
	scnprintf(buf, MAX_LINE_SIZE, "%s %s %llu",
		  info ?: "sys", pm_verb(state.event), usecs64);

	probe_cb_insert_data(buf); /* Don't touch this */

	jprobe_return(); /* Required by jprobe framework */
}

static int __init probe_init(void)
{
	return idd_jprobe_init(SYMBOL_NAME, jroutine, SYSFS_NAME);
}

static void __exit probe_exit(void)
{
	idd_jprobe_free();
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krzysztof Drobinski <krzysztof.drobinski@sonymobile.com>");
