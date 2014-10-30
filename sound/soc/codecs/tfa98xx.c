/* Copyright (C) 2013 Sony Mobile Communications Inc.
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

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <sound/soc.h>
#include <sound/tfa98xx.h>
#include "tfa98xx_if.h"
#include "hal/inc/NXP_I2C.h"
#include "tfa/inc/Tfa98xx.h"
#include "tfa/inc/Tfa98xx_Registers.h"
#include "tfa/inc/Tfa98xx_internals.h"

static Tfa98xx_handle_t handles[3] = {-1, -1, -1};

static bool speaker_amp_control_enable;
static bool speaker_amp_control_on;

#define TOP    0
#define BOTTOM 1
#define BOTH   2


enum {
	SPEAKER_TYPE_NONE = -1,
	SPEAKER_TYPE_PLAYBACK = 0,
	SPEAKER_TYPE_RINGTONE,
	SPEAKER_TYPE_SFORCE_PLAYBACK,
	SPEAKER_TYPE_SPEAKERPHONE,
	SPEAKER_TYPE_VOICECALL,
	SPEAKER_TYPE_FM,
	SPEAKER_TYPE_MAX};

enum {
	SPEAKER_CHANNEL_NONE = -1,
	SPEAKER_CHANNEL_TOP = 0,
	SPEAKER_CHANNEL_BOTTOM,
	SPEAKER_CHANNEL_BOTH};

enum {
	SPEAKER_LR_NORMAL = 0,
	SPEAKER_LR_SWAP};

enum {
	SPEAKER_BYPASS_DSP_OFF = 0,
	SPEAKER_BYPASS_DSP_ON};

enum {
	SPEAKER_UNSYNCED = 0,
	SPEAKER_SYNCED};

static int speaker_type = SPEAKER_TYPE_NONE;
static int speaker_channel = SPEAKER_CHANNEL_NONE;
static int speaker_lr = SPEAKER_LR_NORMAL;
static int speaker_bypass_dsp = SPEAKER_BYPASS_DSP_OFF;
static int speaker_synced = SPEAKER_UNSYNCED;
static int speaker_type_now = SPEAKER_TYPE_NONE;
static int speaker_channel_now = SPEAKER_CHANNEL_NONE;
static int speaker_lr_now = SPEAKER_LR_NORMAL;
static int speaker_bypass_dsp_now = SPEAKER_BYPASS_DSP_OFF;
static int speaker_synced_now = SPEAKER_UNSYNCED;

static const char *const speaker_type_text[] = {
	"Playback", "Ringtone", "SForcePlayback",
	"Speakerphone", "Voicecall", "FM"};

static const struct soc_enum speaker_type_enum =
	SOC_ENUM_SINGLE_EXT(8, speaker_type_text);

static const char *const speaker_channel_text[] = {"Top", "Bottom", "Both"};
static const struct soc_enum speaker_channel_enum =
	SOC_ENUM_SINGLE_EXT(3, speaker_channel_text);

static const char *const speaker_lr_text[] = {"Normal", "Swap"};
static const struct soc_enum speaker_lr_enum =
	SOC_ENUM_SINGLE_EXT(2, speaker_lr_text);

static const char *const speaker_bypass_dsp_text[] = {"Off", "On"};
static const struct soc_enum speaker_bypass_dsp_enum =
	SOC_ENUM_SINGLE_EXT(2, speaker_bypass_dsp_text);

static const char *const speaker_synced_text[] = {"Unsynced", "Synced"};
static const struct soc_enum speaker_synced_enum =
	SOC_ENUM_SINGLE_EXT(2, speaker_synced_text);

#define TOP_I2C_ADDRESS  0x68
#define BOTTOM_I2C_ADDRESS  0x6A
#define SAMPLE_RATE 48000
#define SPKR_CALIBRATION_EXTTS_VALUE 26

static struct tfa98xx_param_data patch_data[PATCH_MAX];
static struct tfa98xx_param_data config_data[AMP_MAX];
static struct tfa98xx_param_data speaker_data[AMP_MAX];
static struct tfa98xx_param_data preset_data[TYPE_MAX];
static struct tfa98xx_param_data eql_data[TYPE_MAX];

static struct tfa98xx_param_data *preset_data_table[SPEAKER_TYPE_MAX][2] = {
	/* SPEAKER_TYPE_PLAYBACK */
	{&preset_data[TYPE_HIFISPEAKER_TOP],
	 &preset_data[TYPE_HIFISPEAKER_BOTTOM]},
	/* SPEAKER_TYPE_RINGTONE */
	{&preset_data[TYPE_HIFISPEAKER_RING_TOP],
	 &preset_data[TYPE_HIFISPEAKER_RING_BOTTOM]},
	/* SPEAKER_TYPE_SFORCE_PLAYBACK */
	{&preset_data[TYPE_HIFISPEAKER_SFORCE_TOP],
	 &preset_data[TYPE_HIFISPEAKER_SFORCE_BOTTOM]},
	/*  SPEAKER_TYPE_SPEAKERPHONE */
	{&preset_data[TYPE_VOICECALLSPEAKER_TOP],
	 &preset_data[TYPE_VOICECALLSPEAKER_BOTTOM]},
	/* SPEAKER_TYPE_VOICECALL */
	{&preset_data[TYPE_VOICECALLEARPICE_TOP],
	 NULL},
	/* SPEAKER_TYPE_FM */
	{&preset_data[TYPE_FMSPEAKER_TOP],
	 &preset_data[TYPE_FMSPEAKER_BOTTOM]},
};

static struct tfa98xx_param_data *eql_data_table[SPEAKER_TYPE_MAX][2] = {
	/* SPEAKER_TYPE_PLAYBACK */
	{&eql_data[TYPE_HIFISPEAKER_TOP],
	 &eql_data[TYPE_HIFISPEAKER_BOTTOM]},
	/* SPEAKER_TYPE_RINGTONE */
	{&eql_data[TYPE_HIFISPEAKER_RING_TOP],
	 &eql_data[TYPE_HIFISPEAKER_RING_BOTTOM]},
	/* SPEAKER_TYPE_SFORCE_PLAYBACK */
	{&eql_data[TYPE_HIFISPEAKER_SFORCE_TOP],
	 &eql_data[TYPE_HIFISPEAKER_SFORCE_BOTTOM]},
	/*  SPEAKER_TYPE_SPEAKERPHONE */
	{&eql_data[TYPE_VOICECALLSPEAKER_TOP],
	 &eql_data[TYPE_VOICECALLSPEAKER_BOTTOM]},
	/* SPEAKER_TYPE_VOICECALL */
	{&eql_data[TYPE_VOICECALLEARPICE_TOP],
	 NULL},
	/* SPEAKER_TYPE_FM */
	{&eql_data[TYPE_FMSPEAKER_TOP],
	 &eql_data[TYPE_FMSPEAKER_BOTTOM]},
};


static int tfa98xx_enable(void);
static int tfa98xx_disable(void);
static int tfa98xx_bypass_dsp(Tfa98xx_handle_t handleIn);
static int tfa98xx_unbypass_dsp(Tfa98xx_handle_t handleIn);

static bool speaker_amp_control_enable;
static bool speaker_amp_control_on;

void tfa98xx_speaker_amp_enable(void)
{
	pr_info("speaker_amp_control_enable:%d speaker_amp_control_on:%d\n",
		speaker_amp_control_enable, speaker_amp_control_on);
	if (speaker_amp_control_enable != false
		&& speaker_amp_control_on == false){
		tfa98xx_enable();
		speaker_amp_control_on = true;
	}
}

static void set_speaker_amp_control_enable(bool enable)
{
	if (speaker_amp_control_on == true && enable == false) {
		tfa98xx_disable();
		speaker_amp_control_on = false;
		pr_info("%s speaker_amp disable\n", __func__);
	}
	speaker_amp_control_enable = enable;
}

