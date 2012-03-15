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
*  @file    csx_af.h
*
*  @brief:  This file contains the API for the CSX AF interface
*           debug utility
*
*****************************************************************************/

#if !defined( CSX_AF_H )
#define CSX_AF_H

/* ---- Constants and Types ---------------------------------------------- */

typedef int CSX_AF_HDL;

#define CSX_AF_IO_POINT_NUM_MAX      80	/* Maximum number of CSX I/O points supported */

typedef struct csx_af_info {
	AF_SERVER_POINT_ID af_server_point;

} CSX_AF_INFO;

typedef struct csx_af_point {
	CSX_IO_POINT_INFO csx_io_point_info;
	CSX_AF_INFO csx_af_info;
	CSX_IO_HANDLE csx_io_point_handle;

} CSX_AF_POINT;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#if !defined( SWIG ) && !defined( MAKEDEFS )

#if defined( __KERNEL__ )
int csx_af_add_point(CSX_IO_POINT_INFO *csx_info,
		     CSX_AF_INFO *csx_af_info, CSX_IO_HANDLE *csx_handle);

int csx_af_remove_point(CSX_IO_HANDLE csx_handle);

unsigned int csx_af_get_num_active_points(void);

int csx_af_query_all(CSX_AF_POINT *csx_af_point, unsigned int *num_points);

int csx_af_sync_enable(CSX_IO_MODULE csx_io_module);
int csx_af_sync_disable(CSX_IO_MODULE csx_io_module);
#endif

/* User space client side API */
CSX_AF_HDL csx_af_client_allocate(void);

int csx_af_client_free(CSX_AF_HDL io_handle);

int csx_af_client_add_point(CSX_AF_HDL io_handle,
			    CSX_IO_POINT_INFO *csx_info,
			    CSX_AF_INFO *csx_af_info,
			    CSX_IO_HANDLE *csx_handle);

int csx_af_client_remove_point(CSX_AF_HDL io_handle, CSX_IO_HANDLE csx_handle);

int csx_af_client_get_num_points(CSX_AF_HDL io_handle,
				 unsigned int *num_points);

int csx_af_client_query_all(CSX_AF_HDL io_handle,
			    CSX_AF_POINT *csx_af_point,
			    unsigned int *num_points);

int csx_af_client_sync_enable(CSX_AF_HDL io_handle,
			      CSX_IO_MODULE csx_io_module);

int csx_af_client_sync_disable(CSX_AF_HDL io_handle,
			       CSX_IO_MODULE csx_io_module);

#endif /* !SWIG && !MAKEDEFS */

#endif /* CSX_AF_H */
