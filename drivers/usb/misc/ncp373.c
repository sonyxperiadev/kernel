/* drivers/usb/misc/ncp373.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/usb/ncp373.h>

#define ktime_set_from_us(us) \
	ktime_set((us) / USEC_PER_SEC, ((us) % USEC_PER_SEC) * NSEC_PER_USEC)

#define NCP373_DEBUG_FS_PERM		0644

#define NCP373_WAIT_INIT_TIMEOUT	500000

#define NCP373_WAIT_IN_2_EN		1000
#define NCP373_WAIT_T_ON_VBUS		2000
#define NCP373_WAIT_T_START_VBUS	2000
#define NCP373_WAIT_T_ON_T_START_VBUS	(NCP373_WAIT_T_ON_VBUS + \
						NCP373_WAIT_T_START_VBUS)
#define NCP373_WAIT_EN_2_IN		1000
#define NCP373_DELAY_OC			100000

/* Phase of over current detection */
enum oc_detection_state {
	OC_DET_STOP,
	OC_DET_READY,
	OC_DET_START,
	OC_DET_SETTLE,
};

struct ncp373_internal {
	struct device *dev;
	struct ncp373_platform_data *pdata;
	int flg_irq;
	int flg_irq_en;
	int vbus_is_on;
	struct mutex mutex;
	struct hrtimer timer;
	atomic_t timeout;
	atomic_t flg_check;
	atomic_t oc_det_state;
	atomic_t oc_delay_time;
	wait_queue_head_t wait;
	struct delayed_work oc_delay_work;
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_root;
#endif
};
static struct ncp373_internal *pncp373_internal;

static enum hrtimer_restart ncp373_timer_timeout(struct hrtimer *timer)
{
	struct ncp373_internal *dev = pncp373_internal;

	dev_dbg(dev->dev, "%s: received timer timeout\n", __func__);
	atomic_set(&dev->timeout, 1);
	wake_up(&dev->wait);

	return HRTIMER_NORESTART;
}

static irqreturn_t ncp373_flg_check_irq(int irq, void *data)
{
	struct ncp373_internal *dev = pncp373_internal;

	disable_irq_nosync(irq);

	dev_dbg(dev->dev, "%s: irq=%d received /FLG irq\n", __func__, irq);

	atomic_set(&dev->flg_check, 1);
	wake_up(&dev->wait);

	return IRQ_HANDLED;
}

static char *ncp373_oc_det_state_to_string(int status)
{
	char *ret;

	switch (status) {
	case OC_DET_STOP:
		ret = "STOP  ";
		break;
	case OC_DET_READY:
		ret = "READY ";
		break;
	case OC_DET_START:
		ret = "START ";
		break;
	case OC_DET_SETTLE:
		ret = "SETTLE";
		break;
	default:
		ret = "UNKNOWN";
		break;
	}

	return ret;
}

static void ncp373_oc_notify(struct ncp373_internal *dev)
{
	if (unlikely(OC_DET_START != atomic_cmpxchg(&dev->oc_det_state,
						OC_DET_START, OC_DET_SETTLE))) {
		dev_info(dev->dev, "%s: expected=%s, but now=%s,"
			" mode_disable or remove may have begun.\n",
			__func__, ncp373_oc_det_state_to_string(OC_DET_START),
			ncp373_oc_det_state_to_string(
					atomic_read(&dev->oc_det_state)));
		return;
	}

	dev->pdata->notify_flg_int(dev->dev);

	return;
}

static void ncp373_oc_delay_work(struct work_struct *work)
{
	struct ncp373_internal *dev = pncp373_internal;
	int ret;

	if (unlikely(OC_DET_START != atomic_read(&dev->oc_det_state))) {
		dev_info(dev->dev, "%s: expected=%s, but now=%s,"
			" mode_disable or remove may have begun.\n",
			__func__, ncp373_oc_det_state_to_string(OC_DET_START),
			ncp373_oc_det_state_to_string(
					atomic_read(&dev->oc_det_state)));
		return;
	}

	/* check whether /FLG pin return to Hi */
	ret = dev->pdata->platform_get_flg(dev->dev);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev,
			"%s: failed to read FLG pin ret=%d, assume OC\n",
			__func__, ret);
		ncp373_oc_notify(dev);
	} else if (!ret) {
		dev_info(dev->dev, "%s: /FLG tied to Lo, settle OC\n",
			__func__);
		ncp373_oc_notify(dev);
	} else if (0 < ret) {
		dev_info(dev->dev, "%s: /FLG raise to Hi, return from OC\n",
			__func__);
		if (unlikely(OC_DET_START != atomic_cmpxchg(&dev->oc_det_state,
						OC_DET_START, OC_DET_READY)))
			dev_info(dev->dev, "%s: expected=%s, but now=%s,"
				" mode_disable or remove may have begun.\n",
				__func__,
				ncp373_oc_det_state_to_string(OC_DET_START),
				ncp373_oc_det_state_to_string(
					atomic_read(&dev->oc_det_state)));
		else
			enable_irq(dev->flg_irq);
	}

	return;
}

