/*****************************************************************************
*
* (c)1999-2011 Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2,
* available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/
/**
*
*   @file   csl_arm2sp.h
*
*   @brief  This file contains CSL (DSP) API to ARM2SP interface
*
****************************************************************************/
#ifndef _CSL_ARM2SP_H_
#define _CSL_ARM2SP_H_

/*---- Include Files -------------------------------------------------------*/
#include "mobcom_types.h"

/**
 * @addtogroup CSL ARM2SP interface
 * @{
 */

#define MAX_ARM2SP_DL_GAIN		(1<<14)		/* unity gain in DSP
							   Q1.14 format */
#define MIN_ARM2SP_DL_GAIN		0

#define MAX_ARM2SP_UL_GAIN		((2<<14)-1)	/* 6dB in DSP Q1.14
							  format */
#define MIN_ARM2SP_UL_GAIN		0

#define MAX_ARM2SP2_DL_GAIN		(1<<14)		/* unity gain in DSP
							   Q1.14 format */
#define MIN_ARM2SP2_DL_GAIN		0

#define MAX_ARM2SP2_UL_GAIN		((2<<14)-1)	/* 6dB in DSP Q1.14
							   format */
#define MIN_ARM2SP2_UL_GAIN		0

#define MAX_ARM2SP_CALL_REC_GAIN	((2<<14)-1)	/* 6dB in DSP Q1.14
							   format */
#define MIN_ARM2SP_CALL_REC_GAIN	0

#define MAX_ARM2SP2_CALL_REC_GAIN	((2<<14)-1)	/* 6dB in DSP Q1.14
							   format */
#define MIN_ARM2SP2_CALL_REC_GAIN	0

#define MAX_INP_SP_TO_ARM2SP_MIXER_DL_GAIN	(1<<14)	/* unity gain in DSP
							   Q1.14 format */
#define MIN_INP_SP_TO_ARM2SP_MIXER_DL_GAIN	0

#define MAX_INP_SP_TO_ARM2SP_MIXER_UL_GAIN	((2<<14)-1)	/* 6dB in DSP
								 Q1.14 format*/
#define MIN_INP_SP_TO_ARM2SP_MIXER_UL_GAIN	0

typedef enum CSL_ARM2SP_PLAYBACK_MODE_t {
	CSL_ARM2SP_PLAYBACK_NONE,
	CSL_ARM2SP_PLAYBACK_DL,
	CSL_ARM2SP_PLAYBACK_UL,
	CSL_ARM2SP_PLAYBACK_BOTH
} CSL_ARM2SP_PLAYBACK_MODE_t;

typedef enum CSL_ARM2SP_VOICE_MIX_MODE_t {
	CSL_ARM2SP_VOICE_MIX_NONE,
	CSL_ARM2SP_VOICE_MIX_DL,
	CSL_ARM2SP_VOICE_MIX_UL,
	CSL_ARM2SP_VOICE_MIX_BOTH
} CSL_ARM2SP_VOICE_MIX_MODE_t;

#define CSL_ARM2SP_DL_AFTER_AUDIO_PROC	1
#define CSL_ARM2SP_DL_BEFORE_AUDIO_PROC	0

#define CSL_ARM2SP_UL_AFTER_AUDIO_PROC	0
#define CSL_ARM2SP_UL_BEFORE_AUDIO_PROC	1


/* ---- Function Declarations ---------------------------------------*/
/*********************************************************************/
/**
*
*   CSL_ARM2SP_Init initializes  ARM2SP input buffer.
*
*
**********************************************************************/
void CSL_ARM2SP_Init(void);

/*********************************************************************/
/**
*
*   CSL_ARM2SP_Write writes data to shared memory for ARM2SP voice play.
*
*   @param	inBuf		(in)	source buffer
*   @param	inSize_inBytes  (in)	data size to write
*   @param	writeIndex	(in)	index of ping-pong buffer
*   @param	in48K		(in)	48K signal ?
*   @param	audMode		(in)	stereo ?
*   @return   UInt32			number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP_Write(UInt8 *inBuf, UInt32 inSize_inBytes,
		UInt16 writeIndex, Boolean in48K, UInt8 audMode);

/*********************************************************************/
/**
*
*   CSL_ARM2SP2_Init initializes  ARM2SP2 input buffer.
*
*
**********************************************************************/
void CSL_ARM2SP2_Init(void);

