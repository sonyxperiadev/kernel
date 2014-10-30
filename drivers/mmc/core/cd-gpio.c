/*
 * Generic GPIO card-detect helper
 *
 * Copyright (C) 2011, Guennadi Liakhovetski <g.liakhovetski@gmx.de>
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/mmc/cd-gpio.h>
#include <linux/mmc/host.h>
#include <linux/module.h>
#include <linux/slab.h>

struct mmc_cd_gpio {
	unsigned int gpio;
	bool status;
	char label[0];
#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
	bool pending_detect;
	bool suspended;
#endif
	/* Don't add any fields at the end of this structure as they will
	 * overwrite the label. */
};

int mmc_cd_get_status(struct mmc_host *host)
{
	int ret = -ENOSYS;
	struct mmc_cd_gpio *cd = host->hotplug.handler_priv;

	if (!cd || !gpio_is_valid(cd->gpio))
		goto out;

	ret = !gpio_get_value_cansleep(cd->gpio) ^
		!!(host->caps2 & MMC_CAP2_CD_ACTIVE_HIGH);
out:
	return ret;
}

#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
void mmc_cd_prepare_suspend(struct mmc_host *host, bool pending_detect)
{
	struct mmc_cd_gpio *cd = host->hotplug.handler_priv;
	if (!cd)
		return;

	cd->suspended = true;
	cd->pending_detect = pending_detect;
}
EXPORT_SYMBOL(mmc_cd_prepare_suspend);
#endif

#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
bool mmc_cd_is_pending_detect(struct mmc_host *host)
{
	struct mmc_cd_gpio *cd = host->hotplug.handler_priv;
	if (!cd)
		return false;

	return cd->pending_detect;
}
EXPORT_SYMBOL(mmc_cd_is_pending_detect);
#endif

static irqreturn_t mmc_cd_gpio_irqt(int irq, void *dev_id)
{
	struct mmc_host *host = dev_id;
	struct mmc_cd_gpio *cd = host->hotplug.handler_priv;
#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
	unsigned long flags;
#endif
	int status;

	status = mmc_cd_get_status(host);
	if (unlikely(status < 0))
		goto out;

#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
	if (cd->suspended) {
		/*
		 * host->rescan_disable is normally set to 0 in mmc_resume_bus
		 * but in case of a deferred resume we might get IRQ before
		 * it is called.
		 */
		spin_lock_irqsave(&host->lock, flags);
		host->rescan_disable = 0;
		spin_unlock_irqrestore(&host->lock, flags);

		/*
		 * We've got IRQ just after resuming device but the status
		 * did not change; card might have been swapped, force
		 * redetection.
		 */
		if (status == cd->status)
			status = 2;
	}
	cd->suspended = false;
#endif

	if (status ^ cd->status) {
		pr_info("%s: slot status change detected (%d -> %d), GPIO_ACTIVE_%s\n",
				mmc_hostname(host), cd->status, status,
				(host->caps2 & MMC_CAP2_CD_ACTIVE_HIGH) ?
				"HIGH" : "LOW");
		cd->status = status;

		/* Schedule a card detection after a debounce timeout */
		mmc_detect_change(host, msecs_to_jiffies(100));
	}
out:
	return IRQ_HANDLED;
}

int mmc_cd_gpio_request(struct mmc_host *host, unsigned int gpio)
{
	size_t len = strlen(dev_name(host->parent)) + 4;
	struct mmc_cd_gpio *cd;
	int irq = gpio_to_irq(gpio);
	int ret;

	if (irq < 0)
		return irq;

	cd = kmalloc(sizeof(*cd) + len, GFP_KERNEL);
	if (!cd)
		return -ENOMEM;

	snprintf(cd->label, len, "%s cd", dev_name(host->parent));

	ret = gpio_request_one(gpio, GPIOF_DIR_IN, cd->label);
	if (ret < 0)
		goto egpioreq;

	cd->gpio = gpio;
	host->hotplug.irq = irq;
	host->hotplug.handler_priv = cd;

	ret = mmc_cd_get_status(host);
	if (ret < 0)
		goto eirqreq;

	cd->status = ret;

#ifdef CONFIG_MMC_BLOCK_DEFERRED_RESUME
	cd->pending_detect = false;
	cd->suspended = false;
#endif

	ret = request_threaded_irq(irq, NULL, mmc_cd_gpio_irqt,
				   IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
				   cd->label, host);
	if (ret < 0)
		goto eirqreq;

	return 0;

eirqreq:
	gpio_free(gpio);
egpioreq:
	kfree(cd);
	return ret;
}
EXPORT_SYMBOL(mmc_cd_gpio_request);

void mmc_cd_gpio_free(struct mmc_host *host)
{
	struct mmc_cd_gpio *cd = host->hotplug.handler_priv;

	if (!cd || !gpio_is_valid(cd->gpio))
		return;

	free_irq(host->hotplug.irq, host);
	gpio_free(cd->gpio);
	cd->gpio = -EINVAL;
	kfree(cd);
}
EXPORT_SYMBOL(mmc_cd_gpio_free);
