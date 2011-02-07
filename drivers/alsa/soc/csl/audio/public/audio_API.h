/*******************************************************************************************
Copyright 2004 - 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**

*    @file   audio_API.h
*    @brief  This file contains the interface to the Multi-Media Audio functions.

**/
/****************************************************************************/
/**

*   @defgroup   CSLAudioGroup   Audio Services
*   @ingroup    CSLGroup
*   @brief      This group defines the interfaces to the Chip Support Library Audio services.
*
* Click here to return to the Chip Support Library Overview: \ref CSLOverview
*
* This API is used to playback ringtones and supports sound recording.
*
* A session encapulates an audio playback or recording process.
  It can be referred to by an unique session handle (returned by
  Audio_OpenSession() ) and has four different status:
    - OPENED:  the audio device is locked and opened for the session.
    - STARTED: the audio device is in the middle of playback or recording
    - PAUSED:  the audio device is paused
    - RESUMED: the audio device is resumed after being paused

  Audio_CloseSession() frees up resources used by the session and terminates
  the session.
****************************************************************************/
#ifndef __AUDIO_API_H__
#define __AUDIO_API_H__

#include "mediatypes.h"
#include "audio_consts.h"  //some audio enum's are moved to msp/inc/audio_consts.h

/**
	 Supported Audio Formats
**/

typedef Media_t	AUDIO_FORMAT_t;

#define	AUDIO_FORMAT_AMR					MEDIA_AMR					///< Adjustable Multi-Rate Narrow-Band
#define	AUDIO_FORMAT_AMR_WB					MEDIA_AMR_WB				///< Adjustable Multi-Rate Wide-Band
#define	AUDIO_FORMAT_MIDI					MEDIA_MIDI					///< Standard MIDI format
#define	AUDIO_FORMAT_SMAF					MEDIA_SMAF					///< SMAF for MA3
#define	AUDIO_FORMAT_VMD					MEDIA_VMD					///< VMD
#define	AUDIO_FORMAT_WAVE					MEDIA_WAV					///< WAV
#define	AUDIO_FORMAT_PCM_8B_MONO_8K			MEDIA_PCM_8B_MONO_8K		///< PCM, 8bit,  mono, 8K
#define	AUDIO_FORMAT_PCM_16B_MONO_8K		MEDIA_PCM_16B_MONO_8K		///< PCM, 16bit, mono, 8K
#define	AUDIO_FORMAT_PCM_16B_MONO_11K		MEDIA_PCM_16B_MONO_11K		///< PCM, 16bit, mono, 11K
#define	AUDIO_FORMAT_PCM_16B_MONO_12K		MEDIA_PCM_16B_MONO_12K		///< PCM, 16bit, mono, 12K
#define	AUDIO_FORMAT_PCM_16B_MONO_16K		MEDIA_PCM_16B_MONO_16K		///< PCM, 16bit, mono, 16K
#define	AUDIO_FORMAT_PCM_16B_MONO_22K		MEDIA_PCM_16B_MONO_22K		///< PCM, 16bit, mono, 22K
#define	AUDIO_FORMAT_PCM_16B_MONO_24K		MEDIA_PCM_16B_MONO_24K		///< PCM, 16bit, mono, 24K
#define	AUDIO_FORMAT_PCM_16B_MONO_48K		MEDIA_PCM_16B_MONO_48K		///< PCM, 16bit, mono, 48K
#define	AUDIO_FORMAT_PCM_16B_MONO_441K		MEDIA_PCM_16B_MONO_441K		///< PCM, 16bit, mono, 44.1K
#define	AUDIO_FORMAT_PCM_16B_MONO_32K		MEDIA_PCM_16B_MONO_32K		///< PCM, 16bit, mono, 32K
#define	AUDIO_FORMAT_PCM_16B_STEREO_8K  	MEDIA_PCM_16B_STEREO_8K		///< PCM, 16bit, stereo, 8K
#define	AUDIO_FORMAT_PCM_16B_STEREO_11K		MEDIA_PCM_16B_STEREO_11K	///< PCM, 16bit, stereo, 11K
#define	AUDIO_FORMAT_PCM_16B_STEREO_12K	    MEDIA_PCM_16B_STEREO_12K	///< PCM, 16bit, stereo, 12K
#define	AUDIO_FORMAT_PCM_16B_STEREO_16K		MEDIA_PCM_16B_STEREO_16K	///< PCM, 16bit, stereo, 16K
#define	AUDIO_FORMAT_PCM_16B_STEREO_22K		MEDIA_PCM_16B_STEREO_22K	///< PCM, 16bit, stereo, 22K
#define	AUDIO_FORMAT_PCM_16B_STEREO_24K	    MEDIA_PCM_16B_STEREO_24K	///< PCM, 16bit, stereo, 24K
#define	AUDIO_FORMAT_PCM_16B_STEREO_48K		MEDIA_PCM_16B_STEREO_48K	///< PCM, 16bit, stereo, 48K
#define	AUDIO_FORMAT_PCM_16B_STEREO_441K	MEDIA_PCM_16B_STEREO_441K	///< PCM, 16bit, stereo, 44.1K
#define	AUDIO_FORMAT_PCM_16B_STEREO_32K		MEDIA_PCM_16B_STEREO_32K	///< PCM, 16bit, stereo, 32K

