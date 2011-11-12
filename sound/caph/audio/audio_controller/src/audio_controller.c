/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010 - 2011  Broadcom Corporation                                                        */
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
* @file   audio_controller.c
* @brief
*
******************************************************************************/

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
#include "csl_caph_audioh.h"
#include "csl_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "audio_controller.h"
#include "log.h"
#include "osheap.h"

#ifdef CONFIG_DIGI_MIC
#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))
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
// Private Type and Constant declarations
//=============================================================================

typedef struct node
{
    AUDCTRL_Config_t data;
    struct node*    next;
    struct node*    prev;
} AUDCTRL_Table_t;

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
    AUDIO_SINK_Enum_t spkr;
    CSL_CAPH_DEVICE_e dev;
} AUDCTRL_SPKR_Mapping_t;

static AUDCTRL_SPKR_Mapping_t SPKR_Mapping_Table[AUDIO_SINK_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDIO_SINK_HANDSET,		CSL_CAPH_DEV_EP},
	{AUDIO_SINK_HEADSET,		CSL_CAPH_DEV_HS},
	{AUDIO_SINK_HANDSFREE,		CSL_CAPH_DEV_IHF},
	{AUDIO_SINK_BTM,		    CSL_CAPH_DEV_BT_SPKR},
	{AUDIO_SINK_LOUDSPK,		CSL_CAPH_DEV_IHF},
	{AUDIO_SINK_TTY,		    CSL_CAPH_DEV_HS},
	{AUDIO_SINK_HAC,		    CSL_CAPH_DEV_EP},
	{AUDIO_SINK_USB,		    CSL_CAPH_DEV_MEMORY},
	{AUDIO_SINK_BTS,		    CSL_CAPH_DEV_BT_SPKR},
	{AUDIO_SINK_I2S,		    CSL_CAPH_DEV_FM_TX},
	{AUDIO_SINK_VIBRA,		    CSL_CAPH_DEV_VIBRA},
	{AUDIO_SINK_HEADPHONE,		CSL_CAPH_DEV_HS},
	{AUDIO_SINK_UNDEFINED,		CSL_CAPH_DEV_NONE}
};

typedef struct
{
    AUDIO_SINK_Enum_t spkr;
    AUDIO_SINK_Enum_t auddrv_spkr;
} AUDCTRL_DRVSPKR_Mapping_t;

static AUDCTRL_DRVSPKR_Mapping_t DRVSPKR_Mapping_Table[AUDIO_SINK_TOTAL_COUNT] =
{
    //HW ID                    // Auddrv Spkr ID
    {AUDIO_SINK_HANDSET,    AUDIO_SINK_HANDSET},
    {AUDIO_SINK_HEADSET,    AUDIO_SINK_HEADSET},
    {AUDIO_SINK_HANDSFREE,  AUDIO_SINK_LOUDSPK},
    {AUDIO_SINK_BTM,	       AUDIO_SINK_BTM},
    {AUDIO_SINK_LOUDSPK,    AUDIO_SINK_LOUDSPK},
    {AUDIO_SINK_TTY,	       AUDIO_SINK_HEADSET},
    {AUDIO_SINK_HAC,	       AUDIO_SINK_HANDSET},
    {AUDIO_SINK_USB,	       AUDIO_SINK_USB},
    {AUDIO_SINK_BTS,	       AUDIO_SINK_UNDEFINED},
    {AUDIO_SINK_I2S,	       AUDIO_SINK_I2S},
    {AUDIO_SINK_VIBRA,      AUDIO_SINK_VIBRA},
    {AUDIO_SINK_UNDEFINED,  AUDIO_SINK_UNDEFINED}
};

typedef struct
{
    AUDIO_SOURCE_Enum_t mic;
    CSL_CAPH_DEVICE_e dev;
} AUDIO_SOURCE_Mapping_t;

static AUDIO_SOURCE_Mapping_t MIC_Mapping_Table[AUDIO_SOURCE_TOTAL_COUNT] =
{
	//HW ID				// Device ID
	{AUDIO_SOURCE_UNDEFINED,		    CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_ANALOG_MAIN,		        CSL_CAPH_DEV_ANALOG_MIC},
	{AUDIO_SOURCE_ANALOG_AUX,		        CSL_CAPH_DEV_HS_MIC},
	{AUDIO_SOURCE_DIGI1,     		CSL_CAPH_DEV_DIGI_MIC_L},
	{AUDIO_SOURCE_DIGI2,		        CSL_CAPH_DEV_DIGI_MIC_R},
	{AUDIO_SOURCE_BTM,		        CSL_CAPH_DEV_BT_MIC},
	{AUDIO_SOURCE_USB,       		CSL_CAPH_DEV_MEMORY},
	{AUDIO_SOURCE_I2S,		        CSL_CAPH_DEV_FM_RADIO},
	{AUDIO_SOURCE_DIGI3,	        	CSL_CAPH_DEV_EANC_DIGI_MIC_L},
	{AUDIO_SOURCE_DIGI4,		        CSL_CAPH_DEV_EANC_DIGI_MIC_R},
	{AUDIO_SOURCE_SPEECH_DIGI,       CSL_CAPH_DEV_DIGI_MIC},
	{AUDIO_SOURCE_EANC_DIGI,		    CSL_CAPH_DEV_EANC_DIGI_MIC}
};

 //=============================================================================
 // Private Variables
 //=============================================================================
 static AUDIO_SINK_Enum_t voiceCallSpkr = AUDIO_SINK_UNDEFINED;
 static AUDDRV_PathID_t telephonyPathID;

 static AudioMode_t stAudioMode = AUDIO_MODE_INVALID;
#if defined(USE_NEW_AUDIO_PARAM)
 static AudioApp_t stAudioApp = AUDIO_APP_VOICE_CALL;
#endif

static int telephony_digital_gain_dB = 12;  //dB
static int telephony_ul_gain_mB = 0;  // 0 mB

//left_channel in stereo, or mono:
static int mixerInputGain[AUDIO_SINK_TOTAL_COUNT] = {0}; // Register value.
static int mixerOutputFineGain[AUDIO_SINK_TOTAL_COUNT] = {0};  // Bit12:0, Output Fine Gain
static int mixerOutputBitSelect[AUDIO_SINK_TOTAL_COUNT] = {0};
static int pmu_gain[AUDIO_SINK_TOTAL_COUNT] = {0};

//for right channel in stereo:
static int mixerInputGain_right[AUDIO_SINK_TOTAL_COUNT] = {0}; // Register value.
static int mixerOutputFineGain_right[AUDIO_SINK_TOTAL_COUNT] = {0};	// Bit12:0, Output Fine Gain
static int mixerOutputBitSelect_right[AUDIO_SINK_TOTAL_COUNT] = {0};
static int pmu_gain_right[AUDIO_SINK_TOTAL_COUNT] = {0};

static unsigned int recordGainL[ AUDIO_SOURCE_TOTAL_COUNT ] = {0};
static unsigned int recordGainR[ AUDIO_SOURCE_TOTAL_COUNT ] = {0};


//=============================================================================
// Private function prototypes
//=============================================================================

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

static CSL_CAPH_DEVICE_e getDeviceFromHWID(AUDIO_HW_ID_t hwID);
static CSL_CAPH_DEVICE_e getDeviceFromMic(AUDIO_SOURCE_Enum_t mic);
static CSL_CAPH_DEVICE_e getDeviceFromSpkr(AUDIO_SINK_Enum_t spkr);
static void powerOnExternalAmp( AUDIO_SINK_Enum_t speaker, ExtSpkrUsage_en_t usage_flag, Boolean use );

