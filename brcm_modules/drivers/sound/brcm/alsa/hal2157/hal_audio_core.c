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
#include "consts.h"

//#define	I2S_NONE_FM_DIRECT_PATH
#define	I2S_INCLUDED
#ifdef I2S_INCLUDED
//--#include "i2s_drv.h"
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
#include <plat/bcm_i2sdai.h>
#include "i2s_drv_new.h"
#else
#include "i2s.h"
#endif
#if defined(_BCM2153_)
//--#include "i2s_swbuffer.h"
#endif
#endif
#ifdef POLY_INCLUDED
#include "poly_ringer.h"
#endif
#ifdef VPU_INCLUDED
#include "vpu.h"
#endif
#include "audiohal_bits.h"

#if defined (SS_2153)
#define MAX9877_MAX_VOLUME 31
#endif // SS_2153

//#define ENABLE_DEBUG
#ifdef  ENABLE_DEBUG
#define _DBG_(a)		a
static char debug_buf[80];
#define KI_DPRINTF printk
#define	MSG_LOG    printk
#define	AUD_TRACE_DETAIL	printk
#else
#define _DBG_(a)
#endif
extern Boolean CC_IsThereVoiceCall( void);
extern void post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2);
extern SysAudioParm_t* AUDIO_GetParmAccessPtr(void);

extern void program_equalizer(int equalizer_type);
extern void program_poly_equalizer(int equalizer_type);


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
static speakerVol_t  currSpkVol[AUDIO_ID_ALL][AUDIO_CHNL_TOTAL];

//---------------------------------------------
static struct timer_list tone_timer;
static struct timer_list *toneTimer = NULL;
// Destroy the timer
#define OSTIMER_Destroy(a) del_timer_sync(a)
//----------------------------------------------

static Boolean  BB_IsSpeakerOn = FALSE;
static Boolean  BB_IsMicOn = FALSE;
static Boolean  EXT_IsDevOn = FALSE;
static Boolean  bStatusMicMute = FALSE;
static AudioToneType_en_t eToneType = TONE_TYPE_DTMF; //default is DTMF tone
static HAL_AUDIO_CB2 tone_timer_cb2 = NULL;
static Boolean audio_isVoiceOn = FALSE;

static UInt32 audioParam;
static HAL_AUDIO_Param_PathCfg_t	stCurrPath;
static HAL_AUDIO_Param_CodecCfg_t   stCurrVPMCodecCfg;     //for VPM only
//static HAL_AUDIO_Param_PathCfg_t	pathCfg[AUDIO_ID_ALL];
//static UInt8	pathCfg_enable[AUDIO_ID_ALL];

static AUDIO_MIC_un_t  currMicStatus = {0};
static AudioMode_t currAudioMode;

static UInt32 hDev;
static UInt32 playedBufferCount;
Boolean halAudioFMPlaybackEnabled = FALSE;

static I2S_HANDLE i2s_handle; 
static Boolean i2sPcmLoopback=FALSE;
static int currAACEnc_sampleRate = AUDIO_SAMPLERATE_48000; 
static int currAACEnc_monoStereo = TYPE_STEREO; 
static Boolean i2s_op_mode = FALSE; // slave mode =FALSE, master mode = TRUE
#if defined(BCM2153_FAMILY_BB_CHIP_BOND) 
static Boolean i2s_dma = FALSE;
#endif
static Boolean audioDevCtrl_IsInVoiceCall = FALSE;
#ifdef FEATURE_AUDIO_VOICECALL16K
static Boolean audio_isCall16K = FALSE;
#endif // FEATURE_AUDIO_VOICECALL16K

Boolean AUDIO_IsInVoiceCall(void) { return audioDevCtrl_IsInVoiceCall; }


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

static Result_t HAL_AUDIO_PlayGenericTone(HAL_AUDIO_Param_GenericTone_t *pToneParam);

static void HAL_AUDIO_StopPlaytone(void);

void HAL_AUDIO_ProcessToneTimer(unsigned long ptr );

static void HAL_AUDIO_EnablePath( HAL_AUDIO_Param_PathCfg_t * );
static void HAL_AUDIO_DisablePath( HAL_AUDIO_Param_PathCfg_t * );
static void HAL_AUDIO_SwitchPath( HAL_AUDIO_Param_PathCfg_t * );
#ifdef FEATURE_AUDIO_VOICECALL16K
static void HAL_AUDIO_RateChange( HAL_AUDIO_Param_RateCfg_t *rateCfg );
#endif
static void HAL_AUDIO_SetTuneParam (HAL_AUDIO_Param_Tune_t *tune);
static void HAL_AUDIO_ConfigCodecParam( HAL_AUDIO_Param_CodecCfg_t * );
static void HAL_AUDIO_SetEqualizer( HAL_AUDIO_Param_Equalizer_t * );
static void HAL_AUDIO_SetEqualizerProfile( HAL_AUDIO_Param_EqualizerProfile_t *);
static void HAL_AUDIO_Set3D_Widening( HAL_AUDIO_Param_STW_t * );
static void HAL_AUDIO_SetMixer( HAL_AUDIO_Param_Mixer_t * );
#if defined(SS_2153)
static void HAL_AUDIO_SetCustomGain( HAL_AUDIO_Param_CustomGain_t * );
#endif // SS_2153
static void HAL_AUDIO_CtrlAudAlgorithm( HAL_AUDIO_Param_AlgCfg_t * );

static Boolean bIs_I2S_enabled = FALSE;
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
static void HAL_AUDIO_EnableRxDMA(void);
static void HAL_AUDIO_DisableRxDMA(void);
static int HAL_AUDIO_EnableI2s(Boolean loop, Boolean mode, Boolean dma);
static int HAL_AUDIO_DisableI2s();
#endif

UInt32	hDev_audvoc_PCM=0;
UInt32	hDev_audvoc_NBAMR=0;
UInt32	hDev_audvoc_MP3=0;  //rename it to handler_downloadable_codec?

//extern UInt32 audio_control_generic(UInt32 param1,UInt32 param2,UInt32 param3,UInt32 param4);
//extern UInt32 audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);
extern void program_coeff(int equlizer_type);
extern void program_poly_coeff(int equalizer_type);

// convert the number from range scale_in to range scale_out.
UInt16	AUDIO_Util_Convert( UInt16 input, UInt16 scale_in, UInt16 scale_out)
{
	UInt16 output=0;

	output = ( (UInt32)input * (UInt32)scale_out + (UInt32)(scale_in/2) ) / (UInt32)scale_in;
	return output;
}


Result_t HAL_AUDIO_Core_Init( HAL_AUDIO_CONFIG_st_t *config )
{	
	UInt16  i=0, j=0, volume_max = 0, mixer_gain;
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
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_InitExtAMP();
#endif

	//load the default mixer gain (default equ is bypassed)
	mixer_gain = (UInt16)AUDIO_GetParmAccessPtr()[stCurrPath.outputChnl].audvoc_mixergain;
	AUDIO_DRV_SetAudioMixerGain((int)mixer_gain, (int)mixer_gain);

	//AUDIO_ASIC_Init();
	AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
#if defined(ROXY_AUD_CTRL_FEATURES)
	audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0); // 03082009 michael
#endif

	currAudioMode = AUDIO_MODE_HANDSET;
	HAL_AUDIO_SetAudioMode(currAudioMode);
	program_coeff(EQU_NORMAL);
	program_poly_coeff(EQU_NORMAL);

	stCurrVPMCodecCfg.tMode = AUDIO_MODE_PLAYBACK;
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
	AudioExtDev_Init();
#endif
	for (i=0; i<AUDIO_ID_ALL; i++)
		for (j=0; j<AUDIO_CHNL_TOTAL; j++)
		{
			currSpkVol[i][j] = AUDIO_VOLUME_DEFAULT;
		}

#if defined(FUSE_APPS_PROCESSOR)
	volume_max = APSYSPARM_GetAudioParmAccessPtr()[AUDIO_GetAudioMode()].voice_volume_max;
	initSpkrVol = APSYSPARM_GetAudioParmAccessPtr()[AUDIO_GetAudioMode()].voice_volume_init;
#else
	volume_max = SYSPARM_GetVoiceVolumeMax();
	initSpkrVol = SYSPARM_GetVoiceVolumeInit();
#endif

#ifndef USE_VOLUME_CONTROL_IN_DB
	// convert the value in 1st param from range of 2nd_param to range of 3rd_param:
	initSpkrVol = AUDIO_Util_Convert( initSpkrVol, volume_max, AUDIO_VOLUME_MAX );
//	printk("HAL_AUDIO_Core_Init:volume_max=%d initSpkrVol=%d AUDIO_VOLUME_MAX=%d \n", volume_max, initSpkrVol, AUDIO_VOLUME_MAX);
#endif

	for( i=0; i<AUDIO_MODE_NUMBER; i++)
	{
		currSpkVol[AUDIO_ID_TONE][i] = initSpkrVol;
#ifndef FEATURE_AUDIO_VOICECALL16K
		currSpkVol[AUDIO_ID_CALL][i] = initSpkrVol;
#else // FEATURE_AUDIO_VOICECALL16K
		currSpkVol[AUDIO_ID_CALL8K][i] = initSpkrVol;
		currSpkVol[AUDIO_ID_CALL16K][i] = initSpkrVol;
#endif // FEATURE_AUDIO_VOICECALL16K
	}

	AUDIO_DRV_SetVoiceVolume( initSpkrVol );
	_DBG_(MSG_LOG("HAL_AUDIO_Core_Init() Exit() "));

#ifdef POLY_INCLUDED
	POLY_Run();
#endif
#if 0
	// interal task and message queue
	AudioMsgQ = OSQUEUE_Create( QUEUESIZE_HAL_AUDIO, sizeof( AudioMsg_t ), OSSUSPEND_PRIORITY );
	AudioTask = OSTASK_Create( AudioTaskEntry, "Audio_Task_Internal", TASKPRI_HAL_AUDIO, STACKSIZE_HAL_AUDIO );

#if defined (SS_2153)
	if (audio_tone_sema == NULL) { // 07232009 michael
		audio_tone_sema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
	}
#endif

	HAL_AudioSema = OSSEMAPHORE_Create(1, OSSUSPEND_PRIORITY);
#endif	
	return RESULT_OK;
}


// !! A very special usage of audio controller. Don't go through HAL API.


Result_t HAL_AudioProcess(HAL_AUDIO_ACTION_en_t action_code,HAL_AUDIO_Control_Params_un_t *arg_param,void *callback)
{
	Result_t result = RESULT_OK;
	HAL_AUDIO_ACTION_en_t actionCode;
	HAL_AUDIO_Control_Params_un_t *param;
	//void* HAL_CallBack;
    HAL_AUDIO_CB2 HAL_CallBack;

	// Use semaphor for protection
	//OSSEMAPHORE_Obtain(HAL_AudioSema,TICKS_FOREVER);

	actionCode = action_code;
	param = (HAL_AUDIO_Control_Params_un_t *)arg_param;
	HAL_CallBack = (HAL_AUDIO_CB2)callback;

	switch (actionCode) 
	{
		case ACTION_AUD_SetSpeakerVol:
			{
#if defined (SS_2153)
				UInt16 	spkVol = param->param_speakerVolume.volumeLevel;
#else // SS_2153
				UInt8 	spkVol = param->param_speakerVolume.volumeLevel;
#endif // SS_2153
#if defined(ROXY_AUD_CTRL_FEATURES)

				// 02172009 michael
				if (spkVol == currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl])
				{
					// skip the volume setting at attempt to set the same volume as the current one.
					_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerVolume(): identical to the current volume\r\n"));
					break;
				}

#endif

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
			if (HAL_CallBack)
			{
				(*HAL_CallBack)(TONE_STARTED);
			}	
			break;
		
		case ACTION_AUD_PlayGenericTone:
			HAL_AUDIO_PlayGenericTone(&(param->param_genericTone));
			if (HAL_CallBack)
			{
				(*HAL_CallBack)(TONE_STARTED);
			}	
			break;
		
		case ACTION_AUD_StopPlayTone:
			HAL_AUDIO_StopPlaytone();

			// since both natually finished and manually stopped tone will go to here, how do when
			// distinguish these 2 cases? do we need to?
			if (HAL_CallBack)
			{
				(*HAL_CallBack)(TONE_STOPPED);
			}	
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
			HAL_AUDIO_ConfigCodecParam( &(param->param_codecCfg) );
			break;

		case ACTION_AUD_SetEqualizer:
			HAL_AUDIO_SetEqualizer( &(param->param_equalizer) );
			break;

		case ACTION_AUD_SetEqualizerProfile:
			HAL_AUDIO_SetEqualizerProfile( &(param->param_equalizerProfile) );
			break;

		case ACTION_AUD_SetSTWidening:  
			HAL_AUDIO_Set3D_Widening( &(param->param_3D_widening) );
			break;
						
		case ACTION_AUD_SetMixer:
			HAL_AUDIO_SetMixer( &(param->param_mixer) );
			break;		
#if defined(SS_2153)
		case ACTION_AUD_SetCustomGain:
			HAL_AUDIO_SetCustomGain( &(param->param_custom_gain) );
			break;
#endif // SS_2153

		case ACTION_AUD_CtrlAudAlg:
			HAL_AUDIO_CtrlAudAlgorithm( &(param->param_algCfg) );
			break;
#ifdef FEATURE_AUDIO_VOICECALL16K
		case ACTION_AUD_RateChange:
			HAL_AUDIO_RateChange( &(param->param_rateCfg) );
			break;
#endif // FEATURE_AUDIO_VOICECALL16K

		case ACTION_AUD_TUNE:
			HAL_AUDIO_SetTuneParam (&(param->param_tune) );
			break;
		default:
			result = RESULT_ERROR;
			break;
	}

//	OSSEMAPHORE_Release(HAL_AudioSema);

	return result;
}
	


#if !defined(CUSTOMER_OEM)
//------------------------------------------
// not define CUSTOMER_OEM, BRCM code only.
//----------------------------------------- 
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
#if defined(ROXY_AUD_CTRL_FEATURES)
			*(volatile UInt32 *)ANACR0_REG |= 0x00000001;
#endif
			bOnFlag_EXT = TRUE;
			break;

		case AUDIO_ID_TONE:
