/******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*      @file  arch/arm/plat-bcmap/include/plat/bcm_rfkill.h
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
#ifndef BCMBLT_RFKILL_SETTINGS_H
#define BCMBLT_RFKILL_SETTINGS_H

/* Constants and Types */
#define BCMBLT_VREG_ON          1
#define BCMBLT_VREG_OFF         0
#define BCMBLT_N_RESET_ON       0	/* keep in reset */
#define BCMBLT_N_RESET_OFF      1
#define BCMBLT_AUX0_ON          1
#define BCMBLT_AUX0_OFF         0
#define BCMBLT_AUX1_ON          0
#define BCMBLT_AUX1_OFF         1

/* Variable Externs*/
/* Function Prototypes */

#endif /* BCMBLT_RFKILL_SETTINGS_H */
