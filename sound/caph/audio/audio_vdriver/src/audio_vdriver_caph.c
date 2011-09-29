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
* @file   audio_vdriver_caph.c
* @brief  Audio VDriver API for CAPH
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "dspcmd.h"
#include "csl_apcmd.h"
#include "audio_consts.h"
#include "bcm_fuse_sysparm_CIB.h"
#include "ostask.h"
#include "log.h"
#include "csl_caph.h"
#include "csl_apcmd.h"
#include "csl_caph_audioh.h"
#include "csl_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "csl_caph_gain.h"
#include <mach/comms/platform_mconfig.h>
#include "io.h"
#if defined(ENABLE_DMA_VOICE)
#include "csl_dsp_caph_control_api.h"
#endif

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//=============================================================================
// Public Variable declarations
//=============================================================================

typedef void (*AUDDRV_User_CB) (UInt32 param1, UInt32 param2, UInt32 param3);

AUDDRV_MIC_Enum_t   currVoiceMic = AUDDRV_MIC_NONE;   //used in pcm i/f control. assume one mic, one spkr.
AUDDRV_SPKR_Enum_t  currVoiceSpkr = AUDDRV_SPKR_NONE;  //used in pcm i/f control. assume one mic, one spkr.
Boolean inVoiceCall = FALSE;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================


//static Boolean voiceInPathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut
static Boolean voicePlayOutpathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN

static void *sUserCB = NULL;
static CSL_CAPH_DEVICE_e sink = CSL_CAPH_DEV_NONE;
static AUDDRV_SPKR_Enum_t currSpkr = AUDDRV_SPKR_NONE;
static AUDDRV_MIC_Enum_t currMic = AUDDRV_MIC_NONE;
static AUDIO_SAMPLING_RATE_t currSampleRate = AUDIO_SAMPLING_RATE_UNDEFINED; 
static Boolean eciEQOn = FALSE; // If TRUE, bypass EQ filter setting request from audio controller.

static Boolean bInVoiceCall = FALSE;
static UInt32 audDev = 0;

static Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, void* coeff);
static Result_t AUDDRV_HWControl_EnableSideTone(AudioMode_t audio_mode);
static Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain);

static Result_t AUDDRV_HWControl_DisableSideTone(AudioMode_t audio_mode);

//=============================================================================
// Private function prototypes
//=============================================================================

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

static UInt32* AUDIO_GetIHF48KHzBufferBaseAddress (void);
//=============================================================================
// Functions
//=============================================================================


//=============================================================================
//
// Function Name: AUDDRV_Telephony_InitHW
//
// Description:   Enable the HW for Telephone voice call
//
//=============================================================================
void AUDDRV_Telephony_InitHW (AUDDRV_MIC_Enum_t mic,
				     AUDDRV_SPKR_Enum_t speaker,
				  AUDIO_SAMPLING_RATE_t	sample_rate,
				  void * pData)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
	UInt32 dev = 0;
    Int16 tempGain = 0;
    AudioMode_t mode = AUDIO_MODE_HANDSET;

    UInt32 *memAddr = 0;

    pData = pData;
    mode = mode; 
    tempGain = tempGain;
    Log_DebugPrintf(LOGID_AUDIO, 
                    "\n\r\t* AUDDRV_Telephony_InitHW mic=%d, spkr=%d sample_rate=%ld*\n\r", 
                    mic, speaker, sample_rate);
	
    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    currSpkr = speaker;
    currMic = mic;
    currSampleRate = sample_rate;
    mode = AUDDRV_GetAudioMode();
    //DL
    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = 0;
    config.source = CSL_CAPH_DEV_DSP;
    config.sink = AUDDRV_GetDRVDeviceFromSpkr(speaker);
    dev |= AUDDRV_SPEAKER;
    config.dmaCH = CSL_CAPH_DMA_NONE;    
    //If DSP DL goes to IHF, Sample rate should be 48KHz.
    if (speaker == AUDDRV_SPKR_IHF)
    {
        config.source = CSL_CAPH_DEV_DSP_throughMEM;
    	config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    }
    else
    {
    	config.src_sampleRate = sample_rate;
    }
    config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;	

	if(config.sink == CSL_CAPH_DEV_HS)
	{
	    config.chnlNum = AUDIO_CHANNEL_STEREO;
	}
	else
	{
	    config.chnlNum = AUDIO_CHANNEL_MONO;
	}

    config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

    sink = config.sink;
	if(sink == CSL_CAPH_DEV_IHF)
	{
		memAddr = AUDIO_GetIHF48KHzBufferBaseAddress();
       
        config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
		config.source = CSL_CAPH_DEV_DSP_throughMEM; //csl_caph_EnablePath() handles the case DSP_MEM when sink is IHF
        
		csl_caph_hwctrl_setDSPSharedMemForIHF((UInt32)memAddr);
	}
	else
	{
	    config.source = CSL_CAPH_DEV_DSP;
	}

    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_l);	
    config.mixGain.mixInGainL = AUDDRV_GetMixerInputGain(tempGain);
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_l);
    config.mixGain.mixOutGainL = AUDDRV_GetMixerOutputFineGain(tempGain);	
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_l);
    config.mixGain.mixOutCoarseGainL = AUDDRV_GetMixerOutputCoarseGain(tempGain);

    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_r);	
    config.mixGain.mixInGainR = AUDDRV_GetMixerInputGain(tempGain);	
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_r);
    config.mixGain.mixOutGainR = AUDDRV_GetMixerOutputFineGain(tempGain);	
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_r);
    config.mixGain.mixOutCoarseGainR = AUDDRV_GetMixerOutputCoarseGain(tempGain);

    ((AUDDRV_PathID_t *)pData)->dlPathID = csl_caph_hwctrl_EnablePath(config);

    //UL
    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = 0;
    config.source = AUDDRV_GetDRVDeviceFromMic(mic);
    dev |= AUDDRV_MIC1;
    config.sink = CSL_CAPH_DEV_DSP;
    config.dmaCH = CSL_CAPH_DMA_NONE;    
    config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.snk_sampleRate = sample_rate;	
    config.chnlNum = AUDIO_CHANNEL_MONO;
    config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

    ((AUDDRV_PathID_t *)pData)->ulPathID = csl_caph_hwctrl_EnablePath(config);
    //If Dual Mic is enabled. Theoretically DMIC3 or DMIC4 are used
    //Here Let us assume it is DMIC3. It can be changed.
    if(AUDDRV_IsDualMicEnabled()==TRUE)
    {
    	dev |= AUDDRV_MIC2;
        config.streamID = CSL_CAPH_STREAM_NONE;
        config.pathID = 0;
        config.source = MIC_NOISE_CANCEL;
        config.sink = CSL_CAPH_DEV_DSP;
        config.dmaCH = CSL_CAPH_DMA_NONE;    
        config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        config.snk_sampleRate = sample_rate;	
        config.chnlNum = AUDIO_CHANNEL_MONO;
        config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

        ((AUDDRV_PathID_t *)pData)->ul2PathID = csl_caph_hwctrl_EnablePath(config);
    }

    //Config sidetone
    AUDDRV_SetHWSidetoneFilter(AUDDRV_GetAudioMode(),
				AUDIO_GetParmAccessPtr());

    // Set new filter coef.
    AUDDRV_SetAudioMode( AUDDRV_GetAudioMode(), dev);
 
    return;
}