#if defined (SS_2153)
//++ BRCM add CSP#252841
			AUDIO_DRV_SetVoiceVolume(0); // 09012009 michael (to suppress DL channel)
//-- BRCM add CSP#252841
#endif
			AUDIO_DRV_SetDTMFPathOn(stCurrPath.outputChnl );
			break;
		
#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoicePathOn(currAudioMode);
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			AUDIO_DRV_SetVoicePathOn(currAudioMode, audioID);
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:
		case AUDIO_ID_VPU_VT_LOOPBACK:
			AUDIO_DRV_SetAMRPathOn(stCurrPath.outputChnl );
			break;

		case AUDIO_ID_WAVETONE_POLY:
			//set the poly path to mono/stereo per spk type
 			AUDIO_DRV_SetPolyChannel(stCurrPath.outputChnl);
			//load the special FIR/IIR filter for wavetone (if necessary)
			//audio_control_generic(PROGRAM_WAVETONE_FIR_IIR_FILTER, 0, 0, 0);
#if defined(ROXY_TONE_FEATURES)
			AUDIO_DRV_SetPolyKeytonePathOn(stCurrPath.outputChnl );
#else
			AUDIO_DRV_SetPolyPathOn(stCurrPath.outputChnl );
#endif
			break;
			
		case AUDIO_ID_POLY:
			//reset to mono playback
			AUDIO_DRV_SetPolyLRChannel(LOUT_LIN, ROUT_LIN);
			//reload FIR/IIR filter for midi
			//audio_control_generic(PROGRAM_POLY_FIR_IIR_FILTER, 0, 0, 0);
			AUDIO_DRV_SetPolyPathOn(stCurrPath.outputChnl );
			break;
			
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathOn(stCurrPath.outputChnl );
			break;
				
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
#ifndef FEATURE_AUDIO_VOICECALL16K			
			AUDIO_DRV_SetVoicePathOn(currAudioMode);
#else
			AUDIO_DRV_SetVoicePathOn(currAudioMode, audioID);
#endif
			break;
			
		default:
			break;
	}


	BB_IsSpeakerOn = AUDIO_ON;
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
	if (((TRUE == bOnFlag_EXT)&&(FALSE == EXT_IsDevOn))||(EXT_IsDevOn))

	{
		AudioExtDev_PowerOn(&stCurrPath);
		EXT_IsDevOn = TRUE;

//		if(audioID != AUDIO_ID_IIS)
//		{
//			if((AUDIO_CHNL_SPEAKER == stCurrPath.outputChnl)&&( TRUE==HAL_EXT_AUDIO_NeedIisMclkForLoudSpkr ))
//				PWRMGMT_IIS_MCLK_Enable( IIS_MCLOCK_CLIENT_BITMASK_WM8960 );
//		}
	}
#endif
	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n",
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}
#endif   ///end of "#if !defined(CUSTOMER_OEM)"




#if !defined(CUSTOMER_OEM)
//------------------------------------------
// not define CUSTOMER_OEM, BRCM code only.
//----------------------------------------- 
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

#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			//AUDIO_DRV_SetVoiceVolume(0);
			//recover the volume level to init level. this is to make sure the amr-nb ringtone sounds 
			//the same level as first time. 
			AUDIO_DRV_SetVoiceVolume( initSpkrVol );
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			AUDIO_DRV_SetVoiceVolume( initSpkrVol );
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_TONE:
			AUDIO_DRV_SetToneVolume(eToneType, 0);
			break;

		case AUDIO_ID_VPU:					
		case AUDIO_ID_VPU_TELEPHONY:
#ifdef USE_DSP_AUDIO_AMR_VOLUME
			AUDIO_DRV_SetAMRVolume(0);
#else
			AUDIO_DRV_SetAMRPathGain(0);
#endif //#ifdef USE_DSP_AUDIO_AMR_VOLUME
			break;
			
		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathGain(0, 0);
			break;

		case AUDIO_ID_WAVETONE_POLY:
		case AUDIO_ID_POLY:
			AUDIO_DRV_SetPolyPathGain(0, 0);
			break;

		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
			AUDIO_DRV_SetVoiceVolume( initSpkrVol );
			break;
				
		default:
			break;
	}
	OSTASK_Sleep(10);// wait until take effect
#if defined(ROXY_TONE_FEATURES)
	currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl]=0;
#endif
	switch( audioID )
	{	
		case AUDIO_ID_IIS:
			break;
			
		case AUDIO_ID_EXT_ANLG:
#if defined(ROXY_AUD_CTRL_FEATURES)
			*(volatile UInt32 *)ANACR0_REG |= 0x00000001;
#endif			
			break;

		case AUDIO_ID_TONE:
#if defined (SS_2153)
//++ BRCM add CSP#252841
			AUDIO_DRV_SetVoiceVolume((UInt8)currSpkVol[AUDIO_ID_CALL][stCurrPath.outputChnl]); // 09012009 michael (to recover DL channel)
//-- BRCM add CSP#252841
#endif
			AUDIO_DRV_SetDTMFPathOff(stCurrPath.outputChnl );
			break;
		
#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:		
			AUDIO_DRV_SetVoicePathOff(currAudioMode);
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			AUDIO_DRV_SetVoicePathOff(currAudioMode);
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:
		case AUDIO_ID_VPU_VT_LOOPBACK:
			AUDIO_DRV_SetAMRPathOff(stCurrPath.outputChnl );
			break;

		case AUDIO_ID_WAVETONE_POLY:
#if defined(ROXY_TONE_FEATURES)
			AUDIO_DRV_SetPolyKeytonePathOff(stCurrPath.outputChnl );
			break;
#endif
		case AUDIO_ID_POLY:
			AUDIO_DRV_SetPolyPathOff(stCurrPath.outputChnl );
			break;
			
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathOff(stCurrPath.outputChnl );
			break;
				
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
			AUDIO_DRV_SetVoicePathOff(currAudioMode);
			break;
			
		default:
			break;
	}

	BB_IsSpeakerOn = AUDIO_OFF;
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
	if(EXT_IsDevOn)
	{	
		AudioExtDev_PowerOff( );
		EXT_IsDevOn =FALSE;
	}
#endif	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerOff()  ID=%d, CH=%d, Speaker=%d, ExtDev=%d\r\n", audioID, stCurrPath.outputChnl, BB_IsSpeakerOn, EXT_IsDevOn));

	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}

#endif    //end of #if !defined(CUSTOMER_OEM)



/* Turn the Microphone on
	param	id		audio user ID
*/
void HAL_AUDIO_SetMicrophoneOn( AudioClientID_en_t audioID)
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetMicrophoneOn(): audioID=%d\r\n", audioID ));

	switch(audioID)
	{
#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			// voice uses 8k as default
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, TRUE);
			AUDIO_DRV_ConnectMicrophoneUplink(TRUE);			
			currMicStatus.stMIC.CALL_MIC_ON = TRUE;
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			AUDIO_DRV_SetDualMicPathOnOff(currAudioMode, TRUE);
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, TRUE);
			AUDIO_DRV_ConnectMicrophoneUplink(TRUE);			
			currMicStatus.stMIC.CALL_MIC_ON = TRUE;
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
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
		
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, TRUE);
			AUDIO_DRV_ConnectMicrophoneUplink(TRUE);			
			currMicStatus.stMIC.CALL_MIC_ON = TRUE;
			break;			
			
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
		case AUDIO_ID_FM_RECORD_PCM:
		case AUDIO_ID_FM_RECORD_PCM_44_1K:
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
#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			currMicStatus.stMIC.CALL_MIC_ON = FALSE;
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			currMicStatus.stMIC.CALL_MIC_ON = FALSE;
			break;			
#endif // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_RECORD:
		case AUDIO_ID_VPU_TELEPHONY:
		case AUDIO_ID_VPU_VT_LOOPBACK:
#if defined(ROXY_AUD_CTRL_FEATURES)
			// voice uses 8k as default
			AUDIO_DRV_ConnectMicrophoneUplink(FALSE);
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, FALSE);
#else
			currMicStatus.stMIC.VPU_MIC_ON = FALSE;
#endif
			break;

		case AUDIO_ID_AMRWB_RECORD: 
#if defined(ROXY_AUD_CTRL_FEATURES) 
			// amr-wb uses 16k
			AUDIO_DRV_SetMicrophonePath_16K(FALSE);
			AUDIO_DRV_SetMicrophonePathOnOff(currAudioMode, FALSE);	
#else
			currMicStatus.stMIC.AUDVOC_MIC_ON = FALSE;
#endif
			break;

		case AUDIO_ID_HQ_AUDIO_RECORD:
#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			// audio codec path, including DSP. Need to rework on this!!!
#endif //#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			break;
		
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:		
			currMicStatus.stMIC.CALL_MIC_ON = FALSE;
			break;
			
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
		case AUDIO_ID_FM_RECORD_PCM:
		case AUDIO_ID_FM_RECORD_PCM_44_1K:
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
	AUDIO_DRV_SetDualMicPathOnOff(currAudioMode, FALSE);
	AudioExtDev_SetBiasOnOff(currAudioMode, FALSE);
#if defined(ROXY_AUD_CTRL_FEATURES)
	AUDIO_DRV_MicrophoneMuteOnOff(FALSE);
	BB_IsMicOn = AUDIO_OFF;
#endif
	bStatusMicMute = FALSE;
		
	BB_IsMicOn = AUDIO_OFF;

	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}


#if !defined(CUSTOMER_OEM)
//------------------------------------------
// not define CUSTOMER_OEM, BRCM code only.
//----------------------------------------- 
static void HAL_AUDIO_MuteSpeaker()
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_MuteSpeaker()\r\n"));

	switch( stCurrPath.audioID)
	{
#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoiceVolume( 0 );  //better use a separate mute function.!!
			AUDIO_DRV_MuteAMRPathOnOff(TRUE);
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			AUDIO_DRV_SetVoiceVolume( 0 );  //better use a separate mute function.!!
#if defined(FUSE_APPS_PROCESSOR)		
			audio_control_dsp(MUTE_DSP_DL, 0, 0, 0);	
#else
			VOLUMECTRL_MuteRx();
#endif
			AUDIO_DRV_MuteAMRPathOnOff(TRUE);
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_TONE:
			AUDIO_DRV_MuteDTMFPathOnOff(TRUE);
			break;

		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:	
		case AUDIO_ID_VPU_VT_LOOPBACK:

			AUDIO_DRV_MuteAMRPathOnOff(TRUE);
			break;
			
		case AUDIO_ID_WAVETONE_POLY:					
		case AUDIO_ID_POLY:
			AUDIO_DRV_MutePolyPathOnOff(TRUE);
			break;
							
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_MuteAudioPathOnOff(TRUE);
			break;
			
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
			AUDIO_DRV_SetVoiceVolume( 0 );  //better use a separate mute function.!!
			AUDIO_DRV_MuteAMRPathOnOff(TRUE);
			break;
			
		default:
			_DBG_(KI_DPRINTF("HAL_AUDIO_MuteSpeaker(): wrong current ID=%d\r\n", stCurrPath.audioID));
			break;
	}
	
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
	if(EXT_IsDevOn)
		AudioExtDev_SetVolume(0);  //better use a separate mute function.!!
#endif
}

#endif   //end of "#if !defined(CUSTOMER_OEM)"


#if !defined(CUSTOMER_OEM)
//------------------------------------------
// not define CUSTOMER_OEM, BRCM code only.
//----------------------------------------- 
static void HAL_AUDIO_UnmuteSpeaker()
{
	speakerVol_t index = currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];
	_DBG_(KI_DPRINTF("HAL_AUDIO_UnmuteSpeaker()\r\n"));
	
	switch( stCurrPath.audioID)
	{
#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			index = currSpkVol[AUDIO_ID_CALL][stCurrPath.outputChnl];
			AUDIO_DRV_SetVoiceVolume( (UInt8)index );
			AUDIO_DRV_MuteAMRPathOnOff(FALSE);
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			index = currSpkVol[AUDIO_ID_CALL][stCurrPath.outputChnl];
#if defined(FUSE_APPS_PROCESSOR)		
			audio_control_dsp(UNMUTE_DSP_DL, 0, 0, 0);	
#else
			VOLUMECTRL_UnMuteRx();
#endif
			AUDIO_DRV_SetVoiceVolume( (UInt8)index );
			AUDIO_DRV_MuteAMRPathOnOff(FALSE);
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
									
		case AUDIO_ID_TONE:
			AUDIO_DRV_MuteDTMFPathOnOff(FALSE);
			break;

		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:	
		case AUDIO_ID_VPU_VT_LOOPBACK:
			AUDIO_DRV_MuteAMRPathOnOff(FALSE);
			break;
			
		case AUDIO_ID_WAVETONE_POLY:					
		case AUDIO_ID_POLY:
			AUDIO_DRV_MutePolyPathOnOff(FALSE);
			break;
							
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_MuteAudioPathOnOff(FALSE);
			break;
			
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
			index = currSpkVol[AUDIO_ID_CALL][stCurrPath.outputChnl];
			AUDIO_DRV_SetVoiceVolume( (UInt8)index );
			AUDIO_DRV_MuteAMRPathOnOff(FALSE);
			break;
			
		default:
			_DBG_(KI_DPRINTF("HAL_AUDIO_UnmuteSpeaker(): wrong current ID=%d\r\n", stCurrPath.audioID));
			break;
	}
	
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
	//volume level is per customer's request
	if(EXT_IsDevOn)
#if defined (SS_2153)
					AudioExtDev_SetVolume(AudioGain_ExtDev(index, MAX9877_MAX_VOLUME));
#else // SS_2153
					AudioExtDev_SetVolume(index);
#endif // SS_2153
#endif
}
#endif   //end of "#if !defined(CUSTOMER_OEM)"

static void HAL_AUDIO_MuteMicrophone()
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_MuteMicrophone()\r\n"));
#if defined(ROXY_AUD_CTRL_FEATURES)
	if (stCurrPath.audioID == AUDIO_ID_EXT_ANLG) // 05142009 michael
			return;
#endif

	AUDIO_DRV_MicrophoneMuteOnOff(TRUE);
	bStatusMicMute = TRUE;
}

