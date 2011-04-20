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
* @file   chal_audiopopath.c
* @brief  Audio cHAL layer to Polyringer Output Path
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
#include "chal_audiopopath.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

// set ramping time
#define  SLOPE_GAIN_MODE	0


typedef struct
{
    cUInt32 base;         // may be mapped and != DSP_AUDIO_BASE_ADDR
	UInt32	inited;
} chal_audio_popath_t;

// rely on zero inited
static chal_audio_popath_t sPOPath = {AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiopopath_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiopopath_Init(cUInt32 baseAddr)
{
    if (sPOPath.inited == 0) 
    {
        sPOPath.base = baseAddr;
		sPOPath.inited = 1;
        return (CHAL_HANDLE)&sPOPath;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}


//============================================================================
//
// Function Name: chal_audiopopath_EnableDMA
//
// Description:   Enable DMA in Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_EnableDMA(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	if (enable==TRUE)
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_POLYAUDMOD_R, POLY_OUT_DMA_EN, 1);
	else
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_POLYAUDMOD_R, POLY_OUT_DMA_EN, 0);
}

//============================================================================
//
// Function Name: chal_audiopopath_SetSampleRate
//
// Description:   Set sampling rate Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_SetSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		)
{
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_POLYAUDMOD_R, PSAMPRATE, CHAL_ConvertAudioSampleRate(samp_rate) );
}

//============================================================================
//
// Function Name: chal_audiopopath_SelectFilter
//
// Description:   Select Polyringer Output Path Filter
//
//============================================================================
cVoid chal_audiopopath_SelectFilter(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_FILTER_TYPE_en	filter_select
		)
{
	/**
	0: FIR compensation filter selected 
	1: IIR compensation filter selected  (Type: R/W)
	**/
	if( filter_select == CHAL_AUDIO_FIR)
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_POLYAUDMOD_R, PCOMPMOD, 0);
	else
	{
		if( filter_select == CHAL_AUDIO_IIR)
			BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_POLYAUDMOD_R, PCOMPMOD, 1);
		else
			return;
	}
}

//============================================================================
//
// Function Name: chal_audiopopath_Enable
//
// Description:   Enable or Disable Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	/**
	0: turn off 
	1: enable (Type: R/W)

	 Note: Disabling the polyringer path when DSP_AUDIO_AMCR.AUDEN=0 and DSP_AUDIO_STEREOAUDMOD.AUDEN=0 
	 will cause the contents of the DSP_AUDIO_MPM_ALGC_ADDRREG.ADDR=[0x00 to 0x27] registers to be reset.  
	 These registers must be reprogrammed when the polyringer path is enabled.
	**/
	if(FALSE == enable)
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_POLYAUDMOD_R, PLYEN, 0);
	else
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_POLYAUDMOD_R, PLYEN, 1);
}

//============================================================================
//
// Function Name: chal_audiopopath_Mute
//
// Description:   Mute or UnMute Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_Mute(
		CHAL_HANDLE handle,
		Boolean		mute_left,
		Boolean		mute_right
		)
{
	if(mute_left==TRUE)
	{
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLSLOPGAINEN, 1);
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLSLOPMOD, SLOPE_GAIN_MODE);
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLTARGETGAIN, 0);

		{
		//volatile UInt16 right_gain = BRCM_READ_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRTARGETGAIN);
		 //to make left channel gain change take effect.
		//BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRTARGETGAIN, right_gain);
		}
	}
	
	if(mute_right==TRUE)
	{
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRSLOPGAINEN, 1);
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRSLOPMOD, SLOPE_GAIN_MODE);
		BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRTARGETGAIN, 0);
	}
}

//============================================================================
//
// Function Name: chal_audiopopath_ClrFifo
//
// Description:   Clear FIFO in Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_ClrFifo(CHAL_HANDLE handle)
{
	/**
	0: do nothing
	1: clear OUTPUT FIFO ( has to set back to 0 after clear the FIFO) (Type: R/W)
	**/
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PFIFOCTRL_R, POFIFOCRL0, 1);
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PFIFOCTRL_R, POFIFOCRL0, 0);

	/**
	0: do nothing
	1: clear INPUT FIFO ( has to set back to 0 after clear the FIFO); need at least two audio clock for FIFO clear (Type: R/W)
	**/
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PFIFOCTRL_R, PIFIFOCRL1, 1);
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PFIFOCTRL_R, PIFIFOCRL1, 0);
}

//============================================================================
//
// Function Name: chal_audiopopath_SetFifoThres
//
// Description:   Set FIFO threshold in Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		out_thres,
		cUInt16		in_thres
		)
{
	//input FIFO threshold 
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PFIFOCTRL_R, PIFIFOTHRES1, in_thres);
	//output FIFO threshold
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PFIFOCTRL_R, POFIFOTHRES0, out_thres);
}

//============================================================================
//
// Function Name: chal_audiopopath_ReadFifoStatus
//
// Description:   Read FIFO status in Polyringer Output Path
//
//============================================================================
cUInt16 chal_audiopopath_ReadFifoStatus(CHAL_HANDLE handle)
{
	return BRCM_READ_REG(sPOPath.base, DSP_AUDIO_PIFIFOST_R);;
}

