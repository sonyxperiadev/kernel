/*
 * SuperH Pin Function Controller pinmux support.
 *
 * Copyright (C) 2012  Paul Mundt
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#define DRV_NAME "sh-pfc"

#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "core.h"
#include "../core.h"
#include "../pinconf.h"

struct sh_pfc_pin_config {
	u32 type;
};

struct sh_pfc_pinctrl {
	struct pinctrl_dev *pctl;
	struct pinctrl_desc pctl_desc;

	struct sh_pfc *pfc;

	struct pinctrl_pin_desc *pins;
	struct sh_pfc_pin_config *configs;
};

static int sh_pfc_get_groups_count(struct pinctrl_dev *pctldev)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);

	return pmx->pfc->info->nr_groups;
}

static const char *sh_pfc_get_group_name(struct pinctrl_dev *pctldev,
					 unsigned selector)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);

	return pmx->pfc->info->groups[selector].name;
}

static int sh_pfc_get_group_pins(struct pinctrl_dev *pctldev, unsigned selector,
				 const unsigned **pins, unsigned *num_pins)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);

	*pins = pmx->pfc->info->groups[selector].pins;
	*num_pins = pmx->pfc->info->groups[selector].nr_pins;

	return 0;
}

static void sh_pfc_pin_dbg_show(struct pinctrl_dev *pctldev, struct seq_file *s,
				unsigned offset)
{
	seq_printf(s, "%s", DRV_NAME);
}

static const struct pinctrl_ops sh_pfc_pinctrl_ops = {
	.get_groups_count	= sh_pfc_get_groups_count,
	.get_group_name		= sh_pfc_get_group_name,
	.get_group_pins		= sh_pfc_get_group_pins,
	.pin_dbg_show		= sh_pfc_pin_dbg_show,
};

static int sh_pfc_get_functions_count(struct pinctrl_dev *pctldev)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);

	return pmx->pfc->info->nr_functions;
}

static const char *sh_pfc_get_function_name(struct pinctrl_dev *pctldev,
					    unsigned selector)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);

	return pmx->pfc->info->functions[selector].name;
}

static int sh_pfc_get_function_groups(struct pinctrl_dev *pctldev,
				      unsigned selector,
				      const char * const **groups,
				      unsigned * const num_groups)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);

	*groups = pmx->pfc->info->functions[selector].groups;
	*num_groups = pmx->pfc->info->functions[selector].nr_groups;

	return 0;
}

static int sh_pfc_func_enable(struct pinctrl_dev *pctldev, unsigned selector,
			      unsigned group)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	struct sh_pfc *pfc = pmx->pfc;
	const struct sh_pfc_pin_group *grp = &pfc->info->groups[group];
	unsigned long flags;
	unsigned int i;
	int ret = 0;

	spin_lock_irqsave(&pfc->lock, flags);

	for (i = 0; i < grp->nr_pins; ++i) {
		int idx = sh_pfc_get_pin_index(pfc, grp->pins[i]);
		struct sh_pfc_pin_config *cfg = &pmx->configs[idx];

		if (cfg->type != PINMUX_TYPE_NONE) {
			ret = -EBUSY;
			goto done;
		}
	}

	for (i = 0; i < grp->nr_pins; ++i) {
		ret = sh_pfc_config_mux(pfc, grp->mux[i], PINMUX_TYPE_FUNCTION);
		if (ret < 0)
			break;
	}

done:
	spin_unlock_irqrestore(&pfc->lock, flags);
	return ret;
}

static void sh_pfc_func_disable(struct pinctrl_dev *pctldev, unsigned selector,
				unsigned group)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	struct sh_pfc *pfc = pmx->pfc;
	const struct sh_pfc_pin_group *grp = &pfc->info->groups[group];
	unsigned long flags;
	unsigned int i;

	spin_lock_irqsave(&pfc->lock, flags);

	for (i = 0; i < grp->nr_pins; ++i) {
		int idx = sh_pfc_get_pin_index(pfc, grp->pins[i]);
		struct sh_pfc_pin_config *cfg = &pmx->configs[idx];

		cfg->type = PINMUX_TYPE_NONE;
	}

	spin_unlock_irqrestore(&pfc->lock, flags);
}

static int sh_pfc_gpio_request_enable(struct pinctrl_dev *pctldev,
				      struct pinctrl_gpio_range *range,
				      unsigned offset)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	struct sh_pfc *pfc = pmx->pfc;
	int idx = sh_pfc_get_pin_index(pfc, offset);
	struct sh_pfc_pin_config *cfg = &pmx->configs[idx];
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&pfc->lock, flags);

	if (cfg->type != PINMUX_TYPE_NONE) {
		dev_err(pfc->dev,
			"Pin %u is busy, can't configure it as GPIO.\n",
			offset);
		ret = -EBUSY;
		goto done;
	}

	if (!pfc->gpio) {
		/* If GPIOs are handled externally the pin mux type need to be
		 * set to GPIO here.
		 */
		const struct sh_pfc_pin *pin = &pfc->info->pins[idx];

		ret = sh_pfc_config_mux(pfc, pin->enum_id, PINMUX_TYPE_GPIO);
		if (ret < 0)
			goto done;
	}

	cfg->type = PINMUX_TYPE_GPIO;

	ret = 0;

