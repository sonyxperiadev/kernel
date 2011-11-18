/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2009 - 2011  Broadcom Corporation                                                        */
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
* @file   audio_vdriver.c
* @brief  Audio VDriver API for CAPH
*
******************************************************************************/

//=============================================================================
// Include directives
//=============================================================================

//#include "osqueue.h"
//#include "msconsts.h"

#include "mobcom_types.h"
#include "resultcode.h"
#include "audio_consts.h"
#include "dspcmd.h"
#include "csl_apcmd.h"

#include "bcm_fuse_sysparm_CIB.h"
#include "ostask.h"
#include "osheap.h"
#include "log.h"
#include "csl_caph.h"
#include "csl_apcmd.h"
#include "csl_dsp.h"
#include "csl_caph_audioh.h"
#include "csl_caph_hwctrl.h"
#include "audio_vdriver.h"
#include <mach/comms/platform_mconfig.h>
#include "io.h"
#include "csl_dsp_cneon_api.h"
#if defined(ENABLE_DMA_VOICE)
#include "csl_dsp_caph_control_api.h"
#endif

#include "audio_pmu_adapt.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//=============================================================================
// Public Variable declarations
//=============================================================================

typedef void (*AUDDRV_User_CB) (UInt32 param1, UInt32 param2, UInt32 param3);

//=============================================================================
// Private Type and Constant declarations
//=============================================================================
#if defined(CONFIG_BCM_MODEM) 
#define AUDIO_MODEM(a) a
#else
#define AUDIO_MODEM(a)      
#endif

//static Boolean voiceInPathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN for both voiceIn and voiceOut
static Boolean voicePlayOutpathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN

static void *sUserCB = NULL;
static CSL_CAPH_DEVICE_e sink = CSL_CAPH_DEV_NONE;
static AUDIO_SINK_Enum_t currSpkr = AUDIO_SINK_UNDEFINED;
static AUDIO_SOURCE_Enum_t currMic = AUDIO_SOURCE_UNDEFINED;
static AUDIO_SAMPLING_RATE_t currSampleRate = AUDIO_SAMPLING_RATE_UNDEFINED;
static Boolean userEQOn = FALSE; // If TRUE, bypass EQ filter setting request from audio controller.

static Boolean bInVoiceCall = FALSE;
static AUDIO_SOURCE_Enum_t  currVoiceMic = AUDIO_SOURCE_UNDEFINED;	 //used in pcm i/f control. assume one mic, one spkr.
static AUDIO_SINK_Enum_t	currVoiceSpkr = AUDIO_SINK_UNDEFINED;	//used in pcm i/f control. assume one mic, one spkr.
static Boolean inVoiceCall = FALSE;
static Boolean bmuteVoiceCall = FALSE;

static Boolean controlFlagForCustomGain = FALSE;

typedef struct Audio_Driver_t
{
	UInt8					isRunning;
	Task_t					task;
	UInt32					taskID;
	Queue_t 				msgQueue;
} Audio_Driver_t;

static Audio_Driver_t sAudDrv = {0};
static UInt32 voiceCallSampleRate = 8000;  // defalut to 8K Hz

static Boolean IsBTM_WB = FALSE;  //this flag remembers if the Bluetooth headset is a wideband headset (16KHz voice)

static audio_codecId_handler_t codecId_handler = NULL;
static UInt8 audio_codecID = 6; //default CODEC ID = AMR NB


static Result_t AUDDRV_HWControl_SetFilter(AUDDRV_HWCTRL_FILTER_e filter, void* coeff);
static Result_t AUDDRV_HWControl_EnableSideTone(AudioMode_t audio_mode);

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

static void AUDDRV_Telephony_InitHW (AUDIO_SOURCE_Enum_t mic,
                                    AUDIO_SINK_Enum_t speaker,
                                    AUDIO_SAMPLING_RATE_t sample_rate,
                                    void * pData);

static void AUDDRV_Telephony_DeinitHW (void *pData);
static void AUDDRV_SetHWSidetoneFilter(AudioMode_t audio_mode,
		SysAudioParm_t* pAudioParm);
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr (AUDIO_SINK_Enum_t spkr);
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromMic (AUDIO_SOURCE_Enum_t mic);


//=============================================================================
// Functions
//=============================================================================


extern void Audio_InitRpc(void);

extern void VPU_Capture_Request(UInt16 buf_index);
extern void VPU_Render_Request(UInt16 bufferIndex);
extern Result_t AUDDRV_USB_HandleDSPInt(UInt16 arg0, UInt16 inBuf, UInt16 outBuf);
extern void ARM2SP_Render_Request(UInt16 buf_index);
extern void ARM2SP2_Render_Request(UInt16 buf_index);
extern void AP_ProcessStatusMainAMRDone(UInt16 codecType);
extern void VOIP_ProcessVOIPDLDone(void);
extern void AUDLOG_ProcessLogChannel(UInt16 audio_stream_buffer_idx);
extern void VOIF_Buffer_Request(UInt32 bufferIndex, UInt32 samplingRate);


//=============================================================================
//
// Function Name: AUDDRV_Init
//
// Description:   Inititialize audio system
//
//=============================================================================
void AUDDRV_Init( void )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Init *\n\r");

	if (sAudDrv.isRunning == TRUE)
		return;
	/* register DSP VPU status processing handlers */

	AUDIO_MODEM(CSL_RegisterVPUCaptureStatusHandler((VPUCaptureStatusCB_t)&VPU_Capture_Request);)
