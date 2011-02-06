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
*    @file   hal_audio_core.h
*    @brief  API declaration of hardware abstraction layer for Audio driver.
*   This code is OS independent and Device independent for audio device control.
****************************************************************************/


/**
 * @addtogroup AudioDrvGroup
 * @{
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <mach/reg_sys.h>
#include "audio_ipc_consts.h"
#include "shared.h"
#include "memmap.h"
#include <linux/broadcom/bcm_kril_Interface.h>
#include <linux/broadcom/bcm_fuse_sysparm.h>
#include "hal_audio_config.h"
//#include "consts.h"
#include "audio_consts.h"
#include "hal_audio.h"
#include "hal_audio_config.h"


#ifndef _HAL_AUDIO_ACCESS_H_
#define _HAL_AUDIO_ACCESS_H_

#define TRUE 1
#define FALSE 0
#define MASK_SLOPGAIN_VALUE 0x1FF

#define AUDIO_ON	TRUE
#define AUDIO_OFF	FALSE

#ifdef	USE_HAL_AUDIO_THREAD
#define OSTASK_Sleep msleep
#else
#define OSTASK_Sleep mdelay
#endif

typedef enum
{
	UNMUTE_ALL,
	MUTE_POSITIVE_END,		
	MUTE_NEGAITIVE_END,		
	MUTE_ALL		
} BB_Mute_Action_t;

typedef enum
{
	SPEAKER_400MW,	
	SPEAKER_100MW	
} BB_Spk_Output_t;


typedef enum
{
	CHANNEL_DIFF,	
	CHANNEL_3WIRE_1,
	CHANNEL_3WIRE_2,
	CHANNEL_3WIRE_3,
	CHANNEL_3WIRE_4,
	CHANNEL_MAX	
} BB_Ch_Mode_t;

typedef enum
{
	LOUT_LIN,	
	LOUT_R_L,
	LOUT_RIN,
	LOUT_NO,
	LOUT_MAX	
} BB_LPathCh_Mode_t;

typedef enum
{
	ROUT_RIN,	
	ROUT_R_L,
	ROUT_LIN,
	ROUT_NO,
	ROUT_MAX	
} BB_RPathCh_Mode_t;

typedef struct _AUDVOC_GAIN_SETTING_T_ {
        UInt16 MixerGain[AUDIO_MODE_INVALID];
        UInt16 VcfgrGain[AUDIO_MODE_INVALID];
        UInt16 PgaGain[AUDIO_MODE_INVALID];
        UInt16 MicGain[AUDIO_MODE_INVALID];
        UInt16 PathGain[AUDIO_MODE_INVALID];
} AUDVOC_GAIN_SETTING_T;



typedef UInt16 speakerVol_t;



void AUDIO_DRV_PowerUpRightSpk(void);
void AUDIO_DRV_PowerUpLeftSpk(void);
void AUDIO_DRV_PowerDownRightSpk(void);
void AUDIO_DRV_PowerDownLeftSpk(void);

void AUDIO_DRV_MuteRightSpk(BB_Mute_Action_t muteAction);
void AUDIO_DRV_MuteLeftSpk(BB_Mute_Action_t muteAction);
void AUDIO_DRV_SetRChannelMode(BB_Ch_Mode_t ChMode);
void AUDIO_DRV_SetLChannelMode(BB_Ch_Mode_t ChMode);
//void AUDIO_DRV_SetAudioLRChannel(BB_LPathCh_Mode_t LChMode, BB_RPathCh_Mode_t RChMode);
//void AUDIO_DRV_SetPolyLRChannel(BB_LPathCh_Mode_t LChMode, BB_RPathCh_Mode_t RChMode);
void AUDIO_DRV_SelectRightSpkOutput(BB_Spk_Output_t pathSpeaker);
void AUDIO_DRV_SelectLeftSpkOutput(BB_Spk_Output_t pathSpeaker);
void AUDIO_DRV_SetRightSpkPGA(UInt8 gainSpk);
void AUDIO_DRV_SetLeftSpkPGA(UInt8 gainSpk);
void AUDIO_DRV_SetMICGain(UInt8 gainMIC);

void AUDIO_DRV_PowerUpD2C(void);
void AUDIO_DRV_PowerDownD2C(void);
void AUDIO_DRV_SetSpkDriverSelect(AUDIO_DRV_Spk_Output_t eSpeakerType, AUDIO_CHANNEL_t Ch);


inline void AUDIO_BitwiseWriteDSP(UInt16 addr, UInt16 mask, UInt16 value);
inline void AUDIO_WriteDSP(UInt16 addr, UInt16 value);
void audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);
void audio_control_generic(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4);

void AUDIO_DRV_SetMicrophonePathOnOff(AudioMode_t voiceMode , Boolean on_off);

void AUDIO_DRV_ConnectMicrophoneUplink(Boolean on_off);
inline void AUDIO_DRV_SetMicrophonePath_16K(Boolean on_off);

void AUDIO_DRV_Set_DL_OnOff(Boolean on_off);
void AUDIO_Turn_EC_NS_OnOff(Boolean ec_on_off, Boolean ns_on_off);
inline void AUDIO_ASIC_SetAudioMode(AudioMode_t mode);
inline void program_FIR_IIR_filter(UInt16 mode);
inline void program_poly_FIR_IIR_filter(UInt16 mode);
extern void CAPI2_VOLUMECTRL_SetBasebandVolume(UInt32 tid, UInt8 clientID, UInt8 level, UInt8 chnl, UInt16 *audio_atten, UInt8 extid);
void AUDIO_DRV_SetVoiceVolume( UInt8 Volume);

void AUDIO_DRV_SetPCMOnOff(Boolean	on_off);
void AUDIO_DRV_SetVoicePathOn(AudioMode_t voiceMode);
void AUDIO_DRV_SetVoicePathOff(AudioMode_t voiceMode);
void AUDIO_DRV_SetAudioPathOn(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetAudioPathOff(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetAudioPathGain(UInt16 Lgain, UInt16 Rgain);
void AUDIO_DRV_SetAudioMixerGain(int Lgain, int Rgain);
void AUDIO_DRV_SetAudioEquGain( Int16 *gain );

void AUDIO_DRV_MuteAudioPathOnOff(Boolean mute_on_off);

void AUDIO_DRV_SetVoiceMode(AudioMode_t voiceMode);

void AUDIO_DRV_MicrophoneMuteOnOff(Boolean mute_on_off);

void AUDIO_DRV_SetHifiDacMask(void);
void AUDIO_DRV_FlushFifo(void);


void AUDIO_DRV_InitReg(void);

Result_t AUDIO_DRV_PlayTone(SpeakerTone_t tone_id, UInt32 duration);
Result_t AUDIO_DRV_PlayGenericTone(
	Boolean superimpose,
	UInt16	duration,
	UInt16	f1,
	UInt16	f2,
	UInt16	f3
	);
void AUDIO_DRV_StopPlayTone(void);

void AUDIO_DRV_SetDTMFPathOn(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetDTMFPathOff(AUDIO_CHANNEL_t chan);
void AUDIO_DRV_SetDTMFPathGain(UInt16 Gain);
void AUDIO_DRV_SetToneVolume(AudioToneType_en_t eToneType, speakerVol_t uSpkvol);
void AUDIO_DRV_MuteDTMFPathOnOff(Boolean mute_on_off);

#endif
/** @} */
