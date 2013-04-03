/*
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 * Copyright (C) 2011 Silicon Image Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MHL_SII8334_H__
#define __MHL_SII8334_H__

#ifdef __KERNEL__

#include <linux/i2c.h>

#define PIN_MODE_TTL		0x00
#define PIN_MODE_TTL_N		0x01
#define PIN_MODE_OPENDRAIN	0x10
#define PIN_MODE_OPENDRAIN_N	0x11

struct mhl_sii_platform_data {
	int reset;
	int hpd_pin_mode;
	int int_pin_mode;
	unsigned int adopter_id;
	unsigned int device_id;
	int (*setup_power)(int enable);
	int (*setup_low_power_mode)(int enable);
	int (*setup_gpio)(int enable);
	int (*charging_enable)(int enable, int max_curr);
};

#define SII_DEV_NAME "sii8334"

#ifdef FB_MSM_MHL_SII8334_SP
void mhl_sii_request_write_burst(u8 offset, u8 *data, u8 length);
#endif

#endif /* __KERNEL__ */

#endif /* __MHL_SII8334_H__ */
