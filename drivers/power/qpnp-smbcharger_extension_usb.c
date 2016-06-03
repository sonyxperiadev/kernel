/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
#include <linux/fb.h>
#endif

/*
 * qpnp-smbcharger.c macros
 */
#define IDEV_STS	0x8

#define USB_DP		67
#define USB_DM		68


/*
 * qpnp-smbcharger_extension_usb.c macros
 */
#define DEFAULT_PROP1000_MA		1000
#define DEFAULT_PROP500_MA		500

#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
#define USB_ID_POLL_UP_INTERVAL		1000
#define USB_ID_POLL_UP_PERIOD		100
#endif


/*
 * qpnp-smbcharger.c functions
 */
static int smbchg_read(struct smbchg_chip *chip, u8 *val, u16 addr, int count);
static int get_type(u8 type_reg);


/*
 * qpnp-smbcharger_extension_usb.c functions
 */
#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
/*
 * ID poll part
 */
static void id_poll_queue(struct smbchg_chip *chip)
{
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;

	queue_delayed_work(id_poll->q, &id_poll->dw,
					msecs_to_jiffies(id_poll->up_interval));
}

static void id_poll_cancel(struct smbchg_chip *chip)
{
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;

	cancel_delayed_work(&id_poll->dw);
}

static ssize_t id_poll_up_interval_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;

	if (!chip || !id_poll->enable)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%u\n", id_poll->up_interval);
}

static ssize_t id_poll_up_interval_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;
	unsigned long flags;

	if (!chip || !id_poll->enable)
		return -EINVAL;

	if (kstrtou32(buf, 0, &id_poll->up_interval) < 0) {
		pr_err("id_poll_up_interval cannot read value\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&id_poll->lock, flags);
	if (id_poll->start) {
		/* restart id poll with new interval value. */
		id_poll_cancel(chip);
		id_poll_queue(chip);
	}
	spin_unlock_irqrestore(&id_poll->lock, flags);

	return size;
}

static DEVICE_ATTR(id_poll_up_interval, S_IRUGO | S_IWUSR,
						id_poll_up_interval_show,
						id_poll_up_interval_store);

static ssize_t id_poll_up_period_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;

	if (!chip || !id_poll->enable)
		return -EINVAL;

	return snprintf(buf, PAGE_SIZE, "%u\n", id_poll->up_period);
}

static ssize_t id_poll_up_period_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct smbchg_chip *chip = dev_get_drvdata(dev);
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;

	if (!chip || !id_poll->enable)
		return -EINVAL;

	if (kstrtou32(buf, 0, &id_poll->up_period) < 0) {
		pr_err("id_poll_up_period cannot read value\n");
		return -EINVAL;
	}

	return size;
}

static DEVICE_ATTR(id_poll_up_period, S_IRUGO | S_IWUSR,
						id_poll_up_period_show,
						id_poll_up_period_store);

static void id_poll_pullup(struct smbchg_chip *chip, int pullup)
{
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;

	if (pullup) {
		dev_dbg(chip->dev, "%s: pull up ID pin\n", __func__);
		if (gpio_is_valid(id_poll->pd_gpio))
			gpio_set_value(id_poll->pd_gpio, !id_poll->pd_invert);
	} else {
		if (gpio_is_valid(id_poll->pd_gpio))
			gpio_set_value(id_poll->pd_gpio, id_poll->pd_invert);
		dev_dbg(chip->dev, "%s: pull down ID pin\n", __func__);
	}
}

static void id_poll_update(struct smbchg_chip *chip)
{
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;

	if (id_poll->otg_present || id_poll->lcd_blanked) {
		id_poll_cancel(chip);
		id_poll->start = false;
		if (!completion_done(&id_poll->cancel_poll))
			complete(&id_poll->cancel_poll);
		if (!id_poll->otg_present)
			id_poll_pullup(chip, 0);
	} else if (!id_poll->otg_present && !id_poll->lcd_blanked &&
							!id_poll->start) {
		id_poll_pullup(chip, 0);
		id_poll->start = true;
		id_poll_queue(chip);
	}
}

