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
*  @file   csl_caph_capture.c
*
*  @brief  CSL layer driver for caph capture
*
****************************************************************************/
#include "mobcom_types.h"
#include "resultcode.h"
#include "xassert.h"
#include "log.h"
#include "msconsts.h"
#include "audio_consts.h"
#include "auddrv_def.h"
#include "csl_caph.h"
#include "csl_caph_dma.h"
#include "csl_caph_hwctrl.h"
#include "csl_audio_capture.h"
#ifdef CONFIG_AUDIO_BUILD
#include "dbg.h"
#endif

#define _DBG_(a)
//#define _DBG_(a) (a)
//****************************************************************************
//                        G L O B A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// global variable definitions
//****************************************************************************


//****************************************************************************
//                         L O C A L   S E C T I O N
//****************************************************************************

//****************************************************************************
// local macro declarations
//****************************************************************************
//****************************************************************************
// local typedef declarations
//****************************************************************************
typedef	struct
{
	UInt32		streamID;
	CSL_AUDIO_DEVICE_e 		source;	
	CSL_AUDIO_DEVICE_e 		sink;
	CSL_AUDCAPTURE_CB dmaCB;	
	CSL_CAPH_DMA_CHNL_e	    dmaCH;
	CSL_CAPH_DMA_CHNL_e	    dmaCH2; //temp leave this back compatible

	UInt8                   *ringBuffer;
	UInt32                  numBlocks;
	UInt32                  blockSize;
	AUDIO_BITS_PER_SAMPLE_t bitPerSample;
} CSL_CAPH_Drv_t;

//****************************************************************************
// local variable definitions
//****************************************************************************
static CSL_CAPH_Drv_t	sCaphDrv[CSL_CAPH_STREAM_TOTAL] = {{0}};

//****************************************************************************
// local function declarations
//****************************************************************************
static CSL_CAPH_Drv_t* GetDriverByType (UInt32 streamID);
static CSL_CAPH_STREAM_e CSL_GetStreamIDByDmaCH (CSL_CAPH_DMA_CHNL_e dmaCH);
static void AUDIO_DMA_CB(CSL_CAPH_DMA_CHNL_e chnl);

//******************************************************************************
// local function definitions
//******************************************************************************

