/*
 * Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
 *
 * Original file: clock-cpu-8939.c
 * Adaptation for 8936 (C) 2016, AngeloGioacchino Del Regno <kholk11@gmail.com>
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

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/pm_qos.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/clk/msm-clock-generic.h>
#include <linux/suspend.h>
#include <linux/of_platform.h>
#include <linux/pm_opp.h>
#include <soc/qcom/clock-local2.h>

#include "clock.h"
#include <dt-bindings/clock/msm-cpu-clocks-8939.h>

static DEFINE_VDD_REGS_INIT(vdd_cpu_bc, 1);
static DEFINE_VDD_REGS_INIT(vdd_cpu_lc, 1);
static DEFINE_VDD_REGS_INIT(vdd_cpu_cci, 1);

enum {
	A53SS_MUX_BC,
	A53SS_MUX_CCI,
	A53SS_MUX_LC,
	A53SS_MUX_NUM,
};

static const char *mux_names[] = { "c1", "cci", "c0"};

struct cpu_clk_8936 {
	u32 cpu_reg_mask;
	cpumask_t cpumask;
	bool hw_low_power_ctrl;
	struct pm_qos_request req;
	struct clk c;
};

static struct mux_div_clk a53ssmux_bc = {
	.ops = &rcg_mux_div_ops,
	.safe_freq = 400000000,
	.data = {
		.max_div = 32,
		.min_div = 2,
		.is_half_divider = true,
	},
	.c = {
		.dbg_name = "a53ssmux_bc",
		.ops = &clk_ops_mux_div_clk,
		CLK_INIT(a53ssmux_bc.c),
	},
	.parents = (struct clk_src[8]) {},
	.div_mask = BM(4, 0),
	.src_mask = BM(10, 8) >> 8,
	.src_shift = 8,
};

static struct mux_div_clk a53ssmux_lc = {
	.ops = &rcg_mux_div_ops,
	.safe_freq = 200000000,
	.data = {
		.max_div = 32,
		.min_div = 2,
		.is_half_divider = true,
	},
	.c = {
		.dbg_name = "a53ssmux_lc",
		.ops = &clk_ops_mux_div_clk,
		CLK_INIT(a53ssmux_lc.c),
	},
	.parents = (struct clk_src[8]) {},
	.div_mask = BM(4, 0),
	.src_mask = BM(10, 8) >> 8,
	.src_shift = 8,
};

static struct mux_div_clk a53ssmux_cci = {
	.ops = &rcg_mux_div_ops,
	.safe_freq = 200000000,
	.data = {
		.max_div = 32,
		.min_div = 2,
		.is_half_divider = true,
	},
	.c = {
		.dbg_name = "a53ssmux_cci",
		.ops = &clk_ops_mux_div_clk,
		CLK_INIT(a53ssmux_cci.c),
	},
	.parents = (struct clk_src[8]) {},
	.div_mask = BM(4, 0),
	.src_mask = BM(10, 8) >> 8,
	.src_shift = 8,
};

static void do_nothing(void *unused) { }
#define CPU_LATENCY_NO_L2_PC_US (300)

static inline struct cpu_clk_8936 *to_cpu_clk_8936(struct clk *c)
{
	return container_of(c, struct cpu_clk_8936, c);
}

static enum handoff cpu_clk_8936_handoff(struct clk *c)
{
	c->rate = clk_get_rate(c->parent);
	return HANDOFF_DISABLED_CLK;
}

static long cpu_clk_8936_round_rate(struct clk *c, unsigned long rate)
{
	return clk_round_rate(c->parent, rate);
}

static int cpu_clk_8936_set_rate(struct clk *c, unsigned long rate)
{
	int ret = 0;
	struct cpu_clk_8936 *cpuclk = to_cpu_clk_8936(c);
	bool hw_low_power_ctrl = cpuclk->hw_low_power_ctrl;

	if (hw_low_power_ctrl) {
		memset(&cpuclk->req, 0, sizeof(cpuclk->req));
		cpumask_copy(&cpuclk->req.cpus_affine,
				(const struct cpumask *)&cpuclk->cpumask);
		cpuclk->req.type = PM_QOS_REQ_AFFINE_CORES;
		pm_qos_add_request(&cpuclk->req, PM_QOS_CPU_DMA_LATENCY,
				CPU_LATENCY_NO_L2_PC_US);
		smp_call_function_any(&cpuclk->cpumask, do_nothing,
				NULL, 1);
	}

	ret = clk_set_rate(c->parent, rate);

	if (hw_low_power_ctrl)
		pm_qos_remove_request(&cpuclk->req);

	return ret;
}

static struct clk_ops clk_ops_cpu = {
	.set_rate = cpu_clk_8936_set_rate,
	.round_rate = cpu_clk_8936_round_rate,
	.handoff = cpu_clk_8936_handoff,
};

static struct cpu_clk_8936 a53_bc_clk = {
	.cpu_reg_mask = 0x3,
	.c = {
		.parent = &a53ssmux_bc.c,
		.ops = &clk_ops_cpu,
		.vdd_class = &vdd_cpu_bc,
		.dbg_name = "a53_bc_clk",
		CLK_INIT(a53_bc_clk.c),
	},
};

static struct cpu_clk_8936 a53_lc_clk = {
	.cpu_reg_mask = 0x103,
	.c = {
		.parent = &a53ssmux_lc.c,
		.ops = &clk_ops_cpu,
		.vdd_class = &vdd_cpu_lc,
		.dbg_name = "a53_lc_clk",
		CLK_INIT(a53_lc_clk.c),
	},
};

static struct cpu_clk_8936 cci_clk = {
	.c = {
		.parent = &a53ssmux_cci.c,
		.ops = &clk_ops_cpu,
		.vdd_class = &vdd_cpu_cci,
		.dbg_name = "cci_clk",
		CLK_INIT(cci_clk.c),
	},
};

static struct clk_lookup cpu_clocks_8936[] = {
	CLK_LIST(a53ssmux_bc),
	CLK_LIST(a53ssmux_cci),
	CLK_LIST(a53_bc_clk),
	CLK_LIST(cci_clk),
};


static struct mux_div_clk *a53ssmux[] = {&a53ssmux_bc,
						&a53ssmux_cci, &a53ssmux_lc};

static struct cpu_clk_8936 *cpuclk[] = { &a53_bc_clk, &cci_clk, &a53_lc_clk};

static struct clk *logical_cpu_to_clk(int cpu)
{
	struct device_node *cpu_node = of_get_cpu_node(cpu, NULL);
	u32 reg;

	/* CPU 0/1/2/3 --> a53_bc_clk and mask = 0x103
	 * CPU 4/5/6/7 --> a53_lc_clk and mask = 0x3
	 */
	if (cpu_node && !of_property_read_u32(cpu_node, "reg", &reg)) {
		if ((reg | a53_bc_clk.cpu_reg_mask) == a53_bc_clk.cpu_reg_mask)
			return &a53_lc_clk.c;
		if ((reg | a53_lc_clk.cpu_reg_mask) == a53_lc_clk.cpu_reg_mask)
			return &a53_bc_clk.c;
	}

	return NULL;
}

