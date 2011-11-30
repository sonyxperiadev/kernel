/* drivers/rtc/rtc-bcmhana.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * BCMHANA RTC Driver
 */

/*
 * Frameworks:
 *
 *    - SMP:          Fully supported.    Locking is in place where necessary.
 *    - GPIO:         Fully supported.    No GPIOs are used.
 *    - MMU:          Partiall done.      CHAL layer is broken needs interface like kona keypad
 *    - Dynamic /dev: Not applicable.
 *    - Suspend:      Not done.
 *    - Clocks:       Not done.           Need to move clock to platform data.
 *    - Power:        Not done.
 *
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>

#include <chal/chal_rtc.h>

struct island_rtc {
	CHAL_RTC_HANDLE_t *handle;
	struct rtc_device *dev;
	void __iomem *base;
	unsigned int irq1;
	unsigned int irq2;
	unsigned int max_user_freq;
	struct clk *clock;
};
struct island_rtc *rtc;
CHAL_RTC_HANDLE_t foo;

static unsigned int epoch = 1970;
static DEFINE_SPINLOCK(island_rtc_lock);

/* IRQ Handlers */

/*
 * RTC IRQ hanlder. This routine is invoked when a RTC oneshot timer completes
 */
static irqreturn_t rtc_alm_isr(int irq, void *data)
{
	struct island_rtc *rdev = data;

	/* Disable alarm interrupts because they are oneshot */
	chal_rtc_intDisable(rtc->handle, CHAL_RTC_INT_MATCH);
	chal_rtc_intStatusClr(rtc->handle, CHAL_RTC_INT_MATCH);
	if (chal_rtc_matchInterruptValGet(rtc->handle)) {
		pr_debug("%s: oneshot interrupted\n", __func__);
		rtc_update_irq(rdev->dev, 1, RTC_AF | RTC_IRQF);
	}
	return IRQ_HANDLED;
}

/*
 * RTC IRQ hanlder. This routine is invoked when periodic interrupts occur
 */
static irqreturn_t rtc_per_isr(int irq, void *data)
{
	struct island_rtc *rdev = data;
	pr_debug("%s: periodic interrupted\n", __func__);
	chal_rtc_intStatusClr(rtc->handle, CHAL_RTC_INT_PER);
	if (chal_rtc_periodInterruptValGet(rtc->handle)) {
		pr_debug("%s: periodic interrupted\n", __func__);
		rtc_update_irq(rdev->dev, 1, RTC_PF | RTC_IRQF);
	}
	return IRQ_HANDLED;
}

/* Update control registers */
static void island_rtc_setaie(int to)
{
	pr_debug("%s: aie=%d\n", __func__, to);

	spin_lock_irq(&island_rtc_lock);

	if (to) {
		chal_rtc_intEnable(rtc->handle, CHAL_RTC_INT_MATCH);
	} else {
		chal_rtc_intDisable(rtc->handle, CHAL_RTC_INT_MATCH);
	}
	spin_unlock_irq(&island_rtc_lock);
}

static int island_rtc_setpie(int enabled)
{
	pr_debug("%s: pie=%d\n", __func__, enabled);

	spin_lock_irq(&island_rtc_lock);

	if (enabled) {
		chal_rtc_intEnable(rtc->handle, CHAL_RTC_INT_PER);	/* enables the interrupt */
	} else {
		chal_rtc_intDisable(rtc->handle, CHAL_RTC_INT_PER);	/* disables the interrupt */
	}
	spin_unlock_irq(&island_rtc_lock);

	return 0;
}

static int island_rtc_setfreq(struct device *dev, int freq)
{
	chal_RTC_PER_INTERVAL_e interval = 0xffffffff;	/* invalid */

	pr_debug("%s: freq=%d\n", __func__, freq);
	switch (freq) {
	case 1:
		interval = CHAL_RTC_PER_INTERVAL_1000ms;
		break;
	case 2:
		interval = CHAL_RTC_PER_INTERVAL_500ms;
		break;
	case 4:
		interval = CHAL_RTC_PER_INTERVAL_250ms;
		break;
	case 8:
		interval = CHAL_RTC_PER_INTERVAL_125ms;
		break;
	}

	if (interval != 0xffffffff) {
		pr_debug("%s: OKAY freq=%d interval=%d\n", __func__, freq,
			 interval);
                spin_lock_irq(&island_rtc_lock);
		chal_rtc_periodInterruptValSet(rtc->handle, interval);
                spin_unlock_irq(&island_rtc_lock);
	} else {
		pr_debug("%s: BAD freq=%d\n", __func__, freq);
		return -EINVAL;
	}
	return 0;
}

