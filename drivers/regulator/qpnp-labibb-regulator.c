/* Copyright (c) 2014-2016, The Linux Foundation. All rights reserved.
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
/*
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are Copyright (c) 2015 Sony Mobile Communications Inc,
 * and licensed under the license of the file.
 */

#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/qpnp-labibb-regulator.h>
#include <linux/spmi.h>
#include <linux/string.h>
#include <linux/qpnp-revid.h>
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
#include <linux/interrupt.h>
#include <linux/qpnp/power-on.h>
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

#define QPNP_LABIBB_REGULATOR_DRIVER_NAME	"qcom,qpnp-labibb-regulator"

#define REG_PERPH_TYPE			0x04

#define QPNP_LAB_TYPE			0x24
#define QPNP_IBB_TYPE			0x20

/* Common register value for LAB/IBB */
#define REG_LAB_IBB_LCD_MODE		0x0
#define REG_LAB_IBB_AMOLED_MODE		BIT(7)
#define REG_LAB_IBB_SEC_ACCESS		0xD0
#define REG_LAB_IBB_SEC_UNLOCK_CODE	0xA5

/* LAB register offset definitions */
#define REG_LAB_STATUS1			0x08
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
#define REG_LAB_INT_SET_TYPE		0x11
#define REG_LAB_INT_POLARITY_HIGH	0x12
#define REG_LAB_INT_POLARITY_LOW	0x13
#define REG_LAB_INT_LATCHED_CLR		0x14
#define REG_LAB_INT_EN_SET		0x15
#define REG_LAB_INT_EN_CLR		0x16
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
#define REG_LAB_VOLTAGE			0x41
#define REG_LAB_RING_SUPPRESSION_CTL	0x42
#define REG_LAB_LCD_AMOLED_SEL		0x44
#define REG_LAB_MODULE_RDY		0x45
#define REG_LAB_ENABLE_CTL		0x46
#define REG_LAB_PD_CTL			0x47
#define REG_LAB_CLK_DIV			0x48
#define REG_LAB_IBB_EN_RDY		0x49
#define REG_LAB_CURRENT_LIMIT		0x4B
#define REG_LAB_CURRENT_SENSE		0x4C
#define REG_LAB_PS_CTL			0x50
#define REG_LAB_RDSON_MNGMNT		0x53
#define REG_LAB_PRECHARGE_CTL		0x5E
#define REG_LAB_SOFT_START_CTL		0x5F
#define REG_LAB_SPARE_CTL		0x60

/* LAB register bits definitions */

/* REG_LAB_STATUS1 */
#define LAB_STATUS1_VREG_OK_MASK	BIT(7)
#define LAB_STATUS1_VREG_OK		BIT(7)

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
/* REG_LAB_INT_SET_TYPE */
#define LAB_INT_SET_TYPE_VREG_OK_MASK	BIT(0)
#define LAB_INT_SET_TYPE_VREG_OK_LEVEL	0
#define LAB_INT_SET_TYPE_VREG_OK_EDGE	1

/* REG_LAB_INT_POLARITY_HIGH */
#define LAB_INT_PRY_HIGH_VREG_OK_MASK	BIT(0)
#define LAB_INT_PRY_HIGH_VREG_OK	0

/* REG_LAB_INT_POLARITY_LOW */
#define LAB_INT_PRY_LOW_VREG_OK_MASK	BIT(0)
#define LAB_INT_PRY_LOW_VREG_OK		1

/* REG_LAB_INT_LATCHED_CLR */
#define LAB_INT_LATCHED_CLR_VREG_MASK	BIT(0)
#define LAB_INT_LATCHED_CLR_VREG	1

/* REG_LAB_INT_EN_SET */
#define LAB_INT_EN_SET_VREG_OK_MASK	BIT(0)
#define LAB_INT_EN_SET_VREG_OK		1

/* REG_LAB_INT_EN_CLR */
#define LAB_INT_EN_CLR_VREG_OK_MASK	BIT(0)
#define LAB_INT_EN_CLR_VREG_OK		1
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

/* REG_LAB_VOLTAGE */
#define LAB_VOLTAGE_OVERRIDE_EN		BIT(7)
#define LAB_VOLTAGE_SET_BITS		4
#define LAB_VOLTAGE_SET_MASK		((1 << LAB_VOLTAGE_SET_BITS) - 1)

/* REG_LAB_RING_SUPPRESSION_CTL */
#define LAB_RING_SUPPRESSION_CTL_EN	BIT(7)

/* REG_LAB_MODULE_RDY */
#define LAB_MODULE_RDY_EN		BIT(7)

/* REG_LAB_ENABLE_CTL */
#define LAB_ENABLE_CTL_EN		BIT(7)

/* REG_LAB_PD_CTL */
#define LAB_PD_CTL_STRONG_PULL		BIT(0)
#define LAB_PD_CTL_STRENGTH_MASK	BIT(0)
#define LAB_PD_CTL_DISABLE_PD		BIT(1)
#define LAB_PD_CTL_EN_MASK		BIT(1)

/* REG_LAB_IBB_EN_RDY */
#define LAB_IBB_EN_RDY_EN		BIT(7)

/* REG_LAB_CURRENT_LIMIT */
#define LAB_CURRENT_LIMIT_BITS		3
#define LAB_CURRENT_LIMIT_MASK		((1 << LAB_CURRENT_LIMIT_BITS) - 1)
#define LAB_CURRENT_LIMIT_EN		BIT(7)
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
#define LAB_CURRENT_LIMIT_OVERRIDE	BIT(3)
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

/* REG_LAB_CURRENT_SENSE */
#define LAB_CURRENT_SENSE_GAIN_BITS	2
#define LAB_CURRENT_SENSE_GAIN_MASK	((1 << LAB_CURRENT_SENSE_GAIN_BITS) \
					- 1)

/* REG_LAB_PS_CTL */
#define LAB_PS_CTL_BITS			2
#define LAB_PS_CTL_MASK			((1 << LAB_PS_CTL_BITS) - 1)
#define LAB_PS_CTL_EN			BIT(7)

/* REG_LAB_RDSON_MNGMNT */
#define LAB_RDSON_MNGMNT_NFET_SLEW_EN	BIT(5)
#define LAB_RDSON_MNGMNT_PFET_SLEW_EN	BIT(4)
#define LAB_RDSON_MNGMNT_NFET_BITS	2
#define LAB_RDSON_MNGMNT_NFET_MASK	((1 << LAB_RDSON_MNGMNT_NFET_BITS) - 1)
#define LAB_RDSON_MNGMNT_NFET_SHIFT	2
#define LAB_RDSON_MNGMNT_PFET_BITS	2
#define LAB_RDSON_MNGMNT_PFET_MASK	((1 << LAB_RDSON_MNGMNT_PFET_BITS) - 1)
#define LAB_RDSON_NFET_SW_SIZE_QUARTER	0x0
#define LAB_RDSON_PFET_SW_SIZE_QUARTER	0x0

/* REG_LAB_PRECHARGE_CTL */
#define LAB_PRECHARGE_CTL_EN		BIT(2)
#define LAB_PRECHARGE_CTL_EN_BITS	2
#define LAB_PRECHARGE_CTL_EN_MASK	((1 << LAB_PRECHARGE_CTL_EN_BITS) - 1)

/* REG_LAB_SOFT_START_CTL */
#define LAB_SOFT_START_CTL_BITS		2
#define LAB_SOFT_START_CTL_MASK		((1 << LAB_SOFT_START_CTL_BITS) - 1)

/* REG_LAB_SPARE_CTL */
#define LAB_SPARE_TOUCH_WAKE_BIT	BIT(3)
#define LAB_SPARE_DISABLE_SCP_BIT	BIT(0)

/* IBB register offset definitions */
#define REG_IBB_REVISION4		0x03
#define REG_IBB_STATUS1			0x08
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
#define REG_IBB_INT_SET_TYPE		0x11
#define REG_IBB_INT_POLARITY_HIGH	0x12
#define REG_IBB_INT_POLARITY_LOW	0x13
#define REG_IBB_INT_LATCHED_CLR		0x14
#define REG_IBB_INT_EN_SET		0x15
#define REG_IBB_INT_EN_CLR		0x16
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
#define REG_IBB_VOLTAGE		0x41
#define REG_IBB_RING_SUPPRESSION_CTL	0x42
#define REG_IBB_LCD_AMOLED_SEL		0x44
#define REG_IBB_MODULE_RDY		0x45
#define REG_IBB_ENABLE_CTL		0x46
#define REG_IBB_PD_CTL			0x47
#define REG_IBB_CLK_DIV			0x48
#define REG_IBB_CURRENT_LIMIT		0x4B
#define REG_IBB_PS_CTL			0x50
#define REG_IBB_RDSON_MNGMNT		0x53
#define REG_IBB_PWRUP_PWRDN_CTL_1	0x58
#define REG_IBB_PWRUP_PWRDN_CTL_2	0x59
#define REG_IBB_SOFT_START_CTL		0x5F
#define REG_IBB_SPARE_CTL		0x60
#define REG_IBB_NLIMIT_DAC		0x61

/* IBB register bits definition */

/* REG_IBB_STATUS1 */
#define IBB_STATUS1_VREG_OK_MASK	BIT(7)
#define IBB_STATUS1_VREG_OK		BIT(7)

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
/* REG_IBB_INT_SET_TYPE */
#define IBB_INT_SET_TYPE_VREG_OK_MASK	BIT(0)
#define IBB_INT_SET_TYPE_VREG_OK_LEVEL	0
#define IBB_INT_SET_TYPE_VREG_OK_EDGE	1

/* REG_IBB_INT_POLARITY_HIGH */
#define IBB_INT_PRY_HIGH_VREG_OK_MASK	BIT(0)
#define IBB_INT_PRY_HIGH_VREG_OK	1

/* REG_IBB_INT_POLARITY_LOW */
#define IBB_INT_PRY_LOW_VREG_OK_MASK	BIT(0)
#define IBB_INT_PRY_LOW_VREG_OK		0

/* REG_IBB_INT_LATCHED_CLR */
#define IBB_INT_LATCHED_CLR_VREG_MASK	BIT(0)
#define IBB_INT_LATCHED_CLR_VREG	1

/* REG_IBB_INT_EN_SET */
#define IBB_INT_EN_SET_VREG_OK_MASK	BIT(0)
#define IBB_INT_EN_SET_VREG_OK		1

/* REG_IBB_INT_EN_CLR */
#define IBB_INT_EN_CLR_VREG_OK_MASK	BIT(0)
#define IBB_INT_EN_CLR_VREG_OK		1
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

/* REG_IBB_VOLTAGE */
#define IBB_VOLTAGE_OVERRIDE_EN		BIT(7)
#define IBB_VOLTAGE_SET_BITS		6
#define IBB_VOLTAGE_SET_MASK		((1 << IBB_VOLTAGE_SET_BITS) - 1)

/* REG_IBB_RING_SUPPRESSION_CTL */
#define IBB_RING_SUPPRESSION_CTL_EN	BIT(7)

/* REG_IBB_MODULE_RDY */
#define IBB_MODULE_RDY_EN		BIT(7)

/* REG_IBB_ENABLE_CTL */
#define IBB_ENABLE_CTL_MASK		(BIT(7) | BIT(6))
#define IBB_ENABLE_CTL_SWIRE_RDY	BIT(6)
#define IBB_ENABLE_CTL_MODULE_EN	BIT(7)

/* REG_IBB_PD_CTL */
#define IBB_PD_CTL_HALF_STRENGTH	BIT(0)
#define IBB_PD_CTL_STRENGTH_MASK	BIT(0)
#define IBB_PD_CTL_EN			BIT(7)
#define IBB_PD_CTL_EN_MASK		BIT(7)

/* REG_IBB_CURRENT_LIMIT */
#define IBB_CURRENT_LIMIT_BITS		5
#define IBB_CURRENT_LIMIT_MASK		((1 << IBB_CURRENT_LIMIT_BITS) - 1)
#define IBB_CURRENT_LIMIT_DEBOUNCE_SHIFT	5
#define IBB_CURRENT_LIMIT_EN		BIT(7)
#define IBB_ILIMIT_COUNT_CYC8		0
#define IBB_CURRENT_MAX_500MA		0xA

/* REG_IBB_PS_CTL */
#define IBB_PS_CTL_EN			0x85
#define IBB_PS_CTL_DISABLE		0x5

/* REG_IBB_RDSON_MNGMNT */
#define IBB_NFET_SLEW_EN		BIT(7)
#define IBB_PFET_SLEW_EN		BIT(6)
#define IBB_OVERRIDE_NFET_SW_SIZE	BIT(5)
#define IBB_OVERRIDE_PFET_SW_SIZE	BIT(2)
#define IBB_NFET_SW_SIZE_BITS		2
#define IBB_PFET_SW_SIZE_BITS		2
#define IBB_NFET_SW_SIZE_MASK		((1 << NFET_SW_SIZE_BITS) - 1)
#define IBB_PFET_SW_SIZE_MASK		((1 << PFET_SW_SIZE_BITS) - 1)
#define IBB_NFET_SW_SIZE_SHIFT		3

/* REG_IBB_SOFT_START_CTL */
#define IBB_SOFT_START_CHARGING_RESISTOR_16K	0x3

/* REG_IBB_SPARE_CTL */
#define IBB_BYPASS_PWRDN_DLY2_BIT	BIT(5)
#define IBB_FAST_STARTUP		BIT(3)

/* REG_IBB_NLIMIT_DAC */
#define IBB_NLIMIT_DAC_EN		0x0
#define IBB_NLIMIT_DAC_DISABLE		0x5

/* REG_IBB_PWRUP_PWRDN_CTL_1 */
#define IBB_PWRUP_PWRDN_CTL_1_DLY1_BITS	2
#define IBB_PWRUP_PWRDN_CTL_1_DLY1_MASK	((1<<IBB_PWRUP_PWRDN_CTL_1_DLY1_BITS)-1)
#define IBB_PWRUP_PWRDN_CTL_1_DLY1_SHIFT	4
#define IBB_PWRUP_PWRDN_CTL_1_DLY2_BITS	2
#define IBB_PWRUP_PWRDN_CTL_1_DLY2_MASK	((1<<IBB_PWRUP_PWRDN_CTL_1_DLY2_BITS)-1)
#define IBB_PWRUP_PWRDN_CTL_1_LAB_VREG_OK	BIT(7)
#define IBB_PWRUP_PWRDN_CTL_1_EN_DLY1	BIT(6)
#define PWRUP_PWRDN_CTL_1_DISCHARGE_EN	BIT(2)

/* REG_IBB_PWRUP_PWRDN_CTL_2 */
#define IBB_DIS_DLY_BITS		2
#define IBB_DIS_DLY_MASK		((1 << IBB_DIS_DLY_BITS) - 1)
#define IBB_WAIT_MBG_OK			BIT(2)

