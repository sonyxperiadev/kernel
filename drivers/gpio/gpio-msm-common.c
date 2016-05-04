/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/bitmap.h>
#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqchip/msm-mpm-irq.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#inlcude <linux/wakeup_reason.h>

#include <mach/msm_iomap.h>
#include <mach/gpiomux.h>
#include "gpio-msm-common.h"

#ifdef CONFIG_GPIO_MSM_V3
enum msm_tlmm_register {
	SDC4_HDRV_PULL_CTL = 0x0, /* NOT USED */
	SDC3_HDRV_PULL_CTL = 0x0, /* NOT USED */
	SDC2_HDRV_PULL_CTL = 0x2048,
	SDC1_HDRV_PULL_CTL = 0x2044,
};
#else
enum msm_tlmm_register {
	SDC4_HDRV_PULL_CTL = 0x20a0,
	SDC3_HDRV_PULL_CTL = 0x20a4,
	SDC2_HDRV_PULL_CTL = 0x0, /* NOT USED */
	SDC1_HDRV_PULL_CTL = 0x20a0,
};
#endif

static int tlmm_msm_summary_irq, nr_direct_connect_irqs;

struct tlmm_field_cfg {
	enum msm_tlmm_register reg;
	u8                     off;
};

static const struct tlmm_field_cfg tlmm_hdrv_cfgs[] = {
	{SDC4_HDRV_PULL_CTL, 6}, /* TLMM_HDRV_SDC4_CLK  */
	{SDC4_HDRV_PULL_CTL, 3}, /* TLMM_HDRV_SDC4_CMD  */
	{SDC4_HDRV_PULL_CTL, 0}, /* TLMM_HDRV_SDC4_DATA */
	{SDC3_HDRV_PULL_CTL, 6}, /* TLMM_HDRV_SDC3_CLK  */
	{SDC3_HDRV_PULL_CTL, 3}, /* TLMM_HDRV_SDC3_CMD  */
	{SDC3_HDRV_PULL_CTL, 0}, /* TLMM_HDRV_SDC3_DATA */
	{SDC2_HDRV_PULL_CTL, 6}, /* TLMM_HDRV_SDC2_CLK  */
	{SDC2_HDRV_PULL_CTL, 3}, /* TLMM_HDRV_SDC2_CMD  */
	{SDC2_HDRV_PULL_CTL, 0}, /* TLMM_HDRV_SDC2_DATA */
	{SDC1_HDRV_PULL_CTL, 6}, /* TLMM_HDRV_SDC1_CLK  */
	{SDC1_HDRV_PULL_CTL, 3}, /* TLMM_HDRV_SDC1_CMD  */
	{SDC1_HDRV_PULL_CTL, 0}, /* TLMM_HDRV_SDC1_DATA */
};

static const struct tlmm_field_cfg tlmm_pull_cfgs[] = {
	{SDC4_HDRV_PULL_CTL, 14}, /* TLMM_PULL_SDC4_CLK */
	{SDC4_HDRV_PULL_CTL, 11}, /* TLMM_PULL_SDC4_CMD  */
	{SDC4_HDRV_PULL_CTL, 9},  /* TLMM_PULL_SDC4_DATA */
	{SDC3_HDRV_PULL_CTL, 14}, /* TLMM_PULL_SDC3_CLK  */
	{SDC3_HDRV_PULL_CTL, 11}, /* TLMM_PULL_SDC3_CMD  */
	{SDC3_HDRV_PULL_CTL, 9},  /* TLMM_PULL_SDC3_DATA */
	{SDC2_HDRV_PULL_CTL, 14}, /* TLMM_PULL_SDC2_CLK  */
	{SDC2_HDRV_PULL_CTL, 11}, /* TLMM_PULL_SDC2_CMD  */
	{SDC2_HDRV_PULL_CTL, 9},  /* TLMM_PULL_SDC2_DATA */
	{SDC1_HDRV_PULL_CTL, 13}, /* TLMM_PULL_SDC1_CLK  */
	{SDC1_HDRV_PULL_CTL, 11}, /* TLMM_PULL_SDC1_CMD  */
	{SDC1_HDRV_PULL_CTL, 9},  /* TLMM_PULL_SDC1_DATA */
	{SDC1_HDRV_PULL_CTL, 15}, /* TLMM_PULL_SDC1_RCLK  */
};