/* load a speaker model from a file, as generated by the GUI, or saved from
   a previous execution */
static void setSpeaker(int handle_cnt, Tfa98xx_handle_t handles[],
	struct tfa98xx_param_data *speaker)
{
	enum Tfa98xx_Error err;

	if (speaker == NULL || speaker->data == NULL) {
		pr_info("%s: not use speaker data\n", __func__);
		return;
	}
	pr_debug("%s: start\n", __func__);

	if (handle_cnt >= 2)
		err = Tfa98xx_DspWriteSpeakerParametersMultiple(handle_cnt,
			handles, speaker->size, speaker->data);
	else
		err = Tfa98xx_DspWriteSpeakerParameters(handles[0],
			speaker->size, speaker->data);

	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_DspWriteSpeakerParameters failed\n",
			__func__);

}


static void setConfig(int handle_cnt, Tfa98xx_handle_t handles[],
	struct tfa98xx_param_data *config)
{
	enum Tfa98xx_Error err;

	if (config == NULL || config->data == NULL) {
		pr_info("%s: not use config data\n", __func__);
		return;
	}
	pr_debug("%s: start\n", __func__);

	if (handle_cnt >= 2)
		err = Tfa98xx_DspWriteConfigMultiple(handle_cnt, handles,
			config->size, config->data);
	else
		err = Tfa98xx_DspWriteConfig(handles[0],
			config->size, config->data);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_DspWriteConfig failed\n", __func__);
}

/* load a preset from a file, as generated by the GUI,
   can be done at runtime */
static void setPreset(int handle_cnt, Tfa98xx_handle_t handles[],
	struct tfa98xx_param_data *preset)
{
	enum Tfa98xx_Error err;

	if (preset == NULL || preset->data == NULL) {
		pr_info("%s: not use preset data\n", __func__);
		return;
	}
	pr_debug("%s: start\n", __func__);

	if (handle_cnt >= 2)
		err = Tfa98xx_DspWritePresetMultiple(handle_cnt, handles,
			TFA98XX_PRESET_LENGTH, preset->data);
	else
		err = Tfa98xx_DspWritePreset(handles[0],
			TFA98XX_PRESET_LENGTH, preset->data);

	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_DspWritePreset failed\n", __func__);

}

/* load a set of EQ settings from a file, as generated by the GUI, can be done
   at runtime */
static void setEQ(int handle_cnt, Tfa98xx_handle_t handles[],
	struct tfa98xx_param_data *eq)
{
	enum Tfa98xx_Error err;
	int index;
	int i;
	bool disable;

	if (eq == NULL || eq->data == NULL) {
		pr_info("%s: not use eq data\n", __func__);
		return;
	}
	pr_debug("%s: start\n", __func__);

	index = 0;
	while (index + (BIQUAD_COEFF_SIZE * 3) + 1 <= eq->size) {
		disable = true;
		for (i = 0; i < (BIQUAD_COEFF_SIZE * 3); i++) {
			if (eq->data[index + 1 + i] != 0) {
				disable = false;
				break;
			}
		}
		if (disable == false) {
			if (handle_cnt >= 2)
				err = Tfa98xx_DspSetParamMultiple(
					handle_cnt,
					handles,
					MODULE_BIQUADFILTERBANK,
					eq->data[index],
					(BIQUAD_COEFF_SIZE * 3),
					&eq->data[index + 1]);
			else
				err = Tfa98xx_DspSetParam(
					handles[0],
					MODULE_BIQUADFILTERBANK,
					eq->data[index],
					(BIQUAD_COEFF_SIZE * 3),
					&eq->data[index + 1]);
		} else {
			if (handle_cnt >= 2)
				err = Tfa98xx_DspBiquad_DisableMultiple(
					handle_cnt, handles, eq->data[index]);
			else
				err = Tfa98xx_DspBiquad_Disable(
					handles[0], eq->data[index]);
		}
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: failed %d:%d\n",
				__func__, eq->data[index], err);
		index += (BIQUAD_COEFF_SIZE * 3) + 1;
	}

}

/* load a DSP ROM code patch from file */
static void dspPatch(Tfa98xx_handle_t handle,
	struct tfa98xx_param_data *patch)
{
	enum Tfa98xx_Error err;

	if (patch == NULL || patch->data == NULL) {
		pr_info("%s: not use patch data\n", __func__);
		return;
	}

	err = Tfa98xx_DspPatch(handle, patch->size, patch->data);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_DspPatch failed %d\n", __func__, err);
}

static void waitCalibration(Tfa98xx_handle_t handle, int *calibrateDone)
{
	enum Tfa98xx_Error err;
	int tries = 0;
	unsigned short mtp;
	unsigned short spkrCalibration;
#define WAIT_TRIES 1000

	err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);

	/* in case of calibrate once wait for MTPEX */
	if (mtp & TFA98XX_MTP_MTPOTC) {
		while ((*calibrateDone == 0) && (tries < WAIT_TRIES)) {
			err = Tfa98xx_ReadRegister16(handle,
				TFA98XX_MTP, &mtp);
			/* check MTP bit1 (MTPEX) */
			*calibrateDone = (mtp & TFA98XX_MTP_MTPEX);
			tries++;
		}
	} else { /* poll xmem for calibrate always */
		while ((*calibrateDone == 0) && (tries < WAIT_TRIES)) {
			/* TODO optimise with wait estimation */
			err = Tfa98xx_DspReadMem(handle, 231, 1,
				calibrateDone);
			tries++;
		}
		if (tries == WAIT_TRIES)
			pr_err("%s: calibrateDone 231 timedout\n", __func__);
	}

	err = Tfa98xx_ReadRegister16(handle,
		TFA98XX_SPKR_CALIBRATION, &spkrCalibration);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
			__func__, err);
		return;
	}

	spkrCalibration &= ~(TFA98XX_SPKR_CALIBRATION_TROS_MSK);

	err = Tfa98xx_WriteRegister16(handle,
		TFA98XX_SPKR_CALIBRATION, spkrCalibration);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_WriteRegister16 failed %d\n",
			__func__, err);
		return;
	}
}


