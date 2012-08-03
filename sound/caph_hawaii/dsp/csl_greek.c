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
*   @file   csl_greek.c
*
*   @brief  This file contains DSP CSL API for Greek voices.
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "audio_tuning.h"
#include "sysparm_shared.h"
#include "csl_greek.h"
#include "audio_trace.h"



/*********************************************************************/
/**
*
*   CSL_EnableOmegaVoice enables/disables Omega voice.
*
*   @param    param					(in)	OV enable
*
**********************************************************************/
void CSL_EnableOmegaVoice(UInt16 control)
{
	/* Enable/Disable Omega voice */
	cp_shared_mem->shared_omega_voice_mem.omega_voice_enable = control;

	aTrace(LOG_AUDIO_DSP,
	"CSL_EnableOmegaVoice: state = %d \r\n", control);

}

/*********************************************************************/
/**
*
*   CSL_SetOmegaVoiceParameters sets Omega Voice parameters.
*
*   @param    param					(in)	OV parameters
*
**********************************************************************/
void CSL_SetOmegaVoiceParameters(OmegaVoice_Sysparm_t *param)
{
	/* update Omega voice parameters */
	memcpy(&cp_shared_mem->shared_omega_voice_mem.omega_voice_thres_dB[0],
	       &param->omega_voice_thres_dB[0],
	       sizeof(Int16) * NUM_OMEGA_VOICE_BANDS);
	memcpy(&cp_shared_mem->shared_omega_voice_mem.
	       omega_voice_max_gain_dB[0], &param->omega_voice_max_gain_dB[0],
	       sizeof(Int16) * NUM_OMEGA_VOICE_BANDS);
	cp_shared_mem->shared_omega_voice_mem.omega_voice_gain_step_up_dB256 =
	    param->omega_voice_gain_step_up_dB256;
	cp_shared_mem->shared_omega_voice_mem.omega_voice_gain_step_dn_dB256 =
	    param->omega_voice_gain_step_dn_dB256;
	cp_shared_mem->shared_omega_voice_mem.
	    omega_voice_max_gain_dB_scale_factor =
	    param->omega_voice_max_gain_dB_scale_factor;

}

/*********************************************************************/
/**
*
*   CSL_SetOvGainSpread sets Omega Voice gain spread
*
*   @param    gainSpread	(in)	gain spread
*
**********************************************************************/
void CSL_SetOvGainSpread(UInt16 gainSpread)
{
	/* set gain spread */
	cp_shared_mem->shared_omega_voice_max_allowed_gain_spread_dB =
	    gainSpread;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetOvGainSpread: gain spread = %d \r\n", gainSpread);

}				/* CSL_SetOvGainSpread */

/*********************************************************************/
/**
*
*   CSL_ConfigUplinkNoiseSuppression configures Uplink Noise Suppressor.
*
*   @param    gainSlope				(in)	gain slope
*   @param    inputGain				(in)	gain applied at the input of
*				the Uplink Noise suppressor in linear Q9.6
*   @param    outputGain			(in)	gain applied at the output of
*				the Uplink Noise suppressor in linear Q9.6
*   @param    maxSuppression		(in)	maximum noise suppression in
*				dB Q10.5
*
**********************************************************************/
void CSL_ConfigUplinkNoiseSuppression(UInt16 gainSlope, UInt16 inputGain,
				      UInt16 outputGain, Int16 maxSuppression)
{
	UInt16 i;

	/* configure NS gain slope */
	cp_shared_mem->shared_ul_gain_slope = gainSlope;

	/* set noise suppressor input gain */
	cp_shared_mem->shared_ul_noise_supp_input_gain = inputGain;

	/* set noise suppressor output gain */
	cp_shared_mem->shared_ul_noise_supp_output_gain = outputGain;

	/* configure maximum noise suppression per sub-band */
	for (i = 0; i < 24; i++)
		cp_shared_mem->shared_ul_noise_max_supp_dB[i] = maxSuppression;


	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigUplinkNoiseSuppression: gain slope = %d, input gain = %d, "
	"output gain = %d, max suppression = %d \r\n", gainSlope, inputGain,
	outputGain, maxSuppression);

}

/*********************************************************************/
/**
*
*   CSL_SetDownlinkNoiseSuppressionGainSlope sets Downlink Noise
*	Suppressor gain slope
*
*   @param    gainSlope				(in)	gain slope
*
**********************************************************************/
void CSL_SetDownlinkNoiseSuppressionGainSlope(UInt16 gainSlope)
{
	/* configure NS gain slope */
	cp_shared_mem->shared_dl_gain_slope = gainSlope;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetDownlinkNoiseSuppressionGainSlope: gain slope = %d \r\n",
	gainSlope);

}