done:
	spin_unlock_irqrestore(&pfc->lock, flags);

	return ret;
}

static void sh_pfc_gpio_disable_free(struct pinctrl_dev *pctldev,
				     struct pinctrl_gpio_range *range,
				     unsigned offset)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	struct sh_pfc *pfc = pmx->pfc;
	int idx = sh_pfc_get_pin_index(pfc, offset);
	struct sh_pfc_pin_config *cfg = &pmx->configs[idx];
	unsigned long flags;

	spin_lock_irqsave(&pfc->lock, flags);
	cfg->type = PINMUX_TYPE_NONE;
	spin_unlock_irqrestore(&pfc->lock, flags);
}

static int sh_pfc_gpio_set_direction(struct pinctrl_dev *pctldev,
				     struct pinctrl_gpio_range *range,
				     unsigned offset, bool input)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	struct sh_pfc *pfc = pmx->pfc;
	int new_type = input ? PINMUX_TYPE_INPUT : PINMUX_TYPE_OUTPUT;
	int idx = sh_pfc_get_pin_index(pfc, offset);
	const struct sh_pfc_pin *pin = &pfc->info->pins[idx];
	struct sh_pfc_pin_config *cfg = &pmx->configs[idx];
	unsigned long flags;
	unsigned int dir;
	int ret;

	/* Check if the requested direction is supported by the pin. Not all SoC
	 * provide pin config data, so perform the check conditionally.
	 */
	if (pin->configs) {
		dir = input ? SH_PFC_PIN_CFG_INPUT : SH_PFC_PIN_CFG_OUTPUT;
		if (!(pin->configs & dir))
			return -EINVAL;
	}

	spin_lock_irqsave(&pfc->lock, flags);

	ret = sh_pfc_config_mux(pfc, pin->enum_id, new_type);
	if (ret < 0)
		goto done;

	cfg->type = new_type;

done:
	spin_unlock_irqrestore(&pfc->lock, flags);
	return ret;
}

static const struct pinmux_ops sh_pfc_pinmux_ops = {
	.get_functions_count	= sh_pfc_get_functions_count,
	.get_function_name	= sh_pfc_get_function_name,
	.get_function_groups	= sh_pfc_get_function_groups,
	.enable			= sh_pfc_func_enable,
	.disable		= sh_pfc_func_disable,
	.gpio_request_enable	= sh_pfc_gpio_request_enable,
	.gpio_disable_free	= sh_pfc_gpio_disable_free,
	.gpio_set_direction	= sh_pfc_gpio_set_direction,
};

/* Check whether the requested parameter is supported for a pin. */
static bool sh_pfc_pinconf_validate(struct sh_pfc *pfc, unsigned int _pin,
				    enum pin_config_param param)
{
	int idx = sh_pfc_get_pin_index(pfc, _pin);
	const struct sh_pfc_pin *pin = &pfc->info->pins[idx];

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		return pin->configs &
			(SH_PFC_PIN_CFG_PULL_UP | SH_PFC_PIN_CFG_PULL_DOWN);

	case PIN_CONFIG_BIAS_PULL_UP:
		return pin->configs & SH_PFC_PIN_CFG_PULL_UP;

	case PIN_CONFIG_BIAS_PULL_DOWN:
		return pin->configs & SH_PFC_PIN_CFG_PULL_DOWN;

	default:
		return false;
	}
}

static int sh_pfc_pinconf_get(struct pinctrl_dev *pctldev, unsigned _pin,
			      unsigned long *config)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	struct sh_pfc *pfc = pmx->pfc;
	enum pin_config_param param = pinconf_to_config_param(*config);
	unsigned long flags;
	unsigned int bias;

	if (!sh_pfc_pinconf_validate(pfc, _pin, param))
		return -ENOTSUPP;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_PULL_DOWN:
		if (!pfc->info->ops || !pfc->info->ops->get_bias)
			return -ENOTSUPP;

		spin_lock_irqsave(&pfc->lock, flags);
		bias = pfc->info->ops->get_bias(pfc, _pin);
		spin_unlock_irqrestore(&pfc->lock, flags);

		if (bias != param)
			return -EINVAL;

		*config = 0;
		break;

	default:
		return -ENOTSUPP;
	}

	return 0;
}

