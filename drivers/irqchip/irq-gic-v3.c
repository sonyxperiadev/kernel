/*
 * Copyright (C) 2013-2017 ARM Limited, All Rights Reserved.
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define pr_fmt(fmt)	"GICv3: " fmt

#include <linux/acpi.h>
#include <linux/cpu.h>
#include <linux/cpu_pm.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/percpu.h>
#include <linux/slab.h>
#include <linux/msm_rtb.h>

#include <linux/irqchip.h>
#include <linux/irqchip/arm-gic-common.h>
#include <linux/irqchip/arm-gic-v3.h>
#include <linux/irqchip/irq-partition-percpu.h>

#include <asm/cputype.h>
#include <asm/exception.h>
#include <asm/smp_plat.h>
#include <asm/virt.h>

#include <linux/syscore_ops.h>

#include "irq-gic-common.h"

#define MAX_IRQ			1020U	/* Max number of SGI+PPI+SPI */
#define SPI_START_IRQ		32	/* SPI start irq number */
#define GICD_ICFGR_BITS		2	/* 2 bits per irq in GICD_ICFGR */
#define GICD_ISENABLER_BITS	1	/* 1 bit per irq in GICD_ISENABLER */
#define GICD_IPRIORITYR_BITS	8	/* 8 bits per irq in GICD_IPRIORITYR */

/* 32 bit mask with lower n bits set */
#define UMASK_LOW(n)		(~0U >> (32 - (n)))

/* Number of 32-bit words required to store all irqs, for
 * registers where each word stores configuration for each irq
 * in bits_per_irq bits.
 */
#define NUM_IRQ_WORDS(bits_per_irq)	(DIV_ROUND_UP(MAX_IRQ, \
						      32 / (bits_per_irq)))
#define MAX_IRQS_IGNORE		10

#define IRQ_NR_BOUND(nr)	min((nr), MAX_IRQ)

/* Bitmap to irqs, which are restored */
static DECLARE_BITMAP(irqs_restore, MAX_IRQ);

/* Bitmap to irqs, for which restore is ignored.
 * Presently, only GICD_IROUTER mismatches are
 * ignored.
 */
static DECLARE_BITMAP(irqs_ignore_restore, MAX_IRQ);

struct redist_region {
	void __iomem		*redist_base;
	phys_addr_t		phys_base;
	bool			single_redist;
};

struct gic_chip_data {
	struct fwnode_handle	*fwnode;
	void __iomem		*dist_base;
	struct redist_region	*redist_regions;
	struct rdists		rdists;
	struct irq_domain	*domain;
	u64			redist_stride;
	u32			nr_redist_regions;
	bool			has_rss;
	unsigned int		irq_nr;
	struct partition_desc	*ppi_descs[16];

	u64 saved_spi_router[MAX_IRQ];
	u32 saved_spi_enable[NUM_IRQ_WORDS(GICD_ISENABLER_BITS)];
	u32 saved_spi_cfg[NUM_IRQ_WORDS(GICD_ICFGR_BITS)];
	u32 saved_spi_priority[NUM_IRQ_WORDS(GICD_IPRIORITYR_BITS)];

	u64 changed_spi_router[MAX_IRQ];
	u32 changed_spi_enable[NUM_IRQ_WORDS(GICD_ISENABLER_BITS)];
	u32 changed_spi_cfg[NUM_IRQ_WORDS(GICD_ICFGR_BITS)];
	u32 changed_spi_priority[NUM_IRQ_WORDS(GICD_IPRIORITYR_BITS)];
};

static struct gic_chip_data gic_data __read_mostly;
static DEFINE_STATIC_KEY_TRUE(supports_deactivate_key);

static struct gic_kvm_info gic_v3_kvm_info;
static DEFINE_PER_CPU(bool, has_rss);

enum gicd_save_restore_reg {
	SAVED_ICFGR,
	SAVED_IS_ENABLER,
	SAVED_IPRIORITYR,
	NUM_SAVED_GICD_REGS,
};

/* Stores start address of spi config for saved gicd regs */
static u32 *saved_spi_regs_start[NUM_SAVED_GICD_REGS] = {
	[SAVED_ICFGR] = gic_data.saved_spi_cfg,
	[SAVED_IS_ENABLER] = gic_data.saved_spi_enable,
	[SAVED_IPRIORITYR] = gic_data.saved_spi_priority,
};

/* Stores start address of spi config for changed gicd regs */
static u32 *changed_spi_regs_start[NUM_SAVED_GICD_REGS] = {
	[SAVED_ICFGR] = gic_data.changed_spi_cfg,
	[SAVED_IS_ENABLER] = gic_data.changed_spi_enable,
	[SAVED_IPRIORITYR] = gic_data.changed_spi_priority,
};

/* GICD offset for saved registers */
static u32 gicd_offset[NUM_SAVED_GICD_REGS] = {
	[SAVED_ICFGR] = GICD_ICFGR,
	[SAVED_IS_ENABLER] = GICD_ISENABLER,
	[SAVED_IPRIORITYR] = GICD_IPRIORITYR,
};

/* Bits per irq word, for gicd saved registers */
static u32 gicd_reg_bits_per_irq[NUM_SAVED_GICD_REGS] = {
	[SAVED_ICFGR] = GICD_ICFGR_BITS,
	[SAVED_IS_ENABLER] = GICD_ISENABLER_BITS,
	[SAVED_IPRIORITYR] = GICD_IPRIORITYR_BITS,
};

#define for_each_spi_irq_word(i, reg) \
	for (i = 0; \
	    i < DIV_ROUND_UP(IRQ_NR_BOUND(gic_data.irq_nr) - SPI_START_IRQ, \
			     32 / gicd_reg_bits_per_irq[reg]); \
	    i++)

#define read_spi_word_offset(base, reg, i) \
	readl_relaxed_no_log(	\
			base + gicd_offset[reg] + i * 4 +	\
			SPI_START_IRQ * gicd_reg_bits_per_irq[reg] / 8)

#define restore_spi_word_offset(base, reg, i) \
	writel_relaxed_no_log(	\
			saved_spi_regs_start[reg][i],\
			base + gicd_offset[reg] + i * 4 +	\
			SPI_START_IRQ * gicd_reg_bits_per_irq[reg] / 8)

#define MPIDR_RS(mpidr)			(((mpidr) & 0xF0UL) >> 4)
#define gic_data_rdist()		(this_cpu_ptr(gic_data.rdists.rdist))
#define gic_data_rdist_rd_base()	(gic_data_rdist()->rd_base)
#define gic_data_rdist_sgi_base()	(gic_data_rdist_rd_base() + SZ_64K)

/* Our default, arbitrary priority value. Linux only uses one anyway. */
#define DEFAULT_PMR_VALUE	0xf0

static inline unsigned int gic_irq(struct irq_data *d)
{
	return d->hwirq;
}

static inline int gic_irq_in_rdist(struct irq_data *d)
{
	return gic_irq(d) < 32;
}

static inline void __iomem *gic_dist_base(struct irq_data *d)
{
	if (gic_irq_in_rdist(d))	/* SGI+PPI -> SGI_base for this CPU */
		return gic_data_rdist_sgi_base();

	if (d->hwirq <= 1023)		/* SPI -> dist_base */
		return gic_data.dist_base;

	return NULL;
}

static void gic_do_wait_for_rwp(void __iomem *base)
{
	u32 count = 1000000;	/* 1s! */

	while (readl_relaxed_no_log(base + GICD_CTLR) & GICD_CTLR_RWP) {
		count--;
		if (!count) {
			pr_err_ratelimited("RWP timeout, gone fishing\n");
			return;
		}
		cpu_relax();
		udelay(1);
	};
}

/* Wait for completion of a distributor change */
static void gic_dist_wait_for_rwp(void)
{
	gic_do_wait_for_rwp(gic_data.dist_base);
}

/* Wait for completion of a redistributor change */
static void gic_redist_wait_for_rwp(void)
{
	gic_do_wait_for_rwp(gic_data_rdist_rd_base());
}

#ifdef CONFIG_ARM64

static u64 __maybe_unused gic_read_iar(void)
{
	if (cpus_have_const_cap(ARM64_WORKAROUND_CAVIUM_23154))
		return gic_read_iar_cavium_thunderx();
	else
		return gic_read_iar_common();
}
#endif

