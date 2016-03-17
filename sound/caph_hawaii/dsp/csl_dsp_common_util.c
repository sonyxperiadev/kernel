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
*   @file   csl_dsp_common_utils.c
*
*   @brief  This file contains DSP CSL Utilities common for both AP and CP.
*
****************************************************************************/
#include "mobcom_types.h"
#include "csl_dsp_common_util.h"
#include "audio_trace.h"

static const UInt32 Log2LinearTable[] = {
	32845,			/* index:0     -600mB in Q15.16 linear scale */
	33804,			/* index:1     -575mB in Q15.16 linear scale */
	34792,			/* index:2     -550mB in Q15.16 linear scale */
	35807,			/* index:3     -525mB in Q15.16 linear scale */
	36853,			/* index:4     -500mB in Q15.16 linear scale */
	37929,			/* index:5     -475mB in Q15.16 linear scale */
	39037,			/* index:6     -450mB in Q15.16 linear scale */
	40177,			/* index:7     -425mB in Q15.16 linear scale */
	41350,			/* index:8     -400mB in Q15.16 linear scale */
	42557,			/* index:9     -375mB in Q15.16 linear scale */
	43800,			/* index:10    -350mB in Q15.16 linear scale */
	45079,			/* index:11    -325mB in Q15.16 linear scale */
	46395,			/* index:12    -300mB in Q15.16 linear scale */
	47750,			/* index:13    -275mB in Q15.16 linear scale */
	49145,			/* index:14    -250mB in Q15.16 linear scale */
	50580,			/* index:15    -225mB in Q15.16 linear scale */
	52057,			/* index:16    -200mB in Q15.16 linear scale */
	53577,			/* index:17    -175mB in Q15.16 linear scale */
	55141,			/* index:18    -150mB in Q15.16 linear scale */
	56751,			/* index:19    -125mB in Q15.16 linear scale */
	58409,			/* index:20    -100mB in Q15.16 linear scale */
	60114,			/* index:21    -75mB in Q15.16 linear scale */
	61869,			/* index:22    -50mB in Q15.16 linear scale */
	63676,			/* index:23    -25mB in Q15.16 linear scale */
	65536,			/* index:24    0mB in Q15.16 linear scale */
	67449,			/* index:25    25mB in Q15.16 linear scale */
	69419,			/* index:26    50mB in Q15.16 linear scale */
	71446,			/* index:27    75mB in Q15.16 linear scale */
	73532,			/* index:28    100mB in Q15.16 linear scale */
	75679,			/* index:29    125mB in Q15.16 linear scale */
	77889,			/* index:30    150mB in Q15.16 linear scale */
	80164,			/* index:31    175mB in Q15.16 linear scale */
	82504,			/* index:32    200mB in Q15.16 linear scale */
	84914,			/* index:33    225mB in Q15.16 linear scale */
	87393,			/* index:34    250mB in Q15.16 linear scale */
	89945,			/* index:35    275mB in Q15.16 linear scale */
	92572,			/* index:36    300mB in Q15.16 linear scale */
	95275,			/* index:37    325mB in Q15.16 linear scale */
	98057,			/* index:38    350mB in Q15.16 linear scale */
	100920,			/* index:39    375mB in Q15.16 linear scale */
	103867,			/* index:40    400mB in Q15.16 linear scale */
	106900,			/* index:41    425mB in Q15.16 linear scale */
	110022,			/* index:42    450mB in Q15.16 linear scale */
	113234,			/* index:43    475mB in Q15.16 linear scale */
	116541,			/* index:44    500mB in Q15.16 linear scale */
	119944,			/* index:45    525mB in Q15.16 linear scale */
	123446,			/* index:46    550mB in Q15.16 linear scale */
	127051,			/* index:47    575mB in Q15.16 linear scale */
	130761			/* index:48    600mB in Q15.16 linear scale */
};

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
UInt32 CSL_ConvertMillibel2Scale(Int16 mBGain)
{
	Int16 exp, remainder;
	UInt16 offset, index;
	UInt32 scale;

	/* calculate exponent */
	exp = mBGain / TWO_IN_MILLIBEL;

	/* calculate remainder in [-TWO_IN_MILLIBEL:TWO_IN_MILLIBEL] range */
	remainder = mBGain - exp * TWO_IN_MILLIBEL;

	/* shift remainder to [0:2*TWO_IN_MILLIBEL] range */
	offset = (UInt16) (remainder + TWO_IN_MILLIBEL);

	/* calculate index */
	index = (offset + QUARTERDB_ROUNDER) / QUARTERDB_IN_MILLIBEL;

	scale = Log2LinearTable[index];

	/* adjust scale to exponent value */
	if (exp > 0)
		scale <<= exp;
	else
		scale >>= (-exp);

	return scale;

}				/* ConvertMillibel2Scale */
