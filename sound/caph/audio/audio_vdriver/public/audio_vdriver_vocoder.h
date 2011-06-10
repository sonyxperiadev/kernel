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
*  @file  audio_vdriver_vocoder.h
*
*  @brief Audio Virtual Driver API for voice codec. Used for VT for example.
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


#ifndef	__AUDIO_VDRIVER_VOICE_CODER_H__
#define	__AUDIO_VDRIVER_VOICE_CODER_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif


typedef enum VOCODER_TYPE_t
{
	VOCODER_TYPE_VOICE_NB,
	VOCODER_TYPE_VOICE_WB
} VOCODER_TYPE_t;



typedef Boolean (*AUDDRV_VoiceCoder_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
* Initialize voice codec driver, init internal variables and task queue.
*
*	@param	type	the voice codec driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceCoder_Init( VOCODER_TYPE_t type );


//*********************************************************************
/**
* Shut down voice codec driver, free internal variables and task queue.
*
*	@param	type	the voice codec driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceCoder_Shutdown( VOCODER_TYPE_t type );

//*********************************************************************
/**
* Configure voice codec driver, Set parameters before start render.
*
*	@param	type	the voice codec driver type
*	@param	speedMode	specify the voice data encoding format
*	@param	dataRateSelection	specify the bitrate of the encoded voice data
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceCoder_SetConfig(
                        VOCODER_TYPE_t				type,    
						UInt32						dataRateSelection // used by AMRNB and AMRWB, for ul only
					);



//*********************************************************************
/**
* Register a buffer done callback to voice codec driver. 
*
*	@param	type	the voice codec driver type
*	@param	ulBufDoneCB		the callback function pointer for uplink data
*	@param	dlBufDoneCB		the callback function pointer for downlink data
*	@return	Result_t
*	@note	when driver finished the data in the buffer, driver generates 
*	this callback to notify the client.
**************************************************************************/
Result_t AUDDRV_VoiceCoder_SetBufDoneCB ( 
                     VOCODER_TYPE_t    type,
                     AUDDRV_VoiceCoder_BufDoneCB_t           ulBufDoneCB,
					 AUDDRV_VoiceCoder_BufDoneCB_t           dlBufDoneCB);

//*********************************************************************
/**
* Send audio data to voice codec driver.
*
*	@param	type	the voice codec driver type
*	@param	pBuf	the buffer pointer of the data
*	@param	nSize	the buffer size of the data
*	@return	Result_t
*	@note	Only have API to write dl, don't have API for read ul. UL data 
*			is passed up via a direct callback.
**************************************************************************/
UInt32 AUDDRV_VoiceCoder_WriteDL( 
                    VOCODER_TYPE_t     type,
                    UInt8*                 pBuf,
                    UInt32	               nSize );

//*********************************************************************
/**
* Start the data transfer in voice codec driver.
*
*	@param	type	the voice codec driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceCoder_Start ( VOCODER_TYPE_t      type );


//*********************************************************************
/**
* Stop the data transfer in voice codec driver.
*
*	@param	type	the voice codec driver type
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoiceCoder_Stop( 
                      VOCODER_TYPE_t      type,
                      Boolean                 immediately );



#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_VOICE_CODER_H__
