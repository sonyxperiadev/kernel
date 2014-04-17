/* include/linux/input/kxtik.h - KXTIK accelerometer driver
 *
 * Copyright (C) 2012 Kionix, Inc.
 * Written by Kuching Tan <kuchingtan@kionix.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __KXTIK_H__
#define __KXTIK_H__

#define KXTIK_I2C_NAME          "kxtik"
#define KXTIK_SLAVE_ADDR		0x0F

struct kxtik_platform_data {
	unsigned int min_interval;
	unsigned int poll_interval;

	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;

	bool negate_x;
	bool negate_y;
	bool negate_z;

#define RES_8BIT		0
#define RES_12BIT		(1 << 6)
	u8 res_12bit;
#define KXTIK_G_2G		0
#define KXTIK_G_4G		(1 << 3)
#define KXTIK_G_8G		(1 << 4)
	u8 g_range;

#define ODR12_5F		0
#define ODR25F			1
#define ODR50F			2
#define ODR100F			3
#define ODR200F			4
#define ODR400F			5
#define ODR800F			6

	int (*init) (void);
	void (*exit) (void);
	int (*power_on) (void);
	int (*power_off) (void);
};
#endif /* __KXTIK_H__ */
