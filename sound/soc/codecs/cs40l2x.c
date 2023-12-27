// SPDX-License-Identifier: GPL-2.0
//
// ALSA SoC driver for CS40L20/CS40L25/CS40L25A/CS40L25B
//
// Copyright (C) 2019-2020 Cirrus Logic, Inc.

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

#define CS40L2X_MCLK_FREQ		32768

enum cs40l2x_clk_src {
	CS40L2X_32KHZ_CLK,
	CS40L2X_SCLK
};

struct cs40l2x_codec {
	struct cs40l2x_private *core;
	struct device *dev;
	struct regmap *regmap;
	int tuning;

	unsigned int daifmt;
	int sysclk_rate;

	int tdm_slots;
	int tdm_width;
	int tdm_slot[2];
	bool mute_left;
	bool mute_right;
};

struct cs40l2x_pll_sysclk_config {
	int freq;
	int clk_cfg;
	int fs_mon;
} cs40l2x_pll_sysclk[] = {
	{ 32768,    0x00, 0x00000000 },
	{ 1536000,  0x1b, 0x00054034 },
	{ 3072000,  0x21, 0x0002C01C },
	{ 6144000,  0x28, 0x00018010 },
	{ 9600000,  0x30, 0x00024010 },
	{ 12288000, 0x33, 0x00024010 },
};

static struct cs40l2x_pll_sysclk_config *cs40l2x_get_clk_config(int freq)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cs40l2x_pll_sysclk); i++) {
		if (cs40l2x_pll_sysclk[i].freq == freq)
			return &cs40l2x_pll_sysclk[i];
	}

	return NULL;
}

static int cs40l2x_swap_ext_clk(struct cs40l2x_codec *priv,
				const enum cs40l2x_clk_src src)
{
	struct device *dev = priv->dev;
	struct regmap *regmap = priv->regmap;
	struct cs40l2x_pll_sysclk_config *pll_conf;

	if (src == CS40L2X_32KHZ_CLK)
		pll_conf = cs40l2x_get_clk_config(CS40L2X_MCLK_FREQ);
	else
		pll_conf = cs40l2x_get_clk_config(priv->sysclk_rate);

	if (!pll_conf) {
		dev_err(dev, "Invalid SYS Clock Frequency\n");
		return -EINVAL;
	}

	regmap_update_bits(regmap, CS40L2X_REFCLK_INPUT,
			   CS40L2X_PLL_OPEN_LOOP_MASK,
			   CS40L2X_PLL_OPEN_LOOP_MASK);

	regmap_update_bits(regmap, CS40L2X_REFCLK_INPUT,
			   CS40L2X_PLL_REFCLK_FREQ_MASK,
			   pll_conf->clk_cfg << CS40L2X_PLL_REFCLK_FREQ_SHIFT);

	if (src == CS40L2X_32KHZ_CLK)
		regmap_update_bits(regmap, CS40L2X_REFCLK_INPUT,
				   CS40L2X_PLL_REFCLK_SEL_MASK,
				   CS40L2X_PLLSRC_MCLK);
	else
		regmap_update_bits(regmap, CS40L2X_REFCLK_INPUT,
				   CS40L2X_PLL_REFCLK_SEL_MASK,
				   CS40L2X_PLLSRC_SCLK);

	regmap_update_bits(regmap, CS40L2X_REFCLK_INPUT,
			   CS40L2X_PLL_OPEN_LOOP_MASK, 0);
	regmap_update_bits(regmap, CS40L2X_REFCLK_INPUT,
			   CS40L2X_PLL_REFCLK_EN_MASK,
			   CS40L2X_PLL_REFCLK_EN_MASK);

	usleep_range(1000, 1500);

	return 0;
}

