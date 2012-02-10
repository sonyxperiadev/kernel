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
* This file adds a panic notifier task and is run when there is a system panic.
* It sends a message to CP to crash it if CP has not crashed yet.
*/

#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/proc_fs.h>
#include <linux/broadcom/ipcinterface.h>
#include "cp_crash.h"

#define BCMLOG_OUTDEV_PANIC	1

/* Prototypes */
static int do_cp_crash(struct notifier_block *, unsigned long, void *);

static int do_cp_crash(struct notifier_block *this, unsigned long event,
		       void *ptr)
{
	int ipc_state;

	if (BCMLOG_OUTDEV_PANIC != BCMLOG_GetCpCrashLogDevice())
		goto out;

#ifdef CONFIG_PREEMPT
	/* Ensure that cond_resched() won't try to preempt anybody */
	add_preempt_count(PREEMPT_ACTIVE);
#endif
	ipcs_get_ipc_state(&ipc_state);

	if (!ipc_state)
		goto out;

	if (!cp_crashed)
		IPCCP_SetCPCrashedStatus(IPC_AP_ASSERT);

	ProcessCPCrashedDump(NULL);

out:
#ifdef CONFIG_PREEMPT
	sub_preempt_count(PREEMPT_ACTIVE);
#endif

	return NOTIFY_DONE;
}

static struct notifier_block cp_crash_blk = {
	.notifier_call = do_cp_crash
};

static int __init cp_crash_init(void)
{
	int ret = 0;
	atomic_notifier_chain_register(&panic_notifier_list, &cp_crash_blk);

	printk(KERN_INFO "CP crash driver initialized!\n");
	return ret;
}

late_initcall(cp_crash_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
