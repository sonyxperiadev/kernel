/*
 * Watchdog driver for the KONA architecture
 *
 * Copyright (C) 2011 Broadcom Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/uaccess.h>
#include <mach/hardware.h>
#include <linux/io.h>
#include <mach/rdb/brcm_rdb_secwatchdog.h>

static int nowayout = WATCHDOG_NOWAYOUT;
static unsigned int heartbeat = 60;	/* (secs) Default is 1 minute */
static unsigned long wdt_status;
static spinlock_t wdt_lock;

#define	WDT_IN_USE		0
#define	WDT_OK_TO_CLOSE		1
#define	WDT_CLK_RESOLUTION	4  /* 62.5ms */
#define	WDT_TICK_RATE			16 /* 16 ticks per sec */

static unsigned long base = KONA_SECWD_VA;
static void kona_wdt_enable(void)
{
	unsigned long val;
	spin_lock(&wdt_lock);
	/* Sequence to enable the watchdog */
	val = ( (readl(base + SECWATCHDOG_SDOGCR_OFFSET)  |
		     SECWATCHDOG_SDOGCR_EN_MASK) &
		    (~SECWATCHDOG_SDOGCR_WD_LOAD_FLAG_MASK) ) |
		  ( (WDT_CLK_RESOLUTION << SECWATCHDOG_SDOGCR_CLKS_SHIFT) &
		    SECWATCHDOG_SDOGCR_CLKS_MASK);
	writel(val, base + SECWATCHDOG_SDOGCR_OFFSET);
	while (readl(base + SECWATCHDOG_SDOGCR_OFFSET) & 
		   SECWATCHDOG_SDOGCR_WD_LOAD_FLAG_MASK);
	spin_unlock(&wdt_lock);
}

static void kona_wdt_disable(void)
{
	unsigned long val;

	spin_lock(&wdt_lock);
	/* Sequence to disable the watchdog */
	val = readl(base + SECWATCHDOG_SDOGCR_OFFSET) &
		 (~SECWATCHDOG_SDOGCR_EN_MASK);
	writel(val, base + SECWATCHDOG_SDOGCR_OFFSET);
	while (readl(base + SECWATCHDOG_SDOGCR_OFFSET) & 
		   SECWATCHDOG_SDOGCR_WD_LOAD_FLAG_MASK);
	spin_unlock(&wdt_lock);
}

static void kona_wdt_keepalive(void)
{
	unsigned long val;

	spin_lock(&wdt_lock);
	val = (readl(base + SECWATCHDOG_SDOGCR_OFFSET) & 
		   (~SECWATCHDOG_SDOGCR_LD_MASK) ) |
		  (((heartbeat  * WDT_TICK_RATE) <<
		    SECWATCHDOG_SDOGCR_LD_SHIFT) & 
		   SECWATCHDOG_SDOGCR_LD_MASK);
	writel(val, base + SECWATCHDOG_SDOGCR_OFFSET);
	spin_unlock(&wdt_lock);
}

static int kona_wdt_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(WDT_IN_USE, &wdt_status))
		return -EBUSY;

	clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

	 kona_wdt_enable();

	return nonseekable_open(inode, file);
}

static ssize_t kona_wdt_write(struct file *file, const char *data,
						size_t len, loff_t *ppos)
{
	if (len) {
		if (!nowayout) {
			size_t i;

			clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

			for (i = 0; i != len; i++) {
				char c;

				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					set_bit(WDT_OK_TO_CLOSE, &wdt_status);
			}
		}
		kona_wdt_keepalive();
	}

	return len;
}


static const struct watchdog_info ident = {
	.options	= WDIOF_MAGICCLOSE | WDIOF_SETTIMEOUT |
				WDIOF_KEEPALIVEPING,
	.identity	= "KONA Watchdog",
	.firmware_version = 0,
};

static long kona_wdt_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
{
	int ret = -ENOTTY;
	int time;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = copy_to_user((struct watchdog_info *)arg, &ident,
				   sizeof(ident)) ? -EFAULT : 0;
		break;

	case WDIOC_GETSTATUS:
		ret = put_user(0, (int *)arg);
		break;

	case WDIOC_GETBOOTSTATUS:
		ret = put_user(0, (int *)arg);
		break;

	case WDIOC_KEEPALIVE:
		kona_wdt_enable();
		ret = 0;
		break;

	case WDIOC_SETTIMEOUT:
		ret = get_user(time, (int *)arg);
		if (ret)
			break;

		if (time <= 0 || time > 60) {
			ret = -EINVAL;
			break;
		}

		heartbeat = time;
		kona_wdt_keepalive();
		/* Fall through */

	case WDIOC_GETTIMEOUT:
		ret = put_user(heartbeat, (int *)arg);
		break;
	}

	return ret;
}

static int kona_wdt_release(struct inode *inode, struct file *file)
{
	if (test_bit(WDT_OK_TO_CLOSE, &wdt_status))
		kona_wdt_disable();
	else
		printk(KERN_CRIT "WATCHDOG: Device closed unexpectedly - "
					"timer will not stop\n");
	clear_bit(WDT_IN_USE, &wdt_status);
	clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

	return 0;
}


static const struct file_operations kona_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= kona_wdt_write,
	.unlocked_ioctl	= kona_wdt_ioctl,
	.open		= kona_wdt_open,
	.release	= kona_wdt_release,
};

static struct miscdevice kona_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &kona_wdt_fops,
};

static int __init kona_wdt_init(void)
{
	spin_lock_init(&wdt_lock);
	return misc_register(&kona_wdt_miscdev);
}

static void __exit kona_wdt_exit(void)
{
	misc_deregister(&kona_wdt_miscdev);
}

module_init(kona_wdt_init);
module_exit(kona_wdt_exit);

MODULE_AUTHOR("Broadcom Inc.");
MODULE_DESCRIPTION("KONA Architecture Watchdog");

module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat, "Watchdog heartbeat in seconds (default 60s)");

module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started");

MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);

