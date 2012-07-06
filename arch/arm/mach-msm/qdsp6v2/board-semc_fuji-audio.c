/* Copyright (c) 2010-2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2012 Sony Ericsson Mobile Communications AB
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/mfd/pmic8058.h>
#include <linux/pmic8058-othc.h>
#include <linux/mfd/pmic8901.h>
#include <linux/mfd/msm-adie-codec.h>
#include <linux/regulator/pmic8058-regulator.h>
#include <linux/regulator/pmic8901-regulator.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/machine.h>

#include <mach/qdsp6v2/audio_dev_ctl.h>
#ifdef CONFIG_PMIC8058_MIC_BIAS
#include <mach/pm8058-mic_bias.h>
#endif
#include <mach/mpp.h>
#include <sound/apr_audio.h>
#include <asm/mach-types.h>
#include <asm/uaccess.h>

#include "snddev_icodec.h"
#include "snddev_ecodec.h"
#include "timpani_profile_fuji.h"
#include "snddev_hdmi.h"
#include "snddev_mi2s.h"
#include "snddev_virtual.h"

#ifdef CONFIG_DEBUG_FS
static struct dentry *debugfs_hsed_config;
static void snddev_hsed_config_modify_setting(int type);
static void snddev_hsed_config_restore_setting(void);
#endif

#define SNDDEV_GPIO_SPK_AMP_L 20
#define SNDDEV_GPIO_SPK_AMP_R 16

#define DSP_RAM_BASE_8x60 0x46700000
#define DSP_RAM_SIZE_8x60 0x2000000
static int dspcrashd_pdata_8x60 = 0xDEADDEAD;

static struct resource resources_dspcrashd_8x60[] = {
	{
		.name   = "msm_dspcrashd",
		.start  = DSP_RAM_BASE_8x60,
		.end    = DSP_RAM_BASE_8x60 + DSP_RAM_SIZE_8x60,
		.flags  = IORESOURCE_DMA,
	},
};

struct platform_device msm_device_dspcrashd_8x60 = {
	.name           = "msm_dspcrashd",
	.num_resources  = ARRAY_SIZE(resources_dspcrashd_8x60),
	.resource       = resources_dspcrashd_8x60,
	.dev = { .platform_data = &dspcrashd_pdata_8x60 },
};

static struct resource msm_cdcclk_ctl_resources[] = {
	{
		.name   = "msm_snddev_tx_mclk",
		.start  = 108,
		.end    = 108,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "msm_snddev_rx_mclk",
		.start  = 109,
		.end    = 109,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device msm_cdcclk_ctl_device = {
	.name   = "msm_cdcclk_ctl",
	.num_resources  = ARRAY_SIZE(msm_cdcclk_ctl_resources),
	.resource       = msm_cdcclk_ctl_resources,
};

static struct resource msm_aux_pcm_resources[] = {

	{
		.name   = "aux_pcm_dout",
		.start  = 111,
		.end    = 111,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_din",
		.start  = 112,
		.end    = 112,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_syncout",
		.start  = 113,
		.end    = 113,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "aux_pcm_clkin_a",
		.start  = 114,
		.end    = 114,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device msm_aux_pcm_device = {
	.name   = "msm_aux_pcm",
	.num_resources  = ARRAY_SIZE(msm_aux_pcm_resources),
	.resource       = msm_aux_pcm_resources,
};

static struct resource msm_mi2s_gpio_resources[] = {

	{
		.name   = "mi2s_ws",
		.start  = 101,
		.end    = 101,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "mi2s_sclk",
		.start  = 102,
		.end    = 102,
		.flags  = IORESOURCE_IO,
	},
	{
		.name   = "fm_mi2s_sd",
		.start  = 107,
		.end    = 107,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device msm_mi2s_device = {
	.name		= "msm_mi2s",
	.num_resources	= ARRAY_SIZE(msm_mi2s_gpio_resources),
	.resource	= msm_mi2s_gpio_resources,
};

static int msm_snddev_stereo_poweramp_on(void)
{
	int rc;

	pr_debug("%s: enable stereo spkr amp\n", __func__);
	rc = gpio_request(SNDDEV_GPIO_SPK_AMP_L, "SPK_AMP_L");
	if (rc) {
		pr_err("%s: spkr pamp gpio %d request"
			"failed\n", __func__, SNDDEV_GPIO_SPK_AMP_L);
		return rc;
	}
	rc = gpio_request(SNDDEV_GPIO_SPK_AMP_R, "SPK_AMP_R");
	if (rc) {
		pr_err("%s: spkr pamp gpio %d request"
			"failed\n", __func__, SNDDEV_GPIO_SPK_AMP_R);
		return rc;
	}
	gpio_set_value(SNDDEV_GPIO_SPK_AMP_L, 1);
	gpio_set_value(SNDDEV_GPIO_SPK_AMP_R, 1);

	return rc;
}

static void msm_snddev_stereo_poweramp_off(void)
{

	pr_debug("%s: disable stereo spkr amp\n", __func__);
	gpio_set_value(SNDDEV_GPIO_SPK_AMP_L, 0);
	gpio_set_value(SNDDEV_GPIO_SPK_AMP_R, 0);
	gpio_free(SNDDEV_GPIO_SPK_AMP_L);
	gpio_free(SNDDEV_GPIO_SPK_AMP_R);
	msleep(30);
}

static int msm_snddev_mono_poweramp_on(void)
{
	int rc;

	pr_debug("%s: enable mono spkr amp\n", __func__);
	rc = gpio_request(SNDDEV_GPIO_SPK_AMP_R, "SPK_AMP_R");
	if (rc) {
		pr_err("%s: spkr pamp gpio %d request"
			"failed\n", __func__, SNDDEV_GPIO_SPK_AMP_R);
		return rc;
	}
	gpio_set_value(SNDDEV_GPIO_SPK_AMP_R, 1);

	return rc;
}

static void msm_snddev_mono_poweramp_off(void)
{

	pr_debug("%s: disable mono spkr amp\n", __func__);
	gpio_set_value(SNDDEV_GPIO_SPK_AMP_R, 0);
	gpio_free(SNDDEV_GPIO_SPK_AMP_R);
	msleep(30);
}

/* Regulator 8058_l10 supplies regulator 8058_ncp. */
static struct regulator *snddev_reg_ncp;
static struct regulator *snddev_reg_l10;