/*
 * Supported arch specific irq extension.
 * Default make them NULL.
 */
struct irq_chip msm_gpio_irq_extn = {
	.irq_eoi	= NULL,
	.irq_mask	= NULL,
	.irq_unmask	= NULL,
	.irq_retrigger	= NULL,
	.irq_set_type	= NULL,
	.irq_set_wake	= NULL,
	.irq_disable	= NULL,
};

/**
 * struct msm_gpio_dev: the MSM8660 SoC GPIO device structure
 *
 * @enabled_irqs: a bitmap used to optimize the summary-irq handler.  By
 * keeping track of which gpios are unmasked as irq sources, we avoid
 * having to do __raw_readl calls on hundreds of iomapped registers each time
 * the summary interrupt fires in order to locate the active interrupts.
 *
 * @wake_irqs: a bitmap for tracking which interrupt lines are enabled
 * as wakeup sources.  When the device is suspended, interrupts which are
 * not wakeup sources are disabled.
 */
struct msm_gpio_dev {
	struct gpio_chip gpio_chip;
	unsigned long *enabled_irqs;
	unsigned long *wake_irqs;
	struct irq_domain *domain;
};

static DEFINE_SPINLOCK(tlmm_lock);

static inline struct msm_gpio_dev *to_msm_gpio_dev(struct gpio_chip *chip)
{
	return container_of(chip, struct msm_gpio_dev, gpio_chip);
}

static int msm_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	int rc;
	rc = __msm_gpio_get_inout(offset);
	mb();
	return rc;
}

static void msm_gpio_set(struct gpio_chip *chip, unsigned offset, int val)
{
	__msm_gpio_set_inout(offset, val);
	mb();
}

static int msm_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	__msm_gpio_set_config_direction(offset, 1, 0);
	mb();
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);
	return 0;
}

static int msm_gpio_direction_output(struct gpio_chip *chip,
				unsigned offset,
				int val)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	__msm_gpio_set_config_direction(offset, 0, val);
	mb();
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);
	return 0;
}

#ifndef CONFIG_USE_PINCTRL_IRQ
#ifdef CONFIG_OF
static int msm_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct msm_gpio_dev *g_dev = to_msm_gpio_dev(chip);
	struct irq_domain *domain = g_dev->domain;
	return irq_create_mapping(domain, offset);
}

static inline int msm_irq_to_gpio(struct gpio_chip *chip, unsigned irq)
{
	struct irq_data *irq_data = irq_get_irq_data(irq);
	return irq_data->hwirq;
}
#else
static int msm_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	return MSM_GPIO_TO_INT(offset - chip->base);
}

static inline int msm_irq_to_gpio(struct gpio_chip *chip, unsigned irq)
{
	return irq - MSM_GPIO_TO_INT(chip->base);
}
#endif
#else
static int msm_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	return -EINVAL;
}
#endif

static int msm_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	return msm_gpiomux_get(chip->base + offset);
}

static void msm_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	msm_gpiomux_put(chip->base + offset);
}

static struct msm_gpio_dev msm_gpio = {
	.gpio_chip = {
		.label		  = "msmgpio",
		.base             = 0,
		.direction_input  = msm_gpio_direction_input,
		.direction_output = msm_gpio_direction_output,
		.get              = msm_gpio_get,
		.set              = msm_gpio_set,
		.to_irq           = msm_gpio_to_irq,
		.request          = msm_gpio_request,
		.free             = msm_gpio_free,
	},
};

#ifndef CONFIG_USE_PINCTRL_IRQ
static void msm_gpio_irq_ack(struct irq_data *d)
{
	int gpio = msm_irq_to_gpio(&msm_gpio.gpio_chip, d->irq);

	__msm_gpio_set_intr_status(gpio);
	mb();
}

