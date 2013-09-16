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

#define SEC_EXIT_NORMAL			1
#define SEC_WD_TAP_DELAY		12000
#define SSAPI_ACTIVATE_SEC_WATCHDOG     0x01000006
#define SSAPI_ENABLE_SEC_WATCHDOG       0x01000007
#define SSAPI_DISABLE_SEC_WATCHDOG      0x01000008
#define SSAPI_PAT_SEC_WATCHDOG		0x01000009

static struct delayed_work sec_wd_work;
struct notifier_block sec_wd_panic_block;
struct notifier_block sec_wd_reboot_block;
unsigned int sec_wd_enabled_mode;

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

void sec_wd_enable(void)
{
	secure_api_call(SSAPI_ENABLE_SEC_WATCHDOG, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sec_wd_enable);

void sec_wd_disable(void)
{
	secure_api_call(SSAPI_DISABLE_SEC_WATCHDOG, 0, 0, 0, 0);
}
EXPORT_SYMBOL(sec_wd_disable);

static int sec_wd_panic_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	secure_api_call(SSAPI_DISABLE_SEC_WATCHDOG, 0, 0, 0, 0);
	return NOTIFY_DONE;
}

static int sec_wd_reboot_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	secure_api_call(SSAPI_DISABLE_SEC_WATCHDOG, 0, 0, 0, 0);
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
	unsigned int *reset_reason = (unsigned int *)
				ioremap(SRAM_RST_REASON_BASE, 0x4);
	unsigned int rst;

	pr_debug("%s: reset_reason 0x%x\n", __func__, *reset_reason);

	rst = (*reset_reason) & RST_REASON_MASK;

	iounmap(reset_reason);
	if (rst == POWERON_RESET)
		return 1;
	else
		return 0;

}

static int __devinit sec_wd_probe(struct platform_device *pdev)
{
	sec_wd_enabled_mode = 0;

	if (is_soft_reset_boot() || is_power_on_reset()) {
		sec_wd_enabled_mode = 1;
		sec_wd_panic_block.notifier_call = sec_wd_panic_event;
		sec_wd_panic_block.priority = INT_MAX;

		sec_wd_reboot_block.notifier_call = sec_wd_reboot_event;
		sec_wd_reboot_block.priority = INT_MAX;


		atomic_notifier_chain_register(&panic_notifier_list,
				&sec_wd_panic_block);
		blocking_notifier_chain_register(&reboot_notifier_list,
				&sec_wd_reboot_block);

		printk(KERN_ALERT "________initializing secure watchdog_____\n");
		secure_api_call(SSAPI_ACTIVATE_SEC_WATCHDOG, 0, 0, 0, 0);
		init_patter();
	} else
		printk(KERN_ALERT "_______disabling Sec watchdog_______\n");

	printk(KERN_ALERT "_______sec_wd_probe succesfull_______\n");
	return 0;
}

unsigned int is_sec_wd_enabled(void)
{
	return sec_wd_enabled_mode;
}
EXPORT_SYMBOL(is_sec_wd_enabled);

static int sec_wd_suspend(struct platform_device *pdev)
{
	if  (is_sec_wd_enabled()) {
		printk(KERN_ALERT "_______suspend:disabling secure watchdog_______\n");
		sec_wd_disable();
	}
	return 0;
}

static int sec_wd_resume(struct platform_device *pdev)
{
	if  (is_sec_wd_enabled()) {
		printk(KERN_ALERT "_______resume:enabling secure watchdog_______\n");
		sec_wd_enable();
	}
	return 0;
}

static int __devexit sec_wd_remove(struct platform_device *pdev);


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
	.remove = __devexit_p(sec_wd_remove),
	.suspend = sec_wd_suspend,
	.resume = sec_wd_resume,
};

static int __devexit sec_wd_remove(struct platform_device *pdev)
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

