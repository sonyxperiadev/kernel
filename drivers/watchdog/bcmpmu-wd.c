/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* @file	drivers/watchdog/bcmpmu-wd.c
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
#include <linux/mfd/bcmpmu.h>


#define WDT_0_SEC_TIMEOUT		0
#define WDT_MAX_SEC_TIMEOUT		127

#define WDT_FIRMWARE_VERSION	(0)
#define WDT_CARD_NAME			("bcmpmu Watchdog")


#define WD_WORKQ_NAME			("BCMPMU_WD_WQ")

#define KEEP_ALIVE_WORK_DELAY     \
	msecs_to_jiffies((wd_hw_timeout/2) * 1000)

#define EARLY_PANIC_WORK_DELAY    \
	msecs_to_jiffies((wd_hw_timeout - 5) * 1000)



/*
 * *  Private data
 * */
struct bcmpmu_wdog {
	struct bcmpmu *bcmpmu;
	struct miscdevice miscdev;
	struct workqueue_struct *wd_workqueue;
	struct delayed_work wd_work;
	unsigned long wd_is_opened;
	unsigned long pdata_flags;
};


static struct watchdog_info ident = {
	.identity = WDT_CARD_NAME,
	.options = 0,
	.firmware_version = WDT_FIRMWARE_VERSION,
};


static struct bcmpmu_wdog *wddog;
static int wd_hw_timeout = WDT_MAX_SEC_TIMEOUT;
static int nowayout = WATCHDOG_NOWAYOUT;
static int expect_close;


module_param_named(watchdog_timeout, wd_hw_timeout, int,
		S_IRUGO); /* Read-only by user */

module_param_named(nowayout, nowayout, int, S_IRUGO);


static int bcmpmu_wdog_enable(struct bcmpmu_wdog *wddev, bool enable)
{
	struct bcmpmu *bcmpmu = wddev->bcmpmu;
	int ret;

	pr_info("Inside %s: enable = %d\n", __func__, enable);

	/* Enable/disable The watchdog */
	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_SYS_WDT_EN,
		enable ? bcmpmu->regmap[PMU_REG_SYS_WDT_EN].mask : 0,
		bcmpmu->regmap[PMU_REG_SYS_WDT_EN].mask);

	return ret;

}

static int bcmpmu_wdog_gettimeout(struct bcmpmu_wdog *wddev)
{
	struct bcmpmu *bcmpmu = wddev->bcmpmu;
	int ret;
	unsigned int regVal = 0;

	pr_debug("Inside %s\n", __func__);

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_SYS_WDT_TIME, &regVal,
		bcmpmu->regmap[PMU_REG_SYS_WDT_TIME].mask);

	regVal >>= bcmpmu->regmap[PMU_REG_SYS_WDT_TIME].shift;

	return regVal;
}

static int bcmpmu_wdog_settimeout(struct bcmpmu_wdog *wddev, int timeout)
{

	struct bcmpmu *bcmpmu = wddev->bcmpmu;
	int ret;
	unsigned int regVal = 0;

	pr_debug("Inside %s\n", __func__);

	timeout <<= bcmpmu->regmap[PMU_REG_SYS_WDT_TIME].shift;

	/*unlock write to hostctrl2 reg*/
	bcmpmu_reg_write_unlock(bcmpmu);

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_SYS_WDT_TIME, timeout,
		bcmpmu->regmap[PMU_REG_SYS_WDT_TIME].mask);

	/* Read-back the timeout value and return to the caller */

	ret = bcmpmu->read_dev(bcmpmu, PMU_REG_SYS_WDT_TIME, &regVal,
		bcmpmu->regmap[PMU_REG_SYS_WDT_TIME].mask);

	regVal >>= bcmpmu->regmap[PMU_REG_SYS_WDT_TIME].shift;

	return regVal;
}

static int bcmpmu_wdog_reset(struct bcmpmu_wdog *wddev)
{
	struct bcmpmu *bcmpmu = wddev->bcmpmu;
	int ret;

	pr_debug("Inside %s\n", __func__);

	ret = bcmpmu->write_dev(bcmpmu, PMU_REG_SYS_WDT_CLR,
			bcmpmu->regmap[PMU_REG_SYS_WDT_CLR].mask,
			bcmpmu->regmap[PMU_REG_SYS_WDT_CLR].mask);

	return ret;
}

static void bcmpmu_wdog_keepalive(struct bcmpmu_wdog *wd, unsigned long delay)
{
	cancel_delayed_work_sync(&wd->wd_work);
	queue_delayed_work(wd->wd_workqueue, &wd->wd_work, delay);
}


static ssize_t bcmpmu_wdog_write(struct file *file, const char *data,
				   size_t len, loff_t *ppos)
{
	struct bcmpmu_wdog *wdog = file->private_data;
	size_t i;

	pr_debug("Inside %s\n", __func__);

	if (!nowayout) {
		if (!(wdog->pdata_flags & WATCHDOG_OTP_ENABLED)) {
			expect_close = 0;
			for (i = 0; i != len; i++) {
				char c;
				if (get_user(c, data + i)) {
					pr_info("get user failed\n");
					return -EFAULT;
				}
				if (c == 'V') {
					pr_info("can be closed\n");
					expect_close = 42;
				}
			}
		}
	}

	bcmpmu_wdog_reset(wdog);
	bcmpmu_wdog_keepalive(wdog, EARLY_PANIC_WORK_DELAY);

	return len;
}