static void somc_usb_id_update(struct smbchg_chip *chip, bool otg_present)
{
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;
	unsigned long flags;

	if (!id_poll->enable)
		return;

	spin_lock_irqsave(&id_poll->lock, flags);
	id_poll->otg_present = otg_present;
	id_poll_update(chip);
	spin_unlock_irqrestore(&id_poll->lock, flags);
}

static void id_poll_work(struct work_struct *w)
{
	struct smbchg_chip *chip = container_of(w, struct smbchg_chip,
						usb_params.id_poll.dw.work);
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;
	unsigned long flags;

	spin_lock_irqsave(&id_poll->lock, flags);
	if (!id_poll->start) {
		spin_unlock_irqrestore(&id_poll->lock, flags);
		return;
	}
	id_poll_queue(chip);
	init_completion(&id_poll->cancel_poll);
	spin_unlock_irqrestore(&id_poll->lock, flags);

	pr_smb(PR_MISC, "id poll, interval=%u ms, period=%u us\n",
						id_poll->up_interval,
						id_poll->up_period);

	id_poll_pullup(chip, 1);

	wait_for_completion_timeout(&id_poll->cancel_poll,
					usecs_to_jiffies(id_poll->up_period));

	spin_lock_irqsave(&id_poll->lock, flags);
	if (id_poll->start)
		id_poll_pullup(chip, 0);
	spin_unlock_irqrestore(&id_poll->lock, flags);
}

#ifdef CONFIG_FB
static int fb_notifier_callback(struct notifier_block *self,
					 unsigned long event, void *data)
{
	struct smbchg_chip *chip = container_of(self, struct smbchg_chip,
						usb_params.id_poll.fb_notif);
	struct usb_id_poll *id_poll = &chip->usb_params.id_poll;
	struct fb_event *evdata = data;
	unsigned int blanked;
	unsigned long flags;

	if (!id_poll->enable || !evdata || !evdata->data ||
							event != FB_EVENT_BLANK)
		return 0;

	blanked = !(*(unsigned int *)(evdata->data) == FB_BLANK_UNBLANK);

	pr_smb(PR_MISC, "receive fb event blank=%u->%u, otg_present=%d\n",
			id_poll->lcd_blanked, blanked, id_poll->otg_present);

	if (blanked == id_poll->lcd_blanked)
		return 0;

	spin_lock_irqsave(&id_poll->lock, flags);
	id_poll->lcd_blanked = blanked;
	id_poll_update(chip);
	spin_unlock_irqrestore(&id_poll->lock, flags);

	return 0;
}
#endif
#endif /* CONFIG_QPNP_SMBCHARGER_ID_POLL */


/*
 * Proprietary charger part
 */
static int get_usb_type(struct smbchg_chip *chip)
{
	int rc;
	u8 reg;

	rc = smbchg_read(chip, &reg, chip->misc_base + IDEV_STS, 1);
	if (rc < 0) {
		dev_err(chip->dev, "Couldn't read status 5 rc = %d\n", rc);
		return rc;
	}
	return get_type(reg);
}

static int get_prop_usb_dp_voltage_now(struct smbchg_chip *chip)
{
	int rc = 0;
	struct qpnp_vadc_result results;

	if (IS_ERR_OR_NULL(chip->usb_params.vadc_usb_dp)) {
		chip->usb_params.vadc_usb_dp =
					qpnp_get_vadc(chip->dev, "usb_dp");
		if (IS_ERR(chip->usb_params.vadc_usb_dp))
			return PTR_ERR(chip->usb_params.vadc_usb_dp);
	}

	rc = qpnp_vadc_read(chip->usb_params.vadc_usb_dp, USB_DP, &results);
	if (rc) {
		pr_err("Unable to read usb_dp rc=%d\n", rc);
		return 0;
	} else {
		return results.physical;
	}
}