enum pmic_subtype {
	PMI8994		= 10,
	PMI8950		= 17,
	PMI8996		= 19,
};

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
/* request_irq */
#define REQUEST_IRQ(chip, resource, irq_num, irq_name, irq_handler, flags, rc)\
do {									\
	irq_num = spmi_get_irq_byname(chip->spmi,			\
					resource, irq_name);		\
	if (irq_num < 0) {						\
		dev_err(chip->dev, "Unable to get " irq_name " irq\n");	\
		return -ENXIO;						\
	}								\
	rc = devm_request_irq(chip->dev, irq_num,			\
			irq_handler, flags, irq_name, chip);		\
	if (rc < 0) {							\
		dev_err(chip->dev, "Unable to request " irq_name " irq: %d\n",\
				rc);					\
		return -ENXIO;						\
	}								\
} while (0)

/* enable_irq */
#define ENABLE_IRQ(irq_num, wake)					\
do {									\
	if (wake) {							\
		pr_debug("ENABLE_IRQ_WAKE: %s\n", __func__);		\
		enable_irq_wake(irq_num);				\
	} else {							\
		pr_debug("ENABLE_IRQ: %s\n", __func__);			\
		enable_irq(irq_num);					\
	}								\
} while (0)

/* disable_irq */
#define DISABLE_IRQ(irq_num, wake)					\
do {									\
	if (wake) {							\
		pr_debug("DISABLE_IRQ_WAKE: %s\n", __func__);		\
		disable_irq_wake(irq_num);				\
	} else {							\
		pr_debug("DISABLE_IRQ_NOSYNC: %s\n", __func__);		\
		disable_irq_nosync(irq_num);				\
	}								\
} while (0)

#define IRQF_LAB_FLAGS	(IRQF_DISABLED | IRQF_ONESHOT | IRQF_TRIGGER_LOW)
#define IRQF_IBB_FLAGS	(IRQF_DISABLED | IRQF_ONESHOT | IRQF_TRIGGER_HIGH)

#define DEFAULT_TARGET_CHATTER_CNT	3
#define DEFAULT_TARGET_CHATTER_INTERVAL	500
#define DVDD_SHUTDOWN_RETRY_INTERVAL	500

static irqreturn_t ibb_vreg_handler(int irq, void *_chip);
static irqreturn_t lab_vreg_handler(int irq, void *_chip);

static struct labibb_vreg_status_ctrl {
	struct delayed_work vreg_check_work;
	struct qpnp_labibb *labibb;
	int current_chatter_cnt;
	int target_chatter_cnt;
	int target_chatter_check_interval;
	bool vreg_check_working;
} labibb_vreg_check;

#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

/**
 * enum qpnp_labibb_mode - working mode of LAB/IBB regulators
 * %QPNP_LABIBB_STANDALONE_MODE:	configure LAB/IBB regulator as a
 * standalone regulator
 * %QPNP_LABIBB_LCD_MODE:		configure LAB and IBB regulators
 * together to provide power supply for LCD
 * %QPNP_LABIBB_AMOLED_MODE:		configure LAB and IBB regulators
 * together to provide power supply for AMOLED
 * %QPNP_LABIBB_MAX_MODE		max number of configureable modes
 * supported by qpnp_labibb_regulator
 */
enum qpnp_labibb_mode {
	QPNP_LABIBB_STANDALONE_MODE = 1,
	QPNP_LABIBB_LCD_MODE,
	QPNP_LABIBB_AMOLED_MODE,
	QPNP_LABIBB_MAX_MODE,
};

/**
 * IBB_SW_CONTROL_EN: Specifies IBB is enabled through software.
 * IBB_SW_CONTROL_DIS: Specifies IBB is disabled through software.
 * IBB_HW_CONTROL: Specifies IBB is controlled through SWIRE (hardware).
 */
enum ibb_mode {
	IBB_SW_CONTROL_EN,
	IBB_SW_CONTROL_DIS,
	IBB_HW_CONTROL,
};

static const int ibb_discharge_resistor_plan[] = {
	300,
	64,
	32,
	16,
};

static const int ibb_pwrup_dly_plan[] = {
	1000,
	2000,
	4000,
	8000,
};

static const int ibb_pwrdn_dly_plan[] = {
	1000,
	2000,
	4000,
	8000,
};

static const int lab_clk_div_plan[] = {
	3200,
	2740,
	2400,
	2130,
	1920,
	1750,
	1600,
	1480,
	1370,
	1280,
	1200,
	1130,
	1070,
	1010,
	960,
	910,
};

static const int ibb_clk_div_plan[] = {
	3200,
	2740,
	2400,
	2130,
	1920,
	1750,
	1600,
	1480,
	1370,
	1280,
	1200,
	1130,
	1070,
	1010,
	960,
	910,
};

static const int lab_current_limit_plan[] = {
	200,
	400,
	600,
	800,
};

static const char * const lab_current_sense_plan[] = {
	"0.5x",
	"1x",
	"1.5x",
	"2x"
};

static const int ibb_current_limit_plan[] = {
	0,
	50,
	100,
	150,
	200,
	250,
	300,
	350,
	400,
	450,
	500,
	550,
	600,
	650,
	700,
	750,
	800,
	850,
	900,
	950,
	1000,
	1050,
	1100,
	1150,
	1200,
	1250,
	1300,
	1350,
	1400,
	1450,
	1500,
	1550,
};

static const int ibb_debounce_plan[] = {
	8,
	16,
	32,
	64,
};

static const int lab_ps_threshold_plan[] = {
	20,
	30,
	40,
	50,
};

static const int lab_soft_start_plan[] = {
	200,
	400,
	600,
	800,
};

static const int lab_rdson_nfet_plan[] = {
	25,
	50,
	75,
	100,
};

static const int lab_rdson_pfet_plan[] = {
	25,
	50,
	75,
	100,
};

static const int lab_max_precharge_plan[] = {
	200,
	300,
	400,
	500,
};

struct lab_regulator {
	struct regulator_desc		rdesc;
	struct regulator_dev		*rdev;
	struct mutex			lab_mutex;

	int				curr_volt;
	int				min_volt;

	int				step_size;
	int				slew_rate;
	int				soft_start;

	int				vreg_enabled;
};

struct ibb_regulator {
	struct regulator_desc		rdesc;
	struct regulator_dev		*rdev;
	struct mutex			ibb_mutex;

	int				curr_volt;
	int				min_volt;

	int				step_size;
	int				slew_rate;
	int				soft_start;

	u32				pwrup_dly;
	u32				pwrdn_dly;

	int				vreg_enabled;
};

struct qpnp_labibb {
	struct device			*dev;
	struct spmi_device		*spmi;
	struct pmic_revid_data		*pmic_rev_id;
	u16				lab_base;
	u16				ibb_base;
	struct lab_regulator		lab_vreg;
	struct ibb_regulator		ibb_vreg;
	int				mode;
	bool				ttw_en;
	bool				in_ttw_mode;
	bool				ibb_settings_saved;
	bool				swire_control;
	bool				ttw_force_lab_on;

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
	struct spmi_resource		*lab_spmi_res;
	struct spmi_resource		*ibb_spmi_res;
	int				lab_vreg_irq;
	int				ibb_vreg_irq;
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
};

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
static bool display_on_in_boot;
static int __init continous_splash_setup(char *str)
{
	if (!str)
		return 0;
	if (!strncmp(str, "on", 2))
		display_on_in_boot = true;
	return 0;
}
__setup("display_status=", continous_splash_setup);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

enum ibb_settings_index {
	IBB_PD_CTL = 0,
	IBB_CURRENT_LIMIT,
	IBB_RDSON_MNGMNT,
	IBB_PWRUP_PWRDN_CTL_1,
	IBB_PWRUP_PWRDN_CTL_2,
	IBB_NLIMIT_DAC,
	IBB_PS_CTL,
	IBB_SOFT_START_CTL,
	IBB_SETTINGS_MAX,
};

enum lab_settings_index {
	LAB_SOFT_START_CTL = 0,
	LAB_PS_CTL,
	LAB_RDSON_MNGMNT,
	LAB_SETTINGS_MAX,
};

struct settings {
	u16	address;
	u8	value;
	bool	sec_access;
};

#define SETTING(_id, _sec_access)		\
	[_id] = {				\
		.address = REG_##_id,		\
		.sec_access = _sec_access,	\
	}

static struct settings ibb_settings[IBB_SETTINGS_MAX] = {
	SETTING(IBB_PD_CTL, false),
	SETTING(IBB_CURRENT_LIMIT, true),
	SETTING(IBB_RDSON_MNGMNT, false),
	SETTING(IBB_PWRUP_PWRDN_CTL_1, true),
	SETTING(IBB_PWRUP_PWRDN_CTL_2, true),
	SETTING(IBB_NLIMIT_DAC, false),
	SETTING(IBB_PS_CTL, false),
	SETTING(IBB_SOFT_START_CTL, false),
};

static struct settings lab_settings[LAB_SETTINGS_MAX] = {
	SETTING(LAB_SOFT_START_CTL, false),
	SETTING(LAB_PS_CTL, false),
	SETTING(LAB_RDSON_MNGMNT, false),
};

static int
qpnp_labibb_read(struct qpnp_labibb *labibb, u8 *val,
			u16 base, int count)
{
	int rc = 0;
	struct spmi_device *spmi = labibb->spmi;

	if (base == 0) {
		pr_err("base cannot be zero base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return -EINVAL;
	}

	rc = spmi_ext_register_readl(spmi->ctrl, spmi->sid, base, val, count);
	if (rc) {
		pr_err("SPMI read failed base=0x%02x sid=0x%02x rc=%d\n", base,
				spmi->sid, rc);
		return rc;
	}
	return 0;
}

static int
qpnp_labibb_write(struct qpnp_labibb *labibb, u16 base,
			u8 *val, int count)
{
	int rc = 0;
	struct spmi_device *spmi = labibb->spmi;

	if (base == 0) {
		pr_err("base cannot be zero base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return -EINVAL;
	}

	rc = spmi_ext_register_writel(spmi->ctrl, spmi->sid, base, val, count);
	if (rc) {
		pr_err("write failed base=0x%02x sid=0x%02x rc=%d\n",
			base, spmi->sid, rc);
		return rc;
	}

	return 0;
}

static int
qpnp_labibb_masked_write(struct qpnp_labibb *labibb, u16 base,
						u8 mask, u8 val, int count)
{
	int rc;
	u8 reg;

	rc = qpnp_labibb_read(labibb, &reg, base, count);
	if (rc) {
		pr_err("spmi read failed: addr=%03X, rc=%d\n", base, rc);
		return rc;
	}
	pr_debug("addr = 0x%x read 0x%x\n", base, reg);

	reg &= ~mask;
	reg |= val & mask;

	pr_debug("Writing 0x%x\n", reg);

	rc = qpnp_labibb_write(labibb, base, &reg, count);
	if (rc) {
		pr_err("spmi write failed: addr=%03X, rc=%d\n", base, rc);
		return rc;
	}

	return 0;
}

static int qpnp_labibb_sec_write(struct qpnp_labibb *labibb, u16 base,
					u8 offset, u8 *val, int count)
{
	int rc;
	u8 sec_val = REG_LAB_IBB_SEC_UNLOCK_CODE;

	rc = qpnp_labibb_write(labibb, base + REG_LAB_IBB_SEC_ACCESS, &sec_val,
				1);
	if (rc) {
		pr_err("qpnp_lab_write register %x failed rc = %d\n",
			base + REG_LAB_IBB_SEC_ACCESS, rc);
		return rc;
	}

	rc = qpnp_labibb_write(labibb, base + offset, val, count);
	if (rc)
		pr_err("qpnp_labibb_write failed: addr=%03X, rc=%d\n",
			base + offset, rc);

	return rc;
}

static int qpnp_labibb_sec_masked_write(struct qpnp_labibb *labibb, u16 base,
					u8 offset, u8 mask, u8 val, int count)
{
	int rc;
	u8 sec_val = REG_LAB_IBB_SEC_UNLOCK_CODE;

	rc = qpnp_labibb_write(labibb, base + REG_LAB_IBB_SEC_ACCESS, &sec_val,
				1);
	if (rc) {
		pr_err("qpnp_lab_write register %x failed rc = %d\n",
			base + REG_LAB_IBB_SEC_ACCESS, rc);
		return rc;
	}

	rc = qpnp_labibb_masked_write(labibb, base + offset, mask, val, count);
	if (rc)
		pr_err("qpnp_lab_write register %x failed rc = %d\n",
			base + offset, rc);

	return rc;
}

static int qpnp_labibb_get_matching_idx(const char *val)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(lab_current_sense_plan); i++)
		if (!strcmp(lab_current_sense_plan[i], val))
			return i;

	return -EINVAL;
}

static int qpnp_ibb_set_mode(struct qpnp_labibb *labibb, enum ibb_mode mode)
{
	int rc;
	u8 val;

	if (mode == IBB_SW_CONTROL_EN)
		val = IBB_ENABLE_CTL_MODULE_EN;
	else if (mode == IBB_HW_CONTROL)
		val = IBB_ENABLE_CTL_SWIRE_RDY;
	else if (mode == IBB_SW_CONTROL_DIS)
		val = 0;
	else
		return -EINVAL;

	rc = qpnp_labibb_masked_write(labibb,
		labibb->ibb_base + REG_IBB_ENABLE_CTL,
		IBB_ENABLE_CTL_MASK, val, 1);
	if (rc)
		pr_err("Unable to configure IBB_ENABLE_CTL rc=%d\n", rc);

	return rc;
}

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
static int qpnp_lab_interrupt_enable_ctl(struct qpnp_labibb *labibb,
	bool wake, bool enable, bool inWork)
{
	int rc = 0;

	if (labibb_vreg_check.vreg_check_working && !inWork) {
		pr_debug("%s: vreg_check_worker is already being processed.\n",
								__func__);
		goto exit;
	}

	if (enable) {
		/* lab int latched clr */
		rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
			REG_LAB_INT_LATCHED_CLR,
			LAB_INT_LATCHED_CLR_VREG_MASK,
			LAB_INT_LATCHED_CLR_VREG,
			1);
		if (rc) {
			pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_LATCHED_CLR, rc);
			goto exit;
		}
		/* lab int en set */
		rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
			REG_LAB_INT_EN_SET,
			LAB_INT_EN_SET_VREG_OK_MASK,
			LAB_INT_EN_SET_VREG_OK,
			1);
		if (rc) {
			pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_EN_SET, rc);
			goto exit;
		}
		ENABLE_IRQ(labibb->lab_vreg_irq, wake);
	} else {
		DISABLE_IRQ(labibb->lab_vreg_irq, wake);
		/* lab int en clr */
		rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
			REG_LAB_INT_EN_CLR,
			LAB_INT_EN_CLR_VREG_OK_MASK,
			LAB_INT_EN_CLR_VREG_OK,
			1);
		if (rc) {
			pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_EN_CLR, rc);
			goto exit;
		}
	}

exit:
	return rc;
}

