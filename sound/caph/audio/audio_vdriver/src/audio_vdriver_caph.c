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
#include "auddrv_def.h"
//#include "brcm_rdb_sysmap.h"
//#include "brcm_rdb_syscfg.h"
#include "shared.h"
#include "dspcmd.h"
#ifdef CONFIG_AUDIO_BUILD
#include "ripcmdq.h"
#endif
#include "ripisr.h"
#include "audio_consts.h"
#ifdef CONFIG_AUDIO_BUILD
#include "sysparm.h"
#endif
#include "ostask.h"
#include "audioapi_asic.h"
#include "audio_ipc.h"
#include "log.h"
#include "csl_caph.h"
#include "csl_apcmd.h"
#include "drv_caph.h"
#include "csl_caph_audioh.h"
#include "drv_audio_common.h"
#include "drv_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "platform_mconfig_rhea.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/
//This flag should be removed when Rhea DSP image can support IHF.
#define RHEA_DSP_IHF_FEATURE

//=============================================================================
// Public Variable declarations
//=============================================================================
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && defined(IPC_AUDIO))
extern void CP_Audio_ISR_Handler(StatQ_t status_msg);
#endif

#if !(defined(FUSE_APPS_PROCESSOR) && (defined(IPC_FOR_BSP_ONLY) || defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)))
#else
extern void IPC_Audio_Create_BufferPool( void );
#endif


typedef void (*AUDDRV_User_CB) (UInt32 param1, UInt32 param2, UInt32 param3);

AUDDRV_MIC_Enum_t   currVoiceMic = AUDDRV_MIC_NONE;   //used in pcm i/f control. assume one mic, one spkr.
AUDDRV_SPKR_Enum_t  currVoiceSpkr = AUDDRV_SPKR_NONE;  //used in pcm i/f control. assume one mic, one spkr.
Boolean inVoiceCall = FALSE;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================


//static Boolean voiceInPathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut
static Boolean voicePlayOutpathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN

//static AudioEqualizer_en_t	sEqualizerType = EQU_NORMAL;
static void *sUserCB = NULL;
#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))
static AUDDRV_DEVICE_e sink = AUDDRV_DEV_NONE;
static AUDDRV_SPKR_Enum_t currSpkr = AUDDRV_SPKR_NONE;
static AUDDRV_MIC_Enum_t currMic = AUDDRV_MIC_NONE;
static AUDIO_SAMPLING_RATE_t currSampleRate = AUDIO_SAMPLING_RATE_UNDEFINED; 
static Boolean eciEQOn = FALSE; // If TRUE, bypass EQ filter setting request from audio controller.

#endif
#endif

static Boolean bInVoiceCall = FALSE;
static UInt32 audDev = 0;

//static CB_GetAudioMode_t  client_GetAudioMode = NULL;
//static CB_SetAudioMode_t  client_SetAudioMode = NULL;
//static CB_SetMusicMode_t  client_SetMusicMode = NULL;

#if	defined(FUSE_COMMS_PROCESSOR)
static Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, void* coeff);
static Result_t AUDDRV_HWControl_EnableSideTone(AudioMode_t audio_mode);
static Result_t AUDDRV_HWControl_DisableSideTone(AudioMode_t audio_mode);    
static Result_t AUDDRV_HWControl_SetSideToneGain(UInt32 gain);
#endif
//=============================================================================
// Private function prototypes
//=============================================================================
#ifdef CONFIG_AUDIO_BUILD
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void);
#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()[mode]
#endif

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
    AUDDRV_HWCTRL_CONFIG_t config;
	UInt32 dev = 0;
    AudioMode_t mode = AUDIO_MODE_HANDSET;

    pData = pData;
    mode = mode; 
    Log_DebugPrintf(LOGID_AUDIO, 
                    "\n\r\t* AUDDRV_Telephony_InitHW mic=%d, spkr=%d sample_rate=%ld*\n\r", 
                    mic, speaker, sample_rate);
	
    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));

