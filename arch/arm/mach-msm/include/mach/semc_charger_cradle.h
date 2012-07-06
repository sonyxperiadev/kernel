/* arch/arm/mach-msm/include/semc_charger_cradle.h
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Hiroyuki Namba <Hiroyuki.Namba@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef _SEMC_CHARGER_CRADLE_H_
#define _SEMC_CHARGER_CRADLE_H_

#define SEMC_CHG_CRADLE_NAME "semc_chg_cradle"

struct semc_chg_cradle_platform_data {
	int cradle_detect_gpio;
	char **supplied_to;
	size_t num_supplicants;
	int supply_current_limit_from_cradle;
	int (*gpio_configure)(int);
};

unsigned int semc_charger_cradle_current_ma(void);
int semc_charger_cradle_is_connected(void);

#endif
