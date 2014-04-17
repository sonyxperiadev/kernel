/*****************************************************************************
* Copyright 2003 - 2013 Broadcom Corporation.  All rights reserved.
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
#ifdef CONFIG_ARCH_HAWAII
#include <asm/smp_scu.h>
#endif
#include <asm/smp_plat.h>
#include <asm/io.h>
#ifdef CONFIG_BRCM_CDC
#include <plat/cdc.h>
#endif
#include <mach/smp.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <plat/kona_smp.h>

#ifdef CONFIG_ARCH_HAWAII
/* SCU base address */
static void __iomem *scu_base = (void __iomem *)(KONA_SCU_VA);
#endif

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init kona_smp_init_cpus(void)
{
	unsigned int i;
#if defined(CONFIG_ARCH_HAWAII)
	unsigned int ncores = scu_get_core_count(scu_base);
#elif defined(CONFIG_ARCH_JAVA)
	unsigned int ncores = NR_CPUS;
#endif

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

}

static DEFINE_SPINLOCK(boot_lock);

static void __cpuinit kona_platform_secondary_init(unsigned int cpu)
{
#ifdef CONFIG_BRCM_CDC
/* Bring this CPU to RUN state so that nIRQ nFIQ signals are unblocked */
	cdc_send_cmd_early(CDC_CMD_SDEC, cpu);
#endif
	/*
	 * If any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

static void __init wakeup_secondary(void)
{

	void __iomem *chip_reg_base;

	chip_reg_base = IOMEM(KONA_CHIPREG_VA);

	writel(virt_to_phys(kona_secondary_startup),
			chip_reg_base + CHIPREG_BOOT_2ND_ADDR_OFFSET);

	smp_wmb();

	/*
	 * Send a 'sev' to wake the secondary core from WFE.
	 * Drain the outstanding writes to memory
	 */
	dsb_sev();

	mb();
}

static int __cpuinit kona_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	u32 boot_2nd_addr;
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
	write_pen_release(cpu_logical_map(cpu));

	flush_cache_all();
#ifdef CONFIG_OUTER_CACHE
	outer_flush_all();
#endif

	/* Let go of the secondary core */
	boot_2nd_addr =
		readl_relaxed(KONA_CHIPREG_VA+CHIPREG_BOOT_2ND_ADDR_OFFSET);

	boot_2nd_addr &= ~0x3; /* Clear the CPU ID bits */
	boot_2nd_addr |= cpu;  /* Set CPU number in CPU ID bits */
	writel_relaxed(boot_2nd_addr,
			KONA_CHIPREG_VA+CHIPREG_BOOT_2ND_ADDR_OFFSET);

	/*
	 * Send the secondary CPU a soft interrupt. This will
	 * wake it up in case the secondary CPU is in WFI state.
	 */
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1) {
			break;
		}

		udelay(10);
	}

	/*
	 * Now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

static void __init kona_platform_smp_prepare_cpus(unsigned int max_cpus)
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

	wakeup_secondary();
#ifdef CONFIG_ARCH_HAWAII
	scu_enable(scu_base);
#endif
}



struct smp_operations kona_smp_ops __initdata = {
	.smp_init_cpus		=	kona_smp_init_cpus,
	.smp_prepare_cpus	=	kona_platform_smp_prepare_cpus,
	.smp_secondary_init	=	kona_platform_secondary_init,
	.smp_boot_secondary	=	kona_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		=	kona_platform_cpu_die,
#endif
};
