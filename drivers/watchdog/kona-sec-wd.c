/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file drivers/watchdog/kona-sec-wd.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
/**
 * @file
 * driver/watchdog/kona-sec-wd.c
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <mach/sec_api.h>

#define SEC_EXIT_NORMAL			1
#define SEC_WD_TAP_DELAY		120000
#define SSAPI_ACTIVATE_SEC_WATCHDOG     0x01000006
#define SSAPI_ENABLE_SEC_WATCHDOG       0x01000007
#define SSAPI_DISABLE_SEC_WATCHDOG      0x01000008
#define SSAPI_PAT_SEC_WATCHDOG          0x01000009


static struct delayed_work sec_wd_work;

static void sec_wd_pat(struct work_struct *work)
{
	printk(KERN_ALERT "patting watchdog\n");
	secure_api_call(SSAPI_PAT_SEC_WATCHDOG, 0, 0, 0, 0);
	schedule_delayed_work(&sec_wd_work,
	msecs_to_jiffies(SEC_WD_TAP_DELAY));
}

static void init_patter(void)
{
	INIT_DELAYED_WORK(&sec_wd_work, sec_wd_pat);
	schedule_delayed_work(&sec_wd_work,
	msecs_to_jiffies(SEC_WD_TAP_DELAY));
}

void sec_wd_enable()
{
	secure_api_call(SSAPI_ENABLE_SEC_WATCHDOG, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sec_wd_enable);

void sec_wd_disable()
{
	secure_api_call(SSAPI_DISABLE_SEC_WATCHDOG, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sec_wd_disable);

static int __init sec_wd_init(void)
{
	printk(KERN_ALERT "initializing secure watchdog");
	secure_api_call(SSAPI_ACTIVATE_SEC_WATCHDOG, 0, 0, 0, 0);
	init_patter();
	return 0;
}
module_init(sec_wd_init);

static void __exit sec_wd_exit(void)
{
	printk(KERN_ALERT "inside sec_wd_exit");
}
module_exit(sec_wd_exit);

MODULE_AUTHOR("Oza");
MODULE_DESCRIPTION("Secure Watchdog");
MODULE_LICENSE("GPL");