extern CSL_CAPH_HWConfig_Table_t HWConfig_Table[MAX_AUDIO_PATH];

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
				AUDIO_SOURCE_Enum_t	mic,
				AUDIO_SINK_Enum_t		speaker
				)
{

	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTelephony::  speaker %d, mic %d \n",
                           speaker, mic );

	if((mic == AUDIO_SOURCE_DIGI1)
	   || (mic == AUDIO_SOURCE_DIGI2)
	   || (mic == AUDIO_SOURCE_DIGI3)
	   || (mic == AUDIO_SOURCE_DIGI4)
	   || (mic == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
	}

	// use gains from sysparm as baseline, adjust gains to achieve user-set volume/gain before call AUDDRV_SetAudioMode( ).
	//	AUDDRV_SetAudioMode( ) reads sysparm and reconcile them with user-set volume/gain, then set to HW, DSP.

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_Init ( mic,
			speaker,
		       	(void *)&telephonyPathID);
	voiceCallSpkr = speaker;

	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonySpkrMute (dlSink, speaker, FALSE);
	// in case it was muted from last voice call,
	//AUDCTRL_SetTelephonyMicMute (ulSrc, mic, FALSE);

	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );

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
				AUDIO_SOURCE_Enum_t	mic,
				AUDIO_SINK_Enum_t		speaker
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableTelephony \n" );

	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, FALSE );

	// The following is the sequence we need to follow
	AUDDRV_Telephony_Deinit ((void*)&telephonyPathID);
	voiceCallSpkr = AUDIO_SINK_UNDEFINED;
	if((mic == AUDIO_SOURCE_DIGI1)
	   || (mic == AUDIO_SOURCE_DIGI2)
	   || (mic == AUDIO_SOURCE_DIGI3)
	   || (mic == AUDIO_SOURCE_DIGI4)
	   || (mic == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}

	telephonyPathID.ulPathID = 0;
	telephonyPathID.ul2PathID = 0;
	telephonyPathID.dlPathID = 0;

	return;
}

//============================================================================
//
// Function Name: AUDCTRL_RateChangeTelephony
//
// Description:   Change Nb / WB according to speech codec type used by mobile network
//
//============================================================================
void AUDCTRL_RateChangeTelephony( UInt32 sampleRate )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_RateChangeTelephony::	stAudioMode %d \n",stAudioMode);

	AUDCTRL_SetAudioMode ( stAudioMode );

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_RateChange(sampleRate);

}

//=============================================================================
// Functions
//=============================================================================
UInt32 AUDCTRL_RateGetTelephony()
{
	return AUDDRV_Telephone_GetSampleRate();
}

void AUDCTRL_RateSetTelephony(UInt32 samplerate)
{
	AUDDRV_Telephony_SetSampleRate(samplerate);
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicSpkr
//
// Description:   Set the micphone and speaker to telephony path, previous micophone
//	and speaker is disabled
//
//============================================================================
void AUDCTRL_SetTelephonyMicSpkr(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDIO_SOURCE_Enum_t	mic,
				AUDIO_SINK_Enum_t		speaker
				)
{
	AUDDRV_PathID_t myTelephonyPathID;
	AUDCTRL_Config_t data;

	memcpy(&myTelephonyPathID, &telephonyPathID, sizeof(AUDDRV_PathID_t));
	memset(&data, 0, sizeof(AUDCTRL_Config_t));

	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicSpkr:: speaker %d, mic %d \n",
                         speaker, mic );

//	AUDCTRL_SaveAudioModeFlag(stAudioMode);

	//driver needs to know mode!
	if(stAudioMode == AUDIO_MODE_USB) AUDCTRL_SetAudioMode ( AUDIO_MODE_HANDSET );
	else AUDCTRL_SetAudioMode ( stAudioMode );

	AUDDRV_Telephony_SelectMicSpkr ( mic, speaker,
			(void*)(&myTelephonyPathID) );

	telephonyPathID.dlPathID = myTelephonyPathID.dlPathID;
	telephonyPathID.ulPathID = myTelephonyPathID.ulPathID;
	telephonyPathID.ul2PathID = myTelephonyPathID.ul2PathID;

	voiceCallSpkr = speaker;
	//need to think about better design!!  do mode switch after EC off, mic mute, etc.
	if((mic == AUDIO_SOURCE_DIGI1)
	   || (mic == AUDIO_SOURCE_DIGI2)
	   || (mic == AUDIO_SOURCE_DIGI3)
	   || (mic == AUDIO_SOURCE_DIGI4)
	   || (mic == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
	}
	else
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}

	OSTASK_Sleep( 100 );  //depending on switch to headset or off of headset, PMU is first off or last on.
	powerOnExternalAmp( speaker, TelephonyUseExtSpkr, TRUE );

	//Load the mic gains from sysparm.
//	  AUDCTRL_LoadMicGain(myTelephonyPathID.ulPathID, mic, TRUE);
	//Load the speaker gains form sysparm.
 //   AUDCTRL_LoadSpkrGain(myTelephonyPathID.dlPathID, speaker, TRUE);


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
				AUDIO_SINK_Enum_t		speaker,
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

			/***
			voice call volume control does not use PMU gain

			pmuGain = (Int16) AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;  //0.25 dB
			SetGainOnExternalAmp_mB(speaker, pmuGain*25, PMU_AUDIO_HS_BOTH);
			***/
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
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_HW_ID_t			dlSink_not_used,
				AUDIO_SINK_Enum_t		spk,
				Boolean 				mute
				)
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrMute: mute = 0x%x\n",	mute);

    if(mute)
      AUDDRV_Telephony_MuteSpkr((AUDIO_SINK_Enum_t) spk, (void*)NULL);
    else
      AUDDRV_Telephony_UnmuteSpkr((AUDIO_SINK_Enum_t) spk, (void*)NULL);
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
				AUDIO_SOURCE_Enum_t	mic,
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
// Function Name: AUDCTRL_SetTelephonyMicMute
//
// Description:   mute/unmute ul of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_HW_ID_t			ulSrc_not_used,
				AUDIO_SOURCE_Enum_t	mic,
				Boolean					mute
				)
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicMute: mute = 0x%x\n",  mute);

    if(mute)
      AUDDRV_Telephony_MuteMic ((AUDIO_SOURCE_Enum_t)mic, (void*)NULL );
    else
      AUDDRV_Telephony_UnmuteMic ((AUDIO_SOURCE_Enum_t)mic, (void*)NULL );
}

//*********************************************************************
//	Function Name: AUDCTRL_InVoiceCall
//	@return  TRUE/FALSE (in/out voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCall( void )
{
	return AUDDRV_InVoiceCall();
}

//*********************************************************************
//	Function Name: AUDCTRL_InVoiceCallWB
//	@return  TRUE/FALSE (in/out WB voice call)
//**********************************************************************/
Boolean AUDCTRL_InVoiceCallWB( void )
{
	return AUDDRV_IsVoiceCallWB(AUDDRV_GetAudioMode());
}

//*********************************************************************
//	Get current (voice call) audio mode
//	@return 	mode		(voice call) audio mode
//**********************************************************************/
AudioMode_t AUDCTRL_GetAudioMode( void )
{
	return AUDDRV_GetAudioMode( );
}

