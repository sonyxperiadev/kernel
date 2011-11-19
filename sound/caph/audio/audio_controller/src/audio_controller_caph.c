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
* @file   audio_controller_caph.c
* @brief  
*
******************************************************************************/
#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) 

//AP version:

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"

#include "csl_arm2sp.h"

#include "csl_vpu.h"
#include "audio_consts.h"

#include "bcm_fuse_sysparm_CIB.h"

#include "csl_caph.h"
#include "csl_caph_gain.h"
#include "csl_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "log.h"
#include "osheap.h"

#ifdef CONFIG_DIGI_MIC
#if !defined(NO_PMU)
#include "pmu.h"
#include "hal_pmu.h"
#include "hal_pmu_private.h"
#endif
#endif

#include "audio_pmu_adapt.h"

//There are two loopback paths available in AudioH.
//One is 6.5MHz analog microphone loopback path. It does not support digital mics.
//The other one is HW sidetone path. It supports all the mics. This is prefered.
//Only one loopback path can be enabled at a time.
#define HW_SIDETONE_LOOPBACK
#undef HW_ANALOG_LOOPBACK

//=============================================================================
// Public Variable declarations
//=============================================================================

extern AUDDRV_SPKR_Enum_t voiceCallSpkr;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================

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
    CSL_CAPH_DEVICE_e dev;
} AUDCTRL_HWID_Mapping_t;

static AUDCTRL_HWID_Mapping_t HWID_Mapping_Table[AUDIO_HW_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDIO_HW_NONE,			CSL_CAPH_DEV_NONE},
	{AUDIO_HW_MEM,			CSL_CAPH_DEV_MEMORY},
	{AUDIO_HW_VOICE_OUT,	CSL_CAPH_DEV_NONE},
	{AUDIO_HW_MONO_BT_OUT,	CSL_CAPH_DEV_BT_SPKR},
	{AUDIO_HW_STEREO_BT_OUT,CSL_CAPH_DEV_BT_SPKR},
	{AUDIO_HW_USB_OUT,		CSL_CAPH_DEV_MEMORY},
	{AUDIO_HW_I2S_OUT,		CSL_CAPH_DEV_FM_TX},
	{AUDIO_HW_VOICE_IN,		CSL_CAPH_DEV_NONE},
	{AUDIO_HW_MONO_BT_IN,	CSL_CAPH_DEV_BT_MIC},
	{AUDIO_HW_USB_IN,		CSL_CAPH_DEV_MEMORY},
	{AUDIO_HW_I2S_IN,		CSL_CAPH_DEV_FM_RADIO},
	{AUDIO_HW_DSP_VOICE,	CSL_CAPH_DEV_DSP},
	{AUDIO_HW_EARPIECE_OUT,	CSL_CAPH_DEV_EP},
	{AUDIO_HW_HEADSET_OUT,	CSL_CAPH_DEV_HS},
	{AUDIO_HW_IHF_OUT,		CSL_CAPH_DEV_IHF},
	{AUDIO_HW_SPEECH_IN,	CSL_CAPH_DEV_ANALOG_MIC},
	{AUDIO_HW_NOISE_IN,		CSL_CAPH_DEV_EANC_DIGI_MIC},
	{AUDIO_HW_VIBRA_OUT,	CSL_CAPH_DEV_VIBRA}
};




typedef struct
{
    AUDCTRL_SPEAKER_t spkr;
    CSL_CAPH_DEVICE_e dev;
} AUDCTRL_SPKR_Mapping_t;

static AUDCTRL_SPKR_Mapping_t SPKR_Mapping_Table[AUDCTRL_SPK_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDCTRL_SPK_HANDSET,		CSL_CAPH_DEV_EP},
	{AUDCTRL_SPK_HEADSET,		CSL_CAPH_DEV_HS},
	{AUDCTRL_SPK_HANDSFREE,		CSL_CAPH_DEV_IHF},
	{AUDCTRL_SPK_BTM,		    CSL_CAPH_DEV_BT_SPKR},
	{AUDCTRL_SPK_LOUDSPK,		CSL_CAPH_DEV_IHF},
	{AUDCTRL_SPK_TTY,		    CSL_CAPH_DEV_HS},
	{AUDCTRL_SPK_HAC,		    CSL_CAPH_DEV_EP},
	{AUDCTRL_SPK_USB,		    CSL_CAPH_DEV_MEMORY},
	{AUDCTRL_SPK_BTS,		    CSL_CAPH_DEV_BT_SPKR},
	{AUDCTRL_SPK_I2S,		    CSL_CAPH_DEV_FM_TX},
	{AUDCTRL_SPK_VIBRA,		    CSL_CAPH_DEV_VIBRA},
	{AUDCTRL_SPK_UNDEFINED,		CSL_CAPH_DEV_NONE}
};

typedef struct
{
    AUDCTRL_SPEAKER_t spkr;
    AUDDRV_SPKR_Enum_t auddrv_spkr;
} AUDCTRL_DRVSPKR_Mapping_t;


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

typedef struct
{
    AUDCTRL_MICROPHONE_t mic;
    AUDDRV_MIC_Enum_t auddrv_mic;
} AUDCTRL_DRVMIC_Mapping_t;

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

typedef struct
{
    AUDCTRL_MICROPHONE_t mic;
    CSL_CAPH_DEVICE_e dev;
} AUDCTRL_MIC_Mapping_t;

static AUDCTRL_MIC_Mapping_t MIC_Mapping_Table[AUDCTRL_MIC_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDCTRL_MIC_UNDEFINED,		    CSL_CAPH_DEV_NONE},
	{AUDCTRL_MIC_MAIN,		        CSL_CAPH_DEV_ANALOG_MIC},
	{AUDCTRL_MIC_AUX,		        CSL_CAPH_DEV_HS_MIC},
	{AUDCTRL_MIC_DIGI1,     		CSL_CAPH_DEV_DIGI_MIC_L},
	{AUDCTRL_MIC_DIGI2,		        CSL_CAPH_DEV_DIGI_MIC_R},
	{AUDCTRL_DUAL_MIC_DIGI12,		CSL_CAPH_DEV_DIGI_MIC},
	{AUDCTRL_DUAL_MIC_DIGI21,		CSL_CAPH_DEV_DIGI_MIC},
	{AUDCTRL_DUAL_MIC_ANALOG_DIGI1,	CSL_CAPH_DEV_NONE},
	{AUDCTRL_DUAL_MIC_DIGI1_ANALOG,	CSL_CAPH_DEV_NONE},
	{AUDCTRL_MIC_BTM,		        CSL_CAPH_DEV_BT_MIC},
	{AUDCTRL_MIC_USB,       		CSL_CAPH_DEV_MEMORY},
	{AUDCTRL_MIC_I2S,		        CSL_CAPH_DEV_FM_RADIO},
	{AUDCTRL_MIC_DIGI3,	        	CSL_CAPH_DEV_NONE},
	{AUDCTRL_MIC_DIGI4,		        CSL_CAPH_DEV_NONE},
	{AUDCTRL_MIC_SPEECH_DIGI,       CSL_CAPH_DEV_DIGI_MIC},
	{AUDCTRL_MIC_EANC_DIGI,		    CSL_CAPH_DEV_EANC_DIGI_MIC}
};

 AUDDRV_PathID_t telephonyPathID;
//static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;


static int telephony_digital_gain_dB = 12;  //dB
static int telephony_ul_gain_mB = 0;  // 0 mB


//=============================================================================
// Private function prototypes
//=============================================================================
static void SetGainOnExternalAmp(AUDCTRL_SPEAKER_t speaker, int gain, int left_right);

//on AP:
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetAudioParmAccessPtr();
#endif
}

#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()[mode]
#define AUDIOMODE_PARM_MM_ACCESSOR(mode)	 AUDIO_GetParmMMAccessPtr()[mode]


static void AUDCTRL_CreateTable(void);
static void AUDCTRL_DeleteTable(void);
static CSL_CAPH_DEVICE_e GetDeviceFromHWID(AUDIO_HW_ID_t hwID);
static CSL_CAPH_DEVICE_e GetDeviceFromMic(AUDCTRL_MICROPHONE_t mic);
static CSL_CAPH_DEVICE_e GetDeviceFromSpkr(AUDCTRL_SPEAKER_t spkr);
static CSL_CAPH_PathID AUDCTRL_GetPathIDFromTable(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
                                                AUDCTRL_MICROPHONE_t mic);
static CSL_CAPH_PathID AUDCTRL_GetPathIDFromTableWithSrcSink(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
                                                AUDCTRL_MICROPHONE_t mic);
static void AUDCTRL_UpdatePath (CSL_CAPH_PathID pathID,
                                                AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
                                                AUDCTRL_MICROPHONE_t mic);

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

    AUDCTRL_CreateTable();
	
	csl_caph_hwctrl_init();

	//telephony_digital_gain_dB = 12;  //SYSPARM_GetAudioParamsFromFlash( cur_mode )->voice_volume_init;  //dB
}

//============================================================================
//
// Function Name: AUDCTRL_Shutdown
//
// Description:   De-Initialize audio controller
//
//============================================================================
void AUDCTRL_Shutdown(void)
{
    AUDDRV_Shutdown();
    AUDCTRL_DeleteTable();
}

