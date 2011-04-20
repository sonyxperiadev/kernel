/******************************************************************************
Copyright 2009-2010 Broadcom Corporation.  All rights reserved.

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
* @file   audio_controller_caph.c
* @brief  
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"

#include "audio_consts.h"
#include "auddrv_def.h"
#ifdef LMP_BUILD
#include "sysparm.h"
#include "ostask.h"
#endif
#include "audio_gain_table.h"
#include "csl_caph.h"
#include "drv_caph.h"
#include "drv_caph_hwctrl.h"
//#include "audio_vdriver.h"
//#include "dspcmd.h"
//#include "csl_aud_drv.h"
#include "drv_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "audioapi_asic.h"
//#include "i2s.h"
#include "log.h"
#include "osheap.h"
#include "xassert.h"
#include "drv_audio_common.h"
#include "drv_audio_capture.h"
#include "drv_audio_render.h"

#ifdef LMP_BUILD
#if !defined(NO_PMU)
#include "hal_pmu.h"
#include "hal_pmu_private.h"
#endif
#endif

//=============================================================================
// Public Variable declarations
//=============================================================================


//=============================================================================
// Private Type and Constant declarations
//=============================================================================

typedef struct
{
    AUDDRV_PathID           pathID;
	AUDIO_HW_ID_t			src;
	AUDIO_HW_ID_t			sink;
	AUDCTRL_MICROPHONE_t	mic;
	AUDCTRL_SPEAKER_t		spk;
	AUDIO_CHANNEL_NUM_t		numCh;
	AUDIO_SAMPLING_RATE_t	sr;
} AUDCTRL_Config_t;

typedef struct node
{
    AUDCTRL_Config_t data;
    struct node*    next;
    struct node*    prev;
} AUDCTRL_Table_t;

static AUDCTRL_Table_t* tableHead = NULL;


typedef struct
{
    AUDIO_HW_ID_t hwID;
    AUDDRV_DEVICE_e dev;
} AUDCTRL_HWID_Mapping_t;

static AUDCTRL_HWID_Mapping_t HWID_Mapping_Table[AUDIO_HW_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDIO_HW_NONE,			AUDDRV_DEV_NONE},
	{AUDIO_HW_MEM,			AUDDRV_DEV_MEMORY},
	{AUDIO_HW_VOICE_OUT,	AUDDRV_DEV_NONE},
	{AUDIO_HW_AUDIO_OUT,	AUDDRV_DEV_NONE},
	{AUDIO_HW_PLR_OUT,		AUDDRV_DEV_NONE},
	{AUDIO_HW_MONO_BT_OUT,	AUDDRV_DEV_BT_SPKR},
	{AUDIO_HW_STEREO_BT_OUT,AUDDRV_DEV_BT_SPKR},
	{AUDIO_HW_USB_OUT,		AUDDRV_DEV_MEMORY},
	{AUDIO_HW_I2S_OUT,		AUDDRV_DEV_FM_TX},
	{AUDIO_HW_VOICE_IN,		AUDDRV_DEV_NONE},
	{AUDIO_HW_AUDIO_IN,		AUDDRV_DEV_NONE},
	{AUDIO_HW_MONO_BT_IN,	AUDDRV_DEV_BT_MIC},
	{AUDIO_HW_STEREO_BT_IN,	AUDDRV_DEV_BT_MIC},
	{AUDIO_HW_USB_IN,		AUDDRV_DEV_MEMORY},
	{AUDIO_HW_I2S_IN,		AUDDRV_DEV_FM_RADIO},
	{AUDIO_HW_TAP_VOICE,	AUDDRV_DEV_NONE},
	{AUDIO_HW_TAP_AUDIO,	AUDDRV_DEV_NONE},
	{AUDIO_HW_DSP_VOICE,	AUDDRV_DEV_DSP},
	{AUDIO_HW_DSP_TONE,		AUDDRV_DEV_DSP},
	{AUDIO_HW_EARPIECE_OUT,	AUDDRV_DEV_EP},
	{AUDIO_HW_HEADSET_OUT,	AUDDRV_DEV_HS},
	{AUDIO_HW_IHF_OUT,		AUDDRV_DEV_IHF},
	{AUDIO_HW_SPEECH_IN,	AUDDRV_DEV_ANALOG_MIC},
	{AUDIO_HW_NOISE_IN,		AUDDRV_DEV_EANC_DIGI_MIC}
};




typedef struct
{
    AUDCTRL_SPEAKER_t spkr;
    AUDDRV_DEVICE_e dev;
} AUDCTRL_SPKR_Mapping_t;

static AUDCTRL_SPKR_Mapping_t SPKR_Mapping_Table[AUDCTRL_SPK_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDCTRL_SPK_HANDSET,		AUDDRV_DEV_EP},
	{AUDCTRL_SPK_HEADSET,		AUDDRV_DEV_HS},
	{AUDCTRL_SPK_HANDSFREE,		AUDDRV_DEV_IHF},
	{AUDCTRL_SPK_BTM,		    AUDDRV_DEV_BT_SPKR},
	{AUDCTRL_SPK_LOUDSPK,		AUDDRV_DEV_IHF},
	{AUDCTRL_SPK_TTY,		    AUDDRV_DEV_HS},
	{AUDCTRL_SPK_HAC,		    AUDDRV_DEV_EP},
	{AUDCTRL_SPK_USB,		    AUDDRV_DEV_MEMORY},
	{AUDCTRL_SPK_BTS,		    AUDDRV_DEV_BT_SPKR},
	{AUDCTRL_SPK_I2S,		    AUDDRV_DEV_FM_TX},
	{AUDCTRL_SPK_VIBRA,		    AUDDRV_DEV_VIBRA},
	{AUDCTRL_SPK_UNDEFINED,		AUDDRV_DEV_NONE}
};

typedef struct
{
    AUDCTRL_SPEAKER_t spkr;
    AUDDRV_SPKR_Enum_t auddrv_spkr;
} AUDCTRL_DRVSPKR_Mapping_t;

#if 1
static AUDCTRL_DRVSPKR_Mapping_t DRVSPKR_Mapping_Table[AUDCTRL_SPK_TOTAL_COUNT] =
{
	//HW ID				// Auddrv Spkr ID
	{AUDCTRL_SPK_HANDSET,		AUDDRV_SPKR_EP},
	{AUDCTRL_SPK_HEADSET,		AUDDRV_SPKR_HS},
	{AUDCTRL_SPK_HANDSFREE,		AUDDRV_SPKR_IHF},
	{AUDCTRL_SPK_BTM,		    AUDDRV_SPKR_PCM_IF},
	{AUDCTRL_SPK_LOUDSPK,		AUDDRV_SPKR_IHF},
	{AUDCTRL_SPK_TTY,		    AUDDRV_SPKR_HS},
	{AUDCTRL_SPK_HAC,		    AUDDRV_SPKR_EP},
	{AUDCTRL_SPK_USB,		    AUDDRV_SPKR_USB_IF},
	{AUDCTRL_SPK_BTS,		    AUDDRV_SPKR_PCM_IF},
	{AUDCTRL_SPK_I2S,		    AUDDRV_SPKR_NONE},
	{AUDCTRL_SPK_VIBRA,		    AUDDRV_SPKR_VIBRA},
	{AUDCTRL_SPK_UNDEFINED,		AUDDRV_SPKR_NONE}
};
#endif

typedef struct
{
    AUDCTRL_MICROPHONE_t mic;
    AUDDRV_MIC_Enum_t auddrv_mic;
} AUDCTRL_DRVMIC_Mapping_t;

#if 1
static AUDCTRL_DRVMIC_Mapping_t DRVMIC_Mapping_Table[AUDCTRL_MIC_TOTAL_COUNT] =
{

	{AUDCTRL_MIC_UNDEFINED,		    AUDDRV_MIC_NONE},
	{AUDCTRL_MIC_MAIN,		        AUDDRV_MIC_ANALOG_MAIN},
	{AUDCTRL_MIC_AUX,		        AUDDRV_MIC_ANALOG_AUX},
	{AUDCTRL_MIC_DIGI1,     		AUDDRV_MIC_DIGI1},
	{AUDCTRL_MIC_DIGI2,		        AUDDRV_MIC_DIGI2},
	{AUDCTRL_DUAL_MIC_DIGI12,		AUDDRV_MIC_SPEECH_DIGI},
	{AUDCTRL_DUAL_MIC_DIGI21,		AUDDRV_MIC_SPEECH_DIGI},
	{AUDCTRL_DUAL_MIC_ANALOG_DIGI1,	AUDDRV_MIC_NONE},
	{AUDCTRL_DUAL_MIC_DIGI1_ANALOG,	AUDDRV_MIC_NONE},
	{AUDCTRL_MIC_BTM,		        AUDDRV_MIC_PCM_IF},
	{AUDCTRL_MIC_USB,       		AUDDRV_MIC_USB_IF},
	{AUDCTRL_MIC_I2S,		        AUDDRV_MIC_NONE},
	{AUDCTRL_MIC_DIGI3,	        	AUDDRV_MIC_DIGI3},
	{AUDCTRL_MIC_DIGI4,		        AUDDRV_MIC_DIGI4},
	{AUDCTRL_MIC_SPEECH_DIGI,       AUDDRV_MIC_SPEECH_DIGI},
	{AUDCTRL_MIC_EANC_DIGI,		    AUDDRV_MIC_EANC_DIGI}
};
#endif

typedef struct
{
    AUDCTRL_MICROPHONE_t mic;
    AUDDRV_DEVICE_e dev;
} AUDCTRL_MIC_Mapping_t;

static AUDCTRL_MIC_Mapping_t MIC_Mapping_Table[AUDCTRL_MIC_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDCTRL_MIC_UNDEFINED,		    AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_MAIN,		        AUDDRV_DEV_ANALOG_MIC},
	{AUDCTRL_MIC_AUX,		        AUDDRV_DEV_HS_MIC},
	{AUDCTRL_MIC_DIGI1,     		AUDDRV_DEV_DIGI_MIC_L},
	{AUDCTRL_MIC_DIGI2,		        AUDDRV_DEV_DIGI_MIC_R},
	{AUDCTRL_DUAL_MIC_DIGI12,		AUDDRV_DEV_DIGI_MIC},
	{AUDCTRL_DUAL_MIC_DIGI21,		AUDDRV_DEV_DIGI_MIC},
	{AUDCTRL_DUAL_MIC_ANALOG_DIGI1,	AUDDRV_DEV_NONE},
	{AUDCTRL_DUAL_MIC_DIGI1_ANALOG,	AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_BTM,		        AUDDRV_DEV_BT_MIC},
	{AUDCTRL_MIC_USB,       		AUDDRV_DEV_MEMORY},
	{AUDCTRL_MIC_I2S,		        AUDDRV_DEV_FM_RADIO},
	{AUDCTRL_MIC_DIGI3,	        	AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_DIGI4,		        AUDDRV_DEV_NONE},
	{AUDCTRL_MIC_SPEECH_DIGI,       AUDDRV_DEV_DIGI_MIC},
	{AUDCTRL_MIC_EANC_DIGI,		    AUDDRV_DEV_EANC_DIGI_MIC}
};


#ifdef LMP_BUILD
#if !defined(NO_PMU)
static PMU_HS_st_t pmu_hs;
static PMU_IHF_st_t pmu_ihf;
#endif
#endif

//static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;

//=============================================================================
// Private function prototypes
//=============================================================================
#ifdef LMP_BUILD
#if !defined(NO_PMU)
//on AP:
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetAudioParmAccessPtr();
#endif
}

/*static SysIndMultimediaAudioParm_t* AUDIO_GetParmMMAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetMultimediaAudioParmAccessPtr();
#endif
}*/

