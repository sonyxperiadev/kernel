/*
 * Device driver for monitoring ambient light intensity in (lux), RGB, and
 * color temperature (in kelvin) within the AMS-TAOS TCS family of devices.
 *
 * Copyright (c) 2016, AMS-TAOS USA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __TCS3490_H
#define __TCS3490_H

#include <linux/types.h>

/* Max number of segments allowable in LUX table */
#define TCS3490_MAX_LUX_TABLE_SIZE		9
#define MAX_DEFAULT_TABLE_BYTES (sizeof(int) * TCS3490_MAX_LUX_TABLE_SIZE)

/* Default LUX and Color coefficients */

#define D_Factor	304
#define R_Coef		205
#define G_Coef		1024
#define B_Coef		(-184)
#define CT_Coef		(4659)
#define CT_Offset	(1023)

#define D_Factor1	304
#define R_Coef1		205
#define G_Coef1		1024
#define B_Coef1		(-184)
#define CT_Coef1	(4659)
#define CT_Offset1	(1023)

struct device;

enum tcs3490_pwr_state {
	POWER_ON,
	POWER_OFF,
	POWER_STANDBY,
};

enum tcs3490_ctrl_reg {
	AGAIN_1        = (0 << 0),
	AGAIN_4        = (1 << 0),
	AGAIN_16       = (2 << 0),
	AGAIN_64       = (3 << 0),
};

#define ALS_PERSIST(p) (((p) & 0xf) << 3)

struct tcs3490_parameters {
	u8 als_time;
	u16 als_deltaP;
	u8 als_gain;
	u8 persist;
};

struct lux_segment {
	int d_factor;
	int r_coef;
	int g_coef;
	int b_coef;
	int ct_coef;
	int ct_offset;
};


struct tcs3490_i2c_platform_data {
	/* The following callback for power events received and handled by
	   the driver.  Currently only for SUSPEND and RESUME */
	int (*platform_power)(struct device *dev, enum tcs3490_pwr_state state);
	int (*platform_init)(void);
	void (*platform_teardown)(struct device *dev);
	char const *als_name;
	struct tcs3490_parameters parameters;
	bool als_can_wake;
	struct lux_segment *segment;
	int segment_num;
};

#endif /* __TCS3490_H */
