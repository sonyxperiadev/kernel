/*
 * Copyright (c) 2015 NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/cpufreq.h>
#include <linux/cpuquiet.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/regulator/consumer.h>
#include <linux/types.h>
#include <linux/clk/tegra.h>

#define TEGRA210_CPU_BACKUP_RATE	204000000U

static DEFINE_MUTEX(tegra_cpu_lock);

struct tegra210_cpufreq_priv {
	struct regulator *vdd_cpu_reg;
	struct clk *cpu_g_clk;
	struct clk *cpu_lp_clk;
	struct clk *pllp_clk;
	struct clk *pllx_clk;
	struct clk *dfll_clk;
	struct clk *cpu_clk;
	struct clk *emc_clk;
	struct device *cpu_dev;
	int suspend_index;
	bool dfll_mode;
};

static struct tegra210_cpufreq_priv priv;

static int tegra210_cpu_switch_to_dfll(void)
{
	int ret;

	ret = clk_set_rate(priv.dfll_clk, clk_get_rate(priv.cpu_clk));
	if (ret)
		return ret;

	clk_set_parent(priv.cpu_clk, priv.dfll_clk);

	priv.dfll_mode = true;

	return 0;
}

static void tegra210_cpu_switch_to_pllx(void)
{
	clk_set_parent(priv.cpu_clk, priv.pllp_clk);
	regulator_sync_voltage(priv.vdd_cpu_reg);
	clk_set_parent(priv.cpu_clk, priv.pllx_clk);
	priv.dfll_mode = false;
}

static int tegra210_set_rate_pll(unsigned int new_rate, unsigned int old_rate)
{
	int ret = 0;
	bool dramp = false;
	unsigned long backup_rate, main_rate;

	if (clk_get_parent(priv.cpu_clk) == priv.pllx_clk) {
		main_rate = new_rate;

		dramp = (new_rate > TEGRA210_CPU_BACKUP_RATE) &&
			tegra_pll_can_ramp_to_rate(priv.pllx_clk, new_rate);

		if (dramp ||
			tegra_pll_can_ramp_to_min(priv.pllx_clk, &main_rate)) {
			ret = clk_set_rate(priv.pllx_clk, main_rate);
			if (ret) {
				pr_err("%s: Failed to set rate %lu on %s\n",
					__func__, main_rate,
					__clk_get_name(priv.pllx_clk));
				goto out;
			}

			if (main_rate == new_rate)
				goto out;
		} else {
			pr_err("%s: not ready for dynamic ramp to %u\n",
				__clk_get_name(priv.pllx_clk), new_rate);
		}

		/*
		 * Switch to back-up source, and stay on it if target rate is
		 * below backup rate
		 */
		ret = clk_set_parent(priv.cpu_clk, priv.pllp_clk);
		if (ret) {
			pr_err("%s: Failed to switch cpu to %s\n",
			       __func__, __clk_get_name(priv.pllp_clk));
			goto out;
		}
	}

	backup_rate = min(new_rate, TEGRA210_CPU_BACKUP_RATE);
	if (backup_rate != clk_get_rate(priv.cpu_clk)) {
		ret = clk_set_rate(priv.pllp_clk, backup_rate);
		if (ret) {
			pr_err("%s: Failed to set rate %lu on backup source\n",
			       __func__, backup_rate);
			goto out;
		}
	}
	if (new_rate == backup_rate)
		goto out;

	/*
	 * Switch from backup source to main at rate not exceeding pll VCO
	 * minimum. Use dynamic ramp to reach target rate if it is above VCO
	 * minimum.
	 */
	main_rate = new_rate;
	if (tegra_pll_get_min_ramp_rate(priv.pllx_clk, new_rate, &main_rate)) {
		pr_err("%s: not ready for dynamic ramp to %u\n",
			__clk_get_name(priv.pllx_clk), new_rate);
	}

	ret = clk_set_rate(priv.pllx_clk, main_rate);
	if (ret) {
		pr_err("%s: Failed to set cpu rate %lu on source %s\n",
			__func__, main_rate, __clk_get_name(priv.pllx_clk));
		goto out;
	}

	ret = clk_set_parent(priv.cpu_clk, priv.pllx_clk);
	if (ret) {
		pr_err("%s: Failed to switch cpu to %s\n",
			__func__, __clk_get_name(priv.pllx_clk));
		goto out;
	}

	if (new_rate != main_rate) {
		ret = clk_set_rate(priv.pllx_clk, new_rate);
		if (ret) {
			pr_err("%s: Failed to set cpu rate %u on source%s\n",
				__func__, new_rate,
				__clk_get_name(priv.pllx_clk));
			goto out;
		}
	}

	return 0;