#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()[mode]
#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	 AUDIO_GetParmMMAccessPtr()[mode]

#endif
#endif
static void AUDCTRL_CreateTable(void);
static void AUDCTRL_AddToTable(AUDCTRL_Config_t* data);
static void AUDCTRL_RemoveFromTable(AUDDRV_PathID pathID);
//static void AUDCTRL_DeleteTable(void);
//static AUDCTRL_Config_t AUDCTRL_GetFromTable(AUDDRV_PathID pathID);
static AUDDRV_DEVICE_e GetDeviceFromHWID(AUDIO_HW_ID_t hwID);
static AUDDRV_DEVICE_e GetDeviceFromMic(AUDCTRL_MICROPHONE_t mic);
static AUDDRV_DEVICE_e GetDeviceFromSpkr(AUDCTRL_SPEAKER_t spkr);
static AUDDRV_PathID AUDCTRL_GetPathIDFromTable(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
                                                AUDCTRL_MICROPHONE_t mic);
static void SetGainOnExternalAmp(AUDCTRL_SPEAKER_t speaker, void* gain);


//=============================================================================
// Functions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_Init
//
// Description:   Init function
//
//============================================================================
void AUDCTRL_Init (void)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_Init::  \n"  );

	AUDDRV_Init ();

	(void)AUDDRV_HWControl_Init();
    AUDCTRL_CreateTable();
}
//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrVolume(
				AUDIO_HW_ID_t			dlSink,
				AUDCTRL_SPEAKER_t		speaker,
				UInt32					volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
#if defined(FUSE_APPS_PROCESSOR) && defined(CAPI2_INCLUDED)
	Int16 dspDLGain = 0;
	UInt8	digital_gain_dB = 0;
	UInt16	volume_max = 0;
	UInt8	OV_volume_step = 0;  //OmegaVoice volume step	
//#define  CUSTOMER_SW_SPECIFY_OV_VOLUME_STEP
#undef  CUSTOMER_SW_SPECIFY_OV_VOLUME_STEP

#if defined(CUSTOMER_SW_SPECIFY_OV_VOLUME_STEP)

	//map the digital gain to mmi level for OV. Need to adjust based on mmi level.
	static UInt8 uVol2OV[37]={
		/**
					0, 
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
				   0, 0, 0
				*/
					0, 0, 0, 0, 1, 
					0, 0, 0, 0, 2,
					0, 0, 0, 0, 3,
					0, 0, 0, 0, 4,
					0, 0, 0, 0, 5, 
					0, 0, 0, 0, 6, 
					0, 0, 0, 0, 7, 
					0, 0, 0, 0, 8
				};

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = 0x%x, OV volume step %d \n", volume, uVol2OV[volume] );
	audio_control_generic( AUDDRV_CPCMD_SetBasebandVolume, volume, gain_format, uVol2OV[volume], 0, 0);

#else
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = 0x%x\n", volume);
	if (gain_format == AUDIO_GAIN_FORMAT_VOL_LEVEL)
	{
		if( AUDIO_VOLUME_MUTE == volume )
		{  //mute
			audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkMute, 0, 0, 0, 0, 0);
		}
		else
		{
		
			volume_max = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max;  //dB
			// convert the value in 1st param from range of 2nd_param to range of 3rd_param:
			digital_gain_dB = AUDIO_Util_Convert( volume, AUDIO_VOLUME_MAX, volume_max );

			Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = 0x%x, OV volume step %d (FORMAT_VOL_LEVEL) \n", volume, OV_volume_step );

			//if parm4 (OV_volume_step) is zero, volumectrl.c will calculate OV volume step based on digital_gain_dB, VOICE_VOLUME_MAX and NUM_SUPPORTED_VOLUME_LEVELS.
			audio_control_generic( AUDDRV_CPCMD_SetBasebandVolume, digital_gain_dB, 0, OV_volume_step, 0, 0);
		}
		return;
	}
	else
	if (gain_format == AUDIO_GAIN_FORMAT_Q14_1)		
	{
        dspDLGain = (Int16)AUDDRV_GetDSPDLGain(
			GetDeviceFromSpkr(speaker),
		       	(volume<<1));
	
	audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
			(dspDLGain+100), 0, 0, 0, 0);
		return;
	}
	else	// If AUDIO_GAIN_FORMAT_Q13_2.
	if (gain_format == AUDIO_GAIN_FORMAT_Q13_2)		
	{
        dspDLGain = (Int16)AUDDRV_GetDSPDLGain(
			GetDeviceFromSpkr(speaker),
		       	volume);
	
	audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
			(dspDLGain+100), 0, 0, 0, 0);
		return;
	}
	else	// If AUDIO_GAIN_FORMAT_Q1_14, directly pass to DSP.
	if (gain_format == AUDIO_GAIN_FORMAT_Q1_14)
	{
	audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
			((Int16)volume+100), 0, 0, 0, 0);
	}	
	else	// If AUDIO_GAIN_FORMAT_HW_REG, do nothing.
	{
		return;
	}	
