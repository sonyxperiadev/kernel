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
//#include "ripcmdq.h"
#include "ripisr.h"
#include "audio_consts.h"
//#include "sysparm.h"
#include "ostask.h"
#include "audioapi_asic.h"
#include "audio_ipc.h"
#include "log.h"
#include "csl_caph.h"
#include "csl_apcmd.h"
#include "drv_caph.h"
#include "drv_audio_common.h"
#include "drv_caph_hwctrl.h"
#include "audio_vdriver.h"
#include "platform_mconfig_rhea.h"

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

typedef void (*AUDDRV_User_CB) (UInt32 param1, UInt32 param2, UInt32 param3);

AUDDRV_MIC_Enum_t   currVoiceMic = AUDDRV_MIC_NONE;   //used in pcm i/f control. assume one mic, one spkr.
AUDDRV_SPKR_Enum_t  currVoiceSpkr = AUDDRV_SPKR_NONE;  //used in pcm i/f control. assume one mic, one spkr.
Boolean inVoiceCall = FALSE;

//=============================================================================
// Private Type and Constant declarations
//=============================================================================


static Boolean voiceInPathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut
static Boolean voicePlayOutpathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN

//static AudioEqualizer_en_t	sEqualizerType = EQU_NORMAL;
static void *sUserCB = NULL;

static AUDDRV_PathID ULPathID = 0;
static AUDDRV_PathID DLPathID = 0;

static Boolean bInVoiceCall = FALSE;

static CB_GetAudioMode_t  client_GetAudioMode = NULL;
static CB_SetAudioMode_t  client_SetAudioMode = NULL;

//=============================================================================
// Private function prototypes
//=============================================================================


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
				  AUDIO_SAMPLING_RATE_t		sample_rate)
{
    AUDDRV_HWCTRL_CONFIG_t config;
	Log_DebugPrintf(LOGID_AUDIO, 
                    "\n\r\t* AUDDRV_Telephony_InitHW mic=%d, spkr=%d sample_rate=%ld*\n\r", 
                    mic, speaker, sample_rate);

    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));

#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))
    
    //DL
    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = 0;
    config.sink = AUDDRV_GetDRVDeviceFromSpkr(speaker);
    config.dmaCH = CSL_CAPH_DMA_NONE;    
    config.src_sampleRate = sample_rate;
    config.snk_sampleRate = AUDIO_SAMPLING_RATE_48000;	
    config.chnlNum = AUDIO_CHANNEL_MONO;
    config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;
	if(speaker == AUDDRV_SPKR_IHF)
	{
        // special path for IHF voice call 
        // need to use the physical address  
        AP_SharedMem_t *ap_shared_mem_ptr = SHAREDMEM_GetDsp_SharedMemPtr();
        UInt32 *memAddr = AP_SH_BASE + ((UInt32)&(ap_shared_mem_ptr->shared_aud_out_buf_48k[0][0])
                                    - (UInt32)ap_shared_mem_ptr);

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

    DLPathID = AUDDRV_HWControl_EnablePath(config);

    //UL
    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = 0;
    config.source = AUDDRV_GetDRVDeviceFromMic(mic);
    config.sink = AUDDRV_DEV_DSP;
    config.dmaCH = CSL_CAPH_DMA_NONE;    
    config.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
    config.snk_sampleRate = sample_rate;	
    config.chnlNum = AUDIO_CHANNEL_MONO;
    config.bitPerSample = AUDIO_24_BIT_PER_SAMPLE;

    ULPathID = AUDDRV_HWControl_EnablePath(config);
#endif
#endif   

    // Set new filter coef.
    AUDDRV_SetAudioMode( AUDDRV_GetAudioMode());
 
    return;
}




//=============================================================================
//
// Function Name: AUDDRV_Telephony_DeinitHW
//
// Description:   Disable the HW for Telephone voice call
//
//=============================================================================
void AUDDRV_Telephony_DeinitHW (void)
{
    AUDDRV_HWCTRL_CONFIG_t config;
	Log_DebugPrintf(LOGID_AUDIO, 
                    "\n\r\t* AUDDRV_Telephony_DeinitHW *\n\r");

    memset(&config, 0, sizeof(AUDDRV_HWCTRL_CONFIG_t));
#if defined (FUSE_DUAL_PROCESSOR_ARCHITECTURE)
#if (defined (FUSE_APPS_PROCESSOR) && !defined (FUSE_COMMS_PROCESSOR))

    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = ULPathID;

    (void)AUDDRV_HWControl_DisablePath(config);
    ULPathID = 0;

    config.streamID = AUDDRV_STREAM_NONE;
    config.pathID = DLPathID;

    (void)AUDDRV_HWControl_DisablePath(config);
	VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(FALSE,
							FALSE,
							FALSE);
    DLPathID = 0;
#endif
#endif        
    return;
}



//=============================================================================
//
// Function Name: AUDDRV_Telephony_SelectMicSpkr
//
// Description:   Select the mic and speaker for voice call
//
//=============================================================================


void AUDDRV_Telephony_SelectMicSpkr (
					AUDDRV_MIC_Enum_t			mic,
					AUDDRV_SPKR_Enum_t	speaker )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr mic %d, spkr %d *\n\r", mic, speaker);
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

		if (currVoiceSpkr == AUDDRV_SPKR_PCM_IF)
			AUDDRV_SetPCMOnOff( 1 );
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
		else
		{
			if(currVoiceMic != AUDDRV_MIC_PCM_IF) //need to check mic too.
			AUDDRV_SetPCMOnOff( 0 );
			//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
		}
				
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
		voiceInPathEnabled = TRUE;
	}

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

    //kenren: this code needs to be re-implemented for CAPH. 
    //For now it is commented out.
	// load DSP parameters:
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode, audio_mode, 0, 0, 0, 0 );
	
	currAudioMode = audio_mode; // update mode

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
	client_GetAudioMode = cb;
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
	client_SetAudioMode = cb;
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



