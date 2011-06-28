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
* @file   chal_audioaopath.c
* @brief  Audio cHAL interface to audio output path
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
#include "chal_audioaopath.h"
#include "log.h"



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
	UInt32  inited;
} chal_audio_aopath_t;

// rely on zero inited
static chal_audio_aopath_t sAOPath = {AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audioaopath_Init
//
// Description:   Standard Init entry point for audio output path driver
//
//============================================================================
CHAL_HANDLE chal_audioaopath_Init(cUInt32 baseAddr)
{
    if (sAOPath.inited == 0) 
    {
        sAOPath.base = baseAddr;
		sAOPath.inited = 1;
		return (CHAL_HANDLE)&sAOPath;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}


//============================================================================
//
// Function Name: chal_audioaopath_EnableDMA
//
// Description:   Enable DMA in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_EnableDMA(
		CHAL_HANDLE handle, 
		Boolean		enable 
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	
	if (enable==TRUE)
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, AUD_OUT_DMA_EN, 1);
	else
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, AUD_OUT_DMA_EN, 0);

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_aopath_dma_enable: base = 0x%x, enable = 0x%x\n", sAOPath.base, enable);

}

//============================================================================
//
// Function Name: chal_audioaopath_SetSampleRate
//
// Description:   Set sample rate on Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_SetSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, ASAMPRATE, CHAL_ConvertAudioSampleRate(samp_rate));
	
}

//============================================================================
//
// Function Name: chal_audioaopath_DisableAudInt
//
// Description:   Disable audio interrupt in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_DisableAudInt(
		CHAL_HANDLE handle,
		Boolean		disable
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	
	/** 0: no action
	1: disable the audio interrupt
	**/
	if(FALSE == disable)
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, AUDINTDIS, 0);
	else
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, AUDINTDIS, 1);
}

//============================================================================
//
// Function Name: chal_audioaopath_SelectFilter
//
// Description:   Select Audio Output Path Filter
//
//============================================================================
cVoid chal_audioaopath_SelectFilter(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_FILTER_TYPE_en	filter_select
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	/** 
	0: FIR compensation filter selected for audio path
	1: IIR compensation filter selected for audio path (Type: R/W)
	**/
	if( filter_select == CHAL_AUDIO_FIR)
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, ACOMPMOD, 0);
	else
		if( filter_select == CHAL_AUDIO_IIR)
			BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, ACOMPMOD, 1);
		else
			return;
	
}

//============================================================================
//
// Function Name: chal_audioaopath_Enable
//
// Description:   Enable or Disable AUDVOC Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_Enable(
        CHAL_HANDLE handle,
        Boolean     enable
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	/**
	0: turn off audio path
	1: enable audio path (Type: R/W)

	 Note: Disabling the audio path when DSP_AUDIO_AMCR.AUDEN=0 and 
	 DSP_AUDIO_POLYAUDMOD.PLYEN=0 will cause the contents of the 
	 DSP_AUDIO_MPM_ALGC_ADDRREG.ADDR=[0x00 to 0x27] registers to be reset. 
	 These registers must be reprogrammed when the audio path is enabled.

	**/
	if(FALSE == enable)
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, AUDEN, 0);
	else
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, AUDEN, 1);

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_aopath_enable: base = 0x%x, enable = 0x%x\n", sAOPath.base, enable);
}

//============================================================================
//
// Function Name: chal_audioaopath_Mute
//
// Description:   Mute AUDVOC Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_Mute(
		CHAL_HANDLE handle, 
		Boolean		mute_left, 
		Boolean		mute_right
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	if(mute_left==TRUE)
	{
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALSLOPGAINEN, 1);
		// hard code mode to 6??
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALSLOPMOD, SLOPE_GAIN_MODE);
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALTARGETGAIN, 0);

		Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audioaopath_Mute: left \n");
		{
		//volatile UInt16 right_gain = BRCM_READ_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARTARGETGAIN);
		 //to make left channel gain change take effect.
		//BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARTARGETGAIN, right_gain);
		}
	}
	if(mute_right==TRUE)
	{
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARSLOPGAINEN, 1);
		// hard code mode to 6??
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARSLOPMOD, SLOPE_GAIN_MODE);
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARTARGETGAIN, 0);

		Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audioaopath_Mute: right \n");
	}
}

//============================================================================
//
// Function Name: chal_audioaopath_SetI2SMode
//
// Description:   Enable or Disable I2S_Mode in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_SetI2SMode(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	/**
	0: the audio path take data from DSP/ARM
	1: the audio path is driven by I2S direct path
	**/
	if(FALSE==enable)
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, I2SMODE, 0);
	else
		BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_STEREOAUDMOD_R, I2SMODE, 1);
}

//============================================================================
//
// Function Name: chal_audioaopath_ClrFifo
//
// Description:   Clear FIFO in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_ClrFifo(CHAL_HANDLE handle)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	/**
	0: do nothing
	1: clear OUTPUT FIFO ( has to set back to 0 after clear the FIFO) (Type: R/W)
	**/
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_AFIFOCTRL_R, AOFIFOCRL0, 1);
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_AFIFOCTRL_R, AOFIFOCRL0, 0);

	/**
	0: do nothing
	1: clear INPUT FIFO ( has to set back to 0 after clear the FIFO); need at least two audio clock for FIFO clear (Type: R/W)
	**/
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_AFIFOCTRL_R, AIFIFOCRL1, 1);
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_AFIFOCTRL_R, AIFIFOCRL1, 0);
}

