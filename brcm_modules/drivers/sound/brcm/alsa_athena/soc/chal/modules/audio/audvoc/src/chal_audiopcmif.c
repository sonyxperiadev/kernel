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
* @file   chal_audiopcmif.c
* @brief  Audio cHAL layer to PCM Interface
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "audio_consts.h"
#include "chal_common.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_syscfg.h"
#include "brcm_rdb_dsp_audio.h"
#include "brcm_rdb_util.h"
#include "chal_types.h"
#include "chal_audiomisc.h"
#include "chal_audiopcmif.h"

//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    cUInt32 base;         // may be mapped and != AHB_AUDIO_BASE_ADDR
    cUInt32 syscfg_base;  //
	UInt32	inited;
} chal_audio_pcmif_t;

// rely on zero inited
static chal_audio_pcmif_t sPcmIF = {AHB_AUDIO_BASE_ADDR, SYSCFG_BASE_ADDR, 0};   // Single block

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: chal_audiopcmif_Init
//
// Description:   Init function
//
//============================================================================
CHAL_HANDLE chal_audiopcmif_Init( cUInt32 baseAddr, cUInt32 syscfg_baseAddr )
{
	if (sPcmIF.inited == 0) 
    {
        sPcmIF.base = baseAddr;
		sPcmIF.inited = 1;
        sPcmIF.syscfg_base = syscfg_baseAddr;
        return (CHAL_HANDLE)&sPcmIF;
    }
    else
    {
        // Don't re-initialize a block
        return (CHAL_HANDLE) 0;
    }
}

//============================================================================
//
// Function Name: chal_audiopcmif_SetMode
//
// Description:   Configure PCM Interface
//
//============================================================================
cVoid chal_audiopcmif_SetMode(
		CHAL_HANDLE				handle,
		CHAL_AUDIO_PCMIF_Mode_en mode
		)
{

}

//============================================================================
//
// Function Name: chal_audiopcmif_CfgUserMode
//
// Description:   Configure user mode in PCM Interface
//
//============================================================================
cVoid chal_audiopcmif_CfgUserMode(
		CHAL_HANDLE handle, 
		cUInt16		bitFactor, 
		cUInt16		dataFactor
		)
{
}

//============================================================================
//
// Function Name: chal_audiopcmif_Enable
//
// Description:   Enable or Disable PCM Interface
//
//============================================================================
cVoid chal_audiopcmif_Enable(
		CHAL_HANDLE handle,
		Boolean		enable
		)
{
	BRCM_WRITE_REG_FIELD(sPcmIF.base, DSP_AUDIO_AMCR_R, PCMEN, enable);

	//mux to PCM interface (set to 00)
	BRCM_WRITE_REG_FIELD(sPcmIF.syscfg_base, SYSCFG_IOCR0, PCM_MUX, 0 );
}