/*********************************************************************/
/**
*
*   CSL_ConfigDownlinkNoiseSuppression configures Downlink Noise Suppressor.
*
*   @param    outputGain			(in)	gain applied at the output of
*				the Downlink Noise suppressor in linear Q9.6
*   @param    maxSuppression		(in)	maximum noise suppression in
*				dB Q10.5
*
**********************************************************************/
void CSL_ConfigDownlinkNoiseSuppression(Int16 maxSuppression)
{
	UInt16 i;

	/* configure maximum noise suppression per sub-band */
	for (i = 0; i < 24; i++)
		cp_shared_mem->shared_dl_noise_max_supp_dB[i] = maxSuppression;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigDownlinkNoiseSuppression: max suppression = %d \r\n",
	maxSuppression);

}



/*********************************************************************/
/**
*
*   CSL_SetReverbTimeConstant sets Reverb time constant
*
*   @param    reverb_time_constant	(in)	reverb_time_constant
*
**********************************************************************/
void CSL_SetReverbTimeConstant(UInt16 reverb_time_constant)
{
	/* configure Reverb Time Constant */
	cp_shared_mem->shared_reverb_time_constant = reverb_time_constant;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetReverbTimeConstant: reverb time constant = %d \r\n",
	reverb_time_constant);

}

/*********************************************************************/
/**
*
*   CSL_SetReverbLevel sets Reverb level
*
*   @param    reverb_level	(in)	reverb_level
*
**********************************************************************/
void CSL_SetReverbLevel(Int16 reverb_level)
{
	/* configure Reverb Level */
	cp_shared_mem->shared_reverb_level = reverb_level << 8;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetReverbLevel: reverb level = %d \r\n", reverb_level);

}

/*********************************************************************/
/**
*
*   CSL_SetEchoPathChangeDetectionThreshold sets Detection Threshold
*
*   @param    detection_threshold	(in)	detection_threshold
*
**********************************************************************/
void CSL_SetEchoPathChangeDetectionThreshold(UInt16 detection_threshold)
{
	/* configure Echo path change detection threshold */
	cp_shared_mem->shared_detection_threshold = detection_threshold << 8;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetEchoPathChangeDetectionThreshold: detection threshold = %d \r\n",
	detection_threshold);

}

/*********************************************************************/
/**
*
*   CSL_SetSmartCompressorG1Lin sets G1Lin
*
*   @param    g1lin	(in)	g1lin
*
**********************************************************************/
void CSL_SetSmartCompressorG1Lin(UInt16 g1lin)
{
	/* configure smart compressor g1lin */
	cp_shared_mem->shared_smart_compressor.g1lin = g1lin;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetSmartCompressorG1Lin: g1lin = %d \r\n", g1lin);

}

/*********************************************************************/
/**
*
*   CSL_SetSmartCompressorG2T2 sets G2T2
*
*   @param    g2t2	(in)	g2t2
*
**********************************************************************/
void CSL_SetSmartCompressorG2T2(UInt16 g2t2)
{
	/* configure smart compressor g2t2 */
	cp_shared_mem->shared_smart_compressor.g2t2 = g2t2;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetSmartCompressorG2T2: g2t2 = %d \r\n", g2t2);

}


/*********************************************************************/
/**
*
*   CSL_ConfigSidetoneFilter configures sidetone filter
*
*   @param    enable				(in)	0/1 diable/enable
*   @param    sysGain				(in)	gain
*   @param    scaleFactor			(in)	scale factor
*   @param    outputGain			(in)	output gain
*
**********************************************************************/
void CSL_ConfigSidetoneFilter(UInt16 enable, UInt16 sysGain, UInt16 scaleFactor,
			      UInt16 outputGain)
{
	/* configure disetone filter */
	cp_shared_mem->shared_dsp_sidetone_enable = enable;
	cp_shared_mem->shared_sidetone_biquad_sys_gain = sysGain;
	cp_shared_mem->shared_sidetone_biquad_scale_factor = scaleFactor;
	cp_shared_mem->shared_sidetone_output_gain = outputGain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigSidetoneFilter: enable = %d, sys gain = %d, scale factor = %d, "
	"output gain = %d \r\n", enable, sysGain, scaleFactor, outputGain);

}				/* CSL_ConfigSidetoneFilter */

/*********************************************************************/
/**
*
*   CSL_ConfigECgain configures EC gains
*
*   @param    inputGain				(in)	input gain
*   @param    outputGain			(in)	output gain
*   @param    feedForwardGain		(in)	feed forward gain
*
**********************************************************************/
void CSL_ConfigECGain(UInt16 inputGain, UInt16 outputGain,
		      UInt16 feedForwardGain)
{
	/* configure EC gains */
	cp_shared_mem->shared_echo_cancel_input_gain = inputGain;
	cp_shared_mem->shared_echo_cancel_output_gain = outputGain;
	cp_shared_mem->shared_echo_cancel_feed_forward_gain = feedForwardGain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigECGain: input gain = %d, output gain = %d, "
	"feedforward gain = %d \r\n", inputGain, outputGain, feedForwardGain);

}				/* CSL_ConfigECGain */