#if defined(USE_NEW_AUDIO_PARAM)
//*********************************************************************
//	Save audio mode before call AUDCTRL_SaveAudioModeFlag( )
//	@param		mode		(voice call) audio mode
//	@param		app 		(voice call) audio app
//	@return 	none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode, AudioApp_t app )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SaveAudioModeFlag: mode = %d, app=%d\n",  mode, app);
	stAudioMode = mode;
	stAudioApp = app;
	AUDDRV_SaveAudioMode( mode, app );
}

//*********************************************************************
//	 Set (voice call) audio mode
//	@param		mode		(voice call) audio mode
//	@param		app 	(voice call) audio app
//	@return 	none
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode, AudioApp_t app)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode: mode = %d, app=%d\n",  mode, app);
	AUDCTRL_SaveAudioModeFlag( mode, app );
	AUDDRV_SetAudioMode( mode, app );

//load PMU gain
}
#else
//*********************************************************************
//	Save audio mode before call AUDCTRL_SaveAudioModeFlag( )
//	@param		mode		(voice call) audio mode
//	@return 	none
//**********************************************************************/
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SaveAudioModeFlag: mode = %d\n",  mode);
	stAudioMode = mode;
	AUDDRV_SaveAudioMode( mode );
}

//*********************************************************************
//	 Set (voice call) audio mode
//	@param		mode		(voice call) audio mode
//	@return 	none
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode: mode = %d\n",  mode);
	AUDCTRL_SaveAudioModeFlag( mode );
	AUDDRV_SetAudioMode( mode, AUDDRV_MIC1|AUDDRV_MIC2|AUDDRV_SPEAKER);

//load PMU gain
}
#endif

//*********************************************************************
//Description:
//	Get audio mode from sink
//Parameters
//	mode -- audio mode
//	sink -- Sink device coresponding to audio mode
//Return	none
//**********************************************************************/
//TBD: should return mode
void AUDCTRL_GetAudioModeBySink(AUDIO_SINK_Enum_t sink, AudioMode_t *mode)
{
    switch(sink)
    {
    case AUDIO_SINK_HANDSET:
          *mode = AUDIO_MODE_HANDSET;
          break;

    case AUDIO_SINK_HEADSET:
          *mode = AUDIO_MODE_HEADSET;
          break;

    case AUDIO_SINK_HANDSFREE:
          *mode = AUDIO_MODE_HANDSFREE;
          break;

    case AUDIO_SINK_BTM:
    case AUDIO_SINK_BTS:
          *mode = AUDIO_MODE_BLUETOOTH;
          break;

    case AUDIO_SINK_LOUDSPK:
          *mode = AUDIO_MODE_SPEAKERPHONE;
          break;

    case AUDIO_SINK_TTY:
          *mode = AUDIO_MODE_TTY;
          break;

    case AUDIO_SINK_HAC:
          *mode = AUDIO_MODE_HAC;
          break;

    case AUDIO_SINK_USB:
          *mode = AUDIO_MODE_USB;
          break;

    case AUDIO_SINK_I2S:
    case AUDIO_SINK_VIBRA:
          *mode = AUDIO_MODE_INVALID;
          break;

    default:
          Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_GetAudioModeBySink(): sink %d is out of range\n", sink);
          break;
    }
}