out:
	return ret;
}

static void tegra210_vote_emc_rate(unsigned long cpu_rate)
{
	/* Vote on memory bus frequency based on cpu frequency */
	if (cpu_rate >= 1300000000)
		/* cpu >= 1.3GHz, emc max */
		clk_set_rate(priv.emc_clk,
			clk_round_rate(priv.emc_clk, ULONG_MAX));
	else if (cpu_rate >= 975000000)
		/* cpu >= 975 MHz, emc 400 MHz */
		clk_set_rate(priv.emc_clk, 400000000);
	else if (cpu_rate >= 725000000)
		/* cpu >= 725 MHz, emc 200 MHz */
		clk_set_rate(priv.emc_clk, 200000000);
	else if (cpu_rate >= 500000000)
		/* cpu >= 500 MHz, emc 100 MHz */
		clk_set_rate(priv.emc_clk, 100000000);
	else if (cpu_rate >= 275000000)
		/* cpu >= 275 MHz, emc 50 MHz */
		clk_set_rate(priv.emc_clk, 50000000);
	else
		/* emc min */
		clk_set_rate(priv.emc_clk, 0);
}

static int tegra210_set_target(struct cpufreq_policy *policy,
		unsigned int index)
{
	struct cpufreq_frequency_table *freq_table = policy->freq_table;
	unsigned int old_rate, new_rate;

	mutex_lock(&tegra_cpu_lock);

	new_rate = clk_round_rate(priv.cpu_clk, freq_table[index].frequency * 1000);
	old_rate = clk_get_rate(priv.cpu_clk);

	if (!IS_ERR(priv.emc_clk) && new_rate > old_rate)
		tegra210_vote_emc_rate(new_rate);

	if (priv.dfll_mode)
		clk_set_rate(priv.cpu_clk, new_rate);
	else
		tegra210_set_rate_pll(new_rate, old_rate);

	if (!IS_ERR(priv.emc_clk) && new_rate < old_rate)
		tegra210_vote_emc_rate(new_rate);

	mutex_unlock(&tegra_cpu_lock);

	return 0;
}

static int tegra210_cpufreq_init(struct cpufreq_policy *policy)
{
	struct cpufreq_frequency_table *freq_table;
	struct device_node *np;
	unsigned int transition_latency;
	int ret;

	np = of_cpu_device_node_get(0);
	if (!np)
		return -ENODEV;

	of_init_opp_table(priv.cpu_dev);

	if (of_property_read_u32(np, "clock-latency", &transition_latency))
		transition_latency = CPUFREQ_ETERNAL;

	ret = dev_pm_opp_init_cpufreq_table(priv.cpu_dev, &freq_table);
	if (ret) {
		pr_err("failed to init cpufreq table: %d\n", ret);
		goto out_put_node;
	}

	policy->clk = priv.cpu_clk;
	policy->cpuinfo.transition_latency = transition_latency;
	policy->suspend_freq = freq_table[priv.suspend_index].frequency;

	ret = cpufreq_table_validate_and_show(policy, freq_table);
	if (ret) {
		dev_err(priv.cpu_dev, "%s: invalid frequency table: %d\n",
				__func__, ret);
		goto out_unregister;
	}

	of_node_put(np);

	return 0;
out_unregister:
	dev_pm_opp_free_cpufreq_table(priv.cpu_dev, &freq_table);
out_put_node:
	of_node_put(np);

	return ret;
}

static int tegra210_cpufreq_exit(struct cpufreq_policy *policy)
{
	dev_pm_opp_free_cpufreq_table(priv.cpu_dev, &policy->freq_table);

	return 0;
}

static struct cpufreq_driver tegra210_cpufreq_driver = {
	.flags = CPUFREQ_STICKY | CPUFREQ_NEED_INITIAL_FREQ_CHECK,
	.verify = cpufreq_generic_frequency_table_verify,
	.target_index = tegra210_set_target,
	.get = cpufreq_generic_get,
	.init = tegra210_cpufreq_init,
	.exit = tegra210_cpufreq_exit,
	.name = "cpufreq-tegra210",
	.attr = cpufreq_generic_attr,
};

