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
	int (*probe)(struct platform_device *pdev);
	void (*remove)(void);
	int (*en_set)(int on);
	int (*in_set)(int on);
	int (*flg_get)(void);
	void (*notify_flg_int)(void);
	void (*check_pin_state)(void);
	int oc_delay_time;
};

int ncp373_vbus_switch(int on);

#endif
