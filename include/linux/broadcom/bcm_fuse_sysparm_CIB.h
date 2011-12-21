/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcm_fuse_sysparm_CIB.h
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

#ifndef _BCM_FUSE_SYSPARM_CIB_H_
#define _BCM_FUSE_SYSPARM_CIB_H_

#include "audio_consts.h"  //to get the two constants.

#ifndef _RHEA_
#define _RHEA_
#endif

#define SYSPARM_INDEX_READY_INDICATOR   0x5059504D	
#define MAX_SYSPARM_NAME_SIZE   128

#define   DSP_FEATURE_AHS_SID_UPDATE_BEC_CHECK	/* Additional BEC check to determine SID updaet frame in Rxqual_s calculation */
#ifdef UNDER_LINUX

#define   DSP_FEATURE_SUBBAND_NLP	/* Enable the compilation of ARM code specific to the subband_nlp feature in the DSP */
#define   DSP_FEATURE_SUBBAND_INF_COMP	/* Enable Infinite compression subband compressor */
#define   DSP_FEATURE_SUBBAND_INF_COMP_UL	/* Enable Infinite compression subband compressor sysparm/sheredmem init; not all the chips have ul and dl inf comp */
#define   DSP_FEATURE_EC_DYN_RNG_ENHANCEMENT	/* Enable EC dynamic range enhancements in the DSP */
#define   DSP_FEATURE_SUBBAND_NLP_MARGIN_VECTOR	/* Change single variable subband_nlp_UL_margin and subband_nlp_noise_margin thresholds into vector */
#define   DSP_FEATURE_CLASS_33	/* Enable class 33 feature in the DSP */
#define   DSP_FEATURE_NEW_FACCH_MUTING
#define   DSP_FEATURE_NULLPAGE_AUTOTRACK
#define   DSP_FEATURE_BT_PCM
#define   DSP_FEATURE_AFC_FB
#define   DSP_FEATURE_BB_RX_ADV	/* DSP BB advanced RX feature */
#define   DSP_FEATURE_STACK_DEPTH_CHECKING

#define   DSP_FEATURE_NULLPAGE_AUTOTRACK
#define   DSP_FEATURE_GAIN_DL_UL_NON_JUNO
#define   DSP_FEATURE_USB_HEADSET_GAIN
#define   DSP_FEATURE_AAC_LC_ENCODE_MIC_INPUT
#define   DSP_FEATURE_OTD_SNR_CHECK	/* OTD reported in SCH is updated when passing SNR threshold. */
#define   DSP_FEATURE_NORX0	/* Disable 1st RX slot so the previous frame can increase by one slot for search */
#define   DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE
#define   DSP_FEATURE_OMEGA_VOICE
#define   DSP_FEATURE_FR_MUTE_FRAME
#include <linux/broadcom/chip_version.h>

#endif // UNDER_LINUX



#define GPIO_INIT_FIELD_NUM 5
#define GPIO_INIT_REC_NUM 64
#define IMEI_SIZE					9

#define NUM_OF_FREQ_OFFSETS			8
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


/** Number of byte of Terminal Profile data defined in Sysparm. If the number of bytes is larger
 * than the "MAX_PROFILE_ARRAY_SIZE" defined in USIMAP/SIMAP, the extra bytes are truncated
 * and are not sent to the USIM/SIM.
 */
#define MAX_TERMINAL_PROFILE_ARRAY_SIZE  30

#if defined(RF_DESENSED)
#define DESENSE_TABLE_ROW_SIZE 5
#define DESENSE_TABLE_COLUMN_SIZE 5
#endif

/** TX FREQ SECTION PER BAND */
#define N_FREQ_SECTIONS				8

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

typedef struct
{
  char* name;
  void* ptr;
  unsigned int size;
  unsigned int flag;
}SysparmIndex_t;

