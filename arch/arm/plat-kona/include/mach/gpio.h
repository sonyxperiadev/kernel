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

#define   KONA_MAX_GPIO   192
#define   ARCH_NR_GPIOS   KONA_MAX_GPIO

#include <asm-generic/gpio.h>

#define gpio_get_value(gpio)		 __gpio_get_value(gpio)
#define gpio_set_value(gpio,value)  __gpio_set_value(gpio,value)
#define gpio_cansleep(gpio)		 __gpio_cansleep(gpio)

int __init kona_gpio_init(int num_bank);

#endif /* __PLAT_GPIO_H */