#if 0  // These features are not needed in LMP now.
	AUDIO_MODEM(CSL_RegisterVPURenderStatusHandler((VPURenderStatusCB_t)&VPU_Render_Request);)
	AUDIO_MODEM(CSL_RegisterUSBStatusHandler((USBStatusCB_t)&AUDDRV_USB_HandleDSPInt);)
#endif
	AUDIO_MODEM(CSL_RegisterVoIPStatusHandler((VoIPStatusCB_t)&VOIP_ProcessVOIPDLDone);)
	AUDIO_MODEM(CSL_RegisterMainAMRStatusHandler((MainAMRStatusCB_t)&AP_ProcessStatusMainAMRDone);)
	AUDIO_MODEM(CSL_RegisterARM2SPRenderStatusHandler((ARM2SPRenderStatusCB_t)&ARM2SP_Render_Request);)
	AUDIO_MODEM(CSL_RegisterARM2SP2RenderStatusHandler((ARM2SP2RenderStatusCB_t)&ARM2SP2_Render_Request);)
	AUDIO_MODEM(CSL_RegisterAudioLogHandler((AudioLogStatusCB_t)&AUDLOG_ProcessLogChannel);)
	AUDIO_MODEM(CSL_RegisterVOIFStatusHandler((VOIFStatusCB_t)&VOIF_Buffer_Request);)

	Audio_InitRpc();
	sAudDrv.isRunning = TRUE;
}

//*********************************************************************
//
//	 Shutdown audio driver task
//
//	 @return	void
//	 @note
//**********************************************************************/
void AUDDRV_Shutdown(void)
{
	if (sAudDrv.isRunning == FALSE)
		return;

	sAudDrv.isRunning = FALSE;
}


//=============================================================================
//
// Function Name: AUDDRV_Telephony_Init
//
// Description:   Initialize audio system for voice call
//
//=============================================================================

//Prepare DSP before turn on hardware audio path for voice call.
//	This is part of the control sequence for starting telephony audio.
void AUDDRV_Telephony_Init ( AUDIO_SOURCE_Enum_t	mic,
		AUDIO_SINK_Enum_t speaker,
			void *pData)
{
	///////////////////////////////////////////////////////////////////////
	// Phone Setup Sequence
	// 1. Init CAPH HW
	// 2. Send DSP command DSPCMD_TYPE_AUDIO_ENABLE
	// 3. If requires 48KHz for example in IHF mode, Send VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT
	//////////////////////////////////////////////////////////////////////

	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_Init");

	currVoiceMic = mic;
	currVoiceSpkr = speaker;
	pData = pData;

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Init AP	mic %d, spkr %d *\n\r", mic, speaker);
#else
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Init CP *\n\r");
#endif

	//control HW and flags at AP

	//at beginning
	inVoiceCall = TRUE;  //to prevent sending DSP Audio Enable when enable voice path.

	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );

	if( AUDDRV_GetAudioMode() >= AUDIO_MODE_NUMBER )
	{
		AUDDRV_Telephony_InitHW ( mic,
				speaker,
				AUDIO_SAMPLING_RATE_16000,
					pData);
#if defined(ENABLE_DMA_VOICE)
		csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_16000);
#endif
	} else {
		AUDDRV_Telephony_InitHW ( mic,
				speaker,
				AUDIO_SAMPLING_RATE_8000,
					pData);
#if defined(ENABLE_DMA_VOICE)
		csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_8000);
#endif
	}

	if (voiceCallSampleRate == 16000)
	{
		AUDDRV_SetAudioMode( AUDDRV_GetAudioMode(), AUDIO_APP_VOICE_CALL_WB );	//WB
	}
	else
	{
		AUDDRV_SetAudioMode( AUDDRV_GetAudioMode(), AUDIO_APP_VOICE_CALL );  //NB
	}

	if(speaker == AUDIO_SINK_LOUDSPK)
	{
#if defined(ENABLE_DMA_VOICE)
		//csl_dsp_caph_control_aadmac_disable_path((UInt16)(DSP_AADMAC_SEC_MIC_EN)|(UInt16)(DSP_AADMAC_SPKR_EN));
		csl_dsp_caph_control_aadmac_disable_path((UInt16)(DSP_AADMAC_SPKR_EN));
		csl_dsp_caph_control_aadmac_enable_path((UInt16)(DSP_AADMAC_PRI_MIC_EN));
#endif
		audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );

	// The dealy is to make sure DSPCMD_TYPE_AUDIO_ENABLE is done since it is a command via CP.
		OSTASK_Sleep(1);
		AUDIO_MODEM(VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(TRUE,FALSE,FALSE);)
	}
	else
	{
#if defined(ENABLE_DMA_VOICE)
		//csl_dsp_caph_control_aadmac_enable_path((UInt16)(DSP_AADMAC_PRI_MIC_EN)|(UInt16)(DSP_AADMAC_SEC_MIC_EN)|(UInt16)(DSP_AADMAC_SPKR_EN));
		csl_dsp_caph_control_aadmac_enable_path((UInt16)(DSP_AADMAC_PRI_MIC_EN)|(UInt16)(DSP_AADMAC_SPKR_EN));
#endif
		audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );

		// The dealy is to make sure DSPCMD_TYPE_AUDIO_ENABLE is done since it is a command via CP.
		OSTASK_Sleep(1);
	}

	//after AUDDRV_Telephony_InitHW to make SRST.
	AUDDRV_SetVCflag(TRUE);  //let HW control logic know.

