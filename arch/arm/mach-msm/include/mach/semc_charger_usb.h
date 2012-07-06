/* arch/arm/mach-msm/include/semc_charger_usb.h
 *
 * Copyright (c) 2008-2010, Code Aurora Forum. All rights reserved.
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * All source code in this file is licensed under the following license except
 * where indicated.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can find it at http://www.fsf.org
 */

#ifndef _SEMC_CHARGER_USB_H_
#define _SEMC_CHARGER_USB_H_

#include <mach/msm_hsusb.h>

#define SEMC_CHARGER_USB_NAME "hsusb_chg"
#define SEMC_CHARGER_AC_NAME "ac"

enum semc_charger_connect_status {
	SEMC_CHARGER_PC = 0x01,
	SEMC_CHARGER_WALL = 0x02,
	SEMC_CHARGER_CRADLE = 0x04,
};

void semc_chg_usb_set_supplicants(char **supplied_to, size_t num_supplicants);

int semc_charger_usb_init(int init);
void semc_charger_usb_vbus_draw(unsigned mA);
void semc_charger_usb_connected(enum chg_type chgtype);
unsigned int semc_charger_usb_current_ma(void);

/**
 * semc_charger_get_ac_online_status() return ac/online status.
 * bit-1 indicates Wall charger connection.
 * bit-2 indicates Cradle charger connection.
 * return 0x02: Wall charger is connected.
 *        0x04: Cradle charger is connected
 *        0x06: Wall and Cradle charger are connected
 *
 */
u8 semc_charger_get_ac_online_status(void);

#endif
