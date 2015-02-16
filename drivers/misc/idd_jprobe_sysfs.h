#ifndef _IDD_JPROBE_SYSFS_H
#define _IDD_JPROBE_SYSFS_H
/*
 * Low memory killer kernel probe
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

#include "circular_buffer.h"

static struct kobject *jprobe_kobj;

static ssize_t jprobe_attr_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	unsigned long flags = 0;
	int retval = 0;
	size_t i;
	size_t current_line;

	spin_lock_irqsave(&cbuf_lock, flags);
	current_line = atomic_read(&cb_idx) % MAX_LINES;
	for (i = 0; i < MAX_LINES; i++)
		if (retval < PAGE_SIZE)
			retval += scnprintf(buf + retval, PAGE_SIZE - retval,
					    "%s", cbuf[(MAX_LINES +
							current_line - i) %
						       MAX_LINES]);
		else {
			pr_warn("%s: cbuf to big! Truncating buffer!\n",
			       __func__);
			break;
		}
	spin_unlock_irqrestore(&cbuf_lock, flags);

	return retval;
}

static struct kobj_attribute jprobe_attribute =
	__ATTR(probe, 0444, jprobe_attr_show, NULL);

static struct attribute *attrs[] = {
	&jprobe_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int jprobe_sysfs_attr_init(char *name)
{
	int retval = 0;

	jprobe_kobj = kobject_create_and_add(name, kernel_kobj);
	if (!jprobe_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(jprobe_kobj, &attr_group);
	if (retval)
		kobject_put(jprobe_kobj);

	return retval;
};

static void jprobe_sysfs_attr_free(void)
{
	kobject_put(jprobe_kobj);
};

#endif /* _IDD_JPROBE_SYSFS_H */