#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))
   
    currSpkr = speaker;
    currMic = mic;
    currSampleRate = sample_rate;
    mode = AUDDRV_GetAudioMode();
    //DL
    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = 0;
    config.source = AUDDRV_DEV_DSP;
    config.sink = AUDDRV_GetDRVDeviceFromSpkr(speaker);
    dev |= AUDDRV_SPEAKER;
    config.dmaCH = CSL_CAPH_DMA_NONE;    
    //If DSP DL goes to IHF, Sample rate should be 48KHz.
    if (speaker == AUDDRV_SPKR_IHF)
    {
        config.source = AUDDRV_DEV_DSP_throughMEM;
    	config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    }
    else
    {
    	config.src_sampleRate = sample_rate;
    }
    config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;	

	if(config.sink == AUDDRV_DEV_HS)
	{
	    config.chnlNum = AUDIO_CHANNEL_STEREO;
	}
	else
	{
	    config.chnlNum = AUDIO_CHANNEL_MONO;
	}

    config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

	// Linux only change - Start
    sink = config.sink;
	if(sink == AUDDRV_DEV_IHF)
	{
        // special path for IHF voice call 
        // need to use the physical address  
		// Linux only change
        AP_SharedMem_t *ap_shared_mem_ptr = SHAREDMEM_GetDsp_SharedMemPtr();
        UInt32 *memAddr = (UInt32 *)(AP_SH_BASE + ((UInt32)&(ap_shared_mem_ptr->shared_aud_out_buf_48k[0][0])
				- (UInt32)ap_shared_mem_ptr));

        config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
		config.source = AUDDRV_DEV_DSP_throughMEM; //csl_caph_EnablePath() handles the case DSP_MEM when sink is IHF
        
        AUDDRV_HWControl_SetDSPSharedMeMForIHF((UInt32)memAddr);
        VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(TRUE,
                            FALSE,
                            FALSE);
	}
	else
	{
	    config.source = AUDDRV_DEV_DSP;
	}
	// Linux only change - End
#ifdef CONFIG_AUDIO_BUILD
    config.mixGain.mixInGainL = AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_l;	
    config.mixGain.mixOutGainL = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_l;	
    config.mixGain.mixOutCoarseGainL = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_l;	
    config.mixGain.mixInGainR = AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_r;	
    config.mixGain.mixOutGainR = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_r;	
    config.mixGain.mixOutCoarseGainR = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_r;
#endif		

    ((AUDDRV_PathID_t *)pData)->dlPathID = AUDDRV_HWControl_EnablePath(config);

    //UL
    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = 0;
    config.source = AUDDRV_GetDRVDeviceFromMic(mic);
    dev |= AUDDRV_MIC1;
    config.sink = AUDDRV_DEV_DSP;
    config.dmaCH = CSL_CAPH_DMA_NONE;    
    config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.snk_sampleRate = sample_rate;	
    config.chnlNum = AUDIO_CHANNEL_MONO;
    config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

    ((AUDDRV_PathID_t *)pData)->ulPathID = AUDDRV_HWControl_EnablePath(config);
    //If Dual Mic is enabled. Theoretically DMIC3 or DMIC4 are used
    //Here Let us assume it is DMIC3. It can be changed.
    if(AUDDRV_IsDualMicEnabled()==TRUE)
    {
    	dev |= AUDDRV_MIC2;
        config.streamID = AUDDRV_STREAM_NONE;
        config.pathID = 0;
        config.source = MIC_NOISE_CANCEL;
        config.sink = AUDDRV_DEV_DSP;
        config.dmaCH = CSL_CAPH_DMA_NONE;    
        config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        config.snk_sampleRate = sample_rate;	
        config.chnlNum = AUDIO_CHANNEL_MONO;
        config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

        ((AUDDRV_PathID_t *)pData)->ul2PathID = AUDDRV_HWControl_EnablePath(config);
    }
#ifdef CONFIG_AUDIO_BUILD
    //Config sidetone
    AUDDRV_SetHWSidetoneFilter(AUDDRV_GetAudioMode(),
				AUDIO_GetParmAccessPtr());
#endif				

