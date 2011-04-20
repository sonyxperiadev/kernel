/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   audioapi_asic.h
*
*   @brief  This file contains the ASIC (DSP) Audio API definitions.
*
****************************************************************************/
/**
*   @defgroup   AudioASICDevCtrlGroup   Broadcom Baseband ASIC Audio Device Control API
*   @ingroup    AudioDrvGroup
*   @brief      This group defines the interfaces to control Broadcom
*               baseband ASIC audio device.
****************************************************************************/

/* Requires the following header files before its inclusion in a c file

*/
#ifndef _AUDIOAPI_ASIC_H_
#define _AUDIOAPI_ASIC_H_

#include "tones_def.h"

#define	 MAX_MICROPHONE_LEVELS 14
//#define	 MAX_MICROPHONE_LEVELS 9
//#define	 MAX_MICROPHONE_LEVELS		(AUDIO_IsTuning() ? 9 : SYSPARM_GetMaxMicGain())

//#define	 DEFAULT_MICROPHONE_LEVELS		3
#define	 DEFAULT_MICROPHONE_LEVELS		SYSPARM_GetMicGain()



//*** from speaker.h
#define DEFAULT_TONE_VOLUME 12	

#define	 MAX_SPEAKER_LEVELS			AUDIO_VOLUME_MAX //SYSPARM_GetNumSupportedVolumeLevels()

#define SPEAKER_1f_MAXVOL_SCALE		(0x7fff)
#define SPEAKER_2f_MAXVOL_SCALE		(0x7fff/2)
#define SPEAKER_3f_MAXVOL_SCALE		(0x7fff*3/4)

//******************************************************************************
// Enumerations
//******************************************************************************
//typedef enum
//{
//	AUDIO_CHNL_INTERNAL,	///< use standard microphone-speaker set.
//	AUDIO_CHNL_AUX			///< use aux micorphone-speaker set.
//} AudioChnl_t;

//******************************************************************************
// Function Prototypes
//******************************************************************************


void SPEAKER_StartTone(					///< Generate a supervisory tone
	SpeakerTone_t tone,					///< Selected tone
	UInt8 duration						///< Tone duration (MS)
										///< if duration is 0, generate tone
										///< until SPEAKER_StopTone is called
	);	 

void SPEAKER_StopTone( void );			///< Stop a supervisory tone

void SPEAKER_StartGenericTone(
	Boolean	superimpose,		// flag to enable superimposed tone
	UInt16	tone_duration,		// tone duration in ms
	UInt16	f1,					// first frequency
	UInt16	f2,					// second frequency, if f2==0 not frequency component output
	UInt16	f3					// third frequency,	 if f3==0 not frequency component output
	);


UInt16 SPEAKER_GetToneVolume(void);
void SPEAKER_SetToneVolume(UInt16 vol);
UInt8 SPEAKER_GetToneFreqs(void);

//*** end of speaker.h

/*********************************************************************************/
//
//   function declaration for AUDIO
//
/*********************************************************************************/
void AUDIO_Init(void);

/**
 * @addtogroup AudioDrvGroup
 * @{
 */
/**
*	@brief	Run audio module; initialize audio that requires L1 task and DSP.
*
*   Call this function after AUDIO_Init().
*/
void AUDIO_Run(void);

//Acquire ID in order to call audio control API which
//should be changed to accept ID as the first parameter.
//UInt8 AUDIO_GetAudioID( void );


/**
*	@brief	Mute the Microphone
*/
void AUDIO_MuteMicrophone(UInt8 id);

/**
*	@brief	unMute the Microphone
*/
void AUDIO_UnMuteMicrophone(UInt8 id);


/** @} */

/**
 * @addtogroup AudioASICDevCtrlGroup
 * @{
 */
/**
*	@brief	Set Broadcom baseband ASIC speaker volume
*
*   @return Result_t
*/
Result_t AUDIO_SetSpeakerVol(UInt8 vol);

/**
*	@brief	Get Broadcom baseband ASIC speaker volume.
*
*   @return UInt8
*/
UInt8 AUDIO_GetSpeakerVol(void);

/** @} */

/**
 * @addtogroup AudioDrvGroup
 * @{
 */
/**
*	@brief	Set Broadcom baseband ASIC microphone gain.
*/
Result_t AUDIO_SetMicrophoneGain (UInt8 vol);

/**
*	@brief	Get Broadcom baseband ASIC microphone gain.
*
*   @return UInt8    0~9: actual gain=0dB~27dB (3dB steps)
*/
UInt8 AUDIO_GetMicrophoneGain (void);

/**
*	@brief	Get the microphone gain setting no matter if it is on or off.
*
*   @return UInt8    0~9: actual gain=0dB~27dB (3dB steps)
	This one is obsolete, will be removed.
*/
UInt8 AUDIO_GetMicrophoneGainSetting (void);

/** @} */

/**
 * @addtogroup AudioASICDevCtrlGroup
 * @{
 */
/**
*	@brief	Return the current analog sidetone gain.
*
*   @return UInt8
*/
UInt8 AUDIO_GetSideToneGain (void);

/**
*	@brief	Set analog sidetone gain.
*
*	@param	gain		analog sidetone gain
*   @return Result_t
*/
Result_t AUDIO_SetSideToneGain (UInt8 gain);

/**
*	@brief	Get Digital sidetone gain.
*
*	@param	gain		Digital sidetone gain
*   @return Result_t
*/
UInt16 AUDIO_GetDigitalSideToneGain (void);

/**
*	@brief	Set Digital sidetone gain.
*
*	@param	gain		Digital sidetone gain
*   @return Result_t
*/
Result_t AUDIO_SetDigitalSideToneGain (UInt8 gain);


