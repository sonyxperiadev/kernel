/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

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
// total ring buffer size for cfifo. make sure sync with cfifo config table 
#if defined (_RHEA_)
#define CSL_CFIFO_TOTAL_SIZE 0x2000
#elif defined (_SAMOA_)
#define CSL_CFIFO_TOTAL_SIZE 0x1000
#endif
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
* CAPH OWNER: ARM or DSP
******************************************************************************/
typedef enum
{
    CSL_CAPH_ARM,
    CSL_CAPH_DSP,
}CSL_CAPH_ARM_DSP_e;
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
* CAPH CFIFO Data Sample Rate
******************************************************************************/
typedef enum
{
    CSL_CAPH_SRCM_UNDEFINED,
    CSL_CAPH_SRCM_8KHZ,
    CSL_CAPH_SRCM_16KHZ,
    CSL_CAPH_SRCM_48KHZ,
}CSL_CAPH_CFIFO_SAMPLERATE_e;

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
* CAPH CFIFO FIFO buffer direction: IN: DDR->CFIFO, OUT: CFIFO->DDR
******************************************************************************/
typedef enum
{
	CSL_CAPH_CFIFO_OUT = 0x00,
	CSL_CAPH_CFIFO_IN = 0x01,
}CSL_CAPH_CFIFO_DIRECTION_e;

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
* CAPH AADMAC Channels
******************************************************************************/
typedef struct
{
    CSL_CAPH_DMA_CHNL_e dmaCH;
    CSL_CAPH_DMA_CHNL_e dmaCH2;
}CSL_CAPH_DMA_CHNL_t;

/**
* CAPH AADMAC Channel direction: IN: DDR->CFIFO, OUT: CFIFO->DDR
******************************************************************************/
typedef enum
{
	CSL_CAPH_DMA_IN,
	CSL_CAPH_DMA_OUT,
}CSL_CAPH_DMA_DIRECTION_e;

/**
* CAPH DMA Callback function
******************************************************************************/
typedef void (*CSL_CAPH_DMA_CALLBACK_p)(CSL_CAPH_DMA_CHNL_e chnl);

/**
* CAPH AADMAC Channel configuration parameter
******************************************************************************/
typedef struct
{
    CSL_CAPH_DMA_DIRECTION_e direction;
    CSL_CAPH_CFIFO_FIFO_e fifo;
    CSL_CAPH_DMA_CHNL_e dma_ch;
    UInt8* mem_addr;
    UInt32 mem_size;
    UInt8 Tsize;
    CSL_CAPH_DMA_CALLBACK_p dmaCB;
}CSL_CAPH_DMA_CONFIG_t;

/**
*  CSL CAPH DMA CHANNEL FIFO status
******************************************************************************/
typedef enum
{
    CSL_CAPH_READY_NONE = 0x00,
    CSL_CAPH_READY_LOW = 0x01,
    CSL_CAPH_READY_HIGH =  0x02,
    CSL_CAPH_READY_HIGHLOW = 0x03
} CSL_CAPH_DMA_CHNL_FIFO_STATUS_e;