#endif
#endif   

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
    AUDDRV_HWCTRL_CONFIG_t config;
	Log_DebugPrintf(LOGID_AUDIO, 
                    "\n\r\t* AUDDRV_Telephony_DeinitHW *\n\r");

    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))

    currSpkr = AUDDRV_SPKR_NONE;
    currSampleRate = AUDIO_SAMPLING_RATE_UNDEFINED;

    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = ((AUDDRV_PathID_t *)pData)->ulPathID;

    (void)AUDDRV_HWControl_DisablePath(config);

    if(AUDDRV_IsDualMicEnabled()==TRUE)
    {
        config.streamID = AUDDRV_STREAM_NONE;
        config.pathID = ((AUDDRV_PathID_t *)pData)->ul2PathID;

        (void)AUDDRV_HWControl_DisablePath(config);
    }

    currMic = AUDDRV_MIC_NONE;
    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = ((AUDDRV_PathID_t *)pData)->dlPathID;

    (void)AUDDRV_HWControl_DisablePath(config);

    // Disable sidetone.
    (void)AUDDRV_HWControl_DisableSideTone(AUDDRV_GetAudioMode());
	if(sink == AUDDRV_DEV_IHF)
	{
#ifdef RHEA_DSP_IHF_FEATURE		
		VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(FALSE,
							FALSE,
							FALSE);
#endif		
	}	
	sink = AUDDRV_DEV_NONE;
	audDev = 0;

#endif
#endif        
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
    AUDDRV_PathID pathID = 0;
	pathID = ((AUDDRV_PathID_t *)pData)->dlPathID;
    speaker = speaker; //speaker is not currently used.
    (void)AUDDRV_HWControl_MuteSink(pathID);
    return;
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
    AUDDRV_PathID pathID = 0;
	pathID = ((AUDDRV_PathID_t *)pData)->dlPathID;
    speaker = speaker; //speaker is not currently used.
    (void)AUDDRV_HWControl_UnmuteSink(pathID);
    return;
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
    AUDDRV_PathID pathID = 0;
	pathID = ((AUDDRV_PathID_t *)pData)->ulPathID;
    mic = mic; //mic is not currently used.
    (void)AUDDRV_HWControl_MuteSource(pathID);
    return;
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
    AUDDRV_PathID pathID = 0;
	pathID = ((AUDDRV_PathID_t *)pData)->ulPathID;
    mic = mic; //mic is not currently used.
    (void)AUDDRV_HWControl_UnmuteSource(pathID);
    return;
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
    AUDDRV_HWCTRL_CONFIG_t config;	
    AudioMode_t mode = AUDIO_MODE_HANDSET;
	UInt32 dev = 0;
#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))    
    UInt32 *memAddr = 0;

#endif
#endif      
    mode = mode; 
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr mic %d, spkr %d *\n\r", mic, speaker);

    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))    

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
	config.streamID = AUDDRV_STREAM_NONE;
	config.pathID = ((AUDDRV_PathID_t *)pData)->dlPathID;
	(void)AUDDRV_HWControl_DisablePath(config);
	((AUDDRV_PathID_t *)pData)->dlPathID = 0;

	if(AUDDRV_GetDRVDeviceFromSpkr(currSpkr) == AUDDRV_DEV_IHF)
	{
#ifdef CONFIG_AUDIO_BUILD	
#ifdef RHEA_DSP_IHF_FEATURE		
		memAddr = AUDIO_Return_IHF_48kHz_buffer_base_address();
#endif		
#endif
		AUDDRV_HWControl_SetDSPSharedMeMForIHF((UInt32)memAddr);
#ifdef RHEA_DSP_IHF_FEATURE		
		VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(FALSE,
							FALSE,
							FALSE);
#endif		
	}	
	
	//Enable the new speaker path
   	currSpkr = speaker;
	config.streamID = AUDDRV_STREAM_NONE;
	config.pathID = 0;
	config.source = AUDDRV_DEV_DSP;
	config.sink = AUDDRV_GetDRVDeviceFromSpkr(speaker);
	dev |= AUDDRV_SPEAKER;	
	config.dmaCH = CSL_CAPH_DMA_NONE;    
	//If DSP DL goes to IHF, Sample rate should be 48KHz.
	if (speaker == AUDDRV_SPKR_IHF)
	{
		config.source = AUDDRV_DEV_DSP_throughMEM;
    	config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
	}
	else
	{
    	config.src_sampleRate = currSampleRate;
	}
	config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;	

	if(config.sink == AUDDRV_DEV_HS)
	{
	    config.chnlNum = AUDIO_CHANNEL_STEREO;
	}
	else
	{
	    config.chnlNum = AUDIO_CHANNEL_MONO;
	}

	config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

	sink = config.sink;
	if(sink == AUDDRV_DEV_IHF)
	{
#ifdef CONFIG_AUDIO_BUILD	
#ifdef RHEA_DSP_IHF_FEATURE		
		memAddr = AUDIO_Return_IHF_48kHz_buffer_base_address();
#endif		
#endif
		AUDDRV_HWControl_SetDSPSharedMeMForIHF((UInt32)memAddr);
#ifdef RHEA_DSP_IHF_FEATURE		
		VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(TRUE,
							FALSE,
							FALSE);
#endif		
	}	
