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

#if !defined( VC_DNFO_H )
#define VC_DNFO_H

#include <linux/ioctl.h>

struct vc_dnfo_display_info
{
   unsigned int   width;      /* Base width */
   unsigned int   height;     /* Base height */

   unsigned int   scale;      /* Whether to scale or not the base dimension */
   unsigned int   swidth;     /* Scaled width */
   unsigned int   sheight;    /* Scaled height */

   unsigned int   bpp;        /* BPP for display */
   unsigned int   layer;      /* Layer (as in VC layer) to put the display on */

   unsigned int   xdpi;       /* x-axis dpi */
   unsigned int   ydpi;       /* y-axis dpi */
};

#define VC_DNFO_IOC_MAGIC  'I'

#define VC_DNFO_IOC_DISPLAY_INFO    _IOR( VC_DNFO_IOC_MAGIC, 0, struct vc_dnfo_display_info )

#endif  /* VC_DNFO_H */

