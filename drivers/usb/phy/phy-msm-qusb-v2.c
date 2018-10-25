/*
 * Copyright (c) 2014-2018, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/usb/phy.h>
#include <linux/reset.h>
#include <linux/nvmem-consumer.h>
#include <linux/debugfs.h>
#include <linux/hrtimer.h>

/* QUSB2PHY_PWR_CTRL1 register related bits */
#define PWR_CTRL1_POWR_DOWN		BIT(0)

/* QUSB2PHY_PLL_COMMON_STATUS_ONE register related bits */
#define CORE_READY_STATUS		BIT(0)

/* Get TUNE value from efuse bit-mask */
#define TUNE_VAL_MASK(val, pos, mask)	((val >> pos) & mask)

/* QUSB2PHY_INTR_CTRL register related bits */
#define DMSE_INTR_HIGH_SEL              BIT(4)
#define DPSE_INTR_HIGH_SEL              BIT(3)
#define CHG_DET_INTR_EN                 BIT(2)
#define DMSE_INTR_EN                    BIT(1)
#define DPSE_INTR_EN                    BIT(0)

/* QUSB2PHY_PLL_CORE_INPUT_OVERRIDE register related bits */
#define CORE_PLL_RATE			BIT(0)
#define CORE_PLL_RATE_MUX		BIT(1)
#define CORE_PLL_EN			BIT(2)
#define CORE_PLL_EN_MUX			BIT(3)
#define CORE_PLL_EN_FROM_RESET		BIT(4)
#define CORE_RESET			BIT(5)
#define CORE_RESET_MUX			BIT(6)

#define QUSB2PHY_1P8_VOL_MIN           1800000 /* uV */
#define QUSB2PHY_1P8_VOL_MAX           1800000 /* uV */
#define QUSB2PHY_1P8_HPM_LOAD          30000   /* uA */

#define QUSB2PHY_3P3_VOL_MIN		3075000 /* uV */
#define QUSB2PHY_3P3_VOL_MAX		3200000 /* uV */
#define QUSB2PHY_3P3_HPM_LOAD		30000	/* uA */

#define LINESTATE_DP			BIT(0)
#define LINESTATE_DM			BIT(1)

#define BIAS_CTRL_2_OVERRIDE_VAL	0x28

#define SQ_CTRL1_CHIRP_DISABLE		0x20
#define SQ_CTRL2_CHIRP_DISABLE		0x80

#define DEBUG_CTRL1_OVERRIDE_VAL	0x09

/* PERIPH_SS_PHY_REFGEN_NORTH_BG_CTRL register bits */
#define BANDGAP_BYPASS			BIT(0)

enum qusb_phy_reg {
	PORT_TUNE1,
	PLL_COMMON_STATUS_ONE,
	PWR_CTRL1,
	INTR_CTRL,
	PLL_CORE_INPUT_OVERRIDE,
	TEST1,
	BIAS_CTRL_2,
	SQ_CTRL1,
	SQ_CTRL2,
	DEBUG_CTRL1,
	USB2_PHY_REG_MAX,
};

#ifdef CONFIG_MSM_USB_PHY_SOMC_EXT
#define QUSB2PHY_PORT_TUNE2		0x240
#define QUSB2PHY_PORT_TUNE3		0x244
#define QUSB2PHY_PORT_TUNE4		0x248

#define USB_PHY_HSTX_TRIM		0xF0	/* TUNE1 7:4 */
#define USB_PHY_HSTX_SR			0x0C	/* TUNE1 3:2 */
#define USB_PHY_HSTX_SR_BIAS		0x03	/* TUNE1 1:0 */
#define USB_PHY_SEL_EMPH_HALF_WIDTH	0x10	/* TUNE2   4 */
#define USB_PHY_EN_EMPHASIS		0x0C	/* TUNE2 3:2 */
#define USB_PHY_HS_DISCON_TRIM		0x03	/* TUNE2 1:0 */
#define USB_PHY_CDR_WIDE		0x80	/* TUNE3   7 */
#define USB_PHY_CDR_PULSE_SMPL		0x40	/* TUNE3   6 */
#define USB_PHY_TX2RX_DLY		0x3F	/* TUNE3 5:0 */
#define USB_PHY_HANDOFF_PHSEL		0xE0	/* TUNE4 7:5 */
#define USB_PHY_FORCE_HSRX_ALWAYS_ON	0x10	/* TUNE4   4 */
#define USB_PHY_SQ_FILTER_DIS		0x08	/* TUNE4   3 */
#define USB_PHY_SQ_LEVEL		0x07	/* TUNE4 2:0 */

unsigned int phy_tune2;
unsigned int phy_tune3;
unsigned int phy_tune4;
unsigned int phy_host_tune1;
unsigned int phy_host_tune2;