static irqreturn_t ncp373_overcur_irq(int irq, void *data)
{
	struct ncp373_internal *dev = pncp373_internal;
	int ret;

	dev_info(dev->dev, "%s: irq=%d received /FLG interrupt\n",
		__func__, irq);

	disable_irq_nosync(irq);

	if (unlikely(OC_DET_READY != atomic_cmpxchg(&dev->oc_det_state,
						OC_DET_READY, OC_DET_START))) {
		dev_info(dev->dev, "%s: expected=%s, but now=%s,"
			" mode_disable or remove may have begun.\n",
			__func__, ncp373_oc_det_state_to_string(OC_DET_READY),
			ncp373_oc_det_state_to_string(
					atomic_read(&dev->oc_det_state)));
		return IRQ_HANDLED;
	}

	ret = schedule_delayed_work(&dev->oc_delay_work,
			usecs_to_jiffies(atomic_read(&dev->oc_delay_time)));
	if (unlikely(ret < 0)) {
		dev_info(dev->dev, "%s: failed to start the timer, notify"
				" the overcurrent immediately.\n", __func__);
		ncp373_oc_notify(dev);
	}

	return IRQ_HANDLED;
}

static int ncp373_mode_vbus(void)
{
	struct ncp373_internal *dev = pncp373_internal;
	int ret;
	ktime_t remain;

	if (unlikely(!dev)) {
		dev_err(dev->dev, "%s: device %s is not probed yet.\n",
			__func__, NCP373_DRIVER_NAME);
		return -ENODEV;
	}

	mutex_lock(&dev->mutex);

	/* If VBUS is already on, do nothing. */
	if (dev->vbus_is_on) {
		ret = 0;
		dev_dbg(dev->dev, "%s: to vbus mode, but already on\n",
			__func__);
		goto op_done;
	}

	dev_dbg(dev->dev, "%s: ncp373 to vbus mode\n", __func__);

	/* if enabled, disable FLG irq */
	if (unlikely(dev->flg_irq_en)) {
		dev->flg_irq_en = 0;
		free_irq(dev->flg_irq, dev);
		dev_info(dev->dev, "%s: irq should be disabled, but enabled\n",
			__func__);
	}

	/* enable FLG isr to monitor FLG raise */
	atomic_set(&dev->flg_check, 0);
	ret = request_irq(dev->flg_irq, ncp373_flg_check_irq,
				IRQF_TRIGGER_HIGH, "ncp373_flg_check_irq", dev);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to request irq=%d ret=%d\n",
			__func__, dev->flg_irq, ret);
		goto op_err;
	} else {
		dev->flg_irq_en = 1;
		dev_dbg(dev->dev, "%s: succeed to request irq=%d\n",
			__func__, dev->flg_irq);
	}

	/* apply 5V to IN pin(NCP373) */
	dev_dbg(dev->dev, "%s: set true to IN pin\n", __func__);
	ret = dev->pdata->platform_chip_power(dev->dev, 1);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to enable the IN pin\n",
			__func__);
		goto op_err_irq1;
	}

	/* read /FLG pin to check whether NCP373 and VBUS line are normal */
	hrtimer_cancel(&dev->timer);
	atomic_set(&dev->timeout, 0);
	ret = hrtimer_start(&dev->timer,
			ktime_set_from_us(NCP373_WAIT_INIT_TIMEOUT),
			HRTIMER_MODE_REL);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to start the timer\n", __func__);
		goto op_err_in;
	}

	/* wait for /FLG pin raise */
	wait_event(dev->wait,
		atomic_read(&dev->flg_check) || atomic_read(&dev->timeout));

	remain = hrtimer_get_remaining(&dev->timer);
	hrtimer_cancel(&dev->timer);

	/* disable FLG isr to monitoring FLG raise */
	dev->flg_irq_en = 0;
	free_irq(dev->flg_irq, dev);

	dev_dbg(dev->dev,
		"%s: wait for /FLG raise, remain=%lldus, duration=%lldus\n",
		__func__, ktime_to_us(remain),
		NCP373_WAIT_INIT_TIMEOUT - ktime_to_us(remain));

	/* wait for IN to /EN */
	if (0 < (NCP373_WAIT_IN_2_EN - ktime_to_us(remain)))
		usleep(NCP373_WAIT_IN_2_EN - ktime_to_us(remain));

	/* check whether /FLG is tied Hi.
	 * It is necessary checking "flg_check" earlier,
	 * because there is a possibility that a timer expire
	 * due to timeliness of context switch, though it was normal.
	 */
	if (likely(atomic_read(&dev->flg_check))) {
		dev_dbg(dev->dev, "%s: /FLG raise to Hi, normally\n", __func__);
	} else if (likely(atomic_read(&dev->timeout))) {
		dev_err(dev->dev, "%s: /FLG tied to Lo, failed\n", __func__);
		ret = -EIO;
		goto op_err_in;
	}

	/* enable FLG isr to monitor FLG fall */
	atomic_set(&dev->flg_check, 0);
	ret = request_irq(dev->flg_irq, ncp373_flg_check_irq,
				IRQF_TRIGGER_LOW, "ncp373_flg_check_irq", dev);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to request irq=%d ret=%d\n",
			__func__, dev->flg_irq, ret);
		goto op_err_in;
	} else {
		dev->flg_irq_en = 1;
		dev_dbg(dev->dev, "%s: succeed to request irq=%d\n",
			__func__, dev->flg_irq);
	}

	/* set true to /EN pin */
	dev_dbg(dev->dev, "%s: set true to /EN pin\n", __func__);
	ret = dev->pdata->platform_vbus_power(dev->dev, 1);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to enable the EN pin\n",
			__func__);
		goto op_err_irq2;
	}

	/* read /FLG pin to check whether current is normal,
	 * until Start Up Delay(Tonvbus) + FLAG going up Delay(Tstartvbus)
	 */
	hrtimer_cancel(&dev->timer);
	atomic_set(&dev->timeout, 0);
	ret = hrtimer_start(&dev->timer,
			ktime_set_from_us(NCP373_WAIT_T_ON_T_START_VBUS),
			HRTIMER_MODE_REL);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to start the timer\n", __func__);
		goto op_err_en;
	}

	/* wait for /FLG pin falls */
	wait_event(dev->wait,
		atomic_read(&dev->flg_check) || atomic_read(&dev->timeout));

	remain = hrtimer_get_remaining(&dev->timer);
	hrtimer_cancel(&dev->timer);

	/* disable FLG isr to monitoring FLG fall */
	dev->flg_irq_en = 0;
	free_irq(dev->flg_irq, dev);

	dev_dbg(dev->dev,
		"%s: wait for /FLG fall, remain=%lldus, duration=%lldus\n",
		__func__, ktime_to_us(remain),
		NCP373_WAIT_T_ON_T_START_VBUS - ktime_to_us(remain));

	/* check whether /FLG is tied Lo.
	 * It is necessary checking "flg_check" earlier,
	 * because there is a possibility that a timer expire
	 * due to timeliness of context switch, though it was normal.
	 */
	if (likely(atomic_read(&dev->flg_check))) {
		dev_dbg(dev->dev, "%s: /FLG fall to Lo, normally\n", __func__);
	} else if (likely(atomic_read(&dev->timeout))) {
		dev_err(dev->dev, "%s: /FLG tied to Hi, failed\n", __func__);
		ret = -EIO;
		goto op_err_en;
	}

	/* wait for FLAG going up Delay(Tstartvbus) */
	usleep(NCP373_WAIT_T_ON_T_START_VBUS);

	/* enable FLG isr to monitor the overcurrent */
	atomic_set(&dev->oc_det_state, OC_DET_READY);
	ret = request_irq(dev->flg_irq, ncp373_overcur_irq,
				IRQF_TRIGGER_LOW, "ncp373_overcur_irq", dev);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to request irq=%d ret=%d\n",
			__func__, dev->flg_irq, ret);
		goto op_err_en;
	} else {
		dev->flg_irq_en = 1;
		dev_dbg(dev->dev, "%s: succeed to request irq=%d\n",
			__func__, dev->flg_irq);
	}

	/* All the process to the VBUS mode was completed. */
	dev->vbus_is_on = 1;

	mutex_unlock(&dev->mutex);
	return 0;

