/* drivers/leds/leds-lm3533.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/leds.h>
#include <linux/leds-lm3533_ng.h>


#define ALS_ATTR(_n, _name, _mode, _show, _store) \
struct device_attribute attr_als##_n = __ATTR(_name, _mode, _show, _store)

#define ALS_ZONE_NUM 5

enum lm3533_als_id {
	LM3533_ALS1,
	LM3533_ALS2,
	LM3533_ALS3,
	LM3533_ALS_NUM
};

enum {
	LM3533_SCALERS_NUM = 5,
	LM3533_SCALER_MIN  = 1,
	LM3533_SCALER_MAX  = 16,
};

struct lm3533_intf {
	struct led_classdev ldev;
	u8 banks;
};

struct lm3533_data {
	struct lm3533_platform_data *pdata;
	struct lm3533_intf *intf[LM3533_BANK_NUM];
	struct i2c_client *i2c;
	bool power_off;
	u8 als_tgt[LM3533_ALS_NUM][ALS_ZONE_NUM];
	u8 ptrn_scaler[LM3533_SCALERS_NUM];
};

enum {
	REG_OUT_CONF1   = 0x10,
	REG_OUT_CONF2   = 0x11,
	REG_BANK_ENA    = 0x27,
	REG_ANODE_CON   = 0x25,
	REG_HVBCTL      = 0x1a,
	REG_LVBCTL_BASE = 0x1b,
	REG_PWM_BASE    = 0x14,
	REG_FSC_BASE    = 0x1f,
	REG_BR_BASE     = 0x40,
	REG_CPUMP       = 0x26,
	REG_SCALER_BASE = 0x29,
	REG_ALS_AVGT    = 0x31,
	REG_ALS_ALGO    = 0x32,
	REG_ALS_DDELAY  = 0x33,
	REG_ALS_CURRENT    = 0x30,
	REG_ALS_ZONES_BASE = 0x50,
	REG_ALS_TGT_BASE   = 0x60,
	REG_OVP_BOOST_PWM  = 0x2c,
};

enum {
	LM3533_ALS_ENABLE  = 1 << 0,
	LM3533_SC_Z1_SHIFT = 0,
	LM3533_SC_Z1_OFFS  = 2,
	LM3533_SC_Z2_SHIFT = 0,
	LM3533_SC_Z2_OFFS  = 1,
	LM3533_SC_Z3_SHIFT = 4,
	LM3533_SC_Z3_OFFS  = 1,
	LM3533_SC_Z4_SHIFT = 0,
	LM3533_SC_Z4_OFFS  = 0,
	LM3533_SC_Z5_SHIFT = 4,
	LM3533_SC_Z5_OFFS  = 0,
};

static int lm3533_write(struct lm3533_data *lm, u8 val, u8 reg)
{
	int rc;
	struct i2c_client *i2c = lm->i2c;

	dev_vdbg(&i2c->dev, "%s: 0x%02x to 0x%02x\n", __func__, val, reg);
	rc = i2c_smbus_write_byte_data(i2c, reg, val);
	if (rc)
		dev_err(&i2c->dev, "failed writing at 0x%02x (%d)\n", reg, rc);
	return rc;
}

static int lm3533_setup_banks(struct lm3533_data *lm, u8 enabled)
{
	struct lm3533_bank_config *bank;
	enum lm3533_control_bank b;
	u8 hv_ctl = 0;
	int rc;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		if (!((1 << b) && enabled))
			continue;
		bank = &lm->pdata->b_cnf[b];

		rc = lm3533_write(lm, bank->fsc, REG_FSC_BASE + b);
		if (rc)
			goto err_exit;
		rc = lm3533_write(lm, bank->pwm, REG_PWM_BASE + b);
		if (rc)
			goto err_exit;

		if (b <= LM3533_CBNKB) {
			hv_ctl |= bank->ctl;
		} else {
			rc = lm3533_write(lm, bank->ctl,
				REG_LVBCTL_BASE + b - LM3533_CBNKC);
			if (rc)
				goto err_exit;
		}
	}
	rc = lm3533_write(lm, hv_ctl, REG_HVBCTL);
	if (rc)
		goto err_exit;
	rc = lm3533_write(lm, enabled, REG_BANK_ENA);

err_exit:
	return rc;
}

static int lm3533_setup_leds(struct lm3533_data *lm)
{
	enum lm3533_led i;
	enum lm3533_control_bank bank;
	struct lm3533_led_config *leds = lm->pdata->l_cnf;
	u8 ctl1 = 0, ctl2 = 0, bena = 0, anode = 0;
	int rc;

	for (i = LM3533_HVLED1; i < LM3533_LED_NUM; i++) {
		if (!leds[i].connected)
			continue;

		bank = leds[i].bank;
		bena |= 1 << bank;

		switch (i) {
		case LM3533_HVLED1:
		case LM3533_HVLED2:
			if (bank != LM3533_CBNKA && bank != LM3533_CBNKB)
				goto err_exit;
			ctl1 |= bank << i;
			break;
		case LM3533_LVLED1:
		case LM3533_LVLED2:
		case LM3533_LVLED3:
			if (bank < LM3533_CBNKC)
				goto err_exit;
			ctl1 |= (bank - LM3533_CBNKC) <<
					(((i - LM3533_LVLED1) * 2) + 2);
			break;
		case LM3533_LVLED4:
		case LM3533_LVLED5:
			ctl2 |= (bank - LM3533_CBNKC) <<
					((i - LM3533_LVLED4) * 2);
			break;
		default:
			goto err_exit;
		}
		if (leds[i].cpout)
			anode |= 1 << i;
	}
	rc = lm3533_write(lm, ctl1, REG_OUT_CONF1);
	rc = rc ? rc : lm3533_write(lm, ctl2, REG_OUT_CONF2);
	rc = rc ? rc : lm3533_write(lm, anode, REG_ANODE_CON);
	rc = rc ? rc : lm3533_setup_banks(lm, bena);
	return rc;
err_exit:
	dev_err(&lm->i2c->dev, "%s: bad parameters\n", __func__);
	return -EINVAL;
}

static int lm3533_chip_config(struct lm3533_data *lm)
{
	struct lm3533_platform_data *p = lm->pdata;
	int rc;

	rc = lm3533_write(lm, p->ovp_boost_pwm, REG_OVP_BOOST_PWM);
	rc = rc ? rc : lm3533_write(lm, p->cpump_cnf, REG_CPUMP);
	rc = rc ? rc : lm3533_write(lm, p->als_input_current, REG_ALS_CURRENT);
	rc = rc ? rc : lm3533_write(lm, p->als_algo, REG_ALS_ALGO);
	rc = rc ? rc : lm3533_write(lm, p->als_down_delay, REG_ALS_DDELAY);
	rc = rc ? rc : lm3533_setup_leds(lm);
	return rc;
}

static int lm3533_als_set(struct lm3533_data *lm, bool enable)
{
	u8 ctrl = lm->pdata->als_control;

	dev_dbg(&lm->i2c->dev, "%s: als %s.\n", __func__,
			(enable ? "on" : "off"));
	if (enable)
		ctrl |= LM3533_ALS_ENABLE;
	else
		ctrl &= ~LM3533_ALS_ENABLE;
	return lm3533_write(lm, ctrl, REG_ALS_AVGT);
}

static inline struct lm3533_intf *ldev_to_intf(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct lm3533_intf, ldev);
}

static void lm3533_led_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;

	if (value <= LED_OFF)
		value = 0;
	else if (value >= LED_FULL)
		value = 255;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		if (lm3533_write(lm, value, REG_BR_BASE + b))
			break;
	}
}

static int lm3533_add_intf(struct lm3533_data *lm, const char *nm,
			enum lm3533_control_bank bank)
{
	unsigned i;
	struct device *dev = &lm->i2c->dev;
	struct lm3533_intf *intf = NULL;

	for (i = 0; i < LM3533_BANK_NUM && !intf; i++) {
		if (!lm->intf[i]) {
			intf = kzalloc(sizeof(struct lm3533_intf), GFP_KERNEL);
			if (!intf) {
				dev_err(dev, "%s: mo memory\n", __func__);
				break;
			}
			intf->ldev.name = nm;
			intf->ldev.brightness = LED_OFF;
			intf->ldev.brightness_set = lm3533_led_brightness;
			intf->ldev.blink_set = NULL;
			lm->intf[i] = intf;
			dev_info(dev, "New led '%s'\n", nm);

		} else if (!strncmp(nm, lm->intf[i]->ldev.name, strlen(nm))) {
			intf = lm->intf[i];
		}
	}
	if (intf) {
		intf->banks |= 1 << bank;
		return 0;
	}
	dev_err(dev, "%s: no feee space for interfaces\n", __func__);
	return -ENOMEM;
}

static int lm3533_setup_interfaces(struct lm3533_data *lm)
{
	const char *name;
	int rc;
	enum lm3533_control_bank b;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		name = lm->pdata->b_cnf[b].iname;
		if (!name)
			continue;
		rc = lm3533_add_intf(lm, name, b);
		if (rc)
			return rc;
	}
	return 0;
}

static void lm3533_cleanup(struct lm3533_data *lm)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(lm->intf); i++) {
		kfree(lm->intf[i]);
		lm->intf[i] = NULL;
	}
}

static int lm3533_register_ldevs(struct lm3533_data *lm)
{
	int i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++) {
		rc = led_classdev_register(&lm->i2c->dev, &lm->intf[i]->ldev);
		if (rc)
			goto roll_back;
	}
	return rc;
roll_back:
	while (--i >= 0)
		led_classdev_unregister(&lm->intf[i]->ldev);
	dev_err(&lm->i2c->dev, "%s: failed, rc = %d.\n", __func__, rc);
	return rc;
}

static void lm3533_unregister_ldevs(struct lm3533_data *lm)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++)
		led_classdev_unregister(&lm->intf[i]->ldev);
}

static int lm3533_als_tgt_update(struct lm3533_data *lm, enum lm3533_als_id id,
		unsigned *targets)
{
	unsigned i;
	int rc;
	u8 *t = lm->als_tgt[id];

	for (i = 0; i < ALS_ZONE_NUM; i++) {
		rc = lm3533_write(lm, targets[i],
			REG_ALS_TGT_BASE + (id * ALS_ZONE_NUM) + i);
		if (rc)
			break;
		t[i] = targets[i];
	}
	return rc;
}


static int lm3533_ptrn_scaler_update(struct lm3533_data *lm, unsigned *sclrs)
{
	static const struct {
		u8 offset;
		u8 shift;
	} scaler_regs[ARRAY_SIZE(lm->ptrn_scaler)] = {
		[0] = {LM3533_SC_Z1_OFFS, LM3533_SC_Z1_SHIFT},
		[1] = {LM3533_SC_Z2_OFFS, LM3533_SC_Z2_SHIFT},
		[2] = {LM3533_SC_Z3_OFFS, LM3533_SC_Z3_SHIFT},
		[3] = {LM3533_SC_Z4_OFFS, LM3533_SC_Z4_SHIFT},
		[4] = {LM3533_SC_Z5_OFFS, LM3533_SC_Z5_SHIFT},
	};

	unsigned i;
	int rc;
	u8 val;
	u8 *t = lm->ptrn_scaler;

	for (i = 0; i < ARRAY_SIZE(lm->ptrn_scaler); i++) {
		val = sclrs[i] << scaler_regs[i].shift;
		rc = lm3533_write(lm, val,
				REG_SCALER_BASE + scaler_regs[i].offset);
		if (rc)
			break;
		t[i] = sclrs[i];
	}
	return rc;
}

static ssize_t lm3533_als_tgt_set(struct device *dev, enum lm3533_als_id id,
			const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev->parent);
	unsigned t[ALS_ZONE_NUM];
	int rc;
	int n = sscanf(buf, "%5u,%5u,%5u,%5u,%5u",
			&t[0], &t[1], &t[2], &t[3], &t[4]);

	if (n != ALS_ZONE_NUM)
		return -EINVAL;
	dev_dbg(dev, "%s: %u,%u,%u,%u,%u\n", __func__,
			t[0], t[1], t[2], t[3], t[4]);
	rc = lm3533_als_tgt_update(lm, id, t);
	return rc ? rc : count;
}

static ssize_t lm3533_als1_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	return lm3533_als_tgt_set(dev, LM3533_ALS1, buf, count);
}

static ssize_t lm3533_als2_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	return lm3533_als_tgt_set(dev, LM3533_ALS2, buf, count);
}

static ssize_t lm3533_als3_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	return lm3533_als_tgt_set(dev, LM3533_ALS3, buf, count);
}

static ssize_t lm3533_ptrn_scaler_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev->parent);
	unsigned t[LM3533_SCALERS_NUM];
	int rc;
	int i;
	int n = sscanf(buf, "%5u,%5u,%5u,%5u,%5u",
			&t[0], &t[1], &t[2], &t[3], &t[4]);

	if (n != LM3533_SCALERS_NUM)
		return -EINVAL;

	for (i = 0; i < LM3533_SCALERS_NUM; i++) {
		if (t[i] > LM3533_SCALER_MAX || t[i] < LM3533_SCALER_MIN)
			return -EINVAL;
	}
	rc = lm3533_ptrn_scaler_update(lm, t);
	dev_dbg(dev, "%s: %u,%u,%u,%u,%u\n", __func__,
			t[0], t[1], t[2], t[3], t[4]);
	return rc ? rc : count;
}

static ALS_ATTR(1, curve, S_IWUSR, NULL, lm3533_als1_store);
static ALS_ATTR(2, curve, S_IWUSR, NULL, lm3533_als2_store);
static ALS_ATTR(3, curve, S_IWUSR, NULL, lm3533_als3_store);
static DEVICE_ATTR(scaler, S_IWUSR, NULL, lm3533_ptrn_scaler_store);

static struct attribute *lm3533_als_attr[] = {
	&attr_als1.attr,
	&attr_als2.attr,
	&attr_als3.attr,
};

static int lm3533_to_intf(struct lm3533_data *lm, enum lm3533_control_bank b)
{
	unsigned i;

	for (i = 0; i < LM3533_BANK_NUM && lm->intf[i]; i++) {
		if (lm->intf[i]->banks & (1 << b))
			return i;
	}
	return -ENODEV;
}

static int lm3533_link_controls(struct lm3533_data *lm,
	enum lm3533_control_bank b, bool link_scaler,
	enum lm3533_als_id als, bool enable)
{
	int rc;
	int idx = lm3533_to_intf(lm, b);
	struct kobject *kobj = &lm->intf[idx]->ldev.dev->kobj;

	if (idx < 0)
		return idx;

	if (!enable) {
		sysfs_remove_file(kobj, lm3533_als_attr[als]);
		sysfs_remove_file(kobj, &dev_attr_scaler.attr);
		return 0;
	}
	if (als <  LM3533_ALS_NUM) {
		dev_dbg(&lm->i2c->dev, "Linking '%s' to als%d\n",
				lm->intf[idx]->ldev.name, als + 1);
		rc = sysfs_create_file(kobj, lm3533_als_attr[als]);
		if (rc)
			goto err_exit;
	}
	if (link_scaler) {
		dev_dbg(&lm->i2c->dev, "Linking '%s' to blink scaler\n",
				lm->intf[idx]->ldev.name);
		rc = sysfs_create_file(kobj, &dev_attr_scaler.attr);
		if (rc)
			goto err_exit;
	}
	return 0;
err_exit:
	dev_err(&lm->i2c->dev, "Error creating attribute for '%s' (%d)\n",
				lm->intf[idx]->ldev.name, rc);
	return rc;
}

static int lm3533_link_als(struct lm3533_data *lm, bool enable)
{
	int rc;
	struct lm3533_bank_config *bank;
	enum lm3533_control_bank b;
	u8 tmp;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		enum lm3533_als_id link_als = LM3533_ALS_NUM;
		bool link_scaler = false;

		bank = &lm->pdata->b_cnf[b];
		switch (b) {
		case  LM3533_CBNKA:
			if (bank->ctl & LM3533_HVA_ALS1)
				link_als = LM3533_ALS1;
			break;
		case LM3533_CBNKB:
			if (bank->ctl & LM3533_HVB_ALS2)
				link_als = LM3533_ALS2;
			break;
		default:
			tmp = bank->ctl & (LM3533_LV_ALS2 | LM3533_LV_ALS3);
			if (tmp == LM3533_LV_ALS2)
				link_als = LM3533_ALS2;
			else if (tmp == LM3533_LV_ALS3)
				link_als = LM3533_ALS3;

			link_scaler = bank->blink_als_ctl;
			break;
		}
		if (link_als == LM3533_ALS_NUM && !link_scaler)
			continue;
		rc = lm3533_link_controls(lm, b, link_scaler, link_als, enable);
		if (rc)
			goto err_exit;
	}
err_exit:
	return rc;
}

static int __devinit lm3533_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lm3533_platform_data *pdata = client->dev.platform_data;
	struct device *dev = &client->dev;
	struct lm3533_data *lm;
	int rc;

	dev_info(dev, "%s\n", __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "%s: SMBUS byte data not supported\n", __func__);
		return -EIO;
	}
	if (!pdata) {
		dev_err(dev, "%s: configuration data required.\n", __func__);
		return -EINVAL;
	}
	if (pdata->setup) {
		rc = pdata->setup(dev);
		if (rc)
			return rc;
	}
	lm = kzalloc(sizeof(*lm), GFP_KERNEL);
	if (!lm) {
		rc = -ENOMEM;
		goto err_alloc_data_failed;
	}

	lm->i2c = client;
	lm->pdata = pdata;
	i2c_set_clientdata(client, lm);

	if (lm->pdata->power_on) {
		rc = lm->pdata->power_on(dev);
		if (rc) {
			dev_err(dev, "%s: failed to power on.\n", __func__);
			goto err_power_on;
		}
	}
	rc = lm3533_chip_config(lm);
	if (rc)
		goto err_configure;
	rc = lm3533_setup_interfaces(lm);
	if (rc)
		goto err_interfaces;
	rc = lm3533_register_ldevs(lm);
	if (rc)
		goto err_reg_ldevs;

	rc = lm3533_link_als(lm, true);
	if (rc) {
		(void)lm3533_link_als(lm, false);
		goto err_link_als;
	}
	rc = lm3533_als_set(lm, true);
	if (rc) {
		(void)lm3533_link_als(lm, false);
		goto err_config_als;
	}
	dev_info(dev, "%s: completed.\n", __func__);
	return 0;

err_config_als:
	(void)lm3533_link_als(lm, false);
err_link_als:
	lm3533_unregister_ldevs(lm);
err_reg_ldevs:
err_interfaces:
	lm3533_cleanup(lm);
err_configure:
	if (lm->pdata->power_off)
		lm->pdata->power_off(dev);
err_power_on:
	kfree(lm);
err_alloc_data_failed:
	if (pdata->teardown)
		pdata->teardown(dev);
	dev_err(dev, "%s: failed.\n", __func__);
	return rc;
}

static int __devexit lm3533_remove(struct i2c_client *client)
{
	struct lm3533_data *lm = i2c_get_clientdata(client);

	(void)lm3533_link_als(lm, false);
	lm3533_unregister_ldevs(lm);
	lm3533_cleanup(lm);
	if (lm->pdata->teardown)
		lm->pdata->teardown(&client->dev);
	kfree(lm);
	return 0;
}


#ifdef CONFIG_SUSPEND
static bool lm3533_is_lit(struct lm3533_data *lm)
{
	unsigned i;
	for (i = 0; i < LM3533_BANK_NUM && lm->intf[i]; i++) {
		if (lm->intf[i]->ldev.brightness > LED_OFF)
			return true;
	}
	return false;
}

static int lm3533_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm3533_data *lm = i2c_get_clientdata(client);

	dev_dbg(dev, "%s\n", __func__);
	(void)lm3533_als_set(lm, false);
	if (lm->pdata->power_off && !lm3533_is_lit(lm))
		lm->power_off = !lm->pdata->power_off(dev);
	return 0;
}

static int lm3533_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm3533_data *lm = i2c_get_clientdata(client);

	dev_dbg(dev, "%s\n", __func__);
	if (lm->power_off && lm->pdata->power_on) {
		lm->power_off = !!lm->pdata->power_on(dev);
		if (!lm->power_off)
			(void)lm3533_chip_config(lm);
	}
	(void)lm3533_als_set(lm, true);
	return 0;
}
#else
#define lm3533_suspend NULL
#define lm3533_resume NULL
#endif

static const struct dev_pm_ops lm3533_pm = {
	.suspend = lm3533_suspend,
	.resume = lm3533_resume,
};

static const struct i2c_device_id lm3533_id[] = {
	{LM3533_DEV_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, lm3533_id);

static struct i2c_driver lm3533_driver = {
	.driver = {
		.name = LM3533_DEV_NAME,
		.owner = THIS_MODULE,
		.pm = &lm3533_pm,
	},
	.probe = lm3533_probe,
	.remove = __devexit_p(lm3533_remove),
	.id_table = lm3533_id,
};

static int __init lm3533_init(void)
{
	int err = i2c_add_driver(&lm3533_driver);
	pr_info("%s: lm3533 LMU IC driver, built %s @ %s\n",
		 __func__, __DATE__, __TIME__);
	return err;
}

static void __exit lm3533_exit(void)
{
	i2c_del_driver(&lm3533_driver);
}

module_init(lm3533_init);
module_exit(lm3533_exit);

MODULE_AUTHOR("Aleksej Makarov <aleksej.makarov@sonyericsson.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("lm3533 LMU IC driver");
