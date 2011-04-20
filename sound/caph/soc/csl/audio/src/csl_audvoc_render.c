/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   csl_audvoc_render.c
*
*   @brief  CSL common APIs for audio
*
****************************************************************************/

#include "resultcode.h"
#include "osheap.h"
#include "chal_types.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "chal_audiomisc.h"
#include "chal_audioaopath.h"
#include "chal_audiopopath.h"
#include "osdal_os.h"                 // abstract os types
#include "osdal_os_service.h"         // abstract drivers, including DMA driver
#include "dma_drv.h"
#include "csl_audvoc.h"
#include "csl_audio_render.h"
#include "csl_aud_drv.h"
#include "log.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/


// globle/externals

//
// local defines
//

//#define _DBG_(a)
#define _DBG_(a) (a)

typedef enum
{
	CSL_AUDVOC_STREAM_NONE,	
	CSL_AUDVOC_STREAM_AUDIO,
	CSL_AUDVOC_STREAM_POLYRINGER,
	CSL_AUDVOC_STREAM_TOTAL
} CSL_AUDVOC_STREAM_e;

typedef	struct
{
	UInt32		streamID;
	CSL_AUDIO_DEVICE_e 		source;	
	CSL_AUDIO_DEVICE_e 		sink;
	CSL_AUDRENDER_CB dmaCB;	
	UInt32	    dmaCH;
} CSL_AUDVOC_Drv_t;

static CSL_AUDVOC_Drv_t	sAudvocDrv[CSL_AUDVOC_STREAM_TOTAL] = {0};

static	UInt32				dmaChAudio;

//
//	local functions
//
static Result_t ConfigDMA (
						UInt32 channel, 
						UInt8 *start, 
						UInt32 numBlocks, 
						UInt32 blockSize, 
						OSDAL_DMA_CLIENT dstClient, 
						OSDAL_DMA_DWIDTH dataWidth, 
						OSDAL_DMA_CALLBACK dmaCB
						);
static CSL_AUDVOC_Drv_t* GetDriverByType (UInt32 streamID);
static void AUDIO_DMA_CB(OSDAL_DMA_CALLBACK_STATUS status);
static void PLR_DMA_CB(OSDAL_DMA_CALLBACK_STATUS status);
//
// APIs of audio path render
//

// ==========================================================================
//
// Function Name: csl_audvoc_render_init
//
// Description: Init the audio path render
//
// =========================================================================
UInt32 csl_audio_render_init (CSL_AUDIO_DEVICE_e source, CSL_AUDIO_DEVICE_e sink)
{
	OSDAL_DMA_CLIENT	dstClient = OSDAL_DMA_CLIENT_AUDIO_OUT_FIFO;
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	UInt32 streamID = CSL_AUDVOC_STREAM_NONE;
	
	if (sink == CSL_AUDVOC_DEV_RENDER_POLYRINGER)
		dstClient = OSDAL_DMA_CLIENT_POLYRING_OUT_FIFO;
	
	// Get DMA channel
    if(OSDAL_DMA_Obtain_Channel(OSDAL_DMA_CLIENT_MEMORY, dstClient, &dmaChAudio) != OSDAL_ERR_OK)
    {
        _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_init:: Error, Obtain channel failed.\n"));
		return CSL_AUDVOC_STREAM_NONE;
    }

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_init:: DMA channel for audio path render 0x%x\n", dmaChAudio));

	if (sink == CSL_AUDVOC_DEV_RENDER_AUDIO)
	{
		streamID = CSL_AUDVOC_STREAM_AUDIO;
	}
	else if (sink == CSL_AUDVOC_DEV_RENDER_POLYRINGER)
	{
		streamID = CSL_AUDVOC_STREAM_POLYRINGER;
	}
	else
	{
		return streamID;     
	}
	
	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return CSL_AUDVOC_STREAM_NONE;
	
	memset(audDrv, 0, sizeof(CSL_AUDVOC_Drv_t));
	audDrv->streamID = streamID;
	audDrv->source = source;
	audDrv->sink = sink;
	audDrv->dmaCH = dmaChAudio;
	
	return audDrv->streamID;
}