/*********************************************************************/
/**
*
*   CSL_ConfigECMic2gain configures EC mic2 gains
*
*   @param    inputGain				(in)	input gain
*   @param    outputGain			(in)	output gain
*   @param    feedForwardGain		(in)	feed forward gain
*
**********************************************************************/
void CSL_ConfigECMic2Gain(UInt16 inputGain, UInt16 outputGain,
			  UInt16 feedForwardGain)
{
	/* configure EC Mic2 gains */
	cp_shared_mem->shared_echo_cancel_mic2_input_gain = inputGain;
	cp_shared_mem->shared_echo_cancel_mic2_output_gain = outputGain;
	cp_shared_mem->shared_echo_cancel_mic2_feed_forward_gain =
	    feedForwardGain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigECMic2Gain: input gain = %d, output gain = %d, "
	"feedforward gain = %d \r\n", inputGain, outputGain, feedForwardGain);

}				/* CSL_ConfigECMic2Gain */

/*********************************************************************/
/**
*
*   CSL_ConfigSidetoneExpander configures sidetone expander
*
*   @param    enable				(in)	0/1 - disable/enable
*   @param    alpha					(in)	alpha
*   @param    beta					(in)	beta
*   @param    b						(in)	b
*   @param    c						(in)	c
*   @param    c_div_b				(in)	c_div_b
*   @param    inv_b					(in)	inv_b
*
**********************************************************************/
void CSL_ConfigSidetoneExpander(UInt16 enable, UInt16 alpha, UInt16 beta,
				UInt16 b, UInt16 c, UInt16 c_div_b,
				UInt16 inv_b)
{
	/* configure sidetone expander */
	cp_shared_mem->shared_sidetone_expander_flag = enable;
	cp_shared_mem->sidetone_expander.shared_expander_alpha = alpha;
	cp_shared_mem->sidetone_expander.shared_expander_beta = beta;
	cp_shared_mem->sidetone_expander.shared_expander_b = b;
	cp_shared_mem->sidetone_expander.shared_expander_c = c;
	cp_shared_mem->sidetone_expander.shared_expander_c_div_b = c_div_b;
	cp_shared_mem->sidetone_expander.shared_expander_inv_b = inv_b;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigSidetoneExpander: enable = %d, alpha = %d, beta = %d, "
	"b = %d\r\n", enable, alpha, beta, b);

}				/* CSL_ConfigSidetoneExpander */

/*********************************************************************/
/**
*
*   CSL_ConfigDownlinkExpander configures downlink expander
*
*   @param    enable				(in)	0/1 - disable/enable
*   @param    alpha					(in)	alpha
*   @param    beta					(in)	beta
*   @param    b						(in)	b
*
**********************************************************************/
void CSL_ConfigDownlinkExpander(UInt16 enable, UInt16 alpha, UInt16 beta,
				UInt16 b)
{
	/* configure downlink expander */
	cp_shared_mem->shared_DL_compander_flag = enable;
	cp_shared_mem->expander_DL.shared_expander_alpha = alpha;
	cp_shared_mem->expander_DL.shared_expander_beta = beta;
	cp_shared_mem->expander_DL.shared_expander_b = b;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigDownlinkExpander: enable = %d, alpha = %d, beta = %d, "
	"b = %d\n", enable, alpha, beta, b);

}				/* CSL_ConfigDownlinkExpander */

/*********************************************************************/
/**
*
*   CSL_ConfigUplinkExpander configures uplink expander
*
*   @param    enable				(in)	0/1 - disable/enable
*   @param    alpha					(in)	alpha
*   @param    beta					(in)	beta
*   @param    b						(in)	b
*
**********************************************************************/
void CSL_ConfigUplinkExpander(UInt16 enable, UInt16 alpha, UInt16 beta,
			      UInt16 b)
{
	/* configure uplink expander */
	cp_shared_mem->shared_UL_compander_flag = enable;
	cp_shared_mem->expander_UL.shared_expander_alpha = alpha;
	cp_shared_mem->expander_UL.shared_expander_beta = beta;
	cp_shared_mem->expander_UL.shared_expander_b = b;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigUplinkExpander: enable = %d, alpha = %d, beta = %d, "
	"b = %d \r\n", enable, alpha, beta, b);

}				/* CSL_ConfigUplinkExpander */

