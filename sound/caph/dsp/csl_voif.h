//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*   @file   csl_voif.h
*
*   @brief  This file contains DSP VoIF interface
*
****************************************************************************/
#ifndef _CSL_VOIF_H_
#define _CSL_VOIF_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"

/**
 * @addtogroup CSL VoIF interface
 * @{
 */

//*********************************************************************
/**
*
*   CSL_GetULVoIFBuffer gets pointer to UL VoIF buffer
*
*   @return   Int16*						uplink VoIF buffer	
* 
**********************************************************************/
Int16* CSL_GetULVoIFBuffer(void);

//*********************************************************************
/**
*
*   CSL_GetDLVoIFBuffer gets pointer to DL VoIF buffer
*
*   @param    sampleCount		(in)		number of samples
*   @param    dlBufferIndex		(in)		downlink ping-pong buffer index
*   @return   Int16*						downlink VoIF buffer
* 
**********************************************************************/
Int16* CSL_GetDLVoIFBuffer(UInt16 sampleCount, UInt16 dlBufferIndex);


/** @} */

#endif //_CSL_VOIF_H_
