/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __QPNP_SMBCHARGER_EXTENSION_USB
#define __QPNP_SMBCHARGER_EXTENSION_USB

struct somc_usb_ocp {
	struct regulator_ocp_notification notification;
	spinlock_t		lock;
};

#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
#ifdef CONFIG_FB
struct notifier_block;
#endif

struct usb_id_poll {
	bool				enable;
	bool				start;
	struct completion		cancel_poll;
	bool				otg_present;
	spinlock_t			lock;
	struct delayed_work		dw;
	struct workqueue_struct	*q;
	unsigned int			up_interval;
	unsigned int			up_period;
	int				pd_gpio;
	unsigned int			pd_invert;
#ifdef CONFIG_FB
	struct notifier_block		fb_notif;
#endif
	unsigned int			lcd_blanked;
};
#endif

struct usb_somc_params {
	struct qpnp_vadc_chip		*vadc_usb_dp;
	struct qpnp_vadc_chip		*vadc_usb_dm;
	struct somc_usb_ocp		ocp;
#ifdef CONFIG_QPNP_SMBCHARGER_ID_POLL
	struct usb_id_poll		id_poll;
#endif
};

#endif /* __QPNP_SMBCHARGER_EXTENSION_USB */
