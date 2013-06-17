/****************************************************************************/
/*     Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.     */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*   @file   csl_audio_capture.h
*
*   @brief  This file contains the definition for capture CSL driver layer
*
****************************************************************************/

#ifndef _CSL_AUDIO_CAPTURE_
#define _CSL_AUDIO_CAPTURE_

typedef void (*CSL_AUDCAPTURE_CB) (UInt32 streamID);

struct _CSL_CAPH_Capture_Drv_t {
	UInt32 streamID;
	CSL_CAPH_DEVICE_e source;
	CSL_CAPH_DEVICE_e sink;
	CSL_CAPH_PathID pathID;
	CSL_AUDCAPTURE_CB dmaCB;
	CSL_CAPH_DMA_CHNL_e dmaCH;
	CSL_CAPH_DMA_CHNL_e dmaCH2;	/* temp leave this back compatible */
	UInt8 *ringBuffer;
	UInt32 numBlocks;
	UInt32 blockSize;
	UInt32 blockIndex;
} ;
#define CSL_CAPH_Capture_Drv_t struct _CSL_CAPH_Capture_Drv_t
/**
*
*  @brief  initialize the audio capture block
*
*  @param  source (in) The source of the audio stream.
*  @param  sink (in) The sink of the audio stream.
*
*  @return UInt32 the capture stream ID.
*****************************************************************************/
UInt32 csl_audio_capture_init(CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink);

/**
*
*  @brief  deinitialize the audio capture
*
*  @param  streamID (in) Capture stream ID
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_audio_capture_deinit(UInt32 streamID);

/**
*
*  @brief  configure a audio capture for processing audio stream
*
*  @param   sampleRate (in) Sampling rate for this stream
*  @param   numChannels (in) number of channels
*  @param   bitsPerSample (in) bits/sample
*  @param   ringBuffer (in) start address of ring buffer
*  @param   numBlocks (in) number of blocks in ring buffer
*  @param   blockSize (in) size per each block
*  @param   csl_audio_capture_cb (in) cb registered by capture driver
*  @param   streamID (in) stream id of this stream
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_audio_capture_configure(AUDIO_SAMPLING_RATE_t sampleRate,
				     AUDIO_NUM_OF_CHANNEL_t numChannels,
				     AUDIO_BITS_PER_SAMPLE_t bitsPerSample,
				     UInt8 *ringBuffer,
				     UInt32 numBlocks,
				     UInt32 blockSize,
				     CSL_AUDCAPTURE_CB csl_audio_capture_cb,
				     UInt32 streamID);

/**
*
*  @brief  start the stream for audio capture
*
*  @param   streamID  (in) capture audio stream id
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_audio_capture_start(UInt32 streamID);

/**
*
*  @brief  stop the stream for a caph capture
*
*  @param   streamID  (in) capture audio stream id
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_audio_capture_stop(UInt32 streamID);

/**
*
*  @brief  pause the stream for audio capture
*
*  @param   streamID  (in) capture audio stream id
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_audio_capture_pause(UInt32 streamID);

/**
*
*  @brief  resume the stream for audio capture
*
*  @param   streamID  (in) capture audio stream id
*
*  @return Result_t status
*****************************************************************************/
Result_t csl_audio_capture_resume(UInt32 streamID);

CSL_CAPH_Capture_Drv_t *GetCaptureDriverByType(UInt32 streamID);

#endif /* _CSL_AUDIO_CAPTURE_ */