//*********************************************************************
//Description:
//	Get sink and source device by audio mode
//Parameters
//	mode -- audio mode
//	pMic -- Source device coresponding to audio mode
//	pSpk -- Sink device coresponding to audio mode
//Return   none
//**********************************************************************/
void AUDCTRL_GetVoiceSrcSinkByMode(AudioMode_t mode, AUDIO_SOURCE_Enum_t *pMic, AUDIO_SINK_Enum_t *pSpk)
{
    switch(mode)
    {
    case AUDIO_MODE_HANDSET:
    case AUDIO_MODE_HANDSET_WB:
    case AUDIO_MODE_HAC:
    case AUDIO_MODE_HAC_WB:
        *pMic = AUDIO_SOURCE_ANALOG_MAIN;
        *pSpk = AUDIO_SINK_HANDSET;
        break;

    case AUDIO_MODE_HEADSET:
    case AUDIO_MODE_HEADSET_WB:
    case AUDIO_MODE_TTY:
    case AUDIO_MODE_TTY_WB:
        *pMic = AUDIO_SOURCE_ANALOG_AUX;
        *pSpk = AUDIO_SINK_HEADSET;
        break;

    case AUDIO_MODE_BLUETOOTH:
    case AUDIO_MODE_BLUETOOTH_WB:
        *pMic = AUDIO_SOURCE_BTM;
        *pSpk = AUDIO_SINK_BTM;
        break;

    case	AUDIO_MODE_SPEAKERPHONE:
    case	AUDIO_MODE_SPEAKERPHONE_WB:
        *pMic = AUDIO_SOURCE_ANALOG_MAIN;
        *pSpk = AUDIO_SINK_LOUDSPK;
        break;

    default:
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_GetVoiceSrcSinkByMode() mode %d is out of range\n", mode);
        break;
    }
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
                      AUDIO_SINK_Enum_t    spk,
                      AUDIO_NUM_OF_CHANNEL_t  numCh,
                      AUDIO_SAMPLING_RATE_t   sr,
                      UInt32                  *pPathID
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
    config.source = getDeviceFromHWID(src);
    config.sink =  getDeviceFromSpkr(spk);
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

    if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDIO_SINK_USB)
	{	//USB call
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

	if ((src == AUDIO_HW_MEM || src == AUDIO_HW_I2S_IN) && sink == AUDIO_HW_DSP_VOICE && spk!=AUDIO_SINK_USB)
	{
		config.sink = CSL_CAPH_DEV_DSP_throughMEM; //convert from CSL_CAPH_DEV_EP
	}

	if( sink == AUDIO_HW_USB_OUT || spk == AUDIO_SINK_BTS)
		;
	else
		pathID = csl_caph_hwctrl_EnablePath(config);
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
//============================================================================
//
// Function Name: AUDCTRL_DisablePlay
//
// Description:   disable a playback path
//
//============================================================================
void AUDCTRL_DisablePlay(
				AUDIO_HW_ID_t			src,
				AUDIO_HW_ID_t			sink,
				AUDIO_SINK_Enum_t		spk,
				UInt32					pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID path = 0;
	int i,j;

    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisablePlay: src = 0x%x, sink = 0x%x, spk = 0x%x, pathID %ld.\r\n",
                    src, sink,  spk, pathID);

    if(pathID == 0)
    {
		audio_xassert(0,pathID);
		return;
    }

    if (src == AUDIO_HW_MEM && sink == AUDIO_HW_DSP_VOICE && spk==AUDIO_SINK_USB)
	{	//USB call
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

	if( sink == AUDIO_HW_USB_OUT || spk == AUDIO_SINK_BTS)
		;
	else
	{
		config.pathID = pathID;
		(void) csl_caph_hwctrl_DisablePath(config);
	}

	// Need CSL API to obtain the pathID from the same speaker info.
	// This is to make sure that PMU is not disabled if any other path is using the same speaker

	for (i=0; i<MAX_AUDIO_PATH; i++)
	{
		for(j = 0; j < MAX_SINK_NUM; j++)
		{
			if ((HWConfig_Table[i].sink[j] == getDeviceFromSpkr(spk)) && (HWConfig_Table[i].source == getDeviceFromHWID(src)))
			{
				path = HWConfig_Table[i].pathID;
				break;
			}
		}
	}

    //Disable the PMU for HS/IHF
	if(path)
	{
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisablePlay: pathID %d using the same path still remains, do not turn off PMU.\r\n", path);
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
				AUDIO_SINK_Enum_t		spk,
				AUDIO_GAIN_FORMAT_t     gain_format,
				int						vol_left,
				int						vol_right,
				UInt32					pathID
				)
{
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))
	PMU_AudioGainMapping_t pmuAudioGainMap;
#endif

	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl = CSL_CAPH_SRCM_CH_NONE;

	if( sink == AUDIO_HW_USB_OUT || spk == AUDIO_SINK_BTS)
		return;

	//determine hardware gains

	/***
	MixerIn Gain:	SRC_M1D0_CH**_GAIN_CTRL : SRC_M1D0_CH**_TARGET_GAIN
	sfix<16,2>
	0x7FFF = 6 dB
	0x4000 (2 to the power of 14), =1.0, =0 dB
	0x0000 the input path is essentially switched off

	MixerOutFineGain:	SRC_SPK0_LT_GAIN_CTRL2 : SPK0_LT_FIXED_GAIN
	13-bit interger unsigned
	0x0000, = 0 dB
	0x0001, = (6.02/256) dB attenuation ~ 0.0235 dB
	0x1FFF	max attenuation

	MixerBitSelect:  SRC_SPK0_LT_GAIN_CTRL1 : SPK0_LT_BIT_SELECT
	3-bit unsigned
	***/

	if (gain_format == AUDIO_GAIN_FORMAT_mB)
	{
		switch( spk )
		{
		case AUDIO_SINK_HANDSET:
			pmu_gain[spk] = 0;
			pmu_gain_right[spk] = 0;
			break;

		case AUDIO_SINK_HEADSET:
#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))
#if 1
			/***** fix PMU gain, adjust CAPH gain **/
			pmu_gain[spk] = (int) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_HEADSET ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain[spk] = pmu_gain[spk] * 25;  //mB
			pmuAudioGainMap = map2pmu_hs_gain( pmu_gain[spk] );

			pmu_gain_right[spk] = (int) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_HEADSET ].ext_speaker_pga_r; //Q13p2 dB
			pmu_gain_right[spk] = pmu_gain_right[spk] * 25;  //mB
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			pmuAudioGainMap = map2pmu_hs_gain( vol_left );
			//determine actual PMU gain in mB:
			pmu_gain[spk] = pmuAudioGainMap.gain_mB;
#endif
#endif
			break;

		case AUDIO_SINK_TTY:
		//case AUDIO_CHNL_HEADPHONE_NO_MIC:

#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))
#if 1
			/***** fix PMU gain, adjust CAPH gain **/
			pmu_gain[spk] = (int) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_TTY ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain[spk] = pmu_gain[spk] * 25;  //mB
			pmuAudioGainMap = map2pmu_hs_gain( pmu_gain[spk] );

			pmu_gain_right[spk] = (int) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_TTY ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain_right[spk] = pmu_gain_right[spk] * 25;  //mB
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			pmuAudioGainMap = map2pmu_hs_gain( vol_left );
			//determine actual PMU gain in mB:
			pmu_gain[spk] = pmuAudioGainMap.gain_mB;
#endif
#endif

			break;

		case AUDIO_SINK_LOUDSPK:
#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))
#if 1
			/***** fixed PMU gain, adjust CAPH gain **/
			pmu_gain[spk] = (int) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_SPEAKERPHONE ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain[spk] = pmu_gain[spk] * 25;  //mB
			pmuAudioGainMap = map2pmu_ihf_gain( pmu_gain[spk] );

			pmu_gain_right[spk] = (int) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_SPEAKERPHONE ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain_right[spk] = pmu_gain_right[spk] * 25;  //mB
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			pmuAudioGainMap = map2pmu_ihf_gain( vol_left );
			pmu_gain[spk] = pmuAudioGainMap.gain_mB;
#endif
#endif

			break;

		default:
			break;
		}


		//set CAPH gain
		vol_left = vol_left - pmu_gain[spk];
		mixerInputGain[spk] = 0; //0 dB

		if( vol_left >= 4214 )
		{
			mixerOutputBitSelect[spk] = 7;
			mixerOutputFineGain[spk] = 0;
		}
		else if( vol_left > 0 )  //0~4213
		{
			mixerOutputBitSelect[spk] = vol_left / 602;
			mixerOutputBitSelect[spk] += 1; //since fine-gain is only an attenuation, round up to the next bit shift
			mixerOutputFineGain[spk] = vol_left - (mixerOutputBitSelect[spk]*602);  //put in attenuation, negative number.
		}
		else if( vol_left == 0 )
		{
			mixerOutputBitSelect[spk] = 0;
			mixerOutputFineGain[spk] = 0;
		}
		else //if(vol_left < 0)
		{
			mixerOutputBitSelect[spk] = 0;
			mixerOutputFineGain[spk] = vol_left; //put in attenuation, negative number.
		}

		vol_right = vol_right - pmu_gain_right[spk];
		mixerInputGain_right[spk] = 0; //0 dB

		if( vol_right >= 4214 )
		{
			mixerOutputBitSelect_right[spk] = 7;
			mixerOutputFineGain_right[spk] = 0;
		}
		else if( vol_right > 0 )  //0~4213
		{
			mixerOutputBitSelect_right[spk] = vol_right / 602;
			mixerOutputBitSelect_right[spk] += 1; //since fine-gain is only an attenuation, round up to the next bit shift
			mixerOutputFineGain_right[spk] = vol_right - (mixerOutputBitSelect_right[spk]*602);  //put in attenuation, negative number.
		}
		else if( vol_right == 0 )
		{
			mixerOutputBitSelect_right[spk] = 0;
			mixerOutputFineGain_right[spk] = 0;
		}
		else //if(vol__right < 0)
		{
			mixerOutputBitSelect_right[spk] = 0;
			mixerOutputFineGain_right[spk] = vol_right; //put in attenuation, negative number.
		}

	}
	else
	{
		return;
	}

	//determine which mixer output to apply the gains to

    Log_DebugPrintf(LOGID_AUDIO,
		"AUDCTRL_SetPlayVolume: pmu_gain %d, mixerInputGain = 0x%x, mixerOutputFineGain = 0x%x, mixerOutputBitSelect %d\n",
		pmu_gain[spk], mixerInputGain[spk], mixerOutputFineGain[spk], mixerOutputBitSelect[spk]);

	Log_DebugPrintf(LOGID_AUDIO,
		"AUDCTRL_SetPlayVolume: sink = 0x%x, spk = 0x%x, gain_format %d, vol_left = 0x%x(%d) vol_right 0x%x(%d)\n",
		sink, spk, gain_format, vol_left, vol_left, vol_right, vol_right);

    speaker = getDeviceFromSpkr(spk);

	Log_DebugPrintf(LOGID_AUDIO,
		"AUDCTRL_SetPlayVolume: pathID %d\n",
		pathID);

	if (pathID != 0)
    {
		path = &HWConfig_Table[pathID-1];
        outChnl = path->srcmRoute[0].outChnl;
    }
    else
    {
       	if (speaker == CSL_CAPH_DEV_EP)
       	{
			outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
       	}
		else
    	if (speaker == CSL_CAPH_DEV_IHF)
    	{
		    outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
			//for the case of Stereo_IHF
			outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R | CSL_CAPH_SRCM_STEREO_CH2_L);
    	}
	    else
	    if (speaker == CSL_CAPH_DEV_HS)
	    {
		    //outChnl = (CSL_CAPH_SRCM_STEREO_CH1_L | CSL_CAPH_SRCM_STEREO_CH1_R);
			outChnl = CSL_CAPH_SRCM_STEREO_CH1;
    	}
    }

	//determine which which mixer input to apply the gains to

    if (pathID != 0)
    {
    	//is the inChnl stereo two channels?
		csl_caph_srcmixer_set_mix_in_gain( path->srcmRoute[0].inChnl,
                                    outChnl,
                                    mixerInputGain[spk],
                                    mixerInputGain[spk]);
    }
    else
    {
    	csl_caph_srcmixer_set_mix_all_in_gain(outChnl, mixerInputGain[spk], mixerInputGain[spk]);
    }
			/***
            //Save the mixer gain information.
            //So that it can be picked up by the
            //next call of csl_caph_hwctrl_EnablePath().
            //This is to overcome the situation in music playback that
            //_SetHWGain() is called before Render Driver calls
            //_EnablePath()
            mixGain.mixInGainL = outGain.mixerInputGain;
            mixGain.mixInGainR = outGain.mixerInputGain;
            if (hw == CSL_CAPH_SRCM_INPUT_GAIN_L)
        	    csl_caph_hwctrl_SetPathRouteConfigMixerInputGainL(pathID, mixGain);
            else if (hw == CSL_CAPH_SRCM_INPUT_GAIN_R)
        	    csl_caph_hwctrl_SetPathRouteConfigMixerInputGainR(pathID, mixGain);
			***/

	csl_caph_srcmixer_set_mix_out_gain( outChnl, mixerOutputFineGain[spk] );
			/****
			LIPING: should save them in audio controller?

            //Save the mixer gain information.
            //So that it can be picked up by the
            //next call of csl_caph_hwctrl_EnablePath().
            //This is to overcome the situation in music playback that
            //_SetHWGain() is called before Render Driver calls
            //_EnablePath()
            mixGain.mixOutGainL = outGain.mixerOutputFineGain&0x1FFF;
            mixGain.mixOutGainR = outGain.mixerOutputFineGain&0x1FFF;
            if ( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_L )
            	csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainL(pathID, mixGain);
            else if ( hw == CSL_CAPH_SRCM_OUTPUT_FINE_GAIN_R )
            	csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainR(pathID, mixGain);


			//I think the following can be deleted?

			//Save the mixer gain information.
			//So that it can be picked up by the
			//next call of csl_caph_hwctrl_EnablePath().
			//This is to overcome the problem that
			//_SetSinkGain() is called before _EnablePath()
			mixGain.mixOutGainL = mixGainL.mixerOutputFineGain&0x1FFF;
			mixGain.mixOutGainR = mixGainR.mixerOutputFineGain&0x1FFF;
			csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainL(
												 pathID,
												 mixGain);
			csl_caph_hwctrl_SetPathRouteConfigMixerOutputFineGainR(
												 pathID,
												 mixGain);
			***/

	csl_caph_srcmixer_set_mix_out_bit_select(outChnl, mixerOutputBitSelect[spk] );
			/***
			//Save the mixer gain information.
			//So that it can be picked up by the
			//next call of csl_caph_hwctrl_EnablePath().
			//This is to overcome the situation in music playback that
			//_SetHWGain() is called before Render Driver calls
			//_EnablePath()
			mixGain.mixOutCoarseGainL = (mixer_out_bitsel & 0x7);
			mixGain.mixOutCoarseGainR = (mixer_out_bitsel & 0x7);
			if (hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_L)
				csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainL(pathID, mixGain);
			else if (hw == CSL_CAPH_SRCM_OUTPUT_COARSE_GAIN_R)
				csl_caph_hwctrl_SetPathRouteConfigMixerOutputCoarseGainR(pathID, mixGain);
			***/

	// Set the gain to the external amplifier
	SetGainOnExternalAmp_mB(spk, pmu_gain[spk], PMU_AUDIO_HS_BOTH);

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
				AUDIO_SINK_Enum_t		spk,
				Boolean					mute,
				UInt32					pathID
				)
{
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetPlayMute: sink = 0x%x,  spk = 0x%x, mute = 0x%x\n",
                    sink, spk, mute);

	if( sink == AUDIO_HW_USB_OUT || spk == AUDIO_SINK_BTS)
		return;

    speaker = getDeviceFromSpkr(spk);

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
// Description:   switch a speaker to a playback path - suggested to use when a single device in use
//
//============================================================================
void AUDCTRL_SwitchPlaySpk(
                AUDIO_HW_ID_t           src,
				AUDIO_HW_ID_t			sink,
				AUDIO_SINK_Enum_t		spk,
				UInt32					pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_DEVICE_e speaker[MAX_SINK_NUM] = {CSL_CAPH_DEV_NONE};
	int i,j,dev_ind=0;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SwitchPlaySpk src = 0x%x, newSink = 0x%x,  newSpk = 0x%x\n",
                    src, sink, spk);
    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }

	//get the current speaker from pathID - need CSL API
	for (i=0; i<MAX_AUDIO_PATH; i++)
	{
		if (HWConfig_Table[i].pathID == pathID)
		{
			for(j = 0; j < MAX_SINK_NUM;j++)
			{
				if(HWConfig_Table[i].sink[j] != CSL_CAPH_DEV_NONE)
				{
					speaker[j] = HWConfig_Table[i].sink[j];
					dev_ind = j;
					break;
				}

			}
		}
		if(!dev_ind)
			break;
	}

	if (!dev_ind)
	{
		if(speaker[dev_ind] == CSL_CAPH_DEV_HS)
			powerOnExternalAmp( AUDIO_SINK_HEADSET, AudioUseExtSpkr, FALSE );
		else if(speaker[dev_ind] == CSL_CAPH_DEV_IHF)
			powerOnExternalAmp( AUDIO_SINK_LOUDSPK, AudioUseExtSpkr, FALSE );
	}

	// add new spk first...
    if (getDeviceFromSpkr(spk) != CSL_CAPH_DEV_NONE)
    {
        config.source = getDeviceFromHWID(src);
        config.sink = getDeviceFromSpkr(spk);
        (void) csl_caph_hwctrl_AddPath(pathID, config);
    }

    // remove current spk
    if (speaker[dev_ind] != CSL_CAPH_DEV_NONE)
    {
        config.source = getDeviceFromHWID(src);
        config.sink = speaker[dev_ind];
        (void) csl_caph_hwctrl_RemovePath(pathID, config);
    }
    if ((spk == AUDIO_SINK_LOUDSPK)||(spk == AUDIO_SINK_HEADSET))
        powerOnExternalAmp( spk, AudioUseExtSpkr, TRUE );
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
				AUDIO_HW_ID_t			sink,
				AUDIO_SINK_Enum_t		spk,
				UInt32					pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_AddPlaySpk: src = 0x%x, newSink = 0x%x,  newSpk = 0x%x\n",
                    src, sink, spk);
    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }
    speaker = getDeviceFromSpkr(spk);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
		//Enable the PMU for HS/IHF.
		if ((spk == AUDIO_SINK_LOUDSPK)||(spk == AUDIO_SINK_HEADSET))
			powerOnExternalAmp( spk, AudioUseExtSpkr, TRUE );

        config.source = getDeviceFromHWID(src);
        config.sink = speaker;
        (void) csl_caph_hwctrl_AddPath(pathID, config);
    }
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
				AUDIO_HW_ID_t			sink,
				AUDIO_SINK_Enum_t		spk,
				UInt32					pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_RemovePlaySpk: src = 0x%x, sink = 0x%x,  spk = 0x%x\n",
                    src, sink, spk);

    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }
    speaker = getDeviceFromSpkr(spk);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
		//Disable the PMU for HS/IHF.
		if ((spk == AUDIO_SINK_LOUDSPK)||(spk == AUDIO_SINK_HEADSET))
			powerOnExternalAmp( spk, AudioUseExtSpkr, FALSE );

        config.source = getDeviceFromHWID(src);
        config.sink = speaker;
        (void) csl_caph_hwctrl_RemovePath(pathID, config);
    }
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
				AUDIO_SOURCE_Enum_t	mic,
				AUDIO_NUM_OF_CHANNEL_t	numCh,
				AUDIO_SAMPLING_RATE_t	sr,
				UInt32					*pPathID)
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
    config.source = getDeviceFromMic(mic);
    config.sink =  getDeviceFromHWID(sink);
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
	*pPathID = pathID;
    //Load the mic gains from sysparm.
    //Can not call the following API here.
    //Because Capture driver really enables the path.
    //AUDCTRL_LoadMicGain(pathID, mic, FALSE);

	//also need to have a table to list the used Mic / Mic's (AUDIO_SOURCE_Enum_t) for each audio mode (audio device).
	// use gains from sysparm as baseline, adjust gains to achieve user-set volume/gain before call AUDDRV_SetAudioMode( ).
	//	AUDDRV_SetAudioMode( ) reads sysparm and reconcile them with user-set volume/gain, then set to HW, DSP.

	Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_EnableRecordMono: path configuration, source = %d, sink = %d, pathID %d.\r\n", config.source, config.sink, pathID);

