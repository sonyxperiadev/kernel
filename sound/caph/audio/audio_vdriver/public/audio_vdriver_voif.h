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
*  @file  audio_vdriver_voif.h
*
*  @brief Audio Virtual Driver API for voice call interface. Used for DyVE for example.
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


#ifndef	__AUDIO_VDRIVER_VOIF_H__
#define	__AUDIO_VDRIVER_VOIF_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif


#define AUDIO_DRIVER_VOIF_ENABLED // flag to turn on/off this object

#ifdef AUDIO_DRIVER_VOIF_ENABLED // real definitions

//*********************************************************************
/**
* Prototype of voif callback function.
*
*	@param	ulData: The data pointer of the UL data. Input of VOIF processing
*	@param	dlData: The data pointer of the DL data. Input of VOIF processing and output VOIF processing.
*	@param	sampleCout: the number of samples, 16 bit per sample
*	@return	void
*	@note	The user code use the callback function to get UL/DL data and write back processed DL.	
**************************************************************************/
typedef void (*VOIF_CB) (Int16 * ulData, Int16 *dlData, UInt32 sampleCount); 


//*********************************************************************
/**
* Start the VOIF processing.
*
*	@param	voifCB: the callback function from application to process the UL/DL data
*	@return	void
*	@note		
**************************************************************************/
void AUDDRV_VOIF_Start (VOIF_CB voifCB);

//*********************************************************************
/**
* Stop the VOIF processing.
*
*	@param	none
*	@return	void
*	@note		
**************************************************************************/
void AUDDRV_VOIF_Stop (void);

//*********************************************************************
/**
* VOIF processing interrupt handler.
*
*	@param	bufferIndex: The index of the ping-pong buffers to hold the DL/DL data
*	@return	void
*	@note	Not for application programming.	
**************************************************************************/
// voif interrupt handler, called by hw interrupt. 
void VOIF_ISR_Handler (UInt32 bufferIndex, UInt32 samplingRate);


#endif // AUDIO_DRIVER_VOIF_ENABLED

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VDRIVER_VOIF_H__
