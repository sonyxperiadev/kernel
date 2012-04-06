/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
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

#ifndef _KONA_OTG_CP_H
#define _KONA_OTG_CP_H

/*
 * ID pin states.
 */
enum kona_otg_cp_id {
	KONA_OTG_CP_ID_UNKNOWN = 0,
	KONA_OTG_CP_ID_DEVICE_A,
	KONA_OTG_CP_ID_DEVICE_B,
};

/*
 * VBUS Voltage States.
 * The SESS_DETECT is gray zone between SESS_END and SESS_VALID, and usually
 * indicates a transition stage when VBUS is being enabled or disabled.
 */
enum kona_otg_cp_volt_state {
	KONA_OTG_CP_VOLT_UNKNOWN = 0,
	KONA_OTG_CP_VOLT_SESS_END,
	KONA_OTG_CP_VOLT_SESS_DETECT,
	KONA_OTG_CP_VOLT_SESS_VALID,
	KONA_OTG_CP_VOLT_VBUS_VALID,
};

struct kona_otg_cp_ops;

struct kona_otg_cp {
	const char *name;
	const struct kona_otg_cp_ops *ops;
};

/*
 * Callback functions to be registered by the lower level driver
 */
struct kona_otg_cp_ops {
	int (*state_poll) (struct kona_otg_cp *otg_cp);

	/* to turn on/off VBUS power */
	int (*vbus_drive) (struct kona_otg_cp *otg_cp, int on);
};

extern int kona_otg_cp_register(struct kona_otg_cp *cp);
extern int kona_otg_cp_unregister(struct kona_otg_cp *cp);
extern int kona_otg_cp_id_change(const struct kona_otg_cp *cp,
		enum kona_otg_cp_id id);
extern int kona_otg_cp_volt_change(const struct kona_otg_cp *cp,
		enum kona_otg_cp_volt_state volt_state);

#endif /* _KONA_OTG_CP_H */
