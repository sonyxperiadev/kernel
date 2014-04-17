/*
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Portions of this software are Copyright 2011 Broadcom Corporation */

#include <linux/init.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#ifdef CONFIG_ROM_SEC_DISPATCHER
#include <mach/secure_api.h>
#endif

static DEFINE_SPINLOCK(l2x0_prefetch_enable_spinlock);
static int l2x0_prefetch_enable_count;
module_param_named(l2x0_prefetch_enable, l2x0_prefetch_enable_count, int,
		   S_IRUGO | S_IWUSR | S_IWGRP);

int rhea_l2x0_prefetch(bool enable)
{
	int op, count;

	op = enable ? SEC_API_ENABLE_L2_PREFETCH : SEC_API_DISABLE_L2_PREFETCH;

	spin_lock(&l2x0_prefetch_enable_spinlock);

	if (enable)
		count = l2x0_prefetch_enable_count++;
	else
		count = --l2x0_prefetch_enable_count;

#ifdef CONFIG_ROM_SEC_DISPATCHER
	if (!count)
		hw_sec_pub_dispatcher(op, SEC_FLAGS);
#endif

	spin_unlock(&l2x0_prefetch_enable_spinlock);

	pr_info("l2x0 prefetch enable: %d %d\n", enable, count);
	return 0;
}
EXPORT_SYMBOL(rhea_l2x0_prefetch);

