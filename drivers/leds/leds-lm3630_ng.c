/*
 * drivers/leds/leds-lm3630_ng.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * Author: Yu Hongxing <Hongxing2.Yu@sonymobile.com>
 *	   Raghavendra Alevoor <raghavendra.alevoor@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>

#define LM3630_DEV_NAME "lm3630"

/* Full scale current register value form current, max 29800 uA */
#define MIN_FSC_UA 5000
#define FSC_UA_LSB 750
#define I_UA_TO_FSC(ua) (((ua) - MIN_FSC_UA) / FSC_UA_LSB)

enum lm3630_control_bank {
	LM3630_CBNKA,
	LM3630_CBNKB,
	LM3630_BANK_NUM,
};

enum lm3630_led {
	LM3630_HVLED1,
	LM3630_HVLED2,
	LM3630_LED_NUM,
};

enum lm3630_pwm {
	LM3630_PWM_ENA_A	= 1 << 0,
	LM3630_PWM_ENA_B	= 1 << 1,
	LM3630_PWM_POL_LOW	= 1 << 2,
	LM3630_PWM_FEDBACK_A	= 1 << 3,
	LM3630_PWM_FEDBACK_B	= 1 << 4,
};

enum lm3630_br_ctl {
	LM3630_LED2_ON_A    = 1 << 0,
	LM3630_LED_EN_B     = 1 << 1,
	LM3630_LED_EN_A     = 1 << 2,
	LM3630_HVB_MAP_LIN  = 1 << 3, /*Clear the bit for exponential map*/
	LM3630_HVA_MAP_LIN  = 1 << 4, /*Clear the bit for exponential map*/
	LM3630_SLEEP_CMD    = 1 << 7,
};

enum lm3630_filter_strength {
	LM3630_FILSTR_512US = 0,
	LM3630_FILSTR_1024US,
	LM3630_FILSTR_2048US,
	LM3630_FILSTR_4096US,
};

enum lm3630_ovp_boost_pwm_config {
	LM3630_BOOST_500KHZ  = 0 << 0,
	LM3630_BOOST_1000KHZ = 1 << 0,
	LM3630_SHIFT_500K_OR_1MHZ = 0 << 1,
	LM3630_SHIFT_450K_OR_950KHZ = 1 << 1,
	LM3630_OCP_600mA  = 0 << 3,
	LM3630_OCP_800mA  = 1 << 3,
	LM3630_OCP_1000mA  = 2 << 3,
	LM3630_OCP_1200mA  = 3 << 3,
	LM3630_OVP_16V  = 0 << 5,
	LM3630_OVP_24V  = 1 << 5,
	LM3630_OVP_32V  = 2 << 5,
	LM3630_OVP_40V  = 3 << 5,
};

enum lm3630_ramp_rate {
	LM3630_RR_4MS,  /* start up time: 4ms; shutdown time: 0 */
	LM3630_RR_261MS,
	LM3630_RR_522MS,
	LM3630_RR_1045MS,
	LM3630_RR_2091MS,
	LM3630_RR_4182MS,
	LM3630_RR_8364MS,
	LM3630_RR_16730MS,
};

enum lm3630_led_fault {
	LM3630_OVP_ENABLED   = 1 << 0,
	LM3630_SHORT_ENABLED  = 1 << 2,
};

struct lm3630_bank_config {
	u8 fsc; /* use I_UA_TO_FSC(ua) */
	const char *iname; /* interface name */
};

struct lm3630_led_config {
	bool connected;
	enum lm3630_control_bank bank;
};

struct lm3630_startup_brightness {
	char const *intf_name;
	u8 brightness;
};

struct lm3630_platform_data {
	struct lm3630_bank_config b_cnf[LM3630_BANK_NUM];
	struct lm3630_led_config l_cnf[LM3630_LED_NUM];
	int gpio_hw_en;
	u8 pwm_feedback; /* ORed from enum lm3630_pwm */
	u8 ctl; /* ORed from enum lm3630_br_ctl */
	u8 filter_str;
	u8 ovp_boost; /* ORed from enum lm3630_ovp_boost_pwm_config */
	u8 led_fault; /* ORed from enum lm3630_led_fault */
	enum lm3630_ramp_rate runtime_rr;
	enum lm3630_ramp_rate startup_rr;
	struct lm3630_startup_brightness startup_brightness[2];

};

static int autosuspend_delay_ms = 100;
module_param(autosuspend_delay_ms, int, S_IRUGO);

