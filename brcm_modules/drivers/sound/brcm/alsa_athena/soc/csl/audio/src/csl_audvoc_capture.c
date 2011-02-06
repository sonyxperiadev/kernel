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
*   @file   csl_audvoc_capture.c
*
*   @brief  CSL common APIs for audio PCM recording
*
****************************************************************************/


#include "mobcom_types.h"
#include "resultcode.h"
//#include "osheap.h"
//#include "msconsts.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "chal_types.h"
#include "brcm_rdb_sysmap.h"
#include "brcm_rdb_dsp_audio.h"
#include "chal_audiomisc.h"
#include "chal_audioaipath.h"
#include "chal_audiomixertap.h"
//#include "csl_aud_queue.h"
#ifdef UNDER_LINUX
#include <linux/kernel.h>
#include <linux/slab.h>
#endif
#include "osdal_os.h"                 // abstract os types
#include "osdal_os_service.h"         // abstract drivers, including DMA driver
#include "csl_audvoc.h"
#include "csl_audio_capture.h"
#include "csl_aud_drv.h"
#include "log.h"

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

//
// local defines
//
typedef enum
{
	CSL_AUDVOC_STREAM_NONE,	
	CSL_AUDVOC_STREAM_AUDIO,
	CSL_AUDVOC_STREAM_BTW,
	CSL_AUDVOC_STREAM_TOTAL
} CSL_AUDVOC_STREAM_e;

//
// local structures
//
typedef	struct
{
	UInt32		streamID;
	CSL_AUDIO_DEVICE_e 		source;	
	CSL_AUDIO_DEVICE_e 		sink;
	CSL_AUDCAPTURE_CB dmaCB;	
	UInt32	    dmaCH;
} CSL_AUDVOC_Drv_t;

//
// local variables
//
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
						   OSDAL_DMA_CLIENT srcClient,
						   OSDAL_DMA_DWIDTH dataWidth,
						   OSDAL_DMA_CALLBACK dmaCB
						   );
static CSL_AUDVOC_Drv_t* GetDriverByType (UInt32 streamID);
static void AUDIO_DMA_CB(OSDAL_DMA_CALLBACK_STATUS status);
static void BTW_DMA_CB(OSDAL_DMA_CALLBACK_STATUS status);
//
// APIs of audio path capture
//