static void HAL_AUDIO_UnmuteMicrophone()
{	
	_DBG_(KI_DPRINTF("HAL_AUDIO_UnmuteMicrophone()\r\n"));
#if defined(ROXY_AUD_CTRL_FEATURES)
	if (stCurrPath.audioID == AUDIO_ID_EXT_ANLG) // 05142009 michael
			return;
#endif

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
	

//Platform team implemented the volume-change check here, Michael in HAL_AudioProcess.  
// Cannot not do both, but Michael's version has been extensively tested, so we use his.
#if !defined(ROXY_AUD_CTRL_FEATURES)
	
	if (speakerVol == currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl])
	{
		// skip the volume setting at attempt to set the same volume as the current one.
		_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerVolume(): identical to the current volume\r\n"));
		return;
	}

#endif
	if (speakerVol > AUDIO_VOLUME_MAX) // 11162008 michael (discard the out-of-range value and set default)
		speakerVol = currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];

	switch(stCurrPath.audioID)
	{
		case AUDIO_ID_IIS:			
			break;

#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			//for coverity, make sure the speakerVol is in the range for voice call
			speakerVol = (speakerVol > 40) ? 40 : speakerVol;
			AUDIO_DRV_SetVoiceVolume( (UInt8)speakerVol );
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			speakerVol = (speakerVol > 40) ? 40 : speakerVol;
			AUDIO_DRV_SetVoiceVolume( (UInt8)speakerVol );
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
			
		case AUDIO_ID_TONE:
			//modified the tone volume here to make it sound close as before
			AUDIO_DRV_SetToneVolume(eToneType, speakerVol/3);
			break;

		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:	
		case AUDIO_ID_VPU_VT_LOOPBACK:			
#ifdef USE_DSP_AUDIO_AMR_VOLUME
			//use second amr out gain to control amr volume
			AUDIO_DRV_SetAMRVolume(speakerVol);
#else
			AUDIO_DRV_SetAMRPathGain(speakerVol & MASK_SLOPGAIN_VALUE);
#endif //#ifdef USE_DSP_AUDIO_AMR_VOLUME
			break;
			
		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathGain(speakerVol & MASK_SLOPGAIN_VALUE, speakerVol & MASK_SLOPGAIN_VALUE);
			break;

		case AUDIO_ID_WAVETONE_POLY:
		case AUDIO_ID_POLY:
			AUDIO_DRV_SetPolyPathGain(speakerVol & MASK_SLOPGAIN_VALUE, speakerVol & MASK_SLOPGAIN_VALUE);
			break;

		default:
			break;
	}

#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
	AudioExtDev_SetVolume(AudioGain_ExtDev(speakerVol, MAX9877_MAX_VOLUME));
#endif

	currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl]=speakerVol;
#ifdef FEATURE_AUDIO_VOICECALL16K
	if((stCurrPath.audioID == AUDIO_ID_CALL8K)||(stCurrPath.audioID == AUDIO_ID_CALL16K)) {
		currSpkVol[AUDIO_ID_CALL8K][stCurrPath.outputChnl]=speakerVol;
		currSpkVol[AUDIO_ID_CALL16K][stCurrPath.outputChnl]=speakerVol;
	}
#endif // FEATURE_AUDIO_VOICECALL16K

}
#else // SS_2153
static void HAL_AUDIO_SetSpeakerVolume(UInt8 speakerVol)
{
	UInt16 uDigVol;
	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerVolume(): audioID=%d, Ch=%d, Vol=%d\r\n", \
		stCurrPath.audioID, stCurrPath.outputChnl, speakerVol));

#if defined(ROXY_AUD_CTRL_FEATURES)
	// 12112008 michael
	if (speakerVol > AUDIO_VOLUME_MAX) // 11162008 michael (discard the out-of-range value and set default)
		speakerVol = currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];
#endif
	switch(stCurrPath.audioID)
	{
		case AUDIO_ID_IIS:			
			break;

#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoiceVolume( speakerVol );
			break;
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			AUDIO_DRV_SetVoiceVolume( speakerVol );
			break;
#endif // FEATURE_AUDIO_VOICECALL16K
			
		case AUDIO_ID_TONE:
			//modified the tone volume here to make it sound close as before
			AUDIO_DRV_SetToneVolume(eToneType, speakerVol/3);
			break;

		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:	
		case AUDIO_ID_VPU_VT_LOOPBACK:
#ifdef USE_DSP_AUDIO_AMR_VOLUME
			//use second amr out gain to control amr volume
			AUDIO_DRV_SetAMRVolume(speakerVol);
#else
			if ( speakerVol==0)				
				uDigVol = 0x00;  //digital mute.
			else
			{
				uDigVol = ((((UInt16)AUDIO_GetParmAccessPtr()[stCurrPath.outputChnl].audvoc_vslopgain) & MASK_SLOPGAIN_VALUE)- 40*4) + (speakerVol * 4);
			}
			AUDIO_DRV_SetAMRPathGain(uDigVol);
#endif //#ifdef USE_DSP_AUDIO_AMR_VOLUME
			break;
			
		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			if ( speakerVol==0)
				uDigVol = 0x00;  //digital mute.
			else
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
				uDigVol = ((((UInt16)(AUDIO_GetParmMultimediaAccessPtr()[AUDIO_DRV_ConvertChnlMode(stCurrPath.outputChnl)].audvoc_aslopgain)) & MASK_SLOPGAIN_VALUE)- 40*4) + (speakerVol * 4);
#else //  BCM2153_FAMILY_BB_CHIP_BOND
				uDigVol = ((((UInt16)AUDIO_GetParmAccessPtr()[stCurrPath.outputChnl].audvoc_aslopgain) & MASK_SLOPGAIN_VALUE)- 40*4) + (speakerVol * 4);
#endif //  BCM2153_FAMILY_BB_CHIP_BOND
			AUDIO_DRV_SetAudioPathGain(uDigVol, uDigVol);
			break;

		case AUDIO_ID_WAVETONE_POLY:
		case AUDIO_ID_POLY:
			if ( speakerVol==0)
				uDigVol = 0x00;  //digital mute.
			else
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
				uDigVol = ((((UInt16)AUDIO_GetParmMultimediaAccessPtr()[AUDIO_DRV_ConvertChnlMode(stCurrPath.outputChnl)].audvoc_pslopgain) & MASK_SLOPGAIN_VALUE)- 40*4) + (speakerVol * 4);
#else //  BCM2153_FAMILY_BB_CHIP_BOND
				uDigVol = ((((UInt16)AUDIO_GetParmAccessPtr()[stCurrPath.outputChnl].audvoc_pslopgain) & MASK_SLOPGAIN_VALUE)- 40*4) + (speakerVol * 4);
#endif //  BCM2153_FAMILY_BB_CHIP_BOND			
			AUDIO_DRV_SetPolyPathGain(uDigVol, uDigVol);
			break;

		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
			AUDIO_DRV_SetVoiceVolume( speakerVol );
			break;
			
		default:
			break;
	}

#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
	AudioExtDev_SetVolume(speakerVol);
#endif

	currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl]=speakerVol;
#ifdef FEATURE_AUDIO_VOICECALL16K
	if((stCurrPath.audioID == AUDIO_ID_CALL8K)||(stCurrPath.audioID == AUDIO_ID_CALL16K)) {
		currSpkVol[AUDIO_ID_CALL8K][stCurrPath.outputChnl]=speakerVol;
		currSpkVol[AUDIO_ID_CALL16K][stCurrPath.outputChnl]=speakerVol;
	}
#endif // FEATURE_AUDIO_VOICECALL16K

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
	_DBG_(KI_DPRINTF( debug_buf ) );
	
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

#if defined (SS_2153)
	OSSEMAPHORE_Obtain(audio_tone_sema, 20); // 07232009 michael
#endif
	res = AUDIO_DRV_PlayGenericTone(pToneParam->superimpose, pToneParam->duration, pToneParam->f1, pToneParam->f2, pToneParam->f3);

	//start the timer
	add_timer(toneTimer);

	return res;
}

#if !defined(CUSTOMER_OEM)
//------------------------------------------
// not define CUSTOMER_OEM, BRCM code only.
//----------------------------------------- 

// Tone timer interrupt callback.
// Since timer is HISR, this function can't block at semaphore.
//Arg ptr has no meaning,its here just to satisfy the linux Timer callback prototype
void HAL_AUDIO_ProcessToneTimer(unsigned long ptr )
{
	
	//Add the Tone stop msg to the HAL_ctrl queue 
	HAL_AUDIO_Ctrl(ACTION_AUD_StopPlayTone,NULL,NULL);
	


}
#endif //end of "#if !defined(CUSTOMER_OEM)"


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

		HAL_AUDIO_SetSpeakerOff(AUDIO_ID_TONE);
	}
}

static void HAL_AUDIO_EnablePath( HAL_AUDIO_Param_PathCfg_t *pathCfgPtr )
{
	Boolean bCur_StatusMicMute;

#if defined (SS_2153)
	UInt16 speakerVolume;
	UInt16 mixer_gain;
	mixer_gain = (UInt16)AUDIO_GetParmAccessPtr()[stCurrPath.outputChnl].audvoc_mixergain;
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

	AUDIO_DRV_PowerUpD2C();
	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n",
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));


	switch(stCurrPath.audioID)
	{
		case AUDIO_ID_IIS2DAC:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153
			AUDIO_DRV_SetIIS2Audio(TRUE); //i2s path to DAC.
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			if  (!i2sPcmLoopback) {
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
			  	if(!i2s_handle)
					i2s_handle=I2SDRV_Init(0); 
#else
			  i2s_handle=i2s_init(0); 
#endif
			}

			if (!bIs_I2S_enabled)
			{
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
			if (i2sPcmLoopback) 
			  HAL_AUDIO_EnableI2s(TRUE, TRUE, FALSE);
			else 
			  HAL_AUDIO_EnableI2s(FALSE, FALSE, FALSE);
#else			
			if (i2sPcmLoopback) 
			  HAL_AUDIO_EnableI2s(TRUE, TRUE);
			else 
			  HAL_AUDIO_EnableI2s(FALSE, FALSE);
#endif
			}
			bIs_I2S_enabled = TRUE;
			break;
#ifdef	I2S_NONE_FM_DIRECT_PATH
		case AUDIO_ID_IIS2DSP:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153
			// Don't change the order of sequence
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);

			// enable the virtual channel to DSP for playback
			HAL_AUDIO_EnableVirtualChannel_PlayFmI2s(&gAudioHalAccess.VirtualChannel_PlayFmI2s);
			// enable Fm radio I2S device
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
			if (!bIs_I2S_enabled)
			i2s_dma = TRUE;
#endif
			HAL_AUDIO_EnableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			// enable Fm radio I2S device software FIFO
			HAL_AUDIO_EnableExtDev_FmI2s_swFifo(&gAudioHalAccess.ExtDev_FmI2s);

			if (!bIs_I2S_enabled)
			{
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
			if (i2sPcmLoopback) 
			  HAL_AUDIO_EnableI2s(TRUE, TRUE, TRUE);
			else 
			  HAL_AUDIO_EnableI2s(FALSE, FALSE, TRUE);
#else		
			if (i2sPcmLoopback) 
			  HAL_AUDIO_EnableI2s(TRUE, TRUE);
			else 
			  HAL_AUDIO_EnableI2s(FALSE, FALSE);
#endif
			}
			bIs_I2S_enabled = TRUE;
			break;
		case AUDIO_ID_FM_RECORD_PCM:
		case AUDIO_ID_FM_RECORD_PCM_44_1K:
			// enable the virtual channel to record PCM
			HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_PCM(&gAudioHalAccess.VirtualChannel_RecordFmI2s_PCM);
			// enable Fm radio I2S device
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
			if (!bIs_I2S_enabled)
				i2s_dma = TRUE;
#endif
			HAL_AUDIO_EnableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			// enable Fm radio I2S device software FIFO
			HAL_AUDIO_EnableExtDev_FmI2s_swFifo(&gAudioHalAccess.ExtDev_FmI2s);

			if (!bIs_I2S_enabled)
			{
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
				if (i2sPcmLoopback) 
				  HAL_AUDIO_EnableI2s(TRUE, TRUE, i2s_dma);
				else 
				  HAL_AUDIO_EnableI2s(FALSE, FALSE, i2s_dma);
#else
			if (i2sPcmLoopback) 
			  HAL_AUDIO_EnableI2s(TRUE, TRUE);
			else 
			  HAL_AUDIO_EnableI2s(FALSE, FALSE); // 08102009 michael (to start RX with right setting)
#endif
			}
			else
			{
				if(i2s_dma==FALSE)
					HAL_AUDIO_EnableRxDMA();
			}
			bIs_I2S_enabled = TRUE;
			break;

		case AUDIO_ID_FM_RECORD_AAC:
			// enable the virtual channels to record FM as AAC, to DSP and from DSP 
			HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_AAC_DSP_In(&gAudioHalAccess.VirtualChannel_RecordFmI2s_AAC_DSP_In);
			HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_AAC_DSP_Out(&gAudioHalAccess.VirtualChannel_RecordFmI2s_AAC_DSP_Out);
			// enable Fm radio I2S device
			HAL_AUDIO_EnableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			// enable Fm radio I2S device software FIFO
			HAL_AUDIO_EnableExtDev_FmI2s_swFifo(&gAudioHalAccess.ExtDev_FmI2s);
			break;
#endif //#ifdef I2S_NONE_FM_DIRECT_PATH			
		case AUDIO_ID_TONE:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
//++ BRCM add CSP#252841
			HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl ); // 08272009 michael
//-- BRCM add CSP#252841
#endif // SS_2153
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			break;

#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153

			bCur_StatusMicMute = bStatusMicMute;

			HAL_AUDIO_MuteMicrophone();
			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);

			
			HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);
			
			OSTASK_Sleep( 40 );

			AUDIO_DRV_EC_NS_OnOff(TRUE, TRUE);
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
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153

			bCur_StatusMicMute = bStatusMicMute;
			audio_isCall16K = (stCurrPath.audioID == AUDIO_ID_CALL16K)? TRUE : FALSE;
			if(pathCfgPtr->outputChnl == AUDIO_CHNL_BLUETOOTH)
				audio_isCall16K = FALSE;

			HAL_AUDIO_MuteMicrophone();
			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			//AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			audio_control_dsp(ENABLE_DSP_AUDIO, 0, audio_isCall16K, 0);

			
			HAL_AUDIO_SetAudioMode( AUDIO_DRV_ConvertVoiceIDChnlMode(stCurrPath.audioID, pathCfgPtr->outputChnl) );
			audio_isVoiceOn = TRUE;
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);
			
			OSTASK_Sleep( 40 );
			
			AUDIO_DRV_EC_NS_OnOff(TRUE, TRUE);
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

