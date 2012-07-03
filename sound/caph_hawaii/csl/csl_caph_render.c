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
*  @file   csl_caph_render.c
*
*  @brief  CSL layer driver for caph render
*
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "msconsts.h"
#include "audio_consts.h"
#include "csl_caph.h"
#include "csl_caph_dma.h"
#include "csl_caph_hwctrl.h"
#include "csl_audio_render.h"
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
static CSL_CAPH_Render_Drv_t sRenderDrv[CSL_CAPH_STREAM_TOTAL] = { {0} };

/***************************************************************************/
/*local function declarations                                              */
/***************************************************************************/
static CSL_CAPH_STREAM_e GetStreamIDByDmaCH(CSL_CAPH_DMA_CHNL_e dmaCH);
static void AUDIO_DMA_CB(CSL_CAPH_DMA_CHNL_e chnl);

/***************************************************************************/
/*local function definitions                                               */
/***************************************************************************/

/****************************************************************************
*
*  Function Name:UInt32 csl_caph_render_init(CSL_CAPH_DEVICE_e source,
*                                                        CSL_CAPH_DEVICE_e sink)
*
*  Description: init CAPH render block
*
****************************************************************************/

/* lready know source and sink, hmm */

UInt32 csl_audio_render_init(CSL_CAPH_DEVICE_e source, CSL_CAPH_DEVICE_e sink)
{
	UInt32 streamID = CSL_CAPH_STREAM_NONE;
	CSL_CAPH_Render_Drv_t *audDrv = NULL;

	aTrace(LOG_AUDIO_CSL,
			"csl_caph_render_init::source=0x%x sink=0x%x.\n",
			source, sink);

	/* allocate a unique streamID */
	streamID = (UInt32) csl_caph_hwctrl_AllocateStreamID();
	audDrv = &sRenderDrv[streamID];

	memset(audDrv, 0, sizeof(CSL_CAPH_Render_Drv_t));

	audDrv->streamID = streamID;
	audDrv->source = source;
	audDrv->sink = sink;
	spin_lock_init(&audDrv->readyStatusLock);
	spin_lock_init(&audDrv->configLock);
	return audDrv->streamID;
}

