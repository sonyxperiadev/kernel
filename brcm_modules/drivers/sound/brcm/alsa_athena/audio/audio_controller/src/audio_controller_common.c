/******************************************************************************
Copyright 2009 Broadcom Corporation.  All rights reserved.

This program is the proprietary software of Broadcom Corporation and/or its 
licensors, and may only be used, duplicated, modified or distributed pursuant 
to the terms and conditions of a separate, written license agreement executed 
between you and Broadcom (an "Authorized License").

Except as set forth in an Authorized License, Broadcom grants no license
(express or implied), right to use, or waiver of any kind with respect to the 
Software, and Broadcom expressly reserves all rights in and to the Software and 
all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, 
THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY 
NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
  
 Except as expressly set forth in the Authorized License,
1. This program, including its structure, sequence and organization, 
constitutes the valuable trade secrets of Broadcom, and you shall use all 
reasonable efforts to protect the confidentiality thereof, and to use this 
information only in connection with your use of Broadcom integrated circuit 
products.

2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
USE OR PERFORMANCE OF THE SOFTWARE.

3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE 
OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
ANY LIMITED REMEDY.
******************************************************************************/
/**
*
* @file   audio_controller.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#if !defined(WIN32)
#ifdef UNDER_LINUX
#include "plat/osdal_os.h"   
#endif

#include "mobcom_types.h"
#include "resultcode.h"

#include "audio_consts.h"
#include "auddrv_def.h"
#ifdef UNDER_LINUX
#include "linux/broadcom/bcm_fuse_sysparm_CIB.h"
#else
#include "sysparm.h"
#endif
#include "audio_gain_table.h"
#include "auddrv_def.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "dspcmd.h"
#include "audio_controller.h"
#ifdef UNDER_LINUX
#include <linux/spinlock_types.h>
#include "plat/bcm_i2sdai.h"
#include "i2s_drv_new.h"
#else
#include "i2s.h"
#endif
#include "log.h"
#include "ostask.h"
#include "xassert.h"

#if !defined(NO_PMU)
#if defined(EXTERNAL_AMP_CONTROL)
#include "hal_pmu.h"
#include "hal_pmu_private.h"
#endif
#endif

//=============================================================================
// Public Variable declarations
//=============================================================================
AUDDRV_SPKR_Enum_t voiceCallSpkr = AUDDRV_SPKR_NONE;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

static AUDDRV_MIC_Enum_t	GetDrvMic (AUDCTRL_MICROPHONE_t mic);
static AUDDRV_SPKR_Enum_t   GetDrvSpk (AUDCTRL_SPEAKER_t speaker);

static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_EnableTelephony
//
// Description:   Enable telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_EnableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	AUDDRV_MIC_Enum_t	micSel;
	AUDDRV_SPKR_Enum_t	spkSel;
	
	// mic selection. 
	micSel = GetDrvMic (mic);

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	spkSel = GetDrvSpk (speaker);

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTelephony::  spkSel %d, micSel %d \n", spkSel, micSel );

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_Init ( micSel, spkSel );
	voiceCallSpkr = spkSel;

	// in case it was muted from last voice call,
	AUDCTRL_SetTelephonySpkrMute (dlSink, speaker, FALSE); 
	// in case it was muted from last voice call,
	AUDCTRL_SetTelephonyMicMute (ulSrc, mic, FALSE); 


	OSTASK_Sleep( 100 );
	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );

	return;
}


void AUDCTRL_RateChangeTelephony( void )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_RateChangeTelephony::  stAudioMode %d \n",stAudioMode);

	AUDCTRL_SetAudioMode ( stAudioMode );

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_RateChange();

}

//============================================================================
//
// Function Name: AUDCTRL_DisableTelephony
//
// Description:   disable telephony path, both dl and ul
//
//============================================================================
void AUDCTRL_DisableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableTelephony \n" );

	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, FALSE );
	OSTASK_Sleep( 100 );

	// The following is the sequence we need to follow
	AUDDRV_Telephony_Deinit ();
	voiceCallSpkr = AUDDRV_SPKR_NONE;

	return;
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
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	AUDDRV_MIC_Enum_t	micSel; 
	AUDDRV_SPKR_Enum_t	spkSel;
	
	micSel = GetDrvMic (mic);
	spkSel = GetDrvSpk (speaker);

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicSpkr::  spkSel %d, micSel %d \n", spkSel, micSel );

	//driver needs to know mode!
	if(stAudioMode == AUDIO_MODE_USB) AUDCTRL_SetAudioMode ( AUDIO_MODE_HANDSET ); //hw
	else AUDCTRL_SetAudioMode ( stAudioMode );

	AUDDRV_Telephony_SelectMicSpkr ( micSel, spkSel );
	voiceCallSpkr = spkSel;
	//need to think about better design!!  do mode switch after EC off, mic mute, etc.

	OSTASK_Sleep( 100 );  //depending on switch to headset or off of headset, PMU is first off or last on.
	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrMute: mute = 0x%x\n",  mute);
	AUDDRV_SetMute( AUDDRV_MUTE_VOICE_OUTPUT, mute);
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicGain
//
// Description:   Set ul gain of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_HW_ID_t			ulSrc,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					gain
				)
{
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
	return AUDDRV_IsVoiceCallWB(AUDIO_MODE_INVALID);
}

//*********************************************************************
//  Get current (voice call) audio mode 
//	@return		mode		(voice call) audio mode 
//**********************************************************************/
AudioMode_t AUDCTRL_GetAudioMode( void )
{
	return AUDDRV_GetAudioMode( );
}

