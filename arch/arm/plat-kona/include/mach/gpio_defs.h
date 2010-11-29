/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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
**   GPIO pin definitions
**
**   This file must contain only defines as it is shared between user and kernel space.
**   As such, it is placed in the linux/broadcom/bcmring directory so it can be exposed
**   using install-headers.  It can not be in the asm/arch directory.
*/

#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

/* ---- Include Files ----------------------------------------------------- */
// #include <cfg_global.h>

/* ---- Public Constants and Types ---------------------------------------- */

// #if CFG_GLOBAL_CHIP == BCM21653 /* HERA */
// #include "gpio_defs_hera.h"
// #else /* BIGISLAND */
#include "gpio_defs_hana.h"
// #endif
#endif /* GPIO_DEFS_H */
