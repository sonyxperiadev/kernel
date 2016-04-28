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
*
*****************************************************************************
*
*  bcm_reset_mmc_utils.c
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
#include <linux/string.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/notifier.h>
#include <linux/preempt.h>
#include <linux/completion.h>
#include <linux/bio.h>
#include <linux/mmc/core.h>
#include <plat/kona_reset_reason.h>

static int
reboot_notifier_callback(struct notifier_block *nb, unsigned long val, void *v)
{
	pr_debug("%s\n", __func__);

	if (v == NULL) {
		goto default_boot;
	}

	if (!strncmp(v, "recovery", 8)) {
		pr_info("Rebooting in recovery mode\n");
		do_set_recovery_boot();
		goto clean_up;
	}

	if (!strncmp(v, "ap_only", 7)) {
		pr_info("Rebooting with ap_only mode\n");
		do_set_ap_only_boot();
		goto clean_up;
	}

	if (!strncmp(v, "bootloader", 10)) {
		pr_info("Rebooting in bootloader mode\n");
		do_set_bootloader_boot();
		goto clean_up;
	}

default_boot:
	/* default reboot reason */
	do_set_poweron_reset_boot();

clean_up:
	return NOTIFY_DONE;
}

/* Structure for register_reboot_notifier() */
static struct notifier_block reboot_notifier = {
	.notifier_call = reboot_notifier_callback,
};

/* Init routine */
int __init bcm_reset_utils_init(void)
{
	register_reboot_notifier(&reboot_notifier);

	return 0;
}

module_init(bcm_reset_utils_init);