#if 0
	// in case it was muted from last record
	AUDCTRL_SetRecordMute (src, mic, FALSE);
#endif
	// Enable DSP UL for Voice Call.
	if(config.sink == CSL_CAPH_DEV_DSP)
	{
		AUDDRV_EnableDSPInput( mic, sr );
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
				AUDIO_SOURCE_Enum_t	mic,
				AUDIO_NUM_OF_CHANNEL_t		numCh,
				AUDIO_SAMPLING_RATE_t	sr,
				UInt32					*pPathID
				)
{
	UInt32 pathID;
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x, sr %ld\n",
                    src, sink, mic, sr);

	if((mic == AUDIO_SOURCE_DIGI1)
	   || (mic == AUDIO_SOURCE_DIGI2)
	   || (mic == AUDIO_SOURCE_DIGI3)
	   || (mic == AUDIO_SOURCE_DIGI4)
	   || (mic == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
	}

	if(mic==AUDIO_SOURCE_SPEECH_DIGI)
	{
		/* Not supported - One stream - two paths use case for record. Will be supported with one path itself */
		AUDCTRL_EnableRecordMono(src, sink, AUDIO_SOURCE_DIGI1, AUDIO_CHANNEL_MONO, sr, &pathID);
		AUDCTRL_EnableRecordMono(src, sink, AUDIO_SOURCE_DIGI2, AUDIO_CHANNEL_MONO, sr, NULL);
	} else {
		AUDCTRL_EnableRecordMono(src, sink, mic, numCh, sr, &pathID);
	}
	*pPathID = pathID;
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
				AUDIO_SOURCE_Enum_t	mic,
				UInt32					pathID
				)
{

    CSL_CAPH_HWCTRL_CONFIG_t config;
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisableRecord: src = 0x%x, sink = 0x%x,  mic = 0x%x\n",
                    src, sink, mic);

	if(mic==AUDIO_SOURCE_SPEECH_DIGI)
	{
		/* Not supported - One stream - two paths use case for record. Will be supported with one path itself */
		memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) csl_caph_hwctrl_DisablePath(config);

		if(pathID == 0)
		{
			audio_xassert(0,pathID);
			return;
		}

		config.pathID = pathID;
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_DisableRecord: pathID %d.\r\n", pathID);
		(void) csl_caph_hwctrl_DisablePath(config);
	}
	else {
		memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
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
	}
	if((mic == AUDIO_SOURCE_DIGI1)
	   || (mic == AUDIO_SOURCE_DIGI2)
	   || (mic == AUDIO_SOURCE_DIGI3)
	   || (mic == AUDIO_SOURCE_DIGI4)
	   || (mic == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}
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
				AUDIO_SOURCE_Enum_t	mic,
                AUDIO_GAIN_FORMAT_t     gainFormat,
				UInt32					gainL,
				UInt32					gainR,
				UInt32					pathID
				)
{
	CSL_CAPH_HWConfig_Table_t *path;
	csl_caph_Mic_Gain_t outGain;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordGain: src = 0x%x,  mic = 0x%x, gainL = 0x%lx, gainR = 0x%lx\n",
                    src, mic, gainL, gainR);

	recordGainL[mic] = gainL;
	recordGainR[mic] = gainR;

	if (!pathID) return;
	path = &HWConfig_Table[pathID-1];

	if( gainFormat == AUDIO_GAIN_FORMAT_mB )
	{
		//switch( mic )  why not this. simply see mic. does audio_caph.c pass down correct mic param?
		switch(path->source)
		{
		case AUDIO_SOURCE_ANALOG_MAIN:
		case AUDIO_SOURCE_ANALOG_AUX:
			outGain = csl_caph_map_mB_gain_to_registerVal(MIC_ANALOG_HEADSET, (int)gainL);
			if (path->source == CSL_CAPH_DEV_ANALOG_MIC)
				csl_caph_audioh_setgain_register(AUDDRV_PATH_ANALOGMIC_INPUT, outGain.micPGA, 0);
			else
				csl_caph_audioh_setgain_register(AUDDRV_PATH_HEADSET_INPUT, outGain.micPGA, 0);

			csl_caph_audioh_setgain_register(AUDDRV_PATH_VIN_INPUT_L, outGain.micCICBitSelect, outGain.micCICFineScale);

			break;

		case AUDIO_SOURCE_DIGI1:
			outGain = csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL, (int)gainL);
			csl_caph_audioh_setgain_register(AUDDRV_PATH_VIN_INPUT_L, outGain.micCICBitSelect, outGain.micCICFineScale);
			break;

		case AUDIO_SOURCE_DIGI2:
		case AUDIO_SOURCE_NOISE_CANCEL: //Mic for noise cancellation. Used in Dual mic case.
			outGain = csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL, (int)gainL);
			csl_caph_audioh_setgain_register(AUDDRV_PATH_VIN_INPUT_R, outGain.micCICBitSelect, outGain.micCICFineScale);

			break;

		case AUDIO_SOURCE_SPEECH_DIGI: //Digital Mic1/Mic2 in recording/Normal Quality Voice call.
			outGain = csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL, (int)gainL);
			csl_caph_audioh_setgain_register(AUDDRV_PATH_VIN_INPUT, outGain.micCICBitSelect, outGain.micCICFineScale);
			break;

		case AUDIO_SOURCE_BTM:  //Bluetooth Mono Headset Mic
		case AUDIO_SOURCE_USB:  //USB headset Mic
		case AUDIO_SOURCE_I2S:
			break;

		case AUDIO_SOURCE_DIGI3: //Only for loopback path
			outGain = csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL, (int)gainL);
			csl_caph_audioh_setgain_register(AUDDRV_PATH_NVIN_INPUT_L, outGain.micCICBitSelect, outGain.micCICFineScale);
			break;

		case AUDIO_SOURCE_DIGI4: //Only for loopback path
			outGain = csl_caph_map_mB_gain_to_registerVal(MIC_DIGITAL, (int)gainL);
			csl_caph_audioh_setgain_register(AUDDRV_PATH_NVIN_INPUT_R, outGain.micCICBitSelect, outGain.micCICFineScale);
			break;

		default:
			break;
		}

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
				AUDIO_SOURCE_Enum_t	mic,
				Boolean					mute,
				UInt32					pathID
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMuteMono: src = 0x%x,  mic = 0x%x, mute = 0x%x\n",
                    src, mic, mute);

	if( src == AUDIO_HW_USB_IN)
		return;
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
				AUDIO_SOURCE_Enum_t		mic,
				Boolean					mute,
				UInt32					pathID
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMute: src = 0x%x,  mic = 0x%x, mute = 0x%x\n",
                    src, mic, mute);
    if(pathID == 0)
    {
		audio_xassert(0,pathID);
		return;
    }

	if(mic==AUDIO_SOURCE_SPEECH_DIGI)
	{
		/* Not supported - One stream - two paths use case for record. Will be supported with one path itself */
		AUDCTRL_SetRecordMuteMono(src, AUDIO_SOURCE_DIGI1, mute,pathID);
		AUDCTRL_SetRecordMuteMono(src, AUDIO_SOURCE_DIGI2, mute,pathID);
	} else {
		AUDCTRL_SetRecordMuteMono(src, mic, mute,pathID);
	}

    return;
}

