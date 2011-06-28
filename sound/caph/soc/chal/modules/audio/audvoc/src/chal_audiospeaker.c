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
* @file   chal_audiospeaker.c
* @brief  Audio cHAL layer of sSpeaker
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "audio_consts.h"
#include "chal_types.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_ahb_tl3r.h"
#include "brcm_rdb_syscfg.h"
#include "brcm_rdb_dsp_audio.h"
#include "brcm_rdb_util.h"
#include "chal_common.h"
#include "chal_audiomisc.h"
#include "chal_audiospeaker.h"
#include "log.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

#define SPEAKER_DRIVER_100MW_IsActive		1
#define SPEAKER_DRIVER_400MW_isNonActive	0

typedef struct
{
    cUInt32						syscfg_base;         // may be mapped and != AHB_AUDIO_base
    cUInt32						dsp_audio_base;         // may be mapped and != AHB_AUDIO_base
    cUInt32						ahb_tl3r_base;         // may be mapped and != AHB_AUDIO_base
	UInt32						inited;
	AUDIO_SPKR_CH_Mode_t	spkrA_mode;
	AUDIO_SPKR_CH_Mode_t	spkrB_mode;
} chal_audio_speaker_t;

// rely on zero inited
static chal_audio_speaker_t sSpeaker = {SYSCFG_BASE_ADDR, AHB_AUDIO_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiospeaker_init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiospeaker_Init(
				cUInt32 syscfg_baseAddr,
				cUInt32 dsp_audio_baseAddr,
                cUInt32 ahb_tl3r_baseAddr
				)
{
    if (sSpeaker.inited == 0) 
    {
		sSpeaker.inited = 1;
        sSpeaker.syscfg_base = syscfg_baseAddr;
        sSpeaker.dsp_audio_base = dsp_audio_baseAddr;
        sSpeaker.ahb_tl3r_base = ahb_tl3r_baseAddr;

 		//*(volatile UInt32 *)AHB_TL3R_TL3_A2D_ACCESS_EN_R = 0xACCE5504;

        *(volatile UInt32 *)(ahb_tl3r_baseAddr + 0x2c) = 0xACCE5504;
        
		//*(volatile UInt32 *)SYSCFG_DSPCTRL = 0x0; // put audio in operating mode (AUDIOSRST=0, DSPSRST=0). (power up default is 0x80).

		//important:
		//*(volatile UInt32 *)0x088800a8 = 0x0;		// ANACR10, Enabled clock
		//*(volatile UInt32 *)0x08880084 = 0x180; 	// ANACR1, mask for DAC power up signal

		   // *(volatile UInt32 *)0x08880084 = 0x180;	 // ANACR1[7], power mask
#if !( defined(_ATHENA_) && (CHIP_REVISION==20) )        // These bits don't exist in Athena B0
		BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR1, IHF_PWD_MASK, 1);
		BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR1, HIFI_DAC_FORCE_PWRUP_DISABLE, 1);
#endif

        return (CHAL_HANDLE)&sSpeaker;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}