static int island_rtc_getfreq(struct device *dev, int *freq)
{
	chal_RTC_PER_INTERVAL_e interval;
	*freq = 0xffffffff;	/* invalid */

	spin_lock_irq(&island_rtc_lock);
	interval = chal_rtc_readReg(rtc->handle, RTC_PERIODIC_TIMER_ADDR);
	spin_unlock_irq(&island_rtc_lock);
	switch (interval) {
	case CHAL_RTC_PER_INTERVAL_125ms:	/* avoid compiler warnings */
		*freq = 8;
		break;
	case CHAL_RTC_PER_INTERVAL_250ms:
		*freq = 4;
		break;
	case CHAL_RTC_PER_INTERVAL_500ms:
		*freq = 2;
		break;
	case CHAL_RTC_PER_INTERVAL_1000ms:
		*freq = 1;
		break;
	case CHAL_RTC_PER_INTERVAL_2000ms:
	case CHAL_RTC_PER_INTERVAL_4000ms:
	case CHAL_RTC_PER_INTERVAL_8000ms:
	case CHAL_RTC_PER_INTERVAL_16000ms:
	case CHAL_RTC_PER_INTERVAL_32000ms:
	case CHAL_RTC_PER_INTERVAL_64000ms:
	case CHAL_RTC_PER_INTERVAL_128000ms:
	case CHAL_RTC_PER_INTERVAL_256000ms:
		break;
	}
	if (*freq == 0xffffffff) {
		pr_debug("%s: Bad interval=%d\n", __func__, interval);
		return -EINVAL;
	}
	pr_debug("%s: interval=%d, freq=%d\n", __func__, interval, *freq);
	return 0;
}

/* Time read/write */

static int island_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	unsigned int epoch_sec, elapsed_sec;

	epoch_sec = mktime(epoch, 1, 1, 0, 0, 0);
	elapsed_sec = chal_rtc_secGet(rtc->handle);

	pr_debug("%s: epoch_sec=%u, elapsed_sec=%u\n", __func__, epoch_sec,
		 elapsed_sec);
	rtc_time_to_tm(epoch_sec + elapsed_sec, rtc_tm);

	pr_debug("read time 0x%02x.0x%02x.0x%02x 0x%02x/0x%02x/0x%02x\n",
		 rtc_tm->tm_year, rtc_tm->tm_mon, rtc_tm->tm_mday,
		 rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);

	return 0;
}

static int island_rtc_settime(struct device *dev, struct rtc_time *time)
{
	unsigned int epoch_sec, current_sec;

	epoch_sec = mktime(epoch, 1, 1, 0, 0, 0);
	current_sec =
	    mktime(time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
		   time->tm_hour, time->tm_min, time->tm_sec);

	chal_rtc_secSet(rtc->handle, current_sec - epoch_sec);
	chal_rtc_ctrlSet(rtc->handle, CHAL_RTC_CTRL_RUN);

	pr_debug("%s: current_sec=%u, epoch_sec=%u\n", __func__, current_sec,
		 epoch_sec);

	pr_debug("set time %02d.%02d.%02d %02d/%02d/%02d\n", time->tm_year,
		 time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min,
		 time->tm_sec);

	return 0;
}

static int island_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	unsigned int epoch_sec, elapsed_sec, alarm_elapsed_sec;
	chal_rtc_TIME_t alm_reg_secs;
	struct rtc_time *alm_tm = &alrm->time;
	alrm->enabled = chal_rtc_intIsEnabled(rtc->handle, CHAL_RTC_INT_MATCH);

	epoch_sec = mktime(epoch, 1, 1, 0, 0, 0);
	elapsed_sec = chal_rtc_secGet(rtc->handle);

	alm_reg_secs = chal_rtc_matchInterruptValGet(rtc->handle);

	/* Handle carry over */
	if ((elapsed_sec & 0x0ffff) > alm_reg_secs) {
		elapsed_sec += 0x10000;
	}
	elapsed_sec &= ~0xffff;	/* clear lower 16 bits for 16-bit alarm match register below */
	alarm_elapsed_sec = elapsed_sec + alm_reg_secs;
	pr_debug
	    ("%s: epoch_sec=%u, elapsed_sec=%u, alm_reg_secs=%lu=0x%lx, alarm_elapsed_sec=%u=0x%x\n",
	     __func__, epoch_sec, elapsed_sec, (unsigned long)alm_reg_secs,
	     (unsigned long)alm_reg_secs, alarm_elapsed_sec, alarm_elapsed_sec);

	rtc_time_to_tm(epoch_sec + alarm_elapsed_sec, alm_tm);
	pr_debug("read alarm %02x %02x.%02x.%02x %02x/%02x/%02x\n",
		 alrm->enabled, alm_tm->tm_year, alm_tm->tm_mon,
		 alm_tm->tm_mday, alm_tm->tm_hour, alm_tm->tm_min,
		 alm_tm->tm_sec);

	return 0;
}

