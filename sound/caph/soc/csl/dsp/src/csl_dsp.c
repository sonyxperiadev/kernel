//*********************************************************************
//
//	Copyright © 2000-2010 Broadcom Corporation
//
//	This program is the proprietary software of Broadcom Corporation
//	and/or its licensors, and may only be used, duplicated, modified
//	or distributed pursuant to the terms and conditions of a separate,
//	written license agreement executed between you and Broadcom (an
//	"Authorized License").  Except as set forth in an Authorized
//	License, Broadcom grants no license (express or implied), right
//	to use, or waiver of any kind with respect to the Software, and
//	Broadcom expressly reserves all rights in and to the Software and
//	all intellectual property rights therein.  IF YOU HAVE NO
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
//	ALL USE OF THE SOFTWARE.
//
//	Except as expressly set forth in the Authorized License,
//
//	1.	This program, including its structure, sequence and
//		organization, constitutes the valuable trade secrets
//		of Broadcom, and you shall use all reasonable efforts
//		to protect the confidentiality thereof, and to use
//		this information only in connection with your use
//		of Broadcom integrated circuit products.
//
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE,
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   csl_dsp.c
*
*   @brief  This file contains AP interface functions to DSP
*
****************************************************************************/

#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "log.h"
#include "shared.h"
#include "csl_dsp.h"
#include "osdw_dsp_drv.h"
#include "csl_arm2sp.h"

#pragma arm section zidata = "shared_rip_mem_sect"
static AP_SharedMem_t	AP_shared_mememory;      
#pragma arm section

static AP_SharedMem_t	*shared_mem	= &AP_shared_mememory; 

AP_SharedMem_t	*vp_shared_mem;

static VPUCaptureStatusCB_t VPUCaptureStatusHandler = NULL;
static VPURenderStatusCB_t VPURenderStatusHandler = NULL;
static USBStatusCB_t USBStatusHandler = NULL;
static VOIFStatusCB_t VOIFStatusHandler = NULL;
static ARM2SPRenderStatusCB_t ARM2SPRenderStatusHandler = NULL;
static ARM2SP2RenderStatusCB_t ARM2SP2RenderStatusHandler = NULL;
static MainAMRStatusCB_t MainAMRStatusHandler = NULL;
static VoIPStatusCB_t VoIPStatusHandler = NULL;
#if defined(ENABLE_SPKPROT)
static UserStatusCB_t UserStatusHandler = NULL;
#endif
static AudioLogStatusCB_t AudioLogStatusHandler = NULL;

// Temporary till audio code contains references to this function
//******************************************************************************
//
// Function Name:	SHAREDMEM_GetSharedMemPtr
//
// Description:		Return pointer to shared memory
//
// Notes:
//
//******************************************************************************
AP_SharedMem_t *SHAREDMEM_GetDsp_SharedMemPtr()// Return pointer to shared memory
{

	return shared_mem;


}	

//*********************************************************************
/**
*
*   VPSHAREDMEM_Init initializes AP interface to DSP.
*
*   @param    dsp_shared_mem (in)	AP shared memory address 
* 
**********************************************************************/
void VPSHAREDMEM_Init(UInt32 dsp_shared_mem)
{
	// Clear out shared memory
	memset(&AP_shared_mememory, 0, sizeof(AP_SharedMem_t));
	
	vp_shared_mem = (AP_SharedMem_t*) dsp_shared_mem;
	
	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_Init: dsp_shared_mem=0x%lx, \n", dsp_shared_mem);

	vp_shared_mem->vp_shared_cmdq_in = 0;
	vp_shared_mem->vp_shared_cmdq_out = 0;
	vp_shared_mem->vp_shared_statusq_in = 0;
	vp_shared_mem->vp_shared_statusq_out = 0;

	/* Setting various mixer gains to unity gain */
	CSL_SetARM2SpeechDLGain(0);
	CSL_SetARM2Speech2DLGain(0);
	CSL_SetInpSpeechToARM2SpeechMixerDLGain(0);

	CSL_SetARM2SpeechULGain(0);
	CSL_SetARM2Speech2ULGain(0);
	CSL_SetInpSpeechToARM2SpeechMixerULGain(0);

	CSL_SetARM2SpeechCallRecordGain(0);
	CSL_SetARM2Speech2CallRecordGain(0);
}

