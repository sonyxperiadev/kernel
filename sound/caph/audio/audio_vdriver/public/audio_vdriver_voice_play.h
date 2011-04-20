/*****************************************************************************
*
*    (c) 2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
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
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceRender_SetConfig(
                        VORENDER_TYPE_t				type,
						VORENDER_PLAYBACK_MODE_t	playbackMode,
						VORENDER_VOICE_MIX_MODE_t   mixMode,
						AUDIO_SAMPLING_RATE_t		samplingRate,
						UInt32						speechMode, // used by AMRNB and AMRWB     
						UInt32						dataRateSelection // used by AMRNB and AMRWB
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

void ARM2SP_Render_Request(VPStatQ_t reqMsg);
void ARM2SP2_Render_Request(VPStatQ_t reqMsg);
void AMRWB_Render_Request(VPStatQ_t reqMsg);

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_VOICE_RENDER_H__
