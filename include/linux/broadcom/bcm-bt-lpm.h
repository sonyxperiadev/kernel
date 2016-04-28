/******************************************************************************
* Copyright (C) 2013 Broadcom Corporation
*
* @file   /kernel/include/linux/broadcom/bcm-bt-rfkill.h
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation version 2.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
******************************************************************************/

/*
 * Broadcom bluetooth low power mode driver platform data definition.
 */

#ifndef BCM_BT_LPM_SETTINGS_H
#define BCM_BT_LPM_SETTINGS_H

struct bcm_bt_lpm_platform_data {
	int bt_wake_gpio;
	int host_wake_gpio;
};

#endif /* BCM_BT_LPM_SETTINGS_H */
