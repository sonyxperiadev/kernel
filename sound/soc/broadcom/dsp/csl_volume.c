/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*   @file   csl_volume.c
*
*   @brief  This file contains DSP CSL API for audio volume control.
*
****************************************************************************/
#include "mobcom_types.h"
#include "shared.h"
#include "csl_volume.h"
#include "csl_dsp_common_util.h"
#include "audio_trace.h"

static Int16 ul_gain = 0;	/* in mB */
static Int16 dl_gain = 0;	/* in mB */

/*********************************************************************/
/**
*
*   CSL_InitGain initializes downlink and uplink gains.
*
*   @param    dlGain			(in)	downlink gain
*   @param    ulGain			(in)	uplink gain
*
**********************************************************************/
void CSL_InitGain(Int16 dlGain, Int16 ulGain)
{
	UInt16 i;

	for (i = 0; i < 5; i++) {
		cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[i] =
		    dlGain;
		cp_shared_mem->shared_echo_fast_NLP_gain[i] = ulGain;

	}

	aTrace(LOG_AUDIO_DSP,
	"CSL_InitGain: DL gain = %d, UL gain = %d \r\n", dlGain, ulGain);

}				/* CSL_InitGain */

/*********************************************************************/
/**
*
*   CSL_InitUnityGain initializes unity gains.
*
*
**********************************************************************/
void CSL_InitUnityGain(void)
{
	UInt16 i;

	for (i = 0; i < 5; i++) {
		cp_shared_mem->shared_usb_headset_gain_ul[i] = 16384;
		cp_shared_mem->shared_usb_headset_gain_dl[i] = 16384;
		cp_shared_mem->shared_gen_tone_gain_rec[i] = 16384;
		cp_shared_mem->shared_gen_tone_gain_ul[i] = 16384;
		cp_shared_mem->shared_btnb_gain_dl[i] = 16384;
		cp_shared_mem->shared_btnb_gain_ul[i] = 16384;
		cp_shared_mem->shared_second_amr_out_gain_dl[i] = 16384;
		cp_shared_mem->shared_second_amr_out_gain_ul[i] = 16384;
		cp_shared_mem->shared_speech_rec_gain_ul[i] = 16384;
		cp_shared_mem->shared_second_amr_out_gain_dl[i] = 16384;
		cp_shared_mem->shared_second_amr_out_gain_ul[i] = 16384;
		cp_shared_mem->shared_speech_rec_gain_ul[i] = 16384;

	}

	aTrace(LOG_AUDIO_DSP, "CSL_InitUnityGain: \r\n");

}				/* CSL_InitUnityGain */

/*********************************************************************/
/**
*
*   CSL_GetDLGain gets downlink gain.
*
*   @param    none
*   @return   int16  gain in millibels
*	(min = -3612 millibel, max = 3612 millibel)
*
**********************************************************************/
Int16 CSL_GetDLGain(void)
{
	return dl_gain;
}

/*********************************************************************/
/**
*
*   CSL_SetDLGain sets downlink gain.
*
*   @param    mBGain				(in)	gain in millibels
*					(min = -3612 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetDLGain(Int16 mBGain)
{
	Boolean result = FALSE;
	UInt32 scale;
	UInt16 gain;

	dl_gain = mBGain;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q9.6 format */
	scale >>= (GAIN_FRACTION_BITS_NUMBER - 6);

	gain = (UInt16) scale;

	/* limit gain to DSP range */
	if (gain > MAX_DL_GAIN) {
		gain = MAX_DL_GAIN;

		result = TRUE;

	}

	/* apply DL gain with an automatic 5ms ramp */
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[1] = gain;
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[2] = gain;
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[3] = gain;
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[4] = gain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetDLGain: gain = %dmB, linear scale = %d \r\n", mBGain, gain);

	return result;

}				/* CSL_SetDLGain */

/*********************************************************************/
/**
*
*   CSL_MuteDL mutes downlink.
*
*
**********************************************************************/
void CSL_MuteDL(void)
{
	/* mute DL */
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[1] = 0;
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[2] = 0;
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[3] = 0;
	cp_shared_mem->shared_echo_NLP_fast_downlink_volume_ctrl[4] = 0;

	aTrace(LOG_AUDIO_DSP, "CSL_MuteDL: \r\n");

}				/* CSL_MuteDL */

/*********************************************************************/
/**
*
*   CSL_GetULGain gets uplink gain.
*
*   @param    none
*   @return   int16  gain in millibels
*	(min = -3612 millibel, max = 3612 millibel)
*
**********************************************************************/
Int16 CSL_GetULGain(void)
{
	return ul_gain;
}

/*********************************************************************/
/**
*
*   CSL_SetULGain sets uplink gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -3612 millibel, max = 3612 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetULGain(Int16 mBGain)
{
	Boolean result = FALSE;
	UInt32 scale;
	UInt16 gain;

	ul_gain = mBGain;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q9.6 format */
	scale >>= (GAIN_FRACTION_BITS_NUMBER - 6);

	gain = (UInt16) scale;

	/* limit gain to DSP range */
	if (gain > MAX_UL_GAIN) {
		gain = MAX_UL_GAIN;

		result = TRUE;

	}

	/* apply UL gain with an automatic 5ms ramp */
	cp_shared_mem->shared_echo_fast_NLP_gain[1] = gain;
	cp_shared_mem->shared_echo_fast_NLP_gain[2] = gain;
	cp_shared_mem->shared_echo_fast_NLP_gain[3] = gain;
	cp_shared_mem->shared_echo_fast_NLP_gain[4] = gain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetULGain: gain = %dmB, linear scale = %d \r\n", mBGain, gain);

	return result;

}				/* CSL_SetULGain */