Result_t AUDIO_PlayTone(AUDIO_CHANNEL_t ch, SpeakerTone_t tone, UInt32 duration);

/**
*	@brief	Set DAC Filter Scale Factor.
*
*	@param	superimpose		superimpose
*	@param	duration		tone duration, Duration in ms
*	@param	f1				frequency
*	@param	f2				frequency
*	@param	f3				frequency
*   @return Result_t
    This function currently is not used in broadcom SW.
*/
Result_t AUDIO_PlayGenericTone(
		Boolean	superimpose,
		UInt16	duration,
		UInt16	f1,
		UInt16	f2,
		UInt16	f3
		);

void AUDIO_StopPlaytone(void);

/**
*	@brief	Convert the tone_id to speaker tone id provided by the DSP.
*
*	@param	tone_id		tone ID
*   @return SpeakerTone_t
*   Notes:			GSM11.14 sec 12.16
*/
SpeakerTone_t AUDIO_GetSpeakerToneFromToneId(UInt8 tone_id);

/**
*	@brief	turn on/off PCM audio interface
*
*	@param	on_off		TRUE:  turn on
*			            FALSE: turn off
*/
void	AUDIO_SetPCMOnOff(Boolean on_off);

/**
*	@brief	Set uplink voice filter at runtime
*
*	@param	voice_adc		pointer to ADC filter table
*	This function is called automatically when switch audio channel.
*   Application SW do not need to call this function.
*/
void	AUDIO_SetVoiceFilterADC(UInt16* voice_adc);

/**
*	@brief	Set downlink voice filter at runtime
*
*	@param	voice_dac		pointer to DAC filter table
*	This function is called automatically when switch audio channel.
*   Application SW do not need to call this function.
*/
void	AUDIO_SetVoiceFilterDAC(UInt16* voice_dac);


/* Audio Tuning prototypes */

/**
*	@brief	Start the audio tuning
*
*   @return Result_t
*/
Result_t AUDIO_StartTuning(void);

/**
*	@brief	Stop the audio tuning
*
*	@param	save		the SYSPARM to store the paramters to flash (or to disgard it)
*   @return Result_t
*/
Result_t AUDIO_StopTuning(Boolean save);

/**
*	@brief	return whether the phone is in tuning mode or not.
*
*   @return boolean
*/
Boolean AUDIO_IsTuning(void);

void		AUDIO_ASIC_SetAudioMode(AudioMode_t audioMode);

/**
*	@brief	return current audio mode.
*
*   @return Result_t
*/
AudioMode_t	AUDIO_GetAudioMode(void);

void		AUDIO_PreSwitchModeSetting(void);
void		AUDIO_PostSwitchModeSetting(void);


/**
*	@brief	Enable internal CTM for TTY mode.
*/
void AUDIO_SetInternalCTM( Boolean enable );

/**
*	@brief	Set one audio parameter during audio tuning.
*
*   @return Result_t
*/
Result_t AUDIO_SetAudioParam(AudioParam_t audioParamType, void* param);

/**
*	@brief	Get one audio parameter during audio tuning.
*/
void* AUDIO_GetAudioParam(AudioParam_t audioParam);

// These pseudo-digital gains are the analog gain values used to convert
// to digital gains. 
UInt8	AUDIO_GetPseudoDigitalRXGain(void);

// These gains are the digital gains used by echo.c only, they are not APIs
UInt16	AUDIO_GetDigitalRXGain(void);
UInt16	AUDIO_GetDigitalTXGain(void);


Boolean		AUDIO_IsECEnabled(void);
Boolean		AUDIO_IsNSEnabled(void);
Boolean		AUDIO_IsECOn(void);
Boolean 	AUDIO_IsDualMicOn(void);
Boolean 	AUDIO_IsDualMicEnabled(void);


void		AUDIO_EnableEC(Boolean true_false);
void		AUDIO_EnableNS(Boolean true_false);
void		AUDIO_EnableDualMic(Boolean true_false);
void 		AUDIO_TurnDualMicOnOff(Boolean on_off);
void		AUDIO_TurnECOnOff(Boolean on_off);
void		AUDIO_TurnNSOnOff(Boolean on_off);
void 		AUDIO_Turn_EC_NS_OnOff(Boolean ec_on_off, Boolean ns_on_off);


UInt16	AUDIO_Util_Convert( UInt16 input, UInt16 scale_in, UInt16 scale_out);

UInt16	AUDIO_GetEQMode(void);
void AUDIO_SetEQMode(UInt16 eq);

 // set gain on the VPU playback path on the downlink path
void AUDIO_SetAMRVolume_DL(UInt16 uSpkvol);
 // set gain on the VPU playback path on the uplink path
void AUDIO_SetAMRVolume_UL(UInt16 uSpkvol);

UInt8 AUDIO_GetToneVolume(void);

void AUDIO_SetToneVolume(UInt8 volume);

#if !defined(_ATHENA_)
#if (defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)&& !defined (FUSE_COMMS_PROCESSOR))
/*****************************************************************************************/
/**
* 
* Function Name: AUDIO_Return_IHF_48kHz_buffer_base_address
*
*   @note     This function returns the base address to the shared memory buffer where
*             the ping-pong 48kHz data would be stored for AADMAC to pick them up
*             for IHF case. This base address needs to be programmed to the 
*             AADMAC_CTRL1 register.
*                                                                                         
*   @return   ptr (UInt32 *) Pointer to the base address of shared memory ping-pong buffer 
*                            for transferring 48kHz speech data from DSP to AADMAC for IHF.
*
**/
/*******************************************************************************************/
UInt32 *AUDIO_Return_IHF_48kHz_buffer_base_address(void);
#endif
#endif

#endif //_AUDIOAPI_ASIC_H_

/** @} */

