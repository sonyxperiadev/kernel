/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*       @file   arch/arm/plat-bcmap/include/plat/bcm_pwm_keypad_bl.h
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

#ifndef BCM_PWM_KEYPAD_BL_H
#define BCM_PWM_KEYPAD_BL_H

#include <linux/pwm_backlight.h>

struct keypad_bl_drv_pdata {
	/*
	 * ID of the GPIO used to enable/disable keypad
	 * backlight PWM
	 */
	int pwm_ctrl_gpio;
	/*
	 * Value of the GPIO in its active state (0 or 1)
	 */
	int pwm_ctrl_gpio_active_state;
	struct platform_pwm_backlight_data bl_pdata;
};

#endif/* BCM_PWM_KEYPAD_BL_H */
