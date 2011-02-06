/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   hal_audio.c
*
*   @brief  This file implements the audio control logic based on the application type.
*
****************************************************************************/
#include "hal_audio.h"
#include "hal_audio_core.h"
#include "hal_audio_config.h"
#include "hal_audio_access.h"
#include <linux/broadcom/bcm_fuse_sysparm.h>
#include <linux/timer.h>
#include <linux/sched.h>

#include "audio_drv.h"
#include "memmap.h"

#ifdef POLY_INCLUDED
#include "poly_ringer.h"
#endif
#ifdef VPU_INCLUDED
#include "vpu.h"
#endif

#if defined (SS_2153)
#define MAX9877_MAX_VOLUME 31
#endif // SS_2153

//#define _DBG_(a)		a
static char debug_buf[80];
#define _DBG_(a)
#define KI_DPRINTF AUD_TRACE_DETAIL
extern SysAudioParm_t* SYSPARM_GetAudioParmAccessPtr(UInt8 AudioMode);

extern UInt8 AUDIO_DRV_GetVoiceVolumeInDB( UInt8 Volume);
extern void program_equalizer(int equalizer_type);


typedef union {
	UInt16				micFlag;
	struct {
		UInt16	CALL_MIC_ON 		: 1;
		UInt16	VPU_MIC_ON			: 1;
		UInt16	AUDVOC_MIC_ON		: 1;
	} stMIC;	
} AUDIO_MIC_un_t;

//volume level defined in sysparm after phone power on
static UInt8 initSpkrVol;

#if defined (SS_2153)
static UInt16 currSpkVol[AUDIO_ID_ALL][AUDIO_CHNL_TOTAL];
#else // SS_2153
static UInt8 currSpkVol[AUDIO_ID_ALL][AUDIO_CHNL_TOTAL];
#endif // SS_2153

//static Timer_t	 toneTimer = 0;
//static TimerID_t toneTimerID;
//---------------------------------------------
static struct timer_list tone_timer;
static struct timer_list *toneTimer = NULL;

typedef	struct	_TMsgHalAudioCtrl
{
	HAL_AUDIO_ACTION_en_t			action_code;
	HAL_AUDIO_Control_Params_un_t		param;
	void								*pCallBack;

}TMsgHalAudioCtrl;

TMsgHalAudioCtrl MsgStopTone;

#define OSTIMER_Destroy(a) del_timer_sync(a)
//----------------------------------------------

static Boolean  BB_IsSpeakerOn = FALSE;
static Boolean  BB_IsMicOn = FALSE;
static Boolean  EXT_IsDevOn = FALSE;
static Boolean  bStatusMicMute = FALSE;
static AudioToneType_en_t eToneType = TONE_TYPE_DTMF; //default is DTMF tone

static UInt32 audioParam;
static HAL_AUDIO_Param_PathCfg_t	stCurrPath;
//static HAL_AUDIO_Param_CodecCfg_t   stCurrVPMCodecCfg;     //for VPM only
//static HAL_AUDIO_Param_PathCfg_t	pathCfg[AUDIO_ID_ALL];
//static UInt8	pathCfg_enable[AUDIO_ID_ALL];

static AUDIO_MIC_un_t  currMicStatus = {0};
static AudioMode_t currAudioMode;

static UInt32 hDev;
static UInt32 playedBufferCount;
Boolean halAudioFMPlaybackEnabled = FALSE;


static Boolean audioDevCtrl_IsInVoiceCall = FALSE;


/**************************************
* FUNCTION WRAPPERS OF some AUDIO API *
* only those AUDIO API functions that affects external device status are
* wrapped here to give a chance for external audio device function call.
*  These functions are used in Call-Control (MN layer) and MMI.
*
* This is part is for keeping backward compatibility when HAL AduioDevCtrl is in use.
***************************************/
void HAL_AUDIO_SetAudioMode(AudioMode_t audioMode);
/*** END OF FUNCTION WRAPPER OF some AUDIO API ***/

void HAL_AUDIO_SetSpeakerOn( AudioClientID_en_t audioID );
void HAL_AUDIO_SetSpeakerOff( AudioClientID_en_t audioID );
void HAL_AUDIO_SetMicrophoneOn( AudioClientID_en_t audioID );
void HAL_AUDIO_SetMicrophoneOff( AudioClientID_en_t audioID );

static void HAL_AUDIO_MuteSpeaker(void);
static void HAL_AUDIO_UnmuteSpeaker(void);
static void HAL_AUDIO_MuteMicrophone(void);
static void HAL_AUDIO_UnmuteMicrophone(void);
static void HAL_AUDIO_SetMicrophoneGain(UInt8 mic_gain_index);

#if defined (SS_2153)
static void HAL_AUDIO_SetSpeakerVolume(UInt16 spkVol);
#else // SS_2153
static void HAL_AUDIO_SetSpeakerVolume(UInt8 spkVol);
#endif // SS_2153

static Result_t HAL_AUDIO_PlayTone(HAL_AUDIO_Param_Tone_t *pToneParam);
void HAL_AUDIO_ProcessToneTimer(unsigned long ptr );

static Result_t HAL_AUDIO_PlayGenericTone(HAL_AUDIO_Param_GenericTone_t *pToneParam);

static void HAL_AUDIO_StopPlaytone(void);

//static void HAL_AUDIO_ProcessToneTimer(TimerID_t id );

static void HAL_AUDIO_EnablePath( HAL_AUDIO_Param_PathCfg_t * );
static void HAL_AUDIO_DisablePath( HAL_AUDIO_Param_PathCfg_t * );
static void HAL_AUDIO_SwitchPath( HAL_AUDIO_Param_PathCfg_t * );
//static void HAL_AUDIO_ConfigCodecParam( HAL_AUDIO_Param_CodecCfg_t * );
static void HAL_AUDIO_SetEqualizer( HAL_AUDIO_Param_Equalizer_t * );
static void HAL_AUDIO_SetEqualizerProfile( HAL_AUDIO_Param_EqualizerProfile_t *);

static UInt16	AUDIO_Util_Convert( UInt16 input, UInt16 scale_in, UInt16 scale_out);


UInt32	hDev_audvoc_PCM=0;
UInt32	hDev_audvoc_NBAMR=0;
UInt32	hDev_audvoc_MP3=0;  //rename it to handler_downloadable_codec?

//extern UInt32 audio_control_generic(UInt32 param1,UInt32 param2,UInt32 param3,UInt32 param4);
//extern UInt32 audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);
extern void program_coeff(int equlizer_type);