//============================================================================
//
// Function Name: chal_audiospeaker_SetMode
//
// Description:   set Speaker driver mode
//
//============================================================================
cVoid chal_audiospeaker_SetMode(
		CHAL_HANDLE					handle,
		CHAL_AUDIO_SPKR_CH_en		spkr,
		AUDIO_SPKR_CH_Mode_t		mode
		)
{
	// The upper layer should always make sure the handle is initialzed.
	// We don't want to check the handle in every single function.

	Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_set_mode: base = 0x%x, spkr = 0x%x, mode = 0x%x\n", sSpeaker.syscfg_base, spkr, mode);

	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 	
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, DRVR_SELL, SPEAKER_DRIVER_100MW_IsActive);
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, MODE_SELL, mode);
			sSpeaker.spkrA_mode = mode;
			break;

		case CHAL_AUDIO_SPKRA_R:
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, DRVR_SELR, SPEAKER_DRIVER_100MW_IsActive);
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, MODE_SELR, mode);
			sSpeaker.spkrA_mode = mode;
			break;

		case CHAL_AUDIO_SPKRB_L:
		case CHAL_AUDIO_SPKRB_R:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_set_mode: Selected sSpeaker doesn't support mode set, spkr = 0x%x\r\n", spkr);
			sSpeaker.spkrB_mode = mode;
			break;

		default:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_set_mode: Unsupported sSpeaker selection, spkr = 0x%x\r\n", spkr);
			break;
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_SetPGA
//
// Description:   Set Speaker PGA
//
// gain is in mB.  Differential mode and 3-wire mode have different gain range.
// Differential[0~ -21]db, 3-wire[-6 ~ -27]db
//============================================================================
cVoid chal_audiospeaker_SetPGA(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		const cInt32			gain
		)
{
	cUInt8	reg_gain = 0;
	AUDIO_SPKR_CH_Mode_t mode = AUDIO_SPKR_CHANNEL_INVALID;

	// The upper layer should always make sure the handle is initialzed.
	// We don't want to check the handle in every single function.

	// convert gain to register values
	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 
		case CHAL_AUDIO_SPKRA_R:
			mode = sSpeaker.spkrA_mode;
			break;

		case CHAL_AUDIO_SPKRB_L:
		case CHAL_AUDIO_SPKRB_R:
			mode = sSpeaker.spkrB_mode;
			break;

		default:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_pga: Unsupported sSpeaker selection, spkr = 0x%x\r\n", spkr);
			break;
	}

	if (mode == AUDIO_SPKR_CHANNEL_DIFF)
	{
		// differential mode
		if( gain > 0 )
			return;
		reg_gain = (((0 - gain)/100)/3)  & 0x0f; //get absolute value
	}
	else
	{
		// 3 wire mode
		if( gain > -600 )
			return;
		reg_gain = (((-600 - gain)/100)/3)  & 0x0f; //get absolute value
	}

	// write to register
	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PGA_CTRLL, reg_gain);
			break;

		case CHAL_AUDIO_SPKRA_R:
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PGA_CTRLR, reg_gain);
			break;

		case CHAL_AUDIO_SPKRB_L:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_pga: Selected sSpeaker doesn't support pga gain set, spkr = 0x%x\r\n", spkr);
			break;

		case CHAL_AUDIO_SPKRB_R:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_pga: Selected sSpeaker doesn't support pga gain set, spkr = 0x%x\r\n", spkr);
			break;

		default:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_pga: Unsupported sSpeaker selection, spkr = 0x%x\r\n", spkr);
			break;
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_SetPGAIndex
//
// Description:   Set Speaker PGA by index
//
//============================================================================
cVoid chal_audiospeaker_SetPGAIndex(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		const cInt32			gain_index
		)
{
	if(gain_index>7)
		return;

	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PGA_CTRLL, gain_index);
			break;

		case CHAL_AUDIO_SPKRA_R:
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PGA_CTRLR, gain_index);
			break;

		case CHAL_AUDIO_SPKRB_L:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_pga_index: Selected sSpeaker doesn't support pga gain set, spkr = 0x%x\r\n", spkr);
			break;

		case CHAL_AUDIO_SPKRB_R:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_pga_index: Selected sSpeaker doesn't support pga gain set, spkr = 0x%x\r\n", spkr);
			break;

		default:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_pga_index: Unsupported sSpeaker selection, spkr = 0x%x\r\n", spkr);
			break;
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_Mute
//
// Description:   Set Speaker mute or un-mute
//
// always mute both postive and negative output
//============================================================================
cVoid chal_audiospeaker_Mute(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					mute
		)
{
	
#define SPKR_MUTE_NONE	0x00	// unmute
#define SPKR_MUTE_BOTH	0x03	// always mute both postive and negative

	cUInt8	reg_mute;

	if (mute)
	{
		reg_mute = SPKR_MUTE_BOTH;
	}
	else
	{
		reg_mute = SPKR_MUTE_NONE;
	}


	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 	
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, MUTE_SELL, reg_mute);
			break;

		case CHAL_AUDIO_SPKRA_R:
			BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, MUTE_SELR, reg_mute);
			break;

		case CHAL_AUDIO_SPKRB_L:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_mute: Selected sSpeaker doesn't support mute, spkr = 0x%x\r\n", spkr);
			break;

		case CHAL_AUDIO_SPKRB_R:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_mute: Selected sSpeaker doesn't support mute, spkr = 0x%x\r\n", spkr);
			break;

		default:
			chal_dprintf( CDBG_ERRO, "ERROR: chal_audio_spkr_mute: Unsupported sSpeaker selection, spkr = 0x%x\r\n", spkr);
			break;
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_SetAnaPwrRamp
//
// Description:   Power Ramp UP or Down
//
//============================================================================
cVoid chal_audiospeaker_SetAnaPwrRamp(
		CHAL_HANDLE handle,
		UInt16		ramp_hex
		)
{
	BRCM_WRITE_REG(sSpeaker.dsp_audio_base, DSP_AUDIO_APRR_R, ramp_hex );
}

