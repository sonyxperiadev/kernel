/*****************************************************************************
* Copyright 2006 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef _BCMPMU_OTG_XCEIV_H
#define _BCMPMU_OTG_XCEIV_H

struct bcm_otg_xceiver {
	struct otg_transceiver xceiver;
	/* ADP functions and associated data structures */
	int (*do_adp_calibration_probe)(struct bcm590xx *bcm590xx);
	int (*do_adp_probe)(struct bcm590xx *bcm590xx);
	int (*do_adp_sense)(struct bcm590xx *bcm590xx);
	int (*do_adp_sense_then_probe)(struct bcm590xx *bcm590xx);	
};

struct bcm_otg_data {
	struct device *dev;
#ifdef CONFIG_MFD_BCMPMU
	struct bcmpmu *bcmpmu;
#else
	struct bcm590xx *bcm590xx;
#endif
	struct clk *otg_clk;
	struct bcm_otg_xceiver otg_xceiver;
	bool host;
	bool vbus_enabled;
};

#endif /* _BCMPMU_OTG_XCEIV_H */