Result_t HAL_AUDIO_Core_Init( HAL_AUDIO_CONFIG_st_t *config )
{	
	int i=0;
	UInt16  volume_max = 0, mixer_gain;
	_DBG_(sprintf(debug_buf, "HAL_AUDIO_Core_Init enter()"));
	_DBG_(KI_DPRINTF( debug_buf ) );

	//reset all power flag
	stCurrPath.audioID = AUDIO_ID_CALL;
	stCurrPath.outputChnl= AUDIO_CHNL_HANDSET;
	stCurrPath.mixerInputPath = AUDVOC_VOICE;

//temporarily excludes zeus platform
#if defined(_BCM2153_) || defined(_BCM213x1_)
	AUDIO_DRV_InitReg();
#endif
	//load the default mixer gain (default equ is bypassed)
	mixer_gain = (UInt16)SYSPARM_GetAudioParmAccessPtr(stCurrPath.outputChnl)->audvoc_mixergain;
	AUDIO_DRV_SetAudioMixerGain((int)mixer_gain, (int)mixer_gain);

	//AUDIO_ASIC_Init();
	AUDIO_Turn_EC_NS_OnOff(FALSE, FALSE);

	currAudioMode = AUDIO_MODE_HANDSET;
	HAL_AUDIO_SetAudioMode(currAudioMode);
	program_coeff(EQU_NORMAL);
	//program_poly_coeff(EQU_NORMAL);

	//stCurrVPMCodecCfg.tMode = AUDIO_MODE_PLAYBACK;

	AudioExtDev_Init();
	memset(&currSpkVol[0][0], AUDIO_VOLUME_DEFAULT, sizeof(currSpkVol));

#if defined(FUSE_APPS_PROCESSOR)
	volume_max = SYSPARM_GetAudioParmAccessPtr(AUDIO_MODE_HANDSET)->voice_volume_max;
	initSpkrVol = SYSPARM_GetAudioParmAccessPtr(AUDIO_MODE_HANDSET)->voice_volume_init;
#endif

#ifndef USE_VOLUME_CONTROL_IN_DB
	// convert the value in 1st param from range of 2nd_param to range of 3rd_param:
	initSpkrVol = AUDIO_Util_Convert( initSpkrVol, volume_max, AUDIO_VOLUME_MAX );
//	printk("HAL_AUDIO_Core_Init:volume_max=%d initSpkrVol=%d AUDIO_VOLUME_MAX=%d \n", volume_max, initSpkrVol, AUDIO_VOLUME_MAX);
#endif

	for( i=0; i<AUDIO_MODE_NUMBER; i++)
	{
		currSpkVol[AUDIO_ID_TONE][i] = initSpkrVol;
		currSpkVol[AUDIO_ID_CALL][i] = initSpkrVol;
	}

	AUDIO_DRV_SetVoiceVolume( initSpkrVol );
	_DBG_(MSG_LOG("HAL_AUDIO_Core_Init() Exit() "));

#ifdef POLY_INCLUDED
	POLY_Run();
#endif

	// interal task and message queue
	//AudioMsgQ = OSQUEUE_Create( QUEUESIZE_HAL_AUDIO, sizeof( AudioMsg_t ), OSSUSPEND_PRIORITY );
	//AudioTask = OSTASK_Create( AudioTaskEntry, "Audio_Task_Internal", TASKPRI_HAL_AUDIO, STACKSIZE_HAL_AUDIO );


	return RESULT_OK;
}


// !! A very special usage of audio controller. Don't go through HAL API.


Result_t HAL_AudioProcess(HAL_AUDIO_ACTION_en_t action_code,HAL_AUDIO_Control_Params_un_t *arg_param,void *callback)
{
	Result_t result = RESULT_OK;
	HAL_AUDIO_ACTION_en_t actionCode;
	HAL_AUDIO_Control_Params_un_t *param;
	void* HAL_CallBack;


	// TODO:  Use semaphor for protection
	//OSSEMAPHORE_Obtain(HAL_AudioSema,TICKS_FOREVER);

	actionCode = action_code;
	param = (HAL_AUDIO_Control_Params_un_t *)arg_param;
	HAL_CallBack = callback;

	switch (actionCode) 
	{
		case ACTION_AUD_SetSpeakerVol:
			{
				UInt8 	spkVol = param->param_speakerVolume.volumeLevel;
				HAL_AUDIO_SetSpeakerVolume( spkVol );
			}
			break;

		case ACTION_AUD_SetMicGain:
			HAL_AUDIO_SetMicrophoneGain(param->param_micgain_index.micGainLevel);
			break;

		case ACTION_AUD_MuteSpeaker:
			HAL_AUDIO_MuteSpeaker();
			break;
			
		case ACTION_AUD_UnmuteSpeaker:
			HAL_AUDIO_UnmuteSpeaker();
			break;
		
		case ACTION_AUD_MuteMic:
			HAL_AUDIO_MuteMicrophone();
			break;
			
		case ACTION_AUD_UnmuteMic:
			HAL_AUDIO_UnmuteMicrophone();
			break;

		case ACTION_AUD_PlayTone:
			HAL_AUDIO_PlayTone(&(param->param_tone));
			break;
		
		case ACTION_AUD_PlayGenericTone:
			HAL_AUDIO_PlayGenericTone(&(param->param_genericTone));
			break;
		
		case ACTION_AUD_StopPlayTone:
			HAL_AUDIO_StopPlaytone();
			break;
		
		case ACTION_AUD_EnablePath:
			HAL_AUDIO_EnablePath( &(param->param_pathCfg) );
			break;
		
		case ACTION_AUD_DisablePath:
			HAL_AUDIO_DisablePath( &(param->param_pathCfg) );
			break;
		
		case ACTION_AUD_SetVolumeWithPath:
			HAL_AUDIO_SwitchPath( &(param->param_pathCfg) );
			break;

		case ACTION_AUD_ConfigCodecParam:
			//HAL_AUDIO_ConfigCodecParam( &(param->param_codecCfg) );
			break;

		case ACTION_AUD_SetEqualizer:
			HAL_AUDIO_SetEqualizer( &(param->param_equalizer) );
			break;

		case ACTION_AUD_SetEqualizerProfile:
			HAL_AUDIO_SetEqualizerProfile( &(param->param_equalizerProfile) );
			break;

		default:
			result = RESULT_ERROR;
			break;
	}
	// TODO:
	//OSSEMAPHORE_Release(HAL_AudioSema);

	return result;
}
	

