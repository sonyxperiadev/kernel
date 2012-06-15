#ifndef __ARM_MMU_H
#define __ARM_MMU_H

#ifdef CONFIG_MMU

typedef struct {
#ifdef CONFIG_CPU_HAS_ASID
	u64 id;
#endif
	unsigned int kvm_seq;
} mm_context_t;

#ifdef CONFIG_CPU_HAS_ASID
#define ASID_BITS	8
#define ASID_MASK	((~0ULL) << ASID_BITS)
#define ASID(mm)	((mm)->context.id & ~ASID_MASK)
#else
#define ASID(mm)	(0)
#endif

#else

/*
 * From nommu.h:
 *  Copyright (C) 2002, David McCullough <davidm@snapgear.com>
 *  modified for 2.6 by Hyok S. Choi <hyok.choi@samsung.com>
 */
typedef struct {
	unsigned long		end_brk;
} mm_context_t;

#endif

/*
 * switch_mm() may do a full cache flush over the context switch,
 * so enable interrupts over the context switch to avoid high
 * latency.
 */
#ifndef CONFIG_CPU_HAS_ASID
#define __ARCH_WANT_INTERRUPTS_ON_CTXSW
#endif

#endif