enum {
	LM3630_REG_ADDR_FIRST = 0x00,
	LM3630_REG_ADDR_LAST  = 0x50,
	LM3630_REG_NUM = LM3630_REG_ADDR_LAST - LM3630_REG_ADDR_FIRST + 1,
	LM3630_PWR_DELAY_US = 1000,
	MAX_RR_MS  = 16730,
	MAX_RR_REG = 7,
};

struct lm3630_intf {
	struct led_classdev ldev;
	u8 banks;
	u8 brightness;
};

struct lm3630_data {
	struct lm3630_platform_data *pdata;
	struct lm3630_intf *intf[LM3630_BANK_NUM];
	struct i2c_client *i2c;
	u8 shadow[LM3630_REG_NUM];
	bool recovery_mode;
	struct mutex lock;
	bool lit_on_suspend;
};

enum {
	REG_CONT   = 0x00,
	REG_CONF   = 0x01,
	REG_BOOST_CONT    = 0x02,
	REG_BRIGHT_A   = 0x03,
	REG_BRIGHT_B   = 0x04,
	REG_CURR_A   = 0x05,
	REG_CURR_B   = 0x06,
	REG_ONOFF_RAMP    = 0x07,
	REG_RUN_RMAP    = 0x08,
	REG_LED_FAULT   = 0x0b,
	REG_SOFT_RES    = 0x0f,
	REG_PWM_OUTL    = 0x12,
	REG_PWM_OUTH   = 0x13,
	REG_REVISION   = 0x1f,
	REG_FILT_STRENG    = 0x50,
};

#define SHADOW(p, x) ((p)->shadow[(x) - LM3630_REG_ADDR_FIRST])

struct lm3630_reg_region {
	u8 start;
	u8 number;
};

static const struct lm3630_reg_region sync_regs[] = {
	{
		.start = REG_CONT,
		.number = 10,
	},

	{
		.start = REG_FILT_STRENG,
		.number = 1,
	},
};

static int lm3630_sync_shadow(struct lm3630_data *lm)
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

static int lm3630_sync_regs(struct lm3630_data *lm)
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

static int lm3630_setup(struct device *dev, struct lm3630_platform_data *pdata)
{
	int rc = gpio_request(pdata->gpio_hw_en, "lm3630_hwen");
	if (rc) {
		dev_err(dev, "failed to request gpio %d\n", pdata->gpio_hw_en);
		goto err_gpio;
	}

	rc = gpio_direction_output(pdata->gpio_hw_en, 1);
	if (rc) {
		dev_err(dev, "failed to set direction output, %d\n", rc);
		gpio_free(pdata->gpio_hw_en);
		goto err_gpio;
	}
err_gpio:
	return rc;
}
static void lm3630_teardown(struct device *dev)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);

	gpio_free(lm->pdata->gpio_hw_en);
	return;
}
static int lm3630_power_on(struct device *dev)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);

	gpio_set_value_cansleep(lm->pdata->gpio_hw_en, 1);
	return 0;
}
static int lm3630_power_off(struct device *dev)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);

	gpio_set_value_cansleep(lm->pdata->gpio_hw_en, 0);
	return 0;
}