//============================================================================
//
// Function Name: AUDCTRL_EnableTelephony
//
// Description:   Enable telephonly path, both ul and dl
//
//============================================================================
void AUDCTRL_EnableTelephony(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	AUDDRV_MIC_Enum_t	micSel;
	AUDDRV_SPKR_Enum_t	spkSel;
	AUDCTRL_Config_t data;


	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;
	// mic selection. 
	micSel = AUDCTRL_GetDrvMic (mic);

	// speaker selection. We hardcode headset,handset and loud speaker right now. 
	// Later, need to provide a configurable table.
	spkSel = AUDCTRL_GetDrvSpk (speaker);

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTelephony::  spkSel %d, micSel %d \n", spkSel, micSel );

	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))
		
	{
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
	}	

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_Init ( micSel, 
			spkSel,
		       	(void *)&telephonyPathID);
	voiceCallSpkr = spkSel;

	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonySpkrMute (dlSink, speaker, FALSE); 
	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonyMicMute (ulSrc, mic, FALSE); 

	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );

    //Load the mic gains from sysparm.
    AUDCTRL_LoadMicGain(telephonyPathID.ulPathID, mic, TRUE);
    //Load the speaker gains form sysparm.
    AUDCTRL_LoadSpkrGain(telephonyPathID.dlPathID, speaker, TRUE);

	//Save UL path to the path table.
	data.pathID = telephonyPathID.ulPathID;
	data.src = ulSrc_not_used;
	data.sink = AUDIO_HW_NONE;
   	data.mic = mic;
	data.spk = AUDCTRL_SPK_UNDEFINED;
	data.numCh = AUDIO_CHANNEL_NUM_NONE;
	data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	AUDCTRL_AddToTable(&data);

    if (AUDDRV_IsDualMicEnabled()==TRUE)
    {
        data.pathID = telephonyPathID.ul2PathID;
	    data.src = ulSrc_not_used;
	    data.sink = AUDIO_HW_NONE;
    	data.mic = AUDCTRL_MIC_NOISE_CANCEL;
	    data.spk = AUDCTRL_SPK_UNDEFINED;
	    data.numCh = AUDIO_CHANNEL_NUM_NONE;
	    data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	    AUDCTRL_AddToTable(&data);
    }

	//Save DL path to the path table.
	data.pathID = telephonyPathID.dlPathID;
	data.src = AUDIO_HW_NONE;
	data.sink = dlSink_not_used;
   	data.mic = AUDCTRL_MIC_UNDEFINED;
	data.spk = speaker;
	data.numCh = AUDIO_CHANNEL_NUM_NONE;
	data.sr = AUDIO_SAMPLING_RATE_UNDEFINED;
	AUDCTRL_AddToTable(&data);

#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(!modeVoiceCall) 
			modeVoiceCall=1;
	} 
#endif
	return;
}
//============================================================================
//
// Function Name: AUDCTRL_DisableTelephony
//
// Description:   disable telephony path, both dl and ul
//
//============================================================================
void AUDCTRL_DisableTelephony(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				AUDCTRL_SPEAKER_t		speaker
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableTelephony \n" );

	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, FALSE );

	// The following is the sequence we need to follow
	AUDDRV_Telephony_Deinit ((void*)&telephonyPathID);
	voiceCallSpkr = AUDDRV_SPKR_NONE;
	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}	

    //Delect from Table
  	AUDCTRL_RemoveFromTable(telephonyPathID.ulPathID);
   	AUDCTRL_RemoveFromTable(telephonyPathID.ul2PathID);
   	AUDCTRL_RemoveFromTable(telephonyPathID.dlPathID);
	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;
	
#if defined(WIN32)
	{
		extern int modeVoiceCall;
		if(modeVoiceCall) 
			modeVoiceCall=0;
	} 
#endif
	return;
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
				Int32					volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
	int pmuGain = 0;
	pmuGain = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = %d, PMU audio gain = 0x%x\n", volume, pmuGain );

	if (gain_format == AUDIO_GAIN_FORMAT_mB)		
	{
		telephony_digital_gain_dB = (volume / 100) + 36;
		if ( telephony_digital_gain_dB > AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max )	//dB
			telephony_digital_gain_dB = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max; //dB

		if( volume <=-10000 )  //less than -100dB
		{  //mute
			audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkMute, 0, 0, 0, 0, 0);
		}
		else
		{
/*********
			OmegaVoice_Sysparm_t *omega_voice_parms = NULL;

			omega_voice_parms = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].omega_voice_parms;	//dB
			audio_control_generic(AUDDRV_CPCMD_SetOmegaVoiceParam,
								(UInt32)(&(omega_voice_parms[telephony_digital_gain_dB])),	//?
								0, 0, 0, 0);
********/

			//if parm4 (OV_volume_step) is zero, volumectrl.c will calculate OV volume step based on digital_gain_dB, VOICE_VOLUME_MAX and NUM_SUPPORTED_VOLUME_LEVELS.
			audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain,
								volume,  //DSP accepts [-3600, 0] mB
								0, 0, 0, 0);
			
			pmuGain = (Int16)AUDDRV_GetPMUGain(GetDeviceFromSpkr(speaker),
					((Int16)volume)/25);   //mB to quarter dB
			if (pmuGain != (Int16)GAIN_NA)
			{
				if (pmuGain == (Int16)(GAIN_SYSPARM))
				{
					//Read from sysparm.
					pmuGain = (Int16)volume; //AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;  //dB
				}
				SetGainOnExternalAmp(speaker, pmuGain, PMU_AUDIO_HS_BOTH);
			}
		}
	}
}

//============================================================================
//
// Function Name: AUDCTRL_GetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
UInt32 AUDCTRL_GetTelephonySpkrVolume( AUDIO_GAIN_FORMAT_t gain_format )
{
	return telephony_digital_gain_dB;
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
				Int16					gain,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicGain: gain = 0x%x\n", gain);
	
	if (gain_format == AUDIO_GAIN_FORMAT_mB)
	{
        telephony_ul_gain_mB = gain;
    }

	audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain, 
				telephony_ul_gain_mB, 0, 0, 0, 0);

	//sysparm.c(4990):	pg1_mem->shared_echo_fast_NLP_gain[1] = SYSPARM_GetAudioParmAccessPtr()->audio_parm[currentAudioMode].echoNlp_parms.echo_nlp_gain;
	//should also load this parameter in SetAudioMode() in CP build.
}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDCTRL_SPEAKER_t		spk,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrMute: mute = 0x%x\n",  mute);

	if(mute)
        AUDDRV_Telephony_MuteSpkr((AUDDRV_SPKR_Enum_t) spk, (void*)NULL);
	else
        AUDDRV_Telephony_UnmuteSpkr((AUDDRV_SPKR_Enum_t) spk, (void*)NULL);
}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicMute
//
// Description:   mute/unmute ul of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicMute: mute = 0x%x\n",  mute);

	if(mute)
        AUDDRV_Telephony_MuteMic ((AUDDRV_MIC_Enum_t)mic, (void*)NULL );
	else
        AUDDRV_Telephony_UnmuteMic ((AUDDRV_MIC_Enum_t)mic, (void*)NULL );
}


//============================================================================
//
// Function Name: AUDCTRL_LoadMicGain
//
// Description:   Load ul gain from sysparm.
//
//============================================================================
void AUDCTRL_LoadMicGain(CSL_CAPH_PathID ulPathID, AUDCTRL_MICROPHONE_t mic, Boolean isDSPNeeded)
{
    UInt16 gainTemp = 0;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_LoadMicGain\n");

	// Set DSP UL gain from sysparm.
	mode = AUDDRV_GetAudioMode();
/***
	Int16 dspULGain = 0;
do not touch DSP UL gain in this function.
this function shall be merged into AUDCTRL_SetAudioMode( ).

    if(isDSPNeeded == TRUE)
    {
	    dspULGain = 64; //AUDIO_GetParmAccessPtr()[mode].echoNlp_parms.echo_nlp_gain;
	    audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain, 
				dspULGain, 0, 0, 0, 0);		
    }
***/

    if((mic == AUDCTRL_MIC_MAIN) 
       ||(mic == AUDCTRL_MIC_AUX)) 
    {
        // Set Mic PGA gain from sysparm.	
      	gainTemp = 74; //AUDIO_GetParmAccessPtr()[mode].mic_pga;
      	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_AMIC_PGA_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);

        // Set AMic DGA coarse gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].amic_dga_coarse_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_AMIC_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
	
        // Set AMic DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].amic_dga_fine_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_AMIC_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
    }

    if((mic == AUDCTRL_MIC_DIGI1) 
       ||(mic == AUDCTRL_MIC_SPEECH_DIGI)) 
    {
        // Set DMic1 DGA coarse gain from sysparm.	
      	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic1_dga_coarse_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC1_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
	
        // Set DMic1 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic1_dga_fine_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC1_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
    }

    if((mic == AUDCTRL_MIC_DIGI2) 
       ||(mic == AUDCTRL_MIC_SPEECH_DIGI)) 
    {
        // Set DMic2 DGA coarse gain from sysparm.	
      	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic2_dga_coarse_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC2_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
	
        // Set DMic2 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic2_dga_fine_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC2_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
    }

    if((AUDDRV_IsDualMicEnabled()==TRUE)
       ||(mic == AUDCTRL_MIC_EANC_DIGI)) 
    {
        // Set DMic3 DGA coarse gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic3_dga_coarse_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC3_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
	
        // Set DMic3 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic3_dga_fine_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC3_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
	
        // Set DMic4 DGA coarse gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic4_dga_coarse_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC4_DGA_COARSE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
	
        // Set DMic4 DGA fine gain from sysparm.	
       	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].dmic4_dga_fine_gain;
       	csl_caph_hwctrl_SetHWGain(ulPathID,
                                CSL_CAPH_DMIC4_DGA_FINE_GAIN,
                                (UInt32) gainTemp,
                                CSL_CAPH_DEV_NONE);
    }
    return;
}



