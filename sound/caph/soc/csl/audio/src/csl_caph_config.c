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
*  @file   csl_caph_config.c
*
*  @brief  csl layer config data for caph driver
*
****************************************************************************/
#include "chal_caph.h"
#include "auddrv_def.h"
#include "csl_caph.h"


//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************
// always uses queue1
UInt32 cfifo_arb_key = 0x24924924;
CAPH_CFIFO_QUEUE_e cfifo_queue = CAPH_CFIFO_QUEUE1;

#if defined (_RHEA_)
CSL_CFIFO_TABLE_t CSL_CFIFO_table[]=
{
// FIFO map, address, size, threshold, owner: 0=ARM, 1=DSP, status: 0=UNUSED, 1=USED
// CFIFO/DMA 11-16 are reserved and connected for DSP
    {CSL_CAPH_CFIFO_NONE,   0x0000, 0x0,  0x0,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO1,  0x0000, 0x80, 0x8, 	CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO2,  0x0080, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO3,  0x0100, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO4,  0x0180, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO5,  0x0200, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO6,  0x0280, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO7,  0x0300, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO8,  0x0380, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO9,  0x0400, 0x80, 0x8,  CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO10, 0x0480, 0x200, 0x100,  CAPH_SSP, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO11, 0x0680, 0x200, 0x100,  CAPH_SSP, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO12, 0x0880, 0x200, 0x100, CAPH_DSP, 0, CSL_CAPH_DMA_CH12},
    {CSL_CAPH_CFIFO_FIFO13, 0x0A80, 0x200, 0x100, CAPH_DSP, 0, CSL_CAPH_DMA_CH13},
    {CSL_CAPH_CFIFO_FIFO14, 0x0C80, 0x4,    0x2,  CAPH_DSP, 0, CSL_CAPH_DMA_CH14},
    {CSL_CAPH_CFIFO_FIFO15, 0x0C84, 0x4,    0x2,  CAPH_DSP, 0, CSL_CAPH_DMA_CH15},
    {CSL_CAPH_CFIFO_FIFO16, 0x0C88, 0x4,    0x2,  CAPH_DSP, 0, CSL_CAPH_DMA_CH16}
};
#elif defined (_SAMOA_)
CSL_CFIFO_TABLE_t CSL_CFIFO_table[]=
{
// FIFO map, address, size in bytes, threshold, owner: 0=ARM, 1=DSP, status: 0=UNUSED, 1=USED
// CFIFO/DMA 11-16 are reserved and connected for DSP
    {CSL_CAPH_CFIFO_NONE,   0x0000, 0x0,     0x0,    CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO1,   0x0000, 0x200, 0x100, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO2,   0x0200, 0x200, 0x100, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO3,   0x0400, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO4,   0x0500, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO5,   0x0600, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO6,   0x0700, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO7,   0x0800, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO8,   0x0900, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO9,   0x0a00, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO10, 0x0b00, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO11, 0x0c00, 0x100, 0x080, CAPH_ARM, 0, CSL_CAPH_DMA_NONE},
    {CSL_CAPH_CFIFO_FIFO12, 0x0d00, 0x100, 0x080, CAPH_DSP, 0, CSL_CAPH_DMA_CH12},
    {CSL_CAPH_CFIFO_FIFO13, 0x0e00, 0x100, 0x080, CAPH_DSP, 0, CSL_CAPH_DMA_CH13},
    {CSL_CAPH_CFIFO_FIFO14, 0x0f00, 0x20,    0x10,    CAPH_DSP, 0, CSL_CAPH_DMA_CH14},
    {CSL_CAPH_CFIFO_FIFO15, 0x0f20, 0x20,    0x10,    CAPH_DSP, 0, CSL_CAPH_DMA_CH15},
    {CSL_CAPH_CFIFO_FIFO16, 0x0f40, 0x20,    0x10,    CAPH_DSP, 0, CSL_CAPH_DMA_CH16}
};
#endif

/* Mixer input gain ramping setting */
UInt16 srcmixer_input_gain_ramp = 0x0000;
/* Mixer output gain slope setting */
UInt16 srcmixer_output_gain_slope = 0x0000;
/* FIFO Threshold2 */
UInt8 srcmixer_fifo_thres2 = 0;



