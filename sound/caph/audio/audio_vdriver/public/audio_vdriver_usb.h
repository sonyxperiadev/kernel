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