//=============================================================================
//
// Function Name: AUDDRV_Telephony_DeinitHW
//
// Description:   Disable the HW for Telephone voice call
//
//=============================================================================
void AUDDRV_Telephony_DeinitHW (void *pData)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
	Log_DebugPrintf(LOGID_AUDIO, 
                    "\n\r\t* AUDDRV_Telephony_DeinitHW *\n\r");

    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    currSpkr = AUDDRV_SPKR_NONE;
    currSampleRate = AUDIO_SAMPLING_RATE_UNDEFINED;

    // Disable sidetone.
    (void)AUDDRV_HWControl_DisableSideTone(AUDDRV_GetAudioMode());

    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = ((AUDDRV_PathID_t *)pData)->ulPathID;

    (void)csl_caph_hwctrl_DisablePath(config);

    if(AUDDRV_IsDualMicEnabled()==TRUE)
    {
        config.streamID = CSL_CAPH_STREAM_NONE;
        config.pathID = ((AUDDRV_PathID_t *)pData)->ul2PathID;

        (void)csl_caph_hwctrl_DisablePath(config);
    }
	
    currMic = AUDDRV_MIC_NONE;
    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = ((AUDDRV_PathID_t *)pData)->dlPathID;

    (void)csl_caph_hwctrl_DisablePath(config);

	sink = CSL_CAPH_DEV_NONE;
	audDev = 0;

    return;
}



//=============================================================================
//
// Function Name: AUDDRV_Telephony_MuteSpkr 
//
// Description:   Mute speaker for voice call
//
//=============================================================================

void AUDDRV_Telephony_MuteSpkr (AUDDRV_SPKR_Enum_t speaker,
					void *pData)
{
    audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkMute, 0, 0, 0, 0, 0);
}


//=============================================================================
//
// Function Name: AUDDRV_Telephony_UnmuteSpkr 
//
// Description:   UnMute speaker for voice call
//
//=============================================================================

void AUDDRV_Telephony_UnmuteSpkr (AUDDRV_SPKR_Enum_t speaker,
					void *pData)
{
    audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkUnmute, 0, 0, 0, 0, 0);
}



//=============================================================================
//
// Function Name: Spkr AUDDRV_Telephony_MuteMic 
//
// Description:   Mute mic for voice call
//
//=============================================================================

void AUDDRV_Telephony_MuteMic (AUDDRV_MIC_Enum_t mic,
					void *pData)
{
	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
}


//=============================================================================
//
// Function Name: Spkr AUDDRV_Telephony_UnmuteMic 
//
// Description:   UnMute mic for voice call
//
//=============================================================================

void AUDDRV_Telephony_UnmuteMic (AUDDRV_MIC_Enum_t mic,
					void *pData)
{
	audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
}


//=============================================================================
//
// Function Name: AUDDRV_Telephony_SelectMicSpkr
//
// Description:   Select the mic and speaker for voice call
//
//=============================================================================