static int get_prop_usb_dm_voltage_now(struct smbchg_chip *chip)
{
	int rc = 0;
	struct qpnp_vadc_result results;

	if (IS_ERR_OR_NULL(chip->usb_params.vadc_usb_dm)) {
		chip->usb_params.vadc_usb_dm =
					qpnp_get_vadc(chip->dev, "usb_dm");
		if (IS_ERR(chip->usb_params.vadc_usb_dm))
			return PTR_ERR(chip->usb_params.vadc_usb_dm);
	}

	rc = qpnp_vadc_read(chip->usb_params.vadc_usb_dm, USB_DM, &results);
	if (rc) {
		pr_err("Unable to read usb_dm rc=%d\n", rc);
		return 0;
	} else {
		return results.physical;
	}
}

static int get_prop_proprietary_charger(struct smbchg_chip *chip)
{
	#define IN_RANGE(val, hi, lo) ((hi >= val) && (val >= lo))
	struct {
		struct {
			int hi;
			int lo;
		} dp, dm;
		int type;
		char *name;
	} chgs[] = {
		{{3600000, 3000000}, {3600000, 3000000},
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY,
				"Sony"},
		{{3000000, 2400000}, {3000000, 2400000},
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY,
				"Proprietary 12w"},
		{{3000000, 2400000}, {2300000, 1700000},
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY,
				"Proprietary 10w"},
		{{2300000, 1700000}, {3000000, 2400000},
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY_1000MA,
				"Proprietary 5w"},
		{{2300000, 1700000}, {2300000, 1700000},
				POWER_SUPPLY_SUB_TYPE_PROPRIETARY_500MA,
				"Proprietary 2.5w"},
	};
	int dp, dm;
	int ret;
	int i;

	dp = get_prop_usb_dp_voltage_now(chip);
	if (IS_ERR_VALUE(dp)) {
		dev_err(chip->dev, "%s: read D+ voltage fail\n", __func__);
		return dp;
	}

	dm = get_prop_usb_dm_voltage_now(chip);
	if (IS_ERR_VALUE(dm)) {
		dev_err(chip->dev, "%s: read D- voltage fail\n", __func__);
		return dm;
	}

	ret = POWER_SUPPLY_SUB_TYPE_UNKNOWN;
	for (i = 0; i < (sizeof(chgs) / sizeof(chgs[0])); i++) {
		if (IN_RANGE(dp, chgs[i].dp.hi, chgs[i].dp.lo) &&
				IN_RANGE(dm, chgs[i].dm.hi, chgs[i].dm.lo)) {
			pr_smb(PR_MISC, "%s: %s charger, D+=%d, D-=%d\n",
					__func__, chgs[i].name, dp, dm);
			ret = chgs[i].type;
			break;
		}
	}

	if (ret == POWER_SUPPLY_SUB_TYPE_UNKNOWN) {
		dev_warn(chip->dev,
				"%s: voltage not in range, D+=%d, D-=%d\n",
							__func__, dp, dm);
		ret = -ERANGE;
	}

	return ret;
}

/*
 * USB OCP part
 */
static const char *rdev_get_name(struct regulator_dev *rdev)
{
	if (rdev->constraints && rdev->constraints->name)
		return rdev->constraints->name;
	else if (rdev->desc->name)
		return rdev->desc->name;
	else
		return "";
}

static int somc_usb_otg_regulator_register_ocp_notification(
				struct regulator_dev *rdev,
				struct regulator_ocp_notification *notification)
{
	struct smbchg_chip *chip = rdev_get_drvdata(rdev);
	struct somc_usb_ocp *ocp = &chip->usb_params.ocp;
	unsigned long flags;

