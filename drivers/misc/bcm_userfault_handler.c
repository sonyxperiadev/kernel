/*****************************************************************************
*  Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

/*
*****************************************************************************
*
*  bcm_userfault_handler.c
*
*  PURPOSE:
*
*     This implements the driver for the Factory Reset feature on eMMC based
*	  devices and other handlers for custom reset strings
*
*
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/debugfs.h>

#define USERFAULT_CRASH_EN	1
#define USERFAULT_CRASH_DIS	0

int bcm_debugcrash_level = USERFAULT_CRASH_DIS;
EXPORT_SYMBOL(bcm_debugcrash_level);

/*
 * This function is used to crash the kernel as well when called
 * if the debuglevel for user faults is set to 1. This would help
 * debug some of user fault issues by getting the ramdump for such faults.\
 */
static int bcm_forcecrash_set(void *data, u64 val)
{
	if (val && (bcm_debugcrash_level == USERFAULT_CRASH_EN))
		panic("Kernel panic caused by user fault");

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(bcm_forcecrash_ops, NULL, bcm_forcecrash_set,
						"%llu\n");

static int __init bcm_debuglevel_late_init(void)
{
	struct dentry *dentry;

	dentry = debugfs_create_dir("bcm_debuglevel", 0);
	if (!dentry) {
		pr_err("Failed to create the directory bcm_debuglevel\n");
		return -EPERM;
	}

	/*
	 * Setting bcm_debugcrash_level to 1 will force all user faults to
	 * generate kernel crash. The default is to crash only on kernel
	 * fault (and not crash on user faults).
	 */
	if (!debugfs_create_u32("bcm_debugcrash_level", 0644,
			dentry, (int *)&bcm_debugcrash_level)) {
		pr_err("Failed to create the file debuglevel in bcm_debugcrash_level dir\n");
		return -EPERM;
	}

	if (!debugfs_create_file("bcm_userfault_forcecrash", 0644,
			dentry, NULL, &bcm_forcecrash_ops)) {
		pr_err("Failed to create the file debuglevel in bcm_forcecrash dir\n");
		return -EPERM;
	}

	return 0;
}

late_initcall(bcm_debuglevel_late_init);