#if defined(ENABLE_DMA_VOICE)
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, 0, 0, 0, 0 );
#else
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );
#endif
	OSTASK_Sleep( 40 );

#if defined(ENABLE_DMA_VOICE)
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, 0, 0, 0, 0 );
#else
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );
#endif
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, TRUE, TRUE, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );

	//Right now, Android HWdep code switches audio device by doing Disable and Enable,
	//therefore when switch audio device while muted we need to mute it here.
	//
	//for the next call, upper layer (HWdep) in Android unmute mic before start the next voice call.
	//
	//but in LMP (it has no Android code) the next call is also muted.
	//I will fixed it in the next commit. Basically when switch audio device in voice call better
	//use AUDDRV_Telephony_SelectMicSpkr( ) instead of calling TelephonyDeinit( ) and TelephonyInit( ).

	//printk("bmuteVoiceCall = %d \r\n", bmuteVoiceCall);
	if (bmuteVoiceCall == FALSE)
	{
		//printk("UnMute\r\n");
		audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
	}

	//per call basis: enable the DTX by calling stack api when call connected
	audio_control_generic( AUDDRV_CPCMD_ENABLE_DSP_DTX, TRUE, 0, 0, 0, 0 );

	if (speaker == AUDIO_SINK_BTM)
		AUDDRV_SetPCMOnOff( 1 );
	else
	{
		if(currVoiceMic != AUDIO_SOURCE_BTM) //need to check mic too.
			AUDDRV_SetPCMOnOff( 0 );
	}

	return;
}


//=============================================================================
//
// Function Name: AUDDRV_Telephony_RateChange
//
// Description:   Change the sample rate for voice call
//
//=============================================================================

void AUDDRV_Telephony_RateChange( UInt32 sampleRate )
{
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_RateChange, sampleRate = %d\n\r", sampleRate);

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_RateChange AP  *\n\r");
#else
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_RateChange CP *\n\r");
#endif

	voiceCallSampleRate = sampleRate;  //remember the rate for current call. (or for the incoming call in ring state.)

	if ( AUDDRV_GetVCflag() )
	{

	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );

	//need to know the mode!  need to set HW to 16Khz.
	AUDDRV_SetAudioMode( AUDDRV_GetAudioMode(), 0);

	//AUDDRV_Enable_Output (AUDDRV_VOICE_OUTPUT, speaker, TRUE, AUDIO_SAMPLING_RATE_8000);
#if defined(ENABLE_DMA_VOICE)
	if (AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ))
		csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_16000);
	else
		csl_dsp_caph_control_aadmac_set_samp_rate(AUDIO_SAMPLING_RATE_8000);
#else
	audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );
#endif
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, 0, 0, 0, 0 );

	//AUDDRV_Enable_Input ( AUDDRV_VOICE_INPUT, mic, AUDIO_SAMPLING_RATE_8000);

	OSTASK_Sleep( 40 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, TRUE, TRUE, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );

	if (bmuteVoiceCall == FALSE)
	{
		audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
	}

	}

	return;
}


//=============================================================================
//
// Function Name: AUDDRV_Telephone_GetSampleRate
//
// Description:   Get the sample rate for voice call
//
//=============================================================================
UInt32 AUDDRV_Telephone_GetSampleRate()
{
	return voiceCallSampleRate;
}

//=============================================================================
//
// Function Name: AUDDRV_Telephone_SetSampleRate
//
// Description:   Set the sample rate for voice call
//
//=============================================================================
void AUDDRV_Telephony_SetSampleRate(UInt32 samplerate)
{
	voiceCallSampleRate = samplerate;
}

//=============================================================================
//
// Function Name: AUDDRV_Telephony_Deinit
//
// Description:   DeInitialize audio system for voice call
//
//=============================================================================

//Prepare DSP before turn off hardware audio path for voice call.
// This is part of the control sequence for ending telephony audio.
void AUDDRV_Telephony_Deinit (void *pData)
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Deinit voicePlayOutpathEnabled = %d*\n\r", voicePlayOutpathEnabled);

	AUDDRV_SetVCflag(FALSE);  //let HW control logic know.

	// a quick fix not to disable voice path for speech playbck when end the phone call.
	if (voicePlayOutpathEnabled == FALSE)
	{
		  //per call basis: disable the DTX by calling stack api when call disconnected
		audio_control_generic( AUDDRV_CPCMD_ENABLE_DSP_DTX, FALSE, 0, 0, 0, 0 );

		audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, FALSE, FALSE, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, FALSE, 0, 0, 0, 0 );
		audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, FALSE, 0, 0, 0, 0 );

		if(currVoiceSpkr == AUDIO_SINK_LOUDSPK)
		{
			AUDIO_MODEM(VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(FALSE,FALSE,FALSE);)
		}

#if defined(ENABLE_DMA_VOICE)
		//csl_dsp_caph_control_aadmac_disable_path((UInt16)DSP_AADMAC_SPKR_EN | (UInt16)DSP_AADMAC_PRI_MIC_EN | (UInt16)DSP_AADMAC_SEC_MIC_EN);
		csl_dsp_caph_control_aadmac_disable_path((UInt16)DSP_AADMAC_SPKR_EN | (UInt16)DSP_AADMAC_PRI_MIC_EN); //no second mic on lmp
