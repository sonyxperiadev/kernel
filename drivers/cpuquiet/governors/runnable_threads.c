/*
 * Copyright (c) 2012-2013 NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/cpuquiet.h>
#include <linux/cpumask.h>
#include <linux/module.h>
#include <linux/pm_qos.h>
#include <linux/jiffies.h>
#include <linux/cpu.h>
#include <linux/sched.h>

#include "../cpuquiet.h"

static struct work_struct runnables_work;
static struct timer_list runnables_timer;

static bool runnables_enabled;
/* configurable parameters */
static unsigned int sample_rate = 200;	  /* msec */

#define NR_FSHIFT_EXP  3
#define NR_FSHIFT      (1 << NR_FSHIFT_EXP)
/* avg run threads * 8 (e.g., 11 = 1.375 threads) */
static unsigned int default_thresholds[] = {
	10, 18, 20, UINT_MAX
};

static unsigned int nr_run_last;
static unsigned int nr_run_hysteresis = 2;	     /* 1 / 2 thread */
static unsigned int default_threshold_level = 4;	/* 1 / 4 thread */
static unsigned int nr_run_thresholds[NR_CPUS];

struct runnables_avg_sample {
	u64 previous_integral;
	unsigned int avg;
	bool integral_sampled;
	u64 prev_timestamp;
};

static DEFINE_PER_CPU(struct runnables_avg_sample, avg_nr_sample);

/*
 * EXP = alpha in the exponential moving average.
 * Alpha = e ^ (-sample_rate / window_size) * FIXED_1
 * Calculated for sample_rate of 20ms, window size of 100ms
 */
#define EXP    1677

static unsigned int get_avg_nr_runnables(void)
{
	unsigned int i, sum = 0;
	static unsigned int avg;
	struct runnables_avg_sample *sample;
	u64 integral, old_integral, delta_integral, delta_time, cur_time;

	for_each_online_cpu(i) {
		sample = &per_cpu(avg_nr_sample, i);
		integral = nr_running_integral(i);
		old_integral = sample->previous_integral;
		sample->previous_integral = integral;
		cur_time = ktime_to_ns(ktime_get());
		delta_time = cur_time - sample->prev_timestamp;
		sample->prev_timestamp = cur_time;

		if (!sample->integral_sampled) {
			sample->integral_sampled = true;
			/*
			 * First sample to initialize prev_integral, skip
			 * avg calculation
			 */
			continue;
		}

		if (integral < old_integral) {
			/* Overflow */
			delta_integral = (ULLONG_MAX - old_integral) + integral;
		} else {
			delta_integral = integral - old_integral;
		}

		/* Calculate average for the previous sample window */
		do_div(delta_integral, delta_time);
		sample->avg = delta_integral;
		sum += sample->avg;
	}

	/*
	 * Exponential moving average
	 * Avgn = Avgn-1 * alpha + new_avg * (1 - alpha)
	 */
	avg *= EXP;
	avg += sum * (FIXED_1 - EXP);
	avg >>= FSHIFT;

	return avg;
}

static int get_action(unsigned int nr_run)
{
	unsigned int nr_cpus = num_online_cpus();
	int max_cpus = pm_qos_request(PM_QOS_MAX_ONLINE_CPUS) ? : 4;
	int min_cpus = pm_qos_request(PM_QOS_MIN_ONLINE_CPUS);

	if ((nr_cpus > max_cpus || nr_run < nr_cpus) && nr_cpus > min_cpus)
		return -1;

	if ((nr_cpus < min_cpus || nr_run > nr_cpus) && nr_cpus < max_cpus)
		return 1;

	return 0;
}

static void runnables_avg_sampler(unsigned long data)
{
	unsigned int nr_run, avg_nr_run;
	int action;

	if (!runnables_enabled)
		return;

	avg_nr_run = get_avg_nr_runnables();
	mod_timer(&runnables_timer, jiffies + msecs_to_jiffies(sample_rate));

	for (nr_run = 1; nr_run < ARRAY_SIZE(nr_run_thresholds); nr_run++) {
		unsigned int nr_threshold = nr_run_thresholds[nr_run - 1];
		if (nr_run_last <= nr_run)
			nr_threshold += NR_FSHIFT / nr_run_hysteresis;
		if (avg_nr_run <= (nr_threshold << (FSHIFT - NR_FSHIFT_EXP)))
			break;
	}

	nr_run_last = nr_run;
	action = get_action(nr_run);

	if (action != 0)
		schedule_work(&runnables_work);
}

