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
*   @file   drv_audio_render.h
*
*   @brief  common APIs for audio render driver
*
****************************************************************************/

#ifndef	__AUDIO_RENDER_DRV_H__
#define	__AUDIO_RENDER_DRV_H__

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

//#if defined (_ATHENA_)

#define AUDRENDER_TYPE_t UInt32

typedef Boolean (*AUDRENDER_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
* Initialize audio render driver, init internal variables and task queue.
*
*	@param	source	audio driver source from upper layer
*	@param	sink  	audio driver sink from upper layer
*	@return	UInt32    streamID to upper layer
*	@note	
**************************************************************************/
UInt32 AUDDRV_AudioRender_Init( AUDDRV_DEVICE_e source, AUDDRV_DEVICE_e sink );

//*********************************************************************
/**
* Shut down audio render driver, free internal variables and task queue.
*
*	@param	streamID	    the audio render driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_Shutdown( UInt32 streamID );


//*********************************************************************
/**
* Set the driver transfer parameters before configure driver, if needed.
*
*	@param	streamID	     the audio render driver stream ID from upper layer
*	@param	callbackThreshold(in ms)	Driver will callback when buffer size is lower than the threshold
*	@param	interruptInterval(in ms)	The DMA intterrupt interval
*	@return	Result_t
*	@note   Driver will use default values if this function is not called	
**************************************************************************/
Result_t AUDDRV_AudioRender_SetTransferParameters(
                        UInt32 streamID,
                        UInt32				callbackThreshold,
                        UInt32				interruptInterval);

//*********************************************************************
/**
* Configure audio render driver, Set parameters before start render.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@param	sample_rate	the PCM data sampling rate
*	@param	num_channel	channel numbers, mono/stereo
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_SetConfig(
                        UInt32 streamID,
                        AUDIO_SAMPLING_RATE_t    sample_rate,
                        AUDIO_CHANNEL_NUM_t    num_channel,
						AUDIO_BITS_PER_SAMPLE_t bitsPerSample);


//*********************************************************************
/**
* Register a buffer done callback to audio render driver. 
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@param	bufDone_cb	the callback function pointer
*	@return	Result_t
*	@note	when driver finished the data in the buffer, driver generates 
*	this callback to notify the client.
**************************************************************************/
Result_t AUDDRV_AudioRender_SetBufDoneCB ( 
                     UInt32 streamID,
                     AUDRENDER_BufDoneCB_t           bufDone_cb );

//*********************************************************************
/**
* Send audio data to audio render driver.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@param	pBuf	the buffer pointer of the data
*	@param	nSize	the buffer size of the data
*	@return	Result_t
*	@note	
**************************************************************************/
UInt32 AUDDRV_AudioRender_Write( 
                    UInt32 streamID,
                    UInt8*                 pBuf,
                    UInt32	               nSize );

//*********************************************************************
/**
* Start the data transfer in audio render driver.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_Start ( UInt32 streamID );

//*********************************************************************
/**
* Pause the data transfer in audio render driver.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_Pause ( UInt32 streamID );

//*********************************************************************
/**
* Resume the data transfer in audio render driver.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_Resume( UInt32 streamID );

//*********************************************************************
/**
* Stop the data transfer in audio render driver.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@param	immediately  stop the audio render driver without waiting for finish if TRUE
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_Stop( 
                      UInt32 streamID,
                      Boolean                 immediately );

//*********************************************************************
/**
* Flush the audio render driver queue.
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_FlushBuffer(UInt32 streamID);

//*********************************************************************
/**
* Get the available data size in the queue the data transfer of audio path render.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
UInt32 AUDDRV_AudioRender_GetQueueLoad(UInt32 streamID);

//*********************************************************************
/**
*   Insert silence into audio queue.
*
*	@param	streamID	      the audio render driver stream ID from upper layer
*	@param	size	the amount of silence in bytes
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioRender_InsertSilence(UInt32 streamID, UInt32 size);

//#endif // defined (_ATHENA_)

#ifdef __cplusplus
}
#endif

#endif // __AUDIO_RENDER_DRV_H__
