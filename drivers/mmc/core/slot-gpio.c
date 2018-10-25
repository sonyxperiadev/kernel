/*
 * Generic GPIO card-detect helper
 *
 * Copyright (C) 2011, Guennadi Liakhovetski <g.liakhovetski@gmx.de>
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/mmc/host.h>
#include <linux/mmc/slot-gpio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/extcon.h>

#include "slot-gpio.h"

struct mmc_gpio {
	struct gpio_desc *ro_gpio;
	struct gpio_desc *cd_gpio;
	bool override_ro_active_level;
	bool override_cd_active_level;
	irqreturn_t (*cd_gpio_isr)(int irq, void *dev_id);
	bool status;
	int uim2_gpio;
#ifdef CONFIG_MMC_SD_DEFERRED_RESUME
	bool pending_detect;
	bool suspended;
#endif
	char *ro_label;
	char cd_label[0];
};

int mmc_gpio_get_status(struct mmc_host *host)
{
	int ret = -ENOSYS;
	struct mmc_gpio *ctx = host->slot.handler_priv;

	if (!ctx || !gpio_is_valid(desc_to_gpio(ctx->cd_gpio)))
		goto out;

	ret = !gpio_get_value_cansleep(desc_to_gpio(ctx->cd_gpio)) ^
		!!(host->caps2 & MMC_CAP2_CD_ACTIVE_HIGH);
out:
	return ret;
}

#ifdef CONFIG_MMC_SD_DEFERRED_RESUME
void mmc_cd_prepare_suspend(struct mmc_host *host, bool pending_detect)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;

	if (!ctx)
		return;

	ctx->suspended = true;
	ctx->pending_detect = pending_detect;
}
EXPORT_SYMBOL(mmc_cd_prepare_suspend);

bool mmc_cd_is_pending_detect(struct mmc_host *host)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;

	if (!ctx)
		return false;

	return ctx->pending_detect;
}
EXPORT_SYMBOL(mmc_cd_is_pending_detect);
#endif

static irqreturn_t mmc_gpio_cd_irqt(int irq, void *dev_id)
{
	/* Schedule a card detection after a debounce timeout */
	struct mmc_host *host = dev_id;
	int present = host->ops->get_cd(host);

	pr_debug("%s: cd gpio irq, gpio state %d (CARD_%s)\n",
		mmc_hostname(host), present, present?"INSERT":"REMOVAL");

	host->trigger_card_event = true;
	mmc_detect_change(host, msecs_to_jiffies(200));

	return IRQ_HANDLED;
}

int mmc_gpio_alloc(struct mmc_host *host)
{
	size_t len = strlen(dev_name(host->parent)) + 4;
	struct mmc_gpio *ctx = devm_kzalloc(host->parent,
				sizeof(*ctx) + 2 * len,	GFP_KERNEL);

	if (ctx) {
		ctx->ro_label = ctx->cd_label + len;
		snprintf(ctx->cd_label, len, "%s cd", dev_name(host->parent));
		snprintf(ctx->ro_label, len, "%s ro", dev_name(host->parent));
		host->slot.handler_priv = ctx;
		host->slot.cd_irq = -EINVAL;
		ctx->uim2_gpio = -EINVAL;
	}

	return ctx ? 0 : -ENOMEM;
}

int mmc_gpio_get_ro(struct mmc_host *host)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;

	if (!ctx || !ctx->ro_gpio)
		return -ENOSYS;

	if (ctx->override_ro_active_level)
		return !gpiod_get_raw_value_cansleep(ctx->ro_gpio) ^
			!!(host->caps2 & MMC_CAP2_RO_ACTIVE_HIGH);

	return gpiod_get_value_cansleep(ctx->ro_gpio);
}
EXPORT_SYMBOL(mmc_gpio_get_ro);

int mmc_gpio_get_cd(struct mmc_host *host)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;
	int ret;

	if (host->extcon) {
		ret =  extcon_get_state(host->extcon, EXTCON_MECHANICAL);
		if (ret < 0)
			dev_err(mmc_dev(host), "%s: Extcon failed to check card state, ret=%d\n",
					__func__, ret);
		return ret;
	}

	if (!ctx || !ctx->cd_gpio)
		return -ENOSYS;

	if (ctx->override_cd_active_level)
		return !gpiod_get_raw_value_cansleep(ctx->cd_gpio) ^
			!!(host->caps2 & MMC_CAP2_CD_ACTIVE_HIGH);

	return gpiod_get_value_cansleep(ctx->cd_gpio);
}
EXPORT_SYMBOL(mmc_gpio_get_cd);

/**
 * mmc_gpio_request_ro - request a gpio for write-protection
 * @host: mmc host
 * @gpio: gpio number requested
 *
 * As devm_* managed functions are used in mmc_gpio_request_ro(), client
 * drivers do not need to worry about freeing up memory.
 *
 * Returns zero on success, else an error.
 */