static void coldStartup(Tfa98xx_handle_t handle)
{
	enum Tfa98xx_Error err;
	unsigned short status;
	int ready = 0;
	int timeout;
	unsigned short dcdcRead = 0;
	unsigned short dcdcBoost = 0;

	/* load the optimal TFA98xx in HW settings */
	err = Tfa98xx_Init(handle);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_Init failed %d\n", __func__, err);
		return;
	}

	/* NXP SL: Sample rate should be set before power up */
	/* Set sample rate to example 48000*/
	err = Tfa98xx_SetSampleRate(handle, SAMPLE_RATE);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_SetSampleRate failed %d\n", __func__, err);

	/* Power On the device by setting bit 0 to 0 of register 9*/
	err = Tfa98xx_Powerdown(handle, 0);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_Powerdown failed %d\n", __func__, err);
		return;
	}

	/* set Max boost coil current 1.92 A */
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_DCDCBOOST, &dcdcRead);
	dcdcRead &= ~(TFA98XX_DCDCBOOST_DCMCC_MSK);
	dcdcRead |= (3 << TFA98XX_DCDCBOOST_DCMCC_POS);
	err = Tfa98xx_WriteRegister16(handle, TFA98XX_DCDCBOOST, dcdcRead);

	/* set Max boost voltage 7.5 V */
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_DCDCBOOST, &dcdcBoost);
	dcdcBoost &= ~(TFA98XX_DCDCBOOST_DCVO_MSK);
	dcdcBoost |= 3;
	err = Tfa98xx_WriteRegister16(handle, TFA98XX_DCDCBOOST, dcdcBoost);

	/* Check the PLL is powered up from status register 0*/
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
			__func__, err);
		return;
	}
	timeout = 0;
	while ((status & TFA98XX_STATUSREG_AREFS_MSK) == 0) {
		/* not ok yet */
		err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG,
			&status);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
				__func__, err);
			return;
		}
		msleep(20);
		timeout++;
		if (timeout > 50) {
			pr_info("%s timeout status:%x\n", __func__, status);
			break;
		}
	}

	/*  powered on
	 *    - now it is allowed to access DSP specifics
	 *    - stall DSP by setting reset
	 * */
	err = Tfa98xx_DspReset(handle, 1);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_DspReset failed %d\n", __func__, err);
		return;
	}

	/*  wait until the DSP subsystem hardware is ready
	 *    note that the DSP CPU is not running yet (RST=1)
	 * */
	timeout = 0;
	while (ready == 0) {
		/* are we ready? */
		err = Tfa98xx_DspSystemStable(handle, &ready);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_DspSystemStable failed %d\n",
				__func__, err);
			return;
		}
		msleep(20);
		timeout++;
		if (timeout > 50) {
			pr_info("%s timeout ready:%d\n", __func__, ready);
			break;
		}
	}

   /* Load cold-boot patch for the first time to force cold start-up.
    *  use the patchload only to write the internal register
    * */
	dspPatch(handle, &patch_data[PATCH_COLDBOOT]);

	err = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
			__func__, err);
		return;
	}

	if ((status & TFA98XX_STATUSREG_ACS_MSK) == 0) {
		pr_err("%s: status & TFA98XX_STATUSREG_ACS_MSK == 0\n",
			__func__);
		return;
	}

	/* cold boot, need to load all parameters and patches */
	/* patch the ROM code */
	dspPatch(handle, &patch_data[PATCH_DSP]);
}

static void setOtc(Tfa98xx_handle_t handle, int otcOn)
{
	enum Tfa98xx_Error err;
	unsigned short mtp;
	unsigned short status;
	unsigned short spkrCalibration;
	int mtpChanged = 0;
	int timeout;

	err = Tfa98xx_ReadRegister16(handle,
		TFA98XX_SPKR_CALIBRATION, &spkrCalibration);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
			__func__, err);
		return;
	}

	spkrCalibration |= TFA98XX_SPKR_CALIBRATION_TROS_MSK;
	spkrCalibration &= ~(TFA98XX_SPKR_CALIBRATION_EXTTS_MSK);
	spkrCalibration |= (SPKR_CALIBRATION_EXTTS_VALUE
		<< TFA98XX_SPKR_CALIBRATION_EXTTS_POS);

	err = Tfa98xx_WriteRegister16(handle,
		TFA98XX_SPKR_CALIBRATION, spkrCalibration);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_WriteRegister16 failed %d\n",
			__func__, err);
		return;
	}

	err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_DspReset failed %d\n", __func__, err);
		return;
	}

	if ((otcOn != 0) && (otcOn != 1)) {
		pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
			__func__, err);
		return;
	}

	/* set reset MTPEX bit if needed */
	if ((mtp & TFA98XX_MTP_MTPOTC) != otcOn) {
		/* need to change the OTC bit, set MTPEX=0 in any case */
		/* unlock key2 */
		err = Tfa98xx_WriteRegister16(handle, 0x0B, 0x5A);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_DspReset failed %d\n",
				__func__, err);
			return;
		}

		/* MTPOTC=otcOn, MTPEX=0 */
		err = Tfa98xx_WriteRegister16(handle, TFA98XX_MTP,
			(unsigned short)otcOn);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_DspReset failed %d\n",
				__func__, err);
			return;
		}

		/* CIMTP=1 */
		err = Tfa98xx_WriteRegister16(handle, 0x62, 1<<11);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_DspReset failed %d\n",
				__func__, err);
			return;
		}

		mtpChanged = 1;

	}

	timeout = 0;
	do {
		msleep(20);
		err = Tfa98xx_ReadRegister16(handle,
			TFA98XX_STATUSREG, &status);
		pr_info("%s %04x\n", __func__, status);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_DspReset failed %d\n",
				__func__, err);
			return;
		}
		timeout++;
		if (timeout > 50) {
			pr_info("%s timeout\n", __func__);
			break;
		}
	} while ((status & TFA98XX_STATUSREG_MTPB_MSK)
		== TFA98XX_STATUSREG_MTPB_MSK);

}

static int checkMTPEX(Tfa98xx_handle_t handle)
{
	unsigned short mtp;
	enum Tfa98xx_Error err;
	err = Tfa98xx_ReadRegister16(handle, TFA98XX_MTP, &mtp);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
			__func__, err);
	}

	if (mtp & (1<<1))  /* check MTP bit1 (MTPEX) */
		return 1;   /* MTPEX is 1, calibration is done */
	else
		return 0;   /* MTPEX is 0, calibration is not done yet */
}

static int set_speaker_lr(Tfa98xx_handle_t handles[2], int speaker_lr)
{
	enum Tfa98xx_Error err = Tfa98xx_Error_Ok;
	int left, right;

	if (speaker_lr == SPEAKER_LR_NORMAL) {
		left = TOP;
		right = BOTTOM;
	} else {
		left = BOTTOM;
		right = TOP;
	}

	err = Tfa98xx_SelectChannel(handles[left], Tfa98xx_Channel_L);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_SelectChannel failed\n", __func__);
		return -EINVAL;
	}

	err = Tfa98xx_SelectChannel(handles[right], Tfa98xx_Channel_R);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_SelectChannel failed\n", __func__);
		return -EINVAL;
	}

	if (speaker_synced == SPEAKER_SYNCED) {
		/* ensure stereo routing is correct: in this example we use
		 * gain is on L channel from 1->2
		 * gain is on R channel from 2->1
		 * on the other channel of DATAO we put Isense
		 */
		err = Tfa98xx_SelectI2SOutputLeft(
			handles[left], Tfa98xx_I2SOutputSel_DSP_Gain);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputLeft failed\n",
				__func__);
			return -EINVAL;
		}

		err = Tfa98xx_SelectStereoGainChannel(
			handles[right], Tfa98xx_StereoGainSel_Left);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectStereoGainChannel failed\n",
				__func__);
			return -EINVAL;
	}

		err = Tfa98xx_SelectI2SOutputRight(
			handles[right], Tfa98xx_I2SOutputSel_DSP_Gain);
	if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputRight failed\n",
				__func__);
		return -EINVAL;
	}

	err = Tfa98xx_SelectStereoGainChannel(
			handles[left], Tfa98xx_StereoGainSel_Right);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_SelectStereoGainChannel failed\n",
			__func__);
		return -EINVAL;
	}

		err = Tfa98xx_SelectI2SOutputRight(
			handles[left], Tfa98xx_I2SOutputSel_CurrentSense);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputRight failed\n",
				__func__);
			return -EINVAL;
		}
		err = Tfa98xx_SelectI2SOutputLeft(
			handles[right], Tfa98xx_I2SOutputSel_CurrentSense);
	if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputLeft failed\n",
				__func__);
		return -EINVAL;
	}
	}

	if (speaker_type == SPEAKER_TYPE_SPEAKERPHONE) {
		err = Tfa98xx_SelectI2SOutputLeft(
			handles[TOP], Tfa98xx_I2SOutputSel_CurrentSense);
	if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputLeft failed\n",
			__func__);
		return -EINVAL;
	}
	err = Tfa98xx_SelectI2SOutputRight(
			handles[TOP], Tfa98xx_I2SOutputSel_DSP_AEC);
	if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputRight failed\n",
				__func__);
		return -EINVAL;
	}

	err = Tfa98xx_SelectI2SOutputLeft(
			handles[BOTTOM], Tfa98xx_I2SOutputSel_DSP_AEC);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputLeft failed\n",
				__func__);
			return -EINVAL;
		}
		err = Tfa98xx_SelectI2SOutputRight(
			handles[BOTTOM], Tfa98xx_I2SOutputSel_DataI3R);
	if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputRight failed\n",
				__func__);
		return -EINVAL;
	}

		err = Tfa98xx_SelectI2SOutputLeft(
			handles[TOP], Tfa98xx_I2SOutputSel_DSP_Gain);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputLeft failed\n",
				__func__);
			return -EINVAL;
		}
		err = Tfa98xx_SelectI2SOutputRight(
			handles[TOP], Tfa98xx_I2SOutputSel_CurrentSense);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputRight failed\n",
				__func__);
			return -EINVAL;
		}
		err = Tfa98xx_SelectStereoGainChannel(
			handles[TOP], Tfa98xx_StereoGainSel_Right);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectStereoGainChannel failed\n",
				__func__);
			return -EINVAL;
		}

		err = Tfa98xx_SelectI2SOutputLeft(
			handles[BOTTOM], Tfa98xx_I2SOutputSel_DSP_AEC);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputLeft failed\n",
				__func__);
			return -EINVAL;
		}
		err = Tfa98xx_SelectI2SOutputRight(
			handles[BOTTOM], Tfa98xx_I2SOutputSel_DSP_Gain);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectI2SOutputRight failed\n",
				__func__);
			return -EINVAL;
		}
		err = Tfa98xx_SelectStereoGainChannel(
			handles[BOTTOM], Tfa98xx_StereoGainSel_Left);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectStereoGainChannel failed\n",
				__func__);
			return -EINVAL;
		}
	}

	return 0;
}

