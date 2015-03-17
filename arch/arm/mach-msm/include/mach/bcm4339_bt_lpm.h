/*
 * Bluetooth Broadcomm and low power control via GPIO
 *
 *  Copyright (C) 2011 Samsung, Inc.
 *  Copyright (C) 2011 Google, Inc.
 *  Copyright (C) 2013 Sony Mobile Communications AB.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __ASM_ARCH_BCM4339_BT_LPM_H
#define __ASM_ARCH_BCM4339_BT_LPM_H

#include <linux/serial_core.h>

extern void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport);

#endif /* __ASM_ARCH_BCM4339_BT_LPM_H  */
