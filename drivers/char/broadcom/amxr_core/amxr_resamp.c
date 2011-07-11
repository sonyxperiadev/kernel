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
*  @file    amxr_resamp.c
*
*  @brief   This file implements the Audio Mixer resampler
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
//#include <gos_basic_types.h>         /* Needed for -EINVAL */
#include "amxr_resamp.h"       /* Resampler definitions */

/* ---- Private Constants and Types -------------------------------------- */

#define DECIM2TO1TAPLEN          46

#define DECIM3TO1TAPLEN          60

#define DECIM4TO1TAPLEN          60

#define DECIM5TO1TAPLEN          60

#define DECIM6TO1TAPLEN          60

#define RESAMP1TO3INTPR          3
#define RESAMP1TO3FILTLEN        32

#define RESAMP3TO2INTPR          2
#define RESAMP3TO2FILTLEN        28

#define RESAMP6TO5INTPR          5
#define RESAMP6TO5FILTLEN        24

#define RESAMP1TO2INTPR          2
#define RESAMP1TO2FILTLEN        18

#define RESAMP4TO5INTPR          5
#define RESAMP4TO5FILTLEN        20

#define RESAMP1TO4INTPR          4
#define RESAMP1TO4FILTLEN        24

#define RESAMP1TO5INTPR          5
#define RESAMP1TO5FILTLEN        24

#define RESAMP1TO6INTPR          6
#define RESAMP1TO6FILTLEN        24

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */

/* Resampling filter coefficients */
static int16_t decim2to1fir[DECIM2TO1TAPLEN] =
{
    27,	   -39,
   -51,	    43,
    86,	   -35,
  -133,	     6,
   188,	    61,
  -235,	  -178,
   229,	   301,
  -195,	  -463,
    76,	   619,
   143,	  -719,
  -462,	   703,
   853,	  -507,
 -1249,	    83,
  1544,	   585,
 -1592,	 -1450,
  1220,	  2363,
  -246,	 -3021,
 -1446,	  2867,
  3733,	  -977,
 -5709,	 -3894,
  3975,	 11126,
 12245,	  8200,
  3387,	   700,
};

static int16_t decim3to1fir[DECIM3TO1TAPLEN] =
{
    -8,	   -14,	    -7,
    12,	    27,	    18,
   -15,	   -46,	   -38,
    13,	    70,	    74,
     3,	   -95,	  -131,
   -47,	   109,	   209,
   141,	   -74,	  -272,
  -263,	    -6,	   319,
   429,	   174,	  -296,
  -603,	  -438,	   147,
   719,	   775,	   172,
  -689,	 -1118,	  -677,
   408,	  1339,	  1325,
   225,	 -1243,	 -1968,
 -1262,	   553,	  2270,
  2595,	  1065,	 -1543,
 -3636,	 -3768,	 -1524,
  2252,	  5973,	  8199,
  8335,	  6761,	  4436,
  2305,	   884,	   203,
};

static int16_t decim4to1fir[DECIM4TO1TAPLEN] =
{
   423,	   329,	    81,	  -224,
  -452,	  -494,	  -319,	    11,
   361,	   575,	   548,	   275,
  -139,	  -521,	  -698,	  -579,
  -195,	   302,	   700,	   819,
   586,	    80,	  -497,	  -901,
  -943,	  -580,	    60,	   725,
  1133,	  1092,	   586,	  -209,
  -987,	 -1427,	 -1321,	  -668,
   319,	  1281,	  1845,	  1758,
   994,	  -233,	 -1533,	 -2468,
 -2694,	 -2069,	  -692,	  1132,
  2993,	  4500,	  5382,	  5545,
  5072,	  4166,	  3078,	  2034,
  1184,	   590,	   237,	    67,
};