static int stereo_speaker_on(
	struct tfa98xx_param_data *speaker[2],
	struct tfa98xx_param_data *config[2],
	struct tfa98xx_param_data *preset[2],
	struct tfa98xx_param_data *eq[2])
{
	enum Tfa98xx_Error err = Tfa98xx_Error_Ok;
	unsigned char h;
	FIXEDPT re25;
	int calibrateDone = 0;

	/* use the generic slave address for optimizations */

	for (h = 0; h < 2; h++) {
		coldStartup(handles[h]);
		/*Set to calibration once*/
		/* Only needed for really first time calibration */
		setOtc(handles[h], 1);

		/* Check if MTPEX bit is set for calibration once mode */
		if (checkMTPEX(handles[h]) == 0) {
			pr_info("%s: DSP not yet calibrated. Calibration will"\
				" start\n", __func__);

			/* ensure no audio during special calibration */
			err = Tfa98xx_SetMute(
				handles[h], Tfa98xx_Mute_Digital);
			if (err != Tfa98xx_Error_Ok) {
				pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
					__func__, h);
				return -EINVAL;
			}
		} else
			pr_info("%s: DSP already calibrated\n", __func__);
	}

	/* load predefined, or fullmodel from data */
	for (h = 0; h < 2; h++)
		setSpeaker(1, &handles[h], speaker[h]);

	/* load the settings */
	for (h = 0; h < 2; h++)
		setConfig(1, &handles[h], config[h]);

	/* load a preset */
	for (h = 0; h < 2; h++)
		setPreset(1, &handles[h], preset[h]);

	/* set the equalizer */
	for (h = 0; h < 2; h++)
		setEQ(1, &handles[h], eq[h]);

	err = set_speaker_lr(handles, speaker_lr);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: set_speaker_lr failed\n", __func__);
		return err;
	}

	/* all settings loaded, signal the DSP to start calibration */
	for (h = 0; h < 2; h++) {
		err = Tfa98xx_SetConfigured(handles[h]);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SetConfigured failed h=%d\n",
				__func__, h);
			return -EINVAL;
		}
	}

	for (h = 0; h < 2; h++) {
		calibrateDone = 0;
		waitCalibration(handles[h], &calibrateDone);
		if (calibrateDone)
			Tfa98xx_DspGetCalibrationImpedance(handles[h], &re25);
		else {
			re25 = 0;
			err = Tfa98xx_Powerdown(handles[h], 1);
			pr_err("%s: Calibration failed, power down" \
				" and return h=%d\n", __func__, h);
			return -EINVAL;
		}
		pr_info("%s re25:%016llx\n", __func__, re25);

		err = Tfa98xx_SetMute(handles[h], Tfa98xx_Mute_Off);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
				__func__, h);
			return -EINVAL;
		}

	}

	return 0;
}

/* warm startup */
static int stereo_speaker_warm_on(
	bool param_change,
	struct tfa98xx_param_data *config[2],
	struct tfa98xx_param_data *preset[2],
	struct tfa98xx_param_data *eq[2])
{
	enum Tfa98xx_Error err = Tfa98xx_Error_Ok;
	unsigned char h;
	int ready;
	int timeout;

	pr_info("%s\n", __func__);
	for (h = 0; h < 2; h++) {
		err = Tfa98xx_Powerdown(handles[h], 0);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_Powerdown failed, h=%d\n",
				__func__, h);
	}

	/* NXP SL: Added checking DSP stable after power up, before unmute */
	/*  wait until the DSP subsystem hardware is ready
	 *    note that the DSP CPU is not running yet (RST=1)
	 * */
	for (h = 0; h < 2; h++) {
		ready = 0;
		timeout = 0;
		while (ready == 0) {
			/* are we ready? */
			err = Tfa98xx_DspSystemStable(handles[h], &ready);
			if (err != Tfa98xx_Error_Ok) {
				pr_err("%s: Tfa98xx_DspSystemStable failed %d\n",
					__func__, err);
				return -EINVAL;
			}
			msleep(20);
			timeout++;
			if (timeout > 50) {
				pr_info("%s timeout ready:%d\n", __func__,
					ready);
				break;
			}
		}
	}

	if (param_change) {
		/* load the settings */
		for (h = 0; h < 2; h++)
			setConfig(1, &handles[h], config[h]);

		/* load a preset */
		for (h = 0; h < 2; h++)
			setPreset(1, &handles[h], preset[h]);

		/* set the equalizer */
		for (h = 0; h < 2; h++)
			setEQ(1, &handles[h], eq[h]);

		err = set_speaker_lr(handles, speaker_lr);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: set_speaker_lr failed\n", __func__);
			return err;
		}
	}

	for (h = 0; h < 2; h++) {
		err = Tfa98xx_SetMute(handles[h], Tfa98xx_Mute_Off);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
				__func__, h);
	}

	return 0;
}

static void stereo_speaker_off(void)
{
	enum Tfa98xx_Error err;
	unsigned int h;
	unsigned short status = 0;
	int timeout;

	pr_info("%s\n", __func__);
	for (h = 0; h < 2; h++) {
		err = Tfa98xx_SetMute(handles[h], Tfa98xx_Mute_Amplifier);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
				__func__, h);
		msleep(33);

	/* NXP SL: Added checking if amplifier is still switching or not
		to avoid pop sound */
	/* now wait for the amplifier to turn off */
	err = Tfa98xx_ReadRegister16(
			handles[h], TFA98XX_STATUSREG, &status);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_ReadRegister16 failed, h=%d\n",
				__func__, h);
		timeout = 0;
		while ((status & TFA98XX_STATUSREG_SWS_MSK)
			== TFA98XX_STATUSREG_SWS_MSK) {
			err = Tfa98xx_ReadRegister16(handles[h],
				TFA98XX_STATUSREG, &status);
			if (err != Tfa98xx_Error_Ok)
				pr_err("%s: Tfa98xx_ReadRegister16 failed,"\
					" h=%d\n", __func__, h);
				timeout++;
			if (timeout > 50) {
				pr_info("%s timeout SWS checking:%d\n",
					__func__, status);
				break;
			}
		}

		err = Tfa98xx_Powerdown(handles[h], 1);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_Powerdown failed, h=%d\n",
				__func__, h);
	}
}