/****************************************************************************
*
*  Function Name:UInt32 csl_audio_capture_init(CSL_AUDIO_DEVICE_e source, 
*                                                        CSL_AUDIO_DEVICE_e sink)
*
*  Description: init CAPH capture block
*
****************************************************************************/
UInt32 csl_audio_capture_init(CSL_AUDIO_DEVICE_e source, CSL_AUDIO_DEVICE_e sink)
{
	CSL_CAPH_DMA_CHNL_t dmaCHs;
	UInt32 streamID = CSL_CAPH_STREAM_NONE;
	CSL_CAPH_Drv_t	*audDrv = NULL;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_init::source=0x%x sink=0x%x.\n", source, sink));

    memset(&dmaCHs, 0, sizeof(CSL_CAPH_DMA_CHNL_t));
	if (((source == CSL_CAPH_DEV_ANALOG_MIC) ||(source == CSL_CAPH_DEV_DIGI_MIC))
		&& (sink == CSL_CAPH_DEV_DSP))
	{
		sink = CSL_CAPH_DEV_DSP_throughMEM;
		dmaCHs.dmaCH = csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CH13);
	}
	// need to diff these 2 streams for eanc
	else if ((source == CSL_CAPH_DEV_EANC_DIGI_MIC)&& (sink == CSL_CAPH_DEV_DSP))
	{
		sink = CSL_CAPH_DEV_DSP_throughMEM;
		dmaCHs.dmaCH = csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CH14);
                dmaCHs.dmaCH2 = csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CH15);	
	}
	else if ((source == CSL_CAPH_DEV_EANC_INPUT)&& (sink == CSL_CAPH_DEV_DSP))
	{
		sink = CSL_CAPH_DEV_DSP_throughMEM;
		dmaCHs.dmaCH = csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CH16);
	}
	else
	{
	    dmaCHs.dmaCH = csl_caph_dma_obtain_channel();
        if (dmaCHs.dmaCH == CSL_CAPH_DMA_NONE)
        {
            // No DMA Channel availble for dmaCH. Return
            return streamID;
        }
	    audio_xassert(dmaCHs.dmaCH<CSL_CAPH_DMA_CH12, dmaCHs.dmaCH);
	}

	streamID = CSL_GetStreamIDByDmaCH(dmaCHs.dmaCH);
	
	audDrv = GetDriverByType(streamID);
	
	memset(audDrv, 0, sizeof(CSL_CAPH_Drv_t));

	audDrv->streamID = streamID;
	audDrv->source = source;
	audDrv->sink = sink;	
	audDrv->dmaCH = dmaCHs.dmaCH;	
	audDrv->dmaCH2 = dmaCHs.dmaCH2;	
	
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
	CSL_CAPH_Drv_t	*audDrv = NULL;

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_deinit::dmaCH=0x%x, dmaCH2-0x%x\n", 
                    audDrv->dmaCH, audDrv->dmaCH2));
	
    if (audDrv->dmaCH != CSL_CAPH_DMA_NONE)
    {
	   // For now comment the following line out.
	   // DMA channel is already released by AUdio Controller.
	   // This line hear can cause crash.
	   // I plan to remove it. After more test. 	    
//        csl_caph_dma_release_channel(audDrv->dmaCH);
    }
    if (audDrv->dmaCH2 != CSL_CAPH_DMA_NONE)
    {
	   // For now comment the following line out.
	   // DMA channel is already released by AUdio Controller.
	   // This line hear can cause crash.
	   // I plan to remove it. After more test. 	    
//        csl_caph_dma_release_channel(audDrv->dmaCH2);
    }
	
	memset(audDrv, 0, sizeof(CSL_CAPH_Drv_t));
	
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_audio_capture_configure
*
*  Description: Configure the CAPH capture
*
****************************************************************************/
Result_t csl_audio_capture_configure( AUDIO_SAMPLING_RATE_t    sampleRate, 
						AUDIO_CHANNEL_NUM_t    numChannels,
						AUDIO_BITS_PER_SAMPLE_t bitsPerSample,						
						UInt8 *ringBuffer,
						UInt32 numBlocks,
						UInt32 blockSize,
						CSL_AUDCAPTURE_CB csl_audio_capture_cb,
						UInt32 streamID )
{
	CSL_CAPH_Drv_t	*audDrv = NULL;
	CSL_CAPH_HWCTRL_STREAM_REGISTER_t stream;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_configure::\n"));

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	

    //Register StreamID to HW Ctrl Driver.
	if(RESULT_OK != csl_caph_hwctrl_RegisterStreamID(audDrv->source, 
                                            audDrv->sink, 
                                            (CSL_CAPH_STREAM_e)audDrv->streamID)) 
    {
        audio_xassert(0, audDrv->streamID);
        return RESULT_ERROR;
    }
	audDrv->dmaCB = csl_audio_capture_cb;

	audDrv->ringBuffer = ringBuffer;
	audDrv->blockSize = blockSize;
	audDrv->numBlocks = numBlocks;
	
#ifdef DSP_FPGA_TEST	
    if (audDrv->sink == CSL_CAPH_DEV_DSP)
		audDrv->ringBuffer = (UInt8*)(&(pSharedMem->shared_eanc_buf1_low[0]));
#endif		

	// : make sure ringbuffer, numblocks and block size are legal for Rhea 
	memset(&stream, 0, sizeof(CSL_CAPH_HWCTRL_STREAM_REGISTER_t));
    stream.streamID = (CSL_CAPH_STREAM_e)audDrv->streamID;
    stream.src_sampleRate = sampleRate;	
    stream.snk_sampleRate = sampleRate; //make it same as source
    stream.chnlNum = numChannels;
    stream.bitPerSample = bitsPerSample;
    stream.pBuf = ringBuffer;
    stream.pBuf2 = NULL;
    stream.size = numBlocks*blockSize;
    stream.dmaCB = AUDIO_DMA_CB;

	if(audDrv->source==CSL_CAPH_DEV_DSP && audDrv->sink==CSL_CAPH_DEV_MEMORY)
	{
		Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_configure::USB call? reset src_sampleRate from %ld to 8000.\r\n", stream.src_sampleRate);
		stream.src_sampleRate = AUDIO_SAMPLING_RATE_8000;
	}
	
   if (RESULT_OK != csl_caph_hwctrl_RegisterStream(&stream))
    {
        audio_xassert(0, streamID);
        return RESULT_ERROR;
    }	
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_capture_start
*
*  Description: Start the data transfer of audio path capture
*
****************************************************************************/
Result_t csl_audio_capture_start (UInt32 streamID)
{
	CSL_CAPH_Drv_t	*audDrv = NULL;
	CSL_CAPH_HWCTRL_CONFIG_t config;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_start::streamID=0x%x\n", streamID));

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	
	
	memset(&config, 0, sizeof(config));
	config.streamID = (CSL_CAPH_STREAM_e)audDrv->streamID;
	config.dmaCH = audDrv->dmaCH;
	config.dmaCH2 = audDrv->dmaCH2;
	
	config.source = audDrv->source;
	config.sink = audDrv->sink;

    (void)csl_caph_hwctrl_EnablePath(config);
	audDrv->bitPerSample = csl_caph_hwctrl_GetDataFormat(config.streamID);	
	
    return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: csl_audio_capture_stop
*
*  Description: Stop the data transfer of audio path capture
*
****************************************************************************/
Result_t csl_audio_capture_stop (UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_stop::streamID=0x%x\n", streamID));

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
Result_t csl_audio_capture_pause (UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_pause::streamID=0x%x\n", streamID));

	config.streamID = (CSL_CAPH_STREAM_e)streamID;	
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
Result_t csl_audio_capture_resume (UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_capture_resume::streamID=0x%x\n", streamID));

	config.streamID = (CSL_CAPH_STREAM_e)streamID;
    (void)csl_caph_hwctrl_ResumePath(config);
	
    return RESULT_OK;
}

// ==========================================================================
//
// Function Name: GetDriverByType
//
// Description: Get the audio render driver reference from the steamID.
//
// =========================================================================
static CSL_CAPH_Drv_t* GetDriverByType (UInt32 streamID)
{
	CSL_CAPH_Drv_t	*audDrv = NULL;

	if (streamID != CSL_CAPH_STREAM_NONE)
		audDrv = &sCaphDrv[streamID];
	else
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "%s GetDriverByType:: Doesn't support audio driver streamID = 0x%x\n", __FILE__, streamID));

	return audDrv;
}

// ==========================================================================
//
// Function Name: AUDIO_DMA_CB
//
// Description: The callback function when there is DMA request
//
// =========================================================================
static void AUDIO_DMA_CB(CSL_CAPH_DMA_CHNL_e chnl)
{
	CSL_CAPH_Drv_t	*audDrv = NULL;
	UInt32     streamID = CSL_CAPH_STREAM_NONE;

	//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB:: DMA callback.\n"));

	// will revisit this when sync with upper layer.
	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE)
	{	
		_DBG_(Log_DebugPrintf(LOGID_AUDIO, "DMARequess fill low half ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE)
	{
		_DBG_(Log_DebugPrintf(LOGID_AUDIO, "DMARequest fill high half ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status( chnl, CSL_CAPH_READY_HIGH);
	}
	streamID = CSL_GetStreamIDByDmaCH(chnl);

	audDrv = GetDriverByType(streamID);
	
	if (audDrv->dmaCB != NULL)
		audDrv->dmaCB(audDrv->streamID);

}

// ==========================================================================
//
// Function Name: CSL_GetStreamIDByDmaCH
//
// Description: Get the audio streamID from the dma channel.
//
// =========================================================================
//: this api can be shared bwteen capture and render, need to put in a common file.
static CSL_CAPH_STREAM_e CSL_GetStreamIDByDmaCH (CSL_CAPH_DMA_CHNL_e dmaCH)
{
	CSL_CAPH_STREAM_e streamID = CSL_CAPH_STREAM_NONE;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "CSL_GetStreamIDByDmaCH:: dmaCH = 0x%x\n", dmaCH));

      switch (dmaCH)
      {
        case CSL_CAPH_DMA_NONE:
            streamID = CSL_CAPH_STREAM_NONE;
            break;
			
         case CSL_CAPH_DMA_CH1:
            streamID = CSL_CAPH_STREAM1;
            break;
			
         case CSL_CAPH_DMA_CH2:
            streamID = CSL_CAPH_STREAM2;
            break;	
			
         case CSL_CAPH_DMA_CH3:
            streamID = CSL_CAPH_STREAM3;
            break;
			
         case CSL_CAPH_DMA_CH4:
            streamID = CSL_CAPH_STREAM4;
            break;
			
         case CSL_CAPH_DMA_CH5:
            streamID = CSL_CAPH_STREAM5;
            break;
			
         case CSL_CAPH_DMA_CH6:
            streamID = CSL_CAPH_STREAM6;
            break;	
			
         case CSL_CAPH_DMA_CH7:
            streamID = CSL_CAPH_STREAM7;
            break;
			
         case CSL_CAPH_DMA_CH8:
            streamID = CSL_CAPH_STREAM8;
            break;
			
         case CSL_CAPH_DMA_CH9:
            streamID = CSL_CAPH_STREAM9;
            break;
			
         case CSL_CAPH_DMA_CH10:
            streamID = CSL_CAPH_STREAM10;
            break;	
			
         case CSL_CAPH_DMA_CH11:
            streamID = CSL_CAPH_STREAM11;
            break;
			
         case CSL_CAPH_DMA_CH12:
            streamID = CSL_CAPH_STREAM12;
            break;
			
         case CSL_CAPH_DMA_CH13:
            streamID = CSL_CAPH_STREAM13;
            break;
			
         case CSL_CAPH_DMA_CH14:
            streamID = CSL_CAPH_STREAM14;
            break;	
			
         case CSL_CAPH_DMA_CH15:
            streamID = CSL_CAPH_STREAM15;
            break;
			
         case CSL_CAPH_DMA_CH16:
            streamID = CSL_CAPH_STREAM16;
            break;
			
        default:
            audio_xassert(0, dmaCH);
		break;	
    };

	return streamID;
}

