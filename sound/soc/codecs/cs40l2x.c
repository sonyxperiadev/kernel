// SPDX-License-Identifier: GPL-2.0
//
// cs40l2x.c  --  ALSA SoC Audio driver for Cirrus Logic CS40L2x
//
// Copyright 2019 Cirrus Logic Inc.

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/mfd/cs40l2x.h>

#include "cs40l2x.h"

struct cs40l2x_codec;
struct cs40l2x_private;

enum cs40l2x_clk_src {
	CS40L2X_32KHZ_CLK,
	CS40L2X_SCLK
};

struct cs40l2x_codec {
	struct cs40l2x_private *core;
	struct device *dev;
	struct regmap *regmap;
	int codec_sysclk;
	int tuning;
	int tuning_prev;
	char *bin_file;
	bool mute_left;
	bool mute_right;
};

struct cs40l2x_pll_sysclk_config {
	int freq;
	int clk_cfg;
};

static const struct cs40l2x_pll_sysclk_config cs40l2x_pll_sysclk[] = {
	{32768, 0x00},
	{1536000, 0x1b},
	{3072000, 0x21},
};

static int cs40l2x_get_clk_config(int freq)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cs40l2x_pll_sysclk); i++) {
		if (cs40l2x_pll_sysclk[i].freq == freq)
			return cs40l2x_pll_sysclk[i].clk_cfg;
	}

	return -EINVAL;
}

static int cs40l2x_swap_ext_clk(struct cs40l2x_codec *cs40l2x_codec,
					const enum cs40l2x_clk_src src)
{
	struct device *dev = cs40l2x_codec->dev;
	struct regmap *regmap = cs40l2x_codec->regmap;
	int clk_cfg, ret;

	if (src == CS40L2X_32KHZ_CLK)
		clk_cfg = cs40l2x_get_clk_config(CS40L2X_MCLK_FREQ);
	else
		clk_cfg = cs40l2x_get_clk_config(cs40l2x_codec->codec_sysclk);

	if (clk_cfg < 0) {
		dev_err(dev, "Invalid SYS Clock Frequency\n");
		return -EINVAL;
	}

	ret = regmap_update_bits(regmap, CS40L2X_PLL_CLK_CTRL,
		CS40L2X_PLL_OPENLOOP_MASK,
		1 << CS40L2X_PLL_OPENLOOP_SHIFT);
	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, CS40L2X_PLL_CLK_CTRL,
			CS40L2X_REFCLK_FREQ_MASK,
			clk_cfg << CS40L2X_REFCLK_FREQ_SHIFT);
	if (ret)
		return ret;

	if (src == CS40L2X_32KHZ_CLK)
		ret = regmap_update_bits(regmap, CS40L2X_PLL_CLK_CTRL,
			CS40L2X_PLL_CLK_SEL_MASK, CS40L2X_PLLSRC_MCLK);
	else
		ret = regmap_update_bits(regmap, CS40L2X_PLL_CLK_CTRL,
			CS40L2X_PLL_CLK_SEL_MASK, CS40L2X_PLLSRC_SCLK);

	if (ret)
		return ret;

	ret = regmap_update_bits(regmap, CS40L2X_PLL_CLK_CTRL,
			CS40L2X_PLL_OPENLOOP_MASK,
			0 << CS40L2X_PLL_OPENLOOP_SHIFT);
	if (ret)
		return ret;

	usleep_range(1000, 1500);

	return 0;
}

static int cs40l2x_clk_en(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *comp = snd_soc_dapm_to_component(w->dapm);
	struct cs40l2x_codec *codec = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_private *core = codec->core;
	struct device *dev = core->dev;
	int ret;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		pm_runtime_get_sync(core->dev);
		mutex_lock(&codec->core->lock);
		core->a2h_enable = true;
		mutex_unlock(&codec->core->lock);

		if (!completion_done(&core->hap_done)) {
			ret = wait_for_completion_timeout(&core->hap_done,
							msecs_to_jiffies(500));
			if (ret < 0) {
				dev_err(dev, "Error on completion event %d\n",
						ret);
				return ret;
			}

			if (ret == 0)
				dev_dbg(dev, "Completion timeout\n");
		}

		ret = cs40l2x_swap_ext_clk(codec, CS40L2X_SCLK);
		if (ret)
			return ret;
		break;
	case SND_SOC_DAPM_PRE_PMD:
		mutex_lock(&codec->core->lock);
		core->a2h_enable = false;
		mutex_unlock(&codec->core->lock);

		pm_runtime_mark_last_busy(core->dev);
		pm_runtime_put_autosuspend(core->dev);
		break;
	default:
		dev_err(dev, "Invalid event %d\n", event);
		return -EINVAL;
	}

	return 0;
}