static struct lm3630_platform_data *lm3630_get_platdata_dt(struct device *dev)
{
	struct lm3630_platform_data *pdata = NULL;
	struct device_node *devnode = dev->of_node;
	u32 i, cnt;
	u32 datalist[3];
	u32 temp_read;
	if (!devnode)
		return NULL;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		dev_err(dev, "Failed to allocate memory for pdata\n");
		return NULL;
	}

	pdata->gpio_hw_en = of_get_named_gpio(devnode, "gpio_hw_en", 0);

	/* Parse pwm_feedback */
	if (of_property_read_u32(devnode, "pwm_feedback", &temp_read)) {
		dev_err(dev, "Failed to get property: pwm_feedback\n");
		goto err_lm3630_get_platdata_dt;
	}
	pdata->pwm_feedback = (u8)temp_read;

	/* Parse ctl */
	if (of_property_read_u32(devnode, "ctl", &temp_read)) {
		dev_err(dev, "Failed to get property: ctl\n");
		goto err_lm3630_get_platdata_dt;
	}
	pdata->ctl = (u8)temp_read;

	/* Parse fsc */
	if (of_property_read_u32_array(devnode, "fsc", datalist, 2)) {
		dev_err(dev, "Failed to get property: fsc\n");
		goto err_lm3630_get_platdata_dt;
	}

	for (i = 0; i < LM3630_BANK_NUM; i++)
		pdata->b_cnf[i].fsc = I_UA_TO_FSC(datalist[i]);

	/* Parse iname */
	cnt = of_property_count_strings(devnode, "iname");
	if (cnt > LM3630_BANK_NUM) {
		dev_err(dev, "Wrong number of interfaces: %d\n", cnt);
		goto err_lm3630_get_platdata_dt;
	}
	for (i = 0; i < cnt; i++) {
		if (of_property_read_string_index(devnode, "iname", i,
			(const char **) &pdata->b_cnf[i].iname)) {
			dev_err(dev, "Failed to get property: "\
					"iname[%d]\n", i);
			goto err_lm3630_get_platdata_dt;
		}
	}

	/* Parse connected */
	if (of_property_read_u32_array(devnode, "connected", datalist, 2)) {
		dev_err(dev, "Failed to get property: connected\n");
		goto err_lm3630_get_platdata_dt;
	}

	for (i = 0; i < LM3630_BANK_NUM; i++)
		pdata->l_cnf[i].connected = datalist[i];

	/* Parse bank */
	if (of_property_read_u32_array(devnode, "bank", datalist, 2)) {
		dev_err(dev, "Failed to get property: bank\n");
		goto err_lm3630_get_platdata_dt;
	}

	for (i = 0; i < LM3630_BANK_NUM; i++)
		pdata->l_cnf[i].bank = datalist[i];

	/* Parse filter_str */
	if (of_property_read_u32(devnode, "filter_str", &temp_read)) {
		dev_err(dev, "Failed to get property: filter_str\n");
		goto err_lm3630_get_platdata_dt;
	}
	pdata->filter_str = (u8)temp_read;

	/* Parse ovp_boost */
	if (of_property_read_u32(devnode, "ovp_boost", &temp_read)) {
		dev_err(dev, "Failed to get property: filter_str\n");
		goto err_lm3630_get_platdata_dt;
	}
	pdata->ovp_boost = (u8)temp_read;

	/* Parse led_fault */
	if (of_property_read_u32(devnode, "led_fault", &temp_read)) {
		dev_err(dev, "Failed to get property: filter_str\n");
		goto err_lm3630_get_platdata_dt;
	}
	pdata->led_fault = (u8)temp_read;

	cnt = of_property_count_strings(devnode, "intf_name");
	if (cnt > LM3630_BANK_NUM) {
		dev_err(dev, "Wrong number of interfaces: %d\n", cnt);
		goto err_lm3630_get_platdata_dt;
	}
	/* Parse intf_name */
	for (i = 0; i < cnt; i++) {
		if (of_property_read_string_index(devnode, "intf_name", i,
			(const char **) &pdata->startup_brightness[i].	\
				intf_name)) {
			dev_err(dev, "Failed to get property: "\
						"inth_name[%d]\n", i);
			goto err_lm3630_get_platdata_dt;
		}
	}

	/* Parse brightness */
	if (of_property_read_u32_array(devnode, "brightness", datalist, 2)) {
		dev_err(dev, "Failed to get property: brightness\n");
		goto err_lm3630_get_platdata_dt;
	}

	for (i = 0; i < LM3630_BANK_NUM; i++)
		pdata->startup_brightness[i].brightness = datalist[i];

	return pdata;

err_lm3630_get_platdata_dt:
	devm_kfree(dev, pdata);
	return NULL;
}

static int lm3630_power_on_driver(struct lm3630_data *lm)
{
	int rc;
	dev_dbg(&lm->i2c->dev, "%s\n", __func__);
	rc = lm3630_power_on(&lm->i2c->dev);
	if (!rc) {
		usleep_range(LM3630_PWR_DELAY_US, LM3630_PWR_DELAY_US + 1000);
		rc = lm3630_sync_regs(lm);
	}
	return rc;
}

static int lm3630_power_off_driver(struct lm3630_data *lm)
{
	dev_dbg(&lm->i2c->dev, "%s\n", __func__);
	return lm3630_power_off(&lm->i2c->dev);
}

static int lm3630_recover(struct lm3630_data *lm)
{
	int rc;

	if (!lm->recovery_mode) {
		dev_warn(&lm->i2c->dev, "no recovery is possible\n");
		return -ENOSYS;
	}

	dev_info(&lm->i2c->dev, "recovering\n");
	lm3630_power_off_driver(lm);
	rc = lm3630_power_on_driver(lm);
	if (!rc)
		dev_info(&lm->i2c->dev, "recovered\n");
	else
		dev_info(&lm->i2c->dev, "failed to recover\n");
	return rc;
}

