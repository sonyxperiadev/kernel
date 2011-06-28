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
*   @file   drv_caph_hwctrl.h
*
*   @brief  This file contains the definition for HW control driver layer
*
****************************************************************************/


#ifndef _DRV_CAPH_HWCTRL_
#define _DRV_CAPH_HWCTRL_

typedef enum {
    AUDDRV_TYPE_TELEPHONY, 
    AUDDRV_TYPE_AUDIO_OUTPUT,
    AUDDRV_TYPE_RINGTONE_OUTPUT,
    //   AUDDRV_TYPE_VOICE_OUTPUT,  //for ARM write
    //   AUDDRV_TYPE_VOICE_INPUT,   //for ARM read
    AUDDRV_TYPE_AUDIO_INPUT,  //HQ Audio
    AUDDRV_TYPE_MixerTap_VB, 
    AUDDRV_TYPE_MixerTap_WB, 
    AUDDRV_TYPE_AMRNB, 
    AUDDRV_TYPE_AMRWB, 
    AUDDRV_TYPE_ARM2SP_Inst1,
    AUDDRV_TYPE_ARM2SP_Inst2,
    AUDDRV_TYPE_USBHeadset
} AUDDRV_TYPE_Enum_t;

typedef enum {
    AUDDRV_ALL_OUTPUT,			//this can only be used when SelectMic( ) and SelectSpkr( ).
    AUDDRV_AUDIO_OUTPUT,
    AUDDRV_RINGTONE_OUTPUT,
    AUDDRV_VOICE_OUTPUT,
    AUDDRV_VOICE_INPUT,
    AUDDRV_AUDIO_INPUT,			//HQ Audio input
    AUDDRV_MIXERTap_VB_INPUT,	//Voice band mixer tap
    AUDDRV_MIXERTap_WB_INPUT,	//Wide band mixer tap
	AUDDRV_INOUT_NUM
} AUDDRV_InOut_Enum_t;

typedef AudioMode_t (*CB_GetAudioMode_t)( void );
typedef void (*CB_SetAudioMode_t) ( AudioMode_t  audio_mode);
typedef void (*CB_SetMusicMode_t) ( AudioMode_t  audio_mode);