#if defined(USE_SYSPARM_FILE)  
#else


const unsigned int eancAIIRFilterCoeff[] = {
    /* AIIR Coefficients */
    0xd97f9b ,
    0x02b858 ,
    0x437495 ,
    0x0011a1 ,
    0x02b858 ,

    0xc33f30 ,
    0x7fffff ,
    0x3caef9 ,
    0xa47395 ,
    0x7fffff ,

    0xe7c868 ,
    0x0bee0c ,
    0x498cfc ,
    0x11e06c ,
    0x0bee0c ,

    0xcbc082 ,
    0x7fffff ,
    0x3e2bf2 ,
    0xba3b71 ,
    0x7fffff
};

const unsigned int eancAFIRFilterCoeff[2][10] = {
    /* AFIR Coefficients */
    {0x006b01f0,    0xf4c5f9ef,    0x1ed7ff08,    0x56754518,    0x1ed74518,
    0xf4c5ff08,    0x006bf9ef,    0x000001f0,    0x00000000 ,    0x00000000},

    {0x008901d2,    0xf4e3f9ec,     0x1ed7ff0b,     0x56754518,     0x1ed74518,
    0xf4c5ff08,  0x006bf9ef,     0x000001f0,     0x00000000,     0x00000000}
};


const unsigned int eanc96kIIRFilterCoeff[] =
{
    0x000000  ,
    0x000000  ,
    0x2ef74e  ,
    0x02514d  ,
    0x02514d  ,
    0xc81f2f ,
    0x237811 ,
    0x662535 ,
    0xdab293 ,
    0x237811 ,
    0xd68f5a ,
    0x568790 ,
    0x5fd504 ,
    0xe7dc24 ,
    0x568790 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x7fffff ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x7fffff ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x7fffff
};


const unsigned int eanc48kIIRFilterCoeff[] =
{
    0xCBD48C ,
    0x0A292D ,
    0x6BE454 ,
    0xEF0B30 ,
    0x0A292D ,
    0x000000 ,
    0x000000 ,
    0x2C9707 ,
    0x5F7C41,
    0x5F7C41,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x7fffff ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x7fffff ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x7fffff ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x000000 ,
    0x7fffff
};

////////////////// Side tone filter coeffient //////////////


const unsigned int stoneFirCoeff[] =
{
	0x000003c4,
    0xfffffdae,
    0xfffffa74,
    0xfffff504,
    0xffffed64,
    0xffffe3d2,
    0xffffd8d8,
    0xffffcd63,
    0xffffc2bd,
    0xffffba76,
    0xffffb640,
    0xffffb7be,
    0xffffc041,
    0xffffd08a,
    0xffffe88a,
    0x00000736,
    0x00002a76,
    0x00004f2e,
    0x0000716c,
    0x00008cc6,
    0x00009cce,
    0x00009da3,
    0x00008c81,
    0x00006857,
    0x0000322e,
    0xffffed64,
    0xffff9fa9,
    0xffff50ac,
    0xffff0983,
    0xfffed3c2,
    0xfffeb86b,
    0xfffebec4,
    0xfffeeb30,
    0xffff3e40,
    0xffffb40e,
    0x0000440e,
    0x0000e162,
    0x00017bbb,
    0x000200bd,
    0x00025dd8,
    0x00028257,
    0x0002618e,
    0x0001f4d5,
    0x00013d1a,
    0x000043db,
    0xffff1b4f,
    0xfffddda8,
    0xfffcab83,
    0xfffba96b,
    0xfffafcce,
    0xfffac88c,
    0xfffb296f,
    0xfffc32f3,
    0xfffdecad,
    0x00005094,
    0x00034a76,
    0x0006b8a6,
    0x000a6deb,
    0x000e3490,
    0x0011d257,
    0x00150cf4,
    0x0017aeb2,
    0x00198ad3,
    0x001a8143,
    0x001a8143,
    0x00198ad3,
    0x0017aeb2,
    0x00150cf4,
    0x0011d257,
    0x000e3490,
    0x000a6deb,
    0x0006b8a6,
    0x00034a76,
    0x00005094,
    0xfffdecad,
    0xfffc32f3,
    0xfffb296f,
    0xfffac88c,
    0xfffafcce,
    0xfffba96b,
    0xfffcab83,
    0xfffddda8,
    0xffff1b4f,
    0x000043db,
    0x00013d1a,
    0x0001f4d5,
    0x0002618e,
    0x00028257,
    0x00025dd8,
    0x000200bd,
    0x00017bbb,
    0x0000e162,
    0x0000440e,
    0xffffb40e,
    0xffff3e40,
    0xfffeeb30,
    0xfffebec4,
    0xfffeb86b,
    0xfffed3c2,
    0xffff0983,
    0xffff50ac,
    0xffff9fa9,
    0xffffed64,
    0x0000322e,
    0x00006857,
    0x00008c81,
    0x00009da3,
    0x00009cce,
    0x00008cc6,
    0x0000716c,
    0x00004f2e,
    0x00002a76,
    0x00000736,
    0xffffe88a,
    0xffffd08a,
    0xffffc041,
    0xffffb7be,
    0xffffb640,
    0xffffba76,
    0xffffc2bd,
    0xffffcd63,
    0xffffd8d8,
    0xffffe3d2,
    0xffffed64,
    0xfffff504,
    0xfffffa74,
    0xfffffdae,
    0x000003c4
};

