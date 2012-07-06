/* kernel/arch/arm/mach-msm/semc_charger_cradle.c
 *
 * Copyright (C) 2011-2012 Sony Ericsson Mobile Communications AB.
 *
 * Author: Hiroyuki Namba <Hiroyuki.Namba@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/slab.h>
#include <linux/err.h>
#include <asm/mach-types.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <mach/semc_charger_cradle.h>
#include <mach/semc_charger_usb.h>
#include <linux/gpio.h>
#include <asm/atomic.h>

#define DETECTION_INTERVAL (HZ / 5) /* 200ms */
#define DETECTION_POLLING_TIME (HZ) /* 1s */
#define CRADLE_WAKELOCK_TIMEOUT (HZ * 2) /* 2sec */
#define CRADLE_CON_WAKELOCK_TIMEOUT (HZ * 5) /* 5sec */

struct semc_chg_cradle_data {
	struct device dev;
	struct power_supply ps;
	struct delayed_work work;
	struct workqueue_struct *wq;
	int connected;
	int chg_current_ma;
	int cradle_detect_gpio;
	int cradle_detect_irq;
	int irq_wake_enabled;
	int prepared_ac_online;
	spinlock_t lock;
	struct wake_lock cradle_wake_lock;
};

static enum power_supply_property semc_chg_cradle_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int semc_chg_cradle_is_prepared_ac_online(void)
{
	return power_supply_get_by_name(SEMC_CHARGER_AC_NAME) ? 1 : 0;
}

static int semc_chg_cradle_get_property(struct power_supply *ps,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	int ret = 0;
	unsigned long flags;
	struct semc_chg_cradle_data *cd =
		container_of(ps, struct semc_chg_cradle_data, ps);

	spin_lock_irqsave(&cd->lock, flags);

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = cd->connected;
		dev_dbg(&cd->dev, "%s() returns %d\n", __func__, val->intval);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	spin_unlock_irqrestore(&cd->lock, flags);

	return ret;
}

static irqreturn_t semc_charger_cradle_det_handler(int irq, void *data)
{
	unsigned long flags;
	struct semc_chg_cradle_data *cd = data;

	dev_dbg(&cd->dev, "%s()\n", __func__);

	spin_lock_irqsave(&cd->lock, flags);

	if (delayed_work_pending(&cd->work))
		cancel_delayed_work(&cd->work);

	queue_delayed_work(cd->wq, &cd->work, DETECTION_INTERVAL);

	spin_unlock_irqrestore(&cd->lock, flags);

	return IRQ_HANDLED;
}

static void semc_charger_cradle_worker(struct work_struct *work)
{
	int state;
	bool changed = false;
	unsigned long flags;
	struct delayed_work *dwork =
		container_of(work, struct delayed_work, work);
	struct semc_chg_cradle_data *cd =
		container_of(dwork, struct semc_chg_cradle_data, work);

	spin_lock_irqsave(&cd->lock, flags);

	state = gpio_get_value(cd->cradle_detect_gpio);
	dev_dbg(&cd->dev, "CRADLE_CHG_DET=%d\n", state);

	/*
	 * GPIO_126 state is 0 : Connected to cradle.
	 *          state is 1 : Not connected to cradle.
	 */
	if (cd->connected != !state) {
		cd->connected = !state;
		changed = true;
	}

	if (cd->prepared_ac_online) {
		if (changed) {
			dev_dbg(&cd->dev, "Connection changed.\n");
			power_supply_changed(&cd->ps);
			if (!(cd->connected))
				wake_lock_timeout(&cd->cradle_wake_lock,
					CRADLE_WAKELOCK_TIMEOUT);
			else
				wake_lock_timeout(&cd->cradle_wake_lock,
					CRADLE_CON_WAKELOCK_TIMEOUT);
		}
	} else {
		/*
		 * If ac/online sysfs is not prepared then
		 * starts worker with polling.
		 * And if it has prepared then call power_supply_changed().
		 */
		if (!semc_chg_cradle_is_prepared_ac_online()) {
			dev_dbg(&cd->dev, "/ac/online is NOT prepared.\n");
			if (delayed_work_pending(&cd->work))
				cancel_delayed_work(&cd->work);
			queue_delayed_work(cd->wq, &cd->work,
					DETECTION_POLLING_TIME);
		} else {
			dev_dbg(&cd->dev, "/ac/online is prepared.\n");
			cd->prepared_ac_online = 1;
			power_supply_changed(&cd->ps);
		}
	}
	spin_unlock_irqrestore(&cd->lock, flags);
}

