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
#include <linux/dma-mapping.h>
#include <plat/scu.h>
#include <mach/secure_api.h>
#include <mach/pm.h>
static u32 *smc_buf_p;
static u32 *smc_buf_v;

struct sec_api {
	u32 id;        /* Service ID */
	int nargs;     /* Number of arguments */
};

#define SEC_API_INIT(i, n)    \
{                             \
	.id = i,              \
	.nargs = n,           \
}

static struct sec_api sec_api[SEC_API_MAX] = {
	SEC_API_INIT(0x01000000, 3),
	SEC_API_INIT(0x01000002, 0),
	SEC_API_INIT(0x01000003, 0),
	SEC_API_INIT(0x0E000006, 4),
	SEC_API_INIT(0x0E00000D, 0),
	SEC_API_INIT(0x0E00000F, 0),
};

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
#ifdef SMC_INSTR
			".arch_extension sec\n"
#endif
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

static inline u32 smc(u32 service, u32 flags, u32 args)
{
	u32 spsr, cp15ctrl;
	u32 ret;

	local_fiq_disable();
	spsr = save_spsr();
	cp15ctrl = save_cp15ctrl();

	/* Flush caches */
	scu_invalidate_all();

#ifdef DORMANT_PROFILE
	clear_ns_gpio();
#endif
	ret = __smc(service, flags, args);

	restore_cp15ctrl(cp15ctrl);
	restore_spsr(spsr);

	return ret;
}

u32 hw_sec_pub_dispatcher(u32 service, u32 flags, ...)
{
	unsigned long cpsr;
	u32 ret;
	va_list list;
	int i, nargs, id;

	BUG_ON(service >= SEC_API_MAX);
	if (smc_buf_v == NULL)
		return -ENODEV;

	local_irq_save(cpsr);

	id = sec_api[service].id;
	nargs = sec_api[service].nargs;

	va_start(list, flags);
	for (i = 0; i < nargs; i++)
		smc_buf_v[i] = va_arg(list, u32);
	va_end(list);

	ret = smc(id, flags, (u32)smc_buf_p);

	local_irq_restore(cpsr);

	return ret;
}
EXPORT_SYMBOL(hw_sec_pub_dispatcher);

int __init smc_init(void)
{
	void *v = NULL;
	dma_addr_t p;

	v = dma_alloc_coherent(NULL, SZ_1K, &p, GFP_ATOMIC);
	if (v == NULL) {
		pr_info("%s: smc dma buffer alloc failed\n", __func__);
		return -ENOMEM;
	}

	smc_buf_v = (u32 *) v;
	smc_buf_p = (u32 *) p;
	pr_info("%s: smc_buf_v:0x%x; smc_buf_p :0x%x\n", __func__,
				(u32)smc_buf_v, (u32)smc_buf_p);

	return 0;
}