module_param(phy_tune2, uint, S_IRUGO | S_IWUSR);
module_param(phy_tune3, uint, S_IRUGO | S_IWUSR);
module_param(phy_tune4, uint, S_IRUGO | S_IWUSR);
module_param(phy_host_tune1, uint, S_IRUGO | S_IWUSR);
module_param(phy_host_tune2, uint, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(phy_tune2, "QUSB PHY v2 TUNE2");
MODULE_PARM_DESC(phy_tune3, "QUSB PHY v2 TUNE3");
MODULE_PARM_DESC(phy_tune4, "QUSB PHY v2 TUNE4");
MODULE_PARM_DESC(phy_host_tune1, "QUSB PHY HOST v2 TUNE1");
MODULE_PARM_DESC(phy_host_tune2, "QUSB PHY HOST v2 TUNE2");
#endif /* CONFIG_MSM_USB_PHY_SOMC_EXT */

struct qusb_phy {
	struct usb_phy		phy;
	struct mutex		lock;
	void __iomem		*base;
	void __iomem		*efuse_reg;
	void __iomem		*tcsr_clamp_dig_n;
	void __iomem		*refgen_north_bg_reg;

	struct clk		*ref_clk_src;
	struct clk		*ref_clk;
	struct clk		*cfg_ahb_clk;
	struct reset_control	*phy_reset;

	struct regulator	*vdd;
	struct regulator	*vdda33;
	struct regulator	*vdda18;
	int			vdd_levels[3]; /* none, low, high */
	int			init_seq_len;
	int			*qusb_phy_init_seq;
	int			host_init_seq_len;
	int			*qusb_phy_host_init_seq;

	unsigned int		*phy_reg;
	int			qusb_phy_reg_offset_cnt;

	u32			tune_val;
	int			efuse_bit_pos;
	int			efuse_num_of_bits;

#ifdef CONFIG_MSM_USB_PHY_SOMC_EXT
	u32			host_tune_val;
	u32			efuse_offset;
#endif /* CONFIG_MSM_USB_PHY_SOMC_EXT */

	int			power_enabled_ref;
	bool			clocks_enabled;
	bool			cable_connected;
	bool			suspended;
	bool			dpdm_enable;

	struct regulator_desc	dpdm_rdesc;
	struct regulator_dev	*dpdm_rdev;

	u32			sq_ctrl1_default;
	u32			sq_ctrl2_default;
	bool			chirp_disable;

	struct pinctrl		*pinctrl;
	struct pinctrl_state	*atest_usb13_suspend;
	struct pinctrl_state	*atest_usb13_active;

	/* emulation targets specific */
	void __iomem		*emu_phy_base;
	bool			emulation;
	int			*emu_init_seq;
	int			emu_init_seq_len;
	int			*phy_pll_reset_seq;
	int			phy_pll_reset_seq_len;
	int			*emu_dcm_reset_seq;
	int			emu_dcm_reset_seq_len;

	/* override TUNEX registers value */
	struct dentry		*root;
	u8			tune[5];
	u8                      bias_ctrl2;

	struct hrtimer		timer;
	int			soc_min_rev;
	bool			host_chirp_erratum;
	bool			override_bias_ctrl2;
};

#ifdef CONFIG_NVMEM
/* Parse qfprom data for deciding on errata work-arounds */
static long qfprom_read(struct device *dev, const char *name)
{
	struct nvmem_cell *cell;
	ssize_t len = 0;
	u32 *buf, val = 0;
	long err = 0;

	cell = nvmem_cell_get(dev, name);
	if (IS_ERR(cell)) {
		err = PTR_ERR(cell);
		dev_err(dev, "failed opening nvmem cell err : %ld\n", err);
		/* If entry does not exist, then that is not an error */
		if (err == -ENOENT)
			err = 0;
		return err;
	}

	buf = (u32 *)nvmem_cell_read(cell, &len);
	if (IS_ERR(buf) || !len) {
		dev_err(dev, "Failed reading nvmem cell, err: %u, bytes fetched: %zd\n",
				*buf, len);
		if (!IS_ERR(buf)) {
			kfree(buf);
			err = -EINVAL;
		} else {
			err = PTR_ERR(buf);
		}
	} else {
		/*
		 * The bits are read from bit-0 to bit-29
		 * We're interested in bits 28:29
		 */
		val = (*buf >> 28) & 0x3;
		kfree(buf);
	}

	nvmem_cell_put(cell);
	return err ? err : (long) val;
}

/* Reads the SoC version */
static int qusb_phy_get_socrev(struct device *dev, struct qusb_phy *qphy)
{
	if (!qphy->host_chirp_erratum)
		return 0;

	qphy->soc_min_rev  = qfprom_read(dev, "minor_rev");
	if (qphy->soc_min_rev < 0)
		dev_err(dev, "failed getting soc_min_rev, err : %d\n",
				qphy->soc_min_rev);

	return qphy->soc_min_rev;
};
#else
/* Reads the SoC version */
static int qusb_phy_get_socrev(struct device *dev, struct qusb_phy *qphy)
{
	return 0;
}
#endif

static void qusb_phy_enable_clocks(struct qusb_phy *qphy, bool on)
{
	dev_dbg(qphy->phy.dev, "%s(): clocks_enabled:%d on:%d\n",
			__func__, qphy->clocks_enabled, on);

	if (!qphy->clocks_enabled && on) {
		clk_prepare_enable(qphy->ref_clk_src);
		if (qphy->ref_clk)
			clk_prepare_enable(qphy->ref_clk);

		if (qphy->cfg_ahb_clk)
			clk_prepare_enable(qphy->cfg_ahb_clk);

		qphy->clocks_enabled = true;
	}

	if (qphy->clocks_enabled && !on) {
		if (qphy->cfg_ahb_clk)
			clk_disable_unprepare(qphy->cfg_ahb_clk);

		if (qphy->ref_clk)
			clk_disable_unprepare(qphy->ref_clk);

		clk_disable_unprepare(qphy->ref_clk_src);
		qphy->clocks_enabled = false;
	}

	dev_dbg(qphy->phy.dev, "%s(): clocks_enabled:%d\n", __func__,
						qphy->clocks_enabled);
}

static int qusb_phy_config_vdd(struct qusb_phy *qphy, int high)
{
	int min, ret;

	min = high ? 1 : 0; /* low or none? */
	ret = regulator_set_voltage(qphy->vdd, qphy->vdd_levels[min],
						qphy->vdd_levels[2]);
	if (ret) {
		dev_err(qphy->phy.dev, "unable to set voltage for qusb vdd\n");
		return ret;
	}

	dev_dbg(qphy->phy.dev, "min_vol:%d max_vol:%d\n",
			qphy->vdd_levels[min], qphy->vdd_levels[2]);
	return ret;
}

static int qusb_phy_enable_power(struct qusb_phy *qphy, bool on)
{
	int ret = 0;

	mutex_lock(&qphy->lock);

	dev_dbg(qphy->phy.dev,
		"%s:req to turn %s regulators. power_enabled_ref:%d\n",
			__func__, on ? "on" : "off", qphy->power_enabled_ref);

	if (on && ++qphy->power_enabled_ref > 1) {
		dev_dbg(qphy->phy.dev, "PHYs' regulators are already on\n");
		goto done;
	}

	if (!on) {
		if (on == qphy->power_enabled_ref) {
			dev_dbg(qphy->phy.dev,
				"PHYs' regulators are already off\n");
			goto done;
		}

		qphy->power_enabled_ref--;
		if (!qphy->power_enabled_ref)
			goto disable_vdda33;

		dev_dbg(qphy->phy.dev, "Skip turning off PHYs' regulators\n");
		goto done;
	}

	ret = qusb_phy_config_vdd(qphy, true);
	if (ret) {
		dev_err(qphy->phy.dev, "Unable to config VDD:%d\n",
							ret);
		goto err_vdd;
	}

	ret = regulator_enable(qphy->vdd);
	if (ret) {
		dev_err(qphy->phy.dev, "Unable to enable VDD\n");
		goto unconfig_vdd;
	}

	ret = regulator_set_load(qphy->vdda18, QUSB2PHY_1P8_HPM_LOAD);
	if (ret < 0) {
		dev_err(qphy->phy.dev, "Unable to set HPM of vdda18:%d\n", ret);
		goto disable_vdd;
	}

	ret = regulator_set_voltage(qphy->vdda18, QUSB2PHY_1P8_VOL_MIN,
						QUSB2PHY_1P8_VOL_MAX);
	if (ret) {
		dev_err(qphy->phy.dev,
				"Unable to set voltage for vdda18:%d\n", ret);
		goto put_vdda18_lpm;
	}

	ret = regulator_enable(qphy->vdda18);
	if (ret) {
		dev_err(qphy->phy.dev, "Unable to enable vdda18:%d\n", ret);
		goto unset_vdda18;
	}

	ret = regulator_set_load(qphy->vdda33, QUSB2PHY_3P3_HPM_LOAD);
	if (ret < 0) {
		dev_err(qphy->phy.dev, "Unable to set HPM of vdda33:%d\n", ret);
		goto disable_vdda18;
	}

	ret = regulator_set_voltage(qphy->vdda33, QUSB2PHY_3P3_VOL_MIN,
						QUSB2PHY_3P3_VOL_MAX);
	if (ret) {
		dev_err(qphy->phy.dev,
				"Unable to set voltage for vdda33:%d\n", ret);
		goto put_vdda33_lpm;
	}

	ret = regulator_enable(qphy->vdda33);
	if (ret) {
		dev_err(qphy->phy.dev, "Unable to enable vdda33:%d\n", ret);
		goto unset_vdd33;
	}

	pr_debug("%s(): QUSB PHY's regulators are turned ON.\n", __func__);

	mutex_unlock(&qphy->lock);
	return ret;

disable_vdda33:
	ret = regulator_disable(qphy->vdda33);
	if (ret)
		dev_err(qphy->phy.dev, "Unable to disable vdda33:%d\n", ret);

unset_vdd33:
	ret = regulator_set_voltage(qphy->vdda33, 0, QUSB2PHY_3P3_VOL_MAX);
	if (ret)
		dev_err(qphy->phy.dev,
			"Unable to set (0) voltage for vdda33:%d\n", ret);

put_vdda33_lpm:
	ret = regulator_set_load(qphy->vdda33, 0);
	if (ret < 0)
		dev_err(qphy->phy.dev, "Unable to set (0) HPM of vdda33\n");

disable_vdda18:
	ret = regulator_disable(qphy->vdda18);
	if (ret)
		dev_err(qphy->phy.dev, "Unable to disable vdda18:%d\n", ret);

unset_vdda18:
	ret = regulator_set_voltage(qphy->vdda18, 0, QUSB2PHY_1P8_VOL_MAX);
	if (ret)
		dev_err(qphy->phy.dev,
			"Unable to set (0) voltage for vdda18:%d\n", ret);

put_vdda18_lpm:
	ret = regulator_set_load(qphy->vdda18, 0);
	if (ret < 0)
		dev_err(qphy->phy.dev, "Unable to set LPM of vdda18\n");

disable_vdd:
	ret = regulator_disable(qphy->vdd);
	if (ret)
		dev_err(qphy->phy.dev, "Unable to disable vdd:%d\n",
							ret);

unconfig_vdd:
	ret = qusb_phy_config_vdd(qphy, false);
	if (ret)
		dev_err(qphy->phy.dev, "Unable unconfig VDD:%d\n",
							ret);
err_vdd:
	dev_dbg(qphy->phy.dev, "QUSB PHY's regulators are turned OFF.\n");

	/* in case of error in turning on regulators */
	if (qphy->power_enabled_ref)
		qphy->power_enabled_ref--;
done:
	mutex_unlock(&qphy->lock);
	return ret;
}

#ifdef CONFIG_MSM_USB_PHY_SOMC_EXT
static u32 qusb_phy_get_tune1_param(struct qusb_phy *qphy)
{
	u8 reg;
	u32 bit_mask = 1;
	u32 tune_val;

	pr_debug("%s(): num_of_bits:%d bit_pos:%d offset:%d\n", __func__,
 				qphy->efuse_num_of_bits,
				qphy->efuse_bit_pos,
				qphy->efuse_offset);

	/* get bit mask based on number of bits to use with efuse reg */
	bit_mask = (bit_mask << qphy->efuse_num_of_bits) - 1;

	/*
	 * if efuse reg is updated (i.e non-zero) then use it to program
	 * tune parameters
	 */
	tune_val = readl_relaxed(qphy->efuse_reg);
 	pr_debug("%s(): bit_mask:%d efuse based tune1 value:%d\n",
				__func__, bit_mask, tune_val);
 
	tune_val = TUNE_VAL_MASK(tune_val, qphy->efuse_bit_pos, bit_mask);
	reg = readb_relaxed(qphy->base + qphy->phy_reg[PORT_TUNE1]);
	if (tune_val) {
		tune_val += qphy->efuse_offset;
		if ((s32)tune_val < 0)
			tune_val = 0x00;
		else if ((s32)tune_val > 0x0f)
			tune_val = 0x0f;
		reg = reg & 0x0f;
		reg |= (tune_val << 4);
 	}
	return reg;
}
#else
static void qusb_phy_get_tune1_param(struct qusb_phy *qphy)
{
	u8 reg;
	u32 bit_mask = 1;

	pr_debug("%s(): num_of_bits:%d bit_pos:%d\n", __func__,
				qphy->efuse_num_of_bits,
				qphy->efuse_bit_pos);

	/* get bit mask based on number of bits to use with efuse reg */
	bit_mask = (bit_mask << qphy->efuse_num_of_bits) - 1;

	/*
	 * if efuse reg is updated (i.e non-zero) then use it to program
	 * tune parameters
	 */
	qphy->tune_val = readl_relaxed(qphy->efuse_reg);
	pr_debug("%s(): bit_mask:%d efuse based tune1 value:%d\n",
				__func__, bit_mask, qphy->tune_val);

	qphy->tune_val = TUNE_VAL_MASK(qphy->tune_val,
				qphy->efuse_bit_pos, bit_mask);
	reg = readb_relaxed(qphy->base + qphy->phy_reg[PORT_TUNE1]);
	if (qphy->tune_val) {
		reg = reg & 0x0f;
		reg |= (qphy->tune_val << 4);
	}

	qphy->tune_val = reg;
}
#endif /* CONFIG_MSM_USB_PHY_SOMC_EXT */



static void qusb_phy_write_seq(void __iomem *base, u32 *seq, int cnt,
		unsigned long delay)
{
	int i;

	pr_debug("Seq count:%d\n", cnt);
	for (i = 0; i < cnt; i = i+2) {
		pr_debug("write 0x%02x to 0x%02x\n", seq[i], seq[i+1]);
		writel_relaxed(seq[i], base + seq[i+1]);
		if (delay)
			usleep_range(delay, (delay + 2000));
	}
}

static void qusb_phy_reset(struct qusb_phy *qphy)
{
	int ret;

	ret = reset_control_assert(qphy->phy_reset);
	if (ret)
		dev_err(qphy->phy.dev, "%s: phy_reset assert failed\n",
								__func__);
	usleep_range(100, 150);

	ret = reset_control_deassert(qphy->phy_reset);
	if (ret)
		dev_err(qphy->phy.dev, "%s: phy_reset deassert failed\n",
							__func__);
}

static void qusb_phy_host_init(struct usb_phy *phy)
{
	u8 reg;
	int p_index;
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);

	dev_dbg(phy->dev, "%s\n", __func__);

	qusb_phy_write_seq(qphy->base, qphy->qusb_phy_host_init_seq,
			qphy->host_init_seq_len, 0);

	if (qphy->efuse_reg) {
		if (!qphy->tune_val)
			qusb_phy_get_tune1_param(qphy);
	} else {
		/* For non fused chips we need to write the TUNE1 param as
		 * specified in DT otherwise we will end up writing 0 to
		 * to TUNE1
		 */
		qphy->tune_val = readb_relaxed(qphy->base +
					qphy->phy_reg[PORT_TUNE1]);
	}

	/* If soc revision is mentioned and host_chirp_erratum flag is set
	 * then override TUNE1 and DEBUG_CTRL1 while honouring efuse values
	 */
	if (qphy->soc_min_rev && qphy->host_chirp_erratum) {
		writel_relaxed(qphy->tune_val | BIT(7),
			qphy->base + qphy->phy_reg[PORT_TUNE1]);
		pr_debug("%s(): Programming TUNE1 parameter as:%x\n",
			__func__, readb_relaxed(qphy->base +
					qphy->phy_reg[PORT_TUNE1]));
		writel_relaxed(DEBUG_CTRL1_OVERRIDE_VAL,
			qphy->base + qphy->phy_reg[DEBUG_CTRL1]);
	} else {
		writel_relaxed(qphy->tune_val,
			qphy->base + qphy->phy_reg[PORT_TUNE1]);
	}

	/* if debugfs based tunex params are set, use that value. */
	for (p_index = 0; p_index < 5; p_index++) {
		if (qphy->tune[p_index])
			writel_relaxed(qphy->tune[p_index],
				qphy->base + qphy->phy_reg[PORT_TUNE1] +
							(4 * p_index));
	}

	if (qphy->refgen_north_bg_reg && qphy->override_bias_ctrl2)
		if (readl_relaxed(qphy->refgen_north_bg_reg) & BANDGAP_BYPASS)
			writel_relaxed(BIAS_CTRL_2_OVERRIDE_VAL,
				qphy->base + qphy->phy_reg[BIAS_CTRL_2]);

	if (qphy->bias_ctrl2)
		writel_relaxed(qphy->bias_ctrl2,
				qphy->base + qphy->phy_reg[BIAS_CTRL_2]);

#ifdef CONFIG_MSM_USB_PHY_SOMC_EXT
	if (qphy->efuse_reg) {
		if (!qphy->host_tune_val)
			qphy->host_tune_val = qusb_phy_get_tune1_param(qphy);

		pr_debug("%s(): Programming host TUNE1 parameter as:%x\n",
				__func__, qphy->host_tune_val);
		writel_relaxed(qphy->host_tune_val,
				qphy->base + qphy->phy_reg[PORT_TUNE1]);
	}

	/* If phy_tune1 modparam set, override tune1 value */
	if (phy_host_tune1) {
		pr_debug("%s(): (modparam) HOST_TUNE1 val:0x%02x\n",
						__func__, phy_host_tune1);
		writel_relaxed(phy_host_tune1,
				qphy->base + qphy->phy_reg[PORT_TUNE1]);
	}
	/* If phy_tune2 modparam set, override tune2 value */
	if (phy_host_tune2) {
		pr_debug("%s(): (modparam) HOST TUNE2 val:0x%02x\n",
						__func__, phy_host_tune2);
		writel_relaxed(phy_host_tune2,
				qphy->base + qphy->phy_reg[PORT_TUNE1] + 0x4);
	}
#endif /* CONFIG_MSM_USB_PHY_SOMC_EXT */

	/* Ensure above write is completed before turning ON ref clk */
	wmb();

	/* Require to get phy pll lock successfully */
	usleep_range(150, 160);

	reg = readb_relaxed(qphy->base + qphy->phy_reg[PLL_COMMON_STATUS_ONE]);
	dev_dbg(phy->dev, "QUSB2PHY_PLL_COMMON_STATUS_ONE:%x\n", reg);
	if (!(reg & CORE_READY_STATUS)) {
		dev_err(phy->dev, "QUSB PHY PLL LOCK fails:%x\n", reg);
		WARN_ON(1);
	}
}

