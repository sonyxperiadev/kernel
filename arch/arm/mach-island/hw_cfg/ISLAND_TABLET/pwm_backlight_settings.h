/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#ifndef PWM_BACKLIGHT_SETTINGS_H
#define PWM_BACKLIGHT_SETTINGS_H

/*
 * Refer to include/linux/pwm_backlight.h for details
 */

#define HW_PWM_BACKLIGHT_PARAM \
{ \
	.pwm_name	= "kona_pwmc:2", \
	.max_brightness	= 255, \
	.dft_brightness	= 255, \
	.pwm_period_ns	= 5000000, \
	.polarity = 1, \
}

#endif /* PWM_BACKLIGHT_SETTINGS_H */
