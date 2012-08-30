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

#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <plat/cpu.h>

#define RHEA_CHIP_REV_B0       0x1
#define RHEA_CHIP_REV_B1       0x2
#define RHEA_CHIP_REV_B2       0x3

#define cpu_is_rhea_B0()    (get_chip_id() == RHEA_CHIP_ID(RHEA_CHIP_REV_B0))
#define cpu_is_rhea_B1()    (get_chip_id() == RHEA_CHIP_ID(RHEA_CHIP_REV_B1))
#define cpu_is_rhea_B2()    (get_chip_id() == RHEA_CHIP_ID(RHEA_CHIP_REV_B2))


#define RHEA_PROD_ID		0x1
#define RHEA_PROD_FMLY_ID	KONA_PROD_FMLY_ID


#define RHEA_CHIP_ID(r)	KONA_CHIP_ID(RHEA_PROD_FMLY_ID,\
					RHEA_PROD_ID, r)


#endif /*__RHEA_CPU_H__*/
