/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
/**
*
*   @file   csl_volume.h
*
*   @brief  This file contains DSP CSL API for audio volume control.
*
****************************************************************************/
#ifndef _CSL_VOLUME_H_
#define _CSL_VOLUME_H_

/* ---- Include Files ------------------------------------------------------- */
#include "mobcom_types.h"

#define DSP_MUTE_GAIN	-10000	/* -10000mB = -100dB */

#define MAX_DL_GAIN		(1<<6)	/* unity gain in DSP Q9.6 format */
#define MIN_DL_GAIN		0

#define MAX_UL_GAIN		(64<<6)	/* 36dB in DSP Q9.6 format */
#define MIN_UL_GAIN		0

#define MAX_UL_SPEECH_REC_GAIN	((2<<14)-1)	/* 6dB in DSP Q1.14 format */
#define MIN_UL_SPEECH_REC_GAIN	0

#define MAX_WBAMR_GAIN	(1<<6)	/* unity gain in DSP Q9.6 format */
#define MIN_WBAMR_GAIN	0

extern Dsp_SharedMem_t *cp_shared_mem;

/**
 * @addtogroup CSL Voulume Control
 * @{
 */

/* ---- Function Declarations ----------------------------------------- */

/*********************************************************************/
/**
*
*   CSL_InitGain initializes downlink and uplink gains.
*
*   @param    dlGain			(in)	downlink gain
*   @param    ulGain			(in)	uplink gain
*
**********************************************************************/
void CSL_InitGain(Int16 dlGain, Int16 ulGain);

/*********************************************************************/
/**
*
*   CSL_InitUnityGain initializes unity gains.
*
*
**********************************************************************/
void CSL_InitUnityGain(void);

/*********************************************************************/
/**
*
*   CSL_GetDLGain gets downlink gain.
*
*   @param    none
*   @return   int16  gain in millibels
*	(min = -3612 millibel, max = 3612 millibel)
*
**********************************************************************/
Int16 CSL_GetDLGain(void);

/*********************************************************************/
/**
*
*   CSL_SetDLGain sets downlink gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -3612 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetDLGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteDL mutes downlink.
*
*
**********************************************************************/
void CSL_MuteDL(void);

/*********************************************************************/
/**
*
*   CSL_GetULGain gets uplink gain.
*
*   @param    none
*   @return   int16  gain in millibels
*	(min = -3612 millibel, max = 3612 millibel)
*
**********************************************************************/
Int16 CSL_GetULGain(void);

/*********************************************************************/
/**
*
*   CSL_SetULGain sets uplink gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -3612 millibel, max = 3612 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetULGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteUL mutes uplink.
*
*
**********************************************************************/
void CSL_MuteUL(void);

/*********************************************************************/
/**
*
*   CSL_SetULSpeechRecordGain sets uplink speech record gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -8428 millibel, max = 602 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetULSpeechRecordGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteULSpeechRecord mutes uplink speech record.
*
*
**********************************************************************/
void CSL_MuteULSpeechRecord(void);

/*********************************************************************/
/**
*
*   CSL_SetWBAMROutputGain sets WB-AMR output gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -3612 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetWBAMROutputGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteWBAMROutput mutes WB-AMR output.
*
*
**********************************************************************/
void CSL_MuteWBAMROutput(void);

/*********************************************************************/
/**
*
*   CSL_SetSpeakerInputGain sets speaker input gain
*
*   @param    inputGain				(in)	input gain
*
**********************************************************************/
void CSL_SetSpeakerInputGain(UInt16 inputGain);

/*********************************************************************/
/**
*
*   CSL_SetDigitalInputClipLevel sets digital input clip level
*
*   @param    clipLevel				(in)	clip level
*
**********************************************************************/
void CSL_SetDigitalInputClipLevel(UInt16 clipLevel);

/*********************************************************************/
/**
*
*   CSL_SetUplinkClipLevel sets uplink clip level
*
*   @param    clipLevel				(in)	clip level
*
**********************************************************************/
void CSL_SetUplinkClipLevel(UInt16 clipLevel);

/** @} */

#endif /*_CSL_VOLUME_H_ */