#endif
		audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );

		audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, FALSE, 0, 0, 0, 0 );

		OSTASK_Sleep( 3 ); //make sure audio is off, rtos does not have this.

		AUDDRV_Telephony_DeinitHW(pData);
	}

	if (AUDIO_MODE_BLUETOOTH == AUDDRV_GetAudioMode() )
		audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );

	//at last
	inVoiceCall = FALSE;

	return;
}


//=============================================================================
//
// Function Name: AUDDRV_Telephony_SelectMicSpkr
//
// Description:   Select the mic and speaker for voice call
//
//=============================================================================

void AUDDRV_Telephony_SelectMicSpkr (AUDIO_SOURCE_Enum_t mic,
                                    AUDIO_SINK_Enum_t speaker,
                                    void *pData)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	UInt32 dev = 0;
	UInt32 *memAddr = 0;

	mode = mode;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_SelectMicSpkr mic %d, spkr %d *\n\r", mic, speaker);

	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

	mode = AUDDRV_GetAudioMode();
	// The sequence from dsp is:
	// 1. first send digital sound cmd to dsp
	// 2. then config and start sspi
	//if( speaker == AUDIO_SINK_BTM || mic==AUDIO_SOURCE_BTM )
	if( mic==AUDIO_SOURCE_BTM )
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
		AUDIO_MODEM(VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(FALSE,FALSE,FALSE);)
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
	if (speaker == AUDIO_SINK_LOUDSPK)
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

	/***
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
	***/

	//already called from audio ctrl AUDCTRL_SetAudioMode( mode ); //this function also sets all HW gains.

	if(sink == CSL_CAPH_DEV_IHF)
	{
		AUDIO_MODEM(VPRIPCMDQ_ENABLE_48KHZ_SPEAKER_OUTPUT(TRUE,FALSE,FALSE);)  //integrate SDB CL 366484
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

		if(((AUDDRV_PathID_t *)pData)->ul2PathID != 0)
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
	//if( speaker == AUDIO_SINK_BTM || mic==AUDIO_SOURCE_BTM )
	if( mic==AUDIO_SOURCE_BTM )
	{
		//use audio_control_dsp( ), and combine this file with csl_aud_drv_hw.c
		AUDDRV_SetPCMOnOff( 1 );
	}
	else
	{
		AUDDRV_SetPCMOnOff( 0 );
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
			    AUDIO_SINK_Enum_t	   mixer_speaker_selection,
			    AUDIO_SAMPLING_RATE_t  sample_rate
			    )
{
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_EnableDSPOutput mixer %d, invoicecall %d, sample_rate %ld *\n\r", mixer_speaker_selection, inVoiceCall, sample_rate);

	OSTASK_Sleep( 5 );	//sometimes BBC video has no audio. This delay may help the mixer filter and mixer gain loading.
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
		if (currVoiceSpkr == AUDIO_SINK_BTM)
			AUDDRV_SetPCMOnOff( 1 );
		else
		{
			if(currVoiceMic != AUDIO_SOURCE_BTM) //need to check mic too.
			AUDDRV_SetPCMOnOff( 0 );
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
			  AUDIO_SOURCE_Enum_t	  mic_selection,
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

#if 0
	currVoiceMic = mic_selection;
	if (currVoiceMic == AUDIO_SOURCE_BTM)
		AUDDRV_SetPCMOnOff( 1 );
	else
	{
		if (currVoiceSpkr != AUDIO_SINK_BTM) //need to check spkr too.
			AUDDRV_SetPCMOnOff( 0 );
	}

#endif

}


//=============================================================================
//
// Function Name: AUDDRV_IsVoiceCallWB
//
// Description:   Return Wideband Voice Call status
//
//=============================================================================

Boolean AUDDRV_IsVoiceCallWB(AudioMode_t audio_mode)
{
	Boolean is_wb;
	if(audio_mode<AUDIO_MODE_NUMBER)
		is_wb = FALSE;
	else
		is_wb = TRUE;
	return is_wb;
}

//=============================================================================
//
// Function Name: AUDDRV_IsCall16K
//
// Description:   Return Voice Call 16KHz sample rate status
//
//=============================================================================

Boolean AUDDRV_IsCall16K(AudioMode_t voiceMode)
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


		case	AUDIO_MODE_BLUETOOTH: //BT headset needs to consider NB or WB too
		case	AUDIO_MODE_BLUETOOTH_WB: //BT headset needs to consider NB or WB too
			is_call16k = IsBTM_WB;
			break;

		default:
			break;
		}
	return is_call16k;
}
//=============================================================================
//
// Function Name: AUDDRV_InVoiceCall
//
// Description:   Return Voice Call status
//
//=============================================================================

Boolean AUDDRV_InVoiceCall( void )
{
	return inVoiceCall;
}


//=============================================================================
//
// Function Name: AUDDRV_SaveAudioMode
//
// Description:   save audio mode.
//
//=============================================================================

// intentionally put this static var here to prevent any code above this lines to use this var.
static AudioMode_t currAudioMode = AUDIO_MODE_HANDSET;	//need to update this on AP and also in audioapi.c on CP.
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
#include "audio_controller.h"