static int qusb_phy_init(struct usb_phy *phy)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);
	int ret, p_index;
	u8 reg;

	dev_dbg(phy->dev, "%s\n", __func__);

	ret = qusb_phy_enable_power(qphy, true);
	if (ret)
		return ret;

	qusb_phy_enable_clocks(qphy, true);

	qusb_phy_reset(qphy);

	if (qphy->qusb_phy_host_init_seq && qphy->phy.flags & PHY_HOST_MODE) {
		qusb_phy_host_init(phy);
		return 0;
	}

	if (qphy->emulation) {
		if (qphy->emu_init_seq)
			qusb_phy_write_seq(qphy->emu_phy_base + 0x8000,
				qphy->emu_init_seq,
					qphy->emu_init_seq_len, 10000);

		if (qphy->qusb_phy_init_seq)
			qusb_phy_write_seq(qphy->base, qphy->qusb_phy_init_seq,
					qphy->init_seq_len, 0);

		/* Wait for 5ms as per QUSB2 RUMI sequence */
		usleep_range(5000, 7000);

		if (qphy->phy_pll_reset_seq)
			qusb_phy_write_seq(qphy->base, qphy->phy_pll_reset_seq,
					qphy->phy_pll_reset_seq_len, 10000);

		if (qphy->emu_dcm_reset_seq)
			qusb_phy_write_seq(qphy->emu_phy_base,
					qphy->emu_dcm_reset_seq,
					qphy->emu_dcm_reset_seq_len, 10000);

		return 0;
	}

	/* Disable the PHY */
	writel_relaxed(readl_relaxed(qphy->base + qphy->phy_reg[PWR_CTRL1]) |
			PWR_CTRL1_POWR_DOWN,
			qphy->base + qphy->phy_reg[PWR_CTRL1]);

	if (qphy->qusb_phy_init_seq)
		qusb_phy_write_seq(qphy->base, qphy->qusb_phy_init_seq,
				qphy->init_seq_len, 0);
	if (qphy->efuse_reg) {
#ifdef CONFIG_MSM_USB_PHY_SOMC_EXT
		if (!qphy->tune_val)
			qphy->tune_val = qusb_phy_get_tune1_param(qphy);
#else
		if (!qphy->tune_val)
			qusb_phy_get_tune1_param(qphy);
#endif

		pr_debug("%s(): Programming TUNE1 parameter as:%x\n", __func__,
				qphy->tune_val);
		writel_relaxed(qphy->tune_val,
				qphy->base + qphy->phy_reg[PORT_TUNE1]);
	}

	/* if debugfs based tunex params are set, use that value. */
	for (p_index = 0; p_index < 5; p_index++) {
		if (qphy->tune[p_index])
			writel_relaxed(qphy->tune[p_index],
				qphy->base + qphy->phy_reg[PORT_TUNE1] +
							(4 * p_index));
	}

	if (qphy->refgen_north_bg_reg && qphy->override_bias_ctrl2)
		if (readl_relaxed(qphy->refgen_north_bg_reg) & BANDGAP_BYPASS)
			writel_relaxed(BIAS_CTRL_2_OVERRIDE_VAL,
				qphy->base + qphy->phy_reg[BIAS_CTRL_2]);

	if (qphy->bias_ctrl2)
		writel_relaxed(qphy->bias_ctrl2,
				qphy->base + qphy->phy_reg[BIAS_CTRL_2]);

