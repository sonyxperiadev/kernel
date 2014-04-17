/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/plat-bcmap/sysfs.c
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

/*
 * SYSFS infrastructure specific Broadcom SoCs
 */
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/kmsg_dump.h>
#include <linux/mfd/bcmpmu.h>
#include <linux/module.h>
#include <plat/kona_reset_reason.h>
#include <mach/io_map.h>
#include <mach/rdb_A0/brcm_rdb_root_rst_mgr_reg.h>

#ifdef CONFIG_KONA_TIMER_UNIT_TESTS
#include <mach/kona_timer.h>
#endif

#include <mach/sram_config.h>

struct kobject *bcm_kobj;

static char *str_reset_reason[] = {
	"power_on_reset",
	"soft_reset",
	"charging",
	"ap_only",
	"bootloader",
	"recovery",
#ifdef CONFIG_BCM_RTC_ALARM_BOOT
	"rtc_alarm",
#endif
	"unknown"
};

unsigned int hard_reset_reason;
EXPORT_SYMBOL(hard_reset_reason);

int is_soft_reset(void)
{
	u32 reg;
	int soft_rst;

	reg = readl(KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_RSTSTS_OFFSET);
	soft_rst = (reg & ROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_MASK)
		>> ROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_SHIFT;

	return soft_rst;
}
EXPORT_SYMBOL(is_soft_reset);

static void set_emu_reset_reason(unsigned int const emu, int val)
{
	unsigned int *rst = (unsigned int *)ioremap(emu, 0x4);
	unsigned short soc0 = 0;

	soc0 = *rst;
	soc0 &= ~(RST_REASON_MASK);
	soc0 |= val;
	*rst = soc0;

	pr_debug("%s: Reset reason: 0x%x", __func__, *rst);

	iounmap(rst);
}

static unsigned int get_emu_reset_reason(unsigned int const emu)
{
	unsigned int *reset_reason = (unsigned int *)ioremap(emu, 0x4);
	unsigned int rst;

	pr_debug("%s: reset_reason 0x%x\n", __func__, *reset_reason);

	rst = (*reset_reason) & RST_REASON_MASK;

	iounmap(reset_reason);

	return rst;
}

static void do_clear_emu_reset_reason(void)
{
	unsigned int *rst = (unsigned int *)ioremap(SRAM_RST_REASON_BASE, 0x4);

	*rst = 0;

	iounmap(rst);
}

unsigned int is_charging_state(void)
{
	unsigned int state;

	state = get_emu_reset_reason(SRAM_RST_REASON_BASE);

	state = state & RST_REASON_MASK;

	pr_debug("%s\n reset reason = 0x%x", __func__, state);
	return (state == CHARGING_STATE) ? 1 : 0;
}

void do_set_poweron_reset_boot(void)
{
	pr_info("%s\n", __func__);
	do_clear_emu_reset_reason();
	set_emu_reset_reason(SRAM_RST_REASON_BASE, POWERON_RESET);
}
EXPORT_SYMBOL(do_set_poweron_reset_boot);

/* Add reboot to bootloader support */
void do_set_bootloader_boot(void)
{
	pr_info("%s\n", __func__);
	do_clear_emu_reset_reason();
	set_emu_reset_reason(SRAM_RST_REASON_BASE, BOOTLOADER_BOOT);
}
EXPORT_SYMBOL(do_set_bootloader_boot);

void do_set_recovery_boot(void)
{
	pr_info("%s\n", __func__);
	do_clear_emu_reset_reason();
	set_emu_reset_reason(SRAM_RST_REASON_BASE, RECOVERY_BOOT);
}
EXPORT_SYMBOL(do_set_recovery_boot);

void do_set_ap_only_boot(void)
{
	pr_debug("%s\n", __func__);
	do_clear_emu_reset_reason();
	set_emu_reset_reason(SRAM_RST_REASON_BASE, AP_ONLY_BOOT);
}
EXPORT_SYMBOL(do_set_ap_only_boot);

void do_clear_ap_only_boot(void)
{
	pr_debug("%s\n", __func__);
	do_clear_emu_reset_reason();
}
EXPORT_SYMBOL(do_clear_ap_only_boot);