/*********************************************************************/
/**
*
*   CSL_MuteUL mutes uplink.
*
*
**********************************************************************/
void CSL_MuteUL(void)
{
	/* mute UL */
	cp_shared_mem->shared_echo_fast_NLP_gain[1] = 0;
	cp_shared_mem->shared_echo_fast_NLP_gain[2] = 0;
	cp_shared_mem->shared_echo_fast_NLP_gain[3] = 0;
	cp_shared_mem->shared_echo_fast_NLP_gain[4] = 0;

	aTrace(LOG_AUDIO_DSP, "CSL_MuteUL: \r\n");

}				/* CSL_MuteUL */

/*********************************************************************/
/**
*
*   CSL_SetULSpeechRecordGain sets uplink speech record gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -8428 millibel, max = 602 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetULSpeechRecordGain(Int16 mBGain)
{
	Boolean result = FALSE;
	UInt32 scale;
	UInt16 gain;
	UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale >>= (GAIN_FRACTION_BITS_NUMBER - 14);

	gain = (UInt16) scale;

	/* limit gain to DSP range */
	if (gain > MAX_UL_SPEECH_REC_GAIN) {
		gain = MAX_UL_SPEECH_REC_GAIN;

		result = TRUE;

	}

	/* apply UL speech record gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		cp_shared_mem->shared_speech_rec_gain_ul[i] = gain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetULSpeechRecordGain: gain = %dmB, linear scale = %d \r\n",
	mBGain, gain);

	return result;

}				/* CSL_SetULSpeechRecordGain */

/*********************************************************************/
/**
*
*   CSL_MuteULSpeechRecord mutes uplink speech record.
*
*
**********************************************************************/
void CSL_MuteULSpeechRecord(void)
{
	UInt16 i;

	/* mute UL speech record */
	for (i = 0; i < 5; i++)
		cp_shared_mem->shared_speech_rec_gain_ul[i] = 0;

	aTrace(LOG_AUDIO_DSP, "CSL_MuteULSpeechRecord: \r\n");

}				/* CSL_MuteULSpeechRecord */

/*********************************************************************/
/**
*
*   CSL_SetWBAMROutputGain sets WB-AMR output gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -3612 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetWBAMROutputGain(Int16 mBGain)
{
	Boolean result = FALSE;
	UInt32 scale;
	UInt16 gain;
	UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q9.6 format */
	scale >>= (GAIN_FRACTION_BITS_NUMBER - 6);

	gain = (UInt16) scale;

	/* limit gain to DSP range */
	if (gain > MAX_WBAMR_GAIN) {
		gain = MAX_WBAMR_GAIN;

		result = TRUE;

	}

	/* apply WB-AMR output gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		cp_shared_mem->shared_wb_mm_output_gain[i] = gain;


	aTrace(LOG_AUDIO_DSP,
	"CSL_SetWBAMROutputGain: gain = %dmB, linear scale = %d \r\n",
	mBGain, gain);

	return result;

}				/* CSL_SetWBAMROutputGain */

/*********************************************************************/
/**
*
*   CSL_MuteWBAMROutput mutes WB-AMR output.
*
*
**********************************************************************/
void CSL_MuteWBAMROutput(void)
{
	UInt16 i;

	/* mute WB-AMR output */
	for (i = 0; i < 5; i++)
		cp_shared_mem->shared_wb_mm_output_gain[i] = 0;


	aTrace(LOG_AUDIO_DSP, "CSL_MuteWBAMROutput: \r\n");

}				/* CSL_MuteWBAMROutput */

/*********************************************************************/
/**
*
*   CSL_SetSpeakerInputGain sets speaker input gain
*
*   @param    inputGain				(in)	input gain
*
**********************************************************************/
void CSL_SetSpeakerInputGain(UInt16 inputGain)
{
	/* set EC speaker input gain */
	cp_shared_mem->shared_echo_spkr_phone_input_gain = inputGain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetSpeakerInputGain: gain = %d \r\n", inputGain);

}				/* CSL_SetSpeakerInputGain */

/*********************************************************************/
/**
*
*   CSL_SetDigitalInputClipLevel sets digital input clip level
*
*   @param    clipLevel				(in)	clip level
*
**********************************************************************/
void CSL_SetDigitalInputClipLevel(UInt16 clipLevel)
{
	/* set digital input clip level */
	cp_shared_mem->shared_echo_digital_input_clip_level = clipLevel;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetDigitalInputClipLevel: clip level = %d \r\n", clipLevel);

}				/* CSL_SetDigitalInputClipLevel */

/*********************************************************************/
/**
*
*   CSL_SetUplinkClipLevel sets uplink clip level
*
*   @param    clipLevel				(in)	clip level
*
**********************************************************************/
void CSL_SetUplinkClipLevel(UInt16 clipLevel)
{
	/* set EC uplink clip level */
	cp_shared_mem->shared_UL_audio_clip_level = clipLevel;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetUplinkClipLevel: clip level = %d \r\n", clipLevel);

}				/* CSL_SetUplinkClipLevel */