#ifdef CONFIG_MSM_USB_PHY_SOMC_EXT
	/* If phy_tune2 modparam set, override tune2 value */
	if (phy_tune2) {
		pr_debug("%s(): (modparam) TUNE2 val:0x%02x\n",
						__func__, phy_tune2);
		writel_relaxed(phy_tune2,
				qphy->base + qphy->phy_reg[PORT_TUNE1] + 0x4);
	}
	/* If phy_tune3 modparam set, override tune3 value */
	if (phy_tune3) {
		pr_debug("%s(): (modparam) TUNE3 val:0x%02x\n",
						__func__, phy_tune3);
		writel_relaxed(phy_tune3,
				qphy->base + qphy->phy_reg[PORT_TUNE1] + 0x8);
	}
	/* If phy_tune4 modparam set, override tune4 value */
	if (phy_tune4) {
		pr_debug("%s(): (modparam) TUNE4 val:0x%02x\n",
						__func__, phy_tune4);
		writel_relaxed(phy_tune4,
				qphy->base + qphy->phy_reg[PORT_TUNE1] + 0xC);
	}
#endif /* CONFIG_MSM_USB_PHY_SOMC_EXT */

	/* ensure above writes are completed before re-enabling PHY */
	wmb();

	/* Enable the PHY */
	writel_relaxed(readl_relaxed(qphy->base + qphy->phy_reg[PWR_CTRL1]) &
			~PWR_CTRL1_POWR_DOWN,
			qphy->base + qphy->phy_reg[PWR_CTRL1]);

	/* Ensure above write is completed before turning ON ref clk */
	wmb();

	/* Require to get phy pll lock successfully */
	usleep_range(150, 160);

	reg = readb_relaxed(qphy->base + qphy->phy_reg[PLL_COMMON_STATUS_ONE]);
	dev_dbg(phy->dev, "QUSB2PHY_PLL_COMMON_STATUS_ONE:%x\n", reg);
	if (!(reg & CORE_READY_STATUS)) {
		dev_err(phy->dev, "QUSB PHY PLL LOCK fails:%x\n", reg);
		WARN_ON(1);
	}
	return 0;
}

static enum hrtimer_restart qusb_dis_ext_pulldown_timer(struct hrtimer *timer)
{
	struct qusb_phy *qphy = container_of(timer, struct qusb_phy, timer);
	int ret = 0;

	if (qphy->pinctrl && qphy->atest_usb13_suspend) {
		ret = pinctrl_select_state(qphy->pinctrl,
				qphy->atest_usb13_suspend);
		if (ret < 0)
			dev_err(qphy->phy.dev,
				"pinctrl state suspend select failed\n");
	}

	return HRTIMER_NORESTART;
}

static void qusb_phy_enable_ext_pulldown(struct usb_phy *phy)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);
	int ret = 0;

	dev_dbg(phy->dev, "%s\n", __func__);

	if (qphy->pinctrl && qphy->atest_usb13_active) {
		ret = pinctrl_select_state(qphy->pinctrl,
				qphy->atest_usb13_active);
		if (ret < 0) {
			dev_err(phy->dev,
					"pinctrl state active select failed\n");
			return;
		}

		hrtimer_start(&qphy->timer, ms_to_ktime(10), HRTIMER_MODE_REL);
	}
}