static int lm3630_write(struct lm3630_data *lm, u8 val, u8 reg)
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
	if (lm3630_recover(lm))
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

static int lm3630_read(struct lm3630_data *lm, u8 *val, u8 reg)
{
	int rc = pm_runtime_get_sync(&lm->i2c->dev);
	if (rc < 0) {
		pm_runtime_put_noidle(&lm->i2c->dev);
		return rc;
	}
	rc = i2c_smbus_read_byte_data(lm->i2c, reg);
	if (rc < 0) {
		rc = lm3630_recover(lm);
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

static int lm3630_setup_banks(struct lm3630_data *lm, u8 enabled)
{
	struct lm3630_bank_config const *bank;
	enum lm3630_control_bank b;
	int rc;

	for (b = LM3630_CBNKA; b < LM3630_BANK_NUM; b++) {
		bank = &lm->pdata->b_cnf[b]; /*b_cnf*/

		if (!((1 << b) & enabled))
			continue;

		rc = lm3630_write(lm, bank->fsc, REG_CURR_A + b);
		if (rc)
			goto err_exit;

	}
err_exit:
	return rc;
}

static int lm3630_setup_leds(struct lm3630_data *lm)
{
	enum lm3630_led i;
	struct lm3630_led_config const *leds = lm->pdata->l_cnf;
	u8 bena = 0;
	int rc = 0;

	for (i = LM3630_HVLED1; i < LM3630_LED_NUM; i++) {
		if (!leds[i].connected)
			continue;

		if (leds[i].bank > LM3630_CBNKB)
			goto err_exit;

		bena |= 1 << leds[i].bank;
	}
	rc = lm3630_setup_banks(lm, bena);
	return rc;
err_exit:
	dev_err(&lm->i2c->dev, "%s: bad parameters\n", __func__);
	return -EINVAL;
}

static int lm3630_chip_config(struct lm3630_data *lm)
{
	struct lm3630_platform_data *p = lm->pdata;
	int rc = 0;
	u8 init_value = SHADOW(lm, REG_CONT);

	if (init_value & LM3630_SLEEP_CMD) {
		rc = lm3630_write(lm, init_value & ~LM3630_SLEEP_CMD,
					REG_CONT);
	}

	rc = rc ? rc : lm3630_write(lm, p->filter_str, REG_FILT_STRENG);
	rc = rc ? rc : lm3630_write(lm, p->ovp_boost, REG_BOOST_CONT);
	rc = rc ? rc : lm3630_write(lm, p->startup_rr, REG_ONOFF_RAMP);
	rc = rc ? rc : lm3630_write(lm, p->runtime_rr, REG_RUN_RMAP);
	rc = rc ? rc : lm3630_write(lm, p->led_fault, REG_LED_FAULT);
	rc = rc ? rc : lm3630_write(lm, p->ctl, REG_CONT);
	rc = rc ? rc : lm3630_write(lm, p->pwm_feedback, REG_CONF);
	rc = rc ? rc : lm3630_setup_leds(lm);
	return rc;
}

static inline struct lm3630_intf *ldev_to_intf(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct lm3630_intf, ldev);
}

static void lm3630_led_brightness(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3630_data *lm = dev_get_drvdata(dev);
	struct lm3630_intf *intf = ldev_to_intf(led_cdev);
	enum lm3630_control_bank b;
	int rc;

	if (value <= LED_OFF)
		value = 0;
	else if (value >= LED_FULL)
		value = 255;

	dev_dbg(dev, "%s: brightness %d -> %d\n", led_cdev->name,
			intf->brightness, value);

	if (value && !intf->brightness) {
		rc = pm_runtime_get_sync(dev);
		if (rc < 0) {
			pm_runtime_put_noidle(dev);
			return;
		}
	}

	for (b = LM3630_CBNKA; b < LM3630_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		if (lm3630_write(lm, value, REG_BRIGHT_A + b))
			break;
	}

	if (!value && intf->brightness) {
		pm_runtime_mark_last_busy(&lm->i2c->dev);
		pm_runtime_put_autosuspend(&lm->i2c->dev);
	}
	intf->brightness = value;
}


static int lm3630_set_fsc(struct led_classdev *led_cdev, u8 fsc)
{
	struct device *dev = led_cdev->dev->parent;
	struct lm3630_data *lm = dev_get_drvdata(dev);
	struct lm3630_intf *intf = ldev_to_intf(led_cdev);
	enum lm3630_control_bank b;
	int rc = 0;
	u8 ctl;
	u8 init_value = SHADOW(lm, REG_CONT);

	if (init_value & LM3630_SLEEP_CMD) {
		rc = lm3630_write(lm, init_value & ~LM3630_SLEEP_CMD,
					REG_CONT);
	}
	for (b = LM3630_CBNKA; b < LM3630_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		ctl = SHADOW(lm, REG_CURR_A + b);
		if (ctl != fsc) {
			rc = lm3630_write(lm, fsc, REG_CURR_A + b);
			if (rc)
				goto err_exit;
		}
	}
err_exit:
	return rc;
}

#define MAX_FULL_SCALE 31

static ssize_t lm3630_fsc_set(struct device *ldev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	unsigned long fsc;
	int rc;

	if (!kstrtoul(buf, 10, &fsc) && (fsc <= MAX_FULL_SCALE))
		rc = lm3630_set_fsc(led_cdev, fsc);
	else
		rc = -EINVAL;
	return rc ? rc : size;
}

static ssize_t lm3630_fsc_show(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct device *dev = led_cdev->dev->parent;
	struct lm3630_data *lm = dev_get_drvdata(dev);
	struct lm3630_intf *intf = ldev_to_intf(led_cdev);
	enum lm3630_control_bank b;
	u8 fsc = 0;
	int rc = 0;

	for (b = LM3630_CBNKA; b < LM3630_BANK_NUM; b++) {
		if (!(intf->banks & (1 << b)))
			continue;
		rc = lm3630_read(lm, &fsc, REG_CURR_A + b);
		break;
	}
	return rc ? rc : scnprintf(buf, PAGE_SIZE, "%u\n", fsc);
}

static ssize_t lm3630_pwm_show(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct device *dev = led_cdev->dev->parent;
	struct lm3630_data *lm = dev_get_drvdata(dev);
	u8 pwmh = 0, pwml = 0;
	int rc = 0;

	rc = lm3630_read(lm, &pwmh, REG_PWM_OUTH);
	rc = rc ? rc : lm3630_read(lm, &pwml, REG_PWM_OUTL);

	return rc ? rc : scnprintf(buf, PAGE_SIZE,
			"PWMH %u, PWML %u", pwmh, pwml);
}

static ssize_t lm3630_cabc_store(struct device *ldev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct device *dev = led_cdev->dev->parent;
	struct lm3630_data *lm = dev_get_drvdata(dev);
	struct lm3630_intf *intf = ldev_to_intf(led_cdev);

	long tmp;
	u8 init_value;
	u8 reg_bit = LM3630_PWM_ENA_A;
	int rc;

	if (kstrtol(buf, 10, &tmp)) {
		dev_err(ldev, "Error reading user data");
		return -EINVAL;
	}
	init_value = SHADOW(lm, REG_CONF);

	if ((intf->banks >> 1) == LM3630_CBNKB)
		reg_bit = LM3630_PWM_ENA_B;

	if (tmp != 0)
		init_value |= reg_bit;
	else
		init_value &= ~reg_bit;

	dev_dbg(ldev, "%s: PWM setting %d\n",
							__func__, init_value);
	rc = lm3630_write(lm, init_value, REG_CONF);
	return rc ? rc : count;
}

static ssize_t lm3630_cabc_show(struct device *ldev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(ldev);
	struct device *dev = led_cdev->dev->parent;
	struct lm3630_data *lm = dev_get_drvdata(dev);
	struct lm3630_intf *intf = ldev_to_intf(led_cdev);
	u8 reg_bit = LM3630_PWM_ENA_A;
	unsigned reg = SHADOW(lm, REG_CONF);

	if ((intf->banks >> 1) == LM3630_CBNKB)
		reg_bit = LM3630_PWM_ENA_B;

	dev_dbg(dev, "%s: PWM setting %d\n",
						__func__, (reg & reg_bit));
	return scnprintf(buf, PAGE_SIZE, "%u",
			(reg & reg_bit));

}

static struct device_attribute led_ctl_attr[] = {
	__ATTR(fsc, 0600, lm3630_fsc_show, lm3630_fsc_set),
	__ATTR(pwm, 0600, lm3630_pwm_show, NULL),
	__ATTR(cabc, 0660, lm3630_cabc_show, lm3630_cabc_store),
	__ATTR_NULL,
};

static int intf_add_attributes(struct lm3630_intf *intf,
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

static void intf_remove_attributes(struct lm3630_intf *intf,
		struct device_attribute *attrs)
{
	int i;
	struct device *dev = intf->ldev.dev;

	for (i = 0; attr_name(attrs[i]); i++)
		device_remove_file(dev, &attrs[i]);
}

static int lm3630_add_intf(struct lm3630_data *lm, const char *nm,
			enum lm3630_control_bank bank)
{
	struct device *dev = &lm->i2c->dev;
	struct lm3630_intf *intf = NULL;
	int rc = 0;

	if (!lm->intf[bank]) {
		intf = kzalloc(sizeof(struct lm3630_intf), GFP_KERNEL);
		if (!intf) {
			dev_err(dev, "%s: mo memory\n", __func__);
			rc = -ENOMEM;
			goto exit;
		}
		intf->ldev.name = nm;
		intf->ldev.brightness = LED_OFF;
		/*intf->ldev.default_trigger = "bkl-trigger";*/
		intf->ldev.brightness_set = lm3630_led_brightness;
		lm->intf[bank] = intf;
		dev_info(dev, "New led '%s'\n", nm);

	} else if (!strncmp(nm, lm->intf[bank]->ldev.name, strlen(nm))) {
		intf = lm->intf[bank];
	}

	if (!intf) {
		dev_err(dev, "%s: no free space for interfaces\n", __func__);
		rc = -ENOMEM;
		goto exit;
	}

	intf->banks |= 1 << bank;
exit:
	return rc;
}

static int lm3630_setup_interfaces(struct lm3630_data *lm)
{
	const char *name;
	int rc;
	enum lm3630_control_bank b;

	for (b = LM3630_CBNKA; b < LM3630_BANK_NUM; b++) {
		name = lm->pdata->b_cnf[b].iname;
		if (!name)
			continue;
		rc = lm3630_add_intf(lm, name, b);
		if (rc)
			return rc;
	}
	return 0;
}

static void lm3630_cleanup(struct lm3630_data *lm)
{
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(lm->intf); i++) {
		kfree(lm->intf[i]);
		lm->intf[i] = NULL;
	}
}

static int lm3630_register_ldevs(struct lm3630_data *lm)
{
	int i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++) {
		rc = led_classdev_register(&lm->i2c->dev, &lm->intf[i]->ldev);
		if (rc)
			goto roll_back;
		rc = intf_add_attributes(lm->intf[i], led_ctl_attr);
		if (rc)
			goto roll_back_ctl;
	}
	return rc;
roll_back_ctl:
	i++;
roll_back:
	while (--i >= 0)
		led_classdev_unregister(&lm->intf[i]->ldev);
	dev_err(&lm->i2c->dev, "%s: failed, rc = %d.\n", __func__, rc);
	return rc;
}