/**
* CAPH AADMAC Channel interrupt
******************************************************************************/
typedef enum
{
	CSL_CAPH_DMA_INT1 = 0x0001,
	CSL_CAPH_DMA_INT2 = 0x0002,
	CSL_CAPH_DMA_INT3 = 0x0004,
	CSL_CAPH_DMA_INT4 = 0x0008,
	CSL_CAPH_DMA_INT5 = 0x0010,
	CSL_CAPH_DMA_INT6 = 0x0020,
	CSL_CAPH_DMA_INT7 = 0x0040,
	CSL_CAPH_DMA_INT8 = 0x0080,
	CSL_CAPH_DMA_INT9 = 0x0100,
	CSL_CAPH_DMA_INT10 = 0x0200,
	CSL_CAPH_DMA_INT11 = 0x0400,
	CSL_CAPH_DMA_INT12 = 0x0800,
	CSL_CAPH_DMA_INT13 = 0x1000,
	CSL_CAPH_DMA_INT14 = 0x2000,
	CSL_CAPH_DMA_INT15 = 0x4000,
	CSL_CAPH_DMA_INT16 = 0x8000,
}CSL_CAPH_DMA_INT_e;

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
* CAPH SSASW trigger
******************************************************************************/
typedef enum
{
    CSL_CAPH_TRIG_NONE = 0x00,
    CSL_CAPH_TRIG_EANC_FIFO_THRESMET = 0x10,
    CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET = 0x11,
    CSL_CAPH_TRIG_ADC_NOISE_FIFOR_THR_MET = 0x12,
    CSL_CAPH_TRIG_VB_THR_MET = 0x13,
    CSL_CAPH_TRIG_HS_THR_MET = 0x14,
    CSL_CAPH_TRIG_IHF_THR_MET =  0x15,
    CSL_CAPH_TRIG_EP_THR_MET = 0x16,
    CSL_CAPH_TRIG_SDT_THR_MET = 0x17,
    CSL_CAPH_TRIG_ADC_VOICE_FIFOL_THR_MET = 0x18,
    CSL_CAPH_TRIG_ADC_NOISE_FIFOL_THR_MET = 0x19,
    CSL_CAPH_TRIG_SSP3_RX0 = 0x30,
    CSL_CAPH_TRIG_SSP3_RX1 = 0x31,
    CSL_CAPH_TRIG_SSP3_RX2 = 0x32,
    CSL_CAPH_TRIG_SSP3_RX3 = 0x33,
    CSL_CAPH_TRIG_SSP3_TX0 = 0x34,
    CSL_CAPH_TRIG_SSP3_TX1 = 0x35,
    CSL_CAPH_TRIG_SSP3_TX2 = 0x36,
    CSL_CAPH_TRIG_SSP3_TX3 = 0x37,
    CSL_CAPH_TRIG_SSP4_RX0 = 0x40,
    CSL_CAPH_TRIG_SSP4_RX1 = 0x41,
    CSL_CAPH_TRIG_SSP4_RX2 = 0x42,
    CSL_CAPH_TRIG_SSP4_RX3 = 0x43,
    CSL_CAPH_TRIG_SSP4_TX0 = 0x44,
    CSL_CAPH_TRIG_SSP4_TX1 = 0x45,
    CSL_CAPH_TRIG_SSP4_TX2 = 0x46,
    CSL_CAPH_TRIG_SSP4_TX3 = 0x47,    
    CSL_CAPH_TRIG_PASSTHROUGH_CH1_FIFO_THRESMET = 0x50,
    CSL_CAPH_TRIG_PASSTHROUGH_CH2_FIFO_THRESMET = 0x51,
    CSL_CAPH_TRIG_TAPSDOWN_CH1_NORM_INT = 0x52,
    CSL_CAPH_TRIG_TAPSDOWN_CH2_NORM_INT = 0x53,
    CSL_CAPH_TRIG_TAPSDOWN_CH3_NORM_INT = 0x54,
    CSL_CAPH_TRIG_TAPSDOWN_CH4_NORM_INT = 0x55,
    CSL_CAPH_TRIG_TAPSUP_CH1_NORM_INT = 0x56,
    CSL_CAPH_TRIG_TAPSUP_CH2_NORM_INT = 0x57,
    CSL_CAPH_TRIG_TAPSUP_CH3_NORM_INT = 0x58,
    CSL_CAPH_TRIG_TAPSUP_CH4_NORM_INT = 0x59,
    CSL_CAPH_TRIG_TAPSUP_CH5_NORM_INT = 0x5A,
    CSL_CAPH_TRIG_TAPSDOWN_CH5_NORM_INT = 0x5B,
    // mixer triggers
    CSL_CAPH_TRIG_MIX1_OUT_THR  =   0x5C,
    CSL_CAPH_TRIG_MIX2_OUT1_THR  =  0x5D,
    CSL_CAPH_TRIG_MIX2_OUT2_THR  =  0x5E
}CSL_CAPH_SWITCH_TRIGGER_e;



