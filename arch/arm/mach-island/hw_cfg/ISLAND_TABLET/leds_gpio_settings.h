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

#define GPIO_LEDS_SETTINGS \
{ \
    { \
        .name = "red", \
        .gpio = 0, \
    }, \
    { \
        .name = "green", \
        .gpio = 6, \
    } \
};

#endif /* LEDS_GPIO_SETTINGS_H */