/**
*
*  @brief  initialize the caph HW control driver
*  
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_Init(void);


/**
*
*  @brief  deinitialize the caph HW control driver
*
*  @param  void 
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DeInit(void);

/**
*
*  @brief  Enable a caph HW path
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return UInt8 pathHandle
*****************************************************************************/
AUDDRV_PathID AUDDRV_HWControl_EnablePath(AUDDRV_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Disable a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DisablePath(AUDDRV_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Pause a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_PausePath(AUDDRV_HWCTRL_CONFIG_t config);

/**
*
*  @brief  Resume a caph HW path 
*
*  @param   config  (in) Caph HW path configuration parameters
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_ResumePath(AUDDRV_HWCTRL_CONFIG_t config);


/**
*
*  @brief  Set the gain for the sink
*
*  @param   pathID  (in) path handle of HW path
*  @param   gainL_mB  (in) L-Ch Gain in Q13.2
*  @param   gainR_mB  (in) R-Ch Gain in Q13.2
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetSinkGain(AUDDRV_PathID pathID, 
                                      UInt16 gainL,
                                      UInt16 gainR);

/**
*
*  @brief  Set the gain for the source
*
*  @param   pathID  (in) path handle of HW path
*  @param   gainL_mB  (in) L-Ch Gain in Q13.2
*  @param   gainR_mB  (in) R-Ch Gain in Q13.2
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetSourceGain(AUDDRV_PathID pathID, 
                                        UInt16 gainL,
                                        UInt16 gainR);

/**
*
*  @brief  Mute the sink
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_MuteSink(AUDDRV_PathID pathID);

/**
*
*  @brief  Mute the source
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_MuteSource(AUDDRV_PathID pathID);

/**
*
*  @brief  Unmute the sink
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_UnmuteSink(AUDDRV_PathID pathID);

/**
*
*  @brief  Unmute the source
*
*  @param   pathID  (in) path handle of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_UnmuteSource(AUDDRV_PathID pathID);

/**
*
*  @brief  Switch to another sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SwitchSink(AUDDRV_DEVICE_e sink);

/**
*
*  @brief  Add a new sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_AddSink(AUDDRV_DEVICE_e sink);

/**
*
*  @brief  Remove a sink
*
*  @param  sink  (in) Sink of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_RemoveSink(AUDDRV_DEVICE_e sink);

/**
*
*  @brief  Switch to another source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SwitchSource(AUDDRV_DEVICE_e source);

/**
*
*  @brief  Add a new source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_AddSource(AUDDRV_DEVICE_e source);

/**
*
*  @brief  Remove a source
*
*  @param  source  (in) Source of HW path
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_RemoveSource(AUDDRV_DEVICE_e source);

/**
*
*  @brief  Load filter coefficients
*
*  @param  filter  (in) the filter to load coefficients
*  @param  coeff  (in) the filter coefficients to load
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, void* coeff);

/**
*
*  @brief  Enable the Sidetone path
*
*  @param  HW path ID
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_EnableSideTone(AudioMode_t audioMode);

/**
*
*  @brief  Disable the Sidetone path
*
*  @param  HW path ID
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DisableSideTone(AudioMode_t audioMode);

/**
*
*  @brief  Set the sidetone gain
*
*  @param  gain (in) the gain
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain);


/**
*
*  @brief  Enable the EANC path
*
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_EnableEANC(void);

/**
*
*  @brief  Disable the EANC path
*
*  @param  void
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_DisableEANC(void);

/**
*
*  @brief  configure fm/pcm port
*
*  @param  fm_port fm port number
*
*  @param  pcm_port pcm port number
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_ConfigSSP(UInt8 fm_port, UInt8 pcm_port);

/**
*
*  @brief  configure ssp tdm mode
*
*  @param  status tdm mode status
*
*  @return none
*****************************************************************************/
void AUDDRV_HWControl_SetSspTdmMode(Boolean status);

/**
*
*  @brief control microphone loop back to output path
*
*  @param  mic     : input microphone device 
*	 spekaer : output speaker
*	 enable_lpbk    : control to loop back 
*			   TRUE - enable loop back in path,
*			   FALSE - disbale loop back in path
*                path    : internal loopback path
*
* i @return      the call result
*
******************************************************************************/

void AUDDRV_SetAudioLoopback( 
			Boolean             enable_lpbk,
			AUDDRV_MIC_Enum_t   mic,
			AUDDRV_SPKR_Enum_t  speaker,
			int path
			);

/****************************************************************************
*
* Function Name: Result_t AUDDRV_HWControl_EnableVibrator(Boolean enable_vibrator,
*													 AUDDRV_VIBRATOR_MODE_Enum_t mode) 
* Description:   Enable/Disable vibrator with default strength
*
* Parameters:    enable_vibrator     : enable (TRUE) or disable (FALSE) operation to vibrator  
*                mode                : by_pass mode ( 0 ) or PCM playback mode (non zero)
*
* Return:       the call result
*
******************************************************************************/

void AUDDRV_HWControl_EnableVibrator(Boolean enable_vibrator, AUDDRV_VIBRATOR_MODE_Enum_t mode);


/****************************************************************************
*
* Function Name: Result_t AUDDRV_HWControl_VibratorStrength (UInt32 strength)
*
* Description:   Change the vibrator strength
*
* Parameters:    strength : strength value to vibrator  
*
* Return:       the call result
*
******************************************************************************/

void AUDDRV_HWControl_VibratorStrength(UInt32 strength);


/**
*
*  @brief  Pass the DSP shared memory address for DL to IHF
*
*  @param  address
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetDSPSharedMeMForIHF(UInt32 addr);

/**
*
*  @brief Set Mixing gain in HW Mixer.
*
*  @param   pathID  (in) path handle of HW path
*  @param  fineGainL  mixer L-ch output fine gain in register value format
*  @param  coarseGainL  mixer L-ch output coarse gain in register value format
*  @param  fineGainR  mixer R-ch output fine gain in register value format
*  @param  coarseGainR  mixer R-ch output coarse gain in register value format
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetMixOutputGain(AUDDRV_PathID pathID, 
						UInt32 fineGainL, 
 						UInt32 coarseGainL,
 						UInt32 fineGainR,
  						UInt32 coarseGainR);


/**
*
*  @brief Set Mixing gain in HW Mixer.
*
*  @param   pathID  (in) path handle of HW path
*  @param  gainL  mixer L-ch input gain in Q13.2 format
*  @param  gainL  mixer R-ch input gain in Q13.2 format
*
*  @return Result_t status
*****************************************************************************/
Result_t AUDDRV_HWControl_SetMixingGain(AUDDRV_PathID pathID, 
						UInt32 gainL, 
 						UInt32 gainR);

/**
*
*  @brief Set Hw gain. For audio tuning purpose only.
*
*  @param  hw   hw gain selection
*  @param  gain  hw gain
*  @param  dev   Output path EP/IHF/HS if available.
*
*  @return Result_t status
*****************************************************************************/
void  AUDDRV_HWControl_SetHWGain(AUDDRV_HW_GAIN_e hw, UInt32 gain, AUDDRV_DEVICE_e dev);
/********************************************************************
*  @brief  Register up callback for getting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_getAudioMode( CB_GetAudioMode_t  cb );

/********************************************************************
*  @brief  Register up callback for setting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setAudioMode( CB_SetAudioMode_t  cb );

/********************************************************************
*  @brief  Register up callback for setting music audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setMusicMode( CB_SetMusicMode_t  cb );



#endif // _DRV_CAPH_HWCTRL_

