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
//
//   ABSTRACT: This file contains AUDVOC driver implementation
//
//   TARGET:   BCM213x1,BCM2153,BCM21551
//
//   TOOLSET:
//
//   HISTORY:
//  01/16/08       	FRANK SHOU            	Initial version
//****************************************************************************
//!
//! \file   hal_audio_access.c 
//! \brief  API wrapper
//! \brief  
//!
//****************************************************************************
#include "hal_audio_access.h"
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
#include "audio_drv.h"
#endif 
#include <linux/broadcom/bcm_fuse_sysparm.h>
#include "consts.h"

void post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2);
// Use it to access all the Audio HAL components.
Audio_HAL_Access_s	gAudioHalAccess;
extern void CAPI2_SPEAKER_StopTone(UInt32 tid, UInt8 clientID);
extern void CAPI2_SPEAKER_StartGenericTone(UInt32 tid, UInt8 clientID, Boolean superimpose, UInt16 tone_duration, UInt16 f1, UInt16 f2, UInt16 f3);
extern void CAPI2_SPEAKER_StartTone(UInt32 tid, UInt8 clientID, SpeakerTone_t tone,	UInt8 duration);  


//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************
// local macro declarations
//****************************************

#include "audiohal_bits.h"

#if defined(ROXY_AUD_CTRL_FEATURES)
#define HEADSET_AUDIO_3WIRE
#else
#undef HEADSET_AUDIO_3WIRE
#endif

#define	HAL_ACCESS_SLEEP
#ifdef HAL_ACCESS_SLEEP
#undef DISABLE_AUDIO_HAL_SLEEP
#else
#define DISABLE_AUDIO_HAL_SLEEP
#endif

#ifdef DISABLE_AUDIO_HAL_SLEEP
#define AudHal_Sleep(a) 
#else
#define AudHal_Sleep(a) OSTASK_Sleep(a)
#endif 

//#define ENABLE_DEBUG
#ifdef  ENABLE_DEBUG
#define _DBG_A_(a)		a
#define AUD_TRACE_DETAIL printk
#define	audioPrintf	printk
#else
#define _DBG_A_(a)
#endif

#define ANACR0_CTRL ((volatile ANACR0_struct *)&REG_SYS_ANACR0)
#define ANACR1_CTRL ((volatile ANACR1_struct *)&REG_SYS_ANACR1)
#ifdef SS_2153
#define ANACR2_CTRL ((volatile ANACR2_struct *)&REG_SYS_ANACR2)
#endif // SS_2153
#define ANACR3_CTRL ((volatile ANACR3_struct *)&REG_SYS_ANACR3)

#define MASK_SLOPGAIN_VALUE		0x1FF
#define MAX_SPK_GAIN_STEPS 		7
#ifndef NUM_OF_EQU_BANDS
#define NUM_OF_EQU_BANDS 5
#endif

#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && defined (__AUDIO_AMP_MAX9877__) && !defined(SS_2153)
#define AUDIO_EXTAMP_ON_DELAY		10
#define AUDIO_EXTAMP_OFF_DELAY		5
#define EXTDEV_SPKVOL_MUTE		MAX9877_SPKVOL_MUTE
#endif

#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && defined (__AUDIO_AMP_NCP2704__) && !defined(SS_2153)
#define AUDIO_EXTAMP_ON_DELAY		10
#define AUDIO_EXTAMP_OFF_DELAY		5 
#define EXTDEV_SPKVOL_MUTE		NCP2704_SPKVOL_MUTE
#endif

//****************************************
// local typedef declarations
//****************************************
typedef union {
	UInt32				REG_ANACR0;	
	struct {
#if  defined(__LITTLE_ENDIAN)
				UInt32	pwrdnd2c			: 1;
				UInt32	pwrdndacL			: 1;
				UInt32	pwrdndacR			: 1;
				UInt32	pwrdndrvL			: 1;
				UInt32	pwrdndrvR			: 1;
				UInt32	pwrdnref			: 1;
				UInt32	bgtcsp				: 3;
				UInt32	iq_dbl				: 1;
				UInt32	cm_sel				: 3;
				UInt32	drvr_selL			: 1;
				UInt32	drvr_selR			: 1;
				UInt32	mode_selL			: 3;
				UInt32	mode_selR			: 3;
				UInt32	mute_selL			: 2;
				UInt32	mute_selR			: 2;
				UInt32	pga_ctrlL			: 3;
				UInt32	pga_ctrlR			: 3;
				UInt32	clkinv				: 1;
#else
				UInt32	clkinv				: 1;
				UInt32	pga_ctrlR			: 3;
				UInt32	pga_ctrlL			: 3;
				UInt32	mute_selR			: 2;
				UInt32	mute_selL			: 2;
				UInt32	mode_selR			: 3;
				UInt32	mode_selL			: 3;
				UInt32	drvr_selR			: 1;
				UInt32	drvr_selL			: 1;
				UInt32	cm_sel				: 3;
				UInt32	iq_dbl				: 1;
				UInt32	bgtcsp				: 3;
				UInt32	pwrdnref			: 1;
				UInt32	pwrdndrvR			: 1;
				UInt32	pwrdndrvL			: 1;
				UInt32	pwrdndacR			: 1;
				UInt32	pwrdndacL			: 1;
				UInt32	pwrdnd2c			: 1;
#endif		
	} FLD_ANACR0;	
} AUDIO_ANACR0_un_t;

typedef struct {
  	AUDIO_ANACR0_un_t HAL_ANACR0;
}ANACR0_struct;

typedef union {
	UInt32				REG_ANACR3;	
	struct {
#if  defined(__LITTLE_ENDIAN)
				UInt32						: 9;
				UInt32	pga_gain			: 6;
				UInt32	pga_mux_sel 		: 1;
				UInt32						: 16;
#else
				UInt32						: 16;
				UInt32	pga_mux_sel 		: 1;
				UInt32	pga_gain			: 6;
				UInt32						: 9;
#endif		
	} FLD_ANACR3;	
} AUDIO_ANACR3_un_t;

typedef struct{
  	AUDIO_ANACR3_un_t HAL_ANACR3;
}ANACR3_struct;

typedef union {
	UInt32				REG_ANACR1;	
	struct {
#if  defined(__LITTLE_ENDIAN)
				UInt32						: 7;
				UInt32	hifidac_pwd_mask	: 1;
				UInt32						: 18;
				UInt32	rampref_en			: 1;
				UInt32	offsetdbl			: 1;
				UInt32	pwrmos_gatecntrlLR  : 2;
				UInt32	pdoffsetgenLR		: 2;
#else
				UInt32	pdoffsetgenLR		: 2;
				UInt32	pwrmos_gatecntrlLR  : 2;
				UInt32	offsetdbl			: 1;
				UInt32	rampref_en			: 1;
				UInt32						: 18;
				UInt32  hifidac_pwd_mask    : 1;
				UInt32                      : 7;
#endif		
	} FLD_ANACR1;	
} AUDIO_ANACR1_un_t;

typedef struct{
	AUDIO_ANACR1_un_t HAL_ANACR1;
}ANACR1_struct;

#ifdef SS_2153
typedef union {
	UInt32				REG_ANACR2;
	struct {
#if  defined(__LITTLE_ENDIAN)
				UInt32	ldo_pwrdn 			: 1;
				UInt32	mic_voice_pwrdn		: 1;
				UInt32						: 30;
#else
				UInt32						: 30;
				UInt32	mic_voice_pwrdn		: 1;
				UInt32	ldo_pwrdn 			: 1;
#endif		
	} FLD_ANACR2;
} AUDIO_ANACR2_un_t;

typedef struct{
  	AUDIO_ANACR2_un_t HAL_ANACR2;
}ANACR2_struct;
#endif // SS_2153

//****************************************
// local variable definitions
//****************************************
static Boolean bPowerDownR =TRUE;
static Boolean bPowerDownL =TRUE;

static AUDIO_DRV_MIC_Path_t MICConfig[AUDIO_MODE_NUMBER_VOICE]={
										HANDSET_MIC, 
										HEADSET_MIC,
										HANDSFREE_MIC,
										BLUETOOTH_MIC,
										SPEAKER_MIC,
										TTY_MIC,
										HAC_MIC,
										USB_MIC,
										NONE_MIC,
										HANDSET_MIC, 
										HEADSET_MIC,
										HANDSFREE_MIC,
										BLUETOOTH_MIC,
										SPEAKER_MIC,
										TTY_MIC,
										HAC_MIC,
										USB_MIC,
										NONE_MIC
										};

static AUDIO_DRV_Spk_Output_t SpeakerConfig[AUDIO_MODE_NUMBER_VOICE]={
							HANDSET_OUTPUT, 
							HEADSET_OUTPUT,
							HANDSFREE_OUTPUT,
							BLUETOOTH_OUTPUT,
							SPEAKER_OUTPUT,
							TTY_OUTPUT,
							HAC_OUTPUT,
							USB_OUTPUT,
							NONE_OUTPUT,
							HANDSET_OUTPUT, 
							HEADSET_OUTPUT,
							HANDSFREE_OUTPUT,
							BLUETOOTH_OUTPUT,
							SPEAKER_OUTPUT,
							TTY_OUTPUT,
							HAC_OUTPUT,
							USB_OUTPUT,
							NONE_OUTPUT
							};

// 3 playback that share the voice path
static Boolean bStatusCall = AUDIO_OFF;
static Boolean bStatusAMR = AUDIO_OFF;
static Boolean bStatusDTMF = AUDIO_OFF;

#if defined(ROXY_AUD_CTRL_FEATURES)
static Boolean bStatusEXIT_DEV = AUDIO_OFF;
#endif

//tracking pcm readback which depends on dsp voice but independ on analog hw
static Boolean bStatusPcmBack = AUDIO_OFF;

// 3 path that share the PGA and DAC
static Boolean bStatusAudio = AUDIO_OFF;
static Boolean bStatusVoice = AUDIO_OFF;
static Boolean bStatusPoly = AUDIO_OFF;

//tracking record status to make sure d2c control is more robust
static Boolean bStatusInputPath = FALSE;

//tracking MicCh status: true->use mic pga; false->use dsp ul gain, for bt etc
static Boolean bStatusMicChUsed = FALSE;
static UInt16 currentMicGain = 0;
	
#define VOICE_PATH_STATUS 		bStatusCall||bStatusAMR||bStatusDTMF
#define MIXER_INPUT_STATUS 	bStatusAudio||bStatusVoice||bStatusPoly

static Boolean bStatusSTAudio = TRUE; // TRUE = ST, FALSE = MN
static Boolean bStatusSTPoly = FALSE; // TRUE = ST, FALSE = MN

#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
static Boolean audio_extamp_on = FALSE;
#endif

//Volume lookup tables to convert to digital dB
//1)map the mmi voice volume to dsp digital gain in dB. 
//2)the max db value was defined in sysparm.
#ifdef USE_VOLUME_CONTROL_IN_DB //customoer MMI
static UInt8 uVol2BB[41]={0,            //volume 0
				   1, 2, 3, 4, 5,       //volume 1-5
				   6, 7, 8, 9, 10,      //volume 6-10
				   11,12,13,14,15,      //volume 11-15
				   16,17,18,19,20,      //volume 16-20
				   21,22,23,24,25,	    //volume 21-25
				   26,27,28,29,30,      //volume 26-30
				   31,32,33,34,35,      //volume 31-35
				   36, 0, 0, 0, 0};	    //volume 36-40
#else //platform MMI
//static UInt8 uVol2BB[41]={0,        //volume 0
//				   3,0,0,0,3,       //volume 1-5
//				   0,0,0,0,6,       //volume 6-10
//				   0,0,0,0,9,       //volume 11-15
//				   0,0,0,0,12,      //volume 16-20
//				   0,0,0,0,15,	    //volume 21-25
//				   0,0,0,0,18,      //volume 26-30
//				   0,0,0,0,21,      //volume 31-35
//				   0,0,0,0,24};	    //volume 36-40

static UInt8 uVol2BB[41]={0,        //volume 0
				   1,1,2,3,3,       //volume 1-5
				   3,4,5,6,6,       //volume 6-10
				   6,7,8,9,9,       //volume 11-15
				   9,10,11,11,12,    //volume 16-20
				   12,13,14,15,15,  //volume 21-25
				   15,16,17,18,18,  //volume 26-30
				   18,19,20,21,21,  //volume 31-35
				   21,22,23,24,24};	//volume 36-40				   
#endif //#ifdef USE_VOLUME_CONTROL_IN_DB

//map the digital gain to mmi level for OV. Need to adjust based on mmi level.
static UInt8 uVol2OV[37]={0, 
				   0, 0, 1, 
				   0, 1, 2, 
				   0, 0, 2, 
				   0, 0, 4, 
				   0, 0, 3, 
				   0, 0, 6, 
				   4, 0, 7, 
				   0, 0, 5, 
				   0, 0, 0, 
				   0, 0, 0, 
				   0, 0, 0, 
				   0, 0, 0};

#if defined(SS_2153)
//moved to hal_audio_access.h
//#define REF_SYSPARM	0xFFFF
//typedef enum {
//	CUSTOM_SPK_PGA,
//	CUSTOM_MIC_PGA,
//	CUSTOM_MIXER_GAIN,
//	CUSTOM_GAIN_TOTAL
//} CUSTOM_GAIN_t;

static UInt16 custom_gain_set[CUSTOM_GAIN_TOTAL] = {REF_SYSPARM, REF_SYSPARM, REF_SYSPARM};
static UInt16 AUDIO_DRV_SelectCustomGain(CUSTOM_GAIN_t mode, UInt16 sysparm_val);
#endif // SS_2153

void AUDIO_DRV_SetAuxMicBiasOnOff(Boolean Bias_on_off, AsicAuxMicBiasMode_t bias_mode);
void AUDIO_DRV_SetMicBiasOnOff(Boolean Bias_on_off);


//#define POWEROFF_CONTROL
#ifdef POWEROFF_CONTROL
#define TimeToPowerOFF 2000 //5000ms
static Timer_t timerForPowerOff;
static Queue_t  bb_audio_pwrctrl_q;

static AUDIO_DRV_Spk_Output_t poweroff_spk_output;
static Task_t	audioPowerOffControl_task;
static void audioPowerOffControl_TaskEntry(void);
void AUDIO_DRV_SetSpkDriverOff_Execute(AUDIO_DRV_Spk_Output_t eSpeakerType);

static void audio_PowerOffControl_action( void )
{
	_DBG_A_(audioPrintf("audio_PowerOffControl_action( )"));

	if(AUDIO_ON == (bStatusPoly||bStatusAudio||bStatusVoice))
		return;
	AUDIO_DRV_SetSpkDriverOff_Execute(poweroff_spk_output);

}

static void audioPowerOffControl_TaskEntry(void)
{
	UInt8 bb_audio_pwrctrl_off;

	while( TRUE )
	{
		OSQUEUE_Pend( bb_audio_pwrctrl_q, (QMsg_t *)&bb_audio_pwrctrl_off, TICKS_FOREVER );
		if (bb_audio_pwrctrl_off)
			audio_PowerOffControl_action();
	}
}