static int cs40l2x_clk_en(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *comp = snd_soc_dapm_to_component(w->dapm);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_private *core = priv->core;
	struct device *dev = core->dev;
	int ret;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		mutex_lock(&core->lock);
		core->a2h_enable = true;
		mutex_unlock(&core->lock);

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

		ret = cs40l2x_swap_ext_clk(priv, CS40L2X_SCLK);
		if (ret)
			return ret;
		break;
	case SND_SOC_DAPM_PRE_PMD:
		ret = cs40l2x_swap_ext_clk(priv, CS40L2X_32KHZ_CLK);
		if (ret)
			return ret;

		mutex_lock(&core->lock);
		core->a2h_enable = false;
		mutex_unlock(&core->lock);
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
	int ret;

	if (!core->dsp_reg)
		return -EINVAL;

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

	ret = regmap_write(regmap, reg_l, !codec->mute_left);
	if (ret)
		return ret;

	return regmap_write(regmap, reg_r, !codec->mute_right);

}

static int cs40l2x_a2h_ev(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *comp = snd_soc_dapm_to_component(w->dapm);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_private *core = priv->core;
	unsigned int reg;

	if (!core->dsp_reg)
		return 0;

	reg = core->dsp_reg(core, "A2HEN", CS40L2X_XM_UNPACKED_TYPE,
			    CS40L2X_ALGO_ID_A2H);
	if (!reg) {
		dev_err(priv->dev, "Cannot find the A2HENABLED register\n");
		return -EINVAL;
	}

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		return regmap_write(priv->regmap, reg, CS40L2X_A2H_ENABLE);
	case SND_SOC_DAPM_PRE_PMD:
		return regmap_write(priv->regmap, reg, CS40L2X_A2H_DISABLE);
	default:
		dev_err(priv->dev, "Invalid A2H event: %d\n", event);
		return -EINVAL;
	}
}

static int cs40l2x_pcm_ev(struct snd_soc_dapm_widget *w,
			  struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *comp = snd_soc_dapm_to_component(w->dapm);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_private *core = priv->core;
	struct regmap *regmap = priv->regmap;
	int ret;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
					 CS40L2X_BST_CTL_SEL_MASK,
					 CS40L2X_BST_CTL_SEL_CLASSH);
		if (ret)
			return ret;

		ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES2,
					 CS40L2X_CLASSH_EN_MASK,
					 CS40L2X_CLASSH_EN_MASK);
		if (ret)
			return ret;

		/* Enable I2S in the DSP */
		ret = regmap_update_bits(regmap, CS40L2X_ASP_ENABLES1,
					 CS40L2X_ASP_RX_ENABLE_MASK,
					 CS40L2X_ASP_RX_ENABLE_MASK);
		if (ret)
			return ret;

		return cs40l2x_ack_write(core, CS40L2X_MBOX_USER_CONTROL,
					 CS40L2X_A2H_I2S_START,
					 CS40L2X_A2H_DISABLE);
	case SND_SOC_DAPM_PRE_PMD:
		ret = regmap_update_bits(regmap, CS40L2X_ASP_ENABLES1,
					 CS40L2X_ASP_RX_ENABLE_MASK, 0);
		if (ret)
			return ret;

		if (!core->cond_class_h_en) {
			ret = regmap_update_bits(regmap, CS40L2X_VBST_CTL_2,
					 CS40L2X_BST_CTL_SEL_MASK, 0);
			if (ret)
				return ret;

			ret = regmap_update_bits(regmap, CS40L2X_BLOCK_ENABLES2,
					 CS40L2X_CLASSH_EN_MASK, 0);
			if (ret)
				return ret;
		}

		return cs40l2x_ack_write(core, CS40L2X_MBOX_USER_CONTROL,
					 CS40L2X_A2H_I2S_END,
					 CS40L2X_A2H_DISABLE);
	default:
		dev_err(priv->dev, "Invalid PCM event: %d\n", event);
		return -EINVAL;
	}
}

