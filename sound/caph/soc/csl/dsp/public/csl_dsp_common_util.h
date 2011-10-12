//*********************************************************************
//
//	Copyright © 2000-2011 Broadcom Corporation
//
//	This program is the proprietary software of Broadcom Corporation
//	and/or its licensors, and may only be used, duplicated, modified
//	or distributed pursuant to the terms and conditions of a separate,
//	written license agreement executed between you and Broadcom (an
//	"Authorized License").  Except as set forth in an Authorized
//	License, Broadcom grants no license (express or implied), right
//	to use, or waiver of any kind with respect to the Software, and
//	Broadcom expressly reserves all rights in and to the Software and
//	all intellectual property rights therein.  IF YOU HAVE NO
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
//	ALL USE OF THE SOFTWARE.
//
//	Except as expressly set forth in the Authorized License,
//
//	1.	This program, including its structure, sequence and
//		organization, constitutes the valuable trade secrets
//		of Broadcom, and you shall use all reasonable efforts
//		to protect the confidentiality thereof, and to use
//		this information only in connection with your use
//		of Broadcom integrated circuit products.
//
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE,
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   csl_dsp_common_utils.h
*
*   @brief  This file contains DSP CSL API for DSP CSL Utilities common for 
*           both AP and CP.
*
****************************************************************************/
#ifndef _CSL_DSP_COMMON_UTILS_H_
#define _CSL_DSP_COMMON_UTILS_H_

// ---- Include Files -------------------------------------------------------
#include "mobcom_types.h"

/**
 * @addtogroup CSL_DSP_COMMON_UTILS
 * @{
 */
/* fixed point scale factor implementation */
#define GAIN_FRACTION_BITS_NUMBER	16
#define FIXED_POINT_UNITY_GAIN		(1<<GAIN_FRACTION_BITS_NUMBER)

#define TWO_IN_MILLIBEL				602
#define QUARTERDB_IN_MILLIBEL		25
#define QUARTERDB_ROUNDER			12
	

// ---- Function Declarations -----------------------------------------

//*********************************************************************
/**
*   CSL_ConvertMillibel2Scale converts gain in millibel to Q15.16 
*								linear scale factor.
*	Floating point implementation is replaced by fixed point with 
*	25mB (0.25 dB) resolution in order to avoid floating point math
*	in Linux kernel.
*	Less than 2% error over assumed -5000mB:5000mB (-50dB:50dB) 
*	dynamic range. 
*
*   @param    mBGain					(in)	gain in millibels
*   @return   UInt32					scale factor in Q15.16 format
* 
**********************************************************************/
UInt32 CSL_ConvertMillibel2Scale(Int16 mBGain);

/** @} */

#endif //_CSL_DSP_COMMON_UTILS_H_

