/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* 	@file	 arch/arm/plat-kona/include/mach/bcm_keypad.h
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

#include <linux/compiler.h>
#include <linux/platform_device.h>

extern struct platform_device bcm_kp_device;

struct bcm_keymap {
	int row;
	int col;
	const char *name;
	int key_code;
};

struct bcm_keypad_platform_info {
	int row_num;
	int col_num;
	struct bcm_keymap *keymap;
	void __iomem *bcm_keypad_base;
};