void AUDDRV_Telephony_SelectMicSpkr (AUDDRV_MIC_Enum_t mic,
					AUDDRV_SPKR_Enum_t speaker,
					void *pData)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;	
    AudioMode_t mode = AUDIO_MODE_HANDSET;
	UInt32 dev = 0;
    Int16 tempGain = 0;
    UInt32 *memAddr = 0;

    mode = mode; 
    tempGain = tempGain;
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr mic %d, spkr %d *\n\r", mic, speaker);

    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    mode = AUDDRV_GetAudioMode();
	// The sequence from dsp is:
	// 1. first send digital sound cmd to dsp
	// 2. then config and start sspi
	//if( speaker == AUDDRV_SPKR_PCM_IF || mic==AUDDRV_MIC_PCM_IF )
	if( mic==AUDDRV_MIC_PCM_IF )
	{
		AUDDRV_SetPCMOnOff( 1 );
	}
	else
	{
		AUDDRV_SetPCMOnOff( 0 );
	}
	
    if (speaker != currSpkr)
    {	    
	//Disable the existing speaker path.	    
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = ((AUDDRV_PathID_t *)pData)->dlPathID;
	(void)csl_caph_hwctrl_DisablePath(config);
	((AUDDRV_PathID_t *)pData)->dlPathID = 0;

	if(AUDDRV_GetDRVDeviceFromSpkr(currSpkr) == CSL_CAPH_DEV_IHF)
	{
		memAddr = AUDIO_GetIHF48KHzBufferBaseAddress();
		csl_caph_hwctrl_setDSPSharedMemForIHF((UInt32)memAddr);
		VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(FALSE,
							FALSE,
							FALSE);
	}	
	
	//Enable the new speaker path
   	currSpkr = speaker;
	config.streamID = CSL_CAPH_STREAM_NONE;
	config.pathID = 0;
	config.source = CSL_CAPH_DEV_DSP;
	config.sink = AUDDRV_GetDRVDeviceFromSpkr(speaker);
	dev |= AUDDRV_SPEAKER;	
	config.dmaCH = CSL_CAPH_DMA_NONE;    
	//If DSP DL goes to IHF, Sample rate should be 48KHz.
	if (speaker == AUDDRV_SPKR_IHF)
	{
		config.source = CSL_CAPH_DEV_DSP_throughMEM;
    	config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
	}
	else
	{
    	config.src_sampleRate = currSampleRate;
	}
	config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;	

	if(config.sink == CSL_CAPH_DEV_HS)
	{
	    config.chnlNum = AUDIO_CHANNEL_STEREO;
	}
	else
	{
	    config.chnlNum = AUDIO_CHANNEL_MONO;
	}

	config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

	sink = config.sink;
	if(sink == CSL_CAPH_DEV_IHF)
	{
		memAddr = AUDIO_GetIHF48KHzBufferBaseAddress();
		csl_caph_hwctrl_setDSPSharedMemForIHF((UInt32)memAddr);
	}	

    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_l);	
    config.mixGain.mixInGainL = AUDDRV_GetMixerInputGain(tempGain);
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_l);
    config.mixGain.mixOutGainL = AUDDRV_GetMixerOutputFineGain(tempGain);	
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_l);
    config.mixGain.mixOutCoarseGainL = AUDDRV_GetMixerOutputCoarseGain(tempGain);

    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_r);	
    config.mixGain.mixInGainR = AUDDRV_GetMixerInputGain(tempGain);	
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_r);
    config.mixGain.mixOutGainR = AUDDRV_GetMixerOutputFineGain(tempGain);	
    tempGain = (Int16)(AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_r);
    config.mixGain.mixOutCoarseGainR = AUDDRV_GetMixerOutputCoarseGain(tempGain);

	if(sink == CSL_CAPH_DEV_IHF)
	{
		VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(TRUE,
							FALSE,
							FALSE); //integrate SDB CL 366484 
	}	

	((AUDDRV_PathID_t *)pData)->dlPathID = csl_caph_hwctrl_EnablePath(config);

	//Config sidetone
	AUDDRV_SetHWSidetoneFilter(AUDDRV_GetAudioMode(),
			AUDIO_GetParmAccessPtr());    

    }

    if (mic != currMic)
    {
	    //Disable the existing mic path.	    
	    config.streamID = CSL_CAPH_STREAM_NONE;
	    config.pathID = ((AUDDRV_PathID_t *)pData)->ulPathID;
	    (void)csl_caph_hwctrl_DisablePath(config);
	    ((AUDDRV_PathID_t *)pData)->ulPathID = 0;

        if((AUDDRV_IsDualMicEnabled()==FALSE)
            &&(((AUDDRV_PathID_t *)pData)->ul2PathID != 0))
        {
            config.streamID = CSL_CAPH_STREAM_NONE;
            config.pathID = ((AUDDRV_PathID_t *)pData)->ul2PathID;
            (void)csl_caph_hwctrl_DisablePath(config);
            ((AUDDRV_PathID_t *)pData)->ul2PathID = 0;
        }


   	    currMic = mic;
        //UL
        config.streamID = CSL_CAPH_STREAM_NONE;
        config.pathID = 0;
        config.source = AUDDRV_GetDRVDeviceFromMic(mic);
		dev |= AUDDRV_MIC1;
        config.sink = CSL_CAPH_DEV_DSP;
        config.dmaCH = CSL_CAPH_DMA_NONE;    
        config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        config.snk_sampleRate = currSampleRate;	
        config.chnlNum = AUDIO_CHANNEL_MONO;
        config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;
        ((AUDDRV_PathID_t *)pData)->ulPathID = csl_caph_hwctrl_EnablePath(config);

        //If Dual Mic is enabled. Theoretically DMIC3 or DMIC4 are used
        //Here Let us assume it is DMIC3. It can be changed.
        if((AUDDRV_IsDualMicEnabled()==TRUE)
            &&(((AUDDRV_PathID_t *)pData)->ul2PathID == 0))
        {
    	    dev |= AUDDRV_MIC2;
            config.streamID = CSL_CAPH_STREAM_NONE;
            config.pathID = 0;
            config.source = MIC_NOISE_CANCEL;
            config.sink = CSL_CAPH_DEV_DSP;
            config.dmaCH = CSL_CAPH_DMA_NONE;    
            config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
            config.snk_sampleRate = currSampleRate;	
            config.chnlNum = AUDIO_CHANNEL_MONO;
            config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;
            ((AUDDRV_PathID_t *)pData)->ul2PathID = csl_caph_hwctrl_EnablePath(config);
        }
    }
    // Set new filter coef.
    AUDDRV_SetAudioMode( AUDDRV_GetAudioMode(), dev);

#if 0
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
#endif    
}

//=============================================================================
//
// Function Name: AUDDRV_EnableDSPOutput
//
// Description:   Enable audio DSP output for voice call
//
//=============================================================================

void AUDDRV_EnableDSPOutput (
				AUDDRV_SPKR_Enum_t     mixer_speaker_selection,
				AUDIO_SAMPLING_RATE_t  sample_rate
				)

