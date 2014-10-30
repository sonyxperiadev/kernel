/*
 * Copyright (C) 2010 Trusted Logic S.A.
 * Copyright (C) 2013 Sony Mobile Communications AB.
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

#ifndef _PN547_H_
#define _PN547_H_

#define PN547_DEVICE_NAME "pn547"

enum pn547_init_deinit_cmd {
	PN547_INIT,
	PN547_DEINIT,
};

enum pn547_set_pwr_cmd {
	PN547_SET_PWR_OFF,
	PN547_SET_PWR_ON,
	PN547_SET_PWR_FWDL,
};

enum pn547_state {
	PN547_STATE_UNKNOWN,
	PN547_STATE_OFF,
	PN547_STATE_ON,
	PN547_STATE_FWDL,
};

struct pn547_i2c_platform_data {
	int irq_gpio;
	int fwdl_en_gpio;
	int ven_gpio;
	int pvdd_en_gpio;
	int configure_gpio;
	int configure_mpp;
	bool dynamic_config;
};

#endif

