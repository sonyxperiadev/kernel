/*
 * Copyright 2012 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php, or by writing to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
/*
 * Notes:
 * This rtc-kona is a fake RTC device based on kona timer, as RTC in PMU
 * can turn on the phone if phone is in shutdown state, so we can't use RTC
 * in PMU as the alarm device, then a new RTC is needed to do the alarm job,
 * at the same time, RTC in PMU can use as power-off alarm.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <mach/kona_timer.h>
#include <mach/timex.h>
#include "rtc-core.h"



#define PRINT_ERROR (1U << 0)
#define PRINT_INIT (1U << 1)
#define PRINT_FLOW (1U << 2)
#define PRINT_DATA (1U << 3)
#define PRINT_INFO (1U << 4)
static int dbg_mask = PRINT_ERROR | PRINT_INIT | PRINT_DATA | PRINT_INFO;
module_param_named(dbg_mask, dbg_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

#define pr_rtc(debug_level, args...) \
		do { \
			if (dbg_mask & PRINT_##debug_level) { \
				pr_info(args); \
			} \
		} while (0)

#define KONA_TIMER_NAME		"aon-timer"
#define KONA_TIMER_CHAN		1

/* Seconds to ticks conversion */
#define sec_to_ticks(x) ((x)*CLOCK_TICK_RATE)
#define ticks_to_sec(x) ((x)/CLOCK_TICK_RATE)

struct rtc_kona {
	struct kona_timer *kt;
	int alarm_irq_enabled;
	struct rtc_device *rtc;
	struct rtc_device *real_rtc;
};

static int get_timekeeping_rtc_time(struct rtc_kona *rtc_info,
	struct rtc_time *tm)
{
	if (!rtc_info->real_rtc)
		rtc_info->real_rtc = rtc_class_open(
			CONFIG_RTC_HCTOSYS_DEVICE);

	if (rtc_info->real_rtc == NULL) {
		struct timespec now;
		/*no rtc avaliable, temply use SW time*/
		pr_rtc(INFO, "use SW time!\n");
		getnstimeofday(&now);
		rtc_time_to_tm(now.tv_sec, tm);
	} else {
		rtc_read_time(rtc_info->real_rtc, tm);
	}

	return 0;
}

static int set_timekeeping_rtc_time(struct rtc_kona *rtc_info,
	struct rtc_time *tm)
{
	if (!rtc_info->real_rtc)
		rtc_info->real_rtc = rtc_class_open(
			CONFIG_RTC_HCTOSYS_DEVICE);

	if (rtc_info->real_rtc)
		rtc_set_time(rtc_info->real_rtc, tm);
	else
		pr_rtc(INFO, "rtc time will lost after reboot!\n");

	return 0;
}

static int rtc_kona_alarm_call_back(void *data)
{
	struct rtc_kona *rtc_info = (struct rtc_kona *)data;

	rtc_update_irq(rtc_info->rtc, 1, RTC_IRQF | RTC_AF);

	pr_rtc(FLOW, "rtc kona alarm!\n");

	return 0;
}

static int rtc_kona_read_time(struct device *dev, struct rtc_time *tm)
{
	struct rtc_kona *rtc_info = dev_get_drvdata(dev);
	int ret = 0;

	get_timekeeping_rtc_time(rtc_info, tm);

	ret = rtc_valid_tm(tm);

	pr_rtc(DATA, "err=%d now time=%d.%d.%d.%d.%d.%d\n", ret,
		tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	return ret;
}

static int rtc_kona_set_time(struct device *dev, struct rtc_time *tm)
{
	struct rtc_kona *rtc_info = dev_get_drvdata(dev);
	int ret = 0;

	pr_rtc(DATA, "%s: time=%d.%d.%d.%d.%d.%d\n",
		__func__,
		tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	set_timekeeping_rtc_time(rtc_info, tm);

	return ret;
}

static int rtc_kona_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct rtc_kona *rtc_info = dev_get_drvdata(dev);
	unsigned int count;
	struct rtc_time tm;
	unsigned long now_time;

	count = kona_timer_get_counter(rtc_info->kt);

	get_timekeeping_rtc_time(rtc_info, &tm);

	rtc_tm_to_time(&tm, &now_time);

	rtc_time_to_tm(now_time + ticks_to_sec(count), &alarm->time);

	alarm->enabled = rtc_info->alarm_irq_enabled;

	pr_rtc(DATA, "alarm time=%d.%d.%d.%d.%d.%d\n",
		alarm->time.tm_year, alarm->time.tm_mon, alarm->time.tm_mday,
		alarm->time.tm_hour, alarm->time.tm_min, alarm->time.tm_sec);

	return 0;
}

static int rtc_kona_alarm_irq_enable(struct device *dev,
		unsigned int enabled)
{
	struct rtc_kona *rtc_info = dev_get_drvdata(dev);
	int ret = 0;

	pr_rtc(FLOW, "alarm %s\n", enabled ? "enabled" : "disabled");
	if (!enabled)
		ret = kona_timer_disable_and_clear(rtc_info->kt);
	rtc_info->alarm_irq_enabled = enabled;
	return ret;
}

static int rtc_kona_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct rtc_kona *rtc_info = dev_get_drvdata(dev);
	int ret = 0;
	unsigned long alarm_time, now_time;
	unsigned long count;
	struct rtc_time tm;

	pr_rtc(DATA, "%s: time=%d.%d.%d.%d.%d.%d\n", __func__,
		alarm->time.tm_year, alarm->time.tm_mon, alarm->time.tm_mday,
		alarm->time.tm_hour, alarm->time.tm_min, alarm->time.tm_sec);

	rtc_tm_to_time(&alarm->time, &alarm_time);
	get_timekeeping_rtc_time(rtc_info, &tm);
	rtc_tm_to_time(&tm, &now_time);

	if (alarm_time <= now_time) {
		pr_rtc(ERROR, "invalid alarm, now %ld, alarm %ld passed\n",
			now_time, alarm_time);
		return -EINVAL;
	}

	count = alarm_time - now_time;

	pr_rtc(INFO, "%s: set alarm count %ld\n", __func__, count);
	if (alarm->enabled) {
		ret = kona_timer_set_match_start(rtc_info->kt,
			sec_to_ticks(count));
		if (ret) {
			pr_rtc(ERROR, "set alarm failed!\n");
			return -EINVAL;
		}
	}

	rtc_kona_alarm_irq_enable(dev, alarm->enabled);

	return ret;
}

