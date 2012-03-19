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

#ifdef CONFIG_KONA_TIMER_UNIT_TESTS
#include <mach/kona_timer.h>
#endif

struct kobject *bcm_kobj;

#define REG_EMU_AREA 0x3404BF90

static char *str_reset_reason[] = {
	"power_on_reset",
	"soft_reset",
	"charging",
	"unknown"
};

static ssize_t
reset_reason_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int index, *reset_reason, rst;

	reset_reason = (unsigned int *)ioremap(REG_EMU_AREA, 0x4);
	pr_debug("%s: reset_reason 0x%x\n", __func__, *reset_reason);
	rst = *reset_reason;
	rst &= 0xf;

	switch (rst) {
	case 0x1:
		index = 0;
		break;
	case 0x3:
		index = 2;
		break;
	default:
		index = 0;
	}

	pr_debug("%s: reset reason index %d\n", __func__, index);
	sprintf(buf, "%s\n", str_reset_reason[index]);

	iounmap(reset_reason);
	return strlen(str_reset_reason[index]) + 1;
}

static ssize_t
reset_reason_store(struct device *dev, struct device_attribute *attr,
		   const char *buf, size_t n)
{
	char reset_reason[32];
	int i;
	unsigned short soc0 = 0;
	unsigned int *rst;

	rst = (unsigned int *)ioremap(REG_EMU_AREA, 0x4);

	if (sscanf(buf, "%s", reset_reason) == 1) {
		pr_debug("%s: Reset reason: %s", __func__, reset_reason);

		for (i = 0; i < ARRAY_SIZE(str_reset_reason); i++) {
			if (strcmp(reset_reason, str_reset_reason[i]) == 0)
				break;
		}

		soc0 = *rst;
		soc0 &= ~(0xf);
		soc0 |= ((i + 1));
		*rst = soc0;

		return n;
	}

	iounmap(rst);
	return -EINVAL;
}

static DEVICE_ATTR(reset_reason, 0664, reset_reason_show, reset_reason_store);

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
		"[rate 32000 (32KHz), 1000000 (1MHz), 19500000 (19.5MHz)] > /sys/bcm/timer_module_cfg \r\n");

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
	NULL,
};

static struct attribute_group bcm_attr_group = {
	.attrs = bcm_attrs,
};

static int __init bcm_sysfs_init(void)
{
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
