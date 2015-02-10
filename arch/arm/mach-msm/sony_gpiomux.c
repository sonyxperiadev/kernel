/* arch/arm/mach-msm/sony_gpiomux.c
 *
 * Copyright (C) 2014 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/of.h>
#include <linux/slab.h>
#include <mach/gpiomux.h>
#include "sony_gpiomux.h"

static struct gpiomux_setting __initdata **merge_set;
static struct gpiomux_setting __initdata **qct_sets;
static struct msm_gpiomux_config __initdata *base_configs;

void __init overwrite_configs(struct msm_gpiomux_configs *base,
		struct msm_gpiomux_configs *specific)
{
	unsigned i, j;
	struct msm_gpiomux_config *b_cfg = base->cfg;
	struct msm_gpiomux_config *s_cfg = specific->cfg;

	for (i = 0; i < specific->ncfg; i++) {
		for (j = 0; j < base->ncfg; j++) {
			if (s_cfg[i].gpio == b_cfg[j].gpio) {
				b_cfg[j].settings[GPIOMUX_ACTIVE] =
					s_cfg[i].settings[GPIOMUX_ACTIVE];
				b_cfg[j].settings[GPIOMUX_SUSPENDED] =
					s_cfg[i].settings[GPIOMUX_SUSPENDED];
			}
		}
	}
}

static void __init gpiomux_merge_setting(unsigned idx, unsigned which)
{
	struct gpiomux_setting *pset = base_configs[idx].settings[which];
	unsigned set_slot = base_configs[idx].gpio
				* GPIOMUX_NSETTINGS + which;

	if (!pset)
		return;

	if ((pset->func == GPIOMUX_FOLLOW_QCT) &&
		(pset->drv == GPIOMUX_FOLLOW_QCT) &&
		(pset->pull == GPIOMUX_FOLLOW_QCT) &&
		(pset->dir == GPIOMUX_FOLLOW_QCT)) {
		base_configs[idx].settings[which] = qct_sets[set_slot];
	} else if ((pset->func == GPIOMUX_FOLLOW_QCT) ||
		(pset->drv == GPIOMUX_FOLLOW_QCT) ||
		(pset->pull == GPIOMUX_FOLLOW_QCT) ||
		(pset->dir == GPIOMUX_FOLLOW_QCT)) {
		if (!qct_sets[set_slot]) {
			base_configs[idx].settings[which] = NULL;
			return;
		}

		merge_set[set_slot] = kzalloc(sizeof(struct gpiomux_setting),
					   GFP_KERNEL);
		if (!merge_set[set_slot]) {
			pr_err("%s: GPIO_%d merge failure\n", __func__,
					base_configs[idx].gpio);
			return;
		}

		*merge_set[set_slot] = *pset;

		if (pset->func == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->func = qct_sets[set_slot]->func;
		if (pset->drv == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->drv = qct_sets[set_slot]->drv;
		if (pset->pull == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->pull = qct_sets[set_slot]->pull;
		if (pset->dir == GPIOMUX_FOLLOW_QCT)
			merge_set[set_slot]->dir = qct_sets[set_slot]->dir;

		base_configs[idx].settings[which] = merge_set[set_slot];
	}
}

int __init sony_init_gpiomux(struct msm_gpiomux_config *configs,
		unsigned nconfigs)
{
	int rc;
	unsigned int ngpio, ngpio_settings, i, c, s;
	struct device_node *of_gpio_node;

	rc = msm_gpiomux_init_dt();
	if (rc) {
		pr_err("%s failed %d\n", __func__, rc);
		return rc;
	}

#if defined(CONFIG_SONY_CAM_V4L2) && defined(CONFIG_MACH_SONY_RHINE)
	msm_tlmm_misc_reg_write(TLMM_SPARE_REG, 0x5);
#endif

	of_gpio_node = of_find_compatible_node(NULL, NULL, "qcom,msm-gpio");
	if (!of_gpio_node) {
		pr_err("%s: Failed to find qcom,msm-gpio node\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(of_gpio_node, "ngpio", &ngpio);
	if (rc) {
		pr_err("%s: Failed to find ngpio property in msm-gpio device " \
			"node %d\n", __func__, rc);
		return rc;
	}

	ngpio_settings = ngpio * GPIOMUX_NSETTINGS;

	merge_set = kzalloc(sizeof(struct gpiomux_setting *) * ngpio_settings,
				GFP_KERNEL);
	if (!merge_set) {
		pr_err("%s: kzalloc failed for merge config\n", __func__);
		return -ENOMEM;
	}

	qct_sets = kzalloc(sizeof(struct gpiomux_setting *) * ngpio_settings,
				GFP_KERNEL);
	if (!qct_sets) {
		kfree(merge_set);
		pr_err("%s: kzalloc failed for reference config\n", __func__);
		return -ENOMEM;
	}

	base_configs = configs;

	/* Set reference configuration */
	gpiomux_arrange_all_qct_configs(qct_sets);

	for (c = 0; c < nconfigs; ++c) {
		for (s = 0; s < GPIOMUX_NSETTINGS; ++s)
			gpiomux_merge_setting(c, s);
	}

	/* Install product-all merged configuration */
	msm_gpiomux_install(configs, nconfigs);

	for (i = 0; i < ngpio_settings; ++i)
		kfree(merge_set[i]);

	kfree(merge_set);
	kfree(qct_sets);

	return 0;
}
