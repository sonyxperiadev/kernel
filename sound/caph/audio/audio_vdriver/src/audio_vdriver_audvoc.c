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
* @file   audio_vdriver_audvoc.c
* @brief  Audio VDriver API for Athena-type Audio HW
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "shared.h"
#include "dspcmd.h"
#include "ripcmdq.h"
#include "ripisr.h"
#include "audio_consts.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
#include "ostask.h"
#include "audioapi_asic.h"
#include "log.h"
#include "syscfg_drv.h"
#include "sysparm.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//=============================================================================
// Public Variable declarations
//=============================================================================
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO))
extern void CP_Audio_ISR_Handler(StatQ_t status_msg);
#else
extern void AP_Audio_ISR_Handler(StatQ_t status_msg);
#endif

#if !(defined(FUSE_APPS_PROCESSOR) && (defined(IPC_FOR_BSP_ONLY) || defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)))
#else
extern void IPC_Audio_Create_BufferPool( void );
#endif

#ifdef VPU_INCLUDED
#if !(defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) )
extern void AP_VPU_ProcessStatus( void );
#endif
#endif

typedef void (*AUDDRV_User_CB) (UInt32 param1, UInt32 param2, UInt32 param3);

AUDDRV_MIC_Enum_t   currVoiceMic = AUDDRV_MIC_NONE;   //used in pcm i/f control. assume one mic, one spkr.
AUDDRV_SPKR_Enum_t  currVoiceSpkr = AUDDRV_SPKR_NONE;  //used in pcm i/f control. assume one mic, one spkr.
Boolean inVoiceCall = FALSE;

extern Boolean voicePlayOutpathEnabled;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN
extern Boolean controlFlagForCustomGain;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

static Boolean voiceInPathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut

static AudioEqualizer_en_t	sEqualizerType = EQU_NORMAL;
static void *sUserCB = NULL;

//=============================================================================
// Private function prototypes
//=============================================================================


//=============================================================================
// Functions
//=============================================================================



// CSL driver will send a DSP_AUDIO_ENABLE cmd to DSP, dsp access AMCR.bit5. 
// But ARM code (audio controller) tracks the usage of this bit and control it.

// PCM I/F is accessed and controlled by DSP.
// CSL driver will send a DSP_ENABLE_DIGITAL_SOUND?? cmd to DSP, dsp turns on/off PCM interface.
// But ARM code (audio controller) tracks the usage of this bit and control it.


#if defined(FUSE_APPS_PROCESSOR)
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmAccessPtr
//!
//! Description:	audio parm acces for FUSE AP only
//!
/////////////////////////////////////////////////////////////////////////////
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetAudioParmAccessPtr();
#endif
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmMMAccessPtr
//!
//! Description:	audio parm multimedia acces for FUSE AP only
//!
/////////////////////////////////////////////////////////////////////////////
static SysIndMultimediaAudioParm_t* AUDIO_GetParmMMAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetMultimediaAudioParmAccessPtr();
#endif
}

#if defined(USE_NEW_AUDIO_PARAM)
#define AUDIOMODE_PARM_ACCESSOR(app, mode)	 AUDIO_GetParmAccessPtr()[app][mode]
#else
#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()[mode]
#endif

#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	 AUDIO_GetParmMMAccessPtr()[mode]


#else //#if defined(FUSE_APPS_PROCESSOR)

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmAccessPtr
//!
//! Description:	audio parm access on CP
//! 
/////////////////////////////////////////////////////////////////////////////
static SysCalDataInd_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return SYSPARM_GetAudioParmAccessPtr();
#endif
}


  //CP
#if defined(USE_NEW_AUDIO_PARAM)
#define AUDIOMODE_PARM_ACCESSOR(app, mode)	 AUDIO_GetParmAccessPtr()->audio_parm[app][mode]
#else
#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()->audio_parm[mode]
#endif

#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()->mmaudio_parm[mode]

#endif //#if defined(FUSE_APPS_PROCESSOR)



//=============================================================================
//
// Function Name: AUDDRV_Telephony_SelectMicSpkr
//
// Description:   Select the mic and speaker for voice call
//
//=============================================================================

void AUDDRV_Telephony_SelectMicSpkr (
					AUDDRV_MIC_Enum_t   mic,
					AUDDRV_SPKR_Enum_t  speaker )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr (1) mic %d, spkr %d *\n\r", mic, speaker);

#if defined(ATHENARAY)
  #if defined(THUNDERBIRD)
			//thunderbird, loud speaker go to PMU
		if( currVoiceMic == mic && currVoiceSpkr == speaker)
			return;
  #else
			//AthenaRay board
			// loud speaker should go to PMU, but right now, using the same as HANDSET
		//if( currVoiceMic == mic && currVoiceSpkr == speaker)
		//	return;
  #endif
