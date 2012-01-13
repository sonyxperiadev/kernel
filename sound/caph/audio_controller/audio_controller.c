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
*
* @brief  manage audio mode and audio gains over CAPH driver and external audio driver.
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
#include "csl_audio_render.h"
#include "csl_audio_capture.h"

#include "audio_vdriver.h"
#include "audio_controller.h"
#include "log.h"

#ifdef CONFIG_DIGI_MIC
#ifdef CONFIG_BCMPMU_AUDIO
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

typedef struct
{
    AUDIO_SINK_Enum_t spkr;
    CSL_CAPH_DEVICE_e dev;
} AUDCTRL_SPKR_Mapping_t;

//must match AUDIO_SINK_Enum_t
static AUDCTRL_SPKR_Mapping_t SPKR_Mapping_Table[AUDIO_SINK_TOTAL_COUNT] =
{
	//sink ino					// Device ID
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
	{AUDIO_SINK_VALID_TOTAL,	CSL_CAPH_DEV_NONE},
	{AUDIO_SINK_MEM,			CSL_CAPH_DEV_MEMORY},
	{AUDIO_SINK_DSP,			CSL_CAPH_DEV_DSP},
	{AUDIO_SINK_UNDEFINED,		CSL_CAPH_DEV_NONE}
};

typedef struct
{
    AUDIO_SINK_Enum_t spkr;
    AUDIO_SINK_Enum_t auddrv_spkr;
} AUDCTRL_DRVSPKR_Mapping_t;

typedef struct
{
    AUDIO_SOURCE_Enum_t mic;
    CSL_CAPH_DEVICE_e dev;
} AUDIO_SOURCE_Mapping_t;

//must match AUDIO_SOURCE_Enum_t
static AUDIO_SOURCE_Mapping_t MIC_Mapping_Table[AUDIO_SOURCE_TOTAL_COUNT] =
{
	//source info							// Device ID
	{AUDIO_SOURCE_UNDEFINED,		    CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_ANALOG_MAIN,		   	CSL_CAPH_DEV_ANALOG_MIC},
	{AUDIO_SOURCE_ANALOG_AUX,		    CSL_CAPH_DEV_HS_MIC},
	{AUDIO_SOURCE_DIGI1,     			CSL_CAPH_DEV_DIGI_MIC_L},
	{AUDIO_SOURCE_DIGI2,		        CSL_CAPH_DEV_DIGI_MIC_R},
	{AUDIO_SOURCE_DIGI3,				CSL_CAPH_DEV_EANC_DIGI_MIC_L},
	{AUDIO_SOURCE_DIGI4,				CSL_CAPH_DEV_EANC_DIGI_MIC_R},
	{AUDIO_SOURCE_MIC_ARRAY1,		    CSL_CAPH_DEV_DIGI_MIC},
	{AUDIO_SOURCE_MIC_ARRAY2,		    CSL_CAPH_DEV_EANC_DIGI_MIC},
	{AUDIO_SOURCE_BTM,		        	CSL_CAPH_DEV_BT_MIC},
	{AUDIO_SOURCE_USB,       			CSL_CAPH_DEV_MEMORY},
	{AUDIO_SOURCE_I2S,		        	CSL_CAPH_DEV_FM_RADIO},
	{AUDIO_SOURCE_RESERVED1,			CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_RESERVED2,			CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_VALID_TOTAL,			CSL_CAPH_DEV_NONE},
	{AUDIO_SOURCE_SPEECH_DIGI,       	CSL_CAPH_DEV_DIGI_MIC},
	{AUDIO_SOURCE_MEM,		    		CSL_CAPH_DEV_MEMORY},
	{AUDIO_SOURCE_DSP,		    		CSL_CAPH_DEV_DSP}
};

 //=============================================================================
 // Private Variables
 //=============================================================================
static AUDIO_SINK_Enum_t voiceCallSpkr = AUDIO_SINK_UNDEFINED;
static AUDIO_SOURCE_Enum_t voiceCallMic = AUDIO_SOURCE_UNDEFINED;

static int telephony_dl_gain_dB = 0;
static int telephony_ul_gain_dB = 0;

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

//static unsigned int recordGainL[ AUDIO_SOURCE_TOTAL_COUNT ] = {0};
//static unsigned int recordGainR[ AUDIO_SOURCE_TOTAL_COUNT ] = {0};

static int path_user_set_gainL[MAX_AUDIO_PATH]={0};
static int path_user_set_gainR[MAX_AUDIO_PATH]={0};
static AUDIO_GAIN_FORMAT_t path_user_set_gainFormat[MAX_AUDIO_PATH]={AUDIO_GAIN_FORMAT_INVALID};

static Boolean fmPlayStarted = FALSE;
/* pathID of the playback path */
static unsigned int playbackPathID = 0;
/* pathID of the recording path */
static unsigned int recordPathID = 0;
//=============================================================================
// Private function prototypes
//=============================================================================
#ifdef CONFIG_BCM_MODEM
extern SysAudioParm_t* AUDIO_GetParmAccessPtr(void);
#else
extern AudioSysParm_t* AUDIO_GetParmAccessPtr(void);
#endif

static void powerOnExternalAmp( AUDIO_SINK_Enum_t speaker, ExtSpkrUsage_en_t usage_flag, Boolean use );

extern CSL_CAPH_HWConfig_Table_t HWConfig_Table[MAX_AUDIO_PATH];

//=============================================================================
// Functions
//=============================================================================

/****************************************************************************
*
*  Function Name: AUDCTRL_GetModeBySpeaker
*
*  Description: Get the mode via speaker
*
****************************************************************************/
static AudioMode_t  AUDCTRL_GetModeBySpeaker(CSL_CAPH_DEVICE_e speaker)
{
	AudioMode_t mode=AUDIO_MODE_HANDSET;

	switch(speaker)
	{
	case CSL_CAPH_DEV_EP:
		mode = AUDIO_MODE_HANDSET;
		break;
	case CSL_CAPH_DEV_HS:
		mode = AUDIO_MODE_HEADSET;
		break;
	case CSL_CAPH_DEV_IHF:
		mode = AUDIO_MODE_SPEAKERPHONE;
		break;
	case CSL_CAPH_DEV_BT_SPKR:
		mode = AUDIO_MODE_BLUETOOTH;
		break;
	case CSL_CAPH_DEV_FM_TX:
		mode = AUDIO_MODE_RESERVE;
		break;
	default:
		break;
	}
	return mode;
}

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

    //access sysparm here will cause system panic. sysparm is not initialzed when this fucniton is called.
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
				AUDIO_SOURCE_Enum_t  source,
				AUDIO_SINK_Enum_t    sink
				)
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_EnableTelephony::  sink %d, mic %d \n",
                           sink, source );

	if( AUDDRV_InVoiceCall() )
	{      //already in voice call
		if( (voiceCallSpkr != sink) || (voiceCallMic != source) )
			AUDCTRL_SetTelephonyMicSpkr( source, sink );

		return;
	}

	if((source == AUDIO_SOURCE_DIGI1)
	   || (source == AUDIO_SOURCE_DIGI2)
	   || (source == AUDIO_SOURCE_DIGI3)
	   || (source == AUDIO_SOURCE_DIGI4)
	   || (source == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Enable PMU power to digital microphone
		powerOnDigitalMic(TRUE);
	}

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_Init ( source, sink );
	voiceCallSpkr = sink;
	voiceCallMic = source;

	powerOnExternalAmp( sink, TelephonyUseExtSpkr, TRUE );

	return;
}
//============================================================================
//
// Function Name: AUDCTRL_DisableTelephony
//
// Description:   disable telephony path, both dl and ul
//
//============================================================================
void AUDCTRL_DisableTelephony( void )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_DisableTelephony \n" );

	// continues speech playback when end the phone call.
	// continues speech recording when end the phone call.
//	if ( FALSE==vopath_enabled && FALSE==DspVoiceIfActive_DL() && FALSE==vipath_enabled )  //VO path is off and DSP IF is off, VI path is off
//	{

	powerOnExternalAmp( voiceCallSpkr, TelephonyUseExtSpkr, FALSE );

	// The following is the sequence we need to follow
	AUDDRV_Telephony_Deinit ( );

	if((voiceCallMic == AUDIO_SOURCE_DIGI1)
	   || (voiceCallMic == AUDIO_SOURCE_DIGI2)
	   || (voiceCallMic == AUDIO_SOURCE_DIGI3)
	   || (voiceCallMic == AUDIO_SOURCE_DIGI4)
	   || (voiceCallMic == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}

	voiceCallSpkr = AUDIO_SINK_UNDEFINED;
	voiceCallMic = AUDIO_SOURCE_UNDEFINED;

	return;
}

//============================================================================
//
// Function Name: AUDCTRL_RateChangeTelephony
//
// Description:   Change Nb / WB according to speech codec type used by mobile network
//
//============================================================================
void AUDCTRL_Telephony_RateChange( unsigned int sample_rate )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_Telephony_RateChange::sample_rate %d \n",sample_rate);

	// This function follows the sequence and enables DSP audio, HW input path and output path.
	AUDDRV_Telephony_RateChange(sample_rate);  //need to load new audio mode
}

