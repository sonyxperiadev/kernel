/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#if !defined( VC_OVL_H )
#define VC_OVL_H

#include <linux/ioctl.h>

struct vc_ovl_info
{
   unsigned int   in_width;      // Input original width
   unsigned int   in_height;     // Input original height
   unsigned int   format;        // Input format

   unsigned int   xaxis;         // Overlay x-coordinate
   unsigned int   yaxis;         // Overlay y-coordinate
   unsigned int   width;         // Overlay width
   unsigned int   height;        // Overlay height
   unsigned int   rotation;      // Overlay rotation
   unsigned int   h_flip;        // Overlay horizontal flip
   unsigned int   v_flip;        // Overlay vertical flip
   int            layer;         // Overlay layer
};

struct vc_ovl_handle
{
   int   handle;
};

struct vc_ovl_data
{
   struct vc_ovl_handle ovl_handle;
   struct vc_ovl_info ovl_info;
};

#define VC_OVL_IOC_MAGIC  'O'

typedef enum
{
   VC_OVL_CMD_NEW,
   VC_OVL_CMD_DEL,
   VC_OVL_CMD_GET,
   VC_OVL_CMD_SET,
   VC_OVL_CMD_CHG_SET,
   VC_OVL_CMD_CHG_CHK,
   VC_OVL_CMD_CHG_CLR,

} VC_OVL_CMD;

#define VC_OVL_IOC_OVL_NEW       _IOR( VC_OVL_IOC_MAGIC, VC_OVL_CMD_NEW, struct vc_ovl_handle )
#define VC_OVL_IOC_OVL_DEL       _IOR( VC_OVL_IOC_MAGIC, VC_OVL_CMD_DEL, struct vc_ovl_handle )

#define VC_OVL_IOC_OVL_GET       _IOR( VC_OVL_IOC_MAGIC, VC_OVL_CMD_GET, struct vc_ovl_data )
#define VC_OVL_IOC_OVL_SET       _IOR( VC_OVL_IOC_MAGIC, VC_OVL_CMD_SET, struct vc_ovl_data )

#define VC_OVL_IOC_OVL_CHG_SET   _IOR( VC_OVL_IOC_MAGIC, VC_OVL_CMD_CHG_SET, struct vc_ovl_handle )
#define VC_OVL_IOC_OVL_CHG_CHK   _IOR( VC_OVL_IOC_MAGIC, VC_OVL_CMD_CHG_CHK, struct vc_ovl_handle )
#define VC_OVL_IOC_OVL_CHG_CLR   _IOR( VC_OVL_IOC_MAGIC, VC_OVL_CMD_CHG_CLR, struct vc_ovl_handle )

#endif  /* VC_OVL_H */

