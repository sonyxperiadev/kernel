/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
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

#include <linux/mfd/bcmpmu.h>

extern void rtc_sysfs_add_device(struct rtc_device *rtc);

#define BCMPMU_PRINT_ERROR (1U << 0)
#define BCMPMU_PRINT_INIT (1U << 1)
#define BCMPMU_PRINT_FLOW (1U << 2)
#define BCMPMU_PRINT_DATA (1U << 3)
static int dbg_mask = BCMPMU_PRINT_ERROR | BCMPMU_PRINT_INIT;

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
	return sprintf(buf, "%X\n", dbg_mask);
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
	struct bcmpmu *bcmpmu;
	wait_queue_head_t wait;
	struct mutex lock;
	int alarm_irq_enabled;
	int update_irq_enabled;
};

#ifdef CONFIG_BCM_RTC_CAL
static bool rtc_cal_run = false;
extern int bcm_rtc_cal_read_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm);
extern int bcm_rtc_cal_set_time(struct bcmpmu_rtc *rdata, struct rtc_time *tm);
extern void bcm_rtc_cal_init(struct bcmpmu_rtc *rdata);
extern void bcm_rtc_cal_shutdown(void);
#endif /* CONFIG_BCM_RTC_CAL*/

static void bcmpmu_rtc_isr(enum bcmpmu_irq irq, void *data)
{
	struct bcmpmu_rtc *rdata = data;

	switch (irq) {
		case PMU_IRQ_RTC_ALARM:
			rtc_update_irq(rdata->rtc, 1, RTC_IRQF | RTC_AF);
			pr_rtc(FLOW, "%s: RTC interrupt Alarm\n",__func__);
			break;
		case PMU_IRQ_RTC_SEC:
			rtc_update_irq(rdata->rtc, 1, RTC_IRQF | RTC_UF);
			pr_rtc(FLOW, "%s: RTC interrupt Sec\n",__func__);
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
	if (enabled)
		ret = rdata->bcmpmu->unmask_irq(rdata->bcmpmu,
							PMU_IRQ_RTC_ALARM);
	else
		ret = rdata->bcmpmu->mask_irq(rdata->bcmpmu, PMU_IRQ_RTC_ALARM);
	if (unlikely(ret))
		goto err;
	rdata->alarm_irq_enabled = enabled;
err:
	return ret;
}

static int bcmpmu_read_time(struct device *dev, struct rtc_time *tm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret = 0;
	unsigned int val;

	mutex_lock(&rdata->lock);

#ifdef CONFIG_BCM_RTC_CAL
	if(rtc_cal_run == false)
	{
		pr_rtc(DATA, "%s: rtc_cal not ready \n",__func__);
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR,
						&val, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		tm->tm_year = val + 100;
		
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT,
						&val, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		if (val >= 1)
			tm->tm_mon = val - 1;
		
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT,
						&val, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		tm->tm_mday = val;
		
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR,
						&val, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		tm->tm_hour = val;
		
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN,
						&val, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		tm->tm_min = val;
		
		ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC,
			&val, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		tm->tm_sec = val;
		
		ret = rtc_valid_tm(tm);
		
		pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
			__func__, ret,
			tm->tm_year,tm->tm_mon,tm->tm_mday,
			tm->tm_hour,tm->tm_min,tm->tm_sec);

	}
	else
	{
		ret = bcm_rtc_cal_read_time(rdata, tm);
	}
#else
	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_year = val + 100;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	if (val >= 1)
		tm->tm_mon = val - 1;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_mday = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_hour = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_min = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC,
		&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	tm->tm_sec = val;
	
	ret = rtc_valid_tm(tm);

	pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
		__func__, ret,
		tm->tm_year,tm->tm_mon,tm->tm_mday,
		tm->tm_hour,tm->tm_min,tm->tm_sec);
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
		__func__,
		tm->tm_year,tm->tm_mon,tm->tm_mday,
		tm->tm_hour,tm->tm_min,tm->tm_sec);

	mutex_lock(&rdata->lock);

#ifdef CONFIG_BCM_RTC_CAL
	if(rtc_cal_run == false)
	{
		pr_rtc(DATA, "%s: rtc_cal not ready \n",__func__);
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
					tm->tm_year - 100, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT,
					tm->tm_mon + 1, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
					tm->tm_mday, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
					tm->tm_hour, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
					tm->tm_min, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
		
		ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
					tm->tm_sec, PMU_BITMASK_ALL);
		if (unlikely(ret))
			goto err;
	}
	else
	{
		ret = bcm_rtc_cal_set_time(rdata, tm);
	}