static int island_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	unsigned int epoch_sec, elapsed_sec;
	struct rtc_time *time = &alrm->time;
	chal_rtc_TIME_t alm_secs;

	pr_debug("%s: %d, %02x/%02x/%02x %02x.%02x.%02x\n",
		 __func__, alrm->enabled, time->tm_mday & 0xff,
		 time->tm_mon & 0xff, time->tm_year & 0xff,
		 time->tm_hour & 0xff, time->tm_min & 0xff, time->tm_sec);

	epoch_sec = mktime(epoch, 1, 1, 0, 0, 0);
	elapsed_sec = chal_rtc_secGet(rtc->handle);
	alm_secs =
	    mktime(time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
		   time->tm_hour, time->tm_min, time->tm_sec);

	pr_debug("%s: epoch_sec=%u, elapsed_sec=%u, alm_secs=%lu\n", __func__,
		 epoch_sec, elapsed_sec, (unsigned long)alm_secs);

	spin_lock_irq(&island_rtc_lock);

	chal_rtc_intDisable(rtc->handle, CHAL_RTC_INT_MATCH);
	chal_rtc_intStatusClr(rtc->handle, CHAL_RTC_INT_MATCH);

	chal_rtc_matchInterruptValSet(rtc->handle, alm_secs);

	if (alrm->enabled) {
		chal_rtc_intEnable(rtc->handle, CHAL_RTC_INT_MATCH);
	}

	spin_unlock_irq(&island_rtc_lock);

	return 0;
}

static int island_rtc_proc(struct device *dev, struct seq_file *seq)
{
	seq_printf(seq, "\nperiodic timer: 0x%x\n",
		   chal_rtc_readReg(rtc->handle, RTC_PERIODIC_TIMER_ADDR));
	seq_printf(seq, "match register: 0x%x\n",
		   chal_rtc_readReg(rtc->handle, RTC_MATCH_REGISTER_ADDR));
	seq_printf(seq, "clear intr register: 0x%x\n",
		   chal_rtc_readReg(rtc->handle, RTC_CLEAR_INTR_ADDR));
	seq_printf(seq, "intr status register: 0x%x\n",
		   chal_rtc_readReg(rtc->handle, RTC_INTERRUPT_STATUS_ADDR));
	seq_printf(seq, "control addr register: 0x%x\n",
		   chal_rtc_readReg(rtc->handle, RTC_CONTROL_ADDR));
	return 0;
}

static int
island_rtc_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case RTC_AIE_OFF:
		island_rtc_setaie(0);
		return 0;
	case RTC_AIE_ON:
		island_rtc_setaie(1);
		return 0;
	case RTC_PIE_OFF:
		island_rtc_setpie(0);
		return 0;
	case RTC_PIE_ON:
		island_rtc_setpie(1);
		return 0;
	case RTC_IRQP_READ:
		{
			int freq;
			int ret = island_rtc_getfreq(dev, &freq);
			if (ret != 0) {
				return ret;
			}
			return put_user(freq, (unsigned long *)arg);
		}
	case RTC_IRQP_SET:
		return island_rtc_setfreq(dev, (int)arg);
	}
	return -ENOIOCTLCMD;
}

static void island_rtc_release(struct device *dev)
{
	island_rtc_setaie(0);
	island_rtc_setpie(0);
}

static const struct rtc_class_ops island_rtcops = {
	.ioctl = island_rtc_ioctl,
	.release = island_rtc_release,
	.read_time = island_rtc_gettime,
	.set_time = island_rtc_settime,
	.read_alarm = island_rtc_getalarm,
	.set_alarm = island_rtc_setalarm,
	.proc = island_rtc_proc,
};

static void island_rtc_enable(struct platform_device *pdev, int en)
{
	if (!en) {
		chal_rtc_ctrlSet(rtc->handle, CHAL_RTC_CTRL_STOP);
	} else {
		chal_rtc_ctrlSet(rtc->handle, CHAL_RTC_CTRL_RUN);
	}
}

