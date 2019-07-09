/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ASM_ARCH_MSM_MSM_KRYO_L2_ACCESSORS_H
#define __ASM_ARCH_MSM_MSM_KRYO_L2_ACCESSORS_H

#ifdef CONFIG_ARCH_QCOM
void set_l2_indirect_reg(u64 reg_addr, u64 val);
u64 get_l2_indirect_reg(u64 reg_addr);
#endif

#endif