/*  
	param	audioID		audio user ID
*/
void HAL_AUDIO_SetSpeakerOn( AudioClientID_en_t audioID )
{
	Boolean 	bOnFlag_EXT = FALSE;
	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerOn()  ID=%d, CH=%d, Speaker=%d, ExtDev=%d\r\n ", audioID, stCurrPath.outputChnl, BB_IsSpeakerOn, EXT_IsDevOn));

	// Fig out which channel is used for BB
	//need to make this configurable for project
	switch ( stCurrPath.outputChnl ) 
	{
		case	AUDIO_CHNL_HANDSET: 					// use voice handset
#if (defined(_BCM2133_) && CHIP_REVISION >= 12) || defined(_BCM2124_) || (CHIPVERSION >= CHIP_VERSION(BCM2152,10))
			//SPEAKER_SetAudioChnl(AUDIO_CHNL_INTERNAL);
#endif
			break;

		case	AUDIO_CHNL_HEADPHONE:						// use steroe headphone
		case	AUDIO_CHNL_TTY: 						// use steroe headphone
		case	AUDIO_CHNL_SPEAKER: 					// use mono loud speaker:
		case	AUDIO_CHNL_HAC:
#if (defined(_BCM2133_) && CHIP_REVISION >= 12) || defined(_BCM2124_) || (CHIPVERSION >= CHIP_VERSION(BCM2152,10))
			//SPEAKER_SetAudioChnl(AUDIO_CHNL_AUX);
#endif
			bOnFlag_EXT=TRUE;
			break;
		
		default:
			break;
	}
	
	switch( audioID )
	{	
		case AUDIO_ID_IIS:
			bOnFlag_EXT = TRUE;
			break;
			
		case AUDIO_ID_EXT_ANLG:
			bOnFlag_EXT = TRUE;
			break;

		case AUDIO_ID_TONE:
			AUDIO_DRV_SetDTMFPathOn(stCurrPath.outputChnl );
			break;

		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoicePathOn(currAudioMode);
			break;


		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathOn(stCurrPath.outputChnl );
			break;
				
		default:
			break;
	}


	BB_IsSpeakerOn = AUDIO_ON;

	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n",
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}



/*	Turn the speaker off
	param	audioID		audio user ID
*/
void HAL_AUDIO_SetSpeakerOff(AudioClientID_en_t audioID )
{			
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerOff(): audioID=%d\r\n", audioID));

	// reduce volume down to 0
	switch( audioID )
	{
		case AUDIO_ID_IIS:
			break;

		case AUDIO_ID_CALL:
			//AUDIO_DRV_SetVoiceVolume(0);
			//recover the volume level to init level. this is to make sure the amr-nb ringtone sounds 
			//the same level as first time. 
			AUDIO_DRV_SetVoiceVolume( initSpkrVol );
			break;

		case AUDIO_ID_TONE:
			AUDIO_DRV_SetToneVolume(eToneType, 0);
			break;

		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathGain(0, 0);
			break;

		default:
			break;
	}
	OSTASK_Sleep(10);// wait until take effect

	switch( audioID )
	{	
		case AUDIO_ID_IIS:
			break;
			
		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_TONE:
			AUDIO_DRV_SetDTMFPathOff(stCurrPath.outputChnl );
			break;

		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoicePathOff(currAudioMode);
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathOff(stCurrPath.outputChnl );
			break;
				
		default:
			break;
	}

	BB_IsSpeakerOn = AUDIO_OFF;
	if(EXT_IsDevOn)
	{	
		AudioExtDev_PowerOff( );
		EXT_IsDevOn =FALSE;
	}
	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerOff()  ID=%d, CH=%d, Speaker=%d, ExtDev=%d\r\n", audioID, stCurrPath.outputChnl, BB_IsSpeakerOn, EXT_IsDevOn));

	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}

/* Turn the Microphone on
	param	id		audio user ID
*/
void HAL_AUDIO_SetMicrophoneOn( AudioClientID_en_t audioID)
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetMicrophoneOn(): audioID=%d\r\n", audioID ));

	switch(audioID)
	{
		case AUDIO_ID_CALL:
			// voice uses 8k as default
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, TRUE);
			AUDIO_DRV_ConnectMicrophoneUplink(TRUE);			
			currMicStatus.stMIC.CALL_MIC_ON = TRUE;
			break;
			
		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_RECORD:
		case AUDIO_ID_VPU_TELEPHONY:
		case AUDIO_ID_VPU_VT_LOOPBACK:
			// voice uses 8k as default
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, TRUE);
			AUDIO_DRV_ConnectMicrophoneUplink(TRUE);
			currMicStatus.stMIC.VPU_MIC_ON = TRUE;
			break;

		case AUDIO_ID_AMRWB_RECORD: 
			// amr-wb uses 16k
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, TRUE);	
			AUDIO_DRV_SetMicrophonePath_16K(TRUE);
			currMicStatus.stMIC.AUDVOC_MIC_ON = TRUE;
			break;

		case AUDIO_ID_HQ_AUDIO_RECORD:
#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			// audio codec path, including DSP. Need to rework on this!!!
#endif //#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			break;
		
			
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
		case AUDIO_ID_FM_RECORD_PCM:
		case AUDIO_ID_FM_RECORD_AAC:
		case AUDIO_ID_TONE:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_PCM_READBACK:	
		default:
			// don't need microphone
			break;
	}


	AudioExtDev_SetBiasOnOff(currAudioMode, TRUE);
	BB_IsMicOn = AUDIO_ON;

	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}
	
/*	param	id		audio user ID */
void HAL_AUDIO_SetMicrophoneOff( AudioClientID_en_t audioID)
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetMicrophoneOff(): audioID=%d currMicStatus=%x\r\n", audioID, currMicStatus ));
	
	switch(audioID)
	{
		case AUDIO_ID_CALL:
			currMicStatus.stMIC.CALL_MIC_ON = FALSE;
			break;
		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_RECORD:
		case AUDIO_ID_VPU_TELEPHONY:
		case AUDIO_ID_VPU_VT_LOOPBACK:
			currMicStatus.stMIC.VPU_MIC_ON = FALSE;
			break;

		case AUDIO_ID_AMRWB_RECORD: 
			currMicStatus.stMIC.AUDVOC_MIC_ON = FALSE;
			break;

		case AUDIO_ID_HQ_AUDIO_RECORD:
#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			// audio codec path, including DSP. Need to rework on this!!!
#endif //#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			break;
		
			
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
		case AUDIO_ID_FM_RECORD_PCM:
		case AUDIO_ID_FM_RECORD_AAC:
		case AUDIO_ID_TONE:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_PCM_READBACK:	
		default:
			// don't need microphone
			break;
	}

	if (currMicStatus.micFlag == TRUE)
		return;

	if (audioID == AUDIO_ID_AMRWB_RECORD)
		AUDIO_DRV_SetMicrophonePath_16K(FALSE);
	else
		AUDIO_DRV_ConnectMicrophoneUplink(FALSE);
	
	AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, FALSE);
	AudioExtDev_SetBiasOnOff(currAudioMode, FALSE);
	bStatusMicMute = FALSE;
		
	BB_IsMicOn = AUDIO_OFF;

	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}


