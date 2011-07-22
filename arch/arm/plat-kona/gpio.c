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

#include <linux/io.h>
#include <asm/gpio.h>
#include <mach/gpio.h>
#include <mach/rdb/brcm_rdb_gpio.h>
#include <mach/io_map.h>


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
	void __iomem * reg_base;
	int num_bank;
	spinlock_t lock;
	struct kona_gpio_bank banks[KONA_MAX_GPIO_BANK];
};

static struct kona_gpio kona_gpio = {
	.reg_base = (void __iomem *)(KONA_GPIO2_VA),  
	.num_bank = 0,
	.banks = {
		{.id = 0, .irq = BCM_INT_ID_GPIO1},
		{.id = 1, .irq = BCM_INT_ID_GPIO2},
		{.id = 2, .irq = BCM_INT_ID_GPIO3},
		{.id = 3, .irq = BCM_INT_ID_GPIO4},
		{.id = 4, .irq = BCM_INT_ID_GPIO5},
		{.id = 5, .irq = BCM_INT_ID_GPIO6},
	},
};

static void kona_gpio_set(struct gpio_chip *chip, unsigned gpio, int value)
{
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem * reg_base = kona_gpio.reg_base;	
	u32 val, reg_offset;
	unsigned long flags;

	(void) chip; /* unused input parameter */ 

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
	void __iomem * reg_base = kona_gpio.reg_base;	
	u32 val, reg_offset;

	(void) chip; /* unused input parameter */ 

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
	void __iomem * reg_base = kona_gpio.reg_base;	
	u32 val; 
	unsigned long flags;

	(void) chip; /* unused input parameter */ 

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
	void __iomem * reg_base = kona_gpio.reg_base;	
	u32 val; 
	unsigned long flags;

	(void) chip; /* unused input parameter */ 

	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + GPIO_CTRL(gpio));
	val &= ~GPIO_GPCTR0_IOTR_MASK;
	val |= GPIO_GPCTR0_IOTR_CMD_0UTPUT;
	__raw_writel(val, reg_base + GPIO_CTRL(gpio));

	val = __raw_readl(reg_base + GPIO_OUT_SET(GPIO_BANK(gpio)));
	val = (value) ? (val | (1 << GPIO_BIT(gpio))) : (val & (~(1 << GPIO_BIT(gpio))));
	__raw_writel(val, reg_base + GPIO_OUT_SET(GPIO_BANK(gpio)));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);

	return 0;
}

static int kona_gpio_set_debounce(struct gpio_chip *chip, unsigned gpio, unsigned debounce)
{
/*
 * bit 8 - debounce enable is not defined either in Island and Rhea RDB header file
 * defined here before RDB is fixed
 */
#define	GPIO_DB_ENABLE (1<<8)

	void __iomem * reg_base = kona_gpio.reg_base;
	u32 val, res;
	unsigned long flags;

	(void) chip; /* unused input parameter */
	
	if (debounce !=0) {
      debounce/=1000;
      
      /* find the MSB */
      res=fls(debounce)-1;
      /* Check if MSB-1 is set (round up or down) */
      if (res>0 && (debounce & 1 << (res-1)) ){
         res++;
      }

	   if (res < GPIO_GPCTR0_DBR_CMD_1MS || res > GPIO_GPCTR0_DBR_CMD_128MS) {
		   printk(KERN_ERR "Debounce value %d000 not in range\n", debounce);
		   return -EINVAL;
	   }

	   spin_lock_irqsave(&kona_gpio.lock, flags);

	   val = __raw_readl(reg_base + GPIO_CTRL(gpio));
	   val &= ~GPIO_GPCTR0_DBR_MASK;
     	val |= GPIO_DB_ENABLE | (res << GPIO_GPCTR0_DBR_SHIFT);
   } else {
      val &= ~GPIO_DB_ENABLE;
   }

	__raw_writel(val, reg_base + GPIO_CTRL(gpio));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);

	return 0;
}

static struct gpio_chip kona_gpio_chip = {
	.label              = "kona-gpio",
	.direction_input    = kona_gpio_direction_input,
	.get                = kona_gpio_get,
	.direction_output   = kona_gpio_direction_output,
	.set                = kona_gpio_set,
	.set_debounce       = kona_gpio_set_debounce,
	.base               = 0,
	.ngpio              = 0,
};

static void kona_gpio_irq_ack(unsigned int irq)
{
	int gpio = irq_to_gpio(irq);
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem * reg_base = kona_gpio.reg_base;	
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&kona_gpio.lock, flags);
	
	val = __raw_readl(reg_base + GPIO_INT_STA(bankId));
	val |= 1 << bit;
	__raw_writel(val, reg_base + GPIO_INT_STA(bankId));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);
}

