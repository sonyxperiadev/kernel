/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/watchdog/bcm59055-wd.c
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include <linux/mfd/bcm590xx/core.h>


#define WDT_0_SEC_TIMEOUT		0
#define WDT_MAX_SEC_TIMEOUT		127


/*
 * *  Private data
 * */
struct bcm59055_wdog {
	struct bcm590xx *bcm590xx;
	struct miscdevice miscdev;
	struct delayed_work reset_wd_timer_wq;
};


static int bcm59055_wdog_enable(struct bcm59055_wdog *wddev)
{
	struct bcm590xx *bcm590xx = wddev->bcm590xx;
	u8 regVal;
	pr_debug("Inside %s\n", __func__);
	/* Enable The watchdog */
	regVal = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HOSTCTRL1);
	regVal |= BCM59055_HSCTRL1_SYS_WDT_EN;
	return bcm590xx_reg_write(bcm590xx, BCM59055_REG_HOSTCTRL1, regVal);;
}

static int bcm59055_wdog_disable(struct bcm59055_wdog *wddev)
{
	struct bcm590xx *bcm590xx = wddev->bcm590xx;
	u8 regVal;
	pr_debug("Inside %s\n", __func__);
	/* Disable The watchdog */
	regVal = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HOSTCTRL1);
	regVal &= ~BCM59055_HSCTRL1_SYS_WDT_EN;
	return bcm590xx_reg_write(bcm590xx, BCM59055_REG_HOSTCTRL1, regVal);;
}

static int bcm59055_wdog_gettimeout(struct bcm59055_wdog *wddev)
{
	struct bcm590xx *bcm590xx = wddev->bcm590xx;
	int time;
	u8 regVal;
	pr_debug("Inside %s\n", __func__);

	regVal = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HOSTCTRL2);
	time = regVal & BCM59055_HSCTRL2_SYS_WDT_TIME_MASK;

	return time;
}

static int bcm59055_wdog_settimeout(struct bcm59055_wdog *wddev, int timeout)
{
	struct bcm590xx *bcm590xx = wddev->bcm590xx;
	u8 regVal;
	pr_debug("Inside %s\n", __func__);

	/* Set timeout */
	regVal = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HOSTCTRL2);

	pr_debug("%s: HOSTACT 0x%x\n", __func__, regVal);

	regVal |= (timeout & BCM59055_HSCTRL2_SYS_WDT_TIME_MASK);

	return bcm590xx_reg_write(bcm590xx, BCM59055_REG_HOSTCTRL2, regVal);
}

static int bcm59055_wdog_reset(struct bcm59055_wdog *wddev)
{
	struct bcm590xx *bcm590xx = wddev->bcm590xx;
	u8 regVal;

	/* Reset the WD to avoid system restart */
	regVal = bcm590xx_reg_read(bcm590xx, BCM59055_REG_HOSTCTRL1);

	pr_debug("%s: HOSTACT 0x%x\n", __func__, regVal);

	regVal |= BCM59055_HSCTRL1_SYS_WDT_CLR;

	return bcm590xx_reg_write(bcm590xx, BCM59055_REG_HOSTCTRL1, regVal);
}

/* Not required: remove it*/
static ssize_t bcm59055_wdog_write(struct file *file, const char *data,
				   size_t len, loff_t *ppos)
{
	pr_info("My dog is watching now!! sent data %s\n", data);
	return len;
}

static long bcm59055_wdog_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	struct bcm59055_wdog *wddev;
	static const struct watchdog_info ident = {
		.identity = "BCM59055 Watchdog",
		.options = WDIOF_SETTIMEOUT,
		.firmware_version = 0,
	};
	long ret;
	int time_out;

	wddev = file->private_data;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = copy_to_user((struct watchdog_info __user *)arg, &ident,
				   sizeof(ident));
		break;
	case WDIOC_GETSTATUS:
		ret = put_user(0, (int __user *)arg);
		break;
	case WDIOC_KEEPALIVE:
		ret = bcm59055_wdog_reset(wddev);
		break;
	case WDIOC_SETTIMEOUT:
		if (get_user(time_out, (int __user *)arg))
			return -EFAULT;
		if (time_out < 0 || time_out > 64)
			return -EINVAL;
		ret = bcm59055_wdog_settimeout(wddev, time_out);
		if (ret)
			break;
		ret = bcm59055_wdog_reset(wddev);
		break;
	case WDIOC_GETTIMEOUT:
		time_out = bcm59055_wdog_gettimeout(wddev);
		ret = put_user(time_out, (int __user *)arg);
		break;
	default:
		ret = -ENOTTY;
	}
	return ret;
}