//============================================================================
//
// Function Name: AUDCTRL_LoadSpkrGain
//
// Description:   Load dl gain from sysparm.
//
//============================================================================
void AUDCTRL_LoadSpkrGain(CSL_CAPH_PathID dlPathID, AUDCTRL_SPEAKER_t speaker, Boolean isDSPNeeded)
{
	Int16 dspDLGain = 0;
	Int16 pmuGain = 0;
    UInt16 gainTemp = 0;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_LoadSpkrGain\n");

	mode = AUDDRV_GetAudioMode();

	// Set DSP DL gain from sysparm.
    if(isDSPNeeded == TRUE)
    {
	    dspDLGain = 64; //AUDIO_GetParmAccessPtr()[mode].echo_nlp_downlink_volume_ctrl;
	    audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain, 
						dspDLGain, 0, 0, 0, 0);
    }
			

    //Load HW Mixer gains from sysparm
	if ((mode == AUDIO_MODE_HANDSET)
		||(mode == AUDIO_MODE_HANDSET_WB)
		||(mode == AUDIO_MODE_HAC)
		||(mode == AUDIO_MODE_HAC_WB))		
	{
		dev = CSL_CAPH_DEV_EP;
	}
	else
	if ((mode == AUDIO_MODE_HEADSET)
		||(mode == AUDIO_MODE_HEADSET_WB)
		||(mode == AUDIO_MODE_TTY)
		||(mode == AUDIO_MODE_TTY_WB))
		
	{
		dev = CSL_CAPH_DEV_HS;
	}
	else
	if ((mode == AUDIO_MODE_SPEAKERPHONE)
		||(mode == AUDIO_MODE_SPEAKERPHONE_WB))
	{
		dev = CSL_CAPH_DEV_IHF;
	}

	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_l;
	csl_caph_hwctrl_SetHWGain(dlPathID, 
                               CSL_CAPH_SRCM_INPUT_GAIN_L, 
                               (UInt32)gainTemp, dev);
	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_r;
	csl_caph_hwctrl_SetHWGain(dlPathID, 
                               CSL_CAPH_SRCM_INPUT_GAIN_R, 
                               (UInt32)gainTemp, dev);
	gainTemp = 96; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_l;
	csl_caph_hwctrl_SetHWGain(dlPathID, 
                               CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L, 
                               (UInt32)gainTemp, dev);
	gainTemp = 96; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_r;
	csl_caph_hwctrl_SetHWGain(dlPathID, 
                               CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R, 
                               (UInt32)gainTemp, dev);
	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_l;
	csl_caph_hwctrl_SetHWGain(dlPathID, 
                               CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L, 
                               (UInt32)gainTemp, dev);
	gainTemp = 0; //AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_r;
	csl_caph_hwctrl_SetHWGain(dlPathID, 
                               CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R, 
                               (UInt32)gainTemp, dev);
	
	//Load PMU gain from sysparm.
	pmuGain = AUDIO_GetParmAccessPtr()[mode].ext_speaker_pga_l;
	SetGainOnExternalAmp(speaker, pmuGain, PMU_AUDIO_HS_LEFT);
	
	pmuGain = AUDIO_GetParmAccessPtr()[mode].ext_speaker_pga_r;
	SetGainOnExternalAmp(speaker, pmuGain, PMU_AUDIO_HS_RIGHT);
	
    return;
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
				AUDIO_SAMPLING_RATE_t	sr,
				UInt32					*pPathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID;
    AUDCTRL_Config_t data;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnablePlay: src = 0x%x, sink = 0x%x, tap = 0x%x, spkr %d \n", 
                    src, sink, tap, spk);
    pathID = 0;
    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    // Enable the path. And get path ID.
    config.streamID = CSL_CAPH_STREAM_NONE; 
    config.pathID = 0;
    config.source = GetDeviceFromHWID(src);
    config.sink =  GetDeviceFromSpkr(spk);
    config.dmaCH = CSL_CAPH_DMA_NONE; 
    config.src_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
    config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

    //Enable the PMU for HS/IHF.
    if ((sink == AUDIO_HW_HEADSET_OUT)||(sink == AUDIO_HW_IHF_OUT)) 
    {
		powerOnExternalAmp( spk, AudioUseExtSpkr, TRUE );
    }
	
    if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDCTRL_SPK_USB)
	{	//USB call
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

#if defined(ENABLE_DMA_ARM2SP)
	if ((src == AUDIO_HW_MEM || src == AUDIO_HW_I2S_IN) && sink == AUDIO_HW_DSP_VOICE && spk!=AUDCTRL_SPK_USB)
	{
		config.sink = CSL_CAPH_DEV_DSP_throughMEM; //convert from AUDDRV_DEV_EP
	}
#endif

	if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		;
	else
		pathID = csl_caph_hwctrl_EnablePath(config);

    //Save this path to the path table.
    data.pathID = pathID;
    data.src = src;
    data.sink = sink;
    data.mic = AUDCTRL_MIC_UNDEFINED;
    data.spk = spk;
    data.numCh = numCh;
    data.sr = sr;
    AUDCTRL_AddToTable(&data);

    //Load the speaker gains form sysparm.
    //Can not call this following API here.
    //Because Render driver really enable the path.
    //AUDCTRL_LoadSpkrGain(pathID, spk, FALSE);

#if 0
	// in case it was muted from last play,
	AUDCTRL_SetPlayMute (sink, spk, FALSE); 
#endif    
	// Enable DSP DL for Voice Call.
	if(config.source == CSL_CAPH_DEV_DSP)
	{
		AUDDRV_EnableDSPOutput(DRVSPKR_Mapping_Table[spk].auddrv_spkr, sr);
	}
	if(pPathID) *pPathID = pathID;
	//Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_EnablePlay: pPathID %x, pathID %d.\r\n", *pPathID, pathID);
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
				AUDCTRL_SPEAKER_t		spk,
				UInt32					inPathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID = 0;

    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	if(inPathID==0) pathID = AUDCTRL_GetPathIDFromTableWithSrcSink(src, sink, spk, AUDCTRL_MIC_UNDEFINED);
	else pathID = inPathID; //do not search for it if pathID is provided, this is to support multi streams to the same destination.
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisablePlay: src = 0x%x, sink = 0x%x, spk = 0x%x, pathID %d:%ld.\r\n", 
                    src, sink,  spk, pathID, inPathID);
	
    if(pathID == 0)
    {
	audio_xassert(0,pathID);
	return;
    }
	
    if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDCTRL_SPK_USB)
	{	//USB call
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

	if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		;
	else
	{
		config.pathID = pathID;
		(void) csl_caph_hwctrl_DisablePath(config);
	}
	//Remove this path from the path table.
	AUDCTRL_RemoveFromTable(pathID);

    //Disable the PMU for HS/IHF.
	pathID = AUDCTRL_GetPathIDFromTableWithSrcSink(src, sink, spk, AUDCTRL_MIC_UNDEFINED);
	if(pathID)
	{
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisablePlay: pathID %d using the same path still remains, do not turn off PMU.\r\n", pathID);
	} else {
		if ((sink == AUDIO_HW_HEADSET_OUT)||(sink == AUDIO_HW_IHF_OUT)) 
		{
			powerOnExternalAmp( spk, AudioUseExtSpkr, FALSE );
		}
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
				AUDIO_GAIN_FORMAT_t     gain_format,
				UInt32					vol_left,
				UInt32					vol_right
				)
{
	Int16 gain_q13p2=0, gain_q13p2_p1=0;
    UInt32 gainHW, gainHW2, gainHW3, gainHW4, gainHW5, gainHW6;  //quarter dB
    int pmuGain = 0x00;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;
    CSL_CAPH_PathID pathID = 0;

    gainHW = gainHW2 = gainHW3 = gainHW4 = gainHW5 = gainHW6 = 0;
    Log_DebugPrintf(LOGID_AUDIO,
		"AUDCTRL_SetPlayVolume: sink = 0x%x, spk = 0x%x, gain_format %d, vol_left = 0x%x(%d) vol_right 0x%x(%d)\n",
		sink, spk, gain_format, vol_left, vol_left, vol_right, vol_right);
    
    speaker = GetDeviceFromSpkr(spk);

    if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		return;
    
    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE,
		    sink, spk, AUDCTRL_MIC_UNDEFINED);
    //if(pathID == 0)
    {
		//audio_xassert(0,pathID);
		//return;

		if (gain_format == AUDIO_GAIN_FORMAT_mB)
		{
			gain_q13p2_p1 = (Int16) vol_left;
			gain_q13p2 = gain_q13p2_p1/25;
			Log_DebugPrintf(LOGID_AUDIO,
				"AUDCTRL_SetPlayVolume: sink = 0x%x, spk = 0x%x, gain_q13p2_p1 = 0x%x(%d) gain_q13p2 0x%x(%d)\n",
				sink, spk, gain_q13p2_p1, gain_q13p2_p1, gain_q13p2, gain_q13p2);
			gainHW = AUDDRV_GetHWDLGain(speaker, gain_q13p2);
			
			gain_q13p2 = (Int16)(vol_right/25);
			Log_DebugPrintf(LOGID_AUDIO,
				"AUDCTRL_SetPlayVolume: sink = 0x%x, spk = 0x%x, vol_right = 0x%x(%d) gain_q13p2 0x%x(%d)\n",
				sink, spk, vol_right, vol_right, gain_q13p2, gain_q13p2);
			gainHW2 = AUDDRV_GetHWDLGain(speaker, gain_q13p2);	
			
			pmuGain = (Int16)AUDDRV_GetPMUGain(speaker, (Int16)(vol_left/25) );
		}
		
		if ((gainHW != (UInt32)GAIN_NA)&&(gainHW2 != (UInt32)GAIN_NA))
		{
				gainHW3 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_input_gain_l);
				gainHW4 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_input_gain_r);
				(void)csl_caph_hwctrl_SetHWGain(pathID, CSL_CAPH_SRCM_INPUT_GAIN_L,
												 gainHW3, speaker);
				(void)csl_caph_hwctrl_SetHWGain(pathID, CSL_CAPH_SRCM_INPUT_GAIN_R,
									 gainHW4, speaker);
		
				gainHW6 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_coarse_gain_l);
				gainHW5 = (UInt32)(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].srcmixer_output_coarse_gain_r);
				csl_caph_hwctrl_SetHWGain(pathID, 
								   CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L,
								   gainHW6, speaker);
		
				csl_caph_hwctrl_SetHWGain(pathID,
								   CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R, 
								   gainHW5,  speaker);

				Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: pathID = 0x%x, speaker = 0x%x, gainHW %d(0x%x), gainHW2 = %d(0x%x)\n",
					pathID, speaker, (UInt16)gainHW, (UInt16)gainHW, (UInt16)gainHW2, (UInt16)gainHW2 );
				csl_caph_hwctrl_SetSinkGain(pathID, speaker, (UInt16)gainHW, (UInt16)gainHW2);
		}
	
    }