void gic_v3_dist_save(void)
{
	void __iomem *base = gic_data.dist_base;
	int reg, i;

	bitmap_zero(irqs_restore, MAX_IRQ);

	for (reg = SAVED_ICFGR; reg < NUM_SAVED_GICD_REGS; reg++) {
		for_each_spi_irq_word(i, reg) {
			saved_spi_regs_start[reg][i] =
				read_spi_word_offset(base, reg, i);
			changed_spi_regs_start[reg][i] = 0;
		}
	}

	for (i = 32; i < IRQ_NR_BOUND(gic_data.irq_nr); i++) {
		gic_data.saved_spi_router[i] =
			gic_read_irouter(base + GICD_IROUTER + i * 8);
		gic_data.changed_spi_router[i] = 0;
	}
}

static void _gicd_check_reg(enum gicd_save_restore_reg reg)
{
	void __iomem *base = gic_data.dist_base;
	u32 *saved_spi_cfg = saved_spi_regs_start[reg];
	u32 *changed_spi_cfg = changed_spi_regs_start[reg];
	u32 bits_per_irq = gicd_reg_bits_per_irq[reg];
	u32 current_cfg = 0;
	int i, j = SPI_START_IRQ, l;
	u32 k;

	for_each_spi_irq_word(i, reg) {
		current_cfg = read_spi_word_offset(base, reg, i);
		if (current_cfg != saved_spi_cfg[i]) {
			for (k = current_cfg ^ saved_spi_cfg[i],
			     l = 0; k ; k >>= bits_per_irq, l++) {
				if (k & UMASK_LOW(bits_per_irq))
					set_bit(j+l, irqs_restore);
			}
			changed_spi_cfg[i] = current_cfg ^ saved_spi_cfg[i];
		}
		j += 32 / bits_per_irq;
	}
}

#define _gic_v3_dist_check_icfgr()	\
		_gicd_check_reg(SAVED_ICFGR)
#define _gic_v3_dist_check_ipriorityr()	\
		_gicd_check_reg(SAVED_IPRIORITYR)
#define _gic_v3_dist_check_isenabler()	\
		_gicd_check_reg(SAVED_IS_ENABLER)

static void _gic_v3_dist_check_irouter(void)
{
	void __iomem *base = gic_data.dist_base;
	u64 current_irouter_cfg = 0;
	int i;

	for (i = 32; i < IRQ_NR_BOUND(gic_data.irq_nr); i++) {
		if (test_bit(i, irqs_ignore_restore))
			continue;
		current_irouter_cfg = gic_read_irouter(
					base + GICD_IROUTER + i * 8);
		if (current_irouter_cfg != gic_data.saved_spi_router[i]) {
			set_bit(i, irqs_restore);
			gic_data.changed_spi_router[i] =
			    current_irouter_cfg ^ gic_data.saved_spi_router[i];
		}
	}
}

static void _gic_v3_dist_restore_reg(enum gicd_save_restore_reg reg)
{
	void __iomem *base = gic_data.dist_base;
	int i;

	for_each_spi_irq_word(i, reg) {
		if (changed_spi_regs_start[reg][i])
			restore_spi_word_offset(base, reg, i);
	}

	/* Commit all restored configurations before subsequent writes */
	wmb();
}

#define _gic_v3_dist_restore_icfgr()	_gic_v3_dist_restore_reg(SAVED_ICFGR)
#define _gic_v3_dist_restore_ipriorityr()		\
		_gic_v3_dist_restore_reg(SAVED_IPRIORITYR)

static void _gic_v3_dist_restore_set_reg(u32 offset)
{
	void __iomem *base = gic_data.dist_base;
	int i, j = SPI_START_IRQ, l;
	int irq_nr = IRQ_NR_BOUND(gic_data.irq_nr) - SPI_START_IRQ;

	for (i = 0; i < DIV_ROUND_UP(irq_nr, 32); i++, j += 32) {
		u32 reg_val = readl_relaxed_no_log(base + offset + i * 4 + 4);
		bool irqs_restore_updated = 0;

		for (l = 0; l < 32; l++) {
			if (test_bit(j+l, irqs_restore)) {
				reg_val |= BIT(l);
				irqs_restore_updated = 1;
			}
		}

		if (irqs_restore_updated) {
			writel_relaxed_no_log(
				reg_val, base + offset + i * 4 + 4);
		}
	}

	/* Commit restored configuration updates before subsequent writes */
	wmb();
}

#define _gic_v3_dist_restore_isenabler()		\
		_gic_v3_dist_restore_set_reg(GICD_ISENABLER)

#define _gic_v3_dist_restore_ispending()		\
		_gic_v3_dist_restore_set_reg(GICD_ISPENDR)

static void _gic_v3_dist_restore_irouter(void)
{
	void __iomem *base = gic_data.dist_base;
	int i;

	for (i = 32; i < IRQ_NR_BOUND(gic_data.irq_nr); i++) {
		if (test_bit(i, irqs_ignore_restore))
			continue;
		if (gic_data.changed_spi_router[i]) {
			gic_write_irouter(gic_data.saved_spi_router[i],
						base + GICD_IROUTER + i * 8);
		}
	}

	/* Commit GICD_IROUTER writes before subsequent writes */
	wmb();
}

static void _gic_v3_dist_clear_reg(u32 offset)
{
	void __iomem *base = gic_data.dist_base;
	int i, j = SPI_START_IRQ, l;
	int irq_nr = IRQ_NR_BOUND(gic_data.irq_nr) - SPI_START_IRQ;

	for (i = 0; i < DIV_ROUND_UP(irq_nr, 32); i++, j += 32) {
		u32 clear = 0;
		bool irqs_restore_updated = 0;

		for (l = 0; l < 32; l++) {
			if (test_bit(j+l, irqs_restore)) {
				clear |= BIT(l);
				irqs_restore_updated = 1;
			}
		}

		if (irqs_restore_updated) {
			writel_relaxed_no_log(
				clear, base + offset + i * 4 + 4);
		}
	}

	/* Commit clearing of irq config before subsequent writes */
	wmb();
}

#define _gic_v3_dist_set_icenabler()		\
		_gic_v3_dist_clear_reg(GICD_ICENABLER)

#define _gic_v3_dist_set_icpending()		\
		_gic_v3_dist_clear_reg(GICD_ICPENDR)

#define _gic_v3_dist_set_icactive()		\
		_gic_v3_dist_clear_reg(GICD_ICACTIVER)

/* Restore GICD state for SPIs. SPI configuration is restored
 * for GICD_ICFGR, GICD_ISENABLER, GICD_IPRIORITYR, GICD_IROUTER
 * registers. Following is the sequence for restore:
 *
 * 1. For SPIs, check whether any of GICD_ICFGR, GICD_ISENABLER,
 *    GICD_IPRIORITYR, GICD_IROUTER, current configuration is
 *    different from saved configuration.
 *
 * For all irqs, with mismatched configurations,
 *
 * 2. Set GICD_ICENABLER and wait for its completion.
 *
 * 3. Restore any changed GICD_ICFGR, GICD_IPRIORITYR, GICD_IROUTER
 *    configurations.
 *
 * 4. Set GICD_ICACTIVER.
 *
 * 5. Set pending for the interrupt.
 *
 * 6. Enable interrupt and wait for its completion.
 *
 */
void gic_v3_dist_restore(void)
{
	_gic_v3_dist_check_icfgr();
	_gic_v3_dist_check_ipriorityr();
	_gic_v3_dist_check_isenabler();
	_gic_v3_dist_check_irouter();

	if (bitmap_empty(irqs_restore, IRQ_NR_BOUND(gic_data.irq_nr)))
		return;

	_gic_v3_dist_set_icenabler();
	gic_dist_wait_for_rwp();

	_gic_v3_dist_restore_icfgr();
	_gic_v3_dist_restore_ipriorityr();
	_gic_v3_dist_restore_irouter();

	_gic_v3_dist_set_icactive();

	_gic_v3_dist_set_icpending();
	_gic_v3_dist_restore_ispending();

	_gic_v3_dist_restore_isenabler();
	gic_dist_wait_for_rwp();

	/* Commit all writes before proceeding */
	wmb();
}

static void gic_enable_redist(bool enable)
{
	void __iomem *rbase;
	u32 count = 1000000;	/* 1s! */
	u32 val;

	rbase = gic_data_rdist_rd_base();

	val = readl_relaxed(rbase + GICR_WAKER);
	if (enable)
		/* Wake up this CPU redistributor */
		val &= ~GICR_WAKER_ProcessorSleep;
	else
		val |= GICR_WAKER_ProcessorSleep;
	writel_relaxed(val, rbase + GICR_WAKER);

	if (!enable) {		/* Check that GICR_WAKER is writeable */
		val = readl_relaxed(rbase + GICR_WAKER);
		if (!(val & GICR_WAKER_ProcessorSleep))
			return;	/* No PM support in this redistributor */
	}

	while (--count) {
		val = readl_relaxed(rbase + GICR_WAKER);
		if (enable ^ (bool)(val & GICR_WAKER_ChildrenAsleep))
			break;
		cpu_relax();
		udelay(1);
	};
	if (!count)
		pr_err_ratelimited("redistributor failed to %s...\n",
				   enable ? "wakeup" : "sleep");
}

