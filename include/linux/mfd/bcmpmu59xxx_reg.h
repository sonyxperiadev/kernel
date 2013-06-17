/*  Copyright 2001 - 2012 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef __LINUX_MFD_BCMPMU59xxx_REG_H_
#define __LINUX_MFD_BCMPMU59xxx_REG_H_

#include "bcmpmu59054_reg.h"

/*ENC_PMU_REG(_8BIT_FLAGS_##_8BIT_MAP_ADD##_REG_ADD) */

#define FLAG_SHIFT		24
#define AMAP_SHIFT		16
#define FLAG_MASK		0xFF000000
#define AMAP_MASK		0xFF0000
#define ADD_MASK		0xFFFF
#define MAP0			0
#define	MAP1			1
#define MAP_MASK		0x1
#define MAP_SHIFT		0
#define FIFO_MODE		1
#define FIFO_MODE_MASK		0x1
#define FIFO_MODE_SHIFT		0
#define PAGE(n)			(n << PAGE_SHIFT)
#define I2C_PAGE_MASK		0x2
#define I2C_PAGE_SHIFT		1
#define DIRECT_MODE		0


#define ENC_PMU_REG(f, m, a)	(((f) << FLAG_SHIFT) | \
				(((m) << AMAP_SHIFT) & AMAP_MASK) | \
				((a) & ADD_MASK))

#define DEC_MAP_ADD(x)		(((x) & AMAP_MASK) >> AMAP_SHIFT)
#define DEC_REG_FLAG(x)		(((x) & FLAG_MASK) >> FLAG_SHIFT)
#define DEC_REG_ADD(x)		((x) & ADD_MASK)

#define DEC_PMU_REG(a, f, m, r) do { \
				f = u8(a >> FLAG_SHIFT);\
				m = u8(a >> AMAP_SHIFT);\
				r = u16(a);\
				} while (0)


#endif