#endif // FEATURE_AUDIO_VOICECALL16K

		case AUDIO_ID_VPU:
        case AUDIO_ID_VPU_VT_LOOPBACK:
#if defined(ROXY_TONE_FEATURES)
			HAL_AUDIO_SetDefaultVoiceVolume(); // 03062009 michael
#endif

			switch(stCurrVPMCodecCfg.tMode)
				{
				case AUDIO_MODE_PLAYBACK:
#if defined(SS_2153)
					AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153
					//HAL_AUDIO_SetMicrophoneOff(stCurrPath.audioID); // don't need MIC
					if(!audio_isVoiceOn)
						HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );					
					HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
					HAL_AUDIO_SetSpeakerVolume(speakerVolume);
					break;		
				case AUDIO_MODE_RECORD:
#if defined(ROXY_AUD_CTRL_FEATURES)
					audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0); // 03082009 michael
#endif
					//HAL_AUDIO_SetSpeakerOff(stCurrPath.audioID);	// don't need SPEAKER
					if(!audio_isVoiceOn)
						HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
#if defined(SS_2153)
					AUDIO_DRV_EC_NS_OnOff(FALSE, TRUE);
					// AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
					audio_control_dsp(CONTROL_ECHO_CANCELLATION, 0, 0, 0);
#endif // SS_2153
					HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);	
					break;	
#if CHIPVERSION >= CHIP_VERSION(BCM2132,32) /* BCM2132C2 (with patch) and later */
				case AUDIO_MODE_TELEPHONY:
#if defined(SS_2153)
					AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153
					HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
					HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
					HAL_AUDIO_SetSpeakerVolume(speakerVolume);
					AUDIO_DRV_EC_NS_OnOff(TRUE, TRUE);
#if defined(ROXY_AUD_CTRL_FEATURES)

					// 12112008 Michael (disable NLP due to volume fluctuation for loopback case only)
					if (stCurrPath.audioID == AUDIO_ID_VPU_VT_LOOPBACK) {
						_DBG_(KI_DPRINTF( "VPU_VT_LOOPBACK: voiceMode = %d", pathCfgPtr->outputChnl ));
						// audio_control_dsp(CONTROL_ECHO_CANCELLATION, 0, 0, 0);
						if (pathCfgPtr->outputChnl == AUDIO_MODE_HEADSET)
							audio_control_dsp(CONTROL_ECHO_CANCELLATION, 0, 0, 0);
						else
							audio_control_dsp(CONTROL_ECHO_CANCELLATION, 1, 0, 0);
					}
#endif					
					HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);

#if !defined(SS_2153)
					// need to unmute the mic for vt lpbk test after the call finished.
					// the mic is muted after call due to dsp ul noise
					if (stCurrPath.audioID == AUDIO_ID_VPU_VT_LOOPBACK)
					{
						bCur_StatusMicMute = bStatusMicMute;
						if (!bCur_StatusMicMute)
							HAL_AUDIO_UnmuteMicrophone();
					}
#endif //#if  !defined (SS_2153)

#ifdef USB_AUDIO_ENABLED
					if (pathCfgPtr->outputChnl==AUDIO_CHNL_USB) {
					  HAL_AUDIO_EnableExtDev_Usb_swFifo(&gAudioHalAccess.ExtDev_Usb); 
					  HAL_AUDIO_EnableExtDev_Usb(&gAudioHalAccess.ExtDev_Usb); 
					  HAL_AUDIO_EnableVirtualChannel_Usb(&gAudioHalAccess.VC_Usb); 
					}
#endif					
#endif   //end of #if CHIPVERSION >= CHIP_VERSION(BCM2132,32) 
				}
			break;
			
		case AUDIO_ID_VPU_RECORD:
#if defined(ROXY_AUD_CTRL_FEATURES)
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0); // 03082009 michael
#endif

#if defined(ROXY_TONE_FEATURES)
			HAL_AUDIO_SetDefaultVoiceVolume(); // 03062009 michael
#endif
			//HAL_AUDIO_SetSpeakerOff(stCurrPath.audioID);	// don't need SPEAKER
			if(!audio_isVoiceOn)
				HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
#if defined(SS_2153)

			AUDIO_DRV_EC_NS_OnOff(FALSE, TRUE);
			// AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(CONTROL_ECHO_CANCELLATION, 0, 0, 0);
#endif // SS_2153
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
			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);

			
			HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);

			OSTASK_Sleep( 40 );

			AUDIO_DRV_EC_NS_OnOff(TRUE, TRUE);
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

		case AUDIO_ID_AMRWB_RECORD:
#if defined(ROXY_AUD_CTRL_FEATURES)
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0); // 03082009 michael
#endif
			if(!audio_isVoiceOn)
				HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
			
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);
			break;


		case AUDIO_ID_PCM_READBACK: // used for pcm readback. 
			AUDIO_DRV_SetPcmReadBackPathOn();
			break;
#ifdef I2S_NONE_FM_DIRECT_PATH
		case AUDIO_ID_IIS: // external I2S device
			if (!bIs_I2S_enabled)
			{
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
			HAL_AUDIO_EnableI2s(FALSE, i2s_op_mode, TRUE);
#else		
			HAL_AUDIO_EnableI2s(FALSE, i2s_op_mode); 
#endif
			}
			bIs_I2S_enabled = TRUE;
		  break;
#endif //I2S_NONE_FM_DIRECT_PATH
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
#if defined(SS_2153)
			AUDIO_DRV_SetAudioMixerGain((UInt16)mixer_gain, (UInt16)mixer_gain);
#endif // SS_2153

			bCur_StatusMicMute = bStatusMicMute;

			HAL_AUDIO_MuteMicrophone();
			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);

			
			HAL_AUDIO_SetAudioMode( (AudioMode_t)pathCfgPtr->outputChnl );
			HAL_AUDIO_SetSpeakerOn(stCurrPath.audioID);
			HAL_AUDIO_SetSpeakerVolume(speakerVolume);
			HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);
			
			OSTASK_Sleep( 40 );
			
			AUDIO_DRV_EC_NS_OnOff(TRUE, TRUE);
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
#if defined(SS_2153)
	if (pathCfgPtr->audioID != stCurrPath.audioID)
	{
		_DBG_(KI_DPRINTF("HAL_AUDIO_DisablePath(): mismatch against current ID=%d, SpkCh=%d\r\n", stCurrPath.audioID, stCurrPath.outputChnl));

	switch(pathCfgPtr->audioID)
	{
			// triaged cases in HAL_AUDIO_DisablePath.
		case AUDIO_ID_IIS2DAC:
			case AUDIO_ID_IIS2DSP:
			case AUDIO_ID_FM_RECORD_PCM:
			case AUDIO_ID_FM_RECORD_AAC:
			case AUDIO_ID_CALL:
			case AUDIO_ID_VPU:
			case AUDIO_ID_VPU_VT_LOOPBACK:
			case AUDIO_ID_VPU_RECORD:
			case AUDIO_ID_VPU_TELEPHONY:			
			case AUDIO_ID_HQ_AUDIO_RECORD:
			case AUDIO_ID_AUDVOC:
			case AUDIO_ID_AMRWB_RECORD:
			case AUDIO_ID_PCM_READBACK: // used for pcm readback. 
			case AUDIO_ID_IIS: // external I2S device
			case AUDIO_ID_TONE:
			case AUDIO_ID_POLY:
			case AUDIO_ID_EXT_ANLG:
			case AUDIO_ID_WAVETONE_POLY:
				break;

			default: // assuming the wrong audio ID from caller of HAL_AUDIO_DisablePath, correct it from stored current path.
				_DBG_(KI_DPRINTF("HAL_AUDIO_DisablePath(): correct mismatch with current ID, SpkCh\r\n"));
				pathCfgPtr->audioID = stCurrPath.audioID;
				pathCfgPtr->outputChnl = stCurrPath.outputChnl;
				break;
		}
	}
#endif // SS_2153

	switch(pathCfgPtr->audioID)
			{
		case AUDIO_ID_IIS2DAC:
			//if if i2s_handle == NULL we have no path enabled,so no point in disabling it.
			if(i2s_handle)
			{	
				HAL_AUDIO_DisableI2s(); 
				bIs_I2S_enabled = FALSE;
				AUDIO_DRV_SetIIS2Audio(FALSE); //i2s path to DAC.
		  		HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
		  		HAL_AUDIO_DisableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			}
			break;
#ifdef	I2S_NONE_FM_DIRECT_PATH		
		case AUDIO_ID_IIS2DSP:
			// Don't change the order of sequence
			HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
			// Disable Fm radio I2S device software FIFO
			HAL_AUDIO_DisableExtDev_FmI2s_swFifo(&gAudioHalAccess.ExtDev_FmI2s);
			// enable Fm radio I2S device
			HAL_AUDIO_DisableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			// disable the virtual channel to DSP for playback
			HAL_AUDIO_DisableVirtualChannel_PlayFmI2s(&gAudioHalAccess.VirtualChannel_PlayFmI2s);
			break;

		case AUDIO_ID_FM_RECORD_PCM:
			// Don't change the order of sequence
			// Disable Fm radio I2S device software FIFO
			HAL_AUDIO_DisableExtDev_FmI2s_swFifo(&gAudioHalAccess.ExtDev_FmI2s);
			// disable Fm radio I2S device
			HAL_AUDIO_DisableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			// disable the virtual channels to record FM as PCM
			HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_PCM(&gAudioHalAccess.VirtualChannel_RecordFmI2s_PCM);
			break;

		case AUDIO_ID_FM_RECORD_AAC:
			// Don't change the order of sequence
			// Disable Fm radio I2S device software FIFO
			HAL_AUDIO_DisableExtDev_FmI2s_swFifo(&gAudioHalAccess.ExtDev_FmI2s);
			// disable Fm radio I2S device
			HAL_AUDIO_DisableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
			// disable the virtual channels to record FM as AAC, to DSP and from DSP. Must disable DSP_Out first. 
			HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_AAC_DSP_Out(&gAudioHalAccess.VirtualChannel_RecordFmI2s_AAC_DSP_Out);
			HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_AAC_DSP_In(&gAudioHalAccess.VirtualChannel_RecordFmI2s_AAC_DSP_In);
			break;
#endif //I2S_NONE_FM_DIRECT_PATH			
#ifndef FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL:
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );

			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
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
#else // FEATURE_AUDIO_VOICECALL16K
		case AUDIO_ID_CALL8K:
		case AUDIO_ID_CALL16K:
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );

			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			audio_isVoiceOn = FALSE;
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
#endif // FEATURE_AUDIO_VOICECALL16K
				
		case AUDIO_ID_VPU:
	case AUDIO_ID_VPU_VT_LOOPBACK:
			switch(stCurrVPMCodecCfg.tMode)
			{
				case AUDIO_MODE_PLAYBACK:
					HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
					break;
					
				case AUDIO_MODE_RECORD:
#if defined(ROXY_AUD_CTRL_FEATURES)
					audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0); // 03082009 michael
#endif
					HAL_AUDIO_SetMicrophoneOff(pathCfgPtr->audioID);
					break;
						
#if CHIPVERSION >= CHIP_VERSION(BCM2132,32) /* BCM2132C2 (with patch) and later */
				case AUDIO_MODE_TELEPHONY:					
					AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
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
#endif
			}
			break;
			
		case AUDIO_ID_VPU_RECORD:
#if defined(ROXY_AUD_CTRL_FEATURES)
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0); // 03082009 michael
#endif
			HAL_AUDIO_SetMicrophoneOff(pathCfgPtr->audioID);
			break;
			
		case AUDIO_ID_VPU_TELEPHONY:
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );
			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
			HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
			HAL_AUDIO_SetMicrophoneOff(pathCfgPtr->audioID);
			break;

		case AUDIO_ID_HQ_AUDIO_RECORD:
#ifdef HAL_AUDIO_HQ_PATH_ENABLED
			// audio codec path, including DSP
			HAL_Audio_DisablePath_ADC_HQ(&gAudioHalAccess.AdcHqAudioPath);
#endif // #ifdef HAL_AUDIO_HQ_PATH_ENABLED
			break;

		case AUDIO_ID_AUDVOC:
				HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
				break;

		case AUDIO_ID_AMRWB_RECORD:
#if defined(ROXY_AUD_CTRL_FEATURES)
			audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0); // 03082009 michael
#endif

			HAL_AUDIO_SetMicrophoneOff(pathCfgPtr->audioID);
			break;

		case AUDIO_ID_PCM_READBACK: // used for pcm readback. 
			AUDIO_DRV_SetPcmReadBackPathOff();
			break;

#ifdef I2S_NONE_FM_DIRECT_PATH
		case AUDIO_ID_IIS: // external I2S device
			HAL_AUDIO_DisableI2s(); 
			bIs_I2S_enabled = FALSE;
			break;
#endif // #ifdef I2S_NONE_FM_DIRECT_PATH

		case AUDIO_ID_TONE:
		case AUDIO_ID_POLY:
		case AUDIO_ID_EXT_ANLG:
		case AUDIO_ID_WAVETONE_POLY:
			HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
			break;
			
		case AUDIO_ID_VoIP_LOOPBACK:
		case AUDIO_ID_VoIP_TELEPHONY:
			AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
			AUDIO_DRV_Set_DL_OnOff( FALSE );

			AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
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

		default:
			_DBG_(KI_DPRINTF("HAL_AUDIO_DisablePath(): wrong path ID=%d\r\n", pathCfgPtr->audioID));
			HAL_AUDIO_SetSpeakerOff(pathCfgPtr->audioID);
			break;
	}
	OSTASK_Sleep(1);
	_DBG_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));
}