/*
 * Routines to disable, enable, EOI and route interrupts
 */
static int gic_peek_irq(struct irq_data *d, u32 offset)
{
	u32 mask = 1 << (gic_irq(d) % 32);
	void __iomem *base;

	if (gic_irq_in_rdist(d))
		base = gic_data_rdist_sgi_base();
	else
		base = gic_data.dist_base;

	return !!(readl_relaxed_no_log
		(base + offset + (gic_irq(d) / 32) * 4) & mask);
}

static void gic_poke_irq(struct irq_data *d, u32 offset)
{
	u32 mask = 1 << (gic_irq(d) % 32);
	void (*rwp_wait)(void);
	void __iomem *base;

	if (gic_irq_in_rdist(d)) {
		base = gic_data_rdist_sgi_base();
		rwp_wait = gic_redist_wait_for_rwp;
	} else {
		base = gic_data.dist_base;
		rwp_wait = gic_dist_wait_for_rwp;
	}

	writel_relaxed(mask, base + offset + (gic_irq(d) / 32) * 4);
	rwp_wait();
}

static void gic_mask_irq(struct irq_data *d)
{
	gic_poke_irq(d, GICD_ICENABLER);
}

static void gic_eoimode1_mask_irq(struct irq_data *d)
{
	gic_mask_irq(d);
	/*
	 * When masking a forwarded interrupt, make sure it is
	 * deactivated as well.
	 *
	 * This ensures that an interrupt that is getting
	 * disabled/masked will not get "stuck", because there is
	 * noone to deactivate it (guest is being terminated).
	 */
	if (irqd_is_forwarded_to_vcpu(d))
		gic_poke_irq(d, GICD_ICACTIVER);
}

static void gic_unmask_irq(struct irq_data *d)
{
	gic_poke_irq(d, GICD_ISENABLER);
}

static int gic_irq_set_irqchip_state(struct irq_data *d,
				     enum irqchip_irq_state which, bool val)
{
	u32 reg;

	if (d->hwirq >= gic_data.irq_nr) /* PPI/SPI only */
		return -EINVAL;

	switch (which) {
	case IRQCHIP_STATE_PENDING:
		reg = val ? GICD_ISPENDR : GICD_ICPENDR;
		break;

	case IRQCHIP_STATE_ACTIVE:
		reg = val ? GICD_ISACTIVER : GICD_ICACTIVER;
		break;

	case IRQCHIP_STATE_MASKED:
		reg = val ? GICD_ICENABLER : GICD_ISENABLER;
		break;

	default:
		return -EINVAL;
	}

	gic_poke_irq(d, reg);
	return 0;
}

static int gic_irq_get_irqchip_state(struct irq_data *d,
				     enum irqchip_irq_state which, bool *val)
{
	if (d->hwirq >= gic_data.irq_nr) /* PPI/SPI only */
		return -EINVAL;

	switch (which) {
	case IRQCHIP_STATE_PENDING:
		*val = gic_peek_irq(d, GICD_ISPENDR);
		break;

	case IRQCHIP_STATE_ACTIVE:
		*val = gic_peek_irq(d, GICD_ISACTIVER);
		break;

	case IRQCHIP_STATE_MASKED:
		*val = !gic_peek_irq(d, GICD_ISENABLER);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static void gic_eoi_irq(struct irq_data *d)
{
	gic_write_eoir(gic_irq(d));
}

static void gic_eoimode1_eoi_irq(struct irq_data *d)
{
	/*
	 * No need to deactivate an LPI, or an interrupt that
	 * is is getting forwarded to a vcpu.
	 */
	if (gic_irq(d) >= 8192 || irqd_is_forwarded_to_vcpu(d))
		return;
	gic_write_dir(gic_irq(d));
}

static int gic_set_type(struct irq_data *d, unsigned int type)
{
	unsigned int irq = gic_irq(d);
	void (*rwp_wait)(void);
	void __iomem *base;

	/* Interrupt configuration for SGIs can't be changed */
	if (irq < 16)
		return -EINVAL;

	/* SPIs have restrictions on the supported types */
	if (irq >= 32 && type != IRQ_TYPE_LEVEL_HIGH &&
			 type != IRQ_TYPE_EDGE_RISING)
		return -EINVAL;

	if (gic_irq_in_rdist(d)) {
		base = gic_data_rdist_sgi_base();
		rwp_wait = gic_redist_wait_for_rwp;
	} else {
		base = gic_data.dist_base;
		rwp_wait = gic_dist_wait_for_rwp;
	}

	return gic_configure_irq(irq, type, base, rwp_wait);
}

static int gic_irq_set_vcpu_affinity(struct irq_data *d, void *vcpu)
{
	if (vcpu)
		irqd_set_forwarded_to_vcpu(d);
	else
		irqd_clr_forwarded_to_vcpu(d);
	return 0;
}

#ifdef CONFIG_PM

static int gic_suspend(void)
{
	return 0;
}

static void gic_show_resume_irq(struct gic_chip_data *gic)
{
	unsigned int i;
	u32 enabled;
	u32 pending[32];
	void __iomem *base = gic_data.dist_base;

	if (!msm_show_resume_irq_mask)
		return;

	for (i = 0; i * 32 < gic->irq_nr; i++) {
		enabled = readl_relaxed(base + GICD_ICENABLER + i * 4);
		pending[i] = readl_relaxed(base + GICD_ISPENDR + i * 4);
		pending[i] &= enabled;
	}

	for (i = find_first_bit((unsigned long *)pending, gic->irq_nr);
	     i < gic->irq_nr;
	     i = find_next_bit((unsigned long *)pending, gic->irq_nr, i+1)) {
		unsigned int irq = irq_find_mapping(gic->domain, i);
		struct irq_desc *desc = irq_to_desc(irq);
		const char *name = "null";

		if (desc == NULL)
			name = "stray irq";
		else if (desc->action && desc->action->name)
			name = desc->action->name;

		pr_warn("%s: %d triggered %s\n", __func__, irq, name);
	}
}

static void gic_resume_one(struct gic_chip_data *gic)
{
	gic_show_resume_irq(gic);
}

static void gic_resume(void)
{
	gic_resume_one(&gic_data);
}

static struct syscore_ops gic_syscore_ops = {
	.suspend = gic_suspend,
	.resume = gic_resume,
};

static int __init gic_init_sys(void)
{
	register_syscore_ops(&gic_syscore_ops);
	return 0;
}
arch_initcall(gic_init_sys);

#endif

static u64 gic_mpidr_to_affinity(unsigned long mpidr)
{
	u64 aff;

	aff = ((u64)MPIDR_AFFINITY_LEVEL(mpidr, 3) << 32 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 2) << 16 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 1) << 8  |
	       MPIDR_AFFINITY_LEVEL(mpidr, 0));

	return aff;
}

static asmlinkage void __exception_irq_entry gic_handle_irq(struct pt_regs *regs)
{
	u32 irqnr;

	do {
		irqnr = gic_read_iar();

		if (likely(irqnr > 15 && irqnr < 1020) || irqnr >= 8192) {
			int err;

			uncached_logk(LOGK_IRQ, (void *)(uintptr_t)irqnr);
			if (static_branch_likely(&supports_deactivate_key))
				gic_write_eoir(irqnr);
			else
				isb();

			err = handle_domain_irq(gic_data.domain, irqnr, regs);
			if (err) {
				WARN_ONCE(true, "Unexpected interrupt received!\n");
				if (static_branch_likely(&supports_deactivate_key)) {
					if (irqnr < 8192)
						gic_write_dir(irqnr);
				} else {
					gic_write_eoir(irqnr);
				}
			}
			continue;
		}
		if (irqnr < 16) {
			uncached_logk(LOGK_IRQ, (void *)(uintptr_t)irqnr);
			gic_write_eoir(irqnr);
			if (static_branch_likely(&supports_deactivate_key))
				gic_write_dir(irqnr);
#ifdef CONFIG_SMP
			/*
			 * Unlike GICv2, we don't need an smp_rmb() here.
			 * The control dependency from gic_read_iar to
			 * the ISB in gic_write_eoir is enough to ensure
			 * that any shared data read by handle_IPI will
			 * be read after the ACK.
			 */
			handle_IPI(irqnr, regs);
#else
			WARN_ONCE(true, "Unexpected SGI received!\n");
#endif
			continue;
		}
	} while (irqnr != ICC_IAR1_EL1_SPURIOUS);
}

