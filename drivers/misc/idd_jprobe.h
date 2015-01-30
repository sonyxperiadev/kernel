#ifndef _IDD_JPROBE_H
#define _IDD_JPROBE_H
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
#include "idd_jprobe_sysfs.h"

static struct jprobe idd_jprobe;

static int idd_jprobe_init(char *symbol_name, void *entry, char *sysfs_name)
{
	int retval = 0;

	idd_jprobe.entry = entry;
	idd_jprobe.kp.symbol_name = symbol_name;

/* Initialize circular buffer */
	retval = probe_cb_init();
	if (retval < 0)
		return retval;
/* Sysfs interface */
	retval = jprobe_sysfs_attr_init(sysfs_name);
	if (retval)
		goto fail;
	retval = register_jprobe(&idd_jprobe);
	if (retval < 0)
		goto fail2;
	printk(KERN_INFO "Planted %s jprobe at %p, handler addr %p\n",
	       idd_jprobe.kp.symbol_name, idd_jprobe.kp.addr, idd_jprobe.entry);

	return retval;

fail2:
	jprobe_sysfs_attr_free();
fail:
	probe_cb_free();

	return retval;
}

static void idd_jprobe_free(void)
{
	jprobe_sysfs_attr_free();
	kfree(cbuf);
	unregister_jprobe(&idd_jprobe);
	printk(KERN_WARNING "idd jprobe at %p unregistered\n",
	       idd_jprobe.kp.addr);
}
#endif /* _IDD_JPROBE_H */