// For Bluetooth stereo headset
#define AUDIO_FORMAT_PCM_8B_MONO_8K_BTS         MEDIA_PCM_8B_MONO_8K_BTS        ///< PCM, 8bit,  mono, 8K
#define AUDIO_FORMAT_PCM_16B_MONO_8K_BTS        MEDIA_PCM_16B_MONO_8K_BTS       ///< PCM, 16bit, mono, 8K
#define AUDIO_FORMAT_PCM_16B_MONO_11K_BTS       MEDIA_PCM_16B_MONO_11K_BTS      ///< PCM, 16bit, mono, 11K
#define AUDIO_FORMAT_PCM_16B_MONO_12K_BTS       MEDIA_PCM_16B_MONO_12K_BTS      ///< PCM, 16bit, mono, 12K
#define AUDIO_FORMAT_PCM_16B_MONO_16K_BTS       MEDIA_PCM_16B_MONO_16K_BTS      ///< PCM, 16bit, mono, 16K
#define AUDIO_FORMAT_PCM_16B_MONO_22K_BTS       MEDIA_PCM_16B_MONO_22K_BTS      ///< PCM, 16bit, mono, 22K
#define AUDIO_FORMAT_PCM_16B_MONO_24K_BTS       MEDIA_PCM_16B_MONO_24K_BTS      ///< PCM, 16bit, mono, 24K
#define AUDIO_FORMAT_PCM_16B_MONO_48K_BTS       MEDIA_PCM_16B_MONO_48K_BTS      ///< PCM, 16bit, mono, 48K
#define AUDIO_FORMAT_PCM_16B_MONO_441K_BTS      MEDIA_PCM_16B_MONO_441K_BTS     ///< PCM, 16bit, mono, 44.1K
#define AUDIO_FORMAT_PCM_16B_MONO_32K_BTS       MEDIA_PCM_16B_MONO_32K_BTS      ///< PCM, 16bit, mono, 32K
#define AUDIO_FORMAT_PCM_16B_STEREO_8K_BTS      MEDIA_PCM_16B_STEREO_8K_BTS     ///< PCM, 16bit, stereo, 8K
#define AUDIO_FORMAT_PCM_16B_STEREO_11K_BTS     MEDIA_PCM_16B_STEREO_11K_BTS    ///< PCM, 16bit, stereo, 11K
#define AUDIO_FORMAT_PCM_16B_STEREO_12K_BTS     MEDIA_PCM_16B_STEREO_12K_BTS    ///< PCM, 16bit, stereo, 12K
#define AUDIO_FORMAT_PCM_16B_STEREO_16K_BTS     MEDIA_PCM_16B_STEREO_16K_BTS    ///< PCM, 16bit, stereo, 16K
#define AUDIO_FORMAT_PCM_16B_STEREO_22K_BTS     MEDIA_PCM_16B_STEREO_22K_BTS    ///< PCM, 16bit, stereo, 22K
#define AUDIO_FORMAT_PCM_16B_STEREO_24K_BTS     MEDIA_PCM_16B_STEREO_24K_BTS    ///< PCM, 16bit, stereo, 24K
#define AUDIO_FORMAT_PCM_16B_STEREO_48K_BTS     MEDIA_PCM_16B_STEREO_48K_BTS    ///< PCM, 16bit, stereo, 48K
#define AUDIO_FORMAT_PCM_16B_STEREO_441K_BTS    MEDIA_PCM_16B_STEREO_441K_BTS  ///< PCM, 16bit, stereo, 44.1K
#define AUDIO_FORMAT_PCM_16B_STEREO_32K_BTS     MEDIA_PCM_16B_STEREO_32K_BTS    ///< PCM, 16bit, stereo, 32K

