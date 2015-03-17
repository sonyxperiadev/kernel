/* arch/arm/mach-msm/board-sony_leo_samba-gpiomux-diff.c
 *
 * Copyright (C) 2014 Sony Mobile Communications AB.
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

#include <linux/module.h>
#include <mach/gpiomux.h>
#include "board-sony_leo-gpiomux-diff.h"

static struct gpiomux_setting gpio_2ma_no_pull_out_low = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

static struct gpiomux_setting gpio_2ma_pull_down_in = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_DOWN,
	.dir  = GPIOMUX_IN,
};

static struct gpiomux_setting tsif = {
	.func = GPIOMUX_FUNC_1,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
};

struct msm_gpiomux_config leo_samba_configs[] __initdata = {
	{ /* TUNER_RST_N */
		.gpio = 13,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* TUNER_PWR_EN */
		.gpio = 14,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_no_pull_out_low,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_no_pull_out_low,
		},
	},
	{ /* TUNER_INT */
		.gpio = 68,
		.settings = {
			[GPIOMUX_ACTIVE]    = &gpio_2ma_pull_down_in,
			[GPIOMUX_SUSPENDED] = &gpio_2ma_pull_down_in,
		},
	},
	{ /* TSIF_CLK */
		.gpio = 89,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif,
			[GPIOMUX_SUSPENDED] = &tsif,
		},
	},
	{ /* TSIF_EN */
		.gpio = 90,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif,
			[GPIOMUX_SUSPENDED] = &tsif,
		},
	},
	{ /* TSIF_DATA */
		.gpio = 91,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif,
			[GPIOMUX_SUSPENDED] = &tsif,
		},
	},
	{ /* TSIF_SYNC */
		.gpio = 92,
		.settings = {
			[GPIOMUX_ACTIVE]    = &tsif,
			[GPIOMUX_SUSPENDED] = &tsif,
		},
	},
};

struct msm_gpiomux_configs leo_samba_gpiomux_cfgs __initdata = {
	leo_samba_configs, ARRAY_SIZE(leo_samba_configs)
};