static int16_t decim5to1fir[DECIM5TO1TAPLEN] =
{
   518,	   194,	  -209,	  -568,	  -769,	
  -740,	  -479,	   -55,	   406,	   763,	
   900,	   765,	   386,	  -129,	  -628,	
  -957,	 -1007,	  -752,	  -255,	   344,	
   866,	  1153,	  1107,	   727,	   110,	
  -574,	 -1127,	 -1383,	 -1253,	  -753,	
    -5,	   795,	  1431,	  1721,	  1567,	
   982,	    89,	  -907,	 -1772,	 -2294,	
 -2330,	 -1843,	  -898,	   353,	  1706,	
  2951,	  3915,	  4490,	  4641,	  4409,	
  3884,	  3187,	  2436,	  1729,	  1134,	
   679,	   365,	   171,	    66,	    19,	
};

static int16_t decim6to1fir[DECIM6TO1TAPLEN] =
{
   439,	   257,	    15,	  -242,	  -465,	  -607,
  -638,	  -547,	  -344,	   -65,	   242,	   519,
   711,	   777,	   699,	   484,	   166,	  -203,
  -555,	  -825,	  -959,	  -925,	  -720,	  -371,
    67,	   524,	   919,	  1182,	  1260,	  1128,
   796,	   305,	  -276,	  -863,	 -1364,	 -1701,
 -1813,	 -1670,	 -1275,	  -663,	   106,	   951,
  1788,	  2537,	  3133,	  3531,	  3712,	  3683,
  3469,	  3114,	  2666,	  2176,	  1692,	  1248,
   869,	   566,	   340,	   185,	    87,	    35,
};

static int16_t resamp1to3fir[RESAMP1TO3INTPR * RESAMP1TO3FILTLEN] =
{
   -24,	    13,	    10,	   -58,	   141,	  -268,	   444,	  -669,	   934,	 -1228,	  1532,	 -1823,	  2081,	 -2282,	  2411,	 30277,	  2411,	 -2282,	  2081,	 -1823,	  1532,	 -1228,	   934,	  -669,	   444,	  -268,	   141,	   -58,	    10,	    13,	   -24,	    31,
    31,	   -55,	    96,	  -157,	   239,	  -334,	   429,	  -506,	   537,	  -487,	   313,	    53,	  -732,	  2030,	 -5147,	 25734,	 14533,	 -6047,	  3646,	 -2373,	  1535,	  -941,	   515,	  -220,	    30,	    77,	  -123,	   129,	  -112,	    88,	   -66,	    54,
    54,	   -66,	    88,	  -112,	   129,	  -123,	    77,	    30,	  -220,	   515,	  -941,	  1535,	 -2373,	  3646,	 -6047,	 14533,	 25734,	 -5147,	  2030,	  -732,	    53,	   313,	  -487,	   537,	  -506,	   429,	  -334,	   239,	  -157,	    96,	   -55,	    31,
};

static int16_t resamp3to2fir[RESAMP3TO2INTPR * RESAMP3TO2FILTLEN] =
{
     4,	    71,	  -139,	   113,	    59,	  -305,	   386,	   -59,	  -468,	   818,	  -480,	  -540,	  1458,	 -1225,	  -414,	  2257,	 -2346,	  -157,	  3455,	 -3913,	  -458,	  6291,	 -5490,	 -6102,	 13689,	 18627,	  7105,	   552,
   -53,	    80,	   -42,	   -76,	   214,	  -226,	   -12,	   413,	  -558,	   237,	   524,	 -1119,	   807,	   504,	 -1834,	  1736,	   284,	 -2768,	  3067,	   134,	 -4505,	  4847,	  1785,	 -9460,	  3391,	 19411,	 13315,	  2651,
};