	spin_lock_irqsave(&ocp->lock, flags);
	if (notification)
		/* register ocp notification */
		ocp->notification = *notification;
	else
		/* unregister ocp notification */
		memset(&ocp->notification, 0, sizeof(ocp->notification));
	spin_unlock_irqrestore(&ocp->lock, flags);

	pr_smb(PR_MISC, "%s: registered ocp notification(notify=%p, ctxt=%p)\n",
						rdev_get_name(rdev),
						ocp->notification.notify,
						ocp->notification.ctxt);

	return 0;
}

static int somc_usb_otg_regulator_ocp_notify(struct smbchg_chip *chip)
{
	struct somc_usb_ocp *ocp = &chip->usb_params.ocp;
	unsigned long flags;

	spin_lock_irqsave(&ocp->lock, flags);
	if (ocp->notification.notify)
		ocp->notification.notify(ocp->notification.ctxt);
	spin_unlock_irqrestore(&ocp->lock, flags);

	return 0;
}

static int somc_usb_register(struct smbchg_chip *chip)
{
	struct usb_somc_params *params = &chip->usb_params;
	struct somc_usb_ocp *ocp = &params->ocp;
#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
	struct device_node *node = chip->dev->of_node;
	struct usb_id_poll *id_poll = &params->id_poll;
#endif

	memset(&ocp->notification, 0, sizeof(ocp->notification));
	spin_lock_init(&ocp->lock);

#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
	id_poll->pd_gpio = -1;
	id_poll->enable = of_property_read_bool(node, "id_poll_enable");
	if (id_poll->enable) {
		dev_info(chip->dev, "id poll enabled\n");

		INIT_DELAYED_WORK(&id_poll->dw, id_poll_work);
		id_poll->q = create_singlethread_workqueue("id_poll_q");

		id_poll->up_interval = USB_ID_POLL_UP_INTERVAL;
		of_property_read_u32(node, "id_poll_up_interval",
						&id_poll->up_interval);
		dev_dbg(chip->dev, "id_poll_up_interval=%dms\n",
						id_poll->up_interval);
		device_create_file(chip->dev, &dev_attr_id_poll_up_interval);

		id_poll->up_period = USB_ID_POLL_UP_PERIOD;
		of_property_read_u32(node, "id_poll_up_period",
						&id_poll->up_period);
		dev_dbg(chip->dev, "id_poll_up_period=%dus\n",
						id_poll->up_period);
		device_create_file(chip->dev, &dev_attr_id_poll_up_period);

		id_poll->pd_gpio = of_get_named_gpio(node,
						"id_poll_pd_gpio", 0);
		if (!gpio_is_valid(id_poll->pd_gpio))
			dev_info(chip->dev, "id_poll_pd is missing\n");
		else
			of_property_read_u32(node, "id_poll_pd_invert",
						&id_poll->pd_invert);

#ifdef CONFIG_FB
		id_poll->fb_notif.notifier_call = fb_notifier_callback;
		if (fb_register_client(&id_poll->fb_notif))
			dev_err(chip->dev, "failed to register fb_notifier\n");
#endif

		init_completion(&id_poll->cancel_poll);
		spin_lock_init(&id_poll->lock);
		id_poll->start = false;
	}
#endif

	pr_smb(PR_MISC, "somc usb register success\n");
	return 0;
}

static void somc_usb_unregister(struct smbchg_chip *chip)
{
#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
	struct usb_somc_params *params = &chip->usb_params;
	struct usb_id_poll *id_poll = &params->id_poll;

	if (id_poll->enable) {
#if defined(CONFIG_FB)
		fb_unregister_client(&id_poll->fb_notif);
#endif
		device_remove_file(chip->dev,
					&dev_attr_id_poll_up_interval);
		device_remove_file(chip->dev, &dev_attr_id_poll_up_period);
		cancel_delayed_work_sync(&id_poll->dw);
		destroy_workqueue(id_poll->q);
	}
#endif
}