static void HAL_AUDIO_MuteSpeaker()
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_MuteSpeaker()\r\n"));

	switch( stCurrPath.audioID)
	{
		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoiceVolume( 0 );  //better use a separate mute function.!!
			//AUDIO_DRV_MuteAMRPathOnOff(TRUE);
			break;

		case AUDIO_ID_TONE:
			//AUDIO_DRV_MuteDTMFPathOnOff(TRUE);
			break;

		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_MuteAudioPathOnOff(TRUE);
			break;
			
		default:
			break;
	}
		
	if(EXT_IsDevOn)
		AudioExtDev_SetVolume(0);  //better use a separate mute function.!!
}

static void HAL_AUDIO_UnmuteSpeaker()
{
#if defined (SS_2153)
	UInt16 index = currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];
#else // SS_2153
	UInt8 index = currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];
#endif // SS_2153

	_DBG_(KI_DPRINTF("HAL_AUDIO_UnmuteSpeaker()\r\n"));
	
	switch( stCurrPath.audioID)
	{
		case AUDIO_ID_CALL:
			index = currSpkVol[AUDIO_ID_CALL][stCurrPath.outputChnl];
			AUDIO_DRV_SetVoiceVolume( (UInt8)index );
			//AUDIO_DRV_MuteAMRPathOnOff(FALSE);
			break;

		case AUDIO_ID_TONE:
			//AUDIO_DRV_MuteDTMFPathOnOff(FALSE);
			break;

		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_MuteAudioPathOnOff(FALSE);
			break;
			
		default:
			break;
	}
	
	//volume level is per customer's request
	if(EXT_IsDevOn)
#if defined (SS_2153)
					AudioExtDev_SetVolume(AudioGain_ExtDev(index, MAX9877_MAX_VOLUME));
#else // SS_2153
					AudioExtDev_SetVolume(index);
#endif // SS_2153
}

static void HAL_AUDIO_MuteMicrophone()
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_MuteMicrophone()\r\n"));

	AUDIO_DRV_MicrophoneMuteOnOff(TRUE);
	bStatusMicMute = TRUE;
}

static void HAL_AUDIO_UnmuteMicrophone()
{	
	_DBG_(KI_DPRINTF("HAL_AUDIO_UnmuteMicrophone()\r\n"));

	AUDIO_DRV_MicrophoneMuteOnOff(FALSE);
	bStatusMicMute = FALSE;
}

static void HAL_AUDIO_SetMicrophoneGain(UInt8 mic_gain_index)
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetMicrophoneGain()\r\n"));

	AUDIO_DRV_SetMICGain(mic_gain_index);
}

#define MASK_SLOPGAIN_VALUE		0x1FF

#if defined (SS_2153)
static void HAL_AUDIO_SetSpeakerVolume(UInt16 speakerVol)
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerVolume(): audioID=%d, Ch=%d, Vol=%d\r\n", \
		stCurrPath.audioID, stCurrPath.outputChnl, speakerVol));
	
	if (speakerVol == currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl])
	{
		// skip the volume setting at attempt to set the same volume as the current one.
		_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerVolume(): identical to the current volume\r\n"));
		return;
	}

	if (speakerVol > AUDIO_VOLUME_MAX) // 11162008 michael (discard the out-of-range value and set default)
		speakerVol = currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];

	switch(stCurrPath.audioID)
	{
		case AUDIO_ID_IIS:			
			break;

		case AUDIO_ID_CALL:
			//for coverity, make sure the speakerVol is in the range for voice call
			speakerVol = (speakerVol > 40) ? 40 : speakerVol;
			AUDIO_DRV_SetVoiceVolume( (UInt8)speakerVol );
			break;

		case AUDIO_ID_TONE:
			//modified the tone volume here to make it sound close as before
			AUDIO_DRV_SetToneVolume(eToneType, speakerVol/3);
			break;

		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathGain(speakerVol & MASK_SLOPGAIN_VALUE, speakerVol & MASK_SLOPGAIN_VALUE);
			break;

		default:
			break;
	}

	AudioExtDev_SetVolume(AudioGain_ExtDev(speakerVol, MAX9877_MAX_VOLUME));

	currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl]=speakerVol;
}
#else // SS_2153
static void HAL_AUDIO_SetSpeakerVolume(UInt8 speakerVol)
{
	UInt16 uDigVol;
	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerVolume(): audioID=%d, Ch=%d, Vol=%d\r\n", \
		stCurrPath.audioID, stCurrPath.outputChnl, speakerVol));

	switch(stCurrPath.audioID)
	{
		case AUDIO_ID_IIS:			
			break;

		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoiceVolume( speakerVol );
			break;

		case AUDIO_ID_TONE:
			//modified the tone volume here to make it sound close as before
			//AUDIO_DRV_SetToneVolume(eToneType, speakerVol/3);
			break;

		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			if ( speakerVol==0)
				uDigVol = 0x00;  //digital mute.
			else
#ifndef	BCM2153_FAMILY_BB_CHIP_BOND
				uDigVol = ((((UInt16)SYSPARM_GetAudioParmAccessPtr(stCurrPath.outputChnl)->audvoc_aslopgain) & MASK_SLOPGAIN_VALUE)- 40*4) + (speakerVol * 4);
#else //BCM2153_FAMILY_BB_CHIP_BOND
				uDigVol = 0x1FF;
#endif //BCM2153_FAMILY_BB_CHIP_BOND
			AUDIO_DRV_SetAudioPathGain(uDigVol, uDigVol);
			break;

		default:
			break;
	}

	//AudioExtDev_SetVolume(speakerVol);

	currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl]=speakerVol;
}
#endif // SS_2153


