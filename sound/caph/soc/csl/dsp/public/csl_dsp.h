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
*   @file   csl_dsp.h
*
*   @brief  This file contains AP interface functions to DSP
*
****************************************************************************/
#ifndef _CSL_DSP_H_
#define _CSL_DSP_H_

#include "mobcom_types.h"

typedef void (*VPUCaptureStatusCB_t)(UInt16 bufferIndex);
typedef void (*VPURenderStatusCB_t)(UInt16 bufferIndex);
typedef void (*WBAMRRenderStatusCB_t)(UInt16 status, UInt16 dsp_read_index, UInt16 dsp_write_index);
typedef void (*USBStatusCB_t)(UInt16 param1, UInt16 param2, UInt16 param3);
typedef void (*VOIFStatusCB_t)(UInt16 param1, UInt16 param2);
typedef void (*ARM2SPRenderStatusCB_t)(UInt16 bufferPosition);
typedef void (*ARM2SP2RenderStatusCB_t)(UInt16 bufferPosition);
typedef void (*WBAMRCaptureStatusCB_t)(UInt16 size, UInt16 index);
typedef void (*MainAMRStatusCB_t)(UInt16 codecType);
typedef void (*VoIPStatusCB_t)(void);
typedef void (*UserStatusCB_t)(UInt32 param1, UInt32 param2, UInt32 param3);
typedef void (*AudioLogStatusCB_t)(UInt16 bufferIndex);

//*********************************************************************
/**
*
*   VPSHAREDMEM_Init initializes AP interface to DSP.
*
*   @param    dsp_shared_mem (in)	AP shared memory address 
* 
**********************************************************************/
void VPSHAREDMEM_Init(UInt32 dsp_shared_mem);

//*********************************************************************
/**
*
*   VPSHAREDMEM_PostCmdQ writes an entry to the VPU command queue.
*
*   @param    status_msg	(in)	input status message pointer 
* 
**********************************************************************/
void VPSHAREDMEM_PostCmdQ(VPCmdQ_t *cmd_msg);

//*********************************************************************
/**
*
*   CSL_RegisterVPUCaptureStatusHandler registers VPU capture status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVPUCaptureStatusHandler(VPUCaptureStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   CSL_RegisterVPURenderStatusHandler registers VPU render status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVPURenderStatusHandler(VPURenderStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   CSL_RegisterUSBStatusHandler registers USB status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterUSBStatusHandler(USBStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   CSL_RegisterVOIFStatusHandler registers VOIF status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVOIFStatusHandler(VOIFStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   CSL_RegisterARM2SPRenderStatusHandler registers main ARM2SP render 
*	status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterARM2SPRenderStatusHandler(ARM2SPRenderStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   CSL_RegisterARM2SP2RenderStatusHandler registers main ARM2SP2 render 
*	status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterARM2SP2RenderStatusHandler(ARM2SP2RenderStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   CSL_RegisterMainAMRStatusHandler registers main AMR status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterMainAMRStatusHandler(MainAMRStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   CSL_RegisterAudioLogHandler registers VoIP status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterVoIPStatusHandler(VoIPStatusCB_t callbackFunction);

#if defined(ENABLE_SPKPROT)
//*********************************************************************
/**
*
*   CSL_RegisterAudioLogHandler registers user status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterUserStatusHandler(UserStatusCB_t callbackFunction);
#endif

//*********************************************************************
/**
*
*   CSL_RegisterAudioLogHandler registers audio logging status handler.
*
*   @param    callbackFunction	(in)	callback function to register 
* 
**********************************************************************/
void CSL_RegisterAudioLogHandler(AudioLogStatusCB_t callbackFunction);

//*********************************************************************
/**
*
*   AP_ProcessStatus processes VPU status message from DSP on AP.
*
* 
**********************************************************************/
void AP_ProcessStatus(void);

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
UInt32 *AUDIO_Return_IHF_48kHz_buffer_base_address(void);

#endif //_CSL_DSP_H_
