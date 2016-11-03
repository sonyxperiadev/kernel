/*
 * Copyright (c) 2013, Sony Mobile Communications AB.
 * Copyright (c) 2013-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <linux/reboot.h>
#include <linux/irqchip/msm-mpm-irq.h>
#include "../core.h"
#include "../pinconf.h"
#include "pinctrl-msm.h"
#include "../pinctrl-utils.h"

#define MAX_NR_GPIO 300
#define PS_HOLD_OFFSET 0x820
#define TLMM_EBI2_EMMC_GPIO_CFG 0x111000

/**
 * struct msm_pinctrl - state for a pinctrl-msm device
 * @dev:            device handle.
 * @pctrl:          pinctrl handle.
 * @chip:           gpiochip handle.
 * @restart_nb:     restart notifier block.
 * @irq_chip_extn:  MPM extension of TLMM irqchip.
 * @irq:            parent irq for the TLMM irq_chip.
 * @lock:           Spinlock to protect register resources as well
 *                  as msm_pinctrl data structures.
 * @enabled_irqs:   Bitmap of currently enabled irqs.
 * @dual_edge_irqs: Bitmap of irqs that need sw emulated dual edge
 *                  detection.
 * @soc;            Reference to soc_data of platform specific data.
 * @regs:           Base address for the TLMM register map.
 */
struct msm_pinctrl {
	struct device *dev;
	struct pinctrl_dev *pctrl;
	struct gpio_chip chip;
	struct notifier_block restart_nb;
	struct irq_chip *irq_chip_extn;
	int irq;

	spinlock_t lock;

	DECLARE_BITMAP(dual_edge_irqs, MAX_NR_GPIO);
	DECLARE_BITMAP(enabled_irqs, MAX_NR_GPIO);
	DECLARE_BITMAP(disabled_pins, MAX_NR_GPIO);

	const struct msm_pinctrl_soc_data *soc;
	void __iomem *regs;
};

static struct msm_pinctrl *msm_pinctrl_data;

static inline struct msm_pinctrl *to_msm_pinctrl(struct gpio_chip *gc)
{
	return container_of(gc, struct msm_pinctrl, chip);
}

static int msm_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->ngroups;
}

static const char *msm_get_group_name(struct pinctrl_dev *pctldev,
				      unsigned group)
{
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->groups[group].name;
}

static int msm_get_group_pins(struct pinctrl_dev *pctldev,
			      unsigned group,
			      const unsigned **pins,
			      unsigned *num_pins)
{
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	*pins = pctrl->soc->groups[group].pins;
	*num_pins = pctrl->soc->groups[group].npins;
	return 0;
}

static const struct pinctrl_ops msm_pinctrl_ops = {
	.get_groups_count	= msm_get_groups_count,
	.get_group_name		= msm_get_group_name,
	.get_group_pins		= msm_get_group_pins,
	.dt_node_to_map		= pinconf_generic_dt_node_to_map_group,
	.dt_free_map		= pinctrl_utils_dt_free_map,
};

static int msm_get_functions_count(struct pinctrl_dev *pctldev)
{
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->nfunctions;
}

static const char *msm_get_function_name(struct pinctrl_dev *pctldev,
					 unsigned function)
{
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	return pctrl->soc->functions[function].name;
}

static int msm_get_function_groups(struct pinctrl_dev *pctldev,
				   unsigned function,
				   const char * const **groups,
				   unsigned * const num_groups)
{
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);

	*groups = pctrl->soc->functions[function].groups;
	*num_groups = pctrl->soc->functions[function].ngroups;
	return 0;
}

static int msm_pinmux_set_mux(struct pinctrl_dev *pctldev,
			      unsigned function,
			      unsigned group)
{
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	const struct msm_pingroup *g;
	unsigned long flags;
	u32 val;
	int i;

	g = &pctrl->soc->groups[group];

	for (i = 0; i < g->nfuncs; i++) {
		if (g->funcs[i] == function)
			break;
	}

	if (WARN_ON(i == g->nfuncs))
		return -EINVAL;

	spin_lock_irqsave(&pctrl->lock, flags);

	val = readl(pctrl->regs + g->ctl_reg);
	val &= ~(0x7 << g->mux_bit);
	val |= i << g->mux_bit;
	writel(val, pctrl->regs + g->ctl_reg);

	spin_unlock_irqrestore(&pctrl->lock, flags);

	return 0;
}

