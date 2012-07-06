/*
 * Platform data for Android USB
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef	__LINUX_USB_ANDROID_H
#define	__LINUX_USB_ANDROID_H

struct android_usb_platform_data {
	int (*update_pid_and_serial_num)(uint32_t, const char *);
	char can_stall;
};

#ifdef CONFIG_USB_ANDROID_GG
void android_enable_usb_gg(uint16_t vendor_id, uint16_t product_id);
#endif

#endif	/* __LINUX_USB_ANDROID_H */