static int cs40l2x_reset_mute(struct cs40l2x_codec *codec)
{
	struct regmap *regmap = codec->regmap;
	struct device *dev = codec->dev;
	struct cs40l2x_private *core = codec->core;
	unsigned int reg_l, reg_r;
	int ret = 0;

	reg_l = core->dsp_reg(core, "AUDIOLEFTENABLE",
			CS40L2X_XM_UNPACKED_TYPE,
			CS40L2X_ALGO_ID_A2H);
	if (!reg_l) {
		dev_err(dev, "Cannot find the AUDIOLEFTENABLE register\n");
		return -EINVAL;
	}

	reg_r = core->dsp_reg(core, "AUDIORIGHTENABLE",
			CS40L2X_XM_UNPACKED_TYPE,
			CS40L2X_ALGO_ID_A2H);
	if (!reg_l) {
		dev_err(dev, "Cannot find the AUDIORIGHTENABLE register\n");
		return -EINVAL;
	}

	ret = regmap_write(regmap, reg_l, codec->mute_left ? 0 : 1);
	if (ret)
		return ret;

	ret = regmap_write(regmap, reg_r, codec->mute_right ? 0 : 1);

	return ret;
}

static int cs40l2x_tuning_swap(struct cs40l2x_codec *codec)
{
	struct cs40l2x_private *core = codec->core;
	struct device *dev = codec->dev;
	const struct firmware *fw;
	int ret = 0;

	ret = request_firmware(&fw, codec->bin_file,
				dev);
	if (ret) {
		dev_err(dev, "Failed to request %s file\n",
				codec->bin_file);
		return ret;
	}

	ret = cs40l2x_coeff_file_parse(core, fw);
	if (ret)
		return ret;

	codec->tuning_prev = codec->tuning;

	ret = cs40l2x_reset_mute(codec);

	return ret;
}

static int cs40l2x_a2h_en(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *comp = snd_soc_dapm_to_component(w->dapm);
	struct cs40l2x_codec *codec = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_private *core = codec->core;
	struct regmap *regmap = codec->regmap;
	struct device *dev = codec->dev;
	unsigned int reg;
	int ret = 0;

	if (core->dsp_reg) {
		reg = core->dsp_reg(core, "A2HEN",
				CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);
	} else {
		dev_warn(dev, "DSP is not ready\n");
		return 0;
	}

	if (!reg) {
		dev_err(dev, "Cannot find the A2HENABLED register\n");
		return -EINVAL;
	}

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		if (codec->tuning != codec->tuning_prev) {
			ret = cs40l2x_tuning_swap(codec);
			if (ret)
				return ret;
		}

		ret = regmap_update_bits(regmap, CS40L2X_BSTCVRT_VCTRL2,
					CS40L2X_BST_CTL_SEL_MASK,
					CS40L2X_BST_CTL_SEL_CLASSH);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_PWR_CTRL3,
			CS40L2X_CLASSH_EN_MASK,
			1 << CS40L2X_CLASSH_EN_SHIFT);
		if (ret)
			return ret;

		/* Enable I2S in the DSP */
		ret = regmap_update_bits(regmap, CS40L2X_SP_ENABLES,
					CS40L2X_ASP_RX_ENABLE_MASK,
					CS40L2X_ASP_RX_ENABLE_MASK);
		if (ret)
			return ret;


		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
			CS40L2X_REINIT_A2H_CMD);
		if (ret)
			return ret;

		usleep_range(3000, 3100);
		ret = regmap_write(regmap, reg, CS40L2X_A2H_ENABLE);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP1RX1_INPUT,
					CS40L2X_ROUTE_ASPRX1);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP1RX5_INPUT,
					CS40L2X_ROUTE_ASPRX2);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
					CS40L2X_A2H_I2S_START);
		if (ret)
			return ret;
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* Mute DSP Input RX1 */
		ret = regmap_write(regmap, CS40L2X_DSP1RX1_INPUT, 0);
		if (ret)
			return ret;

		/* Mute DSP Input RX5 */
		ret = regmap_write(regmap, CS40L2X_DSP1RX5_INPUT, 0);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_SP_ENABLES,
					CS40L2X_ASP_RX_ENABLE_MASK, 0);
		if (ret)
			return ret;

		ret = cs40l2x_swap_ext_clk(codec, CS40L2X_32KHZ_CLK);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
					CS40L2X_A2H_I2S_END);
		if (ret)
			return ret;

		ret = regmap_write(regmap, reg, CS40L2X_A2H_DISABLE);
		break;
	default:
		dev_err(dev, "Invalid event %d\n", event);
		return -EINVAL;
	}
	return ret;
}