static const struct pinmux_ops msm_pinmux_ops = {
	.get_functions_count	= msm_get_functions_count,
	.get_function_name	= msm_get_function_name,
	.get_function_groups	= msm_get_function_groups,
	.set_mux		= msm_pinmux_set_mux,
};

static int msm_config_reg(struct msm_pinctrl *pctrl,
			  const struct msm_pingroup *g,
			  unsigned param,
			  unsigned *mask,
			  unsigned *bit)
{
	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_BUS_HOLD:
	case PIN_CONFIG_BIAS_PULL_UP:
		*bit = g->pull_bit;
		*mask = 3;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		*bit = g->drv_bit;
		*mask = 7;
		break;
	case PIN_CONFIG_OUTPUT:
	case PIN_CONFIG_INPUT_ENABLE:
		*bit = g->oe_bit;
		*mask = 1;
		break;
	default:
		dev_err(pctrl->dev, "Invalid config param %04x\n", param);
		return -ENOTSUPP;
	}

	return 0;
}

static int msm_config_get(struct pinctrl_dev *pctldev,
			  unsigned int pin,
			  unsigned long *config)
{
	dev_err(pctldev->dev, "pin_config_set op not supported\n");
	return -ENOTSUPP;
}

static int msm_config_set(struct pinctrl_dev *pctldev, unsigned int pin,
				unsigned long *configs, unsigned num_configs)
{
	dev_err(pctldev->dev, "pin_config_set op not supported\n");
	return -ENOTSUPP;
}

#define MSM_NO_PULL	0
#define MSM_PULL_DOWN	1
#define MSM_KEEPER	2
#define MSM_PULL_UP	3

static unsigned msm_regval_to_drive(u32 val)
{
	return (val + 1) * 2;
}

static int msm_config_group_get(struct pinctrl_dev *pctldev,
				unsigned int group,
				unsigned long *config)
{
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	unsigned param = pinconf_to_config_param(*config);
	unsigned mask;
	unsigned arg;
	unsigned bit;
	int ret;
	u32 val;

	g = &pctrl->soc->groups[group];

	ret = msm_config_reg(pctrl, g, param, &mask, &bit);
	if (ret < 0)
		return ret;

	val = readl(pctrl->regs + g->ctl_reg);
	arg = (val >> bit) & mask;

	/* Convert register value to pinconf value */
	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		arg = arg == MSM_NO_PULL;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		arg = arg == MSM_PULL_DOWN;
		break;
	case PIN_CONFIG_BIAS_BUS_HOLD:
		arg = arg == MSM_KEEPER;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		arg = arg == MSM_PULL_UP;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		arg = msm_regval_to_drive(arg);
		break;
	case PIN_CONFIG_OUTPUT:
		/* Pin is not output */
		if (!arg)
			return -EINVAL;

		val = readl(pctrl->regs + g->io_reg);
		arg = !!(val & BIT(g->in_bit));
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		/* Pin is output */
		if (arg)
			return -EINVAL;
		arg = 1;
		break;
	default:
		dev_err(pctrl->dev, "Unsupported config parameter: %x\n",
			param);
		return -EINVAL;
	}

	*config = pinconf_to_config_packed(param, arg);

	return 0;
}

