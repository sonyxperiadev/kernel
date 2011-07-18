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
* @file   audio_vdriver_common.c
* @brief  Audio VDriver API common for all HW platforms.
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
#define SYSCFG_BASE_ADDR      0x08880000      /* SYSCFG core */
#include "shared.h"
#include "dspcmd.h"

#include "ripisr.h"
#include "osqueue.h"
#include "msconsts.h"

#include "audio_consts.h"
//#include "ripcmdq.h"
#include "drv_caph.h"
#include "drv_caph_hwctrl.h"
#include "audio_vdriver.h"
//#include "sysparm.h"
#include "ostask.h"
#include "audioapi_asic.h"
#include "log.h"
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))
#include "csl_dsp.h"
#endif

extern void VPU_Capture_Request(UInt16 bufferIndex);
extern void VPU_Render_Request(UInt16 bufferIndex);
extern Result_t AUDDRV_USB_HandleDSPInt(UInt16 arg0, UInt16 inBuf, UInt16 outBuf);
extern void VOIF_ISR_Handler (UInt32 bufferIndex, UInt32 samplingRate);
extern void ARM2SP_Render_Request(UInt16 bufferPosition);
extern void ARM2SP2_Render_Request(UInt16 bufferPosition);
extern void AP_ProcessStatusMainAMRDone(UInt16 codecType);
extern void VOIP_ProcessVOIPDLDone(void);
extern void AUDDRV_User_HandleDSPInt(UInt32 param1, UInt32 param2, UInt32 param3);
extern void AUDLOG_ProcessLogChannel(UInt16 audio_stream_buffer_idx);

extern AUDDRV_MIC_Enum_t   currVoiceMic;   //used in pcm i/f control. assume one mic, one spkr.
extern AUDDRV_SPKR_Enum_t  currVoiceSpkr;  //used in pcm i/f control. assume one mic, one spkr.
extern Boolean inVoiceCall;
//=============================================================================
// Private Type and Constant declarations
//=============================================================================

Boolean voicePlayOutpathEnabled = FALSE;  //this is needed because DSPCMD_AUDIO_ENABLE sets/clears AMCR.AUDEN
Boolean controlFlagForCustomGain = FALSE;

typedef struct Audio_Driver_t
{
    UInt8                   isRunning;
    Task_t                  task;
    UInt32                  taskID;
    Queue_t                 msgQueue;
} Audio_Driver_t;

static Audio_Driver_t sAudDrv = {0};
static UInt32 voiceCallSampleRate = 8000;  // defalut to 8K Hz
//=============================================================================
// Private function prototypes
//=============================================================================
static void AudDrv_TaskEntry(void);

//=============================================================================
// Functions
//=============================================================================


extern void Audio_InitRpc(void);


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

    // create queue and task
    sAudDrv.msgQueue = OSQUEUE_Create( QUEUESIZE_AUDDRV,
                                    sizeof(AUDDRV_REQUEST_MSG_t), OSSUSPEND_PRIORITY );
    OSQUEUE_ChangeName(sAudDrv.msgQueue, "AudDrvQ");

    sAudDrv.task = OSTASK_Create( (TEntry_t) AudDrv_TaskEntry,
                                TASKNAME_AUDDRV,
                                TASKPRI_AUDDRV,
                                STACKSIZE_AUDDRV
                                );
#if !((defined(_RHEA_)||defined(_SAMOA_)))
    AUDDRV_SPKRInit (AUDDRV_SPKR_EP, AUDIO_SPKR_CHANNEL_DIFF);  //Purpose: to initialize the CHAL Audio code with AHB_AUDIO_BASE_ADDR, SYSCFG_BASE_ADDR, AUXMIC_BASE_ADDR.
#endif

#if defined(FUSE_APPS_PROCESSOR)

	/* register DSP VPU status processing handlers */
#ifndef _SAMOA_
	CSL_RegisterVPUCaptureStatusHandler((VPUCaptureStatusCB_t)&VPU_Capture_Request);
#endif	
#ifdef CONFIG_AUDIO_BUILD
	CSL_RegisterVPURenderStatusHandler((VPURenderStatusCB_t)&VPU_Render_Request);
	CSL_RegisterUSBStatusHandler((USBStatusCB_t)&AUDDRV_USB_HandleDSPInt);
	CSL_RegisterVOIFStatusHandler((VOIFStatusCB_t)&VOIF_ISR_Handler);
	CSL_RegisterVoIPStatusHandler((VoIPStatusCB_t)&VOIP_ProcessVOIPDLDone);
	CSL_RegisterMainAMRStatusHandler((MainAMRStatusCB_t)&AP_ProcessStatusMainAMRDone);
#endif	
#ifndef _SAMOA_
	CSL_RegisterARM2SPRenderStatusHandler((ARM2SPRenderStatusCB_t)&ARM2SP_Render_Request);
	CSL_RegisterARM2SP2RenderStatusHandler((ARM2SP2RenderStatusCB_t)&ARM2SP2_Render_Request);