// ==========================================================================
//
// Function Name:  csl_audio_render_deinit
//
// Description: Shutdown the audio path render
//
// =========================================================================
Result_t  csl_audio_render_deinit (UInt32 streamID)
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, " csl_audvoc_render_deinit::release DMA channel, streamID=%d.\n", streamID));	

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	
	
	// release the DMA channel
	OSDAL_DMA_Release_Channel(audDrv->dmaCH);
	
	memset(audDrv, 0, sizeof(CSL_AUDVOC_Drv_t));
	
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: csl_audvoc_render_configure
//
// Description: Configure the audio path render
//
// =========================================================================
Result_t csl_audio_render_configure ( AUDIO_SAMPLING_RATE_t    sampleRate, 
						AUDIO_CHANNEL_NUM_t    numChannels,
						AUDIO_BITS_PER_SAMPLE_t bitsPerSample,
						UInt8 *ringBuffer,
						UInt32 numBlocks,
						UInt32 blockSize,
						CSL_AUDRENDER_CB csl_audio_render_cb,
						UInt32 streamID
						)
{
	OSDAL_DMA_DWIDTH dataWidth = OSDAL_DMA_DATA_SIZE_16BIT; // default is 16
	OSDAL_DMA_CLIENT	dstClient = OSDAL_DMA_CLIENT_AUDIO_OUT_FIFO;
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	OSDAL_DMA_CALLBACK dmaCB = NULL;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_configure:: streamID = 0x%x, sampleRate =0x%x, numChannels = 0x%x, numbBuffers = 0x%x, blockSize = 0x%x\n", 
					streamID, sampleRate, numChannels, numBlocks, blockSize));
	
	if (numChannels==AUDIO_CHANNEL_STEREO || numChannels==AUDIO_CHANNEL_STEREO_LEFT || numChannels==AUDIO_CHANNEL_STEREO_RIGHT)
	{
		dataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
	}

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	

	audDrv->dmaCB = csl_audio_render_cb;
	if (streamID == CSL_AUDVOC_STREAM_AUDIO)
	{
		dmaCB = (OSDAL_DMA_CALLBACK)AUDIO_DMA_CB;
		chal_audioaopath_SetSampleRate (NULL, sampleRate );
	}
	else if (streamID == CSL_AUDVOC_STREAM_POLYRINGER)
	{
		dmaCB = (OSDAL_DMA_CALLBACK)PLR_DMA_CB;
		chal_audiopopath_SetSampleRate (NULL, sampleRate );
		dstClient = OSDAL_DMA_CLIENT_POLYRING_OUT_FIFO;
	}
	
	if ( ConfigDMA (audDrv->dmaCH, ringBuffer, numBlocks, blockSize, dstClient, dataWidth, dmaCB) != RESULT_OK)
	{
		// do clean up
	}

	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: csl_audvoc_render_start
//
// Description: Start the data transfer of audio path render
//
// =========================================================================
Result_t csl_audio_render_start ( UInt32 streamID )
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	AUDDRV_InOut_Enum_t input_path_to_mixer = AUDDRV_AUDIO_OUTPUT;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_start:: streamID = %d\n", streamID));

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	
	
	if(OSDAL_DMA_Start_Transfer(audDrv->dmaCH) != OSDAL_ERR_OK)
	{
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_start::Error, Start transfer failed.\n"));
	}

	if (streamID == CSL_AUDVOC_STREAM_POLYRINGER)
		input_path_to_mixer = AUDDRV_RINGTONE_OUTPUT;
	
	AUDDRV_EnableHWOutput (
			input_path_to_mixer,
			AUDDRV_SPKR_NONE,  //this param bears no meaning in this context.
			FALSE,	//this param bears no meaning in this context.
			AUDIO_SAMPLING_RATE_UNDEFINED,  //this param bears no meaning in this context.
			AUDIO_CHANNEL_STEREO,
			AUDDRV_REASON_DATA_DRIVER,
            NULL
       );

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: csl_audio_render_stop
//
// Description: Stop immediately the data transfer of audio path render
//
// =========================================================================
Result_t csl_audio_render_stop ( UInt32 streamID )
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	AUDDRV_InOut_Enum_t input_path_to_mixer = AUDDRV_AUDIO_OUTPUT;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_stop::Stop DMA transfer, streamID = 0x%x.\n", streamID));

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	

	if (streamID == CSL_AUDVOC_STREAM_POLYRINGER)
		input_path_to_mixer = AUDDRV_RINGTONE_OUTPUT;
	
	AUDDRV_DisableHWOutput ( input_path_to_mixer, AUDDRV_REASON_DATA_DRIVER );
	
	//OSTASK_Sleep(2000);
	// use force_shutdown instead of stop_transfer. DMADRV_Stop_Transfer() will wait
	// until buffer transfer finishes, this will hang when we disable the audio FIFO
	// DMA request when the audio is paused.
	//if(OSDAL_DMA_Stop_Transfer(dmaChAudio) != OSDAL_ERR_OK) // No OSDAL_DMA API for force_shutdown_channel yet, waiting for Drivers team
	if(DMADRV_Force_Shutdown_Channel((DMA_CHANNEL)audDrv->dmaCH) != DMADRV_STATUS_OK)
	{
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_stop::Error, Stop transfer failed.\n"));
	}

	return RESULT_OK;

}