static int msm_config_group_set(struct pinctrl_dev *pctldev,
				unsigned group,
				unsigned long *configs,
				unsigned num_configs)
{
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = pinctrl_dev_get_drvdata(pctldev);
	unsigned long flags;
	unsigned param;
	unsigned mask;
	unsigned arg;
	unsigned bit;
	int ret;
	u32 val;
	int i;

	g = &pctrl->soc->groups[group];

	for (i = 0; i < num_configs; i++) {
		param = pinconf_to_config_param(configs[i]);
		arg = pinconf_to_config_argument(configs[i]);

		ret = msm_config_reg(pctrl, g, param, &mask, &bit);
		if (ret < 0)
			return ret;

		/* Convert pinconf values to register values */
		switch (param) {
		case PIN_CONFIG_BIAS_DISABLE:
			arg = MSM_NO_PULL;
			break;
		case PIN_CONFIG_BIAS_PULL_DOWN:
			arg = MSM_PULL_DOWN;
			break;
		case PIN_CONFIG_BIAS_BUS_HOLD:
			arg = MSM_KEEPER;
			break;
		case PIN_CONFIG_BIAS_PULL_UP:
			arg = MSM_PULL_UP;
			break;
		case PIN_CONFIG_DRIVE_STRENGTH:
			/* Check for invalid values */
			if (arg > 16 || arg < 2 || (arg % 2) != 0)
				arg = -1;
			else
				arg = (arg / 2) - 1;
			break;
		case PIN_CONFIG_OUTPUT:
			/* set output value */
			spin_lock_irqsave(&pctrl->lock, flags);
			val = readl(pctrl->regs + g->io_reg);
			if (arg)
				val |= BIT(g->out_bit);
			else
				val &= ~BIT(g->out_bit);
			writel(val, pctrl->regs + g->io_reg);
			spin_unlock_irqrestore(&pctrl->lock, flags);

			/* enable output */
			arg = 1;
			break;
		case PIN_CONFIG_INPUT_ENABLE:
			/* disable output */
			arg = 0;
			break;
		default:
			dev_err(pctrl->dev, "Unsupported config parameter: %x\n",
				param);
			return -EINVAL;
		}

		/* Range-check user-supplied value */
		if (arg & ~mask) {
			dev_err(pctrl->dev, "config %x: %x is invalid\n", param, arg);
			return -EINVAL;
		}

		spin_lock_irqsave(&pctrl->lock, flags);
		val = readl(pctrl->regs + g->ctl_reg);
		val &= ~(mask << bit);
		val |= arg << bit;
		writel(val, pctrl->regs + g->ctl_reg);
		spin_unlock_irqrestore(&pctrl->lock, flags);
	}

	return 0;
}

static const struct pinconf_ops msm_pinconf_ops = {
	.pin_config_get		= msm_config_get,
	.pin_config_set		= msm_config_set,
	.pin_config_group_get	= msm_config_group_get,
	.pin_config_group_set	= msm_config_group_set,
};

static struct pinctrl_desc msm_pinctrl_desc = {
	.pctlops = &msm_pinctrl_ops,
	.pmxops = &msm_pinmux_ops,
	.confops = &msm_pinconf_ops,
	.owner = THIS_MODULE,
};

static int msm_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = container_of(chip, struct msm_pinctrl, chip);
	unsigned long flags;
	u32 val;

	g = &pctrl->soc->groups[offset];

	spin_lock_irqsave(&pctrl->lock, flags);

	val = readl(pctrl->regs + g->ctl_reg);
	val &= ~BIT(g->oe_bit);
	writel(val, pctrl->regs + g->ctl_reg);

	spin_unlock_irqrestore(&pctrl->lock, flags);

	return 0;
}

static int msm_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = container_of(chip, struct msm_pinctrl, chip);
	unsigned long flags;
	u32 val;

	g = &pctrl->soc->groups[offset];

	spin_lock_irqsave(&pctrl->lock, flags);

	val = readl(pctrl->regs + g->io_reg);
	if (value)
		val |= BIT(g->out_bit);
	else
		val &= ~BIT(g->out_bit);
	writel(val, pctrl->regs + g->io_reg);

	val = readl(pctrl->regs + g->ctl_reg);
	val |= BIT(g->oe_bit);
	writel(val, pctrl->regs + g->ctl_reg);

	spin_unlock_irqrestore(&pctrl->lock, flags);

	return 0;
}

static int msm_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = container_of(chip, struct msm_pinctrl, chip);
	u32 val;

	g = &pctrl->soc->groups[offset];

	val = readl(pctrl->regs + g->io_reg);
	return !!(val & BIT(g->in_bit));
}

static void msm_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = container_of(chip, struct msm_pinctrl, chip);
	unsigned long flags;
	u32 val;

	g = &pctrl->soc->groups[offset];

	spin_lock_irqsave(&pctrl->lock, flags);

	val = readl(pctrl->regs + g->io_reg);
	if (value)
		val |= BIT(g->out_bit);
	else
		val &= ~BIT(g->out_bit);
	writel(val, pctrl->regs + g->io_reg);

	spin_unlock_irqrestore(&pctrl->lock, flags);
}

static int msm_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	int gpio = chip->base + offset;
	return pinctrl_request_gpio(gpio);
}

static void msm_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	int gpio = chip->base + offset;
	return pinctrl_free_gpio(gpio);
}

#ifdef CONFIG_DEBUG_FS
#include <linux/seq_file.h>