static int qpnp_ibb_interrupt_enable_ctl(struct qpnp_labibb *labibb,
	bool wake, bool enable, bool inWork)
{
	int rc = 0;

	if (labibb_vreg_check.vreg_check_working && !inWork) {
		pr_debug("%s: vreg_check_worker is already being processed.\n",
								__func__);
		goto exit;
	}

	if (enable) {
		/* ibb int latched clr */
		rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
			REG_IBB_INT_LATCHED_CLR,
			IBB_INT_LATCHED_CLR_VREG_MASK,
			IBB_INT_LATCHED_CLR_VREG,
			1);
		if (rc) {
			pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_LATCHED_CLR, rc);
			goto exit;
		}
		/* ibb int en set */
		rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
			REG_IBB_INT_EN_SET,
			IBB_INT_EN_SET_VREG_OK_MASK,
			IBB_INT_EN_SET_VREG_OK,
			1);
		if (rc) {
			pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_EN_SET, rc);
			goto exit;
		}
		ENABLE_IRQ(labibb->ibb_vreg_irq, wake);
	} else {
		DISABLE_IRQ(labibb->ibb_vreg_irq, wake);
		/* ibb int en clr */
		rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
			REG_IBB_INT_EN_CLR,
			IBB_INT_EN_CLR_VREG_OK_MASK,
			IBB_INT_EN_CLR_VREG_OK,
			1);
		if (rc) {
			pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_EN_CLR, rc);
			goto exit;
		}
	}
exit:
	return rc;
}

static int qpnp_labibb_interrupt_enable_ctl(struct qpnp_labibb *labibb,
	bool wake, bool enable, bool inWork)
{
	int rc;

	rc = qpnp_lab_interrupt_enable_ctl(labibb, wake, enable, inWork);
	if (rc) {
		pr_err("%s: qpnp_lab_interrupt_enable_ctl failed rc = %d\n",
				__func__, rc);
		goto exit;
	}

	rc = qpnp_ibb_interrupt_enable_ctl(labibb, wake, enable, inWork);
	if (rc)
		pr_err("%s: qpnp_ibb_interrupt_enable_ctl failed rc = %d\n",
				__func__, rc);

exit:
	return rc;
}

static int qpnp_lab_request_interrupt(struct qpnp_labibb *labibb)
{
	int rc = 0;

	if (!labibb->lab_spmi_res) {
		pr_err("%s: lab spmi_resource is NULL\n", __func__);
		goto exit;
	}

	/* lab int set type */
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
		REG_LAB_INT_SET_TYPE,
		LAB_INT_SET_TYPE_VREG_OK_MASK,
		LAB_INT_SET_TYPE_VREG_OK_LEVEL,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_SET_TYPE, rc);
		goto exit;
	}

	/* lab int polarity high */
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
		REG_LAB_INT_POLARITY_HIGH,
		LAB_INT_PRY_HIGH_VREG_OK_MASK,
		LAB_INT_PRY_HIGH_VREG_OK,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_POLARITY_HIGH, rc);
		goto exit;
	}

	/* lab int polarity low */
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
		REG_LAB_INT_POLARITY_LOW,
		LAB_INT_PRY_LOW_VREG_OK_MASK,
		LAB_INT_PRY_LOW_VREG_OK,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_POLARITY_LOW, rc);
		goto exit;
	}

	/* lab int en set */
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
		REG_LAB_INT_EN_SET,
		LAB_INT_EN_SET_VREG_OK_MASK,
		LAB_INT_EN_SET_VREG_OK,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_EN_SET, rc);
		goto exit;
	}

	/* lab int latched clr */
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
		REG_LAB_INT_LATCHED_CLR,
		LAB_INT_LATCHED_CLR_VREG_MASK,
		LAB_INT_LATCHED_CLR_VREG,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_LAB_INT_LATCHED_CLR, rc);
		goto exit;
	}

	/* request irq */
	REQUEST_IRQ(labibb, labibb->lab_spmi_res,
		labibb->lab_vreg_irq, "lab_vreg_not_ok_interrupt",
		lab_vreg_handler, IRQF_LAB_FLAGS, rc);
	if (rc) {
		pr_err("lab request irq failed rc=%d\n", rc);
		goto exit;
	}
exit:
	return rc;
}

static int qpnp_ibb_request_interrupt(struct qpnp_labibb *labibb)
{
	int rc = 0;

	if (!labibb->ibb_spmi_res) {
		pr_err("%s: ibb spmi_resource is NULL\n", __func__);
		goto exit;
	}

	/* ibb int set type */
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
		REG_IBB_INT_SET_TYPE,
		IBB_INT_SET_TYPE_VREG_OK_MASK,
		IBB_INT_SET_TYPE_VREG_OK_LEVEL,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_SET_TYPE, rc);
		goto exit;
	}

	/* ibb int polarity high */
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
		REG_IBB_INT_POLARITY_HIGH,
		IBB_INT_PRY_HIGH_VREG_OK_MASK,
		IBB_INT_PRY_HIGH_VREG_OK,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_POLARITY_HIGH, rc);
		goto exit;
	}

	/* IBB int polarity low */
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
		REG_IBB_INT_POLARITY_LOW,
		IBB_INT_PRY_LOW_VREG_OK_MASK,
		IBB_INT_PRY_LOW_VREG_OK,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_POLARITY_LOW, rc);
		goto exit;
	}

	/* ibb int en set */
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
		REG_IBB_INT_EN_SET,
		IBB_INT_EN_SET_VREG_OK_MASK,
		IBB_INT_EN_SET_VREG_OK,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_EN_SET, rc);
		goto exit;
	}

	/* ibb int latched clr */
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
		REG_IBB_INT_LATCHED_CLR,
		IBB_INT_LATCHED_CLR_VREG_MASK,
		IBB_INT_LATCHED_CLR_VREG,
		1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
				__func__, REG_IBB_INT_LATCHED_CLR, rc);
		goto exit;
	}

	/* request irq */
	REQUEST_IRQ(labibb, labibb->ibb_spmi_res,
		labibb->ibb_vreg_irq, "ibb_vreg_not_ok_interrupt",
		ibb_vreg_handler, IRQF_IBB_FLAGS, rc);
	if (rc) {
		pr_err("ibb request irq failed rc=%d\n", rc);
		goto exit;
	}
exit:
	return rc;
}
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

static int qpnp_lab_dt_init(struct qpnp_labibb *labibb,
				struct device_node *of_node)
{
	int rc = 0;
	u8 i, val;
	u32 tmp;

	if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE) {
		if (labibb->mode == QPNP_LABIBB_LCD_MODE)
			val = REG_LAB_IBB_LCD_MODE;
		else
			val = REG_LAB_IBB_AMOLED_MODE;

		rc = qpnp_labibb_sec_write(labibb, labibb->lab_base,
				REG_LAB_LCD_AMOLED_SEL, &val, 1);

		if (rc) {
			pr_err("qpnp_lab_sec_write register %x failed rc = %d\n",
				REG_LAB_LCD_AMOLED_SEL, rc);
			return rc;
		}

		val = LAB_IBB_EN_RDY_EN;
		rc = qpnp_labibb_sec_write(labibb, labibb->lab_base,
				REG_LAB_IBB_EN_RDY, &val, 1);

		if (rc) {
			pr_err("qpnp_lab_sec_write register %x failed rc = %d\n",
				REG_LAB_IBB_EN_RDY, rc);
			return rc;
		}
	}

	val = 0;

	if (of_property_read_bool(of_node, "qcom,qpnp-lab-full-pull-down"))
		val |= LAB_PD_CTL_STRONG_PULL;

	if (!of_property_read_bool(of_node, "qcom,qpnp-lab-pull-down-enable"))
		val |= LAB_PD_CTL_DISABLE_PD;

	rc = qpnp_labibb_write(labibb, labibb->lab_base + REG_LAB_PD_CTL,
				&val, 1);

	if (rc) {
		pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
				REG_LAB_PD_CTL, rc);
		return rc;
	}

	rc = of_property_read_u32(of_node,
			"qcom,qpnp-lab-switching-clock-frequency", &tmp);
	if (rc) {
		pr_err("get qcom,qpnp-lab-switching-clock-frequency failed rc = %d\n",
			rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(lab_clk_div_plan); val++)
		if (lab_clk_div_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(lab_clk_div_plan)) {
		pr_err("Invalid property in qpnp-lab-switching-clock-frequency\n");
		return -EINVAL;
	}

	rc = qpnp_labibb_write(labibb, labibb->lab_base + REG_LAB_CLK_DIV,
				&val, 1);
	if (rc) {
		pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
			REG_LAB_CLK_DIV, rc);
		return rc;
	}

	rc = of_property_read_u32(of_node,
		"qcom,qpnp-lab-limit-maximum-current", &tmp);

	if (rc) {
		pr_err("get qcom,qpnp-lab-limit-maximum-current failed rc = %d\n",
			rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(lab_current_limit_plan); val++)
		if (lab_current_limit_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(lab_current_limit_plan)) {
		pr_err("Invalid property in qcom,qpnp-lab-limit-maximum-current\n");
		return -EINVAL;
	}

	if (of_property_read_bool(of_node,
		"qcom,qpnp-lab-limit-max-current-enable"))
		val |= LAB_CURRENT_LIMIT_EN;
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
	if (of_property_read_bool(of_node,
		"qcom,qpnp-lab-limit-max-current-enable"))
		val |= LAB_CURRENT_LIMIT_OVERRIDE;
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

	rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_LAB_CURRENT_LIMIT, &val, 1);
	if (rc) {
		pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
				REG_LAB_CURRENT_LIMIT, rc);
		return rc;
	}

	if (of_property_read_bool(of_node,
		"qcom,qpnp-lab-ring-suppression-enable")) {
		val = LAB_RING_SUPPRESSION_CTL_EN;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
					REG_LAB_RING_SUPPRESSION_CTL,
					&val,
					1);
		if (rc) {
			pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
				REG_LAB_RING_SUPPRESSION_CTL, rc);
			return rc;
		}
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-ps-threshold", &tmp);

	if (rc) {
		pr_err("get qcom,qpnp-lab-ps-threshold failed rc = %d\n",
			rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(lab_ps_threshold_plan); val++)
		if (lab_ps_threshold_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(lab_ps_threshold_plan)) {
		pr_err("Invalid property in qcom,qpnp-lab-ps-threshold\n");
		return -EINVAL;
	}

	if (of_property_read_bool(of_node, "qcom,qpnp-lab-ps-enable"))
		val |= LAB_PS_CTL_EN;

	rc = qpnp_labibb_write(labibb, labibb->lab_base + REG_LAB_PS_CTL,
				&val, 1);

	if (rc) {
		pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
				REG_LAB_PS_CTL, rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-pfet-size", &tmp);

	if (rc) {
		pr_err("get qcom,qpnp-lab-pfet-size, rc = %d\n", rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(lab_rdson_pfet_plan); val++)
		if (tmp == lab_rdson_pfet_plan[val])
			break;

	if (val == ARRAY_SIZE(lab_rdson_pfet_plan)) {
		pr_err("Invalid property in qcom,qpnp-lab-pfet-size\n");
		return -EINVAL;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-nfet-size", &tmp);

	if (rc) {
		pr_err("get qcom,qpnp-lab-nfet-size, rc = %d\n", rc);
		return rc;
	}

	for (i = 0; i < ARRAY_SIZE(lab_rdson_nfet_plan); i++)
		if (tmp == lab_rdson_nfet_plan[i])
			break;

	if (i == ARRAY_SIZE(lab_rdson_nfet_plan)) {
		pr_err("Iniid property in qcom,qpnp-lab-nfet-size\n");
		return -EINVAL;
	}

	val |= i << LAB_RDSON_MNGMNT_NFET_SHIFT;
	val |= (LAB_RDSON_MNGMNT_NFET_SLEW_EN | LAB_RDSON_MNGMNT_PFET_SLEW_EN);

	rc = qpnp_labibb_write(labibb, labibb->lab_base + REG_LAB_RDSON_MNGMNT,
				&val, 1);
	if (rc) {
		pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
			REG_LAB_RDSON_MNGMNT, rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-init-voltage",
					&(labibb->lab_vreg.curr_volt));
	if (rc) {
		pr_err("get qcom,qpnp-lab-init-voltage failed, rc = %d\n", rc);
		return rc;
	}

	if (!of_property_read_bool(of_node,
			"qcom,qpnp-lab-use-default-voltage")) {
		if (labibb->lab_vreg.curr_volt < labibb->lab_vreg.min_volt) {
			pr_err("Invalid qcom,qpnp-lab-init-voltage property, qcom,qpnp-lab-init-voltage %d is less than the the minimum voltage %d",
				labibb->lab_vreg.curr_volt,
				labibb->lab_vreg.min_volt);
			return -EINVAL;
		}

		val = DIV_ROUND_UP(labibb->lab_vreg.curr_volt -
				labibb->lab_vreg.min_volt,
				labibb->lab_vreg.step_size);

		if (val > LAB_VOLTAGE_SET_MASK) {
			pr_err("Invalid qcom,qpnp-lab-init-voltage property, qcom,qpnp-lab-init-voltage %d is larger than the max supported voltage %d",
				labibb->lab_vreg.curr_volt,
				labibb->lab_vreg.min_volt +
				labibb->lab_vreg.step_size *
				LAB_VOLTAGE_SET_MASK);
			return -EINVAL;
		}

		labibb->lab_vreg.curr_volt = val * labibb->lab_vreg.step_size +
				labibb->lab_vreg.min_volt;
		val |= LAB_VOLTAGE_OVERRIDE_EN;
	} else {
		val = 0;
	}

	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
				REG_LAB_VOLTAGE,
				LAB_VOLTAGE_SET_MASK |
				LAB_VOLTAGE_OVERRIDE_EN,
				val,
				1);

	if (rc) {
		pr_err("qpnp_lab_regulator_set_voltage write register %x failed rc = %d\n",
			REG_LAB_VOLTAGE, rc);

		return rc;
	}

	if (labibb->swire_control) {
		rc = qpnp_ibb_set_mode(labibb, IBB_HW_CONTROL);
		if (rc)
			pr_err("Unable to set SWIRE_RDY rc=%d\n", rc);
	}

	return rc;
}

static int qpnp_labibb_restore_settings(struct qpnp_labibb *labibb)
{
	int rc, i;

	for (i = 0; i < ARRAY_SIZE(ibb_settings); i++) {
		if (ibb_settings[i].sec_access)
			rc = qpnp_labibb_sec_write(labibb, labibb->ibb_base,
					ibb_settings[i].address,
					&ibb_settings[i].value, 1);
		else
			rc = qpnp_labibb_write(labibb, labibb->ibb_base +
					ibb_settings[i].address,
					&ibb_settings[i].value, 1);

		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				ibb_settings[i].address, rc);
			return rc;
		}
	}

	for (i = 0; i < ARRAY_SIZE(lab_settings); i++) {
		if (lab_settings[i].sec_access)
			rc = qpnp_labibb_sec_write(labibb, labibb->lab_base,
					lab_settings[i].address,
					&lab_settings[i].value, 1);
		else
			rc = qpnp_labibb_write(labibb, labibb->lab_base +
					lab_settings[i].address,
					&lab_settings[i].value, 1);

		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				lab_settings[i].address, rc);
			return rc;
		}
	}

	return 0;
}

