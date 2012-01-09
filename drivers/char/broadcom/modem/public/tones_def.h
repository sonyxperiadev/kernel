//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*   @file   tones_def.h
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
typedef enum {
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
typedef enum {
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