static void msm_gpio_dbg_show_one(struct seq_file *s,
				  struct pinctrl_dev *pctldev,
				  struct gpio_chip *chip,
				  unsigned offset,
				  unsigned gpio)
{
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = container_of(chip, struct msm_pinctrl, chip);
	unsigned func;
	int is_out;
	int drive;
	int pull;
	u32 ctl_reg;

	static const char * const pulls[] = {
		"no pull",
		"pull down",
		"keeper",
		"pull up"
	};

	g = &pctrl->soc->groups[offset];
	ctl_reg = readl(pctrl->regs + g->ctl_reg);

	is_out = !!(ctl_reg & BIT(g->oe_bit));
	func = (ctl_reg >> g->mux_bit) & 7;
	drive = (ctl_reg >> g->drv_bit) & 7;
	pull = (ctl_reg >> g->pull_bit) & 3;

	seq_printf(s, " %-8s: %-3s %d", g->name, is_out ? "out" : "in", func);
	seq_printf(s, " %dmA", msm_regval_to_drive(drive));
	seq_printf(s, " %s", pulls[pull]);
}

static void msm_gpio_dbg_show(struct seq_file *s, struct gpio_chip *chip)
{
	unsigned gpio = chip->base;
	unsigned i;
	struct msm_pinctrl *pctrl = container_of(chip,
			struct msm_pinctrl, chip);

	for (i = 0; i < chip->ngpio; i++, gpio++) {
		if (test_bit(i, pctrl->disabled_pins))
			seq_printf(s, " gpio%d is not accessible.", i);
		else
			msm_gpio_dbg_show_one(s, NULL, chip, i, gpio);
		seq_puts(s, "\n");
	}
}

#else
#define msm_gpio_dbg_show NULL
#endif

static struct gpio_chip msm_gpio_template = {
	.direction_input  = msm_gpio_direction_input,
	.direction_output = msm_gpio_direction_output,
	.get              = msm_gpio_get,
	.set              = msm_gpio_set,
	.request          = msm_gpio_request,
	.free             = msm_gpio_free,
	.dbg_show         = msm_gpio_dbg_show,
};

/* For dual-edge interrupts in software, since some hardware has no
 * such support:
 *
 * At appropriate moments, this function may be called to flip the polarity
 * settings of both-edge irq lines to try and catch the next edge.
 *
 * The attempt is considered successful if:
 * - the status bit goes high, indicating that an edge was caught, or
 * - the input value of the gpio doesn't change during the attempt.
 * If the value changes twice during the process, that would cause the first
 * test to fail but would force the second, as two opposite
 * transitions would cause a detection no matter the polarity setting.
 *
 * The do-loop tries to sledge-hammer closed the timing hole between
 * the initial value-read and the polarity-write - if the line value changes
 * during that window, an interrupt is lost, the new polarity setting is
 * incorrect, and the first success test will fail, causing a retry.
 *
 * Algorithm comes from Google's msmgpio driver.
 */
static void msm_gpio_update_dual_edge_pos(struct msm_pinctrl *pctrl,
					  const struct msm_pingroup *g,
					  struct irq_data *d)
{
	int loop_limit = 100;
	unsigned val, val2, intstat;
	unsigned pol;

	do {
		val = readl(pctrl->regs + g->io_reg) & BIT(g->in_bit);

		pol = readl(pctrl->regs + g->intr_cfg_reg);
		pol ^= BIT(g->intr_polarity_bit);
		writel(pol, pctrl->regs + g->intr_cfg_reg);

		val2 = readl(pctrl->regs + g->io_reg) & BIT(g->in_bit);
		intstat = readl(pctrl->regs + g->intr_status_reg);
		if (intstat || (val == val2))
			return;
	} while (loop_limit-- > 0);
	dev_err(pctrl->dev, "dual-edge irq failed to stabilize, %#08x != %#08x\n",
		val, val2);
}

static void msm_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct msm_pinctrl *pctrl = to_msm_pinctrl(gc);
	const struct msm_pingroup *g;
	unsigned long flags;
	u32 val;

	g = &pctrl->soc->groups[d->hwirq];

	spin_lock_irqsave(&pctrl->lock, flags);

	val = readl(pctrl->regs + g->intr_cfg_reg);
	val &= ~BIT(g->intr_enable_bit);
	writel(val, pctrl->regs + g->intr_cfg_reg);

	clear_bit(d->hwirq, pctrl->enabled_irqs);

	spin_unlock_irqrestore(&pctrl->lock, flags);
	if (pctrl->irq_chip_extn->irq_mask)
		pctrl->irq_chip_extn->irq_mask(d);
}

