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

#ifndef __ISLAND_USBH_CFG_H
#define __ISLAND_USBH_CFG_H

#define USBH_NUM_PORTS 3

struct usbh_port_cfg {
	/* GPIO for host power */
	int pwr_gpio;

	/* GPIO for power fault (over-current) detection */
	int pwr_flt_gpio;

	/* GPIO for PHY reset */
	int reset_gpio;
};

/*
 * USB Host related HW parameters
 */
struct usbh_cfg {
	/* peripheral clock name string */
	char *peri_clk_name;

	/* AHB bus clock name string */
	char *ahb_clk_name;

	/* the block might require optional clock to be enabled */
	char *opt_clk_name;

	unsigned int num_ports;

	struct usbh_port_cfg port[USBH_NUM_PORTS];
};

#endif