#endif

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr (2) mic %d, spkr %d *\n\r", mic, speaker);

	currVoiceMic = mic;
	currVoiceSpkr = speaker;

	//need to follow the sequence. avoid enable again
	AUDDRV_SelectSpkr( AUDDRV_VOICE_OUTPUT, speaker, AUDDRV_SPKR_NONE );

	//select mic input, include DMIC support
	AUDDRV_SelectMic( AUDDRV_VOICE_INPUT, mic);

	//if( speaker == AUDDRV_SPKR_PCM_IF || mic==AUDDRV_MIC_PCM_IF )
	if( mic==AUDDRV_MIC_PCM_IF )
	{
		//use audio_control_dsp( ), and combine this file with csl_aud_drv_hw.c
		AUDDRV_SetPCMOnOff( 1 );

		//not reliable, sometimes there is no audio.
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
	}
	else
	{
		AUDDRV_SetPCMOnOff( 0 );

		//not reliable, sometimes there is no audio.
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
	}
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
				AUDIO_CHANNEL_NUM_t    input_to_mixer,
                void *                 callback
				)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Output mixer %d, en %d input_path_to_mixer %d invoicecall %d, sample_rate %d, callback %d *\n\r", 
          mixer_speaker_selection, enable_speaker, input_path_to_mixer, inVoiceCall, sample_rate, callback);

	AUDDRV_EnableHWOutput( input_path_to_mixer, mixer_speaker_selection, enable_speaker, sample_rate, input_to_mixer,
		AUDDRV_REASON_HW_CTRL, (audio_HWEnabled_Cb_t)callback );
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
	AUDDRV_DisableHWOutput ( path, AUDDRV_REASON_HW_CTRL );

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
			
		default:
			break;
	}
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
	AUDDRV_DisableHWInput ( path, AUDDRV_REASON_HW_CTRL );

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
		}

		if (currVoiceMic == AUDDRV_MIC_PCM_IF)  //turn off PCM
		{
			currVoiceMic = AUDDRV_MIC_NONE;
			if (currVoiceSpkr != AUDDRV_SPKR_PCM_IF)  //turn off PCM
			{
				audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
			}
		} //else, no need to care PCM i/f.

		currVoiceMic = AUDDRV_MIC_NONE;

		break;

	default:
		break;
	}
}


#if defined(USE_NEW_AUDIO_PARAM)
static AudioApp_t currAudioApp = AUDIO_APP_VOICE_CALL;

//=============================================================================
//
// Function Name: AUDDRV_GetAudioApp
//
// Description:   get audio application.
//
//=============================================================================

AudioApp_t AUDDRV_GetAudioApp( void )
{
	return currAudioApp;
}

#endif

//=============================================================================
//
// Function Name: AUDDRV_SaveAudioMode
//
// Description:   save audio mode.
//
//=============================================================================

// intentionally put this static var here to prevent any code above this lines to use this var.
static AudioMode_t currAudioMode = AUDIO_MODE_HANDSET;  //need to update this on AP and also in audioapi.c on CP.
static AudioMode_t currMusicAudioMode = AUDIO_MODE_HANDSET;

#if defined(USE_NEW_AUDIO_PARAM)
void AUDDRV_SaveAudioMode( AudioMode_t audio_mode, AudioApp_t audio_app)
{
	currAudioMode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER);
	currAudioApp = audio_app;

	if (audio_mode >= AUDIO_MODE_NUMBER_VOICE)
	{
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SaveAudioMode() invalid audio_mode==%d\n\r", audio_mode );
	}
	else if (( audio_mode >= AUDIO_MODE_NUMBER ) && (audio_mode < AUDIO_MODE_NUMBER_VOICE ))
	{
//		currAudioMode = (AudioMode_t) (audio_mode - AUDIO_MODE_NUMBER);
		if (audio_app == AUDIO_APP_VOICE_CALL) currAudioApp = AUDIO_APP_VOICE_CALL_WB;
	}

}
#else
void AUDDRV_SaveAudioMode( AudioMode_t audio_mode )
{

	currAudioMode = audio_mode; // update mode
}
#endif