static void msm_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct msm_pinctrl *pctrl = to_msm_pinctrl(gc);
	const struct msm_pingroup *g;
	unsigned long flags;
	u32 val;

	g = &pctrl->soc->groups[d->hwirq];

	spin_lock_irqsave(&pctrl->lock, flags);

	val = readl(pctrl->regs + g->intr_status_reg);
	val &= ~BIT(g->intr_status_bit);
	writel(val, pctrl->regs + g->intr_status_reg);

	val = readl(pctrl->regs + g->intr_cfg_reg);
	val |= BIT(g->intr_enable_bit);
	writel(val, pctrl->regs + g->intr_cfg_reg);

	set_bit(d->hwirq, pctrl->enabled_irqs);

	spin_unlock_irqrestore(&pctrl->lock, flags);
	if (pctrl->irq_chip_extn->irq_unmask)
		pctrl->irq_chip_extn->irq_unmask(d);
}

static void msm_gpio_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct msm_pinctrl *pctrl = to_msm_pinctrl(gc);
	const struct msm_pingroup *g;
	unsigned long flags;
	u32 val;

	g = &pctrl->soc->groups[d->hwirq];

	spin_lock_irqsave(&pctrl->lock, flags);

	val = readl(pctrl->regs + g->intr_status_reg);
	if (g->intr_ack_high)
		val |= BIT(g->intr_status_bit);
	else
		val &= ~BIT(g->intr_status_bit);
	writel(val, pctrl->regs + g->intr_status_reg);

	if (test_bit(d->hwirq, pctrl->dual_edge_irqs))
		msm_gpio_update_dual_edge_pos(pctrl, g, d);

	spin_unlock_irqrestore(&pctrl->lock, flags);
}

static int msm_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct msm_pinctrl *pctrl = to_msm_pinctrl(gc);
	const struct msm_pingroup *g;
	unsigned long flags;
	u32 val;

	g = &pctrl->soc->groups[d->hwirq];

	spin_lock_irqsave(&pctrl->lock, flags);

	/*
	 * For hw without possibility of detecting both edges
	 */
	if (g->intr_detection_width == 1 && type == IRQ_TYPE_EDGE_BOTH)
		set_bit(d->hwirq, pctrl->dual_edge_irqs);
	else
		clear_bit(d->hwirq, pctrl->dual_edge_irqs);

	/* Route interrupts to application cpu */
	val = readl(pctrl->regs + g->intr_target_reg);
	val &= ~(7 << g->intr_target_bit);
	val |= g->intr_target_kpss_val << g->intr_target_bit;
	writel(val, pctrl->regs + g->intr_target_reg);

	/* Update configuration for gpio.
	 * RAW_STATUS_EN is left on for all gpio irqs. Due to the
	 * internal circuitry of TLMM, toggling the RAW_STATUS
	 * could cause the INTR_STATUS to be set for EDGE interrupts.
	 */
	val = readl(pctrl->regs + g->intr_cfg_reg);
	val |= BIT(g->intr_raw_status_bit);
	if (g->intr_detection_width == 2) {
		val &= ~(3 << g->intr_detection_bit);
		val &= ~(1 << g->intr_polarity_bit);
		switch (type) {
		case IRQ_TYPE_EDGE_RISING:
			val |= 1 << g->intr_detection_bit;
			val |= BIT(g->intr_polarity_bit);
			break;
		case IRQ_TYPE_EDGE_FALLING:
			val |= 2 << g->intr_detection_bit;
			val |= BIT(g->intr_polarity_bit);
			break;
		case IRQ_TYPE_EDGE_BOTH:
			val |= 3 << g->intr_detection_bit;
			val |= BIT(g->intr_polarity_bit);
			break;
		case IRQ_TYPE_LEVEL_LOW:
			break;
		case IRQ_TYPE_LEVEL_HIGH:
			val |= BIT(g->intr_polarity_bit);
			break;
		}
	} else if (g->intr_detection_width == 1) {
		val &= ~(1 << g->intr_detection_bit);
		val &= ~(1 << g->intr_polarity_bit);
		switch (type) {
		case IRQ_TYPE_EDGE_RISING:
			val |= BIT(g->intr_detection_bit);
			val |= BIT(g->intr_polarity_bit);
			break;
		case IRQ_TYPE_EDGE_FALLING:
			val |= BIT(g->intr_detection_bit);
			break;
		case IRQ_TYPE_EDGE_BOTH:
			val |= BIT(g->intr_detection_bit);
			val |= BIT(g->intr_polarity_bit);
			break;
		case IRQ_TYPE_LEVEL_LOW:
			break;
		case IRQ_TYPE_LEVEL_HIGH:
			val |= BIT(g->intr_polarity_bit);
			break;
		}
	} else {
		BUG();
	}
	writel(val, pctrl->regs + g->intr_cfg_reg);

	if (test_bit(d->hwirq, pctrl->dual_edge_irqs))
		msm_gpio_update_dual_edge_pos(pctrl, g, d);

	spin_unlock_irqrestore(&pctrl->lock, flags);

	if (type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH))
		__irq_set_handler_locked(d->irq, handle_level_irq);
	else if (type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		__irq_set_handler_locked(d->irq, handle_edge_irq);

	if (pctrl->irq_chip_extn->irq_set_type)
		pctrl->irq_chip_extn->irq_set_type(d, type);
	return 0;
}