static int cs40l2x_vol_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_private *core = cs40l2x->core;
	unsigned int val = 0, reg;
	int ret;

	if (!core->dsp_reg) {
		dev_warn(dev, "DSP not available\n");
		return 0;
	}

	reg = core->dsp_reg(core, "VOLUMELEVEL",
			CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);
	if (!reg) {
		dev_err(dev, "Cannot the the VOLUMELEVEL register\n");
		return -EINVAL;
	}

	pm_runtime_get_sync(core->dev);
	ret = regmap_read(regmap, reg, &val);
	if (ret)
		goto vol_get_err;

	if (val == CS40L2X_VOL_LVL_MAX)
		val = CS40L2X_VOL_LVL_MAX_STEPS;
	else
		val /= (CS40L2X_VOL_LVL_MAX / CS40L2X_VOL_LVL_MAX_STEPS);

vol_get_err:
	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);

	ucontrol->value.integer.value[0] = val;
	return ret;
}

static int cs40l2x_vol_put(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_private *core = cs40l2x->core;
	unsigned int val, reg;
	int ret;

	if (!core->dsp_reg) {
		dev_warn(dev, "DSP not available\n");
		return 0;
	}

	reg = core->dsp_reg(core, "VOLUMELEVEL",
			CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);

	if (!reg) {
		dev_err(dev, "Cannot the the VOLUMELEVEL register\n");
		return -EINVAL;
	}

	val = ucontrol->value.integer.value[0];

	if (val == CS40L2X_VOL_LVL_MAX_STEPS)
		val = CS40L2X_VOL_LVL_MAX;
	else
		val *= (CS40L2X_VOL_LVL_MAX / CS40L2X_VOL_LVL_MAX_STEPS);

	pm_runtime_get_sync(core->dev);

	ret = regmap_write(regmap, reg, val);

	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);
	return ret;
}

static int cs40l2x_tuning_get(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);

	ucontrol->value.enumerated.item[0] = cs40l2x->tuning;

	return 0;
}

static int cs40l2x_tuning_put(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_private *core = cs40l2x->core;
	struct device *dev = cs40l2x->dev;
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int reg;
	int ret;

	if (ucontrol->value.enumerated.item[0] == cs40l2x->tuning)
		return 0;

	cs40l2x->tuning = ucontrol->value.enumerated.item[0];

	memset(cs40l2x->bin_file, 0, PAGE_SIZE);
	cs40l2x->bin_file[PAGE_SIZE - 1] = '\0';

	if (cs40l2x->tuning > 0)
		snprintf(cs40l2x->bin_file, PAGE_SIZE, "cs40l25a_a2h%d.bin",
				cs40l2x->tuning);
	else
		snprintf(cs40l2x->bin_file, PAGE_SIZE, "cs40l25a_a2h.bin");

	if (core->a2h_enable && (cs40l2x->tuning != cs40l2x->tuning_prev)) {
		if (core->dsp_reg) {
			reg = core->dsp_reg(core, "A2HEN",
				CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);
		} else {
			dev_warn(dev, "DSP is not ready\n");
			return 0;
		}

		if (!reg) {
			dev_err(dev, "Cannot find the A2HENABLED register\n");
			return -EINVAL;
		}

		/* Mute DSP Input */
		ret = regmap_write(regmap, CS40L2X_DSP1RX1_INPUT, 0);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP1RX5_INPUT, 0);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
					CS40L2X_A2H_I2S_END);
		if (ret)
			return ret;

		ret = cs40l2x_tuning_swap(cs40l2x);
		if (ret)
			return ret;

		ret = regmap_write(regmap, reg, CS40L2X_A2H_DISABLE);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
			CS40L2X_REINIT_A2H_CMD);
		if (ret)
			return ret;

		ret = regmap_write(regmap, reg, CS40L2X_A2H_ENABLE);
		if (ret)
			return ret;

		usleep_range(3000, 3100);
		ret = regmap_write(regmap, CS40L2X_DSP1RX1_INPUT, CS40L2X_ROUTE_ASPRX1);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP1RX5_INPUT, CS40L2X_ROUTE_ASPRX2);
		if (ret)
			return ret;

		ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
					CS40L2X_A2H_I2S_START);
		if (ret)
			return ret;
	}

	return 0;
}

