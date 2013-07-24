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

#include <linux/wakelock.h>

#define T_NO_ADP_DELAY_MIN_IN_MS	5000
#define T_SRP_FAILURE_MAX_IN_MS 6000
#define T_SESS_END_SRP_START_IN_MS 1600
#define T_B_ADP_DETACH	3200

struct bcm_otg_xceiver {
	struct usb_phy phy;
	struct usb_otg otg;
	struct timer_list srp_failure_timer;
	struct timer_list sess_end_srp_timer;
	struct wake_lock xceiver_wake_lock;
	bool otg_vbus_off;
	bool otg_srp_reqd;
	/* For future expansion */
};

struct bcmpmu_otg_xceiv_data {
	struct device *dev;
	struct bcmpmu59xxx *bcmpmu;

	struct regulator *bcm_hsotg_regulator;
	struct bcm_otg_xceiver otg_xceiver;

	/* OTG Work queue and work struct for each item for work queue */
	struct workqueue_struct *bcm_otg_work_queue;
	struct work_struct bcm_otg_vbus_invalid_work;
	struct work_struct bcm_otg_vbus_valid_work;
	struct work_struct bcm_otg_vbus_a_invalid_work;
	struct work_struct bcm_otg_vbus_a_valid_work;
	struct work_struct bcm_otg_adp_cprb_done_work;
	struct work_struct bcm_otg_adp_change_work;
	struct work_struct bcm_otg_id_status_change_work;
	struct work_struct bcm_otg_chg_detect_work;
	struct work_struct bcm_otg_sess_end_srp_work;
	struct work_struct bcm_otg_sens_end_work;
	struct delayed_work bcm_otg_delayed_adp_work;

	/* OTG notifier blocks for each event */
	struct notifier_block bcm_otg_id_chg_notifier;
	struct notifier_block bcm_otg_vbus_validity_notifier;
	struct notifier_block bcm_otg_session_invalid_notifier;
	struct notifier_block bcm_otg_vbus_a_invalid_notifier;
	struct notifier_block bcm_otg_chg_detection_notifier;
	struct notifier_block bcm_otg_adp_calib_done_notifier;
	struct notifier_block bcm_otg_adp_change_done_notifier;
	struct notifier_block bcm_otg_adp_sns_end_notifier;
	struct notifier_block bcm_otg_xceive_state_notifier;

	bool host;
	bool vbus_enabled;
	bool otg_enabled;
	bool regulator_enabled;
};

bool bcmpmu_otg_xceiv_check_id_gnd(struct bcmpmu_otg_xceiv_data
	*xceiv_data);
bool bcmpmu_otg_xceiv_check_id_rid_a(struct bcmpmu_otg_xceiv_data
	*xceiv_data);
bool bcmpmu_otg_xceiv_check_id_rid_b(struct bcmpmu_otg_xceiv_data
	*xceiv_data);
bool bcmpmu_otg_xceiv_check_id_rid_c(struct bcmpmu_otg_xceiv_data
	*xceiv_data);
void bcmpmu_otg_xceiv_do_srp(struct bcmpmu_otg_xceiv_data
	*xceiv_data);
#endif /* _BCMPMU_OTG_XCEIV_H */
