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
#include <linux/platform_device.h>
#include <mach/io_map.h>
#include <plat/kona_reset_reason.h>
#include <mach/rdb/brcm_rdb_root_rst_mgr_reg.h>
#include <linux/types.h>
#include <linux/io.h>
#include <mach/sram_config.h>
#include <plat/cdc.h>
#include <asm/fiq.h>
#include <linux/broadcom/kona_sec_wd.h>
#include <linux/dma-mapping.h>

#define SEC_EXIT_NORMAL			1
#define SEC_WD_TAP_DELAY		12000
#define SSAPI_ACTIVATE_SEC_WATCHDOG     0x0100000A
#define SSAPI_ENABLE_SEC_WATCHDOG       0x01000007
#define SSAPI_DISABLE_SEC_WATCHDOG      0x01000008
#define SSAPI_PAT_SEC_WATCHDOG		0x01000009


enum sec_wd_state_e {
	SEC_WD_DISABLE = 0,
	SEC_WD_ENABLE = 1,
	WD_STATES,
	SEC_WD_PAT_BEGIN = 0,
	SEC_WD_PAT_DONE,
	SEC_WD_SET_NEXT_EVENT,
	PAT_STATES,
};

struct sec_wd_core_st {
	struct workqueue_struct *sec_wd_wq;
	struct delayed_work sec_wd_work;
	struct notifier_block sec_wd_panic_block;
	struct notifier_block sec_wd_reboot_block;
	unsigned int sec_wd_enabled_mode;
};

struct sec_wd_tracker {
	unsigned int is_sec_wd_on;
	unsigned int is_sec_pat_done;
};

static struct sec_wd_core_st sec_wd_core;
static struct sec_wd_tracker *sec_wd_trk;

static void sec_wd_pat(struct work_struct *work)
{
	struct sec_wd_core_st *swdc = &sec_wd_core;

	if (sec_wd_trk->is_sec_wd_on == SEC_WD_ENABLE) {
		sec_wd_trk->is_sec_pat_done = SEC_WD_PAT_BEGIN;
		secure_api_call(SSAPI_PAT_SEC_WATCHDOG, 0, 0, 0, 0);
		sec_wd_trk->is_sec_pat_done = SEC_WD_PAT_DONE;
		queue_delayed_work_on(0, swdc->sec_wd_wq, &swdc->sec_wd_work,
		msecs_to_jiffies(SEC_WD_TAP_DELAY));
		sec_wd_trk->is_sec_pat_done = SEC_WD_SET_NEXT_EVENT;
	} else {
		printk(KERN_ALERT "patting watchdog disable\n");
	}
}

void sec_wd_touch(void)
{
	if (sec_wd_trk->is_sec_wd_on == SEC_WD_ENABLE) {
		printk(KERN_ALERT "____touch the secure watchdog <prolonged path>___\n");
		secure_api_call(SSAPI_PAT_SEC_WATCHDOG, 0, 0, 0, 0);
	}
}
EXPORT_SYMBOL(sec_wd_touch);

static void init_patter(void)
{
	struct sec_wd_core_st *swdc = &sec_wd_core;
	INIT_DELAYED_WORK(&swdc->sec_wd_work, sec_wd_pat);
	queue_delayed_work_on(0, swdc->sec_wd_wq, &swdc->sec_wd_work,
	msecs_to_jiffies(SEC_WD_TAP_DELAY));
}

void sec_wd_enable(void)
{
	int ret;
	struct sec_wd_core_st *swdc = &sec_wd_core;

	if (sec_wd_trk->is_sec_wd_on == SEC_WD_ENABLE)
		return;

	ret = secure_api_call_local(SSAPI_ENABLE_SEC_WATCHDOG);
	if (ret == 1) {
		sec_wd_trk->is_sec_wd_on = SEC_WD_ENABLE;
		queue_delayed_work_on(0, swdc->sec_wd_wq, &swdc->sec_wd_work,
		msecs_to_jiffies(SEC_WD_TAP_DELAY));
	} else {
		printk(KERN_ERR "___resume:enabling secure watchdog...fail\n");
	}
}
EXPORT_SYMBOL(sec_wd_enable);