/**
* CAPH SWITCH channel configuration parameter
******************************************************************************/
typedef enum 
{
    CSL_CAPH_SWITCH_OWNER,
    CSL_CAPH_SWITCH_BORROWER
}CSL_CAPH_SWITCH_STATUS_e;


/**
* CAPH SWITCH channel configuration parameter
******************************************************************************/
typedef struct
{
    CSL_CAPH_SWITCH_CHNL_e chnl;
    UInt32 FIFO_srcAddr;
    UInt32 FIFO_dstAddr;
    UInt32 FIFO_dst2Addr;
    UInt32 FIFO_dst3Addr;
    UInt32 FIFO_dst4Addr;
    CSL_CAPH_DATAFORMAT_e dataFmt;
    CSL_CAPH_SWITCH_TRIGGER_e trigger;
    CSL_CAPH_SWITCH_STATUS_e status;
}CSL_CAPH_SWITCH_CONFIG_t;


/**
* CAPH SRCMixer Mixer gains
******************************************************************************/
typedef struct CSL_CAPH_SRCM_MIX_GAIN_t
{
	UInt16 mixInGainL;
	UInt16 mixInGainR;
	UInt16 mixOutGainL;
	UInt16 mixOutGainR;
	UInt16 mixOutCoarseGainL;
	UInt16 mixOutCoarseGainR;
}CSL_CAPH_SRCM_MIX_GAIN_t;

/**
* CAPH SRCMixer input channel sample rate
******************************************************************************/
typedef enum
{
	CSL_CAPH_SRCMIN_8KHZ,
	CSL_CAPH_SRCMIN_16KHZ,
	CSL_CAPH_SRCMIN_44_1KHZ,
	CSL_CAPH_SRCMIN_48KHZ,
}CSL_CAPH_SRCM_INSAMPLERATE_e;

/**
* CAPH SRCMixer input channel
******************************************************************************/
typedef enum
{
	CSL_CAPH_SRCM_INCHNL_NONE = 0x0000,
    CSL_CAPH_SRCM_MONO_CH1 = 0x0001,
	CSL_CAPH_SRCM_MONO_CH2 = 0x0002,
	CSL_CAPH_SRCM_MONO_CH3 = 0x0004,
	CSL_CAPH_SRCM_MONO_CH4 = 0x0008,
	CSL_CAPH_SRCM_STEREO_PASS_CH1_L = 0x0010,
	CSL_CAPH_SRCM_STEREO_PASS_CH1_R = 0x0020,
	CSL_CAPH_SRCM_STEREO_PASS_CH2_L = 0x0040,
	CSL_CAPH_SRCM_STEREO_PASS_CH2_R = 0x0080,
	CSL_CAPH_SRCM_STEREO_CH5_L = 0x0100,
	CSL_CAPH_SRCM_STEREO_CH5_R = 0x0200,    
    CSL_CAPH_SRCM_MONO_CH = (CSL_CAPH_SRCM_MONO_CH1
                            |CSL_CAPH_SRCM_MONO_CH2
                            |CSL_CAPH_SRCM_MONO_CH3
                            |CSL_CAPH_SRCM_MONO_CH4),
    CSL_CAPH_SRCM_STEREO_PASS_CH1 = (CSL_CAPH_SRCM_STEREO_PASS_CH1_L
                                    |CSL_CAPH_SRCM_STEREO_PASS_CH1_R),
	CSL_CAPH_SRCM_STEREO_PASS_CH2 = (CSL_CAPH_SRCM_STEREO_PASS_CH2_L
                                    |CSL_CAPH_SRCM_STEREO_PASS_CH2_R),
	CSL_CAPH_SRCM_STEREO_PASS_CH = (CSL_CAPH_SRCM_STEREO_PASS_CH1
                                    |CSL_CAPH_SRCM_STEREO_PASS_CH2),    
	CSL_CAPH_SRCM_STEREO_CH5 = (CSL_CAPH_SRCM_STEREO_CH5_L
                                |CSL_CAPH_SRCM_STEREO_CH5_R),
}CSL_CAPH_SRCM_INCHNL_e;

