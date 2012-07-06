/* kernel/arch/arm/mach-msm/simple_remote_msm8x60_pf.c
 *
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * Authors: Takashi Shiina <takashi.shiina@sonyericsson.com>
 *          Tadashi Kubo <tadashi.kubo@sonyericsson.com>
 *          Joachim Holst <joachim.holst@sonyericsson.com>
 *          Atsushi Iyogi <Atsushi.XA.Iyogi@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <asm/atomic.h>
#include <linux/bitops.h>
#include <mach/gpio.h>
#include <linux/mutex.h>
#include <linux/mfd/marimba.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/msm-adie-codec.h>

#include <linux/mfd/pmic8058.h>
#include <linux/msm_adc.h>
#include <linux/mfd/pm8xxx/core.h>

#include <linux/simple_remote.h>
#include <mach/simple_remote_msm8x60_pf.h>

#define HEADSET_BUTTON_PRESS    0x01
#define HEADSET_BUTTON_RELEASE  0x00

#define IS_BTN_PRESSED BIT(1)
#define DET_INTERRUPT_ENABLED BIT(2)
#define BTNDET_INTERRUPT_ENABLED BIT(3)

#define PM8058_HSED_EN_SIG_MASK               0x3F
#define PM8058_HSED_HYST_PREDIV_MASK          0xC7
#define PM8058_HSED_CLK_PREDIV_MASK           0xF8
#define PM8058_HSED_HYST_CLK_MASK             0x0F
#define PM8058_HSED_PERIOD_CLK_MASK           0xF0

#define PM8058_HSED_HYST_PREDIV               0x38
#define PM8058_HSED_CLK_PREDIV                0x07
#define PM8058_HSED_HYST_CLK                  0xF0
#define PM8058_HSED_PERIOD_CLK                0x0F

#define PM8058_HSED_EN_SIG_SHIFT              0x6
#define PM8058_HSED_HYST_PREDIV_SHIFT         0x3
#define PM8058_HSED_HYST_CLK_SHIFT            0x4

#define PM8058_HSED_SET_TRIG_LEVEL_MIN        200
#define PM8058_HSED_SET_TRIG_LEVEL_MAX        1700

#define HSED_CONTREG1(addr)                   (addr + 1)
#define HSED_CONTREG2(addr)                   (addr + 2)

#define CALC_REG_TO_LEVEL(value)              ((value + 2) * 100)
#define CALC_LEVEL_TO_REG(value)              (value / 100 - 2)
#define CALC_UV_TO_MV(uv)                     (uv / 1000)

#define WAIT_MIC_BIAS_VOLTAGE_STABLE_DELAY    20

#define PA_HPH_EARPA_MSTB_OUT_EN              0x31
#define PA_HPH_CTL2                           0x4C

#define LOCK(x)							\
	do {								\
		dev_dbg(loc_dat->dev, "%s - %d Locking mutex\n", __func__, \
			__LINE__);					\
		mutex_lock(x);						\
	} while (0)

#define UNLOCK(x)							\
	do {								\
		dev_dbg(loc_dat->dev, "%s - %d Unlocking mutex\n",	\
		       __func__, __LINE__);				\
		mutex_unlock(x);					\
	} while (0)

#define TRY_LOCK(x)				mutex_trylock(x)

struct params {
	unsigned int hr_value;
	unsigned int enum_value;
};

enum hsed_micbias_enable {
	HSED_SIGNAL_OFF,
	HSED_SIGNAL_TCXO,
	HSED_SIGNAL_PWM_TCXO,
	HSED_SIGNAL_ALWAYS_ON,
};

static const struct params period_time_vals[] = {
	{
		.hr_value = 1,
		.enum_value = 0,
	},
	{
		.hr_value = 2,
		.enum_value = 1,
	},
	{
		.hr_value = 3,
		.enum_value = 2,
	},
	{
		.hr_value = 4,
		.enum_value = 3,
	},
	{
		.hr_value = 5,
		.enum_value = 4,
	},
	{
		.hr_value = 6,
		.enum_value = 5,
	},
	{
		.hr_value = 7,
		.enum_value = 6,
	},
	{
		.hr_value = 8,
		.enum_value = 7,
	},
	{
		.hr_value = 9,
		.enum_value = 8,
	},
	{
		.hr_value = 10,
		.enum_value = 9,
	},
	{
		.hr_value = 11,
		.enum_value = 10,
	},
	{
		.hr_value = 12,
		.enum_value = 11,
	},
	{
		.hr_value = 13,
		.enum_value = 12,
	},
	{
		.hr_value = 14,
		.enum_value = 13,
	},
	{
		.hr_value = 15,
		.enum_value = 14,
	},
	{
		.hr_value = 16,
		.enum_value = 15,
	},
};

static const struct params hyst_time_vals[] = {
	{
		.hr_value = 1,
		.enum_value = 0,
	},
	{
		.hr_value = 2,
		.enum_value = 1,
	},
	{
		.hr_value = 3,
		.enum_value = 2,
	},
	{
		.hr_value = 4,
		.enum_value = 3,
	},
	{
		.hr_value = 5,
		.enum_value = 4,
	},
	{
		.hr_value = 6,
		.enum_value = 5,
	},
	{
		.hr_value = 7,
		.enum_value = 6,
	},
	{
		.hr_value = 8,
		.enum_value = 7,
	},
	{
		.hr_value = 9,
		.enum_value = 8,
	},
	{
		.hr_value = 10,
		.enum_value = 9,
	},
	{
		.hr_value = 11,
		.enum_value = 10,
	},
	{
		.hr_value = 12,
		.enum_value = 11,
	},
	{
		.hr_value = 13,
		.enum_value = 12,
	},
	{
		.hr_value = 14,
		.enum_value = 13,
	},
	{
		.hr_value = 15,
		.enum_value = 14,
	},
	{
		.hr_value = 16,
		.enum_value = 15,
	},
};

static const struct params period_freq_vals[] = {
	{
		.hr_value = 4,
		.enum_value = 7,
	},
	{
		.hr_value = 8,
		.enum_value = 6,
	},
	{
		.hr_value = 16,
		.enum_value = 5,
	},
	{
		.hr_value = 32,
		.enum_value = 4,
	},
	{
		.hr_value = 64,
		.enum_value = 3,
	},
	{
		.hr_value = 128,
		.enum_value = 2,
	},
	{
		.hr_value = 256,
		.enum_value = 1,
	},
	{
		.hr_value = 512,
		.enum_value = 0,
	},
};

static const struct params hyst_freq_vals[] = {
	{
		.hr_value = 8,
		.enum_value = 7,
	},
	{
		.hr_value = 16,
		.enum_value = 6,
	},
	{
		.hr_value = 32,
		.enum_value = 5,
	},
	{
		.hr_value = 64,
		.enum_value = 4,
	},
	{
		.hr_value = 128,
		.enum_value = 3,
	},
	{
		.hr_value = 256,
		.enum_value = 2,
	},
	{
		.hr_value = 512,
		.enum_value = 1,
	},
	{
		.hr_value = 1024,
		.enum_value = 0,
	},
};



struct local_data {
	struct simple_remote_platform_data *jack_pf;
	long unsigned int simple_remote_pf_flags;
	struct mutex lock;

	/* HSED mic bias config info */
	unsigned int trigger_level;
	unsigned int hsed_hyst_prediv_us;
	unsigned int hsed_period_clkdiv_us;
	unsigned int hsed_hyst_clk_us;
	unsigned int hsed_period_clk_us;

	struct device *dev;
	struct platform_device *device;

	int mic_bias_enable_counter;

	u8 x31_orig_val;
	u8 x4c_orig_val;

	bool hpamp_enabled;
};

