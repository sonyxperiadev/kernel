/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2009 - 2011  Broadcom Corporation                                                 */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2, available at                                           */ 
/*     http://www.broadcom.com/licenses/GPLv2.php   (the "GPL").                                */
/*                                                                                              */
/************************************************************************************************/
/**
*
*   @file   sysparm_shared.h
*
*   @brief  This file contains the audio system parameter functions.
*
****************************************************************************/

//**********************************************************
//**** NOTE ************************************************
//*  sysparm_shared.h in CP image and in Android AP image
//*  should be the SAME.
//*  Change either one of them, the other one should also
//*  be changed.
//**********************************************************

#ifndef _SYSPARM_SHARED_H_
#define _SYSPARM_SHARED_H_

/**
*   @defgroup   SystemParameterGroup	Audio System Parameters
*	@ingroup	SystemCnfgOverviewGroup
*
*   @brief      This group defines the software interface to the audio system parameters
****************************************************************************/

#define NUM_OF_VOICE_COEFF			35
#define NUM_OF_MIC_EQ_BIQUAD			12
#define NUM_OF_SPEAKER_EQ_BIQUAD			12
#define COEF_NUM_OF_EACH_EQ_BIQUAD		5
#define COEF_NUM_OF_EACH_GROUP_HW_SIDETONE		8
#define NUM_OF_GROUP_HW_SIDETONE		16
#define	NUM_OF_SIDETONE_FILTER_COEFF 10
#define	NUM_OF_ECHO_FAR_IN_FILTER_COEFF	10
#define NUM_OF_ECHO_NLP_CNG_FILTER	10
#define NUM_OF_BIQUAD_FILTER_COEF	10
#define NUM_OF_ECHO_STABLE_COEF_THRESH 2
#define NUM_OF_ECHO_NLP_GAIN		6
/** audio related defines */
#define AUDIO_MAGIC_SIZE		16
//#define	AUDIO_MODE_NUMBER		9	///< Up to 9 Audio Profiles (modes) after 213x1
//#define AUDIO_MODE_NUMBER_VOICE	(AUDIO_MODE_NUMBER*2)
#define	AUDIO_5BAND_EQ_MODE_NUMBER		6	///< Up to 6 Audio EQ Profiles (modes)
#define DSP_SUBBAND_COMPANDER_FIR_TAP 11 ///<11 taps
#define DSP_SUBBAND_NLP_FREQ_BINS_WB 24 ///<24 freq bins

#define	NUM_OMEGA_VOICE_BANDS	3
#define NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS	11
#define NUM_OMEGA_VOICE_PARMS (NUM_OMEGA_VOICE_BANDS*NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS)

typedef struct
{
	Int16			omega_voice_thres_dB[NUM_OMEGA_VOICE_BANDS];
	Int16			omega_voice_max_gain_dB[NUM_OMEGA_VOICE_BANDS];
	Int16			omega_voice_gain_step_up_dB256;
	Int16			omega_voice_gain_step_dn_dB256;
	Int16			omega_voice_max_gain_dB_scale_factor;
} OmegaVoice_Sysparm_t;

typedef struct
{
	UInt16	alpha_voice_enable;
	UInt16	alpha_voice_avc_target_dB;
	UInt16	alpha_voice_avc_max_gain_dB;
	UInt16	alpha_voice_avc_step_up_dB256;
	UInt16	alpha_voice_avc_step_down_dB256;
} AlphaVoice_Sysparm_t;

typedef struct
{
	UInt16	kappa_voice_enable;
	UInt16	kappa_voice_high_pitched_tone_target_dB;
	Int16	kappa_voice_fast_detector_loudness_target_dB;
	UInt16	kappa_voice_fast_detector_max_supp_dB;
	Int16	kappa_voice_slow_detector_loudness_target_dB;
	UInt16	kappa_voice_slow_detector_max_supp_dB;
} KappaVoice_Sysparm_t;



