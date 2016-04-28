/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/rtc/rtc-bcm59055.c
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
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/platform_device.h>
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#include <linux/mfd/bcm590xx/core.h>
#include <asm/io.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

struct bcm59055_rtc {
	struct bcm590xx *bcm590xx;
	struct rtc_device *rtc;
	int alarm_enabled;
};

/*
 * Read current time and date in RTC
 */
static int bcm59055_rtc_readtime(struct device *dev, struct rtc_time *tm)
{
	struct bcm59055_rtc *bcm59055_rtc = dev_get_drvdata(dev);
	struct bcm590xx *bcm590xx = bcm59055_rtc->bcm590xx;
	u8 regVal[7];
	int ret = 0;
	
	ret = bcm590xx_mul_reg_read(bcm590xx, BCM59055_REG_RTCSC, 7, &regVal[0]);
	/* Seconds*/
	tm->tm_sec = regVal[0];
	/*minutes*/
	tm->tm_min = regVal[1];
	/*hour*/
	tm->tm_hour = regVal[2];
	/*Day of month*/
	tm->tm_mday = regVal[4];
	/*month*/
	tm->tm_mon = regVal[5];
	/* Year */
	tm->tm_year = regVal[6] + 100;

	pr_debug("%s: year = %d\n", __func__, tm->tm_year);
	pr_debug("%s: mon  = %d\n", __func__, tm->tm_mon);
	pr_debug("%s: mday = %d\n", __func__, tm->tm_mday);
	pr_debug("%s: wday = %d\n", __func__, tm->tm_wday);
	pr_debug("%s: hour = %d\n", __func__, tm->tm_hour);
	pr_debug("%s: min  = %d\n", __func__, tm->tm_min);
	pr_debug("%s: sec  = %d\n", __func__, tm->tm_sec);

	if (ret < 0)
	    printk(KERN_ERR"%s: PMU read error !!! ret: %d \n", __func__, ret);
	else
	    ret = 0;
	
	return ret;
}

/*
 * Set current time and date in RTC
 */
static int bcm59055_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct bcm59055_rtc *bcm59055_rtc = dev_get_drvdata(dev);
	struct bcm590xx *bcm590xx = bcm59055_rtc->bcm590xx;
	int ret = 0, i;
	u8 regval[7];
	
	pr_debug("%s: year = %d\n", __func__, tm->tm_year);
	pr_debug("%s: mon  = %d\n", __func__, tm->tm_mon);
	pr_debug("%s: mday = %d\n", __func__, tm->tm_mday);
	pr_debug("%s: wday = %d\n", __func__, tm->tm_wday);
	pr_debug("%s: hour = %d\n", __func__, tm->tm_hour);
	pr_debug("%s: min  = %d\n", __func__, tm->tm_min);
	pr_debug("%s: sec  = %d\n", __func__, tm->tm_sec);

	for(i=0;i<7;i++)
	    regval[i] = 0;
	regval[0] = tm->tm_sec;
	regval[1] = tm->tm_min;
	regval[2] = tm->tm_hour;
	regval[4] = tm->tm_mday;
	regval[5] = tm->tm_mon;
	regval[6] = tm->tm_year - 100;
	
	ret = bcm590xx_mul_reg_write(bcm590xx, BCM59055_REG_RTCSC, 7, &regval[0]);
	
	return ret;
}

static int bcm59055_rtc_alarm_irq_enable(struct device *dev, unsigned enabled)
{
	int ret = 0;
	struct bcm59055_rtc *bcm59055_rtc = dev_get_drvdata(dev);
	struct bcm590xx *bcm590xx = bcm59055_rtc->bcm590xx;

	pr_debug("%s: enabled: %d\n", __func__, enabled);

	if (enabled)
		ret = bcm590xx_enable_irq(bcm590xx, BCM59055_IRQID_INT8_RTCA1);
	else
		ret = bcm590xx_disable_irq(bcm590xx, BCM59055_IRQID_INT8_RTCA1);

	if (ret < 0)
		pr_err("%s: irq enable/disable failed: %d\n", __func__, ret);

	return ret;
}

