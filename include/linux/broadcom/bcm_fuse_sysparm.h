/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/bcm_fuse_sysparm.h
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

#ifndef _BCM_FUSE_SYSPARM_H_
#define _BCM_FUSE_SYSPARM_H_

#include <linux/broadcom/chip_version.h>

#define PARM_IND_BASE           0x80080000	/* 192k independent sysparm */
#define PARM_IND_SIZE           0x00030000
#define PARM_DEP_BASE           0x800B0000	/* 128k dependent sysparm */
#define PARM_DEP_SIZE           0x00020000

#define SYSPARM_INDEX_READY_INDICATOR   0x5059504D	/* SYSP */
#define MAX_SYSPARM_NAME_SIZE   128

/** Number of byte of Terminal Profile data defined in Sysparm. If the number of bytes is larger
 * than the "MAX_PROFILE_ARRAY_SIZE" defined in USIMAP/SIMAP, the extra bytes are truncated
 * and are not sent to the USIM/SIM.
 */
#define MAX_TERMINAL_PROFILE_ARRAY_SIZE  17

#define IMEI_SIZE 9		/* /< size of IMEI array in dep parms */

/** TX FREQ SECTION PER BAND */
#define N_FREQ_SECTIONS       8

/** audio related defines */
#define AUDIO_MAGIC_SIZE    16

#if defined(_BCM213x1_) || defined(_BCM2153_) || defined(_BCM21551_)	/* FIXME This is not good, you have to add the new #define when we have a new version later. */
#define AUDIO_MODE_NUMBER   9	/* /< Up to 10 Audio Profiles (modes) after 213x1 */
#else
#define AUDIO_MODE_NUMBER   6	/* /< Up to only 6 Audio Profiles (modes) in older chips */
#endif
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
#define AUDIO_MODE_NUMBER_VOICE	(AUDIO_MODE_NUMBER*2)
#else
#define AUDIO_MODE_NUMBER_VOICE	(AUDIO_MODE_NUMBER)
#endif

#define AUDIO_5BAND_EQ_MODE_NUMBER    6	/* /< Up to 6 Audio EQ Profiles (modes) */

#define NUM_OF_VOICE_COEFF      35

#define COEF_PER_GROUP      5	/* /< VOICE_DAC/VOICE_ADC */
#define DSP_SUBBAND_NLP_FREQ_BINS 16	/* /<16 freq bins */
#define DSP_SUBBAND_COMPANDER_FIR_TAP 11	/* /<11 taps */

#define NUM_OF_SIDETONE_FILTER_COEFF 10
#define NUM_OF_ECHO_FAR_IN_FILTER_COEFF 10
#define NUM_OF_ECHO_NLP_CNG_FILTER  10
#define NUM_OF_BIQUAD_FILTER_COEF 10
#define NUM_OF_ECHO_STABLE_COEF_THRESH 2
#define NUM_OF_ECHO_NLP_GAIN    6

#define GPIO_INIT_FIELD_NUM 5
#define GPIO_INIT_REC_NUM 64

#define   DSP_FEATURE_AHS_SID_UPDATE_BEC_CHECK	/* Additional BEC check to determine SID updaet frame in Rxqual_s calculation */
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

#define   NUM_OMEGA_VOICE_BANDS 3
#define   NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS 11
#define   NUM_OMEGA_VOICE_PARMS (NUM_OMEGA_VOICE_BANDS*NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS)

#ifdef BCM2153_FAMILY_BB_CHIP_BOND
#define DSP_SUBBAND_NLP_FREQ_BINS_WB 24	/* /<24 freq bins */
#endif
typedef struct {
	char *name;
	void *ptr;
	unsigned int size;
	unsigned int flag;
} SysparmIndex_t;

