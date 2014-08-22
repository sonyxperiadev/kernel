/*****************************************************************************
*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include <linux/mfd/bcmpmu59xxx_reg.h>
#include "rtc-core.h"

static int dbg_mask = BCMPMU_PRINT_ERROR |
		BCMPMU_PRINT_INIT | BCMPMU_PRINT_DATA | BCMPMU_PRINT_FLOW;

#define SEC_YEAR_BASE 			13  /* 2013 */

static void bcmpmu_rtc_time_fixup(struct device *dev);
#define pr_rtc(debug_level, args...) \
	do { \
		if (dbg_mask & BCMPMU_PRINT_##debug_level) { \
			pr_info(args); \
		} \
	} while (0)
static ssize_t
bcmpmu_rtc_show_dbgmsk(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return snprintf(buf, 5, "%x\n", dbg_mask);
}
static ssize_t
bcmpmu_rtc_set_dbsmsk(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long val = simple_strtoul(buf, NULL, 0);
	if (val > 0xFF || val == 0)
		return -EINVAL;
	dbg_mask = val;
	return n;
}
static DEVICE_ATTR(dbgmask, S_IRUGO | S_IWUSR,
		bcmpmu_rtc_show_dbgmsk, bcmpmu_rtc_set_dbsmsk);

struct bcmpmu_rtc {
	struct rtc_device *rtc;
	struct bcmpmu59xxx *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
	int alarm_irq_enabled;
	int update_irq_enabled;
#ifdef CONFIG_BCM_RTC_ALARM_BOOT
	struct work_struct work;
#endif /*CONFIG_BCM_RTC_ALARM_BOOT*/
};

#ifdef CONFIG_BCM_RTC_CAL
static bool rtc_cal_run;
extern int bcm_rtc_cal_read_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm);
extern int bcm_rtc_cal_set_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm);
extern void bcm_rtc_cal_init(struct bcmpmu_rtc *rdata);
extern void bcm_rtc_cal_shutdown(void);
#endif /* CONFIG_BCM_RTC_CAL*/

#ifdef CONFIG_BCM_RTC_ALARM_BOOT
static void bcmpmu_alarm_notify(struct work_struct *work)
{
	struct bcmpmu_rtc *rdata =
		container_of(work, struct bcmpmu_rtc, work);

	kobject_uevent(&rdata->rtc->dev.kobj, KOBJ_CHANGE);
}
#endif /*CONFIG_BCM_RTC_ALARM_BOOT*/


static void bcmpmu_rtc_isr(enum bcmpmu59xxx_irq irq, void *data)
{
	struct bcmpmu_rtc *rdata = data;

	switch (irq) {
	case PMU_IRQ_RTC_ALARM:
		rtc_update_irq(rdata->rtc, 1, RTC_IRQF | RTC_AF);
		pr_rtc(FLOW, "%s: RTC interrupt Alarm\n", __func__);
#ifdef CONFIG_BCM_RTC_ALARM_BOOT
		schedule_work(&rdata->work);
#endif /*CONFIG_BCM_RTC_ALARM_BOOT*/
		break;
	case PMU_IRQ_RTC_SEC:
		rtc_update_irq(rdata->rtc, 1, RTC_IRQF | RTC_UF);
		pr_rtc(FLOW, "%s: RTC interrupt Sec\n", __func__);
		break;
	default:
		break;
	}
}

static int bcmpmu_alarm_irq_enable(struct device *dev,
		unsigned int enabled)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret = 0;
	mutex_lock(&rdata->lock);

	pr_rtc(FLOW, "%s: RTC alarm %d\n", __func__, enabled);
	if (enabled)
		ret = rdata->bcmpmu->unmask_irq(rdata->bcmpmu,
						PMU_IRQ_RTC_ALARM);
	else {
		ret = rdata->bcmpmu->mask_irq(rdata->bcmpmu,
				PMU_IRQ_RTC_ALARM);
		if (unlikely(ret))
			goto err;

		/* Alarm irq is enabled once PMU is powered on,
		 * clear the alarm time here to avoid the alarm
		 * firing at unwanted time.
		 */
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu,
				PMU_REG_RTCYR_A1, (u8)0xFF);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu,
				PMU_REG_RTCMT_A1, 1);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu,
				PMU_REG_RTCDT_A1, 0);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu,
				PMU_REG_RTCHR_A1, 0);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu,
				PMU_REG_RTCMN_A1, 0);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu,
				PMU_REG_RTCSC_A1, 0);
	}

	if (unlikely(ret))
		goto err;

	rdata->alarm_irq_enabled = enabled;
