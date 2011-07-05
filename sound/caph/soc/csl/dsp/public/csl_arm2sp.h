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
*   @file   csl_arm2sp.h
*
*   @brief  This file contains CSL (DSP) API to ARM2SP interface
*
****************************************************************************/
#ifndef _CSL_ARM2SP_H_
#define _CSL_ARM2SP_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"

/**
 * @addtogroup CSL ARM2SP interface
 * @{
 */

#define MAX_ARM2SP_DL_GAIN			(1<<14)		/* unity gain in DSP Q1.14 format */
#define MIN_ARM2SP_DL_GAIN			0

#define MAX_ARM2SP_UL_GAIN			((2<<14)-1)	/* 6dB in DSP Q1.14 format */
#define MIN_ARM2SP_UL_GAIN			0

#define MAX_ARM2SP2_DL_GAIN			(1<<14)		/* unity gain in DSP Q1.14 format */
#define MIN_ARM2SP2_DL_GAIN			0

#define MAX_ARM2SP2_UL_GAIN			((2<<14)-1)	/* 6dB in DSP Q1.14 format */
#define MIN_ARM2SP2_UL_GAIN			0

#define MAX_ARM2SP_CALL_REC_GAIN	((2<<14)-1)	/* 6dB in DSP Q1.14 format */
#define MIN_ARM2SP_CALL_REC_GAIN	0

#define MAX_ARM2SP2_CALL_REC_GAIN	((2<<14)-1)	/* 6dB in DSP Q1.14 format */
#define MIN_ARM2SP2_CALL_REC_GAIN	0

#define MAX_INP_SP_TO_ARM2SP_MIXER_DL_GAIN  (1<<14)		/* unity gain in DSP Q1.14 format */
#define MIN_INP_SP_TO_ARM2SP_MIXER_DL_GAIN  0
                                          
#define MAX_INP_SP_TO_ARM2SP_MIXER_UL_GAIN  ((2<<14)-1)	/* 6dB in DSP Q1.14 format */
#define MIN_INP_SP_TO_ARM2SP_MIXER_UL_GAIN  0

// ---- Function Declarations -----------------------------------------
//*********************************************************************
/**
*
*   CSL_ARM2SP_Init initializes  ARM2SP input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP_Init(void);

//*********************************************************************
/**
*
*   CSL_ARM2SP_Write writes data to shared memory for ARM2SP voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize		(in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @param    in48K			(in)	48K signal?
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex, Boolean in48K);

//*********************************************************************
/**
*
*   CSL_ARM2SP2_Init initializes  ARM2SP2 input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP2_Init(void);

//*********************************************************************
/**
*
*   CSL_ARM2SP2_Write writes data to shared memory for ARM2SP2 voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize		(in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @param    in48K			(in)	48K signal?
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP2_Write(UInt8 *inBuf, UInt32 inSize_inBytes, UInt16 writeIndex, Boolean in48K);

//*********************************************************************
/**
*
*   CSL_SetARM2SpeechULGain sets ARM2SP downlink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2SpeechDLGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteARM2SpeechDL mutes ARM2SP downlink.
*
* 
**********************************************************************/
void CSL_MuteARM2SpeechDL(void);

//*********************************************************************
/**
*
*   CSL_SetARM2SpeechULGain sets ARM2SP uplink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2SpeechULGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteARM2SpeechUL mutes ARM2SP uplink.
*
* 
**********************************************************************/
void CSL_MuteARM2SpeechUL(void);

//*********************************************************************
/**
*
*   CSL_SetARM2Speech2DLGain sets ARM2SP2 downlink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2Speech2DLGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteARM2Speec2hDL mutes ARM2SP2 downlink.
*
* 
**********************************************************************/
void CSL_MuteARM2Speech2DL(void);

//*********************************************************************
/**
*
*   CSL_SetARM2Speech2ULGain sets ARM2SP2 uplink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2Speech2ULGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteARM2Speech2UL mutes ARM2SP2 uplink.
*
* 
**********************************************************************/
void CSL_MuteARM2Speech2UL(void);

//*********************************************************************
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerDLGain sets Downlink Input Speech 
*   Gain to ARM2SP mixer.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetInpSpeechToARM2SpeechMixerDLGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerDL mutes Downlink Input Speech 
*   to ARM2SP mixer.
*
*   @param    None
*   @return   None
* 
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerDL(void);

//*********************************************************************
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerULGain sets Uplink Input Speech 
*   Gain to ARM2SP mixer.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetInpSpeechToARM2SpeechMixerULGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerUL mutes Uplink Input Speech 
*   to ARM2SP mixer.
*
*   @param    None
*   @return   None
* 
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerUL(void);

//*********************************************************************
/**
*
*   CSL_SetARM2SpeechCallRecordGain sets ARM2SP Call Record gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2SpeechCallRecordGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteARM2SpeechCallRecord mutes ARM2SP Call Record 
*
* 
**********************************************************************/
void CSL_MuteARM2SpeechCallRecord(void);

//*********************************************************************
/**
*
*   CSL_SetARM2Speech2CallRecordGain sets ARM2SP2 Call Record gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2Speech2CallRecordGain(Int16 mBGain);

//*********************************************************************
/**
*
*   CSL_MuteARM2Speech2CallRecord mutes ARM2SP2 Call Record 
*
* 
**********************************************************************/
void CSL_MuteARM2Speech2CallRecord(void);


/** @} */

#endif //_CSL_ARM2SP_H_