#endif
#endif
}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicGain
//
// Description:   Set ul gain of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				Int16					gain    //dB
				)
{

	Int16 dspULGain = 0;
        dspULGain = (Int16)AUDDRV_GetDSPULGain(
			GetDeviceFromMic(mic),
		       	gain);
	//this API only take UInt32 param. pass (gain+100) in that it is Positive integer.
	audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain, (dspULGain+100), 0, 0, 0, 0);
}

//============================================================================
//
// Function Name: AUDCTRL_EnablePlay
//
// Description:   enable a playback path
//
//============================================================================
void AUDCTRL_EnablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID;
    AUDCTRL_Config_t data;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnablePlay: src = 0x%x, sink = 0x%x, tap = 0x%x, spkr %d \n", 
                    src, sink, tap, spk);
    pathID = 0;
    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    // Enable the path. And get path ID.
    config.streamID = AUDDRV_STREAM_NONE; 
    config.pathID = 0;
    config.source = GetDeviceFromHWID(src);
    config.sink =  GetDeviceFromSpkr(spk);
    config.dmaCH = CSL_CAPH_DMA_NONE; 
    config.src_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
    config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

	if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDCTRL_SPK_USB)
	{	//USB call
		config.source = AUDDRV_DEV_DSP;
		config.sink = AUDDRV_DEV_MEMORY;
	}

	if(sink == AUDIO_HW_STEREO_BT_OUT || sink == AUDIO_HW_USB_OUT)
		;
	else
		pathID = AUDDRV_HWControl_EnablePath(config);

    //Save this path to the path table.
    data.pathID = pathID;
    data.src = src;
    data.sink = sink;
    data.mic = AUDCTRL_MIC_UNDEFINED;
    data.spk = spk;
    data.numCh = numCh;
    data.sr = sr;
    AUDCTRL_AddToTable(&data);

    //Enable the PMU for HS/IHF.
    if ((sink == AUDIO_HW_HEADSET_OUT)||(sink == AUDIO_HW_IHF_OUT)) 
    {
		//powerOnExternalAmp( spk, AudioUseExtSpkr, TRUE );
    }

#if 0
	// in case it was muted from last play,
	AUDCTRL_SetPlayMute (sink, spk, FALSE); 
#endif    
	// Enable DSP DL for Voice Call.
	if(config.source == AUDDRV_DEV_DSP)
	{
		AUDDRV_EnableDSPOutput(DRVSPKR_Mapping_Table[spk].auddrv_spkr, sr);
	}


}
//
// Function Name: AUDCTRL_DisablePlay
//
// Description:   disable a playback path
//
//============================================================================
void AUDCTRL_DisablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID = 0;
	
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisablePlay: src = 0x%x, sink = 0x%x, spk = 0x%x\n", 
                    src, sink,  spk);

    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
    pathID = AUDCTRL_GetPathIDFromTable(src, sink, spk, AUDCTRL_MIC_UNDEFINED);

	if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDCTRL_SPK_USB)
	{	//USB call
		config.source = AUDDRV_DEV_DSP;
		config.sink = AUDDRV_DEV_MEMORY;
	}

	if(sink == AUDIO_HW_STEREO_BT_OUT || sink == AUDIO_HW_USB_OUT)
		;
	else
	{
		config.pathID = pathID;
		(void) AUDDRV_HWControl_DisablePath(config);
	}

        //Save this path to the path table.
    AUDCTRL_RemoveFromTable(pathID);

    //Disable the PMU for HS/IHF.
    if ((sink == AUDIO_HW_HEADSET_OUT)||(sink == AUDIO_HW_IHF_OUT)) 
    {
		//powerOnExternalAmp( spk, AudioUseExtSpkr, FALSE );
    }
}
//============================================================================
//
// Function Name: AUDCTRL_SetPlayVolume
//
// Description:   set volume of a playback path
//
//============================================================================
void AUDCTRL_SetPlayVolume(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_GAIN_FORMAT_t     gainF,
				UInt32					vol_left,
				UInt32					vol_right
				)
{
    UInt32 gainHW, gainHW2;
    AUDTABL_GainMapping_t gainMapping;
    AUDDRV_PathID pathID = 0;
    AudioMode_t audioMode = AUDIO_MODE_INVALID;

    gainHW = gainHW2 = 0;
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: Set Play Volume. sink = 0x%x,  spk = 0x%x, vol = 0x%x\n", sink, spk, vol_left);
    
    memset(&gainMapping, 0, sizeof(AUDTABL_GainMapping_t));
    switch(sink)
    {
        case AUDIO_HW_IHF_OUT:
            audioMode = AUDIO_MODE_SPEAKERPHONE;
            break;
        case AUDIO_HW_HEADSET_OUT:
            audioMode = AUDIO_MODE_HEADSET;
            break;
        case AUDIO_HW_EARPIECE_OUT:
            audioMode = AUDIO_MODE_HANDSET;
            break;
        case AUDIO_HW_MONO_BT_OUT:
        case AUDIO_HW_STEREO_BT_OUT:
            audioMode = AUDIO_MODE_BLUETOOTH;
            break;
        case AUDIO_HW_USB_OUT:
            audioMode = AUDIO_MODE_USB;
            break;
        default:
            audioMode = AUDIO_MODE_INVALID;
    }

    gainMapping = AUDTABL_getGainDistribution(audioMode, vol_left);
    gainHW = gainMapping.gainHW;
    if (gainHW == TOTAL_GAIN)
    {
        gainHW = vol_left;
    }
    gainMapping = AUDTABL_getGainDistribution(audioMode, vol_right);
    gainHW2 = gainMapping.gainHW;
    if (gainHW2 == TOTAL_GAIN)
    {
        gainHW2 = vol_right;
    }

	if(sink == AUDIO_HW_STEREO_BT_OUT || sink == AUDIO_HW_USB_OUT)
		return;
    
    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE, sink, spk, AUDCTRL_MIC_UNDEFINED);

    // Set the gain to the audio HW.
    (void) AUDDRV_HWControl_SetSinkGain(pathID, gainHW, gainHW2);

    // Set teh gain to the external amplifier
    //SetGainOnExternalAmp(spk, &(gainMapping.gainPMU));

    return;
}

//============================================================================
//
// Function Name: AUDCTRL_SetPlayMute
//
// Description:   mute/unmute a playback path
//
//============================================================================
void AUDCTRL_SetPlayMute(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
    AUDDRV_PathID pathID = 0;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetPlayMute: sink = 0x%x,  spk = 0x%x, mute = 0x%x\n", 
                    sink, spk, mute);

	if(sink == AUDIO_HW_STEREO_BT_OUT || sink == AUDIO_HW_USB_OUT)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE, sink, spk, AUDCTRL_MIC_UNDEFINED);

    if (mute == TRUE)
    {
        (void) AUDDRV_HWControl_MuteSink(pathID);
    }
    else
    {
        (void) AUDDRV_HWControl_UnmuteSink(pathID);
    }
    return;
}

