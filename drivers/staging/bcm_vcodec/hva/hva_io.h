/*
 * Copyright (c) 2013 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _HVA_IO_H_
#define _HVA_IO_H_

#include <linux/io.h>
#include <linux/types.h>
#include "hva_rdb.h"

#define HVA_REG_ADDR(reg)	(HVA_ ## reg ## _OFFSET)
#define HVA_MEM_ADDR(reg, offset) \
	(HVA_REG_ADDR(reg) + (offset))
#define HVA_REG_WT(hva, reg, value) \
	hva_writel(hva, value, HVA_REG_ADDR(reg))
#define HVA_REG_RD(hva, reg) \
	hva_readl(hva, HVA_REG_ADDR(reg))
#define HVA_MEM_WT(hva, reg, offset, value) \
	hva_writel(hva, value, HVA_MEM_ADDR(reg, offset))
#define HVA_MEM_RD(hva, reg, offset) \
	hva_readl(hva, HVA_MEM_ADDR(reg, offset))

#define HVA_FIELD_MASK(reg, field)	HVA_ ## reg ## _ ## field ## _MASK
#define HVA_FIELD_SHIFT(reg, field)	HVA_ ## reg ## _ ## field ## _SHIFT
#define HVA_FIELD_ENUM(reg, field, en)	HVA_ ## reg ## _ ## field ## _ ## en

#define HVA_SHIFT_FIELD(reg, field, to) \
	(((to) << HVA_FIELD_SHIFT(reg, field)) & HVA_FIELD_MASK(reg, field))

#define HVA_VAL_FIELD_TEST(val, reg, field) \
	((val) & HVA_FIELD_MASK(reg, field))
#define HVA_REG_FIELD_TEST(hva, reg, field) \
	HVA_VAL_FIELD_TEST(HVA_REG_RD(hva, reg), reg, field)
#define HVA_VAL_FIELD_GET(val, reg, field) \
	(((val) >> HVA_FIELD_SHIFT(reg, field)) & \
	(HVA_FIELD_MASK(reg, field) >> HVA_FIELD_SHIFT(reg, field)))
#define HVA_REG_FIELD_GET(hva, reg, field) \
	HVA_VAL_FIELD_GET(HVA_REG_RD(hva, reg), reg, field)
#define HVA_VAL_FIELD_SHIFT_ENUM(reg, field, name) \
	HVA_SHIFT_FIELD(reg, field, HVA_FIELD_ENUM(reg, field, name))
#define HVA_REG_FIELD_SET(hva, reg, field, val) \
	HVA_REG_WT(hva, reg, (((val) << HVA_FIELD_SHIFT(reg, field)) & \
			      HVA_FIELD_MASK(reg, field)))

struct hva {
	unsigned char __iomem *base;
	size_t size;
};

static inline void hva_writel(struct hva *hva, u32 value, u32 reg)
{
	writel(value, hva->base + reg);
}

static inline u32 hva_readl(struct hva *hva, u32 reg)
{
	return readl(hva->base + reg);
}

#endif