void sec_wd_disable(void)
{
	int ret;

	if (sec_wd_trk->is_sec_wd_on == SEC_WD_DISABLE)
		return;

	/* we do not need following piece of code
	 * because disable watchdog is happening in
	 * kona_mach_pm_enter.
	 * let us remove it after extensive testing. */
#if 0
	ret = cancel_delayed_work(&swdc->sec_wd_work);
	if (ret == 0) {
		printk(KERN_ALERT "secure watchdog wq could not be cancelled\n");
		/* workq instance might be running, wait for it */
		flush_workqueue(swdc->sec_wd_wq);
	}
#endif
	/* at this point we are absolutely sure that,
	no work function is running anywhere in the system.
	this is the safest place to disabled watchdog. */

	ret = secure_api_call_local(SSAPI_DISABLE_SEC_WATCHDOG);

	if (ret == 1) {
		sec_wd_trk->is_sec_wd_on = SEC_WD_DISABLE;
	} else {
		printk(KERN_ALERT "___suspend:disabling secure watchdog...fail\n");
	}
}
EXPORT_SYMBOL(sec_wd_disable);

static int sec_wd_panic_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
       if (sec_wd_trk->is_sec_wd_on == SEC_WD_DISABLE)
               return NOTIFY_DONE;

	secure_api_call(SSAPI_DISABLE_SEC_WATCHDOG, 0, 0, 0, 0);
	sec_wd_trk->is_sec_wd_on = SEC_WD_DISABLE;
	return NOTIFY_DONE;
}

static int sec_wd_reboot_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
       if (sec_wd_trk->is_sec_wd_on == SEC_WD_DISABLE)
               return NOTIFY_DONE;

	secure_api_call(SSAPI_DISABLE_SEC_WATCHDOG, 0, 0, 0, 0);
	sec_wd_trk->is_sec_wd_on = SEC_WD_DISABLE;
	return NOTIFY_DONE;
}

int is_soft_reset_boot(void)
{
	u32 reg;
	int soft_rst;

	reg = readl(KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_RSTSTS_OFFSET);
	soft_rst = (reg & ROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_MASK)
		>> ROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_SHIFT;

	return soft_rst;
}

int is_power_on_reset(void)
{
	u32 reg;
	int pon_rst;

	reg = readl(KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_RSTSTS_OFFSET);
	pon_rst = (reg & ROOT_RST_MGR_REG_RSTSTS_PORRST_DET_MASK)
		>> ROOT_RST_MGR_REG_RSTSTS_PORRST_DET_SHIFT;

	return pon_rst;
}

static struct fiq_handler fh = {
		.name = "sec_wd_fiq"
};

static DEFINE_PER_CPU(struct pt_regs, sec_wd_fiq_regs);

/*
 * sec_wd_fiq_reg_setup: Called on each CPU to set up FIQ handler regs
 * input: unused
 * output: none
 * return: none
 */
static void sec_wd_fiq_reg_setup(void *info)
{
	/* NB - secure code may mean handler doesn't see these registers */
	struct pt_regs r;
	get_fiq_regs(&r);
	r.ARM_sp = (unsigned long) (&__get_cpu_var(sec_wd_fiq_regs));
	set_fiq_regs(&r);
}

asmlinkage void sec_wd_fiq(struct pt_regs *regs)
{
	/* If we come in through our assembler, we're on the original SVC
	 * mode stack, with nothing extra pushed. Good for unwinding, bad if
	 * the stack was broken.
	 */
	int err = 0;
	/*
	 * We could be in big trouble if we ever try to claim any spinlocks
	 * that were held at the instant the FIQ triggered. die() makes some
	 * effort to avoid problems with printk(), but try a bit harder to
	 * bust some more locks here.
	 */
	console_verbose();
	die("secure watchdog FIQ", regs, err);
	panic("secure watchdog FIQ");
}