static void lm3630_unregister_ldevs(struct lm3630_data *lm)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++) {
		intf_remove_attributes(lm->intf[i], led_ctl_attr);
		led_classdev_unregister(&lm->intf[i]->ldev);
	}
}


static unsigned lm3630_rtime2reg(unsigned up, unsigned dn)
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

static ssize_t lm3630_rr_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);
	unsigned up, dn;
	int rc;
	int n = sscanf(buf, "%5u,%5u", &up, &dn);

	if (n != 2 || up > MAX_RR_MS || dn > MAX_RR_MS)
		return -EINVAL;
	up = lm3630_rtime2reg(up, dn);
	dev_dbg(dev, "rr_up: %d, rr_down %d\n",
			MAX_RR_MS >> (MAX_RR_REG - (up >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (up & 7)));
	rc = lm3630_write(lm, up, REG_RUN_RMAP);
	return rc ? rc : count;
}

static ssize_t lm3630_rr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);
	unsigned reg = SHADOW(lm, REG_RUN_RMAP);
	return scnprintf(buf, PAGE_SIZE, "%u,%u # min 0 ms, max %d ms",
			MAX_RR_MS >> (MAX_RR_REG - (reg >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (reg & 7)), MAX_RR_MS);
}

static ssize_t lm3630_start_shdn_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);
	unsigned up, dn;
	int rc;
	int n = sscanf(buf, "%5u,%5u", &up, &dn);

	if (n != 2 || up > MAX_RR_MS || dn > MAX_RR_MS)
		return -EINVAL;
	up = lm3630_rtime2reg(up, dn);
	dev_dbg(dev, "ss_up: %d, ss_down %d\n",
			MAX_RR_MS >> (MAX_RR_REG - (up >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (up & 7)));
	rc = lm3630_write(lm, up, REG_ONOFF_RAMP);
	return rc ? rc : count;
}