static int qpnp_labibb_save_settings(struct qpnp_labibb *labibb)
{
	int rc, i;

	for (i = 0; i < ARRAY_SIZE(ibb_settings); i++) {
		rc = qpnp_labibb_read(labibb, &ibb_settings[i].value,
					labibb->ibb_base +
					ibb_settings[i].address, 1);
		if (rc) {
			pr_err("qpnp_labibb_read register %x failed rc = %d\n",
				ibb_settings[i].address, rc);
			return rc;
		}
	}

	for (i = 0; i < ARRAY_SIZE(lab_settings); i++) {
		rc = qpnp_labibb_read(labibb, &lab_settings[i].value,
					labibb->lab_base +
					lab_settings[i].address, 1);
		if (rc) {
			pr_err("qpnp_labibb_read register %x failed rc = %d\n",
				lab_settings[i].address, rc);
			return rc;
		}
	}

	return 0;
}

static int qpnp_labibb_ttw_enter_ibb_common(struct qpnp_labibb *labibb)
{
	int rc = 0;
	u8 val;

	val = 0;
	rc = qpnp_labibb_write(labibb, labibb->ibb_base + REG_IBB_PD_CTL,
				&val, 1);
	if (rc) {
		pr_err("qpnp_labibb_read register %x failed rc = %d\n",
			REG_IBB_PD_CTL, rc);
		return rc;
	}

	val = 0;
	rc = qpnp_labibb_sec_write(labibb, labibb->ibb_base,
				REG_IBB_PWRUP_PWRDN_CTL_1, &val, 1);
	if (rc) {
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_PWRUP_PWRDN_CTL_1, rc);
		return rc;
	}

	val = IBB_WAIT_MBG_OK;
	rc = qpnp_labibb_sec_masked_write(labibb, labibb->ibb_base,
				REG_IBB_PWRUP_PWRDN_CTL_2,
				IBB_DIS_DLY_MASK | IBB_WAIT_MBG_OK, val, 1);
	if (rc) {
		pr_err("qpnp_labibb_sec_write register %x failed rc = %d\n",
			REG_IBB_PWRUP_PWRDN_CTL_2, rc);
		return rc;
	}

	val = IBB_NFET_SLEW_EN | IBB_PFET_SLEW_EN | IBB_OVERRIDE_NFET_SW_SIZE |
		IBB_OVERRIDE_PFET_SW_SIZE;
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
				REG_IBB_RDSON_MNGMNT, 0xFF, val, 1);
	if (rc) {
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_RDSON_MNGMNT, rc);
		return rc;
	}

	val = IBB_CURRENT_LIMIT_EN | IBB_CURRENT_MAX_500MA |
		(IBB_ILIMIT_COUNT_CYC8 << IBB_CURRENT_LIMIT_DEBOUNCE_SHIFT);
	rc = qpnp_labibb_sec_write(labibb, labibb->ibb_base,
				REG_IBB_CURRENT_LIMIT, &val, 1);
	if (rc)
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_CURRENT_LIMIT, rc);

	return rc;
}

static int qpnp_labibb_ttw_enter_ibb_pmi8996(struct qpnp_labibb *labibb)
{
	int rc;
	u8 val;

	val = IBB_BYPASS_PWRDN_DLY2_BIT | IBB_FAST_STARTUP;
	rc = qpnp_labibb_write(labibb, labibb->ibb_base + REG_IBB_SPARE_CTL,
				&val, 1);
	if (rc)
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_SPARE_CTL, rc);

	return rc;
}

static int qpnp_labibb_ttw_enter_ibb_pmi8950(struct qpnp_labibb *labibb)
{
	int rc;
	u8 val;

	val = 0;
	rc = qpnp_labibb_write(labibb, labibb->ibb_base + REG_IBB_NLIMIT_DAC,
				&val, 1);
	if (rc) {
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_NLIMIT_DAC, rc);
		return rc;
	}

	val = IBB_PS_CTL_EN;
	rc = qpnp_labibb_write(labibb, labibb->ibb_base + REG_IBB_PS_CTL,
				&val, 1);
	if (rc) {
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_PS_CTL, rc);
		return rc;
	}

	val = IBB_SOFT_START_CHARGING_RESISTOR_16K;
	rc = qpnp_labibb_write(labibb, labibb->ibb_base +
				REG_IBB_SOFT_START_CTL, &val, 1);
	if (rc) {
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_SOFT_START_CTL, rc);
		return rc;
	}

	val = IBB_MODULE_RDY_EN;
	rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_IBB_MODULE_RDY, &val, 1);
	if (rc)
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_IBB_MODULE_RDY, rc);

	return rc;
}

static int qpnp_labibb_regulator_ttw_mode_enter(struct qpnp_labibb *labibb)
{
	int rc = 0;
	u8 val;

	/* Save the IBB settings before they get modified for TTW mode */
	if (!labibb->ibb_settings_saved) {
		rc = qpnp_labibb_save_settings(labibb);
		if (rc) {
			pr_err("Error in storing IBB setttings, rc=%d\n", rc);
			return rc;
		}
		labibb->ibb_settings_saved = true;
	}

	if (labibb->ttw_force_lab_on) {
		val = LAB_MODULE_RDY_EN;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
					REG_LAB_MODULE_RDY, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_MODULE_RDY, rc);
			return rc;
		}

		/* Prevents LAB being turned off by IBB */
		val = LAB_ENABLE_CTL_EN;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
					REG_LAB_ENABLE_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_ENABLE_CTL, rc);
			return rc;
		}

		val = LAB_RDSON_MNGMNT_NFET_SLEW_EN |
			LAB_RDSON_MNGMNT_PFET_SLEW_EN |
			LAB_RDSON_NFET_SW_SIZE_QUARTER |
			LAB_RDSON_PFET_SW_SIZE_QUARTER;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_LAB_RDSON_MNGMNT, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_RDSON_MNGMNT, rc);
			return rc;
		}

		rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
			REG_LAB_PS_CTL, LAB_PS_CTL_EN, LAB_PS_CTL_EN, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_PS_CTL, rc);
			return rc;
		}
	} else {
		val = LAB_PD_CTL_DISABLE_PD;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_LAB_PD_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_PD_CTL, rc);
			return rc;
		}

		val = LAB_SPARE_DISABLE_SCP_BIT;
		if (labibb->pmic_rev_id->pmic_subtype != PMI8950)
			val |= LAB_SPARE_TOUCH_WAKE_BIT;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_LAB_SPARE_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_SPARE_CTL, rc);
			return rc;
		}

		val = 0;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_LAB_SOFT_START_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_SOFT_START_CTL, rc);
			return rc;
		}
	}

	rc = qpnp_labibb_ttw_enter_ibb_common(labibb);
	if (rc) {
		pr_err("Failed to apply TTW ibb common settings rc=%d\n", rc);
		return rc;
	}

	switch (labibb->pmic_rev_id->pmic_subtype) {
	case PMI8996:
		rc = qpnp_labibb_ttw_enter_ibb_pmi8996(labibb);
		break;
	case PMI8950:
		rc = qpnp_labibb_ttw_enter_ibb_pmi8950(labibb);
		break;
	}
	if (rc) {
		pr_err("Failed to configure TTW-enter for IBB rc=%d\n", rc);
		return rc;
	}

	rc = qpnp_ibb_set_mode(labibb, IBB_HW_CONTROL);
	if (rc) {
		pr_err("Unable to set SWIRE_RDY rc = %d\n", rc);
		return rc;
	}
	labibb->in_ttw_mode = true;
	return 0;
}

static int qpnp_labibb_ttw_exit_ibb_pmi8996(struct qpnp_labibb *labibb)
{
	int rc;
	u8 val;

	val = 0;
	rc = qpnp_labibb_write(labibb, labibb->ibb_base + REG_IBB_SPARE_CTL,
			&val, 1);
	if (rc)
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_SPARE_CTL, rc);

	return rc;
}

static int qpnp_labibb_regulator_ttw_mode_exit(struct qpnp_labibb *labibb)
{
	int rc = 0;
	u8 val;

	if (!labibb->ibb_settings_saved) {
		pr_err("IBB settings are not saved!\n");
		return -EINVAL;
	}

	/* Restore the IBB settings back to switch back to normal mode */
	rc = qpnp_labibb_restore_settings(labibb);
	if (rc) {
		pr_err("Error in restoring IBB setttings, rc=%d\n", rc);
		return rc;
	}

	if (labibb->ttw_force_lab_on) {
		val = 0;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
					REG_LAB_ENABLE_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_ENABLE_CTL, rc);
			return rc;
		}
	} else {
		val = LAB_PD_CTL_STRONG_PULL;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
					REG_LAB_PD_CTL,	&val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
						REG_LAB_PD_CTL, rc);
			return rc;
		}

		val = 0;
		rc = qpnp_labibb_write(labibb, labibb->lab_base +
					REG_LAB_SPARE_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
					REG_LAB_SPARE_CTL, rc);
			return rc;
		}
	}

	switch (labibb->pmic_rev_id->pmic_subtype) {
	case PMI8996:
		rc = qpnp_labibb_ttw_exit_ibb_pmi8996(labibb);
		break;
	}
	if (rc) {
		pr_err("Failed to configure TTW-exit for IBB rc=%d\n", rc);
		return rc;
	}

	labibb->in_ttw_mode = false;
	return rc;
}

static int qpnp_labibb_regulator_enable(struct qpnp_labibb *labibb)
{
	int rc;
	u8 val;
	int dly;
	int retries;
	bool enabled = false;

	if (labibb->ttw_en && !labibb->ibb_vreg.vreg_enabled &&
		labibb->in_ttw_mode) {
		rc = qpnp_labibb_regulator_ttw_mode_exit(labibb);
		if (rc) {
			pr_err("Error in exiting TTW mode rc = %d\n", rc);
			return rc;
		}
	}

	rc = qpnp_ibb_set_mode(labibb, IBB_SW_CONTROL_EN);
	if (rc) {
		pr_err("Unable to set IBB_MODULE_EN rc = %d\n", rc);
		return rc;
	}

	/* total delay time */
	dly = labibb->lab_vreg.soft_start + labibb->ibb_vreg.soft_start
				+ labibb->ibb_vreg.pwrup_dly;
	usleep_range(dly, dly + 100);

	/* after this delay, lab should be enabled */
	rc = qpnp_labibb_read(labibb, &val,
			labibb->lab_base + REG_LAB_STATUS1, 1);
	if (rc) {
		pr_err("read register %x failed rc = %d\n",
			REG_LAB_STATUS1, rc);
		goto err_out;
	}

	pr_debug("soft=%d %d up=%d dly=%d\n",
		labibb->lab_vreg.soft_start, labibb->ibb_vreg.soft_start,
				labibb->ibb_vreg.pwrup_dly, dly);

	if (!(val & LAB_STATUS1_VREG_OK)) {
		pr_err("failed for LAB %x\n", val);
		goto err_out;
	}

	/* poll IBB_STATUS to make sure ibb had been enabled */
	dly = labibb->ibb_vreg.soft_start + labibb->ibb_vreg.pwrup_dly;
	retries = 10;
	while (retries--) {
		rc = qpnp_labibb_read(labibb, &val,
				labibb->ibb_base + REG_IBB_STATUS1, 1);
		if (rc) {
			pr_err("read register %x failed rc = %d\n",
				REG_IBB_STATUS1, rc);
			goto err_out;
		}

		if (val & IBB_STATUS1_VREG_OK) {
			enabled = true;
			break;
		}
		usleep_range(dly, dly + 100);
	}

	if (!enabled) {
		pr_err("failed for IBB %x\n", val);
		goto err_out;
	}

	labibb->lab_vreg.vreg_enabled = 1;
	labibb->ibb_vreg.vreg_enabled = 1;
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
	qpnp_labibb_interrupt_enable_ctl(labibb, false, true, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

	return 0;
err_out:
	rc = qpnp_ibb_set_mode(labibb, IBB_SW_CONTROL_DIS);
	if (rc) {
		pr_err("Unable to set IBB_MODULE_EN rc = %d\n", rc);
		return rc;
	}
	return -EINVAL;
}

static int qpnp_labibb_regulator_disable(struct qpnp_labibb *labibb)
{
	int rc;
	u8 val;
	int dly;
	int retries;
	bool disabled = false;

	/*
	 * When TTW mode is enabled and LABIBB regulators are disabled, it is
	 * recommended not to disable IBB through IBB_ENABLE_CTL when switching
	 * to SWIRE control on entering TTW mode. Hence, just enter TTW mode
	 * and mark the regulators disabled. When we exit TTW mode, normal
	 * mode settings will be restored anyways and regulators will be
	 * enabled as before.
	 */
	if (labibb->ttw_en && !labibb->in_ttw_mode) {
		rc = qpnp_labibb_regulator_ttw_mode_enter(labibb);
		if (rc) {
			pr_err("Error in entering TTW mode rc = %d\n", rc);
			return rc;
		}
		labibb->lab_vreg.vreg_enabled = 0;
		labibb->ibb_vreg.vreg_enabled = 0;
		return 0;
	}

	rc = qpnp_ibb_set_mode(labibb, IBB_SW_CONTROL_DIS);
	if (rc) {
		pr_err("Unable to set IBB_MODULE_EN rc = %d\n", rc);
		return rc;
	}

	/* poll IBB_STATUS to make sure ibb had been disabled */
	dly = labibb->ibb_vreg.pwrdn_dly;
	retries = 2;
	while (retries--) {
		usleep_range(dly, dly + 100);
		rc = qpnp_labibb_read(labibb, &val,
				labibb->ibb_base + REG_IBB_STATUS1, 1);
		if (rc) {
			pr_err("read register %x failed rc = %d\n",
				REG_IBB_STATUS1, rc);
			return rc;
		}

		if (!(val & IBB_STATUS1_VREG_OK)) {
			disabled = true;
			break;
		}
	}

	if (!disabled) {
		pr_err("failed for IBB %x\n", val);
		return -EINVAL;
	}

	labibb->lab_vreg.vreg_enabled = 0;
	labibb->ibb_vreg.vreg_enabled = 0;

	return 0;
}

static int qpnp_lab_regulator_enable(struct regulator_dev *rdev)
{
	int rc;
	u8 val;

	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (!labibb->lab_vreg.vreg_enabled && !labibb->swire_control) {

		if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE)
			return qpnp_labibb_regulator_enable(labibb);

		val = LAB_ENABLE_CTL_EN;
		rc = qpnp_labibb_write(labibb,
			labibb->lab_base + REG_LAB_ENABLE_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_lab_regulator_enable write register %x failed rc = %d\n",
				REG_LAB_ENABLE_CTL, rc);
			return rc;
		}

		udelay(labibb->lab_vreg.soft_start);

		rc = qpnp_labibb_read(labibb, &val,
				labibb->lab_base + REG_LAB_STATUS1, 1);
		if (rc) {
			pr_err("qpnp_lab_regulator_enable read register %x failed rc = %d\n",
				REG_LAB_STATUS1, rc);
			return rc;
		}

		if ((val & LAB_STATUS1_VREG_OK_MASK) != LAB_STATUS1_VREG_OK) {
			pr_err("qpnp_lab_regulator_enable failed\n");
			return -EINVAL;
		}

		labibb->lab_vreg.vreg_enabled = 1;
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
		qpnp_lab_interrupt_enable_ctl(labibb, false, true, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
	}

	return 0;
}

