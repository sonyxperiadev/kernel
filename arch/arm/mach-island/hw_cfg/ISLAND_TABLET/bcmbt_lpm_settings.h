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

//
//---------------------------------------------------------------------------
//
// Design-specific hardware configuration interface file.
//
//---------------------------------------------------------------------------
#ifndef BCMBT_LPM_SETTINGS_H
#define BCMBT_LPM_SETTINGS_H

// ---- Include Files ----------------------------------------

// ---- Constants and Types ----------------------------------
#define GPIO_BT_WAKE		165
#define GPIO_HOST_WAKE		175

#define BT_WAKE_ASSERT		0
#define BT_WAKE_DEASSERT	!(BT_WAKE_ASSERT)

#define HOST_WAKE_ASSERT	0
#define HOST_WAKE_DEASSERT	(!(HOST_WAKE_ASSERT))

// ---- Variable Externs -------------------------------------
// ---- Function Prototypes ----------------------------------

#endif /* BCMBT_LPM_SETTINGS_H */