//============================================================================
//
// Function Name: chal_audiospeaker_PwrdnDacRef
//
// Description:   Power down or up DAC power reference
//
//============================================================================
cVoid chal_audiospeaker_PwrdnDacRef(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwr_down
		)
{
	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 
		case CHAL_AUDIO_SPKRA_R:
			if ( pwr_down )
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNREF, 1);
			else
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNREF, 0);

			//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn_dac_ref anancr0: base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwr_down);
			break;

		case CHAL_AUDIO_SPKRB_L:
		case CHAL_AUDIO_SPKRB_R:
			if ( pwr_down )
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNREF, 1);
			else
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNREF, 0);

			//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn_dac_ref anancr10: base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwr_down);
			break;
			
		default:
			break;
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_PwrdnDacDrv
//
// Description:   Power Down Speaker Driver and DAC
//
// power down DAC and sSpeaker driver. Don't touch DAC reference and D2C.
//  pwrdn = FALSE is to power up.
//============================================================================
cVoid chal_audiospeaker_PwrdnDacDrv (
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwrdn
		)
{
	if(FALSE==pwrdn)  //power up
	{
		switch (spkr)
		{
			case CHAL_AUDIO_SPKRA_L:	
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDACL, 0);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDRVL, 0);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn AL up : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			case CHAL_AUDIO_SPKRA_R:
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDACR, 0);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDRVR, 0);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn AR up : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			case CHAL_AUDIO_SPKRB_L:
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDACL, 0);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDRVL, 0);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn BL up : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			case CHAL_AUDIO_SPKRB_R:
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDACR, 0);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDRVR, 0);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn BR up : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			default:
				break;
		}
	}
	else  //power down
	{
		switch (spkr)
		{
			case CHAL_AUDIO_SPKRA_L:	
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDRVL, 1);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDACL, 1);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn AL down : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			case CHAL_AUDIO_SPKRA_R:
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDRVR, 1);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDNDACR, 1);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn AR down : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			case CHAL_AUDIO_SPKRB_L:
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDRVL, 1);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDACL, 1);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn BL down : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			case CHAL_AUDIO_SPKRB_R:
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDRVR, 1);
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDNDACR, 1);
				//Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn BR down : base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwrdn);
				break;

			default:
				break;
		}
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_PwrdnD2c
//
// Description:   Power Down d2c clock
//
// power down D2C clock, this power down the clock to the microhpone too.
//============================================================================
cVoid chal_audiospeaker_PwrdnD2c(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwr_down
		)
{
	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 
		case CHAL_AUDIO_SPKRA_R:
			if ( pwr_down )
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDND2C, 1);
			else
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR0, PWRDND2C, 0);

			Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn_d2c anacr0: base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwr_down);

			break;

		case CHAL_AUDIO_SPKRB_L: 
		case CHAL_AUDIO_SPKRB_R:
			if ( pwr_down )
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDND2C, 1);
			else
				BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_ANACR10, PWRDND2C, 0);

			Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn_d2c anancr10: base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwr_down);

			break;

		default:
			break;
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_read_ANACR0
//
// Description:   Read ANACR0
//
//============================================================================
UInt32 chal_audiospeaker_read_ANACR0(CHAL_HANDLE  handle )
{
	return ( BRCM_READ_REG(sSpeaker.syscfg_base, SYSCFG_ANACR0 ) );
}

//============================================================================
//
// Function Name: chal_audiospeaker_write_ANACR0
//
// Description:   Write ANACR0
//
//============================================================================
cVoid chal_audiospeaker_write_ANACR0( 
									CHAL_HANDLE  handle,
									cUInt32		value
									)
{
	BRCM_WRITE_REG(sSpeaker.syscfg_base, SYSCFG_ANACR0, value);
}

