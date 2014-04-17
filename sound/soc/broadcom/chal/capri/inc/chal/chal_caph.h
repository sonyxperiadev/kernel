/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use
of this software, this software is licensed to you under the terms of the GNU General Public
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software in any way
with any other Broadcom software provided under a license other than the GPL, without
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   chal_caph.h
*
*   @brief  This file contains the const for caph CHA layer
*
****************************************************************************/

#ifndef _CHAL_CAPH_
#define _CHAL_CAPH_
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>

#define CAPH_SWITCH_CHNL_NUM    16 //added from Rhea cHAL

/**
* CAPH OWNER: ARM or DSP
******************************************************************************/
typedef enum
{
    CAPH_ARM,
    CAPH_DSP,
    CAPH_SSP //added for compilation
}CAPH_ARM_DSP_e;

/**
* CAPH CFIFO IDs
******************************************************************************/
typedef enum
{
    CAPH_CFIFO_VOID = 0x0000,
    CAPH_CFIFO1 = 0x0001,
    CAPH_CFIFO2 = 0x0002,
    CAPH_CFIFO3 = 0x0004,
    CAPH_CFIFO4 = 0x0008,
    CAPH_CFIFO5 = 0x0010,
    CAPH_CFIFO6 = 0x0020,
    CAPH_CFIFO7 = 0x0040,
    CAPH_CFIFO8 = 0x0080,
    CAPH_CFIFO9 = 0x0100,
    CAPH_CFIFO10 = 0x0200,
    CAPH_CFIFO11 = 0x0400,
    CAPH_CFIFO12 = 0x0800,
    CAPH_CFIFO13 = 0x1000,
    CAPH_CFIFO14 = 0x2000,
    CAPH_CFIFO15 = 0x4000,
    CAPH_CFIFO16 = 0x8000,
}CAPH_CFIFO_e;

/**
*  CAPH DMA CHANNEL IDs
******************************************************************************/
typedef enum
{
    CAPH_DMA_CH_VOID = 0x0000,
    CAPH_DMA_CH1 = 0x0001,
    CAPH_DMA_CH2 = 0x0002,
    CAPH_DMA_CH3 = 0x0004,
    CAPH_DMA_CH4 = 0x0008,
    CAPH_DMA_CH5 = 0x0010,
    CAPH_DMA_CH6 = 0x0020,
    CAPH_DMA_CH7 = 0x0040,
    CAPH_DMA_CH8 = 0x0080,
    CAPH_DMA_CH9 = 0x0100,
    CAPH_DMA_CH10 = 0x0200,
    CAPH_DMA_CH11 = 0x0400,
    CAPH_DMA_CH12 = 0x0800,
    CAPH_DMA_CH13 = 0x1000,
    CAPH_DMA_CH14 = 0x2000,
    CAPH_DMA_CH15 = 0x4000,
    CAPH_DMA_CH16 = 0x8000,
} CAPH_DMA_CHANNEL_e;

/**
*  CAPH SSP IDs
******************************************************************************/
typedef enum
{
    CAPH_SSP3 = 0x01,
    CAPH_SSP4 = 0x02,
} CAPH_SSP_e;

/**
*  CAPH AUDIOH FIFO IDs
******************************************************************************/
typedef enum
{
    CAPH_AUDIOH_VIBRA = 0x01,
    CAPH_AUDIOH_HS = 0x02,
    CAPH_AUDIOH_IHF = 0x04,
    CAPH_AUDIOH_EP = 0x08,
    CAPH_AUDIOH_EANC_MIC = 0x10,
    CAPH_AUDIOH_MIC = 0x20,
    CAPH_AUDIOH_ANTINOISE = 0x40,
} CAPH_AUDIOH_FIFO_e;