static void qusb_phy_shutdown(struct usb_phy *phy)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);

	dev_dbg(phy->dev, "%s\n", __func__);

	qusb_phy_enable_power(qphy, false);

}

static u32 qusb_phy_get_linestate(struct qusb_phy *qphy)
{
	u32 linestate = 0;

	if (qphy->cable_connected) {
		if (qphy->phy.flags & PHY_HSFS_MODE)
			linestate |= LINESTATE_DP;
		else if (qphy->phy.flags & PHY_LS_MODE)
			linestate |= LINESTATE_DM;
	}
	return linestate;
}

/**
 * Performs QUSB2 PHY suspend/resume functionality on v2.0 PHY.
 *
 * Note: The PLL_CORE_INPUT_OVERRIDE register is erroneously
 *       called like that. On V2.0 this in reality is
 *       PLL_ANALOG_CONTROLS_ONE.
 *       The PLL_ANALOG_CONTROLS_TWO is ONE + 0x4.
 *
 * @uphy - usb phy pointer.
 * @suspend - to enable suspend or not. 1 - suspend, 0 - resume
 *
 */
#if defined(CONFIG_ARCH_MSM8998) || defined(CONFIG_ARCH_SDM660) || \
    defined(CONFIG_ARCH_SDM630)
static int qusb_phy_set_suspend_legacy(struct usb_phy *phy, int suspend)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);
	u32 linestate = 0, intr_mask = 0;
	static u8 analog_ctrl_two;
	int ret;

	if (qphy->suspended && suspend) {
		dev_dbg(phy->dev, "%s: USB PHY is already suspended\n",
			__func__);
		return 0;
	}

	if (suspend) {
		/* Bus suspend case */
		if (qphy->cable_connected ||
			(qphy->phy.flags & PHY_HOST_MODE)) {

			/* store clock settings like cmos/cml (TWO)*/
			analog_ctrl_two = readl_relaxed(qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE] + 0x4);


			/* use CSR & switch to SE clk (TWO)*/
			writel_relaxed(0xb, qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE] + 0x4);

			/* enable clock bypass (ONE) */
			writel_relaxed(0x90, qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE]);

			/* Disable all interrupts */
			writel_relaxed(0x00,
				qphy->base + qphy->phy_reg[INTR_CTRL]);

			linestate = qusb_phy_get_linestate(qphy);
			/*
			 * D+/D- interrupts are level-triggered, but we are
			 * only interested if the line state changes, so enable
			 * the high/low trigger based on current state. In
			 * other words, enable the triggers _opposite_ of what
			 * the current D+/D- levels are.
			 * e.g. if currently D+ high, D- low (HS 'J'/Suspend),
			 * configure the mask to trigger on D+ low OR D- high
			 */
			intr_mask = DPSE_INTR_EN | DMSE_INTR_EN;
			if (!(linestate & LINESTATE_DP)) /* D+ low */
				intr_mask |= DPSE_INTR_HIGH_SEL;
			if (!(linestate & LINESTATE_DM)) /* D- low */
				intr_mask |= DMSE_INTR_HIGH_SEL;

			writel_relaxed(intr_mask,
				qphy->base + qphy->phy_reg[INTR_CTRL]);

			if (linestate & (LINESTATE_DP | LINESTATE_DM)) {
				/* enable phy auto-resume */
				writel_relaxed(0x91,
					qphy->base + qphy->phy_reg[TEST1]);
				/* flush the previous write before next write */
				wmb();
				writel_relaxed(0x90,
					qphy->base + qphy->phy_reg[TEST1]);
			}

			dev_dbg(phy->dev, "%s: intr_mask = %x\n",
			__func__, intr_mask);

			/* Makes sure that above write goes through */
			wmb();
			qusb_phy_enable_clocks(qphy, false);
		} else { /* Cable disconnect case */

			ret = reset_control_assert(qphy->phy_reset);
			if (ret)
				dev_err(phy->dev, "%s: phy_reset assert failed\n",
						__func__);
			usleep_range(100, 150);
			ret = reset_control_deassert(qphy->phy_reset);
			if (ret)
				dev_err(phy->dev, "%s: phy_reset deassert failed\n",
						__func__);

			/* TWO */
			writel_relaxed(0x1b, qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE] + 0x4);

			/* enable clock bypass (ONE) */
			writel_relaxed(0x90, qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE]);

			writel_relaxed(0x0, qphy->tcsr_clamp_dig_n);
			/*
			 * clamp needs asserted before
			 * power/clocks can be turned off
			 */
			wmb();

			qusb_phy_enable_clocks(qphy, false);
			qusb_phy_enable_power(qphy, false);
		}
		qphy->suspended = true;
	} else {
		/* Bus resume case */
		if (qphy->cable_connected ||
			(qphy->phy.flags & PHY_HOST_MODE)) {
			qusb_phy_enable_clocks(qphy, true);

			/* restore the default clock settings (TWO) */
			writel_relaxed(analog_ctrl_two, qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE] + 0x4);

			/* disable clock bypass (ONE) */
			writel_relaxed(0x80, qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE]);

			/* Clear all interrupts on resume */
			writel_relaxed(0x00,
				qphy->base + qphy->phy_reg[INTR_CTRL]);

			/* Makes sure that above write goes through */
			wmb();
		} else { /* Cable connect case */
			writel_relaxed(0x1, qphy->tcsr_clamp_dig_n);

			/*
			 * clamp needs de-asserted before
			 * power/clocks can be turned on
			 */
			wmb();

			qusb_phy_enable_power(qphy, true);
			ret = reset_control_assert(qphy->phy_reset);
			if (ret)
				dev_err(phy->dev, "%s: phy_reset assert failed\n",
						__func__);
			usleep_range(100, 150);
			ret = reset_control_deassert(qphy->phy_reset);
			if (ret)
				dev_err(phy->dev, "%s: phy_reset deassert failed\n",
						__func__);

			qusb_phy_enable_clocks(qphy, true);
		}
		qphy->suspended = false;
	}

	return 0;
}
#endif

/**
 * Performs QUSB2 PHY suspend/resume functionality.
 *
 * @uphy - usb phy pointer.
 * @suspend - to enable suspend or not. 1 - suspend, 0 - resume
 *
 */
