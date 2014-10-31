/* arch/arm/mach-msm/board-sony_sirius_samba-gpiomux-diff.c
 *
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
#include "board-sony_sirius-gpiomux-diff.h"

static struct gpiomux_setting unused_gpio = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv  = GPIOMUX_DRV_2MA,
	.pull = GPIOMUX_PULL_NONE,
	.dir  = GPIOMUX_OUT_LOW,
};

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

struct msm_gpiomux_config sirius_samba_configs[] __initdata = {
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
	{ /* NC */
		.gpio = 108,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 121,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
	{ /* NC */
		.gpio = 136,
		.settings = { [GPIOMUX_SUSPENDED] = &unused_gpio, },
	},
};

struct msm_gpiomux_configs sirius_samba_gpiomux_cfgs __initdata = {
	sirius_samba_configs, ARRAY_SIZE(sirius_samba_configs)
};
