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
