/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
 * GPIO keys configurations
 */

#ifndef GPIO_KEYS_SETTINGS_H
#define GPIO_KEYS_SETTINGS_H

#include <linux/input.h>
#include <linux/gpio_keys.h>

/* GPIO led definitions for Hana tablet */
static struct gpio_keys_button tabletKeys[] =
{
    {
		.code = KEY_POWER,
        .desc = "Power",
        .gpio = 4,    /* GPIO #4 */
		.active_low	= 1,
    }
};

static struct gpio_keys_platform_data gpio_keys_data =
{
    .nbuttons = ARRAY_SIZE(tabletKeys),
    .buttons = tabletKeys,
};

#endif /* GPIO_KEYS_SETTINGS_H */
