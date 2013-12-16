/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>

#include <linux/io.h>
#include <mach/gpio.h>
#include <asm/gpio.h>
#include <mach/rdb/brcm_rdb_gpio.h>
#include <mach/io_map.h>
#include <mach/memory.h>

#define KONA_GPIO_PASSWD (0x00a5a501)
#define GPIO_PER_BANK (32)
#define KONA_MAX_GPIO_BANK  (KONA_MAX_GPIO/GPIO_PER_BANK)

#define GPIO_BANK(gpio)	((gpio) >> 5)
#define GPIO_BIT(gpio)	((gpio) % GPIO_PER_BANK)

#define GPIO_OUT_STA(bank) (GPIO_GPOR0_OFFSET + (bank * 4))
#define GPIO_IN_STA(bank) (GPIO_GPIR0_OFFSET + (bank * 4))
#define GPIO_OUT_SET(bank) (GPIO_GPORS0_OFFSET + (bank * 4))
#define GPIO_OUT_CLR(bank) (GPIO_GPORC0_OFFSET + (bank * 4))
#define GPIO_INT_STA(bank) (GPIO_ISR0_OFFSET + (bank * 4))
#define GPIO_INT_MSK(bank) (GPIO_IMR0_OFFSET + (bank * 4))
#define GPIO_INT_MSKCLR(bank) (GPIO_IMRC0_OFFSET + (bank * 4))
#define GPIO_PWD_STA(bank) (GPIO_GPPLSR0_OFFSET + (bank * 4))
#define GPIO_CTRL(gpio) (GPIO_GPCTR0_OFFSET + (gpio * 4))

struct kona_gpio_bank {
	int id;
	int irq;
};

struct kona_gpio {
	void __iomem *reg_base;
	int num_bank;
	spinlock_t lock;
	struct kona_gpio_bank banks[KONA_MAX_GPIO_BANK];
};

static struct kona_gpio kona_gpio = {
	.reg_base = (void __iomem *)(KONA_GPIO2_VA),
	.num_bank = 0,
	.banks = {
		  {.id = 0,.irq = BCM_INT_ID_GPIO1},
		  {.id = 1,.irq = BCM_INT_ID_GPIO2},
		  {.id = 2,.irq = BCM_INT_ID_GPIO3},
		  {.id = 3,.irq = BCM_INT_ID_GPIO4},
		  {.id = 4,.irq = BCM_INT_ID_GPIO5},
		  {.id = 5,.irq = BCM_INT_ID_GPIO6},
		  },
};

#ifdef CONFIG_KONA_ATAG_DT
#include <linux/of.h>
#include <linux/of_fdt.h>

#define DT_GPIO_INPUT		(1 << 0)
#define DT_GPIO_OUTPUT_VAL	(1 << 1)
#define DT_GPIO_VALID		(1 << 31)

/* gpio configuration data from DT */
extern uint32_t dt_gpio[];
extern uint32_t dt_pinmux_gpio_mask[];

int __init early_init_dt_scan_gpio(unsigned long node, const char *uname,
				   int depth, void *data)
{
	const char *prop;
	unsigned long size, i;
	uint32_t *p, gpio, cfg;

	//printk(KERN_INFO "%s: node=0x%lx, uname=%s, depth=%d\n", __func__, node, uname, depth);

	if (depth != 1 || strcmp(uname, "gpio") != 0)
		return 0;	/* not found, continue... */

	prop = of_get_flat_dt_prop(node, "reg", &i);

	p = (uint32_t *)prop;
	i = be32_to_cpu(p[1]);
	printk(KERN_INFO "reg: 0x%x, 0x%x\n", be32_to_cpu(p[0]),
	       be32_to_cpu(p[1]));

	/* check the base address passed */
	if (be32_to_cpu(p[0]) != GPIO2_BASE_ADDR) {
		printk(KERN_ERR "Wrong base address!\n");
		return 1;
	}

	prop = of_get_flat_dt_prop(node, "data", &size);
	printk("data(0x%x): size=%ld\n", (unsigned int)prop, size);

	if (i != size / 8) {
		printk(KERN_ERR "Mismatch size! %ld & %ld\n", i, size / 8);
	} else {
		p = (uint32_t *)prop;
		size = i;
		for (i = 0; i < size; i++) {
			gpio = be32_to_cpu(*p++);
			cfg = be32_to_cpu(*p++);
			//printk(KERN_INFO "gpio%d: 0x%x\n", gpio, cfg);

			/* mark and save for late processing */
			dt_gpio[gpio] = cfg | DT_GPIO_VALID;
		}
	}

	return 1;
}
#endif /* CONFIG_KONA_ATAG_DT */