/****************************************************************************
*
*  Function Name:Result_t csl_audio_render_deinit
*
*  Description: De-initialize CSL render layer
*
****************************************************************************/
Result_t csl_audio_render_deinit(UInt32 streamID)
{
	CSL_CAPH_Render_Drv_t	*audDrv = NULL;
	unsigned long flags;

	aTrace(LOG_AUDIO_CSL,
			"csl_caph_render_deinit::streamID=0x%lx\n", streamID);

	audDrv = GetRenderDriverByType(streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

	audDrv->streamID = 0;
	audDrv->source = CSL_CAPH_DEV_NONE;
	audDrv->sink = CSL_CAPH_DEV_NONE;
	audDrv->pathID = 0;
	audDrv->dmaCH = 0;
	audDrv->blockIndex = 0;

	spin_lock_irqsave(&audDrv->configLock, flags);
	audDrv->dmaCB = NULL;
	spin_unlock_irqrestore(&audDrv->configLock, flags);

	audDrv->ringBuffer = NULL;
	audDrv->numBlocks = 0;
	audDrv->blockSize = 0;

	spin_lock_irqsave(&audDrv->readyStatusLock, flags);
	audDrv->readyBlockStatus = CSL_CAPH_READY_NONE;
	spin_unlock_irqrestore(&audDrv->readyStatusLock, flags);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_audio_render_configure
*
*  Description: Configure the CAPH render
*
****************************************************************************/
Result_t csl_audio_render_configure(AUDIO_SAMPLING_RATE_t sampleRate,
				    AUDIO_NUM_OF_CHANNEL_t numChannels,
				    AUDIO_BITS_PER_SAMPLE_t bitsPerSample,
				    UInt8 *ringBuffer,
				    UInt32 numBlocks,
				    UInt32 blockSize,
				    CSL_AUDRENDER_CB csl_audio_render_cb,
				    UInt32 streamID,
					int mixMode)
{
	CSL_CAPH_Render_Drv_t *audDrv = NULL;
	CSL_CAPH_HWCTRL_STREAM_REGISTER_t stream;
	unsigned long flags;

#ifdef DSP_FPGA_TEST
	AP_SharedMem_t *pSharedMem = SHAREDMEM_GetDsp_SharedMemPtr();
#endif

	aTrace(LOG_AUDIO_CSL,
	"csl_caph_render_configure:: streamID = 0x%lx, sampleRate =0x%x,"
	"numChannels = 0x%x, numbBuffers = 0x%lx, blockSize = 0x%lx,"
	"bitsPerSample %d, cb = %lx.\r\n",
		streamID, sampleRate, numChannels, numBlocks, blockSize,
		(int)bitsPerSample,
		(long unsigned int)csl_audio_render_cb);

	audDrv = GetRenderDriverByType(streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

	/*audDrv->numChannels = numChannels;
	audDrv->bitsPerSample = bitsPerSample;
	audDrv->sampleRate = sampleRate;*/

#ifdef DSP_FPGA_TEST
	if (audDrv->source == CSL_CAPH_DEV_DSP)
		ringBuffer =
		    (UInt8
		     *) (&(((AP_SharedMem_t *) pSharedMem)->
			   shared_aud_out_buf_48k[0][0]));
#endif
	/* make sure ringbuffer, numblocks and block size */
	/* are legal for Rhea  */

	memset(&stream, 0, sizeof(CSL_CAPH_HWCTRL_STREAM_REGISTER_t));
	stream.streamID = (CSL_CAPH_STREAM_e) audDrv->streamID;
	stream.src_sampleRate = sampleRate;
	/* stream.snk_sampleRate = sampleRate; */
	stream.chnlNum = numChannels;
	stream.bitPerSample = bitsPerSample;
	stream.pBuf = ringBuffer;
	stream.pBuf2 = NULL;
	stream.size = 2 * blockSize; /*HW only supports 2 buffers*/
	stream.dmaCB = AUDIO_DMA_CB;
	stream.mixMode = mixMode;
	audDrv->pathID = csl_caph_hwctrl_RegisterStream(&stream);
	if (audDrv->pathID == 0) {
		audio_xassert(0, audDrv->pathID);
		return RESULT_ERROR;
	}

	spin_lock_irqsave(&audDrv->configLock, flags);
	audDrv->dmaCB = csl_audio_render_cb;
	spin_unlock_irqrestore(&audDrv->configLock, flags);

	audDrv->dmaCH = csl_caph_hwctrl_GetdmaCH(audDrv->pathID);
	audDrv->ringBuffer = ringBuffer;
	audDrv->numBlocks = numBlocks;
	audDrv->blockSize = blockSize;

	/* assume everytime it starts, the first 2 buffers will be filled
	when the interrupt comes, it will start from buffer 2
	*/
	audDrv->readyBlockStatus = CSL_CAPH_READY_NONE;
	audDrv->blockIndex = 1;
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_render_start
*
*  Description: Start the data transfer of audio path render
*
****************************************************************************/
Result_t csl_audio_render_start(UInt32 streamID)
{
	CSL_CAPH_Render_Drv_t	*audDrv = NULL;

	aTrace(LOG_AUDIO_CSL,
			"csl_audio_render_start::streamID=0x%lx\n", streamID);

	audDrv = GetRenderDriverByType(streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;

	(void)csl_caph_hwctrl_StartPath(audDrv->pathID);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_render_stop
*
*  Description: Stop the data transfer of audio path render
*
****************************************************************************/
Result_t csl_audio_render_stop(UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	aTrace(LOG_AUDIO_CSL,
			"csl_audio_render_stop::streamID=0x%lx\n", streamID);
	config.streamID = (CSL_CAPH_STREAM_e) streamID;
	(void)csl_caph_hwctrl_DisablePath(config);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_render_pause
*
*  Description: Pause the data transfer of audio path render
*
****************************************************************************/
Result_t csl_audio_render_pause(UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	aTrace(LOG_AUDIO_CSL,
			"csl_audio_render_pause::streamID=0x%lx\n", streamID);
	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	config.streamID = (CSL_CAPH_STREAM_e) streamID;
	(void)csl_caph_hwctrl_PausePath(config);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_render_resume
*
*  Description: Resume the data transfer of audio path render
*
****************************************************************************/
Result_t csl_audio_render_resume(UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	aTrace(LOG_AUDIO_CSL,
			"csl_audio_render_resume::streamID=0x%lx\n", streamID);
	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	config.streamID = (CSL_CAPH_STREAM_e) streamID;
	(void)csl_caph_hwctrl_ResumePath(config);

	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_render_buffer_ready
*
*  Description: set the SW_READY to aadmac when a new buffer is ready
*
****************************************************************************/
Result_t csl_audio_render_buffer_ready(UInt32 streamID)
{
	CSL_CAPH_Render_Drv_t *audDrv = NULL;
	unsigned long flags;

	/*aTrace(LOG_AUDIO_CSL,
		"csl_audio_render_buffer_ready:streamID=0x%lx\n", streamID);*/

	audDrv = GetRenderDriverByType(streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;


	spin_lock_irqsave(&audDrv->readyStatusLock, flags);

	/* Set the block(s) that are ready for processing */
	csl_caph_dma_set_ddrfifo_status(audDrv->dmaCH,
					audDrv->readyBlockStatus);

	/* Clear the ready block status */
	audDrv->readyBlockStatus = CSL_CAPH_READY_NONE;

	spin_unlock_irqrestore(&audDrv->readyStatusLock, flags);

	return RESULT_OK;
}

/* ==========================================================================
//
// Function Name: GetRenderDriverByType
//
// Description: Get the audio render driver reference from the steamID.
//
// =========================================================================*/
CSL_CAPH_Render_Drv_t *GetRenderDriverByType(UInt32 streamID)
{
	CSL_CAPH_Render_Drv_t *audDrv = NULL;

	if (sRenderDrv[streamID].streamID != 0) {
		/* valid streamID starts with value of 1*/
		audDrv = &sRenderDrv[streamID];
	}

	return audDrv;
}

/* ==========================================================================
//
// Function Name: AUDIO_DMA_CB
//
// Description: The callback function when there is DMA request
//
// =========================================================================*/
static void AUDIO_DMA_CB(CSL_CAPH_DMA_CHNL_e chnl)
{
	UInt32 streamID = 0;
	CSL_CAPH_Render_Drv_t *audDrv = NULL;
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e buffer_status = CSL_CAPH_READY_NONE;
	CSL_CAPH_DMA_CHNL_FIFO_STATUS_e fifo_status = CSL_CAPH_READY_NONE;
	UInt8 *addr = NULL;
	unsigned long flags;

	/* aTrace(LOG_AUDIO_CSL, "AUDIO_DMA_CB::
	   DMA callback. chnl = %d\n", chnl); */

	streamID = GetStreamIDByDmaCH(chnl);

	/* Collect a snapshot of the status */
	fifo_status = csl_caph_dma_read_ddrfifo_sw_status(chnl);

	audDrv = GetRenderDriverByType(streamID);
	if (audDrv == NULL)
		return;

	/* when system is heavily loaded, both buffers could be emptied by hw.
	in this case need to fill the data on the HIGH buffer and prime the LOW
    buffer to recover. */
	if ((fifo_status & CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE) {
		buffer_status |= CSL_CAPH_READY_HIGH;

		/* Check for need to process both HIGH and LOW blocks
			or just HIGH block */
		spin_lock_irqsave(&audDrv->readyStatusLock, flags);

		if (fifo_status == CSL_CAPH_READY_NONE)
			audDrv->readyBlockStatus |= CSL_CAPH_READY_HIGHLOW;
		else
			audDrv->readyBlockStatus |= CSL_CAPH_READY_HIGH;

		spin_unlock_irqrestore(&audDrv->readyStatusLock, flags);

		/* Handle special case to clear LOW block if undderrun.
		 * Set LOW addr to last used block used and clear */
		if (fifo_status == CSL_CAPH_READY_NONE) {
			addr = audDrv->ringBuffer +
			   audDrv->blockIndex * audDrv->blockSize;
			memset(phys_to_virt((UInt32)addr),
				0, audDrv->blockSize);
			csl_caph_dma_set_lobuffer_address(chnl, addr);
		}

		spin_lock_irqsave(&audDrv->configLock, flags);
		if (audDrv->dmaCB)
			audDrv->dmaCB(audDrv->streamID, buffer_status);
		spin_unlock_irqrestore(&audDrv->configLock, flags);

		audDrv->blockIndex++;
		if (audDrv->blockIndex >= audDrv->numBlocks)
			audDrv->blockIndex = 0;
		addr = audDrv->ringBuffer +
			audDrv->blockIndex * audDrv->blockSize;
		csl_caph_dma_set_hibuffer_address(chnl, addr);

	} else if ((fifo_status & CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE) {
		buffer_status |= CSL_CAPH_READY_LOW;

		spin_lock_irqsave(&audDrv->readyStatusLock, flags);
		audDrv->readyBlockStatus |= CSL_CAPH_READY_LOW;
		spin_unlock_irqrestore(&audDrv->readyStatusLock, flags);

		spin_lock_irqsave(&audDrv->configLock, flags);
		if (audDrv->dmaCB)
			audDrv->dmaCB(audDrv->streamID, buffer_status);
		spin_unlock_irqrestore(&audDrv->configLock, flags);

		audDrv->blockIndex++;
		if (audDrv->blockIndex >= audDrv->numBlocks)
			audDrv->blockIndex = 0;
		addr = audDrv->ringBuffer +
			audDrv->blockIndex * audDrv->blockSize;
		csl_caph_dma_set_lobuffer_address(chnl, addr);
	}
}

/* ==========================================================================
//
// Function Name: GetStreamIDByDmaCH
//
// Description: Get the audio streamID from the dma channel.
//
// =========================================================================*/
static CSL_CAPH_STREAM_e GetStreamIDByDmaCH(CSL_CAPH_DMA_CHNL_e dmaCH)
{
	CSL_CAPH_STREAM_e streamID = CSL_CAPH_STREAM_NONE;
	CSL_CAPH_Render_Drv_t *audDrv = NULL;
	UInt32 i = 0;

	for (i = 0; i < CSL_CAPH_STREAM_TOTAL; i++) {
		audDrv = GetRenderDriverByType(i);
		if (audDrv != NULL) {
			/* aTrace(LOG_AUDIO_CSL,
			   "GetStreamIDByDmaCH::audDrv->dmaCH = %d,
			   dmaCH = %d, i = %d\n",
			   audDrv->dmaCH, dmaCH, i);
			 */

			if (audDrv->dmaCH == dmaCH) {
				streamID = audDrv->streamID;
				break;
			}
		}
	}

	return streamID;
}

/* ==========================================================================
//
// Function Name: csl_audio_render_get_current_position
//
// Description: Get the current render position for this streamID
//
// =========================================================================*/
UInt16 csl_audio_render_get_current_position(UInt32 streamID)
{
	CSL_CAPH_Render_Drv_t *audDrv = NULL;

	if (streamID != CSL_CAPH_STREAM_NONE)
		audDrv = &sRenderDrv[streamID];
	else
		return 0;

	if (audDrv->dmaCH != CSL_CAPH_DMA_NONE)
		return csl_caph_dma_read_currmempointer(audDrv->dmaCH);
	else
		return 0;
}

/* ==========================================================================
//
// Function Name: csl_audio_render_get_current_buffer
//
// Description: Get the current render buffer for this streamID
//
// =========================================================================*/
UInt16 csl_audio_render_get_current_buffer(UInt32 streamID)
{
	CSL_CAPH_Render_Drv_t *audDrv = NULL;

	if (streamID != CSL_CAPH_STREAM_NONE)
		audDrv = &sRenderDrv[streamID];
	else
		return 0;

	if (audDrv->dmaCH != CSL_CAPH_DMA_NONE)
		return csl_caph_dma_check_dmabuffer(audDrv->dmaCH);
	else
		return 0;
}