static int cs40l2x_vol_get(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = priv->regmap;
	struct device *dev = priv->dev;
	struct cs40l2x_private *core = priv->core;
	unsigned int val = 0, reg;
	int ret;

	if (!core->dsp_reg || core->fw_id_remap != CS40L2X_FW_ID_A2H)
		return 0;

	reg = core->dsp_reg(core, "VOLUMELEVEL", CS40L2X_XM_UNPACKED_TYPE,
			    CS40L2X_ALGO_ID_A2H);
	if (!reg) {
		dev_err(dev, "Cannot find the VOLUMELEVEL register\n");
		return -EINVAL;
	}

	pm_runtime_get_sync(priv->dev);

	ret = regmap_read(regmap, reg, &val);
	if (ret)
		goto vol_get_err;

	if (val == CS40L2X_VOL_LVL_MAX)
		val = CS40L2X_VOL_LVL_MAX_STEPS;
	else
		val /= (CS40L2X_VOL_LVL_MAX / CS40L2X_VOL_LVL_MAX_STEPS);

vol_get_err:
	pm_runtime_mark_last_busy(priv->dev);
	pm_runtime_put_autosuspend(priv->dev);

	ucontrol->value.integer.value[0] = val;

	return ret;
}

static int cs40l2x_vol_put(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = priv->regmap;
	struct device *dev = priv->dev;
	struct cs40l2x_private *core = priv->core;
	unsigned int val, reg;
	int ret;

	if (!core->dsp_reg || core->fw_id_remap != CS40L2X_FW_ID_A2H)
		return 0;

	reg = core->dsp_reg(core, "VOLUMELEVEL", CS40L2X_XM_UNPACKED_TYPE,
			    CS40L2X_ALGO_ID_A2H);
	if (!reg) {
		dev_err(dev, "Cannot find the VOLUMELEVEL register\n");
		return -EINVAL;
	}

	val = ucontrol->value.integer.value[0];

	if (val == CS40L2X_VOL_LVL_MAX_STEPS)
		val = CS40L2X_VOL_LVL_MAX;
	else
		val *= (CS40L2X_VOL_LVL_MAX / CS40L2X_VOL_LVL_MAX_STEPS);

	pm_runtime_get_sync(priv->dev);

	ret = regmap_write(regmap, reg, val);

	pm_runtime_mark_last_busy(priv->dev);
	pm_runtime_put_autosuspend(priv->dev);

	return ret;
}

static int cs40l2x_tuning_get(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);

	ucontrol->value.enumerated.item[0] = priv->tuning;

	return 0;
}

static int cs40l2x_tuning_swap(struct cs40l2x_codec *priv, int tuning)
{
	char bin_file[32] = "cs40l25a_a2h.bin";
	const struct firmware *fw;
	int ret;

	if (tuning > 0)
		snprintf(bin_file, sizeof(bin_file), "cs40l25a_a2h%d.bin",
			tuning);

	ret = request_firmware(&fw, bin_file, priv->dev);
	if (ret) {
		dev_err(priv->dev, "Failed to request %s\n", bin_file);
		return ret;
	}

	ret = cs40l2x_coeff_file_parse(priv->core, fw);
	if (ret)
		return ret;

	priv->tuning = tuning;

	ret = cs40l2x_reset_mute(priv);
	if (ret)
		return ret;

	return cs40l2x_ack_write(priv->core, CS40L2X_MBOX_USER_CONTROL,
				 CS40L2X_USER_CTRL_REINIT_A2H,
				 CS40L2X_USER_CTRL_SUCCESS);
}

static int cs40l2x_tuning_put(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct snd_soc_dapm_context *dapm = snd_soc_component_get_dapm(comp);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_private *core = priv->core;
	int tuning = ucontrol->value.enumerated.item[0];
	int ret = 0;

	if (!core->a2h_reinit_min_fw)
		return -EOPNOTSUPP;

	snd_soc_dapm_mutex_lock(dapm);

	if (tuning == priv->tuning)
		goto out_mutex;

	pm_runtime_get_sync(priv->dev);

	ret = cs40l2x_tuning_swap(priv, tuning);

	pm_runtime_mark_last_busy(priv->dev);
	pm_runtime_put_autosuspend(priv->dev);

out_mutex:
	snd_soc_dapm_mutex_unlock(dapm);

	return ret;
}

