/* arch/arm/mach-msm/sony_gpiomux.h
 *
 * Copyright (C) 2014 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef __SONY_GPIOMUX_H
#define __SONY_GPIOMUX_H

#include <linux/kernel.h>
#include <mach/gpiomux.h>

#define GPIOMUX_FOLLOW_QCT INT_MAX

void __init overwrite_configs(struct msm_gpiomux_configs *base,
		struct msm_gpiomux_configs *specific);
void __init gpiomux_arrange_all_qct_configs(struct gpiomux_setting **settings);
int __init sony_init_gpiomux(struct msm_gpiomux_config *configs,
		unsigned nconfigs);

#endif /* __SONY_GPIOMUX_H */
