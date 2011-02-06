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
#include <linux/broadcom/bcm_fuse_sysparm.h>

#define ANACR0_CTRL ((volatile ANACR0_struct *)&REG_SYS_ANACR0)
#define ANACR1_CTRL ((volatile ANACR1_struct *)&REG_SYS_ANACR1)
#define ANACR3_CTRL ((volatile ANACR3_struct *)&REG_SYS_ANACR3)

#define NUM_OF_EQU_BANDS 5
#define	BIT15_MASK		(1 << 15)
#define BIT00_MASK      (1 <<  0)

extern void CAPI2_SPEAKER_StopTone(UInt32 tid, UInt8 clientID);
extern void CAPI2_SPEAKER_StartGenericTone(UInt32 tid, UInt8 clientID, Boolean superimpose, UInt16 tone_duration, UInt16 f1, UInt16 f2, UInt16 f3);
extern void CAPI2_SPEAKER_StartTone(UInt32 tid, UInt8 clientID, SpeakerTone_t tone,	UInt8 duration);  


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

typedef enum {
	AuxMicBias_OFF = 0,
	AuxMicBias_ON_LPMODE,  //0.4V
	AuxMicBias_ON_FPMODE,  //2.1V
	AuxMicBias_INVALID
} AsicAuxMicBiasMode_t;  // Aux Mic Bias

extern void WaitForAudHalShutdown(void);
extern int brcm_ipc_aud_control_send(const void * const, UInt32);
extern SysAudioParm_t* SYSPARM_GetAudioParmAccessPtr(UInt8 AudioMode);
extern void VOLUMECTRL_SetBasebandVolume(UInt8 level, UInt8 chnl, UInt16 *audio_atten, UInt8 extid); 

//****************************************
// local variable definitions
//****************************************
static Boolean bPowerDownR =TRUE;
static Boolean bPowerDownL =TRUE;
static int current_mode = 0;

static AUDIO_DRV_MIC_Path_t MICConfig[9]={HANDSET_MIC, 
										HEADSET_MIC,
										HANDSFREE_MIC,
										BLUETOOTH_MIC,
										SPEAKER_MIC,
										TTY_MIC,
										HAC_MIC,
										USB_MIC,
										NONE_MIC
										};

static AUDIO_DRV_Spk_Output_t SpeakerConfig[9]={HANDSET_OUTPUT, 
							HEADSET_OUTPUT,
							HANDSFREE_OUTPUT,
							BLUETOOTH_OUTPUT,
							SPEAKER_OUTPUT,
							TTY_OUTPUT,
							HAC_OUTPUT,
							USB_OUTPUT,
							NONE_OUTPUT
							};

static AUDVOC_GAIN_SETTING_T sysparmGain;

// 3 playback that share the voice path
static Boolean bStatusCall = AUDIO_OFF;
static Boolean bStatusAMR = AUDIO_OFF;
static Boolean bStatusDTMF = AUDIO_OFF;

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

#define ANALOG_RAMP_UP   0x0087
#define ANALOG_RAMP_DOWN 0x0007

#define ANALOG_HS_RAMP_UP   0x0081
#define ANALOG_HS_RAMP_DOWN 0x0001
#define ANALOG_RAMP_PD		0x4000
#define ANALOG_RAMP_NO		0x8080
#define ANALOG_RAMP_DEFAULT 0x0000


static Boolean bStatusSTAudio = TRUE; // TRUE = ST, FALSE = MN
static Boolean bStatusSTPoly = FALSE; // TRUE = ST, FALSE = MN



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
				   3,3,3,3,3,       //volume 1-5
				   3,3,6,6,6,       //volume 6-10
				   6,6,9,9,9,       //volume 11-15
				   9,9,12,12,12,    //volume 16-20
				   12,12,15,15,15,  //volume 21-25
				   15,15,18,18,18,  //volume 26-30
				   18,18,21,21,21,  //volume 31-35
				   21,21,24,24,24};	//volume 36-40				   
#endif //#ifdef USE_VOLUME_CONTROL_IN_DB

