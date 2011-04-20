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
* @file   chal_audiomixer.c
* @brief  Audio cHAL layer of Audio Output Mixer
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
#include "chal_audiomixer.h"
#include "log.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    cUInt32 base;         // may be mapped and != AHB_AUDIO_base
	UInt32	inited;
	cUInt16	inputs[4]; // the input filed values of 4 sMixers
} chal_audio_mixers_t;

// rely on zero inited
static chal_audio_mixers_t sMixers = {AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiomixer_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiomixer_Init(cUInt32 baseAddr)
{
    if (sMixers.inited == 0) 
    {
		sMixers.base = baseAddr;
		sMixers.inited = 1;
		Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_mixer_init: base = 0x%x \n", sMixers.base );
        return (CHAL_HANDLE)&sMixers;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}


//============================================================================
//
// Function Name: chal_audiomixer_AddInput
//
// Description:   Add a input source to mixer
//
//============================================================================
cVoid chal_audiomixer_AddInput(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_MIXER_en			mixer,
		CHAL_AUDIO_MIXER_INPUT_en	input
		)
{
	sMixers.inputs[mixer] |= input;

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_mixer_add_input: mixer=%d, add input=%d, accu_input 0x%x \n", mixer, input, sMixers.inputs[mixer] );

	BRCM_WRITE_REG_FIELD(sMixers.base, DSP_AUDIO_MIXER_INPUT_SEL_R, PATHSEL, mixer);
	BRCM_WRITE_REG_FIELD(sMixers.base, DSP_AUDIO_MIXER_INPUT_SEL_R, INPUT_TO_MIX, sMixers.inputs[mixer]);
}

//============================================================================
//
// Function Name: chal_audiomixer_RemoveInput
//
// Description:   Remove a input soruce to mixer
//
//============================================================================
cVoid chal_audiomixer_RemoveInput(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_MIXER_en			mixer,
		CHAL_AUDIO_MIXER_INPUT_en	input
		)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audiomixer_RemoveInput: mixer=%d, remove input=0x%x accu_input =%d \n", mixer, (~input), sMixers.inputs[mixer] );

	sMixers.inputs[mixer] &= (~input);

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audiomixer_RemoveInput: accu_input =%d \n", sMixers.inputs[mixer] );


	BRCM_WRITE_REG_FIELD(sMixers.base, DSP_AUDIO_MIXER_INPUT_SEL_R, PATHSEL, mixer);
	BRCM_WRITE_REG_FIELD(sMixers.base, DSP_AUDIO_MIXER_INPUT_SEL_R, INPUT_TO_MIX, sMixers.inputs[mixer]);
}

//============================================================================
//
// Function Name: chal_audiomixer_SetGain
//
// Description:   Set mixer output gain
//
//============================================================================
cVoid chal_audiomixer_SetGain(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		cInt32				gain 
		)
{
	BRCM_WRITE_REG_FIELD(sMixers.base, DSP_AUDIO_MIXER_GAIN_CHSEL_R,  MIXER_GAIN_CHSEL, mixer);
	BRCM_WRITE_REG_FIELD(sMixers.base, DSP_AUDIO_MIXER_GAIN_ADJUST_R, MIXER_GAIN_ADJUST, gain);
}

//============================================================================
//
// Function Name: chal_audiomixer_SetBiquads
//
// Description:   Set mixer IIR number of biquads
//
//============================================================================
cVoid chal_audiomixer_SetBiquads(
		CHAL_HANDLE handle,
		cUInt16		biquads
		)
{
	//BRCM_WRITE_REG_FIELD(sMixers.base, DSP_AUDIO_BIQUAD_CFG_R, MPMBIQUAD_NUM, biquads);
	BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_BIQUAD_CFG_R, biquads);
}

//============================================================================
//
// Function Name: chal_audiomixer_CfgIirCoeff
//
// Description:   Configure filter coefficients in Mixer
//
//============================================================================
cVoid chal_audiomixer_CfgIirCoeff(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		const cUInt32		*coeff
		)
{
	UInt16 i = 0, ui16_1 = 0, start_addr=0;
	UInt32 mixer_data;

	switch( mixer )
	{
		case CHAL_AUDIO_MIXER_DAC1:
			start_addr = 0;
			break;
		
		case CHAL_AUDIO_MIXER_DAC2:
			start_addr = 60;
			break;

		case CHAL_AUDIO_MIXER_DAC3:
			start_addr = 120;
			break;

		case CHAL_AUDIO_MIXER_DAC4:
			start_addr = 180;
			break;
		default:
			break;
	}

	//Mixer IIR Coeff Loading
	BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPMBIQUAD_COEF_ADDR_R, start_addr );
	for (i =0; i<60; i++) {
		mixer_data = coeff[i];
		ui16_1 = mixer_data >>6;  //WDATA[21:6]
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPMBIQUAD_COEF_WDATAH_R, ui16_1 );
		ui16_1 = mixer_data & 0x0000003F;  //WDATA[5:0]
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPMBIQUAD_COEF_WDATAL_R, ui16_1 );
	}	
}

//============================================================================
//
// Function Name: chal_audiomixer_SetMpmDga
//
// Description:   Set MPM DGA (gain)
//
//============================================================================
cVoid chal_audiomixer_SetMpmDga(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				gain
		)
{
}

//============================================================================
//
// Function Name: chal_audiomixer_SetMpmDgaHex
//
// Description:   Set MPM DGA (gain) in form of register value
//
//============================================================================
cVoid chal_audiomixer_SetMpmDgaHex(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				gain_hex 
		)
{
	switch( mixer ) {
	case CHAL_AUDIO_MIXER_DAC1:
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_ADDRREG_R, DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G0_ADDR );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATAHREG_R, 0 );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATALREG_R, gain_hex );
		break;

	case CHAL_AUDIO_MIXER_DAC2:
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_ADDRREG_R, DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G1_ADDR );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATAHREG_R, 0 );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATALREG_R, gain_hex );
		break;

	case CHAL_AUDIO_MIXER_DAC3:
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_ADDRREG_R, DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G2_ADDR );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATAHREG_R, 0 );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATALREG_R, gain_hex );
		break;

	case CHAL_AUDIO_MIXER_DAC4:
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_ADDRREG_R, DSP_AUDIO_MPM_ALGC_ADDRREG_R_ADDR_CMD_MPM_DGA_G3_ADDR );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATAHREG_R, 0 );
		BRCM_WRITE_REG(sMixers.base, DSP_AUDIO_MPM_ALGC_DATALREG_R, gain_hex );
		break;

	default:
		break;
	}
}

//============================================================================
//
// Function Name: chal_audiomixer_EnableAldc
//
// Description:   Enable/Disable speaker protection (ALDC)
//
//============================================================================
cVoid chal_audiomixer_EnableAldc(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		Boolean				enable
		)
{
}

//============================================================================
//
// Function Name: chal_audiomixer_SetAldcIirN
//
// Description:   Set speaker protection (ALDC) IIR coefficient
//
//============================================================================
cVoid chal_audiomixer_SetAldcIirN(
		CHAL_HANDLE			handle,
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				n
		)
{
}

//============================================================================
//
// Function Name: chal_audiomixer_ConfigAldcAttack
//
// Description:   Set speaker protection (ALDC) gain attack and decay
//
//============================================================================
cVoid chal_audiomixer_ConfigAldcAttack(
		CHAL_HANDLE			handle, 
		CHAL_AUDIO_MIXER_en mixer,
		cUInt16				gain_attack_thres,
		cUInt16				gain_attack_step,
		cUInt16				gain_attack_slop,
		cUInt16				gain_decay_slop
		)
{
}