static void __init gic_dist_init(void)
{
	unsigned int i;
	u64 affinity;
	void __iomem *base = gic_data.dist_base;

	/* Disable the distributor */
	writel_relaxed(0, base + GICD_CTLR);
	gic_dist_wait_for_rwp();

	/*
	 * Configure SPIs as non-secure Group-1. This will only matter
	 * if the GIC only has a single security state. This will not
	 * do the right thing if the kernel is running in secure mode,
	 * but that's not the intended use case anyway.
	 */
	for (i = 32; i < gic_data.irq_nr; i += 32)
		writel_relaxed(~0, base + GICD_IGROUPR + i / 8);

	gic_dist_config(base, gic_data.irq_nr, gic_dist_wait_for_rwp);

	/* Enable distributor with ARE, Group1 */
	writel_relaxed(GICD_CTLR_ARE_NS | GICD_CTLR_ENABLE_G1A | GICD_CTLR_ENABLE_G1,
		       base + GICD_CTLR);

	/*
	 * Set all global interrupts to the boot CPU only. ARE must be
	 * enabled.
	 */
	affinity = gic_mpidr_to_affinity(cpu_logical_map(smp_processor_id()));
	for (i = 32; i < gic_data.irq_nr; i++)
		gic_write_irouter(affinity, base + GICD_IROUTER + i * 8);
}

static int gic_iterate_rdists(int (*fn)(struct redist_region *, void __iomem *))
{
	int ret = -ENODEV;
	int i;

	for (i = 0; i < gic_data.nr_redist_regions; i++) {
		void __iomem *ptr = gic_data.redist_regions[i].redist_base;
		u64 typer;
		u32 reg;

		reg = readl_relaxed(ptr + GICR_PIDR2) & GIC_PIDR2_ARCH_MASK;
		if (reg != GIC_PIDR2_ARCH_GICv3 &&
		    reg != GIC_PIDR2_ARCH_GICv4) { /* We're in trouble... */
			pr_warn("No redistributor present @%p\n", ptr);
			break;
		}

		do {
			typer = gic_read_typer(ptr + GICR_TYPER);
			ret = fn(gic_data.redist_regions + i, ptr);
			if (!ret)
				return 0;

			if (gic_data.redist_regions[i].single_redist)
				break;

			if (gic_data.redist_stride) {
				ptr += gic_data.redist_stride;
			} else {
				ptr += SZ_64K * 2; /* Skip RD_base + SGI_base */
				if (typer & GICR_TYPER_VLPIS)
					ptr += SZ_64K * 2; /* Skip VLPI_base + reserved page */
			}
		} while (!(typer & GICR_TYPER_LAST));
	}

	return ret ? -ENODEV : 0;
}

static int __gic_populate_rdist(struct redist_region *region, void __iomem *ptr)
{
	unsigned long mpidr = cpu_logical_map(smp_processor_id());
	u64 typer;
	u32 aff;

	/*
	 * Convert affinity to a 32bit value that can be matched to
	 * GICR_TYPER bits [63:32].
	 */
	aff = (MPIDR_AFFINITY_LEVEL(mpidr, 3) << 24 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 2) << 16 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 1) << 8 |
	       MPIDR_AFFINITY_LEVEL(mpidr, 0));

	typer = gic_read_typer(ptr + GICR_TYPER);
	if ((typer >> 32) == aff) {
		u64 offset = ptr - region->redist_base;
		gic_data_rdist_rd_base() = ptr;
		gic_data_rdist()->phys_base = region->phys_base + offset;

		return 0;
	}

	/* Try next one */
	return 1;
}

static int gic_populate_rdist(void)
{
	if (gic_iterate_rdists(__gic_populate_rdist) == 0)
		return 0;

	/* We couldn't even deal with ourselves... */
	WARN(true, "CPU%d: mpidr %lx has no re-distributor!\n",
	     smp_processor_id(),
	     (unsigned long)cpu_logical_map(smp_processor_id()));
	return -ENODEV;
}

static int __gic_update_vlpi_properties(struct redist_region *region,
					void __iomem *ptr)
{
	u64 typer = gic_read_typer(ptr + GICR_TYPER);
	gic_data.rdists.has_vlpis &= !!(typer & GICR_TYPER_VLPIS);
	gic_data.rdists.has_direct_lpi &= !!(typer & GICR_TYPER_DirectLPIS);

	return 1;
}

static void gic_update_vlpi_properties(void)
{
	gic_iterate_rdists(__gic_update_vlpi_properties);
	pr_info("%sVLPI support, %sdirect LPI support\n",
		!gic_data.rdists.has_vlpis ? "no " : "",
		!gic_data.rdists.has_direct_lpi ? "no " : "");
}

static void gic_cpu_sys_reg_init(void)
{
	int i, cpu = smp_processor_id();
	u64 mpidr = cpu_logical_map(cpu);
	u64 need_rss = MPIDR_RS(mpidr);
	bool group0;
	u32 val, pribits;

	/*
	 * Need to check that the SRE bit has actually been set. If
	 * not, it means that SRE is disabled at EL2. We're going to
	 * die painfully, and there is nothing we can do about it.
	 *
	 * Kindly inform the luser.
	 */
	if (!gic_enable_sre())
		pr_err("GIC: unable to set SRE (disabled at EL2), panic ahead\n");

	pribits = gic_read_ctlr();
	pribits &= ICC_CTLR_EL1_PRI_BITS_MASK;
	pribits >>= ICC_CTLR_EL1_PRI_BITS_SHIFT;
	pribits++;

	/*
	 * Let's find out if Group0 is under control of EL3 or not by
	 * setting the highest possible, non-zero priority in PMR.
	 *
	 * If SCR_EL3.FIQ is set, the priority gets shifted down in
	 * order for the CPU interface to set bit 7, and keep the
	 * actual priority in the non-secure range. In the process, it
	 * looses the least significant bit and the actual priority
	 * becomes 0x80. Reading it back returns 0, indicating that
	 * we're don't have access to Group0.
	 */
	write_gicreg(BIT(8 - pribits), ICC_PMR_EL1);
	val = read_gicreg(ICC_PMR_EL1);
	group0 = val != 0;

	/* Set priority mask register */
	write_gicreg(DEFAULT_PMR_VALUE, ICC_PMR_EL1);

	/*
	 * Some firmwares hand over to the kernel with the BPR changed from
	 * its reset value (and with a value large enough to prevent
	 * any pre-emptive interrupts from working at all). Writing a zero
	 * to BPR restores is reset value.
	 */
	gic_write_bpr1(0);

	if (static_branch_likely(&supports_deactivate_key)) {
		/* EOI drops priority only (mode 1) */
		gic_write_ctlr(ICC_CTLR_EL1_EOImode_drop);
	} else {
		/* EOI deactivates interrupt too (mode 0) */
		gic_write_ctlr(ICC_CTLR_EL1_EOImode_drop_dir);
	}

	/* Always whack Group0 before Group1 */
	if (group0) {
		switch(pribits) {
		case 8:
		case 7:
			write_gicreg(0, ICC_AP0R3_EL1);
			write_gicreg(0, ICC_AP0R2_EL1);
		case 6:
			write_gicreg(0, ICC_AP0R1_EL1);
		case 5:
		case 4:
			write_gicreg(0, ICC_AP0R0_EL1);
		}

		isb();
	}

	switch(pribits) {
	case 8:
	case 7:
		write_gicreg(0, ICC_AP1R3_EL1);
		write_gicreg(0, ICC_AP1R2_EL1);
	case 6:
		write_gicreg(0, ICC_AP1R1_EL1);
	case 5:
	case 4:
		write_gicreg(0, ICC_AP1R0_EL1);
	}

	isb();

	/* ... and let's hit the road... */
	gic_write_grpen1(1);

	/* Keep the RSS capability status in per_cpu variable */
	per_cpu(has_rss, cpu) = !!(gic_read_ctlr() & ICC_CTLR_EL1_RSS);

	/* Check all the CPUs have capable of sending SGIs to other CPUs */
	for_each_online_cpu(i) {
		bool have_rss = per_cpu(has_rss, i) && per_cpu(has_rss, cpu);

		need_rss |= MPIDR_RS(cpu_logical_map(i));
		if (need_rss && (!have_rss))
			pr_crit("CPU%d (%lx) can't SGI CPU%d (%lx), no RSS\n",
				cpu, (unsigned long)mpidr,
				i, (unsigned long)cpu_logical_map(i));
	}

	/**
	 * GIC spec says, when ICC_CTLR_EL1.RSS==1 and GICD_TYPER.RSS==0,
	 * writing ICC_ASGI1R_EL1 register with RS != 0 is a CONSTRAINED
	 * UNPREDICTABLE choice of :
	 *   - The write is ignored.
	 *   - The RS field is treated as 0.
	 */
	if (need_rss && (!gic_data.has_rss))
		pr_crit_once("RSS is required but GICD doesn't support it\n");
}

