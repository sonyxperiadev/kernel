/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/plat-bcmap/irq.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
 * @file
 * @brief Broadcom specific interrupt controller implementation

 * Based on common implementation for ARM VIC

	@defgroup InterruptAPIGroup Interrupt API's
	@brief defines the Broadcom specific interrupt API's
*/
#include <linux/init.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/module.h>

#include <asm/mach/irq.h>
#include <mach/irqs.h>
#include <mach/io_map.h>
#include <mach/memory.h>
/* for BINTC register offsets */
#include <mach/rdb/brcm_rdb_bintc.h>


#include <mach/rdb/brcm_rdb_chipreg.h>

void ipc_set_interrupt_mask(void);


#define INTC_IMR             0x0000
#define INTC_ISR             0x0004
#define INTC_ICR             0x0008
#define INTC_IMSR            0x000c
#define INTC_ISTCR(irq)      (0x0010 + (irq >> 3) * 4)
#define INTC_ICPR            0x0020
#define INTC_ICCR            0x0024
#define INTC_ISELR           0x0028
/*#define INTC_SWIR(x)         (x < 64 ? 0x0030 : 0x0024) */
#define INTC_SICR(x)         (x < 64 ? 0x0034 : 0x0028)
#define INTC_ARM9_SLEEP      0x0038

/* **FIXME** applicable to Rhea/BI; only for BI bup */
/* extracted from Rhea chip_irq.h */
#define NUM_KONAIRQs          224
#define LAST_KONAIRQ          (NUM_KONAIRQs-1)
#define FIRST_BMIRQ           (LAST_KONAIRQ+1)
#define NUM_BMIRQs            56
#define FIRST_BMIRQ           (LAST_KONAIRQ+1)
#define IRQ_TO_BMIRQ(irq)         ((irq)-FIRST_BMIRQ)

/* **FIXME** MAG - use offsets from Rhea RDB... */
#define INTC_SWIR(x)         (x < 64 ? 0x0030 : 0x0020)

enum interrupt_source_type {
	ACTIVE_HIGH = 0x00,
	ACTIVE_LOW = 0x01,
	RISING_EDGE_TRIGGER = 0x04,
	FALLING_EDGE_TRIGGER = 0x05,
	EITHER_EDGE_TRIGGER = 0x06,
};

static DEFINE_SPINLOCK(intc_lock);




static void intc_ack_irq(struct irq_data *d)
{
	unsigned int irq = d->irq;
	void __iomem *base = irq_get_chip_data(irq);
	writel(1 << (irq & 31), base + INTC_ICR);
	/* moreover, clear the soft-triggered, in case it was the reason */
	writel(1 << (irq & 31), base + INTC_SICR(irq));
}

static void intc_mask_irq(struct irq_data *d)
{
	unsigned int irq = d->irq;
	void __iomem *base = irq_get_chip_data(irq);
	irq &= 31;
	writel(readl(base + INTC_IMR) & ~(1 << irq), base + INTC_IMR);
}

static void intc_unmask_irq(struct irq_data *d)
{
	unsigned int irq = d->irq;
	void __iomem *base = irq_get_chip_data(irq);
	irq &= 31;
	writel(readl(base + INTC_IMR) | (1 << irq), base + INTC_IMR);
}

static int intc_set_type(struct irq_data *d, unsigned int flow_type)
{
	unsigned int irq = d->irq;
	void __iomem *base = irq_get_chip_data(irq);
	unsigned int type = 0, val;

	irq &= 31;
	switch (flow_type) {
	case IRQ_TYPE_LEVEL_HIGH:
		type = ACTIVE_HIGH;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		type = ACTIVE_LOW;
		break;
	case IRQ_TYPE_EDGE_RISING:
		type = RISING_EDGE_TRIGGER;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		type = FALLING_EDGE_TRIGGER;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		type = EITHER_EDGE_TRIGGER;
		break;
	default:
		return -EINVAL;
	}

	val = readl(base + INTC_ISTCR(irq));
	type &= 0x7;
	val &= ~(0x7 << ((irq & 0x7) << 2));
	val |= type << ((irq & 0x7) << 2);
	writel(val, base + INTC_ISTCR(irq));

	return 0;
}

