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
*  @file   chal_audio.c
*
*  @brief  chal layer driver for audio device driver
*
****************************************************************************/

#include "chal_audio.h"
#include "chal_audio_int.h"

//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************
static ChalAudioCtrlBlk_t  cHALAudioCB = {0,0};

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




//============================================================================
//
// Function Name: CHAL_HANDLE chal_audio_init(cUInt32 baseAddr)
//
// Description:   Standard Init entry point for cHal
//                first function to call.
//
// Parameters:
//                audioh_base,    mapped address of Hera audio register files
//                sdt_base,         mapped address of Hera sidetone register files
//
// Return:
//             CHAL handle for Hera audio
//
//============================================================================

CHAL_HANDLE chal_audio_init(cUInt32 audioh_base, cUInt32    sdt_base)
{
    cHALAudioCB.audioh_base = audioh_base;
    cHALAudioCB.sdt_base = sdt_base;

    return (CHAL_HANDLE)(&cHALAudioCB);
}

//============================================================================
//
// Function Name: CHAL_HANDLE chal_audio_deinit(CHAL_HANDLE handle)
//
// Description:   Deinit Hera audio
//
// Parameters:
//                handle,    the Hera audio handle
//
// Return:
//             none
//
//============================================================================
cVoid chal_audio_deinit(CHAL_HANDLE handle)
{
    ChalAudioCtrlBlk_t  *pCHALCb = (ChalAudioCtrlBlk_t*)handle;

    pCHALCb->audioh_base  = 0;
    pCHALCb->sdt_base  = 0;

    return;
}