static int of_get_fmax_vdd_class(struct platform_device *pdev, struct clk *c,
								char *prop_name)
{
	struct device_node *of = pdev->dev.of_node;
	int prop_len, i;
	struct clk_vdd_class *vdd = c->vdd_class;
	u32 *array;

	if (!of_find_property(of, prop_name, &prop_len)) {
		dev_err(&pdev->dev, "missing %s\n", prop_name);
		return -EINVAL;
	}

	prop_len /= sizeof(u32);
	if (prop_len % 2) {
		dev_err(&pdev->dev, "bad length %d\n", prop_len);
		return -EINVAL;
	}

	prop_len /= 2;
	vdd->level_votes = devm_kzalloc(&pdev->dev,
				prop_len * sizeof(*vdd->level_votes),
					GFP_KERNEL);
	if (!vdd->level_votes)
		return -ENOMEM;

	vdd->vdd_uv = devm_kzalloc(&pdev->dev, prop_len * sizeof(int),
					GFP_KERNEL);
	if (!vdd->vdd_uv)
		return -ENOMEM;

	c->fmax = devm_kzalloc(&pdev->dev, prop_len * sizeof(unsigned long),
					GFP_KERNEL);
	if (!c->fmax)
		return -ENOMEM;

	array = devm_kzalloc(&pdev->dev,
			prop_len * sizeof(u32) * 2, GFP_KERNEL);
	if (!array)
		return -ENOMEM;

	of_property_read_u32_array(of, prop_name, array, prop_len * 2);
	for (i = 0; i < prop_len; i++) {
		c->fmax[i] = array[2 * i];
		vdd->vdd_uv[i] = array[2 * i + 1];
	}

	devm_kfree(&pdev->dev, array);
	vdd->num_levels = prop_len;
	vdd->cur_level = prop_len;
	vdd->use_max_uV = true;
	c->num_fmax = prop_len;
	return 0;
}