/*********************************************************************/
/**
*
*   CSL_ARM2SP2_Write writes data to shared memory for ARM2SP2 voice play.
*
*   @param	inBuf		(in)	source buffer
*   @param	inSize_inBytes  (in)	data size to write
*   @param	writeIndex	(in)	index of ping-pong buffer
*   @param	in48K		(in)	48K signal ?
*   @param	audMode		(in)	stereo ?
*   @return	 UInt32			number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP2_Write(UInt8 *inBuf, UInt32 inSize_inBytes,
	UInt16 writeIndex,
	Boolean in48K, UInt8 audMode);

/*********************************************************************/
/**
*
*   CSL_SetARM2SpeechDLGain sets ARM2SP downlink gain.
*
*   @param	mBGain	(in)	gain in millibels
*				(min = -8430 millibel,
*				 max = 0 millibel)
*   @return   Boolean		TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2SpeechDLGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteARM2SpeechDL mutes ARM2SP downlink.
*
*
**********************************************************************/
void CSL_MuteARM2SpeechDL(void);

/*********************************************************************/
/**
*
*   CSL_SetARM2SpeechULGain sets ARM2SP uplink gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2SpeechULGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteARM2SpeechUL mutes ARM2SP uplink.
*
*
**********************************************************************/
void CSL_MuteARM2SpeechUL(void);

/*********************************************************************/
/**
*
*   CSL_SetARM2Speech2DLGain sets ARM2SP2 downlink gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2Speech2DLGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteARM2Speech2DL mutes ARM2SP2 downlink.
*
*
**********************************************************************/
void CSL_MuteARM2Speech2DL(void);

/*********************************************************************/
/**
*
*   CSL_SetARM2Speech2ULGain sets ARM2SP2 uplink gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2Speech2ULGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteARM2Speech2UL mutes ARM2SP2 uplink.
*
*
**********************************************************************/
void CSL_MuteARM2Speech2UL(void);

/*********************************************************************/
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerDLGain sets Downlink Input Speech
*   Gain to ARM2SP mixer.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetInpSpeechToARM2SpeechMixerDLGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerDL mutes Downlink Input Speech
*   to ARM2SP mixer.
*
*   @param	None
*   @return   None
*
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerDL(void);

/*********************************************************************/
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerULGain sets Uplink Input Speech
*   Gain to ARM2SP mixer.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetInpSpeechToARM2SpeechMixerULGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerUL mutes Uplink Input Speech
*   to ARM2SP mixer.
*
*   @param	None
*   @return   None
*
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerUL(void);

/*********************************************************************/
/**
*
*   CSL_SetARM2SpeechCallRecordGain sets ARM2SP Call Record gain.
*
*   @param	mBGain				(in)	gain in millibels \
*						(min = -8430 millibel,
*						 max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2SpeechCallRecordGain(Int16 mBGain);

/***********************************************************************/
/**
*
*   CSL_MuteARM2SpeechCallRecord mutes ARM2SP Call Record
*
*
**********************************************************************/
void CSL_MuteARM2SpeechCallRecord(void);

/***********************************************************************/
/**
*
*   CSL_SetARM2Speech2CallRecordGain sets ARM2SP2 Call Record gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2Speech2CallRecordGain(Int16 mBGain);

/***********************************************************************/
/**
*
*   CSL_MuteARM2Speech2CallRecord mutes ARM2SP2 Call Record
*
*
***********************************************************************/
void CSL_MuteARM2Speech2CallRecord(void);

/**********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp_get_phy_base_addr
*
*   @note	This function returns the base address of the low part of
*		ARM2SP Input Buffer
*		for programming the AADMAC (this function should not be
*		used for any software access).
*
*   @return	Physical Base address of the low half of the ARM2SP input
*		buffer
*
**/
/**********************************************************************/
UInt16 *csl_dsp_arm2sp_get_phy_base_addr(void);

/**********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp2_get_phy_base_addr
*
*   @note	This function returns the base address of the low part
*		of ARM2SP2 Input Buffer for programming the AADMAC
*		(this function should not be used for any software
*		access).
*
*   @return	Physical Base address of the low half of the ARM2SP2
*		input buffer
*
**/
/**********************************************************************/
UInt16 *csl_dsp_arm2sp2_get_phy_base_addr(void);