//============================================================================
//
// Function Name: AUDCTRL_AddPlaySpk
//
// Description:   add a speaker to a playback path
//
//============================================================================
void AUDCTRL_AddPlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RemovePlaySpk
//
// Description:   remove a speaker to a playback path
//
//============================================================================
void AUDCTRL_RemovePlaySpk(
				AUDIO_HW_ID_t			sink,
				AUDCTRL_SPEAKER_t		spk
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_EnableRecordMono
//
// Description:   enable a record path for single mic
//
//============================================================================
static void AUDCTRL_EnableRecordMono(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr)
{
    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID;
    AUDCTRL_Config_t data;

    pathID = 0;
    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    // Enable the path. And get path ID.
    config.streamID = AUDDRV_STREAM_NONE; 
    config.pathID = 0;
    config.source = GetDeviceFromMic(mic);
    config.sink =  GetDeviceFromHWID(sink);
    config.dmaCH = CSL_CAPH_DMA_NONE; 
    config.snk_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
	config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

	if(src == AUDIO_HW_STEREO_BT_IN)
	{
		;
	}
	else
	{
		if (src == AUDIO_HW_USB_IN && sink == AUDIO_HW_DSP_VOICE)
		{
			// in this case, the entire data pass is 
			// USB Mic(48K mono) --> DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP input --> DSP sharedmem --> DDR
			// for HW control, need to setup the caph path DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP.
			// the caph path source is MEMORY, the capth path sink is DSP. Also need to set the input sampling rate as 48K, and output sampling rate as 8K or 16 (depending on 
			// the passed in parameter sr), so we know we need to use the HW srcMixer.
			config.source = AUDDRV_DEV_MEMORY;
			config.sink = AUDDRV_DEV_DSP;
		}
		pathID = AUDDRV_HWControl_EnablePath(config);
	}
	
	Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_EnableRecordMono: path configuration, source = %d, sink = %d, pathID %d.\r\n", config.source, config.sink, pathID);

    //Save this path to the path table.
    data.pathID = pathID;
    data.src = src;
    data.sink = sink;
    data.mic = mic;
    data.spk = AUDCTRL_SPK_UNDEFINED;
    data.numCh = numCh;
    data.sr = sr;
    AUDCTRL_AddToTable(&data);
    
#if 0
	// in case it was muted from last record
	AUDCTRL_SetRecordMute (src, mic, FALSE); 
#endif
	// Enable DSP UL for Voice Call.
	if(config.sink == AUDDRV_DEV_DSP)
	{
		AUDDRV_EnableDSPInput(DRVMIC_Mapping_Table[mic].auddrv_mic, sr);
	}
}

//============================================================================
//
// Function Name: AUDCTRL_EnableRecord
//
// Description:   enable a record path
//
//============================================================================
void AUDCTRL_EnableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic,
				AUDIO_CHANNEL_NUM_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x, sr %d\n",
                    src, sink, mic, sr);

	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDDRV_MIC_SPEECH_DIGI))		
	{
		// Enable power to digital microphone
		//powerOnDigitalMic(TRUE);
	}

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 
			|| mic==AUDCTRL_DUAL_MIC_DIGI21 
			|| mic==AUDDRV_MIC_SPEECH_DIGI)
	{
		AUDCTRL_EnableRecordMono(src, sink, AUDCTRL_MIC_DIGI1, AUDIO_CHANNEL_MONO, sr);
		AUDCTRL_EnableRecordMono(src, sink, AUDCTRL_MIC_DIGI2, AUDIO_CHANNEL_MONO, sr);
	} else {
		AUDCTRL_EnableRecordMono(src, sink, mic, numCh, sr);
	}
}

//============================================================================
//
// Function Name: AUDCTRL_DisableRecord
//
// Description:   disable a record path
//
//============================================================================
void AUDCTRL_DisableRecord(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDCTRL_MICROPHONE_t	mic
				)
{

    AUDDRV_HWCTRL_CONFIG_t config;
    AUDDRV_PathID pathID = 0;
	
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x\n", 
                    src, sink, mic);

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 
			|| mic==AUDCTRL_DUAL_MIC_DIGI21 
			|| mic==AUDDRV_MIC_SPEECH_DIGI)
		
	{
		memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, AUDCTRL_MIC_DIGI1);
		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) AUDDRV_HWControl_DisablePath(config);
		AUDCTRL_RemoveFromTable(pathID);

		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, AUDCTRL_MIC_DIGI2);
		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) AUDDRV_HWControl_DisablePath(config);
		AUDCTRL_RemoveFromTable(pathID);
	} else {
		memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, mic);

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		
		if(src == AUDIO_HW_STEREO_BT_IN)
		{
			;
		}
		else 
		{
			if (src == AUDIO_HW_USB_IN && sink == AUDIO_HW_DSP_VOICE)
			{
				// in this case, the entire data pass is 
				// USB Mic(48K mono) --> DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP input --> DSP sharedmem --> DDR
				// for HW control, need to setup the caph path DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP.
				// the caph path source is MEMORY, the capth path sink is DSP. Also need to set the input sampling rate as 48K, and output sampling rate as 8K or 16 (depending on 
				// the passed in parameter sr), so we know we need to use the HW srcMixer.
				config.source = AUDDRV_DEV_MEMORY;
				config.sink = AUDDRV_DEV_DSP;
			}

			(void) AUDDRV_HWControl_DisablePath(config);
		}
		

		//Remove this path from the path table.
		AUDCTRL_RemoveFromTable(pathID);
	}
	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDDRV_MIC_SPEECH_DIGI))		
	{
		// Disable power to digital microphone
		//powerOnDigitalMic(FALSE);
	}	
}


//============================================================================
//
// Function Name: AUDCTRL_AddRecordMic
//
// Description:   add a microphone to a record path
//
//============================================================================
void AUDCTRL_AddRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_RemoveRecordMic
//
// Description:   remove a microphone from a record path
//
//============================================================================
void AUDCTRL_RemoveRecordMic(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic
				)
{
	// Nothing to do.
}

