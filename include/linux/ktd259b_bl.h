/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/ktd259b_bl.h
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
 * Generic S2C based backlight driver data
 * - see drivers/video/backlight/ktd259b_bl.c
 */
#ifndef __LINUX_KTD259B_BL_H
#define __LINUX_KTD259B_BL_H

struct platform_ktd259b_backlight_data {
	unsigned int max_brightness;
	unsigned int dft_brightness;
	unsigned int ctrl_pin;
};

#endif
