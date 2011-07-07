/****************************************************************************
*
*     Copyright (c) 2005 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/* =========================================================================
** Copyright (C) 1998 - 2005 COMNEON GmbH & Co. OHG. All rights reserved.
** =========================================================================
**
** =========================================================================
**
** This document contains proprietary information belonging to COMNEON.
** Passing on and copying of this document, use and communication of its
** contents is not permitted without prior written authorisation.
**
** =========================================================================
**
** Revision Information :
**    File name:   bastypes.h
**    Version:   /main/system/15
**    Date:   2005-01-10    15:45:00
**
** =========================================================================
*/

/* ===========================================================================
**
** History:
**
** Date		Author          Comment
** ---------------------------------------------------------------------------
** 2004-12-13  nourbaks Added SDTENV to avoid compiler error due to long long
** 2004-02-13  schmulbo CN1100002836 avoids compiler warning "unused parameter"
** 2004-01-21  MZI          CN1100002648 LONGLONG_EMUL
** 2003-12-16  GHI          CN1100002477 avoids ANSI C compiler erors
**                          gcc compiler switch -ansi must be used!
** 2003-11-27  GHI          CN1100002320 bastypes.h: near, far, bit
** 2003-11-18  GHI          CN1100002187 Add STD_OK and STD_ERR to bastypes.h
** 2003-10-21  MZI          Added ullong.
** 03-07-21    WMA          created: merged bastypes from different vobs
** ===========================================================================
*/
/*
** ===========================================================================
**
**				 DEFINITIONS
**
** ===========================================================================
*/
#ifndef BASTYPES_H

#ifdef SDL_MODULE_TEST
#include "Umts_asn1_def0.h"
#include "Umts_asn1_def1.h"
#include "Umts_asn1_def2.h"
#include "Umts_asn1_def3.h"
#endif

#define BASTYPES_H 1

#if defined (SDL_MODULE_TEST) || defined (SDTENV)
#ifndef __align
#define __align(i)
#endif
#endif

#ifdef STACK_WIN32_BUILD
typedef unsigned char		Boolean;
typedef unsigned char		UInt8;
#endif

#if defined(PCTEST) || defined(SGOLD) || defined (SGOLDLITE)
#define near
#define far
#define bit
#else
#define near _near
#define far  _far
#define bit  //_bit  Comment out by Xiaoxin. 11/5/04
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif

#define STD_OK    0
#ifndef STD_ERR
#define STD_ERR (-1)
#endif

/*
** ===========================================================================
**
**				   TYPES
**
** ===========================================================================
*/
#define int16               short
#define int32               int

#ifndef MACEHS_HOST
typedef unsigned char       ubyte;         /*           0 to +255        */
typedef signed   char       sbyte;         /*        -128 to +127        */
typedef signed   int16      sshort;        /*      -32768 to +32767      */
#endif
typedef signed   int32      slong;         /* -2147483648 to +2147483647 */

#if !defined ALI_FILE_VER && !defined SDTENV && !defined L1TEST
typedef signed char         S8;
typedef signed short        S16;
typedef signed long         S32;	//BRCM/matthewc 7/25/05 (GNATS TR1054)
typedef signed long long    S64;
/*
** Needed for ARM compatibility
*/
typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned long       U32;	//BRCM/matthewc 7/25/05 (GNATS TR1054)
typedef unsigned long long  U64;
# if defined (__arm)
typedef unsigned int        uint;
# endif
#endif /* ALI_FILE_VER, SDTENV*/

typedef int                 BOOL;               /* TRUE or FALSE */

#ifdef _Windows
   typedef unsigned long    ulong;
   typedef unsigned short   ushort;
   typedef unsigned char       UINT8;

#elif defined(__CYGWIN__)
   /* UNIX emulation on Windows host*/
   typedef unsigned long    ulong;
#else    
    #if !defined X_SCTTYPES_H || defined (_C166) || defined (__arm)
    /* A. Schoefer: ulong/ushort already defined in
                   #include <sys/types.h>
                   (see also scttypes.h for the SDT simulation)
    */
       #ifndef MACEHS_HOST
       typedef unsigned long    ulong;
       typedef unsigned short   ushort;
       #endif
    #endif
#endif

/*
** Definition of 64 bit data types.
*/
#if defined (WIN32) || defined (_Windows)
typedef __int64 ullong;
typedef __int64 sllong;
#define  LL(a) (a)
#define ULL(a) (a)
#elif defined (__arm) ||(defined (__GNUC__) && !defined (__STRICT_ANSI__))
typedef unsigned long long ullong;
typedef   signed long long sllong;
#define  LL(a) (a##LL)  /* eg. LL(0) := 0LL */
#define ULL(a) (a##ULL)
#else /* default, if no 64 bit data types supported */
typedef unsigned long    ulong;			//Added here Xiaoxin. 7/5/04
typedef unsigned short   ushort;
#define LONGLONG_EMUL 1
typedef struct 
{
  ulong hi;
  ulong lo;
} ullong;

typedef struct 
{
  slong hi;
  ulong lo;
} sllong;

#endif

#ifndef MS_UNUSED_VAR
#define MS_UNUSED_VAR(a)  {if(sizeof(a));}
#endif

#ifndef MACEHS_HOST
typedef unsigned long    ulong;			//moved here Xiaoxin. 7/5/04
typedef unsigned short   ushort;
#endif

#endif /* BASTYPES_H */