static int cs40l2x_delay_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_private *core = cs40l2x->core;
	unsigned int val = 0, reg;
	int ret;

	if (!core->dsp_reg) {
		dev_warn(dev, "DSP not available\n");
		return 0;
	}

	reg = core->dsp_reg(core, "LRADELAYSAMPS",
			CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);
	if (!reg) {
		dev_err(dev, "Cannot find the LRADELAYSAMPS register\n");
		return -EINVAL;
	}

	pm_runtime_get_sync(core->dev);
	ret = regmap_read(regmap, reg, &val);
	if (ret)
		goto vol_get_err;

	ucontrol->value.integer.value[0] = val;
vol_get_err:
	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);

	return ret;
}

static int cs40l2x_delay_put(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_private *core = cs40l2x->core;
	unsigned int val, reg, reg_a2h;
	int ret;

	if (!core->dsp_reg) {
		dev_warn(dev, "DSP not available\n");
		return 0;
	}

	reg = core->dsp_reg(core, "LRADELAYSAMPS",
			CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);

	if (!reg) {
		dev_err(dev, "Cannot find the LRADELAYSAMPS register\n");
		return -EINVAL;
	}

	reg_a2h = core->dsp_reg(core, "A2HEN",
				CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);

	if (!reg_a2h) {
		dev_err(dev, "Cannot find the A2HEN register\n");
		return -EINVAL;
	}

	val = ucontrol->value.integer.value[0];

	/* Mute DSP Input RX1 and RX5 */
	ret = regmap_write(regmap, CS40L2X_DSP1RX1_INPUT, 0);
	if (ret)
		return ret;

	ret = regmap_write(regmap, CS40L2X_DSP1RX5_INPUT, 0);
	if (ret)
		return ret;

	ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
				CS40L2X_A2H_I2S_END);
	if (ret)
		return ret;

	ret = regmap_write(regmap, reg_a2h, CS40L2X_A2H_DISABLE);
	if (ret)
		return ret;

	ret = regmap_write(regmap, reg, val);
	if (ret)
		return ret;

	ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
			CS40L2X_REINIT_A2H_CMD);
	if (ret)
		return ret;

	usleep_range(3000, 3100);
	ret = regmap_write(regmap, reg_a2h, CS40L2X_A2H_ENABLE);
	if (ret)
		return ret;

	ret = regmap_write(regmap, CS40L2X_DSP_VIRT1_MBOX_5,
				CS40L2X_A2H_I2S_START);
	if (ret)
		return ret;

	ret = regmap_write(regmap, CS40L2X_DSP1RX1_INPUT,
				CS40L2X_ROUTE_ASPRX1);
	if (ret)
		return ret;

	ret = regmap_write(regmap, CS40L2X_DSP1RX5_INPUT,
				CS40L2X_ROUTE_ASPRX2);
	return ret;
}

