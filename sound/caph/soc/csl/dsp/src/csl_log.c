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
*   @file   csl_log.c
*
*   @brief  This file contains CSL (DSP) API to audio logging interface
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "resultcode.h"
#include "csl_log.h"

extern AP_SharedMem_t	*vp_shared_mem;



//*********************************************************************
/**
*
*   CSL_LOG_Start starts audio logging for specified stream.
* 
*   @param    streamNumber	(in)	stream number 1:4
*   @param    controlInfo	(in)	control information
*
**********************************************************************/
Result_t CSL_LOG_Start(UInt16 streamNumber, UInt16 controlInfo)
{

	Result_t res = RESULT_OK;

	switch(streamNumber)
	{
		case 1:
			vp_shared_mem->shared_audio_stream_0_crtl = controlInfo;

			break;

		case 2:
			vp_shared_mem->shared_audio_stream_1_crtl = controlInfo;

			break;

		case 3:
			vp_shared_mem->shared_audio_stream_2_crtl = controlInfo;

			break;

		case 4:
			vp_shared_mem->shared_audio_stream_3_crtl = controlInfo;

			break;

		default:
			res = RESULT_ERROR;
			break;

	}
	return res;

} // CSL_LOG_Start


//*********************************************************************
/**
*
*   CSL_LOG_Start stops audio logging for specified stream.
* 
*   @param    streamNumber	(in)	stream number 1:4
*
**********************************************************************/
Result_t CSL_LOG_Stop(UInt16 streamNumber, UInt8 *flag)
{
	Result_t res = RESULT_OK;

	switch(streamNumber)
	{
		case 1:
			vp_shared_mem->shared_audio_stream_0_crtl = 0;

			break;

		case 2:
			vp_shared_mem->shared_audio_stream_1_crtl = 0;

			break;

		case 3:
			vp_shared_mem->shared_audio_stream_2_crtl = 0;

			break;

		case 4:
			vp_shared_mem->shared_audio_stream_3_crtl = 0;

			break;

		default:
			res = RESULT_ERROR;
			break;

	}

	if ( vp_shared_mem->shared_audio_stream_0_crtl == 0
		&& vp_shared_mem->shared_audio_stream_1_crtl == 0
		&& vp_shared_mem->shared_audio_stream_2_crtl == 0
		&& vp_shared_mem->shared_audio_stream_3_crtl == 0
		)
	{
		*flag = 1; //inform to audio driver for shut down
	}

	return res;
} // CSL_LOG_Stop


//*********************************************************************
/**
*
*   CSL_LOG_Read reads data from shared memory for audio logging.
* 
*   @param    streamNumber	(in)	stream number 1:4
*   @param    readIndex		(in)	index of ping-pong buffer 
*   @param    outBuf		(out)	output buffer
*   @param    controlInfo	(out)	logging control information
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_LOG_Read(UInt16 streamNumber, UInt16 readIndex, UInt8 *outBuf, UInt16 *controlInfo)
{
 UInt16 size;    //number of 16-bit words
 Audio_Logging_Buf_t* ptr;
 UInt16 sampleRate;	//To indicate 8kHz or 16kHz samplerate

	switch(streamNumber)
	{
		case 1:
			*controlInfo = vp_shared_mem->shared_audio_stream_0_crtl;
			ptr = (Audio_Logging_Buf_t *)&vp_shared_mem->shared_audio_stream_0[readIndex].param[0];
			sampleRate = vp_shared_mem->shared_audio_stream_0[readIndex].rate;

			break;

		case 2:
			*controlInfo = vp_shared_mem->shared_audio_stream_1_crtl;
			ptr = (Audio_Logging_Buf_t *)&vp_shared_mem->shared_audio_stream_1[readIndex].param[0];
			sampleRate = vp_shared_mem->shared_audio_stream_1[readIndex].rate;

			break;

		case 3:
			*controlInfo = vp_shared_mem->shared_audio_stream_2_crtl;
			ptr = (Audio_Logging_Buf_t *)&vp_shared_mem->shared_audio_stream_2[readIndex].param[0];
			sampleRate = vp_shared_mem->shared_audio_stream_2[readIndex].rate;

			break;

		case 4:
			*controlInfo = vp_shared_mem->shared_audio_stream_3_crtl;
			ptr = (Audio_Logging_Buf_t *)&vp_shared_mem->shared_audio_stream_3[readIndex].param[0];
			sampleRate = vp_shared_mem->shared_audio_stream_3[readIndex].rate;

			break;

		default:
			return 0;

	}
	
	/* if nothing to read */
	if(*controlInfo == 0)
	{
		return 0;

	} 


	/* check ctrl point */
	// 0x101 and 0x201 are only compressed frame cases
	if (*controlInfo == 0x101 || *controlInfo == 0x201)
	{
		// Compressed frame captured
		size = LOG_COMPRESSED_SIZE; //162
	}
	else
	{	//Sample rate is 16kHz
		if(sampleRate == 16000)
		{// PCM frame
			size = LOG_WB_SIZE-2; //642
		}
		//Sample rate is 8kHz
		else
		{
			size = LOG_NB_SIZE; //320
		}

	}
			

	/* get logging data */
	memcpy(outBuf, ptr, size);

	if(sampleRate == 16000)
	{
		*(outBuf+640) = (UInt8)(sampleRate&0xf);
		*(outBuf+641) = (UInt8)((sampleRate>>4)&0xf);
		return (size+2); //Sample rate is 16kHz
	}
	else
		return size; //Sample rate is 8kHz

} // CSL_LOG_Read

