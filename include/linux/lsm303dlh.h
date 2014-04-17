/*
 * lsm303dlh.h
 * ST 3-Axis Accelerometer/Magnetometer header file
 *
 * Copyright (C) 2010 STMicroelectronics
 * Author: Carmine Iascone (carmine.iascone@st.com)
 * Author: Matteo Dameno (matteo.dameno@st.com)
 *
 * Copyright (C) 2010 STEricsson
 * Author: Mian Yousaf Kaukab <mian.yousaf.kaukab@stericsson.com>
 * Updated:Preetham Rao Kaskurthi <preetham.rao@stericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LSM303DLH_H__
#define __LSM303DLH_H__

#include <linux/ioctl.h>

#ifdef __KERNEL__
/**
 * struct lsm303dlh_platform_data - platform datastructure for lsm303dlh
 * @name_a: accelerometer name
 * @name_m: magnetometer name
 * @irq_a1: interrupt line 1 of accelerometer
 * @irq_a2: interrupt line 2 of accelerometer
 * @irq_m: interrupt line of magnetometer
 * @axis_map_x: x axis position on the hardware, 0 1 or 2
 * @axis_map_y: y axis position on the hardware, 0 1 or 2
 * @axis_map_z: z axis position on the hardware, 0 1 or 2
 * @negative_x: x axis is orientation, 0 or 1
 * @negative_y: y axis is orientation, 0 or 1
 * @negative_z: z axis is orientation, 0 or 1
 */
struct lsm303dlh_platform_data {
	const char *name_a;
	const char *name_m;
	u32  irq_a1;
	u32  irq_a2;
	u32  irq_m;
	u8  axis_map_x;
	u8  axis_map_y;
	u8  axis_map_z;
	u8  negative_x;
	u8  negative_y;
	u8  negative_z;
};
#endif /* __KERNEL__ */

#endif  /* __LSM303DLH_H__ */