/**
*  @brief  the rate change request function called by CAPI message listener
*
*  @param  codecID		(in) voice call speech codec ID
*
*  @return none
*
****************************************************************************/
void AUDCTRL_Telephony_RequestRateChange(UInt8 codecID)
{
    // 0x0A as per 3GPP 26.103 Sec 6.3 indicates AMR WB  AUDIO_ID_CALL16k
    // 0x06 indicates AMR NB
	if ( codecID == 0x0A ) // AMR-WB
	{
		AUDCTRL_Telephony_RateChange( AUDIO_SAMPLING_RATE_16000 );
	}
	else
	if ( codecID == 0x06 ) // AMR-WB// AMR-NB
	{
		AUDCTRL_Telephony_RateChange( AUDIO_SAMPLING_RATE_8000 );
	}
}

/*=============================================================================
*
* Function Name: AUDCTRL_EC
*
* Description:   DSP Echo cancellation ON/OFF
*
*=============================================================================
*/
void AUDCTRL_EC(Boolean enable, UInt32 arg)
{
    AUDDRV_EC(enable, arg);
}

/*=============================================================================
*
* Function Name: AUDCTRL_NS
*
* Description:   DSP Noise Suppression ON/OFF
*
*=============================================================================
*/
void AUDCTRL_NS(Boolean enable)
{
    AUDDRV_NS(enable);
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicSpkr
//
// Description:   Set the micphone and speaker to telephony path, previous
//              micophone and speaker are disabled.
//      actual audio mode is determined by sink, network speech coder sample
//      rate and BT headset support of WB.
//
//============================================================================
void AUDCTRL_SetTelephonyMicSpkr(
            AUDIO_SOURCE_Enum_t  source,
            AUDIO_SINK_Enum_t	 sink
            )
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicSpkr:: sink %d, source %d \n",
	             sink, source );

    if( source==AUDIO_SOURCE_USB || sink==AUDIO_SINK_USB )
	return;

    if( AUDDRV_InVoiceCall( ) == FALSE )
    {
      voiceCallSpkr = sink;
      voiceCallMic = source;
      //when phone is idle, if PCG changed audio mode, here need to save it on AP side.
#if defined(USE_NEW_AUDIO_PARAM)
      AUDCTRL_SaveAudioModeFlag( AUDDRV_GetAudioModeBySink(sink), AUDCTRL_GetAudioApp() );
#else
      AUDCTRL_SaveAudioModeFlag( AUDDRV_GetAudioModeBySink(sink) );
#endif

	  //if PCG changed audio mode when phone is idle, here need to pass audio mode to CP.
	  //so that parameter read by AT*MAUDTUNE=2 will be for the new audio mode.

#if defined(USE_NEW_AUDIO_PARAM)
	  audio_control_generic( AUDDRV_CPCMD_PassAudioMode, (UInt32)AUDDRV_GetAudioModeBySink(sink), (UInt32)AUDCTRL_GetAudioApp(), 0, 0, 0 );
#else
	  audio_control_generic( AUDDRV_CPCMD_PassAudioMode, (UInt32)AUDDRV_GetAudioModeBySink(sink), 0, 0, 0, 0 );
#endif
      return;
    }

    if(voiceCallMic==source && voiceCallSpkr==sink)
      return;

    if(voiceCallSpkr!=sink)
      powerOnExternalAmp( voiceCallSpkr, TelephonyUseExtSpkr, FALSE );

    if(voiceCallMic!=source)
    {
	if((voiceCallMic == AUDIO_SOURCE_DIGI1)
	|| (voiceCallMic == AUDIO_SOURCE_DIGI2)
	|| (voiceCallMic == AUDIO_SOURCE_DIGI3)
	|| (voiceCallMic == AUDIO_SOURCE_DIGI4)
	|| (voiceCallMic == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}
    }

    if(voiceCallMic!=source)
    {
      if((source == AUDIO_SOURCE_DIGI1)
	|| (source == AUDIO_SOURCE_DIGI2)
	|| (source == AUDIO_SOURCE_DIGI3)
	|| (source == AUDIO_SOURCE_DIGI4)
	|| (source == AUDIO_SOURCE_SPEECH_DIGI))
      {
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
      }
    }

    AUDDRV_Telephony_SelectMicSpkr ( source, sink );

    if(voiceCallSpkr!=sink)
      powerOnExternalAmp( sink, TelephonyUseExtSpkr, TRUE );

    voiceCallSpkr = sink;
    voiceCallMic = source;
}


//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrVolume(
				AUDIO_SINK_Enum_t		speaker,
				int						volume,
				AUDIO_GAIN_FORMAT_t		gain_format
				)
{
    //int pmuGain = 0;
    //pmuGain = (short) AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrVolume: volume = %d \n", volume );

    if (gain_format == AUDIO_GAIN_FORMAT_mB)
    {
      //volume is in range of -3600 mB ~ 0 mB from the API
      telephony_dl_gain_dB = (volume / 100);

      if ( telephony_dl_gain_dB > 0 )
         telephony_dl_gain_dB = 0;

      if ( telephony_dl_gain_dB < -(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max) )
         telephony_dl_gain_dB = -(AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max);

      AUDDRV_SetTelephonySpkrVolume( speaker, telephony_dl_gain_dB*100, gain_format );

      /***
      voice call volume control does not use PMU gain

      pmuGain = (short) AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].ext_speaker_pga_l;  //0.25 dB
      SetGainOnExternalAmp_mB(speaker, pmuGain*25, PMU_AUDIO_HS_BOTH);
      ***/
      }
    else
    if (gain_format == AUDIO_GAIN_FORMAT_DSP_VOICE_VOL_GAIN)
    {
      if(volume>14)  volume=14;  //15 entries: 0 ~ 14.

      telephony_dl_gain_dB = AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].dsp_voice_vol_tbl[volume];
      //values in table are in range of 0 ~ 36 dB.
      //shift to range of -36 ~ 0 dB in DSP
      telephony_dl_gain_dB -= AUDIO_GetParmAccessPtr()[AUDDRV_GetAudioMode()].voice_volume_max;

      AUDDRV_SetTelephonySpkrVolume( speaker, telephony_dl_gain_dB*100, AUDIO_GAIN_FORMAT_mB );
    }
}

//============================================================================
//
// Function Name: AUDCTRL_GetTelephonySpkrVolume
//
// Description:   Set dl volume of telephony path
//
//============================================================================
int AUDCTRL_GetTelephonySpkrVolume( AUDIO_GAIN_FORMAT_t gain_format )
{
    if (gain_format == AUDIO_GAIN_FORMAT_mB)
    {
      return (telephony_dl_gain_dB*100);
    }

    return telephony_dl_gain_dB;
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonySpkrMute
//
// Description:   mute/unmute the dl of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonySpkrMute(
				AUDIO_SINK_Enum_t		spk,
				Boolean 				mute
				)
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonySpkrMute: mute = 0x%x\n",	mute);

    if(mute)
      AUDDRV_Telephony_MuteSpkr( spk );
    else
      AUDDRV_Telephony_UnmuteSpkr( spk );
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicGain
//
// Description:   Set ul gain of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicGain(
				AUDIO_SOURCE_Enum_t  mic,
				Int16                gain,
				AUDIO_GAIN_FORMAT_t  gain_format
				)
{
    if (gain_format == AUDIO_GAIN_FORMAT_mB)
    {
      AUDDRV_SetTelephonyMicGain( mic, gain, gain_format);
      telephony_ul_gain_dB = gain/100;
    }
}

