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

#ifdef CONFIG_MFD_BCMPMU
int bcm_otg_do_adp_calibration_probe(struct bcmpmu *bcmpmu);
int bcm_otg_do_adp_probe(struct bcmpmu *bcmpmu);
int bcm_otg_do_adp_sense(struct bcmpmu *bcmpmu);
int bcm_otg_do_adp_sense_then_probe(struct bcmpmu *bcmpmu);
int bcm_otg_adp_change_callback(struct bcmpmu *bcmpmu, void *cb_data);
#else
int bcm_otg_do_adp_calibration_probe(struct bcm590xx *bcm590xx);
int bcm_otg_do_adp_probe(struct bcm590xx *bcm590xx);
int bcm_otg_do_adp_sense(struct bcm590xx *bcm590xx);
int bcm_otg_do_adp_sense_then_probe(struct bcm590xx *bcm590xx);
int bcm_otg_adp_change_callback(struct bcm590xx *bcm590xx, void *cb_data);
#endif

#endif /* _BCM_OTG_ADP_H */
