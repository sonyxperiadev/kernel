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
#include "csl_caph_gain.h"

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
#if defined (_RHEA_) || defined (_SAMOA_)
#include "csl_caph.h"
#include "csl_caph_gain.h"
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

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetCSLDevice:: dev = 0x%x\n", dev));

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
			
         case AUDDRV_DEV_FM_TX:
            cslDev = CSL_CAPH_DEV_FM_TX;
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
			
         case AUDDRV_DEV_EANC_DIGI_MIC_L:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC_L;
            break;

         case AUDDRV_DEV_EANC_DIGI_MIC_R:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC_R;
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
			
         case AUDDRV_DEV_FM_RADIO:
            cslDev = CSL_CAPH_DEV_FM_RADIO;
            break;
			
         case AUDDRV_DEV_MEMORY:
            cslDev = CSL_CAPH_DEV_MEMORY;
            break;

         case AUDDRV_DEV_SRCM:
            cslDev = CSL_CAPH_DEV_SRCM;
            break;
		
        case AUDDRV_DEV_DSP_throughMEM:
            cslDev = CSL_CAPH_DEV_DSP_throughMEM;
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

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromMic:: mic = 0x%x\n", mic));

      switch (mic)
      {
        case AUDDRV_MIC_NONE:
            dev = AUDDRV_DEV_NONE;
            break;
			
         case AUDDRV_MIC_SPEECH_DIGI:
         case AUDDRV_DUAL_MIC_DIGI12:
         case AUDDRV_DUAL_MIC_DIGI21:
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

         case AUDDRV_MIC_USB_IF:
            dev = AUDDRV_DEV_MEMORY;
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

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromSpkr:: spkr = 0x%x\n", spkr));

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
			
         case AUDDRV_SPKR_PCM_IF:
            dev = AUDDRV_DEV_BT_SPKR;
            break;
		
         case AUDDRV_SPKR_USB_IF:
            dev = AUDDRV_DEV_MEMORY;
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

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetCSLDevice:: dev = 0x%x\n", dev));

      switch (dev)
      {
         case AUDDRV_DEV_MEMORY:
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




/****************************************************************************
*
*  Function Name: csl_caUInt16ph_MicDSP_Gain_t AUDDRV_GetDSPULGain(
*                                         AUDDRV_DEVICE_e mic, UInt16 gain)
*
*  Description: read the DSP UL gain
*
****************************************************************************/
Int16 AUDDRV_GetDSPULGain(AUDDRV_DEVICE_e mic, Int16 gain)
{
    csl_caph_Mic_Gain_t outGain;
    csl_caph_MIC_Path_e cslMic = MIC_ANALOG_HEADSET;

    memset(&outGain, 0, sizeof(csl_caph_Mic_Gain_t));
    switch (mic)
    {
	    case AUDDRV_DEV_ANALOG_MIC:
	    case AUDDRV_DEV_HS_MIC:
		    cslMic = MIC_ANALOG_HEADSET;
		    break;
	    
	    case AUDDRV_DEV_DIGI_MIC_L:
	    case AUDDRV_DEV_DIGI_MIC_R:
	    case AUDDRV_DEV_EANC_DIGI_MIC_L:
	    case AUDDRV_DEV_EANC_DIGI_MIC_R:
		    cslMic = MIC_DIGITAL;
		    break;

	    default:
		    // For all others, just use
		    // DSP DL gain as analog mic.
		    cslMic = MIC_ANALOG_HEADSET;
		    break;
    }

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDSPULGain::mic=0x%x, gain=0x%x\n", mic, gain));
    outGain = csl_caph_gain_GetMicGain(cslMic, gain);
    return outGain.micDSPULGain;
}

/****************************************************************************
*
*  Function Name: Int16 AUDDRV_GetDSPDLGain_Q1_14(
*                                         AUDDRV_DEVICE_e mic, Int16 gain)
*
*  Description: read the DSP DL gain in dB in Q1.14
*
****************************************************************************/
Int16 AUDDRV_GetDSPDLGain_Q1_14(AUDDRV_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case AUDDRV_DEV_EP:
		    cslSpkr = SPKR_EP_DSP;
		    break;
	    
	    case AUDDRV_DEV_HS:
	    case AUDDRV_DEV_IHF:
		    cslSpkr = SPKR_IHF_HS_DSP;
		    break;


	    case AUDDRV_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP_DSP;
		    break;

	    case AUDDRV_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP_DSP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP_DSP;
		    break;

    }

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDSPDLGain_Q1_14::spkr=0x%x, gain=0x%x\n", spkr, gain));
    outGain = csl_caph_gain_GetSpkrGain_Q1_14(cslSpkr, gain);
    return outGain.spkrDSPDLGain;
}


/****************************************************************************
*
*  Function Name: Int16 AUDDRV_GetDSPDLGain(
*                                         AUDDRV_DEVICE_e mic, UInt16 gain)
*
*  Description: read the DSP DL gain in mdB in Q15
*
****************************************************************************/
Int16 AUDDRV_GetDSPDLGain(AUDDRV_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case AUDDRV_DEV_EP:
		    cslSpkr = SPKR_EP_DSP;
		    break;
	    
	    case AUDDRV_DEV_HS:
	    case AUDDRV_DEV_IHF:
		    cslSpkr = SPKR_IHF_HS_DSP;
		    break;

	    case AUDDRV_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP_DSP;
		    break;

	    case AUDDRV_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP_DSP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP_DSP;
		    break;

    }

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDSPDLGain::spkr=0x%x, gain=0x%x\n", spkr, gain));
    outGain = csl_caph_gain_GetSpkrGain(cslSpkr, gain);
    return outGain.spkrDSPDLGain;
}