#if defined(WIN32)
	return;
#endif
   
	//Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetPlayVolume: pmuGain = 0x%x, gainHW %x:%x:%x:%x:%x:%x\n", pmuGain, gainHW, gainHW2, gainHW3, gainHW4, gainHW5, gainHW6);

	// Set the gain to the external amplifier
    if (pmuGain == (Int16)GAIN_SYSPARM)
    {
		pmuGain = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;
        SetGainOnExternalAmp(spk, pmuGain, PMU_AUDIO_HS_LEFT);

		pmuGain = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_r;
		SetGainOnExternalAmp(spk, pmuGain, PMU_AUDIO_HS_RIGHT);
    }
    else
    if (pmuGain != (Int16)GAIN_NA)
    {
        SetGainOnExternalAmp(spk, pmuGain, PMU_AUDIO_HS_BOTH);
    }
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
    //CSL_CAPH_PathID pathID = 0;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetPlayMute: sink = 0x%x,  spk = 0x%x, mute = 0x%x\n", 
                    sink, spk, mute);

	if( sink == AUDIO_HW_USB_OUT || spk == AUDCTRL_SPK_BTS)
		return;

    speaker = GetDeviceFromSpkr(spk);

    //pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE, sink, spk, AUDCTRL_MIC_UNDEFINED);
    //if(pathID == 0)
    //{
	//audio_xassert(0,pathID);
	//return;
    //}
    

    if (mute == TRUE)
    {
        (void) csl_caph_hwctrl_MuteSink(0 /*pathID*/, speaker);
    }
    else
    {
        (void) csl_caph_hwctrl_UnmuteSink(0 /*pathID*/, speaker);
    }
    return;
}

//============================================================================
//
// Function Name: AUDCTRL_SwitchPlaySpk
//
// Description:   switch a speaker to a playback path
//
//============================================================================
void AUDCTRL_SwitchPlaySpk(
                AUDIO_HW_ID_t           src,
				AUDIO_HW_ID_t			curSink,
				AUDCTRL_SPEAKER_t		curSpk,
				AUDIO_HW_ID_t			newSink,
				AUDCTRL_SPEAKER_t		newSpk
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID = 0;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SwitchPlaySpk src = 0x%x, curSink = 0x%x,  curSpk = 0x%x, newSink = 0x%x,  newSpk = 0x%x\n", 
                    src, curSink, curSpk, newSink, newSpk);


    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE, curSink, curSpk, AUDCTRL_MIC_UNDEFINED);
    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }
 
    // add new spk first... 
    if ((curSpk == AUDCTRL_SPK_LOUDSPK)||(curSpk == AUDCTRL_SPK_HEADSET))	
        powerOnExternalAmp( curSpk, AudioUseExtSpkr, FALSE );	    
    speaker = GetDeviceFromSpkr(newSpk);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
        config.source = GetDeviceFromHWID(src);
        config.sink = speaker;
        (void) csl_caph_hwctrl_AddPath(pathID, config);
    }
   
    // remove current spk
    speaker = GetDeviceFromSpkr(curSpk);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
        config.source = GetDeviceFromHWID(src);
        config.sink = speaker;
        (void) csl_caph_hwctrl_RemovePath(pathID, config);
    } 
    if ((newSpk == AUDCTRL_SPK_LOUDSPK)||(newSpk == AUDCTRL_SPK_HEADSET))	
        powerOnExternalAmp( newSpk, AudioUseExtSpkr, TRUE );	    
    

    // update path structure
    AUDCTRL_UpdatePath(pathID, src, newSink, newSpk, AUDCTRL_MIC_UNDEFINED); 

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
                AUDIO_HW_ID_t           src,
				AUDIO_HW_ID_t			curSink,
				AUDCTRL_SPEAKER_t		curSpk,
				AUDIO_HW_ID_t			newSink,
				AUDCTRL_SPEAKER_t		newSpk
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID = 0;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_AddPlaySpk: src = 0x%x, newSink = 0x%x,  newSpk = 0x%x\n", 
                    src, newSink, newSpk);


    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE, curSink, curSpk, AUDCTRL_MIC_UNDEFINED);
    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }
   

    speaker = GetDeviceFromSpkr(newSpk);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
		//Enable the PMU for HS/IHF.
		if ((newSink == AUDIO_HW_HEADSET_OUT)||(newSink == AUDIO_HW_IHF_OUT))
			powerOnExternalAmp( newSpk, AudioUseExtSpkr, TRUE );

        config.source = GetDeviceFromHWID(src);
        config.sink = speaker;
        (void) csl_caph_hwctrl_AddPath(pathID, config);
    }
    
//    AUDCTRL_UpdatePath(pathID, src, newSink, newSpk, AUDCTRL_MIC_UNDEFINED); 
    
    return;
    
}

