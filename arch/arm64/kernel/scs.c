/*
 * Shadow Call Stack support.
 *
 * Copyright (C) 2018 Google LLC
 */

#include <linux/percpu.h>
#include <linux/vmalloc.h>
#include <asm/scs.h>

DEFINE_PER_CPU(unsigned long *, irq_shadow_call_stack_ptr);

#ifndef CONFIG_SHADOW_CALL_STACK_VMAP
DEFINE_PER_CPU(unsigned long [SCS_SIZE/sizeof(long)], irq_shadow_call_stack)
	__aligned(SCS_SIZE);
#endif

void scs_init_irq(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
#ifdef CONFIG_SHADOW_CALL_STACK_VMAP
		unsigned long *p;

		p = __vmalloc_node_range(SCS_SIZE, SCS_SIZE,
					 VMALLOC_START, VMALLOC_END,
					 SCS_GFP, PAGE_KERNEL,
					 0, cpu_to_node(cpu),
					 __builtin_return_address(0));

		per_cpu(irq_shadow_call_stack_ptr, cpu) = p;
#else
		per_cpu(irq_shadow_call_stack_ptr, cpu) =
			per_cpu(irq_shadow_call_stack, cpu);
#endif /* CONFIG_SHADOW_CALL_STACK_VMAP */
	}
}
