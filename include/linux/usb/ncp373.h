/* include/linux/usb/ncp373.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _NCP373_H
#define _NCP373_H

#define NCP373_DRIVER_NAME	"ncp373"

struct ncp373_platform_data {
	int (*platform_init)(struct device *dev);
	void (*platform_release)(struct device *dev);
	int (*platform_chip_power)(struct device *dev, int on);
	int (*platform_vbus_power)(struct device *dev, int on);
	int (*platform_get_flg)(struct device *dev);
	void (*notify_flg_int)(struct device *dev);
	int oc_delay_time;
};

int ncp373_vbus_switch(int on);

#endif