static atomic_t preg_ref_cnt;

static int msm_snddev_voltage_on(void)
{
	int rc;
	pr_debug("%s\n", __func__);

	if (atomic_inc_return(&preg_ref_cnt) > 1)
		return 0;

	snddev_reg_l10 = regulator_get(NULL, "8058_l10");
	if (IS_ERR(snddev_reg_l10)) {
		pr_err("%s: regulator_get(%s) failed (%ld)\n", __func__,
			"l10", PTR_ERR(snddev_reg_l10));
		return -EBUSY;
	}

	rc = regulator_set_voltage(snddev_reg_l10, 2850000, 2850000);
	if (rc < 0)
		pr_err("%s: regulator_set_voltage(l10) failed (%d)\n",
			__func__, rc);

	rc = regulator_enable(snddev_reg_l10);
	if (rc < 0)
		pr_err("%s: regulator_enable(l10) failed (%d)\n", __func__, rc);

	snddev_reg_ncp = regulator_get(NULL, "8058_ncp");
	if (IS_ERR(snddev_reg_ncp)) {
		pr_err("%s: regulator_get(%s) failed (%ld)\n", __func__,
			"ncp", PTR_ERR(snddev_reg_ncp));
		return -EBUSY;
	}

	rc = regulator_set_voltage(snddev_reg_ncp, 1800000, 1800000);
	if (rc < 0) {
		pr_err("%s: regulator_set_voltage(ncp) failed (%d)\n",
			__func__, rc);
		goto regulator_fail;
	}

	rc = regulator_enable(snddev_reg_ncp);
	if (rc < 0) {
		pr_err("%s: regulator_enable(ncp) failed (%d)\n", __func__, rc);
		goto regulator_fail;
	}

	return rc;

regulator_fail:
	regulator_put(snddev_reg_ncp);
	snddev_reg_ncp = NULL;
	return rc;
}

static void msm_snddev_voltage_off(void)
{
	int rc;
	pr_debug("%s\n", __func__);

	if (!snddev_reg_ncp)
		goto done;

	if (atomic_dec_return(&preg_ref_cnt) == 0) {
		rc = regulator_disable(snddev_reg_ncp);
		if (rc < 0)
			pr_err("%s: regulator_disable(ncp) failed (%d)\n",
				__func__, rc);
		regulator_put(snddev_reg_ncp);

		snddev_reg_ncp = NULL;
	} else {
		pr_debug("%s: the voltage still need on!\n", __func__);
		return;
	}

done:
	if (!snddev_reg_l10)
		return;

	rc = regulator_disable(snddev_reg_l10);
	if (rc < 0)
		pr_err("%s: regulator_disable(l10) failed (%d)\n",
			__func__, rc);

	regulator_put(snddev_reg_l10);

	snddev_reg_l10 = NULL;
}

static int msm_snddev_enable_amic_power(void)
{
	int ret = 0;
#ifdef CONFIG_PMIC8058_MIC_BIAS
	ret = pm8058_mic_bias_enable(true);
	if (ret < 0)
		pr_err("%s: Activating pmic8058 mic bias failed\n", __func__);
#endif
	return ret;
}

static void msm_snddev_disable_amic_power(void)
{
#ifdef CONFIG_PMIC8058_MIC_BIAS
	int ret = 0;
	ret = pm8058_mic_bias_enable(false);
	if (ret < 0)
		pr_err("%s: Deactivating pmic8058 mic bias failed\n", __func__);
#endif
}

static struct adie_codec_action_unit iearpiece_48KHz_osr256_actions[] =
	EAR_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry iearpiece_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_settings),
};

static struct snddev_icodec_data snddev_iearpiece_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_rx",
	.copp_id = 0,
	.profile = &iearpiece_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
};

static struct platform_device msm_iearpiece_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_iearpiece_data },
};

static struct adie_codec_action_unit imic_48KHz_osr256_actions[] =
	AMIC_PRI_MONO_OSR_256;

static struct adie_codec_hwsetting_entry imic_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile imic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_settings,
	.setting_sz = ARRAY_SIZE(imic_settings),
};

static struct snddev_icodec_data snddev_imic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_tx",
	.copp_id = 1,
	.profile = &imic_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_imic_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_imic_data },
};

