/*******************************************************************************
* Copyright 2010-2012 Broadcom Corporation.  All rights reserved.
*
*       @file   include/linux/broadcom/bcm_audio.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
*
*   @file   sysparm_shared.h
*
*   @brief  This file contains the audio system parameter functions.
*
****************************************************************************/

/**********************************************************
*
*  sysparm_shared.h in CP image and in Android AP image
*  should be the SAME.
*  Change either one of them, the other one should also
*  be changed.
**********************************************************/

#ifndef _SYSPARM_SHARED_H_
#define _SYSPARM_SHARED_H_

/**
*	@defgroup   SystemParameterGroup	Audio System Parameters
*	@ingroup    SystemCnfgOverviewGroup
*
*	@brief      This group defines the software interface to the
*		    audio system parameters
****************************************************************************/

#define NUM_OF_VOICE_COEFF			35
#define NUM_OF_MIC_EQ_BIQUAD			12
#define NUM_OF_SPEAKER_EQ_BIQUAD		12
#define COEF_NUM_OF_EACH_EQ_BIQUAD		5
#define COEF_NUM_OF_EACH_GROUP_HW_SIDETONE	8
#define NUM_OF_GROUP_HW_SIDETONE		16
#define	NUM_OF_SIDETONE_FILTER_COEFF 10
#define	NUM_OF_ECHO_FAR_IN_FILTER_COEFF		10
#define NUM_OF_ECHO_NLP_CNG_FILTER		10
#define NUM_OF_BIQUAD_FILTER_COEF		10
#define NUM_OF_ECHO_STABLE_COEF_THRESH		2
#define NUM_OF_ECHO_NLP_GAIN			6
/* audio related defines */
#define AUDIO_MAGIC_SIZE			16

/* #define	AUDIO_MODE_NUMBER		9 */
/* #define AUDIO_MODE_NUMBER_VOICE	(AUDIO_MODE_NUMBER*2) */
#define	AUDIO_5BAND_EQ_MODE_NUMBER		6	/* 6 Audio EQ modes */
#define DSP_SUBBAND_COMPANDER_FIR_TAP		11	/* 11 taps */
#define DSP_SUBBAND_NLP_FREQ_BINS_WB		24	/* 24 freq bins */

#define	NUM_OMEGA_VOICE_BANDS			3
#define NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS	11
#define NUM_OMEGA_VOICE_PARMS		\
(NUM_OMEGA_VOICE_BANDS*NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS)


struct _OmegaVoice_Sysparm_t {
	Int16		omega_voice_thres_dB[NUM_OMEGA_VOICE_BANDS];
	Int16		omega_voice_max_gain_dB[NUM_OMEGA_VOICE_BANDS];
	Int16		omega_voice_gain_step_up_dB256;
	Int16		omega_voice_gain_step_dn_dB256;
	Int16		omega_voice_max_gain_dB_scale_factor;
} ;
#define OmegaVoice_Sysparm_t struct _OmegaVoice_Sysparm_t

struct _AlphaVoice_Sysparm_t {
	UInt16	alpha_voice_enable;
	UInt16	alpha_voice_avc_target_dB;
	UInt16	alpha_voice_avc_max_gain_dB;
	UInt16	alpha_voice_avc_step_up_dB256;
	UInt16	alpha_voice_avc_step_down_dB256;
};
#define AlphaVoice_Sysparm_t struct _AlphaVoice_Sysparm_t

struct _KappaVoice_Sysparm_t {
	UInt16	kappa_voice_enable;
	UInt16	kappa_voice_high_pitched_tone_target_dB;
	Int16	kappa_voice_fast_detector_loudness_target_dB;
	UInt16	kappa_voice_fast_detector_max_supp_dB;
	Int16	kappa_voice_slow_detector_loudness_target_dB;
	UInt16	kappa_voice_slow_detector_max_supp_dB;
};
#define KappaVoice_Sysparm_t struct _KappaVoice_Sysparm_t


