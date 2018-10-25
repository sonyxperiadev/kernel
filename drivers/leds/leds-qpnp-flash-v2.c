/* Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)	"flashv2: %s: " fmt, __func__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/regmap.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/regulator/consumer.h>
#include <linux/leds-qpnp-flash.h>
#include <linux/leds-qpnp-flash-v2.h>
#include <linux/qpnp/qpnp-revid.h>
#include <linux/log2.h>
#include "leds.h"

#define	FLASH_LED_REG_LED_STATUS1(base)		(base + 0x08)

#define	FLASH_LED_REG_LED_STATUS2(base)		(base + 0x09)
#define	FLASH_LED_VPH_DROOP_FAULT_MASK		BIT(4)

#define	FLASH_LED_REG_INT_RT_STS(base)		(base + 0x10)

#define	FLASH_LED_REG_SAFETY_TMR(base)		(base + 0x40)
#define	FLASH_LED_SAFETY_TMR_ENABLE		BIT(7)

#define	FLASH_LED_REG_TGR_CURRENT(base)		(base + 0x43)

#define	FLASH_LED_REG_MOD_CTRL(base)		(base + 0x46)
#define	FLASH_LED_MOD_CTRL_MASK			BIT(7)
#define	FLASH_LED_MOD_ENABLE			BIT(7)

#define	FLASH_LED_REG_IRES(base)		(base + 0x47)

#define	FLASH_LED_REG_STROBE_CFG(base)		(base + 0x48)
#define	FLASH_LED_STROBE_MASK			GENMASK(1, 0)

#define	FLASH_LED_REG_STROBE_CTRL(base)		(base + 0x49)
#define	FLASH_LED_HW_SW_STROBE_SEL_BIT		BIT(2)
#define	FLASH_HW_STROBE_MASK			GENMASK(2, 0)

#define	FLASH_LED_EN_LED_CTRL(base)		(base + 0x4C)
#define	FLASH_LED_ENABLE			BIT(0)

#define	FLASH_LED_REG_HDRM_PRGM(base)		(base + 0x4D)
#define	FLASH_LED_HDRM_VOL_MASK			GENMASK(7, 4)
#define	FLASH_LED_HDRM_VOL_SHIFT		4

#define	FLASH_LED_REG_HDRM_AUTO_MODE_CTRL(base)	(base + 0x50)
#define	FLASH_LED_REG_WARMUP_DELAY(base)	(base + 0x51)

#define	FLASH_LED_REG_ISC_DELAY(base)		(base + 0x52)
#define	FLASH_LED_ISC_WARMUP_DELAY_MASK		GENMASK(1, 0)
#define	FLASH_LED_ISC_WARMUP_DELAY_SHIFT		6

#define	FLASH_LED_REG_THERMAL_RMP_DN_RATE(base)	(base + 0x55)
#define	THERMAL_OTST1_RAMP_CTRL_MASK		BIT(7)
#define	THERMAL_OTST1_RAMP_CTRL_SHIFT		7
#define	THERMAL_DERATE_SLOW_SHIFT		4
#define	THERMAL_DERATE_SLOW_MASK		GENMASK(6, 4)
#define	THERMAL_DERATE_FAST_MASK		GENMASK(2, 0)

#define	FLASH_LED_REG_THERMAL_THRSH1(base)	(base + 0x56)
#define	FLASH_LED_THERMAL_THRSH_MASK		GENMASK(2, 0)

#define	FLASH_LED_REG_THERMAL_THRSH2(base)	(base + 0x57)
#define	FLASH_LED_REG_THERMAL_THRSH3(base)	(base + 0x58)

#define	FLASH_LED_REG_THERMAL_HYSTERESIS(base)	(base + 0x59)
#define	FLASH_LED_THERMAL_HYSTERESIS_MASK	GENMASK(1, 0)

#define	FLASH_LED_REG_THERMAL_DEBOUNCE(base)	(base + 0x5A)
#define	FLASH_LED_THERMAL_DEBOUNCE_MASK		GENMASK(1, 0)

#define	FLASH_LED_REG_VPH_DROOP_THRESHOLD(base)	(base + 0x61)
#define	FLASH_LED_VPH_DROOP_HYSTERESIS_MASK	GENMASK(5, 4)
#define	FLASH_LED_VPH_DROOP_THRESHOLD_MASK	GENMASK(2, 0)
#define	FLASH_LED_VPH_DROOP_HYST_SHIFT		4

#define	FLASH_LED_REG_VPH_DROOP_DEBOUNCE(base)	(base + 0x62)
#define	FLASH_LED_VPH_DROOP_DEBOUNCE_MASK	GENMASK(1, 0)

#define	FLASH_LED_REG_ILED_GRT_THRSH(base)	(base + 0x67)
#define	FLASH_LED_ILED_GRT_THRSH_MASK		GENMASK(5, 0)

#define	FLASH_LED_REG_LED1N2_ICLAMP_LOW(base)	(base + 0x68)
#define	FLASH_LED_REG_LED1N2_ICLAMP_MID(base)	(base + 0x69)
#define	FLASH_LED_REG_LED3_ICLAMP_LOW(base)	(base + 0x6A)

#define	FLASH_LED_REG_LED3_ICLAMP_MID(base)	(base + 0x6B)
#define	FLASH_LED_CURRENT_MASK			GENMASK(6, 0)

#define	FLASH_LED_REG_MITIGATION_SEL(base)	(base + 0x6E)
#define	FLASH_LED_CHGR_MITIGATION_SEL_MASK	GENMASK(5, 4)
#define	FLASH_LED_LMH_MITIGATION_SEL_MASK	GENMASK(1, 0)

#define	FLASH_LED_REG_MITIGATION_SW(base)	(base + 0x6F)
#define	FLASH_LED_LMH_MITIGATION_EN_MASK	BIT(0)
#define	FLASH_LED_CHGR_MITIGATION_EN_MASK	BIT(4)
#define	FLASH_LED_CHGR_MITIGATION_ENABLE	BIT(4)

#define	FLASH_LED_REG_LMH_LEVEL(base)		(base + 0x70)
#define	FLASH_LED_LMH_LEVEL_MASK		GENMASK(1, 0)

#define	FLASH_LED_REG_MULTI_STROBE_CTRL(base)	(base + 0x71)
#define	LED3_FLASH_ONCE_ONLY_BIT		BIT(1)
#define LED1N2_FLASH_ONCE_ONLY_BIT		BIT(0)

#define	FLASH_LED_REG_LPG_INPUT_CTRL(base)	(base + 0x72)
#define	LPG_INPUT_SEL_BIT			BIT(0)

#define	FLASH_LED_REG_CURRENT_DERATE_EN(base)	(base + 0x76)
#define	FLASH_LED_CURRENT_DERATE_EN_MASK	GENMASK(2, 0)

#define	VPH_DROOP_DEBOUNCE_US_TO_VAL(val_us)	(val_us / 8)
#define	VPH_DROOP_HYST_MV_TO_VAL(val_mv)	(val_mv / 25)
#define	VPH_DROOP_THRESH_VAL_TO_UV(val)		((val + 25) * 100000)
#define	MITIGATION_THRSH_MA_TO_VAL(val_ma)	(val_ma / 100)
#define	THERMAL_HYST_TEMP_TO_VAL(val, divisor)	(val / divisor)

#define	FLASH_LED_WARMUP_DELAY_DEFAULT			2
#define	FLASH_LED_ISC_DELAY_DEFAULT			3
#define	FLASH_LED_VPH_DROOP_DEBOUNCE_DEFAULT		2
#define	FLASH_LED_VPH_DROOP_HYST_DEFAULT		2
#define	FLASH_LED_VPH_DROOP_THRESH_DEFAULT		5
#define	BHARGER_FLASH_LED_VPH_DROOP_THRESH_DEFAULT	7
#define	FLASH_LED_DEBOUNCE_MAX				3
#define	FLASH_LED_HYSTERESIS_MAX			3
#define	FLASH_LED_VPH_DROOP_THRESH_MAX			7
#define	THERMAL_DERATE_SLOW_MAX				314592
#define	THERMAL_DERATE_FAST_MAX				512
#define	THERMAL_DEBOUNCE_TIME_MAX			64
#define	THERMAL_DERATE_HYSTERESIS_MAX			3
#define	FLASH_LED_THERMAL_THRSH_MIN			3
#define	FLASH_LED_THERMAL_THRSH_MAX			7
#define	FLASH_LED_THERMAL_OTST_LEVELS			3
#define	FLASH_LED_VLED_MAX_DEFAULT_UV			3500000
#define	FLASH_LED_IBATT_OCP_THRESH_DEFAULT_UA		4500000
#define	FLASH_LED_RPARA_DEFAULT_UOHM			0
#define	FLASH_LED_LMH_LEVEL_DEFAULT			0
#define	FLASH_LED_LMH_MITIGATION_ENABLE			1
#define	FLASH_LED_LMH_MITIGATION_DISABLE		0
#define	FLASH_LED_CHGR_MITIGATION_DISABLE		0
#define	FLASH_LED_LMH_MITIGATION_SEL_DEFAULT		2
#define	FLASH_LED_MITIGATION_SEL_MAX			2
#define	FLASH_LED_CHGR_MITIGATION_SEL_SHIFT		4
#define	FLASH_LED_CHGR_MITIGATION_THRSH_DEFAULT		0xA
#define	FLASH_LED_CHGR_MITIGATION_THRSH_MAX		0x1F
#define	FLASH_LED_LMH_OCV_THRESH_DEFAULT_UV		3700000
#define	FLASH_LED_LMH_RBATT_THRESH_DEFAULT_UOHM		400000
#define	FLASH_LED_IRES_BASE				3
#define	FLASH_LED_IRES_DIVISOR				2500
#define	FLASH_LED_IRES_MIN_UA				5000
#define	FLASH_LED_IRES_DEFAULT_UA			12500
#define	FLASH_LED_IRES_DEFAULT_VAL			0x00
#define	FLASH_LED_HDRM_VOL_DEFAULT_MV			0x80
#define	FLASH_LED_HDRM_VOL_HI_LO_WIN_DEFAULT_MV		0x04
#define	FLASH_LED_HDRM_VOL_BASE_MV			125
#define	FLASH_LED_HDRM_VOL_STEP_MV			25
#define	FLASH_LED_STROBE_CFG_DEFAULT			0x00
#define	FLASH_LED_HW_STROBE_OPTION_1			0x00
#define	FLASH_LED_HW_STROBE_OPTION_2			0x01
#define	FLASH_LED_HW_STROBE_OPTION_3			0x02
#define	FLASH_LED_DISABLE				0x00
#define	FLASH_LED_SAFETY_TMR_DISABLED			0x13
#define	FLASH_LED_MAX_TOTAL_CURRENT_MA			3750
#define	FLASH_LED_IRES5P0_MAX_CURR_MA			640
#define	FLASH_LED_IRES7P5_MAX_CURR_MA			960
#define	FLASH_LED_IRES10P0_MAX_CURR_MA			1280
#define	FLASH_LED_IRES12P5_MAX_CURR_MA			1600
#define	MAX_IRES_LEVELS					4
#define	FLASH_BST_PWM_OVRHD_MIN_UV			300000
#define	FLASH_BST_PWM_OVRHD_MAX_UV			600000

/* notifier call chain for flash-led irqs */
static ATOMIC_NOTIFIER_HEAD(irq_notifier_list);

enum flash_charger_mitigation {
	FLASH_DISABLE_CHARGER_MITIGATION,
	FLASH_HW_CHARGER_MITIGATION_BY_ILED_THRSHLD,
	FLASH_SW_CHARGER_MITIGATION,
};

enum flash_led_type {
	FLASH_LED_TYPE_UNKNOWN,
	FLASH_LED_TYPE_FLASH,
	FLASH_LED_TYPE_TORCH,
};

enum {
	LED1 = 0,
	LED2,
	LED3,
};

enum strobe_type {
	SW_STROBE = 0,
	HW_STROBE,
	LPG_STROBE,
};

/*
 * Configurations for each individual LED
 */
struct flash_node_data {
	struct platform_device		*pdev;
	struct led_classdev		cdev;
	struct pinctrl			*strobe_pinctrl;
	struct pinctrl_state		*hw_strobe_state_active;
	struct pinctrl_state		*hw_strobe_state_suspend;
	int				hw_strobe_gpio;
	int				ires_ua;
	int				default_ires_ua;
	int				max_current;
	int				current_ma;
	int				prev_current_ma;
	u8				duration;
	u8				id;
	u8				ires_idx;
	u8				default_ires_idx;
	u8				hdrm_val;
	u8				current_reg_val;
	u8				strobe_ctrl;
	u8				strobe_sel;
	enum flash_led_type		type;
	bool				led_on;
};


struct flash_switch_data {
	struct platform_device		*pdev;
	struct regulator		*vreg;
	struct pinctrl			*led_en_pinctrl;
	struct pinctrl_state		*gpio_state_active;
	struct pinctrl_state		*gpio_state_suspend;
	struct led_classdev		cdev;
	int				led_mask;
	bool				regulator_on;
	bool				enabled;
	bool				symmetry_en;
};