// for USB devices
#define AUDIO_FORMAT_PCM_USB                    MEDIA_PCM_USB                   ///< PCM to USB devices
#define AUDIO_FORMAT_PCM_8B_MONO_8K_USB         MEDIA_PCM_8B_MONO_8K_USB        ///< PCM, 8bit,  mono, 8K
#define AUDIO_FORMAT_PCM_16B_MONO_8K_USB        MEDIA_PCM_16B_MONO_8K_USB       ///< PCM, 16bit, mono, 8K
#define AUDIO_FORMAT_PCM_16B_MONO_11K_USB       MEDIA_PCM_16B_MONO_11K_USB      ///< PCM, 16bit, mono, 11K
#define AUDIO_FORMAT_PCM_16B_MONO_12K_USB       MEDIA_PCM_16B_MONO_12K_USB      ///< PCM, 16bit, mono, 12K
#define AUDIO_FORMAT_PCM_16B_MONO_16K_USB       MEDIA_PCM_16B_MONO_16K_USB      ///< PCM, 16bit, mono, 16K
#define AUDIO_FORMAT_PCM_16B_MONO_22K_USB       MEDIA_PCM_16B_MONO_22K_USB      ///< PCM, 16bit, mono, 22K
#define AUDIO_FORMAT_PCM_16B_MONO_24K_USB       MEDIA_PCM_16B_MONO_24K_USB      ///< PCM, 16bit, mono, 24K
#define AUDIO_FORMAT_PCM_16B_MONO_48K_USB       MEDIA_PCM_16B_MONO_48K_USB      ///< PCM, 16bit, mono, 48K
#define AUDIO_FORMAT_PCM_16B_MONO_441K_USB      MEDIA_PCM_16B_MONO_441K_USB     ///< PCM, 16bit, mono, 44.1K
#define AUDIO_FORMAT_PCM_16B_MONO_32K_USB       MEDIA_PCM_16B_MONO_32K_USB      ///< PCM, 16bit, mono, 32K
#define AUDIO_FORMAT_PCM_16B_STEREO_8K_USB      MEDIA_PCM_16B_STEREO_8K_USB     ///< PCM, 16bit, stereo, 8K
#define AUDIO_FORMAT_PCM_16B_STEREO_11K_USB     MEDIA_PCM_16B_STEREO_11K_USB    ///< PCM, 16bit, stereo, 11K
#define AUDIO_FORMAT_PCM_16B_STEREO_12K_USB     MEDIA_PCM_16B_STEREO_12K_USB    ///< PCM, 16bit, stereo, 12K
#define AUDIO_FORMAT_PCM_16B_STEREO_16K_USB     MEDIA_PCM_16B_STEREO_16K_USB    ///< PCM, 16bit, stereo, 16K
#define AUDIO_FORMAT_PCM_16B_STEREO_22K_USB     MEDIA_PCM_16B_STEREO_22K_USB    ///< PCM, 16bit, stereo, 22K
#define AUDIO_FORMAT_PCM_16B_STEREO_24K_USB     MEDIA_PCM_16B_STEREO_24K_USB    ///< PCM, 16bit, stereo, 24K
#define AUDIO_FORMAT_PCM_16B_STEREO_48K_USB     MEDIA_PCM_16B_STEREO_48K_USB    ///< PCM, 16bit, stereo, 48K
#define AUDIO_FORMAT_PCM_16B_STEREO_441K_USB    MEDIA_PCM_16B_STEREO_441K_USB   ///< PCM, 16bit, stereo, 44.1K
#define AUDIO_FORMAT_PCM_16B_STEREO_32K_USB     MEDIA_PCM_16B_STEREO_32K_USB    ///< PCM, 16bit, stereo, 32K

// for ARM2SP interface
#define AUDIO_FORMAT_PCM_16B_MONO_8K_ARM2SP     MEDIA_PCM_16B_MONO_8K_ARM2SP    ///< PCM, 16bit, mono, 8K
#define AUDIO_FORMAT_PCM_AMRNB_ARM2SP			MEDIA_PCM_AMRNB_ARM2SP    ///< PCM, 16bit, mono, 8K for AMR-NB SW decoder


#define	AUDIO_FORMAT_IMELODY				MEDIA_IMELODY				///< SMS	  ringtone
#define	AUDIO_FORMAT_EMELODY				MEDIA_EMELODY				///< SEMC   ringtone
#define	AUDIO_FORMAT_NOKIA					MEDIA_NOKIA					///< NOKIA  ringtone
#define	AUDIO_FORMAT_MFI					MEDIA_MFI					///< DoCoMo ringtone
#define	AUDIO_FORMAT_JAVA					MEDIA_JAVA					///< JAVA  tone sequence
#define AUDIO_FORMAT_MP3					MEDIA_MP3					///< MP3
#define AUDIO_FORMAT_AU						MEDIA_AU					///< AU
#define AUDIO_FORMAT_AAC					MEDIA_AAC					///< AAC
#define AUDIO_FORMAT_WMA					MEDIA_WMA					///< WMA
#define AUDIO_FORMAT_M4A					MEDIA_M4A					///< M4A
#define AUDIO_FORMAT_WAV					MEDIA_WAV					///< WAV
#define AUDIO_FORMAT_RA						MEDIA_RA
#define	AUDIO_FORMAT_UNKNOWN				MEDIA_UNKNOWN

#endif	//__AUDIO_API_H__