unsigned int semc_charger_cradle_current_ma(void)
{
	struct semc_chg_cradle_data *cd;
	struct power_supply *psy =
		power_supply_get_by_name(SEMC_CHG_CRADLE_NAME);

	pr_debug("%s: %s\n", SEMC_CHG_CRADLE_NAME, __func__);

	if (!psy) {
		pr_err("%s: %s() No data\n", SEMC_CHG_CRADLE_NAME, __func__);
		return 0;
	}
	cd = container_of(psy, struct semc_chg_cradle_data, ps);

	return cd->chg_current_ma;
}
EXPORT_SYMBOL(semc_charger_cradle_current_ma);

int semc_charger_cradle_is_connected(void)
{
	struct semc_chg_cradle_data *cd;
	struct power_supply *psy =
		power_supply_get_by_name(SEMC_CHG_CRADLE_NAME);

	if (!psy) {
		pr_err("%s: %s() No data\n", SEMC_CHG_CRADLE_NAME, __func__);
		return 0;
	}
	cd = container_of(psy, struct semc_chg_cradle_data, ps);

	dev_info(&cd->dev, "%s() ret=%d\n", __func__, cd->connected);
	return cd->connected;
}
EXPORT_SYMBOL(semc_charger_cradle_is_connected);

static int semc_chg_cradle_suspend(struct platform_device *dev,
				pm_message_t state)
{
	struct power_supply *psy =
		power_supply_get_by_name(SEMC_CHG_CRADLE_NAME);
	struct semc_chg_cradle_data *cd;

	pr_debug("%s: %s\n", SEMC_CHG_CRADLE_NAME, __func__);

	if (!psy) {
		pr_err("%s: %s() No data\n", SEMC_CHG_CRADLE_NAME, __func__);
		return 0;
	}
	cd = container_of(psy, struct semc_chg_cradle_data, ps);

	if (delayed_work_pending(&cd->work) || spin_is_locked(&cd->lock))
		return -EAGAIN;

	return 0;
}

static int semc_chg_cradle_resume(struct platform_device *dev)
{
	pr_debug("%s: %s\n", SEMC_CHG_CRADLE_NAME, __func__);

	return 0;
}