static int qpnp_lab_regulator_disable(struct regulator_dev *rdev)
{
	int rc;
	u8 val;
	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (labibb->lab_vreg.vreg_enabled && !labibb->swire_control) {

		if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE)
			return qpnp_labibb_regulator_disable(labibb);

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
		qpnp_lab_interrupt_enable_ctl(labibb, false, false, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

		val = 0;
		rc = qpnp_labibb_write(labibb,
			labibb->lab_base + REG_LAB_ENABLE_CTL, &val, 1);
		if (rc) {
			pr_err("qpnp_lab_regulator_enable write register %x failed rc = %d\n",
				REG_LAB_ENABLE_CTL, rc);
			return rc;
		}

		labibb->lab_vreg.vreg_enabled = 0;
	}
	return 0;
}

static int qpnp_lab_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (labibb->swire_control)
		return 0;

	return labibb->lab_vreg.vreg_enabled;
}

static int qpnp_lab_regulator_set_voltage(struct regulator_dev *rdev,
				int min_uV, int max_uV, unsigned *selector)
{
	int rc, new_uV;
	u8 val;
	struct qpnp_labibb *labibb = rdev_get_drvdata(rdev);

	if (labibb->swire_control)
		return 0;

	if (min_uV < labibb->lab_vreg.min_volt) {
		pr_err("qpnp_lab_regulator_set_voltage failed, min_uV %d is less than min_volt %d",
			min_uV, labibb->lab_vreg.min_volt);
	}

	val = DIV_ROUND_UP(min_uV - labibb->lab_vreg.min_volt,
				labibb->lab_vreg.step_size);
	new_uV = val * labibb->lab_vreg.step_size + labibb->lab_vreg.min_volt;

	if (new_uV > max_uV) {
		pr_err("qpnp_lab_regulator_set_voltage unable to set voltage (%d %d)\n",
			min_uV, max_uV);
		return -EINVAL;
	}

	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
				REG_LAB_VOLTAGE,
				LAB_VOLTAGE_SET_MASK |
				LAB_VOLTAGE_OVERRIDE_EN,
				val | LAB_VOLTAGE_OVERRIDE_EN,
				1);

	if (rc) {
		pr_err("qpnp_lab_regulator_set_voltage write register %x failed rc = %d\n",
			REG_LAB_VOLTAGE, rc);

		return rc;
	}

	if (new_uV > labibb->lab_vreg.curr_volt)
		udelay(val * labibb->lab_vreg.slew_rate);
	labibb->lab_vreg.curr_volt = new_uV;

	return 0;
}

static int qpnp_lab_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (labibb->swire_control)
		return 0;

	return labibb->lab_vreg.curr_volt;
}

static struct regulator_ops qpnp_lab_ops = {
	.enable			= qpnp_lab_regulator_enable,
	.disable		= qpnp_lab_regulator_disable,
	.is_enabled		= qpnp_lab_regulator_is_enabled,
	.set_voltage		= qpnp_lab_regulator_set_voltage,
	.get_voltage		= qpnp_lab_regulator_get_voltage,
};

static int register_qpnp_lab_regulator(struct qpnp_labibb *labibb,
					struct device_node *of_node)
{
	int rc = 0;
	struct regulator_init_data *init_data;
	struct regulator_desc *rdesc;
	struct regulator_config cfg = {};
	u8 val;
	const char *current_sense_str;
	bool config_current_sense = false;
	u32 tmp;

	if (!of_node) {
		dev_err(labibb->dev, "qpnp lab regulator device tree node is missing\n");
		return -EINVAL;
	}

	init_data = of_get_regulator_init_data(labibb->dev, of_node);
	if (!init_data) {
		pr_err("unable to get regulator init data for qpnp lab regulator\n");
		return -ENOMEM;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-min-voltage",
					&(labibb->lab_vreg.min_volt));
	if (rc < 0) {
		pr_err("qcom,qpnp-lab-min-voltage is missing, rc = %d\n",
			rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-step-size",
					&(labibb->lab_vreg.step_size));
	if (rc < 0) {
		pr_err("qcom,qpnp-lab-step-size is missing, rc = %d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-slew-rate",
					&(labibb->lab_vreg.slew_rate));
	if (rc < 0) {
		pr_err("qcom,qpnp-lab-slew-rate is missing, rc = %d\n",
			rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-soft-start",
					&(labibb->lab_vreg.soft_start));
	if (rc < 0) {
		pr_err("qcom,qpnp-lab-soft-start is missing, rc = %d\n",
			rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(lab_soft_start_plan); val++)
		if (lab_soft_start_plan[val] == labibb->lab_vreg.soft_start)
			break;

	if (val == ARRAY_SIZE(lab_soft_start_plan))
		val = ARRAY_SIZE(lab_soft_start_plan) - 1;

	rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_LAB_SOFT_START_CTL, &val, 1);
	if (rc) {
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_LAB_SOFT_START_CTL, rc);
		return rc;
	}

	labibb->lab_vreg.soft_start = lab_soft_start_plan
				[val & LAB_SOFT_START_CTL_MASK];

	rc = of_property_read_u32(of_node, "qcom,qpnp-lab-max-precharge-time",
				&tmp);
	if (rc) {
		pr_err("get qcom,qpnp-lab-max-precharge-time failed, rc = %d\n",
			rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(lab_max_precharge_plan); val++)
		if (lab_max_precharge_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(lab_max_precharge_plan)) {
		pr_err("Invalid property in qcom,qpnp-lab-max-precharge-time\n");
		return -EINVAL;
	}

	if (of_property_read_bool(of_node,
			"qcom,qpnp-lab-max-precharge-enable"))
		val |= LAB_PRECHARGE_CTL_EN;

	rc = qpnp_labibb_write(labibb, labibb->lab_base +
				REG_LAB_PRECHARGE_CTL, &val, 1);
	if (rc) {
		pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
			REG_LAB_PRECHARGE_CTL, rc);
		return rc;
	}

	if (labibb->mode == QPNP_LABIBB_AMOLED_MODE) {
		/*
		 * default to 1.5 times current gain if
		 * user doesn't specify the current-sense
		 * dt parameter
		 */
		current_sense_str = "1.5x";
		val = qpnp_labibb_get_matching_idx(current_sense_str);
		config_current_sense = true;
	}

	if (of_find_property(of_node,
		"qpnp,qpnp-lab-current-sense", NULL)) {
		config_current_sense = true;
		rc = of_property_read_string(of_node,
			"qpnp,qpnp-lab-current-sense",
			&current_sense_str);
		if (!rc) {
			val = qpnp_labibb_get_matching_idx(
					current_sense_str);
		} else {
			pr_err("qpnp,qpnp-lab-current-sense configured incorrectly rc = %d\n",
				rc);
			return rc;
		}
	}

	if (config_current_sense) {
		rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
			REG_LAB_CURRENT_SENSE,
			LAB_CURRENT_SENSE_GAIN_MASK,
			val, 1);
		if (rc) {
			pr_err("qpnp_labibb_write register %x failed rc = %d\n",
				REG_LAB_CURRENT_SENSE, rc);
			return rc;
		}
	}

	rc = qpnp_labibb_read(labibb, &val,
				labibb->ibb_base + REG_IBB_ENABLE_CTL, 1);
	if (rc) {
		pr_err("qpnp_labibb_read register %x failed rc = %d\n",
			REG_IBB_ENABLE_CTL, rc);
		return rc;
	}

	if (!(val & (IBB_ENABLE_CTL_SWIRE_RDY | IBB_ENABLE_CTL_MODULE_EN))) {
		/* SWIRE_RDY and IBB_MODULE_EN not enabled */
		rc = qpnp_lab_dt_init(labibb, of_node);
		if (rc) {
			pr_err("qpnp-lab: wrong DT parameter specified: rc = %d\n",
				rc);
			return rc;
		}
	} else {
		rc = qpnp_labibb_read(labibb, &val,
			labibb->lab_base + REG_LAB_LCD_AMOLED_SEL, 1);
		if (rc) {
			pr_err("qpnp_labibb_read register %x failed rc = %d\n",
				REG_LAB_LCD_AMOLED_SEL, rc);
			return rc;
		}

		if (val == REG_LAB_IBB_AMOLED_MODE)
			labibb->mode = QPNP_LABIBB_AMOLED_MODE;
		else
			labibb->mode = QPNP_LABIBB_LCD_MODE;

		rc = qpnp_labibb_read(labibb, &val, labibb->lab_base +
					REG_LAB_VOLTAGE, 1);
		if (rc) {
			pr_err("qpnp_lab_read read register %x failed rc = %d\n",
				REG_LAB_VOLTAGE, rc);
			return rc;
		}

		if (val & LAB_VOLTAGE_OVERRIDE_EN) {
			labibb->lab_vreg.curr_volt =
					(val &
					LAB_VOLTAGE_SET_MASK) *
					labibb->lab_vreg.step_size +
					labibb->lab_vreg.min_volt;
		} else if (labibb->mode == QPNP_LABIBB_LCD_MODE) {
			rc = of_property_read_u32(of_node,
				"qcom,qpnp-lab-init-lcd-voltage",
				&(labibb->lab_vreg.curr_volt));
			if (rc) {
				pr_err("get qcom,qpnp-lab-init-lcd-voltage failed, rc = %d\n",
					rc);
				return rc;
			}
		} else {
			rc = of_property_read_u32(of_node,
				"qcom,qpnp-lab-init-amoled-voltage",
				&(labibb->lab_vreg.curr_volt));
			if (rc) {
				pr_err("get qcom,qpnp-lab-init-amoled-voltage failed, rc = %d\n",
					rc);
				return rc;
			}
		}

		labibb->lab_vreg.vreg_enabled = 1;
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
		/* request interrupt */
		rc = qpnp_lab_request_interrupt(labibb);
		if (rc) {
			pr_err("lab request interrupt failed rc=%d\n",
								rc);
			return rc;
		}
		qpnp_lab_interrupt_enable_ctl(labibb, true, true, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
	}

	rc = qpnp_labibb_read(labibb, &val,
			labibb->lab_base + REG_LAB_MODULE_RDY, 1);
	if (rc) {
		pr_err("qpnp_lab_read read register %x failed rc = %d\n",
			REG_LAB_MODULE_RDY, rc);
		return rc;
	}

	if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE &&
			!(val & LAB_MODULE_RDY_EN)) {
		val = LAB_MODULE_RDY_EN;

		rc = qpnp_labibb_write(labibb, labibb->lab_base +
			REG_LAB_MODULE_RDY, &val, 1);

		if (rc) {
			pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
				REG_LAB_MODULE_RDY, rc);
			return rc;
		}
	}

	if (init_data->constraints.name) {
		rdesc			= &(labibb->lab_vreg.rdesc);
		rdesc->owner		= THIS_MODULE;
		rdesc->type		= REGULATOR_VOLTAGE;
		rdesc->ops		= &qpnp_lab_ops;
		rdesc->name		= init_data->constraints.name;

		cfg.dev = labibb->dev;
		cfg.init_data = init_data;
		cfg.driver_data = labibb;
		cfg.of_node = of_node;

		init_data->constraints.valid_ops_mask
				|= REGULATOR_CHANGE_VOLTAGE |
					REGULATOR_CHANGE_STATUS;

		labibb->lab_vreg.rdev = regulator_register(rdesc, &cfg);
		if (IS_ERR(labibb->lab_vreg.rdev)) {
			rc = PTR_ERR(labibb->lab_vreg.rdev);
			labibb->lab_vreg.rdev = NULL;
			pr_err("unable to get regulator init data for qpnp lab regulator, rc = %d\n",
				rc);

			return rc;
		}
	} else {
		dev_err(labibb->dev, "qpnp lab regulator name missing\n");
		return -EINVAL;
	}

	mutex_init(&(labibb->lab_vreg.lab_mutex));
	return 0;
}

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
static void force_labibb_regulator_disable(struct qpnp_labibb *labibb)
{
	int rc;
	u8 val;

	val = 0;
	rc = qpnp_labibb_write(labibb,
			labibb->ibb_base + REG_IBB_ENABLE_CTL, &val, 1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
			__func__, REG_IBB_ENABLE_CTL, rc);
		goto exit;
	}

	if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE)
		goto exit;

	val = 0;
	rc = qpnp_labibb_write(labibb,
			labibb->lab_base + REG_LAB_ENABLE_CTL, &val, 1);
	if (rc) {
		pr_err("%s: write register %x failed rc = %d\n",
			__func__, REG_LAB_ENABLE_CTL, rc);
		goto exit;
	}
exit:
	return;
}

static void vreg_check_worker(struct work_struct *work)
{
	u8 val;
	int rc;
	struct qpnp_labibb *labibb = labibb_vreg_check.labibb;

	if ((!labibb->lab_vreg.vreg_enabled) ||
	    (!labibb->ibb_vreg.vreg_enabled)) {
		pr_debug("%s: false detection display is off\n", __func__);
		goto exit;
	}

	/* lab vreg status check */
	rc = qpnp_labibb_read(labibb, &val,
			labibb->lab_base + REG_LAB_STATUS1, 1);
	if (rc) {
		pr_err("%s: read register %x failed rc = %d\n",
				__func__, REG_LAB_STATUS1, rc);
		goto read_error;
	}

	if (!(val & LAB_STATUS1_VREG_OK))
		goto status_error;

	/* ibb vreg status check */
	rc = qpnp_labibb_read(labibb, &val,
			labibb->ibb_base + REG_IBB_STATUS1, 1);
	if (rc) {
		pr_err("%s: read register %x failed rc = %d\n",
				__func__, REG_IBB_STATUS1, rc);
		goto read_error;
	}

	if (!(val & IBB_STATUS1_VREG_OK))
		goto status_error;

	rc = qpnp_labibb_interrupt_enable_ctl(labibb, false, true, true);
	if (rc) {
		pr_err("%s: qpnp_labibb_interrupt_enable_ctl error\n",
							__func__);
		goto write_error;
	}

	pr_debug("%s: vreg_check_worker done.\n", __func__);
	goto exit;

status_error:
	labibb_vreg_check.current_chatter_cnt++;
	pr_err("%s: VREG_NG Detection [%d]\n",
			__func__, labibb_vreg_check.current_chatter_cnt);
	if (labibb_vreg_check.current_chatter_cnt >=
			labibb_vreg_check.target_chatter_cnt) {
		pr_err("%s: execute qpnp_pon_dvdd_shutdown.\n", __func__);

		/* disable vreg */
		force_labibb_regulator_disable(labibb);
		/* shutdown */
		do {
			rc = -EINVAL;
			if (rc) {
				pr_debug("%s: qpnp_pon_dvdd_shutdown failed rc=%d\n",
						__func__, rc);
				msleep(DVDD_SHUTDOWN_RETRY_INTERVAL);
			}
		} while (rc);
		goto exit;
	}

read_error:
write_error:
	schedule_delayed_work(&labibb_vreg_check.vreg_check_work,
		msecs_to_jiffies(
			labibb_vreg_check.target_chatter_check_interval));
	return;

exit:
	/* initialize */
	labibb_vreg_check.current_chatter_cnt = 0;
	labibb_vreg_check.vreg_check_working = false;
	return;
}
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

/** This API is used to set the pull down strength of LAB regulator
 * regulator: the reglator device
 * strength: if strength is 0, LAB regulator will be set to half strength.
 * otherwise, LAB regulator will be set to full strength
 */