op_err_en:
	atomic_set(&dev->oc_det_state, OC_DET_STOP);
	dev->pdata->platform_vbus_power(dev->dev, 0);
	usleep(NCP373_WAIT_EN_2_IN);
op_err_irq2:
	if (dev->flg_irq_en) {
		dev->flg_irq_en = 0;
		free_irq(dev->flg_irq, dev);
	}
op_err_in:
	dev->pdata->platform_chip_power(dev->dev, 0);
op_err_irq1:
	if (dev->flg_irq_en) {
		dev->flg_irq_en = 0;
		free_irq(dev->flg_irq, dev);
	}
op_err:
op_done:
	mutex_unlock(&dev->mutex);
	return ret;
}

static int ncp373_mode_disable(void)
{
	struct ncp373_internal *dev = pncp373_internal;
	int old_oc_det_state;

	if (unlikely(!dev)) {
		dev_err(dev->dev, "%s: device %s is not probed yet.\n",
			__func__, NCP373_DRIVER_NAME);
		return -ENODEV;
	}

	mutex_lock(&dev->mutex);

	/* If VBUS is already off, do nothing. */
	if (!dev->vbus_is_on) {
		dev_dbg(dev->dev, "%s: to disable mode, but already off\n",
			__func__);
		goto op_done;
	}

	old_oc_det_state = atomic_xchg(&dev->oc_det_state, OC_DET_STOP);
	dev_dbg(dev->dev, "%s: ncp373 to disable mode, oc_det_state=%s\n",
		__func__, ncp373_oc_det_state_to_string(old_oc_det_state));
	dev->vbus_is_on = 0;

	/* disable FLG irq */
	if (likely(dev->flg_irq_en)) {
		dev->flg_irq_en = 0;
		free_irq(dev->flg_irq, dev);
	}

	cancel_delayed_work_sync(&dev->oc_delay_work);

	/* set false to /EN pin(NCP373) */
	dev->pdata->platform_vbus_power(dev->dev, 0);

	/* wait for /EN to IN */
	usleep(NCP373_WAIT_EN_2_IN);

	/* apply gnd by using mpp to IN pin(NCP373) */
	dev->pdata->platform_chip_power(dev->dev, 0);

op_done:
	mutex_unlock(&dev->mutex);
	return 0;
}