/*
 * Flash LED configuration read from device tree
 */
struct flash_led_platform_data {
	struct pmic_revid_data	*pmic_rev_id;
	int			*thermal_derate_current;
	int			all_ramp_up_done_irq;
	int			all_ramp_down_done_irq;
	int			led_fault_irq;
	int			ibatt_ocp_threshold_ua;
	int			vled_max_uv;
	int			rpara_uohm;
	int			lmh_rbatt_threshold_uohm;
	int			lmh_ocv_threshold_uv;
	int			thermal_derate_slow;
	int			thermal_derate_fast;
	int			thermal_hysteresis;
	int			thermal_debounce;
	int			thermal_thrsh1;
	int			thermal_thrsh2;
	int			thermal_thrsh3;
	int			hw_strobe_option;
	u32			led1n2_iclamp_low_ma;
	u32			led1n2_iclamp_mid_ma;
	u32			led3_iclamp_low_ma;
	u32			led3_iclamp_mid_ma;
	u32			bst_pwm_ovrhd_uv;
	u8			isc_delay;
	u8			warmup_delay;
	u8			current_derate_en_cfg;
	u8			vph_droop_threshold;
	u8			vph_droop_hysteresis;
	u8			vph_droop_debounce;
	u8			lmh_mitigation_sel;
	u8			chgr_mitigation_sel;
	u8			lmh_level;
	u8			iled_thrsh_val;
	bool			hdrm_auto_mode_en;
	bool			thermal_derate_en;
	bool			otst_ramp_bkup_en;
};

/*
 * Flash LED data structure containing flash LED attributes
 */
struct qpnp_flash_led {
	struct flash_led_platform_data	*pdata;
	struct platform_device		*pdev;
	struct regmap			*regmap;
	struct flash_node_data		*fnode;
	struct flash_switch_data	*snode;
	struct power_supply		*bms_psy;
	struct power_supply		*main_psy;
	struct power_supply		*usb_psy;
	struct notifier_block		nb;
	spinlock_t			lock;
	int				num_fnodes;
	int				num_snodes;
	int				enable;
	int				total_current_ma;
	u16				base;
	bool				trigger_lmh;
	bool				trigger_chgr;
};

static int thermal_derate_slow_table[] = {
	128, 256, 512, 1024, 2048, 4096, 8192, 314592,
};

static int thermal_derate_fast_table[] = {
	32, 64, 96, 128, 256, 384, 512,
};

static int otst1_threshold_table[] = {
	85, 79, 73, 67, 109, 103, 97, 91,
};

static int otst2_threshold_table[] = {
	110, 104, 98, 92, 134, 128, 122, 116,
};

static int otst3_threshold_table[] = {
	125, 119, 113, 107, 149, 143, 137, 131,
};

static int max_ires_curr_ma_table[MAX_IRES_LEVELS] = {
	FLASH_LED_IRES12P5_MAX_CURR_MA, FLASH_LED_IRES10P0_MAX_CURR_MA,
	FLASH_LED_IRES7P5_MAX_CURR_MA, FLASH_LED_IRES5P0_MAX_CURR_MA
};

static inline int get_current_reg_code(int target_curr_ma, int ires_ua)
{
	if (!ires_ua || !target_curr_ma || (target_curr_ma < (ires_ua / 1000)))
		return 0;

	return DIV_ROUND_CLOSEST(target_curr_ma * 1000, ires_ua) - 1;
}

static int qpnp_flash_led_read(struct qpnp_flash_led *led, u16 addr, u8 *data)
{
	int rc;
	uint val;

	rc = regmap_read(led->regmap, addr, &val);
	if (rc < 0) {
		pr_err("Unable to read from 0x%04X rc = %d\n", addr, rc);
		return rc;
	}

	pr_debug("Read 0x%02X from addr 0x%04X\n", val, addr);
	*data = (u8)val;
	return 0;
}

static int qpnp_flash_led_write(struct qpnp_flash_led *led, u16 addr, u8 data)
{
	int rc;

	rc = regmap_write(led->regmap, addr, data);
	if (rc < 0) {
		pr_err("Unable to write to 0x%04X rc = %d\n", addr, rc);
		return rc;
	}

	pr_debug("Wrote 0x%02X to addr 0x%04X\n", data, addr);
	return 0;
}

static int
qpnp_flash_led_masked_read(struct qpnp_flash_led *led, u16 addr, u8 mask,
								u8 *val)
{
	int rc;

	rc = qpnp_flash_led_read(led, addr, val);
	if (rc < 0)
		return rc;

	*val &= mask;
	return rc;
}

static int
qpnp_flash_led_masked_write(struct qpnp_flash_led *led, u16 addr, u8 mask,
								u8 val)
{
	int rc;

	rc = regmap_update_bits(led->regmap, addr, mask, val);
	if (rc < 0)
		pr_err("Unable to update bits from 0x%04X, rc = %d\n", addr,
			rc);
	else
		pr_debug("Wrote 0x%02X to addr 0x%04X\n", val, addr);

	return rc;
}

static enum
led_brightness qpnp_flash_led_brightness_get(struct led_classdev *led_cdev)
{
	return led_cdev->brightness;
}

static int qpnp_flash_led_init_settings(struct qpnp_flash_led *led)
{
	int rc, i, addr_offset;
	u8 val = 0, mask, strobe_mask = 0, strobe_ctrl;

	for (i = 0; i < led->num_fnodes; i++) {
		addr_offset = led->fnode[i].id;
		rc = qpnp_flash_led_write(led,
			FLASH_LED_REG_HDRM_PRGM(led->base + addr_offset),
			led->fnode[i].hdrm_val);
		if (rc < 0)
			return rc;

		val |= 0x1 << led->fnode[i].id;

		if (led->fnode[i].strobe_sel == HW_STROBE) {
			if (led->fnode[i].id == LED3)
				strobe_mask |= LED3_FLASH_ONCE_ONLY_BIT;
			else
				strobe_mask |= LED1N2_FLASH_ONCE_ONLY_BIT;
		}

		if (led->fnode[i].id == LED3 &&
				led->fnode[i].strobe_sel == LPG_STROBE)
			strobe_mask |= LED3_FLASH_ONCE_ONLY_BIT;
		/*
		 * As per the hardware recommendation, to use LED2/LED3 in HW
		 * strobe mode, LED1 should be set to HW strobe mode as well.
		 */
		if (led->fnode[i].strobe_sel == HW_STROBE &&
		      (led->fnode[i].id == LED2 || led->fnode[i].id == LED3)) {
			mask = FLASH_HW_STROBE_MASK;
			addr_offset = led->fnode[LED1].id;
			/*
			 * HW_STROBE: enable, TRIGGER: level,
			 * POLARITY: active high
			 */
			strobe_ctrl = BIT(2) | BIT(0);
			rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_STROBE_CTRL(
				led->base + addr_offset),
				mask, strobe_ctrl);
			if (rc < 0)
				return rc;
		}
	}

	rc = qpnp_flash_led_masked_write(led,
		FLASH_LED_REG_MULTI_STROBE_CTRL(led->base),
		strobe_mask, 0);
	if (rc < 0)
		return rc;

	if (led->fnode[LED3].strobe_sel == LPG_STROBE) {
		rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_LPG_INPUT_CTRL(led->base),
			LPG_INPUT_SEL_BIT, LPG_INPUT_SEL_BIT);
		if (rc < 0)
			return rc;
	}

	rc = qpnp_flash_led_write(led,
				FLASH_LED_REG_HDRM_AUTO_MODE_CTRL(led->base),
				val);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_ISC_DELAY(led->base),
			FLASH_LED_ISC_WARMUP_DELAY_MASK,
			led->pdata->isc_delay);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_WARMUP_DELAY(led->base),
			FLASH_LED_ISC_WARMUP_DELAY_MASK,
			led->pdata->warmup_delay);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_CURRENT_DERATE_EN(led->base),
			FLASH_LED_CURRENT_DERATE_EN_MASK,
			led->pdata->current_derate_en_cfg);
	if (rc < 0)
		return rc;

	val = (led->pdata->otst_ramp_bkup_en << THERMAL_OTST1_RAMP_CTRL_SHIFT);
	mask = THERMAL_OTST1_RAMP_CTRL_MASK;
	if (led->pdata->thermal_derate_slow >= 0) {
		val |= (led->pdata->thermal_derate_slow <<
				THERMAL_DERATE_SLOW_SHIFT);
		mask |= THERMAL_DERATE_SLOW_MASK;
	}

	if (led->pdata->thermal_derate_fast >= 0) {
		val |= led->pdata->thermal_derate_fast;
		mask |= THERMAL_DERATE_FAST_MASK;
	}

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_THERMAL_RMP_DN_RATE(led->base),
			mask, val);
	if (rc < 0)
		return rc;

	if (led->pdata->thermal_debounce >= 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_THERMAL_DEBOUNCE(led->base),
				FLASH_LED_THERMAL_DEBOUNCE_MASK,
				led->pdata->thermal_debounce);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->thermal_hysteresis >= 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_THERMAL_HYSTERESIS(led->base),
				FLASH_LED_THERMAL_HYSTERESIS_MASK,
				led->pdata->thermal_hysteresis);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->thermal_thrsh1 >= 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_THERMAL_THRSH1(led->base),
				FLASH_LED_THERMAL_THRSH_MASK,
				led->pdata->thermal_thrsh1);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->thermal_thrsh2 >= 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_THERMAL_THRSH2(led->base),
				FLASH_LED_THERMAL_THRSH_MASK,
				led->pdata->thermal_thrsh2);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->thermal_thrsh3 >= 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_THERMAL_THRSH3(led->base),
				FLASH_LED_THERMAL_THRSH_MASK,
				led->pdata->thermal_thrsh3);
		if (rc < 0)
			return rc;
	}

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_VPH_DROOP_DEBOUNCE(led->base),
			FLASH_LED_VPH_DROOP_DEBOUNCE_MASK,
			led->pdata->vph_droop_debounce);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_VPH_DROOP_THRESHOLD(led->base),
			FLASH_LED_VPH_DROOP_THRESHOLD_MASK,
			led->pdata->vph_droop_threshold);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_VPH_DROOP_THRESHOLD(led->base),
			FLASH_LED_VPH_DROOP_HYSTERESIS_MASK,
			led->pdata->vph_droop_hysteresis);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_MITIGATION_SEL(led->base),
			FLASH_LED_LMH_MITIGATION_SEL_MASK,
			led->pdata->lmh_mitigation_sel);
	if (rc < 0)
		return rc;

	val = led->pdata->chgr_mitigation_sel
				<< FLASH_LED_CHGR_MITIGATION_SEL_SHIFT;
	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_MITIGATION_SEL(led->base),
			FLASH_LED_CHGR_MITIGATION_SEL_MASK,
			val);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_LMH_LEVEL(led->base),
			FLASH_LED_LMH_LEVEL_MASK,
			led->pdata->lmh_level);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_ILED_GRT_THRSH(led->base),
			FLASH_LED_ILED_GRT_THRSH_MASK,
			led->pdata->iled_thrsh_val);
	if (rc < 0)
		return rc;

	if (led->pdata->led1n2_iclamp_low_ma) {
		val = get_current_reg_code(led->pdata->led1n2_iclamp_low_ma,
						led->fnode[LED1].ires_ua);
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_LED1N2_ICLAMP_LOW(led->base),
				FLASH_LED_CURRENT_MASK, val);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->led1n2_iclamp_mid_ma) {
		val = get_current_reg_code(led->pdata->led1n2_iclamp_mid_ma,
						led->fnode[LED1].ires_ua);
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_LED1N2_ICLAMP_MID(led->base),
				FLASH_LED_CURRENT_MASK, val);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->led3_iclamp_low_ma) {
		val = get_current_reg_code(led->pdata->led3_iclamp_low_ma,
						led->fnode[LED3].ires_ua);
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_LED3_ICLAMP_LOW(led->base),
				FLASH_LED_CURRENT_MASK, val);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->led3_iclamp_mid_ma) {
		val = get_current_reg_code(led->pdata->led3_iclamp_mid_ma,
						led->fnode[LED3].ires_ua);
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_LED3_ICLAMP_MID(led->base),
				FLASH_LED_CURRENT_MASK, val);
		if (rc < 0)
			return rc;
	}

	if (led->pdata->hw_strobe_option > 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_STROBE_CFG(led->base),
				FLASH_LED_STROBE_MASK,
				led->pdata->hw_strobe_option);
		if (rc < 0)
			return rc;
	}

	return 0;
}

