/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
* @file   audio_controller.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"

#include "audio_consts.h"
#include "auddrv_def.h"
#ifdef CONFIG_AUDIO_BUILD
#include "sysparm.h"
#include "csl_aud_drv.h"
#endif
#include "drv_caph.h"
#include "drv_caph_hwctrl.h"
#include "audio_gain_table.h"
#include "auddrv_def.h"
#include "audio_vdriver.h"
#include "dspcmd.h"
#include "audio_controller.h"
#ifdef CONFIG_AUDIO_BUILD
#include "i2s.h"
#include "ostask.h"
#endif
#include "log.h"
#include "xassert.h"
#include "audioapi_asic.h"

#if !defined(NO_PMU)
#if defined(EXTERNAL_AMP_CONTROL)
#ifdef CONFIG_AUDIO_BUILD
#include "pmu.h"
#endif
#include "hal_pmu.h"
#include "hal_pmu_private.h"
#endif
#endif

//=============================================================================
// Public Variable declarations
//=============================================================================
AUDDRV_SPKR_Enum_t voiceCallSpkr = AUDDRV_SPKR_NONE;
extern AUDDRV_PathID_t telephonyPathID;
//=============================================================================
// Private Type and Constant declarations
//=============================================================================


static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;
#if defined(USE_NEW_AUDIO_PARAM)
static AudioApp_t stAudioApp = AUDIO_APP_VOICE_CALL;
#endif
//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================
void AUDCTRL_RateChangeTelephony( UInt32 sampleRate )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_RateChangeTelephony::  stAudioMode %d \n",stAudioMode);

	AUDCTRL_SetAudioMode ( stAudioMode );

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_RateChange(sampleRate);

}