//map the digital gain to mmi level for OV. Need to adjust based on mmi level.
static UInt8 uVol2OV[37]={0, 
				   0, 0, 1, 
				   0, 0, 2, 
				   0, 0, 3, 
				   0, 0, 4, 
				   0, 0, 5, 
				   0, 0, 6, 
				   0, 0, 7, 
				   0, 0, 8, 
				   0, 0, 0, 
				   0, 0, 0, 
				   0, 0, 0, 
				   0, 0, 0};

#define MAX_SPK_GAIN_STEPS 7


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
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pwrdnref = 0;
        TempReg.FLD_ANACR0.pwrdndrvR = 0;
        TempReg.FLD_ANACR0.pwrdndacR = 0;
        TempReg.FLD_ANACR0.pwrdnd2c = 0;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
        bPowerDownR = FALSE;
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
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pwrdnref = 0;
        TempReg.FLD_ANACR0.pwrdndrvL = 0;
        TempReg.FLD_ANACR0.pwrdndacL = 0;
        TempReg.FLD_ANACR0.pwrdnd2c = 0;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
        bPowerDownL = FALSE;
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
        bPowerDownR = TRUE;
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pwrdndrvR = 1;
        TempReg.FLD_ANACR0.pwrdndacR = 1;
        if (bPowerDownL) {
                TempReg.FLD_ANACR0.pwrdnref = 1;
//                TempReg.FLD_ANACR0.pwrdnd2c = 1;
        }
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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
        bPowerDownL = TRUE;
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pwrdndrvL = 1;
        TempReg.FLD_ANACR0.pwrdndacL = 1;
        if (bPowerDownR) {
                TempReg.FLD_ANACR0.pwrdnref = 1;
//                TempReg.FLD_ANACR0.pwrdnd2c = 1;
        }
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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

        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pga_ctrlR = MAX_SPK_GAIN_STEPS -gainSpk;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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

        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pga_ctrlL = MAX_SPK_GAIN_STEPS -gainSpk;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.mute_selR = muteAction;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.mute_selL = muteAction;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.drvr_selR = pathSpeaker;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.drvr_selL = pathSpeaker;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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

        if (ChMode>=CHANNEL_MAX)
                return;
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.mode_selR = ChMode;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
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

        if (ChMode>=CHANNEL_MAX)
                return;
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.mode_selL = ChMode;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

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

        TempReg.REG_ANACR3 = ANACR3_CTRL->HAL_ANACR3.REG_ANACR3;
        TempReg.FLD_ANACR3.pga_mux_sel = pathMIC;
        ANACR3_CTRL->HAL_ANACR3.REG_ANACR3 = TempReg.REG_ANACR3;
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
static void AUDIO_DRV_SetAuxMicBiasOnOff(Boolean Bias_on_off, AsicAuxMicBiasMode_t bias_mode)
{
	if(Bias_on_off && (bias_mode == AuxMicBias_ON_FPMODE))
		*(volatile UInt32 *)AUXMIC_AUXEN_REG |=BIT00_MASK;
	else
		*(volatile UInt32 *)AUXMIC_AUXEN_REG &=~(BIT00_MASK);
	return;
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
static void AUDIO_DRV_SetMicBiasOnOff(Boolean Bias_on_off)
{
	return;
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
//! Function Name:	AUDIO_DRV_SetAudioMixerGain
//!
//! Description:	set mixer gain (volume)
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void AUDIO_DRV_SetAudioMixerGain(int Lgain, int Rgain)
{

	AUDIO_WriteDSP(DSP_AUDIO_REG_APVLMXGAIN, Lgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_APVRMXGAIN, Rgain);
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
//! Function Name:	AUDIO_DRV_PowerUpD2C
//!
//! Description:	power up d2c
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_PowerUpD2C(void)
{
        AUDIO_ANACR0_un_t TempReg;
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.pwrdnd2c = 0x0;
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_PowerDownD2C
//!
//! Description:	power up d2c
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_PowerDownD2C(void)
{
    AUDIO_ANACR0_un_t TempReg;
    audio_control_dsp(CHECK_DSP_AUDIO,0,0,0);
    //wait until dsp finished turing off the aduio
    WaitForAudHalShutdown();
    TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
    TempReg.FLD_ANACR0.pwrdnd2c = 0x1;
    ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_LoadGainSetting
//!
//! Description:	load gain setting
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_LoadGainSetting(void)
{
	AudioMode_t	mode = AUDIO_MODE_HANDSET;
	for (;mode<AUDIO_MODE_INVALID;mode++)
	{
		if (NULL != SYSPARM_GetAudioParmAccessPtr(mode))
		{
			sysparmGain.MixerGain[mode] = SYSPARM_GetAudioParmAccessPtr(mode)->audvoc_mixergain;
			sysparmGain.VcfgrGain[mode] = SYSPARM_GetAudioParmAccessPtr(mode)->audvoc_vcfgr;
			sysparmGain.PgaGain[mode] = SYSPARM_GetAudioParmAccessPtr(mode)->speaker_pga;
			sysparmGain.MicGain[mode] = SYSPARM_GetAudioParmAccessPtr(mode)->mic_pga;
			sysparmGain.PathGain[mode] = SYSPARM_GetAudioParmAccessPtr(mode)->audvoc_vslopgain; // voice path
			pr_info("mode=%d MixerGain=0x%x VcfgrGain=0x%x PgaGain=0x%x MicGain=0x%x PathGain=0x%x\n", mode,
						sysparmGain.MixerGain[mode], sysparmGain.VcfgrGain[mode], sysparmGain.PgaGain[mode], sysparmGain.MicGain[mode], sysparmGain.PathGain[mode]);
		}
	}
//	if(sysparmGain.VcfgrGain[0] != 0x30)
	{
//		printk("sysparmGain.VcfgrGain[0]=0x%x assign to 0x30\n", sysparmGain.VcfgrGain[0]);
//		sysparmGain.VcfgrGain[0] = 0x30;

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

	switch(eSpeakerType)
	{
		case LEFT_400MW:
			AUDIO_DRV_SetLeftSpkPGA(sysparmGain.PgaGain[Ch]);
			AUDIO_DRV_MuteLeftSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetLChannelMode(CHANNEL_DIFF);	
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_DIFF);
			AUDIO_DRV_PowerUpD2C();	
			break;

		case RIGHT_400MW:
			AUDIO_DRV_SetRightSpkPGA(sysparmGain.PgaGain[Ch]);
			AUDIO_DRV_MuteRightSpk(UNMUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			AUDIO_DRV_SelectRightSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetRChannelMode(CHANNEL_DIFF);
			AUDIO_DRV_SelectLeftSpkOutput(SPEAKER_400MW);
			AUDIO_DRV_SetLChannelMode(CHANNEL_DIFF);	
			AUDIO_DRV_PowerUpD2C();	
			break;

		case RIGHT_PLUS_LEFT_100MW:
			AUDIO_DRV_SetRightSpkPGA(sysparmGain.PgaGain[Ch]);
			AUDIO_DRV_SetLeftSpkPGA(sysparmGain.PgaGain[Ch]);

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
//! Function Name:	brcm_aud_post_msg
//!
//! Description:	ipc api for posting dsp msgs
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
//IPC
void brcm_aud_post_msg(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2)
{
        CmdQ_t msg;

        msg.cmd = cmd;
        msg.arg0 = arg0;
        msg.arg1 = arg1;
        msg.arg2 = arg2;

        brcm_ipc_aud_control_send((void *)&msg, sizeof(msg));
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_PostMsgToDSP
//!
//! Description:	audio post dsp msgs
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
inline void AUDIO_PostMsgToDSP(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2)
{
        brcm_aud_post_msg(cmd, arg0, arg1, arg2);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_PostFastMsgToDSP
//!
//! Description:	audio post fast dsp msgs
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
/* used for fuse platform to put msg into dsp fast cmd Q */
inline void AUDIO_PostFastMsgToDSP(UInt16 cmd, UInt16 arg0, UInt16 arg1, UInt16 arg2)
{
        brcm_aud_post_msg(cmd+COMMAND_IPC_FAST_CMD_OFFSET, arg0, arg1, arg2);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_BitwiseWriteDSP
//!
//! Description:	audio bitwise write dsp regs
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
inline void AUDIO_BitwiseWriteDSP(UInt16 addr, UInt16 mask, UInt16 value)
{
        brcm_aud_post_msg(COMMAND_BITWISE_WRITE, addr, mask, value);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_WriteDSP
//!
//! Description:	audio write dsp regs
//!
//! Notes:
//! 
/////////////////////////////////////////////////////////////////////////////
inline void AUDIO_WriteDSP(UInt16 addr, UInt16 value)
{
        brcm_aud_post_msg(COMMAND_WRITE, addr, value, 0);
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
void AUDIO_DRV_SetPCMOnOff(Boolean	on_off)
{


	// By default the PCM port is occupied by trace port on development board
	if(on_off)
	{	
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
void AUDIO_DRV_SetMicrophonePathOnOff(AudioMode_t voiceMode , Boolean on_off)
{
	UInt16 MicGain;
	AUDIO_DRV_MIC_Path_t MicCh;
	AsicAuxMicBiasMode_t BiasMode = AuxMicBias_ON_LPMODE;

	//_DBG_A_(audioPrintf("AUDIO_DRV_SetMicrophonePathOnOff() voiceMode=%d on_off=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n",
	//	voiceMode, on_off, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));
	pr_info("==============AUDIO_DRV_SetMicrophonePathOnOff  voiceMode=%d \n", (int)voiceMode);
	if (on_off)
	{
		MicGain = (UInt16)SYSPARM_GetAudioParmAccessPtr(voiceMode)->mic_pga;
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
			//*(volatile UInt32 *)AUXMIC_CMC_REG &=~(BIT00_MASK);
			AUDIO_DRV_SetMicBiasOnOff(on_off);
			break;

		case AUDIO_AUX_MIC:
			if (on_off)
			{
#ifndef BROOKLYN_HW
				*(volatile UInt32 *)AUXMIC_CMC_REG |=BIT00_MASK;
				AUDIO_DRV_SetAuxMicBiasOnOff(on_off, BiasMode);
#endif //#ifndef BROOKLYN_HW
			}
			else if (bStatusVoice == AUDIO_OFF)
			{
#ifndef BROOKLYN_HW
				*(volatile UInt32 *)AUXMIC_CMC_REG &=~(BIT00_MASK);
				AUDIO_DRV_SetAuxMicBiasOnOff(on_off, BiasMode);
#endif //#ifndef BROOKLYN_HW
			}
			break;

		default :
			break;
	}

	if (voiceMode == AUDIO_MODE_BLUETOOTH)
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

/////////////////////////////////////////////////////////////////////////////
//!
//!
//! Function Name:	AUDIO_DRV_SetMicrophonePath_16K
//!
//! Description:	set/unset microphone to 16K
//!
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetMicrophonePath_16K(Boolean on_off)
{
	if(on_off)
		AUDIO_PostMsgToDSP(COMMAND_AUDIO_ENABLE, 1, 1, 1);
	else
		AUDIO_PostMsgToDSP(COMMAND_AUDIO_ENABLE, 0, 0, 0);
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
	AUDIO_ASIC_SetAudioMode(voiceMode);
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
    //KRIL_DevSpecific_Cmd(BCM_AUDIO_CLIENT, RIL_DEVSPECIFICPARAM_BCM_VOLUMECTRL_SETBASEBANDVOLUME, &Volume,  sizeof(Volume));
    UInt8 digital_gain_dB = 0;
    UInt16  volume_max = 24; // FIXME:
    volume_max = (UInt16)SYSPARM_GetAudioParmAccessPtr(AUDIO_MODE_HANDSET)->voice_volume_max;

    if (uVol2BB[Volume] > volume_max)
        digital_gain_dB = volume_max;
    else
        digital_gain_dB = uVol2BB[Volume];

    VOLUMECTRL_SetBasebandVolume(digital_gain_dB, uVol2OV[digital_gain_dB], 0, 0 );
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

#if (defined(_BCM2153_))
	//PMU_AMPVDD( TRUE );
#endif

	pr_info("==============AUDIO_DRV_SetSpkDriverOutputOn  spk=%d L=%d R=%d \r\n", eSpeakerType, bPowerDownL, bPowerDownR);

	switch(eSpeakerType)
	{
		case LEFT_400MW:
			if((bPowerDownL==FALSE))
				break;
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
			OSTASK_Sleep(1);
			AUDIO_DRV_PowerUpLeftSpk();
			AUDIO_DRV_PowerDownRightSpk();
			break;
		case RIGHT_400MW:
			if((bPowerDownR==FALSE))
				break;
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
			OSTASK_Sleep(1);
			AUDIO_DRV_PowerUpRightSpk();
			AUDIO_DRV_PowerDownLeftSpk();
			break;
		case RIGHT_PLUS_LEFT_100MW:
			if((bPowerDownL==FALSE)&&(bPowerDownR==FALSE))
				break;
#ifdef HEADSET_AUDIO_3WIRE
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_NO);
			OSTASK_Sleep(1);
			AUDIO_DRV_PowerUpRightSpk();
			AUDIO_DRV_PowerUpLeftSpk();
#else // regular single ended
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_UP);
			OSTASK_Sleep(1);
			AUDIO_DRV_PowerUpRightSpk();
			AUDIO_DRV_PowerUpLeftSpk();
			OSTASK_Sleep(5);
#endif
			break;

		default: // no audio output
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			OSTASK_Sleep(5);
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
			OSTASK_Sleep(40);
			AUDIO_DRV_PowerDownRightSpk();	
			AUDIO_DRV_PowerDownLeftSpk();	
			OSTASK_Sleep(5);
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
			OSTASK_Sleep(40);
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
//! Function Name:	AUDIO_DRV_SetSpkDriverDown
//!
//! Description:	power down the speaker driver
//!
//! Notes:			
//! 
/////////////////////////////////////////////////////////////////////////////
void AUDIO_DRV_SetSpkDriverDown(AUDIO_DRV_Spk_Output_t eSpeakerType)
{
	//_DBG_A_(audioPrintf("AUDIO_DRV_SetSpkDriverDown(%d) AUDIO_ON?=%d\r\n",eSpeakerType, (bStatusAudio||bStatusVoice||bStatusPoly)));
	
	switch(eSpeakerType)
	{
		case LEFT_400MW:
		case RIGHT_400MW:
		case RIGHT_PLUS_LEFT_100MW:
			//AUDIO_DRV_PowerDownD2C();
			OSTASK_Sleep(1);
			break;

		default: // no audio output
			AUDIO_DRV_MuteRightSpk(MUTE_ALL);
			AUDIO_DRV_MuteLeftSpk(MUTE_ALL);
			OSTASK_Sleep(5);
			AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_HS_RAMP_DOWN);
			OSTASK_Sleep(40);
			AUDIO_DRV_PowerDownRightSpk();	
			AUDIO_DRV_PowerDownLeftSpk();	
			OSTASK_Sleep(5);
			break;
	}
#if (defined(_BCM2153_))
	//PMU_AMPVDD( FALSE );
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


	AUDIO_DRV_SetSpkDriverOutputOff	(eSpeakerType);
	AUDIO_DRV_SetSpkDriverDown (eSpeakerType);

	return;
}



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

	AUDIO_DRV_SetSpkDriverSelect(eSpkType, (AUDIO_CHANNEL_t)voiceMode);//AUDIO_DRV_SetSpkDriverOn(eSpkType, (AUDIO_CHANNEL_t)voiceMode);

	audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);

	if (voiceMode  == AUDIO_MODE_BLUETOOTH)
		AUDIO_DRV_SetPCMOnOff(TRUE);
	else
		AUDIO_DRV_SetPCMOnOff(FALSE);

	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, (AUDIO_CHANNEL_t)voiceMode);

	AUDIO_DRV_Set_DL_OnOff(AUDIO_ON);
	//AUDIO_DRV_ConnectMicrophoneUplink(AUDIO_ON);

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VMUT, 0x2, 0);
	pr_info("PgaGain=%d pathgain=0x%x VcfGrGain=0x%x voicemode=%d\n", sysparmGain.PgaGain[voiceMode], sysparmGain.PathGain[2], sysparmGain.VcfgrGain[voiceMode], voiceMode);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,sysparmGain.PathGain[voiceMode]);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, sysparmGain.VcfgrGain[voiceMode] );

	bStatusCall = AUDIO_ON;	
	bStatusVoice = VOICE_PATH_STATUS;
}

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
	
	if(AUDIO_ON == (bStatusAudio||bStatusVoice||bStatusPoly))
	{
#if 1	
	
		if(bStatusVoice == AUDIO_OFF)
		{
			AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,0xa000);
			AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR,0x18);

			if (bStatusPcmBack == AUDIO_OFF)
			{
				AUDIO_DRV_Set_DL_OnOff(AUDIO_OFF);
				audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
			}
			
			if (voiceMode  == AUDIO_MODE_BLUETOOTH)
				AUDIO_DRV_SetPCMOnOff(FALSE);
		}
#endif	
	return;
	}
	
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,0xa000);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR,0x18);
	AUDIO_DRV_SetSpkDriverOff(eSpkType);
	
	if (bStatusPcmBack == AUDIO_OFF)
	{
		AUDIO_DRV_Set_DL_OnOff(AUDIO_OFF);

		audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
	}
	
	if (voiceMode  == AUDIO_MODE_BLUETOOTH)
		AUDIO_DRV_SetPCMOnOff(FALSE);
	

	if ((bStatusInputPath||bStatusPcmBack) == AUDIO_OFF)
		AUDIO_DRV_PowerDownD2C();
	return;
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

	//_DBG_A_(audioPrintf("AUDIO_DRV_MicrophoneMuteOnOff() mute_on_off=%d\r\n", mute_on_off));

	if(mute_on_off) //mute
	{
		audio_control_dsp(MUTE_DSP_UL, 0, 0, 0);	
	}
	else //unmute
	{	
		if (bStatusMicChUsed == TRUE) //recover previous dsp ul gain
		{
		audio_control_dsp(UNMUTE_DSP_UL, 0, 0, 0);	
		}
		else //for none_mic case, like bt, usb
		{
			EchoGain = (Int16)currentMicGain;
			//the index [0 14] for converting mic pga to dsp ul gain on cp
			audio_control_dsp(SET_DSP_UL_GAIN, EchoGain, 0, 0);
		}
	}
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


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetAudioLRChannel
//!
//! Description:	set channel mode on audio path
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
static void AUDIO_DRV_SetAudioLRChannel(BB_LPathCh_Mode_t LChMode, BB_RPathCh_Mode_t RChMode)
{
        if (LChMode > LOUT_NO)
                LChMode = LOUT_NO;
        if (RChMode > ROUT_NO)
                RChMode = ROUT_NO;
        AUDIO_WriteDSP(DSP_AUDIO_REG_ALRCH, (LChMode<<2)|RChMode);
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

	//_DBG_A_(audioPrintf("AUDIO_DRV_SetTonePathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
	//	chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));	

	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VSLOPGAIN, 0xa000, 0xa000);

	AUDIO_DRV_SetSpkDriverSelect(eSpkType, chan);//AUDIO_DRV_SetSpkDriverOn(eSpkType, chan);
#ifdef USE_DSP_AUDIO_ENABLE
#if defined(ROXY_TONE_FEATURES)
// 	audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0); // 04012009 michael
	if ((bStatusVoice == AUDIO_OFF) || (!isToneRunning))
		audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#else
		audio_control_dsp(ENABLE_DSP_AUDIO, 0, 0, 0);
#endif  //end of "#if defined(ROXY_TONE_FEATURES)"
#else	
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x20);
#endif
#if defined(SS_2153) //for customer OEM/MMI		
	if (chan  == AUDIO_CHNL_BLUETOOTH)
		AUDIO_DRV_SetPCMOnOff(TRUE);
	else
		AUDIO_DRV_SetPCMOnOff(FALSE);
#endif	//#if defined(SS_2153) //for customer OEM/MMI	
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, chan);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && defined (__AUDIO_AMP_MAX9877__)&& !defined(SS_2153)
	AUDIO_DRV_SetExtAMPOn(AUDIO_DRV_ConvertChnlMode(chan));
#endif

	AUDIO_DRV_Set_DL_OnOff(AUDIO_ON);
						
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_VMUT, 0x2, 0);
#if 1	//Akhil	
#if defined(FUSE_APPS_PROCESSOR)
	//AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vslopgain);
	//AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vcfgr);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,sysparmGain.PathGain[chan]);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, sysparmGain.VcfgrGain[chan] );

#else
	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vslopgain);
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vcfgr);
#endif
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
	
	//_DBG_A_(audioPrintf("AUDIO_DRV_SetTonePathOff() bVoice=%d bAudio=%d bPoly=%d bCall=%d bAMR=%d bDTMF=%d bPcmBack=%d\r\n", 
	//	bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF, bStatusPcmBack));
	
	if(AUDIO_ON == (bStatusAudio||bStatusVoice||bStatusPoly))
	{
		//_DBG_A_(audioPrintf("AUDIO_DRV_SetTonePathOff() bStatusAudio=%d,bStatusPoly=%d\r\n", bStatusAudio, bStatusPoly));
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
			
#if defined(SS_2153) //for customer OEM/MMI			
			if (chan  == AUDIO_CHNL_BLUETOOTH)
				AUDIO_DRV_SetPCMOnOff(FALSE);
#endif //#if defined(SS_2153) //for customer OEM/MMI			
		}
