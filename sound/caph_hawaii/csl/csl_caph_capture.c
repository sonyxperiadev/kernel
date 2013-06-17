/******************************************************************************/
/* Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.           */
/*     Unless you and Broadcom execute a separate written software license    */
/*     agreement governing use of this software, this software is licensed to */
/*     you under the terms of the GNU General Public License version 2        */
/*    (the GPL), available at                                                 */
/*                                                                            */
/*          http://www.broadcom.com/licenses/GPLv2.php                        */
/*                                                                            */
/*     with the following added to such license:                              */
/*                                                                            */
/*     As a special exception, the copyright holders of this software give    */
/*     you permission to link this software with independent modules, and to  */
/*     copy and distribute the resulting executable under terms of your       */
/*     choice, provided that you also meet, for each linked independent       */
/*     module, the terms and conditions of the license of that module.        */
/*     An independent module is a module which is not derived from this       */
/*     software.  The special exception does not apply to any modifications   */
/*     of the software.                                                       */
/*                                                                            */
/*     Notwithstanding the above, under no circumstances may you combine this */
/*     software in any way with any other Broadcom software provided under a  */
/*     license other than the GPL, without Broadcom's express prior written   */
/*     consent.                                                               */
/******************************************************************************/

/**
*
*  @file   csl_caph_capture.c
*
*  @brief  CSL layer driver for caph capture
*
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "msconsts.h"
#include "audio_consts.h"
#include "csl_caph.h"
#include "csl_caph_dma.h"
#include "csl_caph_hwctrl.h"
#include "csl_audio_capture.h"
#include "audio_trace.h"

/***************************************************************************/
/*                       G L O B A L   S E C T I O N                       */
/***************************************************************************/

/***************************************************************************/
/*lobal variable definitions                                               */
/***************************************************************************/

/***************************************************************************/
/*                        L O C A L   S E C T I O N                        */
/***************************************************************************/

/***************************************************************************/
/*local macro declarations                                                 */
/***************************************************************************/
/***************************************************************************/
/*local typedef declarations                                               */
/***************************************************************************/

/***************************************************************************/
/*local variable definitions                                               */
/***************************************************************************/
static CSL_CAPH_Capture_Drv_t sCaptureDrv[CSL_CAPH_STREAM_TOTAL] = { {0} };

/***************************************************************************/
/*local function declarations                                              */
/***************************************************************************/

static CSL_CAPH_STREAM_e GetStreamIDByDmaCH(CSL_CAPH_DMA_CHNL_e dmaCH);
static void AUDIO_DMA_CB(CSL_CAPH_DMA_CHNL_e chnl);

/***************************************************************************/
/*local function declarations                                              */
/***************************************************************************/

/****************************************************************************
*
*  Function Name:UInt32 csl_audio_capture_init(CSL_CAPH_DEVICE_e source,
*                                                        CSL_CAPH_DEVICE_e sink)
*
*  Description: init CAPH capture block
*
****************************************************************************/
UInt32 csl_audio_capture_init(CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
	UInt32 streamID = CSL_CAPH_STREAM_NONE;
	CSL_CAPH_Capture_Drv_t	*audDrv = NULL;

	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_audio_capture_init::source=0x%x sink=0x%x.\n", source,
	       sink);
	/* allocate a unique streamID */
	streamID = (UInt32) csl_caph_hwctrl_AllocateStreamID();
	audDrv = GetCaptureDriverByType(streamID);

	memset(audDrv, 0, sizeof(CSL_CAPH_Capture_Drv_t));

	audDrv->streamID = streamID;
	audDrv->source = source;
	audDrv->sink = sink;
	spin_lock_init(&audDrv->configLock);

	return audDrv->streamID;
}