static void msm_gpio_irq_mask(struct irq_data *d)
{
	int gpio = msm_irq_to_gpio(&msm_gpio.gpio_chip, d->irq);
	unsigned long irq_flags;

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	__msm_gpio_set_intr_cfg_enable(gpio, 0);
	__clear_bit(gpio, msm_gpio.enabled_irqs);
	mb();
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);

	if (msm_gpio_irq_extn.irq_mask)
		msm_gpio_irq_extn.irq_mask(d);

}

static void msm_gpio_irq_unmask(struct irq_data *d)
{
	int gpio = msm_irq_to_gpio(&msm_gpio.gpio_chip, d->irq);
	unsigned long irq_flags;

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	__set_bit(gpio, msm_gpio.enabled_irqs);
	if (!__msm_gpio_get_intr_cfg_enable(gpio)) {
		__msm_gpio_set_intr_status(gpio);
		__msm_gpio_set_intr_cfg_enable(gpio, 1);
		mb();
	}
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);

	if (msm_gpio_irq_extn.irq_mask)
		msm_gpio_irq_extn.irq_unmask(d);
}

static void msm_gpio_irq_disable(struct irq_data *d)
{
	if (msm_gpio_irq_extn.irq_disable)
		msm_gpio_irq_extn.irq_disable(d);
}

static int msm_gpio_irq_set_type(struct irq_data *d, unsigned int flow_type)
{
	int gpio = msm_irq_to_gpio(&msm_gpio.gpio_chip, d->irq);
	unsigned long irq_flags;

	spin_lock_irqsave(&tlmm_lock, irq_flags);

	if (flow_type & IRQ_TYPE_EDGE_BOTH)
		__irq_set_handler_locked(d->irq, handle_edge_irq);
	else
		__irq_set_handler_locked(d->irq, handle_level_irq);

	__msm_gpio_set_intr_cfg_type(gpio, flow_type);

	mb();
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);

	if (msm_gpio_irq_extn.irq_set_type)
		msm_gpio_irq_extn.irq_set_type(d, flow_type);

	return 0;
}

/*
 * When the summary IRQ is raised, any number of GPIO lines may be high.
 * It is the job of the summary handler to find all those GPIO lines
 * which have been set as summary IRQ lines and which are triggered,
 * and to call their interrupt handlers.
 */
static irqreturn_t msm_summary_irq_handler(int irq, void *data)
{
	unsigned long i;
	struct irq_desc *desc = irq_to_desc(irq);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	int ngpio = msm_gpio.gpio_chip.ngpio;

	chained_irq_enter(chip, desc);

	for (i = find_first_bit(msm_gpio.enabled_irqs, ngpio);
	     i < ngpio;
	     i = find_next_bit(msm_gpio.enabled_irqs, ngpio, i + 1)) {
		if (__msm_gpio_get_intr_status(i))
			generic_handle_irq(msm_gpio_to_irq(&msm_gpio.gpio_chip,
							   i));
	}

	chained_irq_exit(chip, desc);
	return IRQ_HANDLED;
}

static int msm_gpio_irq_set_wake(struct irq_data *d, unsigned int on)
{
	int gpio = msm_irq_to_gpio(&msm_gpio.gpio_chip, d->irq);
	int ngpio = msm_gpio.gpio_chip.ngpio;

	if (on) {
		if (bitmap_empty(msm_gpio.wake_irqs, ngpio))
			irq_set_irq_wake(tlmm_msm_summary_irq, 1);
		set_bit(gpio, msm_gpio.wake_irqs);
	} else {
		clear_bit(gpio, msm_gpio.wake_irqs);
		if (bitmap_empty(msm_gpio.wake_irqs, ngpio))
			irq_set_irq_wake(tlmm_msm_summary_irq, 0);
	}

	if (msm_gpio_irq_extn.irq_set_wake)
		msm_gpio_irq_extn.irq_set_wake(d, on);

	return 0;
}