#ifdef CONFIG_AUDIO_BUILD	
	config.mixGain.mixInGainL = AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_l;	
	config.mixGain.mixOutGainL = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_l;	
	config.mixGain.mixOutCoarseGainL = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_l;	
	config.mixGain.mixInGainR = AUDIO_GetParmAccessPtr()[mode].srcmixer_input_gain_r;	
	config.mixGain.mixOutGainR = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_fine_gain_r;	
	config.mixGain.mixOutCoarseGainR = AUDIO_GetParmAccessPtr()[mode].srcmixer_output_coarse_gain_r;
#endif		
	
	((AUDDRV_PathID_t *)pData)->dlPathID = AUDDRV_HWControl_EnablePath(config);
#ifdef CONFIG_AUDIO_BUILD
	//Config sidetone
	AUDDRV_SetHWSidetoneFilter(AUDDRV_GetAudioMode(),
			AUDIO_GetParmAccessPtr());    
#endif			
    }

    if (mic != currMic)
    {
	    //Disable the existing mic path.	    
	    config.streamID = AUDDRV_STREAM_NONE;
	    config.pathID = ((AUDDRV_PathID_t *)pData)->ulPathID;
	    (void)AUDDRV_HWControl_DisablePath(config);
	    ((AUDDRV_PathID_t *)pData)->ulPathID = 0;

        if((AUDDRV_IsDualMicEnabled()==FALSE)
            &&(((AUDDRV_PathID_t *)pData)->ul2PathID != 0))
        {
            config.streamID = AUDDRV_STREAM_NONE;
            config.pathID = ((AUDDRV_PathID_t *)pData)->ul2PathID;
            (void)AUDDRV_HWControl_DisablePath(config);
            ((AUDDRV_PathID_t *)pData)->ul2PathID = 0;
        }


   	    currMic = mic;
        //UL
        config.streamID = AUDDRV_STREAM_NONE;
        config.pathID = 0;
        config.source = AUDDRV_GetDRVDeviceFromMic(mic);
		dev |= AUDDRV_MIC1;
        config.sink = AUDDRV_DEV_DSP;
        config.dmaCH = CSL_CAPH_DMA_NONE;    
        config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
        config.snk_sampleRate = currSampleRate;	
        config.chnlNum = AUDIO_CHANNEL_MONO;
        config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;
        ((AUDDRV_PathID_t *)pData)->ulPathID = AUDDRV_HWControl_EnablePath(config);

        //If Dual Mic is enabled. Theoretically DMIC3 or DMIC4 are used
        //Here Let us assume it is DMIC3. It can be changed.
        if((AUDDRV_IsDualMicEnabled()==TRUE)
            &&(((AUDDRV_PathID_t *)pData)->ul2PathID == 0))
        {
    	    dev |= AUDDRV_MIC2;
            config.streamID = AUDDRV_STREAM_NONE;
            config.pathID = 0;
            config.source = MIC_NOISE_CANCEL;
            config.sink = AUDDRV_DEV_DSP;
            config.dmaCH = CSL_CAPH_DMA_NONE;    
            config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
            config.snk_sampleRate = currSampleRate;	
            config.chnlNum = AUDIO_CHANNEL_MONO;
            config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;
            ((AUDDRV_PathID_t *)pData)->ul2PathID = AUDDRV_HWControl_EnablePath(config);
        }
    }