//============================================================================
//
// Function Name: AUDCTRL_AddRecordMic
//
// Description:   add a microphone to a record path
//
//============================================================================
void AUDCTRL_AddRecordMic(
                          AUDIO_HW_ID_t		src,
                          AUDIO_SOURCE_Enum_t	mic
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
                            AUDIO_HW_ID_t	src,
                            AUDIO_SOURCE_Enum_t	mic
                            )
{
	// Nothing to do.
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
                              AUDIO_SOURCE_Enum_t mic,
                              AUDIO_SINK_Enum_t	speaker
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
    CSL_CAPH_PathID pathID;
    AUDCTRL_Config_t data;
    AUDIO_HW_ID_t audPlayHw, audRecHw;
    CSL_CAPH_HWCTRL_CONFIG_t hwCtrlConfig;
	//Int16 tempGain = 0;
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;
	int i,j;

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);

    memset(&hwCtrlConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    audPlayHw = audRecHw = AUDIO_HW_NONE;
    source = sink = CSL_CAPH_DEV_NONE;
    audSpkr = CSL_CAPH_DEV_NONE;
    pathID = 0;
    memset(&data, 0, sizeof(AUDCTRL_Config_t));
    switch (mic)
    {
        case AUDIO_SOURCE_ANALOG_MAIN:
            source = CSL_CAPH_DEV_ANALOG_MIC;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDIO_SOURCE_ANALOG_AUX:
            source = CSL_CAPH_DEV_HS_MIC;
            audRecHw = AUDIO_HW_VOICE_IN;
            break;
        case AUDIO_SOURCE_SPEECH_DIGI:
            source = CSL_CAPH_DEV_DIGI_MIC;
            break;
        case AUDIO_SOURCE_DIGI1:
            source = CSL_CAPH_DEV_DIGI_MIC_L;
            break;
        case AUDIO_SOURCE_DIGI2:
            source = CSL_CAPH_DEV_DIGI_MIC_R;
            break;
        case AUDIO_SOURCE_DIGI3:
            source = CSL_CAPH_DEV_EANC_DIGI_MIC_L;
            break;
        case AUDIO_SOURCE_DIGI4:
            source = CSL_CAPH_DEV_EANC_DIGI_MIC_R;
            break;
        case AUDIO_SOURCE_I2S:
            source = CSL_CAPH_DEV_FM_RADIO;
            audRecHw = AUDIO_HW_I2S_IN;
            break;
        case AUDIO_SOURCE_BTM:
            source = CSL_CAPH_DEV_BT_MIC;
            audRecHw = AUDIO_HW_MONO_BT_IN;
            break;
        default:
            source = CSL_CAPH_DEV_ANALOG_MIC;
            audRecHw = AUDIO_HW_I2S_IN;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
            break;
    }

    switch (speaker)
    {
        case AUDIO_SINK_HANDSET:
            sink = CSL_CAPH_DEV_EP;
            audSpkr = CSL_CAPH_DEV_EP;
            audPlayHw = AUDIO_HW_EARPIECE_OUT;
            audio_mode = AUDIO_MODE_HANDSET;
            break;
        case AUDIO_SINK_HEADSET:
            sink = CSL_CAPH_DEV_HS;
            audSpkr = CSL_CAPH_DEV_HS;
            audPlayHw = AUDIO_HW_HEADSET_OUT;
            audio_mode = AUDIO_MODE_HEADSET;
            break;
        case AUDIO_SINK_LOUDSPK:
            sink = CSL_CAPH_DEV_IHF;
            audSpkr = CSL_CAPH_DEV_IHF;
            audio_mode = AUDIO_MODE_SPEAKERPHONE;
            break;
        case AUDIO_SINK_I2S:
            sink = CSL_CAPH_DEV_FM_TX;
            audPlayHw = AUDIO_HW_I2S_OUT;
            // No audio mode available for this case.
            // for now just use AUDIO_MODE_HANDSFREE
            audio_mode = AUDIO_MODE_HANDSFREE;
            break;
        case AUDIO_SINK_BTM:
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
            AUDCTRL_EnableRecord (audRecHw, sinkTemp, mic, AUDIO_CHANNEL_MONO, 48000,NULL);
            return;
        }
#if 0 //removed this to make fm radio work using xpft script
	    if (source == CSL_CAPH_DEV_FM_RADIO)
	    {
            AUDCTRL_EnableRecord (audRecHw, audPlayHw, mic, AUDIO_CHANNEL_STEREO, 48000);
	        if ((speaker == AUDIO_SINK_LOUDSPK)||(speaker == AUDIO_SINK_HEADSET))
	            powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );
	        return;
	    }