static int qusb_phy_set_suspend(struct usb_phy *phy, int suspend)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);
	u32 linestate = 0, intr_mask = 0;

	if (qphy->suspended && suspend) {
		dev_dbg(phy->dev, "%s: USB PHY is already suspended\n",
			__func__);
		return 0;
	}

	if (suspend) {
		/* Bus suspend case */
		if (qphy->cable_connected ||
			(qphy->phy.flags & PHY_HOST_MODE)) {
			/* Disable all interrupts */
			writel_relaxed(0x00,
				qphy->base + qphy->phy_reg[INTR_CTRL]);

			linestate = qusb_phy_get_linestate(qphy);
			/*
			 * D+/D- interrupts are level-triggered, but we are
			 * only interested if the line state changes, so enable
			 * the high/low trigger based on current state. In
			 * other words, enable the triggers _opposite_ of what
			 * the current D+/D- levels are.
			 * e.g. if currently D+ high, D- low (HS 'J'/Suspend),
			 * configure the mask to trigger on D+ low OR D- high
			 */
			intr_mask = DPSE_INTR_EN | DMSE_INTR_EN;
			if (!(linestate & LINESTATE_DP)) /* D+ low */
				intr_mask |= DPSE_INTR_HIGH_SEL;
			if (!(linestate & LINESTATE_DM)) /* D- low */
				intr_mask |= DMSE_INTR_HIGH_SEL;

			writel_relaxed(intr_mask,
				qphy->base + qphy->phy_reg[INTR_CTRL]);

			/* hold core PLL into reset */
			writel_relaxed(CORE_PLL_EN_FROM_RESET |
				CORE_RESET | CORE_RESET_MUX,
				qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE]);

			if (linestate & (LINESTATE_DP | LINESTATE_DM)) {
				/* enable phy auto-resume */
				writel_relaxed(0x91,
					qphy->base + qphy->phy_reg[TEST1]);
				/* flush the previous write before next write */
				wmb();
				writel_relaxed(0x90,
					qphy->base + qphy->phy_reg[TEST1]);
			}

			dev_dbg(phy->dev, "%s: intr_mask = %x\n",
			__func__, intr_mask);

			/* Makes sure that above write goes through */
			wmb();
			qusb_phy_enable_clocks(qphy, false);
		} else { /* Cable disconnect case */
			/* Disable all interrupts */
			writel_relaxed(0x00,
				qphy->base + qphy->phy_reg[INTR_CTRL]);
			qusb_phy_reset(qphy);
			qusb_phy_enable_clocks(qphy, false);
			qusb_phy_enable_power(qphy, false);
		}
		qphy->suspended = true;
	} else {
		/* Bus resume case */
		if (qphy->cable_connected ||
			(qphy->phy.flags & PHY_HOST_MODE)) {
			qusb_phy_enable_clocks(qphy, true);
			/* Clear all interrupts on resume */
			writel_relaxed(0x00,
				qphy->base + qphy->phy_reg[INTR_CTRL]);

			/* bring core PLL out of reset */
			writel_relaxed(CORE_PLL_EN_FROM_RESET, qphy->base +
				qphy->phy_reg[PLL_CORE_INPUT_OVERRIDE]);

			/* Makes sure that above write goes through */
			wmb();
		} else { /* Cable connect case */
			qusb_phy_enable_clocks(qphy, true);
		}
		qphy->suspended = false;
	}

	return 0;
}

static int qusb_phy_notify_connect(struct usb_phy *phy,
					enum usb_device_speed speed)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);

	qphy->cable_connected = true;

	dev_dbg(phy->dev, "QUSB PHY: connect notification cable_connected=%d\n",
							qphy->cable_connected);
	return 0;
}

static int qusb_phy_notify_disconnect(struct usb_phy *phy,
					enum usb_device_speed speed)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);

	qphy->cable_connected = false;

	dev_dbg(phy->dev, "QUSB PHY: connect notification cable_connected=%d\n",
							qphy->cable_connected);
	return 0;
}

#if !defined(CONFIG_ARCH_MSM8998) && !defined(CONFIG_ARCH_SDM660) && \
    !defined(CONFIG_ARCH_SDM630)
static int qusb_phy_disable_chirp(struct usb_phy *phy, bool disable)
{
	struct qusb_phy *qphy = container_of(phy, struct qusb_phy, phy);
	int ret = 0;

	dev_dbg(phy->dev, "%s qphy chirp disable %d disable %d\n", __func__,
			qphy->chirp_disable, disable);

	mutex_lock(&qphy->lock);

	if (qphy->chirp_disable == disable) {
		ret = -EALREADY;
		goto done;
	}

	qphy->chirp_disable = disable;

	if (disable) {
		qphy->sq_ctrl1_default =
			readl_relaxed(qphy->base + qphy->phy_reg[SQ_CTRL1]);
		qphy->sq_ctrl2_default =
			readl_relaxed(qphy->base + qphy->phy_reg[SQ_CTRL2]);

		writel_relaxed(SQ_CTRL1_CHIRP_DISABLE,
				qphy->base + qphy->phy_reg[SQ_CTRL1]);
		readl_relaxed(qphy->base + qphy->phy_reg[SQ_CTRL1]);

		writel_relaxed(SQ_CTRL1_CHIRP_DISABLE,
				qphy->base + qphy->phy_reg[SQ_CTRL2]);
		readl_relaxed(qphy->base + qphy->phy_reg[SQ_CTRL2]);

		goto done;
	}

	writel_relaxed(qphy->sq_ctrl1_default,
			qphy->base + qphy->phy_reg[SQ_CTRL1]);
	readl_relaxed(qphy->base + qphy->phy_reg[SQ_CTRL1]);

	writel_relaxed(qphy->sq_ctrl2_default,
			qphy->base + qphy->phy_reg[SQ_CTRL2]);
	readl_relaxed(qphy->base + qphy->phy_reg[SQ_CTRL2]);
done:
	mutex_unlock(&qphy->lock);
	return ret;
}
#endif

static int qusb_phy_dpdm_regulator_enable(struct regulator_dev *rdev)
{
	int ret = 0;
	struct qusb_phy *qphy = rdev_get_drvdata(rdev);

	dev_dbg(qphy->phy.dev, "%s dpdm_enable:%d\n",
				__func__, qphy->dpdm_enable);

	if (!qphy->dpdm_enable) {
		ret = qusb_phy_enable_power(qphy, true);
		if (ret < 0) {
			dev_dbg(qphy->phy.dev,
				"dpdm regulator enable failed:%d\n", ret);
			return ret;
		}
		qphy->dpdm_enable = true;
		qusb_phy_reset(qphy);
	}

	return ret;
}

static int qusb_phy_dpdm_regulator_disable(struct regulator_dev *rdev)
{
	int ret = 0;
	struct qusb_phy *qphy = rdev_get_drvdata(rdev);

	dev_dbg(qphy->phy.dev, "%s dpdm_enable:%d\n",
				__func__, qphy->dpdm_enable);

	if (qphy->dpdm_enable) {
		ret = qusb_phy_enable_power(qphy, false);
		if (ret < 0) {
			dev_dbg(qphy->phy.dev,
				"dpdm regulator disable failed:%d\n", ret);
			return ret;
		}
		qphy->dpdm_enable = false;
	}

	return ret;
}

static int qusb_phy_dpdm_regulator_is_enabled(struct regulator_dev *rdev)
{
	struct qusb_phy *qphy = rdev_get_drvdata(rdev);

	dev_dbg(qphy->phy.dev, "%s qphy->dpdm_enable = %d\n", __func__,
					qphy->dpdm_enable);
	return qphy->dpdm_enable;
}

static struct regulator_ops qusb_phy_dpdm_regulator_ops = {
	.enable		= qusb_phy_dpdm_regulator_enable,
	.disable	= qusb_phy_dpdm_regulator_disable,
	.is_enabled	= qusb_phy_dpdm_regulator_is_enabled,
};

static int qusb_phy_regulator_init(struct qusb_phy *qphy)
{
	struct device *dev = qphy->phy.dev;
	struct regulator_config cfg = {};
	struct regulator_init_data *init_data;

	init_data = devm_kzalloc(dev, sizeof(*init_data), GFP_KERNEL);
	if (!init_data)
		return -ENOMEM;

	init_data->constraints.valid_ops_mask |= REGULATOR_CHANGE_STATUS;
	qphy->dpdm_rdesc.owner = THIS_MODULE;
	qphy->dpdm_rdesc.type = REGULATOR_VOLTAGE;
	qphy->dpdm_rdesc.ops = &qusb_phy_dpdm_regulator_ops;
	qphy->dpdm_rdesc.name = kbasename(dev->of_node->full_name);

	cfg.dev = dev;
	cfg.init_data = init_data;
	cfg.driver_data = qphy;
	cfg.of_node = dev->of_node;

	qphy->dpdm_rdev = devm_regulator_register(dev, &qphy->dpdm_rdesc, &cfg);
	if (IS_ERR(qphy->dpdm_rdev))
		return PTR_ERR(qphy->dpdm_rdev);

	return 0;
}