/*********************************************************************/
/**
*
*   CSL_InitDownlinkCompressor initializes downlink compressor
*
*
**********************************************************************/
void CSL_InitDownlinkCompressor(void)
{
	/* initialize downlink compressor */
	cp_shared_mem->compress_DL.t2lin = 15849;
	cp_shared_mem->compress_DL.g2t2 = 46248;
	cp_shared_mem->compress_DL.g3t3 = 46248;
	cp_shared_mem->compress_DL.g4t4 = 46248;
	cp_shared_mem->compress_DL.alpha = 16384;
	cp_shared_mem->compress_DL.beta = 41;
	cp_shared_mem->compress_DL.env = 0;
	cp_shared_mem->compress_DL.g1lin = 4086;
	cp_shared_mem->compress_DL.step2 = 0;
	cp_shared_mem->compress_DL.step3 = 0;
	cp_shared_mem->compress_DL.step4 = 0;

	aTrace(LOG_AUDIO_DSP, "CSL_InitDownlinkCompressor: \r\n");

}				/* CSL_InitDownlinkCompressor */

/*********************************************************************/
/**
*
*   CSL_InitUplinkCompressor initializes uplink compressor
*
*
**********************************************************************/
void CSL_InitUplinkCompressor(void)
{
	/* initialize uplink compressor */
	cp_shared_mem->compress_UL.t2lin = 3981;
	cp_shared_mem->compress_UL.g2t2 = 43152;
	cp_shared_mem->compress_UL.g3t3 = 43152;
	cp_shared_mem->compress_UL.g4t4 = 43152;
	cp_shared_mem->compress_UL.alpha = 8192;
	cp_shared_mem->compress_UL.beta = 82;
	cp_shared_mem->compress_UL.env = 0;
	cp_shared_mem->compress_UL.g1lin = 8153;
	cp_shared_mem->compress_UL.step2 = 0;
	cp_shared_mem->compress_UL.step3 = 0;
	cp_shared_mem->compress_UL.step4 = 10922;

	aTrace(LOG_AUDIO_DSP, "CSL_InitUplinkCompressor: \r\n");

}				/* CSL_InitUplinkCompressor */

/*********************************************************************/
/**
*
*   CSL_InitSubbandNLP initializes subband NLP
*
*
**********************************************************************/
void CSL_InitSubbandNLP(void)
{
	/* initialize subband NLP */
	/* Threshold to exceed before adjusting ERL_ERLE downwards */
	cp_shared_mem->shared_subband_nlp_dl_distortion_adj_th = 1400;
	/* Threshold to exceed before faster NS adaptation logic kicks in */
	cp_shared_mem->shared_subband_nlp_fast_ns_adaptation_th = 0;
	cp_shared_mem->shared_subband_dl_energy_window_profile[0] = 0;

	/* Threshold to exceed before adjusting ERL_ERLE downwards */
	cp_shared_mem->shared_subband_nlp_dl_distortion_adj_th = 60;


	aTrace(LOG_AUDIO_DSP, "CSL_InitSubbandNLP: \r\n");

}				/* CSL_InitSubbandNLP */

/*********************************************************************/
/**
*
*   CSL_ConfigEC configures EC
*
*   @param    ecLength				(in)	EC length
*   @param    ERL_dB				(in)	ERL
*   @param    stepSizeGain			(in)	gain step size
*
**********************************************************************/
void CSL_ConfigEC(UInt16 ecLength, Int16 ERL_dB, Int16 stepSizeGain)
{
	/* configure EC */
	cp_shared_mem->shared_ecLen = ecLength;
	cp_shared_mem->shared_DT_TH_ERL_dB = ERL_dB;
	cp_shared_mem->shared_echo_step_size_gain = stepSizeGain;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigEC: length = %d, ERL_dB = %d, step size gain = %d \r\n",
	ecLength, ERL_dB, stepSizeGain);

}				/* CSL_ConfigEC */

/*********************************************************************/
/**
*
*   CSL_InitCNGFilterCoeff initializes EC CNG filter coefficients
*
*
**********************************************************************/
void CSL_InitCNGFilterCoeff(Int16 *coefPtr)
{
	UInt16 i;

	for (i = 0; i < 10; i++)
		cp_shared_mem->shared_echo_cng_filt_coefs[i] = coefPtr[i];

	aTrace(LOG_AUDIO_DSP, "CSL_InitCNGFilterCoeff: \r\n");

}				/* CSL_InitCNGFilterCoeff */

/*********************************************************************/
/**
*
*   CSL_SetNLPExpanderUpperLimit sets NLP expander upper limit
*
*   @param    limit					(in)	NLP expander upper limit
*
**********************************************************************/
void CSL_SetNLPExpanderUpperLimit(UInt16 limit)
{
	/* set NLP expander upper limit */
	cp_shared_mem->shared_echoNlp_expander_upper_limit = limit;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetNLPExpanderUpperLimit: NLP expander upper limit "
	"= %d \r\n", limit);

}				/* CSL_SetNLPExpanderUpperLimit */