static void kona_gpio_set(struct gpio_chip *chip, unsigned gpio, int value)
{
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem *reg_base = kona_gpio.reg_base;
	u32 val, reg_offset;
	unsigned long flags;

	(void)chip;		/* unused input parameter */

	/* determine the GPIO pin direction 
	 */
	val = __raw_readl(reg_base + GPIO_CTRL(gpio));
	val &= GPIO_GPCTR0_IOTR_MASK;

	/* this function only applies to output pin
	 */
	if (GPIO_GPCTR0_IOTR_CMD_INPUT == val)
		return;

	reg_offset = value ? GPIO_OUT_SET(bankId) : GPIO_OUT_CLR(bankId);

	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + reg_offset);
	val |= 1 << bit;

	__raw_writel(val, reg_base + reg_offset);

	spin_unlock_irqrestore(&kona_gpio.lock, flags);
}

static int kona_gpio_get(struct gpio_chip *chip, unsigned gpio)
{
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem *reg_base = kona_gpio.reg_base;
	u32 val, reg_offset;

	(void)chip;		/* unused input parameter */

	/* determine the GPIO pin direction 
	 */
	val = __raw_readl(reg_base + GPIO_CTRL(gpio));
	val &= GPIO_GPCTR0_IOTR_MASK;

	/* read the GPIO bank status
	 */
	reg_offset = (GPIO_GPCTR0_IOTR_CMD_INPUT == val) ?
	    GPIO_IN_STA(bankId) : GPIO_OUT_STA(bankId);
	val = __raw_readl(reg_base + reg_offset);

	/* return the specified bit status  
	 */
	return ((val >> bit) & 1);
}

static int kona_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	void __iomem *reg_base = kona_gpio.reg_base;
	u32 val;
	unsigned long flags;

	(void)chip;		/* unused input parameter */

	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + GPIO_CTRL(gpio));
	val &= ~GPIO_GPCTR0_IOTR_MASK;
	val |= GPIO_GPCTR0_IOTR_CMD_INPUT;
	__raw_writel(val, reg_base + GPIO_CTRL(gpio));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);

	return 0;
}

static int kona_gpio_direction_output(struct gpio_chip *chip, unsigned gpio,
				      int value)
{
	void * __iomem reg_base = kona_gpio.reg_base;
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	u32 val, reg_offset;
	unsigned long flags;

	(void)chip; /* unused input parameter */

	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + GPIO_CTRL(gpio));
	val &= ~GPIO_GPCTR0_IOTR_MASK;
	val |= GPIO_GPCTR0_IOTR_CMD_0UTPUT;
	__raw_writel(val, reg_base + GPIO_CTRL(gpio));

	reg_offset = value ? GPIO_OUT_SET(bankId) : GPIO_OUT_CLR(bankId);

	val = __raw_readl(reg_base + reg_offset);
	val |= 1 << bit;

	__raw_writel(val, reg_base + reg_offset);

	spin_unlock_irqrestore(&kona_gpio.lock, flags);

	return 0;
}

static int kona_gpio_set_debounce(struct gpio_chip *chip, unsigned gpio,
				  unsigned debounce)
{
/*
 * bit 8 - debounce enable is not defined either in Island and Rhea RDB header file
 * defined here before RDB is fixed
 */
#define	GPIO_DB_ENABLE (1<<8)

	void __iomem *reg_base = kona_gpio.reg_base;
	u32 val, res = 0;
	unsigned long flags;

	(void)chip;		/* unused input parameter */

	if ((debounce > 0 && debounce < 1000) || debounce > 128000) {
		printk(KERN_ERR "Debounce value %d not in range\n", debounce);
		return -EINVAL;
	}

	/* calculate debounce bit value */
	if (debounce != 0) {
		/* Convert to ms */
		debounce /= 1000;

		/* find the MSB */
		res = fls(debounce) - 1;

		/* Check if MSB-1 is set (round up or down) */
		if (res > 0 && (debounce & 1 << (res - 1))) {
			res++;
		}
	}

	/* spin lock for read-modify-write of the GPIO register */
	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + GPIO_CTRL(gpio));
	val &= ~GPIO_GPCTR0_DBR_MASK;

	if (debounce == 0) {
		/* disable debounce */
		val &= ~GPIO_DB_ENABLE;
	} else {
		val |= GPIO_DB_ENABLE | (res << GPIO_GPCTR0_DBR_SHIFT);
	}

	__raw_writel(val, reg_base + GPIO_CTRL(gpio));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);

	return 0;
}

