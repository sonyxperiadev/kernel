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
*  @file   csl_caph_render.c
*
*  @brief  CSL layer driver for caph render
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
#include "csl_audio_render.h"
#ifdef CONFIG_AUDIO_BUILD
#include "dbg.h"
#endif

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
	CSL_AUDRENDER_CB        dmaCB;	
	CSL_CAPH_DMA_CHNL_e	    dmaCH;
	CSL_CAPH_DMA_CHNL_e	    dmaCH2;
	AUDIO_CHANNEL_NUM_t     numChannels;
	AUDIO_BITS_PER_SAMPLE_t bitsPerSample;
	AUDIO_SAMPLING_RATE_t	sampleRate;
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
*  Function Name:UInt32 csl_caph_render_init(CSL_AUDIO_DEVICE_e source, 
*                                                        CSL_AUDIO_DEVICE_e sink)
*
*  Description: init CAPH render block
*
****************************************************************************/
UInt32 csl_audio_render_init(CSL_AUDIO_DEVICE_e source, CSL_AUDIO_DEVICE_e sink)
{
	CSL_CAPH_DMA_CHNL_e dmaCH = CSL_CAPH_DMA_NONE;
	UInt32 streamID = CSL_CAPH_STREAM_NONE;
	CSL_CAPH_Drv_t	*audDrv = NULL;
	CSL_CAPH_DMA_CHNL_e dmaCH2 = CSL_CAPH_DMA_NONE;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_render_init::source=0x%x sink=0x%x.\n", source, sink));
	if (source == CSL_CAPH_DEV_DSP) // any sink case? fixed the dmach for dsp
	{
		source = CSL_CAPH_DEV_DSP_throughMEM;
		dmaCH = csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CH12);
	}
	else
	{
   		dmaCH = csl_caph_dma_obtain_channel();
	    audio_xassert(dmaCH<CSL_CAPH_DMA_CH12, dmaCH);
	}

	if(sink == CSL_CAPH_DEV_DSP_throughMEM)
	{
		dmaCH2 = csl_caph_dma_obtain_channel();
	}

	if (dmaCH == CSL_CAPH_DMA_NONE)
		return streamID;
	
	streamID = (UInt32)CSL_GetStreamIDByDmaCH(dmaCH);
	audDrv = GetDriverByType(streamID);
	
	memset(audDrv, 0, sizeof(CSL_CAPH_Drv_t));

	audDrv->streamID = streamID;
	audDrv->source = source;
	audDrv->sink = sink;	
	audDrv->dmaCH = dmaCH;
	audDrv->dmaCH2 = dmaCH2;
	// moved to configure
	//audDrv->dmaCB = (CSL_CAPH_DMA_CALLBACK_p)AUDIO_DMA_CB;
	
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
	CSL_CAPH_Drv_t	*audDrv = NULL;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_render_deinit::streamID=0x%x\n", streamID));

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	
	if (audDrv->dmaCH != CSL_CAPH_DMA_NONE)
    {   
	   // For now comment the following line out.
	   // DMA channel is already released by AUdio Controller.
	   // This line hear can cause crash.
	   // I plan to remove it. After more test. 
//        csl_caph_dma_release_channel(audDrv->dmaCH);
    }
	memset(audDrv, 0, sizeof(CSL_CAPH_Drv_t));
	
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: Result_t csl_audio_render_configure
*
*  Description: Configure the CAPH render
*
****************************************************************************/
Result_t csl_audio_render_configure(AUDIO_SAMPLING_RATE_t    sampleRate, 
						AUDIO_CHANNEL_NUM_t    numChannels,
						AUDIO_BITS_PER_SAMPLE_t bitsPerSample,
						UInt8 *ringBuffer,
						UInt32 numBlocks,
						UInt32 blockSize,
						CSL_AUDRENDER_CB csl_audio_render_cb,
						UInt32 streamID)
{
	CSL_CAPH_Drv_t	*audDrv = NULL;
	CSL_CAPH_HWCTRL_STREAM_REGISTER_t stream;
		
#ifdef DSP_FPGA_TEST	
	AP_SharedMem_t* pSharedMem = SHAREDMEM_GetDsp_SharedMemPtr();
#endif

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_caph_render_configure:: streamID = 0x%x, sampleRate =0x%x, numChannels = 0x%x, numbBuffers = 0x%x, blockSize = 0x%x, bitsPerSample %d.\r\n", 
					streamID, sampleRate, numChannels, numBlocks, blockSize, bitsPerSample));
	
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
	
	audDrv->dmaCB = csl_audio_render_cb;
    audDrv->numChannels = numChannels;
    audDrv->bitsPerSample = bitsPerSample;
	audDrv->sampleRate = sampleRate;	

#ifdef DSP_FPGA_TEST		
        if (audDrv->source == AUDDRV_DEV_DSP)
            ringBuffer = (UInt8*)(&(((AP_SharedMem_t *)pSharedMem)->shared_aud_out_buf_48k[0][0]));