/**
*  CAPH SRC MIXER TAP CHANNEL  IDs
******************************************************************************/
typedef enum
{
    CAPH_SRCM_TAP_OUTCH1 = 0x01,
    CAPH_SRCM_TAP_OUTCH2 = 0x02,
    CAPH_SRCM_TAP_OUTCH3 = 0x04,
    CAPH_SRCM_TAP_OUTCH4 = 0x08,
    CAPH_SRCM_TAP_INCH1 = 0x10,
    CAPH_SRCM_TAP_INCH2 = 0x20,
    CAPH_SRCM_TAP_INCH3 = 0x40,
    CAPH_SRCM_TAP_INCH4 = 0x80,
} CAPH_SRCM_TAP_CHNL_e;

/**
*  CAPH CFIFO CHANNEL DIR
******************************************************************************/
typedef enum
{
    CAPH_CFIFO_IN = 0x01,
    CAPH_CFIFO_OUT = 0x00,
} CAPH_CFIFO_CHNL_DIRECTION_e;

/**
*  CAPH CFIFO QUEUE IDs
******************************************************************************/
typedef enum
{
    CAPH_CFIFO_QUEUE1 = 0x01,
    CAPH_CFIFO_QUEUE2 = 0x02,
    CAPH_CFIFO_QUEUE3 = 0x03,
} CAPH_CFIFO_QUEUE_e;

/**
*  CAPH CFIFO status
******************************************************************************/
typedef enum
{
    CAPH_CFIFO_ALMOST_EMPTY = 0x8000,
    CAPH_CFIFO_ALMOST_FULL  = 0x4000,
    CAPH_CFIFO_OVF          = 0x2000,
    CAPH_CFIFO_THR_MET      = 0x1000,
    CAPH_CFIFO_UDF          = 0x0800
} CAPH_CFIFO_FIFO_STATUS_e;

/**
*  CAPH DMA CHANNEL struct
******************************************************************************/
typedef struct
{
    CAPH_CFIFO_CHNL_DIRECTION_e direction;
    cUInt32 address;
    cUInt32 size;
    CAPH_CFIFO_e CFIFO_buf;
    cUInt8 Tsize;
} CAPH_DMA_CHNL_CFG_t;

/**
*  CAPH DMA CHANNEL FIFO status
******************************************************************************/
typedef enum
{
    CAPH_READY_NONE = 0x00,
    CAPH_READY_LOW = 0x01,
    CAPH_READY_HIGH =  0x02,
    CAPH_READY_HIGHLOW = 0x03
} CAPH_DMA_CHNL_FIFO_STATUS_e;

/**
*  CAPH SWITCH CHANNEL IDs
******************************************************************************/
typedef enum
{
    CAPH_SWITCH_CH_VOID = 0x0000,
    CAPH_SWITCH_CH1 = 0x0001,
    CAPH_SWITCH_CH2 = 0x0002,
    CAPH_SWITCH_CH3 = 0x0004,
    CAPH_SWITCH_CH4 = 0x0008,
    CAPH_SWITCH_CH5 = 0x0010,
    CAPH_SWITCH_CH6 = 0x0020,
    CAPH_SWITCH_CH7 = 0x0040,
    CAPH_SWITCH_CH8 = 0x0080,
    CAPH_SWITCH_CH9 = 0x0100,
    CAPH_SWITCH_CH10 = 0x0200,
    CAPH_SWITCH_CH11 = 0x0400,
    CAPH_SWITCH_CH12 = 0x0800,
    CAPH_SWITCH_CH13 = 0x1000,
    CAPH_SWITCH_CH14 = 0x2000,
    CAPH_SWITCH_CH15 = 0x4000,
    CAPH_SWITCH_CH16 = 0x8000,
} CAPH_SWITCH_CHNL_e;

/**
*  CAPH DST status
******************************************************************************/
typedef enum
{
    CAPH_DST_OK,        /* DST added to configuration. */
    CAPH_DST_NONE,    /* all 4 DSTs are used. No DST available. */
    CAPH_DST_USED,    /*  The same FIFO address already used */
} CAPH_DST_STATUS_e;