#ifdef BCM2153_FAMILY_BB_CHIP_BOND
Boolean HAL_AUDIO_IsRatechange(HAL_AUDIO_Param_PathCfg_t *pathCfgPtr)
{
	Boolean status = FALSE;
	if(stCurrPath.audioID== AUDIO_ID_CALL)
	{
		
	}

	stCurrPath.audioID= pathCfgPtr->audioID;
	stCurrPath.outputChnl= pathCfgPtr->outputChnl;
	return status;
}
#endif //  BCM2153_FAMILY_BB_CHIP_BOND

static void HAL_AUDIO_SwitchPath( HAL_AUDIO_Param_PathCfg_t *pathCfgPtr )
{
	//KI_DLOGV4("HAL_AUDIO_SwitchPath()  ID, SpkCh, Vol", stCurrPath.audioID, stCurrPath.outputChnl, stCurrPath.volumeLevel, 0);
	_DBG_(KI_DPRINTF("HAL_AUDIO_SwitchPath()  ID=%d, SpkCh=%d\r\n", pathCfgPtr->audioID, pathCfgPtr->outputChnl));

	// an ugly thing for FM radio play. We should discard this function and its API
	if (pathCfgPtr->audioID == AUDIO_ID_IIS2DSP)
	{
		HAL_AUDIO_SetSpeakerVolume(pathCfgPtr->volumeLevel);
	}
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && defined (__AUDIO_AMP_MAX9877__)&& !defined(SS_2153)	
    else if (pathCfgPtr->audioID == AUDIO_ID_USER_EXT_AMP)
    	{
	    	// reconfigure the ext amp to broadcast or other modes on the fly. 
	    	// for broadcast case, set outputChnl = AUDIO_CHNL_RESERVE
	    	// !!! make sure path was enabled first with correct ID, channel and vol etc !!!
	    	// !!! this swtich path only touches/reconfigures user ext amp                   !!!
		AUDIO_DRV_SetExtAMPOn(AUDIO_DRV_ConvertChnlMode(pathCfgPtr->outputChnl));
    	}
#endif	
	else	
	{	
		// 2 actions happen in switch path and need to adress them separately for upper layer
		if ((stCurrPath.audioID != pathCfgPtr->audioID) || (stCurrPath.outputChnl != pathCfgPtr->outputChnl))
			HAL_AUDIO_EnablePath(pathCfgPtr);
		else
			HAL_AUDIO_SetSpeakerVolume(pathCfgPtr->volumeLevel);
	}
}

#ifdef FEATURE_AUDIO_VOICECALL16K
//******************************************************************************
//
// Function Name: HAL_AUDIO_RateChange
//
// Description:	change sampling rate for voice call.
//
// Notes: , .... This API should be called in a call.
//
//******************************************************************************
static void HAL_AUDIO_RateChange( HAL_AUDIO_Param_RateCfg_t *rateCfg )
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_RateChange(): audioID=%d\r\n", rateCfg->audioID));
	if( (rateCfg->audioID!=AUDIO_ID_CALL8K) && (rateCfg->audioID!=AUDIO_ID_CALL16K) )
		return;
	stCurrPath.audioID = rateCfg->audioID;

	audio_isCall16K = (stCurrPath.audioID == AUDIO_ID_CALL16K)? TRUE : FALSE;
	if(stCurrPath.outputChnl == AUDIO_CHNL_BLUETOOTH)
		audio_isCall16K = FALSE;

#if defined(FUSE_APPS_PROCESSOR)		
	audio_control_dsp(MUTE_DSP_DL, 0, 0, 0);	
#else
	VOLUMECTRL_MuteRx();
#endif

	HAL_AUDIO_MuteMicrophone();
	AUDIO_DRV_EC_NS_OnOff(FALSE, FALSE);
	audio_control_dsp(TURN_UL_COMPANDER_OFF, 0, 0, 0);
	//AUDIO_DRV_ConnectMicrophoneUplink( FALSE );
	AUDIO_DRV_Set_DL_OnOff( FALSE );
	audio_control_dsp(ENABLE_DSP_AUDIO, 0, audio_isCall16K, 0);
	HAL_AUDIO_SetAudioMode( AUDIO_DRV_ConvertVoiceIDChnlMode(stCurrPath.audioID, stCurrPath.outputChnl)); // keep current path configuration

	AUDIO_DRV_Set_DL_OnOff(AUDIO_ON);
	HAL_AUDIO_SetMicrophoneOn(stCurrPath.audioID);

	OSTASK_Sleep( 40 );

	AUDIO_DRV_EC_NS_OnOff(TRUE, TRUE);
	audio_control_dsp(TURN_UL_COMPANDER_ON, 0, 0, 0);

  
}
#endif // FEATURE_AUDIO_VOICECALL16K

// Some tune parameters need to be set through HAL. For example, external amplifier.
static void HAL_AUDIO_SetTuneParam (HAL_AUDIO_Param_Tune_t *tune)
{
	switch (tune->paramType)
	{
		case HAL_AUDIO_TUNE_PARAM_EXTAMP_PGA:
			AudioExtDev_SetVolume(tune->paramValue);
			break;
		
		case HAL_AUDIO_TUNE_PARAM_EXTAMP_PREPGA:
			break;
		
		default:
			break;
	}
}

//VPU driver should not be part of audvoc. 2152 has it.
static void HAL_AUDIO_ConfigCodecParam(HAL_AUDIO_Param_CodecCfg_t *codecCfgPtr)
{
	Int16 equ_profile;
	AudioClientID_en_t  		halAudioID = codecCfgPtr->audioID;
#ifdef	I2S_NONE_FM_DIRECT_PATH
#if defined(_BCM2153_) || defined(_BCM213x1_) || defined (_BCM21551_)
	AUDVOC_CFG_AMR_NB_INFO amr_cfg;
#endif
#endif //#ifdef	I2S_NONE_FM_DIRECT_PATH
	_DBG_(KI_DPRINTF("HAL_AUDIO_ConfigCodecParam, audioID= %d\r\n",halAudioID));

	switch(halAudioID)
	{
#ifdef	I2S_NONE_FM_DIRECT_PATH	
		case AUDIO_ID_IIS: // output via I2S		
#ifdef I2S_INCLUDED
			if(codecCfgPtr->param1!=0)
			{
				I2S_SetBufSize(codecCfgPtr->param1);
			}

#if defined(ROXY_AUD_CTRL_FEATURES)			
			// set external device's sampling rate
			AudioExtDev_SetSampleRate( codecCfgPtr->sampleRate );
#endif

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
#endif //#ifdef	I2S_NONE_FM_DIRECT_PATH
		case AUDIO_ID_IIS2DAC:
//			HAL_AUDIO_EnableExtDev_FmI2s(&gAudioHalAccess.ExtDev_FmI2s);
//			equ_profile = (Int16)audio_control_generic(READ_AUDVOC_AEQMODE, 0, 0, 0);
//			program_coeff(equ_profile);
			AUDIO_DRV_SetAudioSampleRate(codecCfgPtr->sampleRate);
			break;
#ifdef	I2S_NONE_FM_DIRECT_PATH
		case AUDIO_ID_IIS2DSP:
			equ_profile = (Int16)audio_control_generic(READ_AUDVOC_AEQMODE, 0, 0, 0);
			program_coeff(equ_profile);
			AUDIO_DRV_SetAudioSampleRate(codecCfgPtr->sampleRate);
			break;
#endif //#ifdef I2S_NONE_FM_DIRECT_PATH
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
	mixer_gain = (UInt16)AUDIO_GetParmAccessPtr()[stCurrPath.outputChnl].audvoc_mixergain;
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
	else if (equ->mixerInputPath == AUDVOC_PATH_POLYR)
	{
		AUDIO_DRV_SetPolyEquGain(equ->gain);
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

		case AUDVOC_PATH_POLYR:
			program_poly_equalizer( eq_profile->equalizerID );
		break;
	}
	audio_control_generic(WRITE_AUDVOC_AEQMODE, (UInt32)eq_profile->equalizerID, 0, 0);
}

static void HAL_AUDIO_SetMixer( HAL_AUDIO_Param_Mixer_t *mixer )
{
#if defined(_BCM2153_) || defined(_BCM213x1_) || defined (_BCM21551_)
	UInt8  sources = 0;
	UInt16 default_mixer_setting;
	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetMixer(): audio=%d pr=%d voice=%d\r\n",mixer->musicSelect, mixer->polyringerSelect, mixer->voiceSelect));
	if (mixer->musicSelect == FALSE) //audio source
	{
		audvoc_write_register(AUDVOC_ALSLOPGAIN_REG, BIT15_MASK); //mute audio, set ALSLOPGAIN=0
		audvoc_write_register(AUDVOC_ARSLOPGAIN_REG, BIT15_MASK); //mute audio, set ARSLOPGAIN=0
	}
	else
	{	
		sources += 1;
	}
	
	if (mixer->polyringerSelect == FALSE) //pr source
	{
		audvoc_write_register(AUDVOC_PLSLOPGAIN_REG, BIT15_MASK); //mute pr, set PLSLOPGAIN=0
		audvoc_write_register(AUDVOC_PRSLOPGAIN_REG, BIT15_MASK); //mute pr, set PRSLOPGAIN=0	
	}
	else
	{	
		sources += 1;
	}	
	
	if (mixer->voiceSelect == FALSE) //voice source
	{	
		//audvoc_write_register(AUDVOC_VSLOPGAIN_REG, BIT15_MASK); //mute voice, set VSLOPGAIN=0; 
    }
	else
	{	
		sources += 1;
	}
#if 0 // mixer should be set in HAL_Audio separately, not in here.
	if (sources == 0)  //no source
	{
		audvoc_write_register(AUDVOC_APVLMXGAIN_REG, 0);
		audvoc_write_register(AUDVOC_APVRMXGAIN_REG, 0);	
	}
	else  //balance the APVL/RMXGAIN based on sources and right/left levels
	{	
		default_mixer_setting = 0x0400>>(sources-1);
		audvoc_write_register(AUDVOC_APVLMXGAIN_REG, \
			(UInt16)(default_mixer_setting*(100+mixer->leftLevel-mixer->rightLevel)/100));
		audvoc_write_register(AUDVOC_APVRMXGAIN_REG, \
			(UInt16)(default_mixer_setting*(100+mixer->rightLevel-mixer->leftLevel)/100));
	}
#endif
#endif //defined(_BCM2153_) || defined(_BCM213x1_) || defined (_BCM21551_)
}

#if defined(SS_2153)
static void HAL_AUDIO_SetCustomGain( HAL_AUDIO_Param_CustomGain_t * custom_gain)
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetCustomGain(): spk_pga=%d, mic_pga=%d, mixer_gain=%d\r\n", custom_gain->spk_pga, custom_gain->mic_pga, custom_gain->mixer_gain ));

	AUDIO_DRV_SetCustomGain(custom_gain->spk_pga, custom_gain->mic_pga, custom_gain->mixer_gain);
}
#endif // SS_2153
static void HAL_AUDIO_Set3D_Widening( HAL_AUDIO_Param_STW_t *threeD )
{
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
#if 0 //not sure if needed
	//AUDIO_DRV_SetAudioMixerGain( (UInt16)SYSPARM_GetAudioParmAccessPtr()->audio_parm[audioMode].audvoc_mixergain, 
		//(UInt16)SYSPARM_GetAudioParmAccessPtr()->audio_parm[audioMode].audvoc_mixergain );

	//the following logic exists to accomodate current DSP.
	if( audioMode == AUDIO_MODE_TTY )
	{	//switch to TTY mode
		AUDIO_DRV_SetInternalCTM( TRUE );
	}
	else
	{	if( currAudioMode == AUDIO_MODE_TTY )
			//switch away from TTY mode.
			AUDIO_DRV_SetInternalCTM( FALSE );
	}
#endif
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
			audioParam = AUDIO_DRV_GetMICGainIndex();
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
#ifdef  I2S_NONE_FM_DIRECT_PATH			
            audioParam = I2S_GetPosition();
#endif //I2S_NONE_FM_DIRECT_PATH
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

    _DBG_(sprintf(debug_buf, "HAL_AUDIO_GetParam, Param= %d, Ptr = 0x%x\r\n", audioParam, getParamPtr->paramPtr));
    _DBG_(KI_DPRINTF( debug_buf ) );

    return RESULT_OK;
}


#ifdef I2S_NONE_FM_DIRECT_PATH
static void audvoc_write_register(UInt16 addr, UInt16 value)
{
	post_msg(COMMAND_WRITE, addr, value, 0);
}
#pragma arm section zidata="uncacheable"
static UInt16 i2s_buffers[NUM_I2S_BLOCKS*I2S_DMA_BLOCK]; 
#pragma arm section

static int buf_wr_idx=0, buf_rd_idx=0; 
static I2S_HANDLE i2s_handle; 

#endif //I2S_NONE_FM_DIRECT_PATH


//
// FM I2S radio related.
//
//
//	Should be in a new file hal_audio_fm.c. and in AP build.
//
// Enable FM I2S device
void HAL_AUDIO_EnableExtDev_FmI2s(Audio_Ext_Device_s *ExtDev_FmI2s)
{

	// track the number of users 
	ExtDev_FmI2s->userCount++;

	if (ExtDev_FmI2s->state == AUDIO_PATH_DEVICE_ON)
		return;

#if defined (BCM2153_FAMILY_BB_CHIP_BOND)
	if(i2s_dma == FALSE)
		return;
#endif
	if (!i2sPcmLoopback) {
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
	  i2s_handle=I2SDRV_Init(0);
#else
	  i2s_handle=i2s_init(0); 
#endif
	}

	// store state
	ExtDev_FmI2s->state = AUDIO_PATH_DEVICE_ON;
}

// Disable FM I2S device
void HAL_AUDIO_DisableExtDev_FmI2s(Audio_Ext_Device_s *ExtDev_FmI2s)
{
	if (ExtDev_FmI2s->state == AUDIO_PATH_DEVICE_OFF)
		return;

	// track the number of users 
	ExtDev_FmI2s->userCount--;
	
	// still have other users need it. don't disable.
	if (ExtDev_FmI2s->userCount > 0)
		return;

	// FIXME. 
	// No i2s_drv.c API avaliable........

	// store state
	ExtDev_FmI2s->state = AUDIO_PATH_DEVICE_OFF;
}
#ifdef	I2S_NONE_FM_DIRECT_PATH
int 
  HAL_AUDIO_setI2sPcmLoopback(Boolean isLoopback, int sample_rate, int monoStereo)
  {
    i2sPcmLoopback=isLoopback;
   
    if (isLoopback) {
      currAACEnc_sampleRate=sample_rate; 
      currAACEnc_monoStereo=monoStereo; 
      return 0; 
    } else {
      currAACEnc_sampleRate=AUDIO_SAMPLERATE_48000;
      currAACEnc_monoStereo = TYPE_STEREO; 
      return 0; 
    }
  }
  
 int 
  HAL_AUDIO_setI2sHandle(I2S_HANDLE handle)
  {
    i2s_handle = handle; 
    return 0; 
  }