static int qpnp_flash_led_hw_strobe_enable(struct flash_node_data *fnode,
						int hw_strobe_option, bool on)
{
	int rc = 0;

	/*
	 * If the LED controlled by this fnode is not GPIO controlled
	 * for the given strobe_option, return.
	 */
	if (hw_strobe_option == FLASH_LED_HW_STROBE_OPTION_1)
		return 0;
	else if (hw_strobe_option == FLASH_LED_HW_STROBE_OPTION_2
						&& fnode->id != LED3)
		return 0;
	else if (hw_strobe_option == FLASH_LED_HW_STROBE_OPTION_3
						&& fnode->id == LED1)
		return 0;

	if (gpio_is_valid(fnode->hw_strobe_gpio)) {
		gpio_set_value(fnode->hw_strobe_gpio, on ? 1 : 0);
	} else if (fnode->strobe_pinctrl && fnode->hw_strobe_state_active &&
					fnode->hw_strobe_state_suspend) {
		rc = pinctrl_select_state(fnode->strobe_pinctrl,
			on ? fnode->hw_strobe_state_active :
			fnode->hw_strobe_state_suspend);
		if (rc < 0) {
			pr_err("failed to change hw strobe pin state\n");
			return rc;
		}
	}

	return rc;
}

static int qpnp_flash_led_regulator_enable(struct qpnp_flash_led *led,
				struct flash_switch_data *snode, bool on)
{
	int rc = 0;

	if (!snode || !snode->vreg)
		return 0;

	if (snode->regulator_on == on)
		return 0;

	if (on)
		rc = regulator_enable(snode->vreg);
	else
		rc = regulator_disable(snode->vreg);

	if (rc < 0) {
		pr_err("regulator_%s failed, rc=%d\n",
			on ? "enable" : "disable", rc);
		return rc;
	}

	snode->regulator_on = on ? true : false;
	return 0;
}

static int get_property_from_fg(struct qpnp_flash_led *led,
		enum power_supply_property prop, int *val)
{
	int rc;
	union power_supply_propval pval = {0, };

	if (!led->bms_psy) {
		pr_err("no bms psy found\n");
		return -EINVAL;
	}

	rc = power_supply_get_property(led->bms_psy, prop, &pval);
	if (rc) {
		pr_err("bms psy doesn't support reading prop %d rc = %d\n",
			prop, rc);
		return rc;
	}

	*val = pval.intval;
	return rc;
}

#define VOLTAGE_HDRM_DEFAULT_MV		350
#define BHARGER_VOLTAGE_HDRM_DEFAULT_MV	400
#define BHARGER_HEADROOM_OFFSET_MV	50
static int qpnp_flash_led_get_voltage_headroom(struct qpnp_flash_led *led)
{
	int i, voltage_hdrm_mv = 0, voltage_hdrm_max = 0;
	u8 pmic_subtype = led->pdata->pmic_rev_id->pmic_subtype;

	for (i = 0; i < led->num_fnodes; i++) {
		if (led->fnode[i].led_on) {
			if (led->fnode[i].id < 2) {
				if (led->fnode[i].current_ma < 750)
					voltage_hdrm_mv = 125;
				else if (led->fnode[i].current_ma < 1000)
					voltage_hdrm_mv = 175;
				else if (led->fnode[i].current_ma < 1250)
					voltage_hdrm_mv = 250;
				else
					voltage_hdrm_mv = 350;
			} else {
				if (led->fnode[i].current_ma < 375)
					voltage_hdrm_mv = 125;
				else if (led->fnode[i].current_ma < 500)
					voltage_hdrm_mv = 175;
				else if (led->fnode[i].current_ma < 625)
					voltage_hdrm_mv = 250;
				else
					voltage_hdrm_mv = 350;
			}

			if (pmic_subtype == PMI632_SUBTYPE)
				voltage_hdrm_mv += BHARGER_HEADROOM_OFFSET_MV;

			voltage_hdrm_max = max(voltage_hdrm_max,
						voltage_hdrm_mv);
		}
	}

	if (!voltage_hdrm_max)
		return (pmic_subtype == PMI632_SUBTYPE) ?
					BHARGER_VOLTAGE_HDRM_DEFAULT_MV :
						VOLTAGE_HDRM_DEFAULT_MV;

	return voltage_hdrm_max;
}

#define UCONV			1000000LL
#define MCONV			1000LL
#define FLASH_VDIP_MARGIN	50000
#define BOB_EFFICIENCY		900LL
#define VIN_FLASH_MIN_UV	3300000LL
static int qpnp_flash_led_calc_max_current(struct qpnp_flash_led *led,
					      int *max_current)
{
	int ocv_uv, ibat_now, voltage_hdrm_mv, rc;
	int rbatt_uohm = 0;
	int64_t ibat_flash_ua, avail_flash_ua, avail_flash_power_fw;
	int64_t ibat_safe_ua, vin_flash_uv, vph_flash_uv, vph_flash_vdip;

	/* RESISTANCE = esr_uohm + rslow_uohm */
	rc = get_property_from_fg(led, POWER_SUPPLY_PROP_RESISTANCE,
			&rbatt_uohm);
	if (rc < 0) {
		pr_err("bms psy does not support resistance, rc=%d\n", rc);
		return rc;
	}

	/* If no battery is connected, return max possible flash current */
	if (!rbatt_uohm) {
		*max_current = FLASH_LED_MAX_TOTAL_CURRENT_MA;
		return 0;
	}

	rc = get_property_from_fg(led, POWER_SUPPLY_PROP_VOLTAGE_OCV, &ocv_uv);
	if (rc < 0) {
		pr_err("bms psy does not support OCV, rc=%d\n", rc);
		return rc;
	}

	rc = get_property_from_fg(led, POWER_SUPPLY_PROP_CURRENT_NOW,
			&ibat_now);
	if (rc < 0) {
		pr_err("bms psy does not support current, rc=%d\n", rc);
		return rc;
	}

	rbatt_uohm += led->pdata->rpara_uohm;
	voltage_hdrm_mv = qpnp_flash_led_get_voltage_headroom(led);
	vph_flash_vdip =
		VPH_DROOP_THRESH_VAL_TO_UV(led->pdata->vph_droop_threshold)
							+ FLASH_VDIP_MARGIN;

	/* Check if LMH_MITIGATION needs to be triggered */
	if (!led->trigger_lmh && (ocv_uv < led->pdata->lmh_ocv_threshold_uv ||
			rbatt_uohm > led->pdata->lmh_rbatt_threshold_uohm)) {
		led->trigger_lmh = true;
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MITIGATION_SW(led->base),
				FLASH_LED_LMH_MITIGATION_EN_MASK,
				FLASH_LED_LMH_MITIGATION_ENABLE);
		if (rc < 0) {
			pr_err("trigger lmh mitigation failed, rc=%d\n", rc);
			return rc;
		}

		/* Wait for LMH mitigation to take effect */
		udelay(100);

		return qpnp_flash_led_calc_max_current(led, max_current);
	}

	/*
	 * Calculate the maximum current that can pulled out of the battery
	 * before the battery voltage dips below a safe threshold.
	 */
	ibat_safe_ua = div_s64((ocv_uv - vph_flash_vdip) * UCONV,
				rbatt_uohm);

	if (ibat_safe_ua <= led->pdata->ibatt_ocp_threshold_ua) {
		/*
		 * If the calculated current is below the OCP threshold, then
		 * use it as the possible flash current.
		 */
		ibat_flash_ua = ibat_safe_ua - ibat_now;
		vph_flash_uv = vph_flash_vdip;
	} else {
		/*
		 * If the calculated current is above the OCP threshold, then
		 * use the ocp threshold instead.
		 *
		 * Any higher current will be tripping the battery OCP.
		 */
		ibat_flash_ua = led->pdata->ibatt_ocp_threshold_ua - ibat_now;
		vph_flash_uv = ocv_uv - div64_s64((int64_t)rbatt_uohm
				* led->pdata->ibatt_ocp_threshold_ua, UCONV);
	}
	/* Calculate the input voltage of the flash module. */
	vin_flash_uv = max((led->pdata->vled_max_uv +
				(voltage_hdrm_mv * MCONV)), VIN_FLASH_MIN_UV);
	/* Calculate the available power for the flash module. */
	avail_flash_power_fw = BOB_EFFICIENCY * vph_flash_uv * ibat_flash_ua;
	/*
	 * Calculate the available amount of current the flash module can draw
	 * before collapsing the battery. (available power/ flash input voltage)
	 */
	avail_flash_ua = div64_s64(avail_flash_power_fw, vin_flash_uv * MCONV);
	pr_debug("avail_iflash=%lld, ocv=%d, ibat=%d, rbatt=%d, trigger_lmh=%d\n",
		avail_flash_ua, ocv_uv, ibat_now, rbatt_uohm, led->trigger_lmh);
	*max_current = min(FLASH_LED_MAX_TOTAL_CURRENT_MA,
			(int)(div64_s64(avail_flash_ua, MCONV)));
	return 0;
}

static int is_main_psy_available(struct qpnp_flash_led *led)
{
	if (!led->main_psy) {
		led->main_psy = power_supply_get_by_name("main");
		if (!led->main_psy) {
			pr_err_ratelimited("Couldn't get main_psy\n");
			return -ENODEV;
		}
	}

	return 0;
}

static int is_usb_psy_available(struct qpnp_flash_led *led)
{
	if (!led->usb_psy) {
		led->usb_psy = power_supply_get_by_name("usb");
		if (!led->usb_psy) {
			pr_err_ratelimited("Couldn't get usb_psy\n");
			return -ENODEV;
		}
	}

	return 0;
}

#define CHGBST_EFFICIENCY		800LL
#define CHGBST_FLASH_VDIP_MARGIN	10000
#define VIN_FLASH_UV			5000000
#define BHARGER_FLASH_LED_MAX_TOTAL_CURRENT_MA		1500
#define BHARGER_FLASH_LED_WITH_OTG_MAX_TOTAL_CURRENT_MA	1100
static int qpnp_flash_led_calc_bharger_max_current(struct qpnp_flash_led *led,
						    int *max_current)
{
	union power_supply_propval pval = {0, };
	int ocv_uv, ibat_now, voltage_hdrm_mv, flash_led_max_total_curr_ma, rc;
	int rbatt_uohm = 0, usb_present, otg_enable;
	int64_t ibat_flash_ua, avail_flash_ua, avail_flash_power_fw;
	int64_t ibat_safe_ua, vin_flash_uv, vph_flash_uv, vph_flash_vdip;
	int64_t bst_pwm_ovrhd_uv;

	rc = is_usb_psy_available(led);
	if (rc < 0)
		return rc;

	rc = power_supply_get_property(led->usb_psy, POWER_SUPPLY_PROP_SCOPE,
					&pval);
	if (rc < 0) {
		pr_err("usb psy does not support usb present, rc=%d\n", rc);
		return rc;
	}
	otg_enable = pval.intval;

	/* RESISTANCE = esr_uohm + rslow_uohm */
	rc = get_property_from_fg(led, POWER_SUPPLY_PROP_RESISTANCE,
			&rbatt_uohm);
	if (rc < 0) {
		pr_err("bms psy does not support resistance, rc=%d\n", rc);
		return rc;
	}

	/* If no battery is connected, return max possible flash current */
	if (!rbatt_uohm) {
		*max_current = (otg_enable == POWER_SUPPLY_SCOPE_SYSTEM) ?
			       BHARGER_FLASH_LED_WITH_OTG_MAX_TOTAL_CURRENT_MA :
			       BHARGER_FLASH_LED_MAX_TOTAL_CURRENT_MA;
		return 0;
	}

	rc = get_property_from_fg(led, POWER_SUPPLY_PROP_VOLTAGE_OCV, &ocv_uv);
	if (rc < 0) {
		pr_err("bms psy does not support OCV, rc=%d\n", rc);
		return rc;
	}

	rc = get_property_from_fg(led, POWER_SUPPLY_PROP_CURRENT_NOW,
			&ibat_now);
	if (rc < 0) {
		pr_err("bms psy does not support current, rc=%d\n", rc);
		return rc;
	}

	bst_pwm_ovrhd_uv = led->pdata->bst_pwm_ovrhd_uv;

	rc = power_supply_get_property(led->usb_psy, POWER_SUPPLY_PROP_PRESENT,
							&pval);
	if (rc < 0) {
		pr_err("usb psy does not support usb present, rc=%d\n", rc);
		return rc;
	}
	usb_present = pval.intval;

	rbatt_uohm += led->pdata->rpara_uohm;
	voltage_hdrm_mv = qpnp_flash_led_get_voltage_headroom(led);
	vph_flash_vdip =
		VPH_DROOP_THRESH_VAL_TO_UV(led->pdata->vph_droop_threshold)
						+ CHGBST_FLASH_VDIP_MARGIN;

	/* Check if LMH_MITIGATION needs to be triggered */
	if (!led->trigger_lmh && (ocv_uv < led->pdata->lmh_ocv_threshold_uv ||
			rbatt_uohm > led->pdata->lmh_rbatt_threshold_uohm)) {
		led->trigger_lmh = true;
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MITIGATION_SW(led->base),
				FLASH_LED_LMH_MITIGATION_EN_MASK,
				FLASH_LED_LMH_MITIGATION_ENABLE);
		if (rc < 0) {
			pr_err("trigger lmh mitigation failed, rc=%d\n", rc);
			return rc;
		}

		/* Wait for LMH mitigation to take effect */
		udelay(100);

		return qpnp_flash_led_calc_bharger_max_current(led,
							       max_current);
	}

	/*
	 * Calculate the maximum current that can pulled out of the battery
	 * before the battery voltage dips below a safe threshold.
	 */
	ibat_safe_ua = div_s64((ocv_uv - vph_flash_vdip) * UCONV,
				rbatt_uohm);

	if (ibat_safe_ua <= led->pdata->ibatt_ocp_threshold_ua) {
		/*
		 * If the calculated current is below the OCP threshold, then
		 * use it as the possible flash current.
		 */
		ibat_flash_ua = ibat_safe_ua - ibat_now;
		vph_flash_uv = vph_flash_vdip;
	} else {
		/*
		 * If the calculated current is above the OCP threshold, then
		 * use the ocp threshold instead.
		 *
		 * Any higher current will be tripping the battery OCP.
		 */
		ibat_flash_ua = led->pdata->ibatt_ocp_threshold_ua - ibat_now;
		vph_flash_uv = ocv_uv - div64_s64((int64_t)rbatt_uohm
				* led->pdata->ibatt_ocp_threshold_ua, UCONV);
	}

	/* when USB is present or OTG is enabled, VIN_FLASH is always at 5V */
	if (usb_present || (otg_enable == POWER_SUPPLY_SCOPE_SYSTEM))
		vin_flash_uv = VIN_FLASH_UV;
	else
		/* Calculate the input voltage of the flash module. */
		vin_flash_uv = max((led->pdata->vled_max_uv +
				   (voltage_hdrm_mv * MCONV)),
				    vph_flash_uv + bst_pwm_ovrhd_uv);

	/* Calculate the available power for the flash module. */
	avail_flash_power_fw = CHGBST_EFFICIENCY * vph_flash_uv * ibat_flash_ua;
	/*
	 * Calculate the available amount of current the flash module can draw
	 * before collapsing the battery. (available power/ flash input voltage)
	 */
	avail_flash_ua = div64_s64(avail_flash_power_fw, vin_flash_uv * MCONV);
	flash_led_max_total_curr_ma = otg_enable ?
			       BHARGER_FLASH_LED_WITH_OTG_MAX_TOTAL_CURRENT_MA :
			       BHARGER_FLASH_LED_MAX_TOTAL_CURRENT_MA;
	*max_current = min(flash_led_max_total_curr_ma,
			(int)(div64_s64(avail_flash_ua, MCONV)));

	pr_debug("avail_iflash=%lld, ocv=%d, ibat=%d, rbatt=%d, trigger_lmh=%d max_current=%lld usb_present=%d otg_enable=%d\n",
		avail_flash_ua, ocv_uv, ibat_now, rbatt_uohm, led->trigger_lmh,
		(*max_current * MCONV), usb_present, otg_enable);
	return 0;
}