int qpnp_lab_set_pd_strength(struct regulator *regulator, u32 strength)
{
	struct qpnp_labibb *labibb;
	u8 val;
	int rc = 0;

	if (strength > 0)
		val = LAB_PD_CTL_STRONG_PULL;
	else
		val = 0;

	labibb = regulator_get_drvdata(regulator);

	mutex_lock(&(labibb->lab_vreg.lab_mutex));
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
				REG_LAB_PD_CTL,
				LAB_PD_CTL_STRENGTH_MASK,
				val,
				1);
	mutex_unlock(&(labibb->lab_vreg.lab_mutex));

	if (rc)
		pr_err("qpnp_lab_set_pd_strength write register %x failed rc = %d\n",
				REG_LAB_PD_CTL, rc);

	return rc;
}
EXPORT_SYMBOL(qpnp_lab_set_pd_strength);

/** This API is used to enable pull down of LAB regulator
 * regulator: the reglator device
 * enable: if enable is true, this API will enable pull down of LAB regulator.
 * otherwise, it will disable pull down for LAB regulator
 */
int qpnp_lab_pd_enable_ctl(struct regulator *regulator, bool enable)
{
	struct qpnp_labibb *labibb;
	u8 val;
	int rc = 0;

	if (enable)
		val = 0;
	else
		val = LAB_PD_CTL_DISABLE_PD;

	labibb = regulator_get_drvdata(regulator);

	mutex_lock(&(labibb->lab_vreg.lab_mutex));
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
				REG_LAB_PD_CTL,
				LAB_PD_CTL_EN_MASK,
				val,
				1);
	mutex_unlock(&(labibb->lab_vreg.lab_mutex));

	if (rc)
		pr_err("qpnp_lab_pd_enable_ctl write register %x failed rc = %d\n",
				REG_LAB_PD_CTL, rc);

	return rc;
}
EXPORT_SYMBOL(qpnp_lab_pd_enable_ctl);

/** This API is used to set the pull down strength of IBB regulator
 * regulator: the reglator device
 * strength: if strength is 0, IBB regulator will be set to half strength.
 * otherwise, IBB regulator will be set to full strength
 */
int qpnp_ibb_set_pd_strength(struct regulator *regulator, u32 strength)
{
	struct qpnp_labibb *labibb;
	u8 val;
	int rc = 0;

	if (strength > 0)
		val = 0;
	else
		val = IBB_PD_CTL_HALF_STRENGTH;

	labibb = regulator_get_drvdata(regulator);

	mutex_lock(&(labibb->ibb_vreg.ibb_mutex));
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
				REG_IBB_PD_CTL,
				IBB_PD_CTL_STRENGTH_MASK,
				val,
				1);
	mutex_unlock(&(labibb->ibb_vreg.ibb_mutex));

	if (rc)
		pr_err("qpnp_ibb_set_pd_strength write register %x failed rc = %d\n",
				REG_IBB_PD_CTL, rc);

	return rc;
}
EXPORT_SYMBOL(qpnp_ibb_set_pd_strength);

/** This API is used to enable pull down of IBB regulator
 * regulator: the reglator device
 * enable: if enable is true, this API will enable pull down of IBB regulator.
 * otherwise, it will disable pull down for IBB regulator
 */
int qpnp_ibb_pd_enable_ctl(struct regulator *regulator, bool enable)
{
	struct qpnp_labibb *labibb;
	u8 val;
	int rc = 0;

	if (enable)
		val = IBB_PD_CTL_EN;
	else
		val = 0;

	labibb = regulator_get_drvdata(regulator);

	mutex_lock(&(labibb->ibb_vreg.ibb_mutex));
	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
				REG_IBB_PD_CTL,
				IBB_PD_CTL_EN_MASK,
				val,
				1);
	mutex_unlock(&(labibb->ibb_vreg.ibb_mutex));

	if (rc)
		pr_err("qpnp_ibb_pd_enable_ctl write register %x failed rc = %d\n",
				REG_IBB_PD_CTL, rc);

	return rc;
}
EXPORT_SYMBOL(qpnp_ibb_pd_enable_ctl);

/** This API is used to set the power up delay for IBB regulator
 * regulator: the reglator device
 * val: the delay in us for power up of IBB regulator
 */
int qpnp_ibb_set_pwrup_dly(struct regulator *regulator, u32 val)
{
	struct qpnp_labibb *labibb;
	int rc = 0;
	u8 reg;

	labibb = regulator_get_drvdata(regulator);


	for (reg = 0; reg < ARRAY_SIZE(ibb_pwrup_dly_plan); reg++)
		if (val == ibb_pwrup_dly_plan[reg])
			break;

	if (reg == ARRAY_SIZE(ibb_pwrup_dly_plan))
		reg = ARRAY_SIZE(ibb_pwrup_dly_plan) - 1;

	mutex_lock(&(labibb->ibb_vreg.ibb_mutex));
	if (labibb->ibb_vreg.pwrup_dly == ibb_pwrup_dly_plan[reg]) {
		rc = 0;
		goto _exit;
	}

	rc = qpnp_labibb_sec_masked_write(labibb, labibb->ibb_base,
				REG_IBB_PWRUP_PWRDN_CTL_1,
				IBB_PWRUP_PWRDN_CTL_1_DLY1_MASK <<
				IBB_PWRUP_PWRDN_CTL_1_DLY1_SHIFT,
				reg << IBB_PWRUP_PWRDN_CTL_1_DLY1_SHIFT,
				1);

	if (rc) {
		pr_err("qpnp_ibb_sec_masked_write register %x failed rc = %d\n",
			REG_IBB_PWRUP_PWRDN_CTL_1, rc);
		goto _exit;
	}

	labibb->ibb_vreg.pwrup_dly = ibb_pwrup_dly_plan[reg];

_exit:
	mutex_unlock(&(labibb->ibb_vreg.ibb_mutex));
	return rc;
}
EXPORT_SYMBOL(qpnp_ibb_set_pwrup_dly);

/** This API is used to set the power down delay for IBB regulator
 * regulator: the reglator device
 * val: the delay in us for power down of IBB regulator
 */
int qpnp_ibb_set_pwrdn_dly(struct regulator *regulator, u32 val)
{
	struct qpnp_labibb *labibb;
	int rc = 0;
	u8 reg;

	labibb = regulator_get_drvdata(regulator);

	for (reg = 0; reg < ARRAY_SIZE(ibb_pwrdn_dly_plan); reg++)
		if (val == ibb_pwrdn_dly_plan[reg])
			break;

	if (reg == ARRAY_SIZE(ibb_pwrdn_dly_plan))
		reg = ARRAY_SIZE(ibb_pwrdn_dly_plan) - 1;

	mutex_lock(&(labibb->ibb_vreg.ibb_mutex));
	if (labibb->ibb_vreg.pwrdn_dly == ibb_pwrdn_dly_plan[reg]) {
		rc = 0;
		goto _exit;
	}

	rc = qpnp_labibb_sec_masked_write(labibb, labibb->ibb_base,
				REG_IBB_PWRUP_PWRDN_CTL_1,
				IBB_PWRUP_PWRDN_CTL_1_DLY2_MASK,
				reg,
				1);

	if (rc) {
		pr_err("qpnp_labibb_sec_masked_write register %x failed rc = %d\n",
			REG_IBB_PWRUP_PWRDN_CTL_1, rc);
		goto _exit;
	}

	labibb->ibb_vreg.pwrdn_dly = ibb_pwrdn_dly_plan[reg];

_exit:
	mutex_unlock(&(labibb->ibb_vreg.ibb_mutex));
	return rc;
}
EXPORT_SYMBOL(qpnp_ibb_set_pwrdn_dly);

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
/** This API is used to set current max of LAB regulator
 * regulator: the reglator device
 * limit: current max of LAB regulator
 */
int qpnp_lab_set_current_max(struct regulator *regulator, u32 limit)
{
	struct qpnp_labibb *labibb;
	int rc = 0;
	u8 reg;

	labibb = regulator_get_drvdata(regulator);

	for (reg = 0; reg < ARRAY_SIZE(lab_current_limit_plan); reg++)
		if (lab_current_limit_plan[reg] == limit)
			break;

	if (reg == ARRAY_SIZE(lab_current_limit_plan))
		reg = ARRAY_SIZE(lab_current_limit_plan) - 1;

	mutex_lock(&(labibb->lab_vreg.lab_mutex));
	rc = qpnp_labibb_masked_write(labibb, labibb->lab_base +
				REG_LAB_CURRENT_LIMIT,
				LAB_CURRENT_LIMIT_MASK,
				reg,
				1);
	if (rc)
		pr_err("%s write register %x failed rc = %d\n",
			__func__, REG_LAB_CURRENT_LIMIT, rc);

	mutex_unlock(&(labibb->lab_vreg.lab_mutex));
	return rc;
}
EXPORT_SYMBOL(qpnp_lab_set_current_max);

/** This API is used to set current max of IBB regulator
 * regulator: the reglator device
 * limit: current max of IBB regulator
 */
int qpnp_ibb_set_current_max(struct regulator *regulator, u32 limit)
{
	struct qpnp_labibb *labibb;
	int rc = 0;
	u8 reg;

	labibb = regulator_get_drvdata(regulator);

	for (reg = 0; reg < ARRAY_SIZE(ibb_current_limit_plan); reg++)
		if (ibb_current_limit_plan[reg] == limit)
			break;

	if (reg == ARRAY_SIZE(ibb_current_limit_plan))
		reg = ARRAY_SIZE(ibb_current_limit_plan) - 1;

	mutex_lock(&(labibb->ibb_vreg.ibb_mutex));

	rc = qpnp_labibb_sec_masked_write(labibb, labibb->ibb_base,
				REG_IBB_CURRENT_LIMIT,
				IBB_CURRENT_LIMIT_MASK,
				reg,
				1);
	if (rc)
		pr_err("%s write secure register %x failed rc = %d\n",
			__func__, REG_IBB_CURRENT_LIMIT, rc);

	mutex_unlock(&(labibb->ibb_vreg.ibb_mutex));
	return rc;
}
EXPORT_SYMBOL(qpnp_ibb_set_current_max);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

static int qpnp_ibb_dt_init(struct qpnp_labibb *labibb,
				struct device_node *of_node)
{
	int rc = 0;
	u32 i, tmp;
	u8 val;