#endif
	// : make sure ringbuffer, numblocks and block size are legal for Rhea 
	memset(&stream, 0, sizeof(CSL_CAPH_HWCTRL_STREAM_REGISTER_t));
    stream.streamID = (CSL_CAPH_STREAM_e)audDrv->streamID;
    stream.src_sampleRate = sampleRate;	
    //stream.snk_sampleRate = sampleRate;
    stream.chnlNum = numChannels;
    stream.bitPerSample = bitsPerSample;
    stream.pBuf = ringBuffer;
    stream.pBuf2 = NULL;
    stream.size = numBlocks*blockSize;
    stream.dmaCB = AUDIO_DMA_CB;
    if (RESULT_OK != csl_caph_hwctrl_RegisterStream(&stream))
	{
		audio_xassert(0, streamID);
		return RESULT_ERROR;
	}
	
	return RESULT_OK;
}

/****************************************************************************
*
*  Function Name: csl_audio_render_start
*
*  Description: Start the data transfer of audio path render
*
****************************************************************************/
Result_t csl_audio_render_start (UInt32 streamID)
{
	CSL_CAPH_Drv_t	*audDrv = NULL;
	CSL_CAPH_HWCTRL_CONFIG_t config;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_render_start::streamID=0x%x\n", streamID));

	audDrv = GetDriverByType (streamID);

	if (audDrv == NULL)
		return RESULT_ERROR;	
	
	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	config.streamID = (CSL_CAPH_STREAM_e)audDrv->streamID;
	config.dmaCH = audDrv->dmaCH;
	config.dmaCH2 = audDrv->dmaCH2;

	config.source = audDrv->source;
	config.sink = audDrv->sink;
	config.src_sampleRate = audDrv->sampleRate;
	config.snk_sampleRate = 0;//AUDIO_SAMPLING_RATE_48000;
	config.chnlNum = audDrv->numChannels;
	config.bitPerSample = audDrv->bitsPerSample;

	(void)csl_caph_hwctrl_EnablePath(config);
	
	return RESULT_OK;
}


/****************************************************************************
*
*  Function Name: csl_audio_render_stop
*
*  Description: Stop the data transfer of audio path render
*
****************************************************************************/
Result_t csl_audio_render_stop (UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_render_stop::streamID=0x%x\n", streamID));
	config.streamID = (CSL_CAPH_STREAM_e)streamID;
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
Result_t csl_audio_render_pause (UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;

	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_render_pause::streamID=0x%x\n", streamID));
	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
	config.streamID = (CSL_CAPH_STREAM_e)streamID;
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
Result_t csl_audio_render_resume (UInt32 streamID)
{
	CSL_CAPH_HWCTRL_CONFIG_t config;
	
	_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "csl_audio_render_resume::streamID=0x%x\n", streamID));
	memset(&config, 0, sizeof(CSL_CAPH_HWCTRL_CONFIG_t));
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
	UInt32 streamID = 0; 
	CSL_CAPH_Drv_t	*audDrv = NULL;

	//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "AUDIO_DMA_CB:: DMA callback.\n"));

	// will revisit this when sync with upper layer.
	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) & CSL_CAPH_READY_LOW) == CSL_CAPH_READY_NONE)
	{	
		//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "DMARequest fill low half ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status(chnl, CSL_CAPH_READY_LOW);
		// for use with fpga test only. not needed for real case
		//audDrv->bufDoneCb (audDrv->srcBuf, audDrv->srcBufSize, audDrv->streamID);
	}

	if ((csl_caph_dma_read_ddrfifo_sw_status(chnl) &CSL_CAPH_READY_HIGH) == CSL_CAPH_READY_NONE)
	{
		//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "DMARequest fill high half ch=0x%x \r\n", chnl));
		csl_caph_dma_set_ddrfifo_status( chnl, CSL_CAPH_READY_HIGH);
		// for use with fpga test only. not needed for real case
		//audDrv->bufDoneCb (audDrv->srcBuf, audDrv->srcBufSize, audDrv->streamID);
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
static CSL_CAPH_STREAM_e CSL_GetStreamIDByDmaCH (CSL_CAPH_DMA_CHNL_e dmaCH)
{
	CSL_CAPH_STREAM_e streamID = CSL_CAPH_STREAM_NONE;

	//_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "CSL_GetStreamIDByDmaCH:: dmaCH = 0x%x\n", dmaCH));

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

// ==========================================================================
//
// Function Name: csl_audio_render_get_current_position
//
// Description: Get the current render position for this streamID
//
// =========================================================================
UInt16 csl_audio_render_get_current_position (UInt32 streamID)
{
	CSL_CAPH_Drv_t	*audDrv = NULL;

	if (streamID != CSL_CAPH_STREAM_NONE)
		audDrv = &sCaphDrv[streamID];
	else
	{	
		_DBG_(Log_DebugPrintf(LOGID_SOC_AUDIO, "%s :: Unknown streamID = 0x%x\n", __FILE__, streamID));
		return NULL; /*audio_xassert(0, streamID);*/
	}
	
	if (audDrv->dmaCH != CSL_CAPH_DMA_NONE )
	    return csl_caph_dma_read_currmempointer(audDrv->dmaCH);
	else
		return NULL; /* audio_xassert(0, audDrv->dmaCH);*/
}
