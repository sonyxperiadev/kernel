//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
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
*   @file   audio_tones_def.h
*
*   @brief  This file defines tone types.
*
****************************************************************************/

#ifndef _INC_AUDIO_TONES_DEF_H_
#define _INC_AUDIO_TONES_DEF_H_

/****************************************************************
 * The following part configures the audio baseband code..
 ****************************************************************/


/****************************************************************
 * The following part shows all the constant values needed in audio baseband code.
 ****************************************************************/


/**
	tone types
 users of this enum could be CAPI, modem, csl driver, peripheral driver.
**/
typedef enum
{
// DTMF Tones
	SPEAKERTONE_ZERO = 0,
	SPEAKERTONE_ONE,
	SPEAKERTONE_TWO,
	SPEAKERTONE_THREE,
	SPEAKERTONE_FOUR,
	SPEAKERTONE_FIVE,
	SPEAKERTONE_SIX,
	SPEAKERTONE_SEVEN,
	SPEAKERTONE_EIGHT,
	SPEAKERTONE_NINE,
	SPEAKERTONE_POUND,
	SPEAKERTONE_STAR,

// Supervisory Tones
	SPEAKERTONE_DIALING,
	SPEAKERTONE_BUSY,
	SPEAKERTONE_RINGING,
	SPEAKERTONE_CONGESTION,
	SPEAKERTONE_RADIO_ACKN,
	SPEAKERTONE_NO_RADIO_PATH,
	SPEAKERTONE_CALL_WAITING,
	SPEAKERTONE_ERROR
} SpeakerTone_t;

/**
	tone types
**/
typedef enum
{
// DTMF Tones
	AUDIO_TONE_ZERO = 0,
	AUDIO_TONE_ONE,
	AUDIO_TONE_TWO,
	AUDIO_TONE_THREE,
	AUDIO_TONE_FOUR,
	AUDIO_TONE_FIVE,
	AUDIO_TONE_SIX,
	AUDIO_TONE_SEVEN,
	AUDIO_TONE_EIGHT,
	AUDIO_TONE_NINE,
	AUDIO_TONE_POUND,
	AUDIO_TONE_STAR,

// Supervisory Tones
	AUDIO_TONE_DIALING,
	AUDIO_TONE_BUSY,
	AUDIO_TONE_RINGING,
	AUDIO_TONE_CONGESTION,
	AUDIO_TONE_RADIO_ACKN,
	AUDIO_TONE_NO_RADIO_PATH,
	AUDIO_TONE_CALL_WAITING,
	AUDIO_TONE_ERROR
} AUDIO_TONE_ID_t;

//????
//! Audio tone type.
typedef enum {
	TONE_TYPE_DTMF = 2,
	TONE_TYPE_GENERIC,
	TONE_TYPE_ALL
} AudioToneType_en_t;

#endif