int mmc_gpio_request_ro(struct mmc_host *host, unsigned int gpio)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;
	int ret;

	if (!gpio_is_valid(gpio))
		return -EINVAL;

	ret = devm_gpio_request_one(host->parent, gpio, GPIOF_DIR_IN,
				    ctx->ro_label);
	if (ret < 0)
		return ret;

	ctx->override_ro_active_level = true;
	ctx->ro_gpio = gpio_to_desc(gpio);

	return 0;
}
EXPORT_SYMBOL(mmc_gpio_request_ro);

void mmc_gpiod_request_cd_irq(struct mmc_host *host)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;
	int ret, irq;

	if (host->slot.cd_irq >= 0 || !ctx || !ctx->cd_gpio)
		return;

	irq = gpiod_to_irq(ctx->cd_gpio);

	/*
	 * Even if gpiod_to_irq() returns a valid IRQ number, the platform might
	 * still prefer to poll, e.g., because that IRQ number is already used
	 * by another unit and cannot be shared.
	 */
	if (irq >= 0 && host->caps & MMC_CAP_NEEDS_POLL)
		irq = -EINVAL;

       ret = mmc_gpio_get_status(host);
       if (ret < 0)
               pr_warn("%s: failed to init cd_gpio status\n", mmc_hostname(host));
       else
               ctx->status = ret;

	if (irq >= 0) {
		if (!ctx->cd_gpio_isr)
			ctx->cd_gpio_isr = mmc_gpio_cd_irqt;
		ret = devm_request_threaded_irq(host->parent, irq,
			NULL, ctx->cd_gpio_isr,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
			ctx->cd_label, host);
		if (ret < 0)
			irq = ret;
	}

	host->slot.cd_irq = irq;

	if (irq < 0)
		host->caps |= MMC_CAP_NEEDS_POLL;
}
EXPORT_SYMBOL(mmc_gpiod_request_cd_irq);

static int mmc_card_detect_notifier(struct notifier_block *nb,
				       unsigned long event, void *ptr)
{
	struct mmc_host *host = container_of(nb, struct mmc_host,
					     card_detect_nb);

	host->trigger_card_event = true;
	mmc_detect_change(host, 0);

	return NOTIFY_DONE;
}

void mmc_register_extcon(struct mmc_host *host)
{
	struct extcon_dev *extcon = host->extcon;
	int err;

	if (!extcon)
		return;

	host->card_detect_nb.notifier_call = mmc_card_detect_notifier;
	err = extcon_register_notifier(extcon, EXTCON_MECHANICAL,
				       &host->card_detect_nb);
	if (err) {
		dev_err(mmc_dev(host), "%s: extcon_register_notifier() failed ret=%d\n",
			__func__, err);
		host->caps |= MMC_CAP_NEEDS_POLL;
	}
}
EXPORT_SYMBOL(mmc_register_extcon);

void mmc_unregister_extcon(struct mmc_host *host)
{
	struct extcon_dev *extcon = host->extcon;
	int err;

	if (!extcon)
		return;

	err = extcon_unregister_notifier(extcon, EXTCON_MECHANICAL,
					 &host->card_detect_nb);
	if (err)
		dev_err(mmc_dev(host), "%s: extcon_unregister_notifier() failed ret=%d\n",
			__func__, err);
}
EXPORT_SYMBOL(mmc_unregister_extcon);

/* Register an alternate interrupt service routine for
 * the card-detect GPIO.
 */
void mmc_gpio_set_cd_isr(struct mmc_host *host,
			 irqreturn_t (*isr)(int irq, void *dev_id))
{
	struct mmc_gpio *ctx = host->slot.handler_priv;

	WARN_ON(ctx->cd_gpio_isr);
	ctx->cd_gpio_isr = isr;
}
EXPORT_SYMBOL(mmc_gpio_set_cd_isr);

/**
 * mmc_gpio_request_cd - request a gpio for card-detection
 * @host: mmc host
 * @gpio: gpio number requested
 * @debounce: debounce time in microseconds
 *
 * As devm_* managed functions are used in mmc_gpio_request_cd(), client
 * drivers do not need to worry about freeing up memory.
 *
 * If GPIO debouncing is desired, set the debounce parameter to a non-zero
 * value. The caller is responsible for ensuring that the GPIO driver associated
 * with the GPIO supports debouncing, otherwise an error will be returned.
 *
 * Returns zero on success, else an error.
 */
int mmc_gpio_request_cd(struct mmc_host *host, unsigned int gpio,
			unsigned int debounce)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;
	int ret;

	ret = devm_gpio_request_one(host->parent, gpio, GPIOF_DIR_IN,
				    ctx->cd_label);
	if (ret < 0)
		/*
		 * don't bother freeing memory. It might still get used by other
		 * slot functions, in any case it will be freed, when the device
		 * is destroyed.
		 */
		return ret;

	if (debounce) {
		ret = gpio_set_debounce(gpio, debounce);
		if (ret < 0)
			return ret;
	}

	ctx->override_cd_active_level = true;
	ctx->cd_gpio = gpio_to_desc(gpio);