{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableDSPOutput mixer %d, invoicecall %d, sample_rate %ld *\n\r", mixer_speaker_selection, inVoiceCall, sample_rate);

	OSTASK_Sleep( 5 );  //sometimes BBC video has no audio. This delay may help the mixer filter and mixer gain loading.
	currVoiceSpkr = mixer_speaker_selection;

	if(inVoiceCall != TRUE)
	{
		//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_DL
		if (sample_rate == AUDIO_SAMPLING_RATE_8000)
		{
#if defined(ENABLE_DMA_VOICE)
			csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_8000);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, DSP_AADMAC_SPKR_EN, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 0, 0, 0, 0 );
#else
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 0, 0, 0, 0 );
#endif
		}
		else
		{
#if defined(ENABLE_DMA_VOICE)
			csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_16000);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, DSP_AADMAC_SPKR_EN, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 0, 0, 0, 0 );
#else
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 1, 0, 0, 0 );
#endif
		}
		voicePlayOutpathEnabled = TRUE;

		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Enable_Output: inVoiceCall = %d, voicePlayOutpathEnabled = %d\n\r", inVoiceCall, voicePlayOutpathEnabled);

#if 0
		if (currVoiceSpkr == AUDDRV_SPKR_PCM_IF)
			AUDDRV_SetPCMOnOff( 1 );
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
		else
		{
			if(currVoiceMic != AUDDRV_MIC_PCM_IF) //need to check mic too.
			AUDDRV_SetPCMOnOff( 0 );
			//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
		}
#endif
				
	}
}


//=============================================================================
//
// Function Name: AUDDRV_EnableDSPInput
//
// Description:   Enable audio DSP output for voice call
//
//=============================================================================

void AUDDRV_EnableDSPInput (
				AUDDRV_MIC_Enum_t     mic_selection,
				AUDIO_SAMPLING_RATE_t  sample_rate
				)

{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableDSPInput mic_selection %d *\n\r", mic_selection );

	if(inVoiceCall != TRUE)
	{
		//if inVoiceCall== TRUE, assume the telphony_init() function sends ENABLE and CONNECT_UL
		if (sample_rate == AUDIO_SAMPLING_RATE_8000)
		{
#if defined(ENABLE_DMA_VOICE)
			csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_8000);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 0, 0, 0, 0);
			//audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, DSP_AADMAC_PRI_MIC_EN|DSP_AADMAC_SEC_MIC_EN, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, DSP_AADMAC_PRI_MIC_EN, 0, 0, 0, 0 ); //no second mic on lmp
#else
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 0, 0, 0, 0);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
#endif
		}
		else
		{
#if defined(ENABLE_DMA_VOICE)
			csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_16000);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 0, 0, 0, 0);
			//audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, DSP_AADMAC_PRI_MIC_EN|DSP_AADMAC_SEC_MIC_EN, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, DSP_AADMAC_PRI_MIC_EN, 0, 0, 0, 0 ); //no second mic on lmp
#else
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 1, 0, 0, 0);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
#endif
		}
//		voiceInPathEnabled = TRUE;
	}

#if	0

	currVoiceMic = mic_selection;
	if (currVoiceMic == AUDDRV_MIC_PCM_IF)
		AUDDRV_SetPCMOnOff( 1 );
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
	else
	{
		if (currVoiceSpkr != AUDDRV_SPKR_PCM_IF) //need to check spkr too.
			AUDDRV_SetPCMOnOff( 0 );
			//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
	}

#endif

}


//=============================================================================
//
// Function Name: AUDDRV_SetULSpeechRecordGain
//
// Description:   set UL speech recording gain
//
//=============================================================================

void AUDDRV_SetULSpeechRecordGain(Int16 gain)
{
	audio_control_generic( AUDDRV_CPCMD_SetULSpeechRecordGain, 
				(UInt32)gain, 0, 0, 0, 0 );
    return;
}

//=============================================================================
//
// Function Name: AUDDRV_SetPCMOnOff
//
// Description:   	set PCM on/off for BT
// 
//=============================================================================

void AUDDRV_SetPCMOnOff(Boolean	on_off)
{
	  //return;

#define DSP_AUDIO_REG_AMCR  0xe540

	// By default the PCM port is occupied by trace port on development board
	if(on_off)
	{
#define SYSCFG_IOCR0                     (SYSCFG_BASE_ADDR + 0x0000)  /* IOCR0 bit I/O Configuration Register 0       */
#define SYSCFG_IOCR0_PCM_MUX                       0x00C00000 

		//mux to PCM interface (set to 00)
	//	*(volatile UInt32*) SYSCFG_IOCR0 &= ~(SYSCFG_IOCR0_PCM_MUX);

		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);

	}
	else
	{
		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);

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
//static AudioMode_t currMusicAudioMode = AUDIO_MODE_HANDSET;

void AUDDRV_SaveAudioMode( AudioMode_t audio_mode )
{
	currAudioMode = audio_mode; // update mode
}

UInt32 AUDDRV_GetAudioDev()
{
	return audDev;
}

//=============================================================================
//
// Function Name: AUDDRV_SetAudioMode
//
// Description:   set audio mode.
//
//=============================================================================

void AUDDRV_SetAudioMode( AudioMode_t audio_mode, UInt32 dev)
{
	SysAudioParm_t* pAudioParm;
	pAudioParm = AUDIO_GetParmAccessPtr();
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_mode==%d\n\r", audio_mode );

	// load DSP parameters:
	//if ( audio_mode >= AUDIO_MODE_NUMBER )
	if ( audio_mode >= AUDIO_MODE_NUMBER_VOICE )
		return;  //error

	currAudioMode = audio_mode; // update mode
	//currMusicAudioMode = currAudioMode;

	if ( AUDDRV_InVoiceCall() )
	{
		// BTM needs to support NB or WB too
		if ((audio_mode == AUDIO_MODE_BLUETOOTH_WB) || (audio_mode == AUDIO_MODE_BLUETOOTH) )
		{
			if (AUDDRV_IsBTMWB())
				audio_mode = AUDIO_MODE_BLUETOOTH_WB;
			else
				audio_mode = AUDIO_MODE_BLUETOOTH;
		}
		Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() AUDDRV_InVoiceCall audio_mode=%d \n\r", audio_mode );
	}

	audio_control_generic( AUDDRV_CPCMD_PassAudioMode, 
				(UInt32)audio_mode, 0, 0, 0, 0 );
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, 
				(UInt32)audio_mode, 0, 0, 0, 0 );