static struct adie_codec_action_unit headset_ab_cpls_48KHz_osr256_actions[] =
	HEADSET_AB_CPLS_48000_OSR_256;

static struct adie_codec_hwsetting_entry headset_ab_cpls_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = headset_ab_cpls_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(headset_ab_cpls_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile headset_ab_cpls_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = headset_ab_cpls_settings,
	.setting_sz = ARRAY_SIZE(headset_ab_cpls_settings),
};

static struct snddev_icodec_data snddev_ihs_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_rx",
	.copp_id = 0,
	.profile = &headset_ab_cpls_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.voltage_on = msm_snddev_voltage_on,
	.voltage_off = msm_snddev_voltage_off,
};

static struct platform_device msm_headset_stereo_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_ihs_stereo_rx_data },
};

static struct adie_codec_action_unit ispkr_stereo_48KHz_osr256_actions[] =
	SPEAKER_PRI_STEREO_48000_OSR_256;

static struct adie_codec_hwsetting_entry ispkr_stereo_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispkr_stereo_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispkr_stereo_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispkr_stereo_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ispkr_stereo_settings,
	.setting_sz = ARRAY_SIZE(ispkr_stereo_settings),
};

static struct snddev_icodec_data snddev_ispkr_stereo_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_stereo_rx",
	.copp_id = 0,
	.profile = &ispkr_stereo_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_stereo_poweramp_on,
	.pamp_off = msm_snddev_stereo_poweramp_off,
};

static struct platform_device msm_ispkr_stereo_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_ispkr_stereo_data },
};

static struct adie_codec_action_unit ispkr_mono_48KHz_osr256_actions[] =
	SPEAKER_PRI_MONO_48000_OSR_256;

static struct adie_codec_hwsetting_entry ispkr_mono_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispkr_mono_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispkr_mono_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispkr_mono_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ispkr_mono_settings,
	.setting_sz = ARRAY_SIZE(ispkr_mono_settings),
};

static struct snddev_icodec_data snddev_ispkr_mono_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_rx",
	.copp_id = 0,
	.profile = &ispkr_mono_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_mono_poweramp_on,
	.pamp_off = msm_snddev_mono_poweramp_off,
};

static struct platform_device msm_ispkr_mono_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_ispkr_mono_data },
};

static struct adie_codec_action_unit ispeaker_tx_48KHz_osr256_actions[] =
	SPKR_AMIC_PRI_MONO_OSR_256;

static struct adie_codec_hwsetting_entry ispeaker_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ispeaker_tx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ispeaker_tx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ispeaker_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ispeaker_tx_settings,
	.setting_sz = ARRAY_SIZE(ispeaker_tx_settings),
};

static struct snddev_icodec_data snddev_ispkr_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_mono_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &ispeaker_tx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_ispkr_mic_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_ispkr_mic_data },
};

static struct adie_codec_action_unit secondary_imic_48KHz_osr256_actions[] =
	AMIC_SEC_MONO_OSR_256;

static struct adie_codec_hwsetting_entry secondary_imic_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = secondary_imic_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(secondary_imic_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile secondary_imic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = secondary_imic_settings,
	.setting_sz = ARRAY_SIZE(secondary_imic_settings),
};

static struct snddev_icodec_data snddev_secondary_imic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "secondary_mic_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &secondary_imic_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_secondary_imic_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_secondary_imic_data },
};

static struct snddev_hdmi_data snddev_hdmi_stereo_rx_data = {
	.capability = SNDDEV_CAP_RX ,
	.name = "hdmi_stereo_rx",
	.copp_id = HDMI_RX,
	.channel_mode = 0,
	.default_sample_rate = 48000,
};

static struct platform_device msm_snddev_hdmi_stereo_rx_device = {
	.name = "snddev_hdmi",
	.dev = { .platform_data = &snddev_hdmi_stereo_rx_data },
};

static struct snddev_mi2s_data snddev_mi2s_fm_tx_data = {
	.capability = SNDDEV_CAP_TX ,
	.name = "fmradio_stereo_tx",
	.copp_id = MI2S_TX,
	.channel_mode = 2, /* stereo */
	.sd_lines = MI2S_SD3, /* sd3 */
	.sample_rate = 48000,
};

static struct platform_device msm_mi2s_fm_tx_device = {
	.name = "snddev_mi2s",
	.dev = { .platform_data = &snddev_mi2s_fm_tx_data },
};

static struct snddev_mi2s_data snddev_mi2s_fm_rx_data = {
	.capability = SNDDEV_CAP_RX ,
	.name = "fmradio_stereo_rx",
	.copp_id = MI2S_RX,
	.channel_mode = 2, /* stereo */
	.sd_lines = MI2S_SD3, /* sd3 */
	.sample_rate = 48000,
};

static struct platform_device msm_mi2s_fm_rx_device = {
	.name = "snddev_mi2s",
	.id = 1,
	.dev = { .platform_data = &snddev_mi2s_fm_rx_data },
};

static struct adie_codec_action_unit iheadset_mic_tx_osr256_actions[] =
	HEADSET_AMIC2_TX_MONO_PRI_OSR_256;

static struct adie_codec_hwsetting_entry iheadset_mic_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iheadset_mic_tx_osr256_actions,
		.action_sz = ARRAY_SIZE(iheadset_mic_tx_osr256_actions),
	}
};

