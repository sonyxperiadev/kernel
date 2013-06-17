/**************************************************************************
 * Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.          */
/*                                                                        */
/*     Unless you and Broadcom execute a separate written software license*/
/*     agreement governing use of this software, this software is licensed*/
/*     to you under the terms of the GNU General Public License version 2 */
/*     (the GPL), available at                                            */
/*                                                                        */
/*     http://www.broadcom.com/licenses/GPLv2.php                         */
/*                                                                        */
/*     with the following added to such license:                          */
/*                                                                        */
/*     As a special exception, the copyright holders of this software give*/
/*     you permission to link this software with independent modules, and */
/*     to copy and distribute the resulting executable under terms of your*/
/*     choice, provided that you also meet, for each linked             */
/*     independent module, the terms and conditions of the license of that*/
/*     module.An independent module is a module which is not derived from */
/*     this software.  The special exception does not apply to any        */
/*     modifications of the software.                                     */
/*                                                                        */
/*     Notwithstanding the above, under no circumstances may you combine  */
/*     this software in any way with any other Broadcom software provided */
/*     under a license other than the GPL,                                */
/*     without Broadcom's express prior written consent.                  */
/*                                                                        */
/**************************************************************************/
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

/**
 * CAPH CFIFO FIFO buffers
 */
enum CSL_CAPH_CFIFO_FIFO_e {
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
};

#define CSL_CAPH_CFIFO_FIFO_e enum CSL_CAPH_CFIFO_FIFO_e


/**
 * CAPH AADMAC Channels
 */
enum CSL_CAPH_DMA_CHNL_e {
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
};

#define CSL_CAPH_DMA_CHNL_e enum CSL_CAPH_DMA_CHNL_e

/**
 * CAPH SSASW Channel
 */
enum CSL_CAPH_SWITCH_CHNL_e {
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
};

#define CSL_CAPH_SWITCH_CHNL_e enum CSL_CAPH_SWITCH_CHNL_e

/**
 * CAPH Data Format: Stereo or Mono, 16bit or 24bit
 */
enum CSL_CAPH_DATAFORMAT_e {
	CSL_CAPH_16BIT_MONO,
	CSL_CAPH_16BIT_STEREO,
	CSL_CAPH_24BIT_MONO,
	CSL_CAPH_24BIT_STEREO,
};

#define CSL_CAPH_DATAFORMAT_e enum CSL_CAPH_DATAFORMAT_e

/**
 * CAPH DMA OWNER: ARM or DSP
 */
enum CSL_CAPH_ARM_DSP_e {
	CSL_CAPH_ARM,
	CSL_CAPH_DSP,
};

#define CSL_CAPH_ARM_DSP_e enum CSL_CAPH_ARM_DSP_e

/**
 * CAPH Devices
 */
enum CSL_CAPH_DEVICE_e {
	CSL_CAPH_DEV_NONE,
	CSL_CAPH_DEV_EP,	/*Earpiece */
	CSL_CAPH_DEV_HS,	/*Headset speaker */
	CSL_CAPH_DEV_IHF,	/*IHF speaker */
	CSL_CAPH_DEV_VIBRA,	/*Vibra output */
	CSL_CAPH_DEV_FM_TX,	/*FM TX broadcaster */
	CSL_CAPH_DEV_BT_SPKR,	/*Bluetooth headset speaker */
	CSL_CAPH_DEV_DSP,	/*DSP direct connection with
				 *SRCMixer in voice call
				 */
	CSL_CAPH_DEV_DIGI_MIC,	/*Two Digital microphones */
	/* CSL_CAPH_DEV_DIGI_MIC_L=CSL_CAPH_DEV_DIGI_MIC,*/
	/*Digital microphones: left channel*/
	CSL_CAPH_DEV_DIGI_MIC_L,	/*Digital microphones: left channel */
	CSL_CAPH_DEV_DIGI_MIC_R,	/*Digital microphones: right channel */
	CSL_CAPH_DEV_EANC_DIGI_MIC,	/*Two Noise Digital microphones for
					 *EANC control
					 */
	CSL_CAPH_DEV_EANC_DIGI_MIC_L,	/*ENAC digital microphones:
					 *left channel
					 */
	CSL_CAPH_DEV_EANC_DIGI_MIC_R,	/*ENAC digital microphones:
					 *right channel
					 */
	CSL_CAPH_DEV_SIDETONE_INPUT,	/*Sidetone path input */
	CSL_CAPH_DEV_EANC_INPUT,	/*EANC Anti-noise input */
	CSL_CAPH_DEV_ANALOG_MIC,	/*Phone analog mic */
	CSL_CAPH_DEV_HS_MIC,	/*Headset mic */
	CSL_CAPH_DEV_BT_MIC,	/*Bluetooth headset mic */
	CSL_CAPH_DEV_FM_RADIO,	/*FM Radio playback */
	CSL_CAPH_DEV_MEMORY,	/*DDR memory */
	CSL_CAPH_DEV_DSP_throughMEM,	/*DSP connection through shared mem */
	CSL_CAPH_ECHO_REF_MIC, /* echo path mic in caph to dsp */
	CSL_CAPH_DEV_MAXNUM,
};

#define CSL_CAPH_DEVICE_e enum CSL_CAPH_DEVICE_e

enum CSL_CAPH_CFG_ARM2SP_e {
	CSL_CAPH_CFG_ARM2SP_FM,
	CSL_CAPH_CFG_ARM2SP_HQ,
};

#define CSL_CAPH_CFG_ARM2SP_e enum CSL_CAPH_CFG_ARM2SP_e

#endif /* _CSL_CAPH_ */