// Echo Canceller Comfort noise and NLP sysparms RON 4/28/2005
typedef struct
{
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
#ifdef DSP_FEATURE_SUBBAND_NLP
	UInt16          echo_subband_nlp_distortion_thresh[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 			echo_subband_nlp_ul_margin[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16          echo_subband_nlp_noise_margin[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16			echo_subband_nlp_erl_erle_adj_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16			echo_subband_nlp_ul_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16			echo_subband_nlp_noise_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB]; 
	UInt16			echo_subband_nlp_dt_fine_control;
	UInt16			nlp_distortion_coupling;		 //Used in NLP distortion coupling estimate
	UInt16			reverb_time_constant;			 //Used in reverb control
	Int16			reverb_level;					 //Used in reverb control
#endif
} EchoNlp_t;
typedef struct
{
	UInt16			audio_agc_enable;	///< Enable/Disable uplink and downlink audio agc
	UInt16			max_thresh;
	UInt16			hi_thresh;
	UInt16			low_thresh;
	UInt16			decay;
	UInt16			max_idx;
	UInt16			min_idx;
	UInt16			step_down;
	UInt16			max_step_down;
	UInt16			step_up;
} AudioAGC_t;

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
	Subband_Compander_Band_t fir1;    //midband
	Subband_Compander_Band_t fir2;    //lowband
	Subband_Compander_Band_t stream3; //highband
	Subband_Compander_Band_t fullband; //fullband
	Int16 compress_coef_fir1[DSP_SUBBAND_COMPANDER_FIR_TAP];
	Int16 compress_coef_fir2[DSP_SUBBAND_COMPANDER_FIR_TAP];
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
	Int32 voice_speaker_eq[NUM_OF_SPEAKER_EQ_BIQUAD*COEF_NUM_OF_EACH_EQ_BIQUAD];	
	UInt16 voice_speaker_hpf_enable;
	UInt16 voice_speaker_hpf_cutoff_freq;
	
#if 0
	UInt16 music_speaker_biquad_num;
	Int16 music_speaker_scale_input[NUM_OF_SPEAKER_EQ_BIQUAD];	
	Int32 music_speaker_scale_output;	
	Int32 music_speaker_eq[NUM_OF_SPEAKER_EQ_BIQUAD*COEF_NUM_OF_EACH_EQ_BIQUAD];
#endif	
	UInt16 hw_sidetone_enable;
	UInt16 hw_sidetone_gain;
	Int32 hw_sidetone_eq[COEF_NUM_OF_EACH_GROUP_HW_SIDETONE*NUM_OF_GROUP_HW_SIDETONE];
#endif	

	UInt16 echo_path_change_detection_threshold;				//Used in echo path change detection
	Smart_Compressor_t smart_compressor;    				//smart compressor
} SysAudioParm_t;

typedef struct
{
	UInt32 treq_biquad_num;
	UInt32 treq_coef[12*10];	
	Int32 fm_radio_digital_vol[NUM_OF_ENTRY_IN_FM_RADIO_DIGITAL_VOLUME]; //in mB (0.01 dB).
} SysIndMultimediaAudioParm_t;

UInt16 SYSPARM_GetLogFormat(void);
SysAudioParm_t* APSYSPARM_GetAudioParmAccessPtr(void);
SysIndMultimediaAudioParm_t * APSYSPARM_GetMultimediaAudioParmAccessPtr(void);

UInt16 SYSPARM_GetDefault4p2VoltReading(void);
UInt16 SYSPARM_GetActual4p2VoltReading(void);
UInt16 SYSPARM_GetBattLowThresh(void);
UInt16 SYSPARM_GetActualLowVoltReading(void);

UInt8 *SYSPARM_GetGPIO_Default_Value(UInt8 gpio_index);

/* retrieves IMEI string from sysparms only (doesn't check MS database */
/* value, as is done in CP sysparm.c) */
Boolean SYSPARM_GetImeiStr(UInt8 *inImeiStrPtr);

#endif /* _BCM_FUSE_SYSPARM_CIB_H_ */