#if (defined(_BCM2132_) && CHIP_REVISION >= 33) || (defined(_BCM2133_) && CHIP_REVISION >= 11) || defined(_BCM2124_) || CHIPVERSION >= CHIP_VERSION(BCM2152, 10)
/* Echo Canceller Comfort noise and NLP sysparms RON 4/28/2005 */
typedef struct {
	Int16			echo_cng_bias;
	UInt16			echo_cng_enable;
	UInt16			echo_nlp_max_supp;
	UInt16			echo_nlp_enable;
	UInt16			echo_subband_nlp_enable;
	UInt16			echo_nlp_gain;
	UInt16			echo_nlp_ul_brk_in_thresh;
	UInt16			echo_nlp_min_ul_brk_in_thresh;
	UInt16			echo_nlp_ul_gain_table[NUM_OF_ECHO_NLP_GAIN];
	UInt16			echo_nlp_dl_gain_table[NUM_OF_ECHO_NLP_GAIN];
	UInt16			nlp_ul_energy_window;
	UInt16			nlp_dl_energy_window;
	UInt16			echo_nlp_dtalk_ul_gain;
	UInt16			echo_nlp_dtalk_dl_agc_idx;/* not used yet */
	UInt16			echo_nlp_idle_ul_gain;/* not used yet */
	UInt16			echo_nlp_idle_dl_agc_idx;/* not used yet */
	UInt16			echo_nlp_ul_active_dl_agc_idx;/* not used yet */
	Int16			echo_nlp_relative_offset;
	UInt16			echo_nlp_relative_dl_w_thresh;
	UInt16			echo_nlp_relative_dl_energy_window;
	UInt16			echo_nlp_relative_dl_energy_decay;
	UInt16			echo_nlp_min_dl_pwr;
	UInt16			echo_nlp_min_ul_pwr;
	UInt16			echo_nlp_dtalk_dl_gain;
	UInt16			echo_nlp_dl_idle_ul_gain;
	UInt16			echo_nlp_ul_active_dl_gain;
	UInt16			echo_nlp_ul_idle_dl_gain;
	UInt16			echo_nlp_ul_idle_ul_gain;
	Int16			echo_nlp_relative_offset_dtalk;
	Int16			echo_nlp_relative_offset_dl_active;
	Int16			echo_nlp_relative_offset_ul_active;
	UInt16			echo_nlp_dtalk_hang_count;
	UInt16			echo_nlp_ul_active_hang_count;
	UInt16			echo_nlp_relative_ul_active_dl_loss;
	UInt16			echo_nlp_relative_ul_idle_dl_loss;
	UInt16			echo_nlp_relative_dl_idle_ul_loss;
	UInt16			echo_nlp_dl_persistence_thresh;
	UInt16			echo_nlp_dl_persistence_leak;
	UInt16			echo_nlp_dl_loss_adjust_thld;
	UInt16			echo_nlp_ul_active_dl_loss_adjust_step;
	UInt16			echo_nlp_ul_idle_dl_loss_adjust_step;
	UInt16			echo_nlp_ul_active_dl_loss_min_val;
	UInt16			echo_nlp_ul_idle_dl_loss_min_val;
	UInt16			echo_nlp_dl_idle_dl_gain;
#ifdef DSP_FEATURE_SUBBAND_NLP
	UInt16          echo_subband_nlp_distortion_thresh[DSP_SUBBAND_NLP_FREQ_BINS];
	UInt16 			echo_subband_nlp_dl_delay_adj;
	UInt16 			echo_subband_nlp_ul_margin[DSP_SUBBAND_NLP_FREQ_BINS];
	UInt16          echo_subband_nlp_noise_margin[DSP_SUBBAND_NLP_FREQ_BINS];
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
	UInt16 echo_subband_nlp_erl_erle_adj_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 echo_subband_nlp_ul_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
	UInt16 echo_subband_nlp_noise_margin_wb[DSP_SUBBAND_NLP_FREQ_BINS_WB];
#endif
	UInt16 			echo_subband_nlp_filtered_gain_alpha;
	UInt16 			echo_subband_nlp_hpf_coef_a;
	UInt16 			echo_subband_nlp_hpf_coef_b;
	UInt16			echo_subband_nlp_dt_fine_control;
#endif
} EchoNlp_t;