// ==========================================================================
//
// Function Name: csl_audvoc_capture_init
//
// Description: Init audio path capture. 
//
// =========================================================================
UInt32 csl_audio_capture_init( CSL_AUDIO_DEVICE_e source, CSL_AUDIO_DEVICE_e sink )
{
	OSDAL_DMA_CLIENT	srcClient = OSDAL_DMA_CLIENT_AUDIO_IN_FIFO;
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	UInt32 streamID = CSL_AUDVOC_STREAM_NONE;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_capture_init:: \n");
	if (source == CSL_AUDVOC_DEV_CAPTURE_BTW)
		srcClient = OSDAL_DMA_CLIENT_AUDIO_WB_MIXERTAP;
		
	// Get DMA channel
    if(OSDAL_DMA_Obtain_Channel(srcClient, OSDAL_DMA_CLIENT_MEMORY, &dmaChAudio) != OSDAL_ERR_OK)
    {
        Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_capture_init:: Error, Obtain channel failed.\n");
		return CSL_AUDVOC_STREAM_NONE;
    }

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_capture_init:: DMA channel for audio path capture 0x%x\n", dmaChAudio);

	if (source == CSL_AUDVOC_DEV_CAPTURE_AUDIO)
	{
		streamID = CSL_AUDVOC_STREAM_AUDIO;
	}
	else if (source == CSL_AUDVOC_DEV_CAPTURE_BTW)
	{
		streamID = CSL_AUDVOC_STREAM_BTW;
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
// Function Name: csl_audvoc_capture_deinit
//
// Description: Shut down audio path capture. 
//
// =========================================================================
Result_t csl_audio_capture_deinit( UInt32 streamID )
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_capture_deinit::release DMA channel, streamID=%d.\n", streamID);	

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
// Function Name: csl_audvoc_capture_configure
//
// Description: Configure audio path capture. 
//
// =========================================================================
Result_t csl_audio_capture_configure( AUDIO_SAMPLING_RATE_t    sampleRate, 
						AUDIO_CHANNEL_NUM_t    numChannels,
						AUDIO_BITS_PER_SAMPLE_t bitsPerSample,						
						UInt8 *ringBuffer,
						UInt32 numBlocks,
						UInt32 blockSize,
						CSL_AUDCAPTURE_CB csl_audio_capture_cb,
						UInt32 streamID
						)
{
	OSDAL_DMA_DWIDTH dataWidth = OSDAL_DMA_DATA_SIZE_16BIT; // default is 16
	OSDAL_DMA_CLIENT	srcClient = OSDAL_DMA_CLIENT_AUDIO_IN_FIFO;
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	OSDAL_DMA_CALLBACK dmaCB = NULL;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_capture_configure:: DMA streamID = 0x%x, sampleRate =0x%x, numChannels = 0x%x, bits/sample = 0x%x numbBuffers = 0x%x, blockSize = 0x%x\n", 
					streamID, sampleRate, numChannels, bitsPerSample, numBlocks, blockSize);

	if (numChannels == AUDIO_CHANNEL_STEREO)
	{
		dataWidth = OSDAL_DMA_DATA_SIZE_32BIT;
	}

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	

	audDrv->dmaCB = csl_audio_capture_cb;
	if (streamID == CSL_AUDVOC_STREAM_AUDIO)
	{
		dmaCB = (OSDAL_DMA_CALLBACK)AUDIO_DMA_CB;
	}
	else if (streamID == CSL_AUDVOC_STREAM_BTW)
	{
		dmaCB = (OSDAL_DMA_CALLBACK)BTW_DMA_CB;
		srcClient = OSDAL_DMA_CLIENT_AUDIO_WB_MIXERTAP;
	}
	if ( ConfigDMA (audDrv->dmaCH, ringBuffer, numBlocks, blockSize, srcClient, dataWidth, dmaCB) != RESULT_OK)
	{
		// do clean up
	}

	return RESULT_OK;
}


// ==========================================================================
//
// Function Name: csl_audvoc_capture_start
//
// Description: Start the data transfer of the audio path capture
//
// =========================================================================
Result_t csl_audio_capture_start( UInt32 streamID )
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_capture_start:: DMA streamID = %d\n", streamID);

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	
	
	if(OSDAL_DMA_Start_Transfer(audDrv->dmaCH) != OSDAL_ERR_OK)
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audvoc_capture_start::Error, Start transfer failed.\n");
	}

	if (streamID == CSL_AUDVOC_STREAM_BTW)
	{
		Log_DebugPrintf(LOGID_AUDIO, "csl_audio_capture_start BT wb tap \n");
	
		AUDDRV_Enable_MixerTap ( AUDDRV_MIXERTap_WB_INPUT, 
			AUDDRV_SPKR_NONE,   ////this param bears no meaning in this context.
			AUDDRV_SPKR_NONE,   ////this param bears no meaning in this context.
			AUDIO_SAMPLING_RATE_UNDEFINED,  //this param bears no meaning in this context
			AUDDRV_REASON_DATA_DRIVER
			);
	}
	else
	{
		AUDDRV_EnableHWInput ( AUDDRV_AUDIO_INPUT, 
			AUDDRV_MIC_NONE,    //this param bears no meaning in this context.
			AUDIO_SAMPLING_RATE_UNDEFINED,  //this param bears no meaning in this context.
			AUDDRV_REASON_DATA_DRIVER
		   );
	}

	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: csl_audvoc_capture_pause
