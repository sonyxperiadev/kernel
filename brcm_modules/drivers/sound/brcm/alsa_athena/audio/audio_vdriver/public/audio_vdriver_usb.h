/*****************************************************************************
*
*    (c) 2010 Broadcom Corporation
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
*  @file  audio_vdriver_usb.h
*
*  @brief Audio Virtual Driver API for usb.
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


#ifndef	__AUDIO_VDRIVER_USB_H__
#define	__AUDIO_VDRIVER_USB_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	USBAUD_None,
	USBAUD_UL = 0x1,
	USBAUD_DL = 0x2
} USBAUD_TYPE_t;

typedef Boolean (*AUDDRV_USB_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
* Initialize audio USB virtual driver, init internal variables and task queue.
*
*	@param	none
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_USB_Init( void );


//*********************************************************************
/**
* Shut down audio USB virtual driver, free internal variables and task queue.
*
*	@param  none	
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_USB_Shutdown( void );

//*********************************************************************
/**
* Configure audio USB virtual driver, Set parameters before start render.
*
*	@param	speedMode	specify the voice data encoding format
*	@param	dataRateSelection	specify the bitrate of the encoded voice data
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_USB_SetConfig( AUDIO_SAMPLING_RATE_t inSampleRate);

//*********************************************************************
/**
* Register a buffer done callback to audio USB vdriver. 
*
*	@param	ulBufDoneCB		the callback function pointer for uplink data
*	@param	dlBufDoneCB		the callback function pointer for downlink data
*	@return	Result_t
*	@note	when driver finished the data in the buffer, driver generates 
*	this callback to notify the client.
**************************************************************************/
Result_t AUDDRV_USB_SetBufDoneCB ( AUDDRV_USB_BufDoneCB_t ulBufDoneCB, AUDDRV_USB_BufDoneCB_t dlBufDoneCB );

//*********************************************************************
/**
* Handle interrupt from DSP.
*
*	@param	usb_link	Indicates whether status from DSP is for UL and/or DL buffer.	
*	@return	Result_t
*	@note			Fill shared mem with UL data and read DL data from DSP when applicable.	
*	
**************************************************************************/
Result_t AUDDRV_USB_HandleDSPInt( UInt16 arg0, UInt16 inBuf, UInt16 outBuf );

//*********************************************************************
/**
* Read DL audio data from audio USB driver queue.
*
*	@param	pBuf	the buffer pointer of the data
*	@param	nSize	the buffer size of the data
*	@return	Result_t
*	@note	
*	
**************************************************************************/
UInt32 AUDDRV_USB_ReadDLBuf( UInt8* pBuf, UInt32 nSize );

//*********************************************************************
/**
* Send UL audio data to audio USB driver queue.
*
*	@param	pBuf	the buffer pointer of the data
*	@param	nSize	the buffer size of the data
*	@return	Result_t
*	@note
*	
**************************************************************************/
UInt32 AUDDRV_USB_WriteULBuf( UInt8* pBuf, UInt32 nSize );

//*********************************************************************
/**
* Start the data transfer in audio USB vdriver.
*
*	@param	none
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_USB_Start ( void );


//*********************************************************************
/**
* Stop the data transfer in audio USB vdriver.
*
*	@param  Boolean TRUE: stop immediately, FALSE: ???	
*	@return	Result_t
*	@note	
**************************************************************************/
Result_t AUDDRV_USB_Stop( Boolean	immediately );

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_USB_H__
