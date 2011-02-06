/******************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

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
* @file   audio_vdriver_common.c
* @brief  Audio VDriver API common for all HW platforms.
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "auddrv_def.h"
//#include "brcm_rdb_sysmap.h"
//#include "brcm_rdb_syscfg.h"
#define SYSCFG_BASE_ADDR      0x08880000      /* SYSCFG core */
#include "shared.h"
#include "dspcmd.h"
#include "ripcmdq.h"
#include "ripisr.h"
#include "audio_consts.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#ifdef UNDER_LINUX
#include "linux/broadcom/bcm_fuse_sysparm_CIB.h"
#else
#include "sysparm.h"
#endif
#include "ostask.h"
#include "audioapi_asic.h"
#include "log.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//=============================================================================
// Public Variable declarations
//=============================================================================

AUDDRV_MIC_Enum_t   currVoiceMic = AUDDRV_MIC_NONE;   //used in pcm i/f control. assume one mic, one spkr.
AUDDRV_SPKR_Enum_t  currVoiceSpkr = AUDDRV_SPKR_NONE;  //used in pcm i/f control. assume one mic, one spkr.
Boolean inVoiceCall = FALSE;
//=============================================================================
// Private Type and Constant declarations
//=============================================================================


static Boolean voiceInPathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut
static Boolean voicePlayOutpathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN

//=============================================================================
// Private function prototypes
//=============================================================================

//=============================================================================
// Functions
//=============================================================================

// CSL driver will send a DSP_ENABLE_DIGITAL_SOUND?? cmd to DSP, 
// But ARM code (audio controller) turns on/off PCM interface.

//=============================================================================
//
// Function Name: AUDDRV_Telephony_Init
//
// Description:   Initialize audio system for voice call
//
//=============================================================================

//Prepare DSP before turn on hardware audio path for voice call.
//  This is part of the control sequence for starting telephony audio.
void AUDDRV_Telephony_Init ( AUDDRV_MIC_Enum_t  mic, AUDDRV_SPKR_Enum_t speaker )
{
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_Init");

	currVoiceMic = mic;
	currVoiceSpkr = speaker;

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Init AP  mic %d, spkr %d *\n\r", mic, speaker);
#else
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Init CP *\n\r");
#endif

	//control HW and flags at AP

	//at beginning
	inVoiceCall = TRUE;  //to prevent sending DSP Audio Enable when enable voice path.

	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
#ifdef	tempInterface_DSP_FEATURE_DUALMIC_BTOUTPUTGAINS
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
#endif 
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );
	
	if( AUDDRV_GetAudioMode() >= AUDIO_MODE_NUMBER )
		AUDDRV_Telephony_InitHW ( mic, speaker, AUDIO_SAMPLING_RATE_16000 );
	else
		AUDDRV_Telephony_InitHW ( mic, speaker, AUDIO_SAMPLING_RATE_8000 );

	//after AUDDRV_Telephony_InitHW to make SRST.
	AUDDRV_SetVCflag(TRUE);  //let HW control logic know.

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );

	OSTASK_Sleep( 40 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );

	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, TRUE, TRUE, 0, 0, 0 );
#ifdef	tempInterface_DSP_FEATURE_DUALMIC_BTOUTPUTGAINS
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
#endif 
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );

	  //per call basis: enable the DTX by calling stack api when call connected
	audio_control_generic( AUDDRV_CPCMD_ENABLE_DSP_DTX, TRUE, 0, 0, 0, 0 );

	if (speaker == AUDDRV_SPKR_PCM_IF)
		AUDDRV_SetPCMOnOff( 1 );
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
	else
	{
		if(currVoiceMic != AUDDRV_MIC_PCM_IF) //need to check mic too.
			AUDDRV_SetPCMOnOff( 0 );
			//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
	}

	return;
}


//=============================================================================
//
// Function Name: AUDDRV_Telephony_RateChange
//
// Description:   Change the sample rate for voice call
//
//=============================================================================