static int cs40l2x_delay_get(struct snd_kcontrol *kcontrol,
			     struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = priv->regmap;
	struct device *dev = priv->dev;
	struct cs40l2x_private *core = priv->core;
	unsigned int val = 0, reg;
	int ret;

	if (!core->dsp_reg || core->fw_id_remap != CS40L2X_FW_ID_A2H)
		return 0;

	reg = core->dsp_reg(core, "LRADELAYSAMPS", CS40L2X_XM_UNPACKED_TYPE,
			    CS40L2X_ALGO_ID_A2H);
	if (!reg) {
		dev_err(dev, "Cannot find the LRADELAYSAMPS register\n");
		return -EINVAL;
	}

	pm_runtime_get_sync(priv->dev);

	ret = regmap_read(regmap, reg, &val);
	if (!ret)
		ucontrol->value.integer.value[0] = val;

	pm_runtime_mark_last_busy(priv->dev);
	pm_runtime_put_autosuspend(priv->dev);

	return ret;
}

static int cs40l2x_delay_put(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *comp = snd_soc_kcontrol_component(kcontrol);
	struct snd_soc_dapm_context *dapm = snd_soc_component_get_dapm(comp);
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct regmap *regmap = priv->regmap;
	struct device *dev = priv->dev;
	struct cs40l2x_private *core = priv->core;
	unsigned int val, reg;
	int ret;

	if (!core->a2h_reinit_min_fw)
		return -EOPNOTSUPP;

	if (!core->dsp_reg || core->fw_id_remap != CS40L2X_FW_ID_A2H)
		return 0;

	reg = core->dsp_reg(core, "LRADELAYSAMPS", CS40L2X_XM_UNPACKED_TYPE,
			    CS40L2X_ALGO_ID_A2H);

	if (!reg) {
		dev_err(dev, "Cannot find the LRADELAYSAMPS register\n");
		return -EINVAL;
	}

	val = ucontrol->value.integer.value[0];

	snd_soc_dapm_mutex_lock(dapm);
	pm_runtime_get_sync(priv->dev);

	ret = regmap_write(regmap, reg, val);
	if (!ret)
		ret = cs40l2x_ack_write(core, CS40L2X_MBOX_USER_CONTROL,
					CS40L2X_USER_CTRL_REINIT_A2H,
					CS40L2X_USER_CTRL_SUCCESS);

	pm_runtime_mark_last_busy(priv->dev);
	pm_runtime_put_autosuspend(priv->dev);
	snd_soc_dapm_mutex_unlock(dapm);

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
	struct cs40l2x_private *core = cs40l2x->core;

	cs40l2x->mute_left = ucontrol->value.integer.value[0];
	cs40l2x->mute_right = ucontrol->value.integer.value[1];

	pm_runtime_get_sync(core->dev);

	cs40l2x_reset_mute(cs40l2x);

	pm_runtime_mark_last_busy(core->dev);
	pm_runtime_put_autosuspend(core->dev);

	return 0;
}

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

static const char * const cs40l2x_out_mux_texts[] = { "Off", "PCM", "A2H" };
static SOC_ENUM_SINGLE_VIRT_DECL(cs40l2x_out_mux_enum, cs40l2x_out_mux_texts);
static const struct snd_kcontrol_new cs40l2x_out_mux =
	SOC_DAPM_ENUM("Haptics Source", cs40l2x_out_mux_enum);