static int mono_speaker_on(
	int channel,
	struct tfa98xx_param_data *speaker,
	struct tfa98xx_param_data *config,
	struct tfa98xx_param_data *preset,
	struct tfa98xx_param_data *eq)
{
	enum Tfa98xx_Error err;
	FIXEDPT re25;
	int calibrateDone = 0;

	/* use the generic slave address for optimizations */

	coldStartup(handles[channel]);
	/*Set to calibration once*/
	/* Only needed for really first time calibration */
	setOtc(handles[channel], 1);

	/* Check if MTPEX bit is set for calibration once mode */
	if (checkMTPEX(handles[channel]) == 0) {
		pr_info("%s: DSP not yet calibrated. Calibration will start\n",
			__func__);

		/* ensure no audio during special calibration */
		err = Tfa98xx_SetMute(handles[channel], Tfa98xx_Mute_Digital);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_ReadRegister16 failed\n",
				__func__);
			return -EINVAL;
		}
	} else
		pr_info("%s: DSP already calibrated\n", __func__);

	/* load predefined, or fullmodel from file */
	setSpeaker(1, &handles[channel], speaker);
	/* load the settings */
	setConfig(1, &handles[channel], config);
	/* load a preset */
	setPreset(1, &handles[channel], preset);
	/* set the equalizer */
	setEQ(1, &handles[channel], eq);

	err = Tfa98xx_SelectChannel(handles[channel], Tfa98xx_Channel_L_R);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_SelectChannel failed\n", __func__);
		return -EINVAL;
	}

	/* all settings loaded, signal the DSP to start calibration */
	err = Tfa98xx_SetConfigured(handles[channel]);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_SetConfigured failed\n", __func__);
		return -EINVAL;
	}

	waitCalibration(handles[channel], &calibrateDone);
	if (calibrateDone)
		Tfa98xx_DspGetCalibrationImpedance(handles[channel], &re25);
	else {
		re25 = 0;
		err = Tfa98xx_Powerdown(handles[channel], 1);
		pr_err("%s: Tfa98xx_Powerdown failed, channel=%d\n",
			__func__, channel);
		return -EINVAL;
	}

	err = Tfa98xx_SetMute(handles[channel], Tfa98xx_Mute_Off);
	if (err != Tfa98xx_Error_Ok) {
		pr_err("%s: Tfa98xx_SetMute failed, channel=%d\n",
			__func__, channel);
		return -EINVAL;
	}

	return 0;
}

static int mono_speaker_warm_on(
	int channel,
	bool param_change,
	struct tfa98xx_param_data *config,
	struct tfa98xx_param_data *preset,
	struct tfa98xx_param_data *eq)
{
	enum Tfa98xx_Error err = Tfa98xx_Error_Ok;
	int ready;
	int timeout;

	pr_info("%s\n", __func__);
	err = Tfa98xx_Powerdown(handles[channel], 0);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_Powerdown failed\n", __func__);
		err = Tfa98xx_SetMute(
			handles[channel], Tfa98xx_Mute_Amplifier);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_SetMute failed\n", __func__);

	/*  wait until the DSP subsystem hardware is ready
	 *    note that the DSP CPU is not running yet (RST=1)
	 * */
	ready = 0;
	timeout = 0;
	while (ready == 0) {
		/* are we ready? */
		err = Tfa98xx_DspSystemStable(handles[channel], &ready);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_DspSystemStable failed %d\n",
				__func__, err);
			return -EINVAL;
		}
		msleep(20);
		timeout++;
		if (timeout > 50) {
			pr_info("%s timeout ready:%d\n", __func__, ready);
			break;
		}
	}

	if (param_change) {
		/* load the settings */
		setConfig(1, &handles[channel], config);
		/* load a preset */
		setPreset(1, &handles[channel], preset);
		/* set the equalizer */
		setEQ(1, &handles[channel], eq);

		err = Tfa98xx_SelectChannel(handles[channel],
						Tfa98xx_Channel_L_R);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_SelectChannel failed\n", __func__);
			return -EINVAL;
		}
	}

	err = Tfa98xx_SetMute(handles[channel], Tfa98xx_Mute_Off);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_SetMute failed\n", __func__);

	return 0;
}

static void mono_speaker_off(int channel)
{
	enum Tfa98xx_Error err;
	unsigned short status = 0;
	int timeout;

	pr_info("%s channel:%d\n", __func__, channel);

	err = Tfa98xx_SetMute(handles[channel], Tfa98xx_Mute_Amplifier);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
			__func__, channel);
		msleep(33);

	/* NXP SL: Added checking if amplifier is still switching or not
		to avoid pop sound */
	/* now wait for the amplifier to turn off */
	err = Tfa98xx_ReadRegister16(handles[channel], TFA98XX_STATUSREG,
		&status);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_ReadRegister16 failed, h=%d\n",
			__func__, channel);
	timeout = 0;
	while ((status & TFA98XX_STATUSREG_SWS_MSK)
		== TFA98XX_STATUSREG_SWS_MSK) {
		err = Tfa98xx_ReadRegister16(handles[channel],
			TFA98XX_STATUSREG, &status);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_ReadRegister16 failed, h=%d\n",
				__func__, channel);
		timeout++;
		if (timeout > 50) {
			pr_info("%s timeout SWS checking:%d\n",
				__func__, status);
			break;
		}
	}

	err = Tfa98xx_Powerdown(handles[channel], 1);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_Powerdown failed, h=%d\n",
			__func__, channel);
}

static int bypass_dsp_speaker_on(int channel)
{
	enum Tfa98xx_Error err = Tfa98xx_Error_Ok;
	pr_info("%s channel:%d\n", __func__, channel);

	err = Tfa98xx_Powerdown(handles[channel], 0);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_Powerdown failed\n", __func__);

	err = Tfa98xx_SetMute(handles[channel], Tfa98xx_Mute_Off);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_SetMute failed\n", __func__);

	err = tfa98xx_bypass_dsp(handles[channel]);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: tfa98xx_bypass_dsp failed\n", __func__);

	return 0;
}

static void bypass_dsp_speaker_off(int channel)
{
	enum Tfa98xx_Error err;

	err = tfa98xx_unbypass_dsp(handles[channel]);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: tfa98xx_unbypass_dsp failed\n", __func__);

	pr_info("%s channel:%d\n", __func__, channel);
	err = Tfa98xx_SetMute(handles[channel], Tfa98xx_Mute_Amplifier);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
			__func__, channel);

	err = Tfa98xx_Powerdown(handles[channel], 1);
	if (err != Tfa98xx_Error_Ok)
		pr_err("%s: Tfa98xx_Powerdown failed, h=%d\n",
			__func__, channel);
}