/**
* CAPH SRCMixer TAP outnput channel
******************************************************************************/
typedef enum
{
	CSL_CAPH_SRCM_TAP_CH_NONE,
	CSL_CAPH_SRCM_TAP_MONO_CH1,
	CSL_CAPH_SRCM_TAP_MONO_CH2,
	CSL_CAPH_SRCM_TAP_MONO_CH3,
	CSL_CAPH_SRCM_TAP_MONO_CH4,
	CSL_CAPH_SRCM_TAP_STEREO_CH5,
}CSL_CAPH_SRCM_SRC_OUTCHNL_e;

/**
* CAPH SRCMixer output channel
******************************************************************************/
typedef enum
{
    CSL_CAPH_SRCM_CH_NONE = 0x00,
	CSL_CAPH_SRCM_STEREO_CH1_L = 0x01,
    CSL_CAPH_SRCM_STEREO_CH1_R = 0x02,
	CSL_CAPH_SRCM_STEREO_CH1 = (CSL_CAPH_SRCM_STEREO_CH1_L|CSL_CAPH_SRCM_STEREO_CH1_R),
    CSL_CAPH_SRCM_STEREO_CH2_L = 0x04,
	CSL_CAPH_SRCM_STEREO_CH2_R = 0x08,
}CSL_CAPH_SRCM_MIX_OUTCHNL_e;

/**
* CAPH SRCMixer output channel sample rate
******************************************************************************/
typedef enum
{
	CSL_CAPH_SRCMOUT_8KHZ,
	CSL_CAPH_SRCMOUT_16KHZ,
	CSL_CAPH_SRCMOUT_48KHZ,
}CSL_CAPH_SRCM_OUTSAMPLERATE_e;

/**
* CAPH SRCMixer SRC/Mixing Route Configuration Parameters
******************************************************************************/
typedef struct 
{
	CSL_CAPH_SRCM_INCHNL_e inChnl;
	UInt8 inThres;
	CSL_CAPH_SRCM_INSAMPLERATE_e inSampleRate;
	CSL_CAPH_DATAFORMAT_e inDataFmt;
	CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl;
	CSL_CAPH_SRCM_SRC_OUTCHNL_e tapOutChnl;
	UInt8 outThres;
	CSL_CAPH_SRCM_OUTSAMPLERATE_e outSampleRate;
	CSL_CAPH_DATAFORMAT_e outDataFmt;
	CSL_CAPH_SRCM_MIX_GAIN_t mixGain;
}CSL_CAPH_SRCM_ROUTE_t;

/**
* CAPH SRCMixer FIFO buffer
******************************************************************************/
typedef enum
{
    CSL_CAPH_SRCM_FIFO_NONE,
    CSL_CAPH_SRCM_MONO_CH1_FIFO,
    CSL_CAPH_SRCM_MONO_CH2_FIFO,
    CSL_CAPH_SRCM_MONO_CH3_FIFO,
    CSL_CAPH_SRCM_MONO_CH4_FIFO,
    CSL_CAPH_SRCM_STEREO_PASS_CH1_FIFO,
    CSL_CAPH_SRCM_STEREO_PASS_CH2_FIFO,
    CSL_CAPH_SRCM_STEREO_CH5_FIFO,
    CSL_CAPH_SRCM_TAP_MONO_CH1_FIFO,
    CSL_CAPH_SRCM_TAP_MONO_CH2_FIFO,
    CSL_CAPH_SRCM_TAP_MONO_CH3_FIFO,
    CSL_CAPH_SRCM_TAP_MONO_CH4_FIFO,
    CSL_CAPH_SRCM_TAP_STEREO_CH5_FIFO,
    CSL_CAPH_SRCM_STEREO_CH1_FIFO,
    CSL_CAPH_SRCM_STEREO_CH2_L_FIFO,
    CSL_CAPH_SRCM_STEREO_CH2_R_FIFO,
}CSL_CAPH_SRCM_FIFO_e;

