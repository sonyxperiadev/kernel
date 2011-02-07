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
* @file   chal_audiovipath.c
* @brief  Audio cHAL layer of Voice Input Path
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
#include "chal_audiovipath.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    cUInt32 base;         // may be mapped and != AHB_AUDIO_BASE_ADDR
	UInt32  inited;
} chal_audio_vipath_t;

// rely on zero inited
static chal_audio_vipath_t sVIPath = {AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiovipath_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiovipath_Init( cUInt32 baseAddr )
{
	if (sVIPath.inited == 0) 
    {
        sVIPath.base = baseAddr;
		sVIPath.inited = 1;
        return (CHAL_HANDLE)&sVIPath;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}

//============================================================================
//
// Function Name: chal_audiovipath_Enable
//
// Description:   Enable or Disable Voice Input Path 1
//
//============================================================================
cVoid chal_audiovipath_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	//same as chal_audio_vopath_enable( )
    //note: when ANACR1[7]==1, AMCR[5] only controls digital HW.
	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_AMCR_R, AUDEN, enable);

	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_VINPATH_CTRL_R, VIN_ENABLE, enable);
}

//============================================================================
//
// Function Name: chal_audiovipath2_Enable
//
// Description:   Enable or Disable Voice Input Path 2
//
//============================================================================
cVoid chal_audiovipath2_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	if(enable)
	{
		BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_AMCR_R, AUDEN, enable);
		//BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, RXANA1_EN, 1 );

		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, RXANA2_EN, 1 );
		// do we use RXANA2_EN for VI path2 or VIN_ENABLE (must)
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_SEL, 1 );
		BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_VINPATH_CTRL_R, VIN_ENABLE, 1);
		// voice in path 1. at the end recover the bit setting.
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_SEL, 0 );
	
	}
	else
	{
		//needs to be careful with this:
		//BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_AMCR_R, AUDEN, enable);

		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_SEL, 1 );
		BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_VINPATH_CTRL_R, VIN_ENABLE, enable);
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, RXANA2_EN, 0 );
		// voice in path 1. at the end recover the bit setting.
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_SEL, 0 );
	}
	

}

//============================================================================
//
// Function Name: chal_audiovipath_SelectInput
//
// Description:   Select input source for voice input path
//
//============================================================================
cVoid chal_audiovipath_SelectInput(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_MIC_INPUT_en input_source
		)
{
	// voice in path 1
	BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_SEL, 0 );
	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_VINPATH_CTRL_R, VIN_INPUTSEL, input_source);
}

//============================================================================
//
// Function Name: chal_audiovipath2_SelectInput
//
// Description:   Select input source for voice input path 2
//
//============================================================================
cVoid chal_audiovipath2_SelectInput(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_MIC_INPUT_en input_source
		)
{
	// voice in path 2
	BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_SEL, 1 );
	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_VINPATH_CTRL_R, VIN_INPUTSEL, input_source);

	// voice in path 1. at the end recover the bit setting.
	BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_SEL, 0 );
}

//============================================================================
//
// Function Name: chal_audiovipath_EnableLoopback
//
// Description:   Enable HW loopback
//
//============================================================================
cVoid chal_audiovipath_EnableLoopback (
		CHAL_HANDLE		handle,
		Boolean			enable,
		CHAL_AUDIO_MIC_INPUT_en  mic_in
		)
{
		//1: enable analog loop back
		//0: disable
	BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_ADCCONTROL_R, LOOPEN, enable );

	if( enable )
	{
		/*
typedef enum
{
	CHAL_AUDIO_MIC_INPUT_ADC1,	///< input from ADC1
	CHAL_AUDIO_MIC_INPUT_ADC2,	///< input from ADC2
	CHAL_AUDIO_MIC_INPUT_DMIC1,	///< input from digital mic1
	CHAL_AUDIO_MIC_INPUT_DMIC2	///< input from digital mic2
} CHAL_AUDIO_MIC_INPUT_en;

		AUDIOLOOPBACK_CTRL
		- Loopback source select for DAC2
		00 - Select from Analog ADC1
		01 - Select from Analog ADC2 (not valid for Juno & Athena)
		10 - Select from Digital MIC1
		11 - Select from Digital MIC2
		Reset value is 0x0.
		*/
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC1, mic_in );
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC2, mic_in );
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC3, mic_in );
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC4, mic_in );
	}
	else
	{
		/* 
		Reset value is 0x0.
		*/
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC1, 0 );
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC2, 0 );
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC3, 0 );
		BRCM_WRITE_REG_FIELD( sVIPath.base, DSP_AUDIO_AUDIOLOOPBACK_CTRL_R, AUDIOIN_BITMODE_DAC4, 0 );
	}
}