//=============================================================================
//
// Function Name: AUDDRV_GetAudioMode
//
// Description:   Return Audio mode
//
//=============================================================================
// For new audio param format, currAudioMode only takes value from 0 to 8.
// But the return value of AUDDRV_GetAudioMode can be [0 to 8] for NB and [9 to 17] for WB
AudioMode_t AUDDRV_GetAudioMode( void )
{
#if defined(USE_NEW_AUDIO_PARAM)
	AudioMode_t mode;
	if (currAudioApp == AUDIO_APP_VOICE_CALL) //NB application
		mode = currAudioMode;
	else if(currAudioApp == AUDIO_APP_VOICE_CALL_WB) //WB application
		mode =  (AudioMode_t)(currAudioMode + AUDIO_MODE_NUMBER);
	return mode;
	//extend to check further applications to be NB or WB mode
#else
	return currAudioMode;
#endif
}


//=============================================================================
//
// Function Name: AUDDRV_SetAudioMode
//
// Description:   set audio mode.
//
//=============================================================================
#if defined(USE_NEW_AUDIO_PARAM)
void AUDDRV_SetAudioMode( AudioMode_t audio_mode, AudioApp_t audio_app)
#else
void AUDDRV_SetAudioMode( AudioMode_t audio_mode)
#endif
{

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_mode==%d\n\r", audio_mode );

	// load DSP parameters:
#if defined(USE_NEW_AUDIO_PARAM)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_app==%d\n\r", audio_app );

	currAudioMode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER); // update mode
	currMusicAudioMode = currAudioMode;
	currAudioApp = audio_app;

	if (( audio_mode >= AUDIO_MODE_NUMBER ) && (audio_mode < AUDIO_MODE_NUMBER_VOICE ))
	{
		if (audio_app == AUDIO_APP_VOICE_CALL) currAudioApp = AUDIO_APP_VOICE_CALL_WB;
	}
	else if ((audio_app == AUDIO_APP_VOICE_CALL_WB) && (audio_mode < AUDIO_MODE_NUMBER))
		audio_mode = (AudioMode_t)(audio_mode + AUDIO_MODE_NUMBER);

	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, audio_mode, 0, 0, 0, 0 );
	
#else
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, audio_mode, 0, 0, 0, 0 );
	currAudioMode = audio_mode;

	if ( audio_mode >= AUDIO_MODE_NUMBER )
		currMusicAudioMode = (AudioMode_t) (audio_mode - AUDIO_MODE_NUMBER);
#endif
	//AUDDRV_VoiceDAC

		//should be part of audio mode API
	//or make SYSPARM_GetAudioParmAccessPtr to be the same on AP and CP:
	
	if ( controlFlagForCustomGain == FALSE )
	{
#if defined(USE_NEW_AUDIO_PARAM)
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);

#else
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);
#endif

	}

#if defined(USE_NEW_AUDIO_PARAM)
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT_CFGR, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_vcfgr );	
#else
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT_CFGR, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_vcfgr );
#endif	
	// Only load the slopgain for voice call. For voice playback/record, we don't want to load the sysparm value to overwrite what user set.  <==NO, this is wrong.
	// all voice playback goes throught DSP, and shall use DSP SW gain control on the playback path. The voice slopgain is fixed at the sysparm value.
#if defined(USE_NEW_AUDIO_PARAM)
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_SetAudioMode vopath_slopgain %x, mode %d \n\r", AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_vslopgain, currAudioMode );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_vslopgain );
#else
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_SetAudioMode vopath_slopgain %x, mode %d \n\r", AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_vslopgain, currAudioMode );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_vslopgain );
#endif

	AUDDRV_SetEquType ( AUDDRV_TYPE_AUDIO_OUTPUT, sEqualizerType );
	// AUDDRV_SetEquType ( AUDDRV_TYPE_RINGTONE_OUTPUT, sEqualizerType );

	// remove aslopgain loading from sysparm, user set it.
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_AUDIO_OUTPUT_L, AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).audvoc_aslopgain );
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_AUDIO_OUTPUT_R, AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).audvoc_aslopgain );

	//move to CP.  //to read and set this after the sys parm are readable at AP
	//AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );

	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQ,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQCOF[0][0] );
	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN,	(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHGAIN[0][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHOFST[0][0] );

	AUDDRV_SetFilter( AUDDRV_PIIR,			(const UInt16 *) & AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).PR_DAC_IIR[0] );

	// remove pslopgain loading from sysparm, user set it.
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_RINGTONE_OUTPUT_L, AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).audvoc_pslopgain );
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_RINGTONE_OUTPUT_R, AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).audvoc_pslopgain );

#if defined(USE_NEW_AUDIO_PARAM)
	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).MPMbiquad_cfg );
	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );

	if ( controlFlagForCustomGain == FALSE )
	{
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	}
#else
	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioMode).MPMbiquad_cfg );
	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );

	if ( controlFlagForCustomGain == FALSE )
	{
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	}
#endif

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