//=============================================================================
// Functions
//=============================================================================
UInt32 AUDCTRL_RateGetTelephony()
{
	return AUDDRV_Telephone_GetSampleRate();
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicSpkr
//
// Description:   Set the micphone and speaker to telephony path, previous micophone
//	and speaker is disabled
//
//============================================================================
void AUDCTRL_SetTelephonyMicSpkr(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	AUDDRV_MIC_Enum_t	micSel; 
	AUDDRV_SPKR_Enum_t	spkSel;
    AUDDRV_PathID_t myTelephonyPathID;
	AUDCTRL_Config_t data;
	
    memcpy(&myTelephonyPathID, &telephonyPathID, sizeof(AUDDRV_PathID_t));
    memset(&data, 0, sizeof(AUDCTRL_Config_t));
	micSel = AUDCTRL_GetDrvMic (mic);
	spkSel = AUDCTRL_GetDrvSpk (speaker);

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicSpkr::  spkSel %d, micSel %d \n", spkSel, micSel );

#if defined(_ATHENA_)	
	//driver needs to know mode!
	if(stAudioMode == AUDIO_MODE_USB) AUDCTRL_SetAudioMode ( AUDIO_MODE_HANDSET ); //hw
	else AUDCTRL_SetAudioMode ( stAudioMode );
#else
	AUDCTRL_SaveAudioModeFlag(stAudioMode);	
#endif
	AUDDRV_Telephony_SelectMicSpkr ( micSel, spkSel, 
			(void*)(&myTelephonyPathID) );
    if (telephonyPathID.dlPathID != myTelephonyPathID.dlPathID)
    {
        //Remove the old pathID from the table.
  	    AUDCTRL_RemoveFromTable(telephonyPathID.dlPathID);
        telephonyPathID.dlPathID = myTelephonyPathID.dlPathID;
        if (telephonyPathID.dlPathID != 0)
        {
    	    //Save DL path to the path table.
	        data.pathID = telephonyPathID.dlPathID;
	        data.src = AUDIO_HW_NONE;
	        data.sink = dlSink_not_used;
   	        data.mic = AUDCTRL_MIC_UNDEFINED;
	        data.spk = speaker;
	        data.numCh = AUDIO_CHANNEL_NUM_NONE;
	        data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	        AUDCTRL_AddToTable(&data);
        }
    }
    //If the pathID remains no changed, It may be caused by the CSL
    //layer assigns the same PathID to this new path.
    //So we need to compare the speaker of this new path with
    //the speaker of the old path.
    else
    {
        data = AUDCTRL_GetFromTable(myTelephonyPathID.dlPathID);
	    if (speaker != data.spk)
        {
      	    AUDCTRL_RemoveFromTable(telephonyPathID.dlPathID);
            telephonyPathID.dlPathID = myTelephonyPathID.dlPathID;
            if (telephonyPathID.dlPathID != 0)
            {
    	        //Save DL path to the path table.
	            data.pathID = telephonyPathID.dlPathID;
	            data.src = AUDIO_HW_NONE;
	            data.sink = dlSink_not_used;
   	            data.mic = AUDCTRL_MIC_UNDEFINED;
	            data.spk = speaker;
	            data.numCh = AUDIO_CHANNEL_NUM_NONE;
	            data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	            AUDCTRL_AddToTable(&data);
            }
        }
    }
    if (telephonyPathID.ulPathID != myTelephonyPathID.ulPathID)
    {
        //Remove the old pathID from the table.
  	    AUDCTRL_RemoveFromTable(telephonyPathID.ulPathID);
        telephonyPathID.ulPathID = myTelephonyPathID.ulPathID;
        if (telephonyPathID.ulPathID != 0)
        {
	        //Save UL path to the path table.
    	    data.pathID = telephonyPathID.ulPathID;
	        data.src = ulSrc_not_used;
	        data.sink = AUDIO_HW_NONE;
   	        data.mic = mic;
	        data.spk = AUDCTRL_SPK_UNDEFINED;
	        data.numCh = AUDIO_CHANNEL_NUM_NONE;
	        data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	        AUDCTRL_AddToTable(&data);
        }
    }
    //If the pathID remains no changed, It may be caused by the CSL
    //layer assigns the same PathID to this new path.
    //So we need to compare the mic of this new path with
    //the mic of the old path.
    else
    {
        data = AUDCTRL_GetFromTable(myTelephonyPathID.ulPathID);
	    if (mic != data.mic)
        {
   	        AUDCTRL_RemoveFromTable(telephonyPathID.ulPathID);
            telephonyPathID.ulPathID = myTelephonyPathID.ulPathID;
            if (telephonyPathID.ulPathID != 0)
            {
	            //Save UL path to the path table.
    	        data.pathID = telephonyPathID.ulPathID;
	            data.src = ulSrc_not_used;
	            data.sink = AUDIO_HW_NONE;
   	            data.mic = mic;
	            data.spk = AUDCTRL_SPK_UNDEFINED;
	            data.numCh = AUDIO_CHANNEL_NUM_NONE;
	            data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	            AUDCTRL_AddToTable(&data);
            }
        }
    }

	if (telephonyPathID.ul2PathID != myTelephonyPathID.ul2PathID)
    {
        //Remove the old pathID from the table.
  	    AUDCTRL_RemoveFromTable(telephonyPathID.ul2PathID);
        telephonyPathID.ul2PathID = myTelephonyPathID.ul2PathID;
        if (telephonyPathID.ul2PathID != 0)
        {
	        //Save UL path to the path table.
            data.pathID = telephonyPathID.ul2PathID;
	        data.src = ulSrc_not_used;
	        data.sink = AUDIO_HW_NONE;
    	    data.mic = AUDCTRL_MIC_NOISE_CANCEL;
	        data.spk = AUDCTRL_SPK_UNDEFINED;
	        data.numCh = AUDIO_CHANNEL_NUM_NONE;
	        data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	        AUDCTRL_AddToTable(&data);
        }
    }

	voiceCallSpkr = spkSel;
	//need to think about better design!!  do mode switch after EC off, mic mute, etc.
	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))
	{
#ifdef CONFIG_AUDIO_BUILD
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
#endif	
	}	
	else
	{
#ifdef CONFIG_AUDIO_BUILD	
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
#endif
	}	
		
	OSTASK_Sleep( 100 );  //depending on switch to headset or off of headset, PMU is first off or last on.
	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );
}
//============================================================================
//
// Function Name: AUDCTRL_SetAMRVolume_DL
//
// Description:   set gain on the VPU playback path on the downlink path
//
//============================================================================
void AUDCTRL_SetAMRVolume_DL(UInt16 uVol)
{
#if defined(_ATHENA_) || defined(_RHEA_)
	audio_control_dsp( AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_DL, uVol, 0, 0, 0, 0 );
#endif
}

//============================================================================
//
// Function Name: AUDCTRL_SetAMRVolume_UL
//
// Description:   set gain on the VPU playback path on the uplink path
//
//============================================================================
void AUDCTRL_SetAMRVolume_UL(UInt16 uVol)
{
#if defined(_ATHENA_) || defined(_RHEA_)
	audio_control_dsp( AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_UL, uVol, 0, 0, 0, 0 );
#endif
}

//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCall 
//	@return	 TRUE/FALSE (in/out voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCall( void )
{
	return AUDDRV_InVoiceCall();
}

//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCallWB
//	@return	 TRUE/FALSE (in/out WB voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCallWB( void )
{
	return AUDDRV_IsVoiceCallWB(AUDDRV_GetAudioMode());
}

//*********************************************************************
//  Get current (voice call) audio mode 
//	@return		mode		(voice call) audio mode 
//**********************************************************************/
AudioMode_t AUDCTRL_GetAudioMode( void )
{
	return AUDDRV_GetAudioMode( );
}

#if defined(USE_NEW_AUDIO_PARAM)
//*********************************************************************
//  Save audio mode before call AUDCTRL_SaveAudioModeFlag( )
//	@param		mode		(voice call) audio mode 
//	@param		app			(voice call) audio app 
//	@return		none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode, AudioApp_t app )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SaveAudioModeFlag: mode = %d, app=%d\n",  mode, app);
	stAudioMode = mode;
	stAudioApp = app;
	AUDDRV_SaveAudioMode( mode, app );
}