/*	Play DTMF tone or supervisory tone
	param	pToneParam	structure holding tone params
*/
static Result_t HAL_AUDIO_PlayTone(HAL_AUDIO_Param_Tone_t *pToneParam)
{
	Result_t res = RESULT_OK;

	/* update current path info */
	stCurrPath.audioID = AUDIO_ID_TONE;
	stCurrPath.volumeLevel = pToneParam->volumeLevel;
	stCurrPath.outputChnl = pToneParam->outputChnl;
	eToneType = TONE_TYPE_DTMF;
	
	_DBG_(sprintf(debug_buf, "HAL_AUDIO_PlayTone(), Ch= %d\r\n", stCurrPath.outputChnl  ));
	//_DBG_(KI_DPRINTF( debug_buf ) );
	
	if(toneTimer)
	{
		_DBG_(KI_DPRINTF( "HAL_AUDIO_PlayTone(): Stop timer if it exist to replace the new one"));
		//OSTIMER_Stop(toneTimer);	
		OSTIMER_Destroy(toneTimer);
		//toneTimer = NULL;
	}

	//toneTimer = OSTIMER_Create( HAL_AUDIO_ProcessToneTimer, toneTimerID, pToneParam->duration, 0);
	toneTimer = &tone_timer;
	init_timer(toneTimer);
    toneTimer->function = HAL_AUDIO_ProcessToneTimer;
	toneTimer->data = NULL;
	toneTimer->expires = jiffies + msecs_to_jiffies(pToneParam->duration);
	//printk(KERN_ALERT " Tone: jiffies = %d toneTimer->expires = %d\n",jiffies,toneTimer->expires);
	HAL_AUDIO_SetSpeakerOn(AUDIO_ID_TONE);
	
	HAL_AUDIO_SetSpeakerVolume(pToneParam->volumeLevel);
	
	// start the tone, duration is forever (until it is being stopped)
	// DSP treats the 0 as the continuous DTMF duration, but API should
	// not since we use the duration to stop the tone.
	res = AUDIO_DRV_PlayTone( pToneParam->tone, 0);
	
	//OSTIMER_Start(toneTimer);
	add_timer(toneTimer);
	
	return res;
}




static Result_t HAL_AUDIO_PlayGenericTone(HAL_AUDIO_Param_GenericTone_t *pToneParam)
{
	Result_t res = RESULT_OK;

	/* update current path info */
	stCurrPath.audioID = AUDIO_ID_TONE;
	stCurrPath.volumeLevel = pToneParam->volumeLevel;
	stCurrPath.outputChnl = pToneParam->outputChnl;
	eToneType = TONE_TYPE_GENERIC;
	
	_DBG_(sprintf(debug_buf, "HAL_AUDIO_PlayGenericTone(), Ch = %d\r\n", stCurrPath.outputChnl ));
	_DBG_(KI_DPRINTF( debug_buf ) );

	if(toneTimer)
	{
		_DBG_(KI_DPRINTF( "HAL_AUDIO_PlayGenericTone(): Stop timer if it exist to replace the new one"));
		//OSTIMER_Stop(toneTimer);	
		//if timer already exists destroy it
		OSTIMER_Destroy(toneTimer);
		//toneTimer = &NULL;
	}
	//toneTimer = OSTIMER_Create( HAL_AUDIO_ProcessToneTimer, toneTimerID, pToneParam->duration, 0);
	toneTimer = &tone_timer;
	init_timer(toneTimer);
    toneTimer->function = HAL_AUDIO_ProcessToneTimer;
	toneTimer->data = NULL;
	toneTimer->expires = jiffies + msecs_to_jiffies(pToneParam->duration);
	
	HAL_AUDIO_SetSpeakerOn(AUDIO_ID_TONE);

	HAL_AUDIO_SetSpeakerVolume(pToneParam->volumeLevel);

	res = AUDIO_DRV_PlayGenericTone(pToneParam->superimpose, pToneParam->duration, pToneParam->f1, pToneParam->f2, pToneParam->f3);

	//start the timer
	add_timer(toneTimer);

	return res;
}


// Tone timer interrupt callback.
// Since timer is HISR, this function can't block at semaphore.
//Arg ptr has no meaning,its here just to satisy the linux Timer callback prototype
void HAL_AUDIO_ProcessToneTimer(unsigned long ptr )
{
	
	//Add the Tone stop msg to the HAL_ctrl queue 
	HAL_AUDIO_Ctrl(ACTION_AUD_StopPlayTone,NULL,NULL);
	


}

/*	Stop speaker(DTMF) tone.
	This function is usually called by the timer
*/
static void HAL_AUDIO_StopPlaytone(void)
{

	_DBG_(KI_DPRINTF("HAL_AUDIO_StopPlaytone, Ch= %d.\r\n", stCurrPath.outputChnl));

	if(toneTimer)
	{
		_DBG_(KI_DPRINTF( "HAL_AUDIO_StopPlaytone(): Stop timer"));		
		OSTIMER_Destroy(toneTimer);	
		toneTimer = NULL;
		AUDIO_DRV_StopPlayTone();

		//HAL_AUDIO_SetSpeakerOff(AUDIO_ID_TONE);
		
	}
	
}

static void HAL_AUDIO_EnablePath( HAL_AUDIO_Param_PathCfg_t *pathCfgPtr )
{
	Boolean bCur_StatusMicMute;

#if defined (SS_2153)
	UInt16 speakerVolume;
	UInt16 mixer_gain;
	mixer_gain = (UInt16)SYSPARM_GetAudioParmAccessPtr(stCurrPath.outputChnl)->audvoc_mixergain;
#else // SS_2153
	UInt8 speakerVolume;
#endif // SS_2153
	

	//stop started tone due to channel change.
	//HAL_AUDIO_StopPlaytone();
		 
	// update current path setting
	stCurrPath.audioID= pathCfgPtr->audioID;
	stCurrPath.outputChnl= pathCfgPtr->outputChnl;
	stCurrPath.mixerInputPath = pathCfgPtr->mixerInputPath;

	if ( pathCfgPtr->volumeLevel > AUDIO_VOLUME_MAX )
		speakerVolume= currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];
	else
		speakerVolume= pathCfgPtr->volumeLevel;

	_DBG_(KI_DPRINTF("HAL_AUDIO_EnablePath()  ID=%d, SpkCh=%d, Vol=%d", stCurrPath.audioID, stCurrPath.outputChnl, speakerVolume));

	//make sure dsp audio was off before d2c.
//	if(stCurrPath.audioID==AUDIO_ID_CALL)
//		audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
	AUDIO_DRV_PowerUpD2C();
	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n",
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
//	AUDIO_DRV_SetHifiDacMask();
//	AUDIO_DRV_PowerUpRightSpk();
//	AUDIO_DRV_PowerUpLeftSpk();

	switch(stCurrPath.audioID)
	{
		case AUDIO_ID_TONE:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			break;

		case AUDIO_ID_CALL:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153

			bCur_StatusMicMute = bStatusMicMute;

			HAL_AUDIO_MuteMicrophone();
			AUDIO_Turn_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);

			
			HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);
			OSTASK_Sleep( 40 );

			AUDIO_Turn_EC_NS_OnOff(TRUE, TRUE);
			audio_control_dsp(TURN_UL_COMPANDER_ON, 0, 0, 0);

			if (!bCur_StatusMicMute)
				HAL_AUDIO_UnmuteMicrophone();