static struct gpio_chip kona_gpio_chip = {
	.label = "kona-gpio",
	.direction_input = kona_gpio_direction_input,
	.get = kona_gpio_get,
	.direction_output = kona_gpio_direction_output,
	.set = kona_gpio_set,
	.set_debounce = kona_gpio_set_debounce,
	.base = 0,
	.ngpio = 0,
};

static void kona_gpio_irq_ack(struct irq_data *d)
{
	unsigned int irq = d->irq;
	int gpio = irq_to_gpio(irq);
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem *reg_base = kona_gpio.reg_base;
	unsigned long flags;

	spin_lock_irqsave(&kona_gpio.lock, flags);

	__raw_writel(1 << bit, reg_base + GPIO_INT_STA(bankId));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);
}

static void kona_gpio_irq_mask(struct irq_data *d)
{
	unsigned int irq = d->irq;
	int gpio = irq_to_gpio(irq);
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem *reg_base = kona_gpio.reg_base;
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + GPIO_INT_MSK(bankId));
	val |= 1 << bit;
	__raw_writel(val, reg_base + GPIO_INT_MSK(bankId));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);
}

static void kona_gpio_irq_unmask(struct irq_data *d)
{
	unsigned int irq = d->irq;
	int gpio = irq_to_gpio(irq);
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem *reg_base = kona_gpio.reg_base;
	unsigned long flags;

	spin_lock_irqsave(&kona_gpio.lock, flags);

	__raw_writel(1 << bit, reg_base + GPIO_INT_MSKCLR(bankId));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);
}

static int kona_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	unsigned int irq = d->irq;
	int gpio = irq_to_gpio(irq);
	struct kona_gpio *p_kona_gpio = irq_get_chip_data(irq);
	u32 lvl_type;
	u32 val;
	unsigned long flags;

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_RISING:
		lvl_type = GPIO_GPCTR0_ITR_CMD_RISING_EDGE;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		lvl_type = GPIO_GPCTR0_ITR_CMD_FALLING_EDGE;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		lvl_type = GPIO_GPCTR0_ITR_CMD_BOTH_EDGE;
		break;

	case IRQ_TYPE_LEVEL_HIGH:
	case IRQ_TYPE_LEVEL_LOW:
		/* KONA GPIO doesn't support level triggering.
		 */
	default:
		printk(KERN_ERR "Invalid KONA GPIO irq type 0x%x\n", type);
		return -EINVAL;
	}

	spin_lock_irqsave(&p_kona_gpio->lock, flags);

	val = __raw_readl(p_kona_gpio->reg_base + GPIO_CTRL(gpio));
	val &= ~GPIO_GPCTR0_ITR_MASK;
	val |= lvl_type << GPIO_GPCTR0_ITR_SHIFT;
	__raw_writel(val, p_kona_gpio->reg_base + GPIO_CTRL(gpio));

	spin_unlock_irqrestore(&p_kona_gpio->lock, flags);

	return 0;
}

static void kona_gpio_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	int bit, bankId;
	void __iomem *reg_base = kona_gpio.reg_base;
	unsigned long sta;
	struct kona_gpio_bank *bank = irq_get_handler_data(irq);
	struct irq_chip *chip = irq_desc_get_chip(desc);

	chained_irq_enter(chip, desc);

	bankId = bank->id;
	sta = __raw_readl(reg_base + GPIO_INT_STA(bankId)) &
	    (~(__raw_readl(reg_base + GPIO_INT_MSK(bankId))));

	for_each_set_bit(bit, &sta, 32) {
		/* Clear interrupt before handler is called so we don't
		 * miss any interrupt occurred during executing them.
		 */
		__raw_writel((1 << bit), reg_base + GPIO_INT_STA(bankId));

		/* Invoke interrupt handler.
		 */
		generic_handle_irq(gpio_to_irq(GPIO_PER_BANK * bankId + bit));
	}

	chained_irq_exit(chip, desc);
}

static int kona_gpio_irq_set_wake(struct irq_data *data, unsigned int on)
{
	return 0;
}