#endif

#undef N
#define N CSL_CAPH_DMA_NONE

const CSL_CAPH_HWConfig_DMA_t HWConfig_DMA_Table[CSL_CAPH_DEV_MAXNUM][CSL_CAPH_DEV_MAXNUM] =
{
    /* Source: 
     *  NONE,    EP,      HS,      IHF,     VIBRA,   FMTX,    BTSPKR,  DSP,     DMIC,    DMIC_L,  DMIC_R,  EANCMIC, EANCMICL,EANCMICR,SDT_IN,  EANC_IN, AMIC,    HS_MIC,  BT_MIC,  FMRADIO, MEMORY,  SRCM,   DSP_throughMEM*/
/* Sink: */        
/* NONE */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* EP */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {0,N,N}, {0,N,N}
    },
/* HS */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {1,N,N}, {0,N,N}, {0,N,N}
    },
/* IHF */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N},	{0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {1,N,N}, {0,N,N}, {1,CSL_CAPH_DMA_CH12,N}
    },
/* VIBRA */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {0,N,N}, {0,N,N}
    },
/* FMTX */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {0,N,N}, {0,N,N}
    },
/* BT_SPKR */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {0,N,N}, {0,N,N}
    },
/* DSP */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {0,N,N}, {0,N,N}
    },
/* DIGI_MIC */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* DIGI_MIC_L */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },    
/* DIGI_MIC_R */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },        
/*EANC_DIGI_MIC */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/*EANC_DIGI_MIC_L */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },    
/*EANC_DIGI_MIC_R */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },        
/*SIDETONE_INPUT*/
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* EANC_INPUT */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* ANALOG_MIC */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* HS_MIC */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* BT_MIC */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* FM_RADIO */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    },
/* MEMORY */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {1,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {1,N,N}, {1,N,N}, {1,N,N}, {0,N,N}, {1,N,N}, {0,N,N}
    },
/* SRCM */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {1,N,N}, {0,N,N}, {0,N,N}
    },
/* DSP_throughMEM */
    {
        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {2,CSL_CAPH_DMA_CH13,CSL_CAPH_DMA_CH14},
                                                                                         {1,CSL_CAPH_DMA_CH13,N}, 
                                                                                                  {1,N,CSL_CAPH_DMA_CH14}, 
                                                                                                           {2,CSL_CAPH_DMA_CH13,CSL_CAPH_DMA_CH15}, 
                                                                                                                    {1,CSL_CAPH_DMA_CH13,N}, 
                                                                                                                             {1,N,CSL_CAPH_DMA_CH15}, 
                                                                                                                                      {0,N,N}, {1,CSL_CAPH_DMA_CH16,N}, 
                                                                                                                                                        {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}, {0,N,N}
    }    
};


//****************************************************************************
// global variable definitions
//****************************************************************************


//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// local macro declarations
//****************************************************************************


//****************************************************************************
// local typedef declarations
//****************************************************************************



//****************************************************************************
// local variable definitions
//****************************************************************************


//****************************************************************************
// local function declarations
//****************************************************************************



//******************************************************************************
// local function definitions
//******************************************************************************
