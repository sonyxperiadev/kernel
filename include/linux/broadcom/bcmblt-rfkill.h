/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*      @file include/linux/broadcom/bcmblt-rfkill.h
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
 * Broadcom Bluetooth rfkill power control via GPIO
 *
 */

#ifndef _LINUX_BCMBLT_RFKILL_H
#define _LINUX_BCMBLT_RFKILL_H

#include <linux/rfkill.h>

#define BCMBLT_UNUSED_GPIO (-1)

struct bcmblt_rfkill_platform_data {
	int vreg_gpio;
	int n_reset_gpio;
	int aux0_gpio;
	int aux1_gpio;
	struct rfkill *rfkill;	/* for driver only */
};

#endif
