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
*   @file   csl_vpu.h
*
*   @brief  This file contains DSP API to VPU interface
*
****************************************************************************/
#ifndef _CSL_VPU_H_
#define _CSL_VPU_H_

/* ---- Include Files ----------------------------------------------------- */
#include "mobcom_types.h"
#include "shared.h"

extern AP_SharedMem_t *vp_shared_mem;

typedef enum {
	CSL_VP_SPEECH_MODE_NONE = 0,
	CSL_VP_SPEECH_MODE_FR = 1,
	CSL_VP_SPEECH_MODE_EFR = 2,
	CSL_VP_SPEECH_MODE_LINEAR_PCM_8K = 3,
	CSL_VP_SPEECH_MODE_AMR = 4,
	CSL_VP_SPEECH_MODE_AMR_2 = 5,
	CSL_VP_SPEECH_MODE_LINEAR_PCM_16K = 6
} CSL_VP_Speech_Mode_t;


/**
 * @addtogroup CSL VPU Interface
 * @{
 */

/* ---- Function Declarations ----------------------------------------- */

/*********************************************************************/
/**
*
*   CSL_VPU_Enable enables VPU interface.
*
*
**********************************************************************/
void CSL_VPU_Enable(void);

/*********************************************************************/
/**
*
*   CSL_VPU_Init initializes VPU interface.
*
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*
**********************************************************************/
void CSL_VPU_Init(UInt16 speechMode, UInt16 numFramesPerInterrupt);

/*********************************************************************/
/**
*
*   CSL_VPU_ReadPCM reads PCM data from shared memory for VPU voice record.
*
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex		(in)	index of ping-pong buffer
*   @param    speechMode	(in)	speech mode (codec and sample rate)
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_VPU_ReadPCM(UInt8 *outBuf, UInt32 outSize, UInt16 bufIndex,
		       UInt16 speechMode);
/*********************************************************************/
/**
*
*   CSL_VPU_WritePCM writes PCM data to shared memory for VPU voice playback.
*
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex			(in)	index of ping-pong buffer
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_VPU_WritePCM(UInt8 *inBuf, UInt32 inSize, UInt16 writeIndex,
			UInt16 speechMode, UInt16 numFramesPerInterrupt);

/*********************************************************************/
/**
*
*   CSL_VPU_ReadAMRNB reads AMR-NB data from shared memory for VPU voice record.
*
*   @param    outBuf		(out)	destination buffer
*   @param    outSize		(in)	data size to read
*   @param    bufIndex		(in)	index of ping-pong buffer
*   @return   UInt32				number of bytes read to the buffer
*
**********************************************************************/
UInt32 CSL_VPU_ReadAMRNB(UInt8 *outBuf, UInt32 outSize, UInt16 bufIndex);

/*********************************************************************/
/**
*
*   CSL_VPU_WriteAMRNB writes AMR-NB data to shared memory for VPU playback.
*
*   @param    inBuf					(in)	source buffer
*   @param    inSize				(in)	data size to read
*   @param    writeIndex			(in)	index of ping-pong buffer
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*   @return   UInt32				number of bytes written from the buffer
*
**********************************************************************/
UInt32 CSL_VPU_WriteAMRNB(UInt8 *inBuf, UInt32 inSize, UInt16 writeIndex,
			  UInt16 speechMode, UInt32 numFramesPerInterrupt);

/*********************************************************************/
/**
*
*   CSL_SetDlSpeechRecGain sets DL Speech Record gain.
*
*   @param    mBGain				(in)	gain in millibels
*	(min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetDlSpeechRecGain(Int16 mBGain);

/*********************************************************************/
/**
*
*   CSL_MuteDlSpeechRec mutes DL Speech Record
*
*
**********************************************************************/
void CSL_MuteDlSpeechRec(void);


/** @} */

#endif /* _CSL_VPU_H_ */