static unsigned int get_lightest_loaded_cpu(void)
{
	unsigned long min_avg_runnables = ULONG_MAX;
	unsigned int cpu = nr_cpu_ids;
	int i;

	for_each_online_cpu(i) {
		struct runnables_avg_sample *s = &per_cpu(avg_nr_sample, i);
		unsigned int nr_runnables = s->avg;
		if (i > 0 && min_avg_runnables > nr_runnables) {
			cpu = i;
			min_avg_runnables = nr_runnables;
		}
	}

	return cpu;
}

static void runnables_work_func(struct work_struct *work)
{
	unsigned int cpu = nr_cpu_ids;
	int action;

	if (!runnables_enabled)
		return;

	action = get_action(nr_run_last);
	if (action > 0)
		cpu = cpumask_next_zero(0, cpu_online_mask);
	else if (action < 0)
		cpu = get_lightest_loaded_cpu();

	if (cpu > nr_cpu_ids)
		return;

	if (action > 0)
		cpuquiet_wake_cpu(cpu, false);
	if (action < 0)
		cpuquiet_quiesce_cpu(cpu, false);
}

CPQ_SIMPLE_ATTRIBUTE(sample_rate, 0644, uint);
CPQ_SIMPLE_ATTRIBUTE(nr_run_hysteresis, 0644, uint);

static struct attribute *runnables_attrs[] = {
	&sample_rate_attr.attr,
	&nr_run_hysteresis_attr.attr,
	NULL,
};

static struct attribute_group runnables_group = {
	.name = "runnable_threads",
	.attrs = runnables_attrs,
};

static int runnables_sysfs_init(void)
{
	return cpuquiet_register_attrs(&runnables_group);
}

static void runnables_sysfs_exit(void)
{
	cpuquiet_unregister_attrs(&runnables_group);
}

static void runnables_stop(void)
{
	runnables_enabled = false;
	del_timer_sync(&runnables_timer);
	cancel_work_sync(&runnables_work);
	runnables_sysfs_exit();
}

static int runnables_start(void)
{
	int i, err, arch_specific_sample_rate;

	err = runnables_sysfs_init();
	if (err)
		return err;

	INIT_WORK(&runnables_work, runnables_work_func);

	init_timer(&runnables_timer);
	runnables_timer.function = runnables_avg_sampler;

	arch_specific_sample_rate = cpuquiet_get_avg_hotplug_latency();
	if (arch_specific_sample_rate)
		/*
		 * Sample at least 10 times as slowly as overhead for one
		 * hotplug event.
		 */
		sample_rate = arch_specific_sample_rate * 10;

	for (i = 0; i < ARRAY_SIZE(nr_run_thresholds); ++i) {
		if (i < ARRAY_SIZE(default_thresholds))
			nr_run_thresholds[i] = default_thresholds[i];
		else if (i == (ARRAY_SIZE(nr_run_thresholds) - 1))
			nr_run_thresholds[i] = UINT_MAX;
		else
			nr_run_thresholds[i] = i + 1 +
				NR_FSHIFT / default_threshold_level;
	}

	runnables_enabled = true;

	runnables_avg_sampler(0);

	return 0;
}

static struct cpuquiet_governor runnables_governor = {
	.name			= "runnable",
	.start			= runnables_start,
	.stop			= runnables_stop,
	.owner			= THIS_MODULE,
};

static int __init init_runnables(void)
{
	return cpuquiet_register_governor(&runnables_governor);
}

static void __exit exit_runnables(void)
{
	cpuquiet_unregister_governor(&runnables_governor);
}

MODULE_LICENSE("GPL");
#ifdef CONFIG_CPU_QUIET_DEFAULT_GOV_RUNNABLE
fs_initcall(init_runnables);
#else
module_init(init_runnables);
#endif
module_exit(exit_runnables);