static struct rtc_class_ops rtc_kona_ops = {
	.read_time		= rtc_kona_read_time,
	.set_time		= rtc_kona_set_time,
	.read_alarm		= rtc_kona_read_alarm,
	.set_alarm		= rtc_kona_set_alarm,
	.alarm_irq_enable	= rtc_kona_alarm_irq_enable,
};

static int __devinit rtc_kona_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct rtc_kona *rtc_info;
	struct timer_ch_cfg cfg;

	pr_rtc(INIT, "rtc_kona_probe!\n");

	rtc_info = kzalloc(sizeof(struct rtc_kona), GFP_KERNEL);
	if (!rtc_info) {
		pr_rtc(ERROR, "no memory avaliable!\n");
		return -ENOMEM;
	}

	rtc_info->kt = kona_timer_request(KONA_TIMER_NAME,
		KONA_TIMER_CHAN);
	if (!rtc_info->kt) {
		pr_rtc(ERROR, "no kona timer channel avaliable %d\n",
			KONA_TIMER_CHAN);
		ret = -ENODEV;
		goto __error;
	}

	cfg.arg = rtc_info;
	cfg.mode = MODE_ONESHOT;
	cfg.cb = rtc_kona_alarm_call_back;

	ret = kona_timer_config(rtc_info->kt, &cfg);
	if (ret) {
		pr_rtc(ERROR, "config failed %d\n", ret);
		goto __error;
	}

	platform_set_drvdata(pdev, rtc_info);
	device_init_wakeup(&pdev->dev, 1);

	rtc_info->rtc = rtc_device_register(pdev->name, &pdev->dev,
		&rtc_kona_ops, THIS_MODULE);
	if (IS_ERR(rtc_info->rtc)) {
		ret = PTR_ERR(rtc_info->rtc);
		pr_rtc(ERROR, "register rtc device failed, %d\n", ret);
		goto __error;
	}

	pr_rtc(INIT, "rtc-kona initialized.\n");

	return ret;

__error:
	if (rtc_info->kt)
		kona_timer_free(rtc_info->kt);
	kfree(rtc_info);
	return ret;
}

static int __devexit rtc_kona_remove(struct platform_device *pdev)
{
	struct rtc_kona *rtc_info = platform_get_drvdata(pdev);

	if (!IS_ERR(rtc_info->rtc))
		rtc_device_unregister(rtc_info->rtc);

	if (rtc_info->kt)
		kona_timer_free(rtc_info->kt);

	kfree(rtc_info);

	return 0;
}

static struct platform_driver rtc_kona_driver = {
	.driver = {
		.name = "rtc_kona",
	},
	.probe = rtc_kona_probe,
	.remove = __devexit_p(rtc_kona_remove),
};

static int __init rtc_kona_init(void)
{
	return platform_driver_register(&rtc_kona_driver);
}
subsys_initcall(rtc_kona_init);

static void __exit rtc_kona_exit(void)
{
	platform_driver_unregister(&rtc_kona_driver);
}
module_exit(rtc_kona_exit);

MODULE_DESCRIPTION("BCM RTC-KONA driver");
MODULE_LICENSE("GPL");