	if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE) {
		if (labibb->mode == QPNP_LABIBB_LCD_MODE)
			val = REG_LAB_IBB_LCD_MODE;
		else
			val = REG_LAB_IBB_AMOLED_MODE;

		rc = qpnp_labibb_sec_write(labibb, labibb->ibb_base,
				REG_LAB_LCD_AMOLED_SEL, &val, 1);

		if (rc) {
			pr_err("qpnp_labibb_sec_write register %x failed rc = %d\n",
				REG_IBB_LCD_AMOLED_SEL, rc);
			return rc;
		}
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-lab-pwrdn-delay",
					&tmp);
	if (rc < 0) {
		pr_err("qcom,qpnp-ibb-lab-pwrdn-delay is missing, rc = %d\n",
			rc);
		return rc;
	}

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
	qpnp_labibb_interrupt_enable_ctl(labibb, false, false, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

	val = 0;

	for (val = 0; val < ARRAY_SIZE(ibb_pwrdn_dly_plan); val++)
		if (ibb_pwrdn_dly_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(ibb_pwrdn_dly_plan)) {
		pr_err("Invalid property in qcom,qpnp-ibb-lab-pwrdn-delay\n");
		return -EINVAL;
	}

	labibb->ibb_vreg.pwrdn_dly = tmp;

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-lab-pwrup-delay",
					&tmp);
	if (rc < 0) {
		pr_err("qcom,qpnp-ibb-lab-pwrup-delay is missing, rc = %d\n",
			rc);
		return rc;
	}

	for (i = 0; i < ARRAY_SIZE(ibb_pwrup_dly_plan); i++)
		if (ibb_pwrup_dly_plan[i] == tmp)
			break;

	if (i == ARRAY_SIZE(ibb_pwrup_dly_plan)) {
		pr_err("Invalid property in qcom,qpnp-ibb-lab-pwrup-delay\n");
		return -EINVAL;
	}

	labibb->ibb_vreg.pwrup_dly = tmp;

	val |= (i << IBB_PWRUP_PWRDN_CTL_1_DLY1_SHIFT);

	if (of_property_read_bool(of_node, "qcom,qpnp-ibb-en-discharge"))
		val |= PWRUP_PWRDN_CTL_1_DISCHARGE_EN;

	if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE)
		val |= (IBB_PWRUP_PWRDN_CTL_1_EN_DLY1 |
				IBB_PWRUP_PWRDN_CTL_1_LAB_VREG_OK);

	rc = qpnp_labibb_sec_write(labibb, labibb->ibb_base,
				REG_IBB_PWRUP_PWRDN_CTL_1,
				&val,
				1);
	if (rc) {
		pr_err("qpnp_labibb_sec_write register %x failed rc = %d\n",
			REG_IBB_PWRUP_PWRDN_CTL_1, rc);
		return rc;
	}

	val = 0;

	if (!of_property_read_bool(of_node, "qcom,qpnp-ibb-full-pull-down"))
		val |= IBB_PD_CTL_HALF_STRENGTH;

	if (of_property_read_bool(of_node, "qcom,qpnp-ibb-pull-down-enable"))
		val |= IBB_PD_CTL_EN;

	rc = qpnp_labibb_write(labibb, labibb->ibb_base + REG_IBB_PD_CTL,
				&val, 1);

	if (rc) {
		pr_err("qpnp_lab_dt_init write register %x failed rc = %d\n",
				REG_IBB_PD_CTL, rc);
		return rc;
	}

	rc = of_property_read_u32(of_node,
			"qcom,qpnp-ibb-switching-clock-frequency", &tmp);
	if (rc) {
		pr_err("get qcom,qpnp-ibb-switching-clock-frequency failed rc = %d\n",
			rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(ibb_clk_div_plan); val++)
		if (ibb_clk_div_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(ibb_clk_div_plan)) {
		pr_err("Invalid property in qpnp-ibb-switching-clock-frequency\n");
		return -EINVAL;
	}

	rc = qpnp_labibb_write(labibb, labibb->ibb_base + REG_IBB_CLK_DIV,
				&val, 1);
	if (rc) {
		pr_err("qpnp_ibb_dt_init write register %x failed rc = %d\n",
			REG_IBB_CLK_DIV, rc);
		return rc;
	}

	rc = of_property_read_u32(of_node,
		"qcom,qpnp-ibb-limit-maximum-current", &tmp);

	if (rc) {
		pr_err("get qcom,qpnp-ibb-limit-maximum-current failed rc = %d\n",
			rc);
		return rc;
	}

	for (val = 0; val < ARRAY_SIZE(ibb_current_limit_plan); val++)
		if (ibb_current_limit_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(ibb_current_limit_plan)) {
		pr_err("Invalid property in qcom,qpnp-ibb-limit-maximum-current\n");
		return -EINVAL;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-debounce-cycle",
		&tmp);

	if (rc) {
		pr_err("get qcom,qpnp-ibb-debounce-cycle failed rc = %d\n",
			rc);
		return rc;
	}

	for (i = 0; i < ARRAY_SIZE(ibb_debounce_plan); i++)
		if (ibb_debounce_plan[i] == tmp)
			break;

	if (i == ARRAY_SIZE(ibb_debounce_plan)) {
		pr_err("Invalid property in qcom,qpnp-ibb-debounce-cycle\n");
		return -EINVAL;
	}

	val |= (i << IBB_CURRENT_LIMIT_DEBOUNCE_SHIFT);

	if (of_property_read_bool(of_node,
		"qcom,qpnp-ibb-limit-max-current-enable"))
		val |= IBB_CURRENT_LIMIT_EN;

	rc = qpnp_labibb_sec_write(labibb, labibb->ibb_base,
					REG_IBB_CURRENT_LIMIT,
					&val,
					1);
	if (rc) {
		pr_err("qpnp_labibb_sec_write register %x failed rc = %d\n",
				REG_IBB_CURRENT_LIMIT, rc);
		return rc;
	}

	if (of_property_read_bool(of_node,
		"qcom,qpnp-ibb-ring-suppression-enable")) {
		val = IBB_RING_SUPPRESSION_CTL_EN;
		rc = qpnp_labibb_write(labibb, labibb->ibb_base +
					REG_IBB_RING_SUPPRESSION_CTL,
					&val,
					1);
		if (rc) {
			pr_err("qpnp_ibb_dt_init write register %x failed rc = %d\n",
				REG_IBB_RING_SUPPRESSION_CTL, rc);
			return rc;
		}
	}

	if (of_property_read_bool(of_node, "qcom,qpnp-ibb-ps-enable")) {
		val = IBB_PS_CTL_EN;
		rc = qpnp_labibb_write(labibb, labibb->ibb_base +
					REG_IBB_PS_CTL,
					&val,
					1);
		if (rc) {
			pr_err("qpnp_ibb_dt_init write register %x failed rc = %d\n",
				REG_IBB_PS_CTL, rc);
			return rc;
		}

		val = IBB_NLIMIT_DAC_EN;
		rc = qpnp_labibb_write(labibb, labibb->ibb_base +
					REG_IBB_NLIMIT_DAC,
					&val,
					1);
		if (rc) {
			pr_err("qpnp_ibb_dt_init write register %x failed rc = %d\n",
				REG_IBB_NLIMIT_DAC, rc);
			return rc;
		}
	} else {
		val = IBB_PS_CTL_DISABLE;
		rc = qpnp_labibb_write(labibb, labibb->ibb_base +
					REG_IBB_PS_CTL,
					&val,
					1);
		if (rc) {
			pr_err("qpnp_ibb_dt_init write register %x failed rc = %d\n",
				REG_IBB_PS_CTL, rc);
			return rc;
		}

		val = IBB_NLIMIT_DAC_DISABLE;
		rc = qpnp_labibb_write(labibb, labibb->ibb_base +
					REG_IBB_NLIMIT_DAC,
					&val,
					1);
		if (rc) {
			pr_err("qpnp_ibb_dt_init write register %x failed rc = %d\n",
				REG_IBB_NLIMIT_DAC, rc);
			return rc;
		}
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-init-voltage",
					&(labibb->ibb_vreg.curr_volt));
	if (rc) {
		pr_err("get qcom,qpnp-ibb-init-voltage failed, rc = %d\n", rc);
		return rc;
	}

	if (!of_property_read_bool(of_node,
			"qcom,qpnp-ibb-use-default-voltage")) {
		if (labibb->ibb_vreg.curr_volt < labibb->ibb_vreg.min_volt) {
			pr_err("Invalid qcom,qpnp-ibb-init-voltage property, qcom,qpnp-ibb-init-voltage %d is less than the the minimum voltage %d",
				labibb->ibb_vreg.curr_volt,
				labibb->ibb_vreg.min_volt);
			return -EINVAL;
		}

		val = DIV_ROUND_UP(labibb->ibb_vreg.curr_volt -
				labibb->ibb_vreg.min_volt,
				labibb->ibb_vreg.step_size);

		if (val > IBB_VOLTAGE_SET_MASK) {
			pr_err("Invalid qcom,qpnp-ibb-init-voltage property, qcom,qpnp-lab-init-voltage %d is larger than the max supported voltage %d",
				labibb->ibb_vreg.curr_volt,
				labibb->ibb_vreg.min_volt +
				labibb->ibb_vreg.step_size *
				IBB_VOLTAGE_SET_MASK);
			return -EINVAL;
		}

		labibb->ibb_vreg.curr_volt = val * labibb->ibb_vreg.step_size +
				labibb->ibb_vreg.min_volt;
		val |= IBB_VOLTAGE_OVERRIDE_EN;
	} else {
		val = 0;
	}

	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
				REG_IBB_VOLTAGE,
				IBB_VOLTAGE_SET_MASK |
				IBB_VOLTAGE_OVERRIDE_EN,
				val,
				1);

	if (rc)
		pr_err("qpnp_ibb_masked_write write register %x failed rc = %d\n",
			REG_IBB_VOLTAGE, rc);


	return rc;
}

static int qpnp_ibb_regulator_enable(struct regulator_dev *rdev)
{
	int rc;
	u8 val;

	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (!labibb->ibb_vreg.vreg_enabled && !labibb->swire_control) {

		if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE)
			return qpnp_labibb_regulator_enable(labibb);

		rc = qpnp_ibb_set_mode(labibb, IBB_SW_CONTROL_EN);
		if (rc) {
			pr_err("Unable to set IBB_MODULE_EN rc = %d\n", rc);
			return rc;
		}

		udelay(labibb->ibb_vreg.soft_start);

		rc = qpnp_labibb_read(labibb, &val,
				labibb->ibb_base + REG_IBB_STATUS1, 1);
		if (rc) {
			pr_err("qpnp_ibb_regulator_enable read register %x failed rc = %d\n",
				REG_IBB_STATUS1, rc);
			return rc;
		}

		if ((val & IBB_STATUS1_VREG_OK_MASK) != IBB_STATUS1_VREG_OK) {
			pr_err("qpnp_ibb_regulator_enable failed\n");
			return -EINVAL;
		}

		labibb->ibb_vreg.vreg_enabled = 1;
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
		qpnp_ibb_interrupt_enable_ctl(labibb, false, true, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
	}
	return 0;
}

static int qpnp_ibb_regulator_disable(struct regulator_dev *rdev)
{
	int rc;
	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (labibb->ibb_vreg.vreg_enabled && !labibb->swire_control) {

		if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE)
			return qpnp_labibb_regulator_disable(labibb);

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
		qpnp_ibb_interrupt_enable_ctl(labibb, false, false, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

		rc = qpnp_ibb_set_mode(labibb, IBB_SW_CONTROL_DIS);
		if (rc) {
			pr_err("Unable to set IBB_MODULE_EN rc = %d\n", rc);
			return rc;
		}

		labibb->ibb_vreg.vreg_enabled = 0;
	}
	return 0;
}

static int qpnp_ibb_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (labibb->swire_control)
		return 0;

	return labibb->ibb_vreg.vreg_enabled;
}

static int qpnp_ibb_regulator_set_voltage(struct regulator_dev *rdev,
				int min_uV, int max_uV, unsigned *selector)
{
	int rc, new_uV;
	u8 val;
	struct qpnp_labibb *labibb = rdev_get_drvdata(rdev);

	if (labibb->swire_control)
		return 0;

	if (min_uV < labibb->ibb_vreg.min_volt) {
		pr_err("qpnp_ibb_regulator_set_voltage failed, min_uV %d is less than min_volt %d",
			min_uV, labibb->ibb_vreg.min_volt);
		return -EINVAL;
	}

	val = DIV_ROUND_UP(min_uV - labibb->ibb_vreg.min_volt,
				labibb->ibb_vreg.step_size);
	new_uV = val * labibb->ibb_vreg.step_size + labibb->ibb_vreg.min_volt;

	if (new_uV > max_uV) {
		pr_err("qpnp_ibb_regulator_set_voltage unable to set voltage (%d %d)\n",
			min_uV, max_uV);
		return -EINVAL;
	}

	rc = qpnp_labibb_masked_write(labibb, labibb->ibb_base +
				REG_IBB_VOLTAGE,
				IBB_VOLTAGE_SET_MASK |
				IBB_VOLTAGE_OVERRIDE_EN,
				val | IBB_VOLTAGE_OVERRIDE_EN,
				1);

	if (rc) {
		pr_err("qpnp_ibb_regulator_set_voltage write register %x failed rc = %d\n",
			REG_IBB_VOLTAGE, rc);

		return rc;
	}

	if (new_uV > labibb->ibb_vreg.curr_volt)
		udelay(val * labibb->ibb_vreg.slew_rate);
	labibb->ibb_vreg.curr_volt = new_uV;

	return 0;
}

static int qpnp_ibb_regulator_get_voltage(struct regulator_dev *rdev)
{
	struct qpnp_labibb *labibb  = rdev_get_drvdata(rdev);

	if (labibb->swire_control)
		return 0;

	return labibb->ibb_vreg.curr_volt;
}

static struct regulator_ops qpnp_ibb_ops = {
	.enable			= qpnp_ibb_regulator_enable,
	.disable		= qpnp_ibb_regulator_disable,
	.is_enabled		= qpnp_ibb_regulator_is_enabled,
	.set_voltage		= qpnp_ibb_regulator_set_voltage,
	.get_voltage		= qpnp_ibb_regulator_get_voltage,
};

static int register_qpnp_ibb_regulator(struct qpnp_labibb *labibb,
					struct device_node *of_node)
{
	int rc = 0;
	struct regulator_init_data *init_data;
	struct regulator_desc *rdesc;
	struct regulator_config cfg = {};
	u8 val, ibb_enable_ctl;
	u32 tmp;

	if (!of_node) {
		dev_err(labibb->dev, "qpnp ibb regulator device tree node is missing\n");
		return -EINVAL;
	}