#else
	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR,
				tm->tm_year - 100, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT,
				tm->tm_mon + 1, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT,
				tm->tm_mday, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR,
				tm->tm_hour, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN,
				tm->tm_min, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC,
				tm->tm_sec, PMU_BITMASK_ALL);
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
	unsigned int val;

	mutex_lock(&rdata->lock);

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCYR_ALM,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_year = val + 100;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMT_ALM,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	if (val >= 1)
		alarm->time.tm_mon = val - 1;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCDT_ALM,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_mday = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCHR_ALM,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_hour = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCMN_ALM,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_min = val;

	ret = rdata->bcmpmu->read_dev(rdata->bcmpmu, PMU_REG_RTCSC_ALM,
					&val, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;
	alarm->time.tm_sec = val;

	alarm->enabled = rdata->alarm_irq_enabled;
	ret = rtc_valid_tm(&alarm->time);

	pr_rtc(DATA, "%s: err=%d time=%d.%d.%d.%d.%d.%d\n",
		__func__, ret,
		alarm->time.tm_year,alarm->time.tm_mon,alarm->time.tm_mday,
		alarm->time.tm_hour,alarm->time.tm_min,alarm->time.tm_sec);

err:
	mutex_unlock(&rdata->lock);
	return ret;
}

static int bcmpmu_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct bcmpmu_rtc *rdata = dev_get_drvdata(dev);
	int ret;

	pr_rtc(DATA, "%s: time=%d.%d.%d.%d.%d.%d\n",
		__func__,
		alarm->time.tm_year,alarm->time.tm_mon,alarm->time.tm_mday,
		alarm->time.tm_hour,alarm->time.tm_min,alarm->time.tm_sec);
						
	mutex_lock(&rdata->lock);
	
	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCYR_ALM,
				alarm->time.tm_year - 100, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMT_ALM,
				alarm->time.tm_mon + 1, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCDT_ALM,
				alarm->time.tm_mday, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCHR_ALM,
				alarm->time.tm_hour, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCMN_ALM,
				alarm->time.tm_min, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	ret = rdata->bcmpmu->write_dev(rdata->bcmpmu, PMU_REG_RTCSC_ALM,
				alarm->time.tm_sec, PMU_BITMASK_ALL);
	if (unlikely(ret))
		goto err;

	bcmpmu_alarm_irq_enable(dev, alarm->enabled);
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

static int __devinit bcmpmu_rtc_probe(struct platform_device *pdev)
{
	int ret = 0;
	unsigned int val;

	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
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
	device_init_wakeup(&pdev->dev, 1);
	platform_set_drvdata(pdev, rdata);
	rdata->rtc = rtc_device_register(pdev->name,
			&pdev->dev, &bcmpmu_rtc_ops, THIS_MODULE);
	if (IS_ERR(rdata->rtc)) {
		ret = PTR_ERR(rdata->rtc);
		goto err;
	}

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

#ifdef CONFIG_BCM_RTC_CAL
	bcm_rtc_cal_init(rdata);
	rtc_cal_run = true;
#endif /*CONFIG_BCM_RTC_CAL*/

	/* Workarond the invalid value, to be removed after RTCADJ interrupt
	is handled properly */
	bcmpmu->read_dev(bcmpmu, PMU_REG_RTCDT, &val, PMU_BITMASK_ALL);
	if (val == 0) {
		bcmpmu->write_dev(bcmpmu, PMU_REG_RTCDT, 1, PMU_BITMASK_ALL);
		bcmpmu->write_dev(bcmpmu, PMU_REG_RTCYR, 0, PMU_BITMASK_ALL);
	}

	device_set_wakeup_capable(&pdev->dev, 1);
	rtc_sysfs_add_device(rdata->rtc);
	ret = device_create_file(&rdata->rtc->dev, &dev_attr_dbgmask);
	
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

static int __devexit bcmpmu_rtc_remove(struct platform_device *pdev)
{
	struct bcmpmu *bcmpmu = pdev->dev.platform_data;
	struct bcmpmu_rtc *rdata = platform_get_drvdata(pdev);

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

static int bcmpmu_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	/* no action required */
	pr_rtc(FLOW, "%s: ####\n", __func__);
	return 0;
}

static int bcmpmu_rtc_resume(struct platform_device *pdev)
{
	/* disable the RTC ALRM interrupt
	 * as android will take care of it now.
	*/
	pr_rtc(FLOW, "%s: ####\n", __func__);
	bcmpmu_alarm_irq_enable(&pdev->dev, false);
	return 0;
}

static struct platform_driver bcmpmu_rtc_driver = {
	.driver = {
		.name = "bcmpmu_rtc",
	},
	.probe = bcmpmu_rtc_probe,
	.remove = __devexit_p(bcmpmu_rtc_remove),
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