static int16_t resamp6to5fir[RESAMP6TO5INTPR * RESAMP6TO5FILTLEN] =
{
    21,	   -52,	    56,	    15,	  -168,	   302,	  -201,	  -286,	   811,	  -803,	  -173,	  1613,	 -2033,	   268,	  2797,	 -4030,	   658,	  5604,	 -7047,	 -3757,	 16598,	 17365,	  5041,	   164,
    25,	   -40,	    18,	    68,	  -190,	   224,	    -8,	  -467,	   750,	  -386,	  -720,	  1739,	 -1334,	  -900,	  3294,	 -2904,	 -1459,	  6291,	 -4460,	 -6874,	 13258,	 19168,	  7199,	   486,
    24,	   -23,	   -17,	    99,	  -170,	   113,	   162,	  -541,	   556,	    58,	 -1081,	  1538,	  -457,	 -1816,	  3184,	 -1377,	 -3174,	  5955,	 -1475,	 -8833,	  9165,	 20150,	  9689,	  1064,
    20,	    -5,	   -42,	   105,	  -119,	    -4,	   278,	  -505,	   278,	   445,	 -1209,	  1079,	   426,	 -2336,	  2530,	   266,	 -4237,	  4720,	  1460,	 -9479,	  4678,	 20109,	 12360,	  1979,
    13,	    10,	   -53,	    87,	   -52,	  -104,	   325,	  -381,	   -22,	   708,	 -1101,	   466,	  1156,	 -2400,	  1488,	  1736,	 -4520,	  2840,	  3930,	 -8826,	   225,	 18924,	 15005,	  3294,
};

static int16_t resamp1to2fir[RESAMP1TO2INTPR * RESAMP1TO2FILTLEN] =
{
   -234,	   386,	  -573,	   681,	  -745,	   610,	  -173,	  -643,	  1830,	 -3258,	  4600,	 -5257,	  4227,	    16,	 -9270,	 21309,	 17600,	  1588,
   -122,	   111,	   -24,	  -207,	   512,	  -979,	  1538,	 -2060,	  2344,	 -2127,	  1121,	   929,	 -4095,	  7943,	-10566,	  5685,	 25201,	  7495,
};

static int16_t resamp4to5fir[RESAMP4TO5INTPR * RESAMP4TO5FILTLEN] =
{
    -2,	    30,	   -95,	   210,	  -381,	   615,	  -903,	  1178,	 -1512,	  1849,	 -2186,	  2543,	 -2986,	  3677,	 -4969,	  7665,	-13584,	 25216,	 16067,	   322,
    26,	   -26,	     2,	    68,	  -202,	   418,	  -719,	  1069,	 -1554,	  2118,	 -2766,	  3511,	 -4399,	  5546,	 -7184,	  9754,	-13861,	 17944,	 21792,	  1224,
    37,	   -59,	    80,	   -78,	    32,	    81,	  -273,	   536,	  -969,	  1519,	 -2203,	  3028,	 -4013,	  5199,	 -6657,	  8447,	-10229,	  8545,	 26687,	  3080,
    32,	   -61,	   108,	  -165,	   214,	  -237,	   219,	  -153,	   -65,	   381,	  -829,	  1415,	 -2131,	  2954,	 -3812,	  4476,	 -4074,	 -1140,	 29499,	  6189,
    14,	   -37,	    85,	  -165,	   275,	  -407,	   553,	  -701,	   760,	  -787,	   735,	  -599,	   398,	  -195,	   140,	  -603,	  2576,	 -9075,	 29176,	 10619,
};

static int16_t resamp1to4fir[RESAMP1TO4INTPR * RESAMP1TO4FILTLEN] =
{
   -28,	     0,	    77,	  -237,	   507,	  -892,	  1373,	 -1907,	  2430,	 -2873,	  3168,	 29450,	  3168,	 -2873,	  2430,	 -1907,	  1373,	  -892,	   507,	  -237,	    77,	     0,	   -28,	    41,	
    31,	   -84,	   186,	  -349,	   562,	  -785,	   950,	  -952,	   631,	   335,	 -3111,	 27032,	 11737,	 -5513,	  3345,	 -2042,	  1147,	  -540,	   162,	    35,	  -108,	   109,	   -86,	    70,	
    68,	  -113,	   190,	  -284,	   357,	  -349,	   178,	   259,	 -1108,	  2666,	 -5970,	 20494,	 20494,	 -5970,	  2666,	 -1108,	   259,	   178,	  -349,	   357,	  -284,	   190,	  -113,	    68,	
    70,	   -86,	   109,	  -108,	    35,	   162,	  -540,	  1147,	 -2042,	  3345,	 -5513,	 11737,	 27032,	 -3111,	   335,	   631,	  -952,	   950,	  -785,	   562,	  -349,	   186,	   -84,	    31,	
};