err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_read_time(struct device *dev, struct rtc_time *tm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret = 0;
	u8 val;

	mutex_lock(&rdata->lock);
#ifdef CONFIG_BCM_RTC_CAL
	if (rtc_cal_run == false) {
		pr_rtc(DATA, "%s: rtc_cal not ready\n", __func__);
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu,
						PMU_REG_RTCYR, &val);
		if (unlikely(ret))
			goto err;
		tm->tm_year = val + 100;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu,
						PMU_REG_RTCMT_WD, &val);
		if (unlikely(ret))
			goto err;
		if (val >= 1)
			tm->tm_mon = val - 1;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu,
						PMU_REG_RTCDT, &val);
		if (unlikely(ret))
			goto err;
		tm->tm_mday = val;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR,
						&val);
		if (unlikely(ret))
			goto err;
		tm->tm_hour = val;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN,
						&val);
		if (unlikely(ret))
			goto err;
		tm->tm_min = val;

		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC,
						&val);
		if (unlikely(ret))
			goto err;
		tm->tm_sec = val;

		ret = rtc_valid_tm(tm);

		pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
			__func__, ret, tm->tm_year, tm->tm_mon,
			tm->tm_mday, tm->tm_hour, tm->tm_min,
			tm->tm_sec);

	} else {
		ret = bcm_rtc_cal_read_time(rdata, tm);
	}
#else
	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_year = val + 100;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT_WD,
					&val);
	if (unlikely(ret))
		goto err;
	if (val >= 1)
		tm->tm_mon = (val & 0xF) - 1;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_mday = (val & 0x1F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_hour = (val & 0x1F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_min = (val & 0x3F);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC,
					&val);
	if (unlikely(ret))
		goto err;
	tm->tm_sec = (val & 0x3F);

	ret = rtc_valid_tm(tm);

	pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
		__func__, ret, tm->tm_year, tm->tm_mon,
		tm->tm_mday, tm->tm_hour, tm->tm_min,
		tm->tm_sec);
#endif /* CONFIG_BCM_RTC_CAL*/


err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_set_time(struct device *dev, struct rtc_time *tm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;

	pr_rtc(DATA, "%s: time=%d.%d.%d.%d.%d.%d\n",
		__func__, tm->tm_year, tm->tm_mon,
		tm->tm_mday, tm->tm_hour,
		tm->tm_min, tm->tm_sec);

	mutex_lock(&rdata->lock);

#ifdef CONFIG_BCM_RTC_CAL
	if (rtc_cal_run == false) {
		pr_rtc(DATA, "%s: rtc_cal not ready\n", __func__);
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
					tm->tm_year - 100);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_WD,
					tm->tm_mon + 1);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
					tm->tm_mday);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
					tm->tm_hour);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
					tm->tm_min);
		if (unlikely(ret))
			goto err;

		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
					tm->tm_sec);
		if (unlikely(ret))
			goto err;
	} else {
		ret = bcm_rtc_cal_set_time(rdata, tm);
	}
#else
	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
				tm->tm_year - 100);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_WD,
				tm->tm_mon + 1);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
				tm->tm_mday);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
				tm->tm_hour);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
				tm->tm_min);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
				tm->tm_sec);
	if (unlikely(ret))
		goto err;
#endif /* CONFIG_BCM_RTC_CAL*/

err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;
	u8 val;

	mutex_lock(&rdata->lock);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_year = val + 100;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT_A1,
					&val);
	if (unlikely(ret))
		goto err;
	if (val >= 1)
		alarm->time.tm_mon = val - 1;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_mday = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_hour = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_min = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC_A1,
					&val);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_sec = val;

	alarm->enabled = rdata->alarm_irq_enabled;
	ret = rtc_valid_tm(&alarm->time);

	pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
		__func__, ret, alarm->time.tm_year, alarm->time.tm_mon,
		alarm->time.tm_mday, alarm->time.tm_hour,
		alarm->time.tm_min, alarm->time.tm_sec);