//============================================================================
//
// Function Name: AUDCTRL_EnableTap
//
// Description:   enable a tap path
//
//============================================================================
void AUDCTRL_EnableTap(
				AUDIO_HW_ID_t			tap,
				AUDCTRL_SPEAKER_t		spk,
				AUDIO_SAMPLING_RATE_t	sr
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_DisableTap
//
// Description:   disable a tap path
//
//============================================================================
void AUDCTRL_DisableTap( AUDIO_HW_ID_t	tap)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordGain
//
// Description:   set gain of a record path
//
//============================================================================
void AUDCTRL_SetRecordGain(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				UInt32					gainL,
				UInt32					gainR
				)
{
    AUDDRV_PathID pathID = 0;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordGain: src = 0x%x,  mic = 0x%x, gainL = 0x%x, gainR = 0x%x\n", src, mic, gainL, gainR);

	if(src == AUDIO_HW_STEREO_BT_IN || src == AUDIO_HW_USB_IN)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(src, AUDIO_HW_NONE, AUDCTRL_SPK_UNDEFINED, mic);

    (void) AUDDRV_HWControl_SetSourceGain(pathID, gainL, gainR);

    return;
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordMute
//
// Description:   mute/unmute a record path
//
//============================================================================
void AUDCTRL_SetRecordMute(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
    AUDDRV_PathID pathID = 0;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMute: src = 0x%x,  mic = 0x%x, mute = 0x%x\n", 
                    src, mic, mute);

	if(src == AUDIO_HW_STEREO_BT_IN || src == AUDIO_HW_USB_IN)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(src, AUDIO_HW_NONE, AUDCTRL_SPK_UNDEFINED, mic);

    if (mute == TRUE)
    {
        (void) AUDDRV_HWControl_MuteSource(pathID);
    }
    else
    {
        (void) AUDDRV_HWControl_UnmuteSource(pathID);
    }
    return;    
}

//============================================================================
//
// Function Name: AUDCTRL_SetTapGain
//
// Description:   set gain of a tap path
//
//============================================================================
void AUDCTRL_SetTapGain(
				AUDIO_HW_ID_t			tap,
				UInt32					gain
				)
{
}

//============================================================================
//
// Function Name: AUDCTRL_SetAudioLoopback
//
// Description:   Set the loopback path
// 
//============================================================================
void AUDCTRL_SetAudioLoopback( 
                              Boolean enable_lpbk,
                              AUDCTRL_MICROPHONE_t mic,
                              AUDCTRL_SPEAKER_t	speaker
                             )
{
    AUDDRV_DEVICE_e source, sink;
    static AUDDRV_SPKR_Enum_t audSpkr;
    static AUDDRV_MIC_Enum_t audMic;
    AUDDRV_PathID pathID;
    AUDCTRL_Config_t data;
    AUDIO_HW_ID_t audPlayHw, audRecHw;

    AUDDRV_HWCTRL_CONFIG_t hwCtrlConfig;

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);

    source = sink = AUDDRV_DEV_NONE;
    switch (mic)
    {
        case AUDCTRL_MIC_MAIN:
            source = AUDDRV_DEV_ANALOG_MIC;
            audMic = AUDDRV_MIC_ANALOG_MAIN;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDCTRL_MIC_AUX:
            source = AUDDRV_DEV_HS_MIC;
            audMic = AUDDRV_MIC_ANALOG_AUX;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDCTRL_MIC_SPEECH_DIGI:
            source = AUDDRV_DEV_DIGI_MIC;
            audMic = AUDDRV_MIC_SPEECH_DIGI;
            break;	    
        case AUDCTRL_MIC_DIGI1:
            source = AUDDRV_DEV_DIGI_MIC_L;
            audMic = AUDDRV_MIC_DIGI1;
            break;
        case AUDCTRL_MIC_DIGI2:
            source = AUDDRV_DEV_DIGI_MIC_R;
            audMic = AUDDRV_MIC_DIGI2;
            break;
        case AUDCTRL_MIC_DIGI3:
            source = AUDDRV_DEV_EANC_DIGI_MIC_L;
            audMic = AUDDRV_MIC_DIGI3;
            break;
        case AUDCTRL_MIC_DIGI4:
            source = AUDDRV_DEV_EANC_DIGI_MIC_R;
            audMic = AUDDRV_MIC_DIGI4;
            break;
        case AUDCTRL_MIC_I2S:
            source = AUDDRV_DEV_FM_RADIO;
            audRecHw = AUDIO_HW_I2S_IN;
            break;
        case AUDCTRL_MIC_BTM:
            source = AUDDRV_DEV_BT_MIC;
            audRecHw = AUDIO_HW_MONO_BT_IN;
            break;
        default:
            audMic = AUDDRV_MIC_ANALOG_MAIN;
            source = AUDDRV_DEV_ANALOG_MIC;
            audRecHw = AUDIO_HW_I2S_IN;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
            break;
    }

    switch (speaker)
    {
        case AUDCTRL_SPK_HANDSET:
            sink = AUDDRV_DEV_EP;
            audSpkr = AUDDRV_SPKR_EP;
            audPlayHw = AUDIO_HW_EARPIECE_OUT;
            break;
        case AUDCTRL_SPK_HEADSET:
            sink = AUDDRV_DEV_HS;
            audSpkr = AUDDRV_SPKR_HS;
            audPlayHw = AUDIO_HW_HEADSET_OUT;
            break;
        case AUDCTRL_SPK_LOUDSPK:
            sink = AUDDRV_DEV_IHF;
            audSpkr = AUDDRV_SPKR_IHF;
            break;
        case AUDCTRL_SPK_I2S:
            sink = AUDDRV_DEV_FM_TX;
            audPlayHw = AUDIO_HW_I2S_OUT;
            break;
        case AUDCTRL_SPK_BTM:
            sink = AUDDRV_DEV_BT_SPKR;
            audPlayHw = AUDIO_HW_MONO_BT_OUT;
            break;
        default:
            audSpkr = AUDDRV_SPKR_EP;
            sink = AUDDRV_DEV_EP;
            audPlayHw = AUDIO_HW_EARPIECE_OUT;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);
            break;
    }

    if(enable_lpbk)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: Enable loopback \n");

	// For I2S/PCM loopback
        if (((source == AUDDRV_DEV_FM_RADIO) && (sink == AUDDRV_DEV_FM_TX)) ||
		((source == AUDDRV_DEV_BT_MIC) && (sink == AUDDRV_DEV_BT_SPKR)))
        {
            // I2S hard coded to use ssp3, BT PCM to use ssp4. This could be changed later
            AUDCTRL_EnablePlay (AUDIO_HW_SPEECH_IN, audPlayHw, AUDIO_HW_NONE, speaker, AUDIO_CHANNEL_MONO, 48000);
            AUDCTRL_EnableRecord (audRecHw, AUDIO_HW_EARPIECE_OUT, mic, AUDIO_CHANNEL_MONO, 48000);
            return;
        }

	    if (source == AUDDRV_DEV_FM_RADIO)
	    {
            AUDCTRL_EnableRecord (audRecHw, audPlayHw, mic, AUDIO_CHANNEL_STEREO, 48000);
	        if ((speaker == AUDCTRL_SPK_LOUDSPK)||(speaker == AUDCTRL_SPK_HEADSET))	
	            //powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );	    
	        return;
	    }

        //  Microphone pat
	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDDRV_MIC_SPEECH_DIGI))		
	{
		// Enable power to digital microphone
		//powerOnDigitalMic(TRUE);
	}
	// enable HW path
        hwCtrlConfig.streamID = AUDDRV_STREAM_NONE;
        hwCtrlConfig.source = source;
        hwCtrlConfig.sink = sink;
        hwCtrlConfig.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.chnlNum = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        hwCtrlConfig.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

        pathID = AUDDRV_HWControl_EnablePath(hwCtrlConfig);

        // Enable Loopback ctrl
	if ((source == AUDDRV_DEV_ANALOG_MIC) && ((sink == AUDDRV_DEV_EP) || (sink == AUDDRV_DEV_IHF)))
            AUDDRV_SetAudioLoopback(enable_lpbk, audMic, audSpkr, 0);

        //Save this path to the path table.
        data.pathID = pathID;
        data.src = AUDIO_HW_VOICE_IN;
        data.sink = AUDIO_HW_VOICE_OUT;
        data.mic = mic;
        data.spk = speaker;
        data.numCh = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        data.sr = AUDIO_SAMPLING_RATE_48000;
        AUDCTRL_AddToTable(&data);
	//Enable PMU for headset/IHF
	if ((speaker == AUDCTRL_SPK_LOUDSPK)
	    ||(speaker == AUDCTRL_SPK_HEADSET))	
	{
	//powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );	   
	} 
    } else {
        // Disable Analog Mic path
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: Disable loopback\n");

	// Disable I2S/PCM loopback
        if (((source == AUDDRV_DEV_FM_RADIO) && (sink == AUDDRV_DEV_FM_TX)) ||
		((source == AUDDRV_DEV_BT_MIC) && (sink == AUDDRV_DEV_BT_SPKR)))
        {
            // I2S configured to use ssp3, BT PCM to use ssp4.
            AUDCTRL_DisablePlay (AUDIO_HW_SPEECH_IN, audPlayHw, speaker);
            AUDCTRL_DisableRecord (audRecHw, AUDIO_HW_EARPIECE_OUT, mic);
            return;
        }

	    if (source == AUDDRV_DEV_FM_RADIO)
	    {
            AUDCTRL_DisableRecord (audRecHw, audPlayHw, mic);
	        if ((speaker == AUDCTRL_SPK_LOUDSPK)||(speaker == AUDCTRL_SPK_HEADSET))	
	            //powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );	    
	        return;
	    }

	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDDRV_MIC_SPEECH_DIGI))		
	{
		// Enable power to digital microphone
		//powerOnDigitalMic(FALSE);
	}	
	//Enable PMU for headset/IHF
	if ((speaker == AUDCTRL_SPK_LOUDSPK)
	    ||(speaker == AUDCTRL_SPK_HEADSET))	
	//powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );	    

        memset(&hwCtrlConfig, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
        pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_VOICE_IN, AUDIO_HW_VOICE_OUT, speaker, mic);
        hwCtrlConfig.pathID = pathID;
        (void) AUDDRV_HWControl_DisablePath(hwCtrlConfig);

	if ((source == AUDDRV_DEV_ANALOG_MIC) && ((sink == AUDDRV_DEV_EP) || (sink == AUDDRV_DEV_IHF)))
        AUDDRV_SetAudioLoopback(enable_lpbk, audMic, audSpkr, 0);

        //Remove this path to the path table.
        AUDCTRL_RemoveFromTable(pathID);
    }
}

void AUDCTRL_SetEQ( 
				AUDIO_HW_ID_t	audioPath,
				AudioEqualizer_en_t  equType
				)
{ 
	/* Will fill in code.
	AUDDRV_SetEquType( AUDDRV_TYPE_AUDIO_OUTPUT, equType );
	AUDDRV_SetEquType( AUDDRV_TYPE_RINGTONE_OUTPUT, equType );
	*/
}

void AUDCTRL_ConfigSSP( UInt8 fm_port, UInt8 pcm_port)
{
	AUDDRV_HWControl_ConfigSSP (fm_port, pcm_port);
}

//============================================================================
//
// Function Name: AUDCTRL_EnableBypassVibra
//
// Description:   Enable the Vibrator bypass
// 
//============================================================================
 void  AUDCTRL_EnableBypassVibra(void)
 {
	 AUDDRV_HWControl_EnableVibrator(TRUE, AUDDRV_VIBRATOR_BYPASS_MODE);
 }

//============================================================================
//
// Function Name: AUDCTRL_EnableBypassVibra
//
// Description:   Disable the Vibrator bypass
// 
//============================================================================
 void  AUDCTRL_DisableBypassVibra(void)
 {
	 AUDDRV_HWControl_EnableVibrator(FALSE, AUDDRV_VIBRATOR_BYPASS_MODE);
 }

//============================================================================
//
// Function Name: AUDCTRL_SetBypassVibraStrength
//
// Description:   Set the strenth to vibrator
// 
//============================================================================
 void  AUDCTRL_SetBypassVibraStrength(UInt32 Strength, int direction)
 {
	 UInt32 vib_power;

	 vib_power = (0x7fff/100)*Strength;

	 Strength = ((Strength > 100) ? 100 : Strength);
	 vib_power = ((direction == 0) ?  vib_power : (0xffff - vib_power + 1 ));

	 AUDDRV_HWControl_VibratorStrength(vib_power);
 }