static int16_t resamp1to5fir[RESAMP1TO5INTPR * RESAMP1TO5FILTLEN] =
{
   -28,	     0,	    77,	  -237,	   507,	  -892,	  1373,	 -1907,	  2431,	 -2873,	  3168,	 29452,	  3168,	 -2873,	  2431,	 -1907,	  1373,	  -892,	   507,	  -237,	    77,	     0,	   -28,	    41,	
    21,	   -71,	   173,	  -342,	   574,	  -840,	  1075,	 -1184,	  1016,	  -275,	 -2110,	 27889,	  9932,	 -5111,	  3274,	 -2099,	  1250,	  -646,	   251,	   -28,	   -68,	    88,	   -76,	    66,	
    57,	  -109,	   201,	  -328,	   463,	  -548,	   503,	  -216,	  -476,	  1907,	 -5258,	 23503,	 17126,	 -6141,	  3151,	 -1605,	   680,	  -137,	  -138,	   233,	  -221,	   165,	  -108,	    73,	
    73,	  -108,	   165,	  -221,	   233,	  -138,	  -137,	   680,	 -1605,	  3151,	 -6141,	 17126,	 23503,	 -5258,	  1907,	  -476,	  -216,	   503,	  -548,	   463,	  -328,	   201,	  -109,	    57,	
    66,	   -76,	    88,	   -68,	   -28,	   251,	  -646,	  1250,	 -2099,	  3274,	 -5111,	  9932,	 27889,	 -2110,	  -275,	  1016,	 -1184,	  1075,	  -840,	   574,	  -342,	   173,	   -71,	    21,	
};

static int16_t resamp1to6fir[RESAMP1TO6INTPR * RESAMP1TO6FILTLEN] =
{
   -28,	     0,	    77,	  -237,	   507,	  -892,	  1373,	 -1907,	  2431,	 -2873,	  3168,	 29452,	  3168,	 -2873,	  2431,	 -1907,	  1373,	  -892,	   507,	  -237,	    77,	     0,	   -28,	    41,	
    13,	   -61,	   161,	  -333,	   577,	  -867,	  1149,	 -1330,	  1271,	  -698,	 -1367,	 28362,	  8742,	 -4799,	  3192,	 -2113,	  1303,	  -707,	   305,	   -68,	   -42,	    73,	   -69,	    63,	
    47,	  -100,	   199,	  -345,	   517,	  -665,	   712,	  -546,	     0,	  1265,	 -4465,	 25238,	 14756,	 -5981,	  3315,	 -1848,	   915,	  -330,	     0,	   145,	  -173,	   142,	  -100,	    73,	
    68,	  -113,	   190,	  -284,	   357,	  -349,	   178,	   259,	 -1108,	  2666,	 -5971,	 20496,	 20496,	 -5971,	  2666,	 -1108,	   259,	   178,	  -349,	   357,	  -284,	   190,	  -113,	    68,	
    73,	  -100,	   142,	  -173,	   145,	     0,	  -330,	   915,	 -1848,	  3315,	 -5981,	 14756,	 25238,	 -4465,	  1265,	     0,	  -546,	   712,	  -665,	   517,	  -345,	   199,	  -100,	    47,	
    63,	   -69,	    73,	   -42,	   -68,	   305,	  -707,	  1303,	 -2113,	  3192,	 -4799,	  8742,	 28362,	 -1367,	  -698,	  1271,	 -1330,	  1149,	  -867,	   577,	  -333,	   161,	   -61,	    13,	
};