//*********************************************************************
//  Save audio mode before call AUDCTRL_SetAudioMode( )
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
	AUDDRV_SetAudioMode( mode );
}


//=============================================================================
// Private function definitions
//=============================================================================

//============================================================================
//
// Function Name: GetDrvMic
//
// Description:   convert audio controller microphone enum to auddrv microphone enum
//
//============================================================================
static AUDDRV_MIC_Enum_t GetDrvMic (AUDCTRL_MICROPHONE_t mic)
{
	AUDDRV_MIC_Enum_t micSel=AUDDRV_MIC_MAIN;

	// microphone selection. We hardcode microphone for headset,handset and loud right now. 
	// Later, need to provide a configurable table.
	switch (mic)
	{
		case AUDCTRL_MIC_MAIN:
			micSel = AUDDRV_MIC_MAIN;
			break;
		case AUDCTRL_MIC_AUX:
			micSel = AUDDRV_MIC_AUX;
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
		case AUDCTRL_DUAL_MIC_ANALOG_DIGI1:
			micSel = AUDDRV_DUAL_MIC_ANALOG_DIGI1;
			break;
		case AUDCTRL_DUAL_MIC_ANALOG_DIGI2:
			micSel = AUDDRV_DUAL_MIC_ANALOG_DIGI2;
			break;

		case AUDCTRL_MIC_BTM:
			micSel = AUDDRV_MIC_PCM_IF;
			break;
		//case AUDCTRL_MIC_BTS:
			//break;
		case AUDCTRL_MIC_I2S:
			break;

		case AUDCTRL_MIC_USB:
		default:
			Log_DebugPrintf(LOGID_AUDIO,"GetDrvMic: Unsupported microphpne type. mic = 0x%x\n", mic);
			break;
	}

	return micSel;
}


//============================================================================
//
// Function Name: GetDrvSpk
//
// Description:   convert audio controller speaker enum to auddrv speaker enum
//
//============================================================================
static AUDDRV_SPKR_Enum_t GetDrvSpk (AUDCTRL_SPEAKER_t speaker)
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
		default:
			Log_DebugPrintf(LOGID_AUDIO,"GetDrvSpk: Unsupported Speaker type. spk = 0x%x\n", speaker);
			break;
	}

	return spkSel;
}


#else // WIN32

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "audio_gain_table.h"
#include "auddrv_def.h"
#include "csl_aud_drv.h"
#include "audio_manager_commons.h"
#include "audio_controller.h"
#include "i2s.h"
#include "log.h"
#include "xassert.h"
//=============================================================================
// Public Variable declarations
//=============================================================================

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_EnableTelephony
//
// Description:   Enable telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_EnableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RateChangeTelephony
//
// Description:   RateChange telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_RateChangeTelephony()
{
}

//============================================================================
//
// Function Name: AUDCTRL_DisableTelephony
//
// Description:   disable telephony path, both dl and ul
//
//============================================================================
void AUDCTRL_DisableTelephony(
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
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
				AUDIO_HW_ID_t			ulSrc,
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicGain
//
// Description:   Set ul gain of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_HW_ID_t			ulSrc,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					gain
				)
{
}

//*********************************************************************
//
//  Get current (voice call) audio mode 

//	@return		mode		(voice call) audio mode 
//   @note      
//**********************************************************************/
AudioMode_t AUDCTRL_GetAudioMode( void )
{
	return (AudioMode_t)0;
}

//*********************************************************************
//  Save audio mode before call AUDCTRL_SetAudioMode( )
//	@param		mode		(voice call) audio mode 
//	@return		none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode )
{
}

//*********************************************************************
//
//   Set (voice call) audio mode 

//	@param		mode		(voice call) audio mode 
///	@return		
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode )
{
}


//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCall 
//	@return	 TRUE/FALSE (in/out voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCall( void )
{
#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(modeVoiceCall) return TRUE;
	}
#endif
	return FALSE;
}

//*********************************************************************
//  Function Name: AUDCTRL_InVoiceCallWB
//	@return	 TRUE/FALSE (in/out WB voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCallWB( void )
{
#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(modeVoiceCall==2) return TRUE;
	}
#endif
	return FALSE;
}

//=============================================================================
// Private function definitions
//=============================================================================

#endif //WIN32
