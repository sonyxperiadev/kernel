/**
 * dwc3_otg.h - DesignWare USB3 DRD Controller OTG
 *
 * Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LINUX_USB_DWC3_OTG_H
#define __LINUX_USB_DWC3_OTG_H

#include <linux/workqueue.h>
#include <linux/power_supply.h>

#include <linux/usb/otg.h>
#include "power.h"

#ifdef CONFIG_SONY_USB_EXTENSIONS
#include <linux/regulator/consumer.h>
#include <linux/wakelock.h>
#include <linux/notifier.h>

#define DWC3_1000MA_CHG_MAX 1000
#define DWC3_500MA_CHG_MAX 500
#endif

#define DWC3_IDEV_CHG_MAX 1500
#define DWC3_HVDCP_CHG_MAX 1800

/*
 * Module param to override current drawn for DCP charger
 * Declared in dwc3-msm module
 */
extern int dcp_max_current;

struct dwc3_charger;

/**
 * struct dwc3_otg: OTG driver data. Shared by HCD and DCD.
 * @otg: USB OTG Transceiver structure.
 * @regs: ioremapped register base address.
 * @sm_work: OTG state machine work.
 * @charger: DWC3 external charger detector
 * @inputs: OTG state machine inputs
 */
struct dwc3_otg {
	struct usb_otg		otg;
	struct dwc3		*dwc;
	void __iomem		*regs;
	struct regulator	*vbus_otg;
	int			cpe_gpio;
	struct delayed_work	sm_work;
	struct dwc3_charger	*charger;
	struct dwc3_ext_xceiv	*ext_xceiv;
#define ID		 0
#define B_SESS_VLD	 1
#define B_SUSPEND	2
	unsigned long inputs;
	struct power_supply	*psy;
	struct completion	dwc3_xcvr_vbus_init;
	int			charger_retry_count;
	int			vbus_retry_count;
#ifdef CONFIG_SONY_USB_EXTENSIONS
 #define A_VBUS_DROP_DET 3
	struct wake_lock	host_wakelock;
	struct notifier_block	usbdev_nb;
	int			device_count;
	struct delayed_work	no_device_work;
	unsigned int		retry_count;
	bool			no_device_timeout_enabled;
#endif
};

/**
 * USB charger types
 *
 * DWC3_INVALID_CHARGER	Invalid USB charger.
 * DWC3_SDP_CHARGER	Standard downstream port. Refers to a downstream port
 *                      on USB compliant host/hub.
 * DWC3_DCP_CHARGER	Dedicated charger port (AC charger/ Wall charger).
 * DWC3_CDP_CHARGER	Charging downstream port. Enumeration can happen and
 *                      IDEV_CHG_MAX can be drawn irrespective of USB state.
 * DWC3_PROPRIETARY_CHARGER A proprietary charger pull DP and DM to specific
 *                     voltages between 2.0-3.3v for identification.
 * DWC3_FLOATED_CHARGER Non standard charger whose data lines are floating.
 */
enum dwc3_chg_type {
	DWC3_INVALID_CHARGER = 0,
	DWC3_SDP_CHARGER,
	DWC3_DCP_CHARGER,
	DWC3_CDP_CHARGER,
	DWC3_PROPRIETARY_CHARGER,
#ifdef CONFIG_SONY_USB_EXTENSIONS
	DWC3_PROPRIETARY_1000MA,
	DWC3_PROPRIETARY_500MA,
#endif
	DWC3_FLOATED_CHARGER,
};

struct dwc3_charger {
	enum dwc3_chg_type	chg_type;
	unsigned		max_power;
	bool			charging_disabled;

	bool			skip_chg_detect;

	/* start/stop charger detection, provided by external charger module */
	void	(*start_detection)(struct dwc3_charger *charger, bool start);

	/* to notify OTG about charger detection completion, provided by OTG */
	void	(*notify_detection_complete)(struct usb_otg *otg,
						struct dwc3_charger *charger);
};

/* for external charger driver */
extern int dwc3_set_charger(struct usb_otg *otg, struct dwc3_charger *charger);

enum dwc3_id_state {
	DWC3_ID_GROUND = 0,
	DWC3_ID_FLOAT,
};

/* external transceiver that can perform connect/disconnect monitoring in LPM */
struct dwc3_ext_xceiv {
	enum dwc3_id_state	id;
#ifdef CONFIG_SONY_USB_EXTENSIONS
	bool			ocp;
#endif
	bool			bsv;
	bool			suspend;

	/* to notify OTG about LPM exit event, provided by OTG */
	void	(*notify_ext_events)(struct usb_otg *otg);
	/* for block reset USB core */
	void	(*ext_block_reset)(struct dwc3_ext_xceiv *ext_xceiv,
					bool core_reset);
#ifdef CONFIG_SONY_USB_EXTENSIONS
	/* to register ocp_notification */
	struct regulator_ocp_notification ext_ocp_notification;
#endif
};

/* for external transceiver driver */
extern int dwc3_set_ext_xceiv(struct usb_otg *otg,
				struct dwc3_ext_xceiv *ext_xceiv);
#endif /* __LINUX_USB_DWC3_OTG_H */