//============================================================================
//
// Function Name: AUDCTRL_RemovePlaySpk
//
// Description:   remove a speaker to a playback path
//
//============================================================================
void AUDCTRL_RemovePlaySpk(
                AUDIO_HW_ID_t           src,
				AUDIO_HW_ID_t			priSink,
				AUDCTRL_SPEAKER_t		priSpk,
				AUDIO_HW_ID_t			secSink,
				AUDCTRL_SPEAKER_t		secSpk
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID = 0;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_RemovePlaySpk: src = 0x%x, sink = 0x%x,  spk = 0x%x\n", 
                    src, secSink, secSpk);


    pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_NONE, priSink, priSpk, AUDCTRL_MIC_UNDEFINED);
    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }
    

    speaker = GetDeviceFromSpkr(secSpk);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
		//Disable the PMU for HS/IHF.
		if ((secSink == AUDIO_HW_HEADSET_OUT)||(secSink == AUDIO_HW_IHF_OUT))
			powerOnExternalAmp( secSpk, AudioUseExtSpkr, FALSE );

        config.source = GetDeviceFromHWID(src);
        config.sink = speaker;
        (void) csl_caph_hwctrl_RemovePath(pathID, config);
    }
    
    // don't know how to update the path now.
    //AUDCTRL_UpdatePath(pathID, src, sink, spk, AUDCTRL_MIC_UNDEFINED); 
    
    return;
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
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID;
    AUDCTRL_Config_t data;

    pathID = 0;
    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    // Enable the path. And get path ID.
    config.streamID = CSL_CAPH_STREAM_NONE; 
    config.pathID = 0;
    config.source = GetDeviceFromMic(mic);
    config.sink =  GetDeviceFromHWID(sink);
    config.dmaCH = CSL_CAPH_DMA_NONE; 
    config.snk_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
	config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

	if (src == AUDIO_HW_USB_IN && sink == AUDIO_HW_DSP_VOICE)
	{
		// in this case, the entire data pass is 
		// USB Mic(48K mono) --> DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP input --> DSP sharedmem --> DDR
		// for HW control, need to setup the caph path DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP.
		// the caph path source is MEMORY, the capth path sink is DSP. Also need to set the input sampling rate as 48K, and output sampling rate as 8K or 16 (depending on 
		// the passed in parameter sr), so we know we need to use the HW srcMixer.
		config.source = CSL_CAPH_DEV_MEMORY;
		config.sink = CSL_CAPH_DEV_DSP;
	}
	if(config.sink==CSL_CAPH_DEV_DSP)
		config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;
	pathID = csl_caph_hwctrl_EnablePath(config);
	
    //Load the mic gains from sysparm.
    //Can not call the following API here.
    //Because Capture driver really enables the path.
    //AUDCTRL_LoadMicGain(pathID, mic, FALSE);
 
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
	if(config.sink == CSL_CAPH_DEV_DSP)
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
                    "AUDCTRL_EnableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x, sr %ld\n",
                    src, sink, mic, sr);

	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	{
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
	}

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 
			|| mic==AUDCTRL_DUAL_MIC_DIGI21 
			|| mic==AUDCTRL_MIC_SPEECH_DIGI)
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

    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID = 0;
	
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x\n", 
                    src, sink, mic);

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 
			|| mic==AUDCTRL_DUAL_MIC_DIGI21 
			|| mic==AUDCTRL_MIC_SPEECH_DIGI)
		
	{
		memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, AUDCTRL_MIC_DIGI1);
		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) csl_caph_hwctrl_DisablePath(config);
		AUDCTRL_RemoveFromTable(pathID);

		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, AUDCTRL_MIC_DIGI2);
		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) csl_caph_hwctrl_DisablePath(config);
		AUDCTRL_RemoveFromTable(pathID);
	} else {
		memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
		pathID = AUDCTRL_GetPathIDFromTable(src, sink, AUDCTRL_SPK_UNDEFINED, mic);
		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}
		

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		
		if (src == AUDIO_HW_USB_IN && sink == AUDIO_HW_DSP_VOICE)
		{
			// in this case, the entire data pass is 
			// USB Mic(48K mono) --> DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP input --> DSP sharedmem --> DDR
			// for HW control, need to setup the caph path DDR --> (via AADMAC, Caph switch)HW srcMixer input CH2 --> HW srcMixer tapout CH2 --> DSP.
			// the caph path source is MEMORY, the capth path sink is DSP. Also need to set the input sampling rate as 48K, and output sampling rate as 8K or 16 (depending on 
			// the passed in parameter sr), so we know we need to use the HW srcMixer.
			config.source = CSL_CAPH_DEV_MEMORY;
			config.sink = CSL_CAPH_DEV_DSP;
		}

		(void) csl_caph_hwctrl_DisablePath(config);
		

		//Remove this path from the path table.
		AUDCTRL_RemoveFromTable(pathID);
	}
	if((mic == AUDCTRL_MIC_DIGI1) 
	   || (mic == AUDCTRL_MIC_DIGI2) 
	   || (mic == AUDCTRL_MIC_DIGI3) 
	   || (mic == AUDCTRL_MIC_DIGI4) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
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
// Function Name: AUDCTRL_SetRecordGainMono
//
// Description:   set gain of a record path for a single mic
//
//============================================================================
static void AUDCTRL_SetRecordGainMono(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
                AUDIO_GAIN_FORMAT_t     gainFormat,
				Int16					gainL,
				Int16					gainR
				)
{
    CSL_CAPH_PathID pathID = 0;
    Int16 gainLTemp = 0; //quarter dB
    Int16 gainRTemp = 0;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordGainMono: src = 0x%x,  mic = 0x%x, gainL = 0x%x, gainR = 0x%x\n", src, mic, gainL, gainR);

	if( src == AUDIO_HW_USB_IN)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(src, AUDIO_HW_NONE, AUDCTRL_SPK_UNDEFINED, mic);
    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }

	if(gainFormat == AUDIO_GAIN_FORMAT_mB)
	{
        gainLTemp = (gainL/25);
        gainRTemp = (gainR/25);
    }

    (void) csl_caph_hwctrl_SetSourceGain(pathID, gainLTemp, gainRTemp);

    return;
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
                AUDIO_GAIN_FORMAT_t     gainFormat,
				UInt32					gainL,
				UInt32					gainR
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordGain: src = 0x%x,  mic = 0x%x, gainL = 0x%lx, gainR = 0x%lx\n", src, mic, gainL, gainR);

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 || mic==AUDCTRL_DUAL_MIC_DIGI21 || mic==AUDCTRL_MIC_SPEECH_DIGI)
	{
		AUDCTRL_SetRecordGainMono(src, AUDCTRL_MIC_DIGI1, gainFormat, (Int16)gainL, (Int16)gainR);
		AUDCTRL_SetRecordGainMono(src, AUDCTRL_MIC_DIGI2, gainFormat, (Int16)gainL, (Int16)gainR);
	} else {
		AUDCTRL_SetRecordGainMono(src, mic, gainFormat, (Int16)gainL, (Int16)gainR);
	}

    return;
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordMuteMono
//
// Description:   mute/unmute a record path for a single mic
//
//============================================================================
static void AUDCTRL_SetRecordMuteMono(
				AUDIO_HW_ID_t			src,
				AUDCTRL_MICROPHONE_t	mic,
				Boolean					mute
				)
{
    CSL_CAPH_PathID pathID = 0;
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMuteMono: src = 0x%x,  mic = 0x%x, mute = 0x%x\n", 
                    src, mic, mute);

	if( src == AUDIO_HW_USB_IN)
		return;

    pathID = AUDCTRL_GetPathIDFromTable(src, AUDIO_HW_NONE, AUDCTRL_SPK_UNDEFINED, mic);

    if(pathID == 0)
    {
	audio_xassert(0,pathID);
	return;
    }	

    if (mute == TRUE)
    {
        (void) csl_caph_hwctrl_MuteSource(pathID);
    }
    else
    {
        (void) csl_caph_hwctrl_UnmuteSource(pathID);
    }

    return;    
}

//============================================================================
//
// Function Name: AUDCTRL_SetRecordMuteMono
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
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMute: src = 0x%x,  mic = 0x%x, mute = 0x%x\n", 
                    src, mic, mute);

	if(mic==AUDCTRL_DUAL_MIC_DIGI12 || mic==AUDCTRL_DUAL_MIC_DIGI21 || mic==AUDCTRL_MIC_SPEECH_DIGI)
	{
		AUDCTRL_SetRecordMuteMono(src, AUDCTRL_MIC_DIGI1, mute);
		AUDCTRL_SetRecordMuteMono(src, AUDCTRL_MIC_DIGI2, mute);
	} else {
		AUDCTRL_SetRecordMuteMono(src, mic, mute);
	}

    return;    
}