static int cs40l2x_mute_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_private *core = cs40l2x->core;
	unsigned int val_r, val_l, reg_l, reg_r;
	int ret = 0;

	if (!core->dsp_reg) {
		dev_warn(dev, "DSP not available\n");
		return 0;
	}

	reg_l = core->dsp_reg(core, "AUDIOLEFTENABLE",
			CS40L2X_XM_UNPACKED_TYPE,
			CS40L2X_ALGO_ID_A2H);
	if (!reg_l) {
		dev_err(dev, "Cannot find the AUDIOLEFTENABLE register\n");
		return -EINVAL;
	}

	reg_r = core->dsp_reg(core, "AUDIORIGHTENABLE",
			CS40L2X_XM_UNPACKED_TYPE,
			CS40L2X_ALGO_ID_A2H);
	if (!reg_l) {
		dev_err(dev, "Cannot find the AUDIORIGHTENABLE register\n");
		return -EINVAL;
	}
	pm_runtime_get_sync(core->dev);
	ret = regmap_read(regmap, reg_l, &val_l);
	if (ret)
		goto mute_get_err;

	ret = regmap_read(regmap, reg_r, &val_r);
	if (ret)
		goto mute_get_err;

	ucontrol->value.integer.value[0] = !val_l;
	ucontrol->value.integer.value[1] = !val_r;

mute_get_err:
	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);

	return ret;
}

static int cs40l2x_mute_put(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = cs40l2x->regmap;
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_private *core = cs40l2x->core;
	unsigned int val, reg_l, reg_r;
	int ret;

	if (!core->dsp_reg) {
		dev_warn(dev, "DSP not available\n");
		return 0;
	}

	reg_l = core->dsp_reg(core, "AUDIOLEFTENABLE",
			CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);
	if (!reg_l) {
		dev_err(dev, "Cannot find the AUDIOLEFTENABLE register\n");
		return -EINVAL;
	}

	reg_r = core->dsp_reg(core, "AUDIORIGHTENABLE",
			CS40L2X_XM_UNPACKED_TYPE,
				CS40L2X_ALGO_ID_A2H);
	if (!reg_l) {
		dev_err(dev, "Cannot find the AUDIORIGHTENABLE register\n");
		return -EINVAL;
	}

	val = ucontrol->value.integer.value[0];

	pm_runtime_get_sync(core->dev);
	ret = regmap_write(regmap, reg_l, val ? 0 : 1);
	if (ret)
		goto mute_put_err;

	cs40l2x->mute_left = val;

	val = ucontrol->value.integer.value[1];

	ret = regmap_write(regmap, reg_r, val ? 0 : 1);
	if (ret)
		goto mute_put_err;

	cs40l2x->mute_right = val;
mute_put_err:
	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);

	return ret;
}

static const struct snd_kcontrol_new cs40l2x_a2h =
	SOC_DAPM_SINGLE("Switch", SND_SOC_NOPM, 0, 1, 0);

static const struct snd_kcontrol_new cs40l2x_controls[] = {
	SOC_SINGLE_EXT("A2H Volume Level", 0, 0, CS40L2X_VOL_LVL_MAX_STEPS, 0,
		cs40l2x_vol_get, cs40l2x_vol_put),
	SOC_SINGLE_EXT("A2H Tuning", 0, 0, CS40L2X_A2H_MAX_TUNING, 0,
		cs40l2x_tuning_get, cs40l2x_tuning_put),
	SOC_SINGLE_EXT("A2H Delay", 0, 0, CS40L2X_A2H_DELAY_MAX, 0,
		cs40l2x_delay_get, cs40l2x_delay_put),
	SOC_DOUBLE_EXT("A2H Mute Switch", 0, 0, 1, 1, 0,
		cs40l2x_mute_get, cs40l2x_mute_put),
};