err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;

	pr_rtc(DATA, "%s: time=%d.%d.%d.%d.%d.%d\n",
		__func__, alarm->time.tm_year, alarm->time.tm_mon,
		alarm->time.tm_mday, alarm->time.tm_hour,
		alarm->time.tm_min, alarm->time.tm_sec);
	mutex_lock(&rdata->lock);

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR_A1,
				alarm->time.tm_year - 100);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_A1,
				alarm->time.tm_mon + 1);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT_A1,
				alarm->time.tm_mday);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR_A1,
				alarm->time.tm_hour);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN_A1,
				alarm->time.tm_min);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC_A1,
				alarm->time.tm_sec);
	if (unlikely(ret))
		goto err;

#ifdef CONFIG_BCM_RTC_ALARM_BOOT
	if (alarm->enabled)
		ret = rdata->bcmpmu->unmask_irq(rdata->bcmpmu,
				PMU_IRQ_RTC_ALARM);
	else
		ret = rdata->bcmpmu->mask_irq(rdata->bcmpmu, PMU_IRQ_RTC_ALARM);
	rdata->alarm_irq_enabled = alarm->enabled;
#endif
err:
	mutex_unlock(&rdata->lock);
	return ret;
}

/*
static int bcmpmu_update_irq_enable(struct device *dev,
		unsigned int enabled)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret = 0;
	if (enabled)
		ret = rdata->bcmpmu->unmask_irq(rdata->bcmpmu, PMU_IRQ_RTC_SEC);
	else
		ret = rdata->bcmpmu->mask_irq(rdata->bcmpmu, PMU_IRQ_RTC_SEC);
	if (unlikely(ret))
		goto err;
	rdata->update_irq_enabled = enabled;
err:
	return ret;
}
*/


static struct rtc_class_ops bcmpmu_rtc_ops = {
	.read_time		= bcmpmu_read_time,
	.set_time		= bcmpmu_set_time,
	.read_alarm		= bcmpmu_read_alarm,
	.set_alarm		= bcmpmu_set_alarm,
	.alarm_irq_enable	= bcmpmu_alarm_irq_enable,
};

static int bcmpmu_rtc_probe(struct platform_device *pdev)
{
	int ret = 0;
	u8 val;

	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_rtc *rdata;

	pr_rtc(INIT, "%s: called.\n", __func__);

	rdata = kzalloc(sizeof(struct bcmpmu_rtc), GFP_KERNEL);
	if (rdata == NULL) {
		dev_err(&pdev->dev, "failed to alloc mem\n");
		return -ENOMEM;
	}
	rdata->bcmpmu = bcmpmu;

	init_waitqueue_head(&rdata->wait);
	mutex_init(&rdata->lock);
	bcmpmu->rtcinfo = (void *)rdata;
	rdata->update_irq_enabled = 0;
	rdata->alarm_irq_enabled = 0;

	platform_set_drvdata(pdev, rdata);
	device_init_wakeup(&pdev->dev, 1);
	rdata->rtc = rtc_device_register(pdev->name,
			&pdev->dev, &bcmpmu_rtc_ops, THIS_MODULE);
	if (IS_ERR(rdata->rtc)) {
		ret = PTR_ERR(rdata->rtc);
		goto err;
	}

#ifdef CONFIG_BCM_RTC_ALARM_BOOT
	INIT_WORK(&rdata->work, bcmpmu_alarm_notify);
#endif /*CONFIG_BCM_RTC_ALARM_BOOT*/

	ret = bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTC_ALARM,
			bcmpmu_rtc_isr, rdata);
	if (ret) {
		pr_rtc(ERROR, "In %s: Unable to allocate Alarm IRQ: %d.\n",
						__func__, PMU_IRQ_RTC_ALARM);
		goto err_irq_alarm;
	}
	ret = bcmpmu->register_irq(bcmpmu, PMU_IRQ_RTC_SEC,
			bcmpmu_rtc_isr, rdata);
	if (ret) {
		pr_rtc(ERROR, "In %s: Unable to allocate SEC IRQ: %d.\n",
						__func__, PMU_IRQ_RTC_SEC);
		goto err_irq_sec;
	}
	ret = bcmpmu->mask_irq(bcmpmu, PMU_IRQ_RTC_SEC);
	if (ret)
		pr_rtc(ERROR, "%s: Failed to disable RTC 1S interrupt\n",
						__func__);
