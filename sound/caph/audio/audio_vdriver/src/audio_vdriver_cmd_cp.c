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
*   @file   audio_vdriver_cmd_cp.c
*
*   @brief  
*
****************************************************************************/

#if (    ( defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) ) \
      || ( !defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) && !defined(MIN_BSP_BUILD) )  )
               //COMMS,  APPS_PLUS.  yes.
               //APBsp_Plus??  no.
               //ATHENA_EDGE_CP_STANDALONE build, yes


#include "mobcom_types.h"
#include "resultcode.h"

#include "audio_consts.h"
#include "auddrv_def.h"
#include "tones_def.h"
#include "audioapi_asic.h"
#include "sharedmem.h"
#include "dspcmd.h"
#include "ripcmdq.h"
#include "volumectrl.h"
#include "audio_vdriver.h"
#include "log.h"

#include "audioapi_asic.h"
#include "audio_vdriver.h"
#include "volumectrl.h"
#include "sysparm.h"
#include "audio_manager_commons.h"
#include "vpu.h"

#include "xassert.h"


//speech ul voice recording gain control
#define SPEECH_REC_GAIN_UL_DEFAULT 16384 //in Q1.14 format

void AUDIO_UpdateAudioMode(AudioMode_t mode);
void AUDIO_Turn_UL_CompanderOnOff(Boolean on_off);
void AUDIO_SetULDigitalGain( UInt8 gain);

extern void Voice_EnableDTX (Boolean enable);

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	audio_control_generic
//!
//! Description:	Used for AP controlling CP not covered by regular control
//!
//! Notes: param1=case id, param2 ~ param4 = control params
//! 
/////////////////////////////////////////////////////////////////////////////

UInt32 audio_control_generic(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4, UInt32 param5, UInt32 param6)
{
 	UInt8	digital_gain_dB = 0;
	UInt32 resp=0;

	switch (param1)
	{
		case AUDDRV_CPCMD_SetBasebandVolume:

			digital_gain_dB = param2;
			VOLUMECTRL_SetBasebandVolume( digital_gain_dB, param4, 0, 0 );  //param4 is OmegaVoice volume step

			//DL voice volume contorl in DSP does not have effect on WB-AMR playback's output.
			//as a fix solution, here, set WB-AMR playback volume.
			VOLUMECTRL_Set_WB_AMR_output_gain( digital_gain_dB, 0, 0, 0 );

			break;

		case AUDDRV_CPCMD_SetBasebandDownlinkMute:
			VOLUMECTRL_MuteRx( );
			break;

		case AUDDRV_CPCMD_SetBasebandDownlinkUnmute:
			VOLUMECTRL_UnMuteRx( );
			break;

		case AUDDRV_CPCMD_SetBasebandUplinkGain:
			VOLUMECTRL_SetDigitalTxGain( (Int16) (param2 - 100) );  //param2 in dB
			break;

		case AUDDRV_CPCMD_SetBasebandDownlinkGain:
			VOLUMECTRL_SetDigitalRxGain( (Int16) (param2 - 100) );  //param2 in dB
			break;

		case AUDDRV_CPCMD_SetAudioMode:
			{
				AudioMode_t  currAudioMode;

				currAudioMode = AUDIO_GetAudioMode();

				AUDIO_ASIC_SetAudioMode( (AudioMode_t)param2 );
				//the following logic exists to accomodate current DSP.
				if( param2 == AUDIO_MODE_TTY )
				{	//switch to TTY mode
					AUDIO_SetInternalCTM( TRUE );
				}
				else
				{	if( currAudioMode == AUDIO_MODE_TTY )
						//switch away from TTY mode.
						AUDIO_SetInternalCTM( FALSE );
				}
			}
			break;

		case AUDDRV_CPCMD_READ_AUDVOC_AEQMODE: //read audvoc_aeqMode
			resp = (UInt32) AUDDRV_GetEquType( AUDDRV_TYPE_AUDIO_OUTPUT );
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_generic READ_AUDVOC_AEQMODE = %d resp = %d\r\n", resp);
			break;

		case AUDDRV_CPCMD_WRITE_AUDVOC_AEQMODE: //set audvoc_aeqMode
			resp = param2;
			AUDDRV_SetEquType( AUDDRV_TYPE_AUDIO_OUTPUT, (AudioEqualizer_en_t)param2 );
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_generic WRITE_AUDVOC_AEQMODE = %d \r\n", param2);
			
//for test			{
//			PAGE5_SharedMem_t *pg5_mem = SHAREDMEM_GetPage5SharedMemPtr();
			
		//pg5_mem->shared_vp_compander_flag = 0;  //for VPU compander.
		//pg5_mem->shared_vp_compander_flag = 2;  //for VPU compander.
		//pg5_mem->shared_vp_compander_flag = 3;  //for VPU compander.
//			pg5_mem->shared_vp_compander_flag = param2;  //for VPU compander.
//			}

			break;

		case AUDDRV_CPCMD_GET_CP_AUDIO_MODE:
			resp = (UInt32)AUDIO_GetAudioMode();
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_generic GET_AUDIO_MODE resp = %d\r\n", resp);
			break;

		case AUDDRV_CPCMD_UPDATE_AUDIO_MODE:
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_generic UPDATE_AUDIO_MODE old mode = %d\r\n", AUDIO_GetAudioMode());
			AUDIO_UpdateAudioMode((AudioMode_t) param2);
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_generic UPDATE_AUDIO_MODE new mode = %d\r\n", AUDIO_GetAudioMode());
			break;

		case AUDDRV_CPCMD_ENABLE_DSP_DTX:
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_generic ENABLE_DSP_DTX = %d\r\n", param2);
			Voice_EnableDTX( (Boolean)param2 );
			break;
		
		default:
			break;
	}
	return resp;
}


