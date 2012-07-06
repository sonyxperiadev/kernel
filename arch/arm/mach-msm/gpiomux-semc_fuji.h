/* arch/arm/mach-msm/gpiomux-semc_fuji.h
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#ifndef ARCH_ARM_MACH_MSM_GPIOMUX_SEMC_FUJI_H
#define ARCH_ARM_MACH_MSM_GPIOMUX_SEMC_FUJI_H

#include <mach/irqs.h>
#include <mach/gpiomux.h>
#include "gpiomux-8x60.h"

extern struct msm_gpiomux_configs semc_fuji_gpiomux_cfgs[] __initdata;

#define PMIC_GPIO_SDC3_DET 22
#define PM8058_GPIO_INT           88
#define PM8901_GPIO_INT           91


#endif
