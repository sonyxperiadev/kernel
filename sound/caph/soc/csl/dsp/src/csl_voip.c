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
*   @file   csl_voip.c
*
*   @brief  This file contains DSP VoIP interface
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "csl_voip.h"
#include "log.h"

extern AP_SharedMem_t	*vp_shared_mem;

static const UInt16 sVoIPDataLen[7] = {0, 322, 160, 38, 166, 642, 70};


//*********************************************************************
/**
*
*   CSL_WriteDLVoIPData writes VoIP data to DSP shared memory
*
*   @param    codec_type	(in)		codec type
*   @param    pSrc			(in)		source of the data to write
* 
**********************************************************************/
void CSL_WriteDLVoIPData(UInt16 codec_type, UInt16 *pSrc)
{
	VOIP_Buffer_t *pDst = &(vp_shared_mem->VOIP_DL_buf);
	UInt8 index = 0;
	UInt16 data_len = 0;

	index = (codec_type & 0xf000) >> 12;
	if (index >= 7)
  		Log_DebugPrintf(LOGID_AUDIO, "===== CSL_WriteDLVoIPData, invalid codec type!!!\n");
	else
		data_len = sVoIPDataLen[index];

	memcpy(pDst, pSrc, data_len);
}

//*********************************************************************
/**
*
*   CSL_ReadULVoIPData reads VoIP data from DSP shared memory
*
*   @param    codec_type	(in)		codec type
*   @param    pDst			(in)		destination of read data
*   @return   UInt8						size of data in bytes
* 
**********************************************************************/
UInt8 CSL_ReadULVoIPData(UInt16 codec_type, UInt16 *pDst)
{
	UInt16 *pSrc = (UInt16*)&(vp_shared_mem->VOIP_UL_buf.voip_vocoder);
	UInt8 index = 0;
	UInt16 data_len = 0;

	index = (codec_type & 0xf000) >> 12;
	if (index >= 7)
  		Log_DebugPrintf(LOGID_AUDIO, "===== CSL_ReadULVoIPData, invalid codec type!!!\n");
	else
		data_len = sVoIPDataLen[index];
	
	memcpy(pDst, pSrc, data_len);
	
	return data_len;
}

