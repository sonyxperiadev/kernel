/* arch/arm/mach-msm/board-sony_shinano-wifi.h
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __ARCH_ARM_MACH_MSM_BOARD_SONY_SHINANO_WIFI_H
#define __ARCH_ARM_MACH_MSM_BOARD_SONY_SHINANO_WIFI_H

extern int shinano_wifi_status_register(
	void (*callback)(int card_present, void *dev_id), void *dev_id);
extern unsigned int shinano_wifi_status(struct device *dev);

#endif