//============================================================================
//
// Function Name: AUDCTRL_SetMixingGain
//
// Description:   set mixing gain of a path
//
//============================================================================
void AUDCTRL_SetMixingGain(AUDIO_HW_ID_t src,
			AUDIO_HW_ID_t sink,
			AUDCTRL_MICROPHONE_t mic,
			AUDCTRL_SPEAKER_t spk,
			AUDCTRL_MIX_SELECT_t mixSelect,
			Boolean isDSPGain,
			Boolean dspSpeechProcessingNeeded,
			UInt32 gain,
			UInt32 inPathID)
{
    CSL_CAPH_PathID pathID = 0;
    Int16 dspGain = 0;
    if (isDSPGain)
    {
        //Call DSP CSL interface to set DSP gain.
	switch (mixSelect)
	{
        case AUDCTRL_MIX_EAR_AUDIO_PLAY:
		//The only use case where HW mixer is needed:
		// 48/44.1KHZ Music playback to HS/EP/USB
		// during a voice call.
       if ((spk == AUDCTRL_SPK_HANDSET)
		     ||(spk == AUDCTRL_SPK_HEADSET)
		     ||(spk == AUDCTRL_SPK_USB)) 
       {
            pathID = AUDCTRL_GetPathIDFromTable(src, sink, spk, mic);
            if(pathID == 0)
            {
                audio_xassert(0,pathID);
                return;
            }
                (void) csl_caph_hwctrl_SetMixingGain(pathID, 
                            gain, gain);
        }
		else // for IHF
		{
                    //Set shared_newaudiofifo_gain_dl
		}
                break;
        case AUDCTRL_MIX_EAR_TONE:
		dspGain = (Int16)gain;
		// It is decided that tone will always use
		// ARM2SP interface #2.
	    if(FALSE == CSL_SetARM2Speech2DLGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;
        case AUDCTRL_MIX_EAR_SPEECH_PLAY:
		dspGain = (Int16)gain;
		// It is decided that speech playback will always use
		// ARM2SP interface #1.
	    if(FALSE == CSL_SetARM2SpeechDLGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;
        case AUDCTRL_MIX_EAR_DL:
		dspGain = (Int16)gain;
	    if(FALSE == CSL_SetInpSpeechToARM2SpeechMixerDLGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_UL_MIC:
		dspGain = (Int16)gain;
	    if(FALSE == CSL_SetInpSpeechToARM2SpeechMixerULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_UL_TONE:
		dspGain = (Int16)gain;
		// It is decided that tone will always use
		// ARM2SP interface #2.		
	    if(FALSE == CSL_SetARM2Speech2ULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_UL_SPEECH_PLAY:
        case AUDCTRL_MIX_UL_AUDIO_PLAY:
		dspGain = (Int16)gain;
		// It is decided that speech/music playback will always use
		// ARM2SP interface #1.
	    if(FALSE == CSL_SetARM2SpeechULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_SPEECH_REC_TONE:
		dspGain = (Int16)gain;
	    if(FALSE == CSL_SetARM2SpeechULGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
        case AUDCTRL_MIX_SPEECH_REC_MIC:
		dspGain = (Int16)gain;
	    AUDDRV_SetULSpeechRecordGain(dspGain);
		break;	
        case AUDCTRL_MIX_SPEECH_REC_DL:
		dspGain = (Int16)gain;
        if(FALSE == CSL_SetDlSpeechRecGain(dspGain))
		{
                    audio_xassert(0,0);
		}
		break;	
	    default:
		;
	}
    }
    else
    {
		if(inPathID) 
			pathID = inPathID;
        else 
			pathID = AUDCTRL_GetPathIDFromTable(src, sink, spk, mic);
        if(pathID == 0)
        {
            audio_xassert(0,pathID);
            return;
        }
        (void) csl_caph_hwctrl_SetMixingGain(pathID, gain, gain);
    }
    return;
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
    //Sidetone FIR filter coeffs.
	static UInt32 sidetoneCoeff[128] = {
								0x7FFFFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
							};
    UInt32 *coeff = &sidetoneCoeff[0];

    CSL_AUDIO_DEVICE_e source, sink;
    static CSL_CAPH_DEVICE_e audSpkr;
    //static AUDDRV_MIC_Enum_t audMic;
    CSL_CAPH_PathID pathID;
    AUDCTRL_Config_t data;
    AUDIO_HW_ID_t audPlayHw, audRecHw;

    CSL_CAPH_HWCTRL_CONFIG_t hwCtrlConfig;
	Int16 tempGain = 0;
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);

    memset(&hwCtrlConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    audPlayHw = audRecHw = AUDIO_HW_NONE;
    source = sink = CSL_CAPH_DEV_NONE;
    audSpkr = CSL_CAPH_DEV_NONE;
    //audMic = AUDDRV_MIC_NONE;
    pathID = 0;
    memset(&data, 0, sizeof(AUDCTRL_Config_t));
    switch (mic)
    {
        case AUDCTRL_MIC_MAIN:
            source = CSL_CAPH_DEV_ANALOG_MIC;
            //audMic = AUDDRV_MIC_ANALOG_MAIN;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDCTRL_MIC_AUX:
            source = CSL_CAPH_DEV_HS_MIC;
            //audMic = AUDDRV_MIC_ANALOG_AUX;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDCTRL_MIC_SPEECH_DIGI:
            source = CSL_CAPH_DEV_DIGI_MIC;
            //audMic = AUDDRV_MIC_SPEECH_DIGI;
            break;	    
        case AUDCTRL_MIC_DIGI1:
            source = CSL_CAPH_DEV_DIGI_MIC_L;
            //audMic = AUDDRV_MIC_DIGI1;
            break;
        case AUDCTRL_MIC_DIGI2:
            source = CSL_CAPH_DEV_DIGI_MIC_R;
            //audMic = AUDDRV_MIC_DIGI2;
            break;
        case AUDCTRL_MIC_DIGI3:
            source = CSL_CAPH_DEV_EANC_DIGI_MIC_L;
            //audMic = AUDDRV_MIC_DIGI3;
            break;
        case AUDCTRL_MIC_DIGI4:
            source = CSL_CAPH_DEV_EANC_DIGI_MIC_R;
            //audMic = AUDDRV_MIC_DIGI4;
            break;
        case AUDCTRL_MIC_I2S:
            source = CSL_CAPH_DEV_FM_RADIO;
            audRecHw = AUDIO_HW_I2S_IN;
            break;
        case AUDCTRL_MIC_BTM:
            source = CSL_CAPH_DEV_BT_MIC;
            audRecHw = AUDIO_HW_MONO_BT_IN;
            break;
        default:
            //audMic = AUDDRV_MIC_ANALOG_MAIN;
            source = CSL_CAPH_DEV_ANALOG_MIC;
            audRecHw = AUDIO_HW_I2S_IN;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
            break;
    }

    switch (speaker)
    {
        case AUDCTRL_SPK_HANDSET:
            sink = CSL_CAPH_DEV_EP;
            audSpkr = CSL_CAPH_DEV_EP;
            audPlayHw = AUDIO_HW_EARPIECE_OUT;
            audio_mode = AUDIO_MODE_HANDSET;
            break;
        case AUDCTRL_SPK_HEADSET:
            sink = CSL_CAPH_DEV_HS;
            audSpkr = CSL_CAPH_DEV_HS;
            audPlayHw = AUDIO_HW_HEADSET_OUT;
            audio_mode = AUDIO_MODE_HEADSET;
            break;
        case AUDCTRL_SPK_LOUDSPK:
            sink = CSL_CAPH_DEV_IHF;
            audSpkr = CSL_CAPH_DEV_IHF;
            audio_mode = AUDIO_MODE_SPEAKERPHONE;
            break;
        case AUDCTRL_SPK_I2S:
            sink = CSL_CAPH_DEV_FM_TX;
            audPlayHw = AUDIO_HW_I2S_OUT;
            // No audio mode available for this case.
            // for now just use AUDIO_MODE_HANDSFREE
            audio_mode = AUDIO_MODE_HANDSFREE;
            break;
        case AUDCTRL_SPK_BTM:
            sink = CSL_CAPH_DEV_BT_SPKR;
            audPlayHw = AUDIO_HW_MONO_BT_OUT;
            audio_mode = AUDIO_MODE_BLUETOOTH;
            break;
        default:
            audSpkr = CSL_CAPH_DEV_EP;
            sink = CSL_CAPH_DEV_EP;
            audPlayHw = AUDIO_HW_EARPIECE_OUT;
            audio_mode = AUDIO_MODE_HANDSET;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);
            break;
    }

	audio_control_generic( AUDDRV_CPCMD_PassAudioMode, 
            (UInt32)audio_mode, 0, 0, 0, 0);
    if(enable_lpbk)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: Enable loopback \n");

	// For I2S/PCM loopback
        if (((source == CSL_CAPH_DEV_FM_RADIO) && (sink == CSL_CAPH_DEV_FM_TX)) ||
		((source == CSL_CAPH_DEV_BT_MIC) && (sink == CSL_CAPH_DEV_BT_SPKR)))
        {
            // I2S hard coded to use ssp3, BT PCM to use ssp4. This could be changed later
			AUDIO_HW_ID_t srcTemp = AUDIO_HW_I2S_IN, sinkTemp = AUDIO_HW_I2S_OUT;
			if(source == CSL_CAPH_DEV_BT_MIC) srcTemp = AUDIO_HW_MONO_BT_IN;
			if(sink == CSL_CAPH_DEV_BT_SPKR) sinkTemp = AUDIO_HW_MONO_BT_OUT;
            AUDCTRL_EnablePlay (srcTemp, audPlayHw, AUDIO_HW_NONE, speaker, AUDIO_CHANNEL_MONO, 48000, NULL);
            AUDCTRL_EnableRecord (audRecHw, sinkTemp, mic, AUDIO_CHANNEL_MONO, 48000);
            return;
        }
#if 0 //removed this to make fm radio work using xpft script
	    if (source == AUDDRV_DEV_FM_RADIO)
	    {
            AUDCTRL_EnableRecord (audRecHw, audPlayHw, mic, AUDIO_CHANNEL_STEREO, 48000);
	        if ((speaker == AUDCTRL_SPK_LOUDSPK)||(speaker == AUDCTRL_SPK_HEADSET))	
	            powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );	    
	        return;
	    }
#endif
        //  Microphone pat
    	if((mic == AUDCTRL_MIC_DIGI1) 
    	   || (mic == AUDCTRL_MIC_DIGI2) 
    	   || (mic == AUDCTRL_MIC_DIGI3) 
    	   || (mic == AUDCTRL_MIC_DIGI4) 
    	   || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
    	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
    	   || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	    {
		    // Enable power to digital microphone
    		powerOnDigitalMic(TRUE);
        }
	// enable HW path
        hwCtrlConfig.streamID = CSL_CAPH_STREAM_NONE;
        hwCtrlConfig.source = source;
        hwCtrlConfig.sink = sink;
        hwCtrlConfig.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
        hwCtrlConfig.chnlNum = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        hwCtrlConfig.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_input_gain_l);	
        hwCtrlConfig.mixGain.mixInGainL = AUDDRV_GetMixerInputGain(tempGain);
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_fine_gain_l);
        hwCtrlConfig.mixGain.mixOutGainL = AUDDRV_GetMixerOutputFineGain(tempGain);	
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_coarse_gain_l);
        hwCtrlConfig.mixGain.mixOutCoarseGainL = AUDDRV_GetMixerOutputCoarseGain(tempGain);

        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_input_gain_r);	
        hwCtrlConfig.mixGain.mixInGainR = AUDDRV_GetMixerInputGain(tempGain);	
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_fine_gain_r);
        hwCtrlConfig.mixGain.mixOutGainR = AUDDRV_GetMixerOutputFineGain(tempGain);	
        tempGain = (Int16)(AUDIO_GetParmAccessPtr()[audio_mode].srcmixer_output_coarse_gain_r);
        hwCtrlConfig.mixGain.mixOutCoarseGainR = AUDDRV_GetMixerOutputCoarseGain(tempGain);

        pathID = csl_caph_hwctrl_EnablePath(hwCtrlConfig);

#ifdef HW_SIDETONE_LOOPBACK        
        //Enable the sidetone path.
		csl_caph_hwctrl_ConfigSidetoneFilter(coeff);
		csl_caph_hwctrl_SetSidetoneGain(0); // Set sidetone gain to 0dB.
		csl_caph_hwctrl_EnableSidetone(sink);
#endif        
    
        // Enable Loopback ctrl
	    //Enable PMU for headset/IHF
    	if ((speaker == AUDCTRL_SPK_LOUDSPK)
    	    ||(speaker == AUDCTRL_SPK_HEADSET))	
	        powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );	    
		// up merged : remove the comment later on
		// after mergining latest changes
	    if (((source == CSL_CAPH_DEV_ANALOG_MIC) 
	            || (source == CSL_CAPH_DEV_HS_MIC)) 
            && ((sink == CSL_CAPH_DEV_EP) 
                || (sink == CSL_CAPH_DEV_IHF)
                || (sink == CSL_CAPH_DEV_HS)))
        {
#ifdef HW_ANALOG_LOOPBACK            
            csl_caph_audio_loopback_control(audSpkr, 0, enable_lpbk);
#endif        
        }

        //Save this path to the path table.
        data.pathID = pathID;
        data.src = AUDIO_HW_VOICE_IN;
        data.sink = AUDIO_HW_VOICE_OUT;
        data.mic = mic;
        data.spk = speaker;
        data.numCh = (speaker == AUDCTRL_SPK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        data.sr = AUDIO_SAMPLING_RATE_48000;
        AUDCTRL_AddToTable(&data);
    }
    else
    {
        // Disable Analog Mic path
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: Disable loopback\n");

	// Disable I2S/PCM loopback
        if (((source == CSL_CAPH_DEV_FM_RADIO) && (sink == CSL_CAPH_DEV_FM_TX)) ||
		((source == CSL_CAPH_DEV_BT_MIC) && (sink == CSL_CAPH_DEV_BT_SPKR)))
        {
            // I2S configured to use ssp3, BT PCM to use ssp4.
            AUDCTRL_DisablePlay (AUDIO_HW_SPEECH_IN, audPlayHw, speaker, 0);
            AUDCTRL_DisableRecord (audRecHw, AUDIO_HW_EARPIECE_OUT, mic);
            return;
        }
#if 0 //removed this to make fm radio work using xpft script
	    if (source == CSL_CAPH_DEV_FM_RADIO)
	    {
            AUDCTRL_DisableRecord (audRecHw, audPlayHw, mic);
	        if ((speaker == AUDCTRL_SPK_LOUDSPK)||(speaker == AUDCTRL_SPK_HEADSET))	
	            powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );	    
	        return;
	    }
#endif
    	if((mic == AUDCTRL_MIC_DIGI1) 
    	   || (mic == AUDCTRL_MIC_DIGI2) 
	       || (mic == AUDCTRL_MIC_DIGI3) 
    	   || (mic == AUDCTRL_MIC_DIGI4) 
	       || (mic == AUDCTRL_DUAL_MIC_DIGI12) 
    	   || (mic == AUDCTRL_DUAL_MIC_DIGI21)
	       || (mic == AUDCTRL_MIC_SPEECH_DIGI))		
	    {
			// Enable power to digital microphone
			powerOnDigitalMic(FALSE);
		}	

        memset(&hwCtrlConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
        pathID = AUDCTRL_GetPathIDFromTable(AUDIO_HW_VOICE_IN, AUDIO_HW_VOICE_OUT, speaker, mic);
    	if(pathID == 0)
	    {
		    audio_xassert(0,pathID);
		    return;
	    }
	
        hwCtrlConfig.pathID = pathID;
		// up merged : remove the comment later on
		// after mergining latest changes
if (((source == CSL_CAPH_DEV_ANALOG_MIC) 
	            || (source == CSL_CAPH_DEV_HS_MIC)) 
            && ((sink == CSL_CAPH_DEV_EP) 
                || (sink == CSL_CAPH_DEV_IHF)
                || (sink == CSL_CAPH_DEV_HS)))
		{
#ifdef HW_ANALOG_LOOPBACK            
		    csl_caph_audio_loopback_control(audSpkr, 0, enable_lpbk);
#endif            
		}

#ifdef HW_SIDETONE_LOOPBACK        
        //Disable Sidetone path.
        csl_caph_hwctrl_DisableSidetone(sink);
#endif

		(void) csl_caph_hwctrl_DisablePath(hwCtrlConfig); //clocks are disabled here, so no register access after this.

		//Enable PMU for headset/IHF
    	if ((speaker == AUDCTRL_SPK_LOUDSPK)
	        ||(speaker == AUDCTRL_SPK_HEADSET))	
		{
			powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );	    
		}
        
        //Remove this path to the path table.
        AUDCTRL_RemoveFromTable(pathID);
    }
}