#if defined(USE_NEW_AUDIO_PARAM)
	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).mic_pga);
	}

	if ( AUDDRV_GetVCflag() )
	{
		  // AUDIO_MODE_BLUETOOTH_WB should not be used for voice call. it should be convert to AUDIO_MODE_BLUETOOTH.
		if ((currAudioApp == AUDIO_APP_VOICE_CALL_WB ) && (currAudioMode != AUDIO_MODE_BLUETOOTH) )
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
		else
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
	}
	else
	{
		if (currAudioApp == AUDIO_APP_VOICE_CALL_WB)  //wideband 
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
		else
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
	}
#else
	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioMode).mic_pga);
	}

	if ( AUDDRV_GetVCflag() )
	{
		  // AUDIO_MODE_BLUETOOTH_WB should not be used for voice call. it should be convert to AUDIO_MODE_BLUETOOTH.
		if ( audio_mode >= AUDIO_MODE_NUMBER && (audio_mode != AUDIO_MODE_BLUETOOTH_WB) )
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
		else
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
	}
	else
	{
		if ( audio_mode >= AUDIO_MODE_NUMBER )  //wideband 
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
		else
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
	}
#endif
}

//=============================================================================
//
// Function Name: AUDDRV_SetMusicMode
//
// Description:   set audio mode.
//
//=============================================================================
#if defined(USE_NEW_AUDIO_PARAM)
void AUDDRV_SetMusicMode( AudioMode_t audio_mode, AudioApp_t audio_app)
#else
void AUDDRV_SetMusicMode( AudioMode_t audio_mode)
#endif
{

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() audio_mode==%d\n\r", audio_mode );

#if defined(USE_NEW_AUDIO_PARAM)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() audio_app==%d\n\r", audio_app );

	currAudioMode = (AudioMode_t)(audio_mode % AUDIO_MODE_NUMBER); // update mode
	currMusicAudioMode = currAudioMode;
	currAudioApp = audio_app;

	if (( audio_mode >= AUDIO_MODE_NUMBER ) && (audio_mode < AUDIO_MODE_NUMBER_VOICE ))
	{
		if (audio_app == AUDIO_APP_VOICE_CALL) currAudioApp = AUDIO_APP_VOICE_CALL_WB;
	}

#else

	currAudioMode = audio_mode;

	if ( audio_mode >= AUDIO_MODE_NUMBER )
		currMusicAudioMode = (AudioMode_t) (audio_mode - AUDIO_MODE_NUMBER);
#endif

	//should be part of audio mode API
	//or make SYSPARM_GetAudioParmAccessPtr to be the same on AP and CP:

	if ( controlFlagForCustomGain == FALSE )
	{
#if defined(USE_NEW_AUDIO_PARAM)
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).speaker_pga);
#else
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);
#endif
	}

	AUDDRV_SetEquType ( AUDDRV_TYPE_AUDIO_OUTPUT, sEqualizerType );
	
	//move to CP.  //to read and set this after the sys parm are readable at AP
	//AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );

	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQ,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQCOF[0][0] );
	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN,	(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHGAIN[0][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHOFST[0][0] );
#if defined(USE_NEW_AUDIO_PARAM)
	AUDDRV_SetFilter( AUDDRV_PIIR,			(const UInt16 *) & AUDIOMODE_PARM_MM_ACCESSOR(currAudioMode).PR_DAC_IIR[0] );

	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).MPMbiquad_cfg );

	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).AUDVOC_MIXER_IIR[0] );

	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).audvoc_mixergain );
	}

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

	if ( controlFlagForCustomGain == FALSE )
	{
		AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioApp,currAudioMode).mic_pga);
	}
#else
	AUDDRV_SetFilter( AUDDRV_PIIR,			(const UInt16 *) & AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).PR_DAC_IIR[0] );

	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioMode).MPMbiquad_cfg );

	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );

	if ( controlFlagForCustomGain == FALSE )
	{
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	}

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

	if ( controlFlagForCustomGain == FALSE )
	{
	AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioMode).mic_pga);
	}
#endif
}

//=============================================================================
//
// Function Name: AUDDRV_GetEquType
//
// Description:   Get Equalizer Type
//
//=============================================================================

