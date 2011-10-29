/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/**
*
*  @file    vcptest_tables.c
*
*  @brief   String tables and related functions
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/string.h>
#include <linux/broadcom/vcp.h>
#include "vcptest_tables.h"

/* ---- Public Variables ------------------------------------------------- */
/* ---- Private Constants and Types -------------------------------------- */
/* ---- Private Variables ------------------------------------------------ */
/* ---- Private Function Prototypes -------------------------------------- */
/* ---- Functions -------------------------------------------------------- */
const VCPTEST_TABLE vcptestModeTbl[] =
{
   {0,      "disable"},
   {1,      "enable"},
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestCodecTbl[] =
{
   {VCP_CODEC_H263,         "h263"},
   {VCP_CODEC_H264,         "h264"},
   {VCP_CODEC_MPEG4,        "mpeg4"},
   {VCP_CODEC_MJPEG,        "mjpeg"},
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoProfile[] =
{
   { VCP_PROFILE_H264_BASELINE , "h264_baseline" },
   { VCP_PROFILE_H264_MAIN     , "h264_main" },
   { VCP_PROFILE_H264_HIGH     , "h264_high" },
   { VCP_PROFILE_MPEG4_SIMPLE  , "mpeg4_simple" },
   { VCP_PROFILE_H263_PROFILE0 , "h263_profile0" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoLevel[] =
{
   { VCP_LEVEL_H264_1    , "h264_1" },
   { VCP_LEVEL_H264_1b   , "h264_1b" },
   { VCP_LEVEL_H264_1_1  , "h264_1.1" },
   { VCP_LEVEL_H264_1_2  , "h264_1.2" },
   { VCP_LEVEL_H264_1_3  , "h264_1.3" },
   { VCP_LEVEL_H264_2    , "h264_2" },
   { VCP_LEVEL_H264_2_1  , "h264_2.1" },
   { VCP_LEVEL_H264_2_2  , "h264_2.2" },
   { VCP_LEVEL_H264_3    , "h264_3" },
   { VCP_LEVEL_H264_3_1  , "h264_3.1" },
   { VCP_LEVEL_H264_3_2  , "h264_3.2" },
   { VCP_LEVEL_H264_4    , "h264_4" },
   { VCP_LEVEL_H264_4_1  , "h264_4.1" },
   { VCP_LEVEL_H264_4_2  , "h264_4.2" },
   { VCP_LEVEL_H264_5    , "h264_5" },
   { VCP_LEVEL_MPEG4_L0  , "mpeg4_l0" },
   { VCP_LEVEL_MPEG4_L0b , "mpeg4_l0b" },
   { VCP_LEVEL_MPEG4_L1  , "mpeg4_l1" },
   { VCP_LEVEL_MPEG4_L2  , "mpeg4_l2" },
   { VCP_LEVEL_MPEG4_L3  , "mpeg4_l3" },
   { VCP_LEVEL_MPEG4_L4a , "mpeg4_l4a" },
   { VCP_LEVEL_MPEG4_L5  , "mpeg4_l5" },
   { VCP_LEVEL_MPEG4_L6  , "mpeg4_l6" },   
   { VCP_LEVEL_H263_L10  , "h263_l10" },
   { VCP_LEVEL_H263_L20  , "h263_l20" },
   { VCP_LEVEL_H263_L30  , "h263_l30" },
   { VCP_LEVEL_H263_L40  , "h263_l40" },
   { VCP_LEVEL_H263_L45  , "h263_l45" },
   { VCP_LEVEL_H263_L50  , "h263_l50" },
   { VCP_LEVEL_H263_L60  , "h263_l60" },
   { VCP_LEVEL_H263_L70  , "h263_l70" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoFrameRate[] =
{
   { VCP_FRAME_RATE_5_FPS , "5fps" },
   { VCP_FRAME_RATE_10_FPS, "10fps" },
   { VCP_FRAME_RATE_15_FPS, "15fps" },
   { VCP_FRAME_RATE_20_FPS, "20fps" },
   { VCP_FRAME_RATE_24_FPS, "24fps" },
   { VCP_FRAME_RATE_30_FPS, "30fps" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoBitRate[] =
{
   { VCP_BIT_RATE_10_KBPS,  "10k" },
   { VCP_BIT_RATE_64_KBPS,  "64k" },
   { VCP_BIT_RATE_256_KBPS, "256k" },
   { VCP_BIT_RATE_384_KBPS, "384k" },
   { VCP_BIT_RATE_512_KBPS, "512k" },
   { VCP_BIT_RATE_768_KBPS, "768k" },
   { VCP_BIT_RATE_1_MBPS,   "1m" },
   { VCP_BIT_RATE_1_6_MBPS, "1.6m" },
   { VCP_BIT_RATE_2_MBPS,   "2m" },
   { VCP_BIT_RATE_3_MBPS,   "3m" },
   { VCP_BIT_RATE_4_MBPS,   "4m" },
   { VCP_BIT_RATE_6_MBPS,   "6m" },
   { VCP_BIT_RATE_8_MBPS,   "8m" },
   { VCP_BIT_RATE_10_MBPS,   "10m" },
   { VCP_BIT_RATE_12_MBPS,   "12m" },
   { VCP_BIT_RATE_14_MBPS,  "14m" },
   { VCP_BIT_RATE_MAX,      "" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoResolution[] =
{
   { VCP_RESOLUTION_SQCIF, "sqcif" },
   { VCP_RESOLUTION_QCIF , "qcif" },
   { VCP_RESOLUTION_QVGA , "qvga" },
   { VCP_RESOLUTION_CIF  , "cif" },
   { VCP_RESOLUTION_VGA  , "vga" },
   { VCP_RESOLUTION_WVGA , "wvga" },
   { VCP_RESOLUTION_4CIF , "4cif" },
   { VCP_RESOLUTION_HD   , "hd" },
   { VCP_RESOLUTION_1080P, "1080p" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoMaxGopSize[] =
{
   { VCP_GOP_SIZE_0_5_SEC     , "0.5s" },
   { VCP_GOP_SIZE_1_SEC       , "1s" },
   { VCP_GOP_SIZE_2_SEC       , "2s" },
   { VCP_GOP_SIZE_4_SEC       , "4s" },
   { VCP_GOP_SIZE_8_SEC       , "8s" },
   { VCP_GOP_SIZE_30_SEC      , "30s" },
   { VCP_GOP_SIZE_NO_I_FRAME  , "no_iframe" },
   { VCP_GOP_SIZE_ALL_I_FRAMES, "all_iframe" },
   { VCP_GOP_SIZE_ALTERNATE_IP, "alternate" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoNalUnitSize[] =
{
   { VCP_NAL_UNIT_SIZE_OFF , "0" },
   { VCP_NAL_UNIT_SIZE_500 , "500" },
   { VCP_NAL_UNIT_SIZE_700 , "700" },
   { VCP_NAL_UNIT_SIZE_1000, "1000" },
   { VCP_NAL_UNIT_SIZE_1200, "1200" },
   { VCP_NAL_UNIT_SIZE_1500, "1500" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoDisplayDevice[] =
{
   { VCP_DISPLAY_LCD  , "lcd" },
   { VCP_DISPLAY_HDMI , "hdmi" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestVideoDisplayTransform[] =
{
   { VCP_DISPLAY_ROT0          , "rot0" },
   { VCP_DISPLAY_MIRROR_ROT0   , "mirror_rot0" },
   { VCP_DISPLAY_MIRROR_ROT180 , "mirror_rot180" },
   { VCP_DISPLAY_ROT180        , "rot180" },
   { VCP_DISPLAY_DUMMY         , "dummy" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestEncTest[] =
{
   { VCPTEST_ENC_FILESAVE_RAW     , "save_raw" },
   { VCPTEST_ENC_FILESAVE_HEADER  , "save_hdr" },
   { VCPTEST_ENC_FILE_NOSAVE      , "nosave" },
   { VCPTEST_ENC_LOOPBACK         , "loopback" },
   { 0 , "" } // Keep this last!
};
const VCPTEST_TABLE vcptestDecTest[] =
{
   { VCPTEST_DEC_LISTEN      , "listen" },
   { VCPTEST_DEC_FILEONCE    , "file" },
   { VCPTEST_DEC_FILENONSTOP , "file_nonstop" },
   { 0 , "" } // Keep this last!
};

/*****************************************************************************
*
*  FUNCTION:   vcptestTableCode2Str
*
*  PURPOSE:    search a list for a string based on the supplied integer value
*
*  PARAMETERS: code     - input  - integer value to search the list for
*              list     - input  - list to search
*
*  RETURNS:    str or "Unknown"
*
*  NOTES:
*
*****************************************************************************/
const char * vcptestTableCode2Str( int code, const VCPTEST_TABLE *list )
{
   while( list->str[ 0 ] != '\0' )
   {
      if( list->code == code )
      {
         return( list->str );
      }
      list++;
   }
   return( "Unknown" );
}
/*****************************************************************************
*
*  FUNCTION:   vcptestTableStr2Code
*
*  PURPOSE:    search a table for an integer code based on the supplied string.  If the string is
*              not matched ensure the code passed to this function is in range.
*
*  PARAMETERS: code     - input/output - on input it's the integer value entered
*                                        on the command line when there is no string entered
*                                      - on output, if a string was matched it is the code
*                                        corresponding to the string.
*              str      - input  - integer value to search the list for
*              table    - input  - table to search
*
*  RETURNS:    0 - valid code
*              1 - invalid code
*
*  NOTES:
*
*****************************************************************************/
int vcptestTableStr2Code( int *code, char *str, const VCPTEST_TABLE *table )
{
   const VCPTEST_TABLE *list;

   /* check if it is an empty command */
   if( (!str) || (strlen(str) == 0) )
   {
      return( 1 );
   }

   list = table;
   while( strcmp("", list->str) )
   {
      if( strcasecmp(str, list->str) == 0 )
      {
         *code = list->code;
         return( 0 );
      }
      list++;
   }
   /* The str did not match any entries in the VCPTEST_TABLE.  Check to see if
    * the code is valid */
   list = table;
   while( strcmp("", list->str) )
   {
      if( *code == list->code )
      {
         /* A valid matching code has been found */
         return( 0 );
      }
      list++;
   }

   return( 1 );
}
