/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#ifndef CAPH_SETTINGS_H
#define CAPH_SETTINGS_H

#if defined(CONFIG_MACH_JAVA_C_5606)
#define HW_CFG_CAPH \
{ \
	.aud_ctrl_plat_cfg = \
	{ \
		.ext_aud_plat_cfg = \
		{ \
			.ihf_ext_amp_gpio = 34, \
			.dock_aud_route_gpio = -1, \
		} \
	} \
};
#elif defined(CONFIG_MACH_JAVA_C_LC1)
#define HW_CFG_CAPH \
{ \
	.aud_ctrl_plat_cfg = \
	{ \
		.ext_aud_plat_cfg = \
		{ \
			.ihf_ext_amp_gpio = 34, \
			.dock_aud_route_gpio = -1, \
		} \
	} \
};
#else
#define HW_CFG_CAPH \
{ \
	.aud_ctrl_plat_cfg = \
	{ \
		.ext_aud_plat_cfg = \
		{ \
			.ihf_ext_amp_gpio = 95, \
			.dock_aud_route_gpio = -1, \
		} \
	} \
};
#endif

#endif /* CAPH_SETTINGS_H */
