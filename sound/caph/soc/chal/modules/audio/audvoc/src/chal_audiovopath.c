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
* @file   chal_audiovopath.c
* @brief  Audio cHAL layer of Voice Output Path
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
#include "chal_audiovopath.h"

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
    cUInt32 base;         // may be mapped and != AHB_AUDIO_BASE_ADDR
	UInt32  inited;
} chal_audio_vopath_t;

// rely on zero inited
static chal_audio_vopath_t sVOPath = {AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiovopath_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiovopath_Init( cUInt32 baseAddr )
{
	if (sVOPath.inited == 0) 
    {
		sVOPath.inited = 1;
        sVOPath.base = baseAddr;
        return (CHAL_HANDLE)&sVOPath;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}

//============================================================================
//
// Function Name: chal_audiovopath_SetSampleRate
//
// Description:   Set sample rate in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_SetSampleRate(
		CHAL_HANDLE					handle,
		AUDIO_SAMPLING_RATE_t		samp_rate
		)
{
	if( AUDIO_SAMPLING_RATE_8000 == samp_rate )
		BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_AMCR_R, MODE_16K, 0);
	else
	if( AUDIO_SAMPLING_RATE_16000 == samp_rate )
		BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_AMCR_R, MODE_16K, 1);
}

//============================================================================
//
// Function Name: chal_audiovopath_Enable
//
// Description:   Enable or Disable AUDVOC Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_AMCR_R, AUDEN, enable);
}

//============================================================================
//
// Function Name: chal_audiovopath_Mute
//
// Description:   Mute or UnMute AUDVOC Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_Mute(
		CHAL_HANDLE handle,
		Boolean		mute
		)
{
	if(TRUE==mute)
	{
		BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_VSLOPGAIN_R, VTARGETGAIN, 0);
	}
}

//============================================================================
//
// Function Name: chal_audiovopath_ClrFifo
//
// Description:   Clear FIFO in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_ClrFifo(CHAL_HANDLE handle)
{
}

//============================================================================
//
// Function Name: chal_audiovopath_SetFifoThres
//
// Description:   Set FIFO threshold in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_SetFifoThres(
		CHAL_HANDLE handle,
		cUInt16		thres
		)
{
}

//============================================================================
//
// Function Name: chal_audiovopath_ReadFifoStatus
//
// Description:   Read FIFO status from Voice Output Path
//
//============================================================================
cUInt16 chal_audiovopath_ReadFifoStatus(CHAL_HANDLE handle)
{
	return 0;
}

//============================================================================
//
// Function Name: chal_audiovopath_WriteFifo
//
// Description:   Fills FIFO with data
//
//============================================================================
cVoid chal_audiovopath_WriteFifo(
		CHAL_HANDLE handle,
		cUInt16		*src,
		cUInt16		length
		)
{
}

//============================================================================
//
// Function Name: chal_audiovopath_SetSlopeGain
//
// Description:   Set slope gain in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_SetSlopeGain(
		CHAL_HANDLE handle,
		cUInt16		gain
		)
{
	//BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_VSLOPGAIN_R, VTARGETGAIN, -gain/100);
}

//============================================================================
//
// Function Name: chal_audiovopath_SetSlopeGainHex
//
// Description:   Set slope gain as hex value in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_SetSlopeGainHex(
		CHAL_HANDLE handle,
		cUInt16		gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_VSLOPGAIN_R, VSLOPGAINEN, 1);
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_VSLOPGAIN_R, VSLOPMOD, SLOPE_GAIN_MODE);
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_VSLOPGAIN_R, VTARGETGAIN, gain_hex);
}

//============================================================================
//
// Function Name: chal_audiovopath_CfgIIRCoeff
//
// Description:   Configure Vocie filter coefficients in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_CfgIIRCoeff(
		CHAL_HANDLE handle,
		const UInt16 *coeff
		)
{
	UInt16 i = 0;

	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_AMCR_R, CRAMSEL, 0);  //select DAC coeff RAM

	//DAC IIR Data Loading
	for ( i=0; i<NUM_OF_DAC_VOICE_COEFF; i++) {
		BRCM_WRITE_REG_FIELD( (sVOPath.base + 2*i), DSP_AUDIO_VCOEFR0_R, VOICEIIRCOEF, coeff[i] );
	} 
}

//============================================================================
//
// Function Name: chal_audiovopath_SetVCFGR
//
// Description:   Set VCFGR gain in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_SetVCFGR (
		CHAL_HANDLE handle,
		cUInt16		gain
		)
{
}

//============================================================================
//
// Function Name: chal_audiovopath_SetVCFGRhex
//
// Description:   Set VCFGR gain as Hex value in Voice Output Path
//
//============================================================================
cVoid chal_audiovopath_SetVCFGRhex (
		CHAL_HANDLE handle,
		cUInt16		gain_hex
		)
{
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_VCFGR_R, VCFGR, gain_hex);
}

//============================================================================
//
// Function Name: chal_audiovopath_DisableDitheringShuffling
//
// Description:   Disable Shuffling, Disable Dithering
//
// 1.	Disable shuffling:  Set bit[0] of DSP_AUDIO_AIR_R register to 1
// 2.	Disable dithering:  Set bit[2] and bit[0] of DSP_AUDIO_SDMDTHER register to 0
//
//============================================================================
cVoid chal_audiovopath_DisableDitheringShuffling (
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_AIR_R, CCR_DIS, 1);
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_SDMDTHER_R, LDEN, 0);
	BRCM_WRITE_REG_FIELD(sVOPath.base, DSP_AUDIO_SDMDTHER_R, RDEN, 0);
}