static struct adie_codec_dev_profile iheadset_mic_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = iheadset_mic_tx_settings,
	.setting_sz = ARRAY_SIZE(iheadset_mic_tx_settings),
};

static struct snddev_icodec_data snddev_headset_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "headset_mono_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &iheadset_mic_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
};

static struct platform_device msm_headset_mic_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_headset_mic_data },
};

static struct adie_codec_action_unit
	ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions[] =
	SPEAKER_HPH_AB_CPL_PRI_STEREO_48000_OSR_256;

static struct adie_codec_hwsetting_entry
	ihs_stereo_speaker_stereo_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(ihs_stereo_speaker_stereo_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_stereo_speaker_stereo_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_stereo_speaker_stereo_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_stereo_speaker_stereo_rx_settings),
};

static struct snddev_icodec_data snddev_ihs_stereo_speaker_stereo_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_speaker_stereo_rx",
	.copp_id = 0,
	.profile = &ihs_stereo_speaker_stereo_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_stereo_poweramp_on,
	.pamp_off = msm_snddev_stereo_poweramp_off,
	.voltage_on = msm_snddev_voltage_on,
	.voltage_off = msm_snddev_voltage_off,
};

static struct platform_device msm_ihs_stereo_speaker_stereo_rx_device = {
	.name = "snddev_icodec",
	.id = 22,
	.dev = { .platform_data = &snddev_ihs_stereo_speaker_stereo_rx_data },
};

static struct adie_codec_action_unit
	ihs_stereo_speaker_mono_rx_48KHz_osr256_actions[] =
	SPEAKER_HPH_AB_CPL_PRI_MONO_48000_OSR_256;

static struct adie_codec_hwsetting_entry
	ihs_stereo_speaker_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihs_stereo_speaker_mono_rx_48KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(ihs_stereo_speaker_mono_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihs_stereo_speaker_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihs_stereo_speaker_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(ihs_stereo_speaker_mono_rx_settings),
};

static struct snddev_icodec_data snddev_ihs_stereo_speaker_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "headset_stereo_speaker_mono_rx",
	.copp_id = 0,
	.profile = &ihs_stereo_speaker_mono_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_mono_poweramp_on,
	.pamp_off = msm_snddev_mono_poweramp_off,
	.voltage_on = msm_snddev_voltage_on,
	.voltage_off = msm_snddev_voltage_off,
};

static struct platform_device msm_ihs_stereo_speaker_mono_rx_device = {
	.name = "snddev_icodec",
	.id = 22,
	.dev = { .platform_data = &snddev_ihs_stereo_speaker_mono_rx_data },
};

static struct adie_codec_action_unit
	ispkr_dual_mic_endfire_tx_real_stereo_48KHz_osr256_actions[] =
	SPKR_AMIC_STEREO_OSR_256;

static struct adie_codec_hwsetting_entry
	ispkr_dual_mic_endfire_tx_real_stereo_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions =
		ispkr_dual_mic_endfire_tx_real_stereo_48KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(ispkr_dual_mic_endfire_tx_real_stereo_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile
	ispkr_dual_mic_endfire_tx_real_stereo_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = ispkr_dual_mic_endfire_tx_real_stereo_settings,
	.setting_sz =
	ARRAY_SIZE(ispkr_dual_mic_endfire_tx_real_stereo_settings),
};

static struct snddev_icodec_data
	snddev_ispkr_dual_mic_endfire_tx_real_stereo_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_dual_mic_endfire_tx_real_stereo",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &ispkr_dual_mic_endfire_tx_real_stereo_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device
	msm_ispkr_dual_mic_endfire_tx_real_stereo_device = {
	.name = "snddev_icodec",
	.dev = {
		.platform_data =
		&snddev_ispkr_dual_mic_endfire_tx_real_stereo_data
	},
};

static struct adie_codec_action_unit
	dual_mic_endfire_8KHz_osr256_actions[] =
	AMIC_DUAL_OSR_256;

static struct adie_codec_hwsetting_entry dual_mic_endfire_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = dual_mic_endfire_8KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(dual_mic_endfire_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile dual_mic_endfire_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = dual_mic_endfire_settings,
	.setting_sz = ARRAY_SIZE(dual_mic_endfire_settings),
};

static struct snddev_icodec_data snddev_dual_mic_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_dual_mic_endfire_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &dual_mic_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_hs_dual_mic_endfire_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_dual_mic_endfire_data },
};

static struct adie_codec_action_unit
	dual_mic_spkr_endfire_8KHz_osr256_actions[] =
	SPKR_AMIC_DUAL_OSR_256;

static struct adie_codec_hwsetting_entry dual_mic_spkr_endfire_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = dual_mic_spkr_endfire_8KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(dual_mic_spkr_endfire_8KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile dual_mic_spkr_endfire_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = dual_mic_spkr_endfire_settings,
	.setting_sz = ARRAY_SIZE(dual_mic_spkr_endfire_settings),
};

static struct snddev_icodec_data snddev_dual_mic_spkr_endfire_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "speaker_dual_mic_endfire_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &dual_mic_spkr_endfire_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_spkr_dual_mic_endfire_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_dual_mic_spkr_endfire_data },
};