// ==========================================================================
//
// Function Name: csl_audio_render_pause
//
// Description: Pause the data transfer of audio path render
//
// =========================================================================
Result_t csl_audio_render_pause (UInt32 streamID)
{
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_pause:: streamID=%d.\n", streamID));

	// disable DMA
	if (streamID == CSL_AUDVOC_STREAM_AUDIO)
		chal_audioaopath_EnableDMA(NULL, FALSE);
	else if (streamID == CSL_AUDVOC_STREAM_POLYRINGER)
		chal_audiopopath_EnableDMA(NULL, FALSE);
	
	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: csl_audio_render_resume
//
// Description: Resume the data transfer of audio path render
//
// =========================================================================
Result_t csl_audio_render_resume( UInt32 streamID )
{
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_render_resume streamID=%d.\n", streamID));

	if (streamID == CSL_AUDVOC_STREAM_AUDIO)
	{
		// pre-fill fifo
		UInt32 zeroSamples[CHAL_AOFIFO_SIZE] = {0};
		chal_audioaopath_WriteFifo (NULL, &zeroSamples[0], CHAL_AOFIFO_SIZE);
		// enable DMA
		chal_audioaopath_EnableDMA(NULL, TRUE);
	}
	else if (streamID == CSL_AUDVOC_STREAM_POLYRINGER)
	{
		// pre-fill fifo
		UInt32 zeroSamples[CHAL_POFIFO_SIZE] = {0};
		chal_audiopopath_WriteFifo (NULL, &zeroSamples[0], CHAL_POFIFO_SIZE);
		// enable DMA
		chal_audiopopath_EnableDMA(NULL, TRUE);		
	}
		
	return RESULT_OK;
}

//
// local functionss
//


// ==========================================================================
//
// Function Name: ConfigDMA
//
// Description: Configure DMA channel 
//
// =========================================================================
static Result_t ConfigDMA (
			UInt32 channel, 
			UInt8 *start, 
			UInt32 numBlocks, 
			UInt32 blockSize, 
			OSDAL_DMA_CLIENT dstClient, 
			OSDAL_DMA_DWIDTH dataWidth, 
			OSDAL_DMA_CALLBACK dmaCB)
{
	OSDAL_Dma_Data dmaData;
	OSDAL_Dma_Buffer_List *cirBufList;
	OSDAL_Dma_Chan_Info chanInfo;
	UInt32 i=0;
	UInt8	*currPos=NULL;
	UInt8	*dstAddr=NULL;

	// Configure DMA channel
    //chanInfo.chanNumber = channel;
    //chanInfo.prot = 0;
    //chanInfo.srcID = OSDAL_DMA_CLIENT_MEMORY;
    //chanInfo.dstID = dstClient;
    chanInfo.type = OSDAL_DMA_FCTRL_MEM_TO_PERI;//_CTRL_PERI;   // need to double check.
    chanInfo.srcBstSize = OSDAL_DMA_BURST_SIZE_64; // audio fifo spec
    chanInfo.dstBstSize = OSDAL_DMA_BURST_SIZE_64;
	chanInfo.srcDataWidth = dataWidth;
	chanInfo.dstDataWidth = dataWidth;
	chanInfo.incMode = OSDAL_DMA_INC_MODE_SRC; // mode_src
    chanInfo.xferCompleteCb = dmaCB;
    chanInfo.freeChan = FALSE;
    chanInfo.bCircular = TRUE;

    if(OSDAL_DMA_Config_Channel(channel, &chanInfo) != OSDAL_ERR_OK)
    {
        _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO,  "ConfigDMA:: Error, Configure channel failed.\n"));
        return RESULT_ERROR;
    }


	switch (dstClient)
	{
		case OSDAL_DMA_CLIENT_AUDIO_OUT_FIFO:
			dstAddr = (UInt8 *) chal_audioaopath_getAFIFO_addr( NULL );
			break;
		
		case OSDAL_DMA_CLIENT_POLYRING_OUT_FIFO:
			dstAddr = (UInt8 *) chal_audiopopath_getPFIFO_addr( NULL );
			break;

		default:
			_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO,  "ConfigDMA:: Error, Wrong destination client, dstClient = 0x%x.\n", dstClient));
			break;
		
	}

	cirBufList = (OSDAL_Dma_Buffer_List *)OSHEAP_Alloc(numBlocks * sizeof(OSDAL_Dma_Buffer_List));

	currPos = start;
	for (i = 0; i < numBlocks; i++)
	{
		cirBufList[i].buffers[0].srcAddr = (UInt32)currPos;
		cirBufList[i].buffers[0].destAddr = (UInt32)dstAddr;
		cirBufList[i].buffers[0].length = blockSize;
		cirBufList[i].buffers[0].bRepeat = 0;
		cirBufList[i].buffers[0].interrupt = 1;

		currPos += blockSize;
	}

	
	dmaData.numBuffer = numBlocks;
    dmaData.pBufList = (OSDAL_Dma_Buffer_List *)cirBufList;  
    if(OSDAL_DMA_Bind_Data(channel, &dmaData) != OSDAL_ERR_OK)
    {
        _DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO,  "ConfigDMA:: Error, Add buffer failed.\n"));
        return RESULT_ERROR;
    }

	OSHEAP_Delete (cirBufList);

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: GetDriverByType
//
// Description: Get the audio render driver reference from the steamID.
//
// =========================================================================
static CSL_AUDVOC_Drv_t* GetDriverByType (UInt32 streamID)
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;

	if (streamID != CSL_AUDVOC_STREAM_NONE)
		audDrv = &sAudvocDrv[streamID];
	else
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "%s GetDriverByType:: Doesn't support audio driver streamID = 0x%x\n", __FILE__, streamID));

	return audDrv;
}

