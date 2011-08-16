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
*   @file   csl_vpu.h
*
*   @brief  This file contains DSP API to VPU interface
*
****************************************************************************/
#ifndef _CSL_VPU_H_
#define _CSL_VPU_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"


/**
 * @addtogroup CSL VPU Interface
 * @{
 */

// ---- Function Declarations -----------------------------------------

//*********************************************************************
/**
*
*   CSL_VPU_Enable enables VPU interface.
*
* 
**********************************************************************/
void CSL_VPU_Enable(void);

//*********************************************************************
/**
*
*   CSL_VPU_Init initializes VPU interface.
*
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
* 
**********************************************************************/
void CSL_VPU_Init(UInt16 speechMode, UInt16 numFramesPerInterrupt);

//*********************************************************************
/**
*
*   CSL_VPU_ReadPCM reads PCM data from shared memory for VPU voice record.
* 
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex		(in)	index of ping-pong buffer
*   @param    speechMode	(in)	speech mode (codec and sample rate)
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_VPU_ReadPCM(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex, UInt16 speechMode);

/**
*
*   CSL_VPU_WritePCM writes PCM data to shared memory for VPU voice playback.
* 
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex			(in)	index of ping-pong buffer
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_VPU_WritePCM(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex, UInt16 speechMode, UInt16 numFramesPerInterrupt);

//*********************************************************************
/**
*
*   CSL_VPU_ReadAMRNB reads AMR-NB data from shared memory for VPU voice record.
* 
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex		(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_VPU_ReadAMRNB(UInt8* outBuf, UInt32 outSize, UInt16 bufIndex);

//*********************************************************************
/**
*
*   CSL_VPU_WriteAMRNB writes AMR-NB data to shared memory for VPU voice playback.
* 
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex			(in)	index of ping-pong buffer
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_VPU_WriteAMRNB(UInt8* inBuf, UInt32 inSize, UInt16 writeIndex, UInt16 speechMode, UInt32 numFramesPerInterrupt);

//*********************************************************************
/**
*
*   CSL_SetDlSpeechRecGain sets DL Speech Record gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetDlSpeechRecGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteDlSpeechRec mutes DL Speech Record 
*
* 
**********************************************************************/
void CSL_MuteDlSpeechRec(void);


/** @} */

#endif //_CSL_VPU_H_