static struct local_data *loc_dat;

static int get_param_value(const struct params *parm, int parm_size,
			   u8 enum_val, unsigned int value)
{
	int i;

	for (i = 0; i < parm_size; i++) {
		if (enum_val) {
			if (parm[i].hr_value >= value)
				return parm[i].enum_value;
		} else {
			if (value == parm[i].enum_value)
				return parm[i].hr_value;
		}
	}

	return -EINVAL;
}

static int simple_remote_pf_read_hsd_adc(unsigned int *adc_value)
{
	int err;
	void *adc_handle;
	struct adc_chan_result adc_result;
	struct completion adc_complete_evt;

	LOCK(&loc_dat->lock);

	err = adc_channel_open(loc_dat->jack_pf->adc_channel, &adc_handle);
	if (err) {
		pr_err("%s: couldnt open channel %d err=%d\n",
			__func__, loc_dat->jack_pf->adc_channel, err);
		goto bail_out;
	}

	init_completion(&adc_complete_evt);

	err = adc_channel_request_conv(adc_handle, &adc_complete_evt);
	if (err) {
		dev_err(loc_dat->dev,
			"adc_channel_request_conv failed\n");
		goto out;
	}
	err = wait_for_completion_interruptible(&adc_complete_evt);
	if (err) {
		dev_err(loc_dat->dev,
			"wait_for_completion_interruptible failed\n");
		goto out;
	}
	err = adc_channel_read_result(adc_handle, &adc_result);
	if (err) {
		dev_err(loc_dat->dev,
			"adc_channel_read_result failed\n");
		goto out;
	}
	if (adc_value)
		*adc_value = adc_result.physical;

out:
	err = adc_channel_close(adc_handle);
	if (err)
		pr_err("%s: couldnt close channel %d err=%d\n",
			__func__, loc_dat->jack_pf->adc_channel, err);

bail_out:
	UNLOCK(&loc_dat->lock);
	return err;
}