//*********************************************************************
/**
*
*   VPSHAREDMEM_PostCmdQ writes an entry to the VPU command queue.
*
*   @param    status_msg	(in)	input status message pointer 
* 
**********************************************************************/
void VPSHAREDMEM_PostCmdQ(VPCmdQ_t *cmd_msg)
{
	VPCmdQ_t 	*p;
	UInt8 	next_cmd_in;

	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_PostCmdQ: cmdq_in=0x%x, cmdq_out=0x%x \n", vp_shared_mem->vp_shared_cmdq_in, vp_shared_mem->vp_shared_cmdq_out);
	next_cmd_in = (UInt8)(( vp_shared_mem->vp_shared_cmdq_in + 1 ) % VP_CMDQ_SIZE);

	assert( next_cmd_in != vp_shared_mem->vp_shared_cmdq_out );

	p = &vp_shared_mem->vp_shared_cmdq[ vp_shared_mem->vp_shared_cmdq_in ];
	p->cmd = cmd_msg->cmd;
	p->arg0 = cmd_msg->arg0;
	p->arg1 = cmd_msg->arg1;
	p->arg2 = cmd_msg->arg2;

	vp_shared_mem->vp_shared_cmdq_in = next_cmd_in;
	Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_PostCmdQ: cmd=0x%x, arg0=0x%x, arg1=%d, arg2=%d \n", cmd_msg->cmd, cmd_msg->arg0, cmd_msg->arg1, cmd_msg->arg2);

	VPSHAREDMEM_TriggerRIPInt();

}


//*********************************************************************
/**
*
*   VPSHAREDMEM_ReadStatusQ reads an entry from the VPU status  queue.
*
*   @param    status_msg	(in)	status message destination pointer 
* 
**********************************************************************/
Boolean VPSHAREDMEM_ReadStatusQ(VPStatQ_t *status_msg)
{
	VPStatQ_t *p;
	UInt8	status_out = vp_shared_mem->vp_shared_statusq_out;
	UInt8	status_in = vp_shared_mem->vp_shared_statusq_in;

	//Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_ReadStatusQ: status_in=0x%x, status_out=0x%x \n", vp_shared_mem->vp_shared_statusq_in, vp_shared_mem->vp_shared_statusq_out);
	if ( status_out == status_in )
	{
		return FALSE;
	}
	else
	{
		p = &vp_shared_mem->vp_shared_statusq[ status_out ];
		status_msg->status = p->status;
		status_msg->arg0 = (UInt16)p->arg0;
		status_msg->arg1 = (UInt16)p->arg1;
		status_msg->arg2 = (UInt16)p->arg2;
		status_msg->arg3 = (UInt16)p->arg3;
		//Log_DebugPrintf(LOGID_AUDIO, " VPSHAREDMEM_ReadStatusQ: status=%d, arg0=%d, arg1=%d, arg2=%d, arg3=%d \n", p->status, p->arg0, p->arg1, p->arg2, p->arg3);

		vp_shared_mem->vp_shared_statusq_out = ( status_out + 1 ) % VP_STATUSQ_SIZE;

		return TRUE;
	}

}


