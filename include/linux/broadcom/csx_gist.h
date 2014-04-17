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
*  @file    csx_gist.h
*
*  @brief:  This file contains the API for the CSX GIST interface
*           debug utility
*
*****************************************************************************/

#if !defined( CSX_GIST_H )
#define CSX_GIST_H

/* ---- Constants and Types ---------------------------------------------- */

typedef int CSX_GIST_HDL;

#define CSX_GIST_IO_POINT_NUM_MAX      80	/* Maximum number of CSX I/O points supported */

typedef struct csx_gist_info {
	GIST_WRITER_PARAMS capture;
	GIST_READER_PARAMS inject;

} CSX_GIST_INFO;

typedef struct csx_gist_point {
	CSX_IO_POINT_INFO csx_io_point_info;
	CSX_GIST_INFO csx_gist_info;
	CSX_IO_HANDLE csx_io_point_handle;

} CSX_GIST_POINT;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#if !defined( SWIG ) && !defined( MAKEDEFS )

#if defined( __KERNEL__ )
int csx_gist_add_point(CSX_IO_POINT_INFO *csx_info,
		       CSX_GIST_INFO *csx_gist_info, CSX_IO_HANDLE *csx_handle);

int csx_gist_remove_point(CSX_IO_HANDLE csx_handle);

unsigned int csx_gist_get_num_active_points(void);

int csx_gist_query_all(CSX_GIST_POINT *csx_gist_point,
		       unsigned int *num_points);

int csx_gist_sync_enable(CSX_IO_MODULE csx_io_module);

int csx_gist_sync_disable(CSX_IO_MODULE csx_io_module);
#endif

/* User space client side API */
CSX_GIST_HDL csx_gist_client_allocate(void);

int csx_gist_client_free(CSX_GIST_HDL io_handle);

int csx_gist_client_add_point(CSX_GIST_HDL io_handle,
			      CSX_IO_POINT_INFO *csx_info,
			      CSX_GIST_INFO *csx_gist_info,
			      CSX_IO_HANDLE *csx_handle);

int csx_gist_client_remove_point(CSX_GIST_HDL io_handle,
				 CSX_IO_HANDLE csx_handle);

int csx_gist_client_get_num_points(CSX_GIST_HDL io_handle,
				   unsigned int *num_points);

int csx_gist_client_query_all(CSX_GIST_HDL io_handle,
			      CSX_GIST_POINT *csx_gist_point,
			      unsigned int *num_points);

int csx_gist_client_sync_enable(CSX_GIST_HDL io_handle,
				CSX_IO_MODULE csx_io_module);

int csx_gist_client_sync_disable(CSX_GIST_HDL io_handle,
				 CSX_IO_MODULE csx_io_module);

#endif /* !SWIG && !MAKEDEFS */

#endif /* CSX_GIST_H */