static long bcmpmu_wdog_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	struct bcmpmu_wdog *wdog;
	long ret = -ENOTTY;
	int time_out;

	wdog = file->private_data;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		if (!wdog->pdata_flags&WATCHDOG_OTP_ENABLED)
			ident.options = WDIOF_SETTIMEOUT|WDIOF_MAGICCLOSE;
		ret = copy_to_user((struct watchdog_info __user *)arg, &ident,
				   sizeof(ident));
		break;
	case WDIOC_GETSTATUS:
		ret = put_user(0, (int __user *)arg);
		break;
	case WDIOC_KEEPALIVE:
		bcmpmu_wdog_keepalive(wddog, EARLY_PANIC_WORK_DELAY);
		ret = bcmpmu_wdog_reset(wdog);
		break;
	case WDIOC_SETTIMEOUT:
		if (get_user(time_out, (int __user *)arg))
			return -EFAULT;
		if (time_out < 0 || time_out > WDT_MAX_SEC_TIMEOUT)
			return -EINVAL;
		ret = bcmpmu_wdog_settimeout(wdog, time_out);
		break;
	case WDIOC_GETTIMEOUT:
		time_out = bcmpmu_wdog_gettimeout(wdog);
		ret = put_user(time_out, (int __user *)arg);
		break;
	default:
		ret = -ENOTTY;
	}
	return ret;
}

static int bcmpmu_wdog_open(struct inode *inode, struct file *file)
{
	pr_info("Inside %s\n", __func__);

	/**
	 * cancel the workqueue which was petting the watchdog
	 * till now. Now, user space process which has opened
	 * the watchdog driver should pet it.Failing to do so,
	 * watchdog driver will do force crash of the kernel
	 */

	/**
	 * cancel the work before setting "wd_is_opened"
	 * This call ensures that either work is cancelled
	 * (if its waiting for execution) or wait for it
	 * to finish (if it already executing)
	 */
	bcmpmu_wdog_keepalive(wddog, EARLY_PANIC_WORK_DELAY);

	if (test_and_set_bit(0, &wddog->wd_is_opened)) {
		pr_info("Watchdog : test_and_set_bit failed!!\n");
		return -EBUSY;
	}

	bcmpmu_wdog_enable(wddog, true);
	bcmpmu_wdog_reset(wddog);
	file->private_data = wddog;

	return nonseekable_open(inode, file);
}

static int bcmpmu_wdog_release(struct inode *inode, struct file *file)
{
	struct bcmpmu_wdog *wdog = file->private_data;

	pr_notice("Watchdog: file is closed\n");

	if (expect_close == 42) {
		clear_bit(0, &wdog->wd_is_opened);
		bcmpmu_wdog_keepalive(wdog, 0);
		/*We reach here only if nowayout is zero and
		WATCHDOG_OTP_ENABLED is NOT defined
		*/
		bcmpmu_wdog_enable(wddog, false);
	} else {
		pr_alert("Watchdog: Unexpected close, nobody feeds me\n");
		cancel_delayed_work_sync(&wddog->wd_work);
	}

	expect_close = 0;

	file->private_data = NULL;

	return 0;
}

static int bcmpmu_wdog_notify_sys(struct notifier_block *this,
				    unsigned long code, void *unused)
{
	if (code == SYS_DOWN || code == SYS_HALT) {
		cancel_delayed_work_sync(&wddog->wd_work);
		/* give a time to halt */
		bcmpmu_wdog_reset(wddog);
	}
	return NOTIFY_DONE;
}

static const struct file_operations bcmpmu_wdog_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.write = bcmpmu_wdog_write,
	.unlocked_ioctl = bcmpmu_wdog_ioctl,
	.open = bcmpmu_wdog_open,
	.release = bcmpmu_wdog_release,
};

static struct notifier_block bcmpmu_wdog_notifier = {
	.notifier_call = bcmpmu_wdog_notify_sys,
};

static void bcmpmu_keepalive_work(struct work_struct *work)
{
	struct bcmpmu_wdog *wddog = container_of(work,
			struct bcmpmu_wdog, wd_work.work);
	pr_debug("Inside %s\n", __func__);

	bcmpmu_wdog_reset(wddog);

	if (!wddog->wd_is_opened) {
		queue_delayed_work(wddog->wd_workqueue,
			&wddog->wd_work,
			KEEP_ALIVE_WORK_DELAY);
	} else {
		panic("Watchog not Serviced !!\n");
	}
}

/**
 * custom API to pet the watchdog from the kernel space
 */

int bcmpmu_watchdog_pet(void)
{
	pr_debug("Inside %s\n", __func__);

	if (wddog)
		return bcmpmu_wdog_reset(wddog);
	else
		return -1;
}
EXPORT_SYMBOL_GPL(bcmpmu_watchdog_pet);