static int qpnp_flash_led_calc_thermal_current_lim(struct qpnp_flash_led *led,
						   int *thermal_current_lim)
{
	int rc;
	u8 thermal_thrsh1, thermal_thrsh2, thermal_thrsh3, otst_status;

	/* Store THERMAL_THRSHx register values */
	rc = qpnp_flash_led_masked_read(led,
			FLASH_LED_REG_THERMAL_THRSH1(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			&thermal_thrsh1);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_read(led,
			FLASH_LED_REG_THERMAL_THRSH2(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			&thermal_thrsh2);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_read(led,
			FLASH_LED_REG_THERMAL_THRSH3(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			&thermal_thrsh3);
	if (rc < 0)
		return rc;

	/* Lower THERMAL_THRSHx thresholds to minimum */
	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_THERMAL_THRSH1(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			FLASH_LED_THERMAL_THRSH_MIN);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_THERMAL_THRSH2(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			FLASH_LED_THERMAL_THRSH_MIN);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_THERMAL_THRSH3(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			FLASH_LED_THERMAL_THRSH_MIN);
	if (rc < 0)
		return rc;

	/* Check THERMAL_OTST status */
	rc = qpnp_flash_led_read(led,
			FLASH_LED_REG_LED_STATUS2(led->base),
			&otst_status);
	if (rc < 0)
		return rc;

	/* Look up current limit based on THERMAL_OTST status */
	if (otst_status)
		*thermal_current_lim =
			led->pdata->thermal_derate_current[otst_status >> 1];

	/* Restore THERMAL_THRESHx registers to original values */
	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_THERMAL_THRSH1(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			thermal_thrsh1);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_THERMAL_THRSH2(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			thermal_thrsh2);
	if (rc < 0)
		return rc;

	rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_THERMAL_THRSH3(led->base),
			FLASH_LED_THERMAL_THRSH_MASK,
			thermal_thrsh3);
	if (rc < 0)
		return rc;

	return 0;
}

static int qpnp_flash_led_get_max_avail_current(struct qpnp_flash_led *led,
						int *max_avail_current)
{
	int thermal_current_lim = 0, rc;
	u8 pmic_subtype = led->pdata->pmic_rev_id->pmic_subtype;

	led->trigger_lmh = false;

	if (pmic_subtype == PMI632_SUBTYPE)
		rc = qpnp_flash_led_calc_bharger_max_current(led,
							max_avail_current);
	else
		rc = qpnp_flash_led_calc_max_current(led, max_avail_current);

	if (rc < 0) {
		pr_err("Couldn't calculate max_avail_current, rc=%d\n", rc);
		return rc;
	}

	if (led->pdata->thermal_derate_en) {
		rc = qpnp_flash_led_calc_thermal_current_lim(led,
			&thermal_current_lim);
		if (rc < 0) {
			pr_err("Couldn't calculate thermal_current_lim, rc=%d\n",
				rc);
			return rc;
		}
	}

	if (thermal_current_lim)
		*max_avail_current = min(*max_avail_current,
					thermal_current_lim);

	return 0;
}

static void qpnp_flash_led_aggregate_max_current(struct flash_node_data *fnode)
{
	struct qpnp_flash_led *led = dev_get_drvdata(&fnode->pdev->dev);

	if (fnode->current_ma)
		led->total_current_ma += fnode->current_ma
						- fnode->prev_current_ma;
	else
		led->total_current_ma -= fnode->prev_current_ma;

	fnode->prev_current_ma = fnode->current_ma;
}

static void qpnp_flash_led_node_set(struct flash_node_data *fnode, int value)
{
	int i = 0;
	int prgm_current_ma = value;
	int min_ma = fnode->ires_ua / 1000;
	struct qpnp_flash_led *led = dev_get_drvdata(&fnode->pdev->dev);
	u8 pmic_subtype = led->pdata->pmic_rev_id->pmic_subtype;

	if (value <= 0)
		prgm_current_ma = 0;
	else if (value < min_ma)
		prgm_current_ma = min_ma;

	fnode->ires_idx = fnode->default_ires_idx;
	fnode->ires_ua = fnode->default_ires_ua;

	prgm_current_ma = min(prgm_current_ma, fnode->max_current);
	if (prgm_current_ma > max_ires_curr_ma_table[fnode->ires_idx]) {
		/* find the matching ires */
		for (i = MAX_IRES_LEVELS - 1; i >= 0; i--) {
			if (prgm_current_ma <= max_ires_curr_ma_table[i]) {
				fnode->ires_idx = i;
				fnode->ires_ua = FLASH_LED_IRES_MIN_UA +
				      (FLASH_LED_IRES_BASE - fnode->ires_idx) *
				      FLASH_LED_IRES_DIVISOR;
				break;
			}
		}
	}
	fnode->current_ma = prgm_current_ma;
	fnode->cdev.brightness = prgm_current_ma;
	fnode->current_reg_val = get_current_reg_code(prgm_current_ma,
					fnode->ires_ua);
	if (prgm_current_ma)
		fnode->led_on = true;

	if (pmic_subtype != PMI632_SUBTYPE &&
	       led->pdata->chgr_mitigation_sel == FLASH_SW_CHARGER_MITIGATION) {
		qpnp_flash_led_aggregate_max_current(fnode);
		led->trigger_chgr = false;
		if (led->total_current_ma >= 1000)
			led->trigger_chgr = true;
	}
}

static int qpnp_flash_led_switch_disable(struct flash_switch_data *snode)
{
	struct qpnp_flash_led *led = dev_get_drvdata(&snode->pdev->dev);
	int i, rc, addr_offset;

	rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_EN_LED_CTRL(led->base),
				snode->led_mask, FLASH_LED_DISABLE);
	if (rc < 0)
		return rc;

	if (led->trigger_lmh) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MITIGATION_SW(led->base),
				FLASH_LED_LMH_MITIGATION_EN_MASK,
				FLASH_LED_LMH_MITIGATION_DISABLE);
		if (rc < 0) {
			pr_err("disable lmh mitigation failed, rc=%d\n", rc);
			return rc;
		}
	}

	if (led->pdata->chgr_mitigation_sel && !led->trigger_chgr) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MITIGATION_SW(led->base),
				FLASH_LED_CHGR_MITIGATION_EN_MASK,
				FLASH_LED_CHGR_MITIGATION_DISABLE);
		if (rc < 0) {
			pr_err("disable chgr mitigation failed, rc=%d\n", rc);
			return rc;
		}
	}

	led->enable--;
	if (led->enable == 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MOD_CTRL(led->base),
				FLASH_LED_MOD_CTRL_MASK, FLASH_LED_DISABLE);
		if (rc < 0)
			return rc;
	}

	for (i = 0; i < led->num_fnodes; i++) {
		if (!led->fnode[i].led_on ||
				!(snode->led_mask & BIT(led->fnode[i].id)))
			continue;

		addr_offset = led->fnode[i].id;
		rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_TGR_CURRENT(led->base + addr_offset),
			FLASH_LED_CURRENT_MASK, 0);
		if (rc < 0)
			return rc;

		led->fnode[i].led_on = false;

		if (led->fnode[i].strobe_sel == HW_STROBE) {
			rc = qpnp_flash_led_hw_strobe_enable(&led->fnode[i],
					led->pdata->hw_strobe_option, false);
			if (rc < 0) {
				pr_err("Unable to disable hw strobe, rc=%d\n",
					rc);
				return rc;
			}
		}
	}

	if (snode->led_en_pinctrl) {
		pr_debug("Selecting suspend state for %s\n", snode->cdev.name);
		rc = pinctrl_select_state(snode->led_en_pinctrl,
				snode->gpio_state_suspend);
		if (rc < 0) {
			pr_err("failed to select pinctrl suspend state rc=%d\n",
				rc);
			return rc;
		}
	}

	snode->enabled = false;
	return 0;
}

static int qpnp_flash_led_symmetry_config(struct flash_switch_data *snode)
{
	struct qpnp_flash_led *led = dev_get_drvdata(&snode->pdev->dev);
	int i, total_curr_ma = 0, num_leds = 0, prgm_current_ma;
	enum flash_led_type type = FLASH_LED_TYPE_UNKNOWN;

	for (i = 0; i < led->num_fnodes; i++) {
		if (snode->led_mask & BIT(led->fnode[i].id)) {
			if (led->fnode[i].type == FLASH_LED_TYPE_FLASH &&
				led->fnode[i].led_on)
				type = FLASH_LED_TYPE_FLASH;

			if (led->fnode[i].type == FLASH_LED_TYPE_TORCH &&
				led->fnode[i].led_on)
				type = FLASH_LED_TYPE_TORCH;
		}
	}

	if (type == FLASH_LED_TYPE_UNKNOWN) {
		pr_err("Incorrect type possibly because of no active LEDs\n");
		return -EINVAL;
	}

	for (i = 0; i < led->num_fnodes; i++) {
		if ((snode->led_mask & BIT(led->fnode[i].id)) &&
			(led->fnode[i].type == type)) {
			total_curr_ma += led->fnode[i].current_ma;
			num_leds++;
		}
	}

	if (num_leds > 0 && total_curr_ma > 0) {
		prgm_current_ma = total_curr_ma / num_leds;
	} else {
		pr_err("Incorrect configuration, num_leds: %d total_curr_ma: %d\n",
			num_leds, total_curr_ma);
		return -EINVAL;
	}

	if (prgm_current_ma == 0) {
		pr_warn("prgm_curr_ma cannot be 0\n");
		return 0;
	}

	pr_debug("num_leds: %d total: %d prgm_curr_ma: %d\n", num_leds,
		total_curr_ma, prgm_current_ma);

	for (i = 0; i < led->num_fnodes; i++) {
		if (snode->led_mask & BIT(led->fnode[i].id) &&
			led->fnode[i].current_ma != prgm_current_ma &&
			led->fnode[i].type == type) {
			qpnp_flash_led_node_set(&led->fnode[i],
				prgm_current_ma);
			pr_debug("%s LED %d current: %d code: %d ires_ua: %d\n",
				(type == FLASH_LED_TYPE_FLASH) ?
					"flash" : "torch",
				led->fnode[i].id, prgm_current_ma,
				led->fnode[i].current_reg_val,
				led->fnode[i].ires_ua);
		}
	}

	return 0;
}