/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	audio_control_dsp
//!
//! Description:	Used for AP controlling DSP in some special cases that are not covered by regular control
//!
//! Notes: param1=msg id, param2 ~ param4 = control params
//! 
/////////////////////////////////////////////////////////////////////////////
UInt32 audio_control_dsp(UInt32 param1, UInt32 param2, UInt32 param3, UInt32 param4, UInt32 param5, UInt32 param6)
{
    UInt32 resp=0;
	
	Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* audio_control_dsp param1 %d, param2 %d param3 %d param4 %d *\n\r", param1, param2, param3, param4 );

	switch (param1)
	{
		case DSPCMD_TYPE_AUDIO_CONNECT_UL:
			RIPCMDQ_Connect_Uplink( param2, param3); //param2: uplink, param3: sample_rate
			break;

		case DSPCMD_TYPE_AUDIO_CONNECT_DL:
			RIPCMDQ_Connect_Downlink(param2, param3); // param2: downlink, param3: sample_rate
			break;

		case DSPCMD_TYPE_SET_WAS_SPEECH_IDLE:
			{
				CmdQ_t	msg;	
				msg.cmd= COMMAND_SET_WAS_SPEECH_IDLE;
				msg.arg0 = 0;
				msg.arg1 = 0;
				msg.arg2 = 0;
				SHAREDMEM_PostCmdQ(&msg);
			}
			break;
	
		case DSPCMD_TYPE_AUDIO_ENABLE:
			{
				CmdQ_t	msg;
				msg.cmd= COMMAND_AUDIO_ENABLE;
				msg.arg0 = (UInt16) param2;  //on_off;
				msg.arg1 = (UInt16) param3;  //1: 16KHz,  0: 8KHz
				msg.arg2 = (UInt16) param4;  // voice call 1:16Khz, 0:8Khz
				SHAREDMEM_PostCmdQ(&msg);

				Log_DebugPrintf(LOGID_AUDIO, "\n\r\t* audio_control_dsp (CP) COMMAND_AUDIO_ENABLE, %d, %d, %d \n\r", param2, param3, param4 );

				//RIPCMDQ_EnableAudio( (Boolean)param2, (Boolean)param3, (Boolean)param4 );
			}
			break;

		case DSPCMD_TYPE_EC_ON:
			AUDIO_TurnECOnOff( param2 );
			break;

		case DSPCMD_TYPE_NS_ON:
			AUDIO_TurnNSOnOff( param2 );
			break;


		case DSPCMD_TYPE_DUAL_MIC_ON:
			AUDIO_TurnDualMicOnOff( param2 );
			break;

		case DSPCMD_TYPE_EC_NS_ON:
			AUDIO_Turn_EC_NS_OnOff( param2, param3 );
			break;

		case DSPCMD_TYPE_CONTROL_ECHO_CANCELLATION:
			{
#ifdef DSP_COMMS_INCLUDED
			CmdQ_t	cmd_msg;
	
			AUDIO_EnableEC(TRUE);
			cmd_msg.cmd = COMMAND_ECHO_CANCELLATION;
			cmd_msg.arg0 = 1;	// 1==ON, 0==Off
			cmd_msg.arg1 = 0;
			cmd_msg.arg2 = 0;	// bit0: NLP enable/disable, bit1: Comfort noise on/off
			SHAREDMEM_PostCmdQ(&cmd_msg);
			//RIPCMDQ_ControlEchoCancellation();
#endif
			}
			break;


		case DSPCMD_TYPE_SET_VOICE_DTX:
			DSPCMD_SetVoiceDtx( param2 );
			break;

		case DSPCMD_TYPE_MAIN_AMR_RUN:
			DSPCMD_SetDSP_AMR_RUN(
				(VP_Mode_AMR_t)param2,
				(Boolean)param3,
				(UInt16)param4
				);
			break;

		case DSPCMD_TYPE_AUDIO_TURN_UL_COMPANDEROnOff:
			AUDIO_Turn_UL_CompanderOnOff( param2 );
			break;

		case AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_DL:
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_dsp AUDIO_SetAMRVolume_DL = %d\r\n", param2);
#ifdef tempINTERFACE_AMR_VOL_CONTROL
			AUDIO_SetAMRVolume_DL((UInt16)param2);
#endif
			break;

		case AUDDRV_CPCMD_SET_DSP_AMR_VOLUME_UL:
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_dsp AUDIO_SetAMRVolume_UL = %d\r\n", param2);
#ifdef tempINTERFACE_AMR_VOL_CONTROL
			AUDIO_SetAMRVolume_UL((UInt16)param2);
#endif
			break;
		
		case DSPCMD_TYPE_SET_DSP_UL_GAIN:
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_dsp SET_DSP_UL_GAIN = %d\r\n", param2);
			AUDIO_SetULDigitalGain((UInt8)param2);

			//this needs to be merged with AUDDRV_CPCMD_SetBasebandUplinkGain?
			//recording gain is controllable in audctrl? has api in audctrl?
			VOLUMECTRL_SetSpeechRecGainUL(SPEECH_REC_GAIN_UL_DEFAULT);
			break;

		case DSPCMD_TYPE_MUTE_DSP_UL:
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_dsp MUTE_DSP_UL\r\n");
			VOLUMECTRL_MuteTx();
			//mute for voice recording which is not controlled by previous api
			VOLUMECTRL_SetSpeechRecGainUL(0);			
			break;

		case DSPCMD_TYPE_UNMUTE_DSP_UL:
			Log_DebugPrintf(LOGID_AUDIO, "audio_control_dsp UNMUTE_DSP_UL\r\n");
			VOLUMECTRL_UnMuteTx();
			//unmute for voice recording which is not controlled by previous api
			VOLUMECTRL_SetSpeechRecGainUL(SPEECH_REC_GAIN_UL_DEFAULT);
			break;

		case DSPCMD_TYPE_COMMAND_VT_AMR_START_STOP:
			Log_DebugPrintf(LOGID_AUDIO,"audio_control_dsp: DSPCMD_TYPE_COMMAND_VT_AMR_START_STOP. param2 = %d\n", param2 );
			VPU_SetFlag_fuse_ap_vt_active( param2 );
			break;

		case DSPCMD_TYPE_COMMAND_DSP_AUDIO_ALIGN:
			Log_DebugPrintf(LOGID_AUDIO,"audio_control_dsp: DSPCMD_TYPE_COMMAND_DSP_AUDIO_ALIGN. param2 = %d\n", param2 );
			SYSPARM_DSPAudioAlign( param2 );
			break;

		default:
			xassert( 0, param1 );
			break;
	} 

	return resp;
}

#endif  //#if	defined(FUSE_COMMS_PROCESSOR)
