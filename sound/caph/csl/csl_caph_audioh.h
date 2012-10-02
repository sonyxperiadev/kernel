/****************************************************************************/
/*     Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.     */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*   @file   csl_caph_audioh.h
*
*   @brief  This file contains the definition for audioh CSL layer
*
****************************************************************************/

#ifndef _CSL_CAPH_AUDIOH_
#define _CSL_CAPH_AUDIOH_

/**
*  CAPH AUDIOH Path
******************************************************************************/

enum _AUDDRV_PATH_Enum_t {
	AUDDRV_PATH_NONE,
	AUDDRV_PATH_VIBRA_OUTPUT,
	AUDDRV_PATH_HEADSET_OUTPUT,
	AUDDRV_PATH_IHF_OUTPUT,
	AUDDRV_PATH_EARPICEC_OUTPUT,
	AUDDRV_PATH_VIN_INPUT,
	AUDDRV_PATH_VIN_INPUT_L,
	AUDDRV_PATH_VIN_INPUT_R,
	AUDDRV_PATH_NVIN_INPUT,
	AUDDRV_PATH_NVIN_INPUT_L,
	AUDDRV_PATH_NVIN_INPUT_R,
	AUDDRV_PATH_ANALOGMIC_INPUT,
	AUDDRV_PATH_EANC_INPUT,
	AUDDRV_PATH_SIDETONE_INPUT,
	AUDDRV_PATH_HEADSET_INPUT,
	AUDDRV_PATH_TOTAL,
};
#define AUDDRV_PATH_Enum_t enum _AUDDRV_PATH_Enum_t

enum _AUDDRV_VIBRATOR_MODE_Enum_t {
	AUDDRV_VIBRATOR_BYPASS_MODE,
	AUDDRV_VIBRATOR_WAVE_MODE
};
#define AUDDRV_VIBRATOR_MODE_Enum_t enum _AUDDRV_VIBRATOR_MODE_Enum_t

/**
*  CAPH AUDIOH Data handling
******************************************************************************/
enum _AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t {
	DRIVER_HANDLE_DATA_TRANSFER = 0,
	DRIVER_HANDLE_DATA_RINGBUFFER,
	CLIENT_HANDLE_DATA_TRANSFER,
	DEVICE_HANDLE_DATA_TRANSFER,
};
#define AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t \
enum _AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t

/**
*  CAPH AUDIOH Path Configuration parameters
******************************************************************************/
struct _CSL_CAPH_AUDIOH_Path_t {
	/* below are same defeinition as audio_config_t */
	int sample_rate;
	int sample_size;
	int sample_mode;
	int sample_pack;
	int eanc_input;
	int eanc_output;
	int started;
};
#define CSL_CAPH_AUDIOH_Path_t struct _CSL_CAPH_AUDIOH_Path_t

/**
* For AudioH, the buffer address
******************************************************************************/
struct _CSL_CAPH_AUDIOH_BUFADDR_t {
	UInt32 bufAddr;
	UInt32 buf2Addr;
};
#define CSL_CAPH_AUDIOH_BUFADDR_t struct _CSL_CAPH_AUDIOH_BUFADDR_t

/**
* For AudioH, test
******************************************************************************/
struct _CSL_CAPH_AUDIOH_DACCTRL_t {
	UInt32 AUDIOTX_TEST_EN;
	UInt32 AUDIOTX_BB_STI;
	UInt32 AUDIOTX_EP_DRV_STO;
};
#define CSL_CAPH_AUDIOH_DACCTRL_t struct _CSL_CAPH_AUDIOH_DACCTRL_t

/**
*  CAPH AUDIOH Control Configuration parameters
******************************************************************************/
struct _audio_config_t {
	int sample_rate;
	int sample_size;
	AUDIO_NUM_OF_CHANNEL_t sample_mode;
	int sample_pack;
	AUDIO_SOURCE_Enum_t eanc_input;
	AUDDRV_PATH_Enum_t eanc_output;
	AUDDRV_PATH_Enum_t sidetone_output;
	AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t data_handle_mode;
};
#define audio_config_t struct _audio_config_t