#ifdef CONFIG_BCM_RTC_CAL
	bcm_rtc_cal_init(rdata);
	rtc_cal_run = true;
#endif /*CONFIG_BCM_RTC_CAL*/

	/* Workarond the invalid value, to be removed after RTCADJ interrupt
	is handled properly */
	bcmpmu->read_dev(bcmpmu, PMU_REG_RTCDT, &val);
	if (val == 0) {
		bcmpmu->write_dev(bcmpmu, PMU_REG_RTCDT, 1);
		bcmpmu->write_dev(bcmpmu, PMU_REG_RTCYR, 0);
		bcmpmu_rtc_time_fixup(&pdev->dev);
	}

	ret = device_create_file(&rdata->rtc->dev, &dev_attr_dbgmask);
#ifdef CONFIG_BCM_RTC_ALARM_BOOT
	bcmpmu_alarm_irq_enable(&pdev->dev, 1);
#endif /*CONFIG_BCM_RTC_ALARM_BOOT*/
	return 0;

err_irq_sec:
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_ALARM);
err_irq_alarm:
	rtc_device_unregister(rdata->rtc);
err:
	platform_set_drvdata(pdev, NULL);
	kfree(rdata);
	return ret;
}

static int bcmpmu_rtc_remove(struct platform_device *pdev)
{
	struct bcmpmu59xxx *bcmpmu = dev_get_drvdata(pdev->dev.parent);
	struct bcmpmu_rtc *rdata = (struct bcmpmu_rtc *)bcmpmu->rtcinfo ;

	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_ALARM);
	bcmpmu->unregister_irq(bcmpmu, PMU_IRQ_RTC_SEC);
#ifdef CONFIG_BCM_RTC_CAL
	rtc_cal_run = false;
	bcm_rtc_cal_shutdown();
#else /*CONFIG_BCM_RTC_CAL*/
	rtc_device_unregister(rdata->rtc);
#endif
	kfree(bcmpmu->rtcinfo);
	return 0;
}


static void bcmpmu_rtc_time_fixup(struct device *dev)
{
	struct rtc_time current_rtc_time;
	memset(&current_rtc_time, 0 , sizeof(struct rtc_time));

	bcmpmu_read_time(dev, &current_rtc_time);
	current_rtc_time.tm_year += SEC_YEAR_BASE;
	bcmpmu_set_time(dev, &current_rtc_time);
}

static int bcmpmu_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* no action required */
	pr_rtc(FLOW, "%s: ####\n", __func__);
#ifndef CONFIG_BCM_RTC_ALARM_BOOT
	bcmpmu_alarm_irq_enable(&pdev->dev, true);
#endif

	return 0;
}

static int bcmpmu_rtc_resume(struct platform_device *pdev)
{
	/* disable the RTC ALRM interrupt
	 * as android will take care of it now.
	*/
	pr_rtc(FLOW, "%s: ####\n", __func__);
#ifndef CONFIG_BCM_RTC_ALARM_BOOT
	bcmpmu_alarm_irq_enable(&pdev->dev, false);
#endif
	return 0;
}

static void bcmpmu_rtc_shutdown(struct platform_device *pdev)
{
	pr_rtc(FLOW, "%s: ####\n", __func__);
#ifndef CONFIG_BCM_RTC_ALARM_BOOT
	bcmpmu_alarm_irq_enable(&pdev->dev, false);
#endif
}


static struct platform_driver bcmpmu_rtc_driver = {
	.shutdown = bcmpmu_rtc_shutdown,
	.driver = {
		.name = "bcmpmu59xxx_rtc",
	},
	.probe = bcmpmu_rtc_probe,
	.remove = bcmpmu_rtc_remove,
	.suspend = bcmpmu_rtc_suspend,
	.resume = bcmpmu_rtc_resume,
};

static int __init bcmpmu_rtc_init(void)
{
	return platform_driver_register(&bcmpmu_rtc_driver);
}
module_init(bcmpmu_rtc_init);

static void __exit bcmpmu_rtc_exit(void)
{
	platform_driver_unregister(&bcmpmu_rtc_driver);
}
module_exit(bcmpmu_rtc_exit);

MODULE_DESCRIPTION("BCM PMIC RTC driver");
MODULE_LICENSE("GPL");