#ifdef USB_AUDIO_ENABLED
			if (pathCfgPtr->outputChnl==AUDIO_CHNL_USB) {
			  HAL_AUDIO_EnableExtDev_Usb_swFifo(&gAudioHalAccess.ExtDev_Usb); 
			  HAL_AUDIO_EnableExtDev_Usb(&gAudioHalAccess.ExtDev_Usb); 
			  HAL_AUDIO_EnableVirtualChannel_Usb(&gAudioHalAccess.VC_Usb); 
			}
#endif
			break;

		case AUDIO_ID_VPU_RECORD:
			//HAL_AUDIO_SetSpeakerOff(stCurrPath.audioID);	// don't need SPEAKER
			HAL_AUDIO_SetAudioMode( (AudioMode_t)(pathCfgPtr->outputChnl) );
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);

			bCur_StatusMicMute = bStatusMicMute;
			if (!bCur_StatusMicMute)
				HAL_AUDIO_UnmuteMicrophone();
			break;
			
		case AUDIO_ID_VPU_TELEPHONY:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153

			HAL_AUDIO_MuteMicrophone();
			AUDIO_Turn_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);

			
			HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);

			OSTASK_Sleep( 40 );

			AUDIO_Turn_EC_NS_OnOff(TRUE, TRUE);
			audio_control_dsp(TURN_UL_COMPANDER_ON, 0, 0, 0);
			HAL_AUDIO_UnmuteMicrophone();

			break;

		case AUDIO_ID_HQ_AUDIO_RECORD:
#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			// audio codec path, including DSP
			HAL_Audio_EnablePath_ADC_HQ(&gAudioHalAccess.AdcHqAudioPath, pathCfgPtr->audioID, pathCfgPtr->channels, pathCfgPtr->inputMIC);
#endif //#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			break;

		case AUDIO_ID_POLY: //midi playback
		case AUDIO_ID_WAVETONE_POLY: //wavetone poly playback
		case AUDIO_ID_AUDVOC: //music playback
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153
			audio_control_generic(UPDATE_AUDIO_MODE, (UInt32)pathCfgPtr->outputChnl, 0, 0);
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			break;

		default:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153
			audio_control_generic(UPDATE_AUDIO_MODE, (UInt32)pathCfgPtr->outputChnl, 0, 0);			
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			break;
	}

}

static void HAL_AUDIO_DisablePath( HAL_AUDIO_Param_PathCfg_t *pathCfgPtr )
{
	// update current path setting
	_DBG_(KI_DPRINTF("HAL_AUDIO_DisablePath()  ID=%d, SpkCh=%d\r\n", pathCfgPtr->audioID, pathCfgPtr->outputChnl));

	//stop started tone due to channel change.
	//HAL_AUDIO_StopPlaytone();
//	AUDIO_DRV_SetHifiDacMask();
//	AUDIO_DRV_PowerDownRightSpk();
//	AUDIO_DRV_PowerDownLeftSpk();

	switch(pathCfgPtr->audioID)
	{


		case AUDIO_ID_CALL:
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );

			AUDIO_Turn_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);

			HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
			HAL_AUDIO_SetMicrophoneOff(pathCfgPtr->audioID);
#ifdef USB_AUDIO_ENABLED
			if (pathCfgPtr->outputChnl==AUDIO_CHNL_USB) {
			  HAL_AUDIO_DisableExtDev_Usb(&gAudioHalAccess.ExtDev_Usb); 
			  HAL_AUDIO_DisableVirtualChannel_Usb(&gAudioHalAccess.VC_Usb); 
			  HAL_AUDIO_DisableExtDev_Usb_swFifo(&gAudioHalAccess.ExtDev_Usb); 
			}
#endif

			break;

		case AUDIO_ID_VPU_RECORD:
			HAL_AUDIO_SetMicrophoneOff(pathCfgPtr->audioID);
			break;
			
		case AUDIO_ID_VPU_TELEPHONY:
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			AUDIO_Turn_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
			HAL_AUDIO_SetMicrophoneOff(pathCfgPtr->audioID);
			break;


		case AUDIO_ID_AUDVOC:
				HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
				break;



		default:
			HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
			break;
	}
//	AUDIO_DRV_FlushFifo();

	OSTASK_Sleep(1);
	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}

#if 1
static void HAL_AUDIO_SwitchPath( HAL_AUDIO_Param_PathCfg_t *pathCfgPtr )
{
	//KI_DLOGV4("HAL_AUDIO_SwitchPath()  ID, SpkCh, Vol", stCurrPath.audioID, stCurrPath.outputChnl, stCurrPath.volumeLevel, 0);
	_DBG_(KI_DPRINTF("HAL_AUDIO_SwitchPath()  ID=%d, SpkCh=%d\r\n", pathCfgPtr->audioID, pathCfgPtr->outputChnl));

	// an ugly thing for FM radio play. We should discard this function and its API
	if (pathCfgPtr->audioID == AUDIO_ID_IIS2DSP)
	{
		HAL_AUDIO_SetSpeakerVolume(pathCfgPtr->volumeLevel);
	}
	else	
	{	
		// 2 actions happen in switch path and need to adress them separately for upper layer
		if ((stCurrPath.audioID != pathCfgPtr->audioID) || (stCurrPath.outputChnl != pathCfgPtr->outputChnl))
			HAL_AUDIO_EnablePath(pathCfgPtr);
		else
			HAL_AUDIO_SetSpeakerVolume(pathCfgPtr->volumeLevel);
	}
}

