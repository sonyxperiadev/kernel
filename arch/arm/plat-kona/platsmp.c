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
#include <linux/delay.h>
#include <mach/memory.h>

#include <asm/cacheflush.h>
#include <asm/hardware/gic.h>
#include <asm/smp_scu.h>
#include <asm/io.h>
#include <mach/smp.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen"
 */
volatile int pen_release = -1;

/* SCU base address */
static void __iomem *scu_base = (void __iomem *)(KONA_SCU_VA);

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
void __init smp_init_cpus(void)
{
	unsigned int i, ncores = scu_get_core_count(scu_base);
	
	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

	set_smp_cross_call(gic_raise_softirq);
}

static DEFINE_SPINLOCK(boot_lock);

void __cpuinit platform_secondary_init(unsigned int cpu)
{
	/*
	 * If any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */
	gic_secondary_init(0);

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	pen_release = -1;
	smp_wmb();

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
#ifdef CONFIG_A9_DORMANT_MODE
	u32 boot_2nd_addr;
#endif
	unsigned long timeout;
	/*

	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * The secondary processor is waiting to be released from
	 * the holding pen - release it, then wait for it to flag
	 * that it has been released by resetting pen_release.
	 *
	 * Note that "pen_release" is the hardware CPU ID, whereas
	 * "cpu" is Linux's internal ID.
	 */
	pen_release = cpu;
	smp_wmb();
	flush_cache_all();
#ifdef CONFIG_OUTER_CACHE
	outer_flush_all();
#endif

#ifdef CONFIG_A9_DORMANT_MODE
	/* Let go of the secondary core */
	boot_2nd_addr =
		readl_relaxed(KONA_CHIPREG_VA+CHIPREG_BOOT_2ND_ADDR_OFFSET);
	boot_2nd_addr |= 1;
	writel_relaxed(boot_2nd_addr,
			KONA_CHIPREG_VA+CHIPREG_BOOT_2ND_ADDR_OFFSET);
#endif

	/*
	 * Send the secondary CPU a soft interrupt. This will
	 * wake it up in case the secondary CPU is in WFI state.
	 */
	gic_raise_softirq(cpumask_of(cpu), 1);


	/* Sample code to wait till the second core acknowledges
	 * while ( readl_relaxed(KONA_CHIPREG_VA+CHIPREG_BOOT_2ND_ADDR_OFFSET)
	 *		& 1 );
	 */

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * Now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

static void __init wakeup_secondary(void)
{
#if defined(CHIPREG_BOOT_2ND_ADDR_OFFSET)
	void __iomem *chipRegBase;

	chipRegBase = IOMEM(KONA_CHIPREG_VA);

	/* Chip-it FPGA has problems writing to this address hence
	 * workaround */
#ifdef CONFIG_MACH_HAWAII_FPGA
	writel((virt_to_phys(kona_secondary_startup) & (~0x3))|0x1, chipRegBase + 0x1C4);
#else
	writel((virt_to_phys(kona_secondary_startup) & (~0x3))|0x1, chipRegBase + CHIPREG_BOOT_2ND_ADDR_OFFSET);
#endif

	smp_wmb();

	/*
	 * Send a 'sev' to wake the secondary core from WFE.
	 * Drain the outstanding writes to memory
	 */
	dsb_sev();
	
	mb();
#endif
}


void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
	int i;
	
	/*
	 * Initialise the present map, which describes the set of CPUs
	 * actually populated at the present time.
	 */
	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);

	/*
	 * Initialise the SCU and wake up the secondary core using
	 * wakeup_secondary().
	 */
	scu_enable(scu_base);
	wakeup_secondary();
}