static struct irq_chip msm_gpio_irq_chip = {
	.name		= "msmgpio",
	.irq_mask	= msm_gpio_irq_mask,
	.irq_unmask	= msm_gpio_irq_unmask,
	.irq_ack	= msm_gpio_irq_ack,
	.irq_set_type	= msm_gpio_irq_set_type,
	.irq_set_wake	= msm_gpio_irq_set_wake,
	.irq_disable	= msm_gpio_irq_disable,
};

#ifdef CONFIG_PM
static int msm_gpio_suspend(void)
{
	unsigned long irq_flags;
	unsigned long i;
	int ngpio = msm_gpio.gpio_chip.ngpio;

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	for_each_set_bit(i, msm_gpio.enabled_irqs, ngpio)
		__msm_gpio_set_intr_cfg_enable(i, 0);

	for_each_set_bit(i, msm_gpio.wake_irqs, ngpio)
		__msm_gpio_set_intr_cfg_enable(i, 1);
	mb();
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);
	return 0;
}

void msm_gpio_show_resume_irq(void)
{
	unsigned long irq_flags;
	int i, irq, intstat;
	int ngpio = msm_gpio.gpio_chip.ngpio;

	if (!msm_show_resume_irq_mask)
		return;

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	for_each_set_bit(i, msm_gpio.wake_irqs, ngpio) {
		intstat = __msm_gpio_get_intr_status(i);
		if (intstat) {
			struct irq_desc *desc;
			const char *name = "null";

			irq = msm_gpio_to_irq(&msm_gpio.gpio_chip, i);

			log_base_wakeup_reason(irq);

			desc = irq_to_desc(irq);
			if (desc == NULL)
				name = "stray irq";
			else if (desc->action && desc->action->name)
				name = desc->action->name;

			pr_warning("%s: %d triggered %s\n",
					__func__, irq, name);
		}
	}
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);
}

static void msm_gpio_resume(void)
{
	unsigned long irq_flags;
	unsigned long i;
	int ngpio = msm_gpio.gpio_chip.ngpio;

	msm_gpio_show_resume_irq();

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	for_each_set_bit(i, msm_gpio.wake_irqs, ngpio)
		__msm_gpio_set_intr_cfg_enable(i, 0);

	for_each_set_bit(i, msm_gpio.enabled_irqs, ngpio)
		__msm_gpio_set_intr_cfg_enable(i, 1);
	mb();
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);
}
#else
#define msm_gpio_suspend NULL
#define msm_gpio_resume NULL
#endif

static struct syscore_ops msm_gpio_syscore_ops = {
	.suspend = msm_gpio_suspend,
	.resume = msm_gpio_resume,
};
#endif /* CONFIG_USE_PINCTRL_IRQ */

static void msm_tlmm_set_field(const struct tlmm_field_cfg *configs,
			       unsigned id, unsigned width, unsigned val)
{
	unsigned long irqflags;
	u32 mask = (1 << width) - 1;
	u32 __iomem *reg = MSM_TLMM_BASE + configs[id].reg;
	u32 reg_val;

	spin_lock_irqsave(&tlmm_lock, irqflags);
	reg_val = __raw_readl(reg);
	reg_val &= ~(mask << configs[id].off);
	reg_val |= (val & mask) << configs[id].off;
	__raw_writel(reg_val, reg);
	mb();
	spin_unlock_irqrestore(&tlmm_lock, irqflags);
}

void msm_tlmm_set_hdrive(enum msm_tlmm_hdrive_tgt tgt, int drv_str)
{
	msm_tlmm_set_field(tlmm_hdrv_cfgs, tgt, 3, drv_str);
}
EXPORT_SYMBOL(msm_tlmm_set_hdrive);

void msm_tlmm_set_pull(enum msm_tlmm_pull_tgt tgt, int pull)
{
	msm_tlmm_set_field(tlmm_pull_cfgs, tgt, 2, pull);
}
EXPORT_SYMBOL(msm_tlmm_set_pull);

int gpio_tlmm_config(unsigned config, unsigned disable)
{
	unsigned gpio = GPIO_PIN(config);
	int ngpio = msm_gpio.gpio_chip.ngpio;

	if (gpio > ngpio)
		return -EINVAL;

	__gpio_tlmm_config(config);
	mb();

	return 0;
}
EXPORT_SYMBOL(gpio_tlmm_config);

