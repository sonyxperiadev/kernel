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
#ifndef BCMBLT_RFKILL_SETTINGS_H
#define BCMBLT_RFKILL_SETTINGS_H

// ---- Include Files ----------------------------------------

// ---- Constants and Types ----------------------------------
/* BCMBLT_RFKILL_GPIO is defined as BT_RST_B - meaning "BlueTooth
   reset". However it is currently connected in such a way that setting it to
   high powers BT chip on, and setting it to low powers BT chip off */
#define BCMBLT_RFKILL_GPIO	177
#define GPIO_BT_WAKE		165
#define GPIO_HOST_WAKE 		175

// ---- Variable Externs -------------------------------------
// ---- Function Prototypes ----------------------------------

#endif /* BCMBLT_RFKILL_SETTINGS_H */