///	AUDDRV_SetDSPFilter(audio_mode, dev, &(pAudioParm[audio_mode]));  // The address in LMP can not be used in CP, it causes CP crash.

	audDev = dev;

}

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
// Function Name: AUDDRV_SetMusicMode
//
// Description:   set audio mode.
//
//=============================================================================

void AUDDRV_SetMusicMode( AudioMode_t audio_mode)
{
}

//=============================================================================
//
// Function Name: AUDDRV_SetVCflag
//
// Description:   Set voice call flag for HW control loic.
//
//=============================================================================
void AUDDRV_SetVCflag( Boolean inVoiceCall )
{
	bInVoiceCall = inVoiceCall;
}

//=============================================================================
//
// Function Name: AUDDRV_GetVCflag
//
// Description:   Get voice call flag.
//
//=============================================================================
Boolean AUDDRV_GetVCflag( void )
{
	return bInVoiceCall;
}



//=============================================================================
//
// Function Name: AUDDRV_SetDSPFilter
//
// Description:   Set DSP UL and DL filter
//
//=============================================================================
void AUDDRV_SetDSPFilter( AudioMode_t audio_mode, 
        UInt32 dev,
		SysAudioParm_t* pAudioParm)
{
	// The address in LMP can not be used in CP, it cause CP crash.
	/******
	if (eciEQOn == FALSE)
		audio_control_generic( AUDDRV_CPCMD_SetFilter, 
				audio_mode, dev, (UInt32)pAudioParm, 0, 0 );
	return;
	**********/
}

//=============================================================================
//
// Function Name: AUDDRV_SetHWSidetoneFilter
//
// Description:   Set HW Sidetone filter
//
//=============================================================================

void AUDDRV_SetHWSidetoneFilter(AudioMode_t audio_mode,  
		SysAudioParm_t* pAudioParm)
{
	Int32 *coeff = NULL;
	UInt16 gain = 0;
	UInt16 enable = 0;

	enable = pAudioParm[audio_mode].hw_sidetone_enable;
	if (!enable)
	{
		AUDDRV_HWControl_DisableSideTone(audio_mode);
		return;	
	}
	
	coeff = &(pAudioParm[audio_mode].hw_sidetone_eq[0]);
	AUDDRV_HWControl_SetFilter(AUDDRV_SIDETONE_FILTER, (void *)coeff);		
	gain = pAudioParm[audio_mode].hw_sidetone_gain;	
	AUDDRV_HWControl_SetSideToneGain((UInt32)gain);	
	AUDDRV_HWControl_EnableSideTone(audio_mode);

	return;
}

//=============================================================================
//
// Function Name: AUDDRV_SetHWGain
//
// Description:   Set HW Gain. In Q13.2
//
//=============================================================================

void AUDDRV_SetHWGain(CSL_CAPH_HW_GAIN_e hw, UInt32 gain)
{
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;
	CSL_CAPH_PathID pathID = 0;

	audio_mode = AUDDRV_GetAudioMode();
	if ((audio_mode == AUDIO_MODE_HANDSET)
		||(audio_mode == AUDIO_MODE_HANDSET_WB)
		||(audio_mode == AUDIO_MODE_HAC)
		||(audio_mode == AUDIO_MODE_HAC_WB))		
	{
		dev = CSL_CAPH_DEV_EP;
	}
	else
	if ((audio_mode == AUDIO_MODE_HEADSET)
		||(audio_mode == AUDIO_MODE_HEADSET_WB)
		||(audio_mode == AUDIO_MODE_TTY)
		||(audio_mode == AUDIO_MODE_TTY_WB))
		
	{
		dev = CSL_CAPH_DEV_HS;
	}
	else
	if ((audio_mode == AUDIO_MODE_SPEAKERPHONE)
		||(audio_mode == AUDIO_MODE_SPEAKERPHONE_WB))
	{
		dev = CSL_CAPH_DEV_IHF;
	}


	csl_caph_hwctrl_SetHWGain(pathID, hw, gain, dev);
	return;
}


//=============================================================================
//
// Function Name: AUDDRV_User_CtrlDSP
//
// Description:   Control DSP Loudspeaker Protection
//
//=============================================================================

void AUDDRV_User_CtrlDSP ( AudioDrvUserParam_t audioDrvUserParam, void *user_CB, UInt32 param1, UInt32 param2 )
{
	Boolean spkProtEna = FALSE;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_CtrlDSP *\n\r");
	if (user_CB != NULL)
		sUserCB = user_CB;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_CtrlDSP, UserCB = %lx *\n\r", (UInt32)sUserCB);
	switch (audioDrvUserParam)
	{
		case AUDDRV_USER_GET_SPKPROT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_GET_SPKPROT *\n\r");
			
			audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, 3, (UInt16) param1, 0,0,0);
			break;
		case AUDDRV_USER_ENA_SPKPROT:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_ENA_SPKPROT *\n\r");
			spkProtEna = (Boolean) param1;
			if (spkProtEna)
				
				audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, (UInt16) spkProtEna, (UInt16) param2, 1,0,0);
			
			else
				audio_control_dsp(DSPCMD_TYPE_COMMAND_SP, (UInt16) spkProtEna, (UInt16) param2, 0,0,0);
			
			break;
		case AUDDRV_USER_SET_EQ:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_SET_EQ *\n\r");

			if ((Boolean)param2 == TRUE)
			{
				eciEQOn = TRUE;
				audio_cmf_filter((AudioCompfilter_t *) param1);
			}
			else
				eciEQOn = FALSE;

			break;
		default:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_User_CtrlDSP: Invalid request %d \n\r", audioDrvUserParam);
			break;
	}
}

//=============================================================================
//
// Function Name: AUDDRV_User_HandleDSPInt
//
// Description:   Handle DSP Interrupt
//
//=============================================================================

