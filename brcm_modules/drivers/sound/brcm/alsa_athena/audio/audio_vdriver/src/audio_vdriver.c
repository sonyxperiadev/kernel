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
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_syscfg.h"
//#define SYSCFG_BASE_ADDR      0x08880000      /* SYSCFG core */
//#define SYSCFG_ANACR0_OFFSET                                              0x00000080
#include "shared.h"
#include "dspcmd.h"
#include "ripcmdq.h"
#include "ripisr.h"
#include "audio_consts.h"
#include "csl_aud_drv.h"
#include "audio_vdriver.h"
//#ifndef NO_SYSPARAM
#ifdef UNDER_LINUX
#include "linux/broadcom/bcm_fuse_sysparm_CIB.h"
#else
#include "sysparm.h"
#endif
//#endif
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

extern AUDDRV_MIC_Enum_t    currVoiceMic;   //used in pcm i/f control. assume one mic, one spkr.
extern AUDDRV_SPKR_Enum_t   currVoiceSpkr;  //used in pcm i/f control. assume one mic, one spkr.
extern Boolean inVoiceCall;
//=============================================================================
// Private Type and Constant declarations
//=============================================================================

#define ANACR0_ModeMask (0x001F9FC0)  //ANACR0 bits [20:15] Mode_selR, Mode_selL, [12:06] Cm_seland, Iq_dbl, BgTcsp.
#define ANACR0_DrvrSelMask (0x006000) //Drvr_selR, Drvr_selL



static AudioEqualizer_en_t	sEqualizerType = EQU_NORMAL;
void Audio_InitRpc();


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
static SysAudioParm_t* AUDIO_GetParmAccessPtr(UInt8 AudioMode)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	//return APSYSPARM_GetAudioParmAccessPtr();
    return SYSPARM_GetAudioParmAccessPtr(AudioMode);
#endif
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	AUDIO_GetParmMMAccessPtr
//!
//! Description:	audio parm multimedia acces for FUSE AP only
//!
/////////////////////////////////////////////////////////////////////////////
static SysIndMultimediaAudioParm_t* AUDIO_GetParmMMAccessPtr(UInt8 AudioMode)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	//return APSYSPARM_GetMultimediaAudioParmAccessPtr();
    return SYSPARM_GetMultimediaAudioParmAccessPtr(AudioMode);
#endif
}

#define AUDIOMODE_PARM_ACCESSOR(mode)	 (*AUDIO_GetParmAccessPtr(mode))
#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	(*AUDIO_GetParmMMAccessPtr(mode))


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
#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()->audio_parm[mode]
#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()->mmaudio_parm[mode]

#endif //#if defined(FUSE_APPS_PROCESSOR)


//=============================================================================
//
// Function Name: AUDDRV_Init
//
// Description:   Initialize audio system
//
//=============================================================================
void AUDDRV_Init( void )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Init *\n\r");

		// both AP and CP
	AUDDRV_SPKRInit( AUDDRV_SPKR_NONE, AUDIO_SPKR_CHANNEL_INVALID );

#if defined(FUSE_APPS_PROCESSOR)

    

#ifdef UNDER_LINUX


    // Register proper handler here
#ifdef VPU_INCLUDED
	RIPISR_Register_VPU_ProcessStatus( (VPU_ProcessStatus_t) &AP_VPU_ProcessStatus );   //not used in runtime?
#endif



#else
	// create the IP here because it will take some time. Otherwise, the first IPC message will take long time.
	IPC_Audio_Create_BufferPool();

	RIPISR_Register_AudioISR_Handler( (Audio_ISR_Handler_t) &AP_Audio_ISR_Handler );  //not used in runtime?

#ifdef VPU_INCLUDED
	RIPISR_Register_VPU_ProcessStatus( (VPU_ProcessStatus_t) &AP_VPU_ProcessStatus );   //not used in runtime?
#endif
#endif


	AUDDRV_RegisterCB_getAudioMode( (CB_GetAudioMode_t) &AUDDRV_GetAudioMode );
	AUDDRV_RegisterCB_setAudioMode( (CB_SetAudioMode_t) &AUDDRV_SetAudioMode );
	AUDDRV_RegisterCB_setMusicMode( (CB_SetMusicMode_t) &AUDDRV_SetMusicMode );

    // Initialize RPC and shared memory here
#ifdef UNDER_LINUX
    Audio_InitRpc();
    VPSHAREDMEM_Init();

