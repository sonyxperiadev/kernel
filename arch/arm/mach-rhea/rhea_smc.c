/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/* Portions of this software are Copyright 2011 Broadcom Corporation */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <asm/cacheflush.h>
#include <mach/memory.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_scu.h>

static inline u32 save_spsr(void)
{
	u32 spsr;
	asm volatile ("mrs  %0,spsr" : "=r"(spsr));
	return spsr;
}

static inline u32 save_cp15ctrl(void)
{
	u32 ctrl;
	asm volatile ("mrc  p15,0,%0,c1,c0,0" : "=r"(ctrl));
	return ctrl;
}

static inline void restore_spsr(u32 spsr)
{
	asm volatile ("msr  spsr_fsxc,%0" : : "r"(spsr));
}

static inline void restore_cp15ctrl(u32 ctrl)
{
	asm volatile ("mcr  p15,0,%0,c1,c0,0" : : "r"(ctrl));
}

static u32 __smc(u32 service, u32 flags, u32 list)
{
	register u32 s asm("r0") = service;
	register u32 f asm("r1") = flags;
	register u32 l asm("r2") = list;
	register u32 ret asm("r0");

	asm volatile (
		__asmeq("%0", "r0")
		__asmeq("%1", "r0")
		__asmeq("%2", "r1")
		__asmeq("%3", "r2")
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"smc	 #0\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		: "=r"(ret)
		: "r"(s), "r"(f), "r"(l)
		);

	return ret;
}

static inline u32 smc(u32 service, u32 flags, va_list *list)
{
	unsigned long cpsr;
	u32 spsr, cp15ctrl;
	u32 args = __virt_to_phys(*(u32 *)list);
	u32 ret;

	local_irq_save(cpsr);
	local_fiq_disable();
	spsr = save_spsr();
	cp15ctrl = save_cp15ctrl();

	/* Flush caches */
	writel(0xFF, KONA_SCU_VA + SCU_INVALIDATE_ALL_OFFSET);
	flush_cache_all();
	outer_flush_all();

	ret = __smc(service, flags, args);

	restore_cp15ctrl(cp15ctrl);
	restore_spsr(spsr);
	local_irq_restore(cpsr);

	return ret;
}

u32 hw_sec_pub_dispatcher(u32 service, u32 flags, ...)
{
	u32 ret;
	va_list list;

	va_start(list, flags);
	ret = smc(service, flags, &list);
	va_end(list);

	return ret;
}
EXPORT_SYMBOL(hw_sec_pub_dispatcher);
