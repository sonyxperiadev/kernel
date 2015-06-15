 /* include/linux/usb/host_ext_event.h
 *
 * USB host event handling function
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2013 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */


#ifndef __HOST_EXT_EVENT_H__
#define __HOST_EXT_EVENT_H__

enum usb_host_ext_event {
	USB_HOST_EXT_EVENT_NONE = -1,
	USB_HOST_EXT_EVENT_VBUS_DROP,
	USB_HOST_EXT_EVENT_INSUFFICIENT_POWER,
};

extern int host_send_uevent(enum usb_host_ext_event event);

#endif
