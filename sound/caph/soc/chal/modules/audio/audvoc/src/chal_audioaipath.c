/*******************************************************************************************
Copyright 2009 - 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
* @file   chal_audioaipath.c
* @brief  Audio cHAL interface for audio input path
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "audio_consts.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_dsp_audio.h"
#include "brcm_rdb_util.h"
#include "chal_types.h"
#include "chal_audiomisc.h"
#include "chal_audioaipath.h"
#include "log.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    cUInt32 base;         // may be mapped and != AHB_AUDIO_BASE_ADDR
	cUInt32 inited;
} chal_audio_aipath_t;

// rely on zero inited
static chal_audio_aipath_t sAIPath = {AHB_AUDIO_BASE_ADDR, 0};   // Single block

const static UInt16   HqAdcIIRCoeff[50] = {
    //=========================================
    //==== write coeff in for normal case test
    //Biquad 1
    0x7438,    //A2 -3016
    0x1b61,    //A1  7009
    0x0785,    //B2  1925
    0x0a8f,    //B1  2703
    0x0785,    //B0  1925
    //Biquad 2
    0x7322,    //A2 -3294
    0x1c1d,    //A1  7197
    0x06f2,    //B2  1778
    0x7e36,    //B1 -0458
    0x06f2,    //B0  1778
    //Biquad 3
    0x7132,    //A2 -3790
    0x1da7,    //A1  7591
    0x07da,    //B2  2010
    0x7989,    //B1 -1655
    0x07da,    //B0  2010
    //Biquad 4
    0x748e,    //A2 -2930
    0x1ae6,    //A1  6886
    0x0800,    //B2  2048
    0x08ed,    //B1  2285
    0x0800,    //B0  2048
    //Biquad 5
    0x73a7,    //A2 -3161
    0x1ad9,    //A1  6873
    0x0800,    //B2  2048
    0x79f7,    //B1 -1545
    0x0800,    //B0  2048
    //Biquad 6
    0x7269,    //A2 -3479
    0x1ad3,    //A1  6867
    0x0800,    //B2  2048
    0x758f,    //B1 -2673
    0x0800,    //B0  2048
    //Biquad 7
    0x714b,    //A2 -3765
    0x1aed,    //A1  6893
    0x0800,    //B2  2048
    0x7421,    //B1 -3039
    0x0800,    //B0  2048
    //Biquad 8
    0x7067,    //A2 -3993
    0x1b40,    //A1  6976
    0x0800,    //B2  2048
    0x73a7,    //B1 -3161
    0x0800,    //B0  2048
    //Biquad 9
    0x7857,    //A2 -1961
    0x6a80,    //A1 -5504
    0x0b4c,    //B2  2892
    0x1684,    //B1  5764
    0x0b4c,    //B0  2892
    //Biquad 10
    0x72b4,    //A2 -3404
    0x64d8,    //A1 -6952
    0x0cc8,    //B2  3272
    0x190f,    //B1  6415
    0x0cc8,    //B0  3272
};


//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audioaipath_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audioaipath_Init(cUInt32 baseAddr)
{
    if (sAIPath.inited == 0) 
    {
        sAIPath.base = baseAddr;
		sAIPath.inited = 1;
        return (CHAL_HANDLE)&sAIPath;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}

//============================================================================
//
// Function Name: chal_audioaipath_EnableDMA
//
// Description:   Enable DMA in Audio Input Path
//
//============================================================================
cVoid chal_audioaipath_EnableDMA(
        CHAL_HANDLE   handle, 
        Boolean       enable
        )
{
	// don't know where to enable the DMA. Is it this one?
	if (enable==TRUE)
	{
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUD_IN_DMA_EN, 1 );
	}
	else
	{
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUD_IN_DMA_EN, 0 );
	}
}

//============================================================================
//
// Function Name: chal_audio_aipath_enable(CHAL_HANDLE handle, cUInt16 enable);
//
// Description:   Enable or Disable AUDVOC Audio Input Path
//
//============================================================================
void chal_audioaipath_Enable(
	   CHAL_HANDLE  handle,
	   Boolean		enable
	   )
{
	if (enable==TRUE)
	{
		// left channel
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOINPATH_SEL, 0 );
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_ENABLE, 1 );
		// right channel
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOINPATH_SEL, 1 );
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_ENABLE, 1 );
	}
	else
	{
		// left channel
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOINPATH_SEL, 0 );
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_ENABLE, 0 );
		// right channel
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOINPATH_SEL, 1 );
		BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_ENABLE, 0 );
	}
}

//============================================================================
//
// Function Name: chal_audioaipath_IIR(CHAL_HANDLE handle, cUInt16 enable);
//
// Description:   Enable or Disable AUDVOC Audio Input Path
//
// AUDIOINCM_ADDR	Audio IN coefficient memory address pointer. (Type: R/W)
//		0-49 : Audio Path 1 IIR coefficients
//		50-99: Audio Path 2 IIR coefficients
//		Coefficients are written in the following order
//		a_x2, a_x1, b_x2, b_x1, b_x0 where x=0..9 is the biquad filter index
//		Default filter coefficients are all zeros.
//
//============================================================================
void chal_audioaipath_CfgIIRCoeff(
	   CHAL_HANDLE  handle,
	   UInt32		channels
	   )
{

	UInt32	index;
	UInt16 data;


	// path 1 IIR coefficients
	BRCM_WRITE_REG_FIELD(sAIPath.base, DSP_AUDIO_AUDIOINCM_ADDR_R, AUDIOINCM_ADDR, 0x0000);

	// left path
	for (index = 0; index < 50; index++)
	{
		data = HqAdcIIRCoeff[index];
		//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "\n\r\t* chal_audioaipath_CfgIIRCoeff index %d is %x *\n\r", index, data );
		  // AUDIOINCM_DATA: The AUDIOINCM_ADDR increase automatically when writing to this register
		BRCM_WRITE_REG(sAIPath.base, DSP_AUDIO_AUDIOINCM_DATA_R, data);
	}
	//if stereo, then continue programming coeff, ADDR will auto-incremented
	if (channels == 2)
	{
		// right path, the same value as left path		
		for (index = 0; index < 50; index++)
		{
			data = HqAdcIIRCoeff[index];
			//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "\n\r\t* chal_audioaipath_CfgIIRCoeff index %d is %x *\n\r", index, data );
			BRCM_WRITE_REG(sAIPath.base, DSP_AUDIO_AUDIOINCM_DATA_R, data);
		}
	}
}

//============================================================================
//
// Function Name: chal_audioaipath_SelectInput
//
// Description:   Select input source for audio input path (stereo)
//
//============================================================================
void chal_audioaipath_SelectInput(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_MIC_INPUT_en		sourceL,
		CHAL_AUDIO_MIC_INPUT_en		sourceR
		)
{
	  //enable ADC1 SW without enabling Voice Out, needed??
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, RXANA1_EN, 1 );
	  //set ADCCONTROL correct mode: 16-bit mode, ena audio int
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOIN_BITMODE, 1 );  // 1 - 16-bit Audio IIR2 output
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOIN_INTEN, 1 );  //should be in enable( )

	  // left channel
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOINPATH_SEL, 0 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_IIR_OUTSEL, 0x2 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_CIC_OUTSEL, 1 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_CIC_FINESCALE, 0x04 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_INPUTSEL, sourceL );

	  // right channel
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_ADCCONTROL_R, AUDIOINPATH_SEL, 1 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_IIR_OUTSEL, 0x2 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_CIC_OUTSEL, 1 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_CIC_FINESCALE, 0x04 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOINPATH_CTRL_R, AIN_INPUTSEL, sourceR );
}


//============================================================================
//
// Function Name: chal_audioaipath_ClrFifo
//
// Description:   Clear FIFO in Audio Input Path
//
//============================================================================
cVoid chal_audioaipath_ClrFifo(CHAL_HANDLE handle)
{
	//clear the FIFO
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_FIFO_CLR, 1 );
	BRCM_WRITE_REG_FIELD( sAIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_FIFO_CLR, 0 );
}

//============================================================================
//
// Function Name: chal_audioaipath_SetFifoThres
//
// Description:   Set FIFO threshold in Audio Input Path
//
//============================================================================
void chal_audioaipath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		in_thres,
		cUInt16		out_thres
		)
{
}

//============================================================================
//
// Function Name: chal_audioaipath_ReadFifoStatus
//
// Description:   Read FIFO statusin Audio Input Path
//
//============================================================================
cUInt16 chal_audioaipath_ReadFifoStatus(CHAL_HANDLE handle)
{
	return BRCM_READ_REG(sAIPath.base, DSP_AUDIO_AUDIOINFIFO_STATUS_R);
}

//============================================================================
//
// Function Name: chal_audioaipath_ReadFifo
//
// Description:   Clear FIFO in Audio Input Path
//
//============================================================================
cUInt16 chal_audioaipath_ReadFifo(
		CHAL_HANDLE handle,
		cUInt32		*dst,
		cUInt16		length
		)
{
		cUInt16 ui16_1, cnt, i;
//		cUInt32 tmp_32;

		ui16_1 = chal_audioaipath_ReadFifoStatus(NULL);
		cnt = (ui16_1 >> 5) & 0x00ff;	//get how many samples in the FIFO
		if (length <= cnt)
			cnt = length;

		for (i=0; i<cnt ; i++)
		{ 
			*dst = BRCM_READ_REG(sAIPath.base, DSP_AUDIO_AIFIFODATA1_R); 
			//for 16-bit case, DATA0 is the same as DATA1
			dst++;
		} 
		return cnt;
}

//============================================================================
//
// Function Name: chal_audioaipath_ReadFifoClr
//
// Description:   Read any residueFIFO in Audio Input Path
//
//============================================================================
void chal_audioaipath_ReadFifoResidue(
		CHAL_HANDLE handle
		)
{
		cUInt16 ui16_1, cnt, i;
		//cUInt32 tmp_32;

		ui16_1 = chal_audioaipath_ReadFifoStatus(NULL);
		cnt = (ui16_1 >> 5) & 0x00ff;	//get how many samples in the FIFO	
		for (i=0; i<cnt ; i++)
		{ 
			//tmp_32 = BRCM_READ_REG(sAIPath.base, DSP_AUDIO_AIFIFODATA1_R); 
			BRCM_READ_REG(sAIPath.base, DSP_AUDIO_AIFIFODATA1_R); 
			//for 16-bit case, DATA0 is the same as DATA1
		} 
}

//============================================================================
//
// Function Name: chal_audioaipath_getAFIFO_addr
//
// Description:   return AFIFO address
//
//============================================================================
cUInt32 chal_audioaipath_getAFIFO_addr( CHAL_HANDLE	handle )
{
#define CHAL_DMA_DEST_ADDR_AIFIFO	(AHB_AUDIO_BASE_ADDR + DSP_AUDIO_AIFIFODATA1_R_OFFSET)

	return CHAL_DMA_DEST_ADDR_AIFIFO;
}
