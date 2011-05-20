//*********************************************************************
//
//	Copyright © 2000-2011 Broadcom Corporation
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
*   @file   csl_arm2sp.c
*
*   @brief  This file contains CSL (DSP) API to ARM2SP interface
*
****************************************************************************/

#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "shared.h"
#include "csl_arm2sp.h"

extern VPSharedMem_t	*vp_shared_mem;

//*********************************************************************
/**
*
*   CSL_ARM2SP_Init initializes  ARM2SP input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP_Init(void)
{
    memset(&vp_shared_mem->shared_Arm2SP_InBuf, 0, sizeof(vp_shared_mem->shared_Arm2SP_InBuf)); // clean buffer 

} // CSL_ARM2SP_Init


//*********************************************************************
/**
*
*   CSL_ARM2SP_Write writes data to shared memory for ARM2SP voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize_inBytes  (in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	bufSize_inWords = ARM2SP_INPUT_SIZE;  //in number of words.
	halfBufSize_inBytes = ARM2SP_INPUT_SIZE;
		
	//beginning of the buffer or the half point in the buffer.
	offset = (writeIndex == 0)? (bufSize_inWords/2) : 0; // offset is in 16bit words


	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ? inSize_inBytes : halfBufSize_inBytes;

	memcpy( (UInt8 *)(&vp_shared_mem->shared_Arm2SP_InBuf[offset]), inBuf, totalCopied_bytes);
	
	return totalCopied_bytes;  	// return the number of bytes has been copied

} // CSL_ARM2SP_Write


//*********************************************************************
/**
*
*   CSL_ARM2SP2_Init initializes  ARM2SP2 input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP2_Init(void)
{
    memset(&vp_shared_mem->shared_Arm2SP2_InBuf, 0, sizeof(vp_shared_mem->shared_Arm2SP2_InBuf)); // clean buffer 

} // CSL_ARM2SP2_Init


//*********************************************************************
/**
*
*   CSL_ARM2SP2_Write writes data to shared memory for ARM2SP2 voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize_inBytes  (in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP2_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	bufSize_inWords = ARM2SP_INPUT_SIZE;  //in number of words.
	halfBufSize_inBytes = ARM2SP_INPUT_SIZE;
		
	//beginning of the buffer or the half point in the buffer.
	offset = (writeIndex == 0)? (bufSize_inWords/2) : 0; // offset is in 16bit words


	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ? inSize_inBytes : halfBufSize_inBytes;

	memcpy( (UInt8 *)(&vp_shared_mem->shared_Arm2SP2_InBuf[offset]), inBuf, totalCopied_bytes);
	
	return totalCopied_bytes;  	// return the number of bytes has been copied

} // CSL_ARM2SP2_Write