static int __exit island_rtc_remove(struct platform_device *dev)
{
	rtc_device_unregister(rtc->dev);
	device_init_wakeup(&dev->dev, 0);

	platform_set_drvdata(dev, NULL);

	island_rtc_setpie(0);
	island_rtc_setaie(0);

	free_irq(rtc->irq2, rtc);
	free_irq(rtc->irq1, rtc);

	clk_disable(rtc->clock);
	clk_put(rtc->clock);

	return 0;
}

static int __devinit island_rtc_probe(struct platform_device *dev)
{
	struct resource *res;
	int ret;

	/* We only accept one device, and it must have an id of -1 */
	if (dev->id != -1)
		return -ENODEV;

	res = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENODEV;
		goto err_out;
	}

	rtc = kzalloc(sizeof(struct island_rtc), GFP_KERNEL);
	if (!rtc) {
		ret = -ENOMEM;
		goto err_out;
	}

        rtc->clock = clk_get(&dev->dev, dev->dev.platform_data);
	if (rtc->clock < 0) {
		ret = -ENXIO;
		goto err_free;
	}

	rtc->irq1 = platform_get_irq(dev, 0);
	if (rtc->irq1 < 0) {
		ret = -ENXIO;
		goto err_free;
	}
	rtc->irq2 = platform_get_irq(dev, 1);
	if (rtc->irq2 < 0) {
		ret = -ENXIO;
		goto err_free;
	}
	rtc->base = ioremap(res->start, resource_size(res));

	rtc->handle = &foo;
	chal_rtc_init(rtc->handle, (uint32_t) rtc->base, 0x00002000);

	if (!rtc->base) {
		ret = -ENOMEM;
		goto err_free;
	}

	island_rtc_enable(dev, 1);

	island_rtc_setfreq(&dev->dev, 1);

	device_init_wakeup(&dev->dev, 1);

	chal_rtc_intDisable(rtc->handle, CHAL_RTC_INT_MATCH);
	chal_rtc_intDisable(rtc->handle, CHAL_RTC_INT_PER);
	chal_rtc_intStatusClr(rtc->handle, CHAL_RTC_INT_MATCH);
	chal_rtc_intStatusClr(rtc->handle, CHAL_RTC_INT_PER);

	rtc->dev =
	    rtc_device_register("island", &dev->dev, &island_rtcops,
				THIS_MODULE);

	if (IS_ERR(rtc)) {
		ret = PTR_ERR(rtc);
		pr_debug("cannot attach rtc\n");
		goto err_device_unregister;
	}

	rtc->max_user_freq = 8;

	ret = request_irq(rtc->irq1, rtc_alm_isr, 0, "island_rtc", rtc);
	if (ret) {
		dev_printk(KERN_ERR, &rtc->dev->dev,
			   "cannot register IRQ%d for periodic rtc\n",
			   rtc->irq1);
		goto err_irq;
	}

	ret = request_irq(rtc->irq2, rtc_per_isr, 0, "island_rtc", rtc);
	if (ret) {
		dev_printk(KERN_ERR, &rtc->dev->dev,
			   "cannot register IRQ%d for match rtc\n", rtc->irq2);
		goto err_irq;
	}

	chal_rtc_ctrlSet(rtc->handle, CHAL_RTC_CTRL_RUN);
	clk_enable(rtc->clock);

	printk(KERN_INFO "RTC: driver initialized properly\n");

	return 0;

 err_irq:
	free_irq(rtc->irq1, rtc);
	free_irq(rtc->irq2, rtc);
 err_device_unregister:
	iounmap(rtc->base);
 err_free:
	kfree(rtc);
 err_out:
	return ret;
}

static struct platform_driver island_rtcdrv = {
	.remove = __exit_p(island_rtc_remove),
	.driver = {
		   .name = "island-rtc",
		   .owner = THIS_MODULE,
		   },
};

static char __initdata banner[] =
    "Island RTC Driver, (c) 2011 Broadcom Corporation\n";

static int __init island_rtc_init(void)
{
	printk(banner);
	return platform_driver_probe(&island_rtcdrv, island_rtc_probe);
}

static void __exit island_rtc_exit(void)
{
	platform_driver_unregister(&island_rtcdrv);
}

module_init(island_rtc_init);
module_exit(island_rtc_exit);

MODULE_DESCRIPTION("Broadcom Island RTC Driver");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:island-rtc");