static ssize_t lm3630_start_shdn_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);
	unsigned reg = SHADOW(lm, REG_ONOFF_RAMP);
	return scnprintf(buf, PAGE_SIZE, "%u,%u # min 0 ms, max %d ms",
			MAX_RR_MS >> (MAX_RR_REG - (reg >> 3)),
			MAX_RR_MS >> (MAX_RR_REG - (reg & 7)), MAX_RR_MS);
}

static struct lm3630_intf *lm3630_intf_get(struct lm3630_data *lm,
		char const *name)
{
	unsigned i;

	if (!name)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(lm->intf) && lm->intf[i]; i++)
		if (!strncmp(lm->intf[i]->ldev.name, name, strlen(name)))
			return lm->intf[i];
	return NULL;
}

static void lm3630_set_startup_br(struct lm3630_data *lm)
{
	struct lm3630_intf *intf;
	struct lm3630_startup_brightness const *p =
			lm->pdata->startup_brightness;
	int i;
	int count = ARRAY_SIZE(lm->pdata->startup_brightness);
	for (i = 0; i < count; i++) {
		intf = lm3630_intf_get(lm, p->intf_name);
		if (intf) {
			dev_info(&lm->i2c->dev, "%s: %d on startup\n",
				p->intf_name, p->brightness);
			lm3630_led_brightness(&intf->ldev, p->brightness);
			intf->ldev.brightness = p->brightness;
		}
		p++;
	}
}