#ifndef CONFIG_USE_PINCTRL_IRQ
int msm_gpio_install_direct_irq(unsigned gpio, unsigned irq,
					unsigned int input_polarity)
{
	unsigned long irq_flags;
	int ngpio = msm_gpio.gpio_chip.ngpio;

	if (gpio >= ngpio || irq >= nr_direct_connect_irqs)
		return -EINVAL;

	spin_lock_irqsave(&tlmm_lock, irq_flags);
	__msm_gpio_install_direct_irq(gpio, irq, input_polarity);
	mb();
	spin_unlock_irqrestore(&tlmm_lock, irq_flags);

	return 0;
}
EXPORT_SYMBOL(msm_gpio_install_direct_irq);

/*
 * This lock class tells lockdep that GPIO irqs are in a different
 * category than their parent, so it won't report false recursion.
 */
static struct lock_class_key msm_gpio_lock_class;

static inline void msm_gpio_set_irq_handler(struct device *dev)
{
	int irq, i;

	if (!dev->of_node) {
		for (i = 0; i < msm_gpio.gpio_chip.ngpio; ++i) {
			irq = msm_gpio_to_irq(&msm_gpio.gpio_chip, i);
			irq_set_lockdep_class(irq, &msm_gpio_lock_class);
			irq_set_chip_and_handler(irq, &msm_gpio_irq_chip,
						 handle_level_irq);
			set_irq_flags(irq, IRQF_VALID);
		}
	}
}

static int msm_gpio_setup_irqchip(struct platform_device *pdev)
{
	int ret = 0, ngpio;

	ngpio = msm_gpio.gpio_chip.ngpio;
	tlmm_msm_summary_irq = platform_get_irq(pdev, 0);
	if (tlmm_msm_summary_irq < 0) {
		pr_err("%s: No interrupt defined for msmgpio\n", __func__);
		return -ENXIO;
	}

	msm_gpio.enabled_irqs = devm_kzalloc(&pdev->dev, sizeof(unsigned long)
					* BITS_TO_LONGS(ngpio), GFP_KERNEL);
	if (!msm_gpio.enabled_irqs) {
		dev_err(&pdev->dev, "%s failed to allocate bitmap\n", __func__);
		return -ENOMEM;
	}

	msm_gpio.wake_irqs = devm_kzalloc(&pdev->dev, sizeof(unsigned long) *
					BITS_TO_LONGS(ngpio), GFP_KERNEL);
	if (!msm_gpio.wake_irqs) {
		dev_err(&pdev->dev, "%s failed to allocated wake_irqs bitmap\n"
				, __func__);
		return -ENOMEM;
	}

	bitmap_zero(msm_gpio.enabled_irqs, ngpio);
	bitmap_zero(msm_gpio.wake_irqs, ngpio);
	msm_gpio_set_irq_handler(&pdev->dev);

	ret = devm_request_irq(&pdev->dev, tlmm_msm_summary_irq,
			msm_summary_irq_handler, IRQF_TRIGGER_HIGH,
			"msmgpio", NULL);
	if (ret) {
		pr_err("Request_irq failed for tlmm_msm_summary_irq - %d\n",
				ret);
		return ret;
	}
	register_syscore_ops(&msm_gpio_syscore_ops);
	return 0;
}
#else
static inline void msm_gpio_set_irq_handler(struct device *dev)
{
	return;
}

static int msm_gpio_setup_irqchip(struct platform_device *pdev)
{
	return 0;
}
#endif

