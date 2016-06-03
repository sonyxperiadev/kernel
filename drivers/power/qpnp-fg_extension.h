/*
 * Authors: Shingo Nakao <shingo.x.nakao@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#ifndef __QPNP_FG_EXTENSION
#define __QPNP_FG_EXTENSION

static int somc_fg_ceil_capacity(struct fg_somc_params *params, int cap);
static void somc_fg_set_slope_limiter(struct fg_chip *chip);
static void somc_fg_set_aging_mode(struct fg_chip *chip, int64_t learned_cc_uah,
				int nom_cap_uah, int *thresh);
static int somc_chg_fg_of_init(struct fg_chip *chip,
			struct device_node *node);
static int somc_fg_register(struct fg_chip *chip);
static void somc_fg_unregister(struct fg_chip *chip);
static int somc_fg_calc_and_store_cc_soc_coeff(struct fg_chip *chip, int16_t cc_mah);
#endif /* __QPNP_FG_EXTENSION */