void AUDDRV_SetAudioMode( AudioMode_t audio_mode, UInt32 arg_dev )
{
	UInt16 gainTemp1=0, gainTemp2=0, gainTemp3=0, gainTemp4=0;

	int mixerInputGain; // Register value.
	int mixerOutputFineGain;  // Bit12:0, Output Fine Gain
	int mixerOutputBitSelect;
	int pmu_gain = 0;

	CSL_CAPH_PathID pathID = 0;
	CSL_CAPH_HWConfig_Table_t *path = NULL;
	CSL_CAPH_SRCM_MIX_OUTCHNL_e outChnl = CSL_CAPH_SRCM_CH_NONE;

	// load DSP parameters:
	//if ( audio_mode >= AUDIO_MODE_NUMBER )
	if ( audio_mode >= AUDIO_MODE_NUMBER_VOICE )
		return;  //error

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

	currAudioMode = audio_mode; // update mode
	//currMusicAudioMode = currAudioMode;

	audio_control_generic( AUDDRV_CPCMD_PassAudioMode,
				(UInt32)audio_mode, 0, 0, 0, 0 );
	audio_control_generic( AUDDRV_CPCMD_SetAudioMode,
				(UInt32)audio_mode, 0, 0, 0, 0 );

	if (userEQOn == FALSE)
	{
		//load speaker EQ filter and Mic EQ filter from sysparm to DSP
		audio_control_generic( AUDDRV_CPCMD_SetFilter, audio_mode, arg_dev, 0, 0, 0 );
	}
	//else
		//There is no need for this function to load the ECI-headset-provided speaker EQ filter and Mic EQ filter to DSP.
		//The ECI headset enable/disable request comes with the data. It means we'll get the coefficients every time they want to switch ECI headset on.
		//audio_cmf_filter((AudioCompfilter_t *) &copy_of_AudioCompfilter );

	//audDev = arg_dev;  //what for?


	//Load the mic gains from sysparm.

	/***
	do not touch DSP UL gain in this function.
	if(isDSPNeeded == TRUE)
	{
		dspULGain = 64; //AUDIO_GetParmAccessPtr()[mode].echoNlp_parms.echo_nlp_gain;
		audio_control_generic( AUDDRV_CPCMD_SetBasebandUplinkGain,
				dspULGain, 0, 0, 0, 0);
	}
	***/

	gainTemp1 = AUDIO_GetParmAccessPtr()[currAudioMode].mic_pga;  //Q13p2
	csl_caph_audioh_setMicPga_by_mB( ((int)gainTemp1)*25 );

	gainTemp1 = AUDIO_GetParmAccessPtr()[currAudioMode].amic_dga_coarse_gain; //Q13p2 dB
	gainTemp2 = AUDIO_GetParmAccessPtr()[currAudioMode].amic_dga_fine_gain; //Q13p2 dB

	gainTemp1 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic1_dga_coarse_gain;
	// dmic1_dga_coarse_gain is the same register as amic_dga_coarse_gain
	gainTemp2 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic1_dga_fine_gain;

	gainTemp3 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic2_dga_coarse_gain;
	gainTemp4 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic2_dga_fine_gain;

	csl_caph_audioh_vin_set_cic_scale_by_mB ( ((int)gainTemp1)*25,
		((int)gainTemp2)*25,
		((int)gainTemp3)*25,
		((int)gainTemp4)*25
		);

	gainTemp1 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic3_dga_coarse_gain;
	gainTemp2 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic3_dga_fine_gain;
	gainTemp3 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic4_dga_coarse_gain;
	gainTemp4 = AUDIO_GetParmAccessPtr()[currAudioMode].dmic4_dga_fine_gain;

	csl_caph_audioh_nvin_set_cic_scale_by_mB ( ((int)gainTemp1)*25,
		((int)gainTemp2)*25,
		((int)gainTemp3)*25,
		((int)gainTemp4)*25
		);


	//Load the speaker gains form sysparm.
/******
	Int16 dspDLGain = 0;

	// Set DSP DL gain from sysparm.
	if(isDSPNeeded == TRUE)
	{
		dspDLGain = 64; //AUDIO_GetParmAccessPtr()[mode].echo_nlp_downlink_volume_ctrl;
		audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkGain,
						dspDLGain, 0, 0, 0, 0);
	}
	********/

	//determine which mixer output to apply the gains to

	switch ( currAudioMode )
	{
	case AUDIO_MODE_HANDSET:
	case AUDIO_MODE_HANDSET_WB:
	case AUDIO_MODE_HAC:
	case AUDIO_MODE_HAC_WB:
		outChnl = CSL_CAPH_SRCM_STEREO_CH2_L;
		break;

	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_HEADSET_WB:
	case AUDIO_MODE_TTY:
	case AUDIO_MODE_TTY_WB:
		//outChnl = (CSL_CAPH_SRCM_STEREO_CH1_L | CSL_CAPH_SRCM_STEREO_CH1_R);
		outChnl = CSL_CAPH_SRCM_STEREO_CH1;
		break;

	case AUDIO_MODE_SPEAKERPHONE:
	case AUDIO_MODE_SPEAKERPHONE_WB:
		outChnl = CSL_CAPH_SRCM_STEREO_CH2_R;
		//for the case of Stereo_IHF
		outChnl = (CSL_CAPH_SRCM_STEREO_CH2_R | CSL_CAPH_SRCM_STEREO_CH2_L);
		break;

	case AUDIO_MODE_BLUETOOTH:
		//does it go through HW mixer gain?
		//outChnl = (CSL_CAPH_SRCM_STEREO_CH1_L | CSL_CAPH_SRCM_STEREO_CH1_R);
		outChnl = CSL_CAPH_SRCM_STEREO_CH1;
		break;

	default:
		break;
	}


	//Load HW Mixer gains from sysparm

	mixerInputGain = (int) AUDIO_GetParmAccessPtr()[currAudioMode].srcmixer_input_gain_l; //Q13p2 dB
	mixerInputGain = mixerInputGain*25; //into mB
	//mixerInputGain = (int) AUDIO_GetParmAccessPtr()[currAudioMode].srcmixer_input_gain_r;
	//mixerInputGain = mixerInputGain*25; //into mB

	//determine which which mixer input to apply the gains to

    //need to find the pathID then the mixer inputs,

	if (pathID != 0)
	{
		csl_caph_srcmixer_set_mix_in_gain( path->srcmRoute[0].inChnl, outChnl, mixerInputGain, mixerInputGain);
	}
	else
	{
		csl_caph_srcmixer_set_mix_all_in_gain( outChnl, mixerInputGain, mixerInputGain);
	}

	mixerOutputFineGain = (int) AUDIO_GetParmAccessPtr()[currAudioMode].srcmixer_output_fine_gain_l; //Q13p2 dB
	mixerOutputFineGain = mixerOutputFineGain*25; //into mB
	//mixerOutputFineGain = (int) AUDIO_GetParmAccessPtr()[currAudioMode].srcmixer_output_fine_gain_r;
	//mixerOutputFineGain = mixerOutputFineGain*25; //into mB


	mixerOutputBitSelect = (int) AUDIO_GetParmAccessPtr()[currAudioMode].srcmixer_output_coarse_gain_l; //Q13p2 dB
	mixerOutputBitSelect = mixerOutputBitSelect / 24; //into bit_shift
	//mixerOutputBitSelect = (int) AUDIO_GetParmAccessPtr()[currAudioMode].srcmixer_output_coarse_gain_r;
	//mixerOutputBitSelect = mixerOutputBitSelect / 24; //into bit_shift

	csl_caph_srcmixer_set_mix_out_bit_select(outChnl, mixerOutputBitSelect);
	csl_caph_srcmixer_set_mix_out_gain( outChnl, mixerOutputFineGain );


	//Load PMU gain from sysparm.
	switch ( currAudioMode )
	{
	case AUDIO_MODE_HANDSET:
	case AUDIO_MODE_HANDSET_WB:
	case AUDIO_MODE_HAC:
	case AUDIO_MODE_HAC_WB:
		break;

	case AUDIO_MODE_HEADSET:
	case AUDIO_MODE_HEADSET_WB:
	case AUDIO_MODE_TTY:
	case AUDIO_MODE_TTY_WB:
		//enum {
		//		PMU_AUDIO_HS_RIGHT,
		//		PMU_AUDIO_HS_LEFT,
		//		PMU_AUDIO_HS_BOTH
		//};

		pmu_gain = (int) AUDIO_GetParmAccessPtr()[currAudioMode].ext_speaker_pga_l; //Q13p2 dB
		SetGainOnExternalAmp_mB(AUDIO_SINK_HEADSET, pmu_gain*25, 1); //PMU_AUDIO_HS_LEFT);

		pmu_gain = (int) AUDIO_GetParmAccessPtr()[currAudioMode].ext_speaker_pga_r; //Q13p2 dB
		SetGainOnExternalAmp_mB(AUDIO_SINK_HEADSET, pmu_gain*25, 0); //PMU_AUDIO_HS_RIGHT);
		break;

	case AUDIO_MODE_SPEAKERPHONE:
	case AUDIO_MODE_SPEAKERPHONE_WB:

		pmu_gain = (int) AUDIO_GetParmAccessPtr()[currAudioMode].ext_speaker_pga_l; //Q13p2 dB
		SetGainOnExternalAmp_mB( AUDIO_SINK_LOUDSPK, pmu_gain*25, 0); //PMU_AUDIO_HS_BOTH);
		break;

	default:
		break;
	}
}


