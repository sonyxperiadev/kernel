/* Copyright (c) 2011, The Linux Foundation. All rights reserved.
 * Copyright (C) 2012 Sony Mobile Communications AB.
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

#ifndef __ASM_ARCH_MSM_SERIAL_HS_LITE_H
#define __ASM_ARCH_MSM_SERIAL_HS_LITE_H

#include <linux/serial_core.h>

struct msm_serial_hslite_platform_data {
	unsigned config_gpio;
	unsigned uart_tx_gpio;
	unsigned uart_rx_gpio;
	int line;
	int type;
	int (*pre_startup)(struct uart_port *);
};

#endif