/*********************************************************************/
/**
*
*   CSL_SetCNG_bias sets CNG_bias
*
*   @param    CNG_bias	(in)	CNG bias
*
**********************************************************************/
void CSL_SetCNG_bias(Int16 CNG_bias)
{
	/* set CNG bias */
	cp_shared_mem->shared_CNG_bias = CNG_bias;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetCNG_bias: CNG_bias = %d \r\n", CNG_bias);

}				/* CSL_SetCNG_bias */

/*********************************************************************/
/**
*
*   CSL_ConfigDualMicParms configures Dual Mic NS/NLP parameters
*
*   @param    dual_mic_ns_level	(in)	Dual mic NS level
*
**********************************************************************/
void CSL_ConfigDualMicParms(UInt16 dual_mic_ns_level)
{
	Int16 i;
	Int16 En_init[24] = { 378, 378, 378, 378, 315, 315, 378, 378, 378, 315,
	315, 315, 189, 189, 189, 189, 189, 189, 189, 189, 189, 189, 189, 189 };
	float ratio = dual_mic_ns_level/100.0;

	/* Set the slopes used in Dual Mic */
	cp_shared_mem->shared_dmic_slope_min = 80 * ratio;
	cp_shared_mem->shared_dmic_slope_max = 128 * ratio;
	cp_shared_mem->shared_dmic_R_max = -3 * 64 * (2.0 - ratio);

	/* Set DMIC NLP Params */
	/*shared_dmic_nlp_parms[0:15] are the thresholds to apply suppression */
	for (i = 0; i < 8; i++)
		cp_shared_mem->shared_dmic_nlp_parms[i] = 9 * 64;
	for (i = 8; i < 16; i++)
		cp_shared_mem->shared_dmic_nlp_parms[i] = 6 * 64;
	for (i = 16; i < 24; i++)
		cp_shared_mem->shared_dmic_nlp_parms[i] = 3 * 64;

	/* shared_dmic_nlp_parms[16:31] are the max suppression gains */
	for (i = 24; i < 48; i++)
		cp_shared_mem->shared_dmic_nlp_parms[i] = -2560 * ratio;

	/* Copy En_init to shared mem needed by DSP */
	memcpy(&cp_shared_mem->shared_dmic_init[128], &En_init[0],
		sizeof(Int16) * 24);

}				/* CSL_ConfigDualMicParms */


/*********************************************************************/
/**
*
*   CSL_ConfigDlCompressor configures DL Compressor
*
*
**********************************************************************/
void CSL_ConfigDlCompressor(void *config)
{

	memcpy(&cp_shared_mem->compress_DL, config,
	       sizeof(Shared_Compressor_t));

	aTrace(LOG_AUDIO_DSP, "CSL_ConfigDlCompressor: \r\n");

}				/* CSL_ConfigDlCompressor */

/*********************************************************************/
/**
*
*   CSL_EnableSubbandCompander controls subband compander
*
*   @param    control	(in)	0/1 - disable/enable
*
**********************************************************************/
void CSL_EnableSubbandCompander(UInt16 control)
{
	/* control subband compander */
	cp_shared_mem->shared_DL_subband_compander_flag = control;

	aTrace(LOG_AUDIO_DSP,
	"CSL_EnableSubbandCompander: state = %d \r\n", control);

}				/* CSL_EnableSubbandCompander */





/*********************************************************************/
/**
*
*   CSL_ConfigUlCompressor configures UL Compressor
*
*
**********************************************************************/
void CSL_ConfigUlCompressor(UInt16 t2lin, UInt16 g2t2, UInt16 g3t3, UInt16 g4t4,
			    UInt16 g1lin)
{
	/* clear UL compressor confuguration */
	memset(&cp_shared_mem->compress_UL, 0, sizeof(Shared_Compressor_t));

	cp_shared_mem->compress_UL.t2lin = t2lin;
	cp_shared_mem->compress_UL.g2t2 = g2t2;
	cp_shared_mem->compress_UL.g3t3 = g3t3;
	cp_shared_mem->compress_UL.g4t4 = g4t4;
	cp_shared_mem->compress_UL.g1lin = g1lin;

	aTrace(LOG_AUDIO_DSP, "CSL_ConfigUlCompressor: \r\n");

}				/* CSL_ConfigUlCompressor */

/*********************************************************************/
/**
*
*   CSL_ConfigBvUlCompressor configures Beta Voice UL Compressor
*
*
**********************************************************************/
void CSL_ConfigBvUlCompressor(void)
{
	memcpy(&cp_shared_mem->compress_ul_g2t2, &cp_shared_mem->compress_UL,
	       sizeof(Shared_Compressor_t));

	aTrace(LOG_AUDIO_DSP, "CSL_ConfigBvUlCompressor: \r\n");

}				/* CSL_ConfigBvUlCompressor */