#endif
        //  Microphone pat
    	if((mic == AUDIO_SOURCE_DIGI1)
    	   || (mic == AUDIO_SOURCE_DIGI2)
    	   || (mic == AUDIO_SOURCE_DIGI3)
    	   || (mic == AUDIO_SOURCE_DIGI4)
    	   || (mic == AUDIO_SOURCE_SPEECH_DIGI))
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
        hwCtrlConfig.chnlNum = (speaker == AUDIO_SINK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        hwCtrlConfig.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

		/*************
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
        ********/

        pathID = csl_caph_hwctrl_EnablePath(hwCtrlConfig);

#ifdef HW_SIDETONE_LOOPBACK
        //Enable the sidetone path.
		csl_caph_hwctrl_ConfigSidetoneFilter(coeff);
		csl_caph_hwctrl_SetSidetoneGain(0); // Set sidetone gain to 0dB.
		csl_caph_hwctrl_EnableSidetone(sink);
#endif

		AUDCTRL_SetAudioMode( audio_mode ); //this function also sets all HW gains.

        // Enable Loopback ctrl
	    //Enable PMU for headset/IHF
    	if ((speaker == AUDIO_SINK_LOUDSPK)
    	    ||(speaker == AUDIO_SINK_HEADSET))
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
            AUDCTRL_DisableRecord (audRecHw, AUDIO_HW_EARPIECE_OUT, mic,0);
            return;
        }