static bool gicv3_nolpi;

static int __init gicv3_nolpi_cfg(char *buf)
{
	return strtobool(buf, &gicv3_nolpi);
}
early_param("irqchip.gicv3_nolpi", gicv3_nolpi_cfg);

static int gic_dist_supports_lpis(void)
{
	return !!(readl_relaxed(gic_data.dist_base + GICD_TYPER) & GICD_TYPER_LPIS) && !gicv3_nolpi;
}

static void gic_cpu_init(void)
{
	void __iomem *rbase;

	/* Register ourselves with the rest of the world */
	if (gic_populate_rdist())
		return;

	gic_enable_redist(true);

	rbase = gic_data_rdist_sgi_base();

	/* Configure SGIs/PPIs as non-secure Group-1 */
	writel_relaxed(~0, rbase + GICR_IGROUPR0);

	gic_cpu_config(rbase, gic_redist_wait_for_rwp);

	/* Give LPIs a spin */
	if (IS_ENABLED(CONFIG_ARM_GIC_V3_ITS) && gic_dist_supports_lpis() &&
					!IS_ENABLED(CONFIG_ARM_GIC_V3_ACL))
		its_cpu_init();

	/* initialise system registers */
	gic_cpu_sys_reg_init();
}

#ifdef CONFIG_SMP

#define MPIDR_TO_SGI_RS(mpidr)	(MPIDR_RS(mpidr) << ICC_SGI1R_RS_SHIFT)
#define MPIDR_TO_SGI_CLUSTER_ID(mpidr)	((mpidr) & ~0xFUL)

static int gic_starting_cpu(unsigned int cpu)
{
	gic_cpu_init();
	return 0;
}

static u16 gic_compute_target_list(int *base_cpu, const struct cpumask *mask,
				   unsigned long cluster_id)
{
	int next_cpu, cpu = *base_cpu;
	unsigned long mpidr = cpu_logical_map(cpu);
	u16 tlist = 0;

	while (cpu < nr_cpu_ids) {
		tlist |= 1 << (mpidr & 0xf);

		next_cpu = cpumask_next(cpu, mask);
		if (next_cpu >= nr_cpu_ids)
			goto out;
		cpu = next_cpu;

		mpidr = cpu_logical_map(cpu);

		if (cluster_id != MPIDR_TO_SGI_CLUSTER_ID(mpidr)) {
			cpu--;
			goto out;
		}
	}
out:
	*base_cpu = cpu;
	return tlist;
}

