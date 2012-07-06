/* arch/arm/mach-msm/charger-fuji_hikari.c
 *
 * Copyright (C) 2011-2012 Sony Ericsson Mobile Communications AB.
 * Copyright (C) 2012 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/battery_chargalg.h>

static struct battery_regulation_vs_temperature id_bat_reg = {
	/* Cold, Normal, Warm, Overheat */
	{5, 45,		55,	127},	/* battery temp */
	{0, 4200,	4200,	0},	/* charge volt */
	{0, USHRT_MAX,	400,	0},	/* charge curr */
};

static struct ambient_temperature_limit limit_tbl = {
	{0,	41,	45,	127},	/* ambient temp: base */
	{4,	4,	36,	0},	/* ambient temp: hysteresis */
};

struct device_data device_data = {
	.id_bat_reg = &id_bat_reg,
	.limit_tbl = &limit_tbl,
	.battery_capacity_mah = 1900,
	.maximum_charging_current_ma = 1525,
	.battery_connector_resistance = 0,
};
