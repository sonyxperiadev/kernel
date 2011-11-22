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

#if !defined( VC_OMX_H )
#define VC_OMX_H

#include <linux/ioctl.h>

#define OMX_PLUGIN__COMPONENT_SUPPORTED__PREFIX_LEN      20
#define OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_DECODER   "OMX.BRCM.vc4.decoder"
#define OMX_PLUGIN__COMPONENT_SUPPORTED__VIDEO_ENCODER   "OMX.BRCM.vc4.encoder"

#define OMX_PLUGIN__COMPONENT_SUPPORTED__MPEG4           "mpeg4"
#define OMX_PLUGIN__COMPONENT_SUPPORTED__H263            "h263"
#define OMX_PLUGIN__COMPONENT_SUPPORTED__AVC             "avc"
#define OMX_PLUGIN__COMPONENT_SUPPORTED__VP8             "vp8"

#define OMX_PLUGIN__COMPONENT_SUPPORTED__NUM            7

#define OMX_PLUGIN__MKNAME(a,b)                          a"."b

#define OMX_PLUGIN__COMPONENT_NAME_LEN                   128

#define OMX_PLUGIN__ENCODING__YUV420                     0
#define OMX_PLUGIN__ENCODING__YUVUV128                   1
#define OMX_PLUGIN__ENCODING__OPAQUE                     2

struct vc_omx_comp_status
{
   char           name[OMX_PLUGIN__COMPONENT_NAME_LEN + 1];
   unsigned int   enabled;
};

struct vc_omx_enc_color
{
   unsigned int   format;
};

struct vc_omx_and_ext
{
   unsigned int   nocreate;
   unsigned int   andext;
};

#define VC_OMX_IOC_MAGIC  'K'

#define VC_OMX_IOC_COMP_STATUS    _IOR( VC_OMX_IOC_MAGIC, 0, struct vc_omx_comp_status )
#define VC_OMX_IOC_ENC_COLOR      _IOR( VC_OMX_IOC_MAGIC, 1, struct vc_omx_enc_color )
#define VC_OMX_IOC_AND_EXT        _IOR( VC_OMX_IOC_MAGIC, 2, struct vc_omx_and_ext )

#endif  /* VC_OMX_H */