/*********************************************************************/
/**
*
*   CSL_ConfigNLP configures NLP
*
*   @param    mode					(in) NLP mode 0/1 - FULLBAND/SUBBAND_MODE
*   @param    minPowerDl			(in)
*   @param    minPowerUl			(in)
*   @param    dtFineControl			(in)
*   @param    erleReset				(in)
*
**********************************************************************/
void CSL_ConfigNLP(UInt16 mode, UInt16 minPowerDl, UInt16 minPowerUl,
		   UInt16 dtFineControl, Int16 erleReset)
{

	cp_shared_mem->shared_nlp_mode = mode;
	cp_shared_mem->shared_echo_nlp_min_dl_pwr = minPowerDl;
	cp_shared_mem->shared_echo_nlp_min_ul_pwr = minPowerUl;
	cp_shared_mem->shared_subband_nlp_dt_fine_control = dtFineControl;
	cp_shared_mem->shared_trigger_erl_erle_reset = erleReset;

	aTrace(LOG_AUDIO_DSP, "CSL_ConfigNLP: "
	"mode = %d, DL min power = %d, UL min power = %d, DT fine control = %d,"
	"erle reset = %d \r\n", mode, minPowerDl, minPowerUl, dtFineControl,
	erleReset);

}				/* CSL_ConfigNLP */

/*********************************************************************/
/**
*
*   CSL_ConfigSubbandNlpDistortionThresh configures distortion thresholds
*
*
**********************************************************************/
void CSL_ConfigSubbandNlpDistortionThresh(void *config)
{
	memcpy(&cp_shared_mem->shared_subband_nlp_erl_erle_adj, config,
	       DSP_SUBBAND_NLP_FREQ_BINS_WB * sizeof(UInt16));

	memcpy(&cp_shared_mem->
	       shared_echoNlpParams_subband_nlp_distortion_thresh, config,
	       DSP_SUBBAND_NLP_FREQ_BINS_WB * sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP,
			"CSL_ConfigSubbandNlpDistortionThresh: \r\n");

}				/* CSL_ConfigSubbandNlpDistortionThresh */

/*********************************************************************/
/**
*
*   CSL_ConfigSubbandNlpUlMargin configures UL margin
*
*
**********************************************************************/
void CSL_ConfigSubbandNlpUlMargin(void *config)
{
	memcpy(&cp_shared_mem->shared_subband_nlp_UL_margin, config,
	       DSP_SUBBAND_NLP_FREQ_BINS_WB * sizeof(UInt16));

	memcpy(&cp_shared_mem->shared_echoNlpParams_subband_nlp_UL_margin,
	       config, DSP_SUBBAND_NLP_FREQ_BINS_WB * sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP,
			"CSL_ConfigSubbandNlpUlMargin: \r\n");

}				/* CSL_ConfigSubbandNlpUlMargin */

/*********************************************************************/
/**
*
*   CSL_ConfigSubbandNlpNoiseMargin configures noise margin
*
*
**********************************************************************/
void CSL_ConfigSubbandNlpNoiseMargin(void *config)
{
	memcpy(&cp_shared_mem->shared_subband_nlp_noise_margin, config,
	       DSP_SUBBAND_NLP_FREQ_BINS_WB * sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP,
			"CSL_ConfigSubbandNlpNoiseMargin: \r\n");

}				/* CSL_ConfigSubbandNlpNoiseMargin */

/*********************************************************************/
/**
*
*   CSL_ConfigAlfaVoice configures Alfa Voice
*
*   @param    enable				(in)
*   @param    avcTarget				(in)
*   @param    avcMaxGain			(in)
*   @param    avcStepUp				(in)
*   @param    avcStepDown			(in)
*
**********************************************************************/
void CSL_ConfigAlfaVoice(UInt16 enable, UInt16 avcTarget, UInt16 avcMaxGain,
			 UInt16 avcStepUp, UInt16 avcStepDown)
{
	cp_shared_mem->shared_alpha_voice_mem.alpha_voice_enable = enable;
	cp_shared_mem->shared_alpha_voice_mem.alpha_voice_avc_target_dB =
	    avcTarget;
	cp_shared_mem->shared_alpha_voice_mem.alpha_voice_avc_max_gain_dB =
	    avcMaxGain;
	cp_shared_mem->shared_alpha_voice_mem.alpha_voice_avc_step_up_dB256 =
	    avcStepUp;
	cp_shared_mem->shared_alpha_voice_mem.alpha_voice_avc_step_down_dB256 =
	    avcStepDown;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigAlfaVoice: enable = %d, avc target = %d, avc max gain = %d, "
	"avc step up = %d,avc step down = %d \r\n",
	enable, avcTarget, avcMaxGain, avcStepUp, avcStepDown);

}				/* CSL_ConfigAlfaVoice */

