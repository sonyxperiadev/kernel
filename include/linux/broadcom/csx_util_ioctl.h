/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
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
*  @file    csx_util_ioctl.h
*
*  @brief   CSX UTIL User IOCTL API definitions
*
*****************************************************************************/
#if !defined( CSX_UTIL_IOCTL_H )
#define CSX_UTIL_IOCTL_H

/* ---- Include Files ---------------------------------------------------- */

#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#include <linux/ioctl.h>
#include <linux/broadcom/csx.h>
#include <linux/broadcom/csx_util.h>

/* ---- Constants and Types ---------------------------------------------- */

/* Type define used to create unique IOCTL number */
#define CSX_UTIL_MAGIC_TYPE                  'S'

/* IOCTL commands */
enum csx_util_cmd_e {
	CSX_UTIL_CMD_ADD_POINT = 0x20,	/* Start at 0x20 arbitrarily */
	CSX_UTIL_CMD_REMOVE_POINT,
	CSX_UTIL_CMD_GET_NUM_POINTS,
	CSX_UTIL_CMD_QUERY_ALL,
	CSX_UTIL_CMD_SYNC_ENABLE,
	CSX_UTIL_CMD_SYNC_DISABLE,
	CSX_UTIL_CMD_LAST	/* Do no delete */
};

/* IOCTL Data structures */
typedef struct csx_util_ioctl_add_point_parms {
	CSX_IO_POINT_INFO *csx_info;
	CSX_UTIL_INFO *csx_util_info;
	CSX_IO_HANDLE *csx_handle;

} CSX_UTIL_IOCTL_ADD_POINT_PARMS;

typedef struct csx_util_ioctl_remove_point_parms {
	CSX_IO_HANDLE csx_handle;

} CSX_UTIL_IOCTL_REMOVE_POINT_PARMS;

typedef struct csx_util_ioctl_get_num_points_parms {
	unsigned int *csx_num_points;

} CSX_UTIL_IOCTL_GET_NUM_POINTS_PARMS;

typedef struct csx_util_ioctl_query_all_parms {
	CSX_UTIL_POINT *csx_util_point;
	unsigned int *num_points;

} CSX_UTIL_IOCTL_QUERY_ALL_PARMS;

typedef struct csx_util_sync_enable_parms {
	CSX_IO_MODULE csx_io_module;

} CSX_UTIL_IOCTL_SYNC_ENABLE_PARMS;

typedef struct csx_util_sync_disable_parms {
	CSX_IO_MODULE csx_io_module;

} CSX_UTIL_IOCTL_SYNC_DISABLE_PARMS;

/* IOCTL numbers */
#define CSX_UTIL_IOCTL_ADD_POINT       _IOR( CSX_UTIL_MAGIC_TYPE, CSX_UTIL_CMD_ADD_POINT, CSX_UTIL_IOCTL_ADD_POINT_PARMS )
#define CSX_UTIL_IOCTL_REMOVE_POINT    _IOR( CSX_UTIL_MAGIC_TYPE, CSX_UTIL_CMD_REMOVE_POINT, CSX_UTIL_IOCTL_REMOVE_POINT_PARMS )
#define CSX_UTIL_IOCTL_GET_NUM_POINTS  _IOR( CSX_UTIL_MAGIC_TYPE, CSX_UTIL_CMD_GET_NUM_POINTS, CSX_UTIL_IOCTL_GET_NUM_POINTS_PARMS )
#define CSX_UTIL_IOCTL_QUERY_ALL       _IOR( CSX_UTIL_MAGIC_TYPE, CSX_UTIL_CMD_QUERY_ALL, CSX_UTIL_IOCTL_QUERY_ALL_PARMS )
#define CSX_UTIL_IOCTL_SYNC_ENABLE     _IOR( CSX_UTIL_MAGIC_TYPE, CSX_UTIL_CMD_SYNC_ENABLE, CSX_UTIL_IOCTL_SYNC_ENABLE_PARMS )
#define CSX_UTIL_IOCTL_SYNC_DISABLE    _IOR( CSX_UTIL_MAGIC_TYPE, CSX_UTIL_CMD_SYNC_DISABLE, CSX_UTIL_IOCTL_SYNC_DISABLE_PARMS )

/* ---- Variable Externs ------------------------------------------ */
/* ---- Function Prototypes --------------------------------------- */

#endif /* CSX_UTIL_IOCTL_H */
