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
*  @file  audio_vdriver_voice_record.h
*
*  @brief Audio Virtual Driver API for voice record
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


#ifndef	__AUDIO_VDRIVER_VOICE_RECORD_H__
#define	__AUDIO_VDRIVER_VOICE_RECORD_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif


//#define AMRWB_CAPTURE_DATA_LOG //comment out for release
typedef enum VOCAPTURE_TYPE_t
{
	VOCAPTURE_TYPE_AMRNB,
	VOCAPTURE_TYPE_PCM,
	VOCAPTURE_TYPE_AMRWB
} VOCAPTURE_TYPE_t;




typedef Boolean (*AUDDRV_VoiceCapture_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
*   Initialize voice capture driver 

*	@param		type		type of voice capture
*	@return		Result_t		            
*   @note					      
**********************************************************************/
Result_t AUDDRV_VoiceCapture_Init( VOCAPTURE_TYPE_t type );

//*********************************************************************
/**
*   Shut down voice capturee driver 

*	@param		type		type of voice capture
*	@return		Result_t		            
*   @note					      
**********************************************************************/ 
Result_t AUDDRV_VoiceCapture_Shutdown( VOCAPTURE_TYPE_t type );


//*********************************************************************
/**
* Set the driver transfer parameters before configure driver, if needed.
*
*	@param	type						The voice capture driver type
*	@param	callbackThreshold(in ms)	Driver will callback when buffer size is lower than the threshold
*	@param	interruptInterval(in ms)	The DMA intterrupt interval
*	@return	Result_t
*	@note   Driver will use default values if this function is not called	
**************************************************************************/
Result_t AUDDRV_VoiceCapture_SetTransferParameters(
                        VOCAPTURE_TYPE_t      type,
                        UInt32				callbackThreshold,
                        UInt32				interruptInterval);

//*********************************************************************
/**
*   Configure voice capturee driver 

*	@param		type		type of voice capture
*	@param		speech_mode	the voice encoding data format
*	@param		amr_data_rate	the bit rate of the encoded voice data
*	@param		record_mode	specify record ul, dl or both
*	@param		sample_rate	specify the sampling rate of the PCM data
*	@param		audio_proc_enable	specify to enable the voice process or not
*	@param		vp_dtx_enable	specify enable dtx or not
*	@return		Result_t		            
*   @note					      
**********************************************************************/ 
Result_t AUDDRV_VoiceCapture_SetConfig(
                        VOCAPTURE_TYPE_t      type,
						UInt32				speech_mode,
						UInt8				amr_data_rate, // VP_Mode_AMR_t for nb, AMR_BandMode_WB_t for wb
						VOCAPTURE_RECORD_MODE_t  record_mode,
                        AUDIO_SAMPLING_RATE_t    sample_rate,
						Boolean				audio_proc_enable,
						Boolean				vp_dtx_enable);



//*********************************************************************
/**
*   Register voice capturee driver' buffer done callback 

*	@param		type		type of voice capture
*	@param		bufDone_cb	the callback function pointer
*	@return		Result_t		            
*   @note		when driver finishes coping the data from the buffer to driver queue
// driver calls this callback to notify.				      
**********************************************************************/ 
Result_t AUDDRV_VoiceCapture_SetBufDoneCB ( 
                     VOCAPTURE_TYPE_t    type,
                     AUDDRV_VoiceCapture_BufDoneCB_t           bufDone_cb );

//*********************************************************************
/**
*   Read data from voice capturee driver 

*	@param		type		type of voice capture
*	@param		pBuf		the buffer pointer of the data copy destination
*	@param		nSize		the buffer size of the data 
*	@return		Result_t		            
*   @note					      
**********************************************************************/ 
UInt32 AUDDRV_VoiceCapture_Read( 
                    VOCAPTURE_TYPE_t     type,
                    UInt8*                 pBuf,
                    UInt32	               nSize );

//*********************************************************************
/**
*   Start the data transfer of voice capturee driver 

*	@param		type		type of voice capture
*	@return		Result_t		            
*   @note					      
**********************************************************************/ 
Result_t AUDDRV_VoiceCapture_Start ( VOCAPTURE_TYPE_t      type );

//*********************************************************************
/**
*   Pause the data transfer of voice capturee driver 

*	@param		type		type of voice capture
*	@return		Result_t		            
*   @note					      
**********************************************************************/ 
Result_t AUDDRV_VoiceCapture_Pause ( VOCAPTURE_TYPE_t      type );

//*********************************************************************
/**
*   Resume the data transfer of voice capturee driver  

*	@param		type		type of voice capture
*	@return		Result_t		            
*   @note					      
**********************************************************************/ 
Result_t AUDDRV_VoiceCapture_Resume( VOCAPTURE_TYPE_t      type );

//*********************************************************************
/**
*   Stop the data transfer of voice capturee driver  

*	@param		type		type of voice capture
*	@return		Result_t		            
*   @note					      
**********************************************************************/ 
Result_t AUDDRV_VoiceCapture_Stop( 
                      VOCAPTURE_TYPE_t      type,
                      Boolean                 immediately );

void AMRWB_Capture_Request(UInt16 size, UInt16 index);

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_VOICE_RECORD_H__