static int msm_gpio_irq_set_wake(struct irq_data *d, unsigned int on)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct msm_pinctrl *pctrl = to_msm_pinctrl(gc);
	unsigned long flags;

	spin_lock_irqsave(&pctrl->lock, flags);

	irq_set_irq_wake(pctrl->irq, on);

	spin_unlock_irqrestore(&pctrl->lock, flags);

	if (pctrl->irq_chip_extn->irq_set_wake)
		pctrl->irq_chip_extn->irq_set_wake(d, on);
	return 0;
}

static struct irq_chip msm_gpio_irq_chip = {
	.name           = "msmgpio",
	.irq_mask       = msm_gpio_irq_mask,
	.irq_unmask     = msm_gpio_irq_unmask,
	.irq_ack        = msm_gpio_irq_ack,
	.irq_set_type   = msm_gpio_irq_set_type,
	.irq_set_wake   = msm_gpio_irq_set_wake,
};

static void msm_gpio_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	struct gpio_chip *gc = irq_desc_get_handler_data(desc);
	const struct msm_pingroup *g;
	struct msm_pinctrl *pctrl = to_msm_pinctrl(gc);
	struct irq_chip *chip = irq_get_chip(irq);
	int irq_pin;
	int handled = 0;
	u32 val;
	int i;

	chained_irq_enter(chip, desc);

	/*
	 * Each pin has it's own IRQ status register, so use
	 * enabled_irq bitmap to limit the number of reads.
	 */
	for_each_set_bit(i, pctrl->enabled_irqs, pctrl->chip.ngpio) {
		g = &pctrl->soc->groups[i];
		val = readl(pctrl->regs + g->intr_status_reg);
		if (val & BIT(g->intr_status_bit)) {
			irq_pin = irq_find_mapping(gc->irqdomain, i);
			generic_handle_irq(irq_pin);
			handled++;
		}
	}

	/* No interrupts were flagged */
	if (handled == 0)
		handle_bad_irq(irq, desc);

	chained_irq_exit(chip, desc);
}

/*
 * Add MPM extensions for the irqchip.
 * Enable the MPM driver to enable/disable
 * suspend/resume based on gpio interrupts
 */

struct irq_chip mpm_pinctrl_extn = {
	.irq_eoi	= NULL,
	.irq_mask	= NULL,
	.irq_unmask	= NULL,
	.irq_retrigger	= NULL,
	.irq_set_type	= NULL,
	.irq_set_wake	= NULL,
	.irq_disable	= NULL,
};

static int msm_gpio_init(struct msm_pinctrl *pctrl)
{
	struct gpio_chip *chip;
	int ret;
	unsigned ngpio = pctrl->soc->ngpios;

	if (WARN_ON(ngpio > MAX_NR_GPIO))
		return -EINVAL;

	chip = &pctrl->chip;
	chip->base = 0;
	chip->ngpio = ngpio;
	chip->label = dev_name(pctrl->dev);
	chip->dev = pctrl->dev;
	chip->owner = THIS_MODULE;
	chip->of_node = pctrl->dev->of_node;

	ret = gpiochip_add(&pctrl->chip);
	if (ret) {
		dev_err(pctrl->dev, "Failed register gpiochip\n");
		return ret;
	}

	ret = gpiochip_add_pin_range(&pctrl->chip, dev_name(pctrl->dev), 0, 0, chip->ngpio);
	if (ret) {
		dev_err(pctrl->dev, "Failed to add pin range\n");
		gpiochip_remove(&pctrl->chip);
		return ret;
	}

	ret = gpiochip_irqchip_add(chip,
				   &msm_gpio_irq_chip,
				   0,
				   handle_edge_irq,
				   IRQ_TYPE_NONE);
	if (ret) {
		dev_err(pctrl->dev, "Failed to add irqchip to gpiochip\n");
		gpiochip_remove(&pctrl->chip);
		return -ENOSYS;
	}

	gpiochip_set_chained_irqchip(chip, &msm_gpio_irq_chip, pctrl->irq,
				     msm_gpio_irq_handler);
	of_mpm_init();

	return 0;
}