void AUDDRV_Telephony_RateChange( void )
{
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_RateChange");

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_RateChange AP  *\n\r");
#else
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_RateChange CP *\n\r");
#endif

	if ( AUDDRV_GetVCflag() )
	{

	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
#ifdef	tempInterface_DSP_FEATURE_DUALMIC_BTOUTPUTGAINS
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
#endif 
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );

	//need to know the mode!  need to set HW to 16Khz.
	AUDDRV_SetAudioMode( AUDDRV_GetAudioMode());

	//AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, speaker, TRUE, AUDIO_SAMPLING_RATE_8000);
	audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, 0, 0, 0, 0 );

	//AUDDRV_Enable_Input ( AUDDRV_VOICE_INPUT, mic, AUDIO_SAMPLING_RATE_8000);
			
	OSTASK_Sleep( 40 );
	
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, TRUE, TRUE, 0, 0, 0 );
#ifdef	tempInterface_DSP_FEATURE_DUALMIC_BTOUTPUTGAINS
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
#endif 
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
	}

	return;
}

//=============================================================================
//
// Function Name: AUDDRV_Telephony_Deinit
//
// Description:   DeInitialize audio system for voice call
//
//=============================================================================

//Prepare DSP before turn off hardware audio path for voice call. 
// This is part of the control sequence for ending telephony audio.
void AUDDRV_Telephony_Deinit (void )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Deinit voicePlayOutpathEnabled = %d*\n\r", voicePlayOutpathEnabled);
	
	AUDDRV_SetVCflag(FALSE);  //let HW control logic know.

	// a quick fix not to disable voice path for speech playbck when end the phone call.
	if (voicePlayOutpathEnabled == FALSE)
	{
		  //per call basis: disable the DTX by calling stack api when call disconnected
		audio_control_generic( AUDDRV_CPCMD_ENABLE_DSP_DTX, FALSE, 0, 0, 0, 0 );

		audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
#ifdef	tempInterface_DSP_FEATURE_DUALMIC_BTOUTPUTGAINS
		audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
#endif 
		audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	
		audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, FALSE, 0, 0, 0, 0 );

		audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );

		OSTASK_Sleep( 3 ); //make sure audio is off

		AUDDRV_Telephony_DeinitHW( );
	}

	if (AUDIO_CHNL_BLUETOOTH == AUDDRV_GetAudioMode() )
		audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );

	//at last
	inVoiceCall = FALSE;

	return;
}


//=============================================================================
//
// Function Name: AUDDRV_Enable_Output
//
// Description:   Enable audio output for voice call
//
//=============================================================================

void AUDDRV_Enable_Output (
				AUDDRV_InOut_Enum_t    input_path_to_mixer,
				AUDDRV_SPKR_Enum_t     mixer_speaker_selection,
				Boolean                enable_speaker,
				AUDIO_SAMPLING_RATE_t  sample_rate,
				AUDIO_CHANNEL_NUM_t    input_to_mixer
				)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Output mixer %d, en %d input_path_to_mixer %d invoicecall %d, sample_rate %d *\n\r", mixer_speaker_selection, enable_speaker, input_path_to_mixer, inVoiceCall, sample_rate);

	AUDDRV_EnableHWOutput( input_path_to_mixer, mixer_speaker_selection, enable_speaker, sample_rate, input_to_mixer,
		AUDDRV_REASON_HW_CTRL );
	OSTASK_Sleep( 5 );  //sometimes BBC video has no audio. This delay may help the mixer filter and mixer gain loading.

	switch(input_path_to_mixer)
	{
		case AUDDRV_VOICE_OUTPUT:
			
			if(inVoiceCall != TRUE)
			{
				//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_DL
				if (sample_rate == AUDIO_SAMPLING_RATE_8000)
				{
					audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
					audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 0, 0, 0, 0 );
				}
				else
				{
					audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
					audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 1, 0, 0, 0 );
				}
				voicePlayOutpathEnabled = TRUE;

				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Output: inVoiceCall = %d, voicePlayOutpathEnabled = %d\n\r", inVoiceCall, voicePlayOutpathEnabled);
			}	
			
			currVoiceSpkr = mixer_speaker_selection;
			if(inVoiceCall != TRUE)
			{
				if (currVoiceSpkr == AUDDRV_SPKR_PCM_IF)
					AUDDRV_SetPCMOnOff( 1 );
					//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
				else
				{
					if(currVoiceMic != AUDDRV_MIC_PCM_IF) //need to check mic too.
						AUDDRV_SetPCMOnOff( 0 );
						//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
				}
				
			}
			//else, Enable_Input( ) set pcm i/f.

			break;

		case AUDDRV_AUDIO_OUTPUT:
			break;

		case AUDDRV_RINGTONE_OUTPUT:
			break;

		default:
			break;
	}
}