/* Echo Canceller Comfort noise and NLP sysparms */
struct _EchoNlp_t {
	Int16		echo_cng_bias;
	UInt16		echo_cng_enable;
	UInt16		echo_subband_nlp_enable;
	UInt16		echo_nlp_gain;
	UInt16		echo_nlp_min_ul_brk_in_thresh;
	UInt16		echo_nlp_dtalk_dl_agc_idx;
	UInt16		echo_nlp_idle_ul_gain;
	UInt16		echo_nlp_idle_dl_agc_idx;
	UInt16		echo_nlp_ul_active_dl_agc_idx;
	UInt16		echo_nlp_min_dl_pwr;
	UInt16		echo_nlp_min_ul_pwr;
	UInt16		echo_nlp_dl_loss_adjust_thld;
	UInt16		echo_nlp_ul_active_dl_loss_adjust_step;
	UInt16		echo_nlp_ul_idle_dl_loss_adjust_step;
	UInt16		echo_nlp_ul_active_dl_loss_min_val;
	UInt16		echo_nlp_ul_idle_dl_loss_min_val;
#ifdef DSP_FEATURE_SUBBAND_NLP
	UInt16		\
	echo_subband_nlp_distortion_thresh[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16		\
	echo_subband_nlp_ul_margin[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16		\
	echo_subband_nlp_noise_margin[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16		\
	echo_subband_nlp_erl_erle_adj_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16		\
	echo_subband_nlp_ul_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16		\
	echo_subband_nlp_noise_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16		echo_subband_nlp_dt_fine_control;
	UInt16		nlp_distortion_coupling; /* NLP distortion coupling */
	UInt16		reverb_time_constant;	/* reverb control */
	Int16		reverb_level;	/* reverb control */
#endif
};
#define EchoNlp_t struct _EchoNlp_t


#ifdef DSP_FEATURE_SUBBAND_INF_COMP
struct _Subband_Compander_Band_t {
	UInt16		t2lin;
	UInt16		g2t2;
	UInt16		g3t3;
	UInt16		g4t4;
	UInt16		alpha;
	UInt16		beta;
	UInt16          env;
	Int16		g1lin;
	Int16		step2;
	Int16		step3;
	Int16		step4;
};
#define Subband_Compander_Band_t struct _Subband_Compander_Band_t

struct _Smart_Compressor_t {

	UInt16		g2t2;
	UInt16          env;
	Int16		g1lin;
};
#define Smart_Compressor_t struct _Smart_Compressor_t

#endif

struct _SysAudioParm_t {
	UInt8 audio_parm_magic[AUDIO_MAGIC_SIZE];
	UInt32 audio_param_address;
	UInt16 audio_channel;
	UInt16 speaker_pga;  /* level, index */
	UInt16 mic_pga;      /* level, index */
	UInt16 max_mic_gain;
	UInt16 sidetone;
	UInt16 audio_dsp_sidetone;  /* means dsp_sidetone_enable. */
	UInt16	ty_mic_gain;
	UInt16	sidetone_tty;
	EchoNlp_t echoNlp_parms;
	UInt16 echo_cancelling_enable;
	UInt16 echo_dual_filter_mode;
	UInt16 echo_nlp_cng_filter[NUM_OF_ECHO_NLP_CNG_FILTER];
	UInt16	comp_filter_coef[NUM_OF_BIQUAD_FILTER_COEF];
	UInt16	comp_biquad_gain;
	UInt16 echo_adapt_norm_factor;
	UInt16 echo_cancel_dtd_hang;
	UInt16 echo_cancel_dtd_thresh;
	UInt16 echo_cancel_hsec_step;
	UInt16 echo_cancel_hsec_mfact;
	UInt16 echo_cancel_hsec_loop;
	UInt16 echo_cancel_max_hsec;
	UInt16 echo_cancel_input_gain;
	UInt16 echo_cancel_output_gain;
	UInt16 echo_cancel_frame_samples;
	UInt16 echo_cancel_update_delay;
	UInt16 echo_feed_forward_gain;
	UInt16 echo_digital_input_clip_level;
	UInt16 echo_spkr_phone_input_gain;
	UInt16 echo_coupling_delay;
	UInt16 echo_en_near_scale_factor;
	UInt16 echo_en_far_scale_factor;
	UInt16 echo_nlp_downlink_volume_ctrl;
	UInt16 ul_noise_suppression_enable;
	UInt16 ul_noise_supp_input_gain;
	UInt16 ul_noise_supp_output_gain;
	UInt16 dl_noise_suppression_enable;
	UInt16 ecLen;
	Int16 DT_TH_ERL_dB;
	UInt16 echo_step_size_gain;
	UInt16	compander_flag ;
	UInt16	expander_alpha;
	UInt16	expander_beta;
	UInt16	expander_upper_limit;
	UInt16	expander_flag_sidetone ;
	UInt16	expander_alpha_sidetone;
	UInt16	expander_beta_sidetone;
	UInt16	expander_b_sidetone;
	UInt16	expander_c_sidetone;
	UInt16	expander_c_div_b_sidetone;
	UInt16	expander_inv_b_sidetone;
	UInt16	compander_flag_ul ;
	UInt16	expander_alpha_ul;
	UInt16	expander_beta_ul;
	UInt16	expander_upper_limit_ul;
	UInt16	compressor_gain_ul;
	UInt16	compressor_alpha_ul;
	UInt16	compressor_beta_ul;
	UInt16	ul_audio_clip_level;

	UInt16	second_amr_out_gain;
	Int16	ul_noise_supp_max;
	Int16	dl_noise_supp_max;
	/* 2133A5, 2124A2,2152A4, 213x1/2153A3 and beyond*/
	UInt16	arm2speech_call_gain;
	UInt16 volume_step_size;
	UInt16 num_supported_volume_levels;

	UInt16 voice_volume_max;  /* in dB. */
	UInt16 voice_volume_init; /* in dB. */
	Int32 dsp_voice_vol_tbl[NUM_OF_ENTRY_IN_DSP_VOICE_VOLUME_TABLE]; /*dB */

	UInt16 sidetone_output_gain;
	UInt16 sidetone_biquad_scale_factor;
	UInt16 sidetone_biquad_sys_gain;
	UInt16 sidetone_filter[NUM_OF_SIDETONE_FILTER_COEFF];

	UInt16	audio_hpf_enable;
	UInt16	audio_ul_hpf_coef_b;
	UInt16	audio_ul_hpf_coef_a;
	UInt16	audio_dl_hpf_coef_b;
	UInt16	audio_dl_hpf_coef_a;

	UInt16	bluetooth_filter_enable;

#ifdef DSP_FEATURE_SUBBAND_INF_COMP
	UInt16  dl_subband_compander_flag;
	Subband_Compander_Band_t fir1;    /* midband */
	Subband_Compander_Band_t fir2;    /* lowband */
	Subband_Compander_Band_t stream3; /* highband */
	Subband_Compander_Band_t fullband; /* fullband */
	Int16 compress_coef_fir1[DSP_SUBBAND_COMPANDER_FIR_TAP];
	Int16 compress_coef_fir2[DSP_SUBBAND_COMPANDER_FIR_TAP];
#endif

	UInt16	omega_voice_enable;
	UInt16	omega_voice_max_allowed_gain_spread_dB;
	OmegaVoice_Sysparm_t \
	omega_voice_parms[NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS];
	AlphaVoice_Sysparm_t alpha_voice_parms;
	KappaVoice_Sysparm_t kappa_voice_parms;

	UInt16 dual_mic_enable;
	UInt16 dual_mic_anc_enable;
	UInt16 dual_mic_nlp_enable;
	UInt16 dual_mic_pre_wnr_enable;
	UInt16 dual_mic_post_wnr_enable;
	UInt16 dual_mic_ns_level;

	UInt16 echo_cancel_mic2_input_gain;
	UInt16 echo_cancel_mic2_output_gain;
	UInt16 echo_mic2_feed_forward_gain;

#if defined(_RHEA_)
	UInt16 amic_dga_coarse_gain;
	UInt16 amic_dga_fine_gain;
	UInt16 dmic1_dga_coarse_gain;
	UInt16 dmic1_dga_fine_gain;
	UInt16 dmic2_dga_coarse_gain;
	UInt16 dmic2_dga_fine_gain;
	UInt16 dmic3_dga_coarse_gain;
	UInt16 dmic3_dga_fine_gain;
	UInt16 dmic4_dga_coarse_gain;
	UInt16 dmic4_dga_fine_gain;

	UInt16 srcmixer_input_gain_l;
	UInt16 srcmixer_output_coarse_gain_l;
	UInt16 srcmixer_output_fine_gain_l;
	UInt16 srcmixer_input_gain_r;
	UInt16 srcmixer_output_coarse_gain_r;
	UInt16 srcmixer_output_fine_gain_r;

	UInt16 ext_speaker_high_gain_mode_enable;
	UInt16 ext_speaker_pga_l;
	UInt16 ext_speaker_pga_r;

	UInt16 voice_mic1_biquad_num;
	Int16 voice_mic1_scale_input[NUM_OF_MIC_EQ_BIQUAD];
	Int16 voice_mic1_scale_output;
	Int32 voice_mic1_eq[NUM_OF_MIC_EQ_BIQUAD*COEF_NUM_OF_EACH_EQ_BIQUAD];
	UInt16 voice_mic1_hpf_enable;
	UInt16 voice_mic1_hpf_cutoff_freq;

	UInt16 voice_mic2_biquad_num;
	Int16 voice_mic2_scale_input[NUM_OF_MIC_EQ_BIQUAD];
	Int16 voice_mic2_scale_output;
	Int32 voice_mic2_eq[NUM_OF_MIC_EQ_BIQUAD*COEF_NUM_OF_EACH_EQ_BIQUAD];
	UInt16 voice_mic2_hpf_enable;
	UInt16 voice_mic2_hpf_cutoff_freq;

	UInt16 voice_speaker_biquad_num;
	Int16 voice_speaker_scale_input[NUM_OF_SPEAKER_EQ_BIQUAD];
	Int16 voice_speaker_scale_output;
	Int32 voice_speaker_eq[NUM_OF_SPEAKER_EQ_BIQUAD * \
COEF_NUM_OF_EACH_EQ_BIQUAD];
	UInt16 voice_speaker_hpf_enable;
	UInt16 voice_speaker_hpf_cutoff_freq;

	UInt16 hw_sidetone_enable;
	UInt16 hw_sidetone_gain;
	Int32 hw_sidetone_eq[COEF_NUM_OF_EACH_GROUP_HW_SIDETONE * \
NUM_OF_GROUP_HW_SIDETONE];
#endif

	UInt16 echo_path_change_detection_threshold; /* echo path change det */
	Smart_Compressor_t smart_compressor;    /*smart compressor */

	/* Multi-Band Compressor parameters */
	UInt16 multiband_comp_g_low;
	UInt16 multiband_comp_g_mid;
	UInt16 multiband_comp_g_high;

	UInt16 multiband_comp_p_low;
	UInt16 multiband_comp_p_mid;
	UInt16 multiband_comp_p_high;

	UInt16 multiband_comp_alpha_low;
	UInt16 multiband_comp_alpha_mid;
	UInt16 multiband_comp_alpha_high;

	UInt16 multiband_comp_beta_low;
	UInt16 multiband_comp_beta_mid;
	UInt16 multiband_comp_beta_high;
	Int32  \
	fm_radio_digital_vol[NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME]; /* mB */
	UInt16 ltpf_enable;

};
#define SysAudioParm_t struct _SysAudioParm_t


struct _SysIndMultimediaAudioParm_t {
	UInt32 treq_biquad_num;
	UInt32 treq_coef[12*10];
};
#define SysIndMultimediaAudioParm_t struct _SysIndMultimediaAudioParm_t

#endif /* _SYSPARM_SHARED_H_ */
