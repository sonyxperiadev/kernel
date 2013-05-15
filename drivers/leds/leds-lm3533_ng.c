/* drivers/leds/leds-lm3533_ng.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012-2013 Sony Mobile Communications AB.
 *
 * Author: Aleksej Makarov <aleksej.makarov@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/leds.h>
#include <linux/leds-lm3533_ng.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pm_runtime.h>

static int autosuspend_delay_ms = 100;
module_param(autosuspend_delay_ms, int, S_IRUGO);

#define ALS_ATTR(_n, _name, _mode, _show, _store) \
struct device_attribute attr_als##_n = __ATTR(_name, _mode, _show, _store)

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
	LM3533_REG_ADDR_FIRST = 0x10,
	LM3533_REG_ADDR_LAST  = 0xff,
	LM3533_REG_NUM = LM3533_REG_ADDR_LAST - LM3533_REG_ADDR_FIRST + 1,
	LM3533_MAX_ALS_CURRENT = 1270,
	LM3533_ALS_CURRENT_PER_BIT = 10,
	LM3533_ALS_ZONE_NUM = 5,
	LM3533_PWR_DELAY_US = 1000,
	MAX_RR_MS  = 16789,
	MAX_RR_REG = 7,
	LM3533_LV_BANKS = (1 << LM3533_CBNKC) | (1 << LM3533_CBNKD) |
		(1 << LM3533_CBNKE) | (1 << LM3533_CBNKF),
};

struct lm3533_intf {
	struct led_classdev ldev;
	u16 dim_up;
	u16 dim_dn;
	u16 on_time;
	u32 off_time;
	u8 banks;
	bool als;
	u8 brightness;
	bool als_on_suspend;
};

struct lm3533_bank {
	u8 als_mask;
	u8 als_ctrl;
	u8 ctrl_reg;
};

struct lm3533_data {
	struct lm3533_platform_data const *pdata;
	struct lm3533_intf *intf[LM3533_BANK_NUM];
	struct lm3533_bank banks[LM3533_BANK_NUM];
	u8 enabled_als;
	u8 ptrn_ctl;
	struct i2c_client *i2c;
	bool power_off;
	u8 shadow[LM3533_REG_NUM];
	bool recovery_mode;
	struct mutex lock;
	int als_users;
	bool als_active;
	u8 als_reg_curr_on_suspend;
	u8 als_reg_curr_saved;
	bool lit_on_suspend;
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
	REG_ALS_R_SEL   = 0x30,
	REG_ALS_AVGT    = 0x31,
	REG_ALS_ALGO    = 0x32,
	REG_ALS_DDELAY  = 0x33,
	REG_ALS_ADC_AVG = 0x38,
	REG_ALS_CURRENT    = 0x30,
	REG_ALS_ZONES_BASE = 0x50,
	REG_ALS_TGT_BASE   = 0x60,
	REG_OVP_BOOST_PWM  = 0x2c,
	REG_START_UP_RR    = 0x12,
	REG_RUN_TIME_RR    = 0x13,
	REG_PTRN_ENABLE    = 0x28,
	REG_PATTERN_BASE   = 0x70,
	REG_PTRN_TDELAY    = 0x70,
	REG_PTRN_TLOW      = 0x71,
	REG_PTRN_THIGH     = 0x72,
	REG_PTRN_LOWLVL    = 0x73,
	REG_PTRN_TUP       = 0x74,
	REG_PTRN_TDOWN     = 0x75,
	REG_START_SHDN_T   = 0x12,
	REG_RT_TRANS_T     = 0x13,
	REG_LED_FAULT      = 0xb2,
};

#define SHADOW(p, x) ((p)->shadow[(x) - LM3533_REG_ADDR_FIRST])
#define PTRN_DELAY_REG(b) (REG_PTRN_TDELAY  + ((b) - LM3533_CBNKC) * 0x10)
#define PTRN_TLOW_REG(b)  (REG_PTRN_TLOW    + ((b) - LM3533_CBNKC) * 0x10)
#define PTRN_THIGH_REG(b) (REG_PTRN_THIGH   + ((b) - LM3533_CBNKC) * 0x10)
#define PTRN_LOWLVL_REG(b)(REG_PTRN_LOWLVL  + ((b) - LM3533_CBNKC) * 0x10)
#define PTRN_T_UP_REG(b)  (REG_PTRN_TUP     + ((b) - LM3533_CBNKC) * 0x10)
#define PTRN_T_DN_REG(b)  (REG_PTRN_TDOWN   + ((b) - LM3533_CBNKC) * 0x10)

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

#define T_STEP1        16384
#define T_THRESHOLD1    1000
#define T_STEP2       131072
#define T_THRESHOLD2    9782
#define T_STEP3       524288
#define REG_BASE_VAL1      0
#define REG_BASE_VAL2   0x3c
#define REG_BASE_VAL3   0x7f

enum lm3533_ptrn_delay {
	PTRN_D_LOW,
	PTRN_D_HIGH,
};

static const u16 dim_times[] = {
	3, 262, 524, 1049, 2097, 4194, 8389, 16780,
};

struct lm3533_reg_region {
	u8 start;
	u8 number;
};

static const struct lm3533_reg_region sync_regs[] = {
	{
		.start = REG_OUT_CONF1,
		.number = 29,
	},
	{
		.start = REG_ALS_CURRENT,
		.number = 3,
	},
	{
		.start = REG_BR_BASE,
		.number = 6,
	},
	{
		.start = REG_ALS_ZONES_BASE,
		.number = 8,
	},
	{
		.start = REG_ALS_TGT_BASE,
		.number = 15,
	},
};

static u8 t2reg(u32 *t_ms, enum lm3533_ptrn_delay type)
{
	u32 reg;
	u16 reg_base;
	u32 step;
	u32 t = *t_ms;

	if (t <= T_THRESHOLD1) {
		step = T_STEP1;
		reg_base = REG_BASE_VAL1;
		*t_ms = 0;
	} else if (type == PTRN_D_LOW && t >= T_THRESHOLD2) {
		step = T_STEP3;
		reg_base = REG_BASE_VAL3;
		t -= T_THRESHOLD2;
		*t_ms = T_THRESHOLD2;
	} else {
		step = T_STEP2;
		reg_base = REG_BASE_VAL2;
		t -= T_THRESHOLD1;
		*t_ms = T_THRESHOLD1;
	}
	reg = t * 1000 / step;
	if (reg > 0xff)
		reg = 0xff;
	*t_ms += step * (reg + 1) / 1000;
	return reg + reg_base;
}

static u8 tlow2reg(u32 *t_ms)
{
	u32 t = *t_ms;
	u8 r = t2reg(&t, PTRN_D_LOW);
	*t_ms = t;
	return r;
}

static u8 thigh2reg(u16 *t_ms)
{
	u32 t = *t_ms;
	u8 r = t2reg(&t, PTRN_D_HIGH);
	*t_ms = t;
	return r;
}

static u8 dim_t2reg(u16 *t_ms)
{
	unsigned i;
	for (i = 0; i < ARRAY_SIZE(dim_times) - 1; i++)
		if (*t_ms <= dim_times[i])
			break;
	*t_ms = dim_times[i];
	return i;
}

static int lm3533_sync_shadow(struct lm3533_data *lm)
{
	unsigned i, k;
	s32 rc;
	u8 reg;

	rc = pm_runtime_get_sync(&lm->i2c->dev);
	if (rc < 0) {
		pm_runtime_put_noidle(&lm->i2c->dev);
		return rc;
	}
	for (i = 0; i < ARRAY_SIZE(sync_regs); i++) {
		reg = sync_regs[i].start;
		for (k = 0; k < sync_regs[i].number; k++, reg++) {
			rc = i2c_smbus_read_byte_data(lm->i2c, reg);
			if (rc >= 0) {
				SHADOW(lm, reg) = rc;
				continue;
			}
			dev_err(&lm->i2c->dev, "failed at 0x%02x (%d)\n",
					reg, rc);
			goto exit;
		}
	}
exit:
	pm_runtime_mark_last_busy(&lm->i2c->dev);
	pm_runtime_put_autosuspend(&lm->i2c->dev);
	return rc < 0 ? rc : 0;
}

static int lm3533_sync_regs(struct lm3533_data *lm)
{
	unsigned i, k;
	int rc;
	u8 reg;

	for (i = 0; i < ARRAY_SIZE(sync_regs); i++) {
		reg = sync_regs[i].start;
		for (k = 0; k < sync_regs[i].number; k++, reg++) {
			rc = i2c_smbus_write_byte_data(lm->i2c,
					reg, SHADOW(lm, reg));
			if (!rc)
				continue;
			dev_err(&lm->i2c->dev, "failed at 0x%02x (%d)\n",
					reg, rc);
			return rc;
		}
	}
	return 0;
}

static int lm3533_power_on(struct lm3533_data *lm)
{
	int rc;
	dev_dbg(&lm->i2c->dev, "%s\n", __func__);
	if (!lm->pdata->power_on)
		return 0;
	rc = lm->pdata->power_on(&lm->i2c->dev);
	if (!rc) {
		usleep_range(LM3533_PWR_DELAY_US, LM3533_PWR_DELAY_US + 1000);
		rc = lm3533_sync_regs(lm);
	}
	return rc;
}

static int lm3533_power_off(struct lm3533_data *lm)
{
	dev_dbg(&lm->i2c->dev, "%s\n", __func__);
	return lm->pdata->power_off ? lm->pdata->power_off(&lm->i2c->dev) : 0;
}

static int lm3533_recover(struct lm3533_data *lm)
{
	int rc;
	if (!(lm->recovery_mode && lm->pdata->power_off
			&& lm->pdata->power_on)) {
		dev_warn(&lm->i2c->dev, "no recovery is possible\n");
		return -ENOSYS;
	}
	dev_info(&lm->i2c->dev, "recovering\n");
	lm3533_power_off(lm);
	rc = lm3533_power_on(lm);
	if (!rc)
		dev_info(&lm->i2c->dev, "recovered\n");
	else
		dev_info(&lm->i2c->dev, "failed to recover\n");
	return rc;
}

static int lm3533_write(struct lm3533_data *lm, u8 val, u8 reg)
{
	int rc;
	struct i2c_client *i2c = lm->i2c;

	dev_dbg(&lm->i2c->dev, "%s: 0x%02x to 0x%02x\n", __func__, val, reg);
	rc = pm_runtime_get_sync(&lm->i2c->dev);
	if (rc < 0) {
		pm_runtime_put_noidle(&lm->i2c->dev);
		return rc;
	}
	rc = i2c_smbus_write_byte_data(i2c, reg, val);
	if (!rc) {
		SHADOW(lm, reg) = val;
		goto done;
	}
	dev_err(&i2c->dev, "failed writing at 0x%02x (%d)\n", reg, rc);
	if (lm3533_recover(lm))
		goto done;
	rc = i2c_smbus_write_byte_data(i2c, reg, val);
	if (rc)
		dev_err(&i2c->dev, "failed writing at 0x%02x (%d)\n", reg, rc);
	else
		SHADOW(lm, reg) = val;
done:
	pm_runtime_mark_last_busy(&lm->i2c->dev);
	pm_runtime_put_autosuspend(&lm->i2c->dev);
	return rc;
}

static int lm3533_read(struct lm3533_data *lm, u8 *val, u8 reg)
{
	int rc = pm_runtime_get_sync(&lm->i2c->dev);
	if (rc < 0) {
		pm_runtime_put_noidle(&lm->i2c->dev);
		return rc;
	}
	rc = i2c_smbus_read_byte_data(lm->i2c, reg);
	if (rc < 0) {
		rc = lm3533_recover(lm);
		rc = rc ? rc : i2c_smbus_read_byte_data(lm->i2c, reg);
	}
	if (rc < 0) {
		dev_err(&lm->i2c->dev, "%s: failed (%d)\n", __func__, rc);
		goto done;
	}
	SHADOW(lm, reg) = rc;
	*val = rc;
	rc = 0;
	dev_vdbg(&lm->i2c->dev, "%s: 0x%02x=0x%02x\n", __func__, reg, rc);
done:
	pm_runtime_mark_last_busy(&lm->i2c->dev);
	pm_runtime_put_autosuspend(&lm->i2c->dev);
	return rc;
}

static int lm3533_setup_banks(struct lm3533_data *lm, u8 enabled)
{
	struct lm3533_bank_config const *bank;
	enum lm3533_control_bank b;
	struct lm3533_bank *cb;
	int rc;
	u8 hv_banks_ctl = SHADOW(lm, REG_HVBCTL);
	u8 ctl;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		cb = &lm->banks[b];
		bank = &lm->pdata->b_cnf[b];

		switch (b) {
		case LM3533_CBNKA:
			cb->ctrl_reg = REG_HVBCTL;
			cb->als_ctrl = bank->ctl & LM3533_HVA_ALS1;
			cb->als_mask = LM3533_HVA_ALS1;
			hv_banks_ctl |= bank->ctl & ~cb->als_mask;
			break;
		case LM3533_CBNKB:
			cb->ctrl_reg = REG_HVBCTL;
			cb->als_ctrl = bank->ctl & LM3533_HVB_ALS2;
			cb->als_mask = LM3533_HVB_ALS2;
			hv_banks_ctl |= bank->ctl & ~cb->als_mask;
			break;
		case LM3533_CBNKC:
		case LM3533_CBNKD:
		case LM3533_CBNKE:
		case LM3533_CBNKF:
		default:
			cb->ctrl_reg = REG_LVBCTL_BASE + b - LM3533_CBNKC;
			cb->als_mask = LM3533_LV_ALS2 | LM3533_LV_ALS3;
			cb->als_ctrl = bank->ctl & cb->als_mask;
			ctl = bank->ctl & ~cb->als_mask;
			rc = lm3533_write(lm, ctl, cb->ctrl_reg);
			if (rc)
				goto err_exit;

			break;
		}
		if (!((1 << b) & enabled))
			continue;

		rc = lm3533_write(lm, bank->fsc, REG_FSC_BASE + b);
		if (rc)
			goto err_exit;
		rc = lm3533_write(lm, bank->pwm, REG_PWM_BASE + b);
		if (rc)
			goto err_exit;
	}

	rc = lm3533_write(lm, hv_banks_ctl, REG_HVBCTL);
err_exit:
	return rc;
}

static int lm3533_setup_leds(struct lm3533_data *lm)
{
	enum lm3533_led i;
	enum lm3533_control_bank bank;
	struct lm3533_led_config const *leds = lm->pdata->l_cnf;
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
	struct lm3533_platform_data const *p = lm->pdata;
	int rc;

	rc = lm3533_write(lm, p->ovp_boost_pwm, REG_OVP_BOOST_PWM);
	rc = rc ? rc : lm3533_write(lm, p->cpump_cnf, REG_CPUMP);
	rc = rc ? rc : lm3533_write(lm, p->als_input_current, REG_ALS_CURRENT);
	rc = rc ? rc : lm3533_write(lm, p->als_algo, REG_ALS_ALGO);
	rc = rc ? rc : lm3533_write(lm, p->als_down_delay, REG_ALS_DDELAY);
	rc = rc ? rc : lm3533_write(lm, p->startup_rr, REG_START_UP_RR);
	rc = rc ? rc : lm3533_write(lm, p->runtime_rr, REG_RUN_TIME_RR);
	rc = rc ? rc : lm3533_write(lm, p->led_fault, REG_LED_FAULT);
	rc = rc ? rc : lm3533_setup_leds(lm);
	return rc;
}

static int lm3533_als_enable_update_locked(struct lm3533_data *lm)
{
	int rc = 0;
	u8 ctrl = lm->pdata->als_control;
	bool need_als;

	need_als = lm->als_users || lm->enabled_als;
	if (need_als && !lm->als_active) {
		dev_dbg(&lm->i2c->dev, "%s: enabling\n", __func__);
		ctrl |= LM3533_ALS_ENABLE;
		rc = pm_runtime_get_sync(&lm->i2c->dev);
		if (rc < 0) {
			pm_runtime_put_noidle(&lm->i2c->dev);
			goto bypass;
		}
	} else if (!need_als && lm->als_active) {
		dev_dbg(&lm->i2c->dev, "%s: disabling\n", __func__);
		ctrl &= ~LM3533_ALS_ENABLE;
		pm_runtime_mark_last_busy(&lm->i2c->dev);
		pm_runtime_put_autosuspend(&lm->i2c->dev);
	} else
		goto bypass;
	rc = lm3533_write(lm, ctrl, REG_ALS_AVGT);
	if (!rc)
		lm->als_active = need_als;
bypass:
	return rc;
}

static inline struct lm3533_intf *ldev_to_intf(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct lm3533_intf, ldev);
}

static int lm3533_als_enable(struct led_classdev *led_cdev, bool enable)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	struct lm3533_bank *cb;
	u8 hv_banks_ctl = SHADOW(lm, REG_HVBCTL);
	u8 reg;
	int rc;

	if (intf->banks & (1 << LM3533_CBNKA)) {
		hv_banks_ctl &= ~lm->banks[LM3533_CBNKA].als_mask;
		if (enable)
			hv_banks_ctl |= lm->banks[LM3533_CBNKA].als_ctrl;
		dev_dbg(dev, "%s: als for bank %d, enable=%d\n", __func__,
				LM3533_CBNKA, enable);
	}
	if (intf->banks & (1 << LM3533_CBNKB)) {
		hv_banks_ctl &= ~lm->banks[LM3533_CBNKB].als_mask;
		if (enable)
			hv_banks_ctl |= lm->banks[LM3533_CBNKB].als_ctrl;
		dev_dbg(dev, "%s: als for bank %d, enable=%d\n", __func__,
				LM3533_CBNKB, enable);
	}
	if (hv_banks_ctl != SHADOW(lm, REG_HVBCTL)) {
		rc = lm3533_write(lm, hv_banks_ctl, REG_HVBCTL);
		if (rc)
			goto err;
	}

	for (b = LM3533_CBNKC; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		cb = &lm->banks[b];
		reg = lm->pdata->b_cnf[b].ctl & ~cb->als_mask;
		if (enable)
			reg |= cb->als_ctrl;
		rc = lm3533_write(lm, reg, cb->ctrl_reg);
		if (rc)
			break;
		dev_dbg(dev, "%s: als for bank %d, enable=%d\n", __func__,
				b, enable);
	}
	mutex_lock(&lm->lock);
	if (enable)
		lm->enabled_als |= intf->banks;
	else
		lm->enabled_als &= ~intf->banks;
	rc = lm3533_als_enable_update_locked(lm);
	mutex_unlock(&lm->lock);
err:
	return rc;
}

static int lm3533_apply_pattern(struct led_classdev *led_cdev)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	int rc = 0;
	u8 up, dn, on, off, ctl;

	ctl = SHADOW(lm, REG_PTRN_ENABLE);

	dev_dbg(dev, "%s: requested: on %d, off %d, fade-in %d, fade-out %d\n",
		__func__, intf->on_time, intf->off_time, intf->dim_up,
		intf->dim_dn);

	if (!((intf->on_time || intf->dim_up) &&
			(intf->off_time || intf->dim_dn))) {
		for (b = LM3533_CBNKC; b < LM3533_BANK_NUM; b++) {
			if (!(intf->banks & (1 << b)))
				continue;
			ctl &= ~(1 << ((b - LM3533_CBNKC) * 2));
			dev_dbg(dev, "%s: off on bank %d\n", __func__, b);
		}
		goto apply_ctl;
	}

	up = dim_t2reg(&intf->dim_up);
	dn = dim_t2reg(&intf->dim_dn);
	on = thigh2reg(&intf->on_time);
	off = tlow2reg(&intf->off_time);

	dev_dbg(dev, "%s: applied: on %d, off %d, fade-in %d, fade-out %d\n",
		__func__, intf->on_time, intf->off_time, intf->dim_up,
		intf->dim_dn);

	for (b = LM3533_CBNKC; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		rc = lm3533_write(lm, off, PTRN_TLOW_REG(b));
		if (rc)
			break;
		rc = lm3533_write(lm, on, PTRN_THIGH_REG(b));
		if (rc)
			break;
		rc = lm3533_write(lm, up, PTRN_T_UP_REG(b));
		if (rc)
			break;
		rc = lm3533_write(lm, dn, PTRN_T_DN_REG(b));
		if (rc)
			break;
		ctl |= 1 << ((b - LM3533_CBNKC) * 2);
		dev_dbg(dev, "%s: applied on bank %d\n", __func__, b);
	}
apply_ctl:
	if (ctl != SHADOW(lm, REG_PTRN_ENABLE)) {
		rc = lm3533_write(lm, 0, REG_PTRN_ENABLE);
		if (!rc)
			rc = lm3533_write(lm, ctl, REG_PTRN_ENABLE);
	}
	return rc;
}

static int lm3533_scaler_enable(struct led_classdev *led_cdev, bool enable)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	int rc = 0;
	u8 ctl = SHADOW(lm, REG_PTRN_ENABLE);

	for (b = LM3533_CBNKC; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		if (enable) {
			ctl |=  1 << ((b - LM3533_CBNKC) * 2 + 1);
			dev_dbg(dev, "%s: enabled on bank %d\n", __func__, b);
		} else {
			ctl &= ~(1 << ((b - LM3533_CBNKC) * 2 + 1));
			dev_dbg(dev, "%s: disabled on bank %d\n", __func__, b);
		}
	}
	if (ctl != SHADOW(lm, REG_PTRN_ENABLE))
		rc = lm3533_write(lm, ctl, REG_PTRN_ENABLE);
	return rc;
}

static void lm3533_led_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	int rc;
	u8 bena;

	if (value <= LED_OFF)
		value = 0;
	else if (value >= LED_FULL)
		value = 255;

	dev_dbg(dev, "%s: brightness %d -> %d, als %d\n", led_cdev->name,
			intf->brightness, value, intf->als);

	if (value && !intf->brightness) {
		rc = pm_runtime_get_sync(dev);
		if (rc < 0) {
			pm_runtime_put_noidle(dev);
			return;
		}
	}
	mutex_lock(&lm->lock);
	bena = SHADOW(lm, REG_BANK_ENA);
	bena = value ? bena | intf->banks : bena & (~intf->banks);
	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		if (lm3533_write(lm, value, REG_BR_BASE + b))
			break;
	}
	if (bena != SHADOW(lm, REG_BANK_ENA))
		(void)lm3533_write(lm, bena, REG_BANK_ENA);
	mutex_unlock(&lm->lock);
	if (intf->als) {
		if (!value && intf->brightness)
			lm3533_als_enable(led_cdev, false);
		else if (value && !intf->brightness)
			lm3533_als_enable(led_cdev, true);
	}
	if (!value && intf->brightness) {
		pm_runtime_mark_last_busy(&lm->i2c->dev);
		pm_runtime_put_autosuspend(&lm->i2c->dev);
	}
	intf->brightness = value;
}

static int lm3533_blink_set(struct led_classdev *led_cdev,
				unsigned long *delay_on,
				unsigned long *delay_off)
{
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	int rc;

	intf->on_time = *delay_on;
	intf->off_time = *delay_off;
	intf->dim_up = intf->dim_dn = 0;
	rc = lm3533_apply_pattern(led_cdev);
	*delay_on = intf->on_time;
	*delay_off = intf->off_time;
	return rc;
}

static ssize_t lm3533_als_enable_set(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	unsigned long enable;
	int rc;

	if (!kstrtoul(buf, 10, &enable) && (enable < 2)) {
		intf->als = enable;
		if (intf->brightness)
			lm3533_als_enable(led_cdev, enable);
		rc = 0;
	} else {
		rc = -EINVAL;
	}
	return rc ? rc : size;
}

static ssize_t lm3533_als_suspend_enable_set(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	unsigned long enable;

	if (kstrtoul(buf, 10, &enable) || enable > 1)
		return -EINVAL;
	intf->als_on_suspend = enable;
	return size;
}

static ssize_t lm3533_pattern_set(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	unsigned long on, off, in, out;
	int rc;

	if (4 == sscanf(buf, "%8lu,%8lu,%8lu,%8lu", &on, &off, &in, &out)) {
		intf->dim_dn = out;
		intf->dim_up = in;
		intf->on_time = on;
		intf->off_time = off;
		rc = lm3533_apply_pattern(led_cdev);
	} else {
		rc = -EINVAL;
	}
	return rc ? rc : size;
}

static ssize_t lm3533_scaler_enable_set(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	unsigned long enable;
	int rc;

	if (!kstrtoul(buf, 10, &enable) && (enable < 2))
		rc = lm3533_scaler_enable(led_cdev, enable);
	else
		rc = -EINVAL;
	return rc ? rc : size;
}

static int lm3533_set_fsc(struct led_classdev *led_cdev, u8 fsc)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	int rc = 0;
	u8 ctl;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		ctl = SHADOW(lm, REG_FSC_BASE + b);
		if (ctl != fsc) {
			rc = lm3533_write(lm, fsc, REG_FSC_BASE + b);
			if (rc)
				goto err_exit;
		}
	}
err_exit:
	return rc;
}

#define MAX_FULL_SCALE 31

static ssize_t lm3533_fsc_set(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	unsigned long fsc;
	int rc;

	if (!kstrtoul(buf, 10, &fsc) && (fsc <= MAX_FULL_SCALE))
		rc = lm3533_set_fsc(led_cdev, fsc);
	else
		rc = -EINVAL;
	return rc ? rc : size;
}

static ssize_t lm3533_fsc_show(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	u8 fsc = 0;
	int rc = 0;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		rc = lm3533_read(lm, &fsc, REG_FSC_BASE + b);
		break;
	}
	return rc ? rc : scnprintf(buf, PAGE_SIZE, "%u\n", fsc);
}

static int lm3533_set_pwm(struct led_classdev *led_cdev, u8 pwm)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	int rc = 0;
	u8 ctl;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		ctl = SHADOW(lm, REG_PWM_BASE + b);
		if (ctl != pwm) {
			rc = lm3533_write(lm, pwm, REG_PWM_BASE + b);
			if (rc)
				goto err_exit;
		}
	}
err_exit:
	return rc;
}

static ssize_t lm3533_pwm_set(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	unsigned long pwm;
	int rc;

	if (!kstrtoul(buf, 10, &pwm))
		rc = lm3533_set_pwm(led_cdev, (pwm & LM3533_PWM_ALL));
	else
		rc = -EINVAL;
	return rc ? rc : size;
}

static ssize_t lm3533_pwm_show(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct device *dev = led_cdev->dev->parent;
	struct lm3533_data *lm = dev_get_drvdata(dev);
	struct lm3533_intf *intf = ldev_to_intf(led_cdev);
	enum lm3533_control_bank b;
	u8 pwm = 0;
	int rc = 0;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		rc = lm3533_read(lm, &pwm, REG_PWM_BASE + b);
		break;
	}
	return rc ? rc : scnprintf(buf, PAGE_SIZE, "%u\n", pwm);
}

static struct device_attribute led_als_attr[] = {
	__ATTR(als_enable, 0200, NULL, lm3533_als_enable_set),
	__ATTR(als_on_suspend_enable, 0200, NULL,
			lm3533_als_suspend_enable_set),
	__ATTR_NULL,
};

static struct device_attribute led_pattern_attr[] = {
	__ATTR(pattern, 0200, NULL, lm3533_pattern_set),
	__ATTR(enable_scaler, 0200, NULL, lm3533_scaler_enable_set),
	__ATTR_NULL,
};

static struct device_attribute led_ctl_attr[] = {
	__ATTR(fsc, 0600, lm3533_fsc_show, lm3533_fsc_set),
	__ATTR(pwm, 0600, lm3533_pwm_show, lm3533_pwm_set),
	__ATTR_NULL,
};

static int intf_add_attributes(struct lm3533_intf *intf,
		struct device_attribute *attrs)
{
	int error = 0;
	int i;
	struct device *dev = intf->ldev.dev;

	for (i = 0; attrs[i].attr.name; i++) {
		error = device_create_file(dev, &attrs[i]);
		if (error) {
			dev_err(dev, "%s: failed.\n", __func__);
			goto err;
		}
	}
	return 0;
err:
	while (--i >= 0)
		device_remove_file(dev, &attrs[i]);
	return error;
}

static void intf_remove_attributes(struct lm3533_intf *intf,
		struct device_attribute *attrs)
{
	int i;
	struct device *dev = intf->ldev.dev;

	for (i = 0; attr_name(attrs[i]); i++)
		device_remove_file(dev, &attrs[i]);
}

static int lm3533_add_intf(struct lm3533_data *lm, const char *nm,
			enum lm3533_control_bank bank)
{
	unsigned i;
	struct device *dev = &lm->i2c->dev;
	struct lm3533_intf *intf = NULL;
	int rc = 0;

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
			intf->ldev.blink_set = lm3533_blink_set;
			lm->intf[i] = intf;
			dev_info(dev, "New led '%s'\n", nm);

		} else if (!strncmp(nm, lm->intf[i]->ldev.name, strlen(nm))) {
			intf = lm->intf[i];
		}
	}
	if (!intf) {
		dev_err(dev, "%s: no feee space for interfaces\n", __func__);
		rc = -ENOMEM;
		goto exit;
	}
	intf->banks |= 1 << bank;
exit:
	return rc;
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

static bool is_pattern_supported(struct lm3533_intf *intf)
{
	return !(intf->banks & (1 << LM3533_CBNKA | 1 << LM3533_CBNKB));
}

static int lm3533_register_ldevs(struct lm3533_data *lm)
{
	int i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++) {
		rc = led_classdev_register(&lm->i2c->dev, &lm->intf[i]->ldev);
		if (rc)
			goto roll_back;
		rc = intf_add_attributes(lm->intf[i], led_als_attr);
		if (rc)
			goto roll_back_als;
		rc = intf_add_attributes(lm->intf[i], led_ctl_attr);
		if (rc)
			goto roll_back_ctl;
		if (!is_pattern_supported(lm->intf[i])) {
			lm->intf[i]->ldev.blink_set = NULL;
			continue;
		}
		rc = intf_add_attributes(lm->intf[i], led_pattern_attr);
		if (rc)
			goto roll_back_pattern;
	}
	return rc;
roll_back_pattern:
	intf_remove_attributes(lm->intf[i], led_ctl_attr);
roll_back_ctl:
	intf_remove_attributes(lm->intf[i], led_als_attr);
roll_back_als:
	i++;
roll_back:
	while (--i >= 0)
		led_classdev_unregister(&lm->intf[i]->ldev);
	dev_err(&lm->i2c->dev, "%s: failed, rc = %d.\n", __func__, rc);
	return rc;
}

static void lm3533_unregister_ldevs(struct lm3533_data *lm)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++) {
		intf_remove_attributes(lm->intf[i], led_als_attr);
		intf_remove_attributes(lm->intf[i], led_ctl_attr);
		if (is_pattern_supported(lm->intf[i]))
			intf_remove_attributes(lm->intf[i], led_pattern_attr);
		led_classdev_unregister(&lm->intf[i]->ldev);
	}
}

static int lm3533_als_tgt_update(struct lm3533_data *lm, enum lm3533_als_id id,
		unsigned *targets)
{
	unsigned i;
	int rc;

	for (i = 0; i < LM3533_ALS_ZONE_NUM; i++) {
		rc = lm3533_write(lm, targets[i],
			REG_ALS_TGT_BASE + (id * LM3533_ALS_ZONE_NUM) + i);
		if (rc)
			break;
	}
	return rc;
}


static int lm3533_ptrn_scaler_update(struct lm3533_data *lm, unsigned *sclrs)
{
	static const struct {
		u8 offset;
		u8 shift;
	} scaler_regs[LM3533_SCALERS_NUM] = {
		[0] = {LM3533_SC_Z1_OFFS, LM3533_SC_Z1_SHIFT},
		[1] = {LM3533_SC_Z2_OFFS, LM3533_SC_Z2_SHIFT},
		[2] = {LM3533_SC_Z3_OFFS, LM3533_SC_Z3_SHIFT},
		[3] = {LM3533_SC_Z4_OFFS, LM3533_SC_Z4_SHIFT},
		[4] = {LM3533_SC_Z5_OFFS, LM3533_SC_Z5_SHIFT},
	};

	unsigned i;
	int rc;
	u8 val;

	for (i = 0; i < LM3533_SCALERS_NUM; i++) {
		val = (sclrs[i] - 1) << scaler_regs[i].shift;
		rc = lm3533_write(lm, val,
				REG_SCALER_BASE + scaler_regs[i].offset);
		if (rc)
			break;
	}
	return rc;
}

static ssize_t lm3533_als_tgt_set(struct device *dev, enum lm3533_als_id id,
			const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev->parent);
	unsigned t[LM3533_ALS_ZONE_NUM];
	int rc;
	int n = sscanf(buf, "%5u,%5u,%5u,%5u,%5u",
			&t[0], &t[1], &t[2], &t[3], &t[4]);

	if (n != LM3533_ALS_ZONE_NUM)
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
	struct lm3533_data *lm = dev_get_drvdata(dev);
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

static ssize_t lm3533_als_current_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned long als_current;
	int rc;

	rc = kstrtoul(buf, 10, &als_current);
	if (rc || als_current > LM3533_MAX_ALS_CURRENT)
		return -EINVAL;
	als_current /= LM3533_ALS_CURRENT_PER_BIT;
	rc = lm3533_write(lm, als_current, REG_ALS_R_SEL);
	return rc ? rc : count;
}

static ssize_t lm3533_als_current_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u uA\n",
			SHADOW(lm, REG_ALS_R_SEL) * LM3533_ALS_CURRENT_PER_BIT);
}

static ssize_t lm3533_als_on_suspend_current_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned long als_current;

	if (kstrtoul(buf, 10, &als_current) ||
			als_current > LM3533_MAX_ALS_CURRENT)
		return -EINVAL;
	lm->als_reg_curr_on_suspend = als_current / LM3533_ALS_CURRENT_PER_BIT;
	return count;
}

static ssize_t lm3533_als_on_suspend_current_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%u uA\n",
			lm->als_reg_curr_on_suspend *
			LM3533_ALS_CURRENT_PER_BIT);
}

static ssize_t lm3533_als_avg_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	u8 adc = 0;
	int rc = lm3533_read(lm, &adc, REG_ALS_ADC_AVG);
	return rc ? rc : scnprintf(buf, PAGE_SIZE, "%u\n", adc);
}

static ssize_t lm3533_als_adc_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "%d\n", lm->als_users);
}

static ssize_t lm3533_als_adc_enable_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned long enable;
	int rc;

	rc = kstrtoul(buf, 10, &enable);
	if (rc || enable > 1)
		return -EINVAL;
	mutex_lock(&lm->lock);
	if (enable)
		lm->als_users++;
	else {
		if (!lm->als_users) {
			dev_err(&lm->i2c->dev, "%s: unbalanced\n", __func__);
			rc = -EINVAL;
			goto exit;
		}
		lm->als_users--;
	}
	dev_dbg(&lm->i2c->dev, "%s: %d als users\n", __func__, lm->als_users);
	rc = lm3533_als_enable_update_locked(lm);
exit:
	mutex_unlock(&lm->lock);
	return rc ? rc : count;
}

static ssize_t lm3533_als_zones_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	return scnprintf(buf, PAGE_SIZE, "z[i].hight, z[i].low:"
		" %u,%u,%u,%u,%u,%u,%u,%u\n",
		SHADOW(lm, REG_ALS_ZONES_BASE),
		SHADOW(lm, REG_ALS_ZONES_BASE + 1),
		SHADOW(lm, REG_ALS_ZONES_BASE + 2),
		SHADOW(lm, REG_ALS_ZONES_BASE + 3),
		SHADOW(lm, REG_ALS_ZONES_BASE + 4),
		SHADOW(lm, REG_ALS_ZONES_BASE + 5),
		SHADOW(lm, REG_ALS_ZONES_BASE + 6),
		SHADOW(lm, REG_ALS_ZONES_BASE + 7));
}

static ssize_t lm3533_als_zones_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned t[(LM3533_ALS_ZONE_NUM - 1) * 2];
	int rc;
	int i;
	int n = sscanf(buf, "%5u,%5u,%5u,%5u,%5u,%5u,%5u,%5u",
			&t[0], &t[1], &t[2], &t[3], &t[4], &t[5], &t[6], &t[7]);

	if (n != ARRAY_SIZE(t))
		return -EINVAL;
	for (i = 0; i < ARRAY_SIZE(t); i++)
		if (t[i] > 255)
			return -EINVAL;
	for (i = rc = 0; i < ARRAY_SIZE(t) && !rc; i++)
		rc = lm3533_write(lm, t[i], REG_ALS_ZONES_BASE + i);

	dev_dbg(dev, "%s: %u,%u,%u,%u,%u,%u,%u,%u\n", __func__,
			t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7]);
	return rc ? rc : count;
}

static unsigned lm3533_rtime2reg(unsigned up, unsigned dn)
{
	unsigned x, i;
	for (i = MAX_RR_REG, x = MAX_RR_MS; x > up && i; i--)
		x >>= 1;
	up = i;
	for (i = MAX_RR_REG, x = MAX_RR_MS; x > dn && i; i--)
		x >>= 1;
	dn = i;
	return (up << 3) | dn;
}

static ssize_t lm3533_rr_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned up, dn;
	int rc;
	int n = sscanf(buf, "%5u,%5u", &up, &dn);

	if (n != 2 || up > MAX_RR_MS || dn > MAX_RR_MS)
		return -EINVAL;
	up = lm3533_rtime2reg(up, dn);
	dev_dbg(dev, "rr_up: %d, rr_down %d\n",
			MAX_RR_MS >> (MAX_RR_REG - (up >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (up & 7)));
	rc = lm3533_write(lm, up, REG_RT_TRANS_T);
	return rc ? rc : count;
}

static ssize_t lm3533_rr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned reg = SHADOW(lm, REG_RT_TRANS_T);
	return scnprintf(buf, PAGE_SIZE, "%u,%u # min 0 ms, max %d ms",
			MAX_RR_MS >> (MAX_RR_REG - (reg >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (reg & 7)), MAX_RR_MS);
}

static ssize_t lm3533_start_shdn_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned up, dn;
	int rc;
	int n = sscanf(buf, "%5u,%5u", &up, &dn);

	if (n != 2 || up > MAX_RR_MS || dn > MAX_RR_MS)
		return -EINVAL;
	up = lm3533_rtime2reg(up, dn);
	dev_dbg(dev, "ss_up: %d, ss_down %d\n",
			MAX_RR_MS >> (MAX_RR_REG - (up >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (up & 7)));
	rc = lm3533_write(lm, up, REG_START_SHDN_T);
	return rc ? rc : count;
}

static ssize_t lm3533_start_shdn_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned reg = SHADOW(lm, REG_START_SHDN_T);
	return scnprintf(buf, PAGE_SIZE, "%u,%u # min 0 ms, max %d ms",
			MAX_RR_MS >> (MAX_RR_REG - (reg >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (reg & 7)), MAX_RR_MS);
}

static ssize_t lm3533_lvbnak_sync_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	unsigned long sync;
	u8 b_ena = SHADOW(lm, REG_BANK_ENA);
	int rc = kstrtoul(buf, 10, &sync);

	dev_dbg(dev, "%s\n", __func__);
	if (rc || sync != 1)
		return -EINVAL;
	rc = lm3533_write(lm, b_ena & (~LM3533_LV_BANKS), REG_BANK_ENA);
	if (rc)
		goto err;
	usleep_range(1000, 2000);
	rc = lm3533_write(lm, b_ena, REG_BANK_ENA);
err:
	return rc ? rc : count;
}

static ALS_ATTR(1, curve, S_IWUSR, NULL, lm3533_als1_store);
static ALS_ATTR(2, curve, S_IWUSR, NULL, lm3533_als2_store);
static ALS_ATTR(3, curve, S_IWUSR, NULL, lm3533_als3_store);

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
	enum lm3533_control_bank b,
	enum lm3533_als_id als, bool enable)
{
	int rc;
	int idx = lm3533_to_intf(lm, b);
	struct kobject *kobj;

	if (idx < 0)
		return idx;

	kobj = &lm->intf[idx]->ldev.dev->kobj;

	if (!enable) {
		sysfs_remove_file(kobj, lm3533_als_attr[als]);
		return 0;
	}
	if (als <  LM3533_ALS_NUM) {
		dev_dbg(&lm->i2c->dev, "Linking '%s' to als%d\n",
				lm->intf[idx]->ldev.name, als + 1);
		rc = sysfs_create_file(kobj, lm3533_als_attr[als]);
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
	struct lm3533_bank_config const *bank;
	enum lm3533_control_bank b;
	u8 tmp;

	for (b = LM3533_CBNKA; b < LM3533_BANK_NUM; b++) {
		enum lm3533_als_id link_als = LM3533_ALS_NUM;

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
			break;
		}
		if (link_als == LM3533_ALS_NUM)
			continue;
		rc = lm3533_link_controls(lm, b, link_als, enable);
		if (rc)
			goto err_exit;
	}
err_exit:
	return rc;
}

static struct lm3533_intf *lm3533_intf_get(struct lm3533_data *lm,
		char const *name)
{
	unsigned i;
	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++)
		if (!strncmp(lm->intf[i]->ldev.name, name, strlen(name)))
			return lm->intf[i];
	return NULL;
}

static void lm3533_set_startup_br(struct lm3533_data *lm)
{
	struct lm3533_intf *intf;
	struct lm3533_startup_brightenss const *p =
			lm->pdata->startup_brightness;
	while (p && p->intf_name) {
		intf = lm3533_intf_get(lm, p->intf_name);
		if (intf) {
			dev_info(&lm->i2c->dev, "%s: %d on startup\n",
				p->intf_name, p->brightness);
			lm3533_led_brightness(&intf->ldev, p->brightness);
		}
		p++;
	}
}

static ssize_t lm3533_reset_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	unsigned long reset;
	int rc;
	struct lm3533_data *lm = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &reset) && reset == 1 ?
		lm3533_recover(lm) : -EINVAL;
	return rc ? rc : count;
}

static const DEVICE_ATTR(reset, S_IWUSR, NULL, lm3533_reset_store);
static const DEVICE_ATTR(als_current, S_IWUSR | S_IRUSR,
		lm3533_als_current_show, lm3533_als_current_store);
static const DEVICE_ATTR(als_suspend_current, S_IWUSR | S_IRUSR,
		lm3533_als_on_suspend_current_show,
		lm3533_als_on_suspend_current_store);
static const DEVICE_ATTR(als_enable, S_IWUSR | S_IRUSR,
		lm3533_als_adc_enable_show, lm3533_als_adc_enable_store);
static const DEVICE_ATTR(als_zones, S_IWUSR | S_IRUSR,
		lm3533_als_zones_show, lm3533_als_zones_store);
static const DEVICE_ATTR(als_result, S_IRUSR, lm3533_als_avg_show, NULL);
static const DEVICE_ATTR(rt_rate_ms, S_IWUSR | S_IRUSR,
		lm3533_rr_show, lm3533_rr_store);
static const DEVICE_ATTR(start_shdn_ms, S_IWUSR | S_IRUSR,
		lm3533_start_shdn_show, lm3533_start_shdn_store);
static const DEVICE_ATTR(scalers, S_IWUSR, NULL, lm3533_ptrn_scaler_store);
static const DEVICE_ATTR(als1_curve, S_IWUSR, NULL, lm3533_als1_store);
static const DEVICE_ATTR(als2_curve, S_IWUSR, NULL, lm3533_als2_store);
static const DEVICE_ATTR(als3_curve, S_IWUSR, NULL, lm3533_als3_store);
static const DEVICE_ATTR(sync_lvbanks, S_IWUSR, NULL, lm3533_lvbnak_sync_store);

static const struct attribute *lm3533_attrs[] = {
	&dev_attr_reset.attr,
	&dev_attr_als_current.attr,
	&dev_attr_als_suspend_current.attr,
	&dev_attr_als_enable.attr,
	&dev_attr_als_zones.attr,
	&dev_attr_als1_curve.attr,
	&dev_attr_als2_curve.attr,
	&dev_attr_als3_curve.attr,
	&dev_attr_als_result.attr,
	&dev_attr_scalers.attr,
	&dev_attr_rt_rate_ms.attr,
	&dev_attr_start_shdn_ms.attr,
	&dev_attr_sync_lvbanks.attr,
	NULL,
};

static int __devinit lm3533_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lm3533_platform_data const *pdata = client->dev.platform_data;
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
	mutex_init(&lm->lock);
	i2c_set_clientdata(client, lm);

	if (lm->pdata->power_on) {
		rc = lm->pdata->power_on(dev);
		if (rc) {
			dev_err(dev, "%s: failed to power on.\n", __func__);
			goto err_power_on;
		}
		usleep_range(LM3533_PWR_DELAY_US, LM3533_PWR_DELAY_US + 1000);
	}
	if (lm->pdata->als_on)
		(void)lm->pdata->als_on(dev);

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_set_autosuspend_delay(dev, autosuspend_delay_ms);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_mark_last_busy(dev);

	rc = lm3533_sync_shadow(lm);
	if (rc)
		goto err_configure;

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

	lm3533_set_startup_br(lm);
	lm->recovery_mode = true;
	rc = sysfs_create_files(&dev->kobj, lm3533_attrs);
	if (rc) {
		dev_err(dev, "%s: failed to create files.\n", __func__);
		goto err_file_create;
	}
	dev_info(dev, "%s: completed.\n", __func__);
	return 0;

err_file_create:
	lm3533_link_als(lm, false);
err_link_als:
	lm3533_unregister_ldevs(lm);
err_reg_ldevs:
err_interfaces:
	lm3533_cleanup(lm);
err_configure:
	pm_runtime_disable(&client->dev);
	if (lm->pdata->als_off)
		(void)lm->pdata->als_off(dev);
err_power_on:
	mutex_destroy(&lm->lock);
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

	sysfs_remove_files(&client->dev.kobj, lm3533_attrs);
	if (!pm_runtime_suspended(&client->dev))
		lm->pdata->power_off(&client->dev);
	if (lm->pdata->als_off)
		(void)lm->pdata->als_off(&client->dev);
	pm_runtime_disable(&client->dev);
	lm3533_cleanup(lm);
	if (lm->pdata->teardown)
		lm->pdata->teardown(&client->dev);
	mutex_destroy(&lm->lock);
	kfree(lm);
	return 0;
}

static void lm3533_shutdown(struct i2c_client *client)
{
	struct lm3533_data *lm = i2c_get_clientdata(client);
	unsigned i;

	if (lm->als_users && !lm->enabled_als) {
		u8 ctrl = lm->pdata->als_control & ~LM3533_ALS_ENABLE;
		(void)lm3533_write(lm, ctrl, REG_ALS_AVGT);
	}

	for (i = 0; i < LM3533_BANK_NUM && lm->intf[i]; i++) {
		if (lm->intf[i]->ldev.brightness > LED_OFF)
			lm3533_led_brightness(&lm->intf[i]->ldev, 0);
	}
	(void)lm3533_link_als(lm, false);
	lm3533_unregister_ldevs(lm);
}

#ifdef CONFIG_PM_RUNTIME
static int lm3533_runtime_suspend(struct device *dev)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	int rc = lm3533_power_off(lm);
	dev_dbg(dev, "%s\n", __func__);
	return rc ? -EAGAIN : 0;
}

static int lm3533_runtime_resume(struct device *dev)
{
	struct lm3533_data *lm = dev_get_drvdata(dev);
	int rc = lm3533_power_on(lm);
	dev_dbg(dev, "%s\n", __func__);
	return rc ? -EAGAIN : 0;
}
#endif

#ifdef CONFIG_PM_SLEEP
static int lm3533_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm3533_data *lm = i2c_get_clientdata(client);
	int rc = 0;
	size_t i;
	bool set_current = false;

	dev_dbg(dev, "%s\n", __func__);

	if (pm_runtime_suspended(dev)) {
		dev_dbg(dev, "%s: runtime-suspended.\n", __func__);
		goto exit_suspend;
	}
	if (mutex_is_locked(&lm->lock))
		return -EBUSY;
	for (i = 0; i < ARRAY_SIZE(lm->intf); i++) {
		struct lm3533_intf *intf = lm->intf[i];
		if (!intf)
			break;
		if (!intf->brightness)
			continue;
		lm->lit_on_suspend = true;
		if (!intf->als && intf->als_on_suspend) {
			set_current = true;
			dev_dbg(dev, "enabling suspend ALS for %s\n",
				intf->ldev.name);
			lm3533_als_enable(&intf->ldev, true);
			lm3533_scaler_enable(&intf->ldev, true);
		}
	}
	if (set_current && lm->als_reg_curr_on_suspend !=
			SHADOW(lm, REG_ALS_CURRENT)) {
		lm->als_reg_curr_saved = SHADOW(lm, REG_ALS_CURRENT);
		rc = lm3533_write(lm, lm->als_reg_curr_on_suspend,
				REG_ALS_R_SEL);
		if (rc)
			goto exit_suspend;
	}
	if (lm->als_users && !lm->enabled_als) {
		u8 ctrl = lm->pdata->als_control & ~LM3533_ALS_ENABLE;
		dev_dbg(dev, "%s: disabling ALS\n", __func__);
		rc = lm3533_write(lm, ctrl, REG_ALS_AVGT);
	}
	if (!(rc || lm->lit_on_suspend))
		rc = lm3533_power_off(lm);
exit_suspend:
	if (!rc && lm->pdata->als_off)
		(void)lm->pdata->als_off(dev);
	return rc ? -EAGAIN : 0;
}

static int lm3533_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm3533_data *lm = i2c_get_clientdata(client);
	int rc = 0;
	size_t i;

	dev_dbg(dev, "%s\n", __func__);
	if (lm->pdata->als_on)
		(void)lm->pdata->als_on(dev);

	if (pm_runtime_suspended(dev)) {
		dev_dbg(dev, "%s: was runtime-suspended.\n", __func__);
		return 0;
	}
	if (lm->lit_on_suspend) {
		if (lm->als_reg_curr_saved != SHADOW(lm, REG_ALS_CURRENT)) {
			rc = lm3533_write(lm, lm->als_reg_curr_saved,
					REG_ALS_R_SEL);
			if (rc)
				goto exit_resume;
		}
		for (i = 0; i < ARRAY_SIZE(lm->intf); i++) {
			struct lm3533_intf *intf = lm->intf[i];
			if (!intf)
				break;
			if (!intf->brightness)
				continue;
			if (!intf->als && intf->als_on_suspend) {
				dev_dbg(dev, "disbling suspend ALS for %s\n",
						intf->ldev.name);
				lm3533_als_enable(&intf->ldev, false);
				lm3533_scaler_enable(&intf->ldev, false);
			}
		}
	} else {
		rc = lm3533_power_on(lm);
		if (rc)
			goto exit_resume;
	}
	if (lm->als_users && !lm->enabled_als) {
		u8 ctrl = lm->pdata->als_control | LM3533_ALS_ENABLE;
		dev_dbg(dev, "%s: enabling ALS\n", __func__);
		rc = lm3533_write(lm, ctrl, REG_ALS_AVGT);
	}
exit_resume:
	return rc ? -EAGAIN : 0;
}
#else
#define lm3533_suspend NULL
#define lm3533_resume NULL
#endif

static const struct dev_pm_ops lm3533_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(lm3533_suspend, lm3533_resume)
	SET_RUNTIME_PM_OPS(lm3533_runtime_suspend, lm3533_runtime_resume, NULL)
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
	.shutdown = lm3533_shutdown,
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