//*********************************************************************
/**
*
*   CSL_RegisterVPUCaptureStatusHandler registers VPU capture status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVPUCaptureStatusHandler(VPUCaptureStatusCB_t callbackFunction)
{
	VPUCaptureStatusHandler = callbackFunction;

}

//*********************************************************************
/**
*
*   CSL_RegisterVPURenderStatusHandler registers VPU render status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVPURenderStatusHandler(VPURenderStatusCB_t callbackFunction)
{
	VPURenderStatusHandler = callbackFunction;

}


//*********************************************************************
/**
*
*   CSL_RegisterUSBStatusHandler registers USB status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterUSBStatusHandler(USBStatusCB_t callbackFunction)
{
	USBStatusHandler = callbackFunction;

}

//*********************************************************************
/**
*
*   CSL_RegisterVOIFStatusHandler registers VOIF status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVOIFStatusHandler(VOIFStatusCB_t callbackFunction)
{
	VOIFStatusHandler = callbackFunction;

}

//*********************************************************************
/**
*
*   CSL_RegisterARM2SPRenderStatusHandler registers main ARM2SP render 
*	status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterARM2SPRenderStatusHandler(ARM2SPRenderStatusCB_t callbackFunction)
{
	ARM2SPRenderStatusHandler = callbackFunction;

}

//*********************************************************************
/**
*
*   CSL_RegisterARM2SP2RenderStatusHandler registers main ARM2SP2 render 
*	status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterARM2SP2RenderStatusHandler(ARM2SP2RenderStatusCB_t callbackFunction)
{
	ARM2SP2RenderStatusHandler = callbackFunction;

}


//*********************************************************************
/**
*
*   CSL_RegisterMainAMRStatusHandler registers main AMR status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterMainAMRStatusHandler(MainAMRStatusCB_t callbackFunction)
{
	MainAMRStatusHandler = callbackFunction;

}

//*********************************************************************
/**
*
*   CSL_RegisterAudioLogHandler registers VoIP status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVoIPStatusHandler(VoIPStatusCB_t callbackFunction)
{
	VoIPStatusHandler = callbackFunction;

}

#if defined(ENABLE_SPKPROT)
//*********************************************************************
/**
*
*   CSL_RegisterAudioLogHandler registers user status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterUserStatusHandler(UserStatusCB_t callbackFunction)
{
	UserStatusHandler = callbackFunction;

}
#endif

//*********************************************************************
/**
*
*   CSL_RegisterAudioLogHandler registers audio logging status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterAudioLogHandler(AudioLogStatusCB_t callbackFunction)
{
	AudioLogStatusHandler = callbackFunction;

}


//*********************************************************************
/**
*
*   AP_ProcessStatus processes VPU status message from DSP on AP.
*
* 
**********************************************************************/
void AP_ProcessStatus(void)
{	
 VPStatQ_t status_msg;

	while(VPSHAREDMEM_ReadStatusQ(&status_msg))
	{    
		switch(status_msg.status)
		{
			case VP_STATUS_RECORDING_DATA_READY:
			{
				if(VPUCaptureStatusHandler != NULL)
				{
					VPUCaptureStatusHandler(status_msg.arg0);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: VPUCaptureStatusHandler is not registered");
				}
				break;

			}

			case VP_STATUS_PLAYBACK_DATA_EMPTY:
			{
				if(VPURenderStatusHandler != NULL)
				{
					VPURenderStatusHandler(status_msg.arg0);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: VPURenderStatusHandler is not registered");
				}
				break;
			
			}

			case VP_STATUS_USB_HEADSET_BUFFER:
			{
				if(USBStatusHandler != NULL)
				{
					USBStatusHandler(status_msg.arg0, status_msg.arg1, status_msg.arg2);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: USBStatusHandler is not registered");
				}
				break;
			}

			case VP_STATUS_VOIF_BUFFER_READY:
			{
				if(VOIFStatusHandler != NULL)
				{
					VOIFStatusHandler(status_msg.arg0, status_msg.arg1);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: VOIFStatusHandler is not registered");
				}
				break;
			}

			case VP_STATUS_ARM2SP_EMPTY:
			{
				if(ARM2SPRenderStatusHandler != NULL)
				{
					ARM2SPRenderStatusHandler(status_msg.arg1);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: ARM2SPRenderStatusHandler is not registered");
				}
				break;
			}

			case VP_STATUS_ARM2SP2_EMPTY:
			{
				if(ARM2SP2RenderStatusHandler != NULL)
				{
					ARM2SP2RenderStatusHandler(status_msg.arg1);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: ARM2SP2RenderStatusHandler is not registered");
				}
				break;
			}
			
			case VP_STATUS_MAIN_AMR_DONE:
			{
				if(MainAMRStatusHandler != NULL)
				{
					MainAMRStatusHandler(status_msg.arg0);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: MainAMRStatusHandler is not registered");
				}
 				break;
			}

			case VP_STATUS_VOIP_DL_DONE:
			{
				if(VoIPStatusHandler != NULL)
				{
					VoIPStatusHandler();
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: VoIPStatusHandler is not registered");
				}
				break;
			}

#if defined(ENABLE_SPKPROT)
			case VP_STATUS_SP:
			{
				if(UserStatusHandler != NULL)
				{
					UserStatusHandler((UInt32)status_msg.arg0, (UInt32)status_msg.arg1, (UInt32)status_msg.arg2);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: UserStatusHandler is not registered");
				}
				break;
			}
#endif

			case VP_STATUS_AUDIO_STREAM_DATA_READY:
			{
				if(AudioLogStatusHandler != NULL)
				{
					AudioLogStatusHandler(status_msg.arg2);
				}
				else
				{
					Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: AudioLogStatusHandler is not registered");
				}
				break;		
			}

			default:
				Log_DebugPrintf(LOGID_AUDIO, "AP DSP Interrupt: Unknown Status received");

				break;

		}

	}

}

/*****************************************************************************************/
/**
* 
* Function Name: AUDIO_Return_IHF_48kHz_buffer_base_address
*
*   @note     This function returns the base address to the shared memory buffer where
*             the ping-pong 48kHz data would be stored for AADMAC to pick them up
*             for IHF case. This base address needs to be programmed to the 
*             AADMAC_CTRL1 register.
*                                                                                         
*   @return   ptr (UInt32 *) Pointer to the base address of shared memory ping-pong buffer 
*                            for transferring 48kHz speech data from DSP to AADMAC for IHF.
*
**/
/*******************************************************************************************/
UInt32 *AUDIO_Return_IHF_48kHz_buffer_base_address(void)
{
   	return(&(vp_shared_mem->shared_aud_out_buf_48k[0][0]));
}

