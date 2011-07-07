/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef VC_VCHI_FB_H
#define VC_VCHI_FB_H

//#include "vchost_config.h"
#include "interface/vchi/vchi.h"
#include "interface/vcos/vcos.h"
#include "vc_fb_defs.h"


typedef struct opaque_vc_vchi_fb_handle_t *VC_VCHI_FB_HANDLE_T;

VC_VCHI_FB_HANDLE_T vc_vchi_fb_init( VCHI_INSTANCE_T vchi_instance,
                                     VCHI_CONNECTION_T **vchi_connections,
                                     uint32_t num_connections );

int32_t vc_vchi_fb_stop( VC_VCHI_FB_HANDLE_T *handle );

int32_t vc_vchi_fb_get_scrn_info( VC_VCHI_FB_HANDLE_T handle,
                                  VC_FB_SCRN scrn,
                                  VC_FB_SCRN_INFO_T *info );

int32_t vc_vchi_fb_alloc( VC_VCHI_FB_HANDLE_T handle,
                          VC_FB_ALLOC_T *alloc,
                          VC_FB_ALLOC_RESULT_T *result );

int32_t vc_vchi_fb_free( VC_VCHI_FB_HANDLE_T handle,
                         uint32_t res_handle );

int32_t vc_vchi_fb_pan( VC_VCHI_FB_HANDLE_T handle,
                        uint32_t res_handle,
                        uint32_t y_offset );

int32_t vc_vchi_fb_swap( VC_VCHI_FB_HANDLE_T handle,
                         uint32_t res_handle,
                         uint32_t active_frame );

#endif // VC_VCHI_FB_H
