/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#ifndef __MEDIATYPES_H__
#define __MEDIATYPES_H__

typedef enum
{
	MEDIA_UNKNOWN,

	// Image
	MEDIA_IMAGE_FIRST,
	MEDIA_JPEG = MEDIA_IMAGE_FIRST,		///< JFIF JPEG
	MEDIA_GIF,							///< Graphic Interchangable Format
	MEDIA_PNG,							///< Portable Network Graphics
	MEDIA_WBMP,							///< Wireless Bitmap

	// EMS Image
	MEDIA_EMS_IMAGE_FIRST,
	MEDIA_EMS_BMP = MEDIA_EMS_IMAGE_FIRST,
	MEDIA_EMS_PREDEF_ANI,				///< Pre-defined EMS animation.
	MEDIA_EMS_USRDEF_ANI,				///< User-defined EMS animation.
	MEDIA_IMAGE_LAST = MEDIA_EMS_USRDEF_ANI,

	// Audio
	MEDIA_AUDIO_FIRST,
	MEDIA_MIDI = MEDIA_AUDIO_FIRST,	///< Standard MIDI format
	MEDIA_SP_MIDI,
	MEDIA_AMR,						///< Adjustable Multi-Rate Narrow-Band
	MEDIA_AMR_WB,					///< Adjustable Multi-Rate Wide-Band
	MEDIA_WAV,						///< WAV
	MEDIA_VMD,
	MEDIA_SMAF,						///< SMAF for MA3
	MEDIA_PCM_8B_MONO_8K,			///< PCM, 8bit, mono, 8K
	MEDIA_PCM_16B_MONO_8K,			///< PCM, 16bit, mono, 8K
	MEDIA_PCM_16B_MONO_11K,			///< PCM, 16bit, mono, 11K
	MEDIA_PCM_16B_MONO_12K,			///< PCM, 16bit, mono, 12K
	MEDIA_PCM_16B_MONO_16K,			///< PCM, 16bit, mono, 16K
	MEDIA_PCM_16B_MONO_22K,			///< PCM, 16bit, mono, 22K
	MEDIA_PCM_16B_MONO_24K,			///< PCM, 16bit, mono, 24K
	MEDIA_PCM_16B_MONO_48K,			///< PCM, 16bit, mono, 48K
	MEDIA_PCM_16B_MONO_441K,		///< PCM, 16bit, mono, 44.1K
	MEDIA_PCM_16B_MONO_32K,			///< PCM, 16bit, mono, 32K
	MEDIA_PCM_16B_STEREO_8K,		///< PCM, 16bit, stereo, 8K
	MEDIA_PCM_16B_STEREO_11K,		///< PCM, 16bit, stereo, 11K
	MEDIA_PCM_16B_STEREO_12K,		///< PCM, 16bit, stereo, 12K
	MEDIA_PCM_16B_STEREO_16K,		///< PCM, 16bit, stereo, 16K
	MEDIA_PCM_16B_STEREO_22K,		///< PCM, 16bit, stereo, 22K
	MEDIA_PCM_16B_STEREO_24K,		///< PCM, 16bit, stereo, 24K
	MEDIA_PCM_16B_STEREO_48K,		///< PCM, 16bit, stereo, 48K
	MEDIA_PCM_16B_STEREO_441K,		///< PCM, 16bit, stereo, 44.1K
	MEDIA_PCM_16B_STEREO_32K,		///< PCM, 16bit, stereo, 32K

    // Audio defined for Bluetooth stereo headset
    MEDIA_PCM_8B_MONO_8K_BTS,           ///< PCM, 8bit, mono, 8K
    MEDIA_PCM_16B_MONO_8K_BTS,          ///< PCM, 16bit, mono, 8K
    MEDIA_PCM_16B_MONO_11K_BTS,         ///< PCM, 16bit, mono, 11K
    MEDIA_PCM_16B_MONO_12K_BTS,         ///< PCM, 16bit, mono, 12K
    MEDIA_PCM_16B_MONO_16K_BTS,         ///< PCM, 16bit, mono, 16K
    MEDIA_PCM_16B_MONO_22K_BTS,         ///< PCM, 16bit, mono, 22K
    MEDIA_PCM_16B_MONO_24K_BTS,         ///< PCM, 16bit, mono, 24K
    MEDIA_PCM_16B_MONO_48K_BTS,         ///< PCM, 16bit, mono, 48K
    MEDIA_PCM_16B_MONO_441K_BTS,        ///< PCM, 16bit, mono, 44.1K
    MEDIA_PCM_16B_MONO_32K_BTS,         ///< PCM, 16bit, mono, 32K
    MEDIA_PCM_16B_STEREO_8K_BTS,        ///< PCM, 16bit, stereo, 8K
    MEDIA_PCM_16B_STEREO_11K_BTS,       ///< PCM, 16bit, stereo, 11K
    MEDIA_PCM_16B_STEREO_12K_BTS,       ///< PCM, 16bit, stereo, 12K
    MEDIA_PCM_16B_STEREO_16K_BTS,       ///< PCM, 16bit, stereo, 16K
    MEDIA_PCM_16B_STEREO_22K_BTS,       ///< PCM, 16bit, stereo, 22K
    MEDIA_PCM_16B_STEREO_24K_BTS,       ///< PCM, 16bit, stereo, 24K
    MEDIA_PCM_16B_STEREO_48K_BTS,       ///< PCM, 16bit, stereo, 48K
    MEDIA_PCM_16B_STEREO_441K_BTS,      ///< PCM, 16bit, stereo, 44.1K
    MEDIA_PCM_16B_STEREO_32K_BTS,       ///< PCM, 16bit, stereo, 32K

    MEDIA_PCM_USB,   			    ///< PCM to USB device
    // Audio defined for USB stereo headset
    MEDIA_PCM_8B_MONO_8K_USB,           ///< PCM, 8bit, mono, 8K
    MEDIA_PCM_16B_MONO_8K_USB,          ///< PCM, 16bit, mono, 8K
    MEDIA_PCM_16B_MONO_11K_USB,         ///< PCM, 16bit, mono, 11K
    MEDIA_PCM_16B_MONO_12K_USB,         ///< PCM, 16bit, mono, 12K
    MEDIA_PCM_16B_MONO_16K_USB,         ///< PCM, 16bit, mono, 16K
    MEDIA_PCM_16B_MONO_22K_USB,         ///< PCM, 16bit, mono, 22K
    MEDIA_PCM_16B_MONO_24K_USB,         ///< PCM, 16bit, mono, 24K
    MEDIA_PCM_16B_MONO_48K_USB,         ///< PCM, 16bit, mono, 48K
    MEDIA_PCM_16B_MONO_441K_USB,        ///< PCM, 16bit, mono, 44.1K
    MEDIA_PCM_16B_MONO_32K_USB,         ///< PCM, 16bit, mono, 32K
    MEDIA_PCM_16B_STEREO_8K_USB,        ///< PCM, 16bit, stereo, 8K
    MEDIA_PCM_16B_STEREO_11K_USB,       ///< PCM, 16bit, stereo, 11K
    MEDIA_PCM_16B_STEREO_12K_USB,       ///< PCM, 16bit, stereo, 12K
    MEDIA_PCM_16B_STEREO_16K_USB,       ///< PCM, 16bit, stereo, 16K
    MEDIA_PCM_16B_STEREO_22K_USB,       ///< PCM, 16bit, stereo, 22K
    MEDIA_PCM_16B_STEREO_24K_USB,       ///< PCM, 16bit, stereo, 24K
    MEDIA_PCM_16B_STEREO_48K_USB,       ///< PCM, 16bit, stereo, 48K
    MEDIA_PCM_16B_STEREO_441K_USB,      ///< PCM, 16bit, stereo, 44.1K
    MEDIA_PCM_16B_STEREO_32K_USB,       ///< PCM, 16bit, stereo, 32K

    // Audio defined for ARM2SP interface
    MEDIA_PCM_16B_MONO_8K_ARM2SP,       ///< PCM, 16bit, mono, 8K
    MEDIA_PCM_AMRNB_ARM2SP,       ///< PCM, 16bit, mono, 8K for AMR-NB SW decoder

	MEDIA_NOKIA,					///< NOKIA  ringtone
	MEDIA_MFI,						///< DoCoMo ringtone
	MEDIA_JAVA,						///< JAVA   tone sequence
	MEDIA_IMELODY,					///< SMS	ringtone
	MEDIA_EMELODY,					///< SEMC	ringtone
	MEDIA_MP3,
	MEDIA_AU,
	MEDIA_AAC,
	MEDIA_WMA,
	MEDIA_M4A,
	MEDIA_RA,


	// EMS Audio
	MEDIA_EMS_AUDIO_FIRST,
	MEDIA_EMS_MLD = MEDIA_EMS_AUDIO_FIRST,
	MEDIA_EMS_PREDEF_MLD,
	MEDIA_AUDIO_LAST = MEDIA_EMS_PREDEF_MLD,

	// Video
	MEDIA_VIDEO_FIRST,
	MEDIA_MP4 = MEDIA_VIDEO_FIRST,
	MEDIA_3GP2,
	MEDIA_3GP,
	MEDIA_RM,
	MEDIA_WMV,
	MEDIA_VIDEO_LAST = MEDIA_WMV,

	// PIM
	MEDIA_SMIL,
	MEDIA_VCARD,
	MEDIA_VCAL,

	// URL Bookmark
	MEDIA_VBOOKMARK,

	MEDIA_MIME_MIXED,
	MEDIA_MIME_RELATED,
	MEDIA_MIME_ALTER,

	MEDIA_TEXT,
	MEDIA_BINARY,
	MEDIA_THEME,

	MEDIA_MPLAYLIST,

	MEDIA_JAVA_JAD,
	MEDIA_JAVA_JAR,

	// DRM
	MEDIA_DRM_FIRST,
	MEDIA_DRM_MSG = MEDIA_DRM_FIRST,	///< OMA 1 DRM message
	MEDIA_DRM_DCF1,						///< OMA 1 DRM Content Format file
	MEDIA_DRM_RO_XML,					///< OMA 1 DRM Rights Object (XML)
	MEDIA_DRM_RO_WBXML,					///< OMA 1 DRM Rights Object (WBXML)
	MEDIA_DRM_DCF2,						///< OMA 2 DRM Content Format file
	MEDIA_DRM_ROAP_TRIGGER,				///< OMA 2 DRM ROAP Trigger
	MEDIA_DRM_OMA2_RO_XML,				///< OMA 2 DRM Rights Object (XML)
	MEDIA_DRM_LAST = MEDIA_DRM_OMA2_RO_XML,

	MEDIA_IMAGE_NO_EMS,
	MEDIA_ALL_IMAGE,
	MEDIA_AUDIO_NO_EMS,
	MEDIA_ALL_AUDIO,
	MEDIA_ALL_OBJECT,
	MEDIA_ALL_VIDEO,

	MEDIA_TOTAL
} Media_t;

#endif		// __MEDIATYPES_H__