//*********************************************************************
//   Set (voice call) audio mode 
//	@param		mode		(voice call) audio mode 
//	@param		app		(voice call) audio app 
//	@return		none
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode, AudioApp_t app)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode: mode = %d, app=%d\n",  mode, app);
	AUDCTRL_SaveAudioModeFlag( mode, app );
	AUDDRV_SetAudioMode( mode, app );
}
#else
//*********************************************************************
//  Save audio mode before call AUDCTRL_SaveAudioModeFlag( )
//	@param		mode		(voice call) audio mode 
//	@return		none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SaveAudioModeFlag: mode = %d\n",  mode);
	stAudioMode = mode;
	AUDDRV_SaveAudioMode( mode );
}

//*********************************************************************
//   Set (voice call) audio mode 
//	@param		mode		(voice call) audio mode 
//	@return		none
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode: mode = %d\n",  mode);
	AUDCTRL_SaveAudioModeFlag( mode );
	AUDDRV_SetAudioMode( mode, AUDDRV_MIC1|AUDDRV_MIC2|AUDDRV_SPEAKER);
}
#endif

//=============================================================================
// Private function definitions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_GetDrvMic
//
// Description:   convert audio controller microphone enum to auddrv microphone enum
//
//============================================================================
AUDDRV_MIC_Enum_t AUDCTRL_GetDrvMic (AUDCTRL_MICROPHONE_t mic)
{
	AUDDRV_MIC_Enum_t micSel=AUDDRV_MIC_ANALOG_MAIN;

	// microphone selection. We hardcode microphone for headset,handset and loud right now. 
	// Later, need to provide a configurable table.
	switch (mic)
	{
		case AUDCTRL_MIC_MAIN:
			micSel = AUDDRV_MIC_ANALOG_MAIN;
			break;
		case AUDCTRL_MIC_AUX:
			micSel = AUDDRV_MIC_ANALOG_AUX;
			break;

		case AUDCTRL_MIC_DIGI1:
			micSel = AUDDRV_MIC_DIGI1;
			break;
		case AUDCTRL_MIC_DIGI2:
			micSel = AUDDRV_MIC_DIGI2;
			break;
		case AUDCTRL_DUAL_MIC_DIGI12:
			micSel = AUDDRV_DUAL_MIC_DIGI12;
			break;
		case AUDCTRL_DUAL_MIC_DIGI21:
			micSel = AUDDRV_DUAL_MIC_DIGI21;
			break;
		case AUDCTRL_MIC_SPEECH_DIGI:
			micSel = AUDDRV_MIC_SPEECH_DIGI;
			break;			
		case AUDCTRL_DUAL_MIC_ANALOG_DIGI1:
			micSel = AUDDRV_DUAL_MIC_ANALOG_DIGI1;
			break;
		case AUDCTRL_DUAL_MIC_DIGI1_ANALOG:
			micSel = AUDDRV_DUAL_MIC_DIGI1_ANALOG;
			break;

		case AUDCTRL_MIC_BTM:
			micSel = AUDDRV_MIC_PCM_IF;
			break;
		//case AUDCTRL_MIC_BTS:
			//break;
		case AUDCTRL_MIC_I2S:
			break;

		case AUDCTRL_MIC_USB:
			micSel = AUDDRV_MIC_USB_IF;
			break;

		case AUDCTRL_MIC_NOISE_CANCEL:
			micSel = AUDDRV_MIC_NOISE_CANCEL;
			break;


		default:
			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_GetDrvMic: Unsupported microphpne type. mic = 0x%x\n", mic);
			break;
	}

	return micSel;
}


//============================================================================
//
// Function Name: AUDCTRL_GetDrvSpk
//
// Description:   convert audio controller speaker enum to auddrv speaker enum
//
//============================================================================
AUDDRV_SPKR_Enum_t AUDCTRL_GetDrvSpk (AUDCTRL_SPEAKER_t speaker)
{
	AUDDRV_SPKR_Enum_t spkSel = AUDDRV_SPKR_NONE;

	Log_DebugPrintf(LOGID_AUDIO,"GetDrvSpk: spk = 0x%x\n", speaker);

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	switch (speaker)
	{
		case AUDCTRL_SPK_HANDSET:
			spkSel = AUDDRV_SPKR_EP;
			break;
		case AUDCTRL_SPK_HEADSET:
			spkSel = AUDDRV_SPKR_HS;
			break;
		case AUDCTRL_SPK_LOUDSPK:
			spkSel = AUDDRV_SPKR_IHF;
			break;
		case AUDCTRL_SPK_BTM:
			spkSel = AUDDRV_SPKR_PCM_IF;
			break;
		case AUDCTRL_SPK_BTS:
			break;
		
		case AUDCTRL_SPK_I2S:
			break;

		case AUDCTRL_SPK_USB:
			spkSel = AUDDRV_SPKR_USB_IF;
			break;
		default:
			Log_DebugPrintf(LOGID_AUDIO,"GetDrvSpk: Unsupported Speaker type. spk = 0x%x\n", speaker);
			break;
	}

	return spkSel;
}