#endif
#if defined(ENABLE_SPKPROT)
	CSL_RegisterUserStatusHandler((UserStatusCB_t)&AUDDRV_User_HandleDSPInt);
#endif
	CSL_RegisterAudioLogHandler((AudioLogStatusCB_t)&AUDLOG_ProcessLogChannel);

	AUDDRV_RegisterCB_getAudioMode( (CB_GetAudioMode_t) &AUDDRV_GetAudioMode );
	AUDDRV_RegisterCB_setAudioMode( (CB_SetAudioMode_t) &AUDDRV_SetAudioMode );
	AUDDRV_RegisterCB_setMusicMode( (CB_SetMusicMode_t) &AUDDRV_SetMusicMode );
#if defined(USE_NEW_AUDIO_PARAM)
	AUDDRV_RegisterCB_getAudioApp( (CB_GetAudioApp_t) &AUDDRV_GetAudioApp );
#endif
	Audio_InitRpc();
#else  //#if defined(FUSE_APPS_PROCESSOR)
	Audio_InitRpc();

	//split file to CP and AP.
	{
	UInt8 cur_mode = (UInt8) AUDDRV_GetAudioMode();
#ifdef CONFIG_AUDIO_BUILD
#if defined(USE_NEW_AUDIO_PARAM)
	UInt8 cur_app = 0; 
	UInt16 voice_vol_init_in_dB = SYSPARM_GetAudioParamsFromFlash( cur_mode, cur_app )->voice_volume_init;  //dB
#else
	UInt16 voice_vol_init_in_dB = SYSPARM_GetAudioParamsFromFlash( cur_mode )->voice_volume_init;  //dB
#endif
#endif
	VOLUMECTRL_SetBasebandVolume( voice_vol_init_in_dB, 0, 0, 0 );  //param4 is OmegaVoice volume step
	}

#if !defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE)
	AUDDRV_RegisterCB_getAudioMode( (CB_GetAudioMode_t) &AUDDRV_GetAudioMode );
	AUDDRV_RegisterCB_setAudioMode( (CB_SetAudioMode_t) &AUDDRV_SetAudioMode );
	AUDDRV_RegisterCB_setMusicMode( (CB_SetMusicMode_t) &AUDDRV_SetMusicMode );
#if defined(USE_NEW_AUDIO_PARAM)
	AUDDRV_RegisterCB_getAudioApp( (CB_GetAudioApp_t) &AUDDRV_GetAudioApp );
#endif
#else
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Init : Registering CP_Audio_ISR_Handler*\n\r");
	RIPISR_Register_AudioISR_Handler( (Audio_ISR_Handler_t) &CP_Audio_ISR_Handler );
#endif

#endif	
	sAudDrv.isRunning = TRUE;
}

//*********************************************************************
//
//   the audio driver task entry
//
//   @return   void             Description
//   @note     
//**********************************************************************/
static void AudDrv_TaskEntry()
{
    AUDDRV_REQUEST_MSG_t    reqMsg;
    AudioMode_t     mode;
    UInt32          dev = 0;
    OSStatus_t      status;

    while(TRUE)
    {
        status = OSQUEUE_Pend( sAudDrv.msgQueue, (QMsg_t *)&reqMsg, TICKS_FOREVER );
        if (status == OSSTATUS_SUCCESS)
        {
            Log_DebugPrintf(LOGID_AUDIO,"AudDrv_TaskEntry:: reqID = 0x%x\n", reqMsg.reqID);

            switch (reqMsg.reqID)
            {
                case AUDDRV_RATE_CHANGE_REQ:

					if (inVoiceCall == TRUE)
					{
                    	mode = AUDDRV_GetAudioMode();
                    	// 0x0A as per 3GPP 26.103 Sec 6.3 indicates AMR WB  AUDIO_ID_CALL16k
                    	// 0x06 indicates AMR NB
                    	if ( reqMsg.param1 == 0x0A ) // AMR-WB
                    	{
                        	voiceCallSampleRate = 16000;
                        	if (mode < AUDIO_MODE_NUMBER)
                            	mode = (AudioMode_t)(mode + AUDIO_MODE_NUMBER);
                    	}
                    	else  // AMR-NB
                    	{
                        	voiceCallSampleRate = 8000;
                        	if (mode >= AUDIO_MODE_NUMBER)
                            	mode = (AudioMode_t)(mode - AUDIO_MODE_NUMBER);
                    	}
						dev = AUDDRV_GetAudioDev();
                    	AUDDRV_SetAudioMode(mode, dev);
                    	AUDDRV_Telephony_RateChange(voiceCallSampleRate);
					}
					else // Not in voice call yet, audio mode will be set during path setup, as dev is not available
					{
                    	// 0x0A as per 3GPP 26.103 Sec 6.3 indicates AMR WB  AUDIO_ID_CALL16k
                    	// 0x06 indicates AMR NB
                    	if ( reqMsg.param1 == 0x0A ) // AMR-WB
                        	voiceCallSampleRate = 16000;
                    	else  // AMR-NB
                        	voiceCallSampleRate = 8000;
					}

					break;

                default:
                    break;
            }
        }
    }
}