static void kona_gpio_irq_mask(unsigned int irq)
{
	int gpio = irq_to_gpio(irq);
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem * reg_base = kona_gpio.reg_base;	
	u32 val;
	unsigned long flags;
	
	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + GPIO_INT_MSK(bankId));
	val |= 1 << bit;
	__raw_writel(val, reg_base + GPIO_INT_MSK(bankId));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);
}

static void kona_gpio_irq_unmask(unsigned int irq)
{
	int gpio = irq_to_gpio(irq);
	int bankId = GPIO_BANK(gpio);
	int bit = GPIO_BIT(gpio);
	void __iomem * reg_base = kona_gpio.reg_base;	
	u32 val;
	unsigned long flags;
	
	spin_lock_irqsave(&kona_gpio.lock, flags);

	val = __raw_readl(reg_base + GPIO_INT_MSKCLR(bankId));
	val |= 1 << bit;
	__raw_writel(val, reg_base + GPIO_INT_MSKCLR(bankId));

	spin_unlock_irqrestore(&kona_gpio.lock, flags);
}


static int kona_gpio_irq_set_type(unsigned int irq, unsigned int type)
{
	int gpio = irq_to_gpio(irq);
	struct kona_gpio *p_kona_gpio = get_irq_chip_data(irq);
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
	struct kona_gpio_bank *bank;
	int bit, bankId;
	void __iomem * reg_base = kona_gpio.reg_base;
	unsigned long sta;

	desc->chip->ack(irq);

	bank = get_irq_data(irq);
	bankId = bank->id;
	sta = __raw_readl(reg_base + GPIO_INT_STA(bankId)) &
		      (~(__raw_readl(reg_base + GPIO_INT_MSK(bankId))));

	for_each_set_bit(bit, &sta, 32)
	{
		/* Clear interrupt before handler is called so we don't
		 * miss any interrupt occurred during executing them.
		 */
		__raw_writel(__raw_readl(reg_base + GPIO_INT_STA(bankId)) | (1 << bit), 
			         reg_base + GPIO_INT_STA(bankId));

		/* KONA GPIOs are all edge triggered. Clear condition
		 * before executing the handler so that we don't miss edges.
		 */ 
		desc->chip->unmask(irq); 
		
		/* Invoke interrupt handler.
		 */ 
		generic_handle_irq(gpio_to_irq(GPIO_PER_BANK * bankId + bit));
	}
}


static struct irq_chip kona_gpio_irq_chip = {
	.name		= "KONA-GPIO",
	.ack		= kona_gpio_irq_ack,
	.mask		= kona_gpio_irq_mask,
	.unmask		= kona_gpio_irq_unmask,
	.set_type	= kona_gpio_irq_set_type,
};


/* This lock class tells lockdep that GPIO irqs are in a different
 * category than their parents, so it won't report false recursion.
 */
static struct lock_class_key gpio_lock_class;

static void kona_gpio_reset(int num_bank)
{
	int i;
	void __iomem * reg_base = kona_gpio.reg_base;

	/* KONA GPIO pins are locked by default.Unlock them so that
	 * their registers can be accessed. 
	 */ 
	for (i = 0; i < num_bank; i++)
	{
		/* This register is password protected. 
		 */ 
		__raw_writel(KONA_GPIO_PASSWD, reg_base + GPIO_GPPWR_OFFSET);

		/* Unlock the bank */ 
		__raw_writel(0x0, reg_base + GPIO_PWD_STA(i));
	}

	/* disable interrupts and clear status
	 */ 
	for (i = 0; i < num_bank; i++)
	{
		__raw_writel(0xFFFFFFFF, reg_base + GPIO_INT_MSK(i));
		__raw_writel(0xFFFFFFFF, reg_base + GPIO_INT_STA(i));
	}
}

int __init kona_gpio_init(int num_bank)
{
	struct kona_gpio_bank *bank;
	int i;

	kona_gpio_reset(num_bank);
	
	kona_gpio.num_bank = num_bank;
	spin_lock_init(&kona_gpio.lock);
	kona_gpio_chip.ngpio = num_bank * GPIO_PER_BANK;
	gpiochip_add(&kona_gpio_chip);

	for (i = IRQ_GPIO_0; i < (IRQ_GPIO_0 + kona_gpio_chip.ngpio ); i++) 
	{
		lockdep_set_class(&irq_desc[i].lock, &gpio_lock_class);
		set_irq_chip_data(i, &kona_gpio);
		set_irq_chip(i, &kona_gpio_irq_chip);
		set_irq_handler(i, handle_simple_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	for (i = 0; i < num_bank; i++)
	{
		bank = &kona_gpio.banks[i];

		set_irq_chained_handler(bank->irq, kona_gpio_irq_handler);
		set_irq_data(bank->irq, bank);
	}

	printk(KERN_INFO "kona gpio initialised.\n");
	return 0;
}


