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