static int bcm59055_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alm)
{
	struct bcm59055_rtc *bcm59055_rtc = dev_get_drvdata(dev);
	struct bcm590xx *bcm590xx = bcm59055_rtc->bcm590xx;
	u8 regVal[7];
	int ret = 0;
	
	ret = bcm590xx_mul_reg_read(bcm590xx, BCM59055_REG_RTCSC_A1, 7, &regVal[0]);
	/* Seconds*/
	alm->time.tm_sec = regVal[0];
	/*minutes*/
	alm->time.tm_min = regVal[1];
	/*hour*/
	alm->time.tm_hour = regVal[2];
	/*Day of month*/
	alm->time.tm_mday = regVal[4];
	/*month*/
	alm->time.tm_mon = regVal[5];
	/* Year */
	alm->time.tm_year = regVal[6] + 100;

	if (ret < 0) {
		printk(KERN_ERR"bcm59055_rtc_read_alarm: PMU read error !!! ret: %d\n", ret);
	}

	printk(KERN_ERR"%s: PMU  ret: %d \n", __func__, ret);

	if (bcm59055_rtc->alarm_enabled)
		alm->enabled = 1;
	else
		alm->enabled = 0;

	pr_debug("%s: alm->year    = %d\n", __func__, alm->time.tm_year);
	pr_debug("%s: alm->mon     = %d\n", __func__, alm->time.tm_mon);
	pr_debug("%s: alm->mday    = %d\n", __func__, alm->time.tm_mday);
	pr_debug("%s: alm->wday    = %d\n", __func__, alm->time.tm_wday);
	pr_debug("%s: alm->hour    = %d\n", __func__, alm->time.tm_hour);
	pr_debug("%s: alm->min     = %d\n", __func__, alm->time.tm_min);
	pr_debug("%s: alm->sec     = %d\n", __func__, alm->time.tm_sec);
	pr_debug("%s: alm->enabled = %d\n", __func__, alm->enabled);
	pr_debug("%s: alm->pending = %d\n", __func__, alm->pending);

	return 0;
	return ret;
}

static int bcm59055_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alm)
{
	struct bcm59055_rtc *bcm59055_rtc = dev_get_drvdata(dev);
	struct bcm590xx *bcm590xx = bcm59055_rtc->bcm590xx;
	int ret = 0, i;
	u8 regval[7];
	
	if (alm->enabled) {
		pr_debug("%s: alm->year     = %d\n", __func__,
			alm->time.tm_year);
		pr_debug("%s: alm->mon      = %d\n", __func__,
			alm->time.tm_mon);
		pr_debug("%s: alm->mday     = %d\n", __func__,
			alm->time.tm_mday);
		pr_debug("%s: alm->wday     = %d\n", __func__,
			alm->time.tm_wday);
		pr_debug("%s: alm->hour     = %d\n", __func__,
			alm->time.tm_hour);
		pr_debug("%s: alm->min      = %d\n", __func__,
			alm->time.tm_min);
		pr_debug("%s: alm->sec      = %d\n", __func__,
			alm->time.tm_sec);
		pr_debug("%s: alm->enabled  = %d\n", __func__,
			alm->enabled);
		pr_debug("%s: alm->pending  = %d\n", __func__,
			alm->pending);
		
		for(i=0;i<7;i++)
		    regval[i] = 0;
		regval[0] = alm->time.tm_sec;
		regval[1] = alm->time.tm_min;
		regval[2] = alm->time.tm_hour;
		regval[4] = alm->time.tm_mday;
		regval[5] = alm->time.tm_mon;
		regval[6] = alm->time.tm_year - 100;
		
		ret = bcm590xx_mul_reg_write(bcm590xx, BCM59055_REG_RTCSC_A1, 7, &regval[0]);
		
	}

	bcm59055_rtc_alarm_irq_enable(dev, alm->enabled);

	return ret;
}

static void bcm59055_rtc_isr(int irq, void *data)
{
	unsigned long events = 0;
	struct bcm59055_rtc *bcm59055_rtc = data;
	pr_info("----bcm59055_rtc_isr----\n");
	switch(irq) {
	    case BCM59055_IRQID_INT8_RTCA1:
	    	events |= RTC_IRQF | RTC_AF;
		rtc_update_irq(bcm59055_rtc->rtc, 1, events);
		break;
	    case BCM59055_IRQID_INT8_RTCADJ:
		printk(KERN_ERR "RTC time is invalid and need to be adjusted \n");
		break;
	}
}

static const struct rtc_class_ops bcm59055_rtc_ops = {
	.read_time = bcm59055_rtc_readtime,
	.set_time = bcm59055_rtc_set_time,
	.read_alarm = bcm59055_rtc_read_alarm,
	.set_alarm = bcm59055_rtc_set_alarm,
	.alarm_irq_enable = bcm59055_rtc_alarm_irq_enable,
};