#define FLASH_LED_MODULE_EN_TIME_MS	300
static int qpnp_flash_poll_vreg_ok(struct qpnp_flash_led *led)
{
	int rc, i;
	union power_supply_propval pval = {0, };

	rc = is_main_psy_available(led);
	if (rc < 0)
		return rc;

	for (i = 0; i < 60; i++) {
		/* wait for the flash vreg_ok to be set */
		mdelay(5);

		rc = power_supply_get_property(led->main_psy,
					POWER_SUPPLY_PROP_FLASH_TRIGGER, &pval);
		if (rc < 0) {
			pr_err("main psy doesn't support reading prop %d rc = %d\n",
				POWER_SUPPLY_PROP_FLASH_TRIGGER, rc);
			return rc;
		}

		if (pval.intval > 0) {
			pr_debug("Flash trigger set\n");
			break;
		}

		if (pval.intval < 0) {
			pr_err("Error during flash trigger %d\n", pval.intval);
			return pval.intval;
		}
	}

	if (!pval.intval) {
		pr_err("Failed to enable the module\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int qpnp_flash_led_switch_set(struct flash_switch_data *snode, bool on)
{
	struct qpnp_flash_led *led = dev_get_drvdata(&snode->pdev->dev);
	u8 pmic_subtype = led->pdata->pmic_rev_id->pmic_subtype;
	int rc, i, addr_offset;
	u8 val, mask;

	if (snode->enabled == on) {
		pr_debug("Switch node is already %s!\n",
			on ? "enabled" : "disabled");
		return 0;
	}

	if (!on) {
		rc = qpnp_flash_led_switch_disable(snode);
		return rc;
	}

	/* Iterate over all active leds for this switch node */
	if (snode->symmetry_en) {
		rc = qpnp_flash_led_symmetry_config(snode);
		if (rc < 0) {
			pr_err("Failed to configure current symmetrically, rc=%d\n",
				rc);
			return rc;
		}
	}

	val = 0;
	for (i = 0; i < led->num_fnodes; i++)
		if (led->fnode[i].led_on &&
				snode->led_mask & BIT(led->fnode[i].id))
			val |= led->fnode[i].ires_idx << (led->fnode[i].id * 2);

	rc = qpnp_flash_led_masked_write(led, FLASH_LED_REG_IRES(led->base),
						FLASH_LED_CURRENT_MASK, val);
	if (rc < 0)
		return rc;

	val = 0;
	for (i = 0; i < led->num_fnodes; i++) {
		if (!led->fnode[i].led_on ||
				!(snode->led_mask & BIT(led->fnode[i].id)))
			continue;

		addr_offset = led->fnode[i].id;
		if (led->fnode[i].strobe_sel == SW_STROBE)
			mask = FLASH_LED_HW_SW_STROBE_SEL_BIT;
		else
			mask = FLASH_HW_STROBE_MASK;
		rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_STROBE_CTRL(led->base + addr_offset),
			mask, led->fnode[i].strobe_ctrl);
		if (rc < 0)
			return rc;

		rc = qpnp_flash_led_masked_write(led,
			FLASH_LED_REG_TGR_CURRENT(led->base + addr_offset),
			FLASH_LED_CURRENT_MASK, led->fnode[i].current_reg_val);
		if (rc < 0)
			return rc;

		rc = qpnp_flash_led_write(led,
			FLASH_LED_REG_SAFETY_TMR(led->base + addr_offset),
			led->fnode[i].duration);
		if (rc < 0)
			return rc;

		val |= FLASH_LED_ENABLE << led->fnode[i].id;

		if (led->fnode[i].strobe_sel == HW_STROBE) {
			rc = qpnp_flash_led_hw_strobe_enable(&led->fnode[i],
					led->pdata->hw_strobe_option, true);
			if (rc < 0) {
				pr_err("Unable to enable hw strobe rc=%d\n",
					rc);
				return rc;
			}
		}
	}

	if (snode->led_en_pinctrl) {
		pr_debug("Selecting active state for %s\n", snode->cdev.name);
		rc = pinctrl_select_state(snode->led_en_pinctrl,
				snode->gpio_state_active);
		if (rc < 0) {
			pr_err("failed to select pinctrl active state rc=%d\n",
				rc);
			return rc;
		}
	}

	if (led->enable == 0) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MOD_CTRL(led->base),
				FLASH_LED_MOD_CTRL_MASK, FLASH_LED_MOD_ENABLE);
		if (rc < 0)
			return rc;

		if (pmic_subtype == PMI632_SUBTYPE) {
			rc = qpnp_flash_poll_vreg_ok(led);
			if (rc < 0) {
				/* Disable the module */
				qpnp_flash_led_masked_write(led,
					FLASH_LED_REG_MOD_CTRL(led->base),
					FLASH_LED_MOD_CTRL_MASK,
					FLASH_LED_DISABLE);

				return rc;
			}
		}
	}
	led->enable++;

	if (led->trigger_lmh) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MITIGATION_SW(led->base),
				FLASH_LED_LMH_MITIGATION_EN_MASK,
				FLASH_LED_LMH_MITIGATION_ENABLE);
		if (rc < 0) {
			pr_err("trigger lmh mitigation failed, rc=%d\n", rc);
			return rc;
		}
		/* Wait for LMH mitigation to take effect */
		udelay(500);
	}

	if (led->pdata->chgr_mitigation_sel && led->trigger_chgr) {
		rc = qpnp_flash_led_masked_write(led,
				FLASH_LED_REG_MITIGATION_SW(led->base),
				FLASH_LED_CHGR_MITIGATION_EN_MASK,
				FLASH_LED_CHGR_MITIGATION_ENABLE);
		if (rc < 0) {
			pr_err("trigger chgr mitigation failed, rc=%d\n", rc);
			return rc;
		}
	}

	rc = qpnp_flash_led_masked_write(led,
					FLASH_LED_EN_LED_CTRL(led->base),
					snode->led_mask, val);
	if (rc < 0)
		return rc;

	snode->enabled = true;
	return 0;
}

static int qpnp_flash_led_regulator_control(struct led_classdev *led_cdev,
					int options, int *max_current)
{
	int rc;
	u8 pmic_subtype;
	struct flash_switch_data *snode;
	struct qpnp_flash_led *led;
	union power_supply_propval ret = {0, };

	snode = container_of(led_cdev, struct flash_switch_data, cdev);
	led = dev_get_drvdata(&snode->pdev->dev);
	pmic_subtype = led->pdata->pmic_rev_id->pmic_subtype;

	if (pmic_subtype == PMI632_SUBTYPE) {
		rc = is_main_psy_available(led);
		if (rc < 0)
			return rc;

		rc = is_usb_psy_available(led);
		if (rc < 0)
			return rc;
	}

	if (!(options & FLASH_LED_PREPARE_OPTIONS_MASK)) {
		pr_err("Invalid options %d\n", options);
		return -EINVAL;
	}

	if (options & ENABLE_REGULATOR) {
		if (pmic_subtype == PMI632_SUBTYPE) {
			ret.intval = 1;
			rc = power_supply_set_property(led->main_psy,
					 POWER_SUPPLY_PROP_FLASH_ACTIVE,
					 &ret);
			if (rc < 0) {
				pr_err("Failed to set FLASH_ACTIVE on charger rc=%d\n",
									rc);
				return rc;
			}
			pr_debug("FLASH_ACTIVE = 1\n");
		} else {
			rc = qpnp_flash_led_regulator_enable(led, snode, true);
			if (rc < 0) {
				pr_err("enable regulator failed, rc=%d\n", rc);
				return rc;
			}
		}
	}

	if (options & DISABLE_REGULATOR) {
		if (pmic_subtype == PMI632_SUBTYPE) {
			ret.intval = 0;
			rc = power_supply_set_property(led->main_psy,
					POWER_SUPPLY_PROP_FLASH_ACTIVE,
					&ret);
			if (rc < 0) {
				pr_err("Failed to set FLASH_ACTIVE on charger rc=%d\n",
									rc);
				return rc;
			}
			pr_debug("FLASH_ACTIVE = 0\n");
		} else {
			rc = qpnp_flash_led_regulator_enable(led, snode, false);
			if (rc < 0) {
				pr_err("disable regulator failed, rc=%d\n", rc);
				return rc;
			}
		}
	}

	if (options & QUERY_MAX_CURRENT) {
		rc = qpnp_flash_led_get_max_avail_current(led, max_current);
		if (rc < 0) {
			pr_err("query max current failed, rc=%d\n", rc);
			return rc;
		}
	}

	return 0;
}

static int qpnp_flash_led_prepare_v2(struct led_trigger *trig, int options,
					int *max_current)
{
	struct led_classdev *led_cdev;
	int rc;

	if (!trig) {
		pr_err("Invalid led_trigger provided\n");
		return -EINVAL;
	}

	led_cdev = trigger_to_lcdev(trig);
	if (!led_cdev) {
		pr_err("Invalid led_cdev in trigger %s\n", trig->name);
		return -EINVAL;
	}

	rc = qpnp_flash_led_regulator_control(led_cdev, options, max_current);

	return rc;
}

static void qpnp_flash_led_brightness_set(struct led_classdev *led_cdev,
						enum led_brightness value)
{
	struct flash_node_data *fnode = NULL;
	struct flash_switch_data *snode = NULL;
	struct qpnp_flash_led *led = NULL;
	int rc;

	/*
	 * strncmp() must be used here since a prefix comparison is required
	 * in order to support names like led:switch_0 and led:flash_1.
	 */
	if (!strncmp(led_cdev->name, "led:switch", strlen("led:switch"))) {
		snode = container_of(led_cdev, struct flash_switch_data, cdev);
		led = dev_get_drvdata(&snode->pdev->dev);
	} else if (!strncmp(led_cdev->name, "led:flash", strlen("led:flash")) ||
			!strncmp(led_cdev->name, "led:torch",
						strlen("led:torch"))) {
		fnode = container_of(led_cdev, struct flash_node_data, cdev);
		led = dev_get_drvdata(&fnode->pdev->dev);
	}

	if (!led) {
		pr_err("Failed to get flash driver data\n");
		return;
	}

	spin_lock(&led->lock);
	if (snode) {
		rc = qpnp_flash_led_switch_set(snode, value > 0);
		if (rc < 0)
			pr_err("Failed to set flash LED switch rc=%d\n", rc);
	} else if (fnode) {
		qpnp_flash_led_node_set(fnode, value);
	}

	spin_unlock(&led->lock);
}

static ssize_t qpnp_flash_led_prepare_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int rc, options, max_current;
	u32 val;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	rc = kstrtouint(buf, 0, &val);
	if (rc < 0)
		return rc;

	if (val != 0 && val != 1)
		return count;

	options = val ? ENABLE_REGULATOR : DISABLE_REGULATOR;

	rc = qpnp_flash_led_regulator_control(led_cdev, options, &max_current);
	if (rc < 0)
		return rc;

	return count;
}

/* sysfs show function for flash_max_current */
static ssize_t qpnp_flash_led_max_current_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc, max_current = 0;
	struct flash_switch_data *snode;
	struct qpnp_flash_led *led;
	struct led_classdev *led_cdev = dev_get_drvdata(dev);

	snode = container_of(led_cdev, struct flash_switch_data, cdev);
	led = dev_get_drvdata(&snode->pdev->dev);

	rc = qpnp_flash_led_get_max_avail_current(led, &max_current);
	if (rc < 0)
		pr_err("query max current failed, rc=%d\n", rc);

	return snprintf(buf, PAGE_SIZE, "%d\n", max_current);
}

/* sysfs attributes exported by flash_led */
static struct device_attribute qpnp_flash_led_attrs[] = {
	__ATTR(max_current, 0664, qpnp_flash_led_max_current_show, NULL),
	__ATTR(enable, 0664, NULL, qpnp_flash_led_prepare_store),
};

static int flash_led_psy_notifier_call(struct notifier_block *nb,
		unsigned long ev, void *v)
{
	struct power_supply *psy = v;
	struct qpnp_flash_led *led =
			container_of(nb, struct qpnp_flash_led, nb);

	if (ev != PSY_EVENT_PROP_CHANGED)
		return NOTIFY_OK;

	if (!strcmp(psy->desc->name, "bms")) {
		led->bms_psy = power_supply_get_by_name("bms");
		if (!led->bms_psy)
			pr_err("Failed to get bms power_supply\n");
		else
			power_supply_unreg_notifier(&led->nb);
	}

	return NOTIFY_OK;
}

static int flash_led_psy_register_notifier(struct qpnp_flash_led *led)
{
	int rc;

	led->nb.notifier_call = flash_led_psy_notifier_call;
	rc = power_supply_reg_notifier(&led->nb);
	if (rc < 0) {
		pr_err("Couldn't register psy notifier, rc = %d\n", rc);
		return rc;
	}

	return 0;
}