//============================================================================
//
// Function Name: chal_audioaopath_SetFifoThres
//
// Description:   Set FIFO threshold in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		out_thres,
		cUInt16		in_thres
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	//input FIFO threshold 
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_AFIFOCTRL_R, AIFIFOTHRES1, in_thres);
	//output FIFO threshold
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_AFIFOCTRL_R, AOFIFOTHRES0, out_thres);
}

//============================================================================
//
// Function Name: chal_audioaopath_ReadFifoStatus
//
// Description:   Read FIFO statusin Audio Output Path
//
//============================================================================
cUInt16 chal_audioaopath_ReadFifoStatus(CHAL_HANDLE handle)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	return BRCM_READ_REG(sAOPath.base, DSP_AUDIO_AIFIFOST_R);
}

//============================================================================
//
// Function Name: chal_audioaopath_WriteFifo
//
// Description:   Fills FIFO with data
//
//============================================================================
cVoid chal_audioaopath_WriteFifo(
		CHAL_HANDLE handle,
		cUInt32		*src,
		cUInt16		length
		)
{
	cUInt16 i=0;
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	for (i=0; i<length ; i++) { 
		BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AIFIFODATA1_R, *(src+i));
	}
}


//============================================================================
//
// Function Name: chal_audioaopath_SetSlopeGain
//
// Description:   Set Left/Right slope gain in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_SetSlopeGain(
		CHAL_HANDLE handle,
		cInt32		left_gain,
		cInt32		right_gain
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	cInt16 gain=0;

	if(left_gain>0 || right_gain>0)
		return;

	// left gain
	gain = (left_gain/25) + 0x1FF;
	if (gain<0)
		gain=0;

	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALTARGETGAIN, gain);
	
	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audioaopath_SetSlopeGain: left gain=0x%x \n", gain );

	// right gain
	gain = (right_gain/25) + 0x1FF;
	if (gain<0)
		gain=0;

	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARTARGETGAIN, gain);

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audioaopath_SetSlopeGain: right gain=0x%x \n", gain );
}

//============================================================================
//
// Function Name: chal_audioaopath_SetSlopeGainLeftHex
//
// Description:   Set Left slope gain in hex value in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_SetSlopeGainLeftHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ALSLOPGAIN_R, ALTARGETGAIN, gain_hex);

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audioaopath_SetSlopeGainLeftHex: gain=0x%x \n", gain_hex );

	{
	//volatile UInt16 right_gain = BRCM_READ_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARTARGETGAIN);
	 //to make left channel gain change take effect.
	//BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARTARGETGAIN, right_gain);
	}
}

//============================================================================
//
// Function Name: chal_audioaopath_SetSlopeGainRightHex
//
// Description:   Set Right slope gain in hex value in Audio Output Path
//
//============================================================================
cVoid chal_audioaopath_SetSlopeGainRightHex(
		CHAL_HANDLE handle,
		cInt32		gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARSLOPGAINEN, 1);
	// hard code mode to 6??
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_ARSLOPGAIN_R, ARTARGETGAIN, gain_hex);

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audioaopath_SetSlopeGainRightHex: gain=0x%x \n", gain_hex );
}

//============================================================================
//
// Function Name: chal_audioaopath_SetAEQGain
//
// Description:   Configure 5-band EQ band gains in Audio Output Path
//
//============================================================================
void chal_audioaopath_SetAEQGain(
		CHAL_HANDLE		handle,
		const UInt16	*gains
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;

	BRCM_WRITE_REG_FIELD(sAOPath.base, DSP_AUDIO_AEQPATHGAIN1_R, AEQPATHGAIN1EN, 0);  //enable

	BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AEQPATHGAIN1_R, *gains );
	BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AEQPATHGAIN2_R, *(gains+1) );
	BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AEQPATHGAIN3_R, *(gains+2) );
	BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AEQPATHGAIN4_R, *(gains+3) );
	BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AEQPATHGAIN5_R, *(gains+4) );
}

//============================================================================
//
// Function Name: chal_audioaopath_CfgAEQCoeff
//
// Description:   Configure 5-band EQ filter coefficients in Audio Output Path
//
//============================================================================
void chal_audioaopath_CfgAEQCoeff(
		CHAL_HANDLE		handle,
		const UInt16	*coeff
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	cInt16 i = 0;

	BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AEQCOFADD_R, 0 );
	for ( i=0; i<NUM_AUDIO_EQ_COEFF; i++) { 
		BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_AEQCOFDATA_R, *(coeff+i) );
	}
}

//============================================================================
//
// Function Name: chal_audioaopath_CfgAFIRCoeff
//
// Description:   Configure FIR filter coefficients in Audio Output Path
//
//============================================================================
void chal_audioaopath_CfgAFIRCoeff(
		CHAL_HANDLE		handle,
		const UInt16	*coeff
		)
{
	//chal_audio_aopath_t * dev = (chal_audio_aopath_t *) handle;
	cInt16 i = 0;

	BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_ACOMPFIRCOFADD_R, 0 );
	for ( i=0; i<NUM_AUDIO_FIR_COEFF; i++) { 
		BRCM_WRITE_REG(sAOPath.base, DSP_AUDIO_ACOMPFIRCOFDATA_R, *(coeff+i) );
	}
}

//============================================================================
//
// Function Name: chal_audioaopath_getAFIFO_addr
//
// Description:   return AFIFO address
//
//============================================================================
cUInt32 chal_audioaopath_getAFIFO_addr( CHAL_HANDLE	handle )
{
#define CHAL_AOFIFO_DMA_DEST_ADDR	(AHB_AUDIO_BASE_ADDR + DSP_AUDIO_AIFIFODATA1_R_OFFSET)

	return CHAL_AOFIFO_DMA_DEST_ADDR;
}

