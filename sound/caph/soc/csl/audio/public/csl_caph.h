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
*   @file   csl_caph.h
*
*   @brief  This file contains the const for caph CSL layer
*
****************************************************************************/

#ifndef _CSL_CAPH_
#define _CSL_CAPH_

#include "mobcom_types.h"
#include "audio_consts.h"


//Per ASIC people, To get the transfer size of 4 32bit words, need to set as 3.
#define CSL_AADMAC_TSIZE    0x3

//According to PCM/I2S SSP CSL design, the transfer size for pcm and i2s should be 
//defined as following.  If the size is less then the following value, the transferring
//will be one-time shot. Otherwise it will be transferred continuously.
#define CSL_I2S_SSP_TSIZE   4096
#define CSL_PCM_SSP_TSIZE   8192


#define	HS_MIN_APP_GAIN_SCALE	00
#define	HS_MAX_APP_GAIN_SCALE	40
#define	HS_MIN_PUM_GAIN_SCALE	0x00
#define	HS_MAX_PUM_GAIN_SCALE	0x3F
//#define	AMIC_MIN_APP_GAIN_SCALE	00
//#define	AMIC_MAX_APP_GAIN_SCALE	40
//#define	AMIC_MIN_PUM_GAIN_SCALE	0x00
//#define	AMIC_MAX_PUM_GAIN_SCALE	0x3F


#define	IHF_MIN_APP_GAIN_SCALE	00
#define	IHF_MAX_APP_GAIN_SCALE	40
#define	IHF_MIN_PUM_GAIN_SCALE	0x00
#define	IHF_MAX_PUM_GAIN_SCALE	0x3F

/**
* CAPH CFIFO FIFO buffers
******************************************************************************/
typedef enum
{
	CSL_CAPH_CFIFO_NONE,
	CSL_CAPH_CFIFO_FIFO1,
	CSL_CAPH_CFIFO_FIFO2,
	CSL_CAPH_CFIFO_FIFO3,
	CSL_CAPH_CFIFO_FIFO4,
	CSL_CAPH_CFIFO_FIFO5,
	CSL_CAPH_CFIFO_FIFO6,
	CSL_CAPH_CFIFO_FIFO7,
	CSL_CAPH_CFIFO_FIFO8,
	CSL_CAPH_CFIFO_FIFO9,
	CSL_CAPH_CFIFO_FIFO10,
	CSL_CAPH_CFIFO_FIFO11,
	CSL_CAPH_CFIFO_FIFO12,
	CSL_CAPH_CFIFO_FIFO13,
	CSL_CAPH_CFIFO_FIFO14,
	CSL_CAPH_CFIFO_FIFO15,
	CSL_CAPH_CFIFO_FIFO16,
}CSL_CAPH_CFIFO_FIFO_e;

/**
* CAPH AADMAC Channels
******************************************************************************/
typedef enum
{
	CSL_CAPH_DMA_NONE,
	CSL_CAPH_DMA_CH1,
	CSL_CAPH_DMA_CH2,
	CSL_CAPH_DMA_CH3,
	CSL_CAPH_DMA_CH4,
	CSL_CAPH_DMA_CH5,
	CSL_CAPH_DMA_CH6,
	CSL_CAPH_DMA_CH7,
	CSL_CAPH_DMA_CH8,
	CSL_CAPH_DMA_CH9,
	CSL_CAPH_DMA_CH10,
	CSL_CAPH_DMA_CH11,
	CSL_CAPH_DMA_CH12,
	CSL_CAPH_DMA_CH13,
	CSL_CAPH_DMA_CH14,
	CSL_CAPH_DMA_CH15,
	CSL_CAPH_DMA_CH16,
	TOTAL_CSL_CAPH_DMA_CH
}CSL_CAPH_DMA_CHNL_e;

/**
* CAPH SSASW Channel
******************************************************************************/
typedef enum
{
	CSL_CAPH_SWITCH_NONE = 0,
	CSL_CAPH_SWITCH_CH1,
	CSL_CAPH_SWITCH_CH2,
	CSL_CAPH_SWITCH_CH3,
	CSL_CAPH_SWITCH_CH4,
	CSL_CAPH_SWITCH_CH5,
	CSL_CAPH_SWITCH_CH6,
	CSL_CAPH_SWITCH_CH7,
	CSL_CAPH_SWITCH_CH8,
	CSL_CAPH_SWITCH_CH9,
	CSL_CAPH_SWITCH_CH10,
	CSL_CAPH_SWITCH_CH11,
	CSL_CAPH_SWITCH_CH12,
	CSL_CAPH_SWITCH_CH13,
	CSL_CAPH_SWITCH_CH14,
	CSL_CAPH_SWITCH_CH15,
	CSL_CAPH_SWITCH_CH16,
} CSL_CAPH_SWITCH_CHNL_e;



