/* Copyright (C) 2012 Sony Mobile Communications AB.
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

#ifndef _ARCH_ARM_MACH_MSM_BOARD_SONY_FUSION3
#define _ARCH_ARM_MACH_MSM_BOARD_SONY_FUSION3

struct device;
struct regulator;

/*
 * sensor_power() should be used for powering the devices
 * connected to L9 (Vdd) and LVS4 (Vio). It takes care of
 * HW restrictions.
 */
extern int sensor_power(bool enable, struct device *dev,
		struct regulator **vdd, struct regulator **vio,
		const char *id_vdd, const char *id_vio);

enum sony_hw {
	HW_UNKNOWN,
	HW_ODIN,
	HW_YUGA,
	HW_YUGA_MAKI,
	HW_POLLUX,
	HW_POLLUX_MAKI,
	HW_DOGO,
	HW_DOGO_MAKI,
};

#define HW_REV_FUTURE	INT_MAX

enum odin_hw_rev {
	HW_REV_ODIN_DP1,
	HW_REV_ODIN_SP1,
	HW_REV_ODIN_SP1_PB,
	HW_REV_ODIN_SP11,
	HW_REV_ODIN_SP12,
	HW_REV_ODIN_AP1,
	HW_REV_ODIN_AP101,
	HW_REV_ODIN_AP105,
	HW_REV_ODIN_FUTURE = HW_REV_FUTURE,
};

enum dogo_hw_rev {
	HW_REV_DOGO_MAKI_DP1,
	HW_REV_DOGO_MAKI_SP1,
	HW_REV_DOGO_MAKI_AP1,
	HW_REV_DOGO_MAKI_TP1,
	HW_REV_DOGO_MAKI_PQ_HVM,
	HW_REV_DOGO_GINA_DP11,
	HW_REV_DOGO_GINA_SP1,
	HW_REV_DOGO_GINA_AP1,
	HW_REV_DOGO_GINA_TP1,
	HW_REV_DOGO_GINA_TP1_PQ_HVM,
	HW_REV_DOGO_GINA_PQ_HVM,
	HW_REV_DOGO_RITA_SP1,
	HW_REV_DOGO_RITA_AP1,
	HW_REV_DOGO_RITA_TP1_PQ_HVM,
	HW_REV_DOGO_RITA_PQ_HVM,
	HW_REV_DOGO_FUTURE = HW_REV_FUTURE,
};

enum yuga_hw_rev {
	HW_REV_YUGA_MAKI_DP0,
	HW_REV_YUGA_MAKI_DP1,
	HW_REV_YUGA_MAKI_DP11,
	HW_REV_YUGA_MAKI_SP1,
	HW_REV_YUGA_MAKI_SP11,
	HW_REV_YUGA_MAKI_AP1,
	HW_REV_YUGA_MAKI_AP11,
	HW_REV_YUGA_MAKI_TP1,
	HW_REV_YUGA_MAKI_PQ,
	HW_REV_YUGA_MAKI_PQ_HVM,
	HW_REV_YUGA_GINA_SP1,
	HW_REV_YUGA_GINA_SP11,
	HW_REV_YUGA_GINA_AP1,
	HW_REV_YUGA_GINA_AP12,
	HW_REV_YUGA_GINA_TP2,
	HW_REV_YUGA_GINA_PQ2,
	HW_REV_YUGA_GINA_PQ2_HVM,
	HW_REV_YUGA_RITA_SP1_SP11,
	HW_REV_YUGA_RITA_AP1,
	HW_REV_YUGA_RITA_AP12,
	HW_REV_YUGA_RITA_TP2,
	HW_REV_YUGA_RITA_PQ2,
	HW_REV_YUGA_RITA_PQ2_HVM,
	HW_REV_YUGA_TINA_TP2,
	HW_REV_YUGA_TINA_TP3,
	HW_REV_YUGA_TINA_HVM,
	HW_REV_YUGA_FUTURE = HW_REV_FUTURE,
};

enum pollux_hw_rev {
	HW_REV_POLLUX_MAKI_DP1,
	HW_REV_POLLUX_MAKI_DP11,
	HW_REV_POLLUX_MAKI_SP1,
	HW_REV_POLLUX_MAKI_AP1,
	HW_REV_POLLUX_MAKI_TP1,
	HW_REV_POLLUX_MAKI_PQ,
	HW_REV_POLLUX_MAKI_PQ_HV,
	HW_REV_POLLUX_GINA_DP11,
	HW_REV_POLLUX_GINA_DP2,
	HW_REV_POLLUX_GINA_SP1,
	HW_REV_POLLUX_GINA_AP1,
	HW_REV_POLLUX_REX_DP1,
	HW_REV_POLLUX_REX_SP1,
	HW_REV_POLLUX_WINDY_DP11,
	HW_REV_POLLUX_WINDY_DP2_DP3,
	HW_REV_POLLUX_WINDY_AP1,
	HW_REV_POLLUX_FUTURE = HW_REV_FUTURE,
};

void __init sony_set_hw_revision(void);
int sony_hw_rev(void);
int sony_hw(void);

#ifdef CONFIG_LEDS_LM3533
#include <linux/leds-lm3533_ng.h>
extern struct lm3533_platform_data lm3533_pdata;
#endif

extern int usb_phy_init_seq_host[];

#endif