/* Resampling lookup table */
static AMXR_RESAMPLE_TABENTRY gResampTable[] =
{
   {
      .rtype         = AMXR_RESAMP_TYPE_3TO1,
      .filterlen     = DECIM3TO1TAPLEN,
      .decim_ratio   = 3,
      .inter_ratio   = 1,
      .coeffs        = decim3to1fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_1TO3,
      .filterlen     = RESAMP1TO3FILTLEN,
      .decim_ratio   = 1,
      .inter_ratio   = 3,
      .coeffs        = resamp1to3fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_2TO1,
      .filterlen     = DECIM2TO1TAPLEN,
      .decim_ratio   = 2,
      .inter_ratio   = 1,
      .coeffs        = decim2to1fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_3TO2,
      .filterlen     = RESAMP3TO2FILTLEN,
      .decim_ratio   = 3,
      .inter_ratio   = 2,
      .coeffs        = resamp3to2fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_6TO5,
      .filterlen     = RESAMP6TO5FILTLEN,
      .decim_ratio   = 6,
      .inter_ratio   = 5,
      .coeffs        = resamp6to5fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_1TO2,
      .filterlen     = RESAMP1TO2FILTLEN,
      .decim_ratio   = 1,
      .inter_ratio   = 2,
      .coeffs        = resamp1to2fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_4TO5,
      .filterlen     = RESAMP4TO5FILTLEN,
      .decim_ratio   = 4,
      .inter_ratio   = 5,
      .coeffs        = resamp4to5fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_4TO1,
      .filterlen     = DECIM4TO1TAPLEN,
      .decim_ratio   = 4,
      .inter_ratio   = 1,
      .coeffs        = decim4to1fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_5TO1,
      .filterlen     = DECIM5TO1TAPLEN,
      .decim_ratio   = 5,
      .inter_ratio   = 1,
      .coeffs        = decim5to1fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_6TO1,
      .filterlen     = DECIM6TO1TAPLEN,
      .decim_ratio   = 6,
      .inter_ratio   = 1,
      .coeffs        = decim6to1fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_1TO4,
      .filterlen     = RESAMP1TO4FILTLEN,
      .decim_ratio   = 1,
      .inter_ratio   = 4,
      .coeffs        = resamp1to4fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_1TO5,
      .filterlen     = RESAMP1TO5FILTLEN,
      .decim_ratio   = 1,
      .inter_ratio   = 5,
      .coeffs        = resamp1to5fir,
   },
   {
      .rtype         = AMXR_RESAMP_TYPE_1TO6,
      .filterlen     = RESAMP1TO6FILTLEN,
      .decim_ratio   = 1,
      .inter_ratio   = 6,
      .coeffs        = resamp1to6fir,
   },

   {
      .rtype         = AMXR_RESAMP_TYPE_NONE,
      .filterlen     = 0,
      .decim_ratio   = 1,
      .inter_ratio   = 1,
      .coeffs        = NULL,
   }
};

/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */

/***************************************************************************/
/**
*  32-bit saturation
*
*  @return  saturated signed 32-bit value
*/
static int32_t saturate32(
   int64_t num                      /*<< (i) signed 64-bit number */
)
{
   int32_t sum = (int32_t)num;
   if ( num > 0x07fffffff )
   {
      sum = 0x7fffffff;
   }
   else if ( num < (-2147483647L - 1) )
   {
      sum = 0x80000000;
   }
   return sum;
}

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
)
{
   int            samptoskip;
   int            interppos;
   int            i, j;
   const int16_t *filt;

   /* Adjust sample pointer to start at the beginning of history
    * to perform forward traversal filtering
    */
   insamp    = insamp - filtlen + 1;
   interppos = 0;
   for ( i = 0; i < numsamp; i++ )
   {
      int16_t *sampptr;
      int64_t  sum;
      int32_t  satnum;

      filt    = &filtcoeff[interppos*filtlen];
      sampptr = insamp;
      sum     = 0x08000L;
      for ( j = 0; j < filtlen; j++ )
      {
         /* do filter in forward traversal */
         sum += (int32_t)(filt[j]) * (int32_t)(*sampptr++) * 2;
      }

      satnum = saturate32( sum );
      *outsamp = (int16_t)(satnum >> 16);
      outsamp++;

      interppos += decimfac;
      samptoskip = interppos / interpfac;
      interppos %= interpfac;
      insamp    += samptoskip;
   }
}