/**
*  CAPH DATA FORMAT
******************************************************************************/
typedef enum
{
    CAPH_MONO_16BIT,
    CAPH_MONO_24BIT,
    CAPH_STEREO_16BIT ,
    CAPH_STEREO_24BIT,
} CAPH_DATA_FORMAT_e;

/**
*  CAPH SWITCH TRIGGER types
******************************************************************************/
#if 0
typedef enum
{
    CAPH_VOID = 0x00,
    CAPH_4KHZ = 0x01,
    CAPH_8KHZ = 0x02,
    CAPH_12KHZ = 0x03,
    CAPH_16KHZ = 0x04,
    CAPH_24KHZ = 0x05,
    CAPH_32KHZ = 0x06,
    CAPH_48KHZ = 0x07,
    CAPH_96KHZ = 0x08,
    CAPH_11_025KHZ = 0x09,
    CAPH_22_5KHZ = 0x0A,
    CAPH_44_1KHZ = 0x0B,
    CAPH_EANC_FIFO_THRESMET = 0x10,
    CAPH_ADC_VOICE_FIFO_THR_MET = 0x11,
    CAPH_ADC_NOISE_FIFO_THR_MET = 0x12,
    CAPH_VB_THR_MET = 0x13,
    CAPH_HS_THR_MET = 0x14,
    CAPH_IHF_THR_MET =  0x15,
    CAPH_EP_THR_MET = 0x16,
    CAPH_SDT_THR_MET = 0x17,
    CAPH_ADC_VOICE_L_FIFO_THR_MET = 0x18,
    CAPH_ADC_NOISE_L_FIFO_THR_MET = 0x19,
    CAPH_MONORX0_REQ = 0x20,
    CAPH_MONORX1_REQ = 0x21,
    CAPH_MONOTX0_REQ = 0x22,
    CAPH_MONOTX1_REQ = 0x23,
    CAPH_STRORX0_REQ = 0x24,
    CAPH_STRORX1_REQ = 0x25,
    CAPH_STROTX0_REQ = 0x26,
    CAPH_STROTX1_REQ = 0x27,
    CAPH_SSP3_TRIGGER = 0x30,
    CAPH_SSP4_TRIGGER = 0x48,
    CAPH_PASSTHROUGH_CH2_FIFO_THRESMET = 0x50,
    CAPH_PASSTHROUGH_CH1_FIFO_THRESMET = 0x51,
    CAPH_TAPSDOWN_CH1_NORM_INT = 0x52,
    CAPH_TAPSDOWN_CH2_NORM_INT = 0x53,
    CAPH_TAPSDOWN_CH3_NORM_INT = 0x54,
    CAPH_TAPSDOWN_CH4_NORM_INT = 0x55,
    CAPH_TAPSUP_CH1_NORM_INT = 0x56,
    CAPH_TAPSUP_CH2_NORM_INT = 0x57,
    CAPH_TAPSUP_CH3_NORM_INT = 0x58,
    CAPH_TAPSUP_CH4_NORM_INT = 0x59,
    CAPH_TAPSUP_CH5_NORM_INT = 0x5A,
    CAPH_TAPSDOWN_CH5_THRESMET = 0x5B,
    CAPH_MIX1_OUT_THRESMET = 0x5C,
    CAPH_MIX2_OUT1_THRESMET = 0x5D,
    CAPH_MIX2_OUT2_THRESMET = 0x5E,

} CAPH_SWITCH_TRIGGER_e;
#endif