typedef struct {
	UInt16 audio_agc_enable;	/* /< Enable/Disable uplink and downlink audio agc */
	UInt16 max_thresh;
	UInt16 hi_thresh;
	UInt16 low_thresh;
	UInt16 decay;
	UInt16 max_idx;
	UInt16 min_idx;
	UInt16 step_down;
	UInt16 max_step_down;
	UInt16 step_up;
} AudioAGC_t;
#endif

#ifdef DSP_FEATURE_SUBBAND_INF_COMP
typedef struct {
	UInt16 t2lin;
	UInt16 g2t2;
	UInt16 g3t3;
	UInt16 g4t4;
	UInt16 alpha;
	UInt16 beta;
	UInt16 env;
	Int16 g1lin;
	Int16 step2;
	Int16 step3;
	Int16 step4;
} Subband_Compander_Band_t;
#endif

#ifdef DSP_FEATURE_OMEGA_VOICE
typedef struct {
	Int16 omega_voice_thres_dB[NUM_OMEGA_VOICE_BANDS];
	Int16 omega_voice_max_gain_dB[NUM_OMEGA_VOICE_BANDS];
	Int16 omega_voice_gain_step_up_dB256;
	Int16 omega_voice_gain_step_dn_dB256;
	Int16 omega_voice_max_gain_dB_scale_factor;
} OmegaVoice_t;
#endif