#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
static void HAL_AUDIO_EnableRxDMA(void)
{
	if (i2s_dma==FALSE) 
		I2SDRV_Start_rx(i2s_handle); 
}

static void HAL_AUDIO_DisableRxDMA(void)
{
	if (i2s_dma==FALSE) 
		{
		I2SDRV_Stop_rx(i2s_handle); 
		I2SDRV_Start_rx_noDMA(i2s_handle);
		}
}
#endif

#endif //#ifdef I2S_NONE_FM_DIRECT_PATH    

#if defined(BCM2153_FAMILY_BB_CHIP_BOND) 
static int HAL_AUDIO_EnableI2s(Boolean loop, Boolean mode, Boolean dma)
#else
static int HAL_AUDIO_EnableI2s(Boolean loop, Boolean mode) 
#endif
{
#if defined(BCM2153_FAMILY_BB_CHIP_BOND) 
	i2s_dma = dma;
#endif

	if (i2sPcmLoopback) {
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
	I2SDRV_Set_mode(i2s_handle, mode); 
	I2SDRV_Set_rx_format(i2s_handle, currAACEnc_sampleRate, 
		(currAACEnc_monoStereo==TYPE_STEREO), I2S_RX_RIGHT_CHAN); 
	I2SDRV_Set_tx_format(i2s_handle, currAACEnc_sampleRate, 
		(currAACEnc_monoStereo==TYPE_STEREO)); 
	I2SDRV_Enable_Loopback(i2s_handle,loop);
	I2SDRV_Start_rx(i2s_handle); 
	I2SDRV_Start_tx(i2s_handle);
#else
	i2s_set_mode(i2s_handle, mode); 
	i2s_set_rx_format(i2s_handle, currAACEnc_sampleRate, 
		(currAACEnc_monoStereo==TYPE_STEREO), I2S_RX_RIGHT_CHAN); 
	i2s_set_tx_format(i2s_handle, currAACEnc_sampleRate, 
		(currAACEnc_monoStereo==TYPE_STEREO)); 
	I2sSetLoopback(loop);
	i2s_start_rx(i2s_handle); 
	i2s_start_tx(i2s_handle); 
#endif
	} else {
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
    I2SDRV_Set_mode(i2s_handle, mode); 
     // i2s_set_rx_format(i2s_handle, I2S_SAMPLERATE_48000HZ, TRUE, I2S_RX_LEFT_CHAN); 
     if (stCurrPath.audioID == AUDIO_ID_FM_RECORD_PCM_44_1K)
	    I2SDRV_Set_rx_format(i2s_handle, I2S_SAMPLERATE_44100HZ, TRUE, I2S_RX_LEFT_CHAN); 
     else
    I2SDRV_Set_rx_format(i2s_handle, I2S_SAMPLERATE_48000HZ, TRUE, I2S_RX_LEFT_CHAN); 
	if (i2s_dma) 
		I2SDRV_Start_rx(i2s_handle); 
	else
		I2SDRV_Start_rx_noDMA(i2s_handle);
#else
	i2s_set_mode(i2s_handle, mode); 
     // i2s_set_rx_format(i2s_handle, I2S_SAMPLERATE_48000HZ, TRUE, I2S_RX_LEFT_CHAN); 
     if (stCurrPath.audioID == AUDIO_ID_FM_RECORD_PCM_44_1K)
	 	  i2s_set_rx_format(i2s_handle, I2S_SAMPLERATE_44100HZ, TRUE, I2S_RX_LEFT_CHAN); 
     else
	i2s_set_rx_format(i2s_handle, I2S_SAMPLERATE_48000HZ, TRUE, I2S_RX_LEFT_CHAN); 
	i2s_start_rx(i2s_handle); 
#endif
	}
   return 0; 
}
 
static int 
HAL_AUDIO_DisableI2s()
{ 
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
	if(i2s_dma)
		I2SDRV_Stop_rx(i2s_handle); 
	else
		I2SDRV_Stop_rx_noDMA(i2s_handle);

	if(i2sPcmLoopback)I2SDRV_Stop_tx(i2s_handle); 
	I2SDRV_Cleanup(i2s_handle); 
#else
	i2s_stop_rx(i2s_handle); 
	if(i2sPcmLoopback)i2s_stop_tx(i2s_handle, FALSE); 
	i2s_cleanup(i2s_handle); 
#endif
	i2s_handle=NULL; 

//	HAL_AUDIO_setI2sPcmLoopback(FALSE, 0, 0); 
	return 0; 
} 


#ifdef I2S_NONE_FM_DIRECT_PATH

Boolean fm_i2s_rx_callback(Boolean in)
{
  UInt16 *data; 
  UInt32 size;

#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
  I2SDRV_Queue_rx_buf(i2s_handle, i2s_buffers+buf_wr_idx*I2S_DMA_BLOCK, I2S_DMA_BLOCK);
#else
  i2s_queue_rx_buf(i2s_handle, i2s_buffers+buf_wr_idx*I2S_DMA_BLOCK, I2S_DMA_BLOCK);
#endif
  buf_wr_idx = (buf_wr_idx+1)%NUM_I2S_BLOCKS; 

  data = i2s_buffers+buf_rd_idx*I2S_DMA_BLOCK; 
  buf_rd_idx = (buf_rd_idx+1)%NUM_I2S_BLOCKS; 

  size = I2S_DMA_BLOCK * 2; // bytes
 
#ifdef HAL_AUDIO_ROUTER_ENABLED
  HAL_Audio_Router_FmI2S_Source_Callback(data, size); 
#else
  HAL_AUDIO_VirtualChannel_PlayFmI2s(data, size, 1); 
#endif
 
  return TRUE; 
}


// Enable FM I2S device's software Fifo
void HAL_AUDIO_EnableExtDev_FmI2s_swFifo(Audio_Ext_Device_s *ExtDev_FmI2s)
{
  int i;  
	// track the number of users 
	ExtDev_FmI2s->userCountSwFifo++;

	if (ExtDev_FmI2s->swFifoState == AUDIO_PATH_DEVICE_ON)
		return;

	memset(i2s_buffers, 0, NUM_I2S_BLOCKS*I2S_DMA_BLOCK*sizeof(UInt16)); 
	
	for (i=0; i<NUM_I2S_BLOCKS-1;i++) {
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
	  I2SDRV_Queue_rx_buf(i2s_handle, i2s_buffers+i*I2S_DMA_BLOCK, I2S_DMA_BLOCK);
#else
	  i2s_queue_rx_buf(i2s_handle, i2s_buffers+i*I2S_DMA_BLOCK, I2S_DMA_BLOCK);
#endif
	}

	buf_wr_idx=i; 
	buf_rd_idx=0;
#if defined(BCM2153_FAMILY_BB_CHIP_BOND)
	I2SDRV_Register_rx_Cb(i2s_handle, fm_i2s_rx_callback); 
#else
	i2s_register_rx_Cb(i2s_handle, fm_i2s_rx_callback);
#endif

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_RegisterSource(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SOURCE_FM_I2S);
#endif

	// store state
	ExtDev_FmI2s->swFifoState = AUDIO_PATH_DEVICE_ON;
}

// Disable FM I2S device's software Fifo
void HAL_AUDIO_DisableExtDev_FmI2s_swFifo(Audio_Ext_Device_s *ExtDev_FmI2s)
{
	if (ExtDev_FmI2s->swFifoState == AUDIO_PATH_DEVICE_OFF)
		return;

	// track the number of users 
	ExtDev_FmI2s->userCountSwFifo--;
	
	// still have other users need it. don't disable.
	if (ExtDev_FmI2s->userCountSwFifo > 0)
		return;

	if(i2s_dma==FALSE)
	{
		HAL_AUDIO_DisableRxDMA();
	}
	else
	{
		HAL_AUDIO_DisableI2s(); 
		bIs_I2S_enabled = FALSE;
	}

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_UnRegisterSource(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SOURCE_FM_I2S);
#endif

	// store state
	ExtDev_FmI2s->swFifoState = AUDIO_PATH_DEVICE_OFF;
}


// enable the virtual channel to DSP for FM radio playback. TODO: take out path/volume control and put in enable_path.
void HAL_AUDIO_EnableVirtualChannel_PlayFmI2s(Virtual_Channel_Dscr_s *VirtualChannel_PlayFmI2s)
{
	UInt32 hDev;
	AUDVOC_CFG_INFO cfg;

	// track the number of users 
	VirtualChannel_PlayFmI2s->userCount++;

	if (VirtualChannel_PlayFmI2s->state == AUDIO_PATH_DEVICE_ON)
		return;
		

	// enable...
	hDev = App_audvoc.Open(DEVICE_BCM2153 | CHANNEL_1,0,0);
	
	cfg.stream_route = STREAM_TO_AUDIO_HW;

	cfg.enable      = 1;
	cfg.path        = AUDVOC_PATH_AUDIO; 
	cfg.codec_type  = PCM_CODEC;
	cfg.volume_L    = 0x80;
	cfg.volume_R    = 0x80;
	App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_ENABLE, (PBYTE)&cfg, sizeof(cfg), NULL, 0 );

	cfg.path        = AUDVOC_PATH_AUDIO;

	if (i2sPcmLoopback) {
	  if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_8000)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_8000; 
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_11030)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_11025;
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_12000)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_12000;
 	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_16000)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_16000;
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_22060)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_22050;
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_24000)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_24000;
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_32000)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_32000;
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_48000)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_48000;
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_44100)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_44100;
	  else if (currAACEnc_sampleRate==AUDIO_SAMPLERATE_96000)
 	  cfg.sample_rate = AUDVOC_SAMPLERATE_96000;
  	  else if (stCurrPath.audioID == AUDIO_ID_FM_RECORD_PCM_44_1K)
	  	cfg.sample_rate =AUDVOC_SAMPLERATE_44100;
	  else 
	    _DBG_(KI_DPRINTF("Unsupported sampling rate %d\r\n", currAACEnc_sampleRate));
	}
 	else 
 	  cfg.sample_rate =AUDVOC_SAMPLERATE_48000; 
 
 	cfg.channel = ((currAACEnc_monoStereo==TYPE_STEREO)? AUDVOC_CHANNEL_STEREO : AUDVOC_CHANNEL_MONO); 

	App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_CONFIG, (PBYTE)&cfg, sizeof(cfg), NULL, 0 );

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_RegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SINK_FM_I2S_PLAY);
#endif

	// store state
	VirtualChannel_PlayFmI2s->hDev = hDev;
	VirtualChannel_PlayFmI2s->state = AUDIO_PATH_DEVICE_ON;

	//Queue a dummy buffer to offset jitter. 
	//Use the last buffer as the dummy buffer so that real play starts from the first.

	HAL_AUDIO_VirtualChannel_PlayFmI2s(i2s_buffers+buf_wr_idx*I2S_DMA_BLOCK, I2S_DMA_BLOCK*2, 0); 
}

// Disble the virtual channel to DSP for FM radio playback. TODO: take out path/volume control and put in enable_path.
void HAL_AUDIO_DisableVirtualChannel_PlayFmI2s(Virtual_Channel_Dscr_s *VirtualChannel_PlayFmI2s)
{
	UInt32 hDev;
	
	
	if (VirtualChannel_PlayFmI2s->state == AUDIO_PATH_DEVICE_OFF)
		return;

	// track the number of users 
	VirtualChannel_PlayFmI2s->userCount--;
	
	// still have other users need it. don't disable.
	if (VirtualChannel_PlayFmI2s->userCount > 0)
		return;

	// disable...
	hDev = VirtualChannel_PlayFmI2s->hDev;

	App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_STOP, NULL, 0, NULL, 0 );

	App_audvoc.Close(hDev);

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_UnRegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SINK_FM_I2S_PLAY);
#endif

	// store state
	VirtualChannel_PlayFmI2s->state = AUDIO_PATH_DEVICE_OFF;
}

// the FM Radio I2S fifo play call back, send the fifo block to virtual channel. 
void HAL_AUDIO_VirtualChannel_PlayFmI2s(UInt16 *data, UInt32 size, int start_channel)
{
	AUDVOC_BUFFER_INFO 	block_info;
	UInt32	hDev;

	if (gAudioHalAccess.VirtualChannel_PlayFmI2s.state == AUDIO_PATH_DEVICE_OFF)
		return;

	hDev = gAudioHalAccess.VirtualChannel_PlayFmI2s.hDev;

	if ((data!= NULL))
	{
	  block_info.flag = 1;
	  block_info.length = size; //  bytes
	  block_info.buffer_type = 0;
	  block_info.buffer = (data);
	  block_info.bits_per_sample	= 16;
	  App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_ADDBUFFER, NULL, 0, (PBYTE)(&block_info), 0 );
	}

	if (start_channel)
	  App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_START, NULL, 0, NULL, 0 );  
}


// enable the virtual channel to DSP for FM radio record as PCM. TODO: code is in virtual driver now. Need to put here.
void HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_PCM(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_PCM)
{
	UInt32 hDev = 0;
	
	// track the number of users 
	VirtualChannel_RecordFmI2s_PCM->userCount++;

	if (VirtualChannel_RecordFmI2s_PCM->state == AUDIO_PATH_DEVICE_ON)
		return;

	// enable.... open virtual channel by using IOControl, return hDev.
	
	// This resgisterSink should be called by OMX. We put here because we don't want to change the current API
#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_RegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SINK_FM_I2S_RECORD_PCM);
#endif

	// store state
	VirtualChannel_RecordFmI2s_PCM->hDev = hDev;
	VirtualChannel_RecordFmI2s_PCM->state = AUDIO_PATH_DEVICE_ON;
}