//============================================================================
//
// Function Name: chal_audiovipath_SetSampleRate
//
// Description:   Set voice input sample rate
//  same as chal_audiovopath_SetSampleRate( ).
//============================================================================
cVoid chal_audiovipath_SetSampleRate(
		CHAL_HANDLE				handle,
		AUDIO_SAMPLING_RATE_t	samp_rate
		)
{
	if(AUDIO_SAMPLING_RATE_8000 == samp_rate )
	{
		BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_AMCR_R, MODE_16K, 0);
		BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH2_16K_MODE_SEL, 0);
	}
	else
	if(AUDIO_SAMPLING_RATE_16000 == samp_rate )
	{
		BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_AMCR_R, MODE_16K, 1);
		BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH2_16K_MODE_SEL, 1);
	}
}

//============================================================================
//
// Function Name: chal_audiovipath_ClrFifo
//
// Description:   Clear FIFO in Audio Input Path
// 
//============================================================================
cVoid chal_audiovipath_ClrFifo(CHAL_HANDLE handle)
{
}

//============================================================================
//
// Function Name: chal_audiovipath_SetFifoThres
//
// Description:   Set FIFO threshold in Audio Input Path
// 
//============================================================================
cVoid chal_audiovipath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		in_thres,
		cUInt16		out_thres
		)
{
}

//============================================================================
//
// Function Name: chal_audiovipath_ReadFifoStatus
//
// Description:   Read FIFO status from Audio Input Path
// 
//============================================================================
cUInt16 chal_audiovipath_ReadFifoStatus(CHAL_HANDLE handle)
{
	return 0;
}

//============================================================================
//
// Function Name: chal_audiovipath_ReadFifo
//
// Description:   Read data from voice input FIFO
// 
//============================================================================
cVoid chal_audiovipath_ReadFifo(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		)
{
}

//============================================================================
//
// Function Name: chal_audiovipath_CfgIIRCoeff
//
// Description:   configure IIR filter coefficients in Voice Output Path
// 
//============================================================================
cVoid chal_audiovipath_CfgIIRCoeff(
		CHAL_HANDLE		handle,
		const UInt16	*coeff
		)
{
	UInt16 i = 0;
/**
DSP_AUDIO_AMCR[7],  DSP_AUDIO_ADCCONTROL[8]
0x               dac voice out (read-write)
10               adc voice in channel 1 (read-write)
11               adc voice in channel 2 (write-only)
*/
	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_AMCR_R, CRAMSEL, 1);  //select ADC coeff RAM
	  // 0: when program , it applies to the  voice in left channel
	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_IIRCM_SEL, 0);
	  //ADC IIR Data Loading
	for ( i=0; i<NUM_OF_ADC_VOICE_COEFF; i++) {
		BRCM_WRITE_REG_FIELD( (sVIPath.base + 2*i), DSP_AUDIO_VCOEFR0_R, VOICEIIRCOEF, coeff[i] );
	}

	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_AMCR_R, CRAMSEL, 1);  //select ADC coeff RAM
	  // - Write IIR coefficients to Voice Path 2 filter using
	  // 1: when program , it applies to the voice in right channel
	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_IIRCM_SEL, 1);
	  //ADC IIR Data Loading
	for ( i=0; i<NUM_OF_ADC_VOICE_COEFF; i++) {
		BRCM_WRITE_REG_FIELD( (sVIPath.base + 2*i), DSP_AUDIO_VCOEFR0_R, VOICEIIRCOEF, coeff[i] );
	}

	  // recover the default setting in this bit.
	BRCM_WRITE_REG_FIELD(sVIPath.base, DSP_AUDIO_ADCCONTROL_R, VINPATH_IIRCM_SEL, 0);
}