static int sh_pfc_pinconf_set(struct pinctrl_dev *pctldev, unsigned _pin,
			      unsigned long config)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	struct sh_pfc *pfc = pmx->pfc;
	enum pin_config_param param = pinconf_to_config_param(config);
	unsigned long flags;

	if (!sh_pfc_pinconf_validate(pfc, _pin, param))
		return -ENOTSUPP;

	switch (param) {
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_DISABLE:
		if (!pfc->info->ops || !pfc->info->ops->set_bias)
			return -ENOTSUPP;

		spin_lock_irqsave(&pfc->lock, flags);
		pfc->info->ops->set_bias(pfc, _pin, param);
		spin_unlock_irqrestore(&pfc->lock, flags);

		break;

	default:
		return -ENOTSUPP;
	}

	return 0;
}

static int sh_pfc_pinconf_group_set(struct pinctrl_dev *pctldev, unsigned group,
				    unsigned long config)
{
	struct sh_pfc_pinctrl *pmx = pinctrl_dev_get_drvdata(pctldev);
	const unsigned int *pins;
	unsigned int num_pins;
	unsigned int i;

	pins = pmx->pfc->info->groups[group].pins;
	num_pins = pmx->pfc->info->groups[group].nr_pins;

	for (i = 0; i < num_pins; ++i)
		sh_pfc_pinconf_set(pctldev, pins[i], config);

	return 0;
}

static const struct pinconf_ops sh_pfc_pinconf_ops = {
	.is_generic			= true,
	.pin_config_get			= sh_pfc_pinconf_get,
	.pin_config_set			= sh_pfc_pinconf_set,
	.pin_config_group_set		= sh_pfc_pinconf_group_set,
	.pin_config_config_dbg_show	= pinconf_generic_dump_config,
};

/* PFC ranges -> pinctrl pin descs */
static int sh_pfc_map_pins(struct sh_pfc *pfc, struct sh_pfc_pinctrl *pmx)
{
	const struct pinmux_range *ranges;
	struct pinmux_range def_range;
	unsigned int nr_ranges;
	unsigned int nr_pins;
	unsigned int i;

	if (pfc->info->ranges == NULL) {
		def_range.begin = 0;
		def_range.end = pfc->info->nr_pins - 1;
		ranges = &def_range;
		nr_ranges = 1;
	} else {
		ranges = pfc->info->ranges;
		nr_ranges = pfc->info->nr_ranges;
	}

	pmx->pins = devm_kzalloc(pfc->dev,
				 sizeof(*pmx->pins) * pfc->info->nr_pins,
				 GFP_KERNEL);
	if (unlikely(!pmx->pins))
		return -ENOMEM;

	pmx->configs = devm_kzalloc(pfc->dev,
				    sizeof(*pmx->configs) * pfc->info->nr_pins,
				    GFP_KERNEL);
	if (unlikely(!pmx->configs))
		return -ENOMEM;

	for (i = 0, nr_pins = 0; i < nr_ranges; ++i) {
		const struct pinmux_range *range = &ranges[i];
		unsigned int number;

		for (number = range->begin; number <= range->end;
		     number++, nr_pins++) {
			struct sh_pfc_pin_config *cfg = &pmx->configs[nr_pins];
			struct pinctrl_pin_desc *pin = &pmx->pins[nr_pins];
			const struct sh_pfc_pin *info =
				&pfc->info->pins[nr_pins];

			pin->number = number;
			pin->name = info->name;
			cfg->type = PINMUX_TYPE_NONE;
		}
	}

	pfc->nr_pins = ranges[nr_ranges-1].end + 1;

	return nr_ranges;
}

int sh_pfc_register_pinctrl(struct sh_pfc *pfc)
{
	struct sh_pfc_pinctrl *pmx;
	int nr_ranges;

	pmx = devm_kzalloc(pfc->dev, sizeof(*pmx), GFP_KERNEL);
	if (unlikely(!pmx))
		return -ENOMEM;

	pmx->pfc = pfc;
	pfc->pinctrl = pmx;

	nr_ranges = sh_pfc_map_pins(pfc, pmx);
	if (unlikely(nr_ranges < 0))
		return nr_ranges;

	pmx->pctl_desc.name = DRV_NAME;
	pmx->pctl_desc.owner = THIS_MODULE;
	pmx->pctl_desc.pctlops = &sh_pfc_pinctrl_ops;
	pmx->pctl_desc.pmxops = &sh_pfc_pinmux_ops;
	pmx->pctl_desc.confops = &sh_pfc_pinconf_ops;
	pmx->pctl_desc.pins = pmx->pins;
	pmx->pctl_desc.npins = pfc->info->nr_pins;

	pmx->pctl = pinctrl_register(&pmx->pctl_desc, pfc->dev, pmx);
	if (pmx->pctl == NULL)
		return -EINVAL;

	return 0;
}

int sh_pfc_unregister_pinctrl(struct sh_pfc *pfc)
{
	struct sh_pfc_pinctrl *pmx = pfc->pinctrl;

	pinctrl_unregister(pmx->pctl);

	pfc->pinctrl = NULL;
	return 0;
}
