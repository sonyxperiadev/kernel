/*****************************************************************************
* Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
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
*  @file    csx.h
*
*  @brief:  This file contains the API for the CSX interface
*
*****************************************************************************/

#if !defined( CSX_H )
#define CSX_H

/* ---- Include Files ---------------------------------------- */

#if defined( __KERNEL__ )
#include <linux/types.h>                     /* Needed for standard types */
#else
#include <stdint.h>
#endif

/* ---- Constants and Types ---------------------------------- */

#if defined( __KERNEL__ )

/* MakeDefs: Off */

/* CSX I/O point function pointers */
typedef int (*CSX_IO_POINT_CB_FP)( char *buf, int len, void *data );
typedef int (*CSX_FRAME_SYNC_FP)( void *data );

typedef struct csx_io_point_fncs
{
   CSX_IO_POINT_CB_FP csxCallback;

} CSX_IO_POINT_FNCS;

/* MakeDefs: On */

#endif   /* __KERNEL__ */

#define CSX_POINT_ENABLE          1     /* Value associated with an enabled capture/inject point */
#define CSX_POINT_DISABLE         0	    /* Value associated with a disabled capture/inject point */
#define CSX_HANDLE_INVALID        0     /* Value associated with invalid handle */

#define CSX_IO_UTIL_GIST               0
#define CSX_IO_UTIL_AUDIO_FRAMEWORK    1
#define CSX_IO_UTIL_SIGMORPH           2
#define CSX_IO_UTIL_NUM_MAX            3

#define CSX_IO_MODULE_HALAUDIO   0
#define CSX_IO_MODULE_HAUSWARE   1
#define CSX_IO_MODULE_NUM_MAX    2

#define CSX_IO_MODE_NONE            0
#define CSX_IO_MODE_CAPTURE         1
#define CSX_IO_MODE_INJECT          2
#define CSX_IO_MODE_CAPTURE_INJECT  3

#define CSX_IO_SYNC_OFF             0
#define CSX_IO_SYNC_ON              1

typedef int             CSX_IO_UTIL;
typedef int             CSX_IO_MODULE;
typedef int             CSX_IO_MODE;
typedef uint32_t        CSX_IO_HANDLE;    /* Require a minimum 32-bit handle for CSX */

typedef struct csx_io_point_info
{
   CSX_IO_MODULE csx_module_id;     /* Identifies module location for I/O point */
   int csx_device_id;               /* Identifies device within module.  Information specific to module */
   int csx_point_id;                /* Identifies point within device.  Information specific to device */
   CSX_IO_MODE csx_io_mode;         /* I/O mode setting */
   int csx_io_sync;                 /* Boolean flag to indicate synchronized enable/disable operation */

} CSX_IO_POINT_INFO;

/*
 * ---- Variable Externs -------------------------------------
 * ---- Function Prototypes ----------------------------------
 */

#endif /* CSX_H */

