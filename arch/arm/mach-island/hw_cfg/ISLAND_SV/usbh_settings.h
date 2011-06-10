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

#ifndef USBH_SETTINGS_H
#define USBH_SETTINGS_H

/*
 * Refer to mach/usbh.h for details
 */

#define HW_USBH_PARAM \
{ \
	.peri_clk_name = "usbh_48m_clk", \
	.ahb_clk_name = "usbh_ahb_clk", \
	.opt_clk_name = "usbh_12m_clk", \
	.num_ports = 2, \
	.port = { \
		[0] = { \
			.pwr_gpio = 1, \
			.pwr_flt_gpio = 5, \
			.reset_gpio = 179, \
		}, \
		[1] = { \
			.pwr_gpio = 0, \
			.pwr_flt_gpio = 4, \
			.reset_gpio = 176, \
		}, \
	}, \
}

#endif /* USBH_SETTINGS_H */