void intc_trigger_softirq(unsigned int irq)
{

    void __iomem *chipreg_base = (void __iomem *)(KONA_CHIPREG_VA);
	unsigned int birq = IRQ_TO_BMIRQ(IRQ_IPC_A2C);

	/* convert to BModem IRQ */

	unsigned long flags;
	unsigned int ReadData=0;;
	/* removed printouts */
	/* printk("intc_trigger_softirq\n"); */
	spin_lock_irqsave(&intc_lock, flags);

	// This SEL code is for safety only, bits in this register are shared for some other operations
	//This is additional read and write to register and may cause extra dealy. 
	// if the code stability is proven , we can remove this code below 3 lines INTR_SEL.
	ReadData=readl(chipreg_base + CHIPREG_MDM_SW_INTR_SEL_OFFSET   );
	ReadData|=(1<<birq);
    writel(ReadData, chipreg_base + CHIPREG_MDM_SW_INTR_SEL_OFFSET  );

	writel((1<<birq), chipreg_base + CHIPREG_MDM_SW_INTR_SET_OFFSET	);

		

	
	spin_unlock_irqrestore(&intc_lock, flags);
}
EXPORT_SYMBOL(intc_trigger_softirq);

#if 0				/* #if defined(CONFIG_PM) */
/**
 * struct intc_device - INTC PM device
 * @sysdev: The system device which is registered.
 * @irq_start: The IRQ number for the base of the INTC.
 * @base: The register base for the INTC.
 * @resume_sources: A bitmask of interrupts for resume.
 * @resume_irqs: The IRQs enabled for resume.
 * @int_select: Save for INTC_ISELR.
 * @int_enable: Save for INTC_IMR.
 */
struct intc_device {
	struct sys_device sysdev;

	void __iomem *base;
	int irq_start;
	u32 resume_sources;
	u32 resume_irqs;
	u32 int_enable;
};

/* we cannot allocate memory when INTCs are initially registered */
static struct intc_device intc_devices[3];

static inline struct intc_device *to_intc(struct sys_device *sys)
{
	return container_of(sys, struct intc_device, sysdev);
}

static int intc_id;

static int intc_class_resume(struct sys_device *dev)
{
	struct intc_device *intc = to_intc(dev);
	void __iomem *base = intc->base;

	pr_debug("%s: resuming intc at %p\n", __func__, base);

	/* set the enabled ints */
	writel(intc->int_enable, base + INTC_IMR);

	return 0;
}

static int intc_class_suspend(struct sys_device *dev, pm_message_t state)
{
	struct intc_device *intc = to_intc(dev);
	void __iomem *base = intc->base;

	pr_debug("%s: suspending intc at %p\n", __func__, base);

	intc->int_enable = readl(base + INTC_IMR);

	/* set the interrupts (if any) that are used for
	 * resuming the system */
	writel(intc->resume_irqs, base + INTC_IMR);

	return 0;
}

struct sysdev_class intc_class = {
	.name = "intc",
	.suspend = intc_class_suspend,
	.resume = intc_class_resume,
};

/**
 * intc_pm_register - Register a INTC for later power management control
 * @base: The base address of the INTC.
 * @irq: The base IRQ for the INTC.
 * @resume_sources: bitmask of interrupts allowed for resume sources.
 *
 * Register the INTC with the system device tree so that it can be notified
 * of suspend and resume requests and ensure that the correct actions are
 * taken to re-instate the settings on resume.
 */
static void __init intc_pm_register(void __iomem * base, unsigned int irq,
				    u32 resume_sources)
{
	struct intc_device *v;

	if (intc_id >= ARRAY_SIZE(intc_devices))
		pr_err("%s: too few INTCs, increase CONFIG_ARM_INTC_NR\n",
		       __func__);
	else {
		v = &intc_devices[intc_id];
		v->base = base;
		v->resume_sources = resume_sources;
		v->irq_start = irq;
		intc_id++;
	}
}

/**
 * intc_pm_init - initicall to register INTC pm
 *
 * This is called via late_initcall() to register
 * the resources for the INTCs due to the early
 * nature of the INTC's registration.
*/
static int __init intc_pm_init(void)
{
	struct intc_device *dev = intc_devices;
	int err;
	int id;

	if (intc_id == 0)
		return 0;

	err = sysdev_class_register(&intc_class);
	if (err) {
		pr_err("%s: cannot register class\n", __func__);
		return err;
	}

	for (id = 0; id < intc_id; id++, dev++) {
		dev->sysdev.id = id;
		dev->sysdev.cls = &intc_class;

		err = sysdev_register(&dev->sysdev);
		if (err) {
			pr_err("%s: failed to register device\n", __func__);
			return err;
		}
	}

	return 0;
}

late_initcall(intc_pm_init);

