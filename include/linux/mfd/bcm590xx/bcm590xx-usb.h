/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   bcm59055-usb.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
* bcm59055-usb.h
*
* PURPOSE:
*
*
*
* NOTES:
*
* ****************************************************************************/

#ifndef __BCM59055_USB_H__
#define __BCM59055_USB_H__

#include <linux/notifier.h>

enum bcmpmu_usb_ctrl_t {
	BCMPMU_CTRL_VBUS_ON_OFF,
	BCMPMU_CTRL_SET_VBUS_DEB_TIME,
	BCMPMU_CTRL_SRP_VBUS_PULSE,
	BCMPMU_CTRL_DISCHRG_VBUS,

	BCMPMU_CTRL_START_STOP_ADP_SENS_PRB,
	BCMPMU_CTRL_START_STOP_ADP_PRB,
	BCMPMU_CTRL_START_ADP_CAL_PRB,
	BCMPMU_CTRL_SET_ADP_PRB_MOD,
	BCMPMU_CTRL_SET_ADP_PRB_CYC_TIME,
	BCMPMU_CTRL_SET_ADP_COMP_METHOD,
	BCMPMU_CTRL_GET_ADP_CHANGE_STATUS,
	BCMPMU_CTRL_GET_ADP_SENSE_STATUS,
	BCMPMU_CTRL_GET_ADP_PRB_RISE_TIMES,

	BCMPMU_CTRL_GET_VBUS_STATUS,
	BCMPMU_CTRL_GET_SESSION_STATUS,
	BCMPMU_CTRL_GET_SESSION_END_STATUS,
	BCMPMU_CTRL_GET_ID_VALUE,
	BCMPMU_CTRL_GET_CHARGER_TYPE
};



enum bcmpmu_event_id_t {
	BCMPMU_USB_EVENT_CHGR_DETECTION,
	BCMPMU_USB_EVENT_IN_RM,
	BCMPMU_USB_EVENT_ADP_CHANGE,
	BCMPMU_USB_EVENT_ADP_SENSE_END,
	BCMPMU_USB_EVENT_ADP_CALIBRATION_DONE,
	BCMPMU_USB_EVENT_ID_CHANGE,
	BCMPMU_USB_EVENT_VBUS_VALID,
	BCMPMU_USB_EVENT_VBUS_INVALID,
	BCMPMU_USB_EVENT_SESSION_VALID,
	BCMPMU_USB_EVENT_SESSION_INVALID,
	BCMPMU_USB_EVENT_SESSION_END_INVALID,
	BCMPMU_USB_EVENT_SESSION_END_VALID,
	BCMPMU_USB_EVENT_VBUS_OVERCURRENT,
	BCMPMU_USB_EVENT_RIC_C_TO_FLOAT,
	BCMPMU_EVENT_MAX
};


enum bcmpmu_otg_id_enc_t {
	A_DEVICE = 0,
	B_DEVICE,
	RESERVED1,
	RID_A_CHG,
	RID_B_CHG,
	RID_C_CHG,
	RESERVED2,
	FLOAT_B_DEVICE
};

enum bcmpmu_charger_typ_t {
	USB_CHARGER_UNKNOWN = 0,
	USB_CHARGER_SDP,
	USB_CHARGER_CDP,
	USB_CHARGER_DCP
};


/* Interface functions */
extern int bcmpmu_usb_add_notifier(u32 event_id, struct notifier_block *notifier);
extern int bcmpmu_usb_remove_notifier(u32 event_id, struct notifier_block *notifier);
extern int bcmpmu_usb_get(int command, void *ptr);
extern int bcmpmu_usb_set(int command, int data, void *ptr);


#endif /* __BCM59055_USB_H__ */