bool ap_only_boot;
EXPORT_SYMBOL(ap_only_boot);

core_param(ap_only_boot, ap_only_boot, bool, 0644);

/**
 * This API checks to see if kernel boot is done for AP_ONLY mode
 * Return Values:
 * 1 = ap-only mode
 * 0 = AP + CP mode
 *
 */
unsigned int is_ap_only_boot(void)
{
	unsigned int rst;

	if (!ap_only_boot)
		rst = get_emu_reset_reason(SRAM_RST_REASON_BASE);
	else
		rst = AP_ONLY_BOOT;
	rst = rst & RST_REASON_MASK;

	pr_debug("%s\n reset_reason = 0x%x", __func__, rst);
	return (rst == AP_ONLY_BOOT) ? 1 : 0;
}
EXPORT_SYMBOL(is_ap_only_boot);

static ssize_t
reset_reason_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int index, rst;

	rst = get_emu_reset_reason(SRAM_RST_REASON_BASE);

	switch (rst) {
	case 0x1:
		index = 0;
		break;
	case 0x3:
		index = 2;
		break;
	case 0x4:
		index = 3;
		break;
	case 0x5:
		index = 4;
		break;
	case 0x6:
		index = 5;
		break;
#ifdef CONFIG_BCM_RTC_ALARM_BOOT
	case 0x7:
		index = 6;
		break;
#endif
	default:
		index = 0;
	}

	pr_debug("%s: reset reason index %d\n", __func__, index);
	snprintf(buf, PAGE_SIZE, "%s\n", str_reset_reason[index]);

	return strlen(str_reset_reason[index]) + 1;
}

static ssize_t
reset_reason_store(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t n)
{
	char reset_reason[32];
	int i;


	if (sscanf(buf, "%s", reset_reason) == 1) {
		pr_debug("%s: Reset reason: %s", __func__, reset_reason);

		for (i = 0; i < ARRAY_SIZE(str_reset_reason); i++) {
			if (strcmp(reset_reason, str_reset_reason[i]) == 0)
				break;
		}

		set_emu_reset_reason(SRAM_RST_REASON_BASE, (i + 1));

		return n;
	}

	return -EINVAL;
}

static DEVICE_ATTR(reset_reason, 0664, reset_reason_show, reset_reason_store);

static ssize_t
hard_reset_store(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t n)
{
	unsigned int in_reset_reason;

	if (sscanf(buf, "%d", &in_reset_reason) == 1) {
		if (in_reset_reason > 15)
			goto err;
		hard_reset_reason = in_reset_reason;
		return n;
	}
err:
	pr_info("\r\nusage: \r\n"
		"enable hard reset : "
		"echo [hard_reset_reason (1-15)] > /sys/bcm/hard_reset\r\n"
		"disable hard reset : "
		"echo 0 > /sys/bcm/hard_reset\r\n");
	return -EINVAL;
}

static ssize_t
hard_reset_show(struct device *dev, struct device_attribute *attr,
		   char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", hard_reset_reason);
}


static DEVICE_ATTR(hard_reset, 0600, hard_reset_show, hard_reset_store);

#ifdef CONFIG_KONA_TIMER_UNIT_TESTS
static ssize_t
kona_timer_module_cfg(struct device *dev, struct device_attribute *attr,
		      const char *buf, size_t n)
{
	char name[255];
	unsigned int rate;

	if (sscanf(buf, "%s %d", name, &rate) == 2) {

		pr_info("timer name:%s rate %d \r\n", name, rate);

		/*
		 * Assuming that kona_timer_modules_init has happend already (this is
		 *safe because this function is called during system timer init
		 *itself
		 */
		if (kona_timer_module_set_rate(name, rate) < 0) {
			pr_err
			    ("kona_timer_module_cfg: Unable to set the said rate \r\n");
			return n;
		}

		pr_info("kona_timer_module_cfg: Configured the module with"
			"rate %d \r\n", rate);

		return n;
	}

	pr_info("\r\nusage: echo [timer_name(aon-timer/slave-timer)]"
		"[rate 32768 (32KHz), 1000000 (1MHz), 19500000 (19.5MHz)] > /sys/bcm/timer_module_cfg \r\n");

	return -EINVAL;
}