static struct adie_codec_action_unit
	ihandset_spkr_hac_rx_48KHz_osr256_actions[] =
	HAC_EAR_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry ihandset_spkr_hac_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ihandset_spkr_hac_rx_48KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(ihandset_spkr_hac_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ihandset_spkr_hac_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ihandset_spkr_hac_rx_settings,
	.setting_sz = ARRAY_SIZE(ihandset_spkr_hac_rx_settings),
};

static struct snddev_icodec_data snddev_ihandset_spkr_hac_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_spkr_hac_rx",
	.copp_id = 0,
	.profile = &ihandset_spkr_hac_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
};

static struct platform_device msm_ihandset_spkr_hac_rx_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_ihandset_spkr_hac_rx_data },
};

static struct adie_codec_action_unit ixloud_speaker_rx_48KHz_osr256_actions[] =
		SPEAKER_PRI_STEREO_XLOUD_48000_OSR_256;

static struct adie_codec_hwsetting_entry ixloud_speaker_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ixloud_speaker_rx_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(ixloud_speaker_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ixloud_speaker_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ixloud_speaker_rx_settings,
	.setting_sz = ARRAY_SIZE(ixloud_speaker_rx_settings),
};

static struct snddev_icodec_data snddev_ixloud_speaker_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "xloud_speaker_stereo_rx",
	.copp_id = PRIMARY_I2S_RX,
	.profile = &ixloud_speaker_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_stereo_poweramp_on,
	.pamp_off = msm_snddev_stereo_poweramp_off,
};

static struct platform_device msm_ixloud_speaker_rx_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_ixloud_speaker_rx_data },
};

static struct adie_codec_action_unit
		ixloud_speaker_mono_rx_48KHz_osr256_actions[] =
		SPEAKER_PRI_MONO_XLOUD_48000_OSR_256;

static struct adie_codec_hwsetting_entry ixloud_speaker_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = ixloud_speaker_mono_rx_48KHz_osr256_actions,
		.action_sz =
		ARRAY_SIZE(ixloud_speaker_mono_rx_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile ixloud_speaker_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = ixloud_speaker_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(ixloud_speaker_mono_rx_settings),
};

static struct snddev_icodec_data snddev_ixloud_speaker_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "xloud_speaker_mono_rx",
	.copp_id = PRIMARY_I2S_RX,
	.profile = &ixloud_speaker_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_mono_poweramp_on,
	.pamp_off = msm_snddev_mono_poweramp_off,
};

static struct platform_device msm_ixloud_speaker_mono_rx_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_ixloud_speaker_mono_rx_data },
};

static struct snddev_icodec_data snddev_iringtone_spkr_stereo_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "ringtone_speaker_stereo_rx",
	.copp_id = 0,
	.profile = &ispkr_stereo_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_stereo_poweramp_on,
	.pamp_off = msm_snddev_stereo_poweramp_off,
};

static struct platform_device msm_iringtone_spkr_stereo_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_iringtone_spkr_stereo_data },
};

static struct snddev_icodec_data snddev_iringtone_spkr_mono_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "ringtone_speaker_mono_rx",
	.copp_id = 0,
	.profile = &ispkr_mono_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_mono_poweramp_on,
	.pamp_off = msm_snddev_mono_poweramp_off,
};

static struct platform_device msm_iringtone_spkr_mono_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_iringtone_spkr_mono_data },
};

static struct snddev_icodec_data snddev_iringtone_xloud_speaker_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "ringtone_xloud_speaker_stereo_rx",
	.copp_id = PRIMARY_I2S_RX,
	.profile = &ixloud_speaker_rx_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_stereo_poweramp_on,
	.pamp_off = msm_snddev_stereo_poweramp_off,
};

static struct platform_device msm_iringtone_xloud_speaker_rx_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_iringtone_xloud_speaker_rx_data },
};

static struct snddev_icodec_data snddev_iringtone_xloud_speaker_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "ringtone_xloud_speaker_mono_rx",
	.copp_id = PRIMARY_I2S_RX,
	.profile = &ixloud_speaker_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_mono_poweramp_on,
	.pamp_off = msm_snddev_mono_poweramp_off,
};

static struct platform_device msm_iringtone_xloud_speaker_mono_rx_device = {
	.name = "snddev_icodec",
	.dev = {
		.platform_data =
		&snddev_iringtone_xloud_speaker_mono_rx_data
	},
};

static struct adie_codec_action_unit imic_asr_osr256_actions[] =
	AMIC_PRI_MONO_ASR_OSR_256;

static struct adie_codec_hwsetting_entry imic_asr_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = imic_asr_osr256_actions,
		.action_sz = ARRAY_SIZE(imic_asr_osr256_actions),
	}
};

static struct adie_codec_dev_profile imic_asr_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = imic_asr_settings,
	.setting_sz = ARRAY_SIZE(imic_asr_settings),
};

static struct snddev_icodec_data snddev_imic_asr_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "handset_asr_tx",
	.copp_id = 1,
	.profile = &imic_asr_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_imic_asr_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_imic_asr_data },
};

static struct snddev_icodec_data snddev_icamcorder_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "camcorder_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &ispeaker_tx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_enable_amic_power,
	.pamp_off = msm_snddev_disable_amic_power,
};

