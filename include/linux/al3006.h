/*
 * This file is part of the AL3006 sensor driver.
 * Chip is combined proximity and ambient light sensor.
 *
 * Copyright (c) 2011 Broadcom Corporation
 *
 * Contact: Broadcom
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 *
 * Filename: al3006.h
 *
 * Summary:
 *	AL3006 sensor dirver head file for kernel version 2.6.36.
 *
 */
#ifndef _AL3006_H
#define _AL3006_H

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-kona.h>

/* platform data for the al3006 driver */

struct al3006_platform_data {
	struct i2c_slave_platform_data i2c_pdata;
	int	(*init_platform_hw)(void);
	void (*exit_platform_hw)(void);
};

#endif