#endif

#else  //#if defined(FUSE_APPS_PROCESSOR)


#if !defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)
		// ATHENA_EDGE_CP_STANDALONE build
	RIPISR_Register_AudioISR_Handler( (Audio_ISR_Handler_t) &AP_Audio_ISR_Handler );  //not used in runtime?
	#ifdef VPU_INCLUDED
	RIPISR_Register_VPU_ProcessStatus( (VPU_ProcessStatus_t) &AP_VPU_ProcessStatus );   //not used in runtime?
	#endif

	AUDDRV_RegisterCB_getAudioMode( (CB_GetAudioMode_t) &AUDDRV_GetAudioMode );
	AUDDRV_RegisterCB_setAudioMode( (CB_SetAudioMode_t) &AUDDRV_SetAudioMode );
	AUDDRV_RegisterCB_setMusicMode( (CB_SetMusicMode_t) &AUDDRV_SetMusicMode );

#else
	RIPISR_Register_AudioISR_Handler( (Audio_ISR_Handler_t) &CP_Audio_ISR_Handler );

# endif


#endif	
}

//=============================================================================
//
// Function Name: AUDDRV_Telephony_SelectMicSpkr
//
// Description:   Select the mic and speaker for voice call
//
//=============================================================================


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
// Function Name: AUDDRV_SaveAudioMode
//
// Description:   save audio mode.
//
//=============================================================================

// intentionally put this static var here to prevent any code above this lines to use this var.
static AudioMode_t currAudioMode = AUDIO_MODE_HANDSET;  //need to update this on AP and also in audioapi.c on CP.
static AudioMode_t currMusicAudioMode = AUDIO_MODE_HANDSET;

void AUDDRV_SaveAudioMode( AudioMode_t audio_mode )
{
	currAudioMode = audio_mode; // update mode
}


//=============================================================================
//
// Function Name: AUDDRV_SetAudioMode
//
// Description:   set audio mode.
//
//=============================================================================

void AUDDRV_SetAudioMode( AudioMode_t audio_mode)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_mode==%d\n\r", audio_mode );

	// load DSP parameters:
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, audio_mode, 0, 0, 0, 0 );
	
	currAudioMode = audio_mode; // update mode

	if ( audio_mode >= AUDIO_MODE_NUMBER )
		currMusicAudioMode = (AudioMode_t) (audio_mode - AUDIO_MODE_NUMBER);

	//AUDDRV_VoiceDAC

		//should be part of audio mode API
	//or make SYSPARM_GetAudioParmAccessPtr to be the same on AP and CP:
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) &= ~ANACR0_ModeMask;  //clear the bits
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_anacr0 & ANACR0_ModeMask ) ;
	//*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ANACR0_DrvrSelMask;

	AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);

	AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT_CFGR, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_vcfgr );
	
	// Only load the slopgain for voice call. For voice playback/record, we don't want to load the sysparm value to overwrite what user set.
	if (inVoiceCall == TRUE)
	{
		AUDDRV_SetGain_Hex( AUDDRV_GAIN_VOICE_OUTPUT,      AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_vslopgain );
	}

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

	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioMode).MPMbiquad_cfg );

	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );

	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

	AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioMode).mic_pga);

	if ( audio_mode >= AUDIO_MODE_NUMBER && (audio_mode != AUDIO_MODE_BLUETOOTH_WB) )
		AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
	else
		AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
}

//=============================================================================
//
// Function Name: AUDDRV_SetMusicMode
//
// Description:   set audio mode.
//
//=============================================================================
#if 0
void AUDDRV_SetMusicMode( AudioMode_t audio_mode)
{
#define	AUDIO_5BAND_EQ_MODE_NUMBER		6	///< Up to 6 Audio EQ Profiles (modes)
    #define EQPATH_SIZE					6  //path 0-5 in EQ.
    #define PR_DAC_IIR_SIZE             25

    /*Kishore start */
    UInt16 AUDVOC_ADAC_FIR[65]; 
    UInt32 AUDVOC_MIXER_IIR[60];
    UInt16 AUDVOC_AEQCOF[ PR_DAC_IIR_SIZE];
	UInt16 AUDVOC_AEQPATHGAIN[ EQPATH_SIZE - 1 ];
	UInt16 AUDVOC_AEQPATHOFST[ EQPATH_SIZE];

    
    UInt16 count,count1;


	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() audio_mode==%d\n\r", audio_mode );

	currAudioMode = audio_mode; // update mode

	if ( audio_mode >= AUDIO_MODE_NUMBER )
		currMusicAudioMode = (AudioMode_t) (audio_mode - AUDIO_MODE_NUMBER);

		//should be part of audio mode API
	//or make SYSPARM_GetAudioParmAccessPtr to be the same on AP and CP:
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) &= ~ANACR0_ModeMask;  //clear the bits
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_anacr0 & ANACR0_ModeMask ) ;
	//*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ANACR0_DrvrSelMask;

	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);

	//AUDDRV_SetEquType ( AUDDRV_TYPE_AUDIO_OUTPUT, sEqualizerType );

    /* Kishore start */