/*********************************************************************/
/**
*
*   CSL_ConfigKappaVoice configures Kappa Voice
*
*   @param    enable				(in)
*   @param    highPitchedToneTarget	(in)
*   @param    fastLoudnessTarget	(in)
*   @param    fastMaxSuppession		(in)
*   @param    slowLoudnessTarget	(in)
*   @param    slowMaxSuppession		(in)
*
**********************************************************************/
void CSL_ConfigKappaVoice(UInt16 enable, UInt16 highPitchedToneTarget,
			  Int16 fastLoudnessTarget, UInt16 fastMaxSuppession,
			  Int16 slowLoudnessTarget, UInt16 slowMaxSuppession)
{
	cp_shared_mem->shared_kappa_voice_mem.kappa_voice_enable = enable;
	cp_shared_mem->shared_kappa_voice_mem.
	    kappa_voice_high_pitched_tone_target_dB = highPitchedToneTarget;
	cp_shared_mem->shared_kappa_voice_mem.
	    kappa_voice_fast_detector_loudness_target_dB = fastLoudnessTarget;
	cp_shared_mem->shared_kappa_voice_mem.
	    kappa_voice_fast_detector_max_supp_dB = fastMaxSuppession;
	cp_shared_mem->shared_kappa_voice_mem.
	    kappa_voice_slow_detector_loudness_target_dB = slowLoudnessTarget;
	cp_shared_mem->shared_kappa_voice_mem.
	    kappa_voice_slow_detector_max_supp_dB = slowMaxSuppession;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigKappaVoice: enable = %d, high pitched tone target = %d, "
	"fast loudness target = %d, fast max suppession = %d, "
	"slow loudness target = %d, slow max suppession = %d \r\n",
	enable, highPitchedToneTarget, fastLoudnessTarget, fastMaxSuppession,
	slowLoudnessTarget, slowMaxSuppession);

}				/* CSL_ConfigKappaVoice */

/*********************************************************************/
/**
*
*   CSL_ConfigUlNoiseSuppressor configures UL max suppression
*
*   @param    maxSuppression	(in)	max suppression
*
**********************************************************************/
void CSL_ConfigUlNoiseSuppressor(Int16 maxSuppression)
{
	UInt16 i;

	/* set max suppression */
	for (i = 0; i < 24; i++)
		cp_shared_mem->shared_ul_noise_max_supp_dB[i] = maxSuppression;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigUlNoiseSuppressor: max suppression= %d \r\n",
	maxSuppression);

}				/* CSL_ConfigUlNoiseSuppressor */

/*********************************************************************/
/**
*
*   CSL_ConfigDlNoiseSuppressor configures DL max suppression
*
*   @param    maxSuppression	(in)	max suppression
*
**********************************************************************/
void CSL_ConfigDlNoiseSuppressor(Int16 maxSuppression)
{
	UInt16 i;

	/* set max suppression */
	for (i = 0; i < 24; i++)
		cp_shared_mem->shared_dl_noise_max_supp_dB[i] = maxSuppression;

	aTrace(LOG_AUDIO_DSP,
	"CSL_ConfigDlNoiseSuppressor: max suppression = %d \r\n",
	maxSuppression);

}				/* CSL_ConfigDlNoiseSuppressor */

/*********************************************************************/
/**
*
*   CSL_EnableLongTermPostFiltering controls Long Term Post Filtering
*
*   @param    control			(in)	0/1 - disable/enable
*
**********************************************************************/
void CSL_EnableLongTermPostFiltering(UInt16 control)
{
	/* control Long Term Post Filtering */
	cp_shared_mem->shared_ltpf_enable = control;

	aTrace(LOG_AUDIO_DSP,
	"CSL_EnableLongTermPostFiltering: state = %d \r\n", control);

}				/* CSL_EnableLongTermPostFiltering */
/*********************************************************************/
/**
*
*   CSL_SetEchoCouplingDelay sets echo coupling delay
*
*   @param    control			(in)	0-480
*
**********************************************************************/
void CSL_SetEchoCouplingDelay(UInt16 coupling_delay)
{
	/* Set echo coupling delay */
	cp_shared_mem->shared_echo_coupling_delay = coupling_delay;

	aTrace(LOG_AUDIO_DSP,
	"CSL_SetEchoCouplingDelay: state = %d \r\n", coupling_delay);


}	/* CSL_SetEchoCouplingDelay */


/*********************************************************************/
/**
*
*   CSL_EnableMic1Eq controls enable/disable of MIC1 EQ
*   @param    control			(in)	0/1 - disable/enable
*
**********************************************************************/
void CSL_EnableMic1Eq(UInt16 control)
{
	/* controls enable/disable of MIC1 EQ */
	cp_shared_mem->shared_mic1_eq_enable = control;


	aTrace(LOG_AUDIO_DSP,
	"CSL_EnableMic1Eq: state = %d \r\n", control);


}	/* CSL_EnableMic1Eq */