static struct platform_device msm_icamcorder_mic_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_icamcorder_mic_data },
};

static struct adie_codec_action_unit iearpiece_skt_48KHz_osr256_actions[] =
	SKT_EAR_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry iearpiece_skt_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_skt_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_skt_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_skt_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_skt_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_skt_settings),
};

static struct snddev_icodec_data snddev_iearpiece_skt_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_skt_rx",
	.copp_id = 0,
	.profile = &iearpiece_skt_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
};

static struct platform_device msm_iearpiece_skt_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_iearpiece_skt_data },
};

static struct adie_codec_action_unit iearpiece_vzw_48KHz_osr256_actions[] =
	EAR_PRI_MONO_8000_OSR_256;

static struct adie_codec_hwsetting_entry iearpiece_vzw_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = iearpiece_vzw_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE(iearpiece_vzw_48KHz_osr256_actions),
	}
};

static struct adie_codec_dev_profile iearpiece_vzw_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = iearpiece_vzw_settings,
	.setting_sz = ARRAY_SIZE(iearpiece_vzw_settings),
};

static struct snddev_icodec_data snddev_iearpiece_vzw_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "handset_vzw_rx",
	.copp_id = 0,
	.profile = &iearpiece_vzw_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
};

static struct platform_device msm_iearpiece_vzw_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_iearpiece_vzw_data },
};

/* define the value for BT_SCO */

static struct snddev_ecodec_data snddev_bt_sco_earpiece_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_rx",
	.copp_id = PCM_RX,
	.channel_mode = 1,
};

static struct snddev_ecodec_data snddev_bt_sco_mic_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_sco_tx",
	.copp_id = PCM_TX,
	.channel_mode = 1,
};

struct platform_device msm_bt_sco_earpiece_device = {
	.name = "msm_snddev_ecodec",
	.dev = { .platform_data = &snddev_bt_sco_earpiece_data },
};

struct platform_device msm_bt_sco_mic_device = {
	.name = "msm_snddev_ecodec",
	.dev = { .platform_data = &snddev_bt_sco_mic_data },
};

static struct adie_codec_action_unit itty_mono_tx_actions[] =
	TTY_HEADSET_MONO_TX_OSR_256;

static struct adie_codec_hwsetting_entry itty_mono_tx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_mono_tx_actions,
		.action_sz = ARRAY_SIZE(itty_mono_tx_actions),
	},
};

static struct adie_codec_dev_profile itty_mono_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = itty_mono_tx_settings,
	.setting_sz = ARRAY_SIZE(itty_mono_tx_settings),
};

static struct snddev_icodec_data snddev_itty_mono_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &itty_mono_tx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
};

static struct platform_device msm_itty_mono_tx_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_itty_mono_tx_data },
};

static struct adie_codec_action_unit itty_mono_rx_actions[] =
	TTY_HEADSET_MONO_RX_8000_OSR_256;

static struct adie_codec_hwsetting_entry itty_mono_rx_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = itty_mono_rx_actions,
		.action_sz = ARRAY_SIZE(itty_mono_rx_actions),
	},
};

static struct adie_codec_dev_profile itty_mono_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = itty_mono_rx_settings,
	.setting_sz = ARRAY_SIZE(itty_mono_rx_settings),
};

static struct snddev_icodec_data snddev_itty_mono_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE | SNDDEV_CAP_TTY),
	.name = "tty_headset_mono_rx",
	.copp_id = PRIMARY_I2S_RX,
	.profile = &itty_mono_rx_profile,
	.channel_mode = 1,
	.default_sample_rate = 48000,
	.voltage_on = msm_snddev_voltage_on,
	.voltage_off = msm_snddev_voltage_off,
};

static struct platform_device msm_itty_mono_rx_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_itty_mono_rx_data },
};

static struct adie_codec_action_unit linein_pri_actions[] =
	LINEIN_PRI_STEREO_OSR_256;

static struct adie_codec_hwsetting_entry linein_pri_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = linein_pri_actions,
		.action_sz = ARRAY_SIZE(linein_pri_actions),
	},
};

static struct adie_codec_dev_profile linein_pri_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = linein_pri_settings,
	.setting_sz = ARRAY_SIZE(linein_pri_settings),
};

static struct snddev_icodec_data snddev_linein_pri_data = {
	.capability = SNDDEV_CAP_TX,
	.name = "linein_pri_tx",
	.copp_id = PRIMARY_I2S_TX,
	.profile = &linein_pri_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.voltage_on = msm_snddev_voltage_on,
	.voltage_off = msm_snddev_voltage_off,
};

static struct platform_device msm_linein_pri_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_linein_pri_data },
};

static struct adie_codec_action_unit auxpga_lp_lo_actions[] =
	LB_AUXPGA_LO_STEREO;

static struct adie_codec_hwsetting_entry auxpga_lp_lo_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = auxpga_lp_lo_actions,
		.action_sz = ARRAY_SIZE(auxpga_lp_lo_actions),
	},
};

static struct adie_codec_dev_profile auxpga_lp_lo_profile = {
	.path_type = ADIE_CODEC_LB,
	.settings = auxpga_lp_lo_settings,
	.setting_sz = ARRAY_SIZE(auxpga_lp_lo_settings),
};