static int bcm59055_wdog_open(struct inode *inode, struct file *file)
{

	return nonseekable_open(inode, file);
}

static int bcm59055_wdog_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int bcm59055_wdog_notify_sys(struct notifier_block *this,
				    unsigned long code, void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT) {
		// bcm59055_wdog_stop();                /* Turn the WDT off*/
	}
	return NOTIFY_DONE;
}

static struct file_operations bcm59055_wdog_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.write = bcm59055_wdog_write,
	.unlocked_ioctl = bcm59055_wdog_ioctl,
	.open = bcm59055_wdog_open,
	.release = bcm59055_wdog_release,
};

static struct notifier_block bcm59055_wdog_notifier = {
	.notifier_call = bcm59055_wdog_notify_sys,
};

static void bcm59055_keepalive_work(struct work_struct *work)
{
	struct bcm59055_wdog *wddog = container_of(work,
			struct bcm59055_wdog, reset_wd_timer_wq.work);
	pr_debug("Inside %s\n", __func__);
	bcm59055_wdog_reset(wddog);
	schedule_delayed_work(&wddog->reset_wd_timer_wq,
						msecs_to_jiffies(20000));
}

static int __devinit bcm59055_wdog_probe(struct platform_device *pdev)
{
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);
	struct bcm59055_wdog *wddog;
	int ret = 0;

	pr_info("BCM59055 System Watdog Driver\n");

	wddog = kzalloc(sizeof(struct bcm59055_wdog), GFP_KERNEL);
	if (!wddog) {
		pr_info("%s: falied allocate memory\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	wddog->bcm590xx = bcm590xx;
	platform_set_drvdata(pdev, wddog);

	/* register wdt as misc device */
	wddog->miscdev.parent = &pdev->dev;
	wddog->miscdev.name = "watchdog";
	wddog->miscdev.fops = &bcm59055_wdog_fops;
	wddog->miscdev.minor = WATCHDOG_MINOR;

	INIT_DELAYED_WORK(&wddog->reset_wd_timer_wq, bcm59055_keepalive_work);

	ret = register_reboot_notifier(&bcm59055_wdog_notifier);
	if (ret) {
		pr_info("%s: cannot register reboot notifier (err=%d)\n",
				__func__, ret);
		goto err_free;
	}

	ret = misc_register(&wddog->miscdev);
	if (ret) {
		pr_info("%s: failed registering watchdog\n", __func__);
		goto err_free;
	}
	bcm59055_wdog_settimeout(wddog, WDT_MAX_SEC_TIMEOUT);
	bcm59055_wdog_enable(wddog);
	schedule_delayed_work(&wddog->reset_wd_timer_wq,
						msecs_to_jiffies(0));
	return 0;

      err_free:
	if (wddog)
		kfree(wddog);
      err:
	return ret;
}

static int __devexit bcm59055_wdog_remove(struct platform_device *pdev)
{
	struct bcm59055_wdog *wddog = platform_get_drvdata(pdev);

	unregister_reboot_notifier(&bcm59055_wdog_notifier);
	misc_deregister(&wddog->miscdev);
	platform_set_drvdata(pdev, NULL);
	kfree(wddog);
	return 0;
}

static int bcm59055_wdog_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bcm59055_wdog *wddog = platform_get_drvdata(pdev);
	pr_info("Inside %s\n", __func__);
	flush_delayed_work(&wddog->reset_wd_timer_wq);
	return bcm59055_wdog_disable(wddog);
}

static int bcm59055_wdog_resume(struct platform_device *pdev)
{
	struct bcm59055_wdog *wddog = platform_get_drvdata(pdev);
	int ret;
	pr_info("Inside %s\n", __func__);
	ret = bcm59055_wdog_enable(wddog);
	schedule_delayed_work(&wddog->reset_wd_timer_wq,
						msecs_to_jiffies(0));
	return ret;
}

static struct platform_driver bcm59055_wdog_driver = {
	.driver = {
		   .name = "bcm59055-wdog",
		   .owner = THIS_MODULE,
		   },
	.remove = __devexit_p(bcm59055_wdog_remove),
	.probe = bcm59055_wdog_probe,
	.suspend = bcm59055_wdog_suspend,
	.resume = bcm59055_wdog_resume
};

static int __init bcm59055_wdog_init(void)
{
	return platform_driver_register(&bcm59055_wdog_driver);
}

subsys_initcall(bcm59055_wdog_init);

static void __exit bcm59055_wdog_exit(void)
{
	platform_driver_unregister(&bcm59055_wdog_driver);
}

module_exit(bcm59055_wdog_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Watchdog Driver for Broadcom BCM59035 PMU");
MODULE_AUTHOR("TKG");
