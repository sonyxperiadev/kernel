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

#ifndef __CPU_H__
#define __CPU_H__

#include <mach/io_map.h>
#include <mach/memory.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

/*Product family id*/
#define KONA_PROD_FMLY_ID	0x3

#define KONA_CHIP_REV_A0 0
#define KONA_CHIP_REV_A1 1
#define KONA_CHIP_REV_B0 1
#define KONA_CHIP_REV_B1 2

#define KONA_PROD_ID_RHEA		0x1
#define KONA_PROD_ID_HAWAII		0x3
#define KONA_PROD_ID_JAVA		0x4

#define KONA_CHIP_ID_RHEA_B1 KONA_CHIP_ID(KONA_PROD_FMLY_ID, \
				KONA_PROD_ID_RHEA, KONA_CHIP_REV_B1)

#define KONA_CHIP_ID_HAWAII_A0 KONA_CHIP_ID(KONA_PROD_FMLY_ID,\
				KONA_PROD_ID_HAWAII, KONA_CHIP_REV_A0)

#define KONA_CHIP_ID_JAVA_A0 KONA_CHIP_ID(KONA_PROD_FMLY_ID,\
				KONA_PROD_ID_JAVA, KONA_CHIP_REV_A0)

#define KONA_CHIP_ID_JAVA_A1 KONA_CHIP_ID(KONA_PROD_FMLY_ID,\
				KONA_PROD_ID_JAVA, KONA_CHIP_REV_A1)


#define KONA_CHIP_ID(f, p, r) \
		((((f) << CHIPREG_CHIPID_REVID_PRODUCT_FAMILY_ID_SHIFT) &\
			CHIPREG_CHIPID_REVID_PRODUCT_FAMILY_ID_MASK) |\
		(((p) << CHIPREG_CHIPID_REVID_PRODUCT_ID_SHIFT) &\
		    CHIPREG_CHIPID_REVID_PRODUCT_ID_MASK) |\
		(((r) << CHIPREG_CHIPID_REVID_REVID_SHIFT) &\
			   CHIPREG_CHIPID_REVID_REVID_MASK))

static inline u32 get_chip_id(void)
{
	return readl(KONA_CHIPREG_VA + CHIPREG_CHIPID_REVID_OFFSET) &
		(~CHIPREG_CHIPID_REVID_RESERVED_MASK);
}

#endif /* __CPU_H__ */