//=============================================================================
// Private function definitions
//=============================================================================

//============================================================================
//
// Function Name: AUDCTRL_CreateTable
//
// Description:   Create the Table to record the path information.
//
//============================================================================

static void AUDCTRL_CreateTable(void)
{
    tableHead = NULL;
    return;
}
//============================================================================
//
// Function Name: AUDCTRL_AddToTable
//
// Description:   Add a new path into the Table.
//
//============================================================================
static void AUDCTRL_AddToTable(AUDCTRL_Config_t* data)
{
    AUDCTRL_Table_t* newNode = NULL;
    newNode = (AUDCTRL_Table_t *)OSHEAP_Alloc(sizeof(AUDCTRL_Table_t));
	memset(newNode, 0, sizeof(AUDCTRL_Table_t));
    memcpy(&(newNode->data), data, sizeof(AUDCTRL_Config_t));
    newNode->next = tableHead;
    newNode->prev = NULL;
    if (tableHead != NULL)
	    tableHead->prev = newNode;
    tableHead = newNode;
    return;
}
//============================================================================
//
// Function Name: AUDCTRL_RemoveFromTable
//
// Description:   Remove a path from the table.
//
//============================================================================
static void AUDCTRL_RemoveFromTable(AUDDRV_PathID pathID)
{
    AUDCTRL_Table_t* currentNode = tableHead;
    while(currentNode != NULL)
    {
        if ((currentNode->data).pathID == pathID)
        {
            //memset(&(current->data), 0, sizeof(AUDCTRL_Config_t));
			if(currentNode->prev)
            {
                currentNode->prev->next = currentNode->next;
		if (currentNode->next != NULL)
                    currentNode->next->prev = currentNode->prev;
            }
            else if(currentNode->next)
            {
                tableHead = currentNode->next;
                tableHead->prev = NULL;
            }
            else
                tableHead = NULL;
            OSHEAP_Delete(currentNode); 
            currentNode = NULL;
        }
        else
        {
            currentNode = currentNode->next;
        }
    }
    return;
}
//============================================================================
//
// Function Name: AUDCTRL_DeleteTable
//
// Description:   Delete the whole table.
//
//============================================================================
#if 0
static void AUDCTRL_DeleteTable(void)
{
    AUDCTRL_Table_t* current = tableHead;
    AUDCTRL_Table_t* next = NULL;

    while(current != NULL)
    {
        next = current->next;
        memset(current, 0, sizeof(AUDCTRL_Table_t));
   	    OSHEAP_Delete(current); 
        current = next;
    }
    tableHead = NULL;
    return;
}
#endif

//============================================================================
//
// Function Name: AUDCTRL_GetFromTable
//
// Description:   Get a path information from the table.
//
//============================================================================
#if 0
static AUDCTRL_Config_t AUDCTRL_GetFromTable(AUDDRV_PathID pathID)
{
    AUDCTRL_Config_t data; 
    AUDCTRL_Table_t* current = tableHead; 
    memset(&(current->data), 0, sizeof(AUDCTRL_Config_t));

    while(current != NULL)
    {
        if ((current->data).pathID == pathID)
        {
            memcpy(&data, &(current->data), sizeof(AUDCTRL_Config_t));
        }
        else
        {
            current = current->next;
        }
    }
    return data;
}
#endif


//============================================================================
//
// Function Name: AUDCTRL_GetPathIDFromTable
//
// Description:   Get a path ID from the table.
//
//============================================================================
static AUDDRV_PathID AUDCTRL_GetPathIDFromTable(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
				                                AUDCTRL_MICROPHONE_t mic)
{
    AUDCTRL_Table_t* currentNode = tableHead;     
    while(currentNode != NULL)
    {
        if ((((currentNode->data).src == src)&&((currentNode->data).mic == mic))
            ||(((currentNode->data).sink == sink)&&((currentNode->data).spk == spk)))
        {
            return (currentNode->data).pathID;
        }
        else
        {
            currentNode = currentNode->next;
        }
    }
    return 0;
}

//============================================================================
//
// Function Name: GetDeviceFromHWID
//
// Description:   convert audio controller HW ID enum to auddrv device enum
//
//============================================================================
static AUDDRV_DEVICE_e GetDeviceFromHWID(AUDIO_HW_ID_t hwID)
{
	Log_DebugPrintf(LOGID_AUDIO,"GetDeviceFromHWID: hwID = 0x%x\n", hwID);
    return HWID_Mapping_Table[hwID].dev;
}


//============================================================================
//
// Function Name: GetDeviceFromMic
//
// Description:   convert audio controller Mic enum to auddrv device enum
//
//============================================================================
static AUDDRV_DEVICE_e GetDeviceFromMic(AUDCTRL_MICROPHONE_t mic)
{
	Log_DebugPrintf(LOGID_AUDIO,"GetDeviceFromMic: hwID = 0x%x\n", mic);
    return MIC_Mapping_Table[mic].dev;
}


//============================================================================
//
// Function Name: GetDeviceFromSpkr
//
// Description:   convert audio controller Spkr enum to auddrv device enum
//
//============================================================================
static AUDDRV_DEVICE_e GetDeviceFromSpkr(AUDCTRL_SPEAKER_t spkr)
{
	Log_DebugPrintf(LOGID_AUDIO,"GetDeviceFromSpkr: hwID = 0x%x\n", spkr);
    return SPKR_Mapping_Table[spkr].dev;
}

#if 0
//============================================================================
//
// Function Name: GetDrvMic
//
// Description:   convert audio controller microphone enum to auddrv microphone enum
//
//============================================================================
static AUDDRV_MIC_Enum_t GetDrvMic (AUDCTRL_MICROPHONE_t mic)
{
	AUDDRV_MIC_Enum_t micSel=AUDDRV_MIC_MAIN;

	// microphone selection. We hardcode microphone for headset,handset and loud right now. 
	// Later, need to provide a configurable table.
	switch (mic)
	{
		case AUDCTRL_MIC_MAIN:
			micSel = AUDDRV_MIC_MAIN;
			break;
		case AUDCTRL_MIC_AUX:
			micSel = AUDDRV_MIC_AUX;
			break;

		case AUDCTRL_MIC_DIGI1:
			micSel = AUDDRV_MIC_DIGI1;
			break;
		case AUDCTRL_MIC_DIGI2:
			micSel = AUDDRV_MIC_DIGI2;
			break;
		case AUDCTRL_DUAL_MIC_DIGI12:
			micSel = AUDDRV_DUAL_MIC_DIGI12;
			break;
		case AUDCTRL_DUAL_MIC_DIGI21:
			micSel = AUDDRV_DUAL_MIC_DIGI21;
			break;
		case AUDCTRL_DUAL_MIC_ANALOG_DIGI1:
			micSel = AUDDRV_DUAL_MIC_ANALOG_DIGI1;
			break;
		case AUDCTRL_DUAL_MIC_ANALOG_DIGI2:
			micSel = AUDDRV_DUAL_MIC_ANALOG_DIGI2;
			break;

		case AUDCTRL_MIC_BTM:
			micSel = AUDDRV_MIC_PCM_IF;
			break;
		//case AUDCTRL_MIC_BTS:
			//break;
		case AUDCTRL_MIC_I2S:
			break;

		case AUDCTRL_MIC_USB:
		default:
			Log_DebugPrintf(LOGID_AUDIO,"GetDrvMic: Unsupported microphpne type. mic = 0x%x\n", mic);
			break;
	}

	return micSel;
}


//============================================================================
//
// Function Name: GetDrvSpk
//
// Description:   convert audio controller speaker enum to auddrv speaker enum
//
//============================================================================
static AUDDRV_MIXER_SPKR_Enum_t GetDrvSpk (AUDCTRL_SPEAKER_t speaker)
{
	AUDDRV_MIXER_SPKR_Enum_t spkSel = AUDDRV_SPKR_NONE;

	Log_DebugPrintf(LOGID_AUDIO,"GetDrvSpk: spk = 0x%x\n", speaker);

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	switch (speaker)
	{
		case AUDCTRL_SPK_HANDSET:
			spkSel = AUDDRV_MIXER2_SPKR_1R;
			break;
		case AUDCTRL_SPK_HEADSET:
			spkSel = AUDDRV_MIXER34_SPKR_2LR;
			break;
		case AUDCTRL_SPK_LOUDSPK:
			spkSel = AUDDRV_MIXER1_SPKR_1L;
			break;
		case AUDCTRL_SPK_BTM:
			spkSel = AUDDRV_SPKR_PCM_IF;
			break;
		case AUDCTRL_SPK_BTS:
			break;
		
		case AUDCTRL_SPK_I2S:
			break;

		case AUDCTRL_SPK_USB:
			break;
		default:
			Log_DebugPrintf(LOGID_AUDIO,"GetDrvSpk: Unsupported Speaker type. spk = 0x%x\n", speaker);
			break;
	}

	return spkSel;
}

