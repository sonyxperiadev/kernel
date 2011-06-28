/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/**
*
*  @file  audio_vdriver_voice_render.h
*
*  @brief Audio Virtual Driver API for voice render
*
*  @note
*****************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    This group defines the common APIs for audio virtual driver
*
* @ingroup  Audio Component
*****************************************************************************/


#ifndef	__AUDIO_VDRIVER_VOICE_RENDER_H__
#define	__AUDIO_VDRIVER_VOICE_RENDER_H__


/**
*
* @addtogroup Audio
* @{
*/



#ifdef __cplusplus
extern "C" {
#endif


//#define AMRWB_RENDER_DATA_LOG //comment out for release
typedef enum VORENDER_TYPE_t
{
	VORENDER_TYPE_AMRNB,  // use VPU
	VORENDER_TYPE_PCM_VPU,  // use VPU
	VORENDER_TYPE_PCM_ARM2SP, // use ARM2SP
	VORENDER_TYPE_AMRWB,
	VORENDER_TYPE_PCM_ARM2SP2 // use ARM2SP2	
} VORENDER_TYPE_t;



typedef Boolean (*AUDDRV_VoiceRender_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
* Initialize voice render driver, init internal variables and task queue.
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_Init( VORENDER_TYPE_t type );


//*********************************************************************
/**
* Shut down voice render driver, free internal variables and task queue.
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_Shutdown( VORENDER_TYPE_t type );

//*********************************************************************
/**
* Set the driver transfer parameters before configure driver, if needed.
*
*	@param	type						The audio render driver type
*	@param	callbackThreshold(in ms)	Driver will callback when buffer size is lower than the threshold
*	@param	interruptInterval(in ms)	The DMA intterrupt interval
*	@return	Result_t
*	@note   Driver will use default values if this function is not called	
**************************************************************************/
Result_t AUDDRV_VoiceRender_SetTransferParameters(
                        VORENDER_TYPE_t      type,
                        UInt32				callbackThreshold,
                        UInt32				interruptInterval);

//*********************************************************************
/**
* Configure voice render driver, Set parameters before start render.
*
*	@param	type	the voice render driver type
*	@param	playbackMode	specify the play it to ul,dl or both
*	@param	mixMode	specify mix with ul,dl or both
*	@param	samplingRate	the PCM data sampling rate
*	@param	speedMode	specify the voice data encoding format
*	@param	dataRateSelection	specify the bitrate of the encoded voice data
*	@param	numChannels		specify the number of channels
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_SetConfig(
                        VORENDER_TYPE_t				type,
						VORENDER_PLAYBACK_MODE_t	playbackMode,
						VORENDER_VOICE_MIX_MODE_t   mixMode,
						AUDIO_SAMPLING_RATE_t		samplingRate,
						UInt32						speechMode, // used by AMRNB and AMRWB     
						UInt32						dataRateSelection, // used by AMRNB and AMRWB
						AUDIO_CHANNEL_NUM_t			numChannels
						);



//*********************************************************************
/**
* Register a buffer done callback to voice render driver. 
*
*	@param	type	the voice render driver type
*	@param	bufDone_cb	the callback function pointer
*	@return	Result_t
*	@note	when driver finished the data in the buffer, driver generates 
*	this callback to notify the client.
**************************************************************************/
Result_t AUDDRV_VoiceRender_SetBufDoneCB ( 
                     VORENDER_TYPE_t    type,
                     AUDDRV_VoiceRender_BufDoneCB_t           bufDone_cb );

//*********************************************************************
/**
* Send audio data to voice render driver.
*
*	@param	type	the voice render driver type
*	@param	pBuf	the buffer pointer of the data
*	@param	nSize	the buffer size of the data
*	@return	Result_t
*	@note	
**************************************************************************/
UInt32 AUDDRV_VoiceRender_Write( 
                    VORENDER_TYPE_t     type,
                    UInt8*                 pBuf,
                    UInt32	               nSize );

//*********************************************************************
/**
* Start the data transfer in voice render driver.
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_Start ( VORENDER_TYPE_t      type );

//*********************************************************************
/**
* Pause the data transfer in voice render driver.
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_Pause ( VORENDER_TYPE_t      type );

//*********************************************************************
/**
* Resume the data transfer in voice render driver.
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_Resume( VORENDER_TYPE_t      type );

//*********************************************************************
/**
* Stop the data transfer in voice render driver.
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_Stop( 
                      VORENDER_TYPE_t      type,
                      Boolean                 immediately );

//*********************************************************************
/**
*  Flush the voice render driver queue.
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_FlushBuffer(VORENDER_TYPE_t      type);

//*********************************************************************
/**
* Get size of data in driver queue
*
*	@param	type	the voice render driver type
*	@return	Result_t
*	@note	
**************************************************************************/
UInt32 AUDDRV_VoiceRender_GetQueueLoad(VORENDER_TYPE_t      type);

void ARM2SP_Render_Request(UInt16 bufferPosition);
void ARM2SP2_Render_Request(UInt16 bufferPosition);
void AMRWB_Render_Request(UInt16 status, UInt16 dsp_read_index, UInt16 dsp_write_index);

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_VOICE_RENDER_H__