#endif
#endif      
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
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 0, 0, 0, 0 );
		}
		else
		{
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_DL, 1, 1, 0, 0, 0 );
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
// Function Name: AUDDRV_EnableDSPOutput
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
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 0, 0, 0, 0);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 0, 0, 0, 0 );
		}
		else
		{
			audio_control_dsp(DSPCMD_TYPE_AUDIO_CONNECT_UL, 1, 1, 0, 0, 0);
			audio_control_dsp(DSPCMD_TYPE_AUDIO_ENABLE, 1, 1, 0, 0, 0 );
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
// Function Name: AUDDRV_Disable_Output
//
// Description:   Disable audio output for voice call
//
//=============================================================================

void AUDDRV_Disable_Output ( AUDDRV_InOut_Enum_t  path )
{
}


//=============================================================================
//
// Function Name: AUDDRV_Enable_Input
//
// Description:   Enable audio input for voice call
//
//=============================================================================

void AUDDRV_Enable_Input (
                    AUDDRV_InOut_Enum_t      input_path,
                    AUDDRV_MIC_Enum_t        mic_selection,
					AUDIO_SAMPLING_RATE_t    sample_rate )
{
}


//=============================================================================
//
// Function Name: AUDDRV_Disable_Input
//
// Description:   Disable audio input for voice call
//
//=============================================================================

void AUDDRV_Disable_Input (  AUDDRV_InOut_Enum_t      path )
{
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
#ifdef CONFIG_AUDIO_BUILD
	SysAudioParm_t* pAudioParm;
	pAudioParm = AUDIO_GetParmAccessPtr();
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_mode==%d\n\r", audio_mode );
	// load DSP parameters:
	audio_control_generic( AUDDRV_CPCMD_PassAudioMode, 
				(UInt32)audio_mode, 0, 0, 0, 0 );
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, 
				(UInt32)audio_mode, 0, 0, 0, 0 );
	currAudioMode = audio_mode; // update mode

	AUDDRV_SetDSPFilter(audio_mode, dev, &(pAudioParm[audio_mode]));
#if 0
	if ( AUDDRV_GetVCflag() )
	{
		  // AUDIO_MODE_BLUETOOTH_WB should not be used for voice call. it should be convert to AUDIO_MODE_BLUETOOTH.
		if ( audio_mode >= AUDIO_MODE_NUMBER && (audio_mode != AUDIO_MODE_BLUETOOTH_WB) )
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
		else
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
	}
	else
	{
		if ( audio_mode >= AUDIO_MODE_NUMBER )  //wideband 
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_16000 );
		else
			AUDDRV_SetVoicePathSampRate( AUDIO_SAMPLING_RATE_8000 );
	}
#endif

#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))
	audDev = dev;
#endif
#endif

#else
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_SetAudioMode() audio_mode==%d\n\r", audio_mode );

    //kenren: this code needs to be re-implemented for CAPH. 
    //For now it is commented out.
	// load DSP parameters:
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, audio_mode, 0, 0, 0, 0 );
	
	currAudioMode = audio_mode; // update mode
#endif

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



/********************************************************************
*  @brief  Register up callback for getting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_getAudioMode( CB_GetAudioMode_t	cb )
{
//	client_GetAudioMode = cb;
}

/********************************************************************
*  @brief  Register up callback for setting audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setAudioMode( CB_SetAudioMode_t	cb )
{
//	client_SetAudioMode = cb;
}

/********************************************************************
*  @brief  Register up callback for setting music audio mode
*
*  @param  cb  (in)  callback function pointer
*
*  @return none
*
****************************************************************************/
void AUDDRV_RegisterCB_setMusicMode( CB_SetMusicMode_t	cb )
{
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
	return (AudioEqualizer_en_t)0;
}


