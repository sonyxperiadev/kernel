/* arch/arm/mach-msm/keypad-semc_fuji.h
 *
 * Copyright (C) [2011] Sony Ericsson Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#ifndef ARCH_ARM_MACH_MSM_KEYPAD_SEMC_FUJI_H
#define ARCH_ARM_MACH_MSM_KEYPAD_SEMC_FUJI_H

extern struct pm8xxx_keypad_platform_data fuji_keypad_data;

#ifdef CONFIG_INPUT_ASETM2034A
#include <linux/input/asetm2034a.h>
extern struct asetm2034a_keymap asetm2034a_keymap;
#endif /* CONFIG_INPUT_ASETM2034A */

#ifdef CONFIG_FUJI_GPIO_KEYPAD
extern struct gpio_event_platform_data gpio_key_data;
#endif /* CONFIG_FUJI_GPIO_KEYPAD */

#endif