//=============================================================================
//
// Function Name: AUDDRV_Disable_Output
//
// Description:   Disable audio output for voice call
//
//=============================================================================

void AUDDRV_Disable_Output ( AUDDRV_InOut_Enum_t  path )
{
	switch(path) 
	{
		case AUDDRV_VOICE_OUTPUT:
				
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Disable_Output *\n\r" );

			if(inVoiceCall != TRUE)
			{
				if ( voiceInPathEnabled==FALSE )
				{
				//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_DL
				audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 0, 0, 0, 0, 0 );
				audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 0, 0, 0, 0, 0 );
				voicePlayOutpathEnabled = FALSE;
				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Disable_Output: inVoiceCall = %d, voicePlayOutpathEnabled = %d\n\r", voicePlayOutpathEnabled);
				}
			}
			
			if (currVoiceSpkr == AUDDRV_SPKR_PCM_IF)  //turn off PCM i/f
			{
				currVoiceSpkr = AUDDRV_SPKR_NONE;
				if(currVoiceMic != AUDDRV_MIC_PCM_IF)
					audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
			} //else, no need to care PCM i/f.

			currVoiceSpkr = AUDDRV_SPKR_NONE;

			break;
			
			
	    case AUDDRV_RINGTONE_OUTPUT:
			/* this is needed for the case of amr playback followed by mid playback */
			//AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 0, 0, 0);
			break;

		default:
			break;
	}

	OSTASK_Sleep( 3 ); //make sure audio is done

	AUDDRV_DisableHWOutput ( path, AUDDRV_REASON_HW_CTRL );
}



//=============================================================================
//
// Function Name: AUDDRV_Enable_Input
//
// Description:   Enable audio input for voice call
//
//=============================================================================

void AUDDRV_Enable_Input (
                    AUDDRV_InOut_Enum_t      input_path,
                    AUDDRV_MIC_Enum_t        mic_selection,
					AUDIO_SAMPLING_RATE_t    sample_rate )
{
	AUDDRV_EnableHWInput ( input_path, mic_selection, sample_rate,
		AUDDRV_REASON_HW_CTRL );

	switch(input_path) {
	case AUDDRV_VOICE_INPUT:
		
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Input mic_selection %d *\n\r", mic_selection );

		if(inVoiceCall != TRUE)
		{
			//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_UL
			if (sample_rate == AUDIO_SAMPLING_RATE_8000)
			{
				audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 0, 0, 0, 0);
				audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
			}
			else
			{
				audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 1, 0, 0, 0);
				audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
			}
			voiceInPathEnabled = TRUE;
		}

		currVoiceMic = mic_selection;
		if (currVoiceMic == AUDDRV_MIC_PCM_IF)
			AUDDRV_SetPCMOnOff( 1 );
			//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
		else
		{
			if (currVoiceSpkr != AUDDRV_SPKR_PCM_IF) //need to check spkr too.
				AUDDRV_SetPCMOnOff( 0 );
				//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
		}
			
		break;

	default:
		break;
	}
}


//=============================================================================
//
// Function Name: AUDDRV_Disable_Input
//
// Description:   Disable audio input for voice call
//
//=============================================================================