/**
* CAPH OWNER: ARM or DSP
******************************************************************************/
typedef enum
{
    CSL_CAPH_ARM,
    CSL_CAPH_DSP,
}CSL_CAPH_ARM_DSP_e;
/**
* CAPH Data Mode: Stereo or Mono
******************************************************************************/
typedef enum
{
    CSL_CAPH_MONO,
    CSL_CAPH_STEREO,
}CSL_CAPH_Mode_e;

/**
* CAPH Data Format: Stereo or Mono, 16bit or 24bit
******************************************************************************/
typedef enum
{
    CSL_CAPH_16BIT_MONO,
    CSL_CAPH_16BIT_STEREO,
    CSL_CAPH_24BIT_MONO,
    CSL_CAPH_24BIT_STEREO,
}CSL_CAPH_DATAFORMAT_e;


/**
* CAPH SSP ID
******************************************************************************/
typedef enum
{
    CSL_CAPH_SSP_NONE,
    CSL_CAPH_SSP_3,
    CSL_CAPH_SSP_4,
}CSL_CAPH_SSP_e;

typedef struct
{
    UInt32 fm_baseAddr;
    UInt32 pcm_baseAddr;
    CSL_CAPH_SSP_e fm_port;
    CSL_CAPH_SSP_e pcm_port;
}CSL_CAPH_SSP_Config_t;


/**
* CAPH FIFO buffer
******************************************************************************/
typedef enum
{
	CSL_CAPH_AUDIOH_EP_FIFO,
	CSL_CAPH_AUDIOH_HS_FIFO,
	CSL_CAPH_AUDIOH_IHF_FIFO,
	CSL_CAPH_AUDIOH_VIBRA_FIFO,
	CSL_CAPH_AUDIOH_EANC_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC1_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC2_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC3_FIFO,
	CSL_CAPH_AUDIOH_DIGI_MIC4_FIFO,
	CSL_CAPH_SSP3_FIFO,
	CSL_CAPH_SSP4_FIFO,
	CSL_CAPH_SRCMIXER_CH1_FIFO,
	CSL_CAPH_SRCMIXER_CH2_FIFO,
	CSL_CAPH_SRCMIXER_CH3_FIFO,
	CSL_CAPH_SRCMIXER_CH4_FIFO,
	CSL_CAPH_SRCMIXER_CH5_FIFO,
	CSL_CAPH_SRCMIXER_PASSCH1_FIFO,
	CSL_CAPH_SRCMIXER_PASSCH2_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH1_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH2_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH3_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH4_FIFO,
	CSL_CAPH_SRCMIXER_TAP_OUTCH5_FIFO,
	CSL_CAPH_SRCMIXER_MIXER1_OUTFIFO,
	CSL_CAPH_SRCMIXER_MIXER2_LOUTFIFO,
	CSL_CAPH_SRCMIXER_MIXER2_ROUTFIFO,
	CSL_CAPH_CFIFO_FIFO_1,
	CSL_CAPH_CFIFO_FIFO_2,
	CSL_CAPH_CFIFO_FIFO_3,
	CSL_CAPH_CFIFO_FIFO_4,
	CSL_CAPH_CFIFO_FIFO_5,
	CSL_CAPH_CFIFO_FIFO_6,
	CSL_CAPH_CFIFO_FIFO_7,
	CSL_CAPH_CFIFO_FIFO_8,
	CSL_CAPH_CFIFO_FIFO_9,
	CSL_CAPH_CFIFO_FIFO_10,
	CSL_CAPH_CFIFO_FIFO_11,
	CSL_CAPH_CFIFO_FIFO_12,
	CSL_CAPH_CFIFO_FIFO_13,
	CSL_CAPH_CFIFO_FIFO_14,
	CSL_CAPH_CFIFO_FIFO_15,
	CSL_CAPH_CFIFO_FIFO_16,
	CSL_CAPH_FIFO_MAX_NUM
}CSL_CAPH_FIFO_e;

/**
* CAPH Path ID
******************************************************************************/
typedef UInt8 CSL_CAPH_PathID;



/**
* CAPH Audio Stream ID
******************************************************************************/
typedef enum
{
	CSL_CAPH_STREAM_NONE,
	CSL_CAPH_STREAM1,
	CSL_CAPH_STREAM2,
	CSL_CAPH_STREAM3,
	CSL_CAPH_STREAM4,
	CSL_CAPH_STREAM5,
	CSL_CAPH_STREAM6,
	CSL_CAPH_STREAM7,
	CSL_CAPH_STREAM8,
	CSL_CAPH_STREAM9,
	CSL_CAPH_STREAM10,
	CSL_CAPH_STREAM11,
	CSL_CAPH_STREAM12,
	CSL_CAPH_STREAM13,
	CSL_CAPH_STREAM14,
	CSL_CAPH_STREAM15,
	CSL_CAPH_STREAM16,
	CSL_CAPH_STREAM_TOTAL,
} CSL_CAPH_STREAM_e;