static struct snddev_icodec_data snddev_auxpga_lp_lo_data = {
	.capability = SNDDEV_CAP_LB,
	.name = "speaker_stereo_lb",
	.copp_id = PRIMARY_I2S_RX,
	.profile = &auxpga_lp_lo_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.pamp_on = msm_snddev_stereo_poweramp_on,
	.pamp_off = msm_snddev_stereo_poweramp_off,
	.dev_vol_type = SNDDEV_DEV_VOL_ANALOG,
};

static struct platform_device msm_auxpga_lp_lo_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_auxpga_lp_lo_data },
};

static struct adie_codec_action_unit auxpga_lp_hs_actions[] =
	LB_AUXPGA_HPH_AB_CPLS_STEREO;

static struct adie_codec_hwsetting_entry auxpga_lp_hs_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions = auxpga_lp_hs_actions,
		.action_sz = ARRAY_SIZE(auxpga_lp_hs_actions),
	},
};

static struct adie_codec_dev_profile auxpga_lp_hs_profile = {
	.path_type = ADIE_CODEC_LB,
	.settings = auxpga_lp_hs_settings,
	.setting_sz = ARRAY_SIZE(auxpga_lp_hs_settings),
};

static struct snddev_icodec_data snddev_auxpga_lp_hs_data = {
	.capability = SNDDEV_CAP_LB,
	.name = "hs_stereo_lb",
	.copp_id = PRIMARY_I2S_RX,
	.profile = &auxpga_lp_hs_profile,
	.channel_mode = 2,
	.default_sample_rate = 48000,
	.voltage_on = msm_snddev_voltage_on,
	.voltage_off = msm_snddev_voltage_off,
	.dev_vol_type = SNDDEV_DEV_VOL_ANALOG,
};

static struct platform_device msm_auxpga_lp_hs_device = {
	.name = "snddev_icodec",
	.dev = { .platform_data = &snddev_auxpga_lp_hs_data },
};

static struct snddev_virtual_data snddev_uplink_rx_data = {
	.capability = SNDDEV_CAP_RX,
	.name = "uplink_rx",
	.copp_id = VOICE_PLAYBACK_TX,
};

static struct platform_device msm_uplink_rx_device = {
	.name = "snddev_virtual",
	.dev = { .platform_data = &snddev_uplink_rx_data },
};

static struct snddev_hdmi_data snddev_hdmi_non_linear_pcm_rx_data = {
	.capability = SNDDEV_CAP_RX ,
	.name = "hdmi_pass_through",
	.default_sample_rate = 48000,
	.on_apps = 1,
};

static struct platform_device msm_snddev_hdmi_non_linear_pcm_rx_device = {
	.name = "snddev_hdmi",
	.dev = { .platform_data = &snddev_hdmi_non_linear_pcm_rx_data },
};

static struct snddev_ecodec_data snddev_bt_carkit_rx_data = {
	.capability = (SNDDEV_CAP_RX | SNDDEV_CAP_VOICE),
	.name = "bt_carkit_rx",
	.copp_id = PCM_RX,
	.channel_mode = 1,
};

struct platform_device msm_bt_carkit_rx_device = {
	.name = "msm_snddev_ecodec",
	.dev = { .platform_data = &snddev_bt_carkit_rx_data },
};

static struct snddev_ecodec_data snddev_bt_carkit_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_carkit_tx",
	.copp_id = PCM_TX,
	.channel_mode = 1,
};

struct platform_device msm_bt_carkit_tx_device = {
	.name = "msm_snddev_ecodec",
	.dev = { .platform_data = &snddev_bt_carkit_tx_data },
};

static struct snddev_ecodec_data snddev_bt_dsp_sco_tx_data = {
	.capability = (SNDDEV_CAP_TX | SNDDEV_CAP_VOICE),
	.name = "bt_dsp_sco_tx",
	.copp_id = PCM_TX,
	.channel_mode = 1,
};

struct platform_device msm_bt_dsp_sco_tx_device = {
	.name = "msm_snddev_ecodec",
	.dev = { .platform_data = &snddev_bt_dsp_sco_tx_data },
};

#ifdef CONFIG_DEBUG_FS
static struct adie_codec_action_unit
	ihs_stereo_rx_class_d_legacy_48KHz_osr256_actions[] =
	HPH_PRI_D_LEG_STEREO;

static struct adie_codec_hwsetting_entry
	ihs_stereo_rx_class_d_legacy_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions =
		ihs_stereo_rx_class_d_legacy_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE
		(ihs_stereo_rx_class_d_legacy_48KHz_osr256_actions),
	}
};

static struct adie_codec_action_unit
	ihs_stereo_rx_class_ab_legacy_48KHz_osr256_actions[] =
	HPH_PRI_AB_LEG_STEREO;

static struct adie_codec_hwsetting_entry
	ihs_stereo_rx_class_ab_legacy_settings[] = {
	{
		.freq_plan = 48000,
		.osr = 256,
		.actions =
		ihs_stereo_rx_class_ab_legacy_48KHz_osr256_actions,
		.action_sz = ARRAY_SIZE
		(ihs_stereo_rx_class_ab_legacy_48KHz_osr256_actions),
	}
};