//============================================================================
//
// Function Name: chal_audiopopath_WriteFifo
//
// Description:   Fills FIFO with data
//
//============================================================================
cVoid chal_audiopopath_WriteFifo(
		CHAL_HANDLE handle,
		cUInt32		*src,
		cUInt16		length
		)
{
	cUInt16 i=0;
	
	for (i=0; i<length ; i++)
	{ 
		BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PIFIFODATA1_R, *(src+i));
	}
}

//============================================================================
//
// Function Name: chal_audiopopath_SetSlopeGain
//
// Description:   Set Left/Right slope gain in Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_SetSlopeGain(
		CHAL_HANDLE handle,
		cInt32		left_gain,
		cInt32		right_gain
		)
{
	cInt16 gain=0;

	if(left_gain>0 || right_gain>0)
		return;

	// left gain
	gain = (left_gain/25) + 0x1FF;
	if (gain<0)
		gain=0;

	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLTARGETGAIN, gain);

	// right gain
	gain = (right_gain/25) + 0x1FF;
	if (gain<0)
		gain=0;

	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRTARGETGAIN, gain);
}

//============================================================================
//
// Function Name: chal_audiopopath_SetSlopeGainLeftHex
//
// Description:   Set Left slope gain in hex value in Audio Output Path
//
//============================================================================
cVoid chal_audiopopath_SetSlopeGainLeftHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PLSLOPGAIN_R, PLTARGETGAIN, gain_hex);

	{
	//volatile UInt16 right_gain = BRCM_READ_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRTARGETGAIN);
	 //to make left channel gain change take effect.
	//BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRTARGETGAIN, right_gain);
	}
}

//============================================================================
//
// Function Name: chal_audiopopath_SetSlopeGainRightHex
//
// Description:   Set Right slope gain in hex value in Audio Output Path
//
//============================================================================
cVoid chal_audiopopath_SetSlopeGainRightHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PRSLOPGAIN_R, PRTARGETGAIN, gain_hex);
}

//============================================================================
//
// Function Name: chal_audiopopath_SetAEQGain
//
// Description:   Configure 5-band EQ band gains in Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_SetAEQGain(
		CHAL_HANDLE		handle,
		const cUInt16	*gains
		)
{
	BRCM_WRITE_REG_FIELD(sPOPath.base, DSP_AUDIO_PEQPATHGAIN1_R, PEQPATHGAIN1EN, 0); //enable

	BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PEQPATHGAIN1_R, *gains );
	BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PEQPATHGAIN2_R, *(gains+1) );
	BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PEQPATHGAIN3_R, *(gains+2) );
	BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PEQPATHGAIN4_R, *(gains+3) );
	BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PEQPATHGAIN5_R, *(gains+4) );
}

//============================================================================
//
// Function Name: chal_audiopopath_CfgAEQCoeff
//
// Description:   Configure 5-band EQ filter coefficients in Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_CfgAEQCoeff(
		CHAL_HANDLE		handle,
		const cUInt16	*coeff
		)
{
	cInt16 i = 0;

	BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PEQCOFADD_R, 0 );
	for ( i=0; i<NUM_POLY_EQ_COEFF; i++) { 
		BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PEQCOFDATA_R, *(coeff+i) );
	}
}

//============================================================================
//
// Function Name: chal_audiopopath_CfgPIIRCoeff
//
// Description:   configure IIR filter coefficients in Polyringer Output Path
//
//============================================================================
cVoid chal_audiopopath_CfgPIIRCoeff(
		CHAL_HANDLE		handle,
		const cUInt16	*coeff
		)
{
	cInt16 i = 0;

	BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PCOMPIIRCOFADD_R, 0 );
	for ( i=0; i<NUM_POLY_IIR_COEFF; i++) { 
		BRCM_WRITE_REG(sPOPath.base, DSP_AUDIO_PCOMPIIRCOFDATA_R, *(coeff+i) );
	}
}

UInt16 CHAL_ConvertAudioSampleRate(AUDIO_SAMPLING_RATE_t samp_rate)
{
	if(samp_rate==AUDIO_SAMPLING_RATE_8000)
		return 0;
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_12000)
		return 1;
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_16000)
		return 2;
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_24000)
		return 3;
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_32000)
		return 4;
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_48000)
		return 5;
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_11030)
		return 6;  //0110:  11.025KHz
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_22060)
		return 7;  //0111:  22.05KHz
	else
	if(samp_rate==AUDIO_SAMPLING_RATE_44100)
		return 8;

	return 5;
}

//============================================================================
//
// Function Name: chal_audiopopath_getPFIFO_addr
//
// Description:   return PFIFO address
//
//============================================================================
cUInt32 chal_audiopopath_getPFIFO_addr( CHAL_HANDLE	handle )
{
#define CHAL_POFIFO_DMA_DEST_ADDR	(AHB_AUDIO_BASE_ADDR + DSP_AUDIO_PIFIFODATA1_R_OFFSET)

	return CHAL_POFIFO_DMA_DEST_ADDR;
}