#ifndef USE_DSP_AUDIO_AMR_VOLUME
		else if(bStatusVoice == AUDIO_ON)
		{
			if(bStatusCall == AUDIO_ON)
			{
#if 1 //akhil
#if defined(FUSE_APPS_PROCESSOR)
//				AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vslopgain);
//				AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()[chan].audvoc_vcfgr);
				AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,sysparmGain.PathGain[chan]);
				AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, sysparmGain.VcfgrGain[chan] );


#else
				AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vslopgain);
				AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR, (UInt16)AUDIO_GetParmAccessPtr()->audio_parm[chan].audvoc_vcfgr);
#endif
#endif //if 0
			}
		}
#endif //#ifndef USE_DSP_AUDIO_AMR_VOLUME		
		return;
	}

	AUDIO_WriteDSP(DSP_AUDIO_REG_VSLOPGAIN,0xa000);	
	AUDIO_WriteDSP(DSP_AUDIO_REG_VCFGR,0x18);
#if defined ( BCM2153_FAMILY_BB_CHIP_BOND ) && defined (__AUDIO_AMP_MAX9877__)&& !defined(SS_2153)
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
	
#if defined(SS_2153) //for customer OEM/MMI	
	if (chan  == AUDIO_CHNL_BLUETOOTH)
		AUDIO_DRV_SetPCMOnOff(FALSE);