static int tfa98xx_enable(void)
{
	int ret = 0;
	enum Tfa98xx_Error err;
	unsigned short status1 = 0;
	unsigned short status2 = 0;
	bool param_change = false;
	struct tfa98xx_param_data *preset_ptr[2] = {NULL, NULL};
	struct tfa98xx_param_data *config_ptr[2] = {NULL, NULL};
	struct tfa98xx_param_data *speaker_ptr[2] = {NULL, NULL};
	struct tfa98xx_param_data *eq_ptr[2] = {NULL, NULL};

	if (speaker_type >= 0
		&& speaker_type <= SPEAKER_TYPE_VOICECALL
		&& speaker_channel >= 0
		&& speaker_channel <= SPEAKER_CHANNEL_BOTH
		&& speaker_lr >= 0
		&& speaker_lr <= SPEAKER_LR_SWAP)
		pr_info("%s type:%s channel:%s lr:%s\n",
			__func__,
			speaker_type_text[speaker_type],
			speaker_channel_text[speaker_channel],
			speaker_lr_text[speaker_lr]);
	else
		pr_info("%s type:%d channel:%d lr:%d\n",
			__func__, speaker_type, speaker_channel, speaker_lr);

	if (speaker_type_now != speaker_type
		|| speaker_channel_now != speaker_channel) {
		tfa98xx_disable();
		param_change = true;
	}
	speaker_ptr[TOP] = &speaker_data[AMP_TOP];
	speaker_ptr[BOTTOM] = &speaker_data[AMP_BOTTOM];

	if (speaker_type == SPEAKER_TYPE_VOICECALL) {
		config_ptr[TOP] = &config_data[AMP_RECEIVER];
		config_ptr[BOTTOM] = NULL;
		pr_info("config_ptr = AMP_RECEIVER");
	} else {
		config_ptr[TOP] = &config_data[AMP_TOP];
		config_ptr[BOTTOM] = &config_data[AMP_BOTTOM];
		pr_info("config_ptr = AMP_Normal");
	}

	if (speaker_type >= 0 && speaker_type < SPEAKER_TYPE_MAX) {
		preset_ptr[TOP] = preset_data_table[speaker_type][TOP];
		preset_ptr[BOTTOM] = preset_data_table[speaker_type][BOTTOM];
		eq_ptr[TOP] = eql_data_table[speaker_type][TOP];
		eq_ptr[BOTTOM] = eql_data_table[speaker_type][BOTTOM];
	} else {
		pr_err("%s: Invalid speaker type:%x", __func__, speaker_type);
		return -EINVAL;
	}

	if (speaker_bypass_dsp == SPEAKER_BYPASS_DSP_ON) {
		if (speaker_channel == SPEAKER_CHANNEL_TOP
		 || speaker_channel == SPEAKER_CHANNEL_BOTH) {
			err = bypass_dsp_speaker_on(TOP);
			if (err != Tfa98xx_Error_Ok) {
				pr_err("%s: bypass_speaker_on failed %d\n",
					__func__, err);
			}
		}
		if (speaker_channel == SPEAKER_CHANNEL_BOTTOM
		 || speaker_channel == SPEAKER_CHANNEL_BOTH) {
			err = bypass_dsp_speaker_on(BOTTOM);
			if (err != Tfa98xx_Error_Ok) {
				pr_err("%s: bypass_speaker_on failed %d\n",
					__func__, err);
			}
		}

	} else if (speaker_channel == SPEAKER_CHANNEL_TOP) {
		if (preset_ptr[TOP] == NULL) {
			pr_err("%s: Invalid speaker type:%x channel:%d",
				__func__, speaker_type, speaker_channel);
			return -EINVAL;
		}
		err = Tfa98xx_ReadRegister16(
			handles[TOP], TFA98XX_STATUSREG, &status1);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
				__func__, err);
		}
		if ((status1 & TFA98XX_STATUSREG_ACS_MSK) != 0) {
			pr_info("cold start %04x\n", status1);
			ret = mono_speaker_on(
				TOP,
				speaker_ptr[TOP],
				config_ptr[TOP],
				preset_ptr[TOP],
				eq_ptr[TOP]);
		} else {
			pr_info("warm start %04x param_change %d\n",
				status1, param_change);
			ret = mono_speaker_warm_on(
				TOP,
				param_change,
				config_ptr[TOP],
				preset_ptr[TOP],
				eq_ptr[TOP]);
		}

	} else if (speaker_channel == SPEAKER_CHANNEL_BOTTOM) {
		if (preset_ptr[BOTTOM] == NULL) {
			pr_err("%s: Invalid speaker type:%x channel:%d",
				__func__, speaker_type, speaker_channel);
			return -EINVAL;
		}
		err = Tfa98xx_ReadRegister16(
			handles[BOTTOM], TFA98XX_STATUSREG, &status1);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
				__func__, err);
		}
		if ((status1 & TFA98XX_STATUSREG_ACS_MSK) != 0) {
			pr_info("cold start %04x\n", status1);
			ret = mono_speaker_on(
				BOTTOM,
				speaker_ptr[BOTTOM],
				config_ptr[BOTTOM],
				preset_ptr[BOTTOM],
				eq_ptr[BOTTOM]);
		} else {
			pr_info("warm start %04x param_change %d\n",
				status1, param_change);
			ret = mono_speaker_warm_on(
				BOTTOM,
				param_change,
				config_ptr[BOTTOM],
				preset_ptr[BOTTOM],
				eq_ptr[BOTTOM]);
		}

	} else if (speaker_channel == SPEAKER_CHANNEL_BOTH) {
		if (preset_ptr[TOP] == NULL || preset_ptr[BOTTOM] == NULL) {
			pr_err("%s: Invalid speaker type:%x channel:%d",
				__func__, speaker_type, speaker_channel);
			return -EINVAL;
		}
		err = Tfa98xx_ReadRegister16(
			handles[TOP], TFA98XX_STATUSREG, &status1);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
				__func__, err);
		}
		err = Tfa98xx_ReadRegister16(
			handles[BOTTOM], TFA98XX_STATUSREG, &status2);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Tfa98xx_ReadRegister16 failed %d\n",
				__func__, err);
		}
		if ((status1 & TFA98XX_STATUSREG_ACS_MSK) != 0
			|| (status2 & TFA98XX_STATUSREG_ACS_MSK) != 0) {
			pr_info("cold start %04x %04x\n", status1, status2);
			ret = stereo_speaker_on(
				speaker_ptr,
				config_ptr,
				preset_ptr,
				eq_ptr);
		} else {
			if ((speaker_lr != speaker_lr_now) && !param_change) {
				pr_info("speaker LR change %d\n", speaker_lr);
				ret = set_speaker_lr(handles, speaker_lr);
			}
			pr_info("warm start %04x %04x param_change %d\n",
				status1, status2, param_change);
			ret = stereo_speaker_warm_on(
				param_change,
				config_ptr,
				preset_ptr,
				eq_ptr);
		}
	}
	speaker_type_now = speaker_type;
	speaker_channel_now = speaker_channel;
	speaker_lr_now = speaker_lr;
	speaker_bypass_dsp_now = speaker_bypass_dsp;
	speaker_synced_now = speaker_synced;

	return ret;
}

static int tfa98xx_disable(void)
{
	if (speaker_bypass_dsp_now == SPEAKER_BYPASS_DSP_ON) {
		if (speaker_channel_now == SPEAKER_CHANNEL_TOP
		 || speaker_channel_now == SPEAKER_CHANNEL_BOTH) {
			bypass_dsp_speaker_off(TOP);
		}
		if (speaker_channel_now == SPEAKER_CHANNEL_BOTTOM
		 || speaker_channel_now == SPEAKER_CHANNEL_BOTH) {
			bypass_dsp_speaker_off(BOTTOM);
		}
	} else {
	switch (speaker_channel_now) {
	case SPEAKER_CHANNEL_NONE:
		break;
	case SPEAKER_CHANNEL_TOP:
		mono_speaker_off(TOP);
		break;
	case SPEAKER_CHANNEL_BOTTOM:
		mono_speaker_off(BOTTOM);
		break;
	case SPEAKER_CHANNEL_BOTH:
		stereo_speaker_off();
		break;
	default:
		pr_err("%s: Invalid speaker type %x",
			__func__, speaker_channel_now);
		return -EINVAL;
	}
	}

	return 0;
}

/*
 * Bypass DSP handling
 */