void AUDDRV_User_HandleDSPInt ( UInt32 param1, UInt32 param2, UInt32 param3 )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_HandleDSPInt *\n\r");
	if(sUserCB)
		((AUDDRV_User_CB) sUserCB) (param1, param2, param3);
}

//=============================================================================
//
// Function Name: AUDDRV_IsDualMicEnable
//
// Description:   Check whether Dual Mic is enabled
//
//=============================================================================

Boolean AUDDRV_IsDualMicEnabled(void)
{
    AudioMode_t mode = AUDIO_MODE_HANDSET;
    mode = AUDDRV_GetAudioMode();
    return (AUDIO_GetParmAccessPtr()[mode].dual_mic_enable != 0);   //in parm_audio.txt, VOICE_DUALMIC_ENABLE
}


//#if	defined(FUSE_COMMS_PROCESSOR)

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, 
*                                                     void* coeff)    
*
*  Description: Load filter coefficients
*
****************************************************************************/
static Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, void* coeff)
{
    if (filter == AUDDRV_SIDETONE_FILTER)
    {
        csl_caph_audioh_sidetone_load_filter((UInt32 *)coeff);
    }
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_EnableSideTone(AudioMode_t audio_mode)    
*  
*  Description: Enable Sidetone path
*
****************************************************************************/
static Result_t AUDDRV_HWControl_EnableSideTone(AudioMode_t audio_mode)
{
    AUDDRV_PATH_Enum_t pathId = AUDDRV_PATH_VIBRA_OUTPUT; 
    switch(audio_mode)
    {
        case AUDIO_MODE_HEADSET:
        case AUDIO_MODE_TTY:
        case AUDIO_MODE_HEADSET_WB:
        case AUDIO_MODE_TTY_WB:
	    pathId = AUDDRV_PATH_HEADSET_OUTPUT;
	    break;
        case AUDIO_MODE_HANDSET:
        case AUDIO_MODE_HAC:
        case AUDIO_MODE_HANDSET_WB:
        case AUDIO_MODE_HAC_WB:
	    pathId = AUDDRV_PATH_EARPICEC_OUTPUT;
	    break;
        case AUDIO_MODE_SPEAKERPHONE:
        case AUDIO_MODE_SPEAKERPHONE_WB:
	    pathId = AUDDRV_PATH_IHF_OUTPUT;
	    break;
	    
	default:
            ;
    }
    csl_caph_audioh_sidetone_control((int)pathId, TRUE);
    return RESULT_OK;
}

/****************************************************************************
*
*  Function Name:Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain)
*
*  Description: Set the sidetone gain
*
****************************************************************************/
static Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain)
{
	csl_caph_audioh_sidetone_set_gain(gain);
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t AUDDRV_HWControl_DisableSideTone(AudioMode_t audio_mode)    
*  
*  Description: Disable Sidetone path
*
****************************************************************************/
static Result_t AUDDRV_HWControl_DisableSideTone(AudioMode_t audio_mode)    
{
    AUDDRV_PATH_Enum_t pathId = AUDDRV_PATH_VIBRA_OUTPUT; 
    switch(audio_mode)
    {
        case AUDIO_MODE_HEADSET:
        case AUDIO_MODE_TTY:
        case AUDIO_MODE_HEADSET_WB:
        case AUDIO_MODE_TTY_WB:
	    pathId = AUDDRV_PATH_HEADSET_OUTPUT;
	    break;
        case AUDIO_MODE_HANDSET:
        case AUDIO_MODE_HAC:
        case AUDIO_MODE_HANDSET_WB:
        case AUDIO_MODE_HAC_WB:
	    pathId = AUDDRV_PATH_EARPICEC_OUTPUT;
	    break;
        case AUDIO_MODE_SPEAKERPHONE:
        case AUDIO_MODE_SPEAKERPHONE_WB:
	    pathId = AUDDRV_PATH_IHF_OUTPUT;
	    break;
	    
	default:
            ;
    }
    csl_caph_audioh_sidetone_control((int)pathId, FALSE);	
    return RESULT_OK;
}




// move from drv_audio_commom.c



// ==========================================================================
//
// Function Name: AUDDRV_GetDRVDeviceFromMic
//
// Description: Get the audio driver Device from the Microphone selection.
//
// =========================================================================
CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromMic (AUDDRV_MIC_Enum_t mic)
{
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromMic:: mic = 0x%x\n", mic);

      switch (mic)
      {
        case AUDDRV_MIC_NONE:
            dev = CSL_CAPH_DEV_NONE;
            break;
			
         case AUDDRV_MIC_SPEECH_DIGI:
         case AUDDRV_DUAL_MIC_DIGI12:
         case AUDDRV_DUAL_MIC_DIGI21:
            dev = CSL_CAPH_DEV_DIGI_MIC;
            break;
			
         case AUDDRV_MIC_DIGI1:
            dev = CSL_CAPH_DEV_DIGI_MIC_L;
            break;	
			
         case AUDDRV_MIC_DIGI2:
            dev = CSL_CAPH_DEV_DIGI_MIC_R;
            break;
			
         case AUDDRV_MIC_EANC_DIGI:
            dev = CSL_CAPH_DEV_EANC_DIGI_MIC;
            break;
			
         case AUDDRV_MIC_ANALOG_MAIN:
            dev = CSL_CAPH_DEV_ANALOG_MIC;
            break;

         case AUDDRV_MIC_ANALOG_AUX:
            dev = CSL_CAPH_DEV_HS_MIC;
            break;

         case AUDDRV_MIC_PCM_IF:
            dev = CSL_CAPH_DEV_BT_MIC;
            break;

         case AUDDRV_MIC_USB_IF:
            dev = CSL_CAPH_DEV_MEMORY;
            break;


        default:
			break;	
    };

	return dev;
}

// ==========================================================================
//
// Function Name: AUDDRV_GetDRVDeviceFromSpkr
//
// Description: Get the audio driver Device from the Speaker selection.
//
// =========================================================================
CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr (AUDDRV_SPKR_Enum_t spkr)
{
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromSpkr:: spkr = 0x%x\n", spkr);

      switch (spkr)
      {
        case AUDDRV_SPKR_NONE:
            dev = CSL_CAPH_DEV_NONE;
            break;
			
         case AUDDRV_SPKR_EP:
            dev = CSL_CAPH_DEV_EP;
            break;
			
         case AUDDRV_SPKR_IHF:
            dev = CSL_CAPH_DEV_IHF;
            break;	
			
         case AUDDRV_SPKR_HS:
            dev = CSL_CAPH_DEV_HS;
            break;
			
         case AUDDRV_SPKR_VIBRA:
            dev = CSL_CAPH_DEV_VIBRA;
            break;
			
         case AUDDRV_SPKR_PCM_IF:
            dev = CSL_CAPH_DEV_BT_SPKR;
            break;
		
         case AUDDRV_SPKR_USB_IF:
            dev = CSL_CAPH_DEV_MEMORY;
            break;

        default:
			break;	
    };

	return dev;
}


/****************************************************************************
*
*  Function Name: csl_caUInt16ph_MicDSP_Gain_t AUDDRV_GetDSPULGain(
*                                         CSL_CAPH_DEVICE_e mic, UInt16 gain)
*
*  Description: read the DSP UL gain
*
****************************************************************************/
Int16 AUDDRV_GetDSPULGain(CSL_CAPH_DEVICE_e mic, Int16 gain)
{
    csl_caph_Mic_Gain_t outGain;
    csl_caph_MIC_Path_e cslMic = MIC_ANALOG_HEADSET;

    memset(&outGain, 0, sizeof(csl_caph_Mic_Gain_t));
    switch (mic)
    {
	    case CSL_CAPH_DEV_ANALOG_MIC:
	    case CSL_CAPH_DEV_HS_MIC:
		    cslMic = MIC_ANALOG_HEADSET;
		    break;
	    
	    case CSL_CAPH_DEV_DIGI_MIC_L:
	    case CSL_CAPH_DEV_DIGI_MIC_R:
	    case CSL_CAPH_DEV_EANC_DIGI_MIC_L:
	    case CSL_CAPH_DEV_EANC_DIGI_MIC_R:
		    cslMic = MIC_DIGITAL;
		    break;

	    default:
		    // For all others, just use
		    // DSP DL gain as analog mic.
		    cslMic = MIC_ANALOG_HEADSET;
		    break;
    }

    Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDSPULGain::mic=0x%x, gain=0x%x\n", mic, gain);
    outGain = csl_caph_gain_GetMicGain(cslMic, gain);
    return outGain.micDSPULGain;
}


/****************************************************************************
*
*  Function Name: Int16 AUDDRV_GetDSPDLGain(
*                                         CSL_CAPH_DEVICE_e mic, UInt16 gain)
*
*  Description: read the DSP DL gain in mdB in Q15
*
****************************************************************************/
Int16 AUDDRV_GetDSPDLGain(CSL_CAPH_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case CSL_CAPH_DEV_EP:
		    cslSpkr = SPKR_EP_DSP;
		    break;
	    
	    case CSL_CAPH_DEV_HS:
	    case CSL_CAPH_DEV_IHF:
		    cslSpkr = SPKR_IHF_HS_DSP;
		    break;

	    case CSL_CAPH_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP_DSP;
		    break;

	    case CSL_CAPH_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP_DSP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP_DSP;
		    break;

    }

    Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDSPDLGain::spkr=0x%x, gain=0x%x\n", spkr, gain);
    outGain = csl_caph_gain_GetSpkrGain(cslSpkr, gain);
    return outGain.spkrDSPDLGain;
}