/***************************************************************************/
/**
*  Check whether resampler rates are feasible and select the
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
)
{
   AMXR_RESAMP_TYPE   rtype;

   if ( src_hz != dst_hz )
   {
      int ratio, ratio_mod;

      if ( src_hz > dst_hz )
      {
         /* Down-sampling */
         ratio     = src_hz / dst_hz;
         ratio_mod = ( src_hz % dst_hz );

         if ( ratio_mod == 0 )
         {
            /* Integral ratios: only support 6:1, 5:1, 4:1, 3:1 and 2:1 */
            switch ( ratio )
            {
               case 2:
                  rtype = AMXR_RESAMP_TYPE_2TO1;
                  break;
               case 3:
                  rtype = AMXR_RESAMP_TYPE_3TO1;
                  break;
               case 4:
                  rtype = AMXR_RESAMP_TYPE_4TO1;
                  break;
               case 5:
                  rtype = AMXR_RESAMP_TYPE_5TO1;
                  break;
               case 6:
                  rtype = AMXR_RESAMP_TYPE_6TO1;
                  break;
               default:
                  /* Unsupported ratio */
                  return -EINVAL;
            }
         }
         else
         {
            /* Fractional decimation */
            if ( ratio == 1 && (dst_hz % ratio_mod) == 0 )
            {
               if (( dst_hz/ratio_mod ) == 2 )
               {
                  rtype = AMXR_RESAMP_TYPE_3TO2;
               }
               else if (( dst_hz/ratio_mod ) == 5 )
               {
                  rtype = AMXR_RESAMP_TYPE_6TO5;
               }
               else
               {
                  /* Unsupported ratio */
                  return -EINVAL;
               }
            }
            else
            {
               /* Unsupported ratio */
               return -EINVAL;
            }
         }
      }
      else
      {
         /* Up-sampling */
         ratio     = dst_hz / src_hz;
         ratio_mod = ( dst_hz % src_hz );

         if ( ratio_mod == 0 )
         {
            switch ( ratio )
            {
               case 2:
                  rtype = AMXR_RESAMP_TYPE_1TO2;
                  break;
               case 3:
                  rtype = AMXR_RESAMP_TYPE_1TO3;
                  break;
               case 4:
                  rtype = AMXR_RESAMP_TYPE_1TO4;
                  break;
               case 5:
                  rtype = AMXR_RESAMP_TYPE_1TO5;
                  break;
               case 6:
                  rtype = AMXR_RESAMP_TYPE_1TO6;
                  break;
               default:
                  /* Unsupported ratio */
                  return -EINVAL;
            }
         }
         else
         {
            /* Fractional interpolation: only support 4:5 */
            if ( ratio == 1 && (src_hz/ratio_mod) == 5 && (src_hz % ratio_mod) == 0 )
            {
               rtype = AMXR_RESAMP_TYPE_4TO5;
            }
            else
            {
               /* Unsupported ratio */
               return -EINVAL;
            }
         }
      }
   }
   else
   {
      rtype = AMXR_RESAMP_TYPE_NONE;
   }

   *rtypep = rtype;

   return 0;
}

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
)
{
   int i;

   for ( i = 0; i < sizeof(gResampTable)/sizeof(gResampTable[0]); i++ )
   {
      if ( gResampTable[i].rtype == rtype )
      {
         /* Found resampler */
         *tablep = &gResampTable[i];
         return 0;
      }
   }

   /* Resampler type not found */
   return -EINVAL;
}

