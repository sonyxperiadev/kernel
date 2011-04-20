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
*   @file   drv_audio_capture.h
*
*   @brief  common APIs for audio PCM capture driver
*
****************************************************************************/

#ifndef	__AUDIO_CAPTURE_DRV_H__
#define	__AUDIO_CAPTURE_DRV_H__

/**
*
* @addtogroup AudioDriverGroup
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

//#if defined (_ATHENA_)


typedef Boolean (*AUDCAPTURE_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
* Initialize audio capture driver, init internal variables and task queue.
*
*	@param	source	audio driver source from upper layer
*	@param	sink  	audio driver sink from upper layer
*	@return	UInt32    streamID to upper layer
*	@note	
**************************************************************************/ 
UInt32 AUDDRV_AudioCapture_Init( AUDDRV_DEVICE_e source, AUDDRV_DEVICE_e sink );

//*********************************************************************
/**
* Shut down audio capture driver, free internal variables and task queue.
*
*	@param	streamID	    the audio capture driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioCapture_Shutdown( UInt32 streamID);


//*********************************************************************
/**
* Set the driver transfer parameters before configure driver, if needed.
*
*	@param	streamID	     the audio capture driver stream ID from upper layer
*	@param	callbackThreshold(in ms)	Driver will callback when buffer size is lower than the threshold
*	@param	interruptInterval(in ms)	The DMA intterrupt interval
*	@return	Result_t
*	@note   Driver will use default values if this function is not called	
**************************************************************************/
Result_t AUDDRV_AudioCapture_SetTransferParameters(
                        UInt32 streamID,
                        UInt32				callbackThreshold,
                        UInt32				interruptInterval);

//*********************************************************************
/**
* Configure audio capture driver, Set parameters before start capture.
*
*	@param	streamID	      the audio capture driver stream ID from upper layer
*	@param	sample_rate	the PCM data sampling rate
*	@param	num_channel	channel numbers, mono/stereo
*	@param	bitsPerSample bits/sample
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioCapture_SetConfig ( 
                        UInt32 streamID,
                     AUDIO_SAMPLING_RATE_t   sample_rate,
                     AUDIO_CHANNEL_NUM_t	 num_channel,
                     AUDIO_BITS_PER_SAMPLE_t bitsPerSample);

//*********************************************************************
/**
* Register a buffer done callback to audio capture driver. 
*
*	@param	streamID	      the audio capture driver stream ID from upper layer
*	@param	bufDone_cb	the callback function pointer
*	@return	Result_t
*	@note	when driver finished the data in the buffer, driver generates 
*	this callback to notify the client.
**************************************************************************/
Result_t AUDDRV_AudioCapture_SetBufDoneCB ( 
                     UInt32 streamID,
                     AUDCAPTURE_BufDoneCB_t             bufDone_cb );

//*********************************************************************
/**
* Read audio data from audio capture driver.
*
*	@param	streamID	      the audio capture driver stream ID from upper layer
*	@param	pBuf	the buffer pointer of the data
*	@param	nSize	the buffer size of the data
*	@return	Result_t
*	@note	
**************************************************************************/
UInt32 AUDDRV_AudioCapture_Read(
                    UInt32 streamID,
                     UInt8                   *pBuf,
                     UInt32                  nSize  );

//*********************************************************************
/**
* Start the data transfer in audio capture driver.
*
*	@param	streamID	      the audio capture driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioCapture_Start ( UInt32 streamID );

//*********************************************************************
/**
* Pause the data transfer in audio capture driver.
*
*	@param	streamID	      the audio capture driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioCapture_Pause ( UInt32 streamID );

//*********************************************************************
/**
* Resume the data transfer in audio capture driver.
*
*	@param	streamID	      the audio capture driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioCapture_Resume( UInt32 streamID );

//*********************************************************************
/**
* Stop the data transfer in audio capture driver.
*
*	@param	streamID	      the audio capture driver stream ID from upper layer
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_AudioCapture_Stop  ( UInt32 streamID );

//#endif // defined (_ATHENA_)

#ifdef __cplusplus
}
#endif

#endif // __AUDIO_CAPTURE_DRV_H__