static void get_speed_bin(struct platform_device *pdev, int *bin)
{
	struct resource *res;
	void __iomem *base;
	u32 pte_efuse;

	*bin = 0;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "efuse");
	if (!res) {
		dev_info(&pdev->dev,
			 "No speed/PVS binning available. Defaulting to 0!\n");
		return;
	}

	base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!base) {
		dev_warn(&pdev->dev,
			 "Unable to read efuse data. Defaulting to 0!\n");
		return;
	}

	pte_efuse = readl_relaxed(base);
	devm_iounmap(&pdev->dev, base);

	*bin = (pte_efuse >> 2) & 0x7;

	dev_info(&pdev->dev, "Speed bin: %d PVS Version: 0\n", *bin);
}

static int of_get_clk_src(struct platform_device *pdev,
				struct clk_src *parents, int mux_id)
{
	struct device_node *of = pdev->dev.of_node;
	int mux_parents, i, j, index;
	struct clk *c;
	char clk_name[] = "clk-xxx-x";

	mux_parents = of_property_count_strings(of, "clock-names");
	if (mux_parents <= 0) {
		dev_err(&pdev->dev, "missing clock-names\n");
		return -EINVAL;
	}
	j = 0;

	for (i = 0; i < 8; i++) {
		snprintf(clk_name, ARRAY_SIZE(clk_name), "clk-%s-%d",
							mux_names[mux_id], i);
		index = of_property_match_string(of, "clock-names", clk_name);
		if (IS_ERR_VALUE(index))
			continue;

		parents[j].sel = i;
		parents[j].src = c = devm_clk_get(&pdev->dev, clk_name);
		if (IS_ERR(c)) {
			if (c != ERR_PTR(-EPROBE_DEFER))
				dev_err(&pdev->dev, "clk_get: %s\n fail",
						clk_name);
			return PTR_ERR(c);
		}
		j++;
	}

	return j;
}

static int cpu_parse_devicetree(struct platform_device *pdev, int mux_id)
{
	struct resource *res;
	int rc;
	char rcg_name[] = "apcs-xxx-rcg-base";
	char vdd_name[] = "vdd-xxx";
	struct regulator *regulator;

	snprintf(rcg_name, ARRAY_SIZE(rcg_name), "apcs-%s-rcg-base",
						mux_names[mux_id]);
	res = platform_get_resource_byname(pdev,
					IORESOURCE_MEM, rcg_name);
	if (!res) {
		dev_err(&pdev->dev, "missing %s\n", rcg_name);
		return -EINVAL;
	}
	a53ssmux[mux_id]->base = devm_ioremap(&pdev->dev, res->start,
							resource_size(res));
	if (!a53ssmux[mux_id]->base) {
		dev_err(&pdev->dev, "ioremap failed for %s\n", rcg_name);
		return -ENOMEM;
	}

	snprintf(vdd_name, ARRAY_SIZE(vdd_name), "vdd-%s", mux_names[mux_id]);
	regulator = devm_regulator_get(&pdev->dev, vdd_name);
	if (IS_ERR(regulator)) {
		if (PTR_ERR(regulator) != -EPROBE_DEFER)
			dev_err(&pdev->dev, "unable to get regulator\n");
		return PTR_ERR(regulator);
	}
	cpuclk[mux_id]->c.vdd_class->regulator[0] = regulator;

	rc = of_get_clk_src(pdev, a53ssmux[mux_id]->parents, mux_id);
	if (IS_ERR_VALUE(rc))
		return rc;

	a53ssmux[mux_id]->num_parents = rc;

	return 0;
}

static long corner_to_voltage(unsigned long corner, struct device *dev)
{
	struct dev_pm_opp *oppl;
	long uv;

	rcu_read_lock();
	oppl = dev_pm_opp_find_freq_exact(dev, corner, true);
	rcu_read_unlock();
	if (IS_ERR_OR_NULL(oppl))
		return -EINVAL;

	rcu_read_lock();
	uv = dev_pm_opp_get_voltage(oppl);
	rcu_read_unlock();

	return uv;
}