#define MPIDR_TO_SGI_AFFINITY(cluster_id, level) \
	(MPIDR_AFFINITY_LEVEL(cluster_id, level) \
		<< ICC_SGI1R_AFFINITY_## level ##_SHIFT)

static void gic_send_sgi(u64 cluster_id, u16 tlist, unsigned int irq)
{
	u64 val;

	val = (MPIDR_TO_SGI_AFFINITY(cluster_id, 3)	|
	       MPIDR_TO_SGI_AFFINITY(cluster_id, 2)	|
	       irq << ICC_SGI1R_SGI_ID_SHIFT		|
	       MPIDR_TO_SGI_AFFINITY(cluster_id, 1)	|
	       MPIDR_TO_SGI_RS(cluster_id)		|
	       tlist << ICC_SGI1R_TARGET_LIST_SHIFT);

	pr_devel("CPU%d: ICC_SGI1R_EL1 %llx\n", smp_processor_id(), val);
	gic_write_sgi1r(val);
}

static void gic_raise_softirq(const struct cpumask *mask, unsigned int irq)
{
	int cpu;

	if (WARN_ON(irq >= 16))
		return;

	/*
	 * Ensure that stores to Normal memory are visible to the
	 * other CPUs before issuing the IPI.
	 */
	wmb();

	for_each_cpu(cpu, mask) {
		u64 cluster_id = MPIDR_TO_SGI_CLUSTER_ID(cpu_logical_map(cpu));
		u16 tlist;

		tlist = gic_compute_target_list(&cpu, mask, cluster_id);
		gic_send_sgi(cluster_id, tlist, irq);
	}

	/* Force the above writes to ICC_SGI1R_EL1 to be executed */
	isb();
}

static void gic_smp_init(void)
{
	set_smp_cross_call(gic_raise_softirq);
	cpuhp_setup_state_nocalls(CPUHP_AP_IRQ_GIC_STARTING,
				  "irqchip/arm/gicv3:starting",
				  gic_starting_cpu, NULL);
}

static int gic_set_affinity(struct irq_data *d, const struct cpumask *mask_val,
			    bool force)
{
	unsigned int cpu;
	void __iomem *reg;
	int enabled;
	u64 val;

	if (force)
		cpu = cpumask_first(mask_val);
	else
		cpu = cpumask_any_and(mask_val, cpu_online_mask);

	if (cpu >= nr_cpu_ids)
		return -EINVAL;

	if (gic_irq_in_rdist(d))
		return -EINVAL;

	/* If interrupt was enabled, disable it first */
	enabled = gic_peek_irq(d, GICD_ISENABLER);
	if (enabled)
		gic_mask_irq(d);

	reg = gic_dist_base(d) + GICD_IROUTER + (gic_irq(d) * 8);
	val = gic_mpidr_to_affinity(cpu_logical_map(cpu));

	gic_write_irouter(val, reg);

	/*
	 * If the interrupt was enabled, enabled it again. Otherwise,
	 * just wait for the distributor to have digested our changes.
	 */
	if (enabled)
		gic_unmask_irq(d);
	else
		gic_dist_wait_for_rwp();

	irq_data_update_effective_affinity(d, cpumask_of(cpu));

	return IRQ_SET_MASK_OK_DONE;
}
#else
#define gic_set_affinity	NULL
#define gic_smp_init()		do { } while(0)
#endif

#ifdef CONFIG_CPU_PM
/* Check whether it's single security state view */
static bool gic_dist_security_disabled(void)
{
	return readl_relaxed(gic_data.dist_base + GICD_CTLR) & GICD_CTLR_DS;
}

static int gic_cpu_pm_notifier(struct notifier_block *self,
			       unsigned long cmd, void *v)
{
	if (from_suspend)
		return NOTIFY_OK;

	if (cmd == CPU_PM_EXIT) {
		if (gic_dist_security_disabled())
			gic_enable_redist(true);
		gic_cpu_sys_reg_init();
	} else if (cmd == CPU_PM_ENTER && gic_dist_security_disabled()) {
		gic_write_grpen1(0);
		gic_enable_redist(false);
	}
	return NOTIFY_OK;
}

static struct notifier_block gic_cpu_pm_notifier_block = {
	.notifier_call = gic_cpu_pm_notifier,
};

static void gic_cpu_pm_init(void)
{
	cpu_pm_register_notifier(&gic_cpu_pm_notifier_block);
}

#else
static inline void gic_cpu_pm_init(void) { }
#endif /* CONFIG_CPU_PM */

static struct irq_chip gic_chip = {
	.name			= "GICv3",
	.irq_mask		= gic_mask_irq,
	.irq_unmask		= gic_unmask_irq,
	.irq_eoi		= gic_eoi_irq,
	.irq_set_type		= gic_set_type,
	.irq_set_affinity	= gic_set_affinity,
	.irq_get_irqchip_state	= gic_irq_get_irqchip_state,
	.irq_set_irqchip_state	= gic_irq_set_irqchip_state,
	.flags			= IRQCHIP_SET_TYPE_MASKED |
				  IRQCHIP_SKIP_SET_WAKE |
				  IRQCHIP_MASK_ON_SUSPEND,
};

static struct irq_chip gic_eoimode1_chip = {
	.name			= "GICv3",
	.irq_mask		= gic_eoimode1_mask_irq,
	.irq_unmask		= gic_unmask_irq,
	.irq_eoi		= gic_eoimode1_eoi_irq,
	.irq_set_type		= gic_set_type,
	.irq_set_affinity	= gic_set_affinity,
	.irq_get_irqchip_state	= gic_irq_get_irqchip_state,
	.irq_set_irqchip_state	= gic_irq_set_irqchip_state,
	.irq_set_vcpu_affinity	= gic_irq_set_vcpu_affinity,
	.flags			= IRQCHIP_SET_TYPE_MASKED |
				  IRQCHIP_SKIP_SET_WAKE |
				  IRQCHIP_MASK_ON_SUSPEND,
};

#define GIC_ID_NR	(1U << GICD_TYPER_ID_BITS(gic_data.rdists.gicd_typer))

static int gic_irq_domain_map(struct irq_domain *d, unsigned int irq,
			      irq_hw_number_t hw)
{
	struct irq_chip *chip = &gic_chip;

	if (static_branch_likely(&supports_deactivate_key))
		chip = &gic_eoimode1_chip;

	/* SGIs are private to the core kernel */
	if (hw < 16)
		return -EPERM;
	/* Nothing here */
	if (hw >= gic_data.irq_nr && hw < 8192)
		return -EPERM;
	/* Off limits */
	if (hw >= GIC_ID_NR)
		return -EPERM;

	/* PPIs */
	if (hw < 32) {
		irq_set_percpu_devid(irq);
		irq_domain_set_info(d, irq, hw, chip, d->host_data,
				    handle_percpu_devid_irq, NULL, NULL);
		irq_set_status_flags(irq, IRQ_NOAUTOEN);
	}
	/* SPIs */
	if (hw >= 32 && hw < gic_data.irq_nr) {
		irq_domain_set_info(d, irq, hw, chip, d->host_data,
				    handle_fasteoi_irq, NULL, NULL);
		irq_set_probe(irq);
		irqd_set_single_target(irq_desc_get_irq_data(irq_to_desc(irq)));
	}
	/* LPIs */
	if (hw >= 8192 && hw < GIC_ID_NR) {
		if (!gic_dist_supports_lpis())
			return -EPERM;
		irq_domain_set_info(d, irq, hw, chip, d->host_data,
				    handle_fasteoi_irq, NULL, NULL);
	}

	return 0;
}

#define GIC_IRQ_TYPE_PARTITION	(GIC_IRQ_TYPE_LPI + 1)

static int gic_irq_domain_translate(struct irq_domain *d,
				    struct irq_fwspec *fwspec,
				    unsigned long *hwirq,
				    unsigned int *type)
{
	if (is_of_node(fwspec->fwnode)) {
		if (fwspec->param_count < 3)
			return -EINVAL;

		switch (fwspec->param[0]) {
		case 0:			/* SPI */
			*hwirq = fwspec->param[1] + 32;
			break;
		case 1:			/* PPI */
		case GIC_IRQ_TYPE_PARTITION:
			*hwirq = fwspec->param[1] + 16;
			break;
		case GIC_IRQ_TYPE_LPI:	/* LPI */
			*hwirq = fwspec->param[1];
			break;
		default:
			return -EINVAL;
		}

		*type = fwspec->param[2] & IRQ_TYPE_SENSE_MASK;

		/*
		 * Make it clear that broken DTs are... broken.
		 * Partitionned PPIs are an unfortunate exception.
		 */
		WARN_ON(*type == IRQ_TYPE_NONE &&
			fwspec->param[0] != GIC_IRQ_TYPE_PARTITION);
		return 0;
	}

	if (is_fwnode_irqchip(fwspec->fwnode)) {
		if(fwspec->param_count != 2)
			return -EINVAL;

		*hwirq = fwspec->param[0];
		*type = fwspec->param[1];

		WARN_ON(*type == IRQ_TYPE_NONE);
		return 0;
	}

	return -EINVAL;
}

static int gic_irq_domain_alloc(struct irq_domain *domain, unsigned int virq,
				unsigned int nr_irqs, void *arg)
{
	int i, ret;
	irq_hw_number_t hwirq;
	unsigned int type = IRQ_TYPE_NONE;
	struct irq_fwspec *fwspec = arg;

	ret = gic_irq_domain_translate(domain, fwspec, &hwirq, &type);
	if (ret)
		return ret;

	for (i = 0; i < nr_irqs; i++) {
		ret = gic_irq_domain_map(domain, virq + i, hwirq + i);
		if (ret)
			return ret;
	}

	return 0;
}

static void gic_irq_domain_free(struct irq_domain *domain, unsigned int virq,
				unsigned int nr_irqs)
{
	int i;

	for (i = 0; i < nr_irqs; i++) {
		struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
		irq_set_handler(virq + i, NULL);
		irq_domain_reset_irq_data(d);
	}
}

static int gic_irq_domain_select(struct irq_domain *d,
				 struct irq_fwspec *fwspec,
				 enum irq_domain_bus_token bus_token)
{
	/* Not for us */
        if (fwspec->fwnode != d->fwnode)
		return 0;

	/* If this is not DT, then we have a single domain */
	if (!is_of_node(fwspec->fwnode))
		return 1;

	/*
	 * If this is a PPI and we have a 4th (non-null) parameter,
	 * then we need to match the partition domain.
	 */
	if (fwspec->param_count >= 4 &&
	    fwspec->param[0] == 1 && fwspec->param[3] != 0)
		return d == partition_get_domain(gic_data.ppi_descs[fwspec->param[1]]);

	return d == gic_data.domain;
}

static const struct irq_domain_ops gic_irq_domain_ops = {
	.translate = gic_irq_domain_translate,
	.alloc = gic_irq_domain_alloc,
	.free = gic_irq_domain_free,
	.select = gic_irq_domain_select,
};

static int partition_domain_translate(struct irq_domain *d,
				      struct irq_fwspec *fwspec,
				      unsigned long *hwirq,
				      unsigned int *type)
{
	struct device_node *np;
	int ret;

	np = of_find_node_by_phandle(fwspec->param[3]);
	if (WARN_ON(!np))
		return -EINVAL;

	ret = partition_translate_id(gic_data.ppi_descs[fwspec->param[1]],
				     of_node_to_fwnode(np));
	if (ret < 0)
		return ret;

	*hwirq = ret;
	*type = fwspec->param[2] & IRQ_TYPE_SENSE_MASK;

	return 0;
}

static const struct irq_domain_ops partition_domain_ops = {
	.translate = partition_domain_translate,
	.select = gic_irq_domain_select,
};

static int __init gic_init_bases(void __iomem *dist_base,
				 struct redist_region *rdist_regs,
				 u32 nr_redist_regions,
				 u64 redist_stride,
				 struct fwnode_handle *handle)
{
	u32 typer;
	int gic_irqs;
	int err;

	if (!is_hyp_mode_available())
		static_branch_disable(&supports_deactivate_key);

	if (static_branch_likely(&supports_deactivate_key))
		pr_info("GIC: Using split EOI/Deactivate mode\n");

	gic_data.fwnode = handle;
	gic_data.dist_base = dist_base;
	gic_data.redist_regions = rdist_regs;
	gic_data.nr_redist_regions = nr_redist_regions;
	gic_data.redist_stride = redist_stride;

	/*
	 * Find out how many interrupts are supported.
	 * The GIC only supports up to 1020 interrupt sources (SGI+PPI+SPI)
	 */
	typer = readl_relaxed(gic_data.dist_base + GICD_TYPER);
	gic_data.rdists.gicd_typer = typer;
	gic_irqs = GICD_TYPER_IRQS(typer);
	if (gic_irqs > 1020)
		gic_irqs = 1020;
	gic_data.irq_nr = gic_irqs;

	gic_data.domain = irq_domain_create_tree(handle, &gic_irq_domain_ops,
						 &gic_data);
	irq_domain_update_bus_token(gic_data.domain, DOMAIN_BUS_WIRED);
	gic_data.rdists.rdist = alloc_percpu(typeof(*gic_data.rdists.rdist));
	gic_data.rdists.has_vlpis = true;
	gic_data.rdists.has_direct_lpi = true;

	if (WARN_ON(!gic_data.domain) || WARN_ON(!gic_data.rdists.rdist)) {
		err = -ENOMEM;
		goto out_free;
	}

	gic_data.has_rss = !!(typer & GICD_TYPER_RSS);
	pr_info("Distributor has %sRange Selector support\n",
		gic_data.has_rss ? "" : "no ");

	if (typer & GICD_TYPER_MBIS) {
		err = mbi_init(handle, gic_data.domain);
		if (err)
			pr_err("Failed to initialize MBIs\n");
	}

	set_handle_irq(gic_handle_irq);

	gic_update_vlpi_properties();

	if (IS_ENABLED(CONFIG_ARM_GIC_V3_ITS) && gic_dist_supports_lpis() &&
			!IS_ENABLED(CONFIG_ARM_GIC_V3_ACL))
		its_init(handle, &gic_data.rdists, gic_data.domain);

	gic_smp_init();
	gic_dist_init();
	gic_cpu_init();
	gic_cpu_pm_init();

	return 0;

out_free:
	if (gic_data.domain)
		irq_domain_remove(gic_data.domain);
	free_percpu(gic_data.rdists.rdist);
	return err;
}

static int __init gic_validate_dist_version(void __iomem *dist_base)
{
	u32 reg = readl_relaxed(dist_base + GICD_PIDR2) & GIC_PIDR2_ARCH_MASK;

	if (reg != GIC_PIDR2_ARCH_GICv3 && reg != GIC_PIDR2_ARCH_GICv4)
		return -ENODEV;

	return 0;
}

/* Create all possible partitions at boot time */
static void __init gic_populate_ppi_partitions(struct device_node *gic_node)
{
	struct device_node *parts_node, *child_part;
	int part_idx = 0, i;
	int nr_parts;
	struct partition_affinity *parts;

	parts_node = of_get_child_by_name(gic_node, "ppi-partitions");
	if (!parts_node)
		return;

	nr_parts = of_get_child_count(parts_node);

	if (!nr_parts)
		goto out_put_node;

	parts = kcalloc(nr_parts, sizeof(*parts), GFP_KERNEL);
	if (WARN_ON(!parts))
		goto out_put_node;

	for_each_child_of_node(parts_node, child_part) {
		struct partition_affinity *part;
		int n;

		part = &parts[part_idx];

		part->partition_id = of_node_to_fwnode(child_part);

		pr_info("GIC: PPI partition %s[%d] { ",
			child_part->name, part_idx);

		n = of_property_count_elems_of_size(child_part, "affinity",
						    sizeof(u32));
		WARN_ON(n <= 0);

		for (i = 0; i < n; i++) {
			int err, cpu;
			u32 cpu_phandle;
			struct device_node *cpu_node;

			err = of_property_read_u32_index(child_part, "affinity",
							 i, &cpu_phandle);
			if (WARN_ON(err))
				continue;

			cpu_node = of_find_node_by_phandle(cpu_phandle);
			if (WARN_ON(!cpu_node))
				continue;

			cpu = of_cpu_node_to_id(cpu_node);
			if (WARN_ON(cpu < 0))
				continue;

			pr_cont("%pOF[%d] ", cpu_node, cpu);

			cpumask_set_cpu(cpu, &part->mask);
		}

		pr_cont("}\n");
		part_idx++;
	}

	for (i = 0; i < 16; i++) {
		unsigned int irq;
		struct partition_desc *desc;
		struct irq_fwspec ppi_fwspec = {
			.fwnode		= gic_data.fwnode,
			.param_count	= 3,
			.param		= {
				[0]	= GIC_IRQ_TYPE_PARTITION,
				[1]	= i,
				[2]	= IRQ_TYPE_NONE,
			},
		};

		irq = irq_create_fwspec_mapping(&ppi_fwspec);
		if (WARN_ON(!irq))
			continue;
		desc = partition_create_desc(gic_data.fwnode, parts, nr_parts,
					     irq, &partition_domain_ops);
		if (WARN_ON(!desc))
			continue;

		gic_data.ppi_descs[i] = desc;
	}

out_put_node:
	of_node_put(parts_node);
}

static void __init gic_of_setup_kvm_info(struct device_node *node)
{
	int ret;
	struct resource r;
	u32 gicv_idx;

	gic_v3_kvm_info.type = GIC_V3;

	gic_v3_kvm_info.maint_irq = irq_of_parse_and_map(node, 0);
	if (!gic_v3_kvm_info.maint_irq)
		return;

	if (of_property_read_u32(node, "#redistributor-regions",
				 &gicv_idx))
		gicv_idx = 1;

	gicv_idx += 3;	/* Also skip GICD, GICC, GICH */
	ret = of_address_to_resource(node, gicv_idx, &r);
	if (!ret)
		gic_v3_kvm_info.vcpu = r;

	gic_v3_kvm_info.has_v4 = gic_data.rdists.has_vlpis;
	gic_set_kvm_info(&gic_v3_kvm_info);
}

static int __init gicv3_of_init(struct device_node *node, struct device_node *parent)
{
	void __iomem *dist_base;
	struct redist_region *rdist_regs;
	u64 redist_stride;
	u32 nr_redist_regions;
	int err, i, ignore_irqs_len;
	u32 ignore_restore_irqs[MAX_IRQS_IGNORE] = {0};

	dist_base = of_iomap(node, 0);
	if (!dist_base) {
		pr_err("%pOF: unable to map gic dist registers\n", node);
		return -ENXIO;
	}

	err = gic_validate_dist_version(dist_base);
	if (err) {
		pr_err("%pOF: no distributor detected, giving up\n", node);
		goto out_unmap_dist;
	}

	if (of_property_read_u32(node, "#redistributor-regions", &nr_redist_regions))
		nr_redist_regions = 1;

	rdist_regs = kcalloc(nr_redist_regions, sizeof(*rdist_regs),
			     GFP_KERNEL);
	if (!rdist_regs) {
		err = -ENOMEM;
		goto out_unmap_dist;
	}

	for (i = 0; i < nr_redist_regions; i++) {
		struct resource res;
		int ret;

		ret = of_address_to_resource(node, 1 + i, &res);
		rdist_regs[i].redist_base = of_iomap(node, 1 + i);
		if (ret || !rdist_regs[i].redist_base) {
			pr_err("%pOF: couldn't map region %d\n", node, i);
			err = -ENODEV;
			goto out_unmap_rdist;
		}
		rdist_regs[i].phys_base = res.start;
	}

	if (of_property_read_u64(node, "redistributor-stride", &redist_stride))
		redist_stride = 0;

	err = gic_init_bases(dist_base, rdist_regs, nr_redist_regions,
			     redist_stride, &node->fwnode);
	if (err)
		goto out_unmap_rdist;

	gic_populate_ppi_partitions(node);

	if (static_branch_likely(&supports_deactivate_key))
		gic_of_setup_kvm_info(node);

	ignore_irqs_len = of_property_read_variable_u32_array(node,
				"ignored-save-restore-irqs",
				ignore_restore_irqs,
				0, MAX_IRQS_IGNORE);
	for (i = 0; i < ignore_irqs_len; i++)
		set_bit(ignore_restore_irqs[i], irqs_ignore_restore);

	return 0;

out_unmap_rdist:
	for (i = 0; i < nr_redist_regions; i++)
		if (rdist_regs[i].redist_base)
			iounmap(rdist_regs[i].redist_base);
	kfree(rdist_regs);
out_unmap_dist:
	iounmap(dist_base);
	return err;
}

IRQCHIP_DECLARE(gic_v3, "arm,gic-v3", gicv3_of_init);

#ifdef CONFIG_ACPI
static struct
{
	void __iomem *dist_base;
	struct redist_region *redist_regs;
	u32 nr_redist_regions;
	bool single_redist;
	int enabled_rdists;
	u32 maint_irq;
	int maint_irq_mode;
	phys_addr_t vcpu_base;
} acpi_data __initdata;

static void __init
gic_acpi_register_redist(phys_addr_t phys_base, void __iomem *redist_base)
{
	static int count = 0;

	acpi_data.redist_regs[count].phys_base = phys_base;
	acpi_data.redist_regs[count].redist_base = redist_base;
	acpi_data.redist_regs[count].single_redist = acpi_data.single_redist;
	count++;
}

static int __init
gic_acpi_parse_madt_redist(struct acpi_subtable_header *header,
			   const unsigned long end)
{
	struct acpi_madt_generic_redistributor *redist =
			(struct acpi_madt_generic_redistributor *)header;
	void __iomem *redist_base;

	redist_base = ioremap(redist->base_address, redist->length);
	if (!redist_base) {
		pr_err("Couldn't map GICR region @%llx\n", redist->base_address);
		return -ENOMEM;
	}

	gic_acpi_register_redist(redist->base_address, redist_base);
	return 0;
}

static int __init
gic_acpi_parse_madt_gicc(struct acpi_subtable_header *header,
			 const unsigned long end)
{
	struct acpi_madt_generic_interrupt *gicc =
				(struct acpi_madt_generic_interrupt *)header;
	u32 reg = readl_relaxed(acpi_data.dist_base + GICD_PIDR2) & GIC_PIDR2_ARCH_MASK;
	u32 size = reg == GIC_PIDR2_ARCH_GICv4 ? SZ_64K * 4 : SZ_64K * 2;
	void __iomem *redist_base;

	/* GICC entry which has !ACPI_MADT_ENABLED is not unusable so skip */
	if (!(gicc->flags & ACPI_MADT_ENABLED))
		return 0;

	redist_base = ioremap(gicc->gicr_base_address, size);
	if (!redist_base)
		return -ENOMEM;

	gic_acpi_register_redist(gicc->gicr_base_address, redist_base);
	return 0;
}

static int __init gic_acpi_collect_gicr_base(void)
{
	acpi_tbl_entry_handler redist_parser;
	enum acpi_madt_type type;

	if (acpi_data.single_redist) {
		type = ACPI_MADT_TYPE_GENERIC_INTERRUPT;
		redist_parser = gic_acpi_parse_madt_gicc;
	} else {
		type = ACPI_MADT_TYPE_GENERIC_REDISTRIBUTOR;
		redist_parser = gic_acpi_parse_madt_redist;
	}

	/* Collect redistributor base addresses in GICR entries */
	if (acpi_table_parse_madt(type, redist_parser, 0) > 0)
		return 0;

	pr_info("No valid GICR entries exist\n");
	return -ENODEV;
}

static int __init gic_acpi_match_gicr(struct acpi_subtable_header *header,
				  const unsigned long end)
{
	/* Subtable presence means that redist exists, that's it */
	return 0;
}

static int __init gic_acpi_match_gicc(struct acpi_subtable_header *header,
				      const unsigned long end)
{
	struct acpi_madt_generic_interrupt *gicc =
				(struct acpi_madt_generic_interrupt *)header;

	/*
	 * If GICC is enabled and has valid gicr base address, then it means
	 * GICR base is presented via GICC
	 */
	if ((gicc->flags & ACPI_MADT_ENABLED) && gicc->gicr_base_address) {
		acpi_data.enabled_rdists++;
		return 0;
	}

	/*
	 * It's perfectly valid firmware can pass disabled GICC entry, driver
	 * should not treat as errors, skip the entry instead of probe fail.
	 */
	if (!(gicc->flags & ACPI_MADT_ENABLED))
		return 0;

	return -ENODEV;
}

static int __init gic_acpi_count_gicr_regions(void)
{
	int count;

	/*
	 * Count how many redistributor regions we have. It is not allowed
	 * to mix redistributor description, GICR and GICC subtables have to be
	 * mutually exclusive.
	 */
	count = acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_REDISTRIBUTOR,
				      gic_acpi_match_gicr, 0);
	if (count > 0) {
		acpi_data.single_redist = false;
		return count;
	}

	count = acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_INTERRUPT,
				      gic_acpi_match_gicc, 0);
	if (count > 0) {
		acpi_data.single_redist = true;
		count = acpi_data.enabled_rdists;
	}

	return count;
}