static void audio_PowerOffControl_timerCB(void)
{
	UInt8 bb_audio_pwrctrl_off = TRUE;
	OSStatus_t status;

	status = OSQUEUE_Post( bb_audio_pwrctrl_q, (QMsg_t *)&bb_audio_pwrctrl_off, TICKS_FOREVER );
	if(!(status == OSSTATUS_SUCCESS) )
	{
		_DBG_A_(audioPrintf("bb_audio_pwrctrl_q post fail!!"));
	}
}

void AUDIO_DRV_SetSpkDriverOff(AUDIO_DRV_Spk_Output_t eSpeakerType)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverOff() eSpeakerType =%d\r\n", eSpeakerType));
	
	poweroff_spk_output = eSpeakerType;
	
	if ( timerForPowerOff ==NULL)
	{
		timerForPowerOff = OSTIMER_Create( (TimerEntry_t)audio_PowerOffControl_timerCB, 
								(TimerID_t)0, TimeToPowerOFF, 0 );
	}

	OSTIMER_Reset( timerForPowerOff );
	OSTIMER_Start( timerForPowerOff );
	return;
}

#endif

//****************************************
// local function declarations
//****************************************

//****************************************
// local function definitions
//****************************************

#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)

int	GetAudioExtOutput(AudioMode_t ChMode)
{

	switch(ChMode)
	{

#if defined (__AUDIO_AMP_NCP2704__)
      case AUDIO_MODE_SPEAKERPHONE:
			return NCP2704_OUTPUT_LOUDSPK;
		case AUDIO_MODE_HEADSET:
			return NCP2704_OUTPUT_HEADSET;
		case AUDIO_MODE_HANDSET:
		default:
			return NCP2704_OUTPUT_HANDSET;
#else
		case AUDIO_MODE_SPEAKERPHONE:
			return MAX9877_OUTPUT_LOUDSPK;
		case AUDIO_MODE_HEADSET:
			return MAX9877_OUTPUT_HEADSET;
		case AUDIO_MODE_RESERVE:
			return MAX9877_OUTPUT_HEADSET_AND_LOUDSPK;
		case AUDIO_MODE_HANDSET:
		default:
			return MAX9877_OUTPUT_HANDSET;
#endif
		
	}
}

#else
int    GetAudioExtOutput(AudioMode_t ChMode)
{

       switch(ChMode)
       {
               case AUDIO_MODE_SPEAKERPHONE:
                       return MAX9877_OUTPUT_LOUDSPK;
               case AUDIO_MODE_HEADSET:
                       return MAX9877_OUTPUT_HEADSET;
               case AUDIO_MODE_HANDSET:
               default:
                       return MAX9877_OUTPUT_HANDSET;
       }
}
#endif


#if defined(FUSE_APPS_PROCESSOR)
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmAccessPtr
//!
//! Description:	audio parm acces for FUSE AP only
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return SYSPARM_GetAudioParmAccessPtr(0);
#endif
}
#ifdef BCM2153_FAMILY_BB_CHIP_BOND
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmMultimediaAccessPtr
//!
//! Description:	multimedia audio parm acces for FUSE AP only
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
SysIndMultimediaAudioParm_t * AUDIO_GetParmMultimediaAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetMultimediaAudioParmAccessPtr();
#endif
}
#endif //  BCM2153_FAMILY_BB_CHIP_BOND
#else //#if defined(FUSE_APPS_PROCESSOR)
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmAccessPtr
//!
//! Description:	audio parm acces
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
SysCalDataInd_t* AUDIO_GetParmAccessPtr(void)
	{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return SYSPARM_GetAudioParmAccessPtr();
#endif
}
#endif //#if defined(FUSE_APPS_PROCESSOR)

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSP shared mem and register access
//!
//! Description:	Write value to DSP addr register
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_PostMsgToDSP(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2){
		post_msg(cmd, arg0, arg1, arg2); }

/* used for fuse platform to put msg into dsp fast cmd Q */
void AUDIO_PostFastMsgToDSP(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2){
		post_msg(cmd+COMMAND_IPC_FAST_CMD_OFFSET, arg0, arg1, arg2); }		

void AUDIO_BitwiseWriteDSP(UInt16 addr, UInt16 mask, UInt16 value){
  		post_msg(COMMAND_BITWISE_WRITE, addr, mask, value);}
  
void AUDIO_WriteDSP(UInt16 addr, UInt16 value){
  		post_msg(COMMAND_WRITE, addr, value, 0);}

#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__))&& !defined(SS_2153)
void AUDIO_DRV_SetExtAMPOn(AudioMode_t ChMode)
{
	Boolean spk_enable = FALSE;
	Int16 pre_pga = 0;
	Int16 spk_pga = 0;
	

#if defined(FUSE_APPS_PROCESSOR)
	spk_enable = (Boolean)AUDIO_GetParmAccessPtr()[ChMode].ext_speaker_enable;
	pre_pga = (Int16)AUDIO_GetParmAccessPtr()[ChMode].ext_speaker_preamp_pga;
	spk_pga = (Int16)AUDIO_GetParmAccessPtr()[ChMode].ext_speaker_pga;
#else
	spk_enable = (Boolean)AUDIO_GetParmAccessPtr()->audio_parm[ChMode].ext_speaker_enable;
	pre_pga = (Int16)AUDIO_GetParmAccessPtr()->audio_parm[ChMode].ext_speaker_preamp_pga;
	spk_pga = (Int16)AUDIO_GetParmAccessPtr()->audio_parm[ChMode].ext_speaker_pga;
#endif

	_DBG_A_(AUD_TRACE_DETAIL("[audio AUDIO_DRV_SetExtAMPOn]ChMode=%d spk_enable=%d spk_enable=%d spk_pga=%d audio_extamp_on=%d\r\n", 
		ChMode, spk_enable, spk_enable,spk_pga,audio_extamp_on));

	if(pre_pga<0)
		pre_pga = 0;
	if(spk_pga<0)
		spk_pga = 0;

	if(audio_extamp_on)
	{
		AudioExtDev_SetVolume(EXTDEV_SPKVOL_MUTE);
	}

#if defined (__AUDIO_AMP_NCP2704__)
	AudioExtDev_SetPreGain((AudioExtPreGainNCP2704_t)pre_pga);
#else
   AudioExtDev_SetPreGain((AudioExtPreGainMAX9877_t)pre_pga);
#endif

	if(!spk_enable)
	{
		AudioExtDev_SetVolume(EXTDEV_SPKVOL_MUTE);
		AudioExtDev_PowerOff();
		audio_extamp_on = FALSE;
		return;
	}

	OSTASK_Sleep(AUDIO_EXTAMP_ON_DELAY);
	AudioExtDev_PowerOn(GetAudioExtOutput(ChMode));
	AudioExtDev_SetVolume((UInt8)spk_pga);
	audio_extamp_on = TRUE;
	
}

void AUDIO_DRV_SetExtAMPOff(AudioMode_t ChMode)
{

	AudioExtDev_SetVolume(EXTDEV_SPKVOL_MUTE);
	AudioExtDev_PowerOff();
	audio_extamp_on = FALSE;
	OSTASK_Sleep(AUDIO_EXTAMP_OFF_DELAY);
}

void AUDIO_DRV_MuteExtAMP(void)
{
	AudioExtDev_SetVolume(EXTDEV_SPKVOL_MUTE);
}

void AUDIO_DRV_InitExtAMP(void)
{
	AudioExtDev_Init();
	AudioExtDev_PowerOff();
}

#endif

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SelectMIC
//!
//! Description:	select the MIC input
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SelectMIC(AUDIO_DRV_MIC_Path_t pathMIC)
{
        AUDIO_ANACR3_un_t TempReg; 				 // get a local copy of register structure

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));
	
	TempReg.REG_ANACR3 = ANACR3_CTRL->HAL_ANACR3.REG_ANACR3;
	TempReg.FLD_ANACR3.pga_mux_sel = pathMIC;
	ANACR3_CTRL->HAL_ANACR3.REG_ANACR3 = TempReg.REG_ANACR3;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));

}





/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetMICGain
//!
//! Description:	set the MIC gain in dB (0 ~ 42dB) or index (0 ~ 14)
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
#define USE_INDEX_AS_INPUT
void AUDIO_DRV_SetMICGain(UInt8 gainMIC)
{
	UInt16 iTemp;
	AUDIO_ANACR3_un_t TempReg; 				 // get a local copy of register structure

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));

#if defined(USE_INDEX_AS_INPUT)
        if (gainMIC>14)	gainMIC=14;
        iTemp = gainMIC*3;
#else
        if (gainMIC>42)	gainMIC=42;
        iTemp = gainMIC;
#endif

#ifdef USE_MIC_PGA_PROTECTION
        if (iTemp > 18) iTemp = 18;  //limit the gain to 18dB
#endif

	TempReg.REG_ANACR3 = ANACR3_CTRL->HAL_ANACR3.REG_ANACR3;
	TempReg.FLD_ANACR3.pga_gain = iTemp;
	ANACR3_CTRL->HAL_ANACR3.REG_ANACR3 = TempReg.REG_ANACR3;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));

}
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetMICGainIndex
//!
//! Description:	set the MIC gain
//!
//! Notes:  iput param is gain index with 3dB gain change.
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetMICGainIndex(UInt8 mic_gain_index)
{
	UInt32 iTemp = 0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));

#if (defined(_BCM213x1_) || defined(_BCM2153_) || defined(_BCM21551_) )
#define ANACR3_INPGA_GAIN_BITMASK  (0x00007E00)
#define INPGA_GAIN_LEVEL_STEPSIZE 3

#ifdef USE_MIC_PGA_PROTECTION
	if (mic_gain_index > 6) mic_gain_index = 6; //cap it to 18dB (change this number accordingly with step size)
#else
	if( mic_gain_index > 14) 
		mic_gain_index = 14; //cap it to 42dB.
#endif

	iTemp = ( ( (mic_gain_index * INPGA_GAIN_LEVEL_STEPSIZE) << 9) & ANACR3_INPGA_GAIN_BITMASK);
	*(volatile UInt32 *)ANACR3_REG &= ~(ANACR3_INPGA_GAIN_BITMASK);  //clear
	*(volatile UInt32 *)ANACR3_REG |= iTemp;  //set