static int qusb_phy_create_debugfs(struct qusb_phy *qphy)
{
	struct dentry *file;
	int ret = 0, i;
	char name[6];

	qphy->root = debugfs_create_dir(dev_name(qphy->phy.dev), NULL);
	if (IS_ERR_OR_NULL(qphy->root)) {
		dev_err(qphy->phy.dev,
			"can't create debugfs root for %s\n",
					dev_name(qphy->phy.dev));
		ret = -ENOMEM;
		goto create_err;
	}

	for (i = 0; i < 5; i++) {
		snprintf(name, sizeof(name), "tune%d", (i + 1));
		file = debugfs_create_x8(name, 0644, qphy->root,
						&qphy->tune[i]);
		if (IS_ERR_OR_NULL(file)) {
			dev_err(qphy->phy.dev,
				"can't create debugfs entry for %s\n", name);
			debugfs_remove_recursive(qphy->root);
			ret = -ENOMEM;
			goto create_err;
		}
	}

	file = debugfs_create_x8("bias_ctrl2", 0644, qphy->root,
						&qphy->bias_ctrl2);
	if (IS_ERR_OR_NULL(file)) {
		dev_err(qphy->phy.dev,
			"can't create debugfs entry for bias_ctrl2\n");
		debugfs_remove_recursive(qphy->root);
		ret = -ENOMEM;
		goto create_err;
	}

create_err:
	return ret;
}

