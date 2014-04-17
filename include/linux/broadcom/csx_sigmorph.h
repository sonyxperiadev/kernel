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
 *  @file    csx_sigmorph.h
 *
 *  @brief:  This file contains the API for the CSX SIGMORPH interface
 *           debug utility
 *
 *****************************************************************************/

#if !defined( CSX_SIGMORPH_H )
#define CSX_SIGMORPH_H

/* ---- Constants and Types ---------------------------------------------- */

#define CSX_SIGMORPH_IO_POINT_NUM_MAX     (80)	/* Maximum number of CSX I/O 
						   points supported */
#define CSX_SIGMORPH_MIX_ENABLE           (1)

typedef int CSX_SIGMORPH_HDL;

/*
   capture_raw:   first capture point. Signal  is saved before any processing
   capture_procd: second capture point. Signal is saved after processing
   inject:        container needed for doing a MIX operation. If not set, 
                  only an AMPLIFY operation can be envoked.
*/
typedef struct csx_sigmorph_info {
	GIST_WRITER_PARAMS capture_raw;	/* capture before processing */
	GIST_WRITER_PARAMS capture_procd;	/* capture post processing */
	GIST_READER_PARAMS inject;	/* inject parameters  */
	int stream_gain;	/* Linear Q12 value */
	int inject_gain;	/* Linear Q12 value */
	int mix;		/* CSX_SIGMORPH_MIX_ENABLE, or 0 */

} CSX_SIGMORPH_INFO;

typedef struct csx_sigmorph_point {
	CSX_IO_POINT_INFO csx_io_point_info;
	CSX_IO_HANDLE csx_io_point_handle;
	CSX_SIGMORPH_INFO csx_sigmorph_info;

} CSX_SIGMORPH_POINT;

/* ---- Function Prototypes ---------------------------------------------- */

#if !defined( SWIG ) && !defined( MAKEDEFS )

#if defined( __KERNEL__ )
int csx_sigmorph_add_point(CSX_IO_POINT_INFO *csx_info,
			   CSX_SIGMORPH_INFO *csx_sigmorph_info,
			   CSX_IO_HANDLE *csx_handle);

int csx_sigmorph_remove_point(CSX_IO_HANDLE csx_handle);

unsigned int csx_sigmorph_get_num_active_points(void);

int csx_sigmorph_query_all(CSX_SIGMORPH_POINT *csx_sigmorph_point,
			   unsigned int *num_points);

int csx_sigmorph_sync_enable(CSX_IO_MODULE csx_io_module);

int csx_sigmorph_sync_disable(CSX_IO_MODULE csx_io_module);

#endif

/* User space client side API */
CSX_SIGMORPH_HDL csx_sigmorph_client_allocate(void);

int csx_sigmorph_client_free(CSX_SIGMORPH_HDL io_handle);

int csx_sigmorph_client_add_point(CSX_SIGMORPH_HDL io_handle,
				  CSX_IO_POINT_INFO *csx_info,
				  CSX_SIGMORPH_INFO *csx_sigmorph_info,
				  CSX_IO_HANDLE *csx_handle);

int csx_sigmorph_client_remove_point(CSX_SIGMORPH_HDL io_handle,
				     CSX_IO_HANDLE csx_handle);

int csx_sigmorph_client_get_num_points(CSX_SIGMORPH_HDL io_handle,
				       unsigned int *num_points);

int csx_sigmorph_client_query_all(CSX_SIGMORPH_HDL io_handle,
				  CSX_SIGMORPH_POINT *csx_sigmorph_point,
				  unsigned int *num_points);

int csx_sigmorph_client_sync_enable(CSX_SIGMORPH_HDL io_handle,
				    CSX_IO_MODULE csx_io_module);

int csx_sigmorph_client_sync_disable(CSX_SIGMORPH_HDL io_handle,
				     CSX_IO_MODULE csx_io_module);

#endif /* !SWIG && !MAKEDEFS */

#endif /* CSX_SIGMORPH_H */