/* irq handler */
static irqreturn_t qpnp_flash_led_irq_handler(int irq, void *_led)
{
	struct qpnp_flash_led *led = _led;
	enum flash_led_irq_type irq_type = INVALID_IRQ;
	int rc;
	u8 irq_status, led_status1, led_status2;

	pr_debug("irq received, irq=%d\n", irq);

	rc = qpnp_flash_led_read(led,
			FLASH_LED_REG_INT_RT_STS(led->base), &irq_status);
	if (rc < 0) {
		pr_err("Failed to read interrupt status reg, rc=%d\n", rc);
		goto exit;
	}

	if (irq == led->pdata->all_ramp_up_done_irq)
		irq_type = ALL_RAMP_UP_DONE_IRQ;
	else if (irq == led->pdata->all_ramp_down_done_irq)
		irq_type = ALL_RAMP_DOWN_DONE_IRQ;
	else if (irq == led->pdata->led_fault_irq)
		irq_type = LED_FAULT_IRQ;

	if (irq_type == ALL_RAMP_UP_DONE_IRQ)
		atomic_notifier_call_chain(&irq_notifier_list,
						irq_type, NULL);

	if (irq_type == LED_FAULT_IRQ) {
		rc = qpnp_flash_led_read(led,
			FLASH_LED_REG_LED_STATUS1(led->base), &led_status1);
		if (rc < 0) {
			pr_err("Failed to read led_status1 reg, rc=%d\n", rc);
			goto exit;
		}

		rc = qpnp_flash_led_read(led,
			FLASH_LED_REG_LED_STATUS2(led->base), &led_status2);
		if (rc < 0) {
			pr_err("Failed to read led_status2 reg, rc=%d\n", rc);
			goto exit;
		}

		if (led_status1)
			pr_emerg("led short/open fault detected! led_status1=%x\n",
				led_status1);

		if (led_status2 & FLASH_LED_VPH_DROOP_FAULT_MASK)
			pr_emerg("led vph_droop fault detected!\n");
	}

	pr_debug("irq handled, irq_type=%x, irq_status=%x\n", irq_type,
		irq_status);

exit:
	return IRQ_HANDLED;
}

int qpnp_flash_led_register_irq_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&irq_notifier_list, nb);
}

int qpnp_flash_led_unregister_irq_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&irq_notifier_list, nb);
}

static inline u8 get_safety_timer_code(u32 duration_ms)
{
	if (!duration_ms)
		return 0;

	return (duration_ms / 10) - 1;
}

static inline u8 get_vph_droop_thresh_code(u32 val_mv)
{
	if (!val_mv)
		return 0;

	return (val_mv / 100) - 25;
}

static int qpnp_flash_led_parse_each_led_dt(struct qpnp_flash_led *led,
			struct flash_node_data *fnode, struct device_node *node)
{
	const char *temp_string;
	int rc, min_ma;
	u32 val;
	bool hw_strobe = 0, edge_trigger = 0, active_high = 0;
	u8 pmic_subtype = led->pdata->pmic_rev_id->pmic_subtype;

	fnode->pdev = led->pdev;
	fnode->cdev.brightness_set = qpnp_flash_led_brightness_set;
	fnode->cdev.brightness_get = qpnp_flash_led_brightness_get;

	rc = of_property_read_string(node, "qcom,led-name", &fnode->cdev.name);
	if (rc < 0) {
		pr_err("Unable to read flash LED names\n");
		return rc;
	}