// Disble the virtual channel to DSP for FM radio record as PCM. TODO: code is in virtual driver now. Need to put here..
void HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_PCM(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_PCM)
{
	UInt32 hDev;
	
	
	if (VirtualChannel_RecordFmI2s_PCM->state == AUDIO_PATH_DEVICE_OFF)
		return;

	// track the number of users 
	VirtualChannel_RecordFmI2s_PCM->userCount--;
	
	// still have other users need it. don't disable.
	if (VirtualChannel_RecordFmI2s_PCM->userCount > 0)
		return;

	// disable...
	hDev = VirtualChannel_RecordFmI2s_PCM->hDev;

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_UnRegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SINK_FM_I2S_RECORD_PCM);
#endif

	// store state
	VirtualChannel_RecordFmI2s_PCM->state = AUDIO_PATH_DEVICE_OFF;
}

// the FM Radio I2S fifo record call back, send the fifo block to virtual channel. 
// !!
// !! TODO: Without changing the current API to MMI/OMX. we have to use this 
// !! very ugly way to hook up audio router and audvoc virtual channel task and message queue. 
// !!
extern P_DEVICE_CHANNEL	p_Device_Channel_I2S_Input;
extern P_DEVICE_CHANNEL p_Device_Channel_AACENC_Input;


void HAL_AUDIO_VirtualChannel_RecordFmI2s_PCM(UInt16 *data, UInt32 size)
{
	AUDVOC_CB_INFO cb_info;
	
	if (gAudioHalAccess.VirtualChannel_RecordFmI2s_PCM.state == AUDIO_PATH_DEVICE_OFF)
		return;

	if ((data!= NULL))
	{
		cb_info.status = AUDVOC_CB_EVENT_PLAY_DONE;
		cb_info.buffer = data;
		cb_info.size   = size; // size is number of bytes. 
		
		// Only call back if the virtual channel is ready(opened).
		// only start feeding PCM when the input channel(aac encoder) has started.
		// otherwise, the PCM feeding queue can eat up all memory, due to the flaws: absenlce of
		// sync between HAL and Audvoc, and no max size protection on memory of PCM feeding queue.
		//if ( p_Device_Channel_AACENC_Input )
		{
			if (p_Device_Channel_I2S_Input != NULL)
			{
				if (p_Device_Channel_I2S_Input->audio_record_callback_func != NULL)
				{
					(*p_Device_Channel_I2S_Input->audio_record_callback_func)(&cb_info);
				}
			}
		}
	}

}


// enable the virtual channel for FM radio record as AAC: PCM into DSP. 
void HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_AAC_DSP_In(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_In)
{
	UInt32 hDev = 0;
	AUDVOC_CFG_INFO cfg;
	
	// track the number of users 
	VirtualChannel_RecordFmI2s_AAC_DSP_In->userCount++;

	if (VirtualChannel_RecordFmI2s_AAC_DSP_In->state == AUDIO_PATH_DEVICE_ON)
		return;

	// enable.... open virtual channel by using IOControl, return hDev.
	hDev = App_audvoc.Open(DEVICE_BCM2153 | CHANNEL_AAC_ENC_OUTPUT,0,0);

	cfg.path		= AUDVOC_LOAD_CODEC;
	cfg.codec_type	= AAC_ENCODEC;	
	cfg.inputSourceType = AACENC_PCMSOURCE_ARM;
	App_audvoc.IOControl(hDev,IOCTL_AUDIO_INSTALL_DOWNLOADABLE_CODEC, (PBYTE)&cfg, sizeof(cfg), NULL, 0 );
	
	App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_CONFIG, (PBYTE)&cfg, sizeof(cfg), NULL, 0 );

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_RegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SINK_FM_I2S_RECORD_AAC);
#endif

	// store state
	VirtualChannel_RecordFmI2s_AAC_DSP_In->hDev = hDev;
	VirtualChannel_RecordFmI2s_AAC_DSP_In->state = AUDIO_PATH_DEVICE_ON;
}

// Disble the virtual channel to DSP for FM radio record as AAC: PCM into DSP. 
void HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_AAC_DSP_In(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_In)
{
	UInt32 hDev;
	
	
	if (VirtualChannel_RecordFmI2s_AAC_DSP_In->state == AUDIO_PATH_DEVICE_OFF)
		return;

	// track the number of users 
	VirtualChannel_RecordFmI2s_AAC_DSP_In->userCount--;
	
	// still have other users need it. don't disable.
	if (VirtualChannel_RecordFmI2s_AAC_DSP_In->userCount > 0)
		return;

	// disable...
	hDev = VirtualChannel_RecordFmI2s_AAC_DSP_In->hDev;
	App_audvoc.Close(hDev);

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_UnRegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_FM_I2S, AUDIO_ROUTER_SINK_FM_I2S_RECORD_AAC);
#endif

	// store state
	VirtualChannel_RecordFmI2s_AAC_DSP_In->state = AUDIO_PATH_DEVICE_OFF;
}


void HAL_AUDIO_VirtualChannel_RecordFmI2s_AAC_DSP_In(UInt16 *data, UInt32 size)
{
	AUDVOC_BUFFER_INFO 	block_info;
	UInt32	hDev;

	if (gAudioHalAccess.VirtualChannel_RecordFmI2s_AAC_DSP_In.state == AUDIO_PATH_DEVICE_OFF)
		return;

	hDev = gAudioHalAccess.VirtualChannel_RecordFmI2s_AAC_DSP_In.hDev;

	// only start feeding PCM when the input channel(aac encoder) has started.
	// otherwise, the PCM feeding queue can eat up all memory, due to the flaws: absenlce of
	// sync between HAL and Audvoc, and no max size protection on memory of PCM feeding queue.
	if ( p_Device_Channel_AACENC_Input && p_Device_Channel_AACENC_Input->Channel_State)
	{
		if ((data!= NULL))
		{
			block_info.flag = 1;
			block_info.length = size; //  bytes
			block_info.buffer_type = 0;
			block_info.buffer = (data);
			block_info.bits_per_sample	= 16;
			App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_ADDBUFFER, NULL, 0, (PBYTE)(&block_info), 0 );
			App_audvoc.IOControl(hDev,IOCTL_AUDIO_OUTPUT_CHANNEL_START, NULL, 0, NULL, 0 );
		}
	}
}

// enable the virtual channel for FM radio record as AAC: AAC outof DSP. code in virtual driver now, need to put here.
void HAL_AUDIO_EnableVirtualChannel_RecordFmI2s_AAC_DSP_Out(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_Out)
{
	UInt32 hDev = 0;
	AUDVOC_CFG_INFO cfg;
	
	// track the number of users 
	VirtualChannel_RecordFmI2s_AAC_DSP_Out->userCount++;

	if (VirtualChannel_RecordFmI2s_AAC_DSP_Out->state == AUDIO_PATH_DEVICE_ON)
		return;

	// enable.... open virtual channel by using IOControl, return hDev.

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_RegisterSource(&gAudioRouterManager, AUDIO_ROUTER_ID_AAC_ENCODER, AUDIO_ROUTER_SOURCE_AAC_ENCODER);
	HAL_Audio_Router_Manager_RegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_AAC_ENCODER, AUDIO_ROUTER_SINK_AAC_ENCODER_RECORD);
#endif

	// store state
	VirtualChannel_RecordFmI2s_AAC_DSP_Out->hDev = hDev;
	VirtualChannel_RecordFmI2s_AAC_DSP_Out->state = AUDIO_PATH_DEVICE_ON;
}

// Disble the virtual channel to DSP for FM radio record as AAC: AAC outof DSP. 
void HAL_AUDIO_DisableVirtualChannel_RecordFmI2s_AAC_DSP_Out(Virtual_Channel_Dscr_s *VirtualChannel_RecordFmI2s_AAC_DSP_Out)
{
	UInt32 hDev;
	
	
	if (VirtualChannel_RecordFmI2s_AAC_DSP_Out->state == AUDIO_PATH_DEVICE_OFF)
		return;

	// track the number of users 
	VirtualChannel_RecordFmI2s_AAC_DSP_Out->userCount--;
	
	// still have other users need it. don't disable.
	if (VirtualChannel_RecordFmI2s_AAC_DSP_Out->userCount > 0)
		return;

	// disable...
	hDev = VirtualChannel_RecordFmI2s_AAC_DSP_Out->hDev;

#ifdef HAL_AUDIO_ROUTER_ENABLED
	HAL_Audio_Router_Manager_UnRegisterSource(&gAudioRouterManager, AUDIO_ROUTER_ID_AAC_ENCODER, AUDIO_ROUTER_SOURCE_AAC_ENCODER);
	HAL_Audio_Router_Manager_UnRegisterSink(&gAudioRouterManager, AUDIO_ROUTER_ID_AAC_ENCODER, AUDIO_ROUTER_SINK_AAC_ENCODER_RECORD);
#endif

	// store state
	VirtualChannel_RecordFmI2s_AAC_DSP_Out->state = AUDIO_PATH_DEVICE_OFF;
}

// the FM Radio I2S fifo record call back, send the fifo block to virtual channel of AAC outof DSP. 
void HAL_AUDIO_VirtualChannel_RecordFmI2s_AAC_DSP_Out(UInt16 *data, UInt32 size)
{
}
#endif //#ifdef I2S_NONE_FM_DIRECT_PATH

#if defined(ROXY_TONE_FEATURES)
static void HAL_AUDIO_SetDefaultVoiceVolume(void)
{
	int i = 0;
	UInt16 initSpkrVol = 0, volume_max = 0;

#if defined(FUSE_APPS_PROCESSOR)
	volume_max = APSYSPARM_GetAudioParmAccessPtr()[AUDIO_GetAudioMode()].voice_volume_max;
	initSpkrVol = APSYSPARM_GetAudioParmAccessPtr()[AUDIO_GetAudioMode()].voice_volume_init;
#else
	volume_max = SYSPARM_GetVoiceVolumeMax();
	initSpkrVol = SYSPARM_GetVoiceVolumeInit();
#endif

	// convert the value in 1st param from range of 2nd_param to range of 3rd_param:
	initSpkrVol = AUDIO_Util_Convert( initSpkrVol, volume_max, AUDIO_VOLUME_MAX );
	for( i=0; i<AUDIO_MODE_NUMBER; i++)
	{
		currSpkVol[AUDIO_ID_TONE][i] = initSpkrVol;
		currSpkVol[AUDIO_ID_CALL][i] = initSpkrVol;
	}

	AUDIO_DRV_SetVoiceVolume( initSpkrVol );  //input param in range of 0-40 logical level
	//AUDIO_DRV_SetVoiceVolume( 36 );  //input param in range of 0-40 logical level

	_DBG_(KI_DPRINTF("HAL_AUDIO_SetDefaultVoiceVolume =%d, =%d",initSpkrVol, volume_max));
	
}
#endif


//******************************************************************************
//
// Function Name: HAL_AUDIO_CtrlAudAlgorithm
//
// Description:	Control dsp audio algorithm for customized BT devices.
//
// Notes: algSwitch bit0: EC, bit1: NS, ....
//
//******************************************************************************
void HAL_AUDIO_CtrlAudAlgorithm( HAL_AUDIO_Param_AlgCfg_t *algCfg )
{
	_DBG_(KI_DPRINTF("HAL_AUDIO_CtrlAudAlgorithm(): algSwitch=%x\r\n", algCfg->algSwitch));

	AUDIO_DRV_EC_NS_OnOff(algCfg->algSwitch & BIT00_MASK, (algCfg->algSwitch & BIT01_MASK)>>BIT01);
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
	HAL_AUDIO_CB2	callback			///< (in) Callback routine
	)
{
	_DBG_(KI_DPRINTF( "HAL_AUDIO_RegisterEventCB: event = %d cb = %x", event, callback ));

	if ((event == HAL_AUDIO_DSP_TONE) && (callback)) //register cb for tone timer
	{
		tone_timer_cb2 = (HAL_AUDIO_CB2)callback;
		return RESULT_OK;
	}
	else
	{
		return RESULT_ERROR;
	}
}










#if defined(CUSTOMER_OEM)
//--------------------------------------------------------------------
// BRCM code and customer code are mixed together
// need to be moved to BRCM_OEM folder or remove the customer code later
// including following code and functions:
//  1. code for including OEM code required Header files 
//  2. void HAL_AUDIO_SetSpeakerOn( AudioClientID_en_t audioID )
//  3. void HAL_AUDIO_SetSpeakerOff(AudioClientID_en_t audioID )
//  4. static void HAL_AUDIO_MuteSpeaker()
//  5. static void HAL_AUDIO_UnmuteSpeaker()
//  6. void HAL_AUDIO_ProcessToneTimer(TimerID_t id ) 
//-------------------------------------------------------------------- 

//code for including OEM code required Header files
/*+ ekko.081106 for audio at test */
#include "audioattest.h" /* Audio_Test_CheckUsingAtCmd */
/*- ekko.081106 for audio at test */
/*+ ekko.081106 send dtmf stop ind */
#include "taskmsgs.h"
 
#if defined( MMI_OEM ) // ***FixMe***: To avoid including haud.h, haud.he, and haud.hi from APPS/Saudio/...
	#include "haud.h"
#endif // MMI_OEM

//#include "audioattest.h" /* Audio_Test_CheckUsingAtCmd */
/*- ekko.081106 send dtmf stop ind */