void AUDDRV_Disable_Input (  AUDDRV_InOut_Enum_t      path )
{
	switch(path) {
	case AUDDRV_VOICE_INPUT:

		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Disable_Input *\n\r" );

		
		if(inVoiceCall != TRUE)
		{
			if ( voicePlayOutpathEnabled == FALSE )
			{
			//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_UL
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 0, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0);
			}
			voiceInPathEnabled = FALSE;
		
		    if (currVoiceMic == AUDDRV_MIC_PCM_IF)  //turn off PCM
		    {
			    currVoiceMic = AUDDRV_MIC_NONE;
			    if (currVoiceSpkr != AUDDRV_SPKR_PCM_IF)  //turn off PCM
			    {
				    audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
			    }
		    } //else, no need to care PCM i/f.

		    OSTASK_Sleep( 3 ); //make sure audio is done

		    currVoiceMic = AUDDRV_MIC_NONE;
        }
	    AUDDRV_DisableHWInput ( path, AUDDRV_REASON_HW_CTRL );

        break;

	default:
		AUDDRV_DisableHWInput ( path, AUDDRV_REASON_HW_CTRL );
		break;
	}
}


//=============================================================================
//
// Function Name: AUDDRV_IsVoiceCallWB
//
// Description:   Return Wideband Voice Call status
//
//=============================================================================

Boolean AUDDRV_IsVoiceCallWB(AudioMode_t audio_mode)
{
	Boolean is_wb;
	if(audio_mode<AUDIO_MODE_NUMBER)
		is_wb = FALSE;
	else
		is_wb = TRUE;
	return is_wb;
}

//=============================================================================
//
// Function Name: AUDDRV_IsCall16K
//
// Description:   Return Voice Call 16KHz sample rate status
//
//=============================================================================

Boolean AUDDRV_IsCall16K(AudioMode_t voiceMode)
{
	Boolean is_call16k = FALSE;
	switch(voiceMode)
		{
		case	AUDIO_MODE_HANDSET_WB:
		case	AUDIO_MODE_HEADSET_WB:
		case	AUDIO_MODE_HANDSFREE_WB:
		//case	AUDIO_MODE_BLUETOOTH_WB:
		case	AUDIO_MODE_SPEAKERPHONE_WB:
		case	AUDIO_MODE_TTY_WB:
		case	AUDIO_MODE_HAC_WB:
		case	AUDIO_MODE_USB_WB:
		case	AUDIO_MODE_RESERVE_WB:
			is_call16k = TRUE;
			break;
		default:
			break;
		}
	return is_call16k;
}
//=============================================================================
//
// Function Name: AUDDRV_InVoiceCall
//
// Description:   Return Voice Call status
//
//=============================================================================

Boolean AUDDRV_InVoiceCall( void )
{
	return inVoiceCall;
}

//=============================================================================
//
// Function Name: AUDDRV_GetAudioParam
//
// Description:   Return Audio Parameters
// 
//=============================================================================

void AUDDRV_GetAudioParam ( AudioParam_t audioParamType, void* param )
{
}
//=============================================================================
//
// Function Name: AUDDRV_SetAudioParam
//
// Description:   Set Audio Parameters
// 
//=============================================================================

void AUDDRV_SetAudioParam ( AudioParam_t audioParamType, void* param )
{
}



//=============================================================================
//
// Function Name: AUDDRV_SetPCMOnOff
//
// Description:   	set PCM on/off for BT
// 
//=============================================================================

void AUDDRV_SetPCMOnOff(Boolean	on_off)
{
	  //return;

#define DSP_AUDIO_REG_AMCR  0xe540

	// By default the PCM port is occupied by trace port on development board
	if(on_off)
	{
#define SYSCFG_IOCR0                     (SYSCFG_BASE_ADDR + 0x0000)  /* IOCR0 bit I/O Configuration Register 0       */
#define SYSCFG_IOCR0_PCM_MUX                       0x00C00000 

		//mux to PCM interface (set to 00)
		//*(volatile UInt32*) SYSCFG_IOCR0 &= ~(SYSCFG_IOCR0_PCM_MUX);

		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);

	}
	else
	{
		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);

	}	
}