// Echo Canceller Comfort noise and NLP sysparms RON 4/28/2005
typedef struct
{
#ifdef DSP_FEATURE_SUBBAND_NLP
	UInt16          echo_subband_nlp_distortion_thresh[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 			echo_subband_nlp_ul_margin[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16          echo_subband_nlp_noise_margin[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16			echo_subband_nlp_erl_erle_adj_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16			echo_subband_nlp_ul_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16			echo_subband_nlp_noise_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB]; 
	UInt16			nlp_distortion_coupling[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16			echo_subband_nlp_dt_fine_control;
	UInt16			reverb_time_constant;			 //Used in reverb control
	Int16			reverb_level;					 //Used in reverb control
#endif
	Int16			echo_cng_bias;
	UInt16			echo_cng_enable;
	UInt16			echo_subband_nlp_enable;
	UInt16			echo_nlp_gain;
	UInt16			echo_nlp_min_ul_brk_in_thresh;
	UInt16			echo_nlp_dtalk_dl_agc_idx;// not used yet
	UInt16			echo_nlp_idle_ul_gain;// not used yet
	UInt16			echo_nlp_idle_dl_agc_idx;// not used yet
	UInt16			echo_nlp_ul_active_dl_agc_idx;// not used yet
	UInt16			echo_nlp_min_dl_pwr;
	UInt16			echo_nlp_min_ul_pwr;
	UInt16			echo_nlp_dl_loss_adjust_thld;
	UInt16			echo_nlp_ul_active_dl_loss_adjust_step;
	UInt16			echo_nlp_ul_idle_dl_loss_adjust_step;
	UInt16			echo_nlp_ul_active_dl_loss_min_val;
	UInt16			echo_nlp_ul_idle_dl_loss_min_val;
	UInt16		nlp_dl_lookahead_enable;
}EchoNlp_t;


#ifdef DSP_FEATURE_SUBBAND_INF_COMP
typedef	struct
{
	UInt16			t2lin;
	UInt16			g2t2;
	UInt16			g3t3;
	UInt16			g4t4;
	UInt16			alpha;
	UInt16			beta;
	UInt16          env;
	Int16			g1lin;
	Int16			step2;
	Int16			step3;
	Int16			step4;
} Subband_Compander_Band_t;
typedef	struct
{

	UInt16			g2t2;
	UInt16          env;
	Int16			g1lin;
} Smart_Compressor_t;
#endif

typedef struct
{
	EchoNlp_t echoNlp_parms;
	UInt8 audio_parm_magic[AUDIO_MAGIC_SIZE];
	UInt32 audio_param_address;
	UInt16 audio_channel;
	UInt16 speaker_pga;  //level, index
	UInt16 mic_pga;      //level, index
	UInt16 max_mic_gain;	
	UInt16 sidetone;
	UInt16 audio_dsp_sidetone;  //means dsp_sidetone_enable.
	UInt16	ty_mic_gain;
	UInt16	sidetone_tty;
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

	UInt16 voice_volume_max;  //in dB.
	UInt16 voice_volume_init; //in dB.
	Int32 dsp_voice_vol_tbl[NUM_OF_ENTRY_IN_DSP_VOICE_VOLUME_TABLE]; //in dB.

	UInt16 sidetone_output_gain;
	UInt16 sidetone_biquad_scale_factor;
	UInt16 sidetone_biquad_sys_gain;
	UInt16 sidetone_filter[ NUM_OF_SIDETONE_FILTER_COEFF ];

	UInt16	audio_hpf_enable;
	UInt16	audio_ul_hpf_coef_b;
	UInt16	audio_ul_hpf_coef_a;
	UInt16	audio_dl_hpf_coef_b;
	UInt16	audio_dl_hpf_coef_a;

	UInt16	bluetooth_filter_enable;

#ifdef DSP_FEATURE_SUBBAND_INF_COMP
	UInt16  dl_subband_compander_flag;
	Subband_Compander_Band_t fullband; /* fullband */
#endif

	UInt16	omega_voice_enable;
	UInt16	omega_voice_max_allowed_gain_spread_dB;
	OmegaVoice_Sysparm_t omega_voice_parms[NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS];
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

#if !defined(_ATHENA_)
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
	Int32 voice_speaker_eq[NUM_OF_SPEAKER_EQ_BIQUAD*COEF_NUM_OF_EACH_EQ_BIQUAD];	
	UInt16 voice_speaker_hpf_enable;
	UInt16 voice_speaker_hpf_cutoff_freq;
	
	UInt16 hw_sidetone_enable;
	UInt16 hw_sidetone_gain;
	Int32 hw_sidetone_eq[COEF_NUM_OF_EACH_GROUP_HW_SIDETONE*NUM_OF_GROUP_HW_SIDETONE];
#endif	

	UInt16 echo_path_change_detection_threshold;			//Used in echo path change detection
	Smart_Compressor_t smart_compressor;    				//smart compressor

	//Multi-Band Compressor parameters


	UInt16 ltpf_enable;
	/* Multi-Band Compressor parameters */
	UInt16 multiband_comp_g[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 multiband_comp_p[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 multiband_comp_dt_p[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 multiband_comp_alpha[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 multiband_comp_beta[DSP_SUBBAND_NLP_FREQ_BINS_WB];

	/* software eq parameters */
	UInt16 mic1_eq_enable;
	UInt16 mic2_eq_enable;
	UInt16 speaker_eq_enable;

	/* PMU ALC support */
	UInt16 alc_enable;
	UInt16 alc_vbat_ref;
	UInt16 alc_thld;
	UInt16 alc_ramp_up_ctrl;
	UInt16 alc_ramp_down_ctrl;
} SysAudioParm_t;

typedef struct
{
	UInt16 speaker_pga;  /*level, index*/
	UInt16 mic_pga;      /*level, index*/
	UInt16 voice_volume_max;  /*in dB.*/
	UInt16 voice_volume_init; /*in dB.*/

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

	/*in mB (0.01 dB).*/
	Int32 fm_radio_digital_vol[NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME];

	UInt16 mpm_niir_coeff;
	UInt16 mpm_gain_attack_step;
	UInt32 mpm_gain_attack_slope;
	UInt32 mpm_gain_decay_slope;
	UInt16 mpm_gain_attack_thre;
	UInt16 ihf_protection_enable;

	/* PMU ALC support */
	UInt16 alc_enable;
	UInt16 alc_vbat_ref;
	UInt16 alc_thld;
	UInt16 alc_ramp_up_ctrl;
	UInt16 alc_ramp_down_ctrl;
} SysMultimediaAudioParm_t;


typedef struct
{
	UInt32 treq_biquad_num;
	UInt32 treq_coef[NUM_OF_SPEAKER_EQ_BIQUAD*COEF_NUM_OF_EACH_EQ_BIQUAD*2];
	UInt32 treq_gain[NUM_OF_SPEAKER_EQ_BIQUAD*2];
	UInt32 treq_outbit_sel[2];
	UInt32 treq_output_gain;
#ifdef CONFIG_ARCH_JAVA
	Int16  mbc_cr[5];
	Int16  mbc_th_adj[5];
	Int16  fbc_cr[5];
	Int16  ale_gain[5];
	Int16  ale_th[5];
	Int16  ale_scale[5];
	Int16  hpf_fq[5];
	Int16  scale[16];
#endif
} SysIndMultimediaAudioParm_t;

#endif /*_SYSPARM_SHARED_H_*/
