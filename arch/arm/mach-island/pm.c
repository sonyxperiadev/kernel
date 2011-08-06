/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/pm.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/suspend.h>
#include <linux/sysfs.h>
#include <linux/module.h>

/*
 * Prepare suspend
 */
static int suspend_prepare(void)
{
	return 0;
}

/*
 * Enter the system sleep state
 */
static int suspend_enter(suspend_state_t state)
{
	int ret = 0;
	switch (state) {
		case PM_SUSPEND_STANDBY:
			printk("%s: Entering SUSPEND_STANDBY mode\n", __func__);
			/* Flow through to suspend to memory */
		case PM_SUSPEND_MEM:
			printk("%s: Entering SUSPEND_MEM mode\n", __func__);
			printk(KERN_INFO "%s: Entering run_from_ARAM()\n", __func__);

			printk("%s: Leaving SUSPEND_STANDBY mode\n", __func__);
			break;
	}
	return ret;
}

/*
 * Called when the system has just left a sleep state
 */
static void suspend_finish(void)
{
	return;
}

static struct platform_suspend_ops suspend_ops ={
#ifndef CONFIG_MAP_ISLAND_SUSPEND_INVALID
	.valid = suspend_valid_only_mem,
#endif
	.prepare = suspend_prepare,
	.enter = suspend_enter,
	.finish = suspend_finish,
};

int __init pm_init(void)
{
	printk("%s: Initializing Power Management ....\n", __func__);
	suspend_set_ops(&suspend_ops);

	return 0;
}
__initcall(pm_init);
