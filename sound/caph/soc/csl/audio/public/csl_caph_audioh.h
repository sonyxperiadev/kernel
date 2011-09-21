/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.                                */

/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/


/**
*
*   @file   csl_caph_audioh.h
*
*   @brief  This file contains the definition for audioh CSL layer
*
****************************************************************************/



#ifndef _CSL_CAPH_AUDIOH_
#define _CSL_CAPH_AUDIOH_

typedef enum 
{
    CSL_CAPH_AUDIOH_IHF_DACL_PWRDOWN = 0x0,
    CSL_CAPH_AUDIOH_IHF_DACR_PWRDOWN = 0x0,
    CSL_CAPH_AUDIOH_IHF_DACLR_PWRDOWN = 0x0,
    CSL_CAPH_AUDIOH_IHF_DACL_PWRUP = 0x1,
    CSL_CAPH_AUDIOH_IHF_DACR_PWRUP = 0x2,
    CSL_CAPH_AUDIOH_IHF_DACLR_PWRUP = 0x3
} CSL_CAPH_AUDIOH_IHF_DAC_PWR_e;


typedef enum 
{
    CSL_CAPH_AUDIOH_DMIC1_DISABLE = 0x0,
    CSL_CAPH_AUDIOH_DMIC2_DISABLE = 0x0,
    CSL_CAPH_AUDIOH_DMIC12_DISABLE = 0x0,
    CSL_CAPH_AUDIOH_DMIC1_ENABLE = 0x1,
    CSL_CAPH_AUDIOH_DMIC2_ENABLE = 0x2,
    CSL_CAPH_AUDIOH_DMIC12_ENABLE = 0x3,
} CSL_CAPH_AUDIOH_VINPATH_DMIC_ENABLE_e;


typedef enum 
{
    CSL_CAPH_AUDIOH_DMIC3_DISABLE = 0x0,
    CSL_CAPH_AUDIOH_DMIC4_DISABLE = 0x0,
    CSL_CAPH_AUDIOH_DMIC34_DISABLE = 0x0,
    CSL_CAPH_AUDIOH_DMIC3_ENABLE = 0x1,
    CSL_CAPH_AUDIOH_DMIC4_ENABLE = 0x2,
    CSL_CAPH_AUDIOH_DMIC34_ENABLE = 0x3,
} CSL_CAPH_AUDIOH_NVINPATH_DMIC_ENABLE_e;

/**
*  CAPH AUDIOH Path
******************************************************************************/

typedef enum 
{
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
} AUDDRV_PATH_Enum_t;


typedef enum {
    AUDDRV_MIC_NONE,

#ifdef _HERA_
    AUDDRV_MIC_AMIC    = 0,	    // Analog MIC/Handset   
	AUDDRV_MIC_DMIC1   = 1,	    // DMIC 1
    AUDDRV_MIC_DMIC2   = 2,	    // DMIC 2
    AUDDRV_MIC_DMIC3   = 4,	    // DMIC 3 
    AUDDRV_MIC_DMIC4   = 8,	    // DMIC 4 
    AUDDRV_MIC_AUXMIC  = 0x10,	// Analog MIC/Headset
#endif

    AUDDRV_MIC_ANALOG_MAIN,
    AUDDRV_MIC_ANALOG_AUX,
    AUDDRV_MIC_DIGI1,  // DMIC1
    AUDDRV_MIC_DIGI2,  // DMIC2
    AUDDRV_MIC_DIGI3,
    AUDDRV_MIC_DIGI4,

	AUDDRV_DUAL_MIC_DIGI12,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_DIGI21,  // DMIC1 and DMIC2
	AUDDRV_DUAL_MIC_ANALOG_DIGI1,
	AUDDRV_DUAL_MIC_DIGI1_ANALOG,

    AUDDRV_MIC_SPEECH_DIGI,
    AUDDRV_MIC_EANC_DIGI,

	AUDDRV_MIC_PCM_IF,
    AUDDRV_MIC_USB_IF,
    AUDDRV_MIC_NOISE_CANCEL,
	AUDDRV_MIC_TOTAL_NUM
} AUDDRV_MIC_Enum_t;

typedef enum {

    AUDDRV_VIBRATOR_BYPASS_MODE,
    AUDDRV_VIBRATOR_WAVE_MODE

} AUDDRV_VIBRATOR_MODE_Enum_t;