static int add_opp(struct clk *c, struct device *cpudev, struct device *vregdev,
			unsigned long max_rate)
{
	unsigned long rate = 0;
	int level;
	long ret, uv, corner;
	bool use_voltages = false;
	struct dev_pm_opp *oppl;
	int j = 1;

	rcu_read_lock();
	/* Check if the regulator driver has already populated OPP tables */
	oppl = dev_pm_opp_find_freq_exact(vregdev, 2, true);
	rcu_read_unlock();
	if (!IS_ERR_OR_NULL(oppl))
		use_voltages = true;

	while (1) {
		rate = c->fmax[j++];
		level = find_vdd_level(c, rate);
		if (level <= 0) {
			pr_warn("clock-cpu: no uv for %lu.\n", rate);
			return -EINVAL;
		}
		uv = corner = c->vdd_class->vdd_uv[level];
		/*
		 * If corner to voltage mapping is available, populate the OPP
		 * table with the voltages rather than corners.
		 */
		if (use_voltages) {
			uv = corner_to_voltage(corner, vregdev);
			if (uv < 0) {
				pr_warn("clock-cpu: no uv for corner %lu\n",
					 corner);
				return uv;
			}
			ret = dev_pm_opp_add(cpudev, rate, uv);
			if (ret) {
				pr_warn("clock-cpu: couldn't add OPP for %lu\n",
					 rate);
				return ret;
			}

		} else {
			/*
			 * Populate both CPU and regulator devices with the
			 * freq-to-corner OPP table to maintain backward
			 * compatibility.
			 */
			ret = dev_pm_opp_add(cpudev, rate, corner);
			if (ret) {
				pr_warn("clock-cpu: couldn't add OPP for %lu\n",
					rate);
				return ret;
			}
			ret = dev_pm_opp_add(vregdev, rate, corner);
			if (ret) {
				pr_warn("clock-cpu: couldn't add OPP for %lu\n",
					rate);
				return ret;
			}
		}
		if (rate >= max_rate)
			break;
	}

	return 0;
}

static void print_opp_table(int a53_c1_cpu)
{
	struct dev_pm_opp *oppfmax, *oppfmin;
	unsigned long apc1_fmax, apc1_fmin;

	apc1_fmax = a53_bc_clk.c.fmax[a53_bc_clk.c.num_fmax - 1];
	apc1_fmin = a53_bc_clk.c.fmax[1];

	rcu_read_lock();
	oppfmax = dev_pm_opp_find_freq_exact(get_cpu_device(a53_c1_cpu),
						apc1_fmax, true);
	oppfmin = dev_pm_opp_find_freq_exact(get_cpu_device(a53_c1_cpu),
						apc1_fmin, true);
	pr_info("clock_cpu: a53_c1: OPP voltage for %lu: %lu\n", apc1_fmin,
		dev_pm_opp_get_voltage(oppfmin));
	pr_info("clock_cpu: a53_c1: OPP voltage for %lu: %lu\n", apc1_fmax,
		dev_pm_opp_get_voltage(oppfmax));
	rcu_read_unlock();
}

static void populate_opp_table(struct platform_device *pdev)
{
	struct platform_device *apc1_dev;
	struct device_node *apc1_node;
	unsigned long apc1_fmax;
	int cpu, a53_c1_cpu;

	apc1_node = of_parse_phandle(pdev->dev.of_node, "vdd-c1-supply", 0);

	if (!apc1_node) {
		pr_err("can't find the apc1 dt node.\n");
		return;
	}

	apc1_dev = of_find_device_by_node(apc1_node);
	if (!apc1_dev) {
		pr_err("can't find the apc1 device node.\n");
		return;
	}

	apc1_fmax = a53_bc_clk.c.fmax[a53_bc_clk.c.num_fmax - 1];

	for_each_possible_cpu(cpu) {
		pr_debug("the CPU number is : %d\n", cpu);
		if (cpu/4 == 0) {
			a53_c1_cpu = cpu;
			WARN(add_opp(&a53_bc_clk.c, get_cpu_device(cpu),
				     &apc1_dev->dev, apc1_fmax),
				     "Failed to add OPP levels for A53 big cluster\n");
		}
	}

	/* One time print during bootup */
	pr_info("clock-cpu-8939: OPP tables populated for cpu 0-%d",
		a53_c1_cpu);

	print_opp_table(a53_c1_cpu);

}