/**********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp_get_size
*
*   @note	 This function returns the size of the whole of ARM2SP Input Buffer
*			 for programming the AADMAC
*
*   @param	Rate = 8000, 16000 or 48000
*   @return   Size of the entire ARM2SP input buffer
*
**/
/**********************************************************************/
UInt16 csl_dsp_arm2sp_get_size(UInt32 rate);

/**********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp2_get_size
*
*   @note	This function returns the size of the whole of ARM2SP2
*		Input Buffer for programming the AADMAC
*
*   @param	Rate = 8000, 16000 or 48000
*   @return   Size of the entire ARM2SP2 input buffer
*
**/
/**********************************************************************/
UInt16 csl_dsp_arm2sp2_get_size(UInt32 Rate);

/**********************************************************************/
/**
*
* Function Name: csl_arm2sp_set_arm2sp
*
*   @note	 This function Starts and Stops the ARM2SP interface.
*
*   @param	UInt32 Rate = 8000, 16000 or 48000
*   @param	CSL_ARM2SP_PLAYBACK_MODE_t playbackMode
*   @param	CSL_ARM2SP_VOICE_MIX_MODE_t mixMode
*   @param	UInt32 numFramesPerInterrupt
*   @param	UInt8 audMode	= 0 -> Mono \BR
*				= 1 -> Stereo
*   @param	UInt16 Reset_out_ptr_flag \BR
*			=0, reset output pointer - shared_Arm2SP2_InBuf_out
*				- of buffer shared_Arm2SP2_InBuf[] to 0.
*				Used for new arm2sp2 session.\BR
*			=1, keep output pointer - shared_Arm2SP2_InBuf_out
*				- unchange.
*				Used for PAUSE/RESUME the same arm2sp2 session.
*   @param	UInt16 dl_mix_or_repl_location
*   @param	UInt16 ul_mix_or_repl_location
*
*   @return   None
*
**/
/**********************************************************************/
void csl_arm2sp_set_arm2sp(UInt32			samplingRate,
				CSL_ARM2SP_PLAYBACK_MODE_t	playbackMode,
				CSL_ARM2SP_VOICE_MIX_MODE_t	mixMode,
				UInt32			numFramesPerInterrupt,
				UInt8			audMode,
				UInt16			Reset_out_ptr_flag,
				UInt16			dl_mix_or_repl_location,
				UInt16			ul_mix_or_repl_location
				);

/**********************************************************************/
/**
*
* Function Name: csl_arm2sp_set_arm2sp2
*
*   @note	 This function Starts and Stops the ARM2SP2 interface.
*
*   @param	UInt32 Rate = 8000, 16000 or 48000
*   @param	CSL_ARM2SP_PLAYBACK_MODE_t playbackMode
*   @param	CSL_ARM2SP_VOICE_MIX_MODE_t mixMode
*   @param	UInt32 numFramesPerInterrupt
*   @param	UInt8 audMode	= 0 -> Mono \BR
*				= 1 -> Stereo
*   @param	UInt16 Reset_out_ptr_flag \BR
*			=0, reset output pointer - shared_Arm2SP2_InBuf_out
*				- of buffer shared_Arm2SP2_InBuf[] to 0.
*				Used for new arm2sp2 session.\BR
*			=1, keep output pointer - shared_Arm2SP2_InBuf_out
*				- unchange.
*				Used for PAUSE/RESUME the same arm2sp2 session.
*   @param	UInt16 dl_mix_or_repl_location
*   @param	UInt16 ul_mix_or_repl_location
*
*   @return   None
*
**/
/**********************************************************************/
void csl_arm2sp_set_arm2sp2(UInt32			samplingRate,
				CSL_ARM2SP_PLAYBACK_MODE_t	playbackMode,
				CSL_ARM2SP_VOICE_MIX_MODE_t	mixMode,
				UInt32			numFramesPerInterrupt,
				UInt8			audMode,
				UInt16			Reset_out_ptr_flag,
				UInt16			dl_mix_or_repl_location,
				UInt16			ul_mix_or_repl_location
				);

/** @} */

#endif /*_CSL_ARM2SP_H_*/