static int tegra210_cpufreq_probe(struct platform_device *pdev)
{
	struct device_node *np;
	struct device *cpu_dev;
	int ret;

	cpu_dev = get_cpu_device(0);
	if (!cpu_dev)
		return -ENODEV;

	priv.cpu_dev = cpu_dev;

	np = of_cpu_device_node_get(0);
	if (!np)
		return -ENODEV;

	priv.vdd_cpu_reg = regulator_get(cpu_dev, "vdd-cpu");
	if (IS_ERR(priv.vdd_cpu_reg)) {
		ret = PTR_ERR(priv.vdd_cpu_reg);
		goto out_put_np;
	}

	priv.cpu_clk = priv.cpu_g_clk = of_clk_get_by_name(np, "cpu_g");
	if (IS_ERR(priv.cpu_g_clk)) {
		ret = PTR_ERR(priv.cpu_g_clk);
		goto out_put_vdd_cpu_reg;
	}

	priv.cpu_lp_clk = of_clk_get_by_name(np, "cpu_lp");
	if (IS_ERR(priv.cpu_lp_clk)) {
		ret = PTR_ERR(priv.cpu_lp_clk);
		goto out_put_cpu_clk;
	}

	priv.dfll_clk = of_clk_get_by_name(np, "dfll");
	if (IS_ERR(priv.dfll_clk)) {
		ret = PTR_ERR(priv.dfll_clk);
		goto out_put_cpu_lp_clk;
	}

	priv.pllx_clk = of_clk_get_by_name(np, "pll_x");
	if (IS_ERR(priv.pllx_clk)) {
		ret = PTR_ERR(priv.pllx_clk);
		goto out_put_dfll_clk;
	}

	priv.pllp_clk = of_clk_get_by_name(np, "pll_p");
	if (IS_ERR(priv.pllp_clk)) {
		ret = PTR_ERR(priv.pllp_clk);
		goto out_put_pllx_clk;
	}

	priv.emc_clk = of_clk_get_by_name(np, "emc");
	if (IS_ERR(priv.emc_clk))
		pr_info("Tegra210_cpufreq: no cpu.emc shared clock found\n");

	rcu_read_lock();
	ret = dev_pm_opp_get_opp_count(cpu_dev);
	rcu_read_unlock();
	if (ret <= 0) {
		pr_debug("OPP table is not ready, deferring probe\n");
		ret = -EPROBE_DEFER;
		goto out_put_pllp_clk;
	}
	priv.suspend_index = ret - 1;

	ret = tegra210_cpu_switch_to_dfll();
	if (ret)
		goto out_put_pllp_clk;

	ret = cpufreq_register_driver(&tegra210_cpufreq_driver);
	if (ret)
		dev_err(priv.cpu_dev, "failed register driver: %d\n", ret);

	return 0;

out_put_pllp_clk:
	clk_put(priv.pllp_clk);
out_put_pllx_clk:
	clk_put(priv.pllx_clk);
out_put_dfll_clk:
	clk_put(priv.dfll_clk);
out_put_cpu_lp_clk:
	clk_put(priv.cpu_lp_clk);
out_put_cpu_clk:
	clk_put(priv.cpu_clk);
out_put_vdd_cpu_reg:
	regulator_put(priv.vdd_cpu_reg);
out_put_np:
	of_node_put(np);

	return ret;
}

static int tegra210_cpufreq_remove(struct platform_device *pdev)
{
	tegra210_cpu_switch_to_pllx();

	clk_put(priv.pllp_clk);
	clk_put(priv.pllx_clk);
	clk_put(priv.dfll_clk);
	clk_put(priv.cpu_clk);
	regulator_put(priv.vdd_cpu_reg);

	return 0;
}

static struct platform_driver tegra210_cpufreq_platdrv = {
	.driver = {
		.name	= "cpufreq-tegra210",
	},
	.probe		= tegra210_cpufreq_probe,
	.remove		= tegra210_cpufreq_remove,
};

static const struct cpuquiet_platform_info tegra_plat_info = {
	.plat_name = "tegra",
	.avg_hotplug_latency_ms = 2,
};

static int __init tegra_cpufreq_init(void)
{
	int ret;
	struct platform_device *pdev;

	if (!of_machine_is_compatible("nvidia,tegra210"))
		return -ENODEV;

	ret = platform_driver_register(&tegra210_cpufreq_platdrv);
	if (ret)
		return ret;

	pdev = platform_device_register_simple("cpufreq-tegra210", -1, NULL, 0);
	if (IS_ERR(pdev)) {
		platform_driver_unregister(&tegra210_cpufreq_platdrv);
		return PTR_ERR(pdev);
	}

	cpuquiet_init(&tegra_plat_info);

	return 0;
}
module_init(tegra_cpufreq_init);

MODULE_AUTHOR("Penny Chiu <pchiu@nvidia.com>");
MODULE_DESCRIPTION("cpufreq driver for NVIDIA tegra210");
MODULE_LICENSE("GPL v2");