/*********************************************************************/
/**
*
*   CSL_EnableMic2Eq controls enable/disable of MIC2 EQ
*   @param    control			(in)	0/1 - disable/enable
*
**********************************************************************/
void CSL_EnableMic2Eq(UInt16 control)
{
	/* controls enable/disable of MIC2 EQ */
	cp_shared_mem->shared_mic2_eq_enable = control;

	aTrace(LOG_AUDIO_DSP,
	"CSL_EnableMic2Eq: state = %d \r\n", control);


} /* CSL_EnableMic2Eq  */


/*********************************************************************/
/**
*
*   CSL_EnableSpeakerEq controls enable/disable of Speaker EQ
*   @param    control			(in)	0/1 - disable/enable
*
**********************************************************************/
void CSL_EnableSpeakerEq(UInt16 control)
{
	/* controls enable/disable of MIC2 EQ */
	cp_shared_mem->shared_speaker_eq_enable = control;

	aTrace(LOG_AUDIO_DSP,
	"CSL_EnableSpeakerEq: state = %d \r\n", control);


} /* CSL_EnableSpeakerEq */


/*********************************************************************/
/**
*
*   CSL_EnableNlpDlLookAhead controls enable/disable of NLP DL Look Ahead
*   @param    control			(in)	0/1 - disable/enable
*
**********************************************************************/
void CSL_EnableNlpDlLookAhead(UInt16 control)
{
	/* controls enable/disable of NLP DL Look Ahead */
	cp_shared_mem->shared_nlp_dl_lookahead_enable = control;

	aTrace(LOG_AUDIO_DSP,
	"CSL_EnableNlpDlLookAhead: state = %d \r\n", control);


} /* CSL_EnableNlpDlLookAhead */



/*********************************************************************/
/**
*
*   CSL_SetSubbandNlpDistortionCoupling configures distotion coupling
*
*
**********************************************************************/
void CSL_SetSubbandNlpDistortionCoupling(void *config)
{
	memcpy(&cp_shared_mem->shared_nlp_distortion_coupling, config,
		DSP_SUBBAND_NLP_FREQ_BINS_WB*sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP, "CSL_SetSubbandNlpDistortionCoupling: \r\n");


} /* CSL_SetSubbandNlpDistortionCoupling */


/*********************************************************************/
/**
*
*   CSL_SetMultiBandCompGain configures MB compressor Gains
*
*
**********************************************************************/
void CSL_SetMultiBandCompGain(void *config)
{
	memcpy(&cp_shared_mem->shared_multiband_comp_g, config,
		DSP_SUBBAND_NLP_FREQ_BINS_WB*sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP, "CSL_SetMultiBandCompGain: \r\n");


} /* CSL_SetMultiBandCompGain */


/*********************************************************************/
/**
*
*   CSL_SetMultiBandCompPeakLimit configures MB compressor Peak limits
*
*
**********************************************************************/
void CSL_SetMultiBandCompPeakLimit(void *config)
{
	memcpy(&cp_shared_mem->shared_multiband_comp_p, config,
		DSP_SUBBAND_NLP_FREQ_BINS_WB*sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP, "CSL_SetMultiBandCompPeakLimit: \r\n");


} /* CSL_SetMultiBandCompPeakLimit */

/*********************************************************************/
/**
*
*   CSL_SetMultiBandCompDtPeakLimit configures MB compressor Peak limits
*	during DT
*
*
**********************************************************************/
void CSL_SetMultiBandCompDtPeakLimit(void *config)
{
	memcpy(&cp_shared_mem->shared_multiband_comp_dt_p, config,
		DSP_SUBBAND_NLP_FREQ_BINS_WB*sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP, "CSL_SetMultiBandCompDtPeakLimit: \r\n");


} /* CSL_SetMultiBandCompDtPeakLimit */


/*********************************************************************/
/**
*
*   CSL_SetMultiBandCompAlpha configures MB compressor attack for
*	each sub-band
*
*
**********************************************************************/
void CSL_SetMultiBandCompAlpha(void *config)
{
	memcpy(&cp_shared_mem->shared_multiband_comp_alpha, config,
		DSP_SUBBAND_NLP_FREQ_BINS_WB*sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP, "CSL_SetMultiBandCompAlpha: \r\n");


} /* CSL_SetMultiBandCompAlpha */


/*********************************************************************/
/**
*
*   CSL_SetMultiBandCompBeta configures MB compressor decay	for
*	each sub-band
*
*
**********************************************************************/
void CSL_SetMultiBandCompBeta(void *config)
{
	memcpy(&cp_shared_mem->shared_multiband_comp_beta, config,
		DSP_SUBBAND_NLP_FREQ_BINS_WB*sizeof(UInt16));

	aTrace(LOG_AUDIO_DSP, "CSL_SetMultiBandCompBeta: \r\n");


} /* CSL_SetMultiBandCompBeta */