static void snddev_hsed_config_modify_setting(int type)
{
	struct platform_device *device;
	struct snddev_icodec_data *icodec_data;

	device = &msm_headset_stereo_device;
	icodec_data = (struct snddev_icodec_data *)device->dev.platform_data;

	if (icodec_data) {
		if (type == 1) {
			icodec_data->voltage_on = NULL;
			icodec_data->voltage_off = NULL;
			icodec_data->profile->settings =
				ihs_stereo_rx_class_d_legacy_settings;
			icodec_data->profile->setting_sz =
			ARRAY_SIZE(ihs_stereo_rx_class_d_legacy_settings);
		} else if (type == 2) {
			icodec_data->voltage_on = NULL;
			icodec_data->voltage_off = NULL;
			icodec_data->profile->settings =
				ihs_stereo_rx_class_ab_legacy_settings;
			icodec_data->profile->setting_sz =
			ARRAY_SIZE(ihs_stereo_rx_class_ab_legacy_settings);
		}
	}
}

static void snddev_hsed_config_restore_setting(void)
{
	struct platform_device *device;
	struct snddev_icodec_data *icodec_data;

	device = &msm_headset_stereo_device;
	icodec_data = (struct snddev_icodec_data *)device->dev.platform_data;

	if (icodec_data) {
		icodec_data->voltage_on = msm_snddev_voltage_on;
		icodec_data->voltage_off = msm_snddev_voltage_off;
		icodec_data->profile->settings = headset_ab_cpls_settings;
		icodec_data->profile->setting_sz =
			ARRAY_SIZE(headset_ab_cpls_settings);
	}
}

static ssize_t snddev_hsed_config_debug_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char *lb_str = filp->private_data;
	char cmd;

	if (get_user(cmd, ubuf))
		return -EFAULT;

	if (!strncmp(lb_str, "msm_hsed_config", 15)) {
		switch (cmd) {
		case '0':
			snddev_hsed_config_restore_setting();
			break;

		case '1':
			snddev_hsed_config_modify_setting(1);
			break;

		case '2':
			snddev_hsed_config_modify_setting(2);
			break;

		default:
			break;
		}
	}
	return cnt;
}

static int snddev_hsed_config_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static const struct file_operations snddev_hsed_config_debug_fops = {
	.open = snddev_hsed_config_debug_open,
	.write = snddev_hsed_config_debug_write
};
#endif

static struct platform_device *snd_devices_semc[] __initdata = {
	&msm_iearpiece_device,
	&msm_imic_device,
	&msm_ispkr_stereo_device,
	&msm_ispkr_mono_device,
	&msm_snddev_hdmi_stereo_rx_device,
	&msm_headset_mic_device,
	&msm_ispkr_mic_device,
	&msm_bt_sco_earpiece_device,
	&msm_bt_sco_mic_device,
	&msm_headset_stereo_device,
	&msm_itty_mono_tx_device,
	&msm_itty_mono_rx_device,
	&msm_mi2s_fm_tx_device,
	&msm_mi2s_fm_rx_device,
	&msm_ihs_stereo_speaker_stereo_rx_device,
	&msm_ihs_stereo_speaker_mono_rx_device,
	&msm_auxpga_lp_hs_device,
	&msm_auxpga_lp_lo_device,
	&msm_linein_pri_device,
	&msm_snddev_hdmi_non_linear_pcm_rx_device,
	&msm_secondary_imic_device,
	&msm_ispkr_dual_mic_endfire_tx_real_stereo_device,
	&msm_hs_dual_mic_endfire_device,
	&msm_spkr_dual_mic_endfire_device,
	&msm_ihandset_spkr_hac_rx_device,
	&msm_ixloud_speaker_rx_device,
	&msm_ixloud_speaker_mono_rx_device,
	&msm_iringtone_spkr_stereo_device,
	&msm_iringtone_spkr_mono_device,
	&msm_iringtone_xloud_speaker_rx_device,
	&msm_iringtone_xloud_speaker_mono_rx_device,
	&msm_bt_carkit_rx_device,
	&msm_bt_carkit_tx_device,
	&msm_bt_dsp_sco_tx_device,
	&msm_imic_asr_device,
	&msm_icamcorder_mic_device,
	&msm_iearpiece_skt_device,
	&msm_iearpiece_vzw_device,
};

static struct platform_device *snd_devices_common[] __initdata = {
	&msm_aux_pcm_device,
	&msm_cdcclk_ctl_device,
	&msm_mi2s_device,
	&msm_uplink_rx_device,
	&msm_device_dspcrashd_8x60,
};

void __init msm_snddev_init(void)
{
	int i;
	int dev_id;

	atomic_set(&preg_ref_cnt, 0);

	for (i = 0, dev_id = 0; i < ARRAY_SIZE(snd_devices_common); i++)
		snd_devices_common[i]->id = dev_id++;

	platform_add_devices(snd_devices_common,
		ARRAY_SIZE(snd_devices_common));

	for (i = 0; i < ARRAY_SIZE(snd_devices_semc); i++)
		snd_devices_semc[i]->id = dev_id++;

	platform_add_devices(snd_devices_semc,
		ARRAY_SIZE(snd_devices_semc));

#ifdef CONFIG_DEBUG_FS
	debugfs_hsed_config = debugfs_create_file("msm_hsed_config",
				S_IFREG | S_IRUGO, NULL,
		(void *) "msm_hsed_config", &snddev_hsed_config_debug_fops);
#endif
}
