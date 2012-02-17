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

#ifndef _BCM_OTG_ADP_H
#define _BCM_OTG_ADP_H

int bcm_otg_do_adp_calibration_probe(struct bcmpmu_otg_xceiv_data *xceiv_data);
int bcm_otg_do_adp_probe(struct bcmpmu_otg_xceiv_data *xceiv_data);
int bcm_otg_do_adp_sense(struct bcmpmu_otg_xceiv_data *xceiv_data);
int bcm_otg_adp_change_callback(struct bcmpmu_otg_xceiv_data *xceiv_data, void *cb_data);
int bcm_otg_adp_init(struct bcmpmu_otg_xceiv_data *xceiv_data);

#endif /* _BCM_OTG_ADP_H */