//added from Rhea cHAL
typedef enum
{
    CAPH_VOID = 0x00,
    CAPH_4KHZ = 0x01,
    CAPH_8KHZ = 0x02,
    CAPH_12KHZ = 0x03,
    CAPH_16KHZ = 0x04,
    CAPH_24KHZ = 0x05,
    CAPH_32KHZ = 0x06,
    CAPH_48KHZ = 0x07,
    CAPH_96KHZ = 0x08,
    CAPH_11_025KHZ = 0x09,
    CAPH_22_5KHZ = 0x0A,
    CAPH_44_1KHZ = 0x0B,
    CAPH_EANC_FIFO_THRESMET = 0x10,
    CAPH_ADC_VOICE_FIFOR_THR_MET = 0x11,
    CAPH_ADC_NOISE_FIFOR_THR_MET = 0x12,
    CAPH_VB_THR_MET = 0x13,
    CAPH_HS_THR_MET = 0x14,
    CAPH_IHF_THR_MET =  0x15,
    CAPH_EP_THR_MET = 0x16,
    CAPH_SDT_THR_MET = 0x17,
    CAPH_ADC_VOICE_FIFOL_THR_MET = 0x18,
    CAPH_ADC_NOISE_FIFOL_THR_MET = 0x19,    
    CAPH_MONORX0_REQ = 0x20,
    CAPH_MONORX1_REQ = 0x21,
    CAPH_MONOTX0_REQ = 0x22,
    CAPH_MONOTX1_REQ = 0x23,
    CAPH_STRORX0_REQ = 0x24,
    CAPH_STRORX1_REQ = 0x25,
    CAPH_STROTX0_REQ = 0x26,
    CAPH_STROTX1_REQ = 0x27,
    CAPH_SSP3_RX0_TRIGGER = 0x30,
    CAPH_SSP3_RX1_TRIGGER = 0x31,
    CAPH_SSP3_RX2_TRIGGER = 0x32,
    CAPH_SSP3_RX3_TRIGGER = 0x33,
    CAPH_SSP3_TX0_TRIGGER = 0x34,
    CAPH_SSP3_TX1_TRIGGER = 0x35,
    CAPH_SSP3_TX2_TRIGGER = 0x36,
    CAPH_SSP3_TX3_TRIGGER = 0x37,
    CAPH_SSP5_RX0_TRIGGER = 0x38,
    CAPH_SSP5_RX1_TRIGGER = 0x39,
    CAPH_SSP5_RX2_TRIGGER = 0x3A,
    CAPH_SSP5_RX3_TRIGGER = 0x3B,
    CAPH_SSP5_TX0_TRIGGER = 0x3C,
    CAPH_SSP5_TX1_TRIGGER = 0x3D,
    CAPH_SSP5_TX2_TRIGGER = 0x3E,
    CAPH_SSP5_TX3_TRIGGER = 0x3F,
    CAPH_SSP4_RX0_TRIGGER = 0x40,
    CAPH_SSP4_RX1_TRIGGER = 0x41,
    CAPH_SSP4_RX2_TRIGGER = 0x42,
    CAPH_SSP4_RX3_TRIGGER = 0x43,
    CAPH_SSP4_TX0_TRIGGER = 0x44,
    CAPH_SSP4_TX1_TRIGGER = 0x45,
    CAPH_SSP4_TX2_TRIGGER = 0x46,
    CAPH_SSP4_TX3_TRIGGER = 0x47,
    CAPH_SSP6_RX0_TRIGGER = 0x48,
    CAPH_SSP6_RX1_TRIGGER = 0x49,
    CAPH_SSP6_RX2_TRIGGER = 0x4A,
    CAPH_SSP6_RX3_TRIGGER = 0x4B,
    CAPH_SSP6_TX0_TRIGGER = 0x4C,
    CAPH_SSP6_TX1_TRIGGER = 0x4D,
    CAPH_SSP6_TX2_TRIGGER = 0x4E,
    CAPH_SSP6_TX3_TRIGGER = 0x4F,
    CAPH_PASSTHROUGH_CH1_FIFO_THRESMET = 0x50,
    CAPH_PASSTHROUGH_CH2_FIFO_THRESMET = 0x51,
    CAPH_TAPSDOWN_CH1_NORM_INT = 0x52,
    CAPH_TAPSDOWN_CH2_NORM_INT = 0x53,
    CAPH_TAPSDOWN_CH3_NORM_INT = 0x54,
    CAPH_TAPSDOWN_CH4_NORM_INT = 0x55,
    CAPH_TAPSUP_CH1_NORM_INT = 0x56,
    CAPH_TAPSUP_CH2_NORM_INT = 0x57,
    CAPH_TAPSUP_CH3_NORM_INT = 0x58,
    CAPH_TAPSUP_CH4_NORM_INT = 0x59,
    CAPH_TAPSUP_CH5_NORM_INT = 0x5A,
    CAPH_TAPSDOWN_CH5_NORM_INT = 0x5B,
    // mixer triggers
    CAPH_TRIG_MIX1_OUT_THR  =   0x5C,
    CAPH_TRIG_MIX2_OUT1_THR  =  0x5D,
    CAPH_TRIG_MIX2_OUT2_THR  =  0x5E,
    CAPH_PASSTHROUGH_CH3_FIFO_THRESMET = 0x60,
    CAPH_PASSTHROUGH_CH4_FIFO_THRESMET = 0x61
} CAPH_SWITCH_TRIGGER_e;