static int tfa98xx_bypass_dsp(Tfa98xx_handle_t handleIn)
{
	enum Tfa98xx_Error err = Tfa98xx_Error_Other;
	unsigned short i2SRead = 0;
	unsigned short sysRead = 0;
	unsigned short sysCtrlRead = 0;
	unsigned short batProtRead = 0;


	if (handleIn == -1)
		return err;

	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_I2SREG, &i2SRead);
	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_I2S_SEL_REG, &sysRead);
	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_SYS_CTRL, &sysCtrlRead);
	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_BAT_PROT, &batProtRead);

	i2SRead &= ~(TFA98XX_I2SREG_CHSA_MSK); /* Set CHSA to bypass DSP */
	sysRead &= ~(TFA98XX_I2S_SEL_REG_DCFG_MSK);/* Set DCDC compensation to
							off */
	sysRead |= TFA98XX_I2S_SEL_REG_SPKR_MSK; /* Set impedance as 8ohm */
	sysCtrlRead &= ~(TFA98XX_SYS_CTRL_DCA_MSK);/* Set DCDC to follower
							mode */
	sysCtrlRead &= ~(TFA98XX_SYS_CTRL_CFE_MSK);/* Disable coolflux */

	batProtRead |= TFA989X_BAT_PROT_BSSBY_MSK;/* Set clipper bypassed */

	/* Set CHSA to bypass DSP */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_I2SREG, i2SRead);
	/* Set DCDC compensation to off and set impedance as 8ohm */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_I2S_SEL_REG, sysRead);
	/* Set DCDC to follower mode and disable coolflux  */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_SYS_CTRL, sysCtrlRead);
	/* Set bypass clipper battery protection */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_BAT_PROT, batProtRead);

	return err;
}

/*
 * Unbypassed the DSP
 */
static int tfa98xx_unbypass_dsp(Tfa98xx_handle_t handleIn)
{
	enum Tfa98xx_Error err = Tfa98xx_Error_Other;
	unsigned short i2SRead = 0;
	unsigned short sysRead = 0;
	unsigned short sysCtrlRead = 0;
	unsigned short batProtRead = 0;

	if (handleIn == -1)
		return err;

	/* basic settings for quickset */
	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_I2SREG, &i2SRead);
	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_I2S_SEL_REG, &sysRead);
	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_SYS_CTRL, &sysCtrlRead);
	err = Tfa98xx_ReadRegister16(handleIn, TFA98XX_BAT_PROT, &batProtRead);

	i2SRead |= TFA98XX_I2SREG_CHSA; /* Set CHSA to Unbypass DSP */
	sysRead |= TFA9890_I2S_SEL_REG_POR;/* Set I2S SEL REG to set
					DCDC compensation to default 100%*/
	sysRead &= ~(TFA98XX_I2S_SEL_REG_SPKR_MSK);/*Set impedance to be
							defined by DSP */
	sysCtrlRead |= TFA98XX_SYS_CTRL_DCA_MSK;/* Set DCDC to active mode*/
	sysCtrlRead |= TFA98XX_SYS_CTRL_CFE_MSK;/* Enable Coolflux */

	batProtRead &= ~(TFA989X_BAT_PROT_BSSBY_MSK);/*Set clipper active */

	/* Set CHSA to Unbypass DSP */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_I2SREG, i2SRead);
	/* Set I2S SEL REG to set DCDC compensation to default 100% and
	Set impedance to be defined by DSP */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_I2S_SEL_REG, sysRead);
	/* Set DCDC to active mode and enable Coolflux */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_SYS_CTRL, sysCtrlRead);
	/* Set bypass clipper battery protection */
	err = Tfa98xx_WriteRegister16(handleIn, TFA98XX_BAT_PROT, batProtRead);

	return err;
}

static void change_stereo_speaker_lr(int speaker_lr)
{
	enum Tfa98xx_Error err;
	unsigned int h;
	unsigned short status = 0;
	int timeout;

	pr_info("%s\n", __func__);
	for (h = 0; h < 2; h++) {
		err = Tfa98xx_SetMute(handles[h], Tfa98xx_Mute_Amplifier);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
				__func__, h);
		msleep(33);

		err = Tfa98xx_ReadRegister16(
			handles[h], TFA98XX_STATUSREG, &status);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_ReadRegister16 failed, h=%d\n",
				__func__, h);
		timeout = 0;
		while ((status & TFA98XX_STATUSREG_SWS_MSK)
			== TFA98XX_STATUSREG_SWS_MSK) {
			err = Tfa98xx_ReadRegister16(handles[h],
				TFA98XX_STATUSREG, &status);
			if (err != Tfa98xx_Error_Ok)
				pr_err("%s: Tfa98xx_ReadRegister16 failed,"\
					" h=%d\n", __func__, h);
				timeout++;
			if (timeout > 50) {
				pr_info("%s timeout SWS checking:%d\n",
					__func__, status);
				break;
			}
		}
	}

	set_speaker_lr(handles, speaker_lr);

	for (h = 0; h < 2; h++) {
		err = Tfa98xx_SetMute(handles[h], Tfa98xx_Mute_Off);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Tfa98xx_SetMute failed, h=%d\n",
				__func__, h);
	}
	speaker_lr_now = speaker_lr;
}

static int tfa98xx_codec_get_speaker_type_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s\n", __func__);
	ucontrol->value.integer.value[0] = speaker_type;
	return 0;
}

static int tfa98xx_codec_put_speaker_type_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s %ld\n", __func__, ucontrol->value.integer.value[0]);
	speaker_type = ucontrol->value.integer.value[0];

	if (speaker_amp_control_on && speaker_type >= 0) {
		set_speaker_amp_control_enable(false);
		set_speaker_amp_control_enable(true);
	}

	return 0;
}


static int tfa98xx_codec_get_speaker_channel_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s\n", __func__);
	ucontrol->value.integer.value[0] = speaker_channel;
	return 0;
}

static int tfa98xx_codec_put_speaker_channel_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s %ld\n", __func__, ucontrol->value.integer.value[0]);
	speaker_channel = ucontrol->value.integer.value[0];
	return 0;
}


static int tfa98xx_codec_get_speaker_lr_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s\n", __func__);
	ucontrol->value.integer.value[0] = speaker_lr;
	return 0;
}

static int tfa98xx_codec_put_speaker_lr_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s %ld\n", __func__, ucontrol->value.integer.value[0]);
	speaker_lr = ucontrol->value.integer.value[0];
	if (speaker_amp_control_on
		&& speaker_channel == BOTH
		&& speaker_channel_now == BOTH
		&& speaker_lr != speaker_lr_now) {
		change_stereo_speaker_lr(speaker_lr);
	}

	return 0;
}

static int tfa98xx_codec_get_speaker_bypass_dsp_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s\n", __func__);
	ucontrol->value.integer.value[0] = speaker_bypass_dsp;
	return 0;
}

static int tfa98xx_codec_put_speaker_bypass_dsp_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s %ld\n", __func__, ucontrol->value.integer.value[0]);
	speaker_bypass_dsp = ucontrol->value.integer.value[0];
	return 0;
}

static int tfa98xx_codec_get_speaker_synced_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s\n", __func__);
	ucontrol->value.integer.value[0] = speaker_synced;
	return 0;
}

static int tfa98xx_codec_put_speaker_synced_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s %ld\n", __func__, ucontrol->value.integer.value[0]);
	speaker_synced = ucontrol->value.integer.value[0];
	return 0;
}


static int tfa98xx_codec_get_speaker_amp_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int tfa98xx_codec_put_speaker_amp_control(
			struct snd_kcontrol *kcontrol,
			struct snd_ctl_elem_value *ucontrol)
{
	pr_info("%s %ld\n", __func__, ucontrol->value.integer.value[0]);
	if (ucontrol->value.integer.value[0] == 0)
		set_speaker_amp_control_enable(false);
	else
		set_speaker_amp_control_enable(true);

	return 0;
}


