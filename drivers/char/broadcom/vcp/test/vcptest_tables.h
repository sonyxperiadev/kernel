/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
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

/*
*****************************************************************************
*
*  pxcon_tables.h
*
*  PURPOSE:
*       Definitions for tables of UI command arguments.
*
*  NOTES:
*
*****************************************************************************/
#ifndef VCPTEST_TABLES_H
#define VCPTEST_TABLES_H

/* ---- Include Files ---------------------------------------- */
/* ---- Constants and Types ---------------------------------- */

   /* Cross reference between a string and value for a command-line argument */
typedef struct
{
   int code;                        /* integer vale of a command line argument */
   const char *str;                 /* string equivalent of the command line argument */
}
VCPTEST_TABLE;

typedef enum
{
   VCPTEST_ENC_FILESAVE_RAW,
   VCPTEST_ENC_FILESAVE_HEADER,
   VCPTEST_ENC_FILE_NOSAVE,
   VCPTEST_ENC_LOOPBACK
}
VCPTEST_ENC_MODE;

typedef enum
{
   VCPTEST_DEC_LISTEN,
   VCPTEST_DEC_FILEONCE,
   VCPTEST_DEC_FILENONSTOP
}
VCPTEST_DEC_MODE;


/* ---- Variable Externs ------------------------------------- */
extern const VCPTEST_TABLE vcptestModeTbl[];
extern const VCPTEST_TABLE vcptestCodecTbl[];
extern const VCPTEST_TABLE vcptestVideoProfile[];
extern const VCPTEST_TABLE vcptestVideoLevel[];
extern const VCPTEST_TABLE vcptestVideoFrameRate[];
extern const VCPTEST_TABLE vcptestVideoBitRate[];
extern const VCPTEST_TABLE vcptestVideoResolution[];
extern const VCPTEST_TABLE vcptestVideoMaxGopSize[];
extern const VCPTEST_TABLE vcptestVideoNalUnitSize[];
extern const VCPTEST_TABLE vcptestVideoDisplayDevice[];
extern const VCPTEST_TABLE vcptestVideoDisplayTransform[];
extern const VCPTEST_TABLE vcptestEncTest[];
extern const VCPTEST_TABLE vcptestDecTest[];

/* ---- Function Prototypes ---------------------------------- */
const char * vcptestTableCode2Str( int code, const VCPTEST_TABLE *list );
int vcptestTableStr2Code( int *code, char *str, const VCPTEST_TABLE *list );


#endif