#endif
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_GetMICGainIndex
//!
//! Description:	get the MIC gain index
//!
//! Notes:  return logic index with 3dB step
//! 
/////////////////////////////////////////////////////////////////////////////
UInt8 AUDIO_DRV_GetMICGainIndex(void)
{
	AUDIO_ANACR3_un_t TempReg;
	TempReg.REG_ANACR3 = ANACR3_CTRL->HAL_ANACR3.REG_ANACR3;
	return TempReg.FLD_ANACR3.pga_gain/3;	
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_PowerUpRightSpk
//!
//! Description:	power up right speaker
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_PowerUpRightSpk(void)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdnref = 0;
	TempReg.FLD_ANACR0.pwrdndrvR = 0;
	TempReg.FLD_ANACR0.pwrdndacR = 0;
	TempReg.FLD_ANACR0.pwrdnd2c = 0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	bPowerDownR = FALSE;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_PowerUpLeftSpk
//!
//! Description:	power up left speaker
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_PowerUpLeftSpk(void)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdnref = 0;
	TempReg.FLD_ANACR0.pwrdndrvL = 0;
	TempReg.FLD_ANACR0.pwrdndacL = 0;
	TempReg.FLD_ANACR0.pwrdnd2c = 0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	bPowerDownL = FALSE;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
	
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_PowerDownRightSpk
//!
//! Description:	power up right speaker
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_PowerDownRightSpk(void)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	bPowerDownR = TRUE;
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvR = 1;
	TempReg.FLD_ANACR0.pwrdndacR = 1;
	if(bPowerDownL)
	{
		TempReg.FLD_ANACR0.pwrdnref = 1;
		//TempReg.FLD_ANACR0.pwrdnd2c = 1;
	}
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_PowerDownLeftSpk
//!
//! Description:	power up left speaker
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_PowerDownLeftSpk(void)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
	
	bPowerDownL = TRUE;
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvL = 1;
	TempReg.FLD_ANACR0.pwrdndacL = 1;
	if(bPowerDownR)
	{
		TempReg.FLD_ANACR0.pwrdnref = 1;
		//TempReg.FLD_ANACR0.pwrdnd2c = 1;
	}
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_MuteRightSpk
//!
//! Description:	mute right speaker output
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_MuteRightSpk(BB_Mute_Action_t muteAction)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.mute_selR = muteAction;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_MuteLeftSpk
//!
//! Description:	mute left speaker output
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_MuteLeftSpk(BB_Mute_Action_t muteAction)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.mute_selL = muteAction;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetRChannelMode
//!
//! Description:	set channel mode on right channel
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetRChannelMode(BB_Ch_Mode_t ChMode)
{
        AUDIO_ANACR0_un_t TempReg;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	if(ChMode>=CHANNEL_MAX)
		return;
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.mode_selR = ChMode;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetLChannelMode
//!
//! Description:	set channel mode on left channel
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetLChannelMode(BB_Ch_Mode_t ChMode)
{
        AUDIO_ANACR0_un_t TempReg;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	if(ChMode>=CHANNEL_MAX)
		return;
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.mode_selL = ChMode;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioLRChannel
//!
//! Description:	set channel mode on audio path
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetAudioLRChannel(BB_LPathCh_Mode_t LChMode, BB_RPathCh_Mode_t RChMode)
{
        if (LChMode > LOUT_NO)
                LChMode = LOUT_NO;
        if (RChMode > ROUT_NO)
                RChMode = ROUT_NO;
        AUDIO_WriteDSP(DSP_AUDIO_REG_ALRCH, (LChMode<<2)|RChMode);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPolyLRChannel
//!
//! Description:	set channel mode on poly path
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetPolyLRChannel(BB_LPathCh_Mode_t LChMode, BB_RPathCh_Mode_t RChMode)
{
	if(LChMode > LOUT_NO)
		LChMode = LOUT_NO;
	if(RChMode > ROUT_NO)
		RChMode = ROUT_NO;
	AUDIO_WriteDSP(DSP_AUDIO_REG_PLRCH, (LChMode<<2)|RChMode);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SelectRightSpkOutput
//!
//! Description:	select right speaker output (100mW or 400mW speaker)
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SelectRightSpkOutput(BB_Spk_Output_t pathSpeaker)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.drvr_selR = pathSpeaker;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));	
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SelectLeftSpkOutput
//!
//! Description:	select left speaker output (100mW or 400mW speaker)
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SelectLeftSpkOutput(BB_Spk_Output_t pathSpeaker)
{
	AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.drvr_selL = pathSpeaker;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetRightSpkPGA
//!
//! Description:	set right speaker PGA value, use (0 ~ 7) as input in oder to back compitable
//! 				with 2133/2152 chip
//!
//! Notes:
//! input param:
//! for 2133, 2152, index lookup table is below:
//! 0
//! 1
//! 2
//! ...
//! 9
//!
//! for 213x1, 2153, index lookup table is below:
//! 0    -21 dB
//! 1    -18 dB
//! 2    -15 dB
//! ...
//! 6     -3 dB
//! 7     0 dB
//!
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetRightSpkPGA(UInt8 gainSpk)
{
        AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure

#if defined(SS_2153)
	gainSpk = (UInt8)AUDIO_DRV_SelectCustomGain(CUSTOM_SPK_PGA, (UInt16) gainSpk); // 02062009 michael
#endif // SS_2153
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pga_ctrlR = MAX_SPK_GAIN_STEPS -gainSpk;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	_DBG_A_(audioPrintf("AUDIO_DRV_SetRightSpkPGA() gainSpk=%d\r\n", gainSpk));
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetLeftSpkPGA
//!
//! Description:	set left speaker PGA value, use (0 ~ 7) as input in oder to back compitable
//! 				with 2133/2152 chip
//!
//! Notes:
//! input param:
//! for 2133, 2152, index lookup table is below:
//! 0
//! 1
//! 2
//! ...
//! 9
//!
//! for 213x1, 2153, index lookup table is below:
//! 0    -21 dB
//! 1    -18 dB
//! 2    -15 dB
//! ...
//! 6     -3 dB
//! 7     0 dB
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetLeftSpkPGA(UInt8 gainSpk)
{
        AUDIO_ANACR0_un_t TempReg; 				 // get a local copy of register structure

#if defined(SS_2153)
	gainSpk = (UInt8)AUDIO_DRV_SelectCustomGain(CUSTOM_SPK_PGA, (UInt16) gainSpk); // 02062009 michael
#endif // SS_2153
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pga_ctrlL = MAX_SPK_GAIN_STEPS -gainSpk;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	_DBG_A_(audioPrintf("AUDIO_DRV_SetLeftSpkPGA() gainSpk=%d\r\n", gainSpk));
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetSpkDriverOn
//!
//! Description:	power on the speaker driver
//!
//! Notes:			
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetSpkDriverOn(AUDIO_DRV_Spk_Output_t eSpeakerType, AUDIO_CHANNEL_t Ch)
{	
	_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverOn() eSpeakerType=%d, Ch =%d \r\n", eSpeakerType, Ch));

	switch(eSpeakerType)
	{
		case LEFT_400MW:
			AUDIO_DRV_MuteLeftSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetLChannelMode(CHANNEL_DIFF);	
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_DIFF);	
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
#if defined(FUSE_APPS_PROCESSOR)
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
#else
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
#endif			
			AUDIO_DRV_PowerDownRightSpk();
			AUDIO_DRV_PowerUpLeftSpk();
			break;

		case RIGHT_400MW:
			AUDIO_DRV_MuteRightSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_DIFF);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetLChannelMode(CHANNEL_DIFF);	
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
#if defined(FUSE_APPS_PROCESSOR)
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
#else
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
#endif			
			AUDIO_DRV_PowerDownLeftSpk();
			AUDIO_DRV_PowerUpRightSpk();
			break;

		case RIGHT_PLUS_LEFT_100MW:
#ifdef HEADSET_AUDIO_3WIRE
			AUDIO_DRV_MuteRightSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(UNMUTE_ALL);
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_3WIRE_2);
			AUDIO_DRV_SetLChannelMode(CHANNEL_3WIRE_2);	
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
#if defined(FUSE_APPS_PROCESSOR)
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
#else
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
#endif			
			AUDIO_DRV_PowerUpRightSpk();
			AUDIO_DRV_PowerUpLeftSpk();
#else // regular single ended
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(5);
#endif
			AUDIO_DRV_MuteRightSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(UNMUTE_ALL);
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_3WIRE_1);
			AUDIO_DRV_SetLChannelMode(CHANNEL_3WIRE_1);	
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_UP);
			//AudHal_Sleep(5);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(40);
#endif
#if defined(FUSE_APPS_PROCESSOR)
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
#else
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
#endif			
			AUDIO_DRV_PowerUpRightSpk();	
			AUDIO_DRV_PowerUpLeftSpk();
#endif
			break;

		default: // no audio output
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			//AudHal_Sleep(5);

			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_DOWN);
			//AudHal_Sleep(150);
			//AudHal_Sleep(10);
			AUDIO_DRV_PowerDownRightSpk();	
			AUDIO_DRV_PowerDownLeftSpk();
			//AudHal_Sleep(5);		
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetSpkDriverSelect
//!
//! Description:	power on the speaker driver
//!
//! Notes: Select output/mode/pga with ANACR1 bit[7]=1
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetSpkDriverSelect(AUDIO_DRV_Spk_Output_t eSpeakerType, AUDIO_CHANNEL_t Ch)
{	
	_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverSelect() eSpeakerType=%d, Ch =%d, bPowerDownL=%d, bPowerDownR=%d \r\n", eSpeakerType, Ch,bPowerDownL, bPowerDownR));

	switch(eSpeakerType)
	{
		case LEFT_400MW:
#if defined(FUSE_APPS_PROCESSOR)
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
#else
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
#endif		
			AUDIO_DRV_MuteLeftSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetLChannelMode(CHANNEL_DIFF);	
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_DIFF);
			AUDIO_DRV_PowerUpD2C();	
			break;

		case RIGHT_400MW:
#if defined(FUSE_APPS_PROCESSOR)
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
#else
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
#endif		
			AUDIO_DRV_MuteRightSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_DIFF);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetLChannelMode(CHANNEL_DIFF);	
			AUDIO_DRV_PowerUpD2C();	
			break;

		case RIGHT_PLUS_LEFT_100MW:
#if defined(FUSE_APPS_PROCESSOR)
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()[Ch].speaker_pga);
#else
			AUDIO_DRV_SetRightSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
			AUDIO_DRV_SetLeftSpkPGA((UInt16)AUDIO_GetParmAccessPtr()->audio_parm[Ch].speaker_pga);
#endif	

#ifdef HEADSET_AUDIO_3WIRE
			AUDIO_DRV_MuteRightSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(UNMUTE_ALL);
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_3WIRE_2);
			AUDIO_DRV_SetLChannelMode(CHANNEL_3WIRE_2);
			AUDIO_DRV_PowerUpD2C();
#else // regular single ended
			AUDIO_DRV_MuteRightSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(UNMUTE_ALL);
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_100MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_3WIRE_1);
			AUDIO_DRV_SetLChannelMode(CHANNEL_3WIRE_1);
			AUDIO_DRV_PowerUpD2C();
#endif
			break;

		default: // no audio output
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			//AudHal_Sleep(5);
			//AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
			//AudHal_Sleep(40);
			//AUDIO_DRV_PowerDownRightSpk();	
			//AUDIO_DRV_PowerDownLeftSpk();	
			//AudHal_Sleep(5);
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetSpkDriverOutputOff
//!
//! Description:	power down the speaker driver
//!
//! Notes: Audio driver output off with ramp
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetSpkDriverOutputOff(AUDIO_DRV_Spk_Output_t eSpeakerType)
{
	// Don't blind turn off the speaker
	_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverOutputOff(%d) AUDIO_ON?=%d\r\n",eSpeakerType, (bStatusAudio||bStatusVoice||bStatusPoly)));
	
	switch(eSpeakerType)
	{
		case RIGHT_400MW:
		case LEFT_400MW:
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_DEFAULT);
			AUDIO_DRV_PowerDownRightSpk();
			AUDIO_DRV_PowerDownLeftSpk();
			break;

		case RIGHT_PLUS_LEFT_100MW:
#ifndef HEADSET_AUDIO_3WIRE
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(40);
#endif
#endif
			AUDIO_DRV_PowerDownRightSpk();
			AUDIO_DRV_PowerDownLeftSpk();
			break;

		default: // no audio output
			break;
	}

	return;
}
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetSpkDriverOutputOn
//!
//! Description:	power on the speaker driver
//!
//! Notes: Audio driver output on with ramp
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetSpkDriverOutputOn(AUDIO_DRV_Spk_Output_t eSpeakerType, AUDIO_CHANNEL_t Ch)
{	
	_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverOutputOn() eSpeakerType=%d, Ch =%d, bPowerDownL=%d, bPowerDownR=%d \r\n", eSpeakerType, Ch,bPowerDownL,bPowerDownR));

#if (defined(_BCM2153_))
	PMU_AMPVDD( TRUE );
#endif

	switch(eSpeakerType)
	{
		case LEFT_400MW:
			if((bPowerDownL==FALSE))
				break;
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(1);
#endif
			AUDIO_DRV_PowerUpLeftSpk();
			AUDIO_DRV_PowerDownRightSpk();
			break;
		case RIGHT_400MW:
			if((bPowerDownR==FALSE))
				break;
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(1);
#endif
			AUDIO_DRV_PowerUpRightSpk();
			AUDIO_DRV_PowerDownLeftSpk();
			break;
		case RIGHT_PLUS_LEFT_100MW:
			if((bPowerDownL==FALSE)&&(bPowerDownR==FALSE))
				break;
#ifdef HEADSET_AUDIO_3WIRE
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(1);
#endif
			AUDIO_DRV_PowerUpRightSpk();
			AUDIO_DRV_PowerUpLeftSpk();
#else // regular single ended
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_UP);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(1);
#endif
			AUDIO_DRV_PowerUpRightSpk();
			AUDIO_DRV_PowerUpLeftSpk();
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(5);
#endif
#endif
			break;

		default: // no audio output
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			AudHal_Sleep(5);
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
			AudHal_Sleep(40);
			AUDIO_DRV_PowerDownRightSpk();	
			AUDIO_DRV_PowerDownLeftSpk();
			AudHal_Sleep(5);
#else
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
			AUDIO_DRV_PowerDownRightSpk();	
			AUDIO_DRV_PowerDownLeftSpk();	
#endif
			break;
	}
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetSpkDriverDown
//!
//! Description:	power down the speaker driver
//!
//! Notes:			
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetSpkDriverDown(AUDIO_DRV_Spk_Output_t eSpeakerType)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverDown(%d) AUDIO_ON?=%d\r\n",eSpeakerType, (bStatusAudio||bStatusVoice||bStatusPoly)));
	
	switch(eSpeakerType)
	{
		case LEFT_400MW:
		case RIGHT_400MW:
		case RIGHT_PLUS_LEFT_100MW:
#if !defined(ROXY_AUD_CTRL_FEATURES)
			//AUDIO_DRV_PowerDownD2C();
			AudHal_Sleep(1);
#else
			AUDIO_DRV_PowerDownD2C();
		//	AudHal_Sleep(1);
#endif
			break;

		default: // no audio output
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
#if !defined(ROXY_AUD_CTRL_FEATURES)
			OSTASK_Sleep(5);
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
			AudHal_Sleep(40);
			AUDIO_DRV_PowerDownRightSpk();	
			AUDIO_DRV_PowerDownLeftSpk();	
			AudHal_Sleep(5);
#else
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
			AUDIO_DRV_PowerDownRightSpk();	
			AUDIO_DRV_PowerDownLeftSpk();	
#endif
			break;
	}
#if (defined(_BCM2153_))
	PMU_AMPVDD( FALSE );
#endif
	return;
}



/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetSpkDriverOff
//!
//! Description:	power down the speaker driver
//!
//! Notes:			
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetSpkDriverOff(AUDIO_DRV_Spk_Output_t eSpeakerType)
{
	// Don't blind turn off the speaker
	_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverOff() eSpeakerType=%d\r\n", eSpeakerType));

	AUDIO_DRV_SetSpkDriverOutputOff	(eSpeakerType);
	AUDIO_DRV_SetSpkDriverDown (eSpeakerType);

	return;
}

Boolean AUDIO_DRV_IsBTMode(AudioMode_t audio_mode)
{
	Boolean is_bt = FALSE;
	switch(audio_mode)
		{
		case AUDIO_MODE_BLUETOOTH: 
		case AUDIO_MODE_BLUETOOTH_WB: 
			is_bt = TRUE;
			break;
		default : 
			// no BT mode
			break;
		}
	return is_bt;
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPCMOnOff
//!
//! Description:	set PCM on/off for BT
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
#define PCM_REG &__REG32(HW_SYS_BASE+0x5C)
void AUDIO_DRV_SetPCMOnOff(Boolean	on_off)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetPCMOnOff() on_off=%d\r\n", on_off));

	// By default the PCM port is occupied by trace port on development board
	if(on_off)
	{
#if (!defined(MARTINI_HW)) || defined(EXPLORER_BOARD_REV)
	#if (defined(VENUSRAY)) || defined(JUNORAY) || defined(TORPEDORAY) || defined(SS_2153) || defined(EXPLORER_BOARD_REV)
		*(volatile UInt32 *)IOCR0_REG &= 0xFFFFF7FF;   // PCM_MUX
	#elif (defined(ZEUSRAY))
		 *(volatile UInt32 *)PCM_REG |= 0x00000080;
	#endif
#endif	
		// Work around for a DSP bug, a variable in the DSP in RAM which is not inited properly by the DSP 
		// which is necessary for the BT PCM path to work.
#if (defined(_BCM2153_) && CHIP_REVISION <= 50)		
		AUDIO_BitwiseWriteDSP(0x4F, 0x0001, 0x0001);
#endif
		AUDIO_PostMsgToDSP(COMMAND_DIGITAL_SOUND, on_off, 0, 0);
		//in theory this is redundant, keep it until amcr control is centralized
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x4000, 0x4000);
	}
	else
	{
#if (defined(_BCM2153_) && CHIP_REVISION <= 50)		
		AUDIO_BitwiseWriteDSP(0x4F, 0x0001, 0x0);
#endif
		AUDIO_PostMsgToDSP(COMMAND_DIGITAL_SOUND, on_off, 0, 0);		
		//in theory this is redundant, keep it until amcr control is centralized
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x4000, 0x0000);
	}	
}

#if 0 //duplicated
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_Set_DL_OnOff
//!
//! Description:	set downlink on/off
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_Set_DL_OnOff(Boolean on_off)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_Set_DL_OnOff() on_off=%d\r\n", on_off));
	
#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
		CAPI2_RIPCMDQ_Connect_Downlink(0, 0, on_off); 
#else
		RIPCMDQ_Connect_Downlink(on_off); 
#endif
}
#endif

#ifdef BCM2153_FAMILY_BB_CHIP_BOND
Boolean AUDIO_DRV_IsCall16K(AudioMode_t voiceMode)
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

