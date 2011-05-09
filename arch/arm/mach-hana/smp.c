/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
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

#include <linux/init.h>
#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>

#include <asm/cacheflush.h>
#include <asm/smp_scu.h>
#include <asm/localtimer.h>
#include <asm/io.h>

#include <mach/csp/mm_io.h>
#include <mach/csp/chipregHw_reg.h>

/* SCU base address */
static void __iomem *scu_base = (void __iomem *)(MM_IO_BASE_SCU);

/*
 * Use SCU config register to count number of cores
 */
static inline unsigned int get_core_count(void)
{
	if (scu_base)
		return scu_get_core_count(scu_base);
	return 1;
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
void __init smp_init_cpus(void)
{
	unsigned int i, ncores = get_core_count();

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);
}

static DEFINE_SPINLOCK(boot_lock);

void __cpuinit platform_secondary_init(unsigned int cpu)
{
	trace_hardirqs_off();

	/*
	 * If any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */

    gic_cpu_init(0, __io(MM_IO_BASE_GICCPU));

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}


int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
#if 1
#define SMP_DO_NOT_WAKEUP_SECOND_CORE
#endif
#ifndef SMP_DO_NOT_WAKEUP_SECOND_CORE

	unsigned long timeout;
	static void __iomem *addr;
	addr = ioremap(MM_ADDR_IO_SRAM, SZ_4K);

	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */

	spin_lock(&boot_lock);

	flush_cache_all();
	__raw_writel(0xDEADBEEF, (addr + 8));
	smp_wmb();

	iounmap(addr);

	/*
	 * Now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */

	spin_unlock(&boot_lock);
#endif
	return 0;
}

static void __init wakeup_secondary(void)
{
	void __iomem *chipRegBase;

	chipRegBase = ioremap(MM_ADDR_IO_CHIPREGS, SZ_4K);

	writel((virt_to_phys(bcmhana_secondary_startup) & (~0x3))|0x1,
			chipRegBase + chipregHw_BOOT_2ND_ADDR_OFFSET);

	iounmap(chipRegBase);

	smp_wmb();

	set_event();
	mb();

}


void __init smp_prepare_cpus(unsigned int max_cpus)
{
	unsigned int ncores = get_core_count();
	unsigned int cpu = smp_processor_id();
	int i;

	/* sanity check */
	if (ncores == 0) {
		printk(KERN_ERR
		       "BCMHANA: strange core count of 0? Default to 1\n");
		ncores = 1;
	}

	if (ncores > NR_CPUS) {
		printk(KERN_WARNING
		       "BCMHANA: no. of cores (%d) greater than configured "
		       "maximum of %d - clipping\n",
		       ncores, NR_CPUS);
		ncores = NR_CPUS;
	}
	smp_store_cpu_info(cpu);

	/*
	 * are we trying to boot more cores than exist?
	 */
	if (max_cpus > ncores)
		max_cpus = ncores;

	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);

#if 0
	if (max_cpus > 1) {
#else
	if (max_cpus > 0) {
#endif
		/*
		 * Enable the local timer or broadcast device for the
		 * boot CPU, but only if we have more than one CPU.
		 */
		percpu_timer_setup();

		/*
		 * Initialise the SCU and wake up the secondary core using
		 * wakeup_secondary().
		 */
		scu_enable(scu_base);
		wakeup_secondary();
	}
}