AudioEqualizer_en_t AUDDRV_GetEquType( AUDDRV_TYPE_Enum_t   path )
{
#if ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_GetEquType (AP before read from CP) %d \n\r", sEqualizerType);
	sEqualizerType = (AudioEqualizer_en_t) audio_control_generic( AUDDRV_CPCMD_READ_AUDVOC_AEQMODE, (UInt32) path, 0, 0, 0, 0 );
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_GetEquType (AP after read from CP) %d \n\r", sEqualizerType);
#else
	UInt16 * ptr=0;
	  // PCG could chagned the equ type from PC. 
	  // Read equ type from sysinterface/dsp/audio/audioapi.c.
	ptr = (UInt16 *) AUDIO_GetAudioParam( PARAM_AUDVOC_AEQMODE );
	sEqualizerType = (AudioEqualizer_en_t) *ptr;
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_GetEquType (CP) %d \n\r", sEqualizerType);
#endif
	return sEqualizerType;
}


//=============================================================================
//
// Function Name: AUDDRV_SetEquType
//
// Description:   Set Equalizer Type
//
//=============================================================================

void AUDDRV_SetEquType( 
					AUDDRV_TYPE_Enum_t   path,
					AudioEqualizer_en_t	 equ_id
					)
{

#if ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
	sEqualizerType = equ_id;

	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_SetEquType (AP) %d \n\r", sEqualizerType);
	audio_control_generic( AUDDRV_CPCMD_WRITE_AUDVOC_AEQMODE, (UInt32) equ_id, 0, 0, 0, 0 );

#else

	SysCalDataInd_t* pSysparm;
	pSysparm = SYSPARM_GetAudioParmAccessPtr();

	sEqualizerType = equ_id;

	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_SetEquType (CP) %d \n\r", sEqualizerType);

	  // CP: update audvoc_aeqMode in sysinterface/dsp/audio/audioapi.c
	AUDIO_SetAudioParam( PARAM_AUDVOC_AEQMODE, (void *) & sEqualizerType );

	 //set these parameters
	AUDDRV_SetFilter( AUDDRV_AEQPATHGAIN, (const UInt16 *)& pSysparm->AUDVOC_AEQPATHGAIN[ sEqualizerType ][0] );
	AUDDRV_SetFilter( AUDDRV_AEQ, (const UInt16 *)& pSysparm->AUDVOC_AEQCOF[ sEqualizerType ][0] );

	AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN, (const UInt16 *)& pSysparm->AUDVOC_PEQPATHGAIN[ sEqualizerType ][0] );
	AUDDRV_SetFilter( AUDDRV_PEQ, (const UInt16 *)& pSysparm->AUDVOC_PEQCOF[ sEqualizerType ][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, (const UInt16 *)& pSysparm->AUDVOC_PEQCOF[ sEqualizerType ][0] );

	//to remove this after the sys parm are readable at AP

	AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );
	Log_DebugPrintf(LOGID_AUDIO, " AUDDRV_SetEquType (CP) FIR [0] %x, [32] %x, [33] %x \n\r", 
			SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0],
			SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[32],
			SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[33]
			);

#endif
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
	// By default the PCM port is occupied by trace port on development board
	if(on_off)
	{
		SYSCFGDRV_Config_Pin_Mux( SYSCFG_PCM_SPI2_GPIO_MUX_PCM_SEL );
		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);
	}
	else
	{
		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);
	}	
}


//=============================================================================
//
// Function Name: AUDDRV_User_CtrlDSP
//
// Description:   Control DSP Loudspeaker Protection
//
//=============================================================================

void AUDDRV_User_CtrlDSP ( AudioDrvUserParam_t audioDrvUserParam, void *user_CB, UInt32 param1, UInt32 param2 )
{
	Boolean spkProtEna = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_CtrlDSP *\n\r");
	if (user_CB != NULL)
		sUserCB = user_CB;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_CtrlDSP, UserCB = %x *\n\r", sUserCB);
	switch (audioDrvUserParam)
	{
		case AUDDRV_USER_GET_SPKPROT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_GET_SPKPROT *\n\r");
			
			audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, 3, (UInt16) param1, 0,0,0);
			break;
		case AUDDRV_USER_ENA_SPKPROT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_ENA_SPKPROT *\n\r");
			spkProtEna = (Boolean) param1;
			if (spkProtEna)
				
				audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, (UInt16) spkProtEna, (UInt16) param2, 1,0,0);
			
			else
				audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, (UInt16) spkProtEna, (UInt16) param2, 0,0,0);
			
			break;
		default:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_User_CtrlDSP: Invalid request %d \n\r", audioDrvUserParam);
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_User_HandleDSPInt
//
// Description:   Handle DSP Interrupt
//
//=============================================================================

void AUDDRV_User_HandleDSPInt ( UInt32 param1, UInt32 param2, UInt32 param3 )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_HandleDSPInt *\n\r");
	if(sUserCB)
		((AUDDRV_User_CB) sUserCB) (param1, param2, param3);
}