int ncp373_vbus_switch(int on)
{
	int ret;

	if (on)
		ret = ncp373_mode_vbus();
	else
		ret = ncp373_mode_disable();

	return ret;
}
EXPORT_SYMBOL(ncp373_vbus_switch);

#ifdef CONFIG_DEBUG_FS
static int ncp373_vbus_switch_oc_delay_time_set(void *data, u64 val)
{
	struct ncp373_internal *dev = pncp373_internal;

	dev_dbg(dev->dev, "%s: set oc delay time to=%dus\n",
		__func__, (int)val);

	if (likely(dev))
		atomic_set(&dev->oc_delay_time, (int)val);

	return 0;
}

static int ncp373_vbus_switch_oc_delay_time_get(void *data, u64 *val)
{
	struct ncp373_internal *dev = pncp373_internal;

	if (likely(dev))
		*val = (u64)atomic_read(&dev->oc_delay_time);
	else
		*val = (u64)~0;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ncp373_vbus_switch_oc_delay_time_fops,
			ncp373_vbus_switch_oc_delay_time_get,
			ncp373_vbus_switch_oc_delay_time_set, "%llu\n");

static int ncp373_vbus_switch_debug_set(void *data, u64 val)
{
	struct ncp373_internal *dev = pncp373_internal;
	dev_dbg(dev->dev, "%s: switch vbus via debugfs\n", __func__);
	return ncp373_vbus_switch((int)!!val);
}

static int ncp373_vbus_switch_debug_get(void *data, u64 *val)
{
	struct ncp373_internal *dev = pncp373_internal;

	if (likely(dev))
		*val = dev->vbus_is_on;
	else
		*val = (u64)~0;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ncp373_vbus_switch_debug_fops,
			ncp373_vbus_switch_debug_get,
			ncp373_vbus_switch_debug_set, "%llu\n");

static int ncp373_debugfs_init(struct ncp373_internal *dev)
{
	struct dentry *dent;

	if (!dev)
		return -ENODEV;

	dent = debugfs_create_dir(NCP373_DRIVER_NAME, 0);
	if (IS_ERR(dent))
		goto dent_err;
	dev->debugfs_root = dent;

	dent = debugfs_create_file("dont_use_via_usbadb", 0000,
					dev->debugfs_root, NULL, NULL);
	if (IS_ERR(dent))
		goto dent_err;

	dent = debugfs_create_file("debug_vbus_switch", NCP373_DEBUG_FS_PERM,
					dev->debugfs_root, NULL,
					&ncp373_vbus_switch_debug_fops);
	if (IS_ERR(dent))
		goto dent_err;

	dent = debugfs_create_file("oc_delay_time", NCP373_DEBUG_FS_PERM,
					dev->debugfs_root, NULL,
					&ncp373_vbus_switch_oc_delay_time_fops);
	if (IS_ERR(dent))
		goto dent_err;

	return 0;
dent_err:
	return PTR_ERR(dent);
}