//============================================================================
//
// Function Name: chal_audiospeaker_AUDIOSRST
//
// Description:   global reset AUDVOC
//
//============================================================================
cVoid chal_audiospeaker_AUDIOSRST( CHAL_HANDLE	handle )
{
	//Put AUDVOC in the following mode: 0 Operating;  1 Reset
	// Reset value is 0x0.

	//wait until remove the wrong definition from brcm_rdb_syscfg.h.
	//#define SYSCFG_DSPCTRL                   (SYSCFG_BASE_ADDR + 0x0014)

	//  BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_DSPCTRL, AUDIOSRST, 1);
	//  BRCM_WRITE_REG_FIELD(sSpeaker.syscfg_base, SYSCFG_DSPCTRL, AUDIOSRST, 0);

	//SYSCFG_DSPCTRL_AUDIOSRST_MASK                                  0x00000100

	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_DSPCTRL_OFFSET) |= SYSCFG_DSPCTRL_AUDIOSRST_MASK;
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDIOSRST bit==1 *\n\r" );
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_DSPCTRL_OFFSET) &= ~(SYSCFG_DSPCTRL_AUDIOSRST_MASK);
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDIOSRST bit==0 *\n\r" );
}

//============================================================================
//
// Function Name: chal_audiospeaker_EpDacPdOffsetGen
//
// Description:   Earpiese DAC power-down offset generation
//
//============================================================================
cVoid chal_audiospeaker_EpDacPdOffsetGen(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_SPKR_CH_en	spkr,
		Boolean					pwr_down
		)
{
	switch (spkr)
	{
		case CHAL_AUDIO_SPKRA_L: 
		case CHAL_AUDIO_SPKRA_R:
			if ( pwr_down )
			{
#if ( defined(_ATHENA_) && (CHIP_REVISION==20) )     //  _MASK does not exist in Athena a0
				* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) |= (SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENL_MASK);
				* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) |= (SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENR_MASK);
#endif
			}
			else
			{
#if ( defined(_ATHENA_) && (CHIP_REVISION==20) )     //  _MASK does not exist in Athena a0
				* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) &= ~(SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENL_MASK);
				* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) &= ~(SYSCFG_ANACR1_IHFEP_DAC_PDOFFSETGENR_MASK);
#endif
			}

			Log_DebugPrintf(LOGID_SOC_AUDIO_DETAIL, "chal_audio_spkr_pwrdn_d2c anacr0: base = 0x%x, spkr = 0x%x, pwr_down = 0x%x\n", sSpeaker.syscfg_base, spkr, pwr_down);

			break;

		case CHAL_AUDIO_SPKRB_L: 
		case CHAL_AUDIO_SPKRB_R:
			break;

		default:
			break;
	}
}

//============================================================================
//
// Function Name: chal_audiospeaker_ForcePwrUpMask
//
// Description:   disable ForcePwrUpMask
//
//============================================================================
cVoid chal_audiospeaker_ForcePwrUpMask( CHAL_HANDLE  handle )
{
#if ( defined(_ATHENA_) && (CHIP_REVISION==20) )     //  _MASK does not exist in Athena a0
	* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) |= (SYSCFG_ANACR1_HEADSET_DAC_PWD_MASK_MASK);
	* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) |= (SYSCFG_ANACR1_IHF_EARPIECE_DAC_FORCE_PWRUP_DISABLE_MASK);
#endif
}

//============================================================================
//
// Function Name: chal_audiospeaker_EpDacPwrGateCtrl
//
// Description:   Earpiese DAC power gate control
//
//============================================================================
cVoid chal_audiospeaker_EpDacPwrGateCtrl( CHAL_HANDLE  handle )
{
#if ( defined(_ATHENA_) && (CHIP_REVISION==20) )     //  _MASK does not exist in Athena a0
	* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) |= (SYSCFG_ANACR1_IHFEP_DAC_PWRMOS_GATECNTRLL_MASK);
	* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) |= (SYSCFG_ANACR1_IHFEP_DAC_PWRMOS_GATECNTRLR_MASK);
#endif
}

//============================================================================
//
// Function Name: chal_audiospeaker_EpDacRampRefEn
//
// Description:   Earpiese DAC ramp reference enable
//
//============================================================================
cVoid chal_audiospeaker_EpDacRampRefEn( CHAL_HANDLE  handle )
{
#if ( defined(_ATHENA_) && (CHIP_REVISION==20) )     //  _MASK does not exist in Athena a0
	* (volatile SYSCFG_ANACR1_TYPE *) (SYSCFG_BASE_ADDR + SYSCFG_ANACR1_OFFSET) |= (SYSCFG_ANACR1_IHFEP_DAC_RAMPREF_EN_MASK);
#endif
}
