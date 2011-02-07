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
*  @file   drv_audio_common.c
*
*  @brief  Command driver code for audio drivers
*
****************************************************************************/
#include "log.h"
#include "xassert.h"
#include "auddrv_def.h"
//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

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
#if defined (_RHEA_)
#include "csl_caph.h"
#include "drv_caph.h"
#include  "drv_audio_common.h"


// ==========================================================================
//
// Function Name: AUDDRV_GetCSLDevice
//
// Description: Get the audio csl Device from the Driver device.
//
// =========================================================================
CSL_CAPH_DEVICE_e AUDDRV_GetCSLDevice (AUDDRV_DEVICE_e dev)
{
	CSL_CAPH_DEVICE_e cslDev = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetCSLDevice:: dev = 0x%x\n", dev);

      switch (dev)
      {
        case AUDDRV_DEV_NONE:
            cslDev = CSL_CAPH_DEV_NONE;
            break;
			
         case AUDDRV_DEV_EP:
            cslDev = CSL_CAPH_DEV_EP;
            break;
			
         case AUDDRV_DEV_HS:
            cslDev = CSL_CAPH_DEV_HS;
            break;	
			
         case AUDDRV_DEV_IHF:
            cslDev = CSL_CAPH_DEV_IHF;
            break;
			
         case AUDDRV_DEV_VIBRA:
            cslDev = CSL_CAPH_DEV_VIBRA;
            break;
			
         case AUDDRV_DEV_I2S_TX:
            cslDev = CSL_CAPH_DEV_I2S_TX;
            break;
			
         case AUDDRV_DEV_BT_SPKR:
            cslDev = CSL_CAPH_DEV_BT_SPKR;
            break;	
			
         case AUDDRV_DEV_DSP:
            cslDev = CSL_CAPH_DEV_DSP;
            break;
			
         case AUDDRV_DEV_DIGI_MIC:
            cslDev = CSL_CAPH_DEV_DIGI_MIC;
            break;


         case AUDDRV_DEV_DIGI_MIC_L:
            cslDev = CSL_CAPH_DEV_DIGI_MIC_L;
            break;

         case AUDDRV_DEV_DIGI_MIC_R:
            cslDev = CSL_CAPH_DEV_DIGI_MIC_R;
            break;
			
         case AUDDRV_DEV_EANC_DIGI_MIC:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC;
            break;
			
         case AUDDRV_DEV_SIDETONE_INPUT:
            cslDev = CSL_CAPH_DEV_SIDETONE_INPUT;
            break;	
			
         case AUDDRV_DEV_EANC_INPUT:
            cslDev = CSL_CAPH_DEV_EANC_INPUT;
            break;
			
         case AUDDRV_DEV_ANALOG_MIC:
            cslDev = CSL_CAPH_DEV_ANALOG_MIC;
            break;
			
         case AUDDRV_DEV_HS_MIC:
            cslDev = CSL_CAPH_DEV_HS_MIC;
            break;
			
         case AUDDRV_DEV_BT_MIC:
            cslDev = CSL_CAPH_DEV_BT_MIC;
            break;	
			
         case AUDDRV_DEV_I2S_RX:
            cslDev = CSL_CAPH_DEV_I2S_RX;
            break;
			
         case AUDDRV_DEV_MEMORY:
            cslDev = CSL_CAPH_DEV_MEMORY;
            break;

         case AUDDRV_DEV_SRCM:
            cslDev = CSL_CAPH_DEV_SRCM;
            break;
			
        default:
            xassert(0, cslDev);
		break;	
    };

	return cslDev;
}

#endif


// ==========================================================================
//
// Function Name: AUDDRV_GetDRVDeviceFromMic
//
// Description: Get the audio driver Device from the Microphone selection.
//
// =========================================================================
AUDDRV_DEVICE_e AUDDRV_GetDRVDeviceFromMic (AUDDRV_MIC_Enum_t mic)
{
	AUDDRV_DEVICE_e dev = AUDDRV_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromMic:: mic = 0x%x\n", mic);

      switch (mic)
      {
        case AUDDRV_MIC_NONE:
            dev = AUDDRV_DEV_NONE;
            break;
			
         case AUDDRV_MIC_SPEECH_DIGI:
            dev = AUDDRV_DEV_DIGI_MIC;
            break;
			
         case AUDDRV_MIC_DIGI1:
            dev = AUDDRV_DEV_DIGI_MIC_L;
            break;	
			
         case AUDDRV_MIC_DIGI2:
            dev = AUDDRV_DEV_DIGI_MIC_R;
            break;
			
         case AUDDRV_MIC_EANC_DIGI:
            dev = AUDDRV_DEV_EANC_DIGI_MIC;
            break;
			
         case AUDDRV_MIC_ANALOG_MAIN:
            dev = AUDDRV_DEV_ANALOG_MIC;
            break;

         case AUDDRV_MIC_ANALOG_AUX:
            dev = AUDDRV_DEV_HS_MIC;
            break;

         case AUDDRV_MIC_PCM_IF:
            dev = AUDDRV_DEV_BT_MIC;
            break;

        default:
           // xassert(0, dev);
		break;	
    };

	return dev;
}

// ==========================================================================
//
// Function Name: AUDDRV_GetDRVDeviceFromSpkr
//
// Description: Get the audio driver Device from the Speaker selection.
//
// =========================================================================
AUDDRV_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr (AUDDRV_SPKR_Enum_t spkr)
{
	AUDDRV_DEVICE_e dev = AUDDRV_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromSpkr:: spkr = 0x%x\n", spkr);

      switch (spkr)
      {
        case AUDDRV_SPKR_NONE:
            dev = AUDDRV_DEV_NONE;
            break;
			
         case AUDDRV_SPKR_EP:
            dev = AUDDRV_DEV_EP;
            break;
			
         case AUDDRV_SPKR_IHF:
            dev = AUDDRV_DEV_IHF;
            break;	
			
         case AUDDRV_SPKR_HS:
            dev = AUDDRV_DEV_HS;
            break;
			
         case AUDDRV_SPKR_VIBRA:
            dev = AUDDRV_DEV_VIBRA;
            break;
			
        default:
            xassert(0, dev);
		break;	
    };

	return dev;
}


#if defined (_ATHENA_)
#include "csl_audvoc.h"

// ==========================================================================
//
// Function Name: AUDDRV_GetCSLDevice
//
// Description: Get the audio csl Device from the Driver device.
//
// =========================================================================
CSL_AUDIO_DEVICE_e AUDDRV_GetCSLDevice (AUDDRV_DEVICE_e dev)
{
	CSL_AUDIO_DEVICE_e cslDev = CSL_AUDVOC_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetCSLDevice:: dev = 0x%x\n", dev);

      switch (dev)
      {
        case AUDDRV_DEV_NONE:
            cslDev = CSL_AUDVOC_DEV_NONE;
            break;
			
         case AUDDRV_RENDER_DEV_AUDIO:
            cslDev = CSL_AUDVOC_DEV_RENDER_AUDIO;
            break;
			
         case AUDDRV_RENDER_DEV_POLYRINGER:
            cslDev = CSL_AUDVOC_DEV_RENDER_POLYRINGER;
            break;	
			
         case AUDDRV_CAPTURE_DEV_AUDIO:
            cslDev = CSL_AUDVOC_DEV_CAPTURE_AUDIO;
            break;
			
         case AUDDRV_CAPTURE_DEV_BTW:
            cslDev = CSL_AUDVOC_DEV_CAPTURE_BTW;
            break;
			
        default:
            xassert(0, cslDev);
		break;	
    };

	return cslDev;
}

#endif