//*********************************************************************
//
//   Shutdown audio driver task
//   
//   @return    void
//   @note                        
//**********************************************************************/
void AUDDRV_Shutdown(void)
{   
    if (sAudDrv.isRunning == FALSE)
        return;

    OSTASK_Destroy(sAudDrv.task);

    OSQUEUE_Destroy(sAudDrv.msgQueue);

    sAudDrv.isRunning = FALSE;
}

//*********************************************************************
//
//   Post rate change message for telephony session
//
//  @param      codec ID
//  @return     void
//   @note     
//**********************************************************************/
void AUDDRV_RequestRateChange(UInt8 codecID)
{
    AUDDRV_REQUEST_MSG_t reqMsg;

    reqMsg.reqID = AUDDRV_RATE_CHANGE_REQ;
    reqMsg.param1 = codecID;
    reqMsg.param2 = 0;
    reqMsg.param3 = 0;

    Log_DebugPrintf(LOGID_AUDIO,"AUDDRV_RequestRateChange\n");
    OSQUEUE_Post(sAudDrv.msgQueue, (QMsg_t*)&reqMsg, TICKS_FOREVER);
}
// CSL driver will send a DSP_ENABLE_DIGITAL_SOUND?? cmd to DSP, 
// But ARM code (audio controller) turns on/off PCM interface.

//=============================================================================
//
// Function Name: AUDDRV_Telephony_Init
//
// Description:   Initialize audio system for voice call
//
//=============================================================================

//Prepare DSP before turn on hardware audio path for voice call.
//  This is part of the control sequence for starting telephony audio.
void AUDDRV_Telephony_Init ( AUDDRV_MIC_Enum_t  mic, 
		AUDDRV_SPKR_Enum_t speaker,
	        void *pData)
{
	Log_DebugPrintf(LOGID_AUDIO, "AUDDRV_Telephony_Init");

	currVoiceMic = mic;
	currVoiceSpkr = speaker;
	pData = pData;

#if defined(FUSE_APPS_PROCESSOR)&&!defined(BSP_ONLY_BUILD)
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* AUDDRV_Telephony_Init AP  mic %d, spkr %d *\n\r", mic, speaker);
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

	audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );
	
	if( AUDDRV_GetAudioMode() >= AUDIO_MODE_NUMBER )
		AUDDRV_Telephony_InitHW ( mic, 
				speaker, 
				AUDIO_SAMPLING_RATE_16000,
			      	pData);
	else
		AUDDRV_Telephony_InitHW ( mic, 
				speaker, 
				AUDIO_SAMPLING_RATE_8000,
			       	pData);

	//after AUDDRV_Telephony_InitHW to make SRST.
	AUDDRV_SetVCflag(TRUE);  //let HW control logic know.

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );

	OSTASK_Sleep( 40 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0, 0 );

	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, TRUE, TRUE, 0, 0, 0 );
#if !(defined(_SAMOA_))
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
#endif
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );

#if !(defined(_SAMOA_))
    //per call basis: enable the DTX by calling stack api when call connected
	audio_control_generic( AUDDRV_CPCMD_ENABLE_DSP_DTX, TRUE, 0, 0, 0, 0 );

	if (speaker == AUDDRV_SPKR_PCM_IF)
		AUDDRV_SetPCMOnOff( 1 );
		//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, TRUE, 0, 0, 0, 0 );
	else
	{
		if(currVoiceMic != AUDDRV_MIC_PCM_IF) //need to check mic too.
			AUDDRV_SetPCMOnOff( 0 );
			//audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );
	}
#endif
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
	audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, TRUE, 0, AUDDRV_IsCall16K( AUDDRV_GetAudioMode() ), 0, 0 );

	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_DL, TRUE, 0, 0, 0, 0 );

	//AUDDRV_Enable_Input ( AUDDRV_VOICE_INPUT, mic, AUDIO_SAMPLING_RATE_8000);
			
	OSTASK_Sleep( 40 );
	
	audio_control_dsp( DSPCMD_TYPE_AUDIO_CONNECT_UL, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_EC_NS_ON, TRUE, TRUE, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_DUAL_MIC_ON, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff, TRUE, 0, 0, 0, 0 );
	audio_control_dsp( DSPCMD_TYPE_UNMUTE_DSP_UL, 0, 0, 0, 0, 0 );
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
	
		audio_control_dsp( DSPCMD_TYPE_AUDIO_ENABLE, FALSE, 0, 0, 0, 0 );

		audio_control_dsp( DSPCMD_TYPE_MUTE_DSP_UL, 0, 0, 0, 0, 0 );

		OSTASK_Sleep( 3 ); //make sure audio is off

		AUDDRV_Telephony_DeinitHW(pData);
	}

	if (AUDIO_CHNL_BLUETOOTH == AUDDRV_GetAudioMode() )
		audio_control_dsp( DSPCMD_TYPE_AUDIO_SET_PCM, FALSE, 0, 0, 0, 0 );

	//at last
	inVoiceCall = FALSE;

	return;
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