typedef enum
{
	AUDDRV_SPKR_NONE,
    AUDDRV_SPKR_EP,
    AUDDRV_SPKR_EP_STEREO,
    AUDDRV_SPKR_IHF,
    AUDDRV_SPKR_IHF_STEREO,
    AUDDRV_SPKR_HS,
	AUDDRV_SPKR_HS_LEFT,
	AUDDRV_SPKR_HS_RIGHT,
	AUDDRV_SPKR_PCM_IF,
    AUDDRV_SPKR_VIBRA,
	AUDDRV_SPKR_USB_IF,
	AUDDRV_SPKR_TOTAL_NUM
} AUDDRV_SPKR_Enum_t;


/**
*  CAPH AUDIOH Data handling
******************************************************************************/
typedef enum 
{
	DRIVER_HANDLE_DATA_TRANSFER = 0,
	DRIVER_HANDLE_DATA_RINGBUFFER,
	CLIENT_HANDLE_DATA_TRANSFER,
	DEVICE_HANDLE_DATA_TRANSFER,
} AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t; 

/**
*  CAPH AUDIOH Path Configuration parameters
******************************************************************************/
typedef struct
{
	// below are same defeinition as audio_config_t 
	int sample_rate;
	int sample_size;
	int sample_mode;
	int sample_pack;
	int	eanc_input;
	int	eanc_output;
} CSL_CAPH_AUDIOH_Path_t;

/**
* For AudioH, the buffer address
******************************************************************************/
typedef struct
{
    UInt32 bufAddr;
    UInt32 buf2Addr;
}CSL_CAPH_AUDIOH_BUFADDR_t;

/**
* For AudioH, test
******************************************************************************/
typedef struct 
{
	UInt32 AUDIOTX_TEST_EN;
	UInt32 AUDIOTX_BB_STI;
	UInt32 AUDIOTX_EP_DRV_STO;
}CSL_CAPH_AUDIOH_DACCTRL_t;


/**
*  CAPH AUDIOH Control Configuration parameters
******************************************************************************/
typedef struct
{
	int sample_rate;
	int sample_size;
	AUDIO_CHANNEL_NUM_t sample_mode;
	int sample_pack;
	AUDDRV_MIC_Enum_t	eanc_input;
	AUDDRV_PATH_Enum_t	eanc_output;
	AUDDRV_PATH_Enum_t	sidetone_output;
	AUDDRV_PATH_DATA_TRANSFER_MODE_Enum_t data_handle_mode; 
} audio_config_t;


void csl_caph_audioh_init (UInt32 baseAddr, UInt32 sdtBaseAddr);
void csl_caph_audioh_deinit(void);
void csl_caph_audioh_config(int path_id, void *pcfg);
void csl_caph_audioh_unconfig(int path_id);
CSL_CAPH_AUDIOH_BUFADDR_t csl_caph_audioh_get_fifo_addr(int path_id);
void csl_caph_audioh_start(int path_id);
void csl_caph_audioh_stop_keep_config(int path_id);
void csl_caph_audioh_stop(int path_id);
void csl_caph_audioh_mute(int path_id, Boolean mute_ctrl);
void csl_caph_audioh_setgain(int path_id ,UInt32 gain, UInt32 gain1);
void csl_caph_audioh_sidetone_control(int path_id, Boolean ctrl);
void csl_caph_audioh_eanc_output_control(int path_id, Boolean ctrl);
void csl_caph_audioh_loopback_control(int lbpath, Boolean ctrl);
void csl_caph_audioh_eanc_input_control(int dmic);
void csl_caph_audioh_sidetone_set_gain(UInt32 gain);
void csl_caph_audioh_sidetone_load_filter(UInt32 *coeff);
void csl_caph_audioh_set_hwgain(CSL_CAPH_HW_GAIN_e hw, UInt32 gain);
void csl_caph_audioh_ihfpath_set_dac_pwr(UInt16 enable_chnl);
CSL_CAPH_AUDIOH_IHF_DAC_PWR_e csl_caph_audioh_ihfpath_get_dac_pwr(void);
void csl_caph_audioh_vinpath_digi_mic_enable(UInt16 ctrl);
void csl_caph_audioh_nvinpath_digi_mic_enable(UInt16 ctrl);
CSL_CAPH_AUDIOH_VINPATH_DMIC_ENABLE_e csl_caph_audioh_vinpath_digi_mic_enable_read(void);
CSL_CAPH_AUDIOH_NVINPATH_DMIC_ENABLE_e csl_caph_audioh_nvinpath_digi_mic_enable_read(void);
#endif // _CSL_CAPH_AUDIOH_
