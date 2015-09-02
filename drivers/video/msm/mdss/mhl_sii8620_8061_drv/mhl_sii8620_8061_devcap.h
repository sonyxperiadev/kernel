/* kernel/drivers/video/msm/mdss/mhl_sii8620_8061_drv/mhl_sii8620_8061_devcap.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 * Copyright (C) 2013 Silicon Image Inc.
 *
 * Author: [Yasuyuki Kino <yasuyuki.kino@sonymobile.com>]
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __MHL_SII8620_8061_DEVCAP_H__
#define __MHL_SII8620_8061_DEVCAP_H__

#include "mhl_defs.h"

enum tdm_vc_assignments {
	TDM_VC_CBUS1	= 0,
	TDM_VC_E_MSC	= 1,
	TDM_VC_T_CBUS	= 2,
	TDM_VC_MAX		= TDM_VC_T_CBUS + 1
};

/* Device Capability Registers */

#define DEVCAP_VAL_DEV_STATE		0
#define DEVCAP_VAL_MHL_VERSION		MHL_VERSION
#define DEVCAP_VAL_DEV_CAT			MHL_DEV_CAT_SOURCE
#define DEVCAP_VAL_ADOPTER_ID_H		0x03
#define DEVCAP_VAL_ADOPTER_ID_L		0xA7
#define DEVCAP_VAL_VID_LINK_MODE	(MHL_DEV_VID_LINK_SUPP_16BPP \
					| MHL_DEV_VID_LINK_SUPP_VGA\
					| MHL_DEV_VID_LINK_SUPP_ISLANDS \
					| MHL_DEV_VID_LINK_SUPPYCBCR422 \
					| MHL_DEV_VID_LINK_SUPPRGB444)
#define DEVCAP_VAL_AUD_LINK_MODE	(MHL_DEV_AUD_LINK_2CH \
					| MHL_DEV_AUD_LINK_8CH)
#define DEVCAP_VAL_VIDEO_TYPE		0
#define DEVCAP_VAL_LOG_DEV_MAP		MHL_DEV_LD_GUI
#define DEVCAP_VAL_BANDWIDTH		0x00
#define DEVCAP_VAL_FEATURE_FLAG		(MHL_FEATURE_RCP_SUPPORT \
					| MHL_FEATURE_RAP_SUPPORT	\
					| MHL_FEATURE_SP_SUPPORT)
#define DEVCAP_VAL_DEVICE_ID_H		0x04
#define DEVCAP_VAL_DEVICE_ID_L		0x05
#define DEVCAP_VAL_SCRATCHPAD_SIZE	MHL_SCRATCHPAD_SIZE
#define DEVCAP_VAL_INT_STAT_SIZE	MHL_INT_AND_STATUS_SIZE
#define DEVCAP_VAL_RESERVED		0

/* Extended Device Capability Registers */

#define XDEVCAP_VAL_ECBUS_SPEEDS	(MHL_XDC_ECBUS_S_075 \
					| MHL_XDC_ECBUS_S_8BIT \
					)

#define XDEVCAP_VAL_TMDS_SPEEDS		(MHL_XDC_TMDS_150 \
					| MHL_XDC_TMDS_300 \
					| MHL_XDC_TMDS_600 \
					)

#define XDEVCAP_VAL_DEV_ROLES		MHL_XDC_DEV_HOST
#define XDEVCAP_VAL_LOG_DEV_MAPX	0
#define XDEVCAP_VAL_RESERVE			0

#endif /*__MHL_SII8620_8061_DEVCAP_H__ */