/****************************************************************************
*
*  Function Name:Result_t csl_audio_capture_deinit(UInt32 streamID)
*
*  Description: De-initialize CSL capture layer
*
****************************************************************************/
Result_t csl_audio_capture_deinit(UInt32 streamID)
{
	CSL_CAPH_Capture_Drv_t *audDrv = NULL;
	unsigned long flags;

	audDrv = GetCaptureDriverByType(streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

	aTrace
	      (LOG_AUDIO_CSL,
	       "csl_audio_capture_deinit::dmaCH=0x%x, dmaCH2-0x%x\n",
	       audDrv->dmaCH, audDrv->dmaCH2);

	audDrv->streamID = 0;
	audDrv->source = CSL_CAPH_DEV_NONE;
	audDrv->sink = CSL_CAPH_DEV_NONE;
	audDrv->pathID = 0;

	spin_lock_irqsave(&audDrv->configLock, flags);
	audDrv->dmaCB = NULL;
	spin_unlock_irqrestore(&audDrv->configLock, flags);

	audDrv->dmaCH = 0;
	audDrv->dmaCH2 = 0;
	audDrv->ringBuffer = NULL;
	audDrv->numBlocks = 0;
	audDrv->blockSize = 0;
	audDrv->blockIndex = 0;

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_audio_capture_configure
*
*  Description: Configure the CAPH capture
*
****************************************************************************/
Result_t csl_audio_capture_configure(AUDIO_SAMPLING_RATE_t sampleRate,
				     AUDIO_NUM_OF_CHANNEL_t numChannels,
				     AUDIO_BITS_PER_SAMPLE_t bitsPerSample,
				     UInt8 *ringBuffer,
				     UInt32 numBlocks,
				     UInt32 blockSize,
				     CSL_AUDCAPTURE_CB csl_audio_capture_cb,
				     UInt32 streamID)
{
	CSL_CAPH_Capture_Drv_t *audDrv = NULL;
	CSL_CAPH_HWCTRL_STREAM_REGISTER_t stream;
	unsigned long flags;

	aTrace
	      (LOG_AUDIO_CSL, "csl_audio_capture_configure::\n");

	audDrv = GetCaptureDriverByType(streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

	/*audDrv->ringBuffer = ringBuffer;
	audDrv->blockSize = blockSize;
	audDrv->numBlocks = numBlocks;*/

	/* : make sure ringbuffer, numblocks and
	block size are legal for Rhea */
	memset(&stream, 0, sizeof(CSL_CAPH_HWCTRL_STREAM_REGISTER_t));
	stream.streamID = (CSL_CAPH_STREAM_e) audDrv->streamID;
#if defined(CONFIG_MAP_VOIP)
		stream.src_sampleRate = AUDIO_SAMPLING_RATE_48000;
#else
		stream.src_sampleRate = sampleRate;
#endif
	stream.snk_sampleRate = sampleRate;	/* make it same as source */
	stream.chnlNum = numChannels;
	stream.bitPerSample = bitsPerSample;
	stream.pBuf = ringBuffer;
	stream.pBuf2 = NULL;
	stream.size = numBlocks * blockSize;
	stream.dmaCB = AUDIO_DMA_CB;

	if (audDrv->source == CSL_CAPH_DEV_DSP
	    && audDrv->sink == CSL_CAPH_DEV_MEMORY) {
		aTrace(LOG_AUDIO_CSL,
			"csl_audio_capture_configure::USB call?"
			"reset src_sampleRate from %u to 8000.\r\n",
			stream.src_sampleRate);
		stream.src_sampleRate = AUDIO_SAMPLING_RATE_8000;
	}
	audDrv->pathID = csl_caph_hwctrl_RegisterStream(&stream);
	if (audDrv->pathID == 0) {
		audio_xassert(0, audDrv->pathID);
		return RESULT_ERROR;
	}
	audDrv->dmaCH = csl_caph_hwctrl_GetdmaCH(audDrv->pathID);

	spin_lock_irqsave(&audDrv->configLock, flags);
	audDrv->dmaCB = csl_audio_capture_cb;
	spin_unlock_irqrestore(&audDrv->configLock, flags);

	audDrv->ringBuffer = ringBuffer;
	audDrv->numBlocks = numBlocks;
	audDrv->blockSize = blockSize;
	/* assume everytime it starts, the first 2 buffers will be filled
	when the interrupt comes, it will start from buffer 2
	*/
	audDrv->blockIndex = 1;

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_capture_start
*
*  Description: Start the data transfer of audio path capture
*
****************************************************************************/
Result_t csl_audio_capture_start(UInt32 streamID)
{
	CSL_CAPH_Capture_Drv_t *audDrv = NULL;

	aTrace
	      (LOG_AUDIO_CSL, "csl_audio_capture_start::streamID=0x%lx\n",
	       streamID);
	audDrv = GetCaptureDriverByType(streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

	(void)csl_caph_hwctrl_StartPath(audDrv->pathID);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_capture_stop
*
*  Description: Stop the data transfer of audio path capture
*
****************************************************************************/
Result_t csl_audio_capture_stop(UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	aTrace
	      (LOG_AUDIO_CSL, "csl_audio_capture_stop::streamID=0x%lx\n",
	       streamID);

	config.streamID = (CSL_CAPH_STREAM_e)streamID;
	(void)csl_caph_hwctrl_DisablePath(config);
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_capture_pause
*
*  Description: Pause the data transfer of audio path capture
*
****************************************************************************/
Result_t csl_audio_capture_pause(UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	aTrace
	      (LOG_AUDIO_CSL, "csl_audio_capture_pause::streamID=0x%lx\n",
	       streamID);
	config.streamID = (CSL_CAPH_STREAM_e) streamID;
	(void)csl_caph_hwctrl_PausePath(config);
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_capture_resume
*
*  Description: Resume the data transfer of audio path capture
*
****************************************************************************/
Result_t csl_audio_capture_resume(UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	aTrace
	      (LOG_AUDIO_CSL, "csl_audio_capture_resume::streamID=0x%lx\n",
	       streamID);
	config.streamID = (CSL_CAPH_STREAM_e) streamID;
	(void)csl_caph_hwctrl_ResumePath(config);

	return RESULT_OK;
}

/* ==========================================================================
//
// Function Name: GetCaptureDriverByType
//
// Description: Get the audio render driver reference from the steamID.
//
// ========================================================================*/
CSL_CAPH_Capture_Drv_t *GetCaptureDriverByType(UInt32 streamID)
{
	CSL_CAPH_Capture_Drv_t *audDrv = NULL;

	audDrv = &sCaptureDrv[streamID];

	return audDrv;
}

/* ==========================================================================
//
// Function Name: AUDIO_DMA_CB
//
// Description: The callback function when there is DMA request
//
// ========================================================================*/
static void AUDIO_DMA_CB(CSL_CAPH_DMA_CHNL_e chnl)
{
	CSL_CAPH_Capture_Drv_t *audDrv = NULL;
	UInt32 streamID = CSL_CAPH_STREAM_NONE;
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e buffer_status
						= CSL_CAPH_READY_NONE;
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e fifo_status
						= CSL_CAPH_READY_NONE;
	UInt8 *addr = NULL;
	unsigned long flags;

	/*aTrace(LOG_AUDIO_CSL,
	       "AUDIO_DMA_CB:: DMA callback.\n"); */

	streamID = GetStreamIDByDmaCH(chnl);

	fifo_status = csl_caph_dma_read_ddrfifo_sw_status(chnl);

	audDrv = GetCaptureDriverByType(streamID);

	if ((fifo_status & CSL_CAPH_READY_LOW) ==
	    CSL_CAPH_READY_NONE) {
		/*aTrace(LOGID_AUDIO,
		   "DMARequess fill low half ch=0x%x \r\n", chnl); */
		/* if (get_chip_rev_id() >= RHEA_CHIP_REV_B0) */ {
			/* move to next block */
			audDrv->blockIndex++;
			if (audDrv->blockIndex >= audDrv->numBlocks)
				audDrv->blockIndex = 0;
			addr = audDrv->ringBuffer +
				audDrv->blockIndex * audDrv->blockSize;
			csl_caph_dma_set_lobuffer_address(chnl, addr);
		}
		buffer_status |= CSL_CAPH_READY_LOW;
	}

	if ((fifo_status & CSL_CAPH_READY_HIGH) ==
	    CSL_CAPH_READY_NONE) {
		/*aTrace(LOGID_AUDIO,
		      "DMARequest fill high half ch=0x%x \r\n", chnl); */
		/* if (get_chip_rev_id() >= RHEA_CHIP_REV_B0) */ {
			/* move to next block */
			audDrv->blockIndex++;
			if (audDrv->blockIndex >= audDrv->numBlocks)
				audDrv->blockIndex = 0;
			addr = audDrv->ringBuffer +
				audDrv->blockIndex * audDrv->blockSize;
			csl_caph_dma_set_hibuffer_address(chnl, addr);
		}
		buffer_status |= CSL_CAPH_READY_HIGH;
	}
	/* Only set status if a DMA CB exists to capture data */
	spin_lock_irqsave(&audDrv->configLock, flags);
	if (audDrv->dmaCB != NULL) {
		csl_caph_dma_set_ddrfifo_status(chnl, buffer_status);
		audDrv->dmaCB(audDrv->streamID, buffer_status);
	}
	spin_unlock_irqrestore(&audDrv->configLock, flags);
}

/* ==========================================================================
//
// Function Name: GetStreamIDByDmaCH
//
// Description: Get the audio streamID from the dma channel.
//
// ========================================================================*/
/*: this api can be shared bwteen capture and render,
     need to put in a common file.
 */
static CSL_CAPH_STREAM_e GetStreamIDByDmaCH(CSL_CAPH_DMA_CHNL_e dmaCH)
{
	CSL_CAPH_STREAM_e streamID = CSL_CAPH_STREAM_NONE;
	CSL_CAPH_Capture_Drv_t	*audDrv = NULL;
	UInt32 i = 0;
	for (i = 0; i < CSL_CAPH_STREAM_TOTAL; i++) {
		audDrv = GetCaptureDriverByType(i);
		if (audDrv != NULL) {
			if (audDrv->dmaCH == dmaCH) {
				streamID = audDrv->streamID;
				break;
			}
		}
	}
	return streamID;
}