static int msm_gpio_probe(struct platform_device *pdev)
{
	int ret, ngpio = 0;
	struct msm_gpio_pdata *pdata = pdev->dev.platform_data;

	if (pdev->dev.of_node) {
		ret = of_property_read_u32(pdev->dev.of_node, "ngpio", &ngpio);
		if (ret) {
			pr_err("%s: Failed to find ngpio property\n", __func__);
			return ret;
		}
		ret = of_property_read_u32(pdev->dev.of_node,
					"qcom,direct-connect-irqs",
					&nr_direct_connect_irqs);
		if (ret) {
			pr_err("%s: Failed to find qcom,direct-connect-irqs property\n"
				, __func__);
			return ret;
		}
	} else {
		ngpio = pdata->ngpio;
		nr_direct_connect_irqs = pdata->direct_connect_irqs;
	}

	msm_gpio.gpio_chip.dev = &pdev->dev;
	msm_gpio.gpio_chip.ngpio = ngpio;
	spin_lock_init(&tlmm_lock);
	ret = msm_gpio_setup_irqchip(pdev);
	if (ret)
		return ret;
	ret = gpiochip_add(&msm_gpio.gpio_chip);
	if (ret < 0)
		return ret;
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id msm_gpio_of_match[] = {
	{.compatible = "qcom,msm-gpio", },
	{ },
};
#endif

static int msm_gpio_remove(struct platform_device *pdev)
{
	int ret;
#ifndef CONFIG_USE_PINCTRL_IRQ
	unregister_syscore_ops(&msm_gpio_syscore_ops);
#endif
	ret = gpiochip_remove(&msm_gpio.gpio_chip);
	if (ret < 0)
		return ret;
	irq_set_handler(tlmm_msm_summary_irq, NULL);

	return 0;
}

static struct platform_driver msm_gpio_driver = {
	.probe = msm_gpio_probe,
	.remove = msm_gpio_remove,
	.driver = {
		.name = "msmgpio",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(msm_gpio_of_match),
	},
};

static void __exit msm_gpio_exit(void)
{
	platform_driver_unregister(&msm_gpio_driver);
}
module_exit(msm_gpio_exit);

static int __init msm_gpio_init(void)
{
	return platform_driver_register(&msm_gpio_driver);
}
postcore_initcall(msm_gpio_init);

#ifdef CONFIG_OF
#ifndef CONFIG_USE_PINCTRL_IRQ
static int msm_gpio_irq_domain_xlate(struct irq_domain *d,
				     struct device_node *controller,
				     const u32 *intspec,
				     unsigned int intsize,
				     unsigned long *out_hwirq,
				     unsigned int *out_type)
{
	if (d->of_node != controller)
		return -EINVAL;
	if (intsize != 2)
		return -EINVAL;

	/* hwirq value */
	*out_hwirq = intspec[0];

	/* irq flags */
	*out_type = intspec[1] & IRQ_TYPE_SENSE_MASK;
	return 0;
}

static int msm_gpio_irq_domain_map(struct irq_domain *d, unsigned int irq,
				   irq_hw_number_t hwirq)
{
	irq_set_lockdep_class(irq, &msm_gpio_lock_class);
	irq_set_chip_and_handler(irq, &msm_gpio_irq_chip,
			handle_level_irq);
	set_irq_flags(irq, IRQF_VALID);

	return 0;
}

static struct irq_domain_ops msm_gpio_irq_domain_ops = {
	.xlate = msm_gpio_irq_domain_xlate,
	.map = msm_gpio_irq_domain_map,
};

int __init msm_gpio_of_init(struct device_node *node,
			    struct device_node *parent)
{
	int ngpio, ret;
	u32 subsys_id;

	ret = of_property_read_u32(node, "ngpio", &ngpio);
	if (ret) {
		WARN(1, "Cannot get numgpios from device tree\n");
		return ret;
	}
	msm_gpio.domain = irq_domain_add_linear(node, ngpio,
			&msm_gpio_irq_domain_ops, &msm_gpio);
	if (!msm_gpio.domain) {
		WARN(1, "Cannot allocate irq_domain\n");
		return -ENOMEM;
	}
	ret = of_property_read_u32(node, "qcom,subsys-id", &subsys_id);
	if (!ret)
		__msm_gpio_set_subsys_id(subsys_id);
	return 0;
}
#endif
#endif

MODULE_AUTHOR("Gregory Bean <gbean@codeaurora.org>");
MODULE_DESCRIPTION("Driver for Qualcomm MSM TLMMv2 SoC GPIOs");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("sysdev:msmgpio");