static bool __init acpi_validate_gic_table(struct acpi_subtable_header *header,
					   struct acpi_probe_entry *ape)
{
	struct acpi_madt_generic_distributor *dist;
	int count;

	dist = (struct acpi_madt_generic_distributor *)header;
	if (dist->version != ape->driver_data)
		return false;

	/* We need to do that exercise anyway, the sooner the better */
	count = gic_acpi_count_gicr_regions();
	if (count <= 0)
		return false;

	acpi_data.nr_redist_regions = count;
	return true;
}

static int __init gic_acpi_parse_virt_madt_gicc(struct acpi_subtable_header *header,
						const unsigned long end)
{
	struct acpi_madt_generic_interrupt *gicc =
		(struct acpi_madt_generic_interrupt *)header;
	int maint_irq_mode;
	static int first_madt = true;

	/* Skip unusable CPUs */
	if (!(gicc->flags & ACPI_MADT_ENABLED))
		return 0;

	maint_irq_mode = (gicc->flags & ACPI_MADT_VGIC_IRQ_MODE) ?
		ACPI_EDGE_SENSITIVE : ACPI_LEVEL_SENSITIVE;

	if (first_madt) {
		first_madt = false;

		acpi_data.maint_irq = gicc->vgic_interrupt;
		acpi_data.maint_irq_mode = maint_irq_mode;
		acpi_data.vcpu_base = gicc->gicv_base_address;

		return 0;
	}

	/*
	 * The maintenance interrupt and GICV should be the same for every CPU
	 */
	if ((acpi_data.maint_irq != gicc->vgic_interrupt) ||
	    (acpi_data.maint_irq_mode != maint_irq_mode) ||
	    (acpi_data.vcpu_base != gicc->gicv_base_address))
		return -EINVAL;

	return 0;
}