#endif //#if defined(SS_2153) //for customer OEM/MMI	

	if ((bStatusInputPath || bStatusPcmBack) == AUDIO_OFF)
		AUDIO_DRV_PowerDownD2C();

	return;
}
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
	//_DBG_A_(audioPrintf("AUDIO_DRV_SetAudioPathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
	//	chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));	
	
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ALSLOPGAIN, 0xa000, 0xa000);
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_ARSLOPGAIN, 0xa000, 0xa000);
    program_FIR_IIR_filter((UInt16)chan);
	
	AUDIO_DRV_SetSpkDriverSelect(eSpkType, chan);//AUDIO_DRV_SetSpkDriverOn(eSpkType, chan);
	//AUDIO_DRV_SetAudioChannel(chan); // 01062009 michael

	// select IIR filter, enable audio path
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x0007, 0x0005);


	//flush the audio fifo for out of sync case if needed.
	//this is needed to flush the fifo at least once after audvoc enabled with new seq and new patch
	//without new patch and new seq, the lowerest working number so far is 65 (reduced from 128)
	audio_control_generic(FLUSH_STEREO_AIFIFODATA, 65, 0, 0); //128

	
	// Need this sleep after select IIR filter, otherwise we can hear pop noise
	AUDIO_DRV_SetSpkDriverOutputOn(eSpkType, chan);
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
	//_DBG_A_(audioPrintf("AUDIO_DRV_SetAudioPathOff() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
	//	chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	bStatusAudio = AUDIO_OFF;
	if(AUDIO_ON == (bStatusVoice||bStatusPoly))
	{
		AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x0003, 0x0002);
		return;
	}

	AUDIO_DRV_SetSpkDriverOff(eSpkType);
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_STEREOAUDMOD, 0x0003, 0x0002);

	if ((bStatusInputPath || bStatusPcmBack) == AUDIO_OFF)
		AUDIO_DRV_PowerDownD2C();
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_DRV_SetBgtcsp
//!
//! Description:	set anacr0 bgtcsp
//!
//! Notes: done once
//!
/////////////////////////////////////////////////////////////////////////////
static void AUDIO_DRV_SetBgtcsp(void)
{
        AUDIO_ANACR0_un_t TempReg;
        TempReg.REG_ANACR0 = ANACR0_CTRL->HAL_ANACR0.REG_ANACR0;
        TempReg.FLD_ANACR0.bgtcsp = 0x6; //3; the default value should be [8:6]=110 per latest datasheet
        ANACR0_CTRL->HAL_ANACR0.REG_ANACR0 = TempReg.REG_ANACR0;
}

