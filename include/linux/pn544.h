/*
 * Copyright (C) 2010 Trusted Logic S.A.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _PN544_H_
#define _PN544_H_

#define PN544_MAGIC 0xE9

/*
 * PN544 power control via ioctl
 * PN544_SET_PWR(0): power off
 * PN544_SET_PWR(1): power on
 * PN544_SET_PWR(>1): power on with firmware download enabled
 */
#define PN544_SET_PWR _IOW(PN544_MAGIC, 0x01, unsigned int)

#define PN544_DEVICE_NAME "pn544"

enum pn544_set_pwr_cmd {
	PN544_SET_PWR_OFF,
	PN544_SET_PWR_ON,
	PN544_SET_PWR_FWDL,
};

enum pn544_state {
	PN544_STATE_UNKNOWN,
	PN544_STATE_OFF,
	PN544_STATE_ON,
	PN544_STATE_FWDL,
};

struct pn544_i2c_platform_data {
	int irq_type;
	int (*chip_config)(enum pn544_state, void *);
	int (*driver_loaded)(void);
	void (*driver_unloaded)(void);
	int (*driver_opened)(void);
	void (*driver_closed)(void);
};

#endif