static int simple_remote_pf_enable_mic_bias(unsigned int enable)
{
	int rc = 0;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;

	LOCK(&loc_dat->lock);

	dev_dbg(loc_dat->dev, "%s - %s MIC Bias\n", __func__,
		enable ? "Enabling" : "Disabling");

	dev_vdbg(loc_dat->dev, "%s - MIC_BIAS_COUNTER = %d\n", __func__,
		 loc_dat->mic_bias_enable_counter);

	if (enable) {
		if (!loc_dat->mic_bias_enable_counter) {
			rc = pm8xxx_readb(loc_dat->dev->parent,
					  HSED_CONTREG1(base_addr), &reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8058 read failed\n");

			reg &= PM8058_HSED_EN_SIG_MASK;
			reg |= (HSED_SIGNAL_PWM_TCXO <<
				 PM8058_HSED_EN_SIG_SHIFT);

			rc = pm8xxx_writeb(loc_dat->dev->parent,
					   HSED_CONTREG1(base_addr), reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8058 write failed\n");

			/* Wait a bit to get basicly stable ADC value */
			msleep(WAIT_MIC_BIAS_VOLTAGE_STABLE_DELAY);
		}
		if (!rc) {
			dev_vdbg(loc_dat->dev,
				"%s - Increasing MIC_BIAS_COUNTER\n",
				__func__);
			loc_dat->mic_bias_enable_counter++;
			dev_vdbg(loc_dat->dev,
				 "%s - MIC_BIAS_COUNTER = %u\n", __func__,
				 loc_dat->mic_bias_enable_counter);
		}
	} else {
		if (1 == loc_dat->mic_bias_enable_counter) {
			rc = pm8xxx_readb(loc_dat->dev->parent,
					  HSED_CONTREG1(base_addr), &reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8058 read failed\n");

			reg &= PM8058_HSED_EN_SIG_MASK;
			reg |= (HSED_SIGNAL_OFF << PM8058_HSED_EN_SIG_SHIFT);

			rc = pm8xxx_writeb(loc_dat->dev->parent,
					   HSED_CONTREG1(base_addr), reg);
			if (rc < 0)
				dev_err(loc_dat->dev, "PM8058 write failed\n");

			if (!rc)
				dev_dbg(loc_dat->dev,
					"%s - MIC Bias disabled\n",
					__func__);
		}
		if (1 <= loc_dat->mic_bias_enable_counter) {
			dev_vdbg(loc_dat->dev,
				"%s - Decreasing MIC_BIAS_COUNTER\n",
				__func__);
			loc_dat->mic_bias_enable_counter--;
			dev_vdbg(loc_dat->dev, "%s - MIC_BIAS_COUNTER = %d\n",
			       __func__,
			       loc_dat->mic_bias_enable_counter);
		} else {
			dev_vdbg(loc_dat->dev, "%s - No need to decrease "
				"MIC_BIAS_COUNTER\n", __func__);
			dev_vdbg(loc_dat->dev, "%s - MIC_BIAS_COUNTER = %u\n",
				 __func__,
				 loc_dat->mic_bias_enable_counter);
		}
	}

	if (rc)
		dev_err(loc_dat->dev, "Unable to toggle MIC Bias\n");

	UNLOCK(&loc_dat->lock);

	return rc;
}


static int simple_remote_pf_set_period_freq(unsigned int value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int ret_val = get_param_value(period_freq_vals,
				ARRAY_SIZE(period_freq_vals), 1, value);

	if (ret_val < 0) {
		dev_err(loc_dat->dev, "%s - Failed to get PMIC value (%u)\n",
			__func__, ret_val);
		return ret_val;
	}

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG1(base_addr), &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg = (reg &  PM8058_HSED_CLK_PREDIV_MASK) | (ret_val);

	ret = pm8xxx_writeb(loc_dat->dev->parent,
			    HSED_CONTREG1(base_addr), reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 write failed\n");
		return ret;
	}

	return 0;
}


static int simple_remote_pf_set_period_time(unsigned int value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int ret_val = get_param_value(period_time_vals,
				ARRAY_SIZE(period_time_vals), 1, value);

	if (ret_val < 0) {
		dev_err(loc_dat->dev, "%s - Failed to get PMIC value (%u)\n",
			__func__, value);
		return ret_val;
	}

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG2(base_addr), &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg = (reg & PM8058_HSED_PERIOD_CLK_MASK) | ret_val;

	ret = pm8xxx_writeb(loc_dat->dev->parent,
			    HSED_CONTREG2(base_addr), reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 write failed\n");
		return ret;
	}

	return 0;
}


static int simple_remote_pf_set_hysteresis_freq(unsigned int value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int ret_val = get_param_value(hyst_freq_vals,
			ARRAY_SIZE(hyst_freq_vals),
			1, value);

	if (ret_val < 0) {
		dev_err(loc_dat->dev, "%s - Failed to get PMIC value (%u)\n",
			__func__, ret_val);
		return ret_val;
	}

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG1(base_addr), &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg &= PM8058_HSED_HYST_PREDIV_MASK;
	reg |= (ret_val << PM8058_HSED_HYST_PREDIV_SHIFT);

	ret = pm8xxx_writeb(loc_dat->dev->parent,
			    HSED_CONTREG1(base_addr), reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 write failed\n");
		return ret;
	}

	return 0;
}


static int simple_remote_pf_set_hysteresis_time(unsigned int value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int ret_val = get_param_value(hyst_time_vals,
			ARRAY_SIZE(hyst_time_vals),
			1, value);

	if (ret_val < 0) {
		dev_err(loc_dat->dev, "%s - Failed to get PMIC value (%u)\n",
			__func__, ret_val);
		return ret_val;
	}

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG2(base_addr), &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg &= PM8058_HSED_HYST_CLK_MASK;
	reg |= ret_val << PM8058_HSED_HYST_CLK_SHIFT;

	ret = pm8xxx_writeb(loc_dat->dev->parent,
			    HSED_CONTREG2(base_addr), reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 write failed\n");
		return ret;
	}

	return 0;
}


static int simple_remote_pf_set_trig_level(unsigned int value)
{
	int ret = -EINVAL;
	u16 base_addr = loc_dat->jack_pf->hsed_base;

	if (value >= PM8058_HSED_SET_TRIG_LEVEL_MIN &&
		value <= PM8058_HSED_SET_TRIG_LEVEL_MAX) {

		ret = pm8xxx_writeb(loc_dat->dev->parent,
				    base_addr, CALC_LEVEL_TO_REG(value));
		if (ret < 0) {
			dev_err(loc_dat->dev, "PM8058 write failed\n");
			return ret;
		}
		loc_dat->trigger_level = value;
	} else {
		dev_warn(loc_dat->dev, "Trig level out of range\n");
	}

	return ret;
}


static int simple_remote_pf_get_period_freq(unsigned int *value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int val;

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG1(base_addr), &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg = (reg &  PM8058_HSED_CLK_PREDIV);

	val = get_param_value(period_freq_vals,
			  ARRAY_SIZE(period_freq_vals), 0,
			  (unsigned int)reg);

	if (0 > val)
		return val;

	*value = val;

	return 0;
}


static int simple_remote_pf_get_period_time(unsigned int *value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int val;

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG2(base_addr), &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg = (reg & PM8058_HSED_PERIOD_CLK);

	val = get_param_value(period_time_vals,
			  ARRAY_SIZE(period_time_vals), 0,
			  (unsigned int)reg);

	if (0 > val)
		return val;

	*value = val;

	return 0;
}


static int simple_remote_pf_get_hysteresis_freq(unsigned int *value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int val;

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG1(base_addr), &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg &= PM8058_HSED_HYST_PREDIV;
	reg = (reg >> PM8058_HSED_HYST_PREDIV_SHIFT);

	val = get_param_value(hyst_freq_vals, ARRAY_SIZE(hyst_freq_vals),
			  0, (unsigned int)reg);

	if (0 > val)
		return val;

	*value = val;

	return 0;
}


static int simple_remote_pf_get_hysteresis_time(unsigned int *value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int val;

	ret = pm8xxx_readb(loc_dat->dev->parent,
			   HSED_CONTREG2(base_addr) , &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	reg &= PM8058_HSED_HYST_CLK;
	reg = reg >> PM8058_HSED_HYST_CLK_SHIFT;

	val = get_param_value(hyst_time_vals, ARRAY_SIZE(hyst_time_vals),
			  0, (unsigned int)reg);

	if (0 > val)
		return val;

	*value = val;

	return 0;
}


static int simple_remote_pf_get_trig_level(unsigned int *value)
{
	int ret = -EINVAL;
	u8 reg;
	u16 base_addr = loc_dat->jack_pf->hsed_base;
	int val;

	ret = pm8xxx_readb(loc_dat->dev->parent, base_addr, &reg);
	if (ret < 0) {
		dev_err(loc_dat->dev, "PM8058 read failed\n");
		return ret;
	}

	val = CALC_REG_TO_LEVEL(reg);

	*value = val;
	return 0;
}


static int simple_remote_pf_get_current_plug_status(u8 *status)
{
	*status = gpio_get_value(loc_dat->jack_pf->headset_detect_read_pin);
	if (loc_dat->jack_pf->invert_plug_det)
		*status = !(*status);

	dev_dbg(loc_dat->dev,
		"%s - Read GPIO status = %u\n", __func__, *status);

	return 0;
}


static int simple_remote_pf_enable_vregs(u8 enable)
{
	int i;
	dev_dbg(loc_dat->dev, "%s - %s vregs\n", __func__,
		enable ? "Enabling" : "Disabling");

	if (enable) {
		/* Turn them on in one direction */
		for (i = 0; i < loc_dat->jack_pf->num_regs; i++) {
			if (regulator_enable(loc_dat->jack_pf->regs[i].reg)) {
				dev_err(loc_dat->dev,
					"%s - Failed to enable regulator %s\n",
					__func__,
					loc_dat->jack_pf->regs[i].name);
				return -EFAULT;
			}
		}
	} else {
		/* And turn them off the other way */
		for (i = loc_dat->jack_pf->num_regs - 1; i >= 0; i--) {
			if (regulator_disable(loc_dat->jack_pf->regs[i].reg)) {
				dev_err(loc_dat->dev,
					"%s - Failed to enable regulator %s\n",
					__func__,
					loc_dat->jack_pf->regs[i].name);
				return -EFAULT;
			}
		}
	}
	return 0;
}


static int simple_remote_pf_enable_hp_amp(u8 enable)
{
	int rc = 0;
	struct marimba config = { .mod_id = MARIMBA_SLAVE_ID_CDC };
	u8 reg_data = 0;

	dev_dbg(loc_dat->dev, "%s - %s HP amp\n", __func__,
	       enable ? "Enabling" : "Disabling");

	LOCK(&loc_dat->lock);
	if (enable) {
		if (loc_dat->hpamp_enabled)
			goto same_state;

		simple_remote_pf_enable_vregs(1);
		adie_codec_powerup(1);

		if (0 > marimba_read(&config, PA_HPH_CTL2,
				     &loc_dat->x4c_orig_val, 1)) {
			dev_err(loc_dat->dev,
				"%s - Failed to read register 0x4C\n",
				 __func__);
			rc = -EIO;
			goto error;
		}

		if (0 > marimba_read(&config, PA_HPH_EARPA_MSTB_OUT_EN,
				     &loc_dat->x31_orig_val, 1)) {
			dev_err(loc_dat->dev,
				"%s - Failed to read register 0x31\n",
				 __func__);
			rc = -EIO;
			goto error;
		}

		dev_dbg(loc_dat->dev, "%s - reg 0x4C = 0x%02X\n", __func__,
			loc_dat->x4c_orig_val);
		dev_dbg(loc_dat->dev, "%s - reg 0x31 = 0x%02X\n", __func__,
			loc_dat->x31_orig_val);

		reg_data = (loc_dat->x31_orig_val | 0x55);
		if (0 > marimba_write(&config, PA_HPH_EARPA_MSTB_OUT_EN,
				      &reg_data, 1)) {
			dev_err(loc_dat->dev,
				"%s - Failed to write register 0x31\n",
				 __func__);
			rc = -EIO;
			goto error;
		}

		reg_data = (loc_dat->x4c_orig_val | 0x29);
		if (0 > marimba_write(&config, PA_HPH_CTL2,
				      &reg_data, 1)) {
			dev_err(loc_dat->dev,
				"%s - Failed to write register 0x4C\n",
				 __func__);
			rc = -EIO;
			goto error;
		}

		reg_data = (loc_dat->x31_orig_val | 0xF5);
		if (0 > marimba_write(&config, PA_HPH_EARPA_MSTB_OUT_EN,
				      &reg_data, 1)) {
			dev_err(loc_dat->dev,
				"%s - Failed to write register 0x31\n",
				 __func__);
			rc = -EIO;
			goto error;
		}

		/* wait a bit to give the HP amp time to start */
		msleep(20);
		loc_dat->hpamp_enabled = true;
	} else {
		if (!loc_dat->hpamp_enabled)
			goto same_state;

		if (0 > marimba_write(&config, PA_HPH_CTL2,
				      &loc_dat->x4c_orig_val, 1)) {
			dev_err(loc_dat->dev,
				"%s - Failed to write register 0x4C\n",
				 __func__);
			rc = -EIO;
			goto error;
		}

		if (0 > marimba_write(&config, PA_HPH_EARPA_MSTB_OUT_EN,
				      &loc_dat->x31_orig_val, 1)) {
			dev_err(loc_dat->dev,
				"%s - Failed to write register 0x31\n",
				 __func__);
			rc = -EIO;
			goto error;
		}

		adie_codec_powerup(0);
		simple_remote_pf_enable_vregs(0);
		loc_dat->hpamp_enabled = false;
	}

same_state:
	dev_info(loc_dat->dev,
		"%s - HP Amp already %s\n", __func__,
		enable ? "enabled" : "disabled");

error:
	if (rc < 0) {
		dev_err(loc_dat->dev,
			"%s - Failed to fully %s HP Amp\n", __func__,
			enable ? "enable" : "disable");
		adie_codec_powerup(0);
		simple_remote_pf_enable_vregs(0);
		loc_dat->hpamp_enabled = false;
	}

	UNLOCK(&loc_dat->lock);

	return rc;
}


static int simple_remote_pf_enable_alternate_mode(u8 enable)
{
	dev_info(loc_dat->dev,
		 "%s - This hardware doesn't support headset CTIA/OMTP mode "
		 "switch\n", __func__);
	return -ENODEV;
}


static int simple_remote_pf_register_plug_detect_interrupt(irq_handler_t func,
							   void *data)
{
	int err = -EALREADY;
	int irq = 0;
	irq_handler_t regfunc = func;

	if (test_bit(DET_INTERRUPT_ENABLED, &loc_dat->simple_remote_pf_flags))
		return err;

	dev_dbg(loc_dat->dev,
		"%s - Interrupt not enabled. Enabling\n", __func__);

	irq = gpio_to_irq(loc_dat->jack_pf->headset_detect_read_pin);
	if (0 <= irq) {
		err = request_threaded_irq(irq, NULL, regfunc,
					   IRQF_TRIGGER_FALLING |
					   IRQF_TRIGGER_RISING,
					   "simple_remote_plug_detect",
					   data);

		if (err) {
			dev_crit(loc_dat->dev, "Failed to subscribe to plug "
				 "detect interrupt\n");
			return err;
		}
	} else {
		dev_crit(loc_dat->dev, "Failed to register interrupt for GPIO "
			 "(%d). GPIO Does not exist\n",
			 loc_dat->jack_pf->headset_detect_read_pin);
		return irq;
	}

	/*
	 * Setting interrupt enabled here, will in worst case generate
	 * a "unmatched irq_wake" print in the kernel log when shutting
	 * down the system, but at least some detection will work.
	 */
	set_bit(DET_INTERRUPT_ENABLED, &loc_dat->simple_remote_pf_flags);

	err = enable_irq_wake(irq);
	if (err)
		dev_crit(loc_dat->dev,
			 "Failed to enable wakeup on interrupt\n");
	else
		dev_dbg(loc_dat->dev,
			"%s - Interrupt successfully registered\n",
			__func__);

	return err;
}


static void simple_remote_pf_unregister_plug_detect_interrupt(void *data)
{
	int irq;
	void *ldat = data;
	if (!test_bit(DET_INTERRUPT_ENABLED, &loc_dat->simple_remote_pf_flags))
		return;

	irq = gpio_to_irq(loc_dat->jack_pf->headset_detect_read_pin);
	if (0 <= irq) {
		disable_irq_wake(irq);
		free_irq(irq, ldat);
		clear_bit(DET_INTERRUPT_ENABLED,
			  &loc_dat->simple_remote_pf_flags);
	} else {
		dev_crit(loc_dat->dev, "Failed to disable plug detect interrupt"
			 ". GPIO (%d) does not exist\n",
			 loc_dat->jack_pf->headset_detect_read_pin);
	}
}

static int simple_remote_pf_register_hssd_button_interrupt(irq_handler_t func,
						    void *data)
{
	int err = -EALREADY;
	int irq = 0;

	if (test_bit(BTNDET_INTERRUPT_ENABLED,
			&loc_dat->simple_remote_pf_flags))
		return err;

	dev_dbg(loc_dat->dev,
		"%s - Interrupt not enabled. Enabling\n", __func__);

	irq = loc_dat->jack_pf->button_detect_irq;

	if (0 <= irq) {
		err = request_threaded_irq(irq, NULL, func,
					IRQF_TRIGGER_FALLING |
					IRQF_TRIGGER_RISING,
					"simple_remote_button_detect",
					data);

		if (err) {
			dev_crit(loc_dat->dev, "Failed to subscribe to "
				"button detect interrupt\n");
			return err;
		}
	} else {
		dev_crit(loc_dat->dev,
			 "Failed to enable button detect interrupt"
			 ". IRQ (%d) does not exist\n",
			 loc_dat->jack_pf->button_detect_irq);
		return irq;
	}

	/*
	 * Setting interrupt enabled here, will in worst case generate
	 * a "unmatched irq_wake" print in the kernel log when shutting
	 * down the system, but at least some detection will work.
	 */
	set_bit(BTNDET_INTERRUPT_ENABLED,
			&loc_dat->simple_remote_pf_flags);

	err = enable_irq_wake(irq);
	if (err)
		dev_crit(loc_dat->dev, "Failed to enable wakeup on "
			"button interrupt\n");
	else
		dev_dbg(loc_dat->dev,
			"%s - Interrupt successfully registered\n",
			__func__);

	return err;
}


static void simple_remote_pf_unregister_hssd_button_interrupt(void *data)
{
	int irq;

	if (!test_bit(BTNDET_INTERRUPT_ENABLED,
		&loc_dat->simple_remote_pf_flags))
		return;

	irq = loc_dat->jack_pf->button_detect_irq;
	if (0 <= irq) {
		disable_irq_wake(irq);
		free_irq(irq, data);
		clear_bit(BTNDET_INTERRUPT_ENABLED,
			&loc_dat->simple_remote_pf_flags);
	} else {
		dev_crit(loc_dat->dev,
		"Failed to disable button detect interrupt"
		". IRQ (%d) does not exist\n",
		loc_dat->jack_pf->button_detect_irq);
	}
}


static struct simple_remote_pf_interface interface = {
	.read_hs_adc = simple_remote_pf_read_hsd_adc,
	.enable_mic_bias = simple_remote_pf_enable_mic_bias,
	.get_current_plug_status = simple_remote_pf_get_current_plug_status,
	.enable_alternate_adc_mode = simple_remote_pf_enable_hp_amp,
	.enable_alternate_headset_mode = simple_remote_pf_enable_alternate_mode,
	.set_period_freq = simple_remote_pf_set_period_freq,
	.set_period_time = simple_remote_pf_set_period_time,
	.set_hysteresis_freq = simple_remote_pf_set_hysteresis_freq,
	.set_hysteresis_time = simple_remote_pf_set_hysteresis_time,
	.set_trig_level = simple_remote_pf_set_trig_level,

	.get_period_freq = simple_remote_pf_get_period_freq,
	.get_period_time = simple_remote_pf_get_period_time,
	.get_hysteresis_freq = simple_remote_pf_get_hysteresis_freq,
	.get_hysteresis_time = simple_remote_pf_get_hysteresis_time,
	.get_trig_level = simple_remote_pf_get_trig_level,

	.register_plug_detect_interrupt =
		simple_remote_pf_register_plug_detect_interrupt,

	.unregister_plug_detect_interrupt =
		simple_remote_pf_unregister_plug_detect_interrupt,

	.register_hssd_button_interrupt =
		simple_remote_pf_register_hssd_button_interrupt,

	.unregister_hssd_button_interrupt =
		simple_remote_pf_unregister_hssd_button_interrupt,
};


static int simple_remote_pf_probe(struct platform_device *pdev)
{
	int ret = -ENOMEM;

	loc_dat = kzalloc(sizeof(*loc_dat), GFP_KERNEL);
	if (!loc_dat)
		return -ENOMEM;

	loc_dat->device = platform_device_alloc(SIMPLE_REMOTE_NAME, -1);
	if (!loc_dat->device)
		goto out;

	loc_dat->jack_pf = pdev->dev.platform_data;
	loc_dat->dev = &pdev->dev;
	loc_dat->device->dev.platform_data = &interface;
	loc_dat->device->dev.parent = &pdev->dev;

	ret = loc_dat->jack_pf->initialize(loc_dat->jack_pf);
	if (ret)
		goto exit_device_put;

	mutex_init(&loc_dat->lock);

	loc_dat->jack_pf->button_detect_irq = platform_get_irq(pdev, 0);
	if (loc_dat->jack_pf->button_detect_irq < 0) {
		ret = -ENXIO;
		goto exit_device_put;
	}

	ret = simple_remote_pf_enable_mic_bias(0);
	if (ret)
		goto exit_device_put;

	ret = platform_device_add(loc_dat->device);
	if (ret)
		goto exit_device_put;

	dev_info(loc_dat->dev, "Successfully registered\n");

	return ret;

exit_device_put:
	platform_device_put(loc_dat->device);
out:
	kfree(loc_dat);
	dev_err(&pdev->dev, "Failed to register driver\n");
	return ret;
}


static int simple_remote_pf_remove(struct platform_device *pdev)
{
	(void)simple_remote_pf_enable_mic_bias(0);
	mutex_destroy(&loc_dat->lock);

	platform_device_unregister(loc_dat->device);

	loc_dat->jack_pf->deinitialize(loc_dat->jack_pf);

	kfree(loc_dat);
	return 0;
}


static struct platform_driver simple_remote_pf = {
	.probe		= simple_remote_pf_probe,
	.remove		= simple_remote_pf_remove,
	.driver		= {
		.name		= SIMPLE_REMOTE_PF_NAME,
		.owner		= THIS_MODULE,
	},
};


static int __init simple_remote_pf_init(void)
{
	return platform_driver_register(&simple_remote_pf);
}


static void __exit simple_remote_pf_exit(void)
{
	platform_driver_unregister(&simple_remote_pf);
}

module_init(simple_remote_pf_init);
module_exit(simple_remote_pf_exit);

MODULE_AUTHOR("Atsushi Iyogi, Joachim Holst, Takashi Shiina, Tadashi Kubo");
MODULE_DESCRIPTION("3.5mm audio jack platform driver");
MODULE_LICENSE("GPL");