/**
* CAPH CFIFO FIFO buffer table
******************************************************************************/
typedef struct 
{
    CSL_CAPH_CFIFO_FIFO_e fifo;
    UInt16 address;	
    UInt16 size;
    UInt16 threshold;
    UInt8 owner;	
    UInt8 status;
    CSL_CAPH_DMA_CHNL_e dmaCH;	
}CSL_CFIFO_TABLE_t;

/**
* CAPH Render/Capture CSL configuration parameters
******************************************************************************/
typedef struct
{
CSL_CAPH_DMA_CHNL_e dmaCH;
CSL_CAPH_CFIFO_FIFO_e fifo;
UInt8* pBuf;
UInt32 size;
CSL_CAPH_DMA_CALLBACK_p dmaCB;
}CSL_CAPH_STREAM_CONFIG_t;


/**
*  CAPH AUDIOH Path
******************************************************************************/

typedef enum 
{
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
* CAPH HW filters
******************************************************************************/
typedef enum
{
    CSL_CAPH_EANC_FILTER1, 
    CSL_CAPH_EANC_FILTER2, 
    CSL_CAPH_SIDETONE_FILTER, 
}CSL_CAPH_HWCTRL_FILTER_e;

/**
* CAPH Audio Information for Render/Capture Driver
******************************************************************************/
//typedef struct
//{
//    CSL_CAPH_CFIFO_FIFO_e fifo;
//}CSL_CAPH_HWCTRL_INFO_t;


#ifdef CONFIG_AUDIO_BUILD
/**
* CAPH Audio PathID for audio path. Used by Audio Controller
******************************************************************************/
typedef UInt8 CSL_CAPH_PathID;
#endif

/**
* CAPH HW path configuration parameters
******************************************************************************/
typedef struct
{
    CSL_CAPH_STREAM_e streamID;
    CSL_CAPH_PathID pathID;    
    CSL_CAPH_DEVICE_e source;
    CSL_CAPH_DEVICE_e sink;
    CSL_CAPH_DMA_CHNL_e dmaCH;
    CSL_CAPH_DMA_CHNL_e dmaCH2;
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;	
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    CSL_CAPH_SRCM_MIX_GAIN_t mixGain;
}CSL_CAPH_HWCTRL_CONFIG_t;


/**
* CAPH HW path configuration parameters
******************************************************************************/
typedef struct
{
    CSL_CAPH_STREAM_e streamID;
    AUDIO_SAMPLING_RATE_t src_sampleRate;
    AUDIO_SAMPLING_RATE_t snk_sampleRate;
    AUDIO_CHANNEL_NUM_t chnlNum;
    AUDIO_BITS_PER_SAMPLE_t bitPerSample;
    UInt8* pBuf;
    UInt8* pBuf2;
    UInt32 size;
    CSL_CAPH_DMA_CALLBACK_p dmaCB;    
}CSL_CAPH_HWCTRL_STREAM_REGISTER_t;


/**
* CAPH HW register base address
******************************************************************************/
typedef struct
{
    UInt32 audioh_baseAddr;
    UInt32 sdt_baseAddr;
    UInt32 srcmixer_baseAddr;
    UInt32 cfifo_baseAddr;
    UInt32 aadmac_baseAddr;
    UInt32 ssasw_baseAddr;
    UInt32 ahintc_baseAddr;
    UInt32 ssp3_baseAddr;
    UInt32 ssp4_baseAddr;	
}CSL_CAPH_HWCTRL_BASE_ADDR_t;

/**
* CAPH HW DMA channel Configuration for different audio HW paths.
******************************************************************************/
typedef struct
{
    UInt8 dmaNum; // 0 <= dmaNum <= 2.
    CSL_CAPH_DMA_CHNL_e dmaCH;
    CSL_CAPH_DMA_CHNL_e dmaCH2;
}CSL_CAPH_HWConfig_DMA_t;

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

#endif // _CSL_CAPH_