/**
 * API to pet watchdog from kernel space
 * in i2c polling mode
 */
int bcmpmu_wdog_pet_polled(void)
{
	int err = 0;

	if (wddog) {
		err = wddog->bcmpmu->set_dev_mode(wddog->bcmpmu, 1);
		if (!err)
			err = bcmpmu_wdog_reset(wddog);
	} else
		pr_err("%s: Failed to device in poll mode\n", __func__);
	return err;
}
EXPORT_SYMBOL(bcmpmu_wdog_pet_polled);

static int __devinit bcmpmu_wdog_probe(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;

	struct bcmpmu_wd_setting *wd_pdata = bcmpmu->pdata->wd_setting;

	int ret = 0;

	pr_info("bcmpmu Watchdog Driver Probe...\n");

	wddog = kzalloc(sizeof(struct bcmpmu_wdog), GFP_KERNEL);
	if (!wddog) {
		pr_info("%s: falied allocate memory\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	wddog->bcmpmu = bcmpmu;
	wddog->miscdev.parent = &pdev->dev;
	wddog->miscdev.name = "watchdog";
	wddog->miscdev.fops = &bcmpmu_wdog_fops;
	wddog->miscdev.minor = WATCHDOG_MINOR;
	wddog->pdata_flags = wd_pdata->flags;

	INIT_DELAYED_WORK(&wddog->wd_work, bcmpmu_keepalive_work);

	wddog->wd_workqueue = create_workqueue(WD_WORKQ_NAME);

	if (wddog->wd_workqueue == NULL) {
		ret = -ENOMEM;
		goto err_free;
	}

	platform_set_drvdata(pdev, wddog);

	ret = register_reboot_notifier(&bcmpmu_wdog_notifier);
	if (ret != 0) {
		pr_info("%s: cannot register reboot notifier (err=%d)\n",
				__func__, ret);
		goto err_free;
	}

	/* Register watchdog driver as misc dev */
	ret = misc_register(&wddog->miscdev);
	if (ret < 0) {
		pr_info("%s: failed registering watchdog\n", __func__);
		goto err_free;
	}

	wd_hw_timeout = bcmpmu_wdog_settimeout(wddog,
				wd_pdata->watchdog_timeout);
	pr_info("%s:wd_hw_timeout = %d\n", __func__, wd_hw_timeout);
	if (wd_hw_timeout != wd_pdata->watchdog_timeout) {
		pr_info("%s:Failed to set watchdog timeout value %d\n",
				__func__, wd_pdata->watchdog_timeout);
	}
	queue_delayed_work(wddog->wd_workqueue,
		&wddog->wd_work,
		msecs_to_jiffies(0));

	return 0;

err_free:
	kfree(wddog);
	wddog = NULL;
err:
	return ret;
}

static int __devexit bcmpmu_wdog_remove(struct platform_device *pdev)
{
	struct bcmpmu_wdog *wddog = platform_get_drvdata(pdev);

	unregister_reboot_notifier(&bcmpmu_wdog_notifier);
	platform_set_drvdata(pdev, NULL);
	if (wddog) {
		misc_deregister(&wddog->miscdev);
		if (wddog->wd_workqueue)
			destroy_workqueue(wddog->wd_workqueue);
		kfree(wddog);
		wddog = NULL;
	}
	return 0;
}

static int bcmpmu_wdog_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct bcmpmu_wdog *wddog = platform_get_drvdata(pdev);
	pr_info("Inside %s\n", __func__);

	/**
	 * cancel the watchdog work &  service the watchdog before entering
	 * into suspend. Platform wakeup timer will wake the system before
	 * the watchdog timer expires will service the watchdog in resume
	 * call
	 */

	if (wddog->wd_is_opened)
		cancel_delayed_work_sync(&wddog->wd_work);
	return bcmpmu_wdog_reset(wddog);
}

static int bcmpmu_wdog_resume(struct platform_device *pdev)
{
	struct bcmpmu_wdog *wddog = platform_get_drvdata(pdev);
	pr_debug("Inside %s\n", __func__);

	if (wddog->wd_is_opened)
		queue_delayed_work(wddog->wd_workqueue, &wddog->wd_work,
				EARLY_PANIC_WORK_DELAY);

	return bcmpmu_wdog_reset(wddog);
}

static struct platform_driver bcmpmu_wdog_driver = {
	.driver = {
		   .name = "bcmpmu-wdog",
		   .owner = THIS_MODULE,
		   },
	.remove = __devexit_p(bcmpmu_wdog_remove),
	.probe = bcmpmu_wdog_probe,
	.suspend = bcmpmu_wdog_suspend,
	.resume = bcmpmu_wdog_resume
};

static int __init bcmpmu_wdog_init(void)
{
	return platform_driver_register(&bcmpmu_wdog_driver);
}

subsys_initcall(bcmpmu_wdog_init);

static void __exit bcmpmu_wdog_exit(void)
{
	platform_driver_unregister(&bcmpmu_wdog_driver);
}

module_exit(bcmpmu_wdog_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Watchdog Driver for Broadcom bcmpmu PMU");
MODULE_AUTHOR("TKG");

