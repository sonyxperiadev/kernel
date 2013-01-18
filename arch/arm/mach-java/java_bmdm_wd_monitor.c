/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/timer.h>
#include <linux/notifier.h>
#include <linux/sysrq.h>

/* Delay of 4sec after the WD interrupt*/
#define BMDM_WD_MN_TIMEOUT		(HZ * 4)
/* This prio should be greater than the
 * notifier prio of other panic notifier
 * handlers, for e.g. ramdump, which
 * does an arch reset in panic notifier
 * handler.*/
#define BMDM_WD_MN_PANIC_NOTIFY_PRIO	(3)

static int bmdm_wd_mn_panic_handler(struct notifier_block *nb,
				unsigned long l, void *buf);
static irqreturn_t bmdm_wd_mn_handler(int irq, void *data);
static void bmdm_wd_mn_timeout(unsigned long arg);

#ifndef CONFIG_MAGIC_SYSRQ
static void bmdm_ap_crash(void);
#endif

static struct notifier_block bmdm_wd_mn_panic = {
	.notifier_call = bmdm_wd_mn_panic_handler,
	.priority = BMDM_WD_MN_PANIC_NOTIFY_PRIO,
};

struct timer_list bmdm_wd_timer;
static int ap_panic;

static int bmdm_wd_mn_panic_handler(struct notifier_block *nb,
			unsigned long l, void *buf)
{
	ap_panic = 1;
	return 0;
}

static void bmdm_wd_mn_timeout(unsigned long arg)
{
	del_timer(&bmdm_wd_timer);

	if (!ap_panic) {
		pr_info("************************************\n");
		pr_info("BMDM WD TIMER TRIGGERED, CRASHING AP\n");
		pr_info("************************************\n");
#ifdef CONFIG_MAGIC_SYSRQ
		handle_sysrq('c');
#else
		bmdm_ap_crash();
#endif
	} else {
		pr_info("************************************************\n");
		pr_info("BMDM WD TIMER TRIGGERED, NO ACTION AS AP CRASHED\n");
		pr_info("************************************************\n");
	}
}

#ifndef CONFIG_MAGIC_SYSRQ
static void bmdm_ap_crash(void)
{
	char *crash = NULL;

	panic_on_oops = 1;
	wmb();
	*crash = 1;
}
#endif

static irqreturn_t bmdm_wd_mn_handler(int irq, void *data)
{
	disable_irq_nosync(BCM_INT_ID_RESERVED160);
	pr_info("RECEIVED CP WATCHDOG INTERRUPT\n");
	pr_info("STARTING COUNTDOWN TIMER\n");
	/* Start the countdown timer */
	init_timer(&bmdm_wd_timer);
	bmdm_wd_timer.function = bmdm_wd_mn_timeout;
	bmdm_wd_timer.data = (unsigned long)NULL;
	bmdm_wd_timer.expires = jiffies + BMDM_WD_MN_TIMEOUT;
	add_timer(&bmdm_wd_timer);

	return IRQ_HANDLED;
}

static int __init bmdm_wd_mn_init(void)
{
	if (request_irq(BCM_INT_ID_RESERVED160, bmdm_wd_mn_handler, 0,
				"BMDM_WD_MN", NULL)) {
		pr_err("******************************************\n");
		pr_err("Error requesting for bmdm wdm irq %d\n",
				BCM_INT_ID_RESERVED160);
		pr_err("AP can no longer monitor CP watchdog\n");
		pr_err("******************************************\n");
		goto err;
	}

	atomic_notifier_chain_register(&panic_notifier_list, &bmdm_wd_mn_panic);
	pr_info("bmdm watchdog monitor driver initialized\n");

err:
	return 0;
}

module_init(bmdm_wd_mn_init);