/****************************************************************************
*
*  Function Name: Int16 AUDDRV_GetHWDLGain(
*                                         CSL_CAPH_DEVICE_e mic, UInt16 gain)
*
*  Description: read the HW DL gain in Q13.2
*
****************************************************************************/
Int16 AUDDRV_GetHWDLGain(CSL_CAPH_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case CSL_CAPH_DEV_EP:
		    cslSpkr = SPKR_EP;
		    break;
	    case CSL_CAPH_DEV_IHF:
		    cslSpkr = SPKR_IHF;
		    break;
	    case CSL_CAPH_DEV_HS:
			cslSpkr = SPKR_HS;
		    break;
	    case CSL_CAPH_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;

	    case CSL_CAPH_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP;
		    break;

    }

    Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetHWDLGain::spkr=0x%x, gain=0x%x\n", spkr, gain);
    outGain = csl_caph_gain_GetSpkrGain(cslSpkr, gain);
    return outGain.spkrHWGain;
}


/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetMixerInputGain(Int16 gain)
*
*  Description: Get the Mixer input gain. Param "gain" is in Q13.2
*               Mixer input gain is register value.
*
****************************************************************************/
UInt16 AUDDRV_GetMixerInputGain(Int16 gain)
{
    csl_caph_Mixer_GainMapping_t outGain;
    outGain = csl_caph_gain_GetMixerGain(gain);
    return outGain.mixerInputGain;
}


/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetMixerOutputFineGain(Int16 gain)
*
*  Description: Get the Mixer output fine gain. Param "gain" is in Q13.2
*               Mixer output fine gain is register value.
*
****************************************************************************/
UInt16 AUDDRV_GetMixerOutputFineGain(Int16 gain)
{
    csl_caph_Mixer_GainMapping_t outGain;
    outGain = csl_caph_gain_GetMixerGain(gain);
    return outGain.mixerOutputFineGain;
}

/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetMixerOutputCoarseGain(Int16 gain)
*
*  Description: Get the Mixer output coarse gain. Param "gain" is in Q13.2
*               Mixer output coarse gain is register value.
*
****************************************************************************/
UInt16 AUDDRV_GetMixerOutputCoarseGain(Int16 gain)
{
    return (UInt16) csl_caph_gain_GetMixerOutputCoarseGain(gain);
}