static const struct snd_soc_dapm_widget cs40l2x_dapm_widgets[] = {
	SND_SOC_DAPM_SUPPLY_S("AIFCLK", 100, SND_SOC_NOPM, 0, 0,
		cs40l2x_clk_en, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),

	/* ASPRX1 is always used in A2H */
	SND_SOC_DAPM_AIF_IN_E("ASPRX1", NULL, 0, SND_SOC_NOPM, 0, 0,
		cs40l2x_a2h_en, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
	SND_SOC_DAPM_AIF_IN("ASPRX2", NULL, 0, SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_MIXER("A2H Mixer", SND_SOC_NOPM, 0, 0, NULL, 0),
	SND_SOC_DAPM_SWITCH("A2H", SND_SOC_NOPM, 0, 0, &cs40l2x_a2h),
	SND_SOC_DAPM_OUTPUT("LRA"),
};

static const struct snd_soc_dapm_route cs40l2x_dapm_routes[] = {
	{ "ASPRX1", NULL, "AIF Playback" },
	{ "ASPRX2", NULL, "AIF Playback" },
	{ "A2H Mixer", NULL, "ASPRX1" },
	{ "A2H Mixer", NULL, "ASPRX2" },
	{ "A2H", "Switch", "A2H Mixer" },
	{ "LRA", NULL, "A2H" },

	{ "AIF Playback", NULL, "AIFCLK" },
};

static int cs40l2x_component_set_sysclk(struct snd_soc_component *component,
					int clk_id, int source,
					unsigned int freq, int dir)
{
	struct cs40l2x_codec *codec = snd_soc_component_get_drvdata(component);
	struct regmap *regmap = codec->regmap;
	struct cs40l2x_private *core = codec->core;
	struct device *dev = codec->dev;
	int clk_cfg, ret = 0;

	clk_cfg = cs40l2x_get_clk_config(freq);
	if (clk_cfg < 0) {
		dev_err(dev, "Invalid Clock Frequency %d\n", freq);
		ret = -EINVAL;
		goto sysclk_err;
	}

	switch (clk_id) {
	case 0:
		break;
	default:
		dev_err(dev, "Invalid Input Clock\n");
		ret = -EINVAL;
		goto sysclk_err;
	}

	codec->codec_sysclk = freq;
	pm_runtime_get_sync(core->dev);
	ret = regmap_write(regmap, CS40L2X_SP_RATE_CTRL, clk_cfg);
	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);
sysclk_err:

	return ret;
}

static const unsigned int cs40l2x_src_rates[] = { 48000 };

static const struct snd_pcm_hw_constraint_list cs40l2x_constraints = {
	.count = ARRAY_SIZE(cs40l2x_src_rates),
	.list = cs40l2x_src_rates,
};

static int cs40l2x_pcm_startup(struct snd_pcm_substream *substream,
			       struct snd_soc_dai *dai)
{
	int ret = 0;

	if (substream->runtime)
		ret = snd_pcm_hw_constraint_list(substream->runtime,
							0,
							SNDRV_PCM_HW_PARAM_RATE,
							&cs40l2x_constraints);

	return ret;
}

static int cs40l2x_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
	struct cs40l2x_codec *cs40l2x =
		snd_soc_component_get_drvdata(codec_dai->component);
	struct device *dev = cs40l2x->dev;
	struct cs40l2x_private *core = cs40l2x->core;
	struct regmap *regmap = cs40l2x->regmap;
	unsigned int lrclk_fmt, sclk_fmt;
	int ret;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		dev_err(dev, "This device can be slave only\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		break;
	default:
		dev_err(dev, "Invalid format. I2S only.\n");
		return -EINVAL;
	}
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_IF:
		lrclk_fmt = 1;
		sclk_fmt = 0;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		lrclk_fmt = 0;
		sclk_fmt = 1;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		lrclk_fmt = 1;
		sclk_fmt = 1;
		break;
	case SND_SOC_DAIFMT_NB_NF:
		lrclk_fmt = 0;
		sclk_fmt = 0;
		break;
	default:
		dev_err(dev,
			"%s: Invalid DAI clock INV\n", __func__);
		return -EINVAL;
	}

	pm_runtime_get_sync(core->dev);
	ret = regmap_update_bits(regmap, CS40L2X_SP_FORMAT,
					CS40L2X_LRCLK_INV_MASK,
					lrclk_fmt << CS40L2X_LRCLK_INV_SHIFT);
	if (ret)
		goto fmt_err;

	ret = regmap_update_bits(regmap, CS40L2X_SP_FORMAT,
					CS40L2X_SCLK_INV_MASK,
					sclk_fmt << CS40L2X_SCLK_INV_SHIFT);
fmt_err:
	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);
	return ret;
}

static int cs40l2x_pcm_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params,
				 struct snd_soc_dai *dai)
{
	struct snd_soc_component *comp = dai->component;
	struct cs40l2x_codec *cs40l2x = snd_soc_component_get_drvdata(comp);
	unsigned int asp_width, asp_wl;
	int ret = 0;

	asp_wl = params_width(params);
	asp_width = params_physical_width(params);

	if (substream->stream != SNDRV_PCM_STREAM_PLAYBACK) {
		ret = -EINVAL;
		goto hw_params_err;
	}

