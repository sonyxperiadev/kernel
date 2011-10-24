/******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*      @file  /kernel/include/linux/broadcom/bcmbt_rfkill.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
******************************************************************************/

/*
 * rfkill driver
 */

/*
 * Design-specific hardware configuration interface file.
 *
 */

#include <linux/rfkill.h>

#ifndef BCMBT_RFKILL_SETTINGS_H
#define BCMBT_RFKILL_SETTINGS_H

/* Constants and Types */
#define BCMBT_VREG_ON          1
#define BCMBT_VREG_OFF         0
#define BCMBT_N_RESET_ON       0	/* keep in reset */
#define BCMBT_N_RESET_OFF      1
#define BCMBT_AUX0_ON          1
#define BCMBT_AUX0_OFF         0
#define BCMBT_AUX1_ON          0
#define BCMBT_AUX1_OFF         1

#define BCMBT_UNUSED_GPIO	(-1)

struct bcmbt_rfkill_platform_data {
	int vreg_gpio;
	int n_reset_gpio;
	int aux0_gpio;
	int aux1_gpio;
	struct rfkill *rfkill;

};

/* Variable Externs*/
/* Function Prototypes */

#endif /* BCMBT_RFKILL_SETTINGS_H */
