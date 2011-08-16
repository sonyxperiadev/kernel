/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/


/**
*
*  @file    amxr_vectmul.c
*
*  @brief   This file implements the C model for the vector multiply 
*           operations.
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include "amxr_vectmul.h"

/* ---- Private Constants and Types -------------------------------------- */
/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  16-bit saturation
*
*  @return  saturated signed 16-bit value
*/
static int16_t saturate16( 
   int32_t num                         /*<< (i) signed 32-bit number */
)
{
   int16_t result = (int16_t)num;
   if ( num > 0x07fff )
   {
      result = 0x7fff;
   }
   else if ( num < -32768 )
   {
      result = 0x8000;
   }
   return result;
}

/***************************************************************************/
/**
*  Vector multiply with Q16 multiplican
*
*  dst[i] = (src[i] * q16) >> 16
*/
void amxrCVectorMpyQ16
(
   int16_t       *dstp,             /*<< (o) Ptr to output samples */
   const int16_t *srcp,             /*<< (i) Ptr to input samples */
   int            numsamp,          /*<< (i) Number of samples to add */
   uint16_t       q16gain           /*<< (i) Q16 linear gain value to multiply with */
)
{
   int k;
   for ( k = 0; k < numsamp; k++ )
   {
      dstp[k] = (int32_t)(((int32_t)(srcp[k]) * (int32_t)q16gain) >> 16);
   }
}

/***************************************************************************/
/**
*  Vector multiply and accumulate with Q16 constant multiplicand
*
*  dst[i] += (src[i] * q16) >> 16
*/
void amxrCVectorMacQ16
(
   int16_t       *dstp,             /*<< (o) Ptr to output samples */
   const int16_t *srcp,             /*<< (i) Ptr to input samples */
   int            numsamp,          /*<< (i) Number of samples to add */
   uint16_t       q16gain           /*<< (i) Q16 linear gain value to multiply with */
)
{
   int      k;
   int16_t  a;
   for ( k = 0; k < numsamp; k++ )
   {
      a        = (int32_t)(((int32_t)(srcp[k]) * (int32_t)q16gain) >> 16);
      dstp[k]  = saturate16( (int32_t)dstp[k] + (int32_t)a );
   }
}
