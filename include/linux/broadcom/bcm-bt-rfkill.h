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
 * Broadcom Bluetooth rfkill power control via GPIO. This file is
 * board dependent!
 */

#include <linux/rfkill.h>

#ifndef BCM_BT_RFKILL_SETTINGS_H
#define BCM_BT_RFKILL_SETTINGS_H

/* Constants and Types */
#define BCM_BT_RFKILL_VREG_ON           1
#define BCM_BT_RFKILL_VREG_OFF          0
#define BCM_BT_RFKILL_N_RESET_ON        0       /* keep in reset */
#define BCM_BT_RFKILL_N_RESET_OFF       1
#define BCM_BT_RFKILL_UNUSED_GPIO       (-1)

struct bcm_bt_rfkill_platform_data {
	int bcm_bt_rfkill_vreg_gpio;
	int bcm_bt_rfkill_n_reset_gpio;
	struct rfkill *rfkill;
};

#endif /* BCM_BT_RFKILL_SETTINGS_H */