#endif
#ifdef LMP_BUILD
#if !defined(NO_PMU) && ( defined( PMU_BCM59038) || defined( PMU_BCM59055 ) || defined( PMU_MAX8986) )

static PMU_HS_Gain_t map2pmu_hs_gain( Int16 db_gain )
{
#if defined(PMU_MAX8986)

	Log_DebugPrintf(LOGID_AUDIO,"map2pmu_hs_gain: gain = 0x%x\n", db_gain);

	if ( db_gain== -19 ) 	return PMU_HSGAIN_19DB_N;
	if ( db_gain== -18 || db_gain== -17 || db_gain== -16)		return PMU_HSGAIN_16DB_N;
	if ( db_gain== -15 || db_gain== -14)		return PMU_HSGAIN_14DB_N;
	if ( db_gain== -13 || db_gain== -12)		return PMU_HSGAIN_12DB_N;
	if ( db_gain== -11 || db_gain== -10)		return PMU_HSGAIN_10DB_N;
	if ( db_gain== -9 ||  db_gain== -8)		return PMU_HSGAIN_8DB_N;
	if ( db_gain== -7 ||  db_gain== -6)		return PMU_HSGAIN_6DB_N;
	if ( db_gain== -5 ||  db_gain== -4)		return PMU_HSGAIN_4DB_N;
	if ( db_gain== -3 ||  db_gain== -2)		return PMU_HSGAIN_2DB_N;
	if ( db_gain== -1 )		return PMU_HSGAIN_1DB_N;
	if ( db_gain== 0 )		return PMU_HSGAIN_0DB;
	if ( db_gain== 1 )		return PMU_HSGAIN_1DB_P;
	if ( db_gain== 2 )		return PMU_HSGAIN_2DB_P;
	if ( db_gain== 3 )		return PMU_HSGAIN_3DB_P;
	if ( db_gain== 4 )		return PMU_HSGAIN_4DB_P;
	// PMU_HSGAIN_4P5DB_P
	if ( db_gain== 5 )		return PMU_HSGAIN_5DB_P;
	// PMU_HSGAIN_5P5DB_P
	if ( db_gain== 6 )		return PMU_HSGAIN_6DB_P;

#else
	if ( db_gain== -18 )  	return PMU_HSGAIN_18DB_N;
	if ( db_gain== -17)  	return PMU_HSGAIN_17DB_N;
	if ( db_gain== -16)		return PMU_HSGAIN_16DB_N;
	if ( db_gain== -15) 	return PMU_HSGAIN_15DB_N;
	if ( db_gain== -14)		return PMU_HSGAIN_14DB_N;
	if ( db_gain== -13)		return PMU_HSGAIN_13DB_N;
	if ( db_gain== -12)		return PMU_HSGAIN_12DB_N;
	if ( db_gain== -11)		return PMU_HSGAIN_11DB_N;
	if ( db_gain== -10)		return PMU_HSGAIN_10DB_N;
	if ( db_gain== -9)		return PMU_HSGAIN_9DB_N;
	if ( db_gain== -8)		return PMU_HSGAIN_8DB_N;
	if ( db_gain== -7)		return PMU_HSGAIN_7DB_N;
	if ( db_gain== -6)		return PMU_HSGAIN_6DB_N;
	if ( db_gain== -5)		return PMU_HSGAIN_5DB_N;
	if ( db_gain== -4)		return PMU_HSGAIN_4DB_N;
	if ( db_gain== -3)		return PMU_HSGAIN_3DB_N;
	if ( db_gain== -2)		return PMU_HSGAIN_2DB_N;
#endif
	
	return PMU_HSGAIN_2DB_N;
}

static PMU_IHF_Gain_t map2pmu_ihf_gain( Int16 db_gain )
{
#if defined(PMU_MAX8986)	

    Log_DebugPrintf(LOGID_AUDIO,"map2pmu_ihf_gain: gain = 0x%x\n", db_gain);

    if ( db_gain== -33 || db_gain== -32 || db_gain== -31 || db_gain== -30 ) return PMU_IHFGAIN_30DB_N;
    if ( db_gain== -29 || db_gain== -28 || db_gain== -27 || db_gain== -26 ) return PMU_IHFGAIN_26DB_N;
    if ( db_gain== -25 || db_gain== -24 || db_gain== -23 || db_gain== -22 ) return PMU_IHFGAIN_22DB_N;
	if ( db_gain== -21 || db_gain== -20 || db_gain== -19 || db_gain== -18 ) return PMU_IHFGAIN_18DB_N;
	if ( db_gain== -17 || db_gain== -16 || db_gain== -15 || db_gain== -14 )	return PMU_IHFGAIN_14DB_N;
	if ( db_gain== -13 || db_gain== -12 )	return PMU_IHFGAIN_12DB_N;
	if ( db_gain== -11 || db_gain== -10 )	return PMU_IHFGAIN_10DB_N;
	if ( db_gain== -9  || db_gain== -8 )		return PMU_IHFGAIN_8DB_N;
	if ( db_gain== -7  || db_gain== -6 )		return PMU_IHFGAIN_6DB_N;
	if ( db_gain== -5  || db_gain== -4 )		return PMU_IHFGAIN_4DB_N;
	if ( db_gain== -3  || db_gain== -2 )		return PMU_IHFGAIN_2DB_N;
	if ( db_gain== -1  || db_gain== 0 )		return PMU_IHFGAIN_0DB;
	if ( db_gain== 1 )		return PMU_IHFGAIN_1DB_P;
	if ( db_gain== 2 )		return PMU_IHFGAIN_2DB_P;
	if ( db_gain== 3 )		return PMU_IHFGAIN_3DB_P;
	if ( db_gain== 4 )		return PMU_IHFGAIN_4DB_P;
	if ( db_gain== 5 )		return PMU_IHFGAIN_5DB_P;
	if ( db_gain== 6 )		return PMU_IHFGAIN_6DB_P;
	if ( db_gain== 7 )		return PMU_IHFGAIN_7DB_P;
	if ( db_gain== 8 )		return PMU_IHFGAIN_8DB_P;
	if ( db_gain== 9 )		return PMU_IHFGAIN_9DB_P;
	if ( db_gain== 10 )		return PMU_IHFGAIN_10DB_P;
	if ( db_gain== 11 )		return PMU_IHFGAIN_11DB_P;
	if ( db_gain== 12 )		return PMU_IHFGAIN_12DB_P;
	// PMU_IHFGAIN_12P5DB_P,
    if ( db_gain== 13 )		return PMU_IHFGAIN_13DB_P;
	// PMU_IHFGAIN_13P5DB_P,
    if ( db_gain== 14 )		return PMU_IHFGAIN_14DB_P;
	// PMU_IHFGAIN_14P5DB_P,
    if ( db_gain== 15 )		return PMU_IHFGAIN_15DB_P;
	// PMU_IHFGAIN_15P5DB_P,
    if ( db_gain== 16 )		return PMU_IHFGAIN_16DB_P;
	// PMU_IHFGAIN_16P5DB_P,
    if ( db_gain== 17 )		return PMU_IHFGAIN_17DB_P;
	// PMU_IHFGAIN_17P5DB_P,
    if ( db_gain== 18 )		return PMU_IHFGAIN_18DB_P;
	// PMU_IHFGAIN_18P5DB_P,
    if ( db_gain== 19 )		return PMU_IHFGAIN_19DB_P;
	// PMU_IHFGAIN_19P5DB_P,
    if ( db_gain== 20 )		return PMU_IHFGAIN_20DB_P;

#else
	if ( db_gain== -18 )	return PMU_IHFGAIN_18DB_N;
	if ( db_gain== -17 )	return PMU_IHFGAIN_16P5DB_N;
	if ( db_gain== -16 )	return PMU_IHFGAIN_16DB_N;
	if ( db_gain== -15 )	return PMU_IHFGAIN_15DB_N;
	if ( db_gain== -14 )	return PMU_IHFGAIN_14DB_N;
	if ( db_gain== -13 )	return PMU_IHFGAIN_13DB_N;
	if ( db_gain== -12 )	return PMU_IHFGAIN_12DB_N;
	if ( db_gain== -11 )	return PMU_IHFGAIN_11DB_N;
	if ( db_gain== -10 )	return PMU_IHFGAIN_10DB_N;
	if ( db_gain== -9 )		return PMU_IHFGAIN_9DB_N;
	if ( db_gain== -8 )		return PMU_IHFGAIN_8DB_N;
	if ( db_gain== -7 )		return PMU_IHFGAIN_7DB_N;
	if ( db_gain== -6 )		return PMU_IHFGAIN_6DB_N;
	if ( db_gain== -5 )		return PMU_IHFGAIN_5DB_N;
	if ( db_gain== -4 )		return PMU_IHFGAIN_4DB_N;
	if ( db_gain== -3 )		return PMU_IHFGAIN_3DB_N;
	if ( db_gain== -2 )		return PMU_IHFGAIN_2DB_N;
	if ( db_gain== -1 )		return PMU_IHFGAIN_1DB_N;
	if ( db_gain== 0 )		return PMU_IHFGAIN_0DB;
	if ( db_gain== 1 )		return PMU_IHFGAIN_1DB_P;
	if ( db_gain== 2 )		return PMU_IHFGAIN_2DB_P;
	if ( db_gain== 3 )		return PMU_IHFGAIN_3DB_P;
	if ( db_gain>= 4 )		return PMU_IHFGAIN_4DB_P;
#endif

	return PMU_IHFGAIN_0DB;
}