/**
*  CAPH SRC Mixer CHANNEL IDs
******************************************************************************/

/* Do not adjust the values of these channels.
 * Otherwise cHAL code will not select the right HW registers
 * Its sequence should follow the sequence of the
 * SRC_M1/2/D0/1_CH1/2/3/4/5/6/7M/L/R_GAIN_CTRL registers.
 */
typedef enum
{
    CAPH_SRCM_CH_NONE = 0x0000,
    CAPH_SRCM_CH1 = 0x0001,
    CAPH_SRCM_CH2 = 0x0002,
    CAPH_SRCM_CH3 = 0x0004,
    CAPH_SRCM_CH4 = 0x0008,
	CAPH_SRCM_CH5_L = 0x0010,
	CAPH_SRCM_CH5_R = 0x0020,
	CAPH_SRCM_CH5 = (CAPH_SRCM_CH5_L | CAPH_SRCM_CH5_R),
	CAPH_SRCM_PASSCH1_L = 0x0040,
	CAPH_SRCM_PASSCH1_R = 0x0080,
	CAPH_SRCM_PASSCH1 = (CAPH_SRCM_PASSCH1_L | CAPH_SRCM_PASSCH1_R),
	CAPH_SRCM_PASSCH2_L = 0x0100,
	CAPH_SRCM_PASSCH2_R = 0x0200,
	CAPH_SRCM_PASSCH2 = (CAPH_SRCM_PASSCH2_L | CAPH_SRCM_PASSCH2_R),
	/*not using the below two CHANNEL as of now
	 * They are for mono mode*/
	CAPH_SRCM_PASSCH1_M = CAPH_SRCM_PASSCH1_R,
	CAPH_SRCM_PASSCH2_M = CAPH_SRCM_PASSCH2_R,
	CAPH_SRCM_PASSCH3 = 0x0400,
	CAPH_SRCM_PASSCH4 = 0x0800
}CAPH_SRCMixer_CHNL_e;

/**
*  CAPH SRC Mixer SRC types
******************************************************************************/
typedef enum
{
    CAPH_8KHz_48KHz = 0x00,  /* 00: 8KHz up convert to 48KHz */
    CAPH_16KHz_48KHz = 0x01,  /* 01: 16KHz up convert to 48KHz */
    CAPH_48KHz_8KHz = 0x02,  /* 10: 48KHz down convert to 8KHz */
    CAPH_48KHz_16KHz = 0x03,  /* 11: 48KHz down convert to 16KHz */
    CAPH_44_1KHz_48KHz = 0x04,
}CAPH_SRCMixer_SRC_e;