AudioMode_t AUDIO_DRV_ConvertVoiceIDChnlMode(AudioClientID_en_t audioID, AUDIO_CHANNEL_t outputChnl)
{
	AudioMode_t audio_mode  = AUDIO_MODE_HANDSET;
	if((audioID != AUDIO_ID_CALL8K)&&(audioID != AUDIO_ID_CALL16K))
	{
		_DBG_A_(audioPrintf("AUDIO_DRV_ConvertVoiceIDChnlMode() audioID is not voice %d\r\n", audioID));		
	}
	switch(outputChnl)
	{
		case	AUDIO_CHNL_HANDSET:
			audio_mode = AUDIO_MODE_HANDSET;
			break;
		case	AUDIO_CHNL_HEADPHONE:
			audio_mode = AUDIO_MODE_HEADSET;
			break;
		case	AUDIO_CHNL_HANDSFREE:
			audio_mode = AUDIO_MODE_HANDSFREE;
			break;
		case	AUDIO_CHNL_BLUETOOTH:
			audio_mode = AUDIO_MODE_BLUETOOTH;
			break;
		case	AUDIO_CHNL_SPEAKER:
			audio_mode = AUDIO_MODE_SPEAKERPHONE;
			break;
		case	AUDIO_CHNL_TTY:
			audio_mode = AUDIO_MODE_TTY;
			break;
		case	AUDIO_CHNL_HAC:
			audio_mode = AUDIO_MODE_HAC;
			break;
		case	AUDIO_CHNL_USB:
			audio_mode = AUDIO_MODE_USB;
			break;
		case	AUDIO_CHNL_BT_STEREO:
			audio_mode = AUDIO_MODE_BLUETOOTH;
			break;
		case	AUDIO_CHNL_RESERVE:
			audio_mode = AUDIO_MODE_RESERVE;
			break;
		default:
			_DBG_A_(audioPrintf("AUDIO_DRV_ConvertVoiceIDChnlMode() outputChnl is out of range %d\r\n", outputChnl));		
			break;
	}
	if(audioID == AUDIO_ID_CALL16K)
		audio_mode += AUDIO_MODE_NUMBER;

	_DBG_A_(audioPrintf("AUDIO_DRV_ConvertVoiceIDChnlMode() audio_mode = %d\r\n", audio_mode));		

	return audio_mode;
}

AudioMode_t AUDIO_DRV_ConvertChnlMode(AUDIO_CHANNEL_t outputChnl)
{
	AudioMode_t audio_mode  = AUDIO_MODE_HANDSET;

	switch(outputChnl)
	{
		case	AUDIO_CHNL_HANDSET:
			audio_mode = AUDIO_MODE_HANDSET;
			break;
		case	AUDIO_CHNL_HEADPHONE:
			audio_mode = AUDIO_MODE_HEADSET;
			break;
		case	AUDIO_CHNL_HANDSFREE:
			audio_mode = AUDIO_MODE_HANDSFREE;
			break;
		case	AUDIO_CHNL_BLUETOOTH:
			audio_mode = AUDIO_MODE_BLUETOOTH;
			break;
		case	AUDIO_CHNL_SPEAKER:
			audio_mode = AUDIO_MODE_SPEAKERPHONE;
			break;
		case	AUDIO_CHNL_TTY:
			audio_mode = AUDIO_MODE_TTY;
			break;
		case	AUDIO_CHNL_HAC:
			audio_mode = AUDIO_MODE_HAC;
			break;
		case	AUDIO_CHNL_USB:
			audio_mode = AUDIO_MODE_USB;
			break;
		case	AUDIO_CHNL_BT_STEREO:
			audio_mode = AUDIO_MODE_BLUETOOTH;
			break;
		case	AUDIO_CHNL_RESERVE:
			audio_mode = AUDIO_MODE_RESERVE;
			break;
		default:
			_DBG_A_(audioPrintf("AUDIO_DRV_ConvertChnlMode() outputChnl is out of range %d\r\n", outputChnl));		
			break;
	}

	_DBG_A_(audioPrintf("AUDIO_DRV_ConvertChnlMode() audio_mode = %d\r\n", audio_mode));
	return audio_mode;

}

AUDIO_CHANNEL_t AUDIO_DRV_ConvertModeChnl(AudioMode_t mode)
{
	AUDIO_CHANNEL_t audio_chnl  = AUDIO_CHNL_HANDSET;

	switch(mode)
	{
		case	AUDIO_MODE_HANDSET:
		case	AUDIO_MODE_HANDSET_WB:
			audio_chnl = AUDIO_CHNL_HANDSET;
			break;
		case	AUDIO_MODE_HEADSET:
		case	AUDIO_MODE_HEADSET_WB:
			audio_chnl = AUDIO_CHNL_HEADPHONE;
			break;
		case	AUDIO_MODE_HANDSFREE:
		case	AUDIO_MODE_HANDSFREE_WB:
			audio_chnl = AUDIO_CHNL_HANDSFREE;
			break;
		case	AUDIO_MODE_BLUETOOTH:
		case	AUDIO_MODE_BLUETOOTH_WB:
			audio_chnl = AUDIO_CHNL_BLUETOOTH;
			break;
		case	AUDIO_MODE_SPEAKERPHONE:
		case	AUDIO_MODE_SPEAKERPHONE_WB:
			audio_chnl = AUDIO_CHNL_SPEAKER;
			break;
		case	AUDIO_MODE_TTY:
		case	AUDIO_MODE_TTY_WB:
			audio_chnl = AUDIO_CHNL_TTY;
			break;
		case	AUDIO_MODE_HAC:
		case	AUDIO_MODE_HAC_WB:
			audio_chnl = AUDIO_CHNL_HAC;
			break;
		case	AUDIO_MODE_USB:
		case	AUDIO_MODE_USB_WB:
			audio_chnl = AUDIO_CHNL_USB;
			break;
		case	AUDIO_MODE_RESERVE:
		case	AUDIO_MODE_RESERVE_WB:
			audio_chnl = AUDIO_CHNL_RESERVE;
			break;
		default:
			_DBG_A_(audioPrintf("AUDIO_DRV_ConvertModeChnl() mode is out of range %d\r\n", mode));		
			break;
	}

	_DBG_A_(audioPrintf("AUDIO_DRV_ConvertModeChnl() audio_chnl = %d\r\n", audio_chnl));
	return audio_chnl;

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetDualMicPathOnOff
//!
//! Description:	turn on/off dual/digital microphone path
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetDualMicPathOnOff(AudioMode_t voiceMode , Boolean on_off)
{
	UInt16 dualmic_configuration = 0; 
	BB_DualMic_Path_t mic_path1, mic_path2;
	UInt16 digmic_reg = 0x0000;
	Boolean digmic_enable, digmic_sampling, dualmic_sw;
	Boolean digmic_clk = FALSE;
	unsigned int value;

	static Boolean prev_Ndigmic_enable = FALSE;
	static Boolean prev_on_off_state = FALSE;

#if defined(FUSE_APPS_PROCESSOR)
	dualmic_configuration = (UInt16)AUDIO_GetParmAccessPtr()[voiceMode].voice_dualmic_enable;
#else
	dualmic_configuration = (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[voiceMode].voice_dualmic_enable;
#endif

	digmic_sampling = AUDIO_DRV_IsCall16K(voiceMode);

	_DBG_A_(audioPrintf("AUDIO_DRV_SetDigitalMicPathOnOff() voiceMode=%d on_off=%d, dualmic_configuration=%d, digmic_sampling=%d\r\n", 
		voiceMode, on_off,dualmic_configuration ,digmic_sampling));


	switch(dualmic_configuration)
	{
		case 1: // 1st analog, 2nd digital
			digmic_enable = TRUE;
			dualmic_sw = TRUE;
			mic_path1 = MIC_ANALOG_ADC1;
			mic_path2 = MIC_DIGITAL_MIC2;
			break;
		case 2: // 1st and 2nd digital
			digmic_enable = TRUE;
			dualmic_sw = TRUE;
			mic_path1 = MIC_DIGITAL_MIC1;
			mic_path2 = MIC_DIGITAL_MIC2;
			break;
		case 0: // no digital mic usage
		default:
			digmic_enable = FALSE;
			dualmic_sw = FALSE;
			mic_path1 = MIC_ANALOG_ADC1;
			mic_path2 = MIC_ANALOG_ADC1;
			break;
	}

	if(on_off)
	{
#if (defined(BCM2153_FAMILY_BB_CHIP_BOND) && !defined(ENABLE_DSPEXTMEM) )
		value = *(unsigned int *)IOCR8_REG;
		if (!prev_on_off_state)
			prev_Ndigmic_enable = (value & 0x00000010) >> 4;
		if(digmic_enable)
			value &= 0xffffffef;
		else
			value |= 0x00000010;
		*(unsigned int *)IOCR8_REG = value;
#endif
	digmic_reg = (BIT13_MASK&(digmic_clk<<BIT13))|(BIT12_MASK&(digmic_enable<<BIT12))|(BIT11_MASK&(digmic_sampling<<BIT11))|(BIT05_MASK&(dualmic_sw<<BIT05))|((BIT09_MASK+BIT08_MASK)&(mic_path2<<BIT08))|((BIT07_MASK+BIT06_MASK)&(mic_path1<<BIT06));
	_DBG_A_(audioPrintf("AUDIO_DRV_SetDigitalMicPathOnOff() digmic_reg =%08x, digmic_clk =%d, digmic_enable = %d,digmic_sampling = %d, dualmic_sw=%d, mic_path1=%d, mic_path2=%d\r\n", 
		digmic_reg, digmic_clk, digmic_enable,digmic_sampling,dualmic_sw, mic_path1,mic_path2));
	
		// AUDIO_WriteDSP(DSP_AUDIO_REG_ADCCONTROL, (UInt16)digmic_reg);
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ADCCONTROL, 0xFFFC, (UInt16)digmic_reg);
	post_msg(0x8b, dualmic_sw, 0, 0);
	}
	else
	{
		digmic_enable = FALSE;
#if (defined(BCM2153_FAMILY_BB_CHIP_BOND) && !defined(ENABLE_DSPEXTMEM) )
		value = *(unsigned int *)IOCR8_REG;
		// value |= 0x00000010;
		if (!prev_Ndigmic_enable)
			value &= 0xffffffef;
		else
			value |= 0x00000010;
		*(unsigned int *)IOCR8_REG = value;
#endif
		_DBG_A_(audioPrintf("AUDIO_DRV_SetDigitalMicPathOnOff() off\r\n"));
		AUDIO_WriteDSP(DSP_AUDIO_REG_ADCCONTROL, 0x00);
	}

	prev_on_off_state = on_off;
}

#endif //  BCM2153_FAMILY_BB_CHIP_BOND

#ifndef FEATURE_AUDIO_VOICECALL16K
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetVoicePathOn
//!
//! Description:	turn on voice path
//!
//! Notes: need to follow the sequence to eliminate the pop noise
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetVoicePathOn(AudioMode_t voiceMode)
{	
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[voiceMode];

	_DBG_A_(audioPrintf("AUDIO_DRV_SetVoicePathOn() voiceMode=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		voiceMode, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	AUDIO_DRV_SetSpkDriverSelect(eSpkType, (AUDIO_CHANNEL_t)voiceMode);//AUDIO_DRV_SetSpkDriverOn(eSpkType, (AUDIO_CHANNEL_t)voiceMode);
#ifdef USE_DSP_AUDIO_ENABLE
	audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#else
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x0020);
#endif
	if (AUDIO_DRV_IsBTMode(voiceMode))
		AUDIO_DRV_SetPCMOnOff(TRUE);
	else
		AUDIO_DRV_SetPCMOnOff(FALSE);

	// For old chip(2133, 2152) we still use the channel information from sysparm
	//uSpkMode = (AudioMode_t)MICConfig[voiceMode];	
	//AUDIO_DRV_SetVoiceVolume(uSpkMode, DEFAULT_VOICE_VOLUME);  //why?
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, (AUDIO_CHANNEL_t)voiceMode);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOn(voiceMode);
#endif

	AUDIO_DRV_Set_DL_OnOff(AUDIO_ON);

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VMUT, 0x2, 0);

#ifdef BSP_ONLY_BUILD
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, 0x91ff);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, 0x0);
#else
#if defined(FUSE_APPS_PROCESSOR)
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()[voiceMode].audvoc_vslopgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()[voiceMode].audvoc_vcfgr);
#else
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[voiceMode].audvoc_vslopgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[voiceMode].audvoc_vcfgr);
#endif	
#endif	

	//per call basis: enable the DTX by calling stack api when call connected
	if (bStatusCall == AUDIO_OFF)
		audio_control_dsp(ENABLE_DSP_DTX, TRUE, 0, 0);
	
	bStatusCall = AUDIO_ON;	
	bStatusVoice = VOICE_PATH_STATUS;
}
#else // FEATURE_AUDIO_VOICECALL16K
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetVoicePathOn
//!
//! Description:	turn on voice path
//!
//! Notes: need to follow the sequence to eliminate the pop noise
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetVoicePathOn(AudioMode_t voiceMode, AudioClientID_en_t audioID)
{	
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[voiceMode];
	Boolean isCall16K = AUDIO_DRV_IsCall16K(voiceMode);

	_DBG_A_(audioPrintf("AUDIO_DRV_SetVoicePathOn() voiceMode=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		voiceMode, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	AUDIO_DRV_SetSpkDriverSelect(eSpkType, (AUDIO_CHANNEL_t)voiceMode);//AUDIO_DRV_SetSpkDriverOn(eSpkType, (AUDIO_CHANNEL_t)voiceMode);

	audio_control_dsp(ENABLE_DSP_AUDIO, 0, isCall16K, 0);

	if (AUDIO_DRV_IsBTMode(voiceMode))
		AUDIO_DRV_SetPCMOnOff(TRUE);
	else
		AUDIO_DRV_SetPCMOnOff(FALSE);

	// For old chip(2133, 2152) we still use the channel information from sysparm
	//uSpkMode = (AudioMode_t)MICConfig[voiceMode];	
	//AUDIO_DRV_SetVoiceVolume(uSpkMode, DEFAULT_VOICE_VOLUME);  //why?
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, (AUDIO_CHANNEL_t)voiceMode);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOn(voiceMode);
#endif

	AUDIO_DRV_Set_DL_OnOff(AUDIO_ON);

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VMUT, 0x2, 0);

#ifdef BSP_ONLY_BUILD
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, 0x91ff);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, 0x0);
#else
#if defined(FUSE_APPS_PROCESSOR)
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()[voiceMode].audvoc_vslopgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()[voiceMode].audvoc_vcfgr);
#else
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[voiceMode].audvoc_vslopgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[voiceMode].audvoc_vcfgr);
#endif	
#endif	

	//per call basis: enable the DTX by calling stack api when call connected
	if (bStatusCall == AUDIO_OFF)
		audio_control_dsp(ENABLE_DSP_DTX, TRUE, 0, 0);
	
	bStatusCall = AUDIO_ON;	
	bStatusVoice = VOICE_PATH_STATUS;
}

