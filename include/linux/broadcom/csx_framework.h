/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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
*  @file    csx_framework.h
*
*  @brief   This files contains the interface for framework
*           components of the CSX interface.
*
****************************************************************************/

#if !defined( CSX_FRAMEWORK_H )
#define CSX_FRAMEWORK_H

#if defined( __KERNEL__ )

/* ---- Include Files ---------------------------------------------------- */
#include <linux/broadcom/csx.h>
#include <linux/broadcom/csx_util.h>

/* ---- Constants and Types ---------------------------------------------- */

/* CSX Module function pointers */
typedef int (*CSX_MODULE_SET_FRAME_SYNC) (CSX_FRAME_SYNC_FP frame_sync_fncp);

typedef int (*CSX_MODULE_SET_POINT) (int csx_device_id,
				     int csx_point_id,
				     CSX_IO_POINT_FNCS *csx_io_point_fncs,
				     void *csx_priv);

typedef struct csx_module_fncs {
	CSX_MODULE_SET_FRAME_SYNC csx_module_set_frame_sync;
	CSX_MODULE_SET_POINT csx_module_set_point;

} CSX_MODULE_FNCS;

/* CSX Utility function pointers */
typedef int (*CSX_UTIL_FRAME_SYNC_EVENT) (CSX_IO_MODULE csx_module_id);

typedef struct csx_util_fncs {
	CSX_UTIL_FRAME_SYNC_EVENT csx_util_frame_sync_event;

} CSX_UTIL_FNCS;

/* CSX Utility control interface function pointers */
typedef const char *(*CSX_UTIL_GET_NAME) (void);

typedef unsigned int (*CSX_UTIL_GET_MAX_POINTS) (void);

typedef unsigned int (*CSX_UTIL_SIZE_OF_INFO_STRUCT) (void);

typedef unsigned int (*CSX_UTIL_SIZE_OF_POINT_STRUCT) (void);

typedef int (*CSX_UTIL_ADD_POINT) (CSX_IO_POINT_INFO *csx_info,
				   CSX_UTIL_INFO *csx_util_info,
				   CSX_IO_HANDLE *csx_handle);

typedef int (*CSX_UTIL_REMOVE_POINT) (CSX_IO_HANDLE csx_handle);

typedef unsigned int (*CSX_UTIL_GET_NUM_ACTIVE_POINTS) (void);

typedef int (*CSX_UTIL_QUERY_ALL) (CSX_UTIL_POINT *csx_util_point,
				   unsigned int *num_points);

typedef int (*CSX_UTIL_SYNC_ENABLE) (CSX_IO_MODULE csx_io_module);

typedef int (*CSX_UTIL_SYNC_DISABLE) (CSX_IO_MODULE csx_io_module);

typedef struct csx_util_ctrl_fncs {
	CSX_UTIL_GET_NAME csx_util_get_name;
	CSX_UTIL_GET_MAX_POINTS csx_util_get_max_points;
	CSX_UTIL_SIZE_OF_INFO_STRUCT csx_util_size_of_info_struct;
	CSX_UTIL_SIZE_OF_POINT_STRUCT csx_util_size_of_point_struct;
	CSX_UTIL_ADD_POINT csx_util_add_point;
	CSX_UTIL_REMOVE_POINT csx_util_remove_point;
	CSX_UTIL_GET_NUM_ACTIVE_POINTS csx_util_get_num_active_points;
	CSX_UTIL_QUERY_ALL csx_util_query_all;
	CSX_UTIL_SYNC_ENABLE csx_util_sync_enable;
	CSX_UTIL_SYNC_DISABLE csx_util_sync_disable;

} CSX_UTIL_CTRL_FNCS;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

CSX_MODULE_FNCS *csx_get_module_fncp(CSX_IO_MODULE csx_module_id);

int csx_register_module(CSX_IO_MODULE csx_module_id,
			CSX_MODULE_FNCS *csx_module_fncp);
int csx_register_util(CSX_IO_UTIL csx_util_id, CSX_UTIL_FNCS *csx_util_fncp,
		      CSX_UTIL_CTRL_FNCS *csx_util_ctrl_fncp);

int csx_deregister_module(CSX_IO_MODULE csx_module_id);
int csx_deregister_util(CSX_IO_UTIL csx_util_id);

int csx_register_util_drv(CSX_IO_UTIL csx_util_id,
			  CSX_UTIL_CTRL_FNCS *csx_util_ctrl_fncp);
int csx_deregister_util_drv(CSX_IO_UTIL csx_util_id);

#endif /* __KERNEL__ */
#endif /* CSX_FRAMEWORK_H */
