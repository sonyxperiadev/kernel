/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/gpio.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
 * This file contains the GPIO numbers on BCM21553 SoC
 */
#ifndef ASM_ARCH_GPIO_H
#define ASM_ARCH_GPIO_H

#include <asm-generic/gpio.h>

struct bcm_gpio_port {
	void __iomem *base;
	struct gpio_chip chip;
	int32_t irq;
};

#define gpio_get_value(gpio)		__gpio_get_value(gpio)
#define gpio_set_value(gpio, value)	__gpio_set_value(gpio, value)
#define gpio_cansleep(gpio)		__gpio_cansleep(gpio)

#endif /*ASM_ARCH_GPIO_H */
