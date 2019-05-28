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

struct somc_typec_mode_ctrl {
	spinlock_t		change_irq_lock;
	struct delayed_work	start_polling_delay;
	struct delayed_work	stop_polling_delay;
	struct workqueue_struct	*polling_wq;
	bool			user_request_polling;
	bool			avoid_first_usbid_change;
	struct wakeup_source	wakeup_source_id_polling;
};

struct usb_somc_params {
	struct iio_channel		*adc_usbdp_chan;
	struct iio_channel		*adc_usbdm_chan;
	struct somc_usb_ocp		ocp;
	struct somc_typec_mode_ctrl	typecctrl;
};

#endif /* __QPNP_SMBCHARGER_EXTENSION_USB */