// ==========================================================================
//
// Function Name: AUDIO_DMA_CB
//
// Description: The callback function when there is DMA request to audio path 
//
// =========================================================================
static void AUDIO_DMA_CB(OSDAL_DMA_CALLBACK_STATUS status)
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB:: DMA callback.\n"));

	if (status != OSDAL_ERR_OK)
	{
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB:: Fatal error! DMA transfer failure.\n"));
		return;
	}
	
	audDrv = GetDriverByType(CSL_AUDVOC_STREAM_AUDIO);
	
	if (audDrv->dmaCB != NULL)
		audDrv->dmaCB(audDrv->streamID);
}

// ==========================================================================
//
// Function Name: PLR_DMA_CB
//
// Description: The callback function when there is DMA request to audio path 
//
// =========================================================================
static void PLR_DMA_CB(OSDAL_DMA_CALLBACK_STATUS status)
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "PLR_DMA_CB:: DMA callback.\n"));

	if (status != OSDAL_ERR_OK)
	{
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "PLR_DMA_CB:: Fatal error! DMA transfer failure.\n"));
		return;
	}
	
	audDrv = GetDriverByType(CSL_AUDVOC_STREAM_POLYRINGER);
	
	if (audDrv->dmaCB != NULL)
		audDrv->dmaCB(audDrv->streamID);
}