static void config_pll(int mux_id)
{
	unsigned long rate, aux_rate;
	struct clk *aux_clk, *main_pll;

	aux_clk = a53ssmux[mux_id]->parents[0].src;
	main_pll = a53ssmux[mux_id]->parents[1].src;

	aux_rate = clk_get_rate(aux_clk);
	rate = clk_get_rate(&a53ssmux[mux_id]->c);
	clk_set_rate(&a53ssmux[mux_id]->c, aux_rate);
	clk_set_rate(main_pll, clk_round_rate(main_pll, 1));
	clk_set_rate(&a53ssmux[mux_id]->c, rate);

	return;
}

static int clock_8936_pm_event(struct notifier_block *notif,
				unsigned long event, void *ptr)
{
	switch (event) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		clk_unprepare(&a53_bc_clk.c);
		clk_unprepare(&cci_clk.c);
		break;
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		clk_prepare(&a53_bc_clk.c);
		clk_prepare(&cci_clk.c);
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
};

static struct notifier_block clock_8936_pm_notifier = {
	.notifier_call = clock_8936_pm_event,
};

static int clock_a53_probe(struct platform_device *pdev)
{
	int speed_bin, rc, cpu, mux_id;
	char prop_name[] = "qcom,speedX-bin-vX-XXX";
	int mux_num;

	get_speed_bin(pdev, &speed_bin);

	mux_num = A53SS_MUX_LC;

	for (mux_id = 0; mux_id < mux_num; mux_id++) {
		rc = cpu_parse_devicetree(pdev, mux_id);
		if (rc)
			return rc;

		snprintf(prop_name, ARRAY_SIZE(prop_name),
					"qcom,speed%d-bin-v0-%s",
					speed_bin, mux_names[mux_id]);

		rc = of_get_fmax_vdd_class(pdev, &cpuclk[mux_id]->c,
								prop_name);
		if (rc) {
			/* Fall back to most conservative PVS table */
			dev_err(&pdev->dev, "Unable to load voltage plan %s!\n",
								prop_name);

			snprintf(prop_name, ARRAY_SIZE(prop_name),
				"qcom,speed0-bin-v0-%s", mux_names[mux_id]);
			rc = of_get_fmax_vdd_class(pdev, &cpuclk[mux_id]->c,
								prop_name);
			if (rc) {
				dev_err(&pdev->dev,
					"Unable to load safe voltage plan\n");
				return rc;
			}
			dev_info(&pdev->dev, "Safe voltage plan loaded.\n");
		}
	}

	rc = of_msm_clock_register(pdev->dev.of_node,
				cpu_clocks_8936,
				ARRAY_SIZE(cpu_clocks_8936));
	if (rc) {
		dev_err(&pdev->dev, "msm_clock_register failed\n");
		return rc;
	}

	for (mux_id = 0; mux_id < A53SS_MUX_CCI; mux_id++) {
		/* Force a PLL reconfiguration */
		config_pll(mux_id);
	}

	/*
	 * We don't want the CPU clocks to be turned off at late init
	 * if CPUFREQ or HOTPLUG configs are disabled. So, bump up the
	 * refcount of these clocks. Any cpufreq/hotplug manager can assume
	 * that the clocks have already been prepared and enabled by the time
	 * they take over.
	 */
	get_online_cpus();
	for_each_online_cpu(cpu) {
		WARN(clk_prepare_enable(&cpuclk[cpu/4]->c),
				"Unable to turn on CPU clock");
		clk_prepare_enable(&cci_clk.c);
	}
	put_online_cpus();

	for_each_possible_cpu(cpu) {
		if (logical_cpu_to_clk(cpu) == &a53_bc_clk.c)
			cpumask_set_cpu(cpu, &a53_bc_clk.cpumask);
		if (logical_cpu_to_clk(cpu) == &a53_lc_clk.c)
			cpumask_set_cpu(cpu, &a53_lc_clk.cpumask);
	}

	a53_lc_clk.hw_low_power_ctrl = false;
	a53_bc_clk.hw_low_power_ctrl = false;

	register_pm_notifier(&clock_8936_pm_notifier);

	populate_opp_table(pdev);

	return 0;
}

static struct of_device_id clock_a53_match_table[] = {
	{.compatible = "qcom,cpu-clock-8936"},
	{}
};

static struct platform_driver clock_a53_driver = {
	.probe = clock_a53_probe,
	.driver = {
		.name = "cpu-clock-8936",
		.of_match_table = clock_a53_match_table,
		.owner = THIS_MODULE,
	},
};

static int __init clock_a53_init(void)
{
	return platform_driver_register(&clock_a53_driver);
}
arch_initcall(clock_a53_init);