//
// Description: Pause the data transfer of the audio path capture
//
// =========================================================================
Result_t csl_audio_capture_pause( UInt32 streamID )
{
	
	Log_DebugPrintf(LOGID_AUDIO, " csl_audvoc_capture_pause:: streamID = 0x%x.\n", streamID);

	if (streamID == CSL_AUDVOC_STREAM_AUDIO)	
		chal_audioaipath_EnableDMA(NULL, FALSE);
	else if (streamID == CSL_AUDVOC_STREAM_BTW)
		chal_audiomixertap_EnableWbDma(NULL, FALSE);
	
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: csl_audvoc_capture_resume
//
// Description: Resume the data transfer of the audio path capture
//
// =========================================================================
Result_t csl_audio_capture_resume( UInt32 streamID )
{
	Log_DebugPrintf(LOGID_AUDIO, " csl_audvoc_capture_resume:: streamID = 0x%x.\n", streamID);

	if (streamID == CSL_AUDVOC_STREAM_AUDIO)
	{
		//	UInt32 zeroSamples[CHAL_AIFIFO_SIZE];
		chal_audioaipath_ReadFifoResidue (NULL);
		chal_audioaipath_EnableDMA(NULL, TRUE); //enable DMA 
	}
	else if (streamID == CSL_AUDVOC_STREAM_BTW)
	{
		UInt16 cnt;
		cnt = chal_audiomixertap_ReadWbFifoEntryCnt(NULL);
		chal_audiomixertap_ReadWbFifo(NULL, NULL, cnt);
		chal_audiomixertap_EnableWbDma(NULL, TRUE);
	}
	
	return RESULT_OK;
}

// ==========================================================================
//
// Function Name: csl_audvoc_capture_stop
//
// Description: Stop the data transfer of the audio path capture
//
// =========================================================================
Result_t csl_audio_capture_stop( UInt32 streamID )
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;
	
	Log_DebugPrintf(LOGID_AUDIO, " csl_audvoc_capture_stop:: DMA streamID = %d\n", streamID);

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	

    if(OSDAL_DMA_Stop_Transfer(audDrv->dmaCH) != OSDAL_ERR_OK)
	{
		Log_DebugPrintf(LOGID_AUDIO, "csl_audvoc_capture_stop::Error, Stop transfer failed.\n");
	}
	
	if (streamID == CSL_AUDVOC_STREAM_AUDIO)
	{
		AUDDRV_DisableHWInput ( AUDDRV_AUDIO_INPUT, AUDDRV_REASON_DATA_DRIVER );
	}
	else if (streamID == CSL_AUDVOC_STREAM_BTW)
	{
		AUDDRV_Disable_MixerTap ( AUDDRV_MIXERTap_WB_INPUT, AUDDRV_REASON_DATA_DRIVER );
	}
	
	
	
	return RESULT_OK;
}

//
// local functions
//