//======================================================================
//Functions used for bit by bit control on ANACR0 and ANACR1 per ASIC requirement for 2153 E1
//pop fix sequence
//======================================================================
void AUDIO_DRV_SetHifiDacMask(void)
{
        AUDIO_ANACR1_un_t TempReg;
        TempReg.REG_ANACR1 = ANACR1_CTRL->HAL_ANACR1.REG_ANACR1;
        TempReg.FLD_ANACR1.hifidac_pwd_mask = 0x1;
        ANACR1_CTRL->HAL_ANACR1.REG_ANACR1 = TempReg.REG_ANACR1;
}

void AUDIO_DRV_FlushFifo(void)
{
	AUDIO_WriteDSP(DSP_AUDIO_REG_AFIFOCTRL, 0xbfc0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_AFIFOCTRL, 0x3f40);
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

	//audio was turned on in dsp for 2153 e1, turned it off
#ifdef USE_DSP_AUDIO_ENABLE
	audio_control_dsp(DISABLE_DSP_AUDIO, 0, 0, 0);
#else
	AUDIO_BitwiseWriteDSP(DSP_AUDIO_REG_AMCR, 0x0020, 0x0);
#endif

	AUDIO_DRV_SetHifiDacMask();

	AUDIO_DRV_SetAudioLRChannel(LOUT_RIN, ROUT_LIN);
	//AUDIO_DRV_SetPolyLRChannel(LOUT_LIN, ROUT_LIN);

	AUDIO_WriteDSP(DSP_AUDIO_REG_AIFIFODATA0, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_AIFIFODATA1, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_AIFIFODATA2, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PIFIFODATA0, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PIFIFODATA1, 0x0);
	AUDIO_WriteDSP(DSP_AUDIO_REG_PIFIFODATA2, 0x0);

	AUDIO_WriteDSP(DSP_AUDIO_REG_APRR, ANALOG_RAMP_DEFAULT);

}


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
	//_DBG_A_(audioPrintf("AUDIO_DRV_PlayTone() tone_id=%d duration=%d\r\n", tone_id, duration));

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
	//_DBG_A_(audioPrintf("AUDIO_DRV_PlayGenericTone() f1=%d\r\n", f1));

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
	//_DBG_A_(audioPrintf("AUDIO_DRV_StopPlayTone() \r\n"));

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

void AUDIO_DRV_SetDTMFPathOn(AUDIO_CHANNEL_t chan)
{
	static AUDIO_CHANNEL_t prev_chan = AUDIO_CHNL_RESERVE;
	
	//_DBG_A_(audioPrintf("AUDIO_DRV_SetDTMFPathOn() chan=%d bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
	//	chan, bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

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
	//_DBG_A_(audioPrintf("AUDIO_DRV_SetDTMFPathOff() bStatusVoice=%d bStatusAudio=%d bStatusPoly=%d bStatusCall=%d bStatusAMR=%d bStatusDTMF=%d\r\n", 
	//	bStatusVoice, bStatusAudio, bStatusPoly, bStatusCall, bStatusAMR, bStatusDTMF));

	bStatusDTMF = AUDIO_OFF;
	bStatusVoice = VOICE_PATH_STATUS;
	//_DBG_A_(audioPrintf("AUDIO_DRV_SetDTMFPathOff() bStatusVoice=%d\r\n", bStatusVoice));
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