#if 0
//VPU driver should not be part of audvoc. 2152 has it.
static void HAL_AUDIO_ConfigCodecParam(HAL_AUDIO_Param_CodecCfg_t *codecCfgPtr)
{
	Int16 equ_profile;
	AudioClientID_en_t  		halAudioID = codecCfgPtr->audioID;

#if defined(_BCM2153_) || defined(_BCM213x1_) || defined (_BCM21551_)
	AUDVOC_CFG_AMR_NB_INFO amr_cfg;
#endif
	_DBG_(KI_DPRINTF("HAL_AUDIO_ConfigCodecParam, audioID= %d\r\n",halAudioID));

	switch(halAudioID)
	{
		case AUDIO_ID_IIS: // output via I2S		
#ifdef I2S_INCLUDED
			if(codecCfgPtr->param1!=0)
			{
				I2S_SetBufSize(codecCfgPtr->param1);
			}

			if(codecCfgPtr->param2!=0)
			  i2s_op_mode = TRUE;
			else
			  i2s_op_mode = FALSE;
			
			I2S_SetupPlay(codecCfgPtr->sampleRate, 
						codecCfgPtr->monoStereo, 
						codecCfgPtr->flag, 
						codecCfgPtr->bitPerSample);
#endif
		break;
	
		case AUDIO_ID_POLY:		
#ifdef POLY_INCLUDED
			POLY_SetParams((PLR_Set_Msg_t *) codecCfgPtr->param1);
#endif
			break;

	case AUDIO_ID_VPU:
	case AUDIO_ID_VPU_VT_LOOPBACK:
			stCurrVPMCodecCfg.audioID = codecCfgPtr->audioID;
			stCurrVPMCodecCfg.tMode   = codecCfgPtr->tMode;
			stCurrVPMCodecCfg.tLink   = codecCfgPtr->tLink;
			stCurrVPMCodecCfg.tFormat = codecCfgPtr->tFormat;
			stCurrVPMCodecCfg.param1  = codecCfgPtr->param1;      //used for AMR bitrate
			stCurrVPMCodecCfg.bufferFillCB  = codecCfgPtr->bufferFillCB;    
			stCurrVPMCodecCfg.bufferDumpCB  = codecCfgPtr->bufferDumpCB;    
			_DBG_(KI_DPRINTF( "HAL_AUDIO_ConfigCodecParam - VPU, b \r\n"));
			break;

		case AUDIO_ID_IIS2DAC:
		  //			HAL_AUDIO_EnableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			equ_profile = (Int16)audio_control_generic(READ_AUDVOC_AEQMODE, 0, 0, 0);
			program_coeff(equ_profile);
			AUDIO_DRV_SetAudioSampleRate(codecCfgPtr->sampleRate);
			break;

		case AUDIO_ID_IIS2DSP:
			equ_profile = (Int16)audio_control_generic(READ_AUDVOC_AEQMODE, 0, 0, 0);
			program_coeff(equ_profile);
			AUDIO_DRV_SetAudioSampleRate(codecCfgPtr->sampleRate);
			break;

		case AUDIO_ID_AUDVOC:
			_DBG_(KI_DPRINTF("HAL_AUDIO_ConfigCodecParam, AUDVOC monoStereo=%d\r\n",codecCfgPtr->monoStereo));

			if(codecCfgPtr->monoStereo == TYPE_MONO) // mono setting
				AUDIO_DRV_SetSTAudioPath(FALSE);
			else // stereo setting
				AUDIO_DRV_SetSTAudioPath(TRUE);
			
            // Since there is a timing dependency between configCodec and enablePath, configCodec should be called
            // before enablePath to ensure channel stero/mono is set correctly in ASIC. But in some cases, 
            // enablePath is called before configureCodec. Therefore, we set the channel mode in ASIC right here.
		    AUDIO_DRV_SetAudioChannel(stCurrPath.outputChnl); // 01062009 michael
			break;

		case AUDIO_ID_WAVETONE_POLY: //from audvoc2
			_DBG_(KI_DPRINTF("HAL_AUDIO_ConfigCodecParam, monoStereo=%d\r\n",codecCfgPtr->monoStereo));
			
			if(codecCfgPtr->monoStereo == TYPE_MONO) // mono setting
				AUDIO_DRV_SetSTPolyPath(FALSE);
			else // stereo setting
				AUDIO_DRV_SetSTPolyPath(TRUE);
			
	        // Since there is a timing dependency between configCodec and enablePath, configCodec should be called
	        // before enablePath to ensure channel stero/mono is set correctly in ASIC. But in some cases, 
	        // enablePath is called before configureCodec. Therefore, we set the channel mode in ASIC right here.
		    AUDIO_DRV_SetPolyChannel(stCurrPath.outputChnl);
			break;
			
		default: 
			break;
	}
}
#endif
#endif

static void HAL_AUDIO_SetEqualizer( HAL_AUDIO_Param_Equalizer_t *equ )
{
#ifdef equ_mixer_test
	UInt8 i, equ_on = 0;
	UInt16 mixer_gain;
#endif	

	_DBG_(KI_DPRINTF("HAL_AUDIO_SetEqualizer(): pg1=%x pg2=%x pg3=%x pg4=%x pg5=%x\r\n",
		equ->gain[0],equ->gain[1],equ->gain[2],equ->gain[3],equ->gain[4]));

#ifdef equ_mixer_test
	for ( i = 0; i <  NUM_OF_EQU_BANDS; i++)
	{
		if (equ->gain[i] != 0)
		{	
			equ_on = 1;
			break;
		}
	}

	//need to update mixer gain based on equ status
	mixer_gain = (UInt16)SYSPARM_GetAudioParmAccessPtr(stCurrPath.outputChnl)->audvoc_mixergain;
	if (equ_on)
	{
		//adjust mixer gain -18 db to compensate equ gain if equ_on
		mixer_gain >>= 3;	
	}
	AUDIO_DRV_SetAudioMixerGain((int)mixer_gain, (int)mixer_gain);
#endif

	if (equ->mixerInputPath == AUDVOC_PATH_AUDIO)
	{
		AUDIO_DRV_SetAudioEquGain(equ->gain);
	}
}

static void HAL_AUDIO_SetEqualizerProfile( HAL_AUDIO_Param_EqualizerProfile_t *eq_profile )
{
	//TRACE_Printf_Sio("HAL_AUDIO_SetEqualizerProfile() profile=%d\r\n", eq_profile->equalizerID);

	switch( eq_profile->mixerInputPath )
	{
		case AUDVOC_PATH_AUDIO:
			program_equalizer( eq_profile->equalizerID );
		break;
	}
	audio_control_generic(WRITE_AUDVOC_AEQMODE, (UInt32)eq_profile->equalizerID, 0, 0);
}


/*	Set audio mode for EC/NS
	This function is usually called in audio tuning,
   otherwise called by AudioDevCtrl_SetVolume_forVPM_SetVoiceVolume().
*/
//this function only sets ASIC.
void HAL_AUDIO_SetAudioMode(AudioMode_t audioMode)
{
	_DBG_(sprintf(debug_buf, "HAL_AUDIO_SetAudioMode: new mode = %d, old mode %d\r\n", audioMode, currAudioMode));
	_DBG_(KI_DPRINTF( debug_buf ) );

	if(currAudioMode == audioMode)
		return;

	AUDIO_DRV_SetVoiceMode( audioMode );

	currAudioMode = audioMode; // update mode
}

