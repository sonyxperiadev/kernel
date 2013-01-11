/******************************************************************************
*
* Copyright 2009 - 2012  Broadcom Corporation
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2 (the GPL),
*  available at
*
*      http://www.broadcom.com/licenses/GPLv2.php
*
*  with the following added to such license:
*
*  As a special exception, the copyright holders of this software give you
*  permission to link this software with independent modules, and to copy and
*  distribute the resulting executable under terms of your choice, provided
*  that you also meet, for each linked independent module, the terms and
*  conditions of the license of that module.
*  An independent module is a module which is not derived from this software.
*  The special exception does not apply to any modifications of the software.
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
******************************************************************************/
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

/* ---- Include Files ----------------------------------------------------- */
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

/* ---- Function Declarations ----------------------------------------- */

/*********************************************************************/
/**
*   CSL_ConvertMillibel2Scale converts gain in millibel to Q15.16
*	linear scale factor.
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

#endif /* _CSL_DSP_COMMON_UTILS_H_ */