// ==========================================================================
//
// Function Name: ConfigDMA
//
// Description: Configure DMA channel 
//
// =========================================================================
static Result_t ConfigDMA(
			UInt32 channel, 
			UInt8 *start, 
			UInt32 numBlocks, 
			UInt32 blockSize, 
			OSDAL_DMA_CLIENT srcClient,
			OSDAL_DMA_DWIDTH dataWidth,
			OSDAL_DMA_CALLBACK dmaCB)
{
	OSDAL_Dma_Data dmaData;
	OSDAL_Dma_Buffer_List *cirBufList;
	OSDAL_Dma_Chan_Info chanInfo;
	UInt32 i=0;
	UInt8	*currPos = 0;
	UInt8	*srcAddr = 0;

    Log_DebugPrintf(LOGID_AUDIO, "ConfigDMA:: channel: 0x%x, numBlocks = 0x%x, blockSize = 0x%x\n", channel, numBlocks, blockSize);

	// Configure DMA channel
    //chanInfo.chanNumber = channel;
    //chanInfo.prot = 0;
    //chanInfo.srcID = srcClient;
    //chanInfo.dstID = DMA_CLIENT_MEMORY;
    chanInfo.type = OSDAL_DMA_FCTRL_PERI_TO_MEM;  
    chanInfo.srcBstSize = OSDAL_DMA_BURST_SIZE_64; // audio fifo spec
    chanInfo.dstBstSize = OSDAL_DMA_BURST_SIZE_64;
    chanInfo.srcDataWidth = dataWidth;
    chanInfo.dstDataWidth = dataWidth;
    chanInfo.incMode = OSDAL_DMA_INC_MODE_DST; // mode_dst
    chanInfo.xferCompleteCb = dmaCB;
    chanInfo.freeChan = FALSE;
    chanInfo.bCircular = TRUE;
   
    if(OSDAL_DMA_Config_Channel(channel, &chanInfo) != OSDAL_ERR_OK)
    {
        Log_DebugPrintf(LOGID_AUDIO, "ConfigDMA:: Error, Configure channel failed.\n");
        return RESULT_ERROR;
    }


	switch (srcClient)
	{
		case OSDAL_DMA_CLIENT_AUDIO_IN_FIFO:
			srcAddr = (UInt8 *) chal_audioaipath_getAFIFO_addr( NULL );
			break;
		
		case OSDAL_DMA_CLIENT_AUDIO_WB_MIXERTAP:
			srcAddr = (UInt8 *) chal_audiomixertap_getAFIFO_addr( NULL );
			break;

		default:
			Log_DebugPrintf(LOGID_AUDIO, "ConfigDMA:: Error, Wrong source client, srcClient = 0x%x.\n", srcClient);
			break;
		
	}

	cirBufList = (OSDAL_Dma_Buffer_List *)OSDAL_ALLOCHEAPMEM(numBlocks * sizeof(OSDAL_Dma_Buffer_List));

	currPos = start;
	for (i = 0; i < numBlocks; i++)
	{
		cirBufList[i].buffers[0].srcAddr = (UInt32)srcAddr;
		cirBufList[i].buffers[0].destAddr = (UInt32)currPos;
		cirBufList[i].buffers[0].length = blockSize;
		cirBufList[i].buffers[0].bRepeat = 0;
		cirBufList[i].buffers[0].interrupt = 1;

		currPos += blockSize;
	}

	dmaData.numBuffer = numBlocks;
    dmaData.pBufList = (OSDAL_Dma_Buffer_List *)cirBufList;  
    if(OSDAL_DMA_Bind_Data(channel, &dmaData) != OSDAL_ERR_OK)
    {
        Log_DebugPrintf(LOGID_AUDIO, "ConfigDMA:: Error, Add buffer failed.\n");
        return RESULT_ERROR;
    }

	OSDAL_FREEHEAPMEM (cirBufList);

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
		Log_DebugPrintf(LOGID_SOC_AUDIO, "%s GetDriverByType:: Doesn't support audio driver streamID = 0x%x\n", __FILE__, streamID);

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

	//Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB:: DMA callback.\n");

	if (status != OSDAL_ERR_OK)
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB:: Fatal error! DMA transfer failure.\n");
		return;
	}
	
	audDrv = GetDriverByType(CSL_AUDVOC_STREAM_AUDIO);
	
	if (audDrv->dmaCB != NULL)
		audDrv->dmaCB(audDrv->streamID);
}

// ==========================================================================
//
// Function Name: BTW_DMA_CB
//
// Description: The callback function when there is DMA request to audio path 
//
// =========================================================================
static void BTW_DMA_CB(OSDAL_DMA_CALLBACK_STATUS status)
{
	CSL_AUDVOC_Drv_t	*audDrv = NULL;

	Log_DebugPrintf(LOGID_SOC_AUDIO, "BTW_DMA_CB:: DMA callback.\n");

	if (status != OSDAL_ERR_OK)
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "BTW_DMA_CB:: Fatal error! DMA transfer failure.\n");
		return;
	}
	
	audDrv = GetDriverByType(CSL_AUDVOC_STREAM_BTW);
	
	if (audDrv->dmaCB != NULL)
		audDrv->dmaCB(audDrv->streamID);
}