//============================================================================
//
// Function Name: AUDCTRL_SetTelephonyMicMute
//
// Description:   mute/unmute ul of telephony path
//
//============================================================================
void AUDCTRL_SetTelephonyMicMute(
				AUDIO_SOURCE_Enum_t		mic,
				Boolean					mute
				)
{
    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetTelephonyMicMute: mute = 0x%x\n",  mute);

    if(mute)
      AUDDRV_Telephony_MuteMic ( mic );
    else
      AUDDRV_Telephony_UnmuteMic ( mic );
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

//=============================================================================
//
// Function Name: AUDCTRL_GetAudioApp
//
// Description:   get audio application.
//
//=============================================================================

AudioApp_t AUDCTRL_GetAudioApp( void )
{
	return AUDDRV_GetAudioApp();
}

//=============================================================================
//
// Function Name: AUDCTRL_SetAudioApp
//
// Description:   set audio application.
//			    should be called by upper layer
//=============================================================================
void AUDCTRL_SetAudioApp( AudioApp_t audio_app )
{
	return AUDDRV_SetAudioApp(audio_app);
}
//*********************************************************************
//	Save audio mode before call AUDCTRL_SaveAudioModeFlag( )
//	@param		mode		(voice call) audio mode
//	@param		app 		(voice call) audio app
//	@return 	none
//**********************************************************************/
/* reused this api from athena, may split it later */
void AUDCTRL_SaveAudioModeFlag( AudioMode_t mode, AudioApp_t audio_app )
{
	Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SaveAudioModeFlag: mode = %d audio_app = %d\n",  mode, audio_app);
	AUDDRV_SaveAudioMode( mode, audio_app );
}

//*********************************************************************
//	 Set (voice call) audio mode
//	@param		mode		(voice call) audio mode
//	@param		app 	(voice call) audio app
//	@return 	none
//
//      actual audio mode is determined by mode, network speech coder sample
//      rate and BT headset support of WB.
//**********************************************************************/
/* reused this api from athena, may split it later */
void AUDCTRL_SetAudioMode( AudioMode_t mode, AudioApp_t audio_app )
{
    AUDIO_SOURCE_Enum_t mic;
    AUDIO_SINK_Enum_t spk;
    Boolean bClk = csl_caph_QueryHWClock();

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode: mode = %d audio_app = %d\n",  mode, audio_app);

	// make sure audio app is the same before return
    if( (mode==AUDDRV_GetAudioMode())&&(audio_app==AUDDRV_GetAudioApp()) )
      return;

    //if ( !AUDDRV_InVoiceCall() )
    //{
    //  AUDDRV_SaveAudioMode( mode );
    //}

	AUDCTRL_SaveAudioModeFlag( mode, audio_app );


    if(!bClk) csl_caph_ControlHWClock(TRUE); //enable clock if it is not enabled.

    AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);

    /* Here may need to consider for other apps like vt and voip etc */
    if((audio_app == AUDIO_APP_VOICE_CALL)
      ||(audio_app == AUDIO_APP_VOICE_CALL_WB))
    {
        if( (voiceCallMic == mic) && (voiceCallSpkr == spk) )
        {
            if( audio_app == AUDIO_APP_VOICE_CALL_WB )
            {
                AUDDRV_Telephony_RateChange( 
                        AUDIO_SAMPLING_RATE_16000 );
            }
            else
            {
                AUDDRV_Telephony_RateChange( 
                        AUDIO_SAMPLING_RATE_8000 );
            }
        }
        else
        {
            if( audio_app == AUDIO_APP_VOICE_CALL_WB )
            {
                AUDDRV_Telephone_SaveSampleRate(
                        AUDIO_SAMPLING_RATE_16000 );
            }
            else
            {
                AUDDRV_Telephone_SaveSampleRate( 
                        AUDIO_SAMPLING_RATE_8000 );
            }

            /* function below checks voiceCallSampleRate */
            AUDCTRL_SetTelephonyMicSpkr( mic, spk );
        }
    }
    else
    if(audio_app == AUDIO_APP_MUSIC)
    {
        /*add code here to switch the mic or speaker when
         *audio mode/app is changed.
         */
    }
     if ( !AUDDRV_InVoiceCall() )
    {
        //for music tuning, if PCG changed audio mode when phone is in idle mode, here need to pass audio mode to CP.
        //so that AT*MAUDTUNE=3 applies parameter change based on the new audio mode on CP side.

        //this command only updates mode in audio_vdriver_caph.c, does not updat mode in audioapi.c.
        // Therefore it is not useful for MP3 audio tuning purpose.
        audio_control_generic( AUDDRV_CPCMD_PassAudioMode, (UInt32)mode, (UInt32)audio_app, 0, 0, 0 );
        //this command updates mode in audioapi.c. It is useful for MP3 audio tuning purpose.
        audio_control_generic( AUDDRV_CPCMD_SetAudioMode, (UInt32)(audio_app*AUDIO_MODE_NUMBER + mode), 0, 0, 0, 0 );
    }

    /*disable clock if it is enabled by this function */
    if(!bClk) csl_caph_ControlHWClock(FALSE); 
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
	AUDDRV_SaveAudioMode( mode );
}

//*********************************************************************
//	 Set (voice call) audio mode
//	@param		mode		(voice call) audio mode
//	@return 	none
//
//      actual audio mode is determined by mode, network speech coder sample
//      rate and BT headset support of WB.
//**********************************************************************/
void AUDCTRL_SetAudioMode( AudioMode_t mode )
{
    AUDIO_SOURCE_Enum_t mic;
    AUDIO_SINK_Enum_t spk;
    Boolean bClk = csl_caph_QueryHWClock();

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode: mode = %d\n",  mode);

    if( mode==AUDDRV_GetAudioMode() )
      return;

    if ( !AUDDRV_InVoiceCall() )
    {
        //for music tuning, if PCG changed audio mode when phone is in idle mode, here need to pass audio mode to CP.
        //so that AT*MAUDTUNE=3 applies parameter change based on the new audio mode on CP side.

        //this command only updates mode in audio_vdriver_caph.c, does not updat mode in audioapi.c.
        // Therefore it is not useful for MP3 audio tuning purpose.
        audio_control_generic( AUDDRV_CPCMD_PassAudioMode, (UInt32)mode, 0, 0, 0, 0 );
        //this command updates mode in audioapi.c. It is useful for MP3 audio tuning purpose.
        audio_control_generic( AUDDRV_CPCMD_SetAudioMode, (UInt32)mode, 0, 0, 0, 0 );
    }

    if(!bClk) csl_caph_ControlHWClock(TRUE); //enable clock if it is not enabled.

    AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);

    if( (voiceCallMic == mic) && (voiceCallSpkr == spk) )
    {
      if( mode>=AUDIO_MODE_NUMBER )
        AUDDRV_Telephony_RateChange( AUDIO_SAMPLING_RATE_16000 );
      else
        AUDDRV_Telephony_RateChange( AUDIO_SAMPLING_RATE_8000 );
    }
    else
    {
      if( mode>=AUDIO_MODE_NUMBER )
        AUDDRV_Telephone_SaveSampleRate( AUDIO_SAMPLING_RATE_16000 );
      else
        AUDDRV_Telephone_SaveSampleRate( AUDIO_SAMPLING_RATE_8000 );

      //the function below checks voiceCallSampleRate
      AUDCTRL_SetTelephonyMicSpkr( mic, spk );
    }

    if(!bClk) csl_caph_ControlHWClock(FALSE); //disable clock if it is enabled by this function.
}
#endif
//*********************************************************************
//	 Set audio mode for music playback
//	@param		mode		audio mode for music playback
//	@return 	none
//
//**********************************************************************/
void AUDCTRL_SetAudioMode_ForMusicPlayback( AudioMode_t mode, unsigned int arg_pathID, Boolean inHWlpbk )
{
    AUDIO_SOURCE_Enum_t mic;
    AUDIO_SINK_Enum_t spk;
    Boolean bClk = csl_caph_QueryHWClock();
	CSL_CAPH_HWConfig_Table_t *path = NULL;

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode_ForMusicPlayback: mode = %d, pathID %d\n",  mode, arg_pathID);

	if(arg_pathID) path = &HWConfig_Table[ arg_pathID - 1 ];
    //if( mode==AUDDRV_GetAudioMode() )
    //  return;

    if ( AUDDRV_InVoiceCall() )
    {
		if(!path) return;  //don't affect voice call audio mode
		if(!path->srcmRoute[0][0].outChnl) return; //if arm2sp does not use HW mixer, no need to set gain
    }

    if(!bClk) csl_caph_ControlHWClock(TRUE); //enable clock if it is not enabled.

    AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);

//set PMU on/off, gain,

//for multicast, need to find the other mode and reconcile on mixer gains. like BT + IHF

    AUDDRV_SetAudioMode_ForMusicPlayback( mode, arg_pathID, inHWlpbk );


    if(!bClk) csl_caph_ControlHWClock(FALSE); //disable clock if it is enabled by this function.
}