static void ncp373_debugfs_cleanup(struct ncp373_internal *dev)
{
	if (!dev)
		return;
	debugfs_remove_recursive(dev->debugfs_root);
}
#else
static int ncp373_debugfs_init(struct ncp373_internal *dev) { return 0; }
static void ncp373_debugfs_cleanup(struct ncp373_internal *dev) { return; }
#endif

static int __init ncp373_probe(struct platform_device *pdev)
{
	int ret;
	struct ncp373_internal *dev;
	struct ncp373_platform_data *pdata;

	pdata = (struct ncp373_platform_data *)pdev->dev.platform_data;
	if (!pdata
		|| !pdata->platform_chip_power
		|| !pdata->platform_vbus_power
		|| !pdata->platform_get_flg
		|| !pdata->notify_flg_int)
		return -EINVAL;

	dev = kzalloc(sizeof(struct ncp373_internal), GFP_KERNEL);
	if (unlikely(!dev))
		return -ENOMEM;

	dev->dev = &pdev->dev;
	dev->pdata = pdata;

	mutex_init(&dev->mutex);

	atomic_set(&dev->timeout, 0);
	atomic_set(&dev->flg_check, 0);
	atomic_set(&dev->oc_det_state, OC_DET_STOP);

	if (dev->pdata && dev->pdata->oc_delay_time)
		atomic_set(&dev->oc_delay_time, dev->pdata->oc_delay_time);
	else
		atomic_set(&dev->oc_delay_time, NCP373_DELAY_OC);

	init_waitqueue_head(&dev->wait);

	hrtimer_init(&dev->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	dev->timer.function = ncp373_timer_timeout;

	INIT_DELAYED_WORK(&dev->oc_delay_work, ncp373_oc_delay_work);

	dev->flg_irq = platform_get_irq(pdev, 0);
	if (unlikely(dev->flg_irq <= 0)) {
		dev_err(dev->dev,
			"%s: could not get /FLG IRQ resource. irq=%d\n",
			__func__, dev->flg_irq);
		ret = -ENODEV;
		goto free_dev;
	}

	if (dev->pdata->platform_init) {
		ret = dev->pdata->platform_init(dev->dev);
		if (unlikely(ret < 0)) {
			dev_err(dev->dev,
				"%s: failed to platform_init ret=%d\n",
				__func__, ret);
			goto free_dev;
		} else {
			dev_dbg(dev->dev, "%s: succeed to platform_init\n",
				__func__);
		}
	}

	ret = ncp373_debugfs_init(dev);
	if (unlikely(ret < 0)) {
		dev_err(dev->dev, "%s: failed to initialize the debugfs\n",
			__func__);
		goto free_dev;
	}

	pncp373_internal = dev;
	dev_info(dev->dev, "%s: registered ncp373 vbus load switch\n",
		__func__);
	return 0;
free_dev:
	ncp373_debugfs_cleanup(dev);
	mutex_destroy(&dev->mutex);
	dev_err(dev->dev,
		"%s: failed to register ncp373 vbus load switch ret=%d\n",
		__func__, ret);
        kfree(dev);
	return ret;
}


static int __devexit ncp373_remove(struct platform_device *pdev)
{
	struct ncp373_internal *dev = pncp373_internal;

	if (likely(dev)) {
		atomic_set(&dev->oc_det_state, OC_DET_STOP);
		hrtimer_cancel(&dev->timer);
		atomic_set(&dev->timeout, 1);

		ncp373_debugfs_cleanup(dev);
		ncp373_mode_disable();

		mutex_lock(&dev->mutex);
		if (dev->pdata && dev->pdata->platform_release)
			dev->pdata->platform_release(dev->dev);
		pncp373_internal = NULL;
		mutex_unlock(&dev->mutex);
		mutex_destroy(&dev->mutex);
		kfree(dev);
	}

	return 0;
}

static struct platform_driver ncp373_driver = {
	.remove         = __devexit_p(ncp373_remove),
	.driver         = {
		.name = NCP373_DRIVER_NAME,
		.owner = THIS_MODULE
	},
};

static int __init ncp373_init(void)
{
	return platform_driver_probe(&ncp373_driver, ncp373_probe);
}

static void __exit ncp373_exit(void)
{
	platform_driver_unregister(&ncp373_driver);
}

module_init(ncp373_init);
module_exit(ncp373_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("NCP373 VBUS load switch");
MODULE_VERSION("1.0");