struct kona_timer *kt;
struct timer_ch_cfg cfg;
int timer_callback(void *p);

/* Note that this is called back from ISR context */
int timer_callback(void *p)
{
	pr_info("timer callback \r\n");
	return 0;
}

static ssize_t
kona_timer_start_test(struct device *dev, struct device_attribute *attr,
		      const char *buf, size_t n)
{
	unsigned int ch_num, mode, count;
	char name[255];

	if (sscanf(buf, "%s %d %d %d", name, &ch_num, &mode, &count) == 4) {

		pr_info
		    ("channel_num:%d mode(0-periodic 1-oneshot):%d count:%d \r\n",
		     ch_num, mode, count);

		if ((kt = kona_timer_request(name, ch_num)) < 0) {
			pr_err("kona_timer_request returned error \r\n");
			goto out;
		}

		cfg.mode = mode;
		cfg.arg = kt;
		cfg.cb = timer_callback;
		cfg.reload = count;

		if (kona_timer_config(kt, &cfg) < 0) {
			pr_err("kona_timer_config returned error \r\n");
			goto out;
		}

		if (kona_timer_set_match_start(kt, count) < 0) {
			pr_err
			    ("kona_timer_set_match_start returned error \r\n");
			goto out;
		}
		pr_info("Timer test started \r\n");
out:
		return n;
	}

	pr_info
	    ("\r\nusage: echo [name (aon-timer/slave-timer)] [channel num (0-3)] [mode(0-periodic"
	     "1-oneshot)] [count value] > /sys/bcm/timer_start_test\r\n");
	return -EINVAL;
}

static ssize_t
kona_timer_stop_test(struct device *dev, struct device_attribute *attr,
		     const char *buf, size_t n)
{
	unsigned int ch_num;

	if (sscanf(buf, "%d", &ch_num) == 1) {
		pr_info("channel_num:%d	\r\n", ch_num);
		if (kona_timer_stop(kt) < 0) {
			pr_err
			    ("Unable to stop the timer kona_timer_stop returned error \r\n");
			goto out;
		}

		if (kona_timer_free(kt) < 0) {
			pr_err("Unable to free the timer \r\n");
			goto out;
		}
		pr_info("Stopped and freed the timer \r\n");
out:
		return n;
	}

	pr_info
	    ("\r\nusage: echo [channel num (0-3)] > /sys/bcm/timer_stop_test\r\n");
	return -EINVAL;
}
#endif

#ifdef CONFIG_KONA_TIMER_UNIT_TESTS
static DEVICE_ATTR(timer_module_cfg, 0666, NULL, kona_timer_module_cfg);
static DEVICE_ATTR(timer_start_test, 0666, NULL, kona_timer_start_test);
static DEVICE_ATTR(timer_stop_test, 0666, NULL, kona_timer_stop_test);
#endif

static struct attribute *bcm_attrs[] = {
#ifdef CONFIG_KONA_TIMER_UNIT_TESTS
	&dev_attr_timer_module_cfg.attr,
	&dev_attr_timer_start_test.attr,
	&dev_attr_timer_stop_test.attr,
#endif
	&dev_attr_reset_reason.attr,
	&dev_attr_hard_reset.attr,
	NULL,
};

static struct attribute_group bcm_attr_group = {
	.attrs = bcm_attrs,
};

static int __init bcm_sysfs_init(void)
{
	/* init reset reason as 0 */
	hard_reset_reason = 0;
	bcm_kobj = kobject_create_and_add("bcm", NULL);
	if (!bcm_kobj)
		return -ENOMEM;
	return sysfs_create_group(bcm_kobj, &bcm_attr_group);
}

static void __exit bcm_sysfs_exit(void)
{
	sysfs_remove_group(bcm_kobj, &bcm_attr_group);
}

module_init(bcm_sysfs_init);
module_exit(bcm_sysfs_exit);