static const struct snd_soc_dapm_widget cs40l2x_dapm_widgets[] = {
SND_SOC_DAPM_SUPPLY_S("ASP PLL", 0, SND_SOC_NOPM, 0, 0, cs40l2x_clk_en,
		      SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
SND_SOC_DAPM_AIF_IN("ASPRX1", NULL, 0, SND_SOC_NOPM, 0, 0),
SND_SOC_DAPM_AIF_IN("ASPRX2", NULL, 0, SND_SOC_NOPM, 0, 0),

SND_SOC_DAPM_PGA_E("PCM", SND_SOC_NOPM, 0, 0, NULL, 0, cs40l2x_pcm_ev,
		   SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),
SND_SOC_DAPM_MIXER_E("A2H", SND_SOC_NOPM, 0, 0, NULL, 0, cs40l2x_a2h_ev,
		     SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD),

SND_SOC_DAPM_MUX("Haptics Source", SND_SOC_NOPM, 0, 0, &cs40l2x_out_mux),
SND_SOC_DAPM_OUTPUT("OUT"),
};

static const struct snd_soc_dapm_route cs40l2x_dapm_routes[] = {
	{ "ASP Playback", NULL, "ASP PLL" },
	{ "ASPRX1", NULL, "ASP Playback" },
	{ "ASPRX2", NULL, "ASP Playback" },

	{ "PCM", NULL, "ASPRX1" },
	{ "PCM", NULL, "ASPRX2" },
	{ "A2H", NULL, "PCM" },

	{ "Haptics Source", "PCM", "PCM" },
	{ "Haptics Source", "A2H", "A2H" },
	{ "OUT", NULL, "Haptics Source" },
};

static int cs40l2x_component_set_sysclk(struct snd_soc_component *comp,
					int clk_id, int source,
					unsigned int freq, int dir)
{
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);

	if (!cs40l2x_get_clk_config(freq)) {
		dev_err(priv->dev, "Invalid clock frequency: %d\n", freq);
		return -EINVAL;
	}

	switch (clk_id) {
	case 0:
		break;
	default:
		dev_err(priv->dev, "Invalid input clock\n");
		return -EINVAL;
	}

	priv->sysclk_rate = freq;

	return 0;
}

static int cs40l2x_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(dai->component);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		dev_err(priv->dev, "This device can be slave only\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		priv->daifmt = CS40L2X_ASP_FMT_I2S;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		priv->daifmt = CS40L2X_ASP_FMT_TDM1;
		break;
	default:
		dev_err(priv->dev, "Unsupported DAI format\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_IF:
		priv->daifmt |= CS40L2X_ASP_FSYNC_INV_MASK;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		priv->daifmt |= CS40L2X_ASP_BCLK_INV_MASK;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		priv->daifmt |= CS40L2X_ASP_FSYNC_INV_MASK |
				CS40L2X_ASP_BCLK_INV_MASK;
		break;
	case SND_SOC_DAIFMT_NB_NF:
		break;
	default:
		dev_err(priv->dev, "Invalid DAI clock format\n");
		return -EINVAL;
	}

	return 0;
}

static int cs40l2x_set_tdm_slot(struct snd_soc_dai *dai, unsigned int tx_mask,
				unsigned int rx_mask, int slots, int slot_width)
{
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(dai->component);

	priv->tdm_slots = slots;
	priv->tdm_width = slot_width;

	/*
	 * Reset to slots 0,1 if TDM is being disabled, and catch the
	 * case where both RX1 and RX2 would be set to slot 0 since
	 * that causes the hardware to flag an error.
	 */
	if (!slots || rx_mask == 0x1)
		rx_mask = 0x3;

	priv->tdm_slot[0] = ffs(rx_mask) - 1;
	rx_mask &= ~(1 << priv->tdm_slot[0]);
	priv->tdm_slot[1] = ffs(rx_mask) - 1;

	return 0;
}