#endif // FEATURE_AUDIO_VOICECALL16K

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetVoicePathOff
//!
//! Description:	turn off voice path
//!
//! Notes: need to follow the sequence to eliminate the pop noise
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetVoicePathOff(AudioMode_t voiceMode)
{	
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[voiceMode];

	bStatusCall = AUDIO_OFF;
	bStatusVoice = VOICE_PATH_STATUS;
	
	_DBG_A_(audioPrintf("AUDIO_DRV_SetVoicePathOff() bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));
    //per call basis: disable the DTX by calling stack api when call disconnected
	audio_control_dsp(ENABLE_DSP_DTX, FALSE, 0, 0);

	if(AUDIO_ON == (bStatusAudio||bStatusVoice||bStatusPoly))
	{
#if 1	
		_DBG_A_(audioPrintf("AUDIO_DRV_SetVoicePathOff() bStatusAudio=%d,bStatusPoly=%d\r\n", bStatusAudio, bStatusPoly));
		if(bStatusVoice == AUDIO_OFF)
		{
			AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,0xa000);
			AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR,0x18);

			if (bStatusPcmBack == AUDIO_OFF)
			{
				AUDIO_DRV_Set_DL_OnOff(AUDIO_OFF);
			
#ifdef USE_DSP_AUDIO_ENABLE
				audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else			
				AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x00);
#endif
			}
			
			if (AUDIO_DRV_IsBTMode(voiceMode))
				AUDIO_DRV_SetPCMOnOff(FALSE);
		}
#endif	
	return;
	}
	
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,0xa000);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR,0x18);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOff(voiceMode);
#endif

	AUDIO_DRV_SetSpkDriverOff(eSpkType);
	
	if (bStatusPcmBack == AUDIO_OFF)
	{
		AUDIO_DRV_Set_DL_OnOff(AUDIO_OFF);
#ifdef USE_DSP_AUDIO_ENABLE
		audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else	
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x00);
#endif
	}
	
	if (AUDIO_DRV_IsBTMode(voiceMode))
		AUDIO_DRV_SetPCMOnOff(FALSE);
	

	if ((bStatusInputPath||bStatusPcmBack) == AUDIO_OFF)
		AUDIO_DRV_PowerDownD2C();
	return;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetVoiceVolume
//!
//! Description:	set voice volume
//!
//! Notes: this api accepts either the logic level (platform mmi) or the db level (customer mmi)
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetVoiceVolume( UInt8 Volume)
{
    UInt8 digital_gain_dB = 0;
	UInt16  volume_max;

#if defined(FUSE_APPS_PROCESSOR)
	volume_max = APSYSPARM_GetAudioParmAccessPtr()[AUDIO_GetAudioMode()].voice_volume_max;
#else
	volume_max = SYSPARM_GetVoiceVolumeMax();
#endif

    if (uVol2BB[Volume] > volume_max)
        digital_gain_dB = volume_max;
    else
        digital_gain_dB = uVol2BB[Volume];

#if (!defined(BSP_ONLY_BUILD))
#if defined(FUSE_APPS_PROCESSOR)
	_DBG_A_(audioPrintf("AUDIO_DRV_SetVoiceVolume() Volume=%d digital_gain_dB=%d OV_volume=%d\r\n", 
		Volume, digital_gain_dB, uVol2OV[digital_gain_dB]));

	CAPI2_VOLUMECTRL_SetBasebandVolume(0, 0, digital_gain_dB, uVol2OV[digital_gain_dB], 0, 0 );
#else 
    VOLUMECTRL_SetBasebandVolume(digital_gain_dB, uVol2OV[digital_gain_dB], 0, 0 );
#endif
#endif
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_GetVoiceVolumeInDB
//!
//! Description:	get voice volume in dB
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
UInt8 AUDIO_DRV_GetVoiceVolumeInDB( UInt8 Volume)
{
	return uVol2BB[Volume];
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetMicrophonePathOnOff
//!
//! Description:	turn on/off microphone path
//!
/////////////////////////////////////////////////////////////////////////////
#define AUDSEL_CONTROL_MASK 	0x0010
void AUDIO_DRV_SetMicrophonePathOnOff(AudioMode_t voiceMode , Boolean on_off)
{
	UInt16 MicGain;
	AUDIO_DRV_MIC_Path_t MicCh;
	AsicAuxMicBiasMode_t BiasMode = AuxMicBias_ON_LPMODE;

	_DBG_A_(audioPrintf("AUDIO_DRV_SetMicrophonePathOnOff() voiceMode=%d on_off=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n",
		voiceMode, on_off, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));
	if (on_off)
	{
#if defined(SS_2153)
#if defined(FUSE_APPS_PROCESSOR)
		MicGain = AUDIO_DRV_SelectCustomGain(CUSTOM_MIC_PGA, (UInt16)AUDIO_GetParmAccessPtr()[voiceMode].mic_pga);
#else
		MicGain = AUDIO_DRV_SelectCustomGain(CUSTOM_MIC_PGA, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[voiceMode].mic_pga);
#endif
#else // SS_2153
#if defined(FUSE_APPS_PROCESSOR)
		MicGain = (UInt16)AUDIO_GetParmAccessPtr()[voiceMode].mic_pga;
#else
		MicGain = (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[voiceMode].mic_pga;
#endif
#endif // SS_2153
	}
	else
	{
		MicGain = 0;
		//mute the microphone for next call
		//this is to suppress the noise leaked to UL in 3G before the call is answered.
		AUDIO_DRV_MicrophoneMuteOnOff(TRUE);
	}
	
	MicCh = MICConfig[voiceMode];

	AUDIO_DRV_SelectMIC(MicCh);

	if (MicCh == NONE_MIC)
	{
		//MicGain is the index [0 14] for converting mic pga to dsp ul gain on cp
		//audio_control_dsp(SET_DSP_UL_GAIN, MicGain, 0, 0);

		//applied when doing unmute due to dsp sequence
		currentMicGain = MicGain;
		bStatusMicChUsed = FALSE;
	}
	else
	{
		//use default dsp ul gain which is 0dB now
		//audio_control_dsp(SET_DSP_UL_GAIN, 0, 0, 0);
		AUDIO_DRV_SetMICGain(MicGain);
		bStatusMicChUsed = TRUE;
	}
	
	switch(MicCh)
	{
		case AUDIO_MAIN_MIC:
#ifdef SS_2153
			if (EarJack_type_check != Earjack_type_4)
			{
				*(volatile UInt32 *)AUXMIC_CMC_REG &=~(BIT00_MASK);
			} // assume that HEADSET BIAS / CMC is controlled by hal_audio_accessory_glue.c in SS design.
#else
			*(volatile UInt32 *)AUXMIC_CMC_REG &=~(BIT00_MASK);
#endif // SS_2153
			AUDIO_DRV_SetAuxMicBiasOnOff(FALSE, 0);
			AUDIO_DRV_SetMicBiasOnOff(on_off);
			break;

		case AUDIO_AUX_MIC:
			if (on_off)
			{
        		BiasMode = AuxMicBias_ON_FPMODE;
#ifndef SS_2153			
				*(volatile UInt32 *)AUXMIC_CMC_REG |=BIT00_MASK;
				AUDIO_DRV_SetAuxMicBiasOnOff(on_off, BiasMode);
#endif // SS_2153
			}
			else if (bStatusVoice == AUDIO_OFF)
			{
#ifndef SS_2153			
				*(volatile UInt32 *)AUXMIC_CMC_REG &=~(BIT00_MASK);
				AUDIO_DRV_SetAuxMicBiasOnOff(on_off, BiasMode);
#endif // SS_2153
			}
#ifdef SS_2153
        	AUDIO_DRV_SetAuxMicBiasConfig(TRUE, BiasMode);
#endif // SS_2153
			break;

		default :
			break;
	}
#if defined(FUSE_COMMS_PROCESSOR)
	//this is needed until call control goes to ap
	RIPCMDQ_Connect_Uplink(on_off); 
#endif

	if (AUDIO_DRV_IsBTMode(voiceMode))
	{
		if (on_off)
			AUDIO_DRV_SetPCMOnOff(TRUE);
		else if (bStatusVoice == AUDIO_OFF)
			AUDIO_DRV_SetPCMOnOff(FALSE);
	}
	
	if ((!on_off) && ((bStatusVoice || bStatusAudio || bStatusPoly||bStatusPcmBack) == AUDIO_OFF))
	{
		AUDIO_DRV_PowerDownD2C();
	}
	
	bStatusInputPath = on_off;
	
}

#if 0 //duplicated
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_ConnectMicrophoneUplink
//!
//! Description:	connect/disconnect microphne to uplink
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_ConnectMicrophoneUplink(Boolean on_off)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_ConnectMicrophoneUplink() on_off=%d bStatusVoice=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", on_off, bStatusVoice,bStatusCall,bStatusAMR,bStatusDTMF));

	if ((on_off) || ((!on_off) && ((bStatusCall == AUDIO_OFF)&&(bStatusAMR == AUDIO_OFF))))
#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
		CAPI2_RIPCMDQ_Connect_Uplink(0, 0, on_off); 
#else
		RIPCMDQ_Connect_Uplink(on_off);	
#endif
}
#endif
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetMicrophonePath_16K
//!
//! Description:	set/unset microphone to 16K
//!
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetMicrophonePath_16K(Boolean on_off)
{
	if(on_off)
		post_msg(COMMAND_AUDIO_ENABLE, 1, 1, 1);
	else
		post_msg(COMMAND_AUDIO_ENABLE, 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_MicrophoneMuteOnOff
//!
//! Description:	Mute/Unmute microphone path
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_MicrophoneMuteOnOff(Boolean mute_on_off)
{
	Int16 EchoGain;

	_DBG_A_(audioPrintf("AUDIO_DRV_MicrophoneMuteOnOff() mute_on_off=%d\r\n", mute_on_off));

#if defined(ROXY_AUD_CTRL_FEATURES)    //*** fix me***does this part can be deleted?
	if(mute_on_off)
		EchoGain = -100;
	else
		EchoGain = 0;	
	
#if (!defined(BSP_ONLY_BUILD))	
#if defined(FUSE_APPS_PROCESSOR)
	CAPI2_ECHO_SetDigitalTxGain(0, 0, EchoGain); 
#else 
	ECHO_SetDigitalTxGain(EchoGain);
#endif
#endif

#else //else of #if defined(ROXY_AUD_CTRL_FEATURES)

	if(mute_on_off) //mute
	{
#if defined(FUSE_APPS_PROCESSOR)		
		audio_control_dsp(MUTE_DSP_UL, 0, 0, 0);	
#else
		VOLUMECTRL_MuteTx();
#endif
	}
	else //unmute
	{	
		if (bStatusMicChUsed == TRUE) //recover previous dsp ul gain
		{
#if defined(FUSE_APPS_PROCESSOR)		
		audio_control_dsp(UNMUTE_DSP_UL, 0, 0, 0);	
#else
		VOLUMECTRL_UnMuteTx();
#endif
		}
		else //for none_mic case, like bt, usb
		{
			EchoGain = (Int16)currentMicGain;
			//the index [0 14] for converting mic pga to dsp ul gain on cp
			audio_control_dsp(SET_DSP_UL_GAIN, EchoGain, 0, 0);
		}
	}

#endif  //end of #if defined(ROXY_AUD_CTRL_FEATURES)

}

#if 0 //duplicated
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_EC_NS_OnOff
//!
//! Description:	turn EC and NS on/off
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_EC_NS_OnOff(Boolean ec_on_off, Boolean ns_on_off)
{	
	_DBG_A_(audioPrintf("AUDIO_DRV_EC_NS_OnOff() ec_on_off=%d ns_on_off=%d\r\n", ec_on_off, ns_on_off));

#if (!defined(BSP_ONLY_BUILD))
#if defined(FUSE_APPS_PROCESSOR)
	CAPI2_AUDIO_Turn_EC_NS_OnOff(0, 0, ec_on_off, ns_on_off); 
#else 
	AUDIO_Turn_EC_NS_OnOff(ec_on_off, ns_on_off);
#endif
#endif
}
#endif
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_PlayTone
//!
//! Description:	play DTMF tone
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
Result_t AUDIO_DRV_PlayTone(SpeakerTone_t tone_id, UInt32 duration)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_PlayTone() tone_id=%d duration=%d\r\n", tone_id, duration));

	if((tone_id&0x7fff) > SPEAKERTONE_ERROR){
		return AUDIO_INVALID_TONE_ID;
	}

	// start the tone, duration is forever (until it is being stopped)
	// DSP treats the 0 as the continuous DTMF duration, but API should
	// not since we use the duration to stop the tone.
#if (!defined(BSP_ONLY_BUILD))	
#if defined(FUSE_APPS_PROCESSOR)
	CAPI2_SPEAKER_StartTone(0, 0, tone_id, 0); 
#else
	SPEAKER_StartTone(tone_id, 0);
#endif
#endif

#if defined(ROXY_TONE_FEATURES)
	isToneRunning = TRUE; // 04012009 michael
#endif
	return RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_PlayGenericTone
//!
//! Description:	play generic tone
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
Result_t AUDIO_DRV_PlayGenericTone(
		Boolean	superimpose,
		UInt16	duration,
		UInt16	f1,
		UInt16	f2,
		UInt16	f3
		)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_PlayGenericTone() f1=%d\r\n", f1));

#if (!defined(BSP_ONLY_BUILD))
#if defined(FUSE_APPS_PROCESSOR)
	CAPI2_SPEAKER_StartGenericTone(0, 0, superimpose, 0, f1, f2, f3 );
#else
	SPEAKER_StartGenericTone(superimpose, 0, f1, f2, f3 );
#endif
#endif

#if defined(ROXY_TONE_FEATURES)
	isToneRunning = TRUE; // 04012009 michael
#endif
	return RESULT_OK;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_StopPlaytone
//!
//! Description:	stop play tone
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_StopPlayTone(void)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_StopPlayTone() \r\n"));

#if (!defined(BSP_ONLY_BUILD))
#if defined(FUSE_APPS_PROCESSOR)
	CAPI2_SPEAKER_StopTone(0, 0);
#else
	SPEAKER_StopTone();
#endif
#endif

#if defined(ROXY_TONE_FEATURES)
	isToneRunning = FALSE; // 04012009 michael
#endif

}
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SwitchVoiceMode
//!
//! Description:	switch voice mode
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetVoiceMode(AudioMode_t voiceMode)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetVoiceMode() voiceMode=%d\r\n", voiceMode));

	AUDIO_ASIC_SetAudioMode(voiceMode);
}
#if 0 //not sure if needed
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetInternalCTM
//!
//! Description:	set CTM for TTY
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------
//MST is MStream
//(from Jie) If CTM is not used, it should be in IDLE mode.
//Due to limited XRAM, DSP CTM shares XRAM with polyringer, MST, VPU AMR.
//Only one thing can run at a time. When CTM running, the other are disabled.
//------------------------------------------------------
void AUDIO_DRV_SetInternalCTM( Boolean enable )
{
	SharedMem_t *mem;
	mem = (SharedMem_t *)(SHAREDMEM_GetSharedMemPtr());

	if( enable )
	{
		//sprintf( Buf, "CTM ON");
		mem->shared_ctm_mode = CTMMODE_RESET;
		mem->shared_ctmBuff[0] = CTMMODE_NORMAL;
		//comment out the obsolete code
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_EFR, 0, 0, 0);	
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_AFS, 0, 0, 0);	
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_AHS, 0, 0, 0);	
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_CS1, 0, 0, 0);	
	}
	else
	{
		//sprintf( Buf, "CTM OFF");
		
		//To disable CTM, we need to set the mode to "CTMMODE_MODE1", similar to AT command 17,0. 
		//In CTMMODE_MODE1, ARM first runs ctm_adaptor_exit()then set mode to idle. The ctm_adaptor_exit() resets CTM. 
		//Please check hal_audio_access.c and set mode to CTMMODE_MODE1 to disable CTM.  --Hanks
		mem->shared_ctm_mode = CTMMODE_MODE1; //CTMMODE_IDLE;
		//comment out the obsolete code
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_EFR, 1, 0, 0);	
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_AFS, 1, 0, 0);	
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_AHS, 1, 0, 0);	
		//AUDIO_PostMsgToDSP(COMMAND_MST_FLAG_CS1, 1, 0, 0);	
	}
}
#endif
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetTonePathOn
//!
//! Description:	power up voice path for tone 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetTonePathOn(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];

	_DBG_A_(audioPrintf("AUDIO_DRV_SetTonePathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));	

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VSLOPGAIN, 0xa000, 0xa000);

	AUDIO_DRV_SetSpkDriverSelect(eSpkType, chan);//AUDIO_DRV_SetSpkDriverOn(eSpkType, chan);
#ifdef USE_DSP_AUDIO_ENABLE
#if defined(ROXY_TONE_FEATURES)
// 	audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0); // 04012009 michael
	if ((bStatusVoice == AUDIO_OFF) || (!isToneRunning))
		audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#else
	if ((bStatusVoice == AUDIO_OFF))
		audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#endif  //end of "#if defined(ROXY_TONE_FEATURES)"
#else	
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x20);
#endif
//#if defined(SS_2153) //for customer OEM/MMI		
	if (chan  == AUDIO_CHNL_BLUETOOTH)
		AUDIO_DRV_SetPCMOnOff(TRUE);
	else
		AUDIO_DRV_SetPCMOnOff(FALSE);
