/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL"). 
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef __PLAT_GPIO_H
#define __PLAT_GPIO_H

#include <linux/init.h>

#define   KONA_MAX_GPIO   192

#ifdef CONFIG_GPIO_PCA953X
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
#define EXPANDED_GPIOS		32	/*2 expander on RheaRayEDN1x, 32 gpio expanded */
#else
#define EXPANDED_GPIOS		16
#endif
#define ARCH_NR_GPIOS		(KONA_MAX_GPIO+EXPANDED_GPIOS)
#else
#ifdef CONFIG_GPIO_TC3589X
#define EXPANDED_GPIOS		24
#define ARCH_NR_GPIOS		(KONA_MAX_GPIO+EXPANDED_GPIOS)
#else
#define ARCH_NR_GPIOS		KONA_MAX_GPIO
#endif
#endif

#include <asm-generic/gpio.h>
#include <mach/irqs.h>

/*
#define gpio_get_value(gpio)		 __gpio_get_value(gpio)
#define gpio_set_value(gpio,value)  __gpio_set_value(gpio,value)
#define gpio_cansleep(gpio)		 __gpio_cansleep(gpio)
*/

int __init kona_gpio_init(int num_bank);

#ifdef CONFIG_KONA_ATAG_DT
uint32_t get_dts_gpio_value(uint32_t index);
#endif /* CONFIG_KONA_ATAG_DT */

#endif /* __PLAT_GPIO_H */