static const struct snd_kcontrol_new tfa98xx_snd_controls[] = {
	SOC_ENUM_EXT("TFA98XX_CHANNEL",
		speaker_channel_enum,
		tfa98xx_codec_get_speaker_channel_control,
		tfa98xx_codec_put_speaker_channel_control),

	SOC_ENUM_EXT("TFA98XX_BYPASS_DSP",
		speaker_bypass_dsp_enum,
		tfa98xx_codec_get_speaker_bypass_dsp_control,
		tfa98xx_codec_put_speaker_bypass_dsp_control),

	SOC_ENUM_EXT("TFA98XX_SYNCED",
		speaker_synced_enum,
		tfa98xx_codec_get_speaker_synced_control,
		tfa98xx_codec_put_speaker_synced_control),

	SOC_ENUM_EXT("TFA98XX_LR",
		speaker_lr_enum,
		tfa98xx_codec_get_speaker_lr_control,
		tfa98xx_codec_put_speaker_lr_control),

	SOC_ENUM_EXT("TFA98XX_PARAM_TYPE",
		speaker_type_enum,
		tfa98xx_codec_get_speaker_type_control,
		tfa98xx_codec_put_speaker_type_control),

	SOC_SINGLE_MULTI_EXT("TFA98XX_SPKR_AMP",
		SND_SOC_NOPM, SND_SOC_NOPM, 255, 0, 1,
		tfa98xx_codec_get_speaker_amp_control,
		tfa98xx_codec_put_speaker_amp_control),
};

static struct snd_soc_dai_driver tfa98xx_dais[] = {
	{
		.name = "tfa98xx-rx",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 1,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
};


static int tfa98xx_codec_probe(struct snd_soc_codec *codec)
{
	enum Tfa98xx_Error err;
	pr_info("%s\n", __func__);
	if (!handle_is_open(handles[TOP])) {
		err = Tfa98xx_Open(TOP_I2C_ADDRESS, &handles[TOP]);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Top Tfa98xx_Open failed\n", __func__);
			return -EINVAL;
		}
		pr_info("TOP channel open %d\n", handles[TOP]);
	}
	if (!handle_is_open(handles[BOTTOM])) {
		err = Tfa98xx_Open(BOTTOM_I2C_ADDRESS, &handles[BOTTOM]);
		if (err != Tfa98xx_Error_Ok) {
			pr_err("%s: Bottom Tfa98xx_Open failed\n", __func__);
			return -EINVAL;
		}
		pr_info("BOTTOM channel open %d\n", handles[BOTTOM]);
	}
	return 0;
}

static int tfa98xx_codec_remove(struct snd_soc_codec *codec)
{
	enum Tfa98xx_Error err;
	pr_info("%s\n", __func__);

	if (handle_is_open(handles[TOP])) {
		err = Tfa98xx_Close(handles[TOP]);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Top Tfa98xx_Close failed\n", __func__);
		handles[TOP] = -1;
	}
	if (handle_is_open(handles[BOTTOM])) {
		err = Tfa98xx_Close(handles[BOTTOM]);
		if (err != Tfa98xx_Error_Ok)
			pr_err("%s: Top Tfa98xx_Close failed\n", __func__);
		handles[TOP] = -1;
	}
	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_tfa98xx = {
	.probe	= tfa98xx_codec_probe,
	.remove	= tfa98xx_codec_remove,

	.controls = tfa98xx_snd_controls,
	.num_controls = ARRAY_SIZE(tfa98xx_snd_controls),
};


#ifdef CONFIG_PM
static int tfa98xx_suspend(struct device *dev)
{
	dev_dbg(dev, "%s: system suspend\n", __func__);
	return 0;
}

static int tfa98xx_resume(struct device *dev)
{
	dev_dbg(dev, "%s: system resume\n", __func__);
	return 0;
}

static const struct dev_pm_ops tfa98xx_pm_ops = {
	.suspend	= tfa98xx_suspend,
	.resume		= tfa98xx_resume,
};
#endif

static int __devinit tfa98xx_probe(struct platform_device *pdev)
{
	int ret = 0;
	pr_info("%s\n", __func__);
	if (pdev->dev.of_node)
		dev_set_name(&pdev->dev, "%s.%d", "tfa98xx-codec", 1);


	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_tfa98xx,
		 tfa98xx_dais, ARRAY_SIZE(tfa98xx_dais));
	return ret;
}
static int __devexit tfa98xx_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static const struct of_device_id tfa98xx_codec_dt_match[] = {
	{ .compatible = "nxp,tfa98xx-codec", },
	{}
};

static struct platform_driver tfa98xx_codec_driver = {
	.probe = tfa98xx_probe,
	.remove = tfa98xx_remove,
	.driver = {
		.name = "tfa98xx-codec",
		.owner = THIS_MODULE,
		.of_match_table = tfa98xx_codec_dt_match,
#ifdef CONFIG_PM
		.pm = &tfa98xx_pm_ops,
#endif
	},
};

static int tfa98xx_open(struct inode *inode, struct file *f)
{
	pr_debug("%s\n", __func__);
	return 0;
}

static long tfa98xx_ioctl(struct file *f,
		unsigned int cmd, unsigned long arg)
{
	struct tfa98xx_param_data param;
	struct tfa98xx_param_data *ptr = NULL;
	pr_debug("%s %08x\n", __func__, cmd);

	if (copy_from_user(&param, (void *)arg, sizeof(param))) {
		pr_err("%s: fail to copy memory handle!\n", __func__);
		return -EFAULT;
	}

	switch (cmd) {
	case TFA98XX_PATCH_PARAM:
		if (param.type < PATCH_MAX)
			ptr = &patch_data[param.type];
		else
			return -EFAULT;
		break;
	case TFA98XX_CONFIG_PARAM:
		if (param.type < AMP_MAX)
			ptr = &config_data[param.type];
		else
			return -EFAULT;
		break;

	case TFA98XX_SPEAKER_PARAM:
		if (param.type < AMP_MAX)
			ptr = &speaker_data[param.type];
		else
			return -EFAULT;
		break;

	case TFA98XX_PRESET_PARAM:
		if (param.type < TYPE_MAX)
			ptr = &preset_data[param.type];
		else
			return -EFAULT;
		break;

	case TFA98XX_EQ_PARAM:
		if (param.type < TYPE_MAX)
			ptr = &eql_data[param.type];
		else
			return -EFAULT;
		break;

	default:
		return -EFAULT;
	};

	if (ptr->data != NULL) {
		kfree(ptr->data);
		ptr->data = NULL;
		ptr->size = 0;
	}

	if (param.data != NULL) {
		if (param.size > PARAM_SIZE_MAX) {
			pr_err("%s: fail param size over\n", __func__);
			return -EFAULT;
		}
		ptr->data = kmalloc(param.size, GFP_KERNEL);
		ptr->size = param.size;
		if (copy_from_user(ptr->data, param.data, param.size)) {
			pr_err("%s: fail to copy memory handle!\n", __func__);
			return -EFAULT;
		}
	}

	return 0;
}

static int tfa98xx_release(struct inode *inode, struct file *f)
{
	pr_debug("%s\n", __func__);
	return 0;
}

static const struct file_operations tfa98xx_fops = {
	.owner = THIS_MODULE,
	.open = tfa98xx_open,
	.release = tfa98xx_release,
	.unlocked_ioctl = tfa98xx_ioctl,
};
struct miscdevice tfa98xx_misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "tfa98xx",
	.fops	= &tfa98xx_fops,
};

static int __init tfa98xx_init(void)
{
	pr_info("%s\n", __func__);
	misc_register(&tfa98xx_misc);
	return platform_driver_register(&tfa98xx_codec_driver);
}

static void __exit tfa98xx_exit(void)
{
	pr_info("%s\n", __func__);
}

module_init(tfa98xx_init);
module_exit(tfa98xx_exit);