//#endif	//#if defined(SS_2153) //for customer OEM/MMI	
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, chan);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOn(AUDIO_DRV_ConvertChnlMode(chan));
#endif

	AUDIO_DRV_Set_DL_OnOff(AUDIO_ON);
						
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VMUT, 0x2, 0);
#if defined(FUSE_APPS_PROCESSOR)
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vslopgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vcfgr);
#else
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vslopgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vcfgr);
#endif	
}
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetTonePathOff
//!
//! Description:	power down voice path for tone
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetTonePathOff(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];
	
	_DBG_A_(audioPrintf("AUDIO_DRV_SetTonePathOff() bVoice=%d bAudio=%d bPoly=%d bCall=%d bAMR=%d bDTMF=%d bPcmBack=%d\r\n", 
		bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF, bStatusPcmBack));
	
	if(AUDIO_ON == (bStatusAudio||bStatusVoice||bStatusPoly))
	{
		_DBG_A_(audioPrintf("AUDIO_DRV_SetTonePathOff() bStatusAudio=%d,bStatusPoly=%d\r\n", bStatusAudio, bStatusPoly));
		if(bStatusVoice == AUDIO_OFF)
		{
			AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,0xa000);
			AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR,0x18);

			if (bStatusPcmBack == AUDIO_OFF)
			{
			AUDIO_DRV_Set_DL_OnOff(AUDIO_OFF);
#ifdef USE_DSP_AUDIO_ENABLE
					audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else			
					AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x00);
#endif	
			}
			
//#if defined(SS_2153) //for customer OEM/MMI			
			if (chan  == AUDIO_CHNL_BLUETOOTH)
				AUDIO_DRV_SetPCMOnOff(FALSE);
//#endif //#if defined(SS_2153) //for customer OEM/MMI			
		}
#ifndef USE_DSP_AUDIO_AMR_VOLUME
		else if(bStatusVoice == AUDIO_ON)
		{
			if(bStatusCall == AUDIO_ON)
			{
#if defined(FUSE_APPS_PROCESSOR)
				AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vslopgain);
				AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vcfgr);
#else
				AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vslopgain);
				AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vcfgr);
#endif
			}
		}
#endif //#ifndef USE_DSP_AUDIO_AMR_VOLUME		
		return;
	}

	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,0xa000);	
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR,0x18);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOff(AUDIO_DRV_ConvertChnlMode(chan));
#endif
	AUDIO_DRV_SetSpkDriverOff(eSpkType);

	if (bStatusPcmBack == AUDIO_OFF)
	{
	AUDIO_DRV_Set_DL_OnOff(AUDIO_OFF);
#ifdef USE_DSP_AUDIO_ENABLE
			audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else
			AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x00);
#endif
	}
	
//#if defined(SS_2153) //for customer OEM/MMI	
	if (chan  == AUDIO_CHNL_BLUETOOTH)
		AUDIO_DRV_SetPCMOnOff(FALSE);
//#endif //#if defined(SS_2153) //for customer OEM/MMI	

	if ((bStatusInputPath || bStatusPcmBack) == AUDIO_OFF)
		AUDIO_DRV_PowerDownD2C();

	return;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAMRPathOn
//!
//! Description:	power up voice path for AMR playback 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetAMRPathOn(AUDIO_CHANNEL_t chan)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAMRPathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	AUDIO_DRV_SetTonePathOn(chan);	
	bStatusAMR = AUDIO_ON;
	bStatusVoice = VOICE_PATH_STATUS;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAMRPathOff
//!
//! Description:	power down voice path for AMR playback 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetAMRPathOff(AUDIO_CHANNEL_t chan)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAMRPathOff() bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	bStatusAMR = AUDIO_OFF;
	bStatusVoice = VOICE_PATH_STATUS;
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAMRPathOff() bStatusVoice=%d\r\n", bStatusVoice));
	AUDIO_DRV_SetTonePathOff(chan);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAMRPathGain
//!
//! Description:	set AMR tone gain (volume) 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetAMRPathGain(UInt16 Gain)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAMRPathGain() gain=%d\r\n", Gain));

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VSLOPGAIN, MASK_SLOPGAIN_VALUE, Gain);
}

#ifdef USE_DSP_AUDIO_AMR_VOLUME
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAMRVolume
//!
//! Description:	set AMR tone volume 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetAMRVolume(UInt16 uSpkvol)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAMRVolume() uSpkvol=%d\r\n", uSpkvol));
	
	audio_control_dsp(SET_DSP_AMR_VOLUME, (UInt32)uSpkvol, 0, 0);
}
#endif //#ifdef USE_DSP_AUDIO_AMR_VOLUME

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_MuteTonePathOnOff
//!
//! Description:	mute operation of voice path for AMR, DTMF tone
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_MuteAMRPathOnOff(Boolean mute_on_off)
{
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VMUT, 0x2, mute_on_off<<1);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetDTMFPathOn
//!
//! Description:	power up voice path for AMR playback 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetDTMFPathOn(AUDIO_CHANNEL_t chan)
{
	static AUDIO_CHANNEL_t prev_chan = AUDIO_CHNL_RESERVE;
	
	_DBG_A_(audioPrintf("AUDIO_DRV_SetDTMFPathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	//only enable the tone path when voice path is off or chan is changed
#if defined(ROXY_TONE_FEATURES)
	if ((bStatusVoice == AUDIO_OFF) || (chan != prev_chan) || (isToneRunning))
#else
	if ((bStatusVoice == AUDIO_OFF) || (chan != prev_chan))
#endif
	{
		AUDIO_DRV_SetTonePathOn(chan);
		prev_chan = chan;
	}
	
	bStatusDTMF = AUDIO_ON;
	bStatusVoice = VOICE_PATH_STATUS;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetDTMFPathOff
//!
//! Description:	power down voice path for AMR playback 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetDTMFPathOff(AUDIO_CHANNEL_t chan)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetDTMFPathOff() bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	bStatusDTMF = AUDIO_OFF;
	bStatusVoice = VOICE_PATH_STATUS;
	_DBG_A_(audioPrintf("AUDIO_DRV_SetDTMFPathOff() bStatusVoice=%d\r\n", bStatusVoice));
	AUDIO_DRV_SetTonePathOff(chan);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetDTMFPathGain
//!
//! Description:	set DTMF tone gain
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetDTMFPathGain(UInt16 Gain)
{
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VSLOPGAIN, MASK_SLOPGAIN_VALUE, Gain);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetToneVolume
//!
//! Description:	set tone volume and convert to scale factor on cp
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetToneVolume(AudioToneType_en_t eToneType, speakerVol_t uSpkvol)
{
	audio_control_dsp(SET_DSP_TONE_VOLUME, (UInt32)eToneType, (UInt32)uSpkvol, 0);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_MuteTonePathOnOff
//!
//! Description:	mute operation of voice path for AMR, DTMF tone
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_MuteDTMFPathOnOff(Boolean mute_on_off)
{
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VMUT, 0x2, mute_on_off<<1);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioPathOn
//!
//! Description:	powerup DAC audio path 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetAudioPathOn(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAudioPathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));	
	
	// we need to disable and re-enable it later to handle the case that the FIFO is full
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x0003, 0x0000); // symmeric with POLY path

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ALSLOPGAIN, 0xa000, 0xa000);
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ARSLOPGAIN, 0xa000, 0xa000);
	
	AUDIO_DRV_SetSpkDriverSelect(eSpkType, chan);//AUDIO_DRV_SetSpkDriverOn(eSpkType, chan);
	AUDIO_DRV_SetAudioChannel(chan); // 01062009 michael

#if 1 //defined (SS_2153)	//This is different from RTOS,without below section, PCM playback won't work
	AUDIO_WriteDSP(DSP_AUDIO_REG_AFIFOCTRL, 0xbfc0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_AFIFOCTRL, 0x4240);

 	//flush the audio fifo for out of sync case if needed.
	//this is needed to flush the fifo at least once after audvoc enabled with new seq and new patch
	//without new patch and new seq, the lowerest working number so far is 65 (reduced from 128)
	audio_control_generic(FLUSH_STEREO_AIFIFODATA, 0x43, 0, 0); //128
#endif // (SS_2153)

	// select IIR filter, enable audio path
	program_FIR_IIR_filter((UInt16)chan);

	OSTASK_Sleep(5); // secure time to write block for filter
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x0007, 0x0005);
 	
	// Need this sleep after select IIR filter, otherwise we can hear pop noise
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, chan);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOn(AUDIO_DRV_ConvertChnlMode(chan));
#endif

	bStatusAudio = AUDIO_ON;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioPathOff
//!
//! Description:	powerdown DAC audio path 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetAudioPathOff(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAudioPathOff() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	bStatusAudio = AUDIO_OFF;
	if(AUDIO_ON == (bStatusVoice||bStatusPoly))
	{
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x0003, 0x0002);
		return;
	}

#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOff(AUDIO_DRV_ConvertChnlMode(chan));
#endif
	AUDIO_DRV_SetSpkDriverOff(eSpkType);
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x0003, 0x0002);

	if ((bStatusInputPath || bStatusPcmBack) == AUDIO_OFF)
		AUDIO_DRV_PowerDownD2C();
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioPathGain
//!
//! Description:	set audio gain (volume) 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetAudioPathGain(UInt16 Lgain, UInt16 Rgain)
{
        AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ALSLOPGAIN, MASK_SLOPGAIN_VALUE, Lgain);
        AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ARSLOPGAIN, MASK_SLOPGAIN_VALUE, Rgain);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioPathGain
//!
//! Description:	set audio gain (volume) 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetAudioSampleRate(AudioSampleRate_en_t sampleRate)
{
	UInt16		regValue = 0x5;	//default sampling rate is 48KHz

	switch(sampleRate)
	{
		case AUDIO_SAMPLERATE_8000:
			regValue = 0;
			break;

		case AUDIO_SAMPLERATE_12000:
			regValue = 0x1;
			break;
			
		case AUDIO_SAMPLERATE_16000:
			regValue = 0x2;
			break;
			
		case AUDIO_SAMPLERATE_24000:
			regValue = 0x3;
			break;
			
		case AUDIO_SAMPLERATE_32000:
			regValue = 0x4;
			break;
			
		case AUDIO_SAMPLERATE_48000:
			regValue = 0x5;
			break;
			
		case AUDIO_SAMPLERATE_11030:
			regValue = 0x6;
			break;
			
		case AUDIO_SAMPLERATE_22060:
			regValue = 0x7;
			break;
			
		case AUDIO_SAMPLERATE_44100:
			regValue = 0x8;
			break;

		default:
			break;
	}
	
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x3C00, regValue<<10);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioPathGain
//!
//! Description:	set audio gain (volume) 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetIIS2Audio(Boolean DirectPathOnOff)
{	
	if(DirectPathOnOff)
		//disable audio interrupt, enable I2S direct path
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x001C, 0x001C);
	else
		//enable audio interrupt, disable I2S direct path
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x001C, 0x0004);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	audvoc_mute_audio_path
//!
//! Description:	mute audio path
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_MuteAudioPathOnOff(Boolean mute_on_off)
{
        AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x2, mute_on_off<<1);
}
 
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPolyPathOn
//!
//! Description:	powerup poly path 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetPolyPathOn(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];

	_DBG_A_(audioPrintf("AUDIO_DRV_SetPolyPathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));
	
//	AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 1, 0, 0);
	// we need to disable and re-enable it later to handle the case that the FIFO is full
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0003, 0x0000);


	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_PLSLOPGAIN, 0xa000, 0xa000);
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_PRSLOPGAIN, 0xa000, 0xa000);

	AUDIO_DRV_SetSpkDriverSelect(eSpkType, chan);//AUDIO_DRV_SetSpkDriverOn(eSpkType, chan);
#if 1  // defined(SS_2153)	//This is different from RTOS,without below section, PCM playback won't work
	AUDIO_WriteDSP(DSP_AUDIO_REG_PFIFOCTRL, 0xbfc0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PFIFOCTRL, 0x4240);

	//flush the poly fifo for out of sync case if needed.
	//this is needed to flush the fifo at least once after ply enabled with new seq and new patch
	//without new patch and new seq, the lowerest working number so far is 65 (reduced from 128)
	audio_control_generic(FLUSH_POLY_PIFIFODATA, 0x43, 0, 0); //128
#endif //#if defined(SS_2153)

	// select IIR like PCM path
	program_poly_FIR_IIR_filter((UInt16)chan);

	OSTASK_Sleep(5); // secure time to write block for filter
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0007, 0x0005);

	/* this is needed for the case of amr playback followed by mid playback */
	//AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 1, 0, 0);
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, chan);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOn(AUDIO_DRV_ConvertChnlMode(chan));
#endif

	bStatusPoly = AUDIO_ON;
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPolyPathOff
//!
//! Description:	powerdown poly path 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetPolyPathOff(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];
	_DBG_A_(audioPrintf("AUDIO_DRV_SetPolyPathOff() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	bStatusPoly = AUDIO_OFF;
	
	if(AUDIO_ON == (bStatusAudio||bStatusVoice))
	{
		/* this is needed for the case of amr playback followed by mid playback */
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0003, 0x0002);
//		AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 0, 0, 0);
		return;
	}

	//AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_DOWN);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOff(AUDIO_DRV_ConvertChnlMode(chan));
#endif

	AUDIO_DRV_SetSpkDriverOff(eSpkType);
	/* this is needed for the case of amr playback followed by mid playback */
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0003, 0x0002);

	if ((bStatusInputPath || bStatusPcmBack)== AUDIO_OFF)
		AUDIO_DRV_PowerDownD2C();