//=============================================================================
//
// Function Name: AUDDRV_SetMusicMode
//
// Description:   set audio mode.
//
//=============================================================================

void AUDDRV_SetMusicMode( AudioMode_t audio_mode) //add a second audio_mode for broadcast case
{
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
// Function Name: AUDDRV_User_CtrlDSP
//
// Description:   Control DSP Algo
//
//=============================================================================
int AUDDRV_User_CtrlDSP ( AudioDrvUserCtrl_t UserCtrlType, Boolean enable, Int32 size, void *param)
{
#if defined(CONFIG_BCM_MODEM)
	static Boolean ConfigSP = FALSE;
	static UInt32 *spCtrl = NULL, *spVar = NULL;

	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_User_CtrlDSP *\n\r");

	switch (UserCtrlType)
	{
		case AUDDRV_USER_SP_QUERY:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_SP_QUERY *\n\r");

			if (param == NULL)
				return -EINVAL;

			csl_dsp_sp_query_msg((UInt32 *)param);

			break;
		case AUDDRV_USER_SP_CTRL:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_SP_CTRL *\n\r");

			if (enable)
			{
				if (size <= 0 || param == NULL)
					return -EINVAL;

				if (spVar != NULL)
					csl_dsp_sp_cnfg_msg((UInt16)enable, 0, 1, (UInt32 *)param, spVar);
				else
				{
					spCtrl = (UInt32 *) kzalloc(size, GFP_KERNEL);
					if (!spCtrl)
						return -ENOMEM;

					memcpy(spCtrl, param, size);
					ConfigSP = TRUE;
				}
			}
			else
			{
				if (spVar != NULL)
					kfree(spVar);
				if (spCtrl != NULL)
					kfree(spCtrl);
				spVar = NULL;
				spCtrl = NULL;
				csl_dsp_sp_cnfg_msg((UInt16)enable, 0, 0, NULL, NULL);
			}
			break;
		case AUDDRV_USER_SP_VAR:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_SET_SP_VAR *\n\r");

			if (ConfigSP == TRUE && spCtrl != NULL)
			{
				if (param == NULL)
					return -EINVAL;

				csl_dsp_sp_cnfg_msg((UInt16)enable, 0, 1, spCtrl, (UInt32 *)param);
				ConfigSP = FALSE;
			}
			else
			{
				if (size <= 0 || param == NULL)
					return -EINVAL;

				if (spVar == NULL)
				{
					spVar = (UInt32 *) kzalloc(size, GFP_KERNEL);
					if (!spVar)
						return -ENOMEM;
					memcpy(spVar, param, size);
				}
				csl_dsp_sp_ctrl_msg((UInt32 *)param);
			}

			break;
		case AUDDRV_USER_EQ_CTRL:
			Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_UserCtrlDSP, AUDDRV_USER_EQ_CTRL *\n\r");

			if (enable == TRUE)
			{
				if (param == NULL)
					return -EINVAL;

				userEQOn = TRUE;
				audio_cmf_filter((AudioCompfilter_t *) param);
			}
			else
				userEQOn = FALSE;

			break;
		default:
			Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_User_CtrlDSP: Invalid request %d \n\r", UserCtrlType);
			break;
	}
#else
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_User_CtrlDSP : dummy for AP only");
#endif
	return 0;
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
// Function Name: AUDDRV_SetPCMOnOff
//
// Description: 	set PCM on/off for BT
//	this command will be removed from Rhea.
//
//=============================================================================

void AUDDRV_SetPCMOnOff(Boolean on_off)
{
	// By default the PCM port is occupied by trace port on development board
	if(on_off)
	{
		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);

	}
	else
	{
		audio_control_dsp(DSPCMD_TYPE_COMMAND_DIGITAL_SOUND, on_off, 0, 0, 0, 0);

	}
}


//=============================================================================
//
// Function Name: AUDDRV_ControlFlagFor_CustomGain
//
// Description:   Set a flag to allow custom gain settings.
//				  If the flag is set the above three parameters are not set
//				  in AUDDRV_SetAudioMode( ) .
//
//=============================================================================

void AUDDRV_ControlFlagFor_CustomGain( Boolean on_off )
{
	controlFlagForCustomGain = on_off;
}

//=============================================================================
//
// Function Name: Spkr AUDDRV_Telephony_MuteMic
//
// Description:   Mute mic for voice call
//
//=============================================================================

void AUDDRV_Telephony_MuteMic (AUDIO_SOURCE_Enum_t mic,
                              void *pData)
{
	bmuteVoiceCall = TRUE;
	//printk(" bmuteVoiceCall = TRUE\r\n");
	audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );
}