/****************************************************************************
*
*  Function Name: Int16 AUDDRV_GetHWDLGain(
*                                         AUDDRV_DEVICE_e mic, UInt16 gain)
*
*  Description: read the HW DL gain in Q13.2
*
****************************************************************************/
Int16 AUDDRV_GetHWDLGain(AUDDRV_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case AUDDRV_DEV_EP:
		    cslSpkr = SPKR_EP;
		    break;
	    
	    case AUDDRV_DEV_HS:
			cslSpkr = SPKR_HS;
		    break;
			
	    case AUDDRV_DEV_IHF:
		    cslSpkr = SPKR_IHF;
		    break;

	    case AUDDRV_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;

	    case AUDDRV_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP;
		    break;

    }

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetHWDLGain::spkr=0x%x, gain=0x%x\n", spkr, gain));
    outGain = csl_caph_gain_GetSpkrGain(cslSpkr, gain);
    return outGain.spkrHWGain;
}



/****************************************************************************
*
*  Function Name: Int16 AUDDRV_GetHWDLGain_Q1_14(
*                                         AUDDRV_DEVICE_e mic, Int16 gain)
*
*  Description: read the HW DL gain in dB in Q1.14
*
****************************************************************************/
Int16 AUDDRV_GetHWDLGain_Q1_14(AUDDRV_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case AUDDRV_DEV_EP:
		    cslSpkr = SPKR_EP;
		    break;
	    
	    case AUDDRV_DEV_HS:
			 cslSpkr = SPKR_HS;
		    break;
				
	    case AUDDRV_DEV_IHF:
		    cslSpkr = SPKR_IHF;
		    break;


	    case AUDDRV_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;

	    case AUDDRV_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP;
		    break;

    }

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetHWDLGain_Q1_14::spkr=0x%x, gain=0x%x\n", spkr, gain));
    outGain = csl_caph_gain_GetSpkrGain_Q1_14(cslSpkr, gain);
    return outGain.spkrHWGain;
}



/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetMixerInputGain(Int16 gain)
*
*  Description: Get the Mixer input gain. Param "gain" is in Q13.2
*               Mixer input gain is register value.
*
****************************************************************************/
UInt16 AUDDRV_GetMixerInputGain(Int16 gain)
{
    csl_caph_Mixer_GainMapping_t outGain;
    outGain = csl_caph_gain_GetMixerGain(gain);
    return outGain.mixerInputGain;
}


/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetMixerOutputFineGain(Int16 gain)
*
*  Description: Get the Mixer output fine gain. Param "gain" is in Q13.2
*               Mixer output fine gain is register value.
*
****************************************************************************/
UInt16 AUDDRV_GetMixerOutputFineGain(Int16 gain)
{
    csl_caph_Mixer_GainMapping_t outGain;
    outGain = csl_caph_gain_GetMixerGain(gain);
    return outGain.mixerOutputFineGain;
}

/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetMixerOutputCoarseGain(Int16 gain)
*
*  Description: Get the Mixer output coarse gain. Param "gain" is in Q13.2
*               Mixer output coarse gain is register value.
*
****************************************************************************/
UInt16 AUDDRV_GetMixerOutputCoarseGain(Int16 gain)
{
    csl_caph_Mixer_GainMapping2_t outGain;
    outGain = csl_caph_gain_GetMixerOutputCoarseGain(gain);
    return outGain.mixerOutputCoarseGain;
}

/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetPMUGain(
*                                         AUDDRV_DEVICE_e mic, UInt16 gain)
*
*  Description: read the PMU gain in dB in Q13,2. Input gain in Q13.2
*
****************************************************************************/
UInt16 AUDDRV_GetPMUGain(AUDDRV_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case AUDDRV_DEV_EP:
		    cslSpkr = SPKR_EP;
		    break;
	    
	    case AUDDRV_DEV_HS:
			cslSpkr = SPKR_HS;
		    break;
			
	    case AUDDRV_DEV_IHF:
		    cslSpkr = SPKR_IHF;
		    break;

	    case AUDDRV_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;

	    case AUDDRV_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP;
		    break;

    }

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetPMUGain::spkr=0x%x, gain=0x%x\n", spkr, gain));
    outGain = csl_caph_gain_GetSpkrGain(cslSpkr, gain);
    return outGain.spkrPMUGain;
}



/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetPMUGain_Q1_14(
*                                         AUDDRV_DEVICE_e mic, UInt16 gain)
*
*  Description: read the DSP DL gain in dB in Q1.14
*
****************************************************************************/
UInt16 AUDDRV_GetPMUGain_Q1_14(AUDDRV_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case AUDDRV_DEV_EP:
		    cslSpkr = SPKR_EP;
		    break;
	    
	    case AUDDRV_DEV_HS:
			 cslSpkr = SPKR_HS;
		    break;
			
	    case AUDDRV_DEV_IHF:
		    cslSpkr = SPKR_IHF;
		    break;


	    case AUDDRV_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;

	    case AUDDRV_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP;
		    break;

    }

    _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetPMUGain_Q1_14::spkr=0x%x, gain=0x%x\n", spkr, gain));
    outGain = csl_caph_gain_GetSpkrGain_Q1_14(cslSpkr, gain);
    return outGain.spkrPMUGain;
}