//	AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPolyPathGain
//!
//! Description:	set stereo poly  gain (volume) 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetPolyPathGain(UInt16 Lgain, UInt16 Rgain)
{
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_PLSLOPGAIN, MASK_SLOPGAIN_VALUE, Lgain);
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_PRSLOPGAIN, MASK_SLOPGAIN_VALUE, Rgain);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_MutePolyPathOnOff
//!
//! Description:	mute poly path
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_MutePolyPathOnOff(Boolean mute_on_off)
{
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x2, mute_on_off<<1);
}
 

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioMixerGain
//!
//! Description:	set mixer gain (volume)
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetAudioMixerGain(int Lgain, int Rgain)
{
#if defined(SS_2153)
	Lgain = (UInt16)AUDIO_DRV_SelectCustomGain(CUSTOM_MIXER_GAIN, (UInt16) Lgain);
	Rgain = (UInt16)AUDIO_DRV_SelectCustomGain(CUSTOM_MIXER_GAIN, (UInt16) Rgain);
#endif // SS_2153
	_DBG_A_(audioPrintf("AUDIO_DRV_SetAudioMixerGain() Lgain=%d Rgain=%d\r\n", Lgain, Rgain));

	AUDIO_WriteDSP(DSP_AUDIO_REG_APVLMXGAIN, Lgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_APVRMXGAIN, Rgain);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioEquGain
//!
//! Description:	set audio equ gain
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetAudioEquGain( Int16 *gain )
{
    UInt16 i, srcAddr;

	srcAddr = (UInt16) DSP_AUDIO_REG_AEQPATHGAIN1; //0xe7e1
	for (i=0; i<NUM_OF_EQU_BANDS; i++)
		AUDIO_WriteDSP(srcAddr+i, ((gain[i]>>6)&0x1ff)|BIT15_MASK);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPolyEquGain
//!
//! Description:	set poly equ gain 
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetPolyEquGain( Int16 *gain )
{
	UInt16 i, srcAddr;
	
	srcAddr = (UInt16) DSP_AUDIO_REG_PEQPATHGAIN1; //0xe7bc
	for (i=0; i<NUM_OF_EQU_BANDS-1; i++)
		AUDIO_WriteDSP(srcAddr+i, ((gain[i]>>6)&0x1ff)|BIT15_MASK);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PEQPATHGAIN5, ((gain[i]>>6)&0x1ff)|BIT15_MASK);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPcmReadBackPathOn
//!
//! Description:	powerup PCM readback path, for arm-nb, vpu
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetPcmReadBackPathOn(void)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetPcmBackPathOn() bStatusVoice=%d\r\n", bStatusVoice));

	bStatusPcmBack = AUDIO_ON;

	if (bStatusVoice)
		return;

	// Need to enable DSP audio
#ifdef USE_DSP_AUDIO_ENABLE
	audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#else	
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x20);
#endif

	// Need to enable VPU downlink(decoder output)
	AUDIO_DRV_Set_DL_OnOff(AUDIO_ON);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPcmReadBackPathOff
//!
//! Description:	powerdown PCM readback path, for arm-nb, vpu
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetPcmReadBackPathOff(void)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_SetPcmBackPathOff() bStatusVoice=%d\r\n", bStatusVoice));

	bStatusPcmBack = AUDIO_OFF;

	if (bStatusVoice)
		return;
	
#ifdef USE_DSP_AUDIO_ENABLE
	audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x0);
#endif
	AUDIO_DRV_Set_DL_OnOff(AUDIO_OFF);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_InitReg
//!
//! Description:	initialize BB register(s )
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_InitReg(void)
{
	AUDIO_DRV_SetBgtcsp();

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)	
//	if ( audio_shutdown_sema == NULL ) {
//		audio_shutdown_sema = OSSEMAPHORE_Create(0, OSSUSPEND_PRIORITY);
//	}
#endif

	//audio was turned on in dsp for 2153 e1, turned it off
#ifdef USE_DSP_AUDIO_ENABLE
	audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x0);
#endif

	AUDIO_DRV_SetHifiDacMask();

	AUDIO_DRV_SetAudioLRChannel(LOUT_RIN, ROUT_LIN);
	AUDIO_DRV_SetPolyLRChannel(LOUT_LIN, ROUT_LIN);

	AUDIO_WriteDSP(DSP_AUDIO_REG_AIFIFODATA0, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_AIFIFODATA1, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_AIFIFODATA2, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PIFIFODATA0, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PIFIFODATA1, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PIFIFODATA2, 0x0);

	AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_DEFAULT);
#ifdef POWEROFF_CONTROL
	bb_audio_pwrctrl_q= OSQUEUE_Create( QUEUESIZE_HAL_AUDIO, sizeof( UInt8 ), OSSUSPEND_PRIORITY );
	audioPowerOffControl_task = OSTASK_Create( audioPowerOffControl_TaskEntry, (TName_t)"audioPowerOffControl", TASKPRI_AUDIO, STACKSIZE_AUDIO );
	if (audioPowerOffControl_task == NULL)
		_DBG_A_(audioPrintf("AUDIO_DRV_InitReg: audioPowerOffControl_task create fail!!\r\n"));	
#endif	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR[0-3]=%08x %08x %08x %08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG,*(volatile UInt32 *)ANACR1_REG,*(volatile UInt32 *)ANACR2_REG,*(volatile UInt32 *)ANACR3_REG));

}



/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetMicBiasOnOff
//!
//! Description:	turn on/off main mic bias
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetMicBiasOnOff(Boolean Bias_on_off)
{
	return;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAuxMicBiasOnOff
//!
//! Description:	turn on/off main aux mic bias
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetAuxMicBiasOnOff(Boolean Bias_on_off, AsicAuxMicBiasMode_t bias_mode)
{
	if(Bias_on_off && (bias_mode == AuxMicBias_ON_FPMODE))
		*(volatile UInt32 *) AUXMIC_AUXEN_REG |=BIT00_MASK;
	else
		*(volatile UInt32 *) AUXMIC_AUXEN_REG &=~(BIT00_MASK);
	return;
}




//
// HQ Audio Recording.........
//
// Temporarily put here, Should have a seperate file for this code.

#ifdef HAL_AUDIO_HQ_PATH_ENABLED

// These will go to sysparm.txt.
const static UInt16   HqAdcIIRCoeff[] = {
    //=========================================
    //==== write coeff in for normal case test
    //Biquad 1
    0x7438,    //A2 -3016
    0x1b61,    //A1  7009
    0x0785,    //B2  1925
    0x0a8f,    //B1  2703
    0x0785,    //B0  1925
    //Biquad 2
    0x7322,    //A2 -3294
    0x1c1d,    //A1  7197
    0x06f2,    //B2  1778
    0x7e36,    //B1 -0458
    0x06f2,    //B0  1778
    //Biquad 3
    0x7132,    //A2 -3790
    0x1da7,    //A1  7591
    0x07da,    //B2  2010
    0x7989,    //B1 -1655
    0x07da,    //B0  2010
    //Biquad 4
    0x748e,    //A2 -2930
    0x1ae6,    //A1  6886
    0x0800,    //B2  2048
    0x08ed,    //B1  2285
    0x0800,    //B0  2048
    //Biquad 5
    0x73a7,    //A2 -3161
    0x1ad9,    //A1  6873
    0x0800,    //B2  2048
    0x79f7,    //B1 -1545
    0x0800,    //B0  2048
    //Biquad 6
    0x7269,    //A2 -3479
    0x1ad3,    //A1  6867
    0x0800,    //B2  2048
    0x758f,    //B1 -2673
    0x0800,    //B0  2048
    //Biquad 7
    0x714b,    //A2 -3765
    0x1aed,    //A1  6893
    0x0800,    //B2  2048
    0x7421,    //B1 -3039
    0x0800,    //B0  2048
    //Biquad 8
    0x7067,    //A2 -3993
    0x1b40,    //A1  6976
    0x0800,    //B2  2048
    0x73a7,    //B1 -3161
    0x0800,    //B0  2048
    //Biquad 9
    0x7857,    //A2 -1961
    0x6a80,    //A1 -5504
    0x0b4c,    //B2  2892
    0x1684,    //B1  5764
    0x0b4c,    //B0  2892
    //Biquad 10
    0x72b4,    //A2 -3404
    0x64d8,    //A1 -6952
    0x0cc8,    //B2  3272
    0x190f,    //B1  6415
    0x0cc8,    //B0  3272
};

// Enable ADC HQ audio recording path
void HAL_Audio_EnablePath_ADC_HQ(Audio_Path_ADC_HQ_s *AudioPath_AdcHq, UInt32 audioID, UInt32 channels, UInt32 inputMic)
{
	UInt16	regValue;
	UInt8	res = FALSE;
	UInt32	index;
	
	// track the number of users 
	AudioPath_AdcHq->userCount++;

	if (AudioPath_AdcHq->pathState == AUDIO_PATH_DEVICE_ON)
		return;


	// mono/stereo. Can be configured later. If mono, only use left channel.
	AudioPath_AdcHq->channels = channels;

	// store input microphone
	AudioPath_AdcHq->inputMic = inputMic;


	TRACE_Printf_Sio("HAL_Audio_EnablePath_ADC_HQ(): channels=%d, inputMic=%d\r\n", channels, inputMic);

	// (1) power on Microphone
	switch (inputMic)
	{
		// not working yet
		case AUDIO_MAIN_DIGITAL_MIC:
			break;

		// not working yet
		case AUDIO_AUX_DIGITAL_MIC:
			break;
	
		// HQ recording should always use Digital mic. 
		// This(analog main mic  is only used to test the HQ path before digital mic works
		case AUDIO_MAIN_MIC:
			_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
				__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));
			
			*(volatile UInt32 *)ANACR3_REG = 0x00002800;

			_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
				__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));
			break;
		
		case AUDIO_AUX_MIC:
			_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
				__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));
			
			*(volatile UInt32 *)ANACR3_REG = 0x00009800;
			
			_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
				__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));
			break;
			
		default:
			break;
	}

	// (2) Enable DSP HQ audio processing
	if (channels == 2)
	{
		 // stereo
		AUDIO_PostMsgToDSP (COMMAND_RESET_HQ_ADC, 0x0003, 0x0000, 0x0000);
	}
	else if (channels == 1)
	{
		// mono
		AUDIO_PostMsgToDSP (COMMAND_RESET_HQ_ADC, 0x0002, 0x0000, 0x0000);
	}
	else
	{
		TRACE_Printf_Sio("HAL_Audio_ADC_HQ_Record_EnablePath(): channels=%d, Should only be 1 or 2.\r\n", channels);
		return;
	}

	// (3) Configure HQ ADC paths IIR Coefficients, always enable left path
	
	// path IIR coefficients
	AUDIO_WriteDSP (DSP_AUDIO_REG_AUDIOINCM_ADDR, 0x0000);
	// left path
	for (index = 0; index < sizeof(HqAdcIIRCoeff)/sizeof(UInt16); index++)
	{
		AUDIO_WriteDSP (DSP_AUDIO_REG_AUDIOINCM_DATA, HqAdcIIRCoeff[index]);
	}
	// right path, the same value as left path
	if (channels == 2)
	{
		for (index = 0; index < sizeof(HqAdcIIRCoeff)/sizeof(UInt16); index++)
		{
			AUDIO_WriteDSP (DSP_AUDIO_REG_AUDIOINCM_DATA, HqAdcIIRCoeff[index]);
		}
	}

	// (4) Enable HQ ADC paths
#ifdef USE_DSP_AUDIO_ENABLE
#if defined(ROXY_TONE_FEATURES)
// 	audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0); // 04012009 michael
	if ((bStatusVoice == AUDIO_OFF) || (!isToneRunning))
		audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#else
	audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#endif 

#else	
	AUDIO_BitwiseWriteDSP (DSP_AUDIO_REG_AMCR, 0x0020, 0x0020);
#endif
	 // These may be different values for different version!!!
#define DSP_AUDIO_ADC_CONTROL_LEFT	0x0260
#define DSP_AUDIO_ADC_CONTROL_RIGHT	0x8260
#define DSP_AUDIO_INPATH_CONTROL_NORMAL 0x4821
	// left path ADC control
	regValue = DSP_AUDIO_ADC_CONTROL_LEFT;
	AUDIO_WriteDSP (DSP_AUDIO_REG_ADCCONTROL, regValue);
	// left path inpath control
	regValue = DSP_AUDIO_INPATH_CONTROL_NORMAL;
	regValue |= ((inputMic) << 1);
	AUDIO_WriteDSP (DSP_AUDIO_REG_AUDIOINPATH_CTRL, regValue);

	if (channels == 2)
	{
		// right path ADC control
		regValue = DSP_AUDIO_ADC_CONTROL_RIGHT;
		AUDIO_WriteDSP (DSP_AUDIO_REG_ADCCONTROL, regValue);
		// right path inpath control
		regValue = DSP_AUDIO_INPATH_CONTROL_NORMAL;
		regValue |= ((inputMic) << 1);
		AUDIO_WriteDSP (DSP_AUDIO_REG_AUDIOINPATH_CTRL, regValue);	
	}

	// (5) enable DSP to respond HQ ADC interrupt. No register #define in .h???
	//AUDIO_BitwiseWriteDSP (DSP_INTC_IMR, DSP_INTC_IMR_AUDINT, DSP_INTC_IMR_AUDINT);
	AUDIO_BitwiseWriteDSP (0xE004, 0x0800, 0x0800);

	// store state
	AudioPath_AdcHq->pathState = AUDIO_PATH_DEVICE_ON;
}

// Disable ADC HQ audio recording path
void HAL_Audio_DisablePath_ADC_HQ(Audio_Path_ADC_HQ_s *AudioPath_AdcHq)
{
	if (AudioPath_AdcHq->pathState == AUDIO_PATH_DEVICE_OFF)
		return;

	// track the number of users 
	AudioPath_AdcHq->userCount--;
	
	// still have other users need it. don't disable.
	if (AudioPath_AdcHq->userCount > 0)
		return;

	TRACE_Printf_Sio("HAL_Audio_DisablePath_ADC_HQ(): \r\n");

	//AUDIO_BitwiseWriteDSP (0xE004, 0x0800, 0x0000);
	
	// select and disable left path
	AUDIO_WriteDSP (DSP_AUDIO_REG_ADCCONTROL, 0x0000);
	AUDIO_WriteDSP (DSP_AUDIO_REG_AUDIOINPATH_CTRL, 0x0000);
	// select and disable right path
	AUDIO_WriteDSP (DSP_AUDIO_REG_ADCCONTROL, 0x8000);
	AUDIO_WriteDSP (DSP_AUDIO_REG_AUDIOINPATH_CTRL, 0x0000);
	// clear interrupt status registers
	//AUDIO_BitwiseWriteDSP (DSP_AUDIO_REG_AUDIOINFIFO_STATUS, 0x0010, 0x0000);
	//AUDIO_BitwiseWriteDSP (DSP_AUDIO_REG_AUDVOC_ISR, 0x0008, 0x0000);
	// disable Audio path
#ifdef USE_DSP_AUDIO_ENABLE
	audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else	
	AUDIO_BitwiseWriteDSP (DSP_AUDIO_REG_AMCR, 0x0020, 0x0000);
#endif	
	// reset DSP HQ ADC processing
	AUDIO_PostMsgToDSP (COMMAND_RESET_HQ_ADC, 0x0000, 0x0000, 0x0000);


	// power off MIC????
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));

	*(volatile UInt32 *)ANACR3_REG = 0x00000000;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR3=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR3_REG));

	// store state
	AudioPath_AdcHq->pathState = AUDIO_PATH_DEVICE_OFF;
}