static int qusb_phy_probe(struct platform_device *pdev)
{
	struct qusb_phy *qphy;
	struct device *dev = &pdev->dev;
	struct resource *res;
	int ret = 0, size = 0;

	qphy = devm_kzalloc(dev, sizeof(*qphy), GFP_KERNEL);
	if (!qphy)
		return -ENOMEM;

	qphy->phy.dev = dev;
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"qusb_phy_base");
	qphy->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(qphy->base))
		return PTR_ERR(qphy->base);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"emu_phy_base");
	if (res) {
		qphy->emu_phy_base = devm_ioremap_resource(dev, res);
		if (IS_ERR(qphy->emu_phy_base)) {
			dev_dbg(dev, "couldn't ioremap emu_phy_base\n");
			qphy->emu_phy_base = NULL;
		}
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						"tcsr_clamp_dig_n_1p8");
	if (res) {
		qphy->tcsr_clamp_dig_n = devm_ioremap_resource(dev, res);
		if (IS_ERR(qphy->tcsr_clamp_dig_n)) {
			dev_dbg(dev, "couldn't ioremap tcsr_clamp_dig_n\n");
			return PTR_ERR(qphy->tcsr_clamp_dig_n);
		}
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"efuse_addr");
	if (res) {
		qphy->efuse_reg = devm_ioremap_nocache(dev, res->start,
							resource_size(res));
		if (!IS_ERR_OR_NULL(qphy->efuse_reg)) {
			ret = of_property_read_u32(dev->of_node,
					"qcom,efuse-bit-pos",
					&qphy->efuse_bit_pos);
			if (!ret) {
				ret = of_property_read_u32(dev->of_node,
						"qcom,efuse-num-bits",
						&qphy->efuse_num_of_bits);
			}
#ifdef CONFIG_MSM_USB_PHY_SOMC_EXT
			if (!ret) {
				ret = of_property_read_u32(dev->of_node,
						"qcom,efuse-offset",
						&qphy->efuse_offset);
			}
#endif /* CONFIG_MSM_USB_PHY_SOMC_EXT */
			if (ret) {
				dev_err(dev,
				"DT Value for efuse is invalid.\n");
				return -EINVAL;
			}
		}
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					"refgen_north_bg_reg_addr");
	if (res)
		qphy->refgen_north_bg_reg = devm_ioremap(dev, res->start,
						resource_size(res));

	/* ref_clk_src is needed irrespective of SE_CLK or DIFF_CLK usage */
	qphy->ref_clk_src = devm_clk_get(dev, "ref_clk_src");
	if (IS_ERR(qphy->ref_clk_src)) {
		dev_dbg(dev, "clk get failed for ref_clk_src\n");
		ret = PTR_ERR(qphy->ref_clk_src);
		return ret;
	}

	/* ref_clk is needed only for DIFF_CLK case, hence make it optional. */
	if (of_property_match_string(pdev->dev.of_node,
				"clock-names", "ref_clk") >= 0) {
		qphy->ref_clk = devm_clk_get(dev, "ref_clk");
		if (IS_ERR(qphy->ref_clk)) {
			ret = PTR_ERR(qphy->ref_clk);
			if (ret != -EPROBE_DEFER)
				dev_dbg(dev,
					"clk get failed for ref_clk\n");
			return ret;
		}

		clk_set_rate(qphy->ref_clk, 19200000);
	}

	if (of_property_match_string(pdev->dev.of_node,
				"clock-names", "cfg_ahb_clk") >= 0) {
		qphy->cfg_ahb_clk = devm_clk_get(dev, "cfg_ahb_clk");
		if (IS_ERR(qphy->cfg_ahb_clk)) {
			ret = PTR_ERR(qphy->cfg_ahb_clk);
			if (ret != -EPROBE_DEFER)
				dev_err(dev,
				"clk get failed for cfg_ahb_clk ret %d\n", ret);
			return ret;
		}
	}

	qphy->phy_reset = devm_reset_control_get(dev, "phy_reset");
	if (IS_ERR(qphy->phy_reset))
		return PTR_ERR(qphy->phy_reset);

	qphy->emulation = of_property_read_bool(dev->of_node,
					"qcom,emulation");

	of_get_property(dev->of_node, "qcom,emu-init-seq", &size);
	if (size) {
		qphy->emu_init_seq = devm_kzalloc(dev,
						size, GFP_KERNEL);
		if (qphy->emu_init_seq) {
			qphy->emu_init_seq_len =
				(size / sizeof(*qphy->emu_init_seq));
			if (qphy->emu_init_seq_len % 2) {
				dev_err(dev, "invalid emu_init_seq_len\n");
				return -EINVAL;
			}

			of_property_read_u32_array(dev->of_node,
				"qcom,emu-init-seq",
				qphy->emu_init_seq,
				qphy->emu_init_seq_len);
		} else {
			dev_dbg(dev,
			"error allocating memory for emu_init_seq\n");
		}
	}

	size = 0;
	of_get_property(dev->of_node, "qcom,phy-pll-reset-seq", &size);
	if (size) {
		qphy->phy_pll_reset_seq = devm_kzalloc(dev,
						size, GFP_KERNEL);
		if (qphy->phy_pll_reset_seq) {
			qphy->phy_pll_reset_seq_len =
				(size / sizeof(*qphy->phy_pll_reset_seq));
			if (qphy->phy_pll_reset_seq_len % 2) {
				dev_err(dev, "invalid phy_pll_reset_seq_len\n");
				return -EINVAL;
			}

			of_property_read_u32_array(dev->of_node,
				"qcom,phy-pll-reset-seq",
				qphy->phy_pll_reset_seq,
				qphy->phy_pll_reset_seq_len);
		} else {
			dev_dbg(dev,
			"error allocating memory for phy_pll_reset_seq\n");
		}
	}

	size = 0;
	of_get_property(dev->of_node, "qcom,emu-dcm-reset-seq", &size);
	if (size) {
		qphy->emu_dcm_reset_seq = devm_kzalloc(dev,
						size, GFP_KERNEL);
		if (qphy->emu_dcm_reset_seq) {
			qphy->emu_dcm_reset_seq_len =
				(size / sizeof(*qphy->emu_dcm_reset_seq));
			if (qphy->emu_dcm_reset_seq_len % 2) {
				dev_err(dev, "invalid emu_dcm_reset_seq_len\n");
				return -EINVAL;
			}

			of_property_read_u32_array(dev->of_node,
				"qcom,emu-dcm-reset-seq",
				qphy->emu_dcm_reset_seq,
				qphy->emu_dcm_reset_seq_len);
		} else {
			dev_dbg(dev,
			"error allocating memory for emu_dcm_reset_seq\n");
		}
	}

	size = 0;
	of_get_property(dev->of_node, "qcom,qusb-phy-reg-offset", &size);
	if (size) {
		qphy->phy_reg = devm_kzalloc(dev, size, GFP_KERNEL);
		if (qphy->phy_reg) {
			qphy->qusb_phy_reg_offset_cnt =
				size / sizeof(*qphy->phy_reg);
			if (qphy->qusb_phy_reg_offset_cnt != USB2_PHY_REG_MAX) {
				dev_err(dev, "invalid reg offset count\n");
				return -EINVAL;
			}

			of_property_read_u32_array(dev->of_node,
					"qcom,qusb-phy-reg-offset",
					qphy->phy_reg,
					qphy->qusb_phy_reg_offset_cnt);
		} else {
			dev_err(dev, "err mem alloc for qusb_phy_reg_offset\n");
			return -ENOMEM;
		}
	} else {
		dev_err(dev, "err provide qcom,qmp-phy-reg-offset\n");
		return -EINVAL;
	}

	size = 0;
	of_get_property(dev->of_node, "qcom,qusb-phy-init-seq", &size);
	if (size) {
		qphy->qusb_phy_init_seq = devm_kzalloc(dev,
						size, GFP_KERNEL);
		if (qphy->qusb_phy_init_seq) {
			qphy->init_seq_len =
				(size / sizeof(*qphy->qusb_phy_init_seq));
			if (qphy->init_seq_len % 2) {
				dev_err(dev, "invalid init_seq_len\n");
				return -EINVAL;
			}

			of_property_read_u32_array(dev->of_node,
				"qcom,qusb-phy-init-seq",
				qphy->qusb_phy_init_seq,
				qphy->init_seq_len);
		} else {
			dev_err(dev,
			"error allocating memory for phy_init_seq\n");
		}
	}

	qphy->host_init_seq_len = of_property_count_elems_of_size(dev->of_node,
				"qcom,qusb-phy-host-init-seq",
				sizeof(*qphy->qusb_phy_host_init_seq));
	if (qphy->host_init_seq_len > 0) {
		qphy->qusb_phy_host_init_seq = devm_kcalloc(dev,
					qphy->host_init_seq_len,
					sizeof(*qphy->qusb_phy_host_init_seq),
					GFP_KERNEL);
		if (qphy->qusb_phy_host_init_seq)
			of_property_read_u32_array(dev->of_node,
				"qcom,qusb-phy-host-init-seq",
				qphy->qusb_phy_host_init_seq,
				qphy->host_init_seq_len);
		else
			return -ENOMEM;
	}

	qphy->host_chirp_erratum = of_property_read_bool(dev->of_node,
					"qcom,host-chirp-erratum");

	qphy->override_bias_ctrl2 = of_property_read_bool(dev->of_node,
					"qcom,override-bias-ctrl2");

	ret = of_property_read_u32_array(dev->of_node, "qcom,vdd-voltage-level",
					 (u32 *) qphy->vdd_levels,
					 ARRAY_SIZE(qphy->vdd_levels));
	if (ret) {
		dev_err(dev, "error reading qcom,vdd-voltage-level property\n");
		return ret;
	}

	qphy->vdd = devm_regulator_get(dev, "vdd");
	if (IS_ERR(qphy->vdd)) {
		dev_err(dev, "unable to get vdd supply\n");
		return PTR_ERR(qphy->vdd);
	}

	qphy->vdda33 = devm_regulator_get(dev, "vdda33");
	if (IS_ERR(qphy->vdda33)) {
		dev_err(dev, "unable to get vdda33 supply\n");
		return PTR_ERR(qphy->vdda33);
	}

	qphy->vdda18 = devm_regulator_get(dev, "vdda18");
	if (IS_ERR(qphy->vdda18)) {
		dev_err(dev, "unable to get vdda18 supply\n");
		return PTR_ERR(qphy->vdda18);
	}

	ret = qusb_phy_get_socrev(&pdev->dev, qphy);
	if (ret == -EPROBE_DEFER) {
		dev_err(&pdev->dev, "SoC version rd: fail: defer for now\n");
		return ret;
	}
	qphy->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(qphy->pinctrl)) {
		ret = PTR_ERR(qphy->pinctrl);
		if (ret == -EPROBE_DEFER)
			return ret;
		dev_err(dev, "pinctrl not available\n");
		goto skip_pinctrl_config;
	}
	qphy->atest_usb13_suspend = pinctrl_lookup_state(qphy->pinctrl,
							"atest_usb13_suspend");
	if (IS_ERR(qphy->atest_usb13_suspend)) {
		dev_err(dev, "pinctrl lookup atest_usb13_suspend failed\n");
		goto skip_pinctrl_config;
	}

	qphy->atest_usb13_active = pinctrl_lookup_state(qphy->pinctrl,
							"atest_usb13_active");
	if (IS_ERR(qphy->atest_usb13_active))
		dev_err(dev, "pinctrl lookup atest_usb13_active failed\n");

	hrtimer_init(&qphy->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	qphy->timer.function = qusb_dis_ext_pulldown_timer;

skip_pinctrl_config:
	mutex_init(&qphy->lock);
	platform_set_drvdata(pdev, qphy);

	qphy->phy.label			= "msm-qusb-phy-v2";
	qphy->phy.init			= qusb_phy_init;
	qphy->phy.set_suspend           = qusb_phy_set_suspend;
	qphy->phy.shutdown		= qusb_phy_shutdown;
	qphy->phy.type			= USB_PHY_TYPE_USB2;
	qphy->phy.notify_connect        = qusb_phy_notify_connect;
	qphy->phy.notify_disconnect     = qusb_phy_notify_disconnect;

#if defined(CONFIG_ARCH_MSM8998) || defined(CONFIG_ARCH_SDM660) || \
    defined(CONFIG_ARCH_SDM630)
	qphy->phy.set_suspend           = qusb_phy_set_suspend_legacy;
#else
	/*
	 * qusb_phy_disable_chirp is not required if soc version is
	 * mentioned and is not base version.
	 */
	if (!qphy->soc_min_rev) {
		qphy->phy.disable_chirp	= qusb_phy_disable_chirp;
	}
#endif

	qphy->phy.start_port_reset	= qusb_phy_enable_ext_pulldown;

	ret = usb_add_phy_dev(&qphy->phy);
	if (ret)
		return ret;

	ret = qusb_phy_regulator_init(qphy);
	if (ret)
		usb_remove_phy(&qphy->phy);

	qusb_phy_create_debugfs(qphy);

	/* de-asseert clamp dig n to reduce leakage on 1p8 upon boot up */
	if (qphy->tcsr_clamp_dig_n)
		writel_relaxed(0x0, qphy->tcsr_clamp_dig_n);

	return ret;
}

static int qusb_phy_remove(struct platform_device *pdev)
{
	struct qusb_phy *qphy = platform_get_drvdata(pdev);

	usb_remove_phy(&qphy->phy);
	qusb_phy_enable_clocks(qphy, false);
	qusb_phy_enable_power(qphy, false);
	debugfs_remove_recursive(qphy->root);

	return 0;
}

static const struct of_device_id qusb_phy_id_table[] = {
	{ .compatible = "qcom,qusb2phy-v2", },
	{ },
};
MODULE_DEVICE_TABLE(of, qusb_phy_id_table);

static struct platform_driver qusb_phy_driver = {
	.probe		= qusb_phy_probe,
	.remove		= qusb_phy_remove,
	.driver = {
		.name	= "msm-qusb-phy-v2",
		.of_match_table = of_match_ptr(qusb_phy_id_table),
	},
};

module_platform_driver(qusb_phy_driver);

MODULE_DESCRIPTION("MSM QUSB2 PHY v2 driver");
MODULE_LICENSE("GPL v2");