//*********************************************************************
//	 Set audio mode for music record
//	@param		mode		audio mode
//	@return 	none
//
//**********************************************************************/
void AUDCTRL_SetAudioMode_ForMusicRecord( AudioMode_t mode, unsigned int arg_pathID )
{
    AUDIO_SOURCE_Enum_t mic;
    AUDIO_SINK_Enum_t spk;
    Boolean bClk = csl_caph_QueryHWClock();

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioMode_ForMusicRecord: mode = %d\n",  mode);

    //if( mode==AUDDRV_GetAudioMode() )
    //  return;

    if ( AUDDRV_InVoiceCall() )
    {
    //FM radio recording should be able to co-exist with voice call
      return;  //don't affect voice call audio mode
    }

    if(!bClk) csl_caph_ControlHWClock(TRUE); //enable clock if it is not enabled.

    AUDCTRL_GetSrcSinkByMode(mode, &mic, &spk);

//no PMU

//for FM recording + voice call, need to fidn separate gains from sysparm and HW paths

	//also need to support audio profile (and/or mode) set from user space code to support multi-profile/app.

    AUDDRV_SetAudioMode_ForMusicRecord( mode, 0 );

    if(!bClk) csl_caph_ControlHWClock(FALSE); //disable clock if it is enabled by this function.
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
void AUDCTRL_GetSrcSinkByMode(AudioMode_t mode, AUDIO_SOURCE_Enum_t *pMic, AUDIO_SINK_Enum_t *pSpk)
{
    switch(mode)
    {
    case AUDIO_MODE_HANDSET:
    case AUDIO_MODE_HAC:
#if !defined(USE_NEW_AUDIO_PARAM)
    case AUDIO_MODE_HANDSET_WB:
    case AUDIO_MODE_HAC_WB:
#endif
        *pMic = AUDIO_SOURCE_ANALOG_MAIN;
        *pSpk = AUDIO_SINK_HANDSET;
        break;

    case AUDIO_MODE_HEADSET:
    case AUDIO_MODE_TTY:
#if !defined(USE_NEW_AUDIO_PARAM)
    case AUDIO_MODE_HEADSET_WB:
    case AUDIO_MODE_TTY_WB:
#endif
        *pMic = AUDIO_SOURCE_ANALOG_AUX;
        *pSpk = AUDIO_SINK_HEADSET;
        break;

    case AUDIO_MODE_BLUETOOTH:
#if !defined(USE_NEW_AUDIO_PARAM)
    case AUDIO_MODE_BLUETOOTH_WB:
#endif
        *pMic = AUDIO_SOURCE_BTM;
        *pSpk = AUDIO_SINK_BTM;
        break;

    case	AUDIO_MODE_SPEAKERPHONE:
#if !defined(USE_NEW_AUDIO_PARAM)
    case	AUDIO_MODE_SPEAKERPHONE_WB:
#endif
        *pMic = AUDIO_SOURCE_ANALOG_MAIN;
        *pSpk = AUDIO_SINK_LOUDSPK;
        break;

    default:
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_GetSrcSinkByMode() mode %d is out of range\n", mode);
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
                      AUDIO_SOURCE_Enum_t     source,
                      AUDIO_SINK_Enum_t	      sink,
                      AUDIO_NUM_OF_CHANNEL_t  numCh,
                      AUDIO_SAMPLING_RATE_t   sr,
                      unsigned int            *pPathID
                      )
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID;
	AudioMode_t mode=AUDIO_MODE_HANDSET;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnablePlay: src = %d, sink = %d \n",
                    source, sink);
    pathID = 0;
    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    // Enable the path. And get path ID.
    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = 0;
    config.source = getDeviceFromSrc(source);
    config.sink =  getDeviceFromSink(sink);
    config.dmaCH = CSL_CAPH_DMA_NONE;
    config.src_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
    config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

    //Enable the PMU for HS/IHF.
    if ((sink == AUDIO_SINK_HEADSET)||(sink == AUDIO_SINK_LOUDSPK))
    {
	//save audio for powerOnExternalAmp() to use.
	mode = AUDDRV_GetAudioModeBySink(sink);
#if defined(USE_NEW_AUDIO_PARAM)
	AUDCTRL_SaveAudioModeFlag(mode, AUDCTRL_GetAudioApp() );
#else
	AUDCTRL_SaveAudioModeFlag(mode);
#endif

	if (source == AUDIO_SOURCE_I2S && AUDDRV_InVoiceCall() == FALSE)
	{
		Log_DebugPrintf(LOGID_AUDIO,
			"AUDCTRL_EnablePlay: FM source src = %d, sink = %d \n",
			source, sink);

		powerOnExternalAmp( sink, FMRadioUseExtSpkr, TRUE );
	}
	else
		powerOnExternalAmp( sink, AudioUseExtSpkr, TRUE );
    }

    if(source == AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	{	//USB call
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

	if((source == AUDIO_SOURCE_MEM || source == AUDIO_SOURCE_I2S) && sink == AUDIO_SINK_DSP)
	{
		config.sink = CSL_CAPH_DEV_DSP_throughMEM;
	}

	if((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB) || sink == AUDIO_SINK_BTS)
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
	// Enable DSP DL
	if(config.source == CSL_CAPH_DEV_DSP || config.sink == CSL_CAPH_DEV_DSP_throughMEM)
	{
		AUDDRV_EnableDSPOutput(sink, sr);
	}

	if (source == AUDIO_SOURCE_I2S && AUDDRV_InVoiceCall() == FALSE)
	{	//to set HW mixer gain for FM
		mode = AUDCTRL_GetModeBySpeaker(config.sink);
		AUDCTRL_SetAudioMode_ForMusicPlayback( mode, pathID, FALSE );

		fmPlayStarted = TRUE;
	
		/*** will test this when FM radio works.
			if( path_user_set_gainFormat[pathID] != AUDIO_GAIN_FORMAT_INVALID )
			{
				AUDCTRL_SetPlayVolume( path->source[0],
						path->sink[0],
						path_user_set_gainL[pathID],
						path_user_set_gainR[pathID],
						path_user_set_gainFormat[pathID],
						audDrv->pathID);
			}
		***/
	}
	if(pPathID) *pPathID = pathID;

    playbackPathID = pathID;
	//Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_EnablePlay: pPathID %x, pathID %d\r\n", *pPathID, pathID);
}
//============================================================================
//
// Function Name: AUDCTRL_DisablePlay
//
// Description:   disable a playback path
//
//============================================================================
void AUDCTRL_DisablePlay(
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID path = 0;
	CSL_CAPH_DEVICE_e src_dev,sink_dev;
	int i,j;

    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisablePlay: src = 0x%x, sink = 0x%x, pathID %d.\r\n",
                    source, sink,  pathID);

    if(pathID == 0)
    {
		audio_xassert(0,pathID);
		return;
    }

	if(source == AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)
	{	//USB call
		config.source = CSL_CAPH_DEV_DSP;
		config.sink = CSL_CAPH_DEV_MEMORY;
	}

	src_dev = getDeviceFromSrc(source);
	sink_dev = getDeviceFromSink(sink);

	// Disable DSP DL
	if(source == AUDIO_SOURCE_DSP || sink == AUDIO_SINK_DSP)
	{
		AUDDRV_DisableDSPOutput();
	}
	if((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB) || sink == AUDIO_SINK_BTS)
		;
	else
	{
		config.pathID = pathID;
		(void) csl_caph_hwctrl_DisablePath(config);
	}

	// Need CSL API to obtain the pathID from the same speaker info.
	// This is to make sure that PMU is not disabled if any other path is using the same speaker

	for (i=0; i< MAX_AUDIO_PATH; i++)
	{
		for(j = 0; j < MAX_SINK_NUM; j++)
		{
			if ((HWConfig_Table[i].sink[j] == sink_dev) && (HWConfig_Table[i].source == src_dev))
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
		if (source == AUDIO_SOURCE_I2S && AUDDRV_InVoiceCall() == FALSE)
		{
			Log_DebugPrintf(LOGID_AUDIO,
	                    "AUDCTRL_DisablePlay: FM source src = %d, sink = %d \n",
	                    source, sink);

			powerOnExternalAmp( sink, FMRadioUseExtSpkr, FALSE );
			fmPlayStarted = FALSE;
		}
		else
		if ((sink == AUDIO_SINK_HEADSET)||(sink == AUDIO_SINK_LOUDSPK))
		{
			powerOnExternalAmp( sink, AudioUseExtSpkr, FALSE );
		}
	}
}

/****************************************************************************
*
*  Function Name: AUDCTRL_StartRender
*
*  Description: Start the data transfer of audio path render
*
****************************************************************************/
Result_t AUDCTRL_StartRender(unsigned int streamID)
{
	AudioMode_t mode=AUDIO_MODE_HANDSET;
	Result_t res;
	CSL_CAPH_Render_Drv_t	*audDrv = NULL;
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	int pathID = 0;

	audDrv = GetRenderDriverByType (streamID);

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDCTRL_StartRender::streamID=0x%x\n", streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

	res = csl_audio_render_start (streamID);

    if(audDrv->pathID)
    	path = &HWConfig_Table[ audDrv->pathID - 1 ];
    else
    	return 0;

	if ( path->status == PATH_OCCUPIED ) mode = AUDCTRL_GetModeBySpeaker(path->sink[0]);

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDCTRL_StartRender::audDrv->pathID=0x%x, path->status=%d, path->sink[0]=%d, mode=%d, mixer in 0x%x out: %d.\n",
		audDrv->pathID, path->status, path->sink[0], mode, path->srcmRoute[0][0].inChnl, path->srcmRoute[0][0].outChnl);

	if(mode == AUDIO_MODE_RESERVE) return RESULT_OK;  //no need to set HW gain for FM TX.
	//also need to support audio profile/mode set from user space code to support multi-profile/app.

	if ( AUDDRV_InVoiceCall() && path->srcmRoute[0][0].outChnl) pathID = audDrv->pathID; //arm2sp may use HW mixer, whose gain should be set
	AUDCTRL_SetAudioMode_ForMusicPlayback( mode, pathID, FALSE );

	//for multi-cast, also use path->sink[1]

	return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: AUDCTRL_StopRender
*
*  Description: Stop the data transfer of audio path render
*
****************************************************************************/
Result_t AUDCTRL_StopRender(unsigned int streamID)
{
	Result_t res;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDCTRL_StopRender::streamID=0x%x\n", streamID);
	res = csl_audio_render_stop(streamID);

	return res;
}


//============================================================================
//
// Function Name: AUDCTRL_SetPlayVolume
//
// Description:   set volume of a playback path
//
//============================================================================
void AUDCTRL_SetPlayVolume(
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				AUDIO_GAIN_FORMAT_t     gain_format,
				int						vol_left,
				int						vol_right,
				unsigned int			pathID
				)
{
    int j = 0, sinkNo = 0;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

#ifdef CONFIG_BCMPMU_AUDIO
	PMU_AudioGainMapping_t pmuAudioGainMap;
#endif

	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl = CSL_CAPH_SRCM_CH_NONE;

	//in idle mode, this command 
	// alsa_amixer cset name=FM-VOL-LEVEL 12,12
	// passes down the pathID==0.  I'm not sure the pathID is correct.
	path_user_set_gainL[pathID]=vol_left;
	path_user_set_gainR[pathID]=vol_right;
	path_user_set_gainFormat[pathID]=gain_format;

	if((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)|| sink == AUDIO_SINK_BTS)
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

	if (gain_format == AUDIO_GAIN_FORMAT_FM_RADIO_DIGITAL_VOLUME_TABLE)
	{
		vol_left = APSYSPARM_GetMultimediaAudioParmAccessPtr()[AUDDRV_GetAudioModeBySink(sink)].fm_radio_digital_vol[vol_left]; //mB
		gain_format = AUDIO_GAIN_FORMAT_mB;
	}

	if (gain_format == AUDIO_GAIN_FORMAT_mB)
	{
		switch( sink )
		{
		case AUDIO_SINK_HANDSET:
			pmu_gain[sink] = 0;
			pmu_gain_right[sink] = 0;
			break;

		case AUDIO_SINK_HEADSET:
#ifdef CONFIG_BCMPMU_AUDIO
#if 1
			/***** fix PMU gain, adjust CAPH gain **/
			pmu_gain[sink] = (short) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_HEADSET ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain[sink] = pmu_gain[sink] * 25;  //mB
			pmuAudioGainMap = map2pmu_hs_gain( pmu_gain[sink] );

			pmu_gain_right[sink] = (short) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_HEADSET ].ext_speaker_pga_r; //Q13p2 dB
			pmu_gain_right[sink] = pmu_gain_right[sink] * 25;  //mB
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			pmuAudioGainMap = map2pmu_hs_gain( vol_left );
			//determine actual PMU gain in mB:
			pmu_gain[sink] = pmuAudioGainMap.gain_mB;
#endif
#endif
			break;

		case AUDIO_SINK_TTY:
		//case AUDIO_CHNL_HEADPHONE_NO_MIC:

#ifdef CONFIG_BCMPMU_AUDIO
#if 1
			/***** fix PMU gain, adjust CAPH gain **/
			pmu_gain[sink] = (short) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_TTY ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain[sink] = pmu_gain[sink] * 25;  //mB
			pmuAudioGainMap = map2pmu_hs_gain( pmu_gain[sink] );

			pmu_gain_right[sink] = (short) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_TTY ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain_right[sink] = pmu_gain_right[sink] * 25;  //mB
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			pmuAudioGainMap = map2pmu_hs_gain( vol_left );
			//determine actual PMU gain in mB:
			pmu_gain[sink] = pmuAudioGainMap.gain_mB;
#endif
#endif

			break;

		case AUDIO_SINK_LOUDSPK:
#ifdef CONFIG_BCMPMU_AUDIO
#if 1
			/***** fixed PMU gain, adjust CAPH gain **/
			pmu_gain[sink] = (short) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_SPEAKERPHONE ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain[sink] = pmu_gain[sink] * 25;  //mB
			pmuAudioGainMap = map2pmu_ihf_gain( pmu_gain[sink] );

			pmu_gain_right[sink] = (short) AUDIO_GetParmAccessPtr()[ AUDIO_MODE_SPEAKERPHONE ].ext_speaker_pga_l; //Q13p2 dB
			pmu_gain_right[sink] = pmu_gain_right[sink] * 25;  //mB
#else
			/***** adjust PMU gain, adjust CAPH gain **/
			pmuAudioGainMap = map2pmu_ihf_gain( vol_left );
			pmu_gain[sink] = pmuAudioGainMap.gain_mB;
#endif
#endif

			break;

		default:
			break;
		}


		//set CAPH gain
		vol_left = vol_left - pmu_gain[sink];
		mixerInputGain[sink] = 0; //0 dB

		if( vol_left >= 4214 )
		{
			mixerOutputBitSelect[sink] = 7;
			mixerOutputFineGain[sink] = 0;
		}
		else if( vol_left > 0 )  //0~4213
		{
			mixerOutputBitSelect[sink] = vol_left / 602;
			mixerOutputBitSelect[sink] += 1; //since fine-gain is only an attenuation, round up to the next bit shift
			mixerOutputFineGain[sink] = vol_left - (mixerOutputBitSelect[sink]*602);  //put in attenuation, negative number.
		}
		else if( vol_left == 0 )
		{
			mixerOutputBitSelect[sink] = 0;
			mixerOutputFineGain[sink] = 0;
		}
		else //if(vol_left < 0)
		{
			mixerOutputBitSelect[sink] = 0;
			mixerOutputFineGain[sink] = vol_left; //put in attenuation, negative number.
		}

		vol_right = vol_right - pmu_gain_right[sink];
		mixerInputGain_right[sink] = 0; //0 dB

		if( vol_right >= 4214 )
		{
			mixerOutputBitSelect_right[sink] = 7;
			mixerOutputFineGain_right[sink] = 0;
		}
		else if( vol_right > 0 )  //0~4213
		{
			mixerOutputBitSelect_right[sink] = vol_right / 602;
			mixerOutputBitSelect_right[sink] += 1; //since fine-gain is only an attenuation, round up to the next bit shift
			mixerOutputFineGain_right[sink] = vol_right - (mixerOutputBitSelect_right[sink]*602);  //put in attenuation, negative number.
		}
		else if( vol_right == 0 )
		{
			mixerOutputBitSelect_right[sink] = 0;
			mixerOutputFineGain_right[sink] = 0;
		}
		else //if(vol__right < 0)
		{
			mixerOutputBitSelect_right[sink] = 0;
			mixerOutputFineGain_right[sink] = vol_right; //put in attenuation, negative number.
		}

	}
	else
	{
		return;
	}

	//determine which mixer output to apply the gains to

    Log_DebugPrintf(LOGID_AUDIO,
		"AUDCTRL_SetPlayVolume: pmu_gain %d, mixerInputGain = 0x%x, mixerOutputFineGain = 0x%x, mixerOutputBitSelect %d\n",
		pmu_gain[sink], mixerInputGain[sink], mixerOutputFineGain[sink], mixerOutputBitSelect[sink]);

	Log_DebugPrintf(LOGID_AUDIO,
		"AUDCTRL_SetPlayVolume: sink = 0x%x, gain_format %d, vol_left = 0x%x(%d) vol_right 0x%x(%d)\n",
		sink, gain_format, vol_left, vol_left, vol_right, vol_right);

    speaker = getDeviceFromSink(sink);

	Log_DebugPrintf(LOGID_AUDIO,
		"AUDCTRL_SetPlayVolume: pathID %d\n",
		pathID);


	//if ( path->status != PATH_OCCUPIED )
	if( FALSE == csl_caph_QueryHWClock() )
	{
		Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_SetPlayVolume: clock is off\n");
	
		//the CAPH clock may be not turned on.
		//defer setting the FM radio audio gain until start render.
		return;
	}


	if (pathID != 0)
    {
		path = &HWConfig_Table[pathID-1];

        // find the sinkNo with the same sink of input speaker
        for(j = 0; j < MAX_SINK_NUM; j++)
        {
            if (path->sink[j] == speaker)
            {
                sinkNo = j;
                break;
            }
        }
        outChnl = path->srcmRoute[sinkNo][0].outChnl;
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
		csl_caph_srcmixer_set_mix_in_gain( path->srcmRoute[sinkNo][0].inChnl,
                                    outChnl,
                                    mixerInputGain[sink],
                                    mixerInputGain[sink]);
    }
    else
    {
    	csl_caph_srcmixer_set_mix_all_in_gain(outChnl, mixerInputGain[sink], mixerInputGain[sink]);
    }

	csl_caph_srcmixer_set_mix_out_gain( outChnl, mixerOutputFineGain[sink] );

	csl_caph_srcmixer_set_mix_out_bit_select(outChnl, mixerOutputBitSelect[sink] );

	// Set the gain to the external amplifier
	SetGainOnExternalAmp_mB(sink, pmu_gain[sink], PMU_AUDIO_HS_BOTH);

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
				AUDIO_SOURCE_Enum_t 	source,
				AUDIO_SINK_Enum_t		sink,
				Boolean					mute,
				unsigned int			pathID
				)
{
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetPlayMute: sink = 0x%x,  source = 0x%x, mute = 0x%x\n",
                    sink, source, mute);

	if((source != AUDIO_SOURCE_DSP && sink == AUDIO_SINK_USB)|| sink == AUDIO_SINK_BTS)
		return;

    speaker = getDeviceFromSink(sink);

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
// Description:   switch a speaker to a playback path - suggested to use when a single device is in associated with single path.
// 				For multicasting use cases, use Add/Remove Spk API
//
//============================================================================
void AUDCTRL_SwitchPlaySpk(
                AUDIO_SOURCE_Enum_t     source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
	CSL_CAPH_DEVICE_e curr_spk = CSL_CAPH_DEV_NONE;
	int i,j;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SwitchPlaySpk src = 0x%x, Sink = 0x%x\n",source, sink);
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
					curr_spk = HWConfig_Table[i].sink[j];
					break;
				}

			}
		}
		if(curr_spk != CSL_CAPH_DEV_NONE)
			break;
	}

	if (curr_spk != CSL_CAPH_DEV_NONE)
	{
		if(curr_spk == CSL_CAPH_DEV_HS)
			powerOnExternalAmp( AUDIO_SINK_HEADSET, AudioUseExtSpkr, FALSE );
		else if(curr_spk == CSL_CAPH_DEV_IHF)
			powerOnExternalAmp( AUDIO_SINK_LOUDSPK, AudioUseExtSpkr, FALSE );
	}

	// add new spk first...
    if (getDeviceFromSink(sink) != CSL_CAPH_DEV_NONE)
    {
        config.source = getDeviceFromSrc(source);
        config.sink = getDeviceFromSink(sink);
        (void) csl_caph_hwctrl_AddPath(pathID, config);
    }

    // remove current spk
    if (curr_spk != CSL_CAPH_DEV_NONE)
    {
        config.source = getDeviceFromSrc(source);
        config.sink = curr_spk;
        (void) csl_caph_hwctrl_RemovePath(pathID, config);
    }

    if ((sink == AUDIO_SINK_LOUDSPK)||(sink == AUDIO_SINK_HEADSET))
        powerOnExternalAmp( sink, AudioUseExtSpkr, TRUE );

	AUDCTRL_SetAudioMode_ForMusicPlayback( AUDDRV_GetAudioModeBySink(sink), 0, FALSE );

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
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO, "AUDCTRL_AddPlaySpk: src = %d, sink = %d, pathID %d \n", source, sink, pathID);

    //if(pathID == 0)
    //{
	//    audio_xassert(0,pathID);
	//    return;
    //}
    speaker = getDeviceFromSink(sink);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
		//Enable the PMU for HS/IHF.
		if ((sink == AUDIO_SINK_LOUDSPK)||(sink == AUDIO_SINK_HEADSET))
			powerOnExternalAmp( sink, AudioUseExtSpkr, TRUE );

        config.source = getDeviceFromSrc(source);
        config.sink = speaker;
        (void) csl_caph_hwctrl_AddPath(pathID, config);
    }

    AUDCTRL_SetAudioMode_ForMusicPlayback( AUDDRV_GetAudioModeBySink(sink), 0, FALSE );

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
                AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				unsigned int			pathID
				)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_DEVICE_e speaker = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_RemovePlaySpk: src = 0x%x, sink = 0x%x\n",
                    source, sink);

    if(pathID == 0)
    {
	    audio_xassert(0,pathID);
	    return;
    }
    speaker = getDeviceFromSink(sink);
    if (speaker != CSL_CAPH_DEV_NONE)
    {
		//Disable the PMU for HS/IHF.
		if ((sink == AUDIO_SINK_LOUDSPK)||(sink == AUDIO_SINK_HEADSET))
			powerOnExternalAmp( sink, AudioUseExtSpkr, FALSE );

        config.source = getDeviceFromSrc(source);
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
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t		sink,
				AUDIO_NUM_OF_CHANNEL_t	numCh,
				AUDIO_SAMPLING_RATE_t	sr,
				unsigned int			*pPathID)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
    CSL_CAPH_PathID pathID;

    pathID = 0;
    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    // Enable the path. And get path ID.
    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = 0;
    config.source = getDeviceFromSrc(source);
    config.sink =  getDeviceFromSink(sink);
    config.dmaCH = CSL_CAPH_DMA_NONE;
    config.snk_sampleRate = sr;
    // For playback, sample rate should be 48KHz.
    config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.chnlNum = numCh;
	config.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;

	if (source == AUDIO_SOURCE_USB && sink == AUDIO_SINK_DSP)
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
		AUDDRV_EnableDSPInput( source, sr );
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
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t 		sink,
				AUDIO_NUM_OF_CHANNEL_t	numCh,
				AUDIO_SAMPLING_RATE_t	sr,
				unsigned int			*pPathID
				)
{
	unsigned int pathID;
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_EnableRecord: src = 0x%x, sink = 0x%x,sr %ld\n",
                    source, sink, sr);

	if((source == AUDIO_SOURCE_DIGI1)
	   || (source == AUDIO_SOURCE_DIGI2)
	   || (source == AUDIO_SOURCE_DIGI3)
	   || (source == AUDIO_SOURCE_DIGI4)
	   || (source == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Enable power to digital microphone
		powerOnDigitalMic(TRUE);
	}

	if(source==AUDIO_SOURCE_SPEECH_DIGI)
	{
		/* Not supported - One stream - two paths use case for record. Will be supported with one path itself */
		AUDCTRL_EnableRecordMono(AUDIO_SOURCE_DIGI1, sink, AUDIO_CHANNEL_MONO, sr, &pathID);
		AUDCTRL_EnableRecordMono(AUDIO_SOURCE_DIGI2, sink, AUDIO_CHANNEL_MONO, sr, NULL);
	} else if(source== AUDIO_SOURCE_MIC_ARRAY1) {
		Log_DebugPrintf(LOGID_AUDIO, "Recording : src = 0x%x, sink = 0x%x, numCh = 0x%x, sr = 0x%x \n",source, sink, numCh, sr);
		numCh = 2; /* stereo format */
		AUDCTRL_EnableRecordMono(source, sink, numCh, sr, &pathID);
	} else {
		AUDCTRL_EnableRecordMono(source, sink, numCh, sr, &pathID);
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
				AUDIO_SOURCE_Enum_t		source,
				AUDIO_SINK_Enum_t 		sink,
				unsigned int			pathID
				)
{

    CSL_CAPH_HWCTRL_CONFIG_t config;
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_DisableRecord: src = 0x%x, sink = 0x%x \n",source, sink);

	// Disable DSP UL
	if(sink == AUDIO_SINK_DSP)
	{
		AUDDRV_DisableDSPInput();
	}
	if(source == AUDIO_SOURCE_SPEECH_DIGI)
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

		if (source == AUDIO_SOURCE_USB && sink == AUDIO_SINK_DSP)
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
	if((source == AUDIO_SOURCE_DIGI1)
	   || (source == AUDIO_SOURCE_DIGI2)
	   || (source == AUDIO_SOURCE_DIGI3)
	   || (source == AUDIO_SOURCE_DIGI4)
	   || (source == AUDIO_SOURCE_SPEECH_DIGI))
	{
		// Disable power to digital microphone
		powerOnDigitalMic(FALSE);
	}
}

/****************************************************************************
*
*  Function Name: AUDCTRL_StartCapturet
*
*  Description: Start the data transfer of audio path capture
*
****************************************************************************/
Result_t AUDCTRL_StartCapture(unsigned int streamID)
{
	AudioMode_t mode=AUDIO_MODE_HANDSET;
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_Capture_Drv_t	*audDrv = NULL;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDCTRL_StartCapture::streamID=0x%x\n", streamID));

	csl_audio_capture_start(streamID);

	audDrv = GetCaptureDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

    if( audDrv->pathID ) path = &HWConfig_Table[ audDrv->pathID - 1 ];
    else return 0;

	if ( path->status == PATH_OCCUPIED )
	{
		if( path->source == CSL_CAPH_DEV_ANALOG_MIC)
		{
			mode = AUDIO_MODE_HANDSET;
			//mode = AUDIO_MODE_SPEAKERPHONE; which mode?
		}
		else
		if( path->source == CSL_CAPH_DEV_HS_MIC)
			mode = AUDIO_MODE_HEADSET;
		else
		if( path->source == CSL_CAPH_DEV_DIGI_MIC
			|| path->source == CSL_CAPH_DEV_DIGI_MIC_L
			|| path->source == CSL_CAPH_DEV_DIGI_MIC_R
			)
		{
			mode = AUDIO_MODE_SPEAKERPHONE;
			//mode = AUDIO_MODE_HANDSET;  //which mode?
		}
		else
		if( path->source == CSL_CAPH_DEV_BT_MIC)
			mode = AUDIO_MODE_BLUETOOTH;
	}

	AUDCTRL_SetAudioMode_ForMusicRecord( mode, 0 );

    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: csl_audio_capture_stop
*
*  Description: Stop the data transfer of audio path capture
*
****************************************************************************/
Result_t AUDCTRL_StopCapture (unsigned int streamID)
{
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDCTRL_StopCapture::streamID=0x%x\n", streamID));

	csl_audio_capture_stop (streamID);

    return RESULT_OK;
}


//============================================================================
//
// Function Name: AUDCTRL_SetRecordGain
//
// Description:   set gain of a record path
//
//============================================================================
void AUDCTRL_SetRecordGain(
				AUDIO_SOURCE_Enum_t		source,
                AUDIO_GAIN_FORMAT_t     gainFormat,
				UInt32					gainL,
				UInt32					gainR,
				unsigned int			pathID
				)
{
	CSL_CAPH_HWConfig_Table_t *path;
	csl_caph_Mic_Gain_t outGain;

	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordGain: src = 0x%x, gainL = 0x%lx, gainR = 0x%lx\n",
                    source,  gainL, gainR);

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
				AUDIO_SOURCE_Enum_t		source,
				Boolean					mute,
				unsigned int			pathID
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMuteMono: src = 0x%x, mute = 0x%x\n",
                    source, mute);

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
				AUDIO_SOURCE_Enum_t		source,
				Boolean					mute,
				unsigned int			pathID
				)
{
	Log_DebugPrintf(LOGID_AUDIO,
                    "AUDCTRL_SetRecordMute: src = 0x%x,  mute = 0x%x\n",
                    source, mute);
    if(pathID == 0)
    {
		audio_xassert(0,pathID);
		return;
    }

	if( source == AUDIO_SOURCE_USB)
		return;

	if(source==AUDIO_SOURCE_SPEECH_DIGI)
	{
		/* Not supported - One stream - two paths use case for record. Will be supported with one path itself */
		AUDCTRL_SetRecordMuteMono(AUDIO_SOURCE_DIGI1, mute,pathID);
		AUDCTRL_SetRecordMuteMono(AUDIO_SOURCE_DIGI2, mute,pathID);
	} else {
		AUDCTRL_SetRecordMuteMono(source, mute,pathID);
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
                              AUDIO_SINK_Enum_t	speaker,
			      int sidetone_mode
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
    CSL_CAPH_DEVICE_e source, sink;
    static CSL_CAPH_DEVICE_e audSpkr;
    CSL_CAPH_PathID pathID;
    CSL_CAPH_HWCTRL_CONFIG_t hwCtrlConfig;
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;
	CSL_CAPH_DEVICE_e src_dev,sink_dev;
	int i,j;

    Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d, speaker = %d, mode = %d\n", mic, speaker, sidetone_mode);

    memset(&hwCtrlConfig, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
    source = sink = CSL_CAPH_DEV_NONE;
    audSpkr = CSL_CAPH_DEV_NONE;
    pathID = 0;
    switch (mic)
    {
        case AUDIO_SOURCE_ANALOG_MAIN:
            source = CSL_CAPH_DEV_ANALOG_MIC;
            break;
        case AUDIO_SOURCE_ANALOG_AUX:
            source = CSL_CAPH_DEV_HS_MIC;
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
            break;
        case AUDIO_SOURCE_BTM:
            source = CSL_CAPH_DEV_BT_MIC;
            break;
        default:
            source = CSL_CAPH_DEV_ANALOG_MIC;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: mic = %d\n", mic);
            break;
    }

    switch (speaker)
    {
        case AUDIO_SINK_HANDSET:
            sink = CSL_CAPH_DEV_EP;
            audSpkr = CSL_CAPH_DEV_EP;
            audio_mode = AUDIO_MODE_HANDSET;
            break;
        case AUDIO_SINK_HEADSET:
            sink = CSL_CAPH_DEV_HS;
            audSpkr = CSL_CAPH_DEV_HS;
            audio_mode = AUDIO_MODE_HEADSET;
            break;
        case AUDIO_SINK_LOUDSPK:
            sink = CSL_CAPH_DEV_IHF;
            audSpkr = CSL_CAPH_DEV_IHF;
            audio_mode = AUDIO_MODE_SPEAKERPHONE;
            break;
        case AUDIO_SINK_I2S:
            sink = CSL_CAPH_DEV_FM_TX;
            // No audio mode available for this case.
            // for now just use AUDIO_MODE_HANDSFREE
            audio_mode = AUDIO_MODE_HANDSFREE;
            break;
        case AUDIO_SINK_BTM:
            sink = CSL_CAPH_DEV_BT_SPKR;
            audio_mode = AUDIO_MODE_BLUETOOTH;
            break;
        default:
            audSpkr = CSL_CAPH_DEV_EP;
            sink = CSL_CAPH_DEV_EP;
            audio_mode = AUDIO_MODE_HANDSET;
            Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: speaker = %d\n", speaker);
            break;
    }

	audio_control_generic( AUDDRV_CPCMD_PassAudioMode,
            (UInt32)audio_mode, 0, 0, 0, 0);  //???
    if(enable_lpbk)
    {
        Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback: Enable loopback with sidetone mode = %d \n", sidetone_mode);

	// For I2S/PCM loopback
        if (((source == CSL_CAPH_DEV_FM_RADIO) && (sink == CSL_CAPH_DEV_FM_TX)) ||
		((source == CSL_CAPH_DEV_BT_MIC) && (sink == CSL_CAPH_DEV_BT_SPKR)))
        {
            // I2S hard coded to use ssp3, BT PCM to use ssp4. This could be changed later
            AUDIO_SOURCE_Enum_t srcTemp = AUDIO_SOURCE_I2S;
            AUDIO_SINK_Enum_t sinkTemp = AUDIO_SINK_I2S;
            if(source == CSL_CAPH_DEV_BT_MIC) srcTemp = AUDIO_SOURCE_BTM;
            if(sink == CSL_CAPH_DEV_BT_SPKR) sinkTemp = AUDIO_SINK_BTM;


            AUDCTRL_EnablePlay (srcTemp, speaker, AUDIO_CHANNEL_MONO, 48000, NULL);
            AUDCTRL_EnableRecord (mic, sinkTemp, AUDIO_CHANNEL_MONO, 48000,NULL);
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
        if(source == CSL_CAPH_DEV_BT_MIC) hwCtrlConfig.src_sampleRate = AUDIO_SAMPLING_RATE_8000; //how about WB?
        if(sink == CSL_CAPH_DEV_BT_SPKR) hwCtrlConfig.snk_sampleRate = AUDIO_SAMPLING_RATE_8000; //how about WB?

        hwCtrlConfig.chnlNum = (speaker == AUDIO_SINK_HEADSET) ? AUDIO_CHANNEL_STEREO : AUDIO_CHANNEL_MONO;
        hwCtrlConfig.bitPerSample = AUDIO_16_BIT_PER_SAMPLE;
	hwCtrlConfig.sidetone_mode= sidetone_mode;


        pathID = csl_caph_hwctrl_EnablePath(hwCtrlConfig);

//#ifdef HW_SIDETONE_LOOPBACK
	if (sidetone_mode) //sidetone_mode = 1 to use sidetone path
	{
        	//Enable the sidetone path.
		//first step: enable sidetone
		csl_caph_hwctrl_EnableSidetone(sink);
		//second step: set filter and gain
		csl_caph_hwctrl_ConfigSidetoneFilter(coeff);
		csl_caph_hwctrl_SetSidetoneGain(0); // Set sidetone gain to 0dB.
	}
	else //loopback does not use sidetone path
	{
		Log_DebugPrintf(LOGID_AUDIO,"AUDCTRL_SetAudioLoopback, sidetone path disabled.\n");
	}
//#endif
#if defined(USE_NEW_AUDIO_PARAM)
		AUDCTRL_SetAudioMode( audio_mode, 0 /*AUDCTRL_GetAudioApp()*/); //this function also sets all HW gains.
#else
		AUDCTRL_SetAudioMode( audio_mode ); //this function also sets all HW gains.
#endif

		//sets all HW gains.
		//AUDCTRL_SetAudioMode is for telephony. But is AUDCTRL_SetAudioMode_ForMusicPlayback enough for both source and sink?
		//AUDCTRL_SetAudioMode( audio_mode ); //this function also sets all HW gains.
		AUDCTRL_SetAudioMode_ForMusicPlayback(audio_mode, pathID, TRUE);

        // Enable Loopback ctrl
	    //Enable PMU for headset/IHF
    	if ((speaker == AUDIO_SINK_LOUDSPK)
    	    ||(speaker == AUDIO_SINK_HEADSET))
	        powerOnExternalAmp( speaker, AudioUseExtSpkr, TRUE );

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
            AUDIO_SOURCE_Enum_t srcTemp = AUDIO_SOURCE_I2S;
            AUDIO_SINK_Enum_t sinkTemp = AUDIO_SINK_I2S;
            if(source == CSL_CAPH_DEV_BT_MIC) srcTemp = AUDIO_SOURCE_BTM;
            if(sink == CSL_CAPH_DEV_BT_SPKR) sinkTemp = AUDIO_SINK_BTM;

            AUDCTRL_DisablePlay (srcTemp, speaker, 0);
            AUDCTRL_DisableRecord (mic,sinkTemp,0);
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

		src_dev = getDeviceFromSrc(mic);
		sink_dev = getDeviceFromSink(speaker);

		//Need CSL API to obtain the info
		for (i=0; i<MAX_AUDIO_PATH; i++)
		{
			for(j = 0; j < MAX_SINK_NUM; j++)
			{
				if ((HWConfig_Table[i].sink[j] == sink_dev) && (HWConfig_Table[i].source == src_dev))
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

//#ifdef HW_SIDETONE_LOOPBACK
        //Disable Sidetone path.
	if (sidetone_mode)  csl_caph_hwctrl_DisableSidetone(sink);
//#endif

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
//move this to driver
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
//move this to driver
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
void  AUDCTRL_SetBTMode(int mode)
{
	 csl_caph_hwctrl_SetBTMode(mode);
}

/********************************************************************
*  @brief  Hardware register access fucntion
*
*  @param  
*
*  @return  int
* note  alsa_amixer cset name=HW-CTL 1 0 100 0  (set EP_MIX_IN_GAIN to 100 mB, 1 dB)
*         alsa_amixer cset name=HW-CTL 1 1 2400 0  (set EP_MIX_BITSEL_GAIN to 24 dB)
*         alsa_amixer cset name=HW-CTL 1 2 -600 0  (set EP_MIX_FINE_GAIN to -6 dB)
*
****************************************************************************/
int	AUDCTRL_HardwareControl( AUDCTRL_HW_ACCESS_TYPE_en_t access_type,
			int arg1,
			int arg2,
			int arg3
			)
{
  PMU_AudioGainMapping_t pmu_gain;
  CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl = CSL_CAPH_SRCM_CH_NONE;
  //CSL_CAPH_HWConfig_Table_t *path = NULL;
  //path = &HWConfig_Table[ VOICECALL_pathID - 1 ]; 

  csl_caph_ControlHWClock(TRUE);
  
  switch ( access_type )
  {
  case AUDCTRL_HW_WRITE_GAIN:
  
    //arg2 is gain in milli Bel
    switch(arg1)
    {
    case AUDCTRL_EP_MIX_IN_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
      //csl_caph_srcmixer_set_mix_in_gain( path->srcmRoute[0][0].inChnl, outChnl, arg2, arg2);
	  csl_caph_srcmixer_set_mix_all_in_gain( outChnl, arg2, arg2);
      break;

    case AUDCTRL_EP_MIX_BITSEL_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
	  csl_caph_srcmixer_set_mix_out_bit_select(outChnl, arg2/600);
      break;

    case AUDCTRL_EP_MIX_FINE_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
      csl_caph_srcmixer_set_mix_out_gain( outChnl, arg2 );
      break;
      
    case AUDCTRL_IHF_MIX_IN_GAIN:
      //outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R | CSL_CAPH_SRCM_STEREO_CH2_L);
      outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
      //csl_caph_srcmixer_set_mix_in_gain( path->srcmRoute[0][0].inChnl, outChnl, arg2, arg2);
      csl_caph_srcmixer_set_mix_all_in_gain( outChnl, arg2, arg2);
      break;
      
    case AUDCTRL_IHF_MIX_BITSEL_GAIN:
      //outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R | CSL_CAPH_SRCM_STEREO_CH2_L);
      outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
      csl_caph_srcmixer_set_mix_out_bit_select(outChnl, arg2/600);
      break;
      
    case AUDCTRL_IHF_MIX_FINE_GAIN:
      //outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R | CSL_CAPH_SRCM_STEREO_CH2_L);
      outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
      csl_caph_srcmixer_set_mix_out_gain( outChnl, arg2 );
      break;
      
    case AUDCTRL_HS_LEFT_MIX_IN_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
	  //csl_caph_srcmixer_set_mix_in_gain( path->srcmRoute[0][0].inChnl, outChnl, arg2, arg2);
      csl_caph_srcmixer_set_mix_all_in_gain( outChnl, arg2, arg2);
      break;
      
    case AUDCTRL_HS_LEFT_MIX_BITSEL_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
      csl_caph_srcmixer_set_mix_out_bit_select(outChnl, arg2/600);
      break;
      
    case AUDCTRL_HS_LEFT_MIX_FINE_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH1_L;
      csl_caph_srcmixer_set_mix_out_gain( outChnl, arg2 );
      break;
      
    case AUDCTRL_HS_RIGHT_MIX_IN_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH1_R;
      //csl_caph_srcmixer_set_mix_in_gain( path->srcmRoute[0][0].inChnl, outChnl, arg2, arg2);
      csl_caph_srcmixer_set_mix_all_in_gain( outChnl, arg2, arg2);
      break;
      
    case AUDCTRL_HS_RIGHT_MIX_BITSEL_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH1_R;
      csl_caph_srcmixer_set_mix_out_bit_select(outChnl, arg2/600);
      break;
      
    case AUDCTRL_HS_RIGHT_MIX_FINE_GAIN:
      outChnl = CSL_CAPH_SRCM_STEREO_CH1_R;
      csl_caph_srcmixer_set_mix_out_gain( outChnl, arg2 );
	  break;
	  
    case AUDCTRL_PMU_HS_RIGHT_GAIN:
      pmu_gain = map2pmu_hs_gain( arg2 );  //from mB to pmu gain enum
      AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_RIGHT, pmu_gain.PMU_gain_enum );
      break;
      
    case AUDCTRL_PMU_HS_LEFT_GAIN:
      pmu_gain = map2pmu_hs_gain( arg2 );  //from mB to pmu gain enum
      AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_LEFT, pmu_gain.PMU_gain_enum );
      break;
      
    case AUDCTRL_PMU_IHF_GAIN:
      pmu_gain = map2pmu_ihf_gain( arg2 );   //from mB to pmu gain enum
      AUDIO_PMU_IHF_SET_GAIN( pmu_gain.PMU_gain_enum );
      break;
      
    case AUDCTRL_PMU_HIGH_GAIN_MODE:
      AUDIO_PMU_HI_GAIN_MODE_EN( arg2 );  //arg2: TRUE or FALSE
      break;

    default:
      break;
    }
    break;

  default:
    break;
  }

  return 0;
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
#ifdef CONFIG_BCMPMU_AUDIO
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
#ifdef CONFIG_BCMPMU_AUDIO

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
// Function Name: getDeviceFromSrc
//
// Description:   convert audio controller Mic enum to auddrv device enum
//
//============================================================================
CSL_CAPH_DEVICE_e getDeviceFromSrc(AUDIO_SOURCE_Enum_t source)
{
	//Log_DebugPrintf(LOGID_AUDIO,"getDeviceFromSrc: source = 0x%x\n", source);
	return MIC_Mapping_Table[source].dev;
}


//============================================================================
//
// Function Name: getDeviceFromSink
//
// Description:   convert audio controller Spkr enum to auddrv device enum
//
//============================================================================
CSL_CAPH_DEVICE_e getDeviceFromSink(AUDIO_SINK_Enum_t sink)
{
	//Log_DebugPrintf(LOGID_AUDIO,"getDeviceFromSink: sink = 0x%x\n", sink);
	return SPKR_Mapping_Table[sink].dev;
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

#ifdef CONFIG_BCMPMU_AUDIO
	static Boolean telephonyUseHS = FALSE;
	static Boolean audioUseHS = FALSE;
	static int audio2UseHS = FALSE;
	static int FMRadioUseHS = FALSE;

	static Boolean telephonyUseIHF = FALSE;
	static Boolean audioUseIHF = FALSE;
	static int audio2UseIHF = FALSE;
	static int FMRadioUseIHF = FALSE;

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

				case FMRadioUseExtSpkr:
					FMRadioUseHS = use;
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

				case FMRadioUseExtSpkr:
					FMRadioUseIHF = use;
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

	//pr_err("powerOnExternalAmp, speaker = %d, IHF_IsOn= %d, HS_IsOn = %d, Boolean_Use=%d, FMRadioUseHS=%d\n", speaker, IHF_IsOn, HS_IsOn, use, FMRadioUseHS);

	if ((telephonyUseHS==FALSE) && (audioUseHS==FALSE) && (FMRadioUseHS==FALSE))
	{
		if ( HS_IsOn != FALSE )
		{
			Log_DebugPrintf(LOGID_AUDIO,"power OFF pmu HS amp\n");

			AUDIO_PMU_HS_SET_GAIN(PMU_AUDIO_HS_BOTH, PMU_HSGAIN_MUTE),
			AUDIO_PMU_HS_POWER(FALSE);
			mdelay(20);
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
			mdelay(75);

		}

		//the ext_speaker_pga_l is in q13.2 format
		hs_gain = (short) AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
		SetGainOnExternalAmp_mB( AUDIO_SINK_HEADSET, hs_gain, PMU_AUDIO_HS_LEFT);

		hs_gain = (short) AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_r;
		SetGainOnExternalAmp_mB( AUDIO_SINK_HEADSET, hs_gain, PMU_AUDIO_HS_RIGHT);

		HS_IsOn = TRUE;
	}

	if ((telephonyUseIHF==FALSE) && (audioUseIHF==FALSE) && (FMRadioUseIHF==FALSE))
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
		ihf_gain = (short) AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_pga_l;
		SetGainOnExternalAmp_mB( AUDIO_SINK_LOUDSPK, ihf_gain, PMU_AUDIO_HS_BOTH);

		ihf_gain = (int) AUDIO_GetParmAccessPtr()[ AUDDRV_GetAudioMode() ].ext_speaker_high_gain_mode_enable;
		AUDIO_PMU_HI_GAIN_MODE_EN(ihf_gain);

		IHF_IsOn = TRUE;
	}

	if ( IHF_IsOn==FALSE && HS_IsOn==FALSE )
		AUDIO_PMU_DEINIT();    //disable the audio PLL after power OFF

#endif

}