/**
*  CAPH SRC Mixer FIFO IDs
******************************************************************************/
typedef enum
{
    CAPH_CH_INFIFO_NONE = 0x0000,
    CAPH_CH1_INFIFO = 0x0001,               /* Mono */
    CAPH_CH2_INFIFO = 0x0002,               /* Mono */
    CAPH_CH3_INFIFO = 0x0004,               /* Mono */
    CAPH_CH4_INFIFO = 0x0008,               /* Mono */
    CAPH_CH5_INFIFO = 0x0010,                   /* Left, Right interleaved */
    CAPH_PASSCH1_INFIFO = 0x0020,       /* Left, Right interleaved */
    CAPH_PASSCH2_INFIFO = 0x0040,       /* Left, Right interleaved */
    CAPH_PASSCH3_INFIFO = 0x0080,       /* mono */
    CAPH_PASSCH4_INFIFO  = 0x0100,       /* mono */
    CAPH_CH1_TAP_OUTFIFO = 0x0200,      /* Mono */
    CAPH_CH2_TAP_OUTFIFO = 0x0400,      /* Mono */
    CAPH_CH3_TAP_OUTFIFO = 0x0800,      /* Mono */
    CAPH_CH4_TAP_OUTFIFO = 0x1000,      /* Mono */
    CAPH_CH5_TAP_OUTFIFO = 0x2000,      /* Stereo */
    CAPH_MIXER1_OUTFIFO  = 0x4000,       /* Left, Right interleaved */
    CAPH_MIXER2_OUTFIFO  = 0x8000,       /* Left, Right interleaved */
    CAPH_MIXER2_OUTFIFO1 = 0x10000,      /* mono */
    CAPH_MIXER2_OUTFIFO2 = 0x20000,      /* mono */
}CAPH_SRCMixer_FIFO_e;



/**
*  CAPH SRC Mixer TAP OUTBUFF status
******************************************************************************/
typedef enum
{
    CAPH_TAP_NONE = 0x00,
    CAPH_TAP_UDF = 0x02,
    CAPH_TAP_OVF = 0x01,
} CAPH_SRCMixer_TAP_OUTBUFF_STATUS_e;

/**
*  CAPH SRC Mixer OUTPUT IDs
******************************************************************************/
typedef enum
{
    CAPH_M_NONE = 0x00,
    CAPH_M0_Left = 0x01,
    CAPH_M0_Right = 0x02,
    CAPH_M1_Left = 0x04,
    CAPH_M1_Right = 0x08,
} CAPH_SRCMixer_OUTPUT_e;

/**
*  CAPH SRC Mixer CHANNEL modes
******************************************************************************/
typedef enum
{
    CAPH_SRCM_STEREO,
    CAPH_SRCM_MONO,
} CAPH_SRCMixer_CHNL_MODE_e;

/**
*  CAPH SRC Mixer Filter Types
******************************************************************************/
typedef enum
{
    CAPH_SRCM_LINEAR_PHASE,
    CAPH_SRCM_MINIMUM_PHASE,
} CAPH_SRCMixer_FILT_TYPE_e;

/*added from rhea CHAL for compilation purpose */
typedef enum
{
    CAPH_AUDIOH_NONE_MIC_GAIN,
    CAPH_AUDIOH_MIC1_COARSE_GAIN,
    CAPH_AUDIOH_MIC1_FINE_GAIN,
    CAPH_AUDIOH_MIC2_COARSE_GAIN,
    CAPH_AUDIOH_MIC2_FINE_GAIN,
    CAPH_AUDIOH_MIC3_COARSE_GAIN,
    CAPH_AUDIOH_MIC3_FINE_GAIN,
    CAPH_AUDIOH_MIC4_COARSE_GAIN,
    CAPH_AUDIOH_MIC4_FINE_GAIN,
} CAPH_AUDIOH_MIC_GAIN_e;

#endif /* _CHAL_CAPH_ */