/*  
	param	audioID		audio user ID
*/
void HAL_AUDIO_SetSpeakerOn( AudioClientID_en_t audioID )
{
	Boolean 	bOnFlag_EXT = FALSE;
	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerOn()  ID=%d, CH=%d, Speaker=%d, ExtDev=%d\r\n ", audioID, stCurrPath.outputChnl, BB_IsSpeakerOn, EXT_IsDevOn));

#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
//#if defined(__SAUDIO_BCOM_EXCLUDED__)   /* taeuk7.park 20081113 */
        if(!Audio_Test_CheckUsingAtCmd())
        {
                AudioExtDev_PowerOff( );  /* need an amp off when dtmf tone is played during a mp3 */
                EXT_IsDevOn =FALSE;        
        }
        else
//#endif    
        {         
                if(EXT_IsDevOn)    
                {	
                        AudioExtDev_PowerOff( );
                        EXT_IsDevOn =FALSE;
                }
	} 
#endif

	// Fig out which channel is used for BB
	//need to make this configurable for project
	switch ( stCurrPath.outputChnl ) 
	{
		case	AUDIO_CHNL_HANDSET: 					// use voice handset
#if (defined(_BCM2133_) && CHIP_REVISION >= 12) || defined(_BCM2124_) || (CHIPVERSION >= CHIP_VERSION(BCM2152,10))
			SPEAKER_SetAudioChnl(AUDIO_CHNL_INTERNAL);
#endif

#if defined(ROXY_00) || defined(ROXY_01)
            bOnFlag_EXT=TRUE;
#endif
			break;
		
		case	AUDIO_CHNL_HEADPHONE:						// use steroe headphone
		case	AUDIO_CHNL_TTY: 						// use steroe headphone
		case	AUDIO_CHNL_SPEAKER: 					// use mono loud speaker:
		case	AUDIO_CHNL_HAC:
#if (defined(_BCM2133_) && CHIP_REVISION >= 12) || defined(_BCM2124_) || (CHIPVERSION >= CHIP_VERSION(BCM2152,10))
			SPEAKER_SetAudioChnl(AUDIO_CHNL_AUX);
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
#if defined(ROXY_AUD_CTRL_FEATURES)
			*(volatile UInt32 *)ANACR0_REG |= 0x00000001;
#endif
			bOnFlag_EXT = TRUE;
			break;
					
		case AUDIO_ID_TONE:
			AUDIO_DRV_SetDTMFPathOn(stCurrPath.outputChnl );
			break;
		
		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoicePathOn(currAudioMode);
			break;
		
		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:
		case AUDIO_ID_VPU_VT_LOOPBACK:
			AUDIO_DRV_SetAMRPathOn(stCurrPath.outputChnl );
			break;
			
		case AUDIO_ID_WAVETONE_POLY:
			//set the poly path to mono/stereo per spk type
 			AUDIO_DRV_SetPolyChannel(stCurrPath.outputChnl);
			//load the special FIR/IIR filter for wavetone (if necessary)
			//audio_control_generic(PROGRAM_WAVETONE_FIR_IIR_FILTER, 0, 0, 0);
#if defined(ROXY_TONE_FEATURES)
			// AUDIO_DRV_SetPolyPathOn(stCurrPath.outputChnl ); 
			AUDIO_DRV_SetPolyKeytonePathOn(stCurrPath.outputChnl );
#endif
			break;
			
		case AUDIO_ID_POLY:
			//reset to mono playback
			AUDIO_DRV_SetPolyLRChannel(LOUT_LIN, ROUT_LIN);
			//reload FIR/IIR filter for midi
			//audio_control_generic(PROGRAM_POLY_FIR_IIR_FILTER, 0, 0, 0);
			AUDIO_DRV_SetPolyPathOn(stCurrPath.outputChnl );
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
	
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
//#if defined(__SAUDIO_BCOM_EXCLUDED__)   /* taeuk7.park 20081113 */
        if(!Audio_Test_CheckUsingAtCmd())
        {
                AudioExtDev_PowerOn(&stCurrPath);
                EXT_IsDevOn = TRUE;

                if(audioID != AUDIO_ID_IIS)
                {
                        if((AUDIO_CHNL_SPEAKER == stCurrPath.outputChnl)&&( TRUE==HAL_EXT_AUDIO_NeedIisMclkForLoudSpkr ))
                        PWRMGMT_IIS_MCLK_Enable( IIS_MCLOCK_CLIENT_BITMASK_WM8960 );
                }        
        }
        else
//#endif 
        {
                if(((TRUE == bOnFlag_EXT)&&(FALSE == EXT_IsDevOn))||(EXT_IsDevOn))
                {
                        AudioExtDev_PowerOn(&stCurrPath);
                        EXT_IsDevOn = TRUE;

                        if(audioID != AUDIO_ID_IIS)
                        {
                                if((AUDIO_CHNL_SPEAKER == stCurrPath.outputChnl)&&( TRUE==HAL_EXT_AUDIO_NeedIisMclkForLoudSpkr ))
                                PWRMGMT_IIS_MCLK_Enable( IIS_MCLOCK_CLIENT_BITMASK_WM8960 );
                        }
                }
        }
#endif
}



/*	Turn the speaker off
	param	audioID		audio user ID
*/
void HAL_AUDIO_SetSpeakerOff(AudioClientID_en_t audioID )
{			
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerOff(): audioID=%d\r\n", audioID));


   // move this code from  the end to here to power down the external AMP first 
   // in order to remove the 2153 pop noise 102908
	BB_IsSpeakerOn = AUDIO_OFF;

	
	//***FixMe***:  Merge error - extraneous "if"
	//	if (AudioExtDev_OnOff()==0)
	//	{
	
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
//#if defined(__SAUDIO_BCOM_EXCLUDED__)   /* taeuk7.park 20081113 */
        if(!Audio_Test_CheckUsingAtCmd())
        {
                AudioExtDev_PowerOff( );
                EXT_IsDevOn =FALSE;        
        }
//#endif	        
        else
        {
           if (AudioExtDev_OnOff()==0)
           {
                if(EXT_IsDevOn)
                {	
                        AudioExtDev_PowerOff( );
                        EXT_IsDevOn =FALSE;
                }
  	   }
        }
#endif
	
	// reduce volume down to 0
	switch( audioID )
	{
		case AUDIO_ID_IIS:
			break;

		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoiceVolume(0);
			break;
			
		case AUDIO_ID_TONE:					
			AUDIO_DRV_SetToneVolume(eToneType, 0);
			break;

		case AUDIO_ID_VPU:					
		case AUDIO_ID_VPU_TELEPHONY:
#ifdef USE_DSP_AUDIO_AMR_VOLUME
			AUDIO_DRV_SetAMRVolume(0);
#else
			AUDIO_DRV_SetAMRPathGain(0);
#endif //#ifdef USE_DSP_AUDIO_AMR_VOLUME
			break;
			
		case AUDIO_ID_EXT_ANLG:
			break;

		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathGain(0, 0);
			break;

		case AUDIO_ID_WAVETONE_POLY:
		case AUDIO_ID_POLY:
			AUDIO_DRV_SetPolyPathGain(0, 0);
			break;

		default:
			break;
	}
	OSTASK_Sleep(10);// wait until take effect	
#if defined(ROXY_TONE_FEATURES)
	currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl]=0;
#endif
	switch( audioID )
	{	
		case AUDIO_ID_IIS:
			break;
			
		case AUDIO_ID_EXT_ANLG:
#if defined(ROXY_AUD_CTRL_FEATURES)
			*(volatile UInt32 *)ANACR0_REG |= 0x00000001;
			break;
#endif					
		case AUDIO_ID_TONE:
			AUDIO_DRV_SetDTMFPathOff(stCurrPath.outputChnl );
			break;
		
		case AUDIO_ID_CALL:		
			AUDIO_DRV_SetVoicePathOff(currAudioMode);
			break;
		
		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:
	case AUDIO_ID_VPU_VT_LOOPBACK:
			AUDIO_DRV_SetAMRPathOff(stCurrPath.outputChnl );
			break;
			
		case AUDIO_ID_WAVETONE_POLY:
#if defined(ROXY_TONE_FEATURES)
			AUDIO_DRV_SetPolyKeytonePathOff(stCurrPath.outputChnl );
			break;
#endif
		case AUDIO_ID_POLY:
			AUDIO_DRV_SetPolyPathOff(stCurrPath.outputChnl );
			break;
			
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_SetAudioPathOff(stCurrPath.outputChnl );
			break;
				
		default:
			break;
	}

#if 0 // move this code to the beginning to power down the external AMP first in order to remove the 2153 pop noise 102908
	BB_IsSpeakerOn = AUDIO_OFF;
	if(EXT_IsDevOn)
	{	
		AudioExtDev_PowerOff( );
		EXT_IsDevOn =FALSE;
	}
#endif
	
	_DBG_(KI_DPRINTF("HAL_AUDIO_SetSpeakerOff()  ID=%d, CH=%d, Speaker=%d, ExtDev=%d\r\n", audioID, stCurrPath.outputChnl, BB_IsSpeakerOn, EXT_IsDevOn));
}



static void HAL_AUDIO_MuteSpeaker()
{
	switch( stCurrPath.audioID)
	{
		case AUDIO_ID_CALL:
			AUDIO_DRV_SetVoiceVolume( 0 );  //better use a separate mute function.!!
			AUDIO_DRV_MuteAMRPathOnOff(TRUE);
			break;
									
		case AUDIO_ID_TONE:
			AUDIO_DRV_MuteDTMFPathOnOff(TRUE);
			break;

		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:	
	case AUDIO_ID_VPU_VT_LOOPBACK:

			AUDIO_DRV_MuteAMRPathOnOff(TRUE);
			break;
							
		case AUDIO_ID_WAVETONE_POLY:					
		case AUDIO_ID_POLY:
			AUDIO_DRV_MutePolyPathOnOff(TRUE);
			break;
							
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_MuteAudioPathOnOff(TRUE);
			break;
			
		default:
			break;
	}

#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)
//#if defined(__SAUDIO_BCOM_EXCLUDED__)   /* taeuk7.park 20081113 */
        if(!Audio_Test_CheckUsingAtCmd())
        {
                AudioExtDev_PowerOff();	
        }
//#endif  
        else
        {
                if(EXT_IsDevOn)	
                        AudioExtDev_SetVolume(0);  //better use a separate mute function.!!
        }
#endif

}


static void HAL_AUDIO_UnmuteSpeaker()
{
	speakerVol_t index = currSpkVol[stCurrPath.audioID][stCurrPath.outputChnl];
	
	switch( stCurrPath.audioID)
	{
		case AUDIO_ID_CALL:
			index = currSpkVol[AUDIO_ID_CALL][stCurrPath.outputChnl];
			AUDIO_DRV_SetVoiceVolume( (UInt8)index );
			AUDIO_DRV_MuteAMRPathOnOff(FALSE);
			break;
									
		case AUDIO_ID_TONE:
			AUDIO_DRV_MuteDTMFPathOnOff(FALSE);
			break;

		case AUDIO_ID_VPU:
		case AUDIO_ID_VPU_TELEPHONY:	
		case AUDIO_ID_VPU_VT_LOOPBACK:
			AUDIO_DRV_MuteAMRPathOnOff(FALSE);
			break;
							
		case AUDIO_ID_WAVETONE_POLY:					
		case AUDIO_ID_POLY:
			AUDIO_DRV_MutePolyPathOnOff(FALSE);
			break;
							
		case AUDIO_ID_IIS2DAC:
		case AUDIO_ID_AUDVOC:
		case AUDIO_ID_IIS2DSP:
			AUDIO_DRV_MuteAudioPathOnOff(FALSE);
			break;
			
		default:
			break;
	}
	
#if !defined(BCM2153_FAMILY_BB_CHIP_BOND) || defined(SS_2153)

//#if defined(__SAUDIO_BCOM_EXCLUDED__)   /* taeuk7.park 20081113 */
        if(!Audio_Test_CheckUsingAtCmd())
        {
                AudioExtDev_PowerOn(&stCurrPath);	
        }
//#endif  
        else
        {
                if(EXT_IsDevOn)	
#if defined (SS_DIGI_GAIN)
			AudioExtDev_SetVolume(AudioGain_ExtDev(index, AUDIO_EXTAMP_MAX_VOLUME));
#else // SS_DIGI_GAIN
			AudioExtDev_SetVolume(index);
#endif // SS_DIGI_GAIN
        }
#endif
	
}


// Tone timer interrupt callback.
// Since timer is HISR, this function can't block at semaphore.
void HAL_AUDIO_ProcessToneTimer(TimerID_t id ) 
{
#if defined(__SAUDIO_BCOM_EXCLUDED__) // ***FixMe***: Requires haud.h/he/hi - don't compile for now
    AudioPlayDtmfReq_t *AudioDtmf_req = NULL;
    
    if(!Audio_Test_CheckUsingAtCmd())
    { /*+ ekko.081106 send dtmf stop ind */
        InterTaskMsg_t *outMsg ;
    
        gv_haud_currentNote ++;
        
        _DBG_(KI_DPRINTF( "HAL_AUDIO_ProcessToneTimer():curNote=%d, isflextone=%d",gv_haud_currentNote, gv_haud_flextoneplaying));    

        if(gv_haud_flextoneplaying )
        {   

            outMsg = AllocInterTaskMsgFromHeap(MSG_AUDIO_DTMF_REQ, sizeof(AudioPlayDtmfReq_t));
            
            AudioDtmf_req = (AudioPlayDtmfReq_t *)outMsg->dataBuf;    
            AudioDtmf_req->IDReq = AUDIO_FlexTonePlay;
            MediaAudio_PostMsg(outMsg);        
        }
	else
        {
            OSStatus_t status;
        
            AudioMsg_t audioMsg;
            audioMsg.actionCode  = ACTION_AUD_StopPlayTone;

            status = OSQUEUE_Post( AudioMsgQ, (QMsg_t *)&audioMsg, TICKS_FOREVER );
            if(!(status == OSSTATUS_SUCCESS) )
            {
                _DBG_(KI_DPRINTF( "assert in HAL_AUDIO_ProcessToneTimer ......\r\n") );
            }	 	
            if(IsToneEvt)
                NU_Set_Events(&tone_events, TONE_PLAYING_EVENT, NU_OR);    

            if( gv_haud_TotalNote  ==   gv_haud_currentNote)
            {
                if(IsToneEvt)
                {
                    NU_Delete_Event_Group(&tone_events);
                    IsToneEvt = FALSE;
                }	
                outMsg = AllocInterTaskMsgFromHeap(MSG_HAUD_DTMF_STOP_IND, 0);
                Htplayer_PostMsg(outMsg);            
            }
       }

     /*- ekko.081106 send dtmf stop ind */   
    }
    else // at command test
    {
	OSStatus_t status;
	
	AudioMsg_t audioMsg;
	audioMsg.actionCode  = ACTION_AUD_StopPlayTone;

	status = OSQUEUE_Post( AudioMsgQ, (QMsg_t *)&audioMsg, TICKS_FOREVER );
	if(!(status == OSSTATUS_SUCCESS) )
	{
		_DBG_(KI_DPRINTF( "assert in HAL_AUDIO_ProcessToneTimer ......\r\n") );
	}
    }
#endif    
}

#endif   //end of  "#if defined(CUSTOMER_OEM)"