static ssize_t lm3630_reset_store(struct device *dev,
		struct device_attribute *dev_attr,
		const char *buf, size_t count)
{
	unsigned long reset;
	int rc;
	struct lm3630_data *lm = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &reset) && reset == 1 ?
		lm3630_recover(lm) : -EINVAL;
	return rc ? rc : count;
}

static const DEVICE_ATTR(reset, S_IWUSR, NULL, lm3630_reset_store);
static const DEVICE_ATTR(rt_rate_ms, S_IWUSR | S_IRUSR,
		lm3630_rr_show, lm3630_rr_store);
static const DEVICE_ATTR(start_shdn_ms, S_IWUSR | S_IRUSR,
		lm3630_start_shdn_show, lm3630_start_shdn_store);

static const struct attribute *lm3630_attrs[] = {
	&dev_attr_reset.attr,
	&dev_attr_rt_rate_ms.attr,
	&dev_attr_start_shdn_ms.attr,
	NULL,
};

static int __devinit lm3630_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct lm3630_platform_data *pdata = NULL;
	struct device *dev = &client->dev;
	struct lm3630_data *lm;
	int rc;

	dev_info(dev, "%s\n", __func__);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "%s: SMBUS byte data not supported\n", __func__);
		return -EIO;
	}

	pdata = lm3630_get_platdata_dt(dev);

	if (!pdata)
		return -ENOMEM;

	if (pdata->gpio_hw_en) {
		rc = lm3630_setup(dev, pdata);
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

	rc = lm3630_power_on(dev);
	if (rc) {
		dev_err(dev, "%s: failed to power on.\n", __func__);
		goto err_power_on;
	}
	usleep_range(LM3630_PWR_DELAY_US, LM3630_PWR_DELAY_US + 1000);

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_set_autosuspend_delay(dev, autosuspend_delay_ms);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_mark_last_busy(dev);

	rc = lm3630_sync_shadow(lm);
	if (rc)
		goto err_configure;

	rc = lm3630_chip_config(lm);
	if (rc)
		goto err_configure;
	rc = lm3630_setup_interfaces(lm);
	if (rc)
		goto err_interfaces;
	rc = lm3630_register_ldevs(lm);
	if (rc)
		goto err_reg_ldevs;
	lm3630_set_startup_br(lm);
	lm->recovery_mode = true;
	rc = sysfs_create_files(&dev->kobj, lm3630_attrs);
	if (rc) {
		dev_err(dev, "%s: failed to create files.\n", __func__);
		goto err_file_create;
	}
	dev_info(dev, "%s: completed.\n", __func__);
	return 0;

err_file_create:
	lm3630_unregister_ldevs(lm);
err_reg_ldevs:
err_interfaces:
	lm3630_cleanup(lm);
err_configure:
	pm_runtime_disable(&client->dev);
err_power_on:
	mutex_destroy(&lm->lock);
	kfree(lm);
err_alloc_data_failed:
	lm3630_teardown(dev);
	dev_err(dev, "%s: failed.\n", __func__);
	return rc;
}