static struct irq_chip kona_gpio_irq_chip = {
	.name = "KONA-GPIO",
	.irq_ack = kona_gpio_irq_ack,
	.irq_mask = kona_gpio_irq_mask,
	.irq_unmask = kona_gpio_irq_unmask,
	.irq_set_type = kona_gpio_irq_set_type,
	.irq_set_wake = kona_gpio_irq_set_wake,
	.irq_disable = kona_gpio_irq_mask,
	.irq_enable = kona_gpio_irq_unmask,
};

/* This lock class tells lockdep that GPIO irqs are in a different
 * category than their parents, so it won't report false recursion.
 */
static struct lock_class_key gpio_lock_class;

static void kona_gpio_reset(int num_bank)
{
	int i;
	void __iomem *reg_base = kona_gpio.reg_base;

	/* KONA GPIO pins are locked by default.Unlock them so that
	 * their registers can be accessed. 
	 */
	for (i = 0; i < num_bank; i++) {
		/* This register is password protected. 
		 */
		__raw_writel(KONA_GPIO_PASSWD, reg_base + GPIO_GPPWR_OFFSET);

		/* Unlock the bank */
		__raw_writel(0x0, reg_base + GPIO_PWD_STA(i));
	}

	/* disable interrupts and clear status
	 */
	for (i = 0; i < num_bank; i++) {
		__raw_writel(0xFFFFFFFF, reg_base + GPIO_INT_MSK(i));
		__raw_writel(0xFFFFFFFF, reg_base + GPIO_INT_STA(i));
	}
}

int __init kona_gpio_init(int num_bank)
{
	struct kona_gpio_bank *bank;
	int i;
#ifdef CONFIG_KONA_ATAG_DT
	uint32_t gpio, mask, j;
#endif

#ifdef CONFIG_OF_GPIO
	struct device_node *np = NULL;

	/*
	 * This isn't ideal, but it gets things hooked up until this
	 * driver is converted into a platform_device
	 */
	np = of_find_compatible_node(NULL, NULL, "bcm,kona-gpio");
	kona_gpio_chip.of_node = np;
#endif


	kona_gpio_reset(num_bank);

	kona_gpio.num_bank = num_bank;
	spin_lock_init(&kona_gpio.lock);
	kona_gpio_chip.ngpio = num_bank * GPIO_PER_BANK;
	BUG_ON(gpiochip_add(&kona_gpio_chip) < 0);

	for (i = IRQ_GPIO_0; i < (IRQ_GPIO_0 + kona_gpio_chip.ngpio); i++) {
		irq_set_lockdep_class(i, &gpio_lock_class);
		irq_set_chip_and_handler(i, &kona_gpio_irq_chip,
					 handle_simple_irq);
		irq_set_chip_data(i, &kona_gpio);
		set_irq_flags(i, IRQF_VALID);
	}

	for (i = 0; i < num_bank; i++) {
		bank = &kona_gpio.banks[i];

		irq_set_chained_handler(bank->irq, kona_gpio_irq_handler);
		irq_set_handler_data(bank->irq, bank);

#ifdef CONFIG_KONA_ATAG_DT
		gpio = i * GPIO_PER_BANK;
		mask = dt_pinmux_gpio_mask[GPIO_BANK(gpio)];
		for (j = 0; j < GPIO_PER_BANK; j++) {
			if (dt_gpio[gpio] & DT_GPIO_VALID) {
				/* Cross-check against pinmux node */
				if (mask & (1 << GPIO_BIT(gpio))) {
					//printk(KERN_INFO "Configure GPIO%d to 0x%x\n", gpio, dt_gpio[gpio]);

					gpio_request(gpio, "gpio");
					if (dt_gpio[gpio] & DT_GPIO_INPUT) {
						gpio_direction_input(gpio);
					} else {	/* output */
						gpio_direction_output(gpio,
							(dt_gpio[gpio] &
							 DT_GPIO_OUTPUT_VAL));
					}
					gpio_free(gpio);
					mask &= ~(1 << GPIO_BIT(gpio));
				} else {
					printk(KERN_ERR
					       "Mismatch GPIO%d. The board may not boot!\n",
					       gpio);
				}
			}
			gpio++;
		}
		if (mask) {
			printk(KERN_ERR
			       "Missing initial cfg for GPIO bank%d (mask=0x%x)\n",
			       i, mask);
		}
#endif
	}

	printk(KERN_INFO "kona gpio initialised.\n");
	return 0;
}

#ifdef CONFIG_KONA_ATAG_DT
uint32_t get_dts_gpio_value(uint32_t index)
{
	return dt_gpio[index];
}
#endif /* CONFIG_KONA_ATAG_DT */