/****************************************************************************
*
*  Function Name: UInt16 AUDDRV_GetPMUGain(
*                                         CSL_CAPH_DEVICE_e mic, UInt16 gain)
*
*  Description: read the PMU gain in dB in Q13,2. Input gain in Q13.2
*
****************************************************************************/
UInt16 AUDDRV_GetPMUGain(CSL_CAPH_DEVICE_e spkr, Int16 gain)
{
    csl_caph_Spkr_Gain_t outGain;
    csl_caph_SPKR_Path_e cslSpkr = SPKR_EP;

    memset(&outGain, 0, sizeof(csl_caph_Spkr_Gain_t));

    switch (spkr)
    {
	    case CSL_CAPH_DEV_EP:
		    cslSpkr = SPKR_EP;
		    break;
	    
	    case CSL_CAPH_DEV_IHF:
		    cslSpkr = SPKR_IHF;
		    break;

		case CSL_CAPH_DEV_HS:
			cslSpkr = SPKR_HS;
		    break;


	    case CSL_CAPH_DEV_BT_SPKR:
		    // For Bluetooth, it is yet
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;

	    case CSL_CAPH_DEV_MEMORY:
		    // This is for USB headset. It is
		    // to decide whether DSP DL gain is 
		    // needed or not.
		    cslSpkr = SPKR_EP;
		    break;
		    
	    default:
		    // For all others, just use
		    // DSP DL gain as Earpiece.
		    cslSpkr = SPKR_EP;
		    break;

    }

    Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetPMUGain::spkr=0x%x, gain=0x%x\n", spkr, gain);
    outGain = csl_caph_gain_GetSpkrGain(cslSpkr, gain);
    return outGain.spkrPMUGain;
}


CSL_CAPH_DEVICE_e AUDDRV_GetCSLDevice (AUDDRV_DEVICE_e dev)
{
      CSL_CAPH_DEVICE_e cslDev = CSL_CAPH_DEV_NONE;

      switch (dev)
      {
        case AUDDRV_DEV_NONE:
            cslDev = CSL_CAPH_DEV_NONE;
            break;
			
         case AUDDRV_DEV_EP:
            cslDev = CSL_CAPH_DEV_EP;
            break;
			
         case AUDDRV_DEV_HS:
            cslDev = CSL_CAPH_DEV_HS;
            break;	
			
         case AUDDRV_DEV_IHF:
            cslDev = CSL_CAPH_DEV_IHF;
            break;
			
         case AUDDRV_DEV_VIBRA:
            cslDev = CSL_CAPH_DEV_VIBRA;
            break;
			
         case AUDDRV_DEV_FM_TX:
            cslDev = CSL_CAPH_DEV_FM_TX;
            break;
			
         case AUDDRV_DEV_BT_SPKR:
            cslDev = CSL_CAPH_DEV_BT_SPKR;
            break;	
			
         case AUDDRV_DEV_DSP:
            cslDev = CSL_CAPH_DEV_DSP;
            break;
			
         case AUDDRV_DEV_DIGI_MIC:
            cslDev = CSL_CAPH_DEV_DIGI_MIC;
            break;


         case AUDDRV_DEV_DIGI_MIC_L:
            cslDev = CSL_CAPH_DEV_DIGI_MIC_L;
            break;

         case AUDDRV_DEV_DIGI_MIC_R:
            cslDev = CSL_CAPH_DEV_DIGI_MIC_R;
            break;
			
         case AUDDRV_DEV_EANC_DIGI_MIC:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC;
            break;
			
         case AUDDRV_DEV_EANC_DIGI_MIC_L:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC_L;
            break;

         case AUDDRV_DEV_EANC_DIGI_MIC_R:
            cslDev = CSL_CAPH_DEV_EANC_DIGI_MIC_R;
            break;

         case AUDDRV_DEV_SIDETONE_INPUT:
            cslDev = CSL_CAPH_DEV_SIDETONE_INPUT;
            break;	
			
         case AUDDRV_DEV_EANC_INPUT:
            cslDev = CSL_CAPH_DEV_EANC_INPUT;
            break;
			
         case AUDDRV_DEV_ANALOG_MIC:
            cslDev = CSL_CAPH_DEV_ANALOG_MIC;
            break;
			
         case AUDDRV_DEV_HS_MIC:
            cslDev = CSL_CAPH_DEV_HS_MIC;
            break;
			
         case AUDDRV_DEV_BT_MIC:
            cslDev = CSL_CAPH_DEV_BT_MIC;
            break;	
			
         case AUDDRV_DEV_FM_RADIO:
            cslDev = CSL_CAPH_DEV_FM_RADIO;
            break;
			
         case AUDDRV_DEV_MEMORY:
            cslDev = CSL_CAPH_DEV_MEMORY;
            break;

         case AUDDRV_DEV_SRCM:
            cslDev = CSL_CAPH_DEV_SRCM;
            break;
		
        case AUDDRV_DEV_DSP_throughMEM:
            cslDev = CSL_CAPH_DEV_DSP_throughMEM;
            break;
    	    
        default:
		break;	
    };

    return cslDev;
}


static UInt32* AUDIO_GetIHF48KHzBufferBaseAddress (void)
{
        // special path for IHF voice call 
        // need to use the physical address  
		// Linux only change
		AP_SharedMem_t *ap_shared_mem_ptr = ioremap_nocache(AP_SH_BASE, AP_SH_SIZE);
		// Linux only : to get the physical address use the virtual address to compute offset and 
		// add to the base address 
		UInt32 *memAddr = (UInt32 *)(AP_SH_BASE + ((UInt32)&(ap_shared_mem_ptr->shared_aud_out_buf_48k[0][0])
													- (UInt32)ap_shared_mem_ptr)); 
        
        return memAddr;

}

//#endif