static int sec_wd_probe(struct platform_device *pdev)
{
	struct sec_wd_core_st *swdc = &sec_wd_core;
	int ret;
	dma_addr_t p;
        void *v;
	sec_wd_trk = dma_alloc_coherent(NULL, sizeof(struct sec_wd_tracker), &p, GFP_KERNEL);
	if (sec_wd_trk == NULL) {
		pr_err("%s:dma_alloc_coherent failed\n", __func__);
		BUG_ON(1);
	}

	swdc->sec_wd_enabled_mode = 1;
	swdc->sec_wd_panic_block.notifier_call = sec_wd_panic_event;
	swdc->sec_wd_panic_block.priority = INT_MAX;

	swdc->sec_wd_reboot_block.notifier_call = sec_wd_reboot_event;
	swdc->sec_wd_reboot_block.priority = INT_MAX;
	sec_wd_trk->is_sec_wd_on = SEC_WD_DISABLE;

	atomic_notifier_chain_register(&panic_notifier_list,
	&swdc->sec_wd_panic_block);
	blocking_notifier_chain_register(&reboot_notifier_list,
	&swdc->sec_wd_reboot_block);
	swdc->sec_wd_wq = create_singlethread_workqueue("sec_wd_wq");
	if (!(swdc->sec_wd_wq)) {
		printk(KERN_ALERT "___sec_wd_probe:alloc workqueue failed\n");
		return -ENOMEM;
	}
	ret = claim_fiq(&fh);
	if (0 > ret) {
		printk(KERN_ERR "%s:%d claim_fiq failed err=%d\n",
		__func__, __LINE__, ret);
		return ret;
	}
	on_each_cpu(sec_wd_fiq_reg_setup, NULL, true);
	set_fiq_handler(&sec_wd_fiq_handler,
	&sec_wd_fiq_handler_end - &sec_wd_fiq_handler);

	printk(KERN_ALERT "_______sec_wd_probe succesfull_______\n");
	return 0;
}

unsigned int sec_wd_activate(void)
{
	struct sec_wd_core_st *swdc = &sec_wd_core;
	printk(KERN_ALERT "______initializing secure watchdog______\n");
	sec_wd_trk->is_sec_wd_on = SEC_WD_ENABLE;
	printk(KERN_ALERT "Active\n");
	secure_api_call(SSAPI_ACTIVATE_SEC_WATCHDOG, 0, 0, 0, 0);
	init_patter();
	return 0;
}
EXPORT_SYMBOL(sec_wd_activate);

unsigned int is_sec_wd_enabled(void)
{
	struct sec_wd_core_st *swdc = &sec_wd_core;
	return swdc->sec_wd_enabled_mode;
}
EXPORT_SYMBOL(is_sec_wd_enabled);

static int sec_wd_remove(struct platform_device *pdev);


static const struct of_device_id sec_wd_match[] = {
	{ .compatible = "bcm,secure_watchdog" },
	{ /* Sentinel */ }
};

static struct platform_driver sec_wd_pltfm_driver = {
	.driver = {
		   .name = "secure_watchdog",
		   .owner = THIS_MODULE,
	   .of_match_table = sec_wd_match,
		   },
	.probe = sec_wd_probe,
	.remove = sec_wd_remove,
};

static int sec_wd_remove(struct platform_device *pdev)
{
	if (is_sec_wd_enabled())
		sec_wd_disable();
	return 0;
}

static int __init sec_wd_init(void)
{
	return platform_driver_register(&sec_wd_pltfm_driver);
}

static void __exit sec_wd_exit(void)
{
	platform_driver_unregister(&sec_wd_pltfm_driver);
}

module_init(sec_wd_init);
module_exit(sec_wd_exit);

MODULE_AUTHOR("Oza");
MODULE_DESCRIPTION("Secure Watchdog");
MODULE_LICENSE("GPL");