static int msm_ps_hold_restart(struct notifier_block *nb, unsigned long action,
			       void *data)
{
	struct msm_pinctrl *pctrl = container_of(nb, struct msm_pinctrl, restart_nb);

	writel(0, pctrl->regs + PS_HOLD_OFFSET);
	mdelay(1000);
	return NOTIFY_DONE;
}

static void msm_pinctrl_setup_pm_reset(struct msm_pinctrl *pctrl)
{
	int i;
	const struct msm_function *func = pctrl->soc->functions;

	for (i = 0; i < pctrl->soc->nfunctions; i++)
		if (!strcmp(func[i].name, "ps_hold")) {
			pctrl->restart_nb.notifier_call = msm_ps_hold_restart;
			pctrl->restart_nb.priority = 128;
			if (register_restart_handler(&pctrl->restart_nb))
				dev_err(pctrl->dev,
					"failed to setup restart handler.\n");
			break;
		}
}

static void msm_pinctrl_ebi2_emmc_enable(struct msm_pinctrl *pctrl,
					unsigned int tlmm_emmc_boot_select)
{
	u32 val;

	val = readl_relaxed(pctrl->regs + TLMM_EBI2_EMMC_GPIO_CFG);
	val |= BIT(tlmm_emmc_boot_select);
	writel_relaxed(val, pctrl->regs + TLMM_EBI2_EMMC_GPIO_CFG);
}

#ifdef CONFIG_PM
static int msm_pinctrl_suspend(void)
{
	return 0;
}

static void msm_pinctrl_resume(void)
{
	int i, irq;
	u32 val;
	unsigned long flags;
	struct irq_desc *desc;
	const struct msm_pingroup *g;
	const char *name = "null";
	struct msm_pinctrl *pctrl = msm_pinctrl_data;

	if (!msm_show_resume_irq_mask)
		return;

	spin_lock_irqsave(&pctrl->lock, flags);
	for_each_set_bit(i, pctrl->enabled_irqs, pctrl->chip.ngpio) {
		g = &pctrl->soc->groups[i];
		val = readl_relaxed(pctrl->regs + g->intr_status_reg);
		if (val & BIT(g->intr_status_bit)) {
			irq = irq_find_mapping(pctrl->chip.irqdomain, i);
			desc = irq_to_desc(irq);
			if (desc == NULL)
				name = "stray irq";
			else if (desc->action && desc->action->name)
				name = desc->action->name;

			pr_warn("%s: %d triggered %s\n", __func__, irq, name);
		}
	}
	spin_unlock_irqrestore(&pctrl->lock, flags);
}
#else
#define msm_pinctrl_suspend NULL
#define msm_pinctrl_resume NULL
#endif

static struct syscore_ops msm_pinctrl_pm_ops = {
	.suspend = msm_pinctrl_suspend,
	.resume = msm_pinctrl_resume,
};