static int cs40l2x_pcm_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params,
				 struct snd_soc_dai *dai)
{
	struct snd_soc_component *comp = dai->component;
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);
	struct cs40l2x_pll_sysclk_config *pll_conf;
	unsigned int mask = CS40L2X_ASP_WIDTH_RX_MASK |
			    CS40L2X_ASP_FMT_MASK |
			    CS40L2X_ASP_FSYNC_INV_MASK |
			    CS40L2X_ASP_BCLK_INV_MASK;
	unsigned int bclk_rate, asp_wl, asp_sl;
	int ret;

	asp_wl = params_width(params);

	if (priv->tdm_slots) {
		bclk_rate = priv->tdm_slots * priv->tdm_width * params_rate(params);
		asp_sl = priv->tdm_width;
	} else {
		bclk_rate = snd_soc_params_to_bclk(params);
		asp_sl = asp_wl;
	}

	dev_dbg(priv->dev,
		"ASP setup for %d bits in %d bit slots, using slots %d, %d\n",
		asp_wl, asp_sl, priv->tdm_slot[0], priv->tdm_slot[1]);

	if (priv->sysclk_rate != bclk_rate)
		dev_warn(priv->dev, "Expect BCLK of %dHz but got %dHz\n",
			 priv->sysclk_rate, bclk_rate);

	pll_conf = cs40l2x_get_clk_config(bclk_rate);
	if (!pll_conf) {
		dev_err(priv->dev, "Invalid BCLK frequency: %d\n", bclk_rate);
		return -EINVAL;
	}

	ret = regmap_write(priv->regmap, CS40L2X_ASP_CONTROL1,
			   pll_conf->clk_cfg);
	if (ret)
		return ret;

	regmap_update_bits(priv->regmap, CS40L2X_ASP_CONTROL2, mask,
			   (asp_sl << CS40L2X_ASP_WIDTH_RX_SHIFT) | priv->daifmt);
	regmap_update_bits(priv->regmap, CS40L2X_ASP_DATA_CONTROL5,
			   CS40L2X_ASP_RX_WL_MASK, asp_wl);
	ret = regmap_update_bits(priv->regmap, CS40L2X_ASP_FRAME_CONTROL5,
				 CS40L2X_ASP_RX1_SLOT_MASK | CS40L2X_ASP_RX2_SLOT_MASK,
				 priv->tdm_slot[0] << CS40L2X_ASP_RX1_SLOT_SHIFT |
				 priv->tdm_slot[1] << CS40L2X_ASP_RX2_SLOT_SHIFT);
	if (ret) {
		dev_err(priv->dev, "Failed to write ASP slots: %d\n", ret);
		return ret;
	}

	ret = regmap_write(priv->regmap, CS40L2X_FS_MON_0, pll_conf->fs_mon);
	if (ret) {
		dev_err(priv->dev, "Failed to write ASP coefficients\n");
		return ret;
	}

	return 0;
}

#define CS40L2X_RATES SNDRV_PCM_RATE_48000
#define CS40L2X_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)

static const struct snd_soc_dai_ops cs40l2x_dai_ops = {
	.set_fmt = cs40l2x_set_dai_fmt,
	.set_tdm_slot = cs40l2x_set_tdm_slot,
	.hw_params = cs40l2x_pcm_hw_params,
};

static struct snd_soc_dai_driver cs40l2x_dai[] = {
	{
		.name = "cs40l2x-pcm",
		.id = 0,
		.playback = {
			.stream_name = "ASP Playback",
			.channels_min = 1,
			.channels_max = 2,
			.rates = CS40L2X_RATES,
			.formats = CS40L2X_FORMATS,
		},
		.ops = &cs40l2x_dai_ops,
		.symmetric_rate = 1,
	},
};

static int cs40l2x_codec_probe(struct snd_soc_component *comp)
{
	struct cs40l2x_codec *priv = snd_soc_component_get_drvdata(comp);

	complete(&priv->core->hap_done);

	priv->sysclk_rate = 1536000;

	priv->tdm_slot[0] = priv->core->pdata.asp_slot_num;
	priv->tdm_slot[1] = priv->tdm_slot[0] + 1;

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
	struct cs40l2x_private *core = dev_get_drvdata(pdev->dev.parent);
	struct cs40l2x_codec *priv;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->core = core;
	priv->regmap = core->regmap;
	priv->dev = &pdev->dev;

	platform_set_drvdata(pdev, priv);

	pm_runtime_enable(&pdev->dev);

	ret = snd_soc_register_component(&pdev->dev, &soc_codec_dev_cs40l2x,
					 cs40l2x_dai, ARRAY_SIZE(cs40l2x_dai));
	if (ret < 0)
		dev_err(&pdev->dev, "Failed to register codec: %d\n", ret);

	return ret;
}

static int cs40l2x_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);

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
