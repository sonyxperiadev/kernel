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
*   @file   csl_vpu.c
*
*   @brief  This file contains CSL (DSP) API to VPU interface
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "csl_vpu.h"
#include "csl_apcmd.h"
#include "csl_dsp_common_util.h"

#define MAX_DL_SP_REC_GAIN	((2<<14)-1)	/* 6dB in DSP Q1.14 format */
#define MIN_DL_SP_REC_GAIN	0

/*********************************************************************/
/**
*
*   CSL_VPU_Enable enables VPU interface.
*
*
**********************************************************************/
void CSL_VPU_Enable(void)
{
	VPRIPCMDQ_VPUEnable();

}

/*********************************************************************/
/**
*
*   CSL_VPU_Init initializes VPU interface.
*
*   @param    speechMode			(in)	speech mode (codec and sample rate)
*   @param    numFramesPerInterrupt	(in)	number frames per interrupt
*
**********************************************************************/
void CSL_VPU_Init(UInt16 speechMode, UInt16 numFramesPerInterrupt)
{
	VPlayBack_Buffer_t *pBuf;
	UInt8 i;

	/* init the pingpong buffer */
	for (i = 0; i < 2; i++) {
		pBuf = &vp_shared_mem->shared_voice_buf.vp_buf[i];
		pBuf->nb_frame = numFramesPerInterrupt;
		pBuf->vp_speech_mode = (VP_Speech_Mode_t) speechMode;
		memset(&pBuf->vp_frame, 0, sizeof(pBuf->vp_frame));

	}

}				/* CSL_VPU_Init */

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
		       UInt16 speechMode)
{
	UInt32 frameSize;
	UInt8 *buffer = NULL;
	UInt8 i, frameCount = 0;
	UInt32 totalCopied = 0;

	if (speechMode == VP_SPEECH_MODE_LINEAR_PCM_16K)
		frameSize = WB_LIN_PCM_FRAME_SIZE * sizeof(UInt16);
	else
		frameSize = LIN_PCM_FRAME_SIZE * sizeof(UInt16);

	frameCount = vp_shared_mem->shared_voice_buf.vr_buf[bufIndex].nb_frame;

	for (i = 0; i < frameCount; i++) {
		buffer =
		    (UInt8 *) (&vp_shared_mem->shared_voice_buf.
			       vr_buf[bufIndex].vr_frame.vectors_lin_PCM[i].
			       param);
		if (outSize - totalCopied >= frameSize) {
			/* copy it from shared memeory */
			memcpy(outBuf, buffer, frameSize);
			outBuf += frameSize;
			totalCopied += frameSize;

		}

	}

	return totalCopied;

}				/* CSL_VPU_ReadPCM */

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
			UInt16 speechMode, UInt16 numFramesPerInterrupt)
{
	UInt32 frameSize;
	UInt8 *buffer = NULL;
	UInt8 i, frameCount = 0;
	UInt32 totalCopied = 0;

	frameSize = LIN_PCM_FRAME_SIZE * sizeof(UInt16);
	frameCount =
	    vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame;
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].vp_speech_mode =
	    (VP_Speech_Mode_t) speechMode;
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame =
	    numFramesPerInterrupt;

	for (i = 0; i < frameCount; i++) {
		buffer =
		    (UInt8 *) (&vp_shared_mem->shared_voice_buf.
			       vp_buf[writeIndex].vp_frame.vectors_lin_PCM[i].
			       param);
		if (inSize - totalCopied >= frameSize) {
			/* copy it to shared memeory */
			memcpy(buffer, inBuf, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize;
		} else {
			memcpy(buffer, inBuf, inSize - totalCopied);
			memset(buffer + inSize - totalCopied, 0,
			       frameSize - (inSize - totalCopied));
		}
	}

	return totalCopied;

}				/* CSL_VPU_WritePCM */

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
UInt32 CSL_VPU_ReadAMRNB(UInt8 *outBuf, UInt32 outSize, UInt16 bufIndex)
{
	UInt32 frameSize;
	UInt8 *buffer = NULL;
	UInt8 i, frameCount = 0;
	UInt32 totalCopied = 0;

	frameSize = sizeof(VR_Frame_AMR_t);

	frameCount = vp_shared_mem->shared_voice_buf.vr_buf[bufIndex].nb_frame;

	for (i = 0; i < frameCount; i++) {
		buffer =
		    (UInt8 *) (&vp_shared_mem->shared_voice_buf.
			       vr_buf[bufIndex].vr_frame.vectors_amr[i]);

		if (outSize - totalCopied >= frameSize) {
			/* copy it from shared memeory */
			memcpy(outBuf, buffer, frameSize);
			outBuf += frameSize;
			totalCopied += frameSize;

		}

	}

	return totalCopied;

}				/* CSL_VPU_ReadAMRNB */

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
			  UInt16 speechMode, UInt32 numFramesPerInterrupt)
{
	UInt32 frameSize;
	UInt8 *buffer = NULL;
	UInt8 i, frameCount = 0;
	UInt32 totalCopied = 0;

	frameSize = sizeof(VR_Frame_AMR_t);
	frameCount =
	    vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame;
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].vp_speech_mode =
	    (VP_Speech_Mode_t) speechMode;
	vp_shared_mem->shared_voice_buf.vp_buf[writeIndex].nb_frame =
	    numFramesPerInterrupt;

	for (i = 0; i < frameCount; i++) {
		buffer =
		    (UInt8 *) (&vp_shared_mem->shared_voice_buf.
			       vp_buf[writeIndex].vp_frame.vectors_amr[i]);

		if (inSize - totalCopied >= frameSize) {
			/* copy it to shared memeory */
			memcpy(buffer, inBuf, frameSize);
			inBuf += frameSize;
			totalCopied += frameSize;
		}

	}

	return totalCopied;

}				/* CSL_VPU_WriteAMRNB */

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
Boolean CSL_SetDlSpeechRecGain(Int16 mBGain)
{
	Boolean result = FALSE;
	UInt32 scale;
	UInt16 gain;
	UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale >>= (GAIN_FRACTION_BITS_NUMBER - 14);

	gain = (UInt16) scale;

	/* limit gain to DSP range */
	if (gain > MAX_DL_SP_REC_GAIN) {
		gain = MAX_DL_SP_REC_GAIN;

		result = TRUE;

	}

	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_speech_rec_gain_dl[i] = gain;


	return result;

}				/* CSL_SetDlSpeechRecGain */

/*********************************************************************/
/**
*
*   CSL_MuteDlSpeechRec mutes DL Speech Record
*
*
**********************************************************************/
void CSL_MuteDlSpeechRec(void)
{
	UInt16 i;

	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_speech_rec_gain_dl[i] =
			MIN_DL_SP_REC_GAIN;

}				/* CSL_MuteDlSpeechRec */