int msm_pinctrl_probe(struct platform_device *pdev,
		      const struct msm_pinctrl_soc_data *soc_data)
{
	struct msm_pinctrl *pctrl;
	struct resource *res;
	int ret;
	u32 tlmm_emmc_boot_select;
	int disabled_pins_num;
	const struct device_node *np = pdev->dev.of_node;

	msm_pinctrl_data = pctrl = devm_kzalloc(&pdev->dev,
				sizeof(*pctrl), GFP_KERNEL);
	if (!pctrl) {
		dev_err(&pdev->dev, "Can't allocate msm_pinctrl\n");
		return -ENOMEM;
	}
	pctrl->dev = &pdev->dev;
	pctrl->soc = soc_data;
	pctrl->chip = msm_gpio_template;

	spin_lock_init(&pctrl->lock);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pctrl->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pctrl->regs))
		return PTR_ERR(pctrl->regs);

	msm_pinctrl_setup_pm_reset(pctrl);
	if (!of_property_read_u32(pctrl->dev->of_node,
					"qcom,tlmm-emmc-boot-select",
						&tlmm_emmc_boot_select)) {
		msm_pinctrl_ebi2_emmc_enable(pctrl, tlmm_emmc_boot_select);
	}

	pctrl->irq = platform_get_irq(pdev, 0);
	if (pctrl->irq < 0) {
		dev_err(&pdev->dev, "No interrupt defined for msmgpio\n");
		return pctrl->irq;
	}

	msm_pinctrl_desc.name = dev_name(&pdev->dev);
	msm_pinctrl_desc.pins = pctrl->soc->pins;
	msm_pinctrl_desc.npins = pctrl->soc->npins;
	pctrl->pctrl = pinctrl_register(&msm_pinctrl_desc, &pdev->dev, pctrl);
	if (!pctrl->pctrl) {
		dev_err(&pdev->dev, "Couldn't register pinctrl driver\n");
		return -ENODEV;
	}

	disabled_pins_num = of_property_count_u32_elems(np, "disabled-pins");
	if (disabled_pins_num > 0) {
		int i;
		u32 pin;

		for (i = 0; i < disabled_pins_num; i++) {
			of_property_read_u32_index(np,
					"disabled-pins", i, &pin);
			if (pin < MAX_NR_GPIO) {
				set_bit(pin, pctrl->disabled_pins);
				dev_info(&pdev->dev, "pin %d disabled\n", pin);
			} else {
				dev_err(&pdev->dev, "pin %d out of range\n",
						pin);
			}
		}
	}
	ret = msm_gpio_init(pctrl);
	if (ret) {
		pinctrl_unregister(pctrl->pctrl);
		return ret;
	}
	pctrl->irq_chip_extn = &mpm_pinctrl_extn;
	platform_set_drvdata(pdev, pctrl);

	register_syscore_ops(&msm_pinctrl_pm_ops);
	dev_dbg(&pdev->dev, "Probed Qualcomm pinctrl driver\n");

	return 0;
}
EXPORT_SYMBOL(msm_pinctrl_probe);

int msm_pinctrl_remove(struct platform_device *pdev)
{
	struct msm_pinctrl *pctrl = platform_get_drvdata(pdev);

	gpiochip_remove(&pctrl->chip);
	pinctrl_unregister(pctrl->pctrl);

	unregister_restart_handler(&pctrl->restart_nb);
	unregister_syscore_ops(&msm_pinctrl_pm_ops);

	return 0;
}
EXPORT_SYMBOL(msm_pinctrl_remove);

#ifdef CONFIG_USE_PINCTRL_IRQ
#include "pinctrl-msm-tlmm.h"
extern int msm_tlmm_of_gp_irq_init(struct device_node *controller,
			    struct irq_chip *chip_extn);

/**
 * struct msm_irq_of_info: represents of init data for tlmm interrupt
 * controllers
 * @compat: compat string for tlmm interrup controller instance.
 * @irq_init: irq chip initialization callback.
 */
struct msm_irq_of_info {
	const char *compat;
	int (*irq_init)(struct device_node *np, struct irq_chip *ic);
};

struct irq_chip mpm_tlmm_irq_extn = {
	.irq_eoi	= NULL,
	.irq_mask	= NULL,
	.irq_unmask	= NULL,
	.irq_retrigger	= NULL,
	.irq_set_type	= NULL,
	.irq_set_wake	= NULL,
	.irq_disable	= NULL,
};

struct msm_irq_of_info msm_tlmm_irq[] = {
#ifdef CONFIG_PINCTRL_MSM_TLMM
	{
		.compat = "qcom,msm-tlmm-gp",
		.irq_init = msm_tlmm_of_gp_irq_init,
	},
#endif
};

int __init msm_tlmm_of_irq_init(struct device_node *controller,
						struct device_node *parent)
{
	int rc, i;
	const char *compat;

	rc = of_property_read_string(controller, "compatible", &compat);
	if (rc)
		return rc;

	for (i = 0; i < ARRAY_SIZE(msm_tlmm_irq); i++) {
		struct msm_irq_of_info *tlmm_info = &msm_tlmm_irq[i];

		if (!of_compat_cmp(tlmm_info->compat, compat, strlen(compat)))
				return tlmm_info->irq_init(controller,
							&mpm_tlmm_irq_extn);

	}
	return -EIO;
}
#endif
