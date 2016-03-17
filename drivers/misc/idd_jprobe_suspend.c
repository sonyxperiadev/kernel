/*
 * Suspend kernel probe
 *
 * Copyright (C) 2014 Sony Mobile Communications AB.
 *
 * Author: Franz Levin <franz.levin@sonymobile.com>
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
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/kprobes.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include "idd_jprobe.h"

#define SYMBOL_NAME "idd_jprobe_suspend_hook"
#define SYSFS_NAME "idd_jprobe_suspend"

/* Prototype must be the same as tracked routine has */
static void jroutine(char *buf)
{
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
MODULE_AUTHOR("Franz Levin <franz.levin@sonymobile.com>");