#ifdef CONFIG_PM
static int bcm59055_rtc_suspend(struct device *dev)
{
	return 0;
}

static int bcm59055_rtc_resume(struct device *dev)
{
#if defined(RTC_ANDROID_ALARM_WORKAROUND)
	/* This option selects temporary fix for alarm handling in 'Android'
	 * environment. This option enables code to disable alarm in the
	 * 'resume' handler of RTC driver. In the normal mode,
	 * android handles all alarms in software without using the RTC chip.
	 * Android sets the alarm in the rtc only in the suspend path (by
	 * calling .set_alarm with struct rtc_wkalrm->enabled set to 1).
	 * In the resume path, android tries to disable alarm by calling
	 * .set_alarm with struct rtc_wkalrm->enabled' field set to 0.
	 * But unfortunately, it memsets the rtc_wkalrm struct to 0, which
	 * causes the rtc lib to flag error and control does not reach this
	 * driver. Hence this workaround.
	 */
	bcm59055_rtc_alarm_irq_enable(dev, 0);
#endif

	return 0;
}

#else
#define bcm59055_rtc_suspend NULL
#define bcm59055_rtc_resume NULL
#endif

static int bcm59055_rtc_probe(struct platform_device *pdev)
{
	struct bcm590xx *bcm590xx = dev_get_drvdata(pdev->dev.parent);
	struct bcm59055_rtc *bcm59055_rtc;
	int ret = 0;

	pr_err("Inside %s\n", __FUNCTION__);

	bcm59055_rtc = kzalloc(sizeof(struct bcm59055_rtc), GFP_KERNEL);
	if (bcm59055_rtc == NULL) {
		pr_err("bcm59055_rtc_probe failed!! ..no memory!!!\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, bcm59055_rtc);
	bcm59055_rtc->bcm590xx = bcm590xx;

	device_init_wakeup(&pdev->dev, 1);

	ret = bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT8_RTCA1, true,
				bcm59055_rtc_isr, bcm59055_rtc);	
	
	ret = bcm590xx_request_irq(bcm590xx, BCM59055_IRQID_INT8_RTCADJ, true,
				bcm59055_rtc_isr, bcm59055_rtc);	
	
	if (ret < 0) {
		pr_err("bcm59055_rtc_probe IRQ register failed !!!\n");
		kfree(bcm59055_rtc);
		return ret;
	}
	bcm59055_rtc->rtc = rtc_device_register(pdev->name, &pdev->dev,
						&bcm59055_rtc_ops, THIS_MODULE);
	if (IS_ERR(bcm59055_rtc->rtc)) {
		pr_err("bcm59055_rtc_probe:rtc_device_register failed !!!\n");
		ret = bcm59055_rtc->rtc;
		kfree(bcm59055_rtc);
		return ret;

	}
	ret = bcm590xx_enable_irq(bcm590xx, BCM59055_IRQID_INT8_RTCADJ);
	
	return 0;

}

static int __devexit bcm59055_rtc_remove(struct platform_device *pdev)
{
	struct bcm59055_rtc *bcm59055_rtc = platform_get_drvdata(pdev);

	bcm590xx_free_irq(bcm59055_rtc->bcm590xx, BCM59055_IRQID_INT8_RTCA1);
	rtc_device_unregister(bcm59055_rtc->rtc);
	kfree(bcm59055_rtc);

	return 0;
}

static struct dev_pm_ops bcm59055_rtc_pm_ops = {
	.suspend = bcm59055_rtc_suspend,
	.resume = bcm59055_rtc_resume,

	.thaw = bcm59055_rtc_resume,
	.restore = bcm59055_rtc_resume,

	.poweroff = bcm59055_rtc_suspend,
};

static struct platform_driver bcm59055_rtc_driver = {
	.probe = bcm59055_rtc_probe,
	.remove = __devexit_p(bcm59055_rtc_remove),
	.driver = {
		   .name = "bcm59055-rtc",
		   .pm = &bcm59055_rtc_pm_ops,
		   },
};

static int __init bcm59055_rtc_init(void)
{
	return platform_driver_register(&bcm59055_rtc_driver);
}

module_init(bcm59055_rtc_init);

static void __exit bcm59055_rtc_exit(void)
{
	platform_driver_unregister(&bcm59055_rtc_driver);
}

module_exit(bcm59055_rtc_exit);

MODULE_DESCRIPTION("RTC driver for the Broadcom BCM59055 PMU");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcm59055-rtc");