/**
* CAPH Devices
******************************************************************************/
typedef enum
{
	CSL_CAPH_DEV_NONE,
	CSL_CAPH_DEV_EP,  /*Earpiece*/
	CSL_CAPH_DEV_HS,  /*Headset speaker*/
	CSL_CAPH_DEV_IHF,  /*IHF speaker*/
	CSL_CAPH_DEV_VIBRA,  /*Vibra output*/
	CSL_CAPH_DEV_FM_TX,  /*FM TX broadcaster*/
	CSL_CAPH_DEV_BT_SPKR,  /*Bluetooth headset speaker*/
	CSL_CAPH_DEV_DSP,  /*DSP direct connection with SRCMixer in voice call*/
	CSL_CAPH_DEV_DIGI_MIC,  /*Two Digital microphones*/
//	CSL_CAPH_DEV_DIGI_MIC_L=CSL_CAPH_DEV_DIGI_MIC,  /*Digital microphones: left channel*/
	CSL_CAPH_DEV_DIGI_MIC_L,  /*Digital microphones: left channel*/
	CSL_CAPH_DEV_DIGI_MIC_R,  /*Digital microphones: right channel*/	
	CSL_CAPH_DEV_EANC_DIGI_MIC,  /*Two Noise Digital microphones for EANC control*/
	CSL_CAPH_DEV_EANC_DIGI_MIC_L,  /*ENAC digital microphones: left channel*/
	CSL_CAPH_DEV_EANC_DIGI_MIC_R,  /*ENAC digital microphones: right channel*/
	CSL_CAPH_DEV_SIDETONE_INPUT,  /*Sidetone path input*/
	CSL_CAPH_DEV_EANC_INPUT,  /*EANC Anti-noise input*/
	CSL_CAPH_DEV_ANALOG_MIC,  /*Phone analog mic*/
	CSL_CAPH_DEV_HS_MIC,  /*Headset mic*/
	CSL_CAPH_DEV_BT_MIC,  /*Bluetooth headset mic*/
	CSL_CAPH_DEV_FM_RADIO,  /*FM Radio playback*/
	CSL_CAPH_DEV_MEMORY,  /*DDR memory*/
	CSL_CAPH_DEV_SRCM,  /*SRCMixer*/
	CSL_CAPH_DEV_DSP_throughMEM,  /*DSP connection through shared mem*/
    CSL_CAPH_DEV_MAXNUM,
}CSL_CAPH_DEVICE_e;
#define CSL_AUDIO_DEVICE_e CSL_CAPH_DEVICE_e


/**
* CAPH HW gain. For tuning purpose only
******************************************************************************/
typedef enum
{
	CSL_CAPH_AMIC_PGA_GAIN,
	CSL_CAPH_AMIC_DGA_COARSE_GAIN,
	CSL_CAPH_AMIC_DGA_FINE_GAIN,
	CSL_CAPH_DMIC1_DGA_COARSE_GAIN,
	CSL_CAPH_DMIC1_DGA_FINE_GAIN,
	CSL_CAPH_DMIC2_DGA_COARSE_GAIN,
	CSL_CAPH_DMIC2_DGA_FINE_GAIN,
	CSL_CAPH_DMIC3_DGA_COARSE_GAIN,
	CSL_CAPH_DMIC3_DGA_FINE_GAIN,
	CSL_CAPH_DMIC4_DGA_COARSE_GAIN,
	CSL_CAPH_DMIC4_DGA_FINE_GAIN,
	CSL_CAPH_SRCM_INPUT_GAIN_L,
	CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L,
	CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L,
	CSL_CAPH_SRCM_INPUT_GAIN_R,
	CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R,
	CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R,
} CSL_CAPH_HW_GAIN_e;

/**
* CAPH Maximum number of HW Path
******************************************************************************/

#define MAX_AUDIO_PATH 32


/**
* CAPH Data format: Unpacked.
******************************************************************************/
#define DATA_UNPACKED	0

/**
* Voice Call UL/DL to/from DSP channel
******************************************************************************/
#define SPEAKER_DL_FROM_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH1
#define MAIN_MIC_UL_TO_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH2
#define EANC_MIC_UL_TO_DSP_CHNL  CSL_CAPH_SRCM_MONO_CH3

/****************************************************************************
*
*  Function Name: csl_caph_hwctrl_SetIHFmode
*
*  Description: Set IHF mode (stereo/mono)
*
****************************************************************************/
void csl_caph_hwctrl_SetIHFmode(Boolean stIHF);

/********************************************************************
*  @brief  Set IHF mode
*
*  @param  IHF mode status (TRUE: stereo | FALSE: mono).
*
*  @return  none
*
****************************************************************************/
Boolean csl_caph_hwctrl_GetIHFmode (void);

#endif // _CSL_CAPH_