typedef struct {
	UInt8 audio_parm_magic[AUDIO_MAGIC_SIZE];
	UInt16 audio_channel;
	UInt16 speaker_pga;  /* level, index */
	Int16  ext_speaker_pga;
	Int16  ext_speaker_preamp_pga;
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
	UInt16 ext_speaker_enable;
#endif
	UInt16 mic_pga;      /* level, index */
	UInt16 max_mic_gain;
#if CHIPVERSION >= CHIP_VERSION(BCM2153, 10)
	UInt32 audvoc_anacr0;
	UInt32 audvoc_anacr1;
	UInt32 audvoc_anacr2;
	UInt32 audvoc_anacr3;
	UInt32 audvoc_anacr4;
	UInt16 audvoc_aprr;
	UInt16 audvoc_vslopgain;
#ifndef BCM2153_FAMILY_BB_CHIP_BOND
	UInt16 audvoc_pslopgain;
	UInt16 audvoc_aslopgain;
#endif				/* BCM2153_FAMILY_BB_CHIP_BOND */
	UInt16 audvoc_mixergain;
	UInt16 audvoc_vcfgr;
#endif
	UInt16 sidetone;
#if (defined(_BCM2132_) && CHIP_REVISION == 33) || (defined(_BCM2133_) && CHIP_REVISION >= 12) || (defined(_BCM2124_) && CHIP_REVISION >= 11) || CHIPVERSION >= CHIP_VERSION(BCM2152, 10)
	UInt16 audio_dsp_sidetone;  /* means dsp_sidetone_enable. */
	UInt16 audio_dl_idle_pga_adj;
	UInt16 audio_ns_ul_idle_adj;
#endif
	UInt16 dac_filter_scale_factor;
#if (defined(_BCM2132_) && CHIP_REVISION >= 33) /* 2132C3 */ || CHIPVERSION >= CHIP_VERSION(BCM2133, 11)
	UInt16	ty_mic_gain;
	UInt16	sidetone_tty;
#endif
	AudioAGC_t ul_agc;
	AudioAGC_t dl_agc;
	EchoNlp_t echoNlp_parms;
	UInt16 echo_cancelling_enable;
	UInt16 echo_dual_filter_mode;
	UInt16 echo_nlp_cng_filter[NUM_OF_ECHO_NLP_CNG_FILTER];
	UInt16 echo_far_in_filter[NUM_OF_ECHO_FAR_IN_FILTER_COEFF];
#if (defined(_BCM2133_) && CHIP_REVISION >= 14) || (defined(_BCM2124_) && CHIP_REVISION >= 11) || CHIPVERSION >= CHIP_VERSION(BCM2152, 13) || defined(_BCM2153_) || defined(_BCM213x1_) || defined(_BCM21551_)
	UInt16	comp_filter_coef[NUM_OF_BIQUAD_FILTER_COEF];
	UInt16	comp_biquad_gain;
	UInt16	echo_farIn_filt_gain;
#endif
	UInt16 echo_adapt_norm_factor;
	UInt16 echo_stable_coef_thresh[NUM_OF_ECHO_STABLE_COEF_THRESH];
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
	UInt16 echo_nlp_timeout_val;
	UInt16 noise_suppression_enable;
	UInt16 noise_supp_input_gain;
	UInt16 noise_supp_output_gain;
	UInt16 voice_dac[NUM_OF_VOICE_COEFF];
	UInt16 voice_adc[NUM_OF_VOICE_COEFF];
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
	UInt16 voice_dualmic_enable;
	UInt16 voice_adc_d1[NUM_OF_VOICE_COEFF];
	UInt16 voice_adc_d2[NUM_OF_VOICE_COEFF];
#endif /* BCM2153_FAMILY_BB_CHIP_BOND */
	UInt16 voice_dac_hpf_enable;
	UInt16 voice_dac_hpf_cutoff_freq;
#if (defined(_BCM2133_) && CHIP_REVISION >= 12) || (defined(_BCM2124_) && CHIP_REVISION >= 11) || CHIPVERSION >= CHIP_VERSION(BCM2152, 10)
	UInt16 ecLen;
	UInt16 blockSize;
	UInt16 block_ms;
	UInt16 SoutUpdateSizeModij;
	UInt16 numBlkPerFrameMinus1;
	UInt16 lpc_r_window_size;
	UInt16 RinLpcBuffSize;
	UInt16 Rin_short_term_len;
	UInt16 Rin_short_term_len_inv;
	UInt16 RinCirBuffSizeModij;
	UInt16 VAD_TH_dB;
	UInt16 VAD_SHORT_TH_dB;
	Int16 DT_TH_ERL_dB;
	UInt16 DT_TH_SOUT_dB;
	UInt16 DT_HANGOVER_TIME_ms;
	UInt16 hangover_counter;
	UInt16 echo_step_size_gain;

	UInt16	compander_flag ;
	UInt16	expander_alpha;
	UInt16	expander_beta;
	UInt16	expander_upper_limit;
	UInt16	expander_c;
	UInt16	expander_c_div_b;
	UInt16	expander_inv_b;
	UInt16	compressor_output_gain;
	UInt16	compressor_gain;
	UInt16	compressor_alpha;
	UInt16	compressor_beta;
#if (defined(_BCM2133_) && CHIP_REVISION >= 14) || (defined(_BCM2124_) && CHIP_REVISION >= 12) || CHIPVERSION >= CHIP_VERSION(BCM2152, 10)
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
	UInt16	expander_c_ul;
	UInt16	expander_c_div_b_ul;
	UInt16	expander_inv_b_ul;
	UInt16	compressor_output_gain_ul;
	UInt16	compressor_gain_ul;
	UInt16	compressor_alpha_ul;
	UInt16	compressor_beta_ul;
	UInt16	ul_audio_clip_level;
#if (defined(_BCM2152_) && CHIP_REVISION >= 14) || CHIPVERSION > CHIP_VERSION(BCM213x1v, 20) || (defined(_BCM2153_) && CHIP_REVISION >= 40) || defined(_BCM21551_)
	UInt16	polyringer_out_gain_dl;
	UInt16	polyringer_out_gain_ul;
#else
	UInt16	polyringer_out_gain;
#endif
	UInt16	second_amr_out_gain;
#endif
	Int16	noise_supp_min;
	Int16	noise_supp_max;
#endif
#if (defined(_BCM2133_) && CHIP_REVISION > 14) || (defined(_BCM2124_) && CHIP_REVISION > 11) || (defined(_BCM2152_) && CHIP_REVISION > 13) || CHIPVERSION > CHIP_VERSION(BCM213x1, 12) || (defined(_BCM2153_) && CHIP_REVISION > 30)
	/* 2133A5, 2124A2,2152A4, 213x1/2153A3 and beyond*/
	UInt16	arm2speech_call_gain;
#endif
	UInt16 volume_step_size;
	UInt16 num_supported_volume_levels;

	UInt16 voice_volume_max;  /* in dB. */
	UInt16 voice_volume_init; /* in dB. */

	UInt16 sidetone_output_gain;
	UInt16 sidetone_biquad_scale_factor;
	UInt16 sidetone_biquad_sys_gain;

	Int16	ec_de_emp_filt_coef;
	Int16	ec_pre_emp_filt_coef;

	UInt16	audio_hpf_enable;
	UInt16	audio_ul_hpf_coef_b;
	UInt16	audio_ul_hpf_coef_a;
	UInt16	audio_dl_hpf_coef_b;
	UInt16	audio_dl_hpf_coef_a;

#if (defined(_BCM2133_) && CHIP_REVISION >= 14) || (defined(_BCM2124_) && CHIP_REVISION >= 11) || CHIPVERSION >= CHIP_VERSION(BCM2152, 13)
	UInt16	bluetooth_filter_enable;
#endif

#ifdef DSP_FEATURE_SUBBAND_INF_COMP
	UInt16  dl_subband_compander_flag;
	Subband_Compander_Band_t fir1;    /* midband */
	Subband_Compander_Band_t fir2;    /* lowband */
	Subband_Compander_Band_t stream3; /* highband */
	Subband_Compander_Band_t fullband; /* fullband */
	Int16 compress_coef_fir1[DSP_SUBBAND_COMPANDER_FIR_TAP];
	Int16 compress_coef_fir2[DSP_SUBBAND_COMPANDER_FIR_TAP];
#endif

#ifndef BCM2153_FAMILY_BB_CHIP_BOND
	UInt16 PR_DAC_IIR[25];	/* Polyringer DAC IIR coefficients */
	UInt16 AUDVOC_ADAC_IIR[25];
	UInt16 AUDVOC_ADAC_IIR_hpf_enable;
	UInt16 AUDVOC_ADAC_IIR_hpf_cutoff_freq;
#endif				/* BCM2153_FAMILY_BB_CHIP_BOND */
#ifdef DSP_FEATURE_OMEGA_VOICE
	UInt16	omega_voice_enable;
	UInt16	omega_voice_max_allowed_gain_spread_dB;
	OmegaVoice_t omega_voice_parms[NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS];
#endif
} SysAudioParm_t;
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
typedef struct {
	UInt16 audvoc_pslopgain;
	UInt16 audvoc_aslopgain;
	UInt16 PR_DAC_IIR[25];	/* Polyringer DAC IIR coefficients */
	UInt16 AUDVOC_ADAC_IIR[25];
	UInt16 AUDVOC_ADAC_IIR_hpf_enable;
	UInt16 AUDVOC_ADAC_IIR_hpf_cutoff_freq;

} SysIndMultimediaAudioParm_t;
#endif /* BCM2153_FAMILY_BB_CHIP_BOND */

UInt16 SYSPARM_GetLogFormat(void);
SysAudioParm_t *SYSPARM_GetAudioParmAccessPtr(UInt8 AudioMode);
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
SysIndMultimediaAudioParm_t *APSYSPARM_GetMultimediaAudioParmAccessPtr(void);
#endif /*  BCM2153_FAMILY_BB_CHIP_BOND */

UInt16 SYSPARM_GetDefault4p2VoltReading(void);
UInt16 SYSPARM_GetActual4p2VoltReading(void);

UInt8 *SYSPARM_GetGPIO_Default_Value(UInt8 gpio_index);

/* retrieves IMEI string from sysparms only (doesn't check MS database */
/* value, as is done in CP sysparm.c) */
Boolean SYSPARM_GetImeiStr(UInt8 *inImeiStrPtr);

#endif /* _BCM_FUSE_SYSPARM_H_ */