#endif


//============================================================================
//
// Function Name: SetGainOnExternalAmp
//
// Description:   Set gain on external amplifier driver
//
//============================================================================
static void SetGainOnExternalAmp(AUDCTRL_SPEAKER_t speaker, void* gain)
{
#if !defined(NO_PMU)
	Log_DebugPrintf(LOGID_AUDIO,
                    "SetGainOnExternalAmp, speaker = %d, gain=%d\n",
                    speaker, *((PMU_IHF_Gain_t*)gain));

	switch(speaker)
	{
		case AUDCTRL_SPK_HEADSET:
		case AUDCTRL_SPK_TTY:
		    pmu_hs.hs_power = TRUE;
    		pmu_hs.hs_path = PMU_AUDIO_HS_BOTH;
	    	pmu_hs.hs_gain = *((PMU_HS_Gain_t*)gain);
		    HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_HS_SET_GAIN, &pmu_hs, NULL);
			break;

		case AUDCTRL_SPK_LOUDSPK:
    		pmu_ihf.ihf_gain = *((PMU_IHF_Gain_t*)gain);
	    	HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_IHF_SET_GAIN, &pmu_ihf, NULL);
			break;

		default:
			break;
	}
#endif
}



//============================================================================
//
// Function Name: powerOnExternalAmp
//
// Description:   call external amplifier driver
//
//============================================================================
AUDCTRL_AUDIO_AMP_ACTION_t powerOnExternalAmp( AUDCTRL_SPEAKER_t speaker, ExtSpkrUsage_en_t usage_flag, Boolean use )
{
//check for current baseband_use_speaker: OR of voice_spkr, audio_spkr, poly_speaker, and second_speaker
//
//ext_use_speaker could be external FM radio, etc.
//baseband and ext not use amp, can power it off.
// PMU driver needs to know AUDIO_CHNL_HEADPHONE type, so call it from here.
//AUDCTRL_SPEAKER_t should be moved to public and let PMU driver includes it.
//and rename it AUD_SPEAKER_t

	AUDCTRL_AUDIO_AMP_ACTION_t retValue = AUDCTRL_AMP_NO_ACTION;

#if !defined(NO_PMU)
	static Boolean telephonyUseHS = FALSE;
	static Boolean audioUseHS = FALSE;

	static Boolean telephonyUseIHF = FALSE;
	static Boolean audioUseIHF = FALSE;

	static Boolean IHF_IsOn = FALSE;
	static Boolean HS_IsOn = FALSE;

	Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp, speaker = %d, IHF_IsOn= %d, HS_IsOn = %d, Boolean_Use=%d\n", speaker, IHF_IsOn, HS_IsOn, use);

	// If the speaker doesn't need PMU, we don't do anything.
	// Otherwise, in concurrent audio paths(one is IHF, the other is EP), the PMU IHF external PGA gain can be overwitten by EP mode gain(0), will mute the PMU.
	if (speaker != AUDCTRL_SPK_HEADSET && speaker != AUDCTRL_SPK_TTY && speaker != AUDCTRL_SPK_LOUDSPK)
	{
		return retValue;
	}
	


	switch(speaker)
	{
		case AUDCTRL_SPK_HEADSET:
		case AUDCTRL_SPK_TTY:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseHS = use;
					if(use)
					{
						telephonyUseIHF = FALSE; //only one output channel for voice call
					}
					break;


				case AudioUseExtSpkr:
					audioUseHS = use;
					break;

				default:
					break;
			}
			break;

		case AUDCTRL_SPK_LOUDSPK:
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseIHF = use;
					if(use)
					{
						telephonyUseHS = FALSE; //only one output channel for voice call
					}
					break;


				case AudioUseExtSpkr:
					audioUseIHF = use;
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	if ((telephonyUseHS==FALSE) && (audioUseHS==FALSE))
	{
		if ( HS_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu HS amp\n");
			pmu_hs.hs_power = FALSE;
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_HS_POWER, &pmu_hs, NULL);
            if (retValue == AUDCTRL_AMP_NO_ACTION) 
            {
                retValue = AUDCTRL_AMP_HS_TURN_OFF;
            }
            else if (retValue == AUDCTRL_AMP_IHF_TURN_OFF)
            {
                retValue = AUDCTRL_AMP_IHF_AND_HS_TURN_OFF;
            }

		}
		HS_IsOn = FALSE;
	}
	else
	{
		int i;
		pmu_hs.hs_power = TRUE;
		pmu_hs.hs_path = PMU_AUDIO_HS_BOTH;
		i = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
		pmu_hs.hs_gain = map2pmu_hs_gain( i );
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (HS on), telephonyUseHS = %d, audioUseHS= %d\n", telephonyUseHS, audioUseHS);

		if ( HS_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, gain %d\n", pmu_hs.hs_gain);
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_HS_POWER, &pmu_hs, NULL);
		}
		HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_HS_SET_GAIN, &pmu_hs, NULL);
		HS_IsOn = TRUE;
	}

	if ((telephonyUseIHF==FALSE) && (audioUseIHF==FALSE))
	{
		if ( IHF_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu IHF amp\n");
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_IHF_POWER_DOWN, &pmu_ihf, NULL);
            if (retValue == AUDCTRL_AMP_NO_ACTION) 
            {
                retValue = AUDCTRL_AMP_IHF_TURN_OFF;
            }
            else if (retValue == AUDCTRL_AMP_HS_TURN_OFF)
            {
                retValue = AUDCTRL_AMP_IHF_AND_HS_TURN_OFF;
            }			
		}
		IHF_IsOn = FALSE;
	}
	else
	{
		int i;
		i = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
		pmu_ihf.ihf_gain = map2pmu_ihf_gain( i );
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (IHF on), telephonyUseIHF = %d, audioUseIHF= %d\n", telephonyUseIHF, audioUseIHF);

		if ( IHF_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu IHF amp, gain %d\n", pmu_ihf.ihf_gain);
			HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_IHF_POWER_UP, &pmu_ihf, NULL);
		}
		HAL_EM_PMU_Ctrl_Private( EM_PMU_ACTION_IHF_SET_GAIN, &pmu_ihf, NULL);
		IHF_IsOn = TRUE;
	}
    Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp: retValue %d\n", retValue);
#endif    
	return retValue;
}



//============================================================================
//
// Function Name: powerOnDigitalMic
//
// Description:   power on/off the Digital Mic
//
//============================================================================
void powerOnDigitalMic(Boolean powerOn)
{
#if !defined(NO_PMU)
	if (powerOn == TRUE)
	{
		// Enable power to digital microphone
		PMU_SetLDOMode(PMU_HVLDO7CTRL,0);
	}
	else //powerOn == FALSE
	{
		// Enable power to digital microphone
		PMU_SetLDOMode(PMU_HVLDO7CTRL,1);
	}
#endif
}
#endif
