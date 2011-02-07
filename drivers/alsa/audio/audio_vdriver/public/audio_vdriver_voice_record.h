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

void AMRWB_Capture_Request(VPStatQ_t reqMsg);

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_VOICE_RECORD_H__
