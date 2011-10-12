/*****************************************************************************
* Copyright 2003 - 2009 Broadcom Corporation.  All rights reserved.
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

#define ISLAND_BOARD_ID ISLAND_BASE
#include "board_template.c"

#ifndef CONFIG_MAP_LITTLE_ISLAND_MODE
#define ISLAND_BOARD_NAME "BCM911160"
#else
#define ISLAND_BOARD_NAME "BCM911160_LI"
#endif
CREATE_BOARD_INSTANCE(ISLAND_BOARD_ID,ISLAND_BOARD_NAME)
