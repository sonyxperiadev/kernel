/*
 *  linux/arch/arm/mach-realview/hotplug.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/smp.h>
#include <linux/cpu.h>
#include <asm/cpu.h>
#include <asm/cputype.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <asm/cp15.h>
#include <asm/cacheflush.h>
#include <plat/kona_pm.h>
#if defined(CONFIG_A9_DORMANT_MODE) || defined(CONFIG_DORMANT_MODE)
#include <mach/dormant.h>
#endif
#include <asm/smp_plat.h>

static inline void cpu_enter_lowpower(void)
{
	unsigned int v;

	flush_cache_all();
	asm volatile(
	"	mcr	p15, 0, %1, c7, c5, 0\n"
	"	mcr	p15, 0, %1, c7, c10, 4\n"
	/*
	 * Turn off coherency
	 */
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	bic	%0, %0, #0x40\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	"	mrc	p15, 0, %0, c1, c0, 0\n"
	"	bic	%0, %0, %2\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	  : "=&r" (v)
	  : "r" (0), "Ir" (CR_C)
	  : "cc");
}

static inline void cpu_leave_lowpower(void)
{
	unsigned int v;

	asm volatile(	"mrc	p15, 0, %0, c1, c0, 0\n"
	"	orr	%0, %0, %1\n"
	"	mcr	p15, 0, %0, c1, c0, 0\n"
	"	mrc	p15, 0, %0, c1, c0, 1\n"
	"	orr	%0, %0, #0x40\n"
	"	mcr	p15, 0, %0, c1, c0, 1\n"
	  : "=&r" (v)
	  : "Ir" (CR_C)
	  : "cc");
}

static inline void platform_do_lowpower(unsigned int cpu, int *spurious)
{
	struct task_struct *idle;


	/*
	 * there is no power-control hardware on this platform, so all
	 * we can do is put the core into WFI; this is safe as the calling
	 * code will have already disabled interrupts
	 */
	local_irq_disable();
	for (;;) {
		/*
		 * here's the WFI
		 */
#if defined(CONFIG_A9_DORMANT_MODE)
		kona_pm_cpu_lowpower();
#elif defined(CONFIG_DORMANT_MODE)
		if (is_dormant_enabled())
			kona_pm_cpu_lowpower();
		else
			asm(".word	0xe320f003\n"
				:
				:
				: "memory", "cc");
#else
		asm(".word	0xe320f003\n"
			:
			:
			: "memory", "cc");
#endif
		if (pen_release == cpu_logical_map(cpu)) {
			/*
			 * OK, proper wakeup, we're done
			 */
			break;
		}

		/*
		 * Getting here, means that we have come out of WFI without
		 * having been woken up - this shouldn't happen
		 *
		 * Just note it happening - when we're woken, we can report
		 * its occurrence.
		 */
		(*spurious)++;
	}

	idle = idle_task(cpu);
	BUG_ON(!idle);
	/*
	For Hawaii and Java, all cores are reset on exit from full dormant.
	An offlined core may exit dormant while in offlined
	state due to full dormant reset (dormant driver push the core back
	to dormant in this case). When an offlined core is brought back to
	online state, init_idle() function invoked by the core that's
	powering up the offlined core will set preempt_count of offlined core's
	idle task to zero. init_idle() is not using any access protection while
	updating preempt_count.

	preempt_count may get corrupted if the offlined core is active due to
	full dormant rest and is also updating  preempt_count through spin_lock
	or other kernel API calls

	Workaround is to force preempt_count to zero before offlined core exits
	to online state.
	*/
	task_thread_info(idle)->preempt_count = 0;

	local_irq_enable();
}

int platform_cpu_kill(unsigned int cpu)
{
	return 1;
}

/*
 * platform-specific code to shutdown a CPU
 *
 * Called with IRQs disabled
 */
void __ref kona_platform_cpu_die(unsigned int cpu)
{
	int spurious = 0;
#ifdef DEBUG
	unsigned int this_cpu = hard_smp_processor_id();

	if (cpu != this_cpu) {
		pr_crit("Eek! platform_cpu_die running on %u, should be %u\n",
			   this_cpu, cpu);
		BUG();
	}
#endif

	pr_notice("CPU%u: shutdown\n", cpu);

#if defined(CONFIG_A9_DORMANT_MODE) || defined(CONFIG_DORMANT_MODE)
	if (is_dormant_enabled())
		platform_do_lowpower(cpu, &spurious);
	else {
#endif
	/*
	 * we're ready for shutdown now, so do it
	 */
	cpu_enter_lowpower();
	platform_do_lowpower(cpu, &spurious);

	/*
	 * bring this CPU back into the world of cache
	 * coherency, and then restore interrupts
	 */
	cpu_leave_lowpower();
#if defined(CONFIG_A9_DORMANT_MODE) || defined(CONFIG_DORMANT_MODE)
	}
#endif

	if (spurious)
		pr_warn("CPU%u: %u spurious wakeup calls\n", cpu, spurious);
}
