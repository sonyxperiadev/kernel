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
*  @file    amxr_resamp.h
*
*****************************************************************************/
#if !defined( AMXR_RESAMP_H )
#define AMXR_RESAMP_H

/* ---- Include Files ---------------------------------------------------- */
#include <linux/types.h>                   /* Needed for int16_t */
#include <linux/broadcom/amxr.h>                          /* AMXR API */

/* ---- Constants and Types ---------------------------------------------- */

/**
*  Types of supported resamplers
*/
typedef enum
{
   AMXR_RESAMP_TYPE_NONE = 0,          /* No resampling needed */  
   AMXR_RESAMP_TYPE_6TO1,              /* Decimate 6:1 */
   AMXR_RESAMP_TYPE_5TO1,              /* Decimate 5:1 */
   AMXR_RESAMP_TYPE_4TO1,              /* Decimate 4:1 */
   AMXR_RESAMP_TYPE_3TO1,              /* Decimate 3:1 */
   AMXR_RESAMP_TYPE_2TO1,              /* Decimate 2:1 */
   AMXR_RESAMP_TYPE_3TO2,              /* Decimate 3:2 */
   AMXR_RESAMP_TYPE_6TO5,              /* Decimate 6:5 */
   AMXR_RESAMP_TYPE_1TO2,              /* Interpolate 1:2 */
   AMXR_RESAMP_TYPE_1TO3,              /* Interpolate 1:3 */
   AMXR_RESAMP_TYPE_1TO4,              /* Interpolate 1:4 */
   AMXR_RESAMP_TYPE_1TO5,              /* Interpolate 1:5 */
   AMXR_RESAMP_TYPE_1TO6,              /* Interpolate 1:6 */
   AMXR_RESAMP_TYPE_4TO5,              /* Interpolate 4:5 */
}
AMXR_RESAMP_TYPE;

/* Maximum filter length (i.e. filter order) supported */
#define AMXR_FILTENLEN_MAXSZ           400

/* Maximum history size and sample frame size (240 sample = 5ms of 48kHz)
 * in 16-bit samples */
#define AMXR_RESAMP_BUFFER_BYTES       ((AMXR_FILTENLEN_MAXSZ + 240) * sizeof(int16_t))

/**
*  Resampler look-up table entry
*/
typedef struct amxr_resample_tabentry
{
   AMXR_RESAMP_TYPE     rtype;         /* Resampler type */
   int                  filterlen;     /* Filter length */
   int                  decim_ratio;   /* Decimation ratio. Resampling = inter/decim */
   int                  inter_ratio;   /* Interpolation ratio. Resampling = inter/decim */
   const int16_t       *coeffs;        /* Pointer to coefficient table */
}
AMXR_RESAMPLE_TABENTRY;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

/***************************************************************************/
/**
*  Check whether resampler rates are feasible and return the 
*  appropriate resampler type.
*
*  @return
*     0        Success, appropriate resampler exists
*     -EINVAL  No appropriate resampler can be found
*/
int amxr_check_resample_rates(
   int                  src_hz,     /*<< (i) Source sampling freq in Hz */
   int                  dst_hz,     /*<< (i) Destination sampling freq in Hz */
   AMXR_RESAMP_TYPE    *rtypep      /*<< (o) Ptr to resampler type */
);

/***************************************************************************/
/**
*  Lookup resampler information.
*
*  @return
*     0        Success, valid resampler information returned
*     -EINVAL  No appropriate resampler can be found
*/
int amxr_get_resampler(
   AMXR_RESAMP_TYPE         rtype,   /*<< (i) Resampler type */
   AMXR_RESAMPLE_TABENTRY **tablep   /*<< (o) Return pointer to resampler table */
);

/***************************************************************************/
/**
*  Generic resampler. Provided an appropriate filter is designed, 
*  this resampler can accommodate any resampling ratio of 
*  interpfac/decimfac.
*/
void amxrCResample
(
   int16_t       *insamp,           /*<< (i) Ptr to input samples */
   int16_t       *outsamp,          /*<< (o) Ptr to output samples */
   int16_t        numsamp,          /*<< (i) Number of samples to generate */
   const int16_t *filtcoeff,        /*<< (i) Ptr to filter coefficients */
   int            filtlen,          /*<< (i) Filter length */
   int            interpfac,        /*<< (i) Interpolation factor */
   int            decimfac          /*<< (i) Decimation factor */
);

#endif /* AMXR_RESAMP_H */