//============================================================================
//
// Function Name: AUDCTRL_SetArm2spParam
//
// Description:   Set Arm2Sp Parameter
// 
//============================================================================

void AUDCTRL_SetArm2spParam( UInt32 mixMode, UInt32 instanceId )
{
    csl_caph_arm2sp_set_param(mixMode, instanceId);
}

//============================================================================
//
// Function Name: AUDCTRL_ConfigSSP
//
// Description:   Set FM/PCM SSP protocol, and port number
// 
//============================================================================
void AUDCTRL_ConfigSSP(AUDCTRL_SSP_PORT_e port, AUDCTRL_SSP_BUS_e bus)
{
	CSL_SSP_PORT_e csl_port;
	CSL_SSP_BUS_e csl_bus;

	if(port==AUDCTRL_SSP_4) csl_port = CSL_SSP_4;
	else if(port==AUDCTRL_SSP_3) csl_port = CSL_SSP_3;
	else return;

	if(bus==AUDCTRL_SSP_I2S) csl_bus = CSL_SSP_I2S;
	else if(bus==AUDCTRL_SSP_PCM) csl_bus = CSL_SSP_PCM;
	else return;
	
	csl_caph_hwctrl_ConfigSSP(csl_port, csl_bus);
}

//============================================================================
//
// Function Name: AUDCTRL_SetSspTdmMode
//
// Description:   Control SSP TDM feature
// 
//============================================================================

void AUDCTRL_SetSspTdmMode( Boolean status )
{
	csl_caph_hwctrl_SetSspTdmMode(status);
}