//=============================================================================
//
// Function Name: Spkr AUDDRV_Telephony_UnmuteMic
//
// Description:   UnMute mic for voice call
//
//=============================================================================

void AUDDRV_Telephony_UnmuteMic (AUDIO_SOURCE_Enum_t mic,
                                void *pData)
{
	bmuteVoiceCall = FALSE;
	//printk(" bmuteVoiceCall = FALSE\r\n");
	audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
}

//=============================================================================
//
// Function Name: AUDDRV_Telephony_MuteSpkr
//
// Description:   Mute speaker for voice call
//
//=============================================================================

void AUDDRV_Telephony_MuteSpkr (AUDIO_SINK_Enum_t speaker,
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

void AUDDRV_Telephony_UnmuteSpkr (AUDIO_SINK_Enum_t speaker,
                                  void *pData)
{
	audio_control_generic( AUDDRV_CPCMD_SetBasebandDownlinkUnmute, 0, 0, 0, 0, 0);
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
// Function Name: AUDDRV_IsDualMicEnable
//
// Description:   Check whether Dual Mic is enabled
//
//=============================================================================

Boolean AUDDRV_IsDualMicEnabled(void)
{
	AudioMode_t mode = AUDIO_MODE_HANDSET;
	mode = AUDDRV_GetAudioMode();
	return (AUDIO_GetParmAccessPtr()[mode].dual_mic_enable != 0);	//in parm_audio.txt, VOICE_DUALMIC_ENABLE
}

//*********************************************************************
/**
*	Get BTM headset NB or WB info

*	@return 	Boolean, TRUE for WB and FALSE for NB (8k)
*	@note
**********************************************************************/
Boolean AUDDRV_IsBTMWB( void )
{
	return IsBTM_WB;
}

//*********************************************************************
/**
*	Set BTM type

*	@param		Boolean isWB
*	@return 	none
*
*	@note	isWB=TRUE for BT WB headset; =FALSE for BT NB (8k) headset.
**********************************************************************/
void AUDDRV_SetBTMTypeWB( Boolean isWB)
{
	IsBTM_WB = isWB;
	//AUDDRV_SetPCMRate(IsBTM_WB);
}

//*********************************************************************
//
//	 Registers callback for rate change request
//
//	@param	   callback function
//	@return 	void
//	 @note
//**********************************************************************/

void AUDDRV_RegisterRateChangeCallback( audio_codecId_handler_t codecId_cb )
{
	Log_DebugPrintf(LOGID_SOC_AUDIO, "\n\r\t*  AUDDRV_RegisterRateChangeCallback, 0x%x\n\r", codecId_cb);
	codecId_handler = codecId_cb;
}

//*********************************************************************
//
//	 Post rate change message for telephony session
//
//	@param		codec ID
//	@return 	void
//	 @note
//**********************************************************************/
void AUDDRV_RequestRateChange(UInt8 codecID)
{
	if((audio_codecID != codecID) && (codecId_handler != NULL)) //if current codecID is same as new, ignore the request
	{
		audio_codecID = codecID;
		codecId_handler(codecID);
	}

}


//=============================================================================
// Private function prototypes
//=============================================================================


//=============================================================================
//
// Function Name: AUDDRV_Telephony_InitHW
//
// Description:   Enable the HW for Telephone voice call
//
//=============================================================================
static void AUDDRV_Telephony_InitHW (AUDIO_SOURCE_Enum_t mic,
                                    AUDIO_SINK_Enum_t speaker,
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
    if (speaker == AUDIO_SINK_LOUDSPK)
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

	/***

	 SetAudioMode() should already set HW gains from SYSPARM

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
    ***/

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
static void AUDDRV_Telephony_DeinitHW (void *pData)
{
    CSL_CAPH_HWCTRL_CONFIG_t config;
	Log_DebugPrintf(LOGID_AUDIO,
                    "\n\r\t* AUDDRV_Telephony_DeinitHW *\n\r");

    memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));

    currSpkr = AUDIO_SINK_UNDEFINED;
    currSampleRate = AUDIO_SAMPLING_RATE_UNDEFINED;

    // Disable sidetone.
    (void)AUDDRV_HWControl_DisableSideTone(AUDDRV_GetAudioMode());

    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = ((AUDDRV_PathID_t *)pData)->ulPathID;

    (void)csl_caph_hwctrl_DisablePath(config);

	if(((AUDDRV_PathID_t *)pData)->ul2PathID != 0)
    {
        config.streamID = CSL_CAPH_STREAM_NONE;
        config.pathID = ((AUDDRV_PathID_t *)pData)->ul2PathID;

        (void)csl_caph_hwctrl_DisablePath(config);
    }

    currMic = AUDIO_SOURCE_UNDEFINED;
    config.streamID = CSL_CAPH_STREAM_NONE;
    config.pathID = ((AUDDRV_PathID_t *)pData)->dlPathID;

    (void)csl_caph_hwctrl_DisablePath(config);

	sink = CSL_CAPH_DEV_NONE;

    return;
}


//=============================================================================
//
// Function Name: AUDDRV_SetHWSidetoneFilter
//
// Description:   Set HW Sidetone filter
//
//=============================================================================

static void AUDDRV_SetHWSidetoneFilter(AudioMode_t audio_mode,
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
	csl_caph_audioh_sidetone_set_gain(gain);
	AUDDRV_HWControl_EnableSideTone(audio_mode);

	return;
}


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
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromMic (AUDIO_SOURCE_Enum_t mic)
{
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromMic:: mic = 0x%x\n", mic);

      switch (mic)
      {
        case AUDIO_SOURCE_UNDEFINED:
            dev = CSL_CAPH_DEV_NONE;
            break;

         case AUDIO_SOURCE_SPEECH_DIGI:
         //case AUDDRV_DUAL_MIC_DIGI12:
         //case AUDDRV_DUAL_MIC_DIGI21:
            dev = CSL_CAPH_DEV_DIGI_MIC;
            break;

         case AUDIO_SOURCE_DIGI1:
            dev = CSL_CAPH_DEV_DIGI_MIC_L;
            break;

         case AUDIO_SOURCE_DIGI2:
            dev = CSL_CAPH_DEV_DIGI_MIC_R;
            break;

         case AUDIO_SOURCE_EANC_DIGI:
            dev = CSL_CAPH_DEV_EANC_DIGI_MIC;
            break;

         case AUDIO_SOURCE_ANALOG_MAIN:
            dev = CSL_CAPH_DEV_ANALOG_MIC;
            break;

         case AUDIO_SOURCE_ANALOG_AUX:
            dev = CSL_CAPH_DEV_HS_MIC;
            break;

         case AUDIO_SOURCE_BTM:
            dev = CSL_CAPH_DEV_BT_MIC;
            break;

         case AUDIO_SOURCE_USB:
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
static CSL_CAPH_DEVICE_e AUDDRV_GetDRVDeviceFromSpkr (AUDIO_SINK_Enum_t spkr)
{
	CSL_CAPH_DEVICE_e dev = CSL_CAPH_DEV_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDDRV_GetDRVDeviceFromSpkr:: spkr = 0x%x\n", spkr);

      switch (spkr)
      {
        case AUDIO_SINK_UNDEFINED:
            dev = CSL_CAPH_DEV_NONE;
            break;

         case AUDIO_SINK_HANDSET:
            dev = CSL_CAPH_DEV_EP;
            break;

         case AUDIO_SINK_LOUDSPK:
            dev = CSL_CAPH_DEV_IHF;
            break;

         case AUDIO_SINK_HEADSET:
            dev = CSL_CAPH_DEV_HS;
            break;

         case AUDIO_SINK_VIBRA:
            dev = CSL_CAPH_DEV_VIBRA;
            break;

         case AUDIO_SINK_BTM:
            dev = CSL_CAPH_DEV_BT_SPKR;
            break;

         case AUDIO_SINK_USB:
            dev = CSL_CAPH_DEV_MEMORY;
            break;

        default:
			break;
    };

	return dev;
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