#if 0 //removed this to make fm radio work using xpft script
	    if (source == CSL_CAPH_DEV_FM_RADIO)
	    {
            AUDCTRL_DisableRecord (audRecHw, audPlayHw, mic);
	        if ((speaker == AUDIO_SINK_LOUDSPK)||(speaker == AUDIO_SINK_HEADSET))
	            powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );
	        return;
	    }
#endif
    	if((mic == AUDIO_SOURCE_DIGI1)
    	   || (mic == AUDIO_SOURCE_DIGI2)
	       || (mic == AUDIO_SOURCE_DIGI3)
    	   || (mic == AUDIO_SOURCE_DIGI4)
	       || (mic == AUDIO_SOURCE_SPEECH_DIGI))
	    {
			// Enable power to digital microphone
			powerOnDigitalMic(FALSE);
		}

        memset(&hwCtrlConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

		for (i=0; i<MAX_AUDIO_PATH; i++)
		{
			for(j = 0; j < MAX_SINK_NUM; j++)
			{
				if ((HWConfig_Table[i].sink[j] == getDeviceFromSpkr(speaker)) && (HWConfig_Table[i].source == getDeviceFromMic(mic)))
				{
					pathID = HWConfig_Table[i].pathID;
					break;
				}
			}
		}
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
    	if ((speaker == AUDIO_SINK_LOUDSPK)
	        ||(speaker == AUDIO_SINK_HEADSET))
		{
			powerOnExternalAmp( speaker, AudioUseExtSpkr, FALSE );
		}

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


//============================================================================
//
// Function Name: SetGainOnExternalAmp_mB
//
// Description:   Set gain on external amplifier driver.
//
// parameter:
//	left_right is of this enum
//enum {
//		PMU_AUDIO_HS_RIGHT,
//		PMU_AUDIO_HS_LEFT,
//		PMU_AUDIO_HS_BOTH
//};
//
//============================================================================
void SetGainOnExternalAmp_mB(AUDIO_SINK_Enum_t speaker, int gain_mB, int left_right)
{
#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))
	PMU_AudioGainMapping_t gain_map;

	switch(speaker)
	{
		case AUDIO_SINK_HEADSET:
		case AUDIO_SINK_TTY:
			gain_map = map2pmu_hs_gain( gain_mB );
			Log_DebugPrintf(LOGID_AUDIO,
						"SetGainOnExternalAmp_mB, hs gain_mB=%d, pmu_gain_enum=%d \n",
						gain_mB, gain_map.PMU_gain_enum );
			AUDIO_PMU_HS_SET_GAIN( left_right, gain_map.PMU_gain_enum );
			break;

		case AUDIO_SINK_LOUDSPK:
			gain_map = map2pmu_ihf_gain( gain_mB );
			Log_DebugPrintf(LOGID_AUDIO,
						"SetGainOnExternalAmp_mB, spk gain_mB=%d, pmu_gain_enum=%d \n",
						gain_mB, gain_map.PMU_gain_enum );
			AUDIO_PMU_IHF_SET_GAIN( gain_map.PMU_gain_enum );
			break;

		default:
			break;
	}

#endif
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
#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))

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
// Function Name: getDeviceFromHWID
//
// Description:   convert audio controller HW ID enum to auddrv device enum
//
//============================================================================
static CSL_CAPH_DEVICE_e getDeviceFromHWID(AUDIO_HW_ID_t hwID)
{
	//Log_DebugPrintf(LOGID_AUDIO,"getDeviceFromHWID: hwID = 0x%x\n", hwID);
	return HWID_Mapping_Table[hwID].dev;
}


//============================================================================
//
// Function Name: getDeviceFromMic
//
// Description:   convert audio controller Mic enum to auddrv device enum
//
//============================================================================
static CSL_CAPH_DEVICE_e getDeviceFromMic(AUDIO_SOURCE_Enum_t mic)
{
	Log_DebugPrintf(LOGID_AUDIO,"getDeviceFromMic: hwID = 0x%x\n", mic);
	return MIC_Mapping_Table[mic].dev;
}


//============================================================================
//
// Function Name: getDeviceFromSpkr
//
// Description:   convert audio controller Spkr enum to auddrv device enum
//
//============================================================================
static CSL_CAPH_DEVICE_e getDeviceFromSpkr(AUDIO_SINK_Enum_t spkr)
{
	//Log_DebugPrintf(LOGID_AUDIO,"getDeviceFromSpkr: hwID = 0x%x\n", spkr);
	return SPKR_Mapping_Table[spkr].dev;
}


//============================================================================
//
// Function Name: powerOnExternalAmp
//
// Description:   call external amplifier driver
//
//============================================================================
static void powerOnExternalAmp(
                            AUDIO_SINK_Enum_t speaker,
                            ExtSpkrUsage_en_t usage_flag,
                            Boolean use
                            )
{
//check for current baseband_use_speaker: OR of voice_spkr, audio_spkr, poly_speaker, and second_speaker
//
//ext_use_speaker could be external FM radio, etc.
//baseband and ext not use amp, can power it off.
// PMU driver needs to know AUDIO_CHNL_HEADPHONE type, so call it from here.
//AUDIO_SINK_Enum_t should be moved to public and let PMU driver includes it.
//and rename it AUD_SPEAKER_t

#if (defined(CONFIG_BCM59055_AUDIO)||defined(CONFIG_BCMPMU_AUDIO))
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
	if (speaker != AUDIO_SINK_HEADSET && speaker != AUDIO_SINK_TTY && speaker != AUDIO_SINK_LOUDSPK && (!IHF_IsOn && !HS_IsOn))
	{
		return;
	}

	if (use == TRUE)
		AUDIO_PMU_INIT();	//enable the audio PLL before power ON


	switch(speaker)
	{
		case AUDIO_SINK_HEADSET:
		case AUDIO_SINK_TTY:
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

		case AUDIO_SINK_LOUDSPK:
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

			AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE),
			AUDIO_PMU_HS_POWER(FALSE);
			OSTASK_Sleep(20);
		}
		HS_IsOn = FALSE;
	}
	else
	{
		int hs_gain = 0;
		Log_DebugPrintf(LOGID_AUDIO,"powerOnExternalAmp (HS on), telephonyUseHS = %d, audioUseHS= %d\n", telephonyUseHS, audioUseHS);

		if ( HS_IsOn != TRUE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power ON pmu HS amp, gain %d\n", hs_gain);
			AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE),
			AUDIO_PMU_HS_POWER(TRUE);
			OSTASK_Sleep(75);

		}

		//the ext_speaker_pga_l is in q13.2 format
		hs_gain = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
		SetGainOnExternalAmp_mB( AUDIO_SINK_HEADSET, hs_gain, PMU_AUDIO_HS_LEFT);

		hs_gain = AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_r;
		SetGainOnExternalAmp_mB( AUDIO_SINK_HEADSET, hs_gain, PMU_AUDIO_HS_RIGHT);

		HS_IsOn = TRUE;
	}

	if ((telephonyUseIHF==FALSE) && (audioUseIHF==FALSE))
	{
		if ( IHF_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu IHF amp\n");
			AUDIO_PMU_IHF_SET_GAIN(PMU_IHFGAIN_MUTE),
			AUDIO_PMU_IHF_POWER(FALSE);
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
		SetGainOnExternalAmp_mB( AUDIO_SINK_LOUDSPK, ihf_gain, PMU_AUDIO_HS_BOTH);

		IHF_IsOn = TRUE;
	}

	if ( IHF_IsOn==FALSE && HS_IsOn==FALSE )
		AUDIO_PMU_DEINIT();    //disable the audio PLL after power OFF

#endif

}