#if 1
        memset(AUDVOC_AEQPATHGAIN,0,sizeof(AUDVOC_AEQPATHGAIN));
        AUDVOC_AEQPATHGAIN[0] = 0x0001;

        memset(AUDVOC_AEQCOF,0,sizeof(AUDVOC_AEQCOF));

        for(count = 1; count < 6; count++)
            AUDVOC_AEQCOF[ (count * 5)  - 1] = 0x1000;

		AUDDRV_SetFilter( AUDDRV_AEQPATHGAIN, AUDVOC_AEQPATHGAIN );
		AUDDRV_SetFilter( AUDDRV_AEQ, AUDVOC_AEQCOF);
#endif
    /* kishore end */
	
	//move to CP.  //to read and set this after the sys parm are readable at AP
	//AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );

	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQ,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQCOF[0][0] );
	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN,	(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHGAIN[0][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHOFST[0][0] );

/* Kishore start*/

    memset(AUDVOC_ADAC_FIR,0,sizeof(AUDVOC_ADAC_FIR));
    AUDVOC_ADAC_FIR[32] = 0x2000;
    AUDDRV_SetFilter( AUDDRV_AFIR,AUDVOC_ADAC_FIR);

    /* Kishore end */



	AUDDRV_SetFilter( AUDDRV_PIIR,			(const UInt16 *) & AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).PR_DAC_IIR[0] );

	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioMode).MPMbiquad_cfg );

/* Kishore start*/
    memset(AUDVOC_MIXER_IIR,0,sizeof(AUDVOC_MIXER_IIR));
    for(count = 1; count < 13; count++)
        AUDVOC_MIXER_IIR[(count * 5)-1] = 0x00100000;

    AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, AUDVOC_MIXER_IIR);
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, AUDVOC_MIXER_IIR);
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, AUDVOC_MIXER_IIR);
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, AUDVOC_MIXER_IIR);
 
    /* Kishore end*/

#if 0
	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
#endif

	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
    AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, 0x0800 );
    AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, 0x0800 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

	AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioMode).mic_pga);
}
#else
void AUDDRV_SetMusicMode( AudioMode_t audio_mode)
{
#define	AUDIO_5BAND_EQ_MODE_NUMBER		6	///< Up to 6 Audio EQ Profiles (modes)
    #define EQPATH_SIZE					6  //path 0-5 in EQ.
    #define PR_DAC_IIR_SIZE             25

    /*Kishore start */
//    UInt16 AUDVOC_ADAC_FIR[65]; 
  //  UInt32 AUDVOC_MIXER_IIR[60];
    UInt16 AUDVOC_AEQCOF[ PR_DAC_IIR_SIZE];
	UInt16 AUDVOC_AEQPATHGAIN[ EQPATH_SIZE - 1 ];
	UInt16 AUDVOC_AEQPATHOFST[ EQPATH_SIZE];


	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() audio_mode==%d\n\r", audio_mode );

	currAudioMode = audio_mode; // update mode

	if ( audio_mode >= AUDIO_MODE_NUMBER )
		currMusicAudioMode = (AudioMode_t) (audio_mode - AUDIO_MODE_NUMBER);

		//should be part of audio mode API
	//or make SYSPARM_GetAudioParmAccessPtr to be the same on AP and CP:
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) &= ~ANACR0_ModeMask;  //clear the bits
	*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ( AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_anacr0 & ANACR0_ModeMask ) ;
	//*(volatile UInt32 *)(SYSCFG_BASE_ADDR + SYSCFG_ANACR0_OFFSET) |= ANACR0_DrvrSelMask;

    Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() currAudioMode==%d AUDDRV_GAIN_SPKR_IHF %x, *\n\r", currAudioMode,  AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga );

	AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_IHF, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_SPKR_EP, AUDIOMODE_PARM_ACCESSOR(currAudioMode).speaker_pga);

    AUDDRV_SetEquType ( AUDDRV_TYPE_AUDIO_OUTPUT, sEqualizerType );