//******************************************************************************
//
// Function Name: HAL_AUDIO_GetParam
//
// Description: Allocate an parameter from current AUDIO operaitons.
//
// Notes:
//
//******************************************************************************
Result_t HAL_AUDIO_GetParam(HAL_AUDIO_Get_Param_t *getParamPtr)
{
    _DBG_(KI_DPRINTF( "HAL_AUDIO_GetParam() Enter"));
    switch ( getParamPtr->paramType )
    {
    case GET_AUDIO_ID:
        audioParam = stCurrPath.audioID;
        break;

    case GET_AUDIO_CHNL:
        audioParam = stCurrPath.outputChnl;
        break;

    case GET_VOLUME_MIN:
        audioParam = AUDIO_VOLUME_MIN;
        break;

    case GET_VOLUME_MAX:
        audioParam = AUDIO_VOLUME_MAX;
        break;

    case GET_VOLUME_DEFAULT:
        audioParam = AUDIO_VOLUME_DEFAULT;
        break; 

    case GET_AUDIO_VOLUMELEVEL: //this will return level in db unit
        audioParam = AUDIO_DRV_GetVoiceVolumeInDB(currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl]);
        break;

    case GET_MICGAIN_MIN:
        audioParam = AUDIO_MICGAIN_MIN;
        break;

    case GET_MICGAIN_MAX:
        audioParam = AUDIO_MICGAIN_MAX;
        break;

    case GET_MICGAIN_DEFAULT:
        audioParam = AUDIO_MICGAIN_DEFAULT;
        break;

    case GET_MICGAIN_INDEX:
        // audioParam = AUDIO_DRV_GetMICGainIndex();
        break;

    case GET_AUDIO_MODE:
        audioParam = currAudioMode;
        break;

    default:
        break;
    }

    switch ( getParamPtr->audioID )
    {
#ifndef FEATURE_AUDIO_VOICECALL16K
    case AUDIO_ID_CALL:
        switch (getParamPtr->paramType)
        {
        case GET_MUTE_STATUS:
            audioParam = bStatusMicMute;
            break;

        default:
            break;
        }
        break;
#else // FEATURE_AUDIO_VOICECALL16K
    case AUDIO_ID_CALL8K:
    case AUDIO_ID_CALL16K:
        switch (getParamPtr->paramType)
        {
        case GET_MUTE_STATUS:
            audioParam = bStatusMicMute;
            break;

        default:
            break;
        }
        break;
#endif // FEATURE_AUDIO_VOICECALL16K

    case AUDIO_ID_IIS:
#ifdef I2S_INCLUDED
        switch (getParamPtr->paramType)
        {
        case GET_PLAYBACK_POSITION:
            audioParam = I2S_GetPosition();
            break;

        default:
            break;
        }
#endif
        break;

    case AUDIO_ID_POLY:
#ifdef POLY_INCLUDED
        switch (getParamPtr->paramType)
        {
        case GET_PLAYBACK_POSITION:
            audioParam = PolyRinger_Parser_GetPosition(POLY_GET_POSITION);
            break;

        case GET_PLAYBACK_DURATION:
            audioParam = PolyRinger_Parser_GetPosition(POLY_GET_DURATION);
            break;

        case GET_GENERIC_PARAM1:
            audioParam = POLY_GetParams((PLR_Get_Msg_t *) getParamPtr->paramPtr);
            break;

        default:
            break;
        }
#endif
        break;

    default:
        break;
    }

    getParamPtr->paramPtr = &audioParam;

    pr_info("   HAL_AUDIO_GetParam--    \r\n");
    _DBG_(sprintf(debug_buf, "HAL_AUDIO_GetParam, Param= %d, Ptr = 0x%x\r\n", audioParam, getParamPtr->paramPtr));
    _DBG_(KI_DPRINTF( debug_buf ) );

    return RESULT_OK;
}





// convert the number from range scale_in to range scale_out.
static UInt16	AUDIO_Util_Convert( UInt16 input, UInt16 scale_in, UInt16 scale_out)
{
	UInt16 output=0;
	//float temp=0;

	output = ( input * scale_out ) / scale_in;
	//output = (UInt16)( temp + 0.5);
	return output;
}

//!
//! HAL_Audio_Ctrl API is a blocking call API. If you need to use it in a non-blocking fashion,
//! please use HAL_Audio_Ctrl_Handler
//!

//!
//! Function Name: HAL_AUDIO_Init
//!
//! Description:  init HAL audio.
//!
//! \param  void
//!
//! \return  void
//!
//! \note  
//!
//! \sa  HAL_AUDIO_Init
//!
//!
Result_t HAL_AUDIO_Init( HAL_AUDIO_CONFIG_st_t *config )
{
	_DBG_(sprintf(debug_buf, "HAL_AUDIO_Init enter()"));
	_DBG_(KI_DPRINTF( debug_buf ) );

	return HAL_AUDIO_Core_Init( config );
}
#ifndef	USE_HAL_AUDIO_THREAD
Result_t HAL_AUDIO_Ctrl(
	HAL_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	)
{
	Result_t status = RESULT_OK;

	{
		HAL_AUDIO_Control_Params_un_t *paudioControlParam = (HAL_AUDIO_Control_Params_un_t *)arg_param;
                if(paudioControlParam)
            pr_info("HAL_AUDIO_Ctrl action=%d, id=%d, channel=%d \r\n", action_code, paudioControlParam->param_pathCfg.audioID, paudioControlParam->param_pathCfg.outputChnl);

	}

    if (ACTION_AUD_GetParam == action_code)
        return HAL_AUDIO_GetParam((HAL_AUDIO_Get_Param_t *)arg_param);

	// !! Don't use message queue and task. All ctrl API will return after complete
	// all the actions. We can add more return error code for a better notice to upper layer.
	status = HAL_AudioProcess(action_code, (HAL_AUDIO_Control_Params_un_t *)arg_param, callback);
	
	return status;
}

#else
Result_t HAL_AUDIO_Ctrl_Blocking(
	HAL_AUDIO_ACTION_en_t action_code,
	void *arg_param,
	void *callback
	)
{
	Result_t status = RESULT_OK;

	{
		HAL_AUDIO_Control_Params_un_t *paudioControlParam = (HAL_AUDIO_Control_Params_un_t *)arg_param;
                if(paudioControlParam)
            pr_info("HAL_AUDIO_Ctrl action=%d, id=%d, channel=%d \r\n", action_code, paudioControlParam->param_pathCfg.audioID, paudioControlParam->param_pathCfg.outputChnl);

	}

    if (ACTION_AUD_GetParam == action_code)
        return HAL_AUDIO_GetParam((HAL_AUDIO_Get_Param_t *)arg_param);

	// !! Don't use message queue and task. All ctrl API will return after complete
	// all the actions. We can add more return error code for a better notice to upper layer.
	status = HAL_AudioProcess(action_code, (HAL_AUDIO_Control_Params_un_t *)arg_param, callback);
	
	return status;
}
#endif
Result_t HAL_AUDIO_RegisterEventCB(
	HAL_AUDIO_EVENT_en_t	event,		///< (in) Event type
	HAL_AUDIO_CB1	callback			///< (in) Callback routine
	)
{
	return RESULT_ERROR;
}



