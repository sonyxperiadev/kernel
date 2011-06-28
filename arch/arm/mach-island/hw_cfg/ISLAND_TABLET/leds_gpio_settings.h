/*****************************************************************************
* Copyright 2009 Broadcom Corporation.  All rights reserved.
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

/*
 * LED GPIO configurations
 */

#ifndef LEDS_GPIO_SETTINGS_H
#define LEDS_GPIO_SETTINGS_H

#include <linux/leds.h>


/* GPIO led definitions for Hana tablet */
static struct gpio_led tabletLeds[] = 
{
    {
        .name = "red",
        .gpio = 0,    /* GPIO #0 */
    },
    {
        .name = "green",
        .gpio = 6,    /* GPIO #6 */
    }
};

static struct gpio_led_platform_data leds_gpio_data =
{
    .num_leds = ARRAY_SIZE(tabletLeds),
    .leds = tabletLeds,
};

#endif /* LEDS_GPIO_SETTINGS_H */
