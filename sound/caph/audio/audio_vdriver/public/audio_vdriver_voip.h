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
*  @file  audio_vdriver_voip.h
*
*  @brief Audio Virtual Driver API for voice codec. Used for VoIP & VT for example.
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


#ifndef	__AUDIO_VDRIVER_VOIP_H__
#define	__AUDIO_VDRIVER_VOIP_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef Boolean (*AUDDRV_VoIP_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
* Initialize voip driver, init internal variables and task queue.
*
*	@param	none
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoIP_Init( void );

//*********************************************************************
/**
* Shut down voip driver, free internal variables and task queue.
*
*	@param	none
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoIP_Shutdown( void );

//*********************************************************************
/**
* Configure voip driver, Set parameters before start render.
*
*	@param	codecType Valid value defined in shared.h, VP_Mode_AMR_t
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoIP_SetConfig( UInt16 codecType );

//*********************************************************************
/**
* Register a buffer done callback to voip driver. 
*
*	@param	ulBufDoneCB		the callback function pointer for uplink data
*	@param	dlBufDoneCB		the callback function pointer for downlink data
*	@return	Result_t
*	@note	when driver finished the data in the buffer, driver generates 
*	this callback to notify the client.
**************************************************************************/
Result_t AUDDRV_VoIP_SetBufDoneCB ( 
		AUDDRV_VoIP_BufDoneCB_t           ulBufDoneCB,
		AUDDRV_VoIP_BufDoneCB_t           dlBufDoneCB);

//*********************************************************************
/**
* Send audio data to voip driver.
*
*	@param	pBuf	the buffer pointer of the data
*	@param	nSize	the buffer size of the data
*	@return	Result_t
*	@note	Only have API to write dl, don't have API for read ul. UL data 
*			is passed up via a direct callback.
**************************************************************************/
UInt32 AUDDRV_VoIP_WriteDL( 
		UInt8*                 pBuf,
		UInt32	               nSize );

//*********************************************************************
/**
* Start the data transfer in voip driver.
*
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoIP_Start ( void );


//*********************************************************************
/**
* Stop the data transfer in voip driver.
*
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_VoIP_Stop( Boolean immediately );

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_VOICE_CODER_H__