#ifdef CONFIG_MMC_SD_DEFERRED_RESUME
	ctx->pending_detect = false;
	ctx->suspended = false;
#endif
	return 0;
}
EXPORT_SYMBOL(mmc_gpio_request_cd);

/**
 * mmc_gpiod_request_cd - request a gpio descriptor for card-detection
 * @host: mmc host
 * @con_id: function within the GPIO consumer
 * @idx: index of the GPIO to obtain in the consumer
 * @override_active_level: ignore %GPIO_ACTIVE_LOW flag
 * @debounce: debounce time in microseconds
 * @gpio_invert: will return whether the GPIO line is inverted or not, set
 * to NULL to ignore
 *
 * Use this function in place of mmc_gpio_request_cd() to use the GPIO
 * descriptor API.  Note that it must be called prior to mmc_add_host()
 * otherwise the caller must also call mmc_gpiod_request_cd_irq().
 *
 * Returns zero on success, else an error.
 */
int mmc_gpiod_request_cd(struct mmc_host *host, const char *con_id,
			 unsigned int idx, bool override_active_level,
			 unsigned int debounce, bool *gpio_invert)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;
	struct gpio_desc *desc;
	int ret;

	if (!con_id)
		con_id = ctx->cd_label;

	desc = devm_gpiod_get_index(host->parent, con_id, idx, GPIOD_IN);
	if (IS_ERR(desc))
		return PTR_ERR(desc);

	if (debounce) {
		ret = gpiod_set_debounce(desc, debounce);
		if (ret < 0)
			return ret;
	}

	if (gpio_invert)
		*gpio_invert = !gpiod_is_active_low(desc);

	ctx->override_cd_active_level = override_active_level;
	ctx->cd_gpio = desc;

	return 0;
}
EXPORT_SYMBOL(mmc_gpiod_request_cd);

/**
 * mmc_gpiod_request_ro - request a gpio descriptor for write protection
 * @host: mmc host
 * @con_id: function within the GPIO consumer
 * @idx: index of the GPIO to obtain in the consumer
 * @override_active_level: ignore %GPIO_ACTIVE_LOW flag
 * @debounce: debounce time in microseconds
 * @gpio_invert: will return whether the GPIO line is inverted or not,
 * set to NULL to ignore
 *
 * Use this function in place of mmc_gpio_request_ro() to use the GPIO
 * descriptor API.
 *
 * Returns zero on success, else an error.
 */
int mmc_gpiod_request_ro(struct mmc_host *host, const char *con_id,
			 unsigned int idx, bool override_active_level,
			 unsigned int debounce, bool *gpio_invert)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;
	struct gpio_desc *desc;
	int ret;

	if (!con_id)
		con_id = ctx->ro_label;

	desc = devm_gpiod_get_index(host->parent, con_id, idx, GPIOD_IN);
	if (IS_ERR(desc))
		return PTR_ERR(desc);

	if (debounce) {
		ret = gpiod_set_debounce(desc, debounce);
		if (ret < 0)
			return ret;
	}

	if (gpio_invert)
		*gpio_invert = !gpiod_is_active_low(desc);

	ctx->override_ro_active_level = override_active_level;
	ctx->ro_gpio = desc;

	return 0;
}
EXPORT_SYMBOL(mmc_gpiod_request_ro);

void mmc_gpio_init_uim2(struct mmc_host *host, unsigned int gpio)
{
	struct mmc_gpio *ctx;

	ctx = host->slot.handler_priv;

	ctx->uim2_gpio = gpio;

	pr_info("## %s: %s: gpio=%d\n", mmc_hostname(host), __func__, gpio);

	mmc_gpio_set_uim2_en(host, 0);
}
EXPORT_SYMBOL(mmc_gpio_init_uim2);

void mmc_gpio_set_uim2_en(struct mmc_host *host, int value)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;

	if (!ctx || !gpio_is_valid(ctx->uim2_gpio)) {
		pr_err("## %s: gpio_set failure: ctx=%p, uim2_gpio=%d\n",
			mmc_hostname(host), ctx, ctx ? ctx->uim2_gpio : 0);
		return;
	}
	gpio_set_value(ctx->uim2_gpio, value);
	pr_info("## %s: %s: gpio=%d value=%d\n", mmc_hostname(host), __func__,
			ctx->uim2_gpio, value);
}
EXPORT_SYMBOL(mmc_gpio_set_uim2_en);

void mmc_gpio_tray_close_set_uim2(struct mmc_host *host, int value)
{
	struct mmc_gpio *ctx = host->slot.handler_priv;

	if (ctx && ctx->status)
		mmc_gpio_set_uim2_en(host, value);
}
EXPORT_SYMBOL(mmc_gpio_tray_close_set_uim2);