	rc = of_property_read_string(node, "label", &temp_string);
	if (!rc) {
		if (!strcmp(temp_string, "flash")) {
			fnode->type = FLASH_LED_TYPE_FLASH;
		} else if (!strcmp(temp_string, "torch")) {
			fnode->type = FLASH_LED_TYPE_TORCH;
		} else {
			pr_err("Wrong flash LED type\n");
			return rc;
		}
	} else {
		pr_err("Unable to read flash LED label\n");
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,id", &val);
	if (!rc) {
		fnode->id = (u8)val;

		if (pmic_subtype == PMI632_SUBTYPE && fnode->id > 1) {
			pr_err("Flash node id = %d not supported\n", fnode->id);
			return -EINVAL;
		}
	} else {
		pr_err("Unable to read flash LED ID\n");
		return rc;
	}

	rc = of_property_read_string(node, "qcom,default-led-trigger",
						&fnode->cdev.default_trigger);
	if (rc < 0) {
		pr_err("Unable to read trigger name\n");
		return rc;
	}

	fnode->default_ires_ua = fnode->ires_ua = FLASH_LED_IRES_DEFAULT_UA;
	fnode->default_ires_idx = fnode->ires_idx = FLASH_LED_IRES_DEFAULT_VAL;
	rc = of_property_read_u32(node, "qcom,ires-ua", &val);
	if (!rc) {
		fnode->default_ires_ua = fnode->ires_ua = val;
		fnode->default_ires_idx = fnode->ires_idx =
			FLASH_LED_IRES_BASE - (val - FLASH_LED_IRES_MIN_UA) /
			FLASH_LED_IRES_DIVISOR;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read current resolution rc=%d\n", rc);
		return rc;
	}

	min_ma = fnode->ires_ua / 1000;
	rc = of_property_read_u32(node, "qcom,max-current", &val);
	if (!rc) {
		if (val < min_ma)
			val = min_ma;
		fnode->max_current = val;
		fnode->cdev.max_brightness = val;
	} else {
		pr_err("Unable to read max current, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,current-ma", &val);
	if (!rc) {
		if (val < min_ma || val > fnode->max_current)
			pr_warn("Invalid operational current specified, capping it\n");
		if (val < min_ma)
			val = min_ma;
		if (val > fnode->max_current)
			val = fnode->max_current;
		fnode->current_ma = val;
		fnode->cdev.brightness = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read operational current, rc=%d\n", rc);
		return rc;
	}

	fnode->duration = FLASH_LED_SAFETY_TMR_DISABLED;
	rc = of_property_read_u32(node, "qcom,duration-ms", &val);
	if (!rc) {
		fnode->duration = get_safety_timer_code(val);
		if (fnode->duration)
			fnode->duration |= FLASH_LED_SAFETY_TMR_ENABLE;
	} else if (rc == -EINVAL) {
		if (fnode->type == FLASH_LED_TYPE_FLASH) {
			pr_err("Timer duration is required for flash LED\n");
			return rc;
		}
	} else {
		pr_err("Unable to read timer duration\n");
		return rc;
	}

	fnode->hdrm_val = FLASH_LED_HDRM_VOL_DEFAULT_MV;
	rc = of_property_read_u32(node, "qcom,hdrm-voltage-mv", &val);
	if (!rc) {
		val = (val - FLASH_LED_HDRM_VOL_BASE_MV) /
						FLASH_LED_HDRM_VOL_STEP_MV;
		fnode->hdrm_val = (val << FLASH_LED_HDRM_VOL_SHIFT) &
							FLASH_LED_HDRM_VOL_MASK;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read headroom voltage\n");
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,hdrm-vol-hi-lo-win-mv", &val);
	if (!rc) {
		fnode->hdrm_val |= (val / FLASH_LED_HDRM_VOL_STEP_MV) &
						~FLASH_LED_HDRM_VOL_MASK;
	} else if (rc == -EINVAL) {
		fnode->hdrm_val |= FLASH_LED_HDRM_VOL_HI_LO_WIN_DEFAULT_MV;
	} else {
		pr_err("Unable to read hdrm hi-lo window voltage\n");
		return rc;
	}

	fnode->strobe_sel = SW_STROBE;
	rc = of_property_read_u32(node, "qcom,strobe-sel", &val);
	if (rc < 0) {
		if (rc != -EINVAL) {
			pr_err("Unable to read qcom,strobe-sel property\n");
			return rc;
		}
	} else {
		if (val < SW_STROBE || val > LPG_STROBE) {
			pr_err("Incorrect strobe selection specified %d\n",
				val);
			return -EINVAL;
		}
		fnode->strobe_sel = (u8)val;
	}

	/*
	 * LPG strobe is allowed only for LED3 and HW strobe option should be
	 * option 2 or 3.
	 */
	if (fnode->strobe_sel == LPG_STROBE) {
		if (led->pdata->hw_strobe_option ==
				FLASH_LED_HW_STROBE_OPTION_1) {
			pr_err("Incorrect strobe option for LPG strobe\n");
			return -EINVAL;
		}
		if (fnode->id != LED3) {
			pr_err("Incorrect LED chosen for LPG strobe\n");
			return -EINVAL;
		}
	}

	if (fnode->strobe_sel == HW_STROBE) {
		edge_trigger = of_property_read_bool(node,
						"qcom,hw-strobe-edge-trigger");
		active_high = !of_property_read_bool(node,
						"qcom,hw-strobe-active-low");
		hw_strobe = 1;
	} else if (fnode->strobe_sel == LPG_STROBE) {
		/* LPG strobe requires level trigger and active high */
		edge_trigger = 0;
		active_high =  1;
		hw_strobe = 1;
	}
	fnode->strobe_ctrl = (hw_strobe << 2) | (edge_trigger << 1) |
				active_high;

	rc = led_classdev_register(&led->pdev->dev, &fnode->cdev);
	if (rc < 0) {
		pr_err("Unable to register led node %d\n", fnode->id);
		return rc;
	}

	fnode->cdev.dev->of_node = node;
	fnode->strobe_pinctrl = devm_pinctrl_get(fnode->cdev.dev);
	if (IS_ERR_OR_NULL(fnode->strobe_pinctrl)) {
		pr_debug("No pinctrl defined for %s, err=%ld\n",
			fnode->cdev.name, PTR_ERR(fnode->strobe_pinctrl));
		fnode->strobe_pinctrl = NULL;
	}

	if (fnode->strobe_sel == HW_STROBE) {
		if (of_find_property(node, "qcom,hw-strobe-gpio", NULL)) {
			fnode->hw_strobe_gpio = of_get_named_gpio(node,
						"qcom,hw-strobe-gpio", 0);
			if (fnode->hw_strobe_gpio < 0) {
				pr_err("Invalid gpio specified\n");
				return fnode->hw_strobe_gpio;
			}
			gpio_direction_output(fnode->hw_strobe_gpio, 0);
		} else if (fnode->strobe_pinctrl) {
			fnode->hw_strobe_gpio = -1;
			fnode->hw_strobe_state_active =
				pinctrl_lookup_state(fnode->strobe_pinctrl,
							"strobe_enable");
			if (IS_ERR_OR_NULL(fnode->hw_strobe_state_active)) {
				pr_err("No active pin for hardware strobe, rc=%ld\n",
					PTR_ERR(fnode->hw_strobe_state_active));
				fnode->hw_strobe_state_active = NULL;
			}

			fnode->hw_strobe_state_suspend =
				pinctrl_lookup_state(fnode->strobe_pinctrl,
							"strobe_disable");
			if (IS_ERR_OR_NULL(fnode->hw_strobe_state_suspend)) {
				pr_err("No suspend pin for hardware strobe, rc=%ld\n",
					PTR_ERR(fnode->hw_strobe_state_suspend)
					);
				fnode->hw_strobe_state_suspend = NULL;
			}
		}
	}

	return 0;
}

static int qpnp_flash_led_parse_and_register_switch(struct qpnp_flash_led *led,
						struct flash_switch_data *snode,
						struct device_node *node)
{
	int rc = 0, num;
	char reg_name[16], reg_sup_name[16];

	rc = of_property_read_string(node, "qcom,led-name", &snode->cdev.name);
	if (rc < 0) {
		pr_err("Failed to read switch node name, rc=%d\n", rc);
		return rc;
	}

	rc = sscanf(snode->cdev.name, "led:switch_%d", &num);
	if (!rc) {
		pr_err("No number for switch device?\n");
		return -EINVAL;
	}

	rc = of_property_read_string(node, "qcom,default-led-trigger",
					&snode->cdev.default_trigger);
	if (rc < 0) {
		pr_err("Unable to read trigger name, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,led-mask", &snode->led_mask);
	if (rc < 0) {
		pr_err("Unable to read led mask rc=%d\n", rc);
		return rc;
	}

	snode->symmetry_en = of_property_read_bool(node, "qcom,symmetry-en");

	if (snode->led_mask < 1 || snode->led_mask > 7) {
		pr_err("Invalid value for led-mask\n");
		return -EINVAL;
	}

	scnprintf(reg_name, sizeof(reg_name), "switch%d-supply", num);
	if (of_find_property(led->pdev->dev.of_node, reg_name, NULL)) {
		scnprintf(reg_sup_name, sizeof(reg_sup_name), "switch%d", num);
		snode->vreg = devm_regulator_get(&led->pdev->dev, reg_sup_name);
		if (IS_ERR_OR_NULL(snode->vreg)) {
			rc = PTR_ERR(snode->vreg);
			if (rc != -EPROBE_DEFER)
				pr_err("Failed to get regulator, rc=%d\n", rc);
			snode->vreg = NULL;
			return rc;
		}
	}

	snode->pdev = led->pdev;
	snode->cdev.brightness_set = qpnp_flash_led_brightness_set;
	snode->cdev.brightness_get = qpnp_flash_led_brightness_get;
	snode->cdev.flags |= LED_KEEP_TRIGGER;
	rc = led_classdev_register(&led->pdev->dev, &snode->cdev);
	if (rc < 0) {
		pr_err("Unable to register led switch node\n");
		return rc;
	}

	snode->cdev.dev->of_node = node;

	snode->led_en_pinctrl = devm_pinctrl_get(snode->cdev.dev);
	if (IS_ERR_OR_NULL(snode->led_en_pinctrl)) {
		pr_debug("No pinctrl defined for %s, err=%ld\n",
			snode->cdev.name, PTR_ERR(snode->led_en_pinctrl));
		snode->led_en_pinctrl = NULL;
	}

	if (snode->led_en_pinctrl) {
		snode->gpio_state_active =
			pinctrl_lookup_state(snode->led_en_pinctrl,
						"led_enable");
		if (IS_ERR_OR_NULL(snode->gpio_state_active)) {
			pr_err("Cannot lookup LED active state\n");
			devm_pinctrl_put(snode->led_en_pinctrl);
			snode->led_en_pinctrl = NULL;
			return PTR_ERR(snode->gpio_state_active);
		}

		snode->gpio_state_suspend =
			pinctrl_lookup_state(snode->led_en_pinctrl,
						"led_disable");
		if (IS_ERR_OR_NULL(snode->gpio_state_suspend)) {
			pr_err("Cannot lookup LED disable state\n");
			devm_pinctrl_put(snode->led_en_pinctrl);
			snode->led_en_pinctrl = NULL;
			return PTR_ERR(snode->gpio_state_suspend);
		}
	}

	return 0;
}

static int get_code_from_table(int *table, int len, int value)
{
	int i;

	for (i = 0; i < len; i++) {
		if (value == table[i])
			break;
	}

	if (i == len) {
		pr_err("Couldn't find %d from table\n", value);
		return -ENODATA;
	}

	return i;
}

static int qpnp_flash_led_parse_common_dt(struct qpnp_flash_led *led,
						struct device_node *node)
{
	struct device_node *revid_node;
	int rc;
	u32 val;
	u8 pmic_subtype;
	bool short_circuit_det, open_circuit_det, vph_droop_det;

	revid_node = of_parse_phandle(node, "qcom,pmic-revid", 0);
	if (!revid_node) {
		pr_err("Missing qcom,pmic-revid property - driver failed\n");
		return -EINVAL;
	}

	led->pdata->pmic_rev_id = get_revid_data(revid_node);
	if (IS_ERR_OR_NULL(led->pdata->pmic_rev_id)) {
		pr_err("Unable to get pmic_revid rc=%ld\n",
			PTR_ERR(led->pdata->pmic_rev_id));
		/*
		 * the revid peripheral must be registered, any failure
		 * here only indicates that the rev-id module has not
		 * probed yet.
		 */
		return -EPROBE_DEFER;
	}

	pmic_subtype = led->pdata->pmic_rev_id->pmic_subtype;
	pr_debug("PMIC subtype %d Digital major %d\n",
		led->pdata->pmic_rev_id->pmic_subtype,
		led->pdata->pmic_rev_id->rev4);
	led->pdata->hdrm_auto_mode_en = of_property_read_bool(node,
							"qcom,hdrm-auto-mode");

	led->pdata->isc_delay = FLASH_LED_ISC_DELAY_DEFAULT;
	rc = of_property_read_u32(node, "qcom,isc-delay-us", &val);
	if (!rc) {
		led->pdata->isc_delay =
				val >> FLASH_LED_ISC_WARMUP_DELAY_SHIFT;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read ISC delay, rc=%d\n", rc);
		return rc;
	}

	led->pdata->warmup_delay = FLASH_LED_WARMUP_DELAY_DEFAULT;
	rc = of_property_read_u32(node, "qcom,warmup-delay-us", &val);
	if (!rc) {
		led->pdata->warmup_delay =
				val >> FLASH_LED_ISC_WARMUP_DELAY_SHIFT;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read WARMUP delay, rc=%d\n", rc);
		return rc;
	}

	short_circuit_det =
		of_property_read_bool(node, "qcom,short-circuit-det");
	open_circuit_det = of_property_read_bool(node, "qcom,open-circuit-det");
	vph_droop_det = of_property_read_bool(node, "qcom,vph-droop-det");
	led->pdata->current_derate_en_cfg = (vph_droop_det << 2) |
				(open_circuit_det << 1) | short_circuit_det;

	led->pdata->thermal_derate_en =
		of_property_read_bool(node, "qcom,thermal-derate-en");

	if (led->pdata->thermal_derate_en) {
		led->pdata->thermal_derate_current =
			devm_kcalloc(&led->pdev->dev,
					FLASH_LED_THERMAL_OTST_LEVELS,
					sizeof(int), GFP_KERNEL);
		if (!led->pdata->thermal_derate_current)
			return -ENOMEM;

		rc = of_property_read_u32_array(node,
					"qcom,thermal-derate-current",
					led->pdata->thermal_derate_current,
					FLASH_LED_THERMAL_OTST_LEVELS);
		if (rc < 0) {
			pr_err("Unable to read thermal current limits, rc=%d\n",
				rc);
			return rc;
		}
	}

	led->pdata->otst_ramp_bkup_en =
		!of_property_read_bool(node, "qcom,otst-ramp-back-up-dis");

	led->pdata->thermal_derate_slow = -EINVAL;
	rc = of_property_read_u32(node, "qcom,thermal-derate-slow", &val);
	if (!rc) {
		if (val < 0 || val > THERMAL_DERATE_SLOW_MAX) {
			pr_err("Invalid thermal_derate_slow %d\n", val);
			return -EINVAL;
		}

		led->pdata->thermal_derate_slow =
			get_code_from_table(thermal_derate_slow_table,
				ARRAY_SIZE(thermal_derate_slow_table), val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read thermal derate slow, rc=%d\n", rc);
		return rc;
	}

	led->pdata->thermal_derate_fast = -EINVAL;
	rc = of_property_read_u32(node, "qcom,thermal-derate-fast", &val);
	if (!rc) {
		if (val < 0 || val > THERMAL_DERATE_FAST_MAX) {
			pr_err("Invalid thermal_derate_fast %d\n", val);
			return -EINVAL;
		}

		led->pdata->thermal_derate_fast =
			get_code_from_table(thermal_derate_fast_table,
				ARRAY_SIZE(thermal_derate_fast_table), val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read thermal derate fast, rc=%d\n", rc);
		return rc;
	}

	led->pdata->thermal_debounce = -EINVAL;
	rc = of_property_read_u32(node, "qcom,thermal-debounce", &val);
	if (!rc) {
		if (val < 0 || val > THERMAL_DEBOUNCE_TIME_MAX) {
			pr_err("Invalid thermal_debounce %d\n", val);
			return -EINVAL;
		}

		if (val >= 0 && val < 16)
			led->pdata->thermal_debounce = 0;
		else
			led->pdata->thermal_debounce = ilog2(val) - 3;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read thermal debounce, rc=%d\n", rc);
		return rc;
	}

	led->pdata->thermal_hysteresis = -EINVAL;
	rc = of_property_read_u32(node, "qcom,thermal-hysteresis", &val);
	if (!rc) {
		if (pmic_subtype == PM660L_SUBTYPE)
			val = THERMAL_HYST_TEMP_TO_VAL(val, 20);
		else
			val = THERMAL_HYST_TEMP_TO_VAL(val, 15);

		if (val < 0 || val > THERMAL_DERATE_HYSTERESIS_MAX) {
			pr_err("Invalid thermal_derate_hysteresis %d\n", val);
			return -EINVAL;
		}

		led->pdata->thermal_hysteresis = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read thermal hysteresis, rc=%d\n", rc);
		return rc;
	}

	led->pdata->thermal_thrsh1 = -EINVAL;
	rc = of_property_read_u32(node, "qcom,thermal-thrsh1", &val);
	if (!rc) {
		led->pdata->thermal_thrsh1 =
			get_code_from_table(otst1_threshold_table,
				ARRAY_SIZE(otst1_threshold_table), val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read thermal thrsh1, rc=%d\n", rc);
		return rc;
	}

	led->pdata->thermal_thrsh2 = -EINVAL;
	rc = of_property_read_u32(node, "qcom,thermal-thrsh2", &val);
	if (!rc) {
		led->pdata->thermal_thrsh2 =
			get_code_from_table(otst2_threshold_table,
				ARRAY_SIZE(otst2_threshold_table), val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read thermal thrsh2, rc=%d\n", rc);
		return rc;
	}

	led->pdata->thermal_thrsh3 = -EINVAL;
	rc = of_property_read_u32(node, "qcom,thermal-thrsh3", &val);
	if (!rc) {
		led->pdata->thermal_thrsh3 =
			get_code_from_table(otst3_threshold_table,
				ARRAY_SIZE(otst3_threshold_table), val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read thermal thrsh3, rc=%d\n", rc);
		return rc;
	}

	led->pdata->vph_droop_debounce = FLASH_LED_VPH_DROOP_DEBOUNCE_DEFAULT;
	rc = of_property_read_u32(node, "qcom,vph-droop-debounce-us", &val);
	if (!rc) {
		led->pdata->vph_droop_debounce =
			VPH_DROOP_DEBOUNCE_US_TO_VAL(val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read VPH droop debounce, rc=%d\n", rc);
		return rc;
	}

	if (led->pdata->vph_droop_debounce > FLASH_LED_DEBOUNCE_MAX) {
		pr_err("Invalid VPH droop debounce specified\n");
		return -EINVAL;
	}

	if (pmic_subtype == PMI632_SUBTYPE)
		led->pdata->vph_droop_threshold =
				    BHARGER_FLASH_LED_VPH_DROOP_THRESH_DEFAULT;
	else
		led->pdata->vph_droop_threshold =
					FLASH_LED_VPH_DROOP_THRESH_DEFAULT;

	rc = of_property_read_u32(node, "qcom,vph-droop-threshold-mv", &val);
	if (!rc) {
		led->pdata->vph_droop_threshold =
			get_vph_droop_thresh_code(val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read VPH droop threshold, rc=%d\n", rc);
		return rc;
	}

	if (led->pdata->vph_droop_threshold > FLASH_LED_VPH_DROOP_THRESH_MAX) {
		pr_err("Invalid VPH droop threshold specified\n");
		return -EINVAL;
	}

	led->pdata->vph_droop_hysteresis =
			FLASH_LED_VPH_DROOP_HYST_DEFAULT;
	rc = of_property_read_u32(node, "qcom,vph-droop-hysteresis-mv", &val);
	if (!rc) {
		led->pdata->vph_droop_hysteresis =
			VPH_DROOP_HYST_MV_TO_VAL(val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to read VPH droop hysteresis, rc=%d\n", rc);
		return rc;
	}

	if (led->pdata->vph_droop_hysteresis > FLASH_LED_HYSTERESIS_MAX) {
		pr_err("Invalid VPH droop hysteresis specified\n");
		return -EINVAL;
	}

	led->pdata->vph_droop_hysteresis <<= FLASH_LED_VPH_DROOP_HYST_SHIFT;

	led->pdata->hw_strobe_option = -EINVAL;
	rc = of_property_read_u32(node, "qcom,hw-strobe-option", &val);
	if (!rc) {
		led->pdata->hw_strobe_option = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse hw strobe option, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,led1n2-iclamp-low-ma", &val);
	if (!rc) {
		led->pdata->led1n2_iclamp_low_ma = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read led1n2_iclamp_low current, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,led1n2-iclamp-mid-ma", &val);
	if (!rc) {
		led->pdata->led1n2_iclamp_mid_ma = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read led1n2_iclamp_mid current, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,led3-iclamp-low-ma", &val);
	if (!rc) {
		led->pdata->led3_iclamp_low_ma = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read led3_iclamp_low current, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,led3-iclamp-mid-ma", &val);
	if (!rc) {
		led->pdata->led3_iclamp_mid_ma = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to read led3_iclamp_mid current, rc=%d\n", rc);
		return rc;
	}

	led->pdata->vled_max_uv = FLASH_LED_VLED_MAX_DEFAULT_UV;
	rc = of_property_read_u32(node, "qcom,vled-max-uv", &val);
	if (!rc) {
		led->pdata->vled_max_uv = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse vled_max voltage, rc=%d\n", rc);
		return rc;
	}

	led->pdata->ibatt_ocp_threshold_ua =
		FLASH_LED_IBATT_OCP_THRESH_DEFAULT_UA;
	rc = of_property_read_u32(node, "qcom,ibatt-ocp-threshold-ua", &val);
	if (!rc) {
		led->pdata->ibatt_ocp_threshold_ua = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse ibatt_ocp threshold, rc=%d\n", rc);
		return rc;
	}

	led->pdata->rpara_uohm = FLASH_LED_RPARA_DEFAULT_UOHM;
	rc = of_property_read_u32(node, "qcom,rparasitic-uohm", &val);
	if (!rc) {
		led->pdata->rpara_uohm = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse rparasitic, rc=%d\n", rc);
		return rc;
	}

	led->pdata->lmh_ocv_threshold_uv =
		FLASH_LED_LMH_OCV_THRESH_DEFAULT_UV;
	rc = of_property_read_u32(node, "qcom,lmh-ocv-threshold-uv", &val);
	if (!rc) {
		led->pdata->lmh_ocv_threshold_uv = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse lmh ocv threshold, rc=%d\n", rc);
		return rc;
	}

	led->pdata->lmh_rbatt_threshold_uohm =
		FLASH_LED_LMH_RBATT_THRESH_DEFAULT_UOHM;
	rc = of_property_read_u32(node, "qcom,lmh-rbatt-threshold-uohm", &val);
	if (!rc) {
		led->pdata->lmh_rbatt_threshold_uohm = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse lmh rbatt threshold, rc=%d\n", rc);
		return rc;
	}

	led->pdata->lmh_level = FLASH_LED_LMH_LEVEL_DEFAULT;
	rc = of_property_read_u32(node, "qcom,lmh-level", &val);
	if (!rc) {
		led->pdata->lmh_level = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse lmh_level, rc=%d\n", rc);
		return rc;
	}

	led->pdata->lmh_mitigation_sel = FLASH_LED_LMH_MITIGATION_SEL_DEFAULT;
	rc = of_property_read_u32(node, "qcom,lmh-mitigation-sel", &val);
	if (!rc) {
		led->pdata->lmh_mitigation_sel = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse lmh_mitigation_sel, rc=%d\n", rc);
		return rc;
	}

	if (led->pdata->lmh_mitigation_sel > FLASH_LED_MITIGATION_SEL_MAX) {
		pr_err("Invalid lmh_mitigation_sel specified\n");
		return -EINVAL;
	}

	if (pmic_subtype == PMI632_SUBTYPE)
		led->pdata->chgr_mitigation_sel =
					FLASH_DISABLE_CHARGER_MITIGATION;
	else
		led->pdata->chgr_mitigation_sel = FLASH_SW_CHARGER_MITIGATION;

	rc = of_property_read_u32(node, "qcom,chgr-mitigation-sel", &val);
	if (!rc) {
		led->pdata->chgr_mitigation_sel = val;
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse chgr_mitigation_sel, rc=%d\n", rc);
		return rc;
	}

	if (led->pdata->chgr_mitigation_sel > FLASH_LED_MITIGATION_SEL_MAX) {
		pr_err("Invalid chgr_mitigation_sel specified\n");
		return -EINVAL;
	}

	led->pdata->iled_thrsh_val = FLASH_LED_CHGR_MITIGATION_THRSH_DEFAULT;
	rc = of_property_read_u32(node, "qcom,iled-thrsh-ma", &val);
	if (!rc) {
		led->pdata->iled_thrsh_val = MITIGATION_THRSH_MA_TO_VAL(val);
	} else if (rc != -EINVAL) {
		pr_err("Unable to parse iled_thrsh_val, rc=%d\n", rc);
		return rc;
	}

	if (led->pdata->iled_thrsh_val > FLASH_LED_CHGR_MITIGATION_THRSH_MAX) {
		pr_err("Invalid iled_thrsh_val specified\n");
		return -EINVAL;
	}

	led->pdata->bst_pwm_ovrhd_uv = FLASH_BST_PWM_OVRHD_MIN_UV;
	rc = of_property_read_u32(node, "qcom,bst-pwm-ovrhd-uv", &val);
	if (!rc) {
		if (val >= FLASH_BST_PWM_OVRHD_MIN_UV &&
					val <= FLASH_BST_PWM_OVRHD_MAX_UV)
			led->pdata->bst_pwm_ovrhd_uv = val;
	}

	led->pdata->all_ramp_up_done_irq =
		of_irq_get_byname(node, "all-ramp-up-done-irq");
	if (led->pdata->all_ramp_up_done_irq < 0)
		pr_debug("all-ramp-up-done-irq not used\n");

	led->pdata->all_ramp_down_done_irq =
		of_irq_get_byname(node, "all-ramp-down-done-irq");
	if (led->pdata->all_ramp_down_done_irq < 0)
		pr_debug("all-ramp-down-done-irq not used\n");

	led->pdata->led_fault_irq =
		of_irq_get_byname(node, "led-fault-irq");
	if (led->pdata->led_fault_irq < 0)
		pr_debug("led-fault-irq not used\n");

	return 0;
}

static int qpnp_flash_led_probe(struct platform_device *pdev)
{
	struct qpnp_flash_led *led;
	struct device_node *node, *temp;
	const char *temp_string;
	unsigned int base;
	int rc, i = 0, j = 0;

	node = pdev->dev.of_node;
	if (!node) {
		pr_err("No flash LED nodes defined\n");
		return -ENODEV;
	}

	rc = of_property_read_u32(node, "reg", &base);
	if (rc < 0) {
		pr_err("Couldn't find reg in node %s, rc = %d\n",
			node->full_name, rc);
		return rc;
	}

	led = devm_kzalloc(&pdev->dev, sizeof(struct qpnp_flash_led),
								GFP_KERNEL);
	if (!led)
		return -ENOMEM;

	led->regmap = dev_get_regmap(pdev->dev.parent, NULL);
	if (!led->regmap) {
		pr_err("Couldn't get parent's regmap\n");
		return -EINVAL;
	}

	led->base = base;
	led->pdev = pdev;
	led->pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct flash_led_platform_data), GFP_KERNEL);
	if (!led->pdata)
		return -ENOMEM;

	qpnp_flash_led_prepare = qpnp_flash_led_prepare_v2;
	rc = qpnp_flash_led_parse_common_dt(led, node);
	if (rc < 0) {
		pr_err("Failed to parse common flash LED device tree\n");
		return rc;
	}

	for_each_available_child_of_node(node, temp) {
		rc = of_property_read_string(temp, "label", &temp_string);
		if (rc < 0) {
			pr_err("Failed to parse label, rc=%d\n", rc);
			return rc;
		}

		if (!strcmp("switch", temp_string)) {
			led->num_snodes++;
		} else if (!strcmp("flash", temp_string) ||
				!strcmp("torch", temp_string)) {
			led->num_fnodes++;
		} else {
			pr_err("Invalid label for led node\n");
			return -EINVAL;
		}
	}

	if (!led->num_fnodes) {
		pr_err("No LED nodes defined\n");
		return -ECHILD;
	}

	led->fnode = devm_kcalloc(&pdev->dev, led->num_fnodes,
				sizeof(*led->fnode),
				GFP_KERNEL);
	if (!led->fnode)
		return -ENOMEM;

	led->snode = devm_kcalloc(&pdev->dev, led->num_snodes,
				sizeof(*led->snode),
				GFP_KERNEL);
	if (!led->snode)
		return -ENOMEM;

	temp = NULL;
	i = 0;
	j = 0;
	for_each_available_child_of_node(node, temp) {
		rc = of_property_read_string(temp, "label", &temp_string);
		if (rc < 0) {
			pr_err("Failed to parse label, rc=%d\n", rc);
			return rc;
		}

		if (!strcmp("flash", temp_string) ||
				!strcmp("torch", temp_string)) {
			rc = qpnp_flash_led_parse_each_led_dt(led,
					&led->fnode[i], temp);
			if (rc < 0) {
				pr_err("Unable to parse flash node %d rc=%d\n",
					i, rc);
				goto error_led_register;
			}
			i++;
		}

		if (!strcmp("switch", temp_string)) {
			rc = qpnp_flash_led_parse_and_register_switch(led,
					&led->snode[j], temp);
			if (rc < 0) {
				pr_err("Unable to parse and register switch node, rc=%d\n",
					rc);
				goto error_switch_register;
			}
			j++;
		}
	}

	/* setup irqs */
	if (led->pdata->all_ramp_up_done_irq >= 0) {
		rc = devm_request_threaded_irq(&led->pdev->dev,
			led->pdata->all_ramp_up_done_irq,
			NULL, qpnp_flash_led_irq_handler,
			IRQF_ONESHOT,
			"qpnp_flash_led_all_ramp_up_done_irq", led);
		if (rc < 0) {
			pr_err("Unable to request all_ramp_up_done(%d) IRQ(err:%d)\n",
				led->pdata->all_ramp_up_done_irq, rc);
			goto error_switch_register;
		}
	}

	if (led->pdata->all_ramp_down_done_irq >= 0) {
		rc = devm_request_threaded_irq(&led->pdev->dev,
			led->pdata->all_ramp_down_done_irq,
			NULL, qpnp_flash_led_irq_handler,
			IRQF_ONESHOT,
			"qpnp_flash_led_all_ramp_down_done_irq", led);
		if (rc < 0) {
			pr_err("Unable to request all_ramp_down_done(%d) IRQ(err:%d)\n",
				led->pdata->all_ramp_down_done_irq, rc);
			goto error_switch_register;
		}
	}

	if (led->pdata->led_fault_irq >= 0) {
		rc = devm_request_threaded_irq(&led->pdev->dev,
			led->pdata->led_fault_irq,
			NULL, qpnp_flash_led_irq_handler,
			IRQF_ONESHOT,
			"qpnp_flash_led_fault_irq", led);
		if (rc < 0) {
			pr_err("Unable to request led_fault(%d) IRQ(err:%d)\n",
				led->pdata->led_fault_irq, rc);
			goto error_switch_register;
		}
	}

	led->bms_psy = power_supply_get_by_name("bms");
	if (!led->bms_psy) {
		rc = flash_led_psy_register_notifier(led);
		if (rc < 0) {
			pr_err("Couldn't register psy notifier, rc = %d\n", rc);
			goto error_switch_register;
		}
	}

	rc = qpnp_flash_led_init_settings(led);
	if (rc < 0) {
		pr_err("Failed to initialize flash LED, rc=%d\n", rc);
		goto unreg_notifier;
	}

	for (i = 0; i < led->num_snodes; i++) {
		for (j = 0; j < ARRAY_SIZE(qpnp_flash_led_attrs); j++) {
			rc = sysfs_create_file(&led->snode[i].cdev.dev->kobj,
					&qpnp_flash_led_attrs[j].attr);
			if (rc < 0) {
				pr_err("sysfs creation failed, rc=%d\n", rc);
				goto sysfs_fail;
			}
		}
	}

	spin_lock_init(&led->lock);

	dev_set_drvdata(&pdev->dev, led);

	return 0;

sysfs_fail:
	for (--j; j >= 0; j--)
		sysfs_remove_file(&led->snode[i].cdev.dev->kobj,
				&qpnp_flash_led_attrs[j].attr);

	for (--i; i >= 0; i--) {
		for (j = 0; j < ARRAY_SIZE(qpnp_flash_led_attrs); j++)
			sysfs_remove_file(&led->snode[i].cdev.dev->kobj,
					&qpnp_flash_led_attrs[j].attr);
	}

	i = led->num_snodes;
unreg_notifier:
	power_supply_unreg_notifier(&led->nb);
error_switch_register:
	while (i > 0)
		led_classdev_unregister(&led->snode[--i].cdev);
	i = led->num_fnodes;
error_led_register:
	while (i > 0)
		led_classdev_unregister(&led->fnode[--i].cdev);

	return rc;
}

static int qpnp_flash_led_remove(struct platform_device *pdev)
{
	struct qpnp_flash_led *led = dev_get_drvdata(&pdev->dev);
	int i, j;

	for (i = 0; i < led->num_snodes; i++) {
		for (j = 0; j < ARRAY_SIZE(qpnp_flash_led_attrs); j++)
			sysfs_remove_file(&led->snode[i].cdev.dev->kobj,
					&qpnp_flash_led_attrs[j].attr);

		if (led->snode[i].regulator_on)
			qpnp_flash_led_regulator_enable(led,
					&led->snode[i], false);
	}

	while (i > 0)
		led_classdev_unregister(&led->snode[--i].cdev);

	i = led->num_fnodes;
	while (i > 0)
		led_classdev_unregister(&led->fnode[--i].cdev);

	power_supply_unreg_notifier(&led->nb);
	return 0;
}

const struct of_device_id qpnp_flash_led_match_table[] = {
	{ .compatible = "qcom,qpnp-flash-led-v2",},
	{ },
};

static struct platform_driver qpnp_flash_led_driver = {
	.driver		= {
		.name = "qcom,qpnp-flash-led-v2",
		.of_match_table = qpnp_flash_led_match_table,
	},
	.probe		= qpnp_flash_led_probe,
	.remove		= qpnp_flash_led_remove,
};

static int __init qpnp_flash_led_init(void)
{
	return platform_driver_register(&qpnp_flash_led_driver);
}
late_initcall(qpnp_flash_led_init);

static void __exit qpnp_flash_led_exit(void)
{
	platform_driver_unregister(&qpnp_flash_led_driver);
}
module_exit(qpnp_flash_led_exit);

MODULE_DESCRIPTION("QPNP Flash LED driver v2");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("leds:leds-qpnp-flash-v2");