static int __devexit lm3630_remove(struct i2c_client *client)
{
	struct lm3630_data *lm = i2c_get_clientdata(client);

	sysfs_remove_files(&client->dev.kobj, lm3630_attrs);
	lm3630_power_off(&client->dev);
	pm_runtime_disable(&client->dev);
	lm3630_cleanup(lm);
	lm3630_teardown(&client->dev);
	mutex_destroy(&lm->lock);
	kfree(lm);
	return 0;
}

static void lm3630_shutdown(struct i2c_client *client)
{
	struct lm3630_data *lm = i2c_get_clientdata(client);
	unsigned i;

	for (i = 0; i < LM3630_BANK_NUM && lm->intf[i]; i++) {
		if (lm->intf[i]->ldev.brightness > LED_OFF)
			lm3630_led_brightness(&lm->intf[i]->ldev, 0);
	}
	lm3630_unregister_ldevs(lm);
}

#ifdef CONFIG_PM_RUNTIME
static int lm3630_runtime_suspend(struct device *dev)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);
	int rc = lm3630_power_off_driver(lm);
	dev_dbg(dev, "%s\n", __func__);
	return rc ? -EAGAIN : 0;
}

static int lm3630_runtime_resume(struct device *dev)
{
	struct lm3630_data *lm = dev_get_drvdata(dev);
	int rc = lm3630_power_on_driver(lm);
	dev_dbg(dev, "%s\n", __func__);
	return rc ? -EAGAIN : 0;
}
#endif

#ifdef CONFIG_PM_SLEEP
static int lm3630_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm3630_data *lm = i2c_get_clientdata(client);
	int rc = 0;
	size_t i;

	dev_dbg(dev, "%s\n", __func__);

	if (pm_runtime_suspended(dev)) {
		dev_dbg(dev, "%s: runtime-suspended.\n", __func__);
		return 0;
	}
	if (mutex_is_locked(&lm->lock))
		return -EBUSY;
	for (i = 0; i < ARRAY_SIZE(lm->intf); i++) {
		struct lm3630_intf *intf = lm->intf[i];
		if (!intf)
			break;
		if (!intf->brightness)
			continue;
		lm->lit_on_suspend = true;
	}
	if (!(rc || lm->lit_on_suspend))
		rc = lm3630_power_off_driver(lm);

	return rc ? -EAGAIN : 0;
}

static int lm3630_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct lm3630_data *lm = i2c_get_clientdata(client);
	int rc = 0;

	dev_dbg(dev, "%s\n", __func__);
	if (pm_runtime_suspended(dev)) {
		dev_dbg(dev, "%s: was runtime-suspended.\n", __func__);
		return 0;
	}
	if (!lm->lit_on_suspend) {
		rc = lm3630_power_on_driver(lm);
		if (rc)
			goto exit_resume;
	}

exit_resume:
	return rc ? -EAGAIN : 0;
}
#else
#define lm3630_suspend NULL
#define lm3630_resume NULL
#endif

static const struct dev_pm_ops lm3630_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(lm3630_suspend, lm3630_resume)
	SET_RUNTIME_PM_OPS(lm3630_runtime_suspend, lm3630_runtime_resume, NULL)
};

static const struct i2c_device_id lm3630_id[] = {
	{LM3630_DEV_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, lm3630_id);

static struct of_device_id lm3630_bl_match_table[] = {
	{ .compatible = "ti,lm3630_bl" },
	{},
};

static struct i2c_driver lm3630_driver = {
	.driver = {
		.name = LM3630_DEV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = lm3630_bl_match_table,
		.pm = &lm3630_pm,
	},
	.probe = lm3630_probe,
	.remove = __devexit_p(lm3630_remove),
	.shutdown = lm3630_shutdown,
	.id_table = lm3630_id,
};

static int __init lm3630_init(void)
{
	int err = i2c_add_driver(&lm3630_driver);
	pr_info("%s: lm3630 LMU IC driver, built %s @ %s\n",
		 __func__, __DATE__, __TIME__);
	return err;
}

static void __exit lm3630_exit(void)
{
	i2c_del_driver(&lm3630_driver);
}

module_init(lm3630_init);
module_exit(lm3630_exit);

MODULE_AUTHOR("Yu Hongxing <Hongxing2.Yu@sonymobile.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("lm3630 LMU IC driver");
