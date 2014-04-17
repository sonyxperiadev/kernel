/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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

#ifndef __RHEA_CPU_H__
#define __RHEA_CPU_H__

#include <plat/cpu.h>

#define cpu_is_rhea_B0()    (get_chip_id() == KONA_CHIP_ID_RHEA_B1)
#define cpu_is_rhea_B1()    (get_chip_id() == KONA_CHIP_ID_RHEA_B1)
#define cpu_is_rhea_B2()    (get_chip_id() == KONA_CHIP_ID_RHEA_B1)

#endif /*__RHEA_CPU_H__*/