	init_data = of_get_regulator_init_data(labibb->dev, of_node);
	if (!init_data) {
		pr_err("unable to get regulator init data for qpnp ibb regulator\n");
		return -ENOMEM;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-min-voltage",
					&(labibb->ibb_vreg.min_volt));
	if (rc < 0) {
		pr_err("qcom,qpnp-ibb-min-voltage is missing, rc = %d\n",
			rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-step-size",
					&(labibb->ibb_vreg.step_size));
	if (rc < 0) {
		pr_err("qcom,qpnp-ibb-step-size is missing, rc = %d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-slew-rate",
					&(labibb->ibb_vreg.slew_rate));
	if (rc < 0) {
		pr_err("qcom,qpnp-ibb-slew-rate is missing, rc = %d\n",
			rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-soft-start",
					&(labibb->ibb_vreg.soft_start));
	if (rc < 0) {
		pr_err("qcom,qpnp-ibb-soft-start is missing, rc = %d\n",
			rc);
		return rc;
	}

	rc = of_property_read_u32(of_node, "qcom,qpnp-ibb-discharge-resistor",
			&tmp);

	if (rc < 0) {
		pr_err("qcom,qpnp-ibb-discharge-resistor is missing, rc = %d\n",
			rc);
		return rc;
	}

	if (labibb->mode == QPNP_LABIBB_AMOLED_MODE) {
		/*
		 * AMOLED mode needs ibb discharge resistor to be
		 * configured for 300KOhm
		 */
		if (tmp < ibb_discharge_resistor_plan[0])
			tmp = ibb_discharge_resistor_plan[0];
	}

	for (val = 0; val < ARRAY_SIZE(ibb_discharge_resistor_plan); val++)
		if (ibb_discharge_resistor_plan[val] == tmp)
			break;

	if (val == ARRAY_SIZE(ibb_discharge_resistor_plan)) {
		pr_err("Invalid property in qcom,qpnp-ibb-discharge-resistor\n");
		return -EINVAL;
	}

	rc = qpnp_labibb_write(labibb, labibb->ibb_base +
			REG_IBB_SOFT_START_CTL, &val, 1);
	if (rc) {
		pr_err("qpnp_labibb_write register %x failed rc = %d\n",
			REG_IBB_SOFT_START_CTL, rc);
		return rc;
	}

	rc = qpnp_labibb_read(labibb, &ibb_enable_ctl,
				labibb->ibb_base + REG_IBB_ENABLE_CTL, 1);
	if (rc) {
		pr_err("qpnp_ibb_read register %x failed rc = %d\n",
			REG_IBB_ENABLE_CTL, rc);
		return rc;
	}

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
	if (display_on_in_boot && !(ibb_enable_ctl & IBB_ENABLE_CTL_MODULE_EN))
		ibb_enable_ctl &= IBB_ENABLE_CTL_MODULE_EN;
#endif

	if (ibb_enable_ctl &
		(IBB_ENABLE_CTL_SWIRE_RDY | IBB_ENABLE_CTL_MODULE_EN)) {
		/* SWIRE_RDY or IBB_MODULE_EN enabled */
		rc = qpnp_labibb_read(labibb, &val,
			labibb->ibb_base + REG_IBB_LCD_AMOLED_SEL, 1);
		if (rc) {
			pr_err("qpnp_labibb_read register %x failed rc = %d\n",
				REG_IBB_LCD_AMOLED_SEL, rc);
			return rc;
		}

		if (val == REG_LAB_IBB_AMOLED_MODE)
			labibb->mode = QPNP_LABIBB_AMOLED_MODE;
		else
			labibb->mode = QPNP_LABIBB_LCD_MODE;

		rc = qpnp_labibb_read(labibb, &val,
				labibb->ibb_base + REG_IBB_VOLTAGE, 1);
		if (rc) {
			pr_err("qpnp_labibb_read read register %x failed rc = %d\n",
				REG_IBB_VOLTAGE, rc);
			return rc;
		}

		if (val & IBB_VOLTAGE_OVERRIDE_EN) {
			labibb->ibb_vreg.curr_volt =
				(val & IBB_VOLTAGE_SET_MASK) *
				labibb->ibb_vreg.step_size +
				labibb->ibb_vreg.min_volt;
		} else if (labibb->mode == QPNP_LABIBB_LCD_MODE) {
			rc = of_property_read_u32(of_node,
				"qcom,qpnp-ibb-init-lcd-voltage",
				&(labibb->ibb_vreg.curr_volt));
			if (rc) {
				pr_err("get qcom,qpnp-ibb-init-lcd-voltage failed, rc = %d\n",
					rc);
				return rc;
			}
		} else {
			rc = of_property_read_u32(of_node,
				"qcom,qpnp-ibb-init-amoled-voltage",
				&(labibb->ibb_vreg.curr_volt));
			if (rc) {
				pr_err("get qcom,qpnp-ibb-init-amoled-voltage failed, rc = %d\n",
					rc);
				return rc;
			}

		}

		rc = qpnp_labibb_read(labibb, &val, labibb->ibb_base +
					REG_IBB_PWRUP_PWRDN_CTL_1, 1);
		if (rc) {
			pr_err("qpnp_labibb_config_init read register %x failed rc = %d\n",
				REG_IBB_PWRUP_PWRDN_CTL_1, rc);
			return rc;
		}

		labibb->ibb_vreg.pwrup_dly = ibb_pwrup_dly_plan[
					(val >>
					IBB_PWRUP_PWRDN_CTL_1_DLY1_SHIFT) &
					IBB_PWRUP_PWRDN_CTL_1_DLY1_MASK];
		labibb->ibb_vreg.pwrdn_dly =  ibb_pwrdn_dly_plan[val &
					IBB_PWRUP_PWRDN_CTL_1_DLY2_MASK];

		labibb->ibb_vreg.vreg_enabled = 1;
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
		/* request interrupt */
		rc = qpnp_ibb_request_interrupt(labibb);
		if (rc) {
			pr_err("ibb request interrupt failed rc=%d\n",
								rc);
			return rc;
		}
		qpnp_ibb_interrupt_enable_ctl(labibb, true, true, false);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
	} else {
		/* SWIRE_RDY and IBB_MODULE_EN not enabled */
		rc = qpnp_ibb_dt_init(labibb, of_node);
		if (rc) {
			pr_err("qpnp-ibb: wrong DT parameter specified: rc = %d\n",
				rc);
			return rc;
		}
	}
	rc = qpnp_labibb_read(labibb, &val,
			labibb->ibb_base + REG_IBB_MODULE_RDY, 1);
	if (rc) {
		pr_err("qpnp_ibb_read read register %x failed rc = %d\n",
			REG_IBB_MODULE_RDY, rc);
		return rc;
	}

	if (labibb->mode != QPNP_LABIBB_STANDALONE_MODE &&
			!(val & IBB_MODULE_RDY_EN)) {
		val = IBB_MODULE_RDY_EN;

		rc = qpnp_labibb_write(labibb, labibb->ibb_base +
			REG_IBB_MODULE_RDY, &val, 1);

		if (rc) {
			pr_err("qpnp_ibb_dt_init write register %x failed rc = %d\n",
				REG_IBB_MODULE_RDY, rc);
			return rc;
		}
	}

	if (init_data->constraints.name) {
		rdesc			= &(labibb->ibb_vreg.rdesc);
		rdesc->owner		= THIS_MODULE;
		rdesc->type		= REGULATOR_VOLTAGE;
		rdesc->ops		= &qpnp_ibb_ops;
		rdesc->name		= init_data->constraints.name;

		cfg.dev = labibb->dev;
		cfg.init_data = init_data;
		cfg.driver_data = labibb;
		cfg.of_node = of_node;

		init_data->constraints.valid_ops_mask
				|= REGULATOR_CHANGE_VOLTAGE |
					REGULATOR_CHANGE_STATUS;

		labibb->ibb_vreg.rdev = regulator_register(rdesc, &cfg);
		if (IS_ERR(labibb->ibb_vreg.rdev)) {
			rc = PTR_ERR(labibb->ibb_vreg.rdev);
			labibb->ibb_vreg.rdev = NULL;
			pr_err("unable to get regulator init data for qpnp ibb regulator, rc = %d\n",
				rc);

			return rc;
		}
	} else {
		dev_err(labibb->dev, "qpnp ibb regulator name missing\n");
		return -EINVAL;
	}

	mutex_init(&(labibb->ibb_vreg.ibb_mutex));
	return 0;
}

static int qpnp_labibb_check_ttw_supported(struct qpnp_labibb *labibb)
{
	int rc = 0;
	u8 val;

	switch (labibb->pmic_rev_id->pmic_subtype) {
	case PMI8996:
		rc = qpnp_labibb_read(labibb, &val,
				labibb->ibb_base + REG_IBB_REVISION4, 1);
		if (rc) {
			pr_err("qpnp_labibb_read register %x failed rc = %d\n",
				REG_IBB_REVISION4, rc);
			return rc;
		}

		/* PMI8996 has revision 1 */
		if (val < 1) {
			pr_err("TTW feature cannot be enabled for revision %d\n",
									val);
			labibb->ttw_en = false;
		}
		/* FORCE_LAB_ON in TTW is not required for PMI8996 */
		labibb->ttw_force_lab_on = false;
		break;
	case PMI8950:
		/* TTW supported for all revisions */
		break;
	default:
		pr_info("TTW mode not supported for PMIC-subtype = %d\n",
					labibb->pmic_rev_id->pmic_subtype);
		labibb->ttw_en = false;
		break;

	}
	return rc;
}

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
static irqreturn_t lab_vreg_handler(int irq, void *_chip)
{
	u8 val;
	int rc;
	struct qpnp_labibb *labibb = _chip;

	pr_err("%s: LAB VREG_NG interrupt!\n", __func__);
	qpnp_labibb_interrupt_enable_ctl(labibb, false, false, false);

	if (!labibb->lab_vreg.vreg_enabled) {
		pr_err("%s: false detection display is off\n", __func__);
		goto exit;
	}

	/* lab status1 */
	rc = qpnp_labibb_read(labibb, &val,
			labibb->lab_base + REG_LAB_STATUS1, 1);
	if (rc) {
		pr_err("%s: read register %x failed rc = %d\n",
				__func__, REG_LAB_STATUS1, rc);
		goto exit;
	}

	if (!(val & LAB_STATUS1_VREG_OK))
		pr_err("%s: LAB VREG NG!!!\n", __func__);
	else
		goto false_detection;

	/* start vreg check */
	if (labibb_vreg_check.vreg_check_working)
		goto exit;

	labibb_vreg_check.vreg_check_working = true;
	schedule_delayed_work(&labibb_vreg_check.vreg_check_work,
		msecs_to_jiffies(
			labibb_vreg_check.target_chatter_check_interval));
	goto exit;

false_detection:
	rc = qpnp_labibb_interrupt_enable_ctl(labibb, false, true, false);
	if (rc)
		pr_err("%s: qpnp_labibb_interrupt_enable_ctl error\n",
							__func__);
exit:
	return IRQ_HANDLED;
}

static irqreturn_t ibb_vreg_handler(int irq, void *_chip)
{
	u8 val;
	int rc;
	struct qpnp_labibb *labibb = _chip;

	pr_err("%s: IBB VREG_NG interrupt!\n", __func__);
	qpnp_labibb_interrupt_enable_ctl(labibb, false, false, false);

	if (!labibb->ibb_vreg.vreg_enabled) {
		pr_err("%s: false detection display is off\n", __func__);
		goto exit;
	}

	/* ibb status1 */
	rc = qpnp_labibb_read(labibb, &val,
			labibb->ibb_base + REG_IBB_STATUS1, 1);
	if (rc) {
		pr_err("%s: read register %x failed rc = %d\n",
				__func__, REG_IBB_STATUS1, rc);
		goto exit;
	}

	if (!(val & IBB_STATUS1_VREG_OK))
		pr_err("%s: IBB VREG NG!!!\n", __func__);
	else
		goto false_detection;

	/* start vreg check */
	if (labibb_vreg_check.vreg_check_working)
		goto exit;

	labibb_vreg_check.vreg_check_working = true;
	schedule_delayed_work(&labibb_vreg_check.vreg_check_work,
		msecs_to_jiffies(
			labibb_vreg_check.target_chatter_check_interval));
	goto exit;

false_detection:
	rc = qpnp_labibb_interrupt_enable_ctl(labibb, false, true, false);
	if (rc)
		pr_err("%s: qpnp_labibb_interrupt_enable_ctl error\n",
							__func__);

exit:
	return IRQ_HANDLED;
}
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

static int qpnp_labibb_regulator_probe(struct spmi_device *spmi)
{
	struct qpnp_labibb *labibb;
	struct resource *resource;
	struct spmi_resource *spmi_resource;
	struct device_node *revid_dev_node;
	const char *mode_name;
	u8 type;
	int rc = 0;

	labibb = devm_kzalloc(&spmi->dev,
			sizeof(struct qpnp_labibb), GFP_KERNEL);
	if (labibb == NULL) {
		pr_err("labibb allocation failed.\n");
		return -ENOMEM;
	}

	labibb->dev = &(spmi->dev);
	labibb->spmi = spmi;

	revid_dev_node = of_parse_phandle(spmi->dev.of_node,
					"qcom,pmic-revid", 0);
	if (!revid_dev_node) {
		pr_err("Missing qcom,pmic-revid property - driver failed\n");
		return -EINVAL;
	}

	labibb->pmic_rev_id = get_revid_data(revid_dev_node);
	if (IS_ERR(labibb->pmic_rev_id)) {
		pr_debug("Unable to get revid data\n");
		return -EPROBE_DEFER;
	}

	rc = of_property_read_string(labibb->dev->of_node,
			"qpnp,qpnp-labibb-mode", &mode_name);
	if (!rc) {
		if (strcmp("lcd", mode_name) == 0) {
			labibb->mode = QPNP_LABIBB_LCD_MODE;
		} else if (strcmp("amoled", mode_name) == 0) {
			labibb->mode = QPNP_LABIBB_AMOLED_MODE;
		} else if (strcmp("stand-alone", mode_name) == 0) {
			labibb->mode = QPNP_LABIBB_STANDALONE_MODE;
		} else {
			pr_err("Invalid device property in qpnp,qpnp-labibb-mode: %s\n",
				mode_name);
			return -EINVAL;
		}
	} else {
		pr_err("qpnp_labibb: qpnp,qpnp-labibb-mode is missing.\n");
		return rc;
	}

	labibb->ttw_en = of_property_read_bool(labibb->dev->of_node,
				"qcom,labibb-touch-to-wake-en");
	if (labibb->ttw_en && labibb->mode != QPNP_LABIBB_LCD_MODE) {
		pr_err("Invalid mode for TTW\n");
		return -EINVAL;
	}

	labibb->ttw_force_lab_on = of_property_read_bool(
		labibb->dev->of_node, "qcom,labibb-ttw-force-lab-on");

	labibb->swire_control = of_property_read_bool(labibb->dev->of_node,
							"qpnp,swire-control");
	if (labibb->swire_control && labibb->mode != QPNP_LABIBB_AMOLED_MODE) {
		pr_err("Invalid mode for SWIRE control\n");
		return -EINVAL;
	}

#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
	/* initialize labibb_vreg_status_ctrl */
	labibb_vreg_check.labibb = labibb;
	labibb_vreg_check.current_chatter_cnt = 0;
	labibb_vreg_check.vreg_check_working = false;
	labibb_vreg_check.target_chatter_cnt = DEFAULT_TARGET_CHATTER_CNT;
	rc = of_property_read_u32(labibb->dev->of_node,
		"somc,vreg-target-chatter-cnt",
		&(labibb_vreg_check.target_chatter_cnt));
	if (rc)
		pr_err("qpnp_labibb: vreg-target-chatter-cnt is missing.\n");

	labibb_vreg_check.target_chatter_check_interval
					= DEFAULT_TARGET_CHATTER_INTERVAL;
	rc = of_property_read_u32(labibb->dev->of_node,
		"somc,vreg-target-chatter-interval",
		&(labibb_vreg_check.target_chatter_check_interval));
	if (rc)
		pr_err("qpnp_labibb: vreg-target-chatter-interval is missing.\n");

	/* initialize vreg_check_worker */
	INIT_DELAYED_WORK(&labibb_vreg_check.vreg_check_work,
				vreg_check_worker);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */

	spmi_for_each_container_dev(spmi_resource, spmi) {
		if (!spmi_resource) {
			pr_err("qpnp_labibb: spmi resource absent\n");
			return -ENXIO;
		}

		resource = spmi_get_resource(spmi, spmi_resource,
						IORESOURCE_MEM, 0);
		if (!(resource && resource->start)) {
			pr_err("node %s IO resource absent!\n",
				spmi->dev.of_node->full_name);
			return -ENXIO;
		}

		rc = qpnp_labibb_read(labibb, &type,
				resource->start + REG_PERPH_TYPE, 1);
		if (rc) {
			pr_err("Peripheral type read failed rc=%d\n", rc);
			goto fail_registration;
		}

		switch (type) {
		case QPNP_LAB_TYPE:
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
			labibb->lab_spmi_res = spmi_resource;
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
			labibb->lab_base = resource->start;
			rc = register_qpnp_lab_regulator(labibb,
				spmi_resource->of_node);
			if (rc)
				goto fail_registration;
		break;

		case QPNP_IBB_TYPE:
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
			labibb->ibb_spmi_res = spmi_resource;
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
			labibb->ibb_base = resource->start;
			rc = register_qpnp_ibb_regulator(labibb,
				spmi_resource->of_node);
			if (rc)
				goto fail_registration;
		break;

		default:
			pr_err("qpnp_labibb: unknown peripheral type %x\n",
				type);
			rc = -EINVAL;
			goto fail_registration;
		}
	}

	if (labibb->ttw_en) {
		rc = qpnp_labibb_check_ttw_supported(labibb);
		if (rc) {
			pr_err("pmic revision check failed for TTW rc=%d\n",
									rc);
			goto fail_registration;
		}
	}

	dev_set_drvdata(&spmi->dev, labibb);
	return 0;

fail_registration:
	if (labibb->lab_vreg.rdev)
		regulator_unregister(labibb->lab_vreg.rdev);
	if (labibb->ibb_vreg.rdev)
		regulator_unregister(labibb->ibb_vreg.rdev);
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
	if (labibb->lab_vreg_irq)
		free_irq(labibb->lab_vreg_irq, labibb);
	if (labibb->ibb_vreg_irq)
		free_irq(labibb->ibb_vreg_irq, labibb);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
	return rc;
}

static int qpnp_labibb_regulator_remove(struct spmi_device *spmi)
{
	struct qpnp_labibb *labibb = dev_get_drvdata(&spmi->dev);

	if (labibb) {
		if (labibb->lab_vreg.rdev)
			regulator_unregister(labibb->lab_vreg.rdev);
		if (labibb->ibb_vreg.rdev)
			regulator_unregister(labibb->ibb_vreg.rdev);
#ifdef SOMC_LABIBB_REGULATOR_ORG_IMPL
		if (labibb->lab_vreg_irq)
			free_irq(labibb->lab_vreg_irq, labibb);
		if (labibb->ibb_vreg_irq)
			free_irq(labibb->ibb_vreg_irq, labibb);
#endif /* SOMC_LABIBB_REGULATOR_ORG_IMPL */
	}
	return 0;
}

static struct of_device_id spmi_match_table[] = {
	{ .compatible = QPNP_LABIBB_REGULATOR_DRIVER_NAME, },
	{ },
};

static struct spmi_driver qpnp_labibb_regulator_driver = {
	.driver		= {
		.name	= QPNP_LABIBB_REGULATOR_DRIVER_NAME,
		.of_match_table = spmi_match_table,
	},
	.probe		= qpnp_labibb_regulator_probe,
	.remove		= qpnp_labibb_regulator_remove,
};

static int __init qpnp_labibb_regulator_init(void)
{
	return spmi_driver_register(&qpnp_labibb_regulator_driver);
}
arch_initcall(qpnp_labibb_regulator_init);

static void __exit qpnp_labibb_regulator_exit(void)
{
	spmi_driver_unregister(&qpnp_labibb_regulator_driver);
}
module_exit(qpnp_labibb_regulator_exit);

MODULE_DESCRIPTION("QPNP labibb driver");
MODULE_LICENSE("GPL v2");
