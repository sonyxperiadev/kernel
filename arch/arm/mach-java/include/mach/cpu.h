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

#ifndef __HAWAII_CPU_H__
#define __HAWAII_CPU_H__

#include <mach/io_map.h>
#include <mach/memory.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <plat/cpu.h>

#define JAVA_CHIP_REV_A0  0

#define JAVA_PROD_ID		0x4
#define JAVA_PROD_FMLY_ID	KONA_PROD_FMLY_ID

#define cpu_is_java_A0() \
			(get_chip_id() == JAVA_CHIP_ID(JAVA_CHIP_REV_A0))

#define JAVA_CHIP_ID(r)		KONA_CHIP_ID(JAVA_PROD_FMLY_ID,\
				JAVA_PROD_ID, r)


#endif /*__HAWAII_CPU_H__*/