//============================================================================
//
// Function Name: AUDCTRL_SetBtMode
//
// Description:   Set BT mode
// 
//============================================================================
void  AUDCTRL_SetBTMode(Boolean mode)
{
	 csl_caph_hwctrl_SetBTMode(mode);
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
	 csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_BYPASS_MODE, TRUE);
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
	 csl_caph_hwctrl_vibrator(AUDDRV_VIBRATOR_BYPASS_MODE, FALSE);
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

	 csl_caph_hwctrl_vibrator_strength(vib_power);
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
void AUDCTRL_AddToTable(AUDCTRL_Config_t* data)
{
    AUDCTRL_Table_t* newNode = NULL;
	//Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_AddToTable: pathID = %d, src = %d, sink = %d, mic = %d, spk = %d\n", data->pathID, data->src, data->sink, data->mic, data->spk);
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
void AUDCTRL_RemoveFromTable(CSL_CAPH_PathID pathID)
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
static void AUDCTRL_DeleteTable(void)
{
    AUDCTRL_Table_t* currentNode = tableHead;
    AUDCTRL_Table_t* next = NULL;

    while(currentNode != NULL)
    {
        next = currentNode->next;
        memset(currentNode, 0, sizeof(AUDCTRL_Table_t));
   	    OSHEAP_Delete(currentNode); 
        currentNode = next;
    }
    tableHead = NULL;
    return;
}

//============================================================================
//
// Function Name: AUDCTRL_GetFromTable
//
// Description:   Get a path information from the table.
//
//============================================================================
AUDCTRL_Config_t AUDCTRL_GetFromTable(CSL_CAPH_PathID pathID)
{
    AUDCTRL_Config_t data; 
    AUDCTRL_Table_t* currentNode = tableHead; 
    memset(&data, 0, sizeof(AUDCTRL_Config_t));

    while(currentNode != NULL)
    {
        if ((currentNode->data).pathID == pathID)
        {
            memcpy(&data, &(currentNode->data), sizeof(AUDCTRL_Config_t));
            return data;
        }
        else
        {
            currentNode= currentNode->next;
        }
    }
    return data;

}

/********************************************************************
*  @brief  Set IHF mode
*
*  @param  IHF mode status (TRUE: stereo | FALSE: mono).
*
*  @return  none
*
****************************************************************************/
void AUDCTRL_SetIHFmode (Boolean stIHF)
{
    csl_caph_hwctrl_SetIHFmode(stIHF);
}

//============================================================================
//
// Function Name: AUDCTRL_GetPathIDFromTable
//
// Description:   Get a path ID from the table.
//
//============================================================================
static CSL_CAPH_PathID AUDCTRL_GetPathIDFromTable(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
				                                AUDCTRL_MICROPHONE_t mic)
{
    AUDCTRL_Table_t* currentNode = tableHead;     
    while(currentNode != NULL)
    {
	    //Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_GetPathIDFromTable: pathID = %d, src = %d, sink = %d, mic = %d, spk = %d\n",
        //            (currentNode->data).pathID, (currentNode->data).src, (currentNode->data).sink, (currentNode->data).mic, (currentNode->data).spk);
		
	
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
// Function Name: AUDCTRL_GetPathIDFromTableWithSrcSink
//
// Description:   Get a path ID from the table.
//
//============================================================================
static CSL_CAPH_PathID AUDCTRL_GetPathIDFromTableWithSrcSink(AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
				                                AUDCTRL_MICROPHONE_t mic)
{

    AUDCTRL_Table_t* currentNode = tableHead;     
    while(currentNode != NULL)
    {
	    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_GetPathIDFromTableWithSrcSink: pathID = %d, src = %d, sink = %d, mic = %d, spk = %d\n",
                    (currentNode->data).pathID, (currentNode->data).src, (currentNode->data).sink, (currentNode->data).mic, (currentNode->data).spk);
    
        if ((((currentNode->data).src == src)&&((currentNode->data).mic == mic))
            &&(((currentNode->data).sink == sink)&&((currentNode->data).spk == spk)))
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
// Function Name: AUDCTRL_UpdatePath
//
// Description:   update a path with new src/sink/spk/mic.
//
//============================================================================
static void AUDCTRL_UpdatePath (CSL_CAPH_PathID pathID,
                                                AUDIO_HW_ID_t src,
                                                AUDIO_HW_ID_t sink,
                                                AUDCTRL_SPEAKER_t spk,
                                                AUDCTRL_MICROPHONE_t mic)
{
    AUDCTRL_Table_t* currentNode = tableHead; 

    while(currentNode != NULL)
    {
        if ((currentNode->data).pathID == pathID)
        {
	        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_UpdatePath:  pathID = %d, src = %d, sink = %d, mic = %d, spk = %d\n",
                    pathID, src, sink, mic, spk);
            (currentNode->data).src = src;
            (currentNode->data).sink = sink;
            (currentNode->data).spk = spk;
            (currentNode->data).mic = mic;
            return;
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
// Function Name: GetDeviceFromHWID
//
// Description:   convert audio controller HW ID enum to auddrv device enum
//
//============================================================================
static CSL_CAPH_DEVICE_e GetDeviceFromHWID(AUDIO_HW_ID_t hwID)
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
static CSL_CAPH_DEVICE_e GetDeviceFromMic(AUDCTRL_MICROPHONE_t mic)
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
static CSL_CAPH_DEVICE_e GetDeviceFromSpkr(AUDCTRL_SPEAKER_t spkr)
{
	Log_DebugPrintf(LOGID_AUDIO,"GetDeviceFromSpkr: hwID = 0x%x\n", spkr);
    return SPKR_Mapping_Table[spkr].dev;
}

//============================================================================
//
// Function Name: SetGainOnExternalAmp
//
// Description:   Set gain on external amplifier driver. Gain in Q13.2
//
// parameter:
//  left_right is of this enum
//enum {
//		PMU_AUDIO_HS_RIGHT,
//		PMU_AUDIO_HS_LEFT,
//		PMU_AUDIO_HS_BOTH
//};
//
//============================================================================
static void SetGainOnExternalAmp(AUDCTRL_SPEAKER_t speaker, int arg_gain, int left_right)
{
#if !defined(NO_PMU)
	int gain=0;

	switch(speaker)
	{
		case AUDCTRL_SPK_HEADSET:
		case AUDCTRL_SPK_TTY:
			gain = map2pmu_hs_gain_fromQ13dot2( arg_gain );
			AUDIO_PMU_HS_SET_GAIN( left_right, gain );
			break;

		case AUDCTRL_SPK_LOUDSPK:
			gain = map2pmu_ihf_gain_fromQ13dot2( arg_gain );
			AUDIO_PMU_IHF_SET_GAIN( gain );
			break;

		default:
			break;
	}

	Log_DebugPrintf(LOGID_AUDIO, 
				"SetGainOnExternalAmp, speaker = %d, arg_gain=%d, gain=%d \n",
				speaker, arg_gain, gain);
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
	// May need to turn off PMU if speaker is not IHF/HS, but its PMU is still on.
	if (speaker != AUDCTRL_SPK_HEADSET && speaker != AUDCTRL_SPK_TTY && speaker != AUDCTRL_SPK_LOUDSPK && (!IHF_IsOn && !HS_IsOn))
	{
		return retValue;
	}

/////////////////////////////////////////////////////////////////////////////////
//  Required ! Linux version only
////////////////////////////////////////////////////////////////////////////////////
	
	if (use == TRUE)
		AUDIO_PMU_INIT(); 	//enable the audio PLL before power ON


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

		default: //not HS/IHF, so turn off HS/IHF PMU if its PMU is on.
			switch(usage_flag)
			{
				case TelephonyUseExtSpkr:
					telephonyUseIHF = FALSE;
					telephonyUseHS = FALSE;
					break;


				case AudioUseExtSpkr:
					audioUseIHF = FALSE;
					audioUseHS = FALSE;
					break;

				default:
					break;
			}
			break;
	}

	if ((telephonyUseHS==FALSE) && (audioUseHS==FALSE))
	{
		if ( HS_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu HS amp\n");

/////////////////////////////////////////////////////////////////////////////////
//  Start PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////
			
            AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE),
            AUDIO_PMU_HS_POWER(FALSE);
            OSTASK_Sleep(20);

/////////////////////////////////////////////////////////////////////////////////
//  End PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////

		}
		HS_IsOn = FALSE;
	}
	else
	{
	
/////////////////////////////////////////////////////////////////////////////////
//  Start PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////

		int hs_path;	
		int hs_gain = 0;
#if defined(CONFIG_BCM59055_AUDIO) || defined(CONFIG_BCMPMU_AUDIO) 
		hs_path = PMU_AUDIO_HS_BOTH;
#endif

		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (HS on), telephonyUseHS = %d, audioUseHS= %d\n", telephonyUseHS, audioUseHS);

		if ( HS_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, gain %d\n", hs_gain);
            //printk("richlu before 59055 hs_set_gain \n");
            AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE),
            //printk("richlu after 59055 hs_set_gain and before hs_power\n");
            AUDIO_PMU_HS_POWER(TRUE);
            //printk("richlu  after 59055 hs_power \n");
            OSTASK_Sleep(75);

		}
		
		//the ext_speaker_pga_l is in q13.2 format
		hs_gain = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
		SetGainOnExternalAmp( AUDCTRL_SPK_HEADSET, hs_gain, PMU_AUDIO_HS_LEFT);

		hs_gain = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_r;
		SetGainOnExternalAmp( AUDCTRL_SPK_HEADSET, hs_gain, PMU_AUDIO_HS_RIGHT);

		HS_IsOn = TRUE;
	}

	if ((telephonyUseIHF==FALSE) && (audioUseIHF==FALSE))
	{
		if ( IHF_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu IHF amp\n");
            AUDIO_PMU_IHF_SET_GAIN(PMU_IHFGAIN_MUTE),
            AUDIO_PMU_IHF_POWER(FALSE);
            
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
		int ihf_gain = 0;

		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (IHF on), telephonyUseIHF = %d, audioUseIHF= %d\n", telephonyUseIHF, audioUseIHF);

		if ( IHF_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu IHF amp, gain %d\n", ihf_gain);
            AUDIO_PMU_IHF_SET_GAIN(PMU_IHFGAIN_MUTE),
			AUDIO_PMU_IHF_POWER(TRUE);
		}

		//the ext_speaker_pga_l is in q13.2 format		
		ihf_gain = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
		SetGainOnExternalAmp( AUDCTRL_SPK_LOUDSPK, ihf_gain, PMU_AUDIO_HS_BOTH);

		IHF_IsOn = TRUE;
	}

	if ( IHF_IsOn==FALSE && HS_IsOn==FALSE )
		AUDIO_PMU_DEINIT();    //disable the audio PLL after power OFF
    Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp: retValue %d\n", retValue);
#endif    
	return retValue;
	
/////////////////////////////////////////////////////////////////////////////////
//  End PMU code. Linux version only
////////////////////////////////////////////////////////////////////////////////////

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
#ifdef CONFIG_DIGI_MIC
		// Enable power to digital microphone
		PMU_SetLDOMode(PMU_HVLDO7CTRL,0);
#endif		
	}
	else //powerOn == FALSE
	{
#ifdef CONFIG_DIGI_MIC
		// Enable power to digital microphone
		PMU_SetLDOMode(PMU_HVLDO7CTRL,1);
#endif		
	}
#endif
}

//============================================================================
//
// Function Name: AUDCTRL_ControlHWClock
//
// Description:   Enable/Disable CAPH clock
//
//============================================================================

void  AUDCTRL_ControlHWClock(Boolean enable)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDCTRL_ControlHWClock enable %d\r\n",enable);
	csl_caph_ControlHWClock(enable);
}

//============================================================================
//
// Function Name: AUDCTRL_ControlHWClock
//
// Description:  Query if CAPH clock is enabled/disabled
//
//============================================================================

Boolean  AUDCTRL_QueryHWClock(void)
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDCTRL_QueryHWClock \r\n");
	return csl_caph_QueryHWClock();
}

#else  //#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)




//CP version:

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"

#include "csl_caph.h"
#include "csl_caph_hwctrl.h"
#include "audio_controller.h"
#include "log.h"


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

	csl_caph_hwctrl_init();
}

//============================================================================
//
// Function Name: AUDCTRL_Shutdown
//
// Description:   De-Initialize audio controller
//
//============================================================================
void AUDCTRL_Shutdown(void)
{

}


#endif //#if defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR)