static struct intc_device *intc_from_irq(unsigned int irq)
{
	struct intc_device *v = intc_devices;
	unsigned int base_irq = irq & ~31;
	int id;

	for (id = 0; id < intc_id; id++, v++) {
		if (v->irq_start == base_irq)
			return v;
	}

	return NULL;
}

static int intc_set_wake(unsigned int irq, unsigned int on)
{
	struct intc_device *v = intc_from_irq(irq);
	unsigned int off = irq & 31;
	u32 bit = 1 << off;

	if (!v)
		return -EINVAL;

	if (!(bit & v->resume_sources))
		return -EINVAL;

	if (on)
		v->resume_irqs |= bit;
	else
		v->resume_irqs &= ~bit;

	return 0;
}

#else
static inline void intc_pm_register(void __iomem *base,
				unsigned int irq,
				u32 arg1)
{
}

#define intc_set_wake NULL
#endif /* CONFIG_PM */

static struct irq_chip intc_chip = {
	.name = "INTC",
	.irq_ack = intc_ack_irq,
	.irq_mask = intc_mask_irq,
	.irq_unmask = intc_unmask_irq,
	.irq_set_type = intc_set_type,
	.irq_set_wake = intc_set_wake,
};



void ipc_set_interrupt_mask()
{

		void __iomem *base = (void __iomem *)(KONA_BINTC_BASE_ADDR);
	
		void __iomem *chipreg_base = (void __iomem *)(KONA_CHIPREG_VA);
		unsigned int ReadData=0;
		unsigned int birq = IRQ_TO_BMIRQ(IRQ_IPC_A2C);


		printk(KERN_ERR "%s birq=%x base=%x chip_base=%x\n",__FUNCTION__,birq,(unsigned int)base,(unsigned int)chipreg_base);
		printk(KERN_ERR "%s DOD birq=%x base=%x chip_base=%x\n",__FUNCTION__,birq,(unsigned int)base,(unsigned int)chipreg_base);


		writel((1<<birq), base + BINTC_IMR0_13_SET_OFFSET	   );

		//Set AP to CP Event Interrupt
		writel((1<<birq), base + BINTC_IMR0_8_SET_OFFSET	   );



		ReadData=readl(chipreg_base + CHIPREG_MDM_SW_INTR_SEL_OFFSET);
		ReadData |= (1<<birq);
		writel(ReadData, chipreg_base + CHIPREG_MDM_SW_INTR_SEL_OFFSET	);

	
		
		
		printk(KERN_ERR "%s AFT  birq=%x base=%x chip_base=%x\n",__FUNCTION__,birq,(unsigned int)base,(unsigned int)chipreg_base);


}

EXPORT_SYMBOL(ipc_set_interrupt_mask);



/** @addtogroup InterruptAPIGroup
	@{
*/

/**
 * bcm_intc_init - initialise a vectored interrupt controller
 * @param base				iomem base address
 * @param irq_start		starting interrupt number, must be muliple of 32
 * @param intc_sources		bitmask of interrupt sources to allow
 * @param resume_sources	bitmask of interrupt sources to allow for resume
 *
 * The INTC module in Broadcom SoC handles 64 interrupts. The first set
 * of 32 interrupts are handled in first block 0x00 of INTC registers,
 * while the second set of 32 interrupts are handled at 0x100. In that
 * case the bcm_intc_init function is called twice, with base set to
 * offset 0x000 and 0x100 within the page. We call this "second block".
 */
void __init bcm_intc_init(void __iomem * base, unsigned int irq_start,
			  u32 intc_sources, u32 resume_sources)
{
	unsigned int i;
	/* Disable all interrupts initially. */

	writel(0, base + INTC_IMR);
	writel(~0, base + INTC_ICR);
	writel(0, base + INTC_ISR);
	writel(0, base + INTC_IMSR);
	writel(~0, base + INTC_SICR(irq_start));
	writel(0, base + INTC_ISELR);

	for (i = 0; i < 32; i++) {
		if (intc_sources & (1 << i)) {
			unsigned int irq = irq_start + i;

			irq_set_chip(irq, &intc_chip);
			irq_set_chip_data(irq, base);
			if (irq != IRQ_GPIO)
				irq_set_handler(irq, handle_level_irq);
			else
				irq_set_handler(irq, handle_simple_irq);
			irq_set_status_flags(irq, IRQF_VALID | IRQF_PROBE);
		}
	}

	intc_pm_register(base, irq_start, resume_sources);
}

/** @} */