#endif // #ifdef HAL_AUDIO_HQ_PATH_ENABLED

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetBgtcsp
//!
//! Description:	set anacr0 bgtcsp
//!
//! Notes: done once
//!
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetBgtcsp(void)
{
	AUDIO_ANACR0_un_t TempReg;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.bgtcsp = 0x6; //3; the default value should be [8:6]=110 per latest datasheet
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));	
}

//======================================================================
//Functions used for bit by bit control on ANACR0 and ANACR1 per ASIC requirement for 2153 E1
//pop fix sequence
//======================================================================
void AUDIO_DRV_SetHifiDacMask(void)
{
	AUDIO_ANACR1_un_t TempReg;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR1=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR1_REG));
	
	TempReg.REG_ANACR1 = ANACR1_CTRL->HAL_ANACR1.REG_ANACR1;
	TempReg.FLD_ANACR1.hifidac_pwd_mask = 0x1;
	ANACR1_CTRL->HAL_ANACR1.REG_ANACR1 = TempReg.REG_ANACR1;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR1=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR1_REG));	

}

void AUDIO_DRV_PowerUpLeftDrv(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvL = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}
void AUDIO_DRV_PowerUpRightDrv(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvR = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

void AUDIO_DRV_PowerUpLeftRightDrv(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvL = 0x0;
	TempReg.FLD_ANACR0.pwrdndrvR = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

void AUDIO_DRV_PowerDownLeftDrv(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvL = 0x1;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}
void AUDIO_DRV_PowerDownRightDrv(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvR = 0x1;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

void AUDIO_DRV_PowerDownLeftRightDrv(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndrvL = 0x1;
	TempReg.FLD_ANACR0.pwrdndrvR = 0x1;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

void AUDIO_DRV_PowerUpLeftDac(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndacL = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}
void AUDIO_DRV_PowerUpRightDac(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndacR = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}

void AUDIO_DRV_PowerUpLeftRightDac(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndacL = 0x0;
	TempReg.FLD_ANACR0.pwrdndacR = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}

void AUDIO_DRV_PowerDownLeftDac(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndacL = 0x1;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}
void AUDIO_DRV_PowerDownRightDac(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndacR = 0x1;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}

void AUDIO_DRV_PowerDownLeftRightDac(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdndacL = 0x1;
	TempReg.FLD_ANACR0.pwrdndacR = 0x1;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}

void AUDIO_DRV_PowerUpRef(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdnref = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}
void AUDIO_DRV_PowerUpD2C(void)
{
	AUDIO_ANACR0_un_t TempReg;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));
		
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdnd2c = 0x0;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));

}

void AUDIO_DRV_PowerDownRef(void)
{
	AUDIO_ANACR0_un_t TempReg;	
	TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
	TempReg.FLD_ANACR0.pwrdnref = 0x1;
	ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;

}
void AUDIO_DRV_PowerDownD2C(void)
{
        AUDIO_ANACR0_un_t TempReg;
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));	
#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	audio_control_dsp(CHECK_DSP_AUDIO, 0, 0, 0);
	//wait until dsp finished turning off the audio
	//OSSEMAPHORE_Obtain(audio_shutdown_sema, 20 );
#endif
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pwrdnd2c = 0x1;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR0=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR0_REG));	
}

void AUDIO_DRV_SetPdOffsetGenLR(void)
{
	AUDIO_ANACR1_un_t TempReg;	
	TempReg.REG_ANACR1 = ANACR1_CTRL->HAL_ANACR1.REG_ANACR1;
	TempReg.FLD_ANACR1.pdoffsetgenLR = 0x3;
	ANACR1_CTRL->HAL_ANACR1.REG_ANACR1 = TempReg.REG_ANACR1;
}

void AUDIO_DRV_ClearPdOffsetGenLR(void)
{
	AUDIO_ANACR1_un_t TempReg;	
	TempReg.REG_ANACR1 = ANACR1_CTRL->HAL_ANACR1.REG_ANACR1;
	TempReg.FLD_ANACR1.pdoffsetgenLR = 0x0;
	ANACR1_CTRL->HAL_ANACR1.REG_ANACR1 = TempReg.REG_ANACR1;
}

void AUDIO_DRV_SetPwrmosGateCntrlLR(void)
{
	AUDIO_ANACR1_un_t TempReg;	
	TempReg.REG_ANACR1 = ANACR1_CTRL->HAL_ANACR1.REG_ANACR1;
	TempReg.FLD_ANACR1.pwrmos_gatecntrlLR = 0x3;
	ANACR1_CTRL->HAL_ANACR1.REG_ANACR1 = TempReg.REG_ANACR1;
}

void AUDIO_DRV_ClearPwrmosGateCntrlLR(void)
{
	AUDIO_ANACR1_un_t TempReg;	
	TempReg.REG_ANACR1 = ANACR1_CTRL->HAL_ANACR1.REG_ANACR1;
	TempReg.FLD_ANACR1.pwrmos_gatecntrlLR = 0x0;
	ANACR1_CTRL->HAL_ANACR1.REG_ANACR1 = TempReg.REG_ANACR1;
}

void AUDIO_DRV_SetRampRefEn(void)
{
	AUDIO_ANACR1_un_t TempReg;
	TempReg.REG_ANACR1 = ANACR1_CTRL->HAL_ANACR1.REG_ANACR1;
	TempReg.FLD_ANACR1.rampref_en = 0x1;
	ANACR1_CTRL->HAL_ANACR1.REG_ANACR1 = TempReg.REG_ANACR1;
}
void AUDIO_DRV_SetSTAudioPath(Boolean mode)
{
	if(mode)
		bStatusSTAudio = TRUE;
	else
		bStatusSTAudio = FALSE;
}
Boolean AUDIO_DRV_GetSTAudioPath(void)
{
	return bStatusSTAudio;
}
void AUDIO_DRV_SetAudioChannel(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];

	switch(eSpkType)
	{
		case LEFT_400MW:
		case RIGHT_400MW:
			if(AUDIO_DRV_GetSTAudioPath())//st
				AUDIO_DRV_SetAudioLRChannel(LOUT_R_L, ROUT_R_L);
			else
				AUDIO_DRV_SetAudioLRChannel(LOUT_LIN, ROUT_LIN);
			break;
		default:
			if(AUDIO_DRV_GetSTAudioPath())//st
				AUDIO_DRV_SetAudioLRChannel(LOUT_RIN, ROUT_LIN);
			else
				AUDIO_DRV_SetAudioLRChannel(LOUT_LIN, ROUT_LIN);
			break;
	}
}
void AUDIO_DRV_SetSTPolyPath(Boolean mode)
{
	if(mode)
		bStatusSTPoly = TRUE;
	else
		bStatusSTPoly = FALSE;
}

Boolean AUDIO_DRV_GetSTPolyPath(void)
{
	return bStatusSTPoly;
}

void AUDIO_DRV_SetPolyChannel(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];

	switch(eSpkType)
	{
		case LEFT_400MW:
		case RIGHT_400MW:
			if(AUDIO_DRV_GetSTPolyPath())//st
				AUDIO_DRV_SetPolyLRChannel(LOUT_R_L, ROUT_R_L);
			else
				AUDIO_DRV_SetPolyLRChannel(LOUT_LIN, ROUT_LIN);
			break;
		default:
			if(AUDIO_DRV_GetSTPolyPath())//st
				AUDIO_DRV_SetPolyLRChannel(LOUT_RIN, ROUT_LIN);
			else
				AUDIO_DRV_SetPolyLRChannel(LOUT_LIN, ROUT_LIN);
			break;
	}
}

#if defined(SS_2153)
void AUDIO_DRV_SetCustomGain(UInt16 spk_pga, UInt16 mic_pga, UInt16 mixer_gain)
{
	custom_gain_set[CUSTOM_SPK_PGA] = spk_pga;
	custom_gain_set[CUSTOM_MIC_PGA] = mic_pga;
	custom_gain_set[CUSTOM_MIXER_GAIN] = mixer_gain;
}

#if defined(ROXY_AUD_CTRL_FEATURES)
UInt16 AUDIO_DRV_GetCustomGain(CUSTOM_GAIN_t gain_index) // 02182009 michael
{
	return custom_gain_set[gain_index];
}
#endif

static UInt16 AUDIO_DRV_SelectCustomGain(CUSTOM_GAIN_t mode, UInt16 sysparm_val)
{
	return ((custom_gain_set[mode] != REF_SYSPARM) ? custom_gain_set[mode] : sysparm_val);
}
#endif // SS_2153

#if defined(ROXY_AUD_CTRL_FEATURES)
Boolean AudioExtDev_OnOff(void)
{
	if((bStatusPoly||bStatusAudio||bStatusVoice))		
		bStatusEXIT_DEV = TRUE;
	else
		bStatusEXIT_DEV = FALSE;
	
	return bStatusEXIT_DEV;
}
#endif

#if defined(ROXY_TONE_FEATURES)
/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPolyKeytonePathOn
//!
//! Description:	powerup poly path for keytone only
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetPolyKeytonePathOn(AUDIO_CHANNEL_t chan)
{
	int counter = 0;
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];
	
	_DBG_A_(audioPrintf("AUDIO_DRV_SetPolyKeytonePathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

//	AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 1, 0, 0); // 10162009 michael
	// we need to disable and re-enable it later to handle the case that the FIFO is full
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0003, 0x0000); 

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_PLSLOPGAIN, 0xa000, 0xa000);
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_PRSLOPGAIN, 0xa000, 0xa000);

	AUDIO_DRV_SetSpkDriverSelect(eSpkType, chan);//AUDIO_DRV_SetSpkDriverOn(eSpkType, chan);
	AUDIO_DRV_SetPolyChannel(chan); // 01062009 michael

	AUDIO_WriteDSP(DSP_AUDIO_REG_PFIFOCTRL, 0xbfc0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PFIFOCTRL, 0x4240);

	audio_control_generic(FLUSH_POLY_PIFIFODATA, 0x43, 0, 0); // 128->65

	// select IIR like PCM path
	program_poly_FIR_IIR_filter((UInt16)chan);

	OSTASK_Sleep(5); // secure time to write block for filter
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0007, 0x0005);

	/* this is needed for the case of amr playback followed by mid playback */
	// Need this sleep after select IIR filter, otherwise we can hear pop noise
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, chan);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) &&(defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOn(AUDIO_DRV_ConvertChnlMode(chan));
#endif

	bStatusPoly = AUDIO_ON;
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetPolyKeytonePathOff
//!
//! Description:	powerdown poly path for keytone only
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetPolyKeytonePathOff(AUDIO_CHANNEL_t chan)
{
	AUDIO_DRV_Spk_Output_t eSpkType = SpeakerConfig[chan];

	_DBG_A_(audioPrintf("AUDIO_DRV_SetPolyKeytonePathOff() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
		chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	bStatusPoly = AUDIO_OFF;
	if(AUDIO_ON == (bStatusAudio||bStatusVoice))
	{
		/* this is needed for the case of amr playback followed by mid playback */
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0003, 0x0002);
//		AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 0, 0, 0);
		return;
	}

#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && (defined (__AUDIO_AMP_MAX9877__) || defined (__AUDIO_AMP_NCP2704__)) && !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOff(AUDIO_DRV_ConvertChnlMode(chan));
#endif
	AUDIO_DRV_SetSpkDriverOff(eSpkType);
	/* this is needed for the case of amr playback followed by mid playback */
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_POLYAUDMOD, 0x0003, 0x0002);
	AUDIO_DRV_PowerDownD2C();
//	AUDIO_PostFastMsgToDSP(FAST_COMMAND_DISABLE_DSP_SLEEP, 0, 0, 0);
}
#endif

#ifdef SS_2153
// just to allow Brooklyn to control AUX MIC bias synchronously with accessory plugging.
void AUDIO_DRV_SetAuxMicBiasConfig(Boolean on_off, AsicAuxMicBiasMode_t BiasMode) // 08052009 michael
{
	if (on_off)
		*(volatile UInt32 *)AUXMIC_CMC_REG |=BIT00_MASK;
	else
		*(volatile UInt32 *)AUXMIC_CMC_REG &=~(BIT00_MASK);

	// AuxMicBias_OFF = 0,
	// AuxMicBias_ON_LPMODE, //0.4V
	// AuxMicBias_ON_FPMODE, //2.1V
	AUDIO_DRV_SetAuxMicBiasOnOff(on_off, BiasMode);

	switch (BiasMode)
	{
		case AuxMicBias_ON_FPMODE:
			AUDIO_DRV_PowerControlMIC(FALSE, FALSE);
			break;
		case AuxMicBias_ON_LPMODE:
			AUDIO_DRV_PowerControlMIC(TRUE, FALSE);
			break;
		case AuxMicBias_OFF:
			AUDIO_DRV_PowerControlMIC(TRUE, TRUE);
			break;
		default:
			break;
	}
}

void AUDIO_DRV_PowerControlMIC(Boolean mic_off, Boolean ldo_off)
{
	AUDIO_ANACR2_un_t TempReg; 				 // get a local copy of register structure

	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR2=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR2_REG));
	
	TempReg.REG_ANACR2 = ANACR2_CTRL->HAL_ANACR2.REG_ANACR2;

	if (mic_off)
		TempReg.FLD_ANACR2.mic_voice_pwrdn = 0x1;
	else
		TempReg.FLD_ANACR2.mic_voice_pwrdn = 0x0;
	if (ldo_off)
		TempReg.FLD_ANACR2.ldo_pwrdn = 0x1;
	else
		TempReg.FLD_ANACR2.ldo_pwrdn = 0x0;

	ANACR2_CTRL->HAL_ANACR2.REG_ANACR2 = TempReg.REG_ANACR2;
	
	_DBG_A_(AUD_TRACE_DETAIL("[audio control][func]=%s [line]=%d ANACR2=%08x\r\n", 
		__FUNCTION__, __LINE__, *(volatile UInt32 *)ANACR2_REG));
}
#endif // SS_2153

void AUDIO_DRV_CtrlAnalogLoopback(Boolean v_onoff)
{
	_DBG_A_(audioPrintf("AUDIO_DRV_CtrlAnalogLoopback() on_off=%d\r\n", v_onoff));

	// analog loopback should be turned off when it's not in use, apparently.
	if (v_onoff)
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ADCCONTROL, 0x0002, BIT01_MASK);
	else
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ADCCONTROL, 0x0002, 0x0000);
}