#if 1
    {
        UInt16* coefptr;
        UInt32* coefptr32;
        int i=0,count=0;

        //set AEQPATHGAIN & AEQCOF
        memset(AUDVOC_AEQPATHGAIN,0,sizeof(AUDVOC_AEQPATHGAIN));
        AUDVOC_AEQPATHGAIN[0] = 0x0001;

        memset(AUDVOC_AEQCOF,0,sizeof(AUDVOC_AEQCOF));

        for(count = 1; count < 6; count++)
            AUDVOC_AEQCOF[ (count * 5)  - 1] = 0x1000;

		AUDDRV_SetFilter( AUDDRV_AEQPATHGAIN, AUDVOC_AEQPATHGAIN );
		AUDDRV_SetFilter( AUDDRV_AEQ, AUDVOC_AEQCOF);


        // Set AFIR
        coefptr = SYSPARM_Get_AUDVOC_ADAC_FIR_Ptr();
        coefptr32 =  & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0];
#if 0
        
        for(i=0; i< 65;i++)
            Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDVOC_ADAC_FIR[%d]= %x, *\n\r", i,  coefptr[i] );

        for(i=0; i< 60;i++)
            Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDVOC_MIXER_IIR[%d]= %x, *\n\r", i,  coefptr32[i] );
#endif


        //memset(AUDVOC_ADAC_FIR,0,sizeof(AUDVOC_ADAC_FIR));
        //AUDVOC_ADAC_FIR[32] = 0x2000;
        AUDDRV_SetFilter( AUDDRV_AFIR,coefptr);
    }

#endif
	
	//move to CP.  //to read and set this after the sys parm are readable at AP
	//AUDDRV_SetFilter( AUDDRV_AFIR,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_ADAC_FIR[0] );

	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQ,			(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQCOF[0][0] );
	//this sysparm at AP is not right. AUDDRV_SetFilter( AUDDRV_PEQPATHGAIN,	(const UInt16 *) & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHGAIN[0][0] );
	//AUDDRV_SetFilter( AUDDRV_PEQPATHOFST, & SYSPARM_GetAudioParmAccessPtr()->AUDVOC_PEQPATHOFST[0][0] );

	AUDDRV_SetFilter( AUDDRV_PIIR,			(const UInt16 *) & AUDIOMODE_PARM_MM_ACCESSOR(currMusicAudioMode).PR_DAC_IIR[0] );
    Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() currAudioMode==%d MPMbiquad_cfg %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioMode).MPMbiquad_cfg );

	AUDDRV_SetMPM( AUDDRV_Mixer_BIQUAD_CFG, AUDIOMODE_PARM_ACCESSOR(currAudioMode).MPMbiquad_cfg );

	AUDDRV_SetFilter( AUDDRV_Mixer1_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer2_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer3_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );
	AUDDRV_SetFilter( AUDDRV_Mixer4_MPMIIR, (const UInt16 *) & AUDIOMODE_PARM_ACCESSOR(currAudioMode).AUDVOC_MIXER_IIR[0] );

	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetMusicMode() currAudioMode==%d AUDDRV_GAIN_MIXER1 %x, *\n\r", currAudioMode, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	//AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
    AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER1, 0x0800 );
    AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER2, 0x0800 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER3, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MIXER4, AUDIOMODE_PARM_ACCESSOR(currAudioMode).audvoc_mixergain );

		//Open loop target gain parameters
  	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA1, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA2, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA3, 0x00 );
	AUDDRV_SetGain_Hex( AUDDRV_GAIN_MPMDGA4, 0x00 );

	AUDDRV_SetGain_Hex( AUDDRV_MIC, AUDIOMODE_PARM_ACCESSOR(currAudioMode).mic_pga);
}
#endif
//=============================================================================
//
// Function Name: AUDDRV_GetAudioMode
//
// Description:   Return Audio mode
//
//=============================================================================

AudioMode_t AUDDRV_GetAudioMode( void )
{
	return currAudioMode;
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