static bool __init gic_acpi_collect_virt_info(void)
{
	int count;

	count = acpi_table_parse_madt(ACPI_MADT_TYPE_GENERIC_INTERRUPT,
				      gic_acpi_parse_virt_madt_gicc, 0);

	return (count > 0);
}

#define ACPI_GICV3_DIST_MEM_SIZE (SZ_64K)
#define ACPI_GICV2_VCTRL_MEM_SIZE	(SZ_4K)
#define ACPI_GICV2_VCPU_MEM_SIZE	(SZ_8K)

static void __init gic_acpi_setup_kvm_info(void)
{
	int irq;

	if (!gic_acpi_collect_virt_info()) {
		pr_warn("Unable to get hardware information used for virtualization\n");
		return;
	}

	gic_v3_kvm_info.type = GIC_V3;

	irq = acpi_register_gsi(NULL, acpi_data.maint_irq,
				acpi_data.maint_irq_mode,
				ACPI_ACTIVE_HIGH);
	if (irq <= 0)
		return;

	gic_v3_kvm_info.maint_irq = irq;

	if (acpi_data.vcpu_base) {
		struct resource *vcpu = &gic_v3_kvm_info.vcpu;

		vcpu->flags = IORESOURCE_MEM;
		vcpu->start = acpi_data.vcpu_base;
		vcpu->end = vcpu->start + ACPI_GICV2_VCPU_MEM_SIZE - 1;
	}

	gic_v3_kvm_info.has_v4 = gic_data.rdists.has_vlpis;
	gic_set_kvm_info(&gic_v3_kvm_info);
}

static int __init
gic_acpi_init(struct acpi_subtable_header *header, const unsigned long end)
{
	struct acpi_madt_generic_distributor *dist;
	struct fwnode_handle *domain_handle;
	size_t size;
	int i, err;

	/* Get distributor base address */
	dist = (struct acpi_madt_generic_distributor *)header;
	acpi_data.dist_base = ioremap(dist->base_address,
				      ACPI_GICV3_DIST_MEM_SIZE);
	if (!acpi_data.dist_base) {
		pr_err("Unable to map GICD registers\n");
		return -ENOMEM;
	}

	err = gic_validate_dist_version(acpi_data.dist_base);
	if (err) {
		pr_err("No distributor detected at @%p, giving up\n",
		       acpi_data.dist_base);
		goto out_dist_unmap;
	}

	size = sizeof(*acpi_data.redist_regs) * acpi_data.nr_redist_regions;
	acpi_data.redist_regs = kzalloc(size, GFP_KERNEL);
	if (!acpi_data.redist_regs) {
		err = -ENOMEM;
		goto out_dist_unmap;
	}

	err = gic_acpi_collect_gicr_base();
	if (err)
		goto out_redist_unmap;

	domain_handle = irq_domain_alloc_fwnode(acpi_data.dist_base);
	if (!domain_handle) {
		err = -ENOMEM;
		goto out_redist_unmap;
	}

	err = gic_init_bases(acpi_data.dist_base, acpi_data.redist_regs,
			     acpi_data.nr_redist_regions, 0, domain_handle);
	if (err)
		goto out_fwhandle_free;

	acpi_set_irq_model(ACPI_IRQ_MODEL_GIC, domain_handle);

	if (static_branch_likely(&supports_deactivate_key))
		gic_acpi_setup_kvm_info();

	return 0;

out_fwhandle_free:
	irq_domain_free_fwnode(domain_handle);
out_redist_unmap:
	for (i = 0; i < acpi_data.nr_redist_regions; i++)
		if (acpi_data.redist_regs[i].redist_base)
			iounmap(acpi_data.redist_regs[i].redist_base);
	kfree(acpi_data.redist_regs);
out_dist_unmap:
	iounmap(acpi_data.dist_base);
	return err;
}
IRQCHIP_ACPI_DECLARE(gic_v3, ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR,
		     acpi_validate_gic_table, ACPI_MADT_GIC_VERSION_V3,
		     gic_acpi_init);
IRQCHIP_ACPI_DECLARE(gic_v4, ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR,
		     acpi_validate_gic_table, ACPI_MADT_GIC_VERSION_V4,
		     gic_acpi_init);
IRQCHIP_ACPI_DECLARE(gic_v3_or_v4, ACPI_MADT_TYPE_GENERIC_DISTRIBUTOR,
		     acpi_validate_gic_table, ACPI_MADT_GIC_VERSION_NONE,
		     gic_acpi_init);
#endif