static int semc_chg_cradle_probe(struct platform_device *pdev)
{
	int rc;
	struct semc_chg_cradle_platform_data *pdata = pdev->dev.platform_data;
	struct semc_chg_cradle_data *cd;

	dev_info(&pdev->dev, "probe\n");

	if (pdata && pdata->gpio_configure) {
		rc = pdata->gpio_configure(1);
		if (rc) {
			dev_err(&pdev->dev, "failed to gpio_configure\n");
			goto probe_exit;
		}
	} else {
		rc = -ENODEV;
		goto probe_exit;
	}

	cd = kzalloc(sizeof(struct semc_chg_cradle_data), GFP_KERNEL);
	if (!cd) {
		dev_err(&pdev->dev, "Memory alloc fail\n");
		rc = -ENOMEM;
		goto probe_exit_hw_deinit;
	}

	cd->ps.name = SEMC_CHG_CRADLE_NAME;
	cd->ps.type = POWER_SUPPLY_TYPE_MAINS;
	cd->ps.properties = semc_chg_cradle_props;
	cd->ps.num_properties = ARRAY_SIZE(semc_chg_cradle_props);
	cd->ps.get_property = semc_chg_cradle_get_property;
	cd->dev = pdev->dev;

	if (pdata->supplied_to) {
		cd->ps.supplied_to = pdata->supplied_to;
		cd->ps.num_supplicants =
			pdata->num_supplicants;
	}
	cd->cradle_detect_gpio = pdata->cradle_detect_gpio;
	cd->chg_current_ma = pdata->supply_current_limit_from_cradle;

	spin_lock_init(&cd->lock);

	cd->wq = create_singlethread_workqueue("chg_cradle_worker");
	if (!cd->wq) {
		dev_err(&pdev->dev, "Failed creating workqueue\n");
		rc = -EIO;
		goto probe_exit_free;
	}

	INIT_DELAYED_WORK(&cd->work, semc_charger_cradle_worker);

	rc = power_supply_register(NULL, &cd->ps);
	if (rc) {
		dev_err(&pdev->dev,
			"Failed register to power_supply class\n");
		goto probe_exit_destroy_wq;
	}

	rc = gpio_to_irq(cd->cradle_detect_gpio);
	if (rc < 0) {
		dev_err(&pdev->dev, "Failed getting IRQ from GPIO\n");
		goto probe_exit_unregister;
	}
	cd->cradle_detect_irq = rc;

	rc = request_threaded_irq(
		cd->cradle_detect_irq,
		NULL,
		semc_charger_cradle_det_handler,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		"cradle_det", cd);
	if (rc) {
		dev_err(&pdev->dev, "Failed request irq.\n");
		goto probe_exit_unregister;
	}

	rc = enable_irq_wake(cd->cradle_detect_irq);
	if (rc)
		dev_err(&pdev->dev, "Failed to enable irq wakeup\n");
	else
		cd->irq_wake_enabled = 1;

	wake_lock_init(&cd->cradle_wake_lock, WAKE_LOCK_SUSPEND, "chg_cradle");

	queue_delayed_work(cd->wq, &cd->work, 0);
	return 0;

probe_exit_unregister:
	power_supply_unregister(&cd->ps);
probe_exit_destroy_wq:
	destroy_workqueue(cd->wq);
probe_exit_free:
	kfree(cd);
probe_exit_hw_deinit:
	if (pdata && pdata->gpio_configure)
		rc = pdata->gpio_configure(0);
probe_exit:
	return rc;
}

static int __devexit semc_chg_cradle_remove(struct platform_device *pdev)
{
	struct semc_chg_cradle_data *cd = platform_get_drvdata(pdev);

	dev_info(&pdev->dev, "remove\n");

	if (cd->irq_wake_enabled)
		disable_irq_wake(cd->cradle_detect_irq);

	free_irq(cd->cradle_detect_irq, 0);

	if (delayed_work_pending(&cd->work))
		cancel_delayed_work_sync(&cd->work);

	destroy_workqueue(cd->wq);

	power_supply_unregister(&cd->ps);

	kfree(cd);
	return 0;
}

static struct platform_driver semc_chg_cradle_driver = {
	.probe = semc_chg_cradle_probe,
	.remove = __exit_p(semc_chg_cradle_remove),
	.suspend = semc_chg_cradle_suspend,
	.resume = semc_chg_cradle_resume,
	.driver = {
		.name = SEMC_CHG_CRADLE_NAME,
		.owner = THIS_MODULE,
	},
};

static int __devinit semc_chg_cradle_init(void)
{
	int rc;

	pr_debug("%s: Initializing...\n", SEMC_CHG_CRADLE_NAME);

	rc = platform_driver_register(&semc_chg_cradle_driver);
	if (rc) {
		pr_err("%s: Failed register platform driver. rc = %d\n",
		       SEMC_CHG_CRADLE_NAME, rc);
	}

	return rc;
}

static void __devexit semc_chg_cradle_exit(void)
{
	pr_debug("%s: Exiting...\n", SEMC_CHG_CRADLE_NAME);

	platform_driver_unregister(&semc_chg_cradle_driver);
}

module_init(semc_chg_cradle_init);
module_exit(semc_chg_cradle_exit);

MODULE_AUTHOR("Hiroyuki Namba");
MODULE_DESCRIPTION("Cradle charger handling for Sony Ericsson Mobile"
		   " Communications");

MODULE_LICENSE("GPL v2");