#define GAIN_SYSPARM 0x8000
#define GAIN_NA 0x8001

enum _csl_caph_MIC_Path_e {
	MIC_ANALOG_HEADSET,
	MIC_DIGITAL,
};
#define csl_caph_MIC_Path_e enum _csl_caph_MIC_Path_e

struct _csl_caph_Mic_Gain_t {
	int gain_in_mB;
	UInt16 micPGA;
	UInt16 micCICFineScale;
	UInt16 micCICBitSelect;
	int micDSPULGain; /* mdB in Q15 */
};
#define csl_caph_Mic_Gain_t struct _csl_caph_Mic_Gain_t

struct _csl_caph_Sidetone_Gain_t {
	short gain;  /*In Q13.2 */
	UInt16 sidetoneGain_Linear;
	UInt16 sidetoneGain_Log;
};
#define csl_caph_Sidetone_Gain_t struct _csl_caph_Sidetone_Gain_t

void *csl_caph_audioh_init(UInt32 baseAddr, UInt32 sdtBaseAddr);
void csl_caph_audioh_deinit(void);
void csl_caph_audioh_config(int path_id, void *pcfg);
void csl_caph_audioh_unconfig(int path_id);
CSL_CAPH_AUDIOH_BUFADDR_t csl_caph_audioh_get_fifo_addr(int path_id);
void csl_caph_audioh_start(int path_id);
void csl_caph_audioh_stop(int path_id);
void csl_caph_audioh_mute(int path_id, Boolean mute_ctrl);

void csl_caph_audioh_setgain_register(int path_id, UInt32 gain,
				      UInt32 fine_scale);

void csl_caph_audioh_setMicPga_by_mB(int gain_mB);
void csl_caph_audioh_vin_set_cic_scale_by_mB(int mic1_coarse_gain,
					     int mic1_fine_gain,
					     int mic2_coarse_gain,
					     int mic2_fine_gain);
void csl_caph_audioh_nvin_set_cic_scale_by_mB(int mic1_coarse_gain,
					      int mic1_fine_gain,
					      int mic2_coarse_gain,
					      int mic2_fine_gain);

void csl_caph_audioh_sidetone_control(int path_id, Boolean ctrl);
void csl_caph_audioh_eanc_output_control(int path_id, Boolean ctrl);
void csl_caph_audioh_loopback_control(int lbpath, Boolean ctrl);
void csl_caph_audioh_eanc_input_control(int dmic);
void csl_caph_audioh_sidetone_set_gain(UInt32 gain);
void csl_caph_audioh_sidetone_load_filter(UInt32 *coeff);
void csl_caph_audioh_vinpath_digi_mic_enable(UInt16 ctrl);
void csl_caph_audioh_nvinpath_digi_mic_enable(UInt16 ctrl);
void csl_caph_audioh_set_minimum_filter(int path_id);
void csl_caph_audioh_set_linear_filter(int path_id);
void csl_caph_audioh_adcpath_global_enable(Boolean enable);
void csl_caph_audioh_start_hs(void);
void csl_caph_audioh_start_ihf(void);
void csl_caph_audioh_stop_hs(void);
void csl_caph_audioh_stop_ihf(void);
#if defined(CONFIG_MFD_BCM59039) | defined(CONFIG_MFD_BCM59042)
void csl_caph_audioh_hs_supply_get_hs_ds_indicator(UInt16 *hs_ds_indicator);
void csl_caph_audioh_hs_supply_set_hs_ds_thres(UInt32 hs_ds_thres);
#endif
/**
*
*  @brief  Find register values for required mic path gain.
*
*  @param  mic	  mic path
*  @param  gain   Requested gain in mB.
*
*  @return csl_caph_Mic_Gain_t	 the structure containing
*  register values for setting the gains on mic path.
*
*****************************************************************************/
csl_caph_Mic_Gain_t csl_caph_map_mB_gain_to_registerVal(csl_caph_MIC_Path_e mic,
							int gain_mB);

#endif /* _CSL_CAPH_AUDIOH_ */