	regmap_update_bits(cs40l2x->regmap, CS40L2X_SP_FORMAT,
			CS40L2X_ASP_WIDTH_RX_MASK,
			asp_width << CS40L2X_ASP_WIDTH_RX_SHIFT);
	regmap_update_bits(cs40l2x->regmap, CS40L2X_SP_RX_WL,
			CS40L2X_ASP_RX_WL_MASK,
			asp_wl);

hw_params_err:
	return ret;
}


#define CS40L2X_RATES SNDRV_PCM_RATE_48000

#define CS40L2X_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

static const struct snd_soc_dai_ops cs40l2x_dai_ops = {
	.startup = cs40l2x_pcm_startup,
	.set_fmt = cs40l2x_set_dai_fmt,
	.hw_params = cs40l2x_pcm_hw_params,
};

static struct snd_soc_dai_driver cs40l2x_dai[] = {
	{
		.name = "cs40l2x-pcm",
		.id = 0,
		.playback = {
			.stream_name = "AIF Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = CS40L2X_RATES,
			.formats = CS40L2X_FORMATS,
		},
		.ops = &cs40l2x_dai_ops,
		.symmetric_rates = 1,
	},
};

static int cs40l2x_codec_probe(struct snd_soc_component *component)
{
	struct cs40l2x_codec *codec = snd_soc_component_get_drvdata(component);
	struct regmap *regmap = codec->regmap;

	codec->bin_file = devm_kzalloc(codec->dev, PAGE_SIZE, GFP_KERNEL);
	if (!codec->bin_file)
		return -ENOMEM;

	codec->bin_file[PAGE_SIZE - 1] = '\0';
	snprintf(codec->bin_file, PAGE_SIZE, "cs40l25a_a2h.bin");
	complete(&codec->core->hap_done);
	/* DSP1RX1_SRC to mute */
	regmap_write(regmap, CS40L2X_DSP1RX1_INPUT, 0);
	/* DSP1RX5_SRC to mute */
	regmap_write(regmap, CS40L2X_DSP1RX5_INPUT, 0);

	codec->codec_sysclk = CS40L2X_SCLK_DEFAULT; /* 1.536MHz */

	return 0;
}

static const struct snd_soc_component_driver soc_codec_dev_cs40l2x = {
	.probe = cs40l2x_codec_probe,
	.set_sysclk = cs40l2x_component_set_sysclk,

	.dapm_widgets		= cs40l2x_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(cs40l2x_dapm_widgets),
	.dapm_routes		= cs40l2x_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(cs40l2x_dapm_routes),
	.controls		= cs40l2x_controls,
	.num_controls		= ARRAY_SIZE(cs40l2x_controls),
};

static int cs40l2x_probe(struct platform_device *pdev)
{
	struct cs40l2x_private *cs40l2x = dev_get_drvdata(pdev->dev.parent);
	struct cs40l2x_codec *cs40l2x_codec;
	int ret;

	cs40l2x_codec = devm_kzalloc(&pdev->dev, sizeof(struct cs40l2x_codec),
					GFP_KERNEL);
	if (!cs40l2x_codec)
		return -ENOMEM;

	cs40l2x_codec->core = cs40l2x;
	cs40l2x_codec->regmap = cs40l2x->regmap;
	cs40l2x_codec->dev = &pdev->dev;

	platform_set_drvdata(pdev, cs40l2x_codec);

	ret = snd_soc_register_component(&pdev->dev, &soc_codec_dev_cs40l2x,
				      cs40l2x_dai, ARRAY_SIZE(cs40l2x_dai));
	if (ret < 0)
		dev_err(&pdev->dev, "Failed to register codec: %d\n", ret);

	return ret;
}

static int cs40l2x_remove(struct platform_device *pdev)
{
	snd_soc_unregister_component(&pdev->dev);
	return 0;
}

static struct platform_driver cs40l2x_codec_driver = {
	.driver = {
		.name = "cs40l2x-codec",
	},
	.probe = cs40l2x_probe,
	.remove = cs40l2x_remove,
};

module_platform_driver(cs40l2x_codec_driver);

MODULE_DESCRIPTION("ASoC CS40L2X driver");
MODULE_AUTHOR("Paul Handrigan <paul.handrigan@cirrus.com");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:cs40l2x-codec");