//=============================================================================
//
// Function Name: AUDDRV_SetDSPFilter
//
// Description:   Set DSP UL and DL filter
//
//=============================================================================
#ifdef CONFIG_AUDIO_BUILD
void AUDDRV_SetDSPFilter( AudioMode_t audio_mode, 
        UInt32 dev,
		SysAudioParm_t* pAudioParm)
{
#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))
	if (eciEQOn == FALSE)
		audio_control_generic( AUDDRV_CPCMD_SetFilter, 
				audio_mode, dev, (UInt32)pAudioParm, 0, 0 );
#elif (!defined (FUSE_APPS_PROCESSOR) && defined (FUSE_COMMS_PROCESSOR))
		audio_control_generic( AUDDRV_CPCMD_SetFilter, 
				audio_mode, dev, (UInt32)pAudioParm, 0, 0 );
#endif
#endif
	return;
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
#endif
//=============================================================================
//
// Function Name: AUDDRV_SetHWGain
//
// Description:   Set HW Gain
//
//=============================================================================

void AUDDRV_SetHWGain(AUDDRV_HW_GAIN_e hw, UInt32 gain)
{
	AudioMode_t audio_mode = AUDIO_MODE_HANDSET;
	AUDDRV_DEVICE_e dev = AUDDRV_DEV_NONE;
	audio_mode = AUDDRV_GetAudioMode();
	if ((audio_mode == AUDIO_MODE_HANDSET)
		||(audio_mode == AUDIO_MODE_HANDSET_WB)
		||(audio_mode == AUDIO_MODE_HAC)
		||(audio_mode == AUDIO_MODE_HAC_WB))		
	{
		dev = AUDDRV_DEV_EP;
	}
	else
	if ((audio_mode == AUDIO_MODE_HEADSET)
		||(audio_mode == AUDIO_MODE_HEADSET_WB)
		||(audio_mode == AUDIO_MODE_TTY)
		||(audio_mode == AUDIO_MODE_TTY_WB))
		
	{
		dev = AUDDRV_DEV_HS;
	}
	else
	if ((audio_mode == AUDIO_MODE_SPEAKERPHONE)
		||(audio_mode == AUDIO_MODE_SPEAKERPHONE_WB))
	{
		dev = AUDDRV_DEV_IHF;
	}


	AUDDRV_HWControl_SetHWGain(hw, gain, dev);
	return;
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
#if 0

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
#endif	
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
#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))
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
#endif
#endif
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
#ifdef CONFIG_AUDIO_BUILD
    AudioMode_t mode = AUDIO_MODE_HANDSET;
    mode = AUDDRV_GetAudioMode();
    return (AUDIO_GetParmAccessPtr()[mode].dual_mic_enable != 0);
#else
	return FALSE; /* remove when above CONFIG_AUDIO_BUILD enabled */
#endif
}
#ifdef CONFIG_AUDIO_BUILD

//=============================================================================
//
// Function Name: AUDIO_GetParmAccessPtr
//
// Description:   Read audio sysparm address.
//
//=============================================================================
#if	defined(FUSE_APPS_PROCESSOR)
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return APSYSPARM_GetAudioParmAccessPtr();
#endif
}
#else
static SysAudioParm_t* AUDIO_GetParmAccessPtr(void)
{
#ifdef BSP_ONLY_BUILD
	return NULL;
#else
	return SYSPARM_GetAudioParmAccessPtr()->audio_parm;
#endif
}
#endif

#define AUDIOMODE_PARM_ACCESSOR(mode)	 AUDIO_GetParmAccessPtr()[mode]
#define AUDIOMODE_PARM_MM_ACCESSOR(mode) AUDIO_GetParmMMAccessPtr()[mode]
#endif


#if	defined(FUSE_COMMS_PROCESSOR)
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
*  Function Name: Result_t AUDDRV_HWControl_EnableSideTone(AUDDRV_PathID pathI)    
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
#endif

