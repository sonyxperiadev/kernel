/*
 * Balanced RQ-based Hotplug driver for MSM SoC
 *
 * Based on drivers/cpuquiet/governors/balanced.c
 * Contains portions of code from EternityProject T3 CPUManager
 * Contains portions of code from NVIDIA CORPORATION
 *
 * Authored by: AngeloGioacchino Del Regno <kholk11@gmail.com>
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

/* Note: NEVER suspend the runqueue averaging worker! */

#include <linux/kernel.h>
#include <linux/cpuquiet.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/pm.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/sched.h>
#include <linux/suspend.h>
#include <linux/tick.h>
#include <linux/workqueue.h>
#include <asm/cputime.h>

#define CPUNAMELEN 8

#define RQ_SAMPLE_TIME_NS	250000000
#define RQ_AVG_TIMER_RATE_NS	10000000
#define RQ_SAMPLE_CAPACITY	(RQ_SAMPLE_TIME_NS / RQ_AVG_TIMER_RATE_NS)

/* Don't put any CPU down prior at least 3x scheduling samplings */
#define DNCORE_DELAY_MS		(3 * (RQ_SAMPLE_TIME_NS / NSEC_PER_MSEC))
#define UPCORE_DELAY_MS		((RQ_SAMPLE_TIME_NS / NSEC_PER_MSEC) / 2)

typedef enum {
	/* Brings one more CPU core on-line */
	CPU_UPCORE,

	/* Keeps CPU core composition unchanged */
	CPU_FAIR,

	/* Offline one CPU core */
	CPU_DNCORE,
} CPU_SPEED_BALANCE;

typedef enum {
	IDLE,
	DOWN,
	UP,
} RQBALANCE_STATE;

#define USERSPACE_RUNNING	0
#define USERSPACE_LOW_POWER	1

#define CLUSTER_LITTLE		0
#define CLUSTER_BIG		1
#define MAX_CLUSTERS		2

struct idle_info {
	u64 idle_last;
	u64 last_timestamp;
	u64 idle_current;
	u64 timestamp;
};

static DEFINE_PER_CPU(struct idle_info, idleinfo);
static DEFINE_PER_CPU(unsigned int, cpu_load);

static struct timer_list load_timer;
static bool load_timer_active;
static bool soc_is_hmp;
static unsigned int available_clusters;

/* configurable parameters */
static unsigned int  balance_level = 60;
static unsigned int  idle_bottom_freq[MAX_CLUSTERS];
static unsigned int  idle_top_freq[MAX_CLUSTERS];
static unsigned int  num_of_cores[MAX_CLUSTERS];
static unsigned long up_delay;
static unsigned long down_delay;
static unsigned long last_change_time;
static unsigned int  load_sample_rate = 20; /* msec */
static unsigned int  userspace_suspend_state = 0;
static struct workqueue_struct *rqbalance_wq;
static struct delayed_work rqbalance_work;
static RQBALANCE_STATE rqbalance_state;
static struct kobject *rqbalance_kobject;
static struct notifier_block pm_notifier_block;

static void calculate_load_timer(unsigned long data)
{
	int i;
	u64 idle_time, elapsed_time;

	if (!load_timer_active)
		return;

	for_each_online_cpu(i) {
		struct idle_info *iinfo = &per_cpu(idleinfo, i);
		unsigned int *load = &per_cpu(cpu_load, i);

		iinfo->idle_last = iinfo->idle_current;
		iinfo->last_timestamp = iinfo->timestamp;
		iinfo->idle_current =
			get_cpu_idle_time_us(i, &iinfo->timestamp);
		elapsed_time = iinfo->timestamp - iinfo->last_timestamp;

		idle_time = iinfo->idle_current - iinfo->idle_last;
		idle_time *= 100;
		do_div(idle_time, elapsed_time);
		*load = 100 - idle_time;
	}
	mod_timer(&load_timer, jiffies + msecs_to_jiffies(load_sample_rate));
}

static void start_load_timer(void)
{
	int i;

	if (load_timer_active)
		return;

	load_timer_active = true;

	for_each_online_cpu(i) {
		struct idle_info *iinfo = &per_cpu(idleinfo, i);

		iinfo->idle_current =
			get_cpu_idle_time_us(i, &iinfo->timestamp);
	}
	mod_timer(&load_timer, jiffies + msecs_to_jiffies(100));
}

static void stop_load_timer(void)
{
	if (!load_timer_active)
		return;

	load_timer_active = false;
	del_timer(&load_timer);
}

static unsigned int get_slowest_cpu_n(void)
{
	unsigned int cpu = nr_cpu_ids;
	unsigned long minload = ULONG_MAX;
	int i;

	for_each_online_cpu(i) {
		unsigned int *load = &per_cpu(cpu_load, i);

		if ((i > 0) && (minload > *load)) {
			cpu = i;
			minload = *load;
		}
	}

	return cpu;
}

static unsigned int cpu_highest_speed(void)
{
	unsigned int maxload = 0;
	int i;

	for_each_online_cpu(i) {
		unsigned int *load = &per_cpu(cpu_load, i);

		maxload = max(maxload, *load);
	}

	return maxload;
}

static unsigned int count_slow_cpus(unsigned int limit)
{
	unsigned int cnt = 0;
	int i;

	for_each_online_cpu(i) {
		unsigned int *load = &per_cpu(cpu_load, i);

		if (*load <= limit)
			cnt++;
	}

	return cnt;
}

static unsigned int num_online_cluster_cpus(bool cluster)
{
	unsigned int cnt = 0;
	int i;

	for_each_online_cpu(i) {
		if (topology_physical_package_id(i) == cluster)
			cnt++;
	}

	return cnt;
}

static inline int get_offline_core(int cluster)
{
	int i;

	for_each_possible_cpu(i) {
		if (topology_physical_package_id(i) != cluster)
			continue;

		if (cpu_is_offline(i))
			return i;
	};

	return 0;
}

/*
 * There are two ladders here for hysteresis. We do not want to offline
 * and online a CPU repeatedly. Thus, the down thresholds are *lower*
 * than the corresponding up threshold.
 * The size of both arrays shall be the same.
 *
 * NOTE: On HMP multi-cluster systems, the last value is used to enable
 *       onlining of the high performance cluster CPUs, but ONLY when
 *       clusterswitch (one cluster at a time) mode is enabled.
 */
static unsigned int nr_run_thresholds[] = {
/*      1,	2,	3,	4 (target on-line num of cpus) */
	150,	400,	500,	520,

/*	5,	6,	7,	8 (HMP target on-line num of CPUs) */
	550,	640,	750,	UINT_MAX
};

static unsigned int nr_down_run_thresholds[] = {
/*	1,	2,	3,	4 (target off-line num of cpus) */
	0,	80,	290,	360,

/*	5,	6,	7,	8 (HMP target off-line num of CPUs) */
	440,	500,	550,	680
};

/*
 * Those are hysteresis values for the high performance CPUs.
 *
 * The logic in this is to add the hysteresis value as it is if we want
 * to calculate our ladders for onlining a core (upcore), but divide by
 * half (each corresponding value) if we want to calculate the ladders
 * for offlining a core (dncore).
 *
 * This logic pretends we want less power consumption at a little
 * performance cost.
 * On mobile devices with passive cooling, this is expected to increase
 * CPU clusters efficiency by reducing heat and subsequently throttling.
 *
 * NOTE: This is used to compose *complete* run thresholds arrays for
 *       the high performance CPUs cluster when clusterswitch mode
 *       is disabled and when we can use both clusters simultaneously.
 */
//static unsigned int hiperf_cpu_hysteresis[] = {
/*	1,	2,	3,	4 (used only for downcore hysteresis) */
//	15,     25,	30,	60
//};

struct runqueue_sample {
	int64_t sample_time;
	unsigned long nr_run;
};

struct runqueue_data {
	unsigned int update_rate;

	/* Circular buffer. */
	struct runqueue_sample nr_run_samples[RQ_SAMPLE_CAPACITY];

	/* Index of the head of the circular buffer = the newest element. */
	uint8_t nr_run_sample_head;

	/* Index of the tail of the circular buffer = oldest element. */
	uint8_t nr_run_sample_tail;

	/* Number of samples in the buffer. */
	uint8_t nr_run_sample_count;

	struct delayed_work work;
	struct workqueue_struct *nr_run_wq;
	spinlock_t lock;
};

static struct runqueue_data *rq_data;

static void rq_work_fn(struct work_struct *work)
{
	unsigned long flags = 0;
	int64_t cur_time = ktime_to_ns(ktime_get());
	struct runqueue_sample* sample = NULL;

	spin_lock_irqsave(&rq_data->lock, flags);
	if (unlikely(rq_data->nr_run_sample_count != RQ_SAMPLE_CAPACITY)) {
		/* Buffer still growing. */
		rq_data->nr_run_sample_head = rq_data->nr_run_sample_count;
		rq_data->nr_run_sample_tail = 0;
		++rq_data->nr_run_sample_count;
	} else {
		/* Buffer already full. We will be clobbering old samples. */
		rq_data->nr_run_sample_head =
			rq_data->nr_run_sample_head == RQ_SAMPLE_CAPACITY - 1 ?
				0 :
				rq_data->nr_run_sample_head + 1;
		rq_data->nr_run_sample_tail =
			rq_data->nr_run_sample_tail == RQ_SAMPLE_CAPACITY - 1 ?
				0 :
				rq_data->nr_run_sample_tail + 1;
	}
	sample = rq_data->nr_run_samples + (rq_data->nr_run_sample_head);

	/* Store the sample. */
	sample->sample_time = cur_time;
	sample->nr_run = nr_running();

	if (likely(rq_data->update_rate != 0))
		queue_delayed_work(rq_data->nr_run_wq, &rq_data->work,
				   msecs_to_jiffies(rq_data->update_rate));

	spin_unlock_irqrestore(&rq_data->lock, flags);
}

unsigned int get_nr_run_avg(void)
{
	struct runqueue_sample nr_run_samples[RQ_SAMPLE_CAPACITY];
	uint8_t nr_run_sample_head;
	uint8_t nr_run_sample_count;

	size_t j;
	struct runqueue_sample* i;
	unsigned int nr_run = 0;
	unsigned long flags = 0;
	int64_t after_time = ktime_to_ns(ktime_get()) - RQ_SAMPLE_TIME_NS;

	spin_lock_irqsave(&rq_data->lock, flags);
	memcpy(nr_run_samples, rq_data->nr_run_samples, sizeof(nr_run_samples));
	nr_run_sample_head = rq_data->nr_run_sample_head;
	nr_run_sample_count = rq_data->nr_run_sample_count;
	spin_unlock_irqrestore(&rq_data->lock, flags);

	for (i = nr_run_samples + nr_run_sample_head, j = 0;
	     j < nr_run_sample_count; ++j) {
		if (i->sample_time < after_time) {
			/* This sample is older than the ones we find relevant. */
			break;
		}

		nr_run += i->nr_run;

		/* Get the previous element in the circular buffer. */
		if (unlikely(i == nr_run_samples)) {
			/* Wrap around. */
			i = nr_run_samples + (RQ_SAMPLE_CAPACITY - 1);
		} else {
			--i;
		}
	}

	if (unlikely(j == 0))
		return 0;

	return nr_run * 100 / j;
}

static void start_rq_work(void)
{
	if (rq_data->nr_run_wq == NULL)
		rq_data->nr_run_wq =
			create_freezable_workqueue("nr_run_avg");

	queue_delayed_work(rq_data->nr_run_wq, &rq_data->work,
			   msecs_to_jiffies(rq_data->update_rate));
	return;
}

static void stop_rq_work(void)
{
	if (rq_data->nr_run_wq)
		cancel_delayed_work(&rq_data->work);
	return;
}

/*
 * This is the brain of the hotplug implementation.
 *
 * This function is called by rqbalance_work_func to determine if
 * the CPU composition needs to be changed.
 */
static CPU_SPEED_BALANCE balanced_speed_balance(void)
{
	unsigned long highest_speed = cpu_highest_speed();
	unsigned long balanced_speed = highest_speed * balance_level / 100;
	unsigned long skewed_speed = balanced_speed / 2;
	unsigned int nr_cpus = num_online_cpus();
	unsigned int max_cpus = cpuquiet_get_cpus(true);
	unsigned int avg_nr_run = get_nr_run_avg();
	unsigned int nr_run;
	bool done;

	/* First use the up thresholds to see if we need to bring CPUs online. */
	pr_debug("%s: Current core count max runqueue: %d\n", __func__,
		 nr_run_thresholds[nr_cpus - 1]);
	for (nr_run = nr_cpus;
		nr_run < ARRAY_SIZE(nr_run_thresholds);
		++nr_run) {
		if (avg_nr_run <= nr_run_thresholds[nr_run - 1])
			break;
	}

	/*
	 * Then use the down thresholds to see if we can offline CPUs.
	 * This looks counter-intuitive, but it does work. The first loop
	 * handles the case where there are more things running than our up
	 * thresholds. If we have now too many CPUs online for too few tasks,
	 * the loop above would be a no-op, and we will fall through to this
	 * down threshold comparison loop.
	 */
	for ( ; nr_run > 1; --nr_run) {
		if (done || avg_nr_run >= nr_down_run_thresholds[nr_run - 1]) {
			/* We have fewer things running than our down threshold.
			   Use one less CPU. */
			break;
		}
	}


	/* Consider when to take a CPU offline.  We need to have at least
	 * one extra idle CPU, or either of: */
	if (count_slow_cpus(skewed_speed) >= 2 ||

	    /* More CPUs than policy allows */
	    (nr_cpus > max_cpus) ||

	    /* Runqueue has been emptied */
	    (nr_run < nr_cpus))
		return CPU_DNCORE;

	/* Consider when we may want to keep CPU composition unchanged.
	 * We need to have no extra idle CPU, or: */
	if (count_slow_cpus(balanced_speed) >= 1 ||

	    /* Already at maximum CPUs allowed by policy */
	    (nr_cpus == max_cpus) ||

	    /* Enough CPUs to process the runqueue */
	    (nr_run <= nr_cpus))
		return CPU_FAIR;

	/* Otherwise, we have to online one more CPU */
	return CPU_UPCORE;
}

static void hotplug_smp_final_decision(unsigned int cpu, bool up)
{
	if (up)
		/* TODO: If we just put another CPU online, reduce the frequency of
		 * the CPU since we have increased the overall computational
		 * power of the processor. */
		cpuquiet_wake_cpu(cpu, false);
	else
		cpuquiet_quiesence_cpu(cpu, false);
}

static void hotplug_hmp_final_decision(unsigned int cpu, bool up)
{
	unsigned int cluster = topology_physical_package_id(cpu);
	unsigned int little_cores_on, big_cores_on;

	if (up) {
		/* Analyze online cores situation */
		big_cores_on = num_online_cluster_cpus(CLUSTER_BIG);
		little_cores_on = num_online_cluster_cpus(CLUSTER_LITTLE);

		/* If we want 4 little cores, we can instead use one
		   big core and two little */
		if (cluster == CLUSTER_LITTLE &&
		    little_cores_on >= num_of_cores[CLUSTER_LITTLE] - 1 &&
		    big_cores_on == 0) {
			cpuquiet_wake_cpu(
				get_offline_core(CLUSTER_BIG), false);
			cpuquiet_quiesence_cpu(
				get_offline_core(CLUSTER_LITTLE), false);
			return;
		};
		cpuquiet_wake_cpu(cpu, false);
	} else {
		cpuquiet_quiesence_cpu(cpu, false);
	}
}

/*
 * This is the main machinery for the hotplug implementation.
 *
 * This function will call itself at the end of approximately every up_delay
 * jiffies.
 *
 * The decision to power up or power down a CPU is made by
 * balanced_speed_balance.
 */
static void rqbalance_work_func(struct work_struct *work)
{
	bool up = false;
	unsigned int cpu = nr_cpu_ids;
	unsigned long now = jiffies;

	CPU_SPEED_BALANCE balance;

	switch (userspace_suspend_state) {
		case USERSPACE_RUNNING:
			break;
		case USERSPACE_LOW_POWER:
		{
			int i;
			if (num_online_cpus() == 1)
				return;
			for (i = 1; i < nr_cpu_ids; i++)
				cpu_down(cpu);
			return;
		}
	}

	switch (rqbalance_state) {
	case IDLE:
		break;
	case DOWN:
		cpu = get_slowest_cpu_n();
		if (cpu < nr_cpu_ids)
			up = false;
		else
			stop_load_timer();

			queue_delayed_work(rqbalance_wq,
						 &rqbalance_work, up_delay);
		break;
	case UP:
		balance = balanced_speed_balance();
		switch (balance) {
		/* cpu speed is up and balanced - one more on-line */
		case CPU_UPCORE:
			cpu = cpumask_next_zero(0, cpu_online_mask);
			if (cpu < nr_cpu_ids)
				up = true;
			break;
		/* cpu speed is up, but skewed - remove one core */
		case CPU_DNCORE:
			cpu = get_slowest_cpu_n();
			if (cpu < nr_cpu_ids)
				up = false;
			break;
		/* cpu speed is up, but under-utilized - do nothing */
		case CPU_FAIR:
		default:
			break;
		}
		queue_delayed_work(
			rqbalance_wq, &rqbalance_work, up_delay);
		break;
	default:
		pr_err("%s: invalid cpuquiet balanced governor state %d\n",
		       __func__, rqbalance_state);
	}

	if (!up && ((now - last_change_time) < down_delay))
		cpu = nr_cpu_ids;

	if (cpu < nr_cpu_ids) {
		last_change_time = now;
		if (!soc_is_hmp)
			hotplug_smp_final_decision(cpu, up);
		else
			hotplug_hmp_final_decision(cpu, up);
	}
}

static int balanced_cpufreq_transition(struct notifier_block *nb,
	unsigned long state, void *data)
{
	struct cpufreq_freqs *freqs = data;
	unsigned long cpu_freq;
	int n;

	/*
	 * If we have at least one BIG CPU online, then take
	 * a decision based on calculations made on BIG.
	 */
	if (soc_is_hmp)
		if (num_online_cluster_cpus(CLUSTER_BIG) > 0)
			n = 1;

	if (state == CPUFREQ_POSTCHANGE || state == CPUFREQ_RESUMECHANGE) {
		cpu_freq = freqs->new;

		switch (rqbalance_state) {
		case IDLE:
			if (cpu_freq >= idle_top_freq[n]) {
				rqbalance_state = UP;
				queue_delayed_work(
					rqbalance_wq, &rqbalance_work, up_delay);
				start_load_timer();
			} else if (cpu_freq <= idle_bottom_freq[n]) {
				rqbalance_state = DOWN;
				queue_delayed_work(
					rqbalance_wq, &rqbalance_work,
					down_delay);
				start_load_timer();
			}
			break;
		case DOWN:
			if (cpu_freq >= idle_top_freq[n]) {
				rqbalance_state = UP;
				queue_delayed_work(
					rqbalance_wq, &rqbalance_work, up_delay);
				start_load_timer();
			}
			break;
		case UP:
			if (cpu_freq <= idle_bottom_freq[n])
				rqbalance_state = DOWN;

			queue_delayed_work(rqbalance_wq,
				&rqbalance_work, up_delay);
			start_load_timer();
			break;
		default:
			pr_err("%s: invalid cpuquiet rqbalance governor "
				"state %d\n", __func__, rqbalance_state);
		}
	}

	return NOTIFY_OK;
}

static struct notifier_block balanced_cpufreq_nb = {
	.notifier_call = balanced_cpufreq_transition,
};

static void delay_callback(struct cpuquiet_attribute *attr)
{
	unsigned long val;

	if (attr) {
		val = (*((unsigned long *)(attr->param)));
		(*((unsigned long *)(attr->param))) = msecs_to_jiffies(val);
	}
}

static inline size_t array_size_from_name(const char* name)
{
	if (!strncmp(name, "idle_bottom_freq", strlen(name)-2))
		return ARRAY_SIZE(idle_bottom_freq);

	if (!strncmp(name, "idle_top_freq", strlen(name)-2))
		return ARRAY_SIZE(idle_top_freq);

	if (!strncmp(name, "nr_down_run_thresholds", strlen(name)-2))
		return ARRAY_SIZE(nr_down_run_thresholds);

	if (!strncmp(name, "nr_run_thresholds", strlen(name)-2))
		return ARRAY_SIZE(nr_run_thresholds);

	return -EINVAL;
}

ssize_t store_uint_array(struct cpuquiet_attribute *cattr,
				const char *buf, size_t count)
{
	const char *i;
	uint8_t j;
	size_t sz = array_size_from_name(cattr->attr.name);

	for (i = buf, j = 0; *i && (j < sz); j++) {
		int read = 0;
		unsigned int val = 0;

		sscanf(i, "%u%n", &val, &read);
		if (!read)
			break;

		i += read;
		((unsigned int*)cattr->param)[j] = val;
	}

	return count;
}

ssize_t show_uint_array(struct cpuquiet_attribute *cattr,
					char *buf)
{
	int i;
	char *temp = buf;
	size_t sz = array_size_from_name(cattr->attr.name);

	for (i = 0; i < sz; i++)
		temp += sprintf(temp, "%u ", ((unsigned int*)cattr->param)[i]);

	temp += sprintf(temp, "\n");
	return temp - buf;
}

CPQ_BASIC_ATTRIBUTE(balance_level, 0644, uint);
CPQ_BASIC_ATTRIBUTE(load_sample_rate, 0644, uint);
CPQ_BASIC_ATTRIBUTE(userspace_suspend_state, 0644, uint);
CPQ_ATTRIBUTE(up_delay, 0644, ulong, delay_callback);
CPQ_ATTRIBUTE(down_delay, 0644, ulong, delay_callback);

CPQ_ATTRIBUTE_CUSTOM(idle_bottom_freq, 0644,
			show_uint_array, store_uint_array);
CPQ_ATTRIBUTE_CUSTOM(idle_top_freq, 0644,
			show_uint_array, store_uint_array);
CPQ_ATTRIBUTE_CUSTOM(nr_down_run_thresholds, 0644,
			show_uint_array, store_uint_array);
CPQ_ATTRIBUTE_CUSTOM(nr_run_thresholds, 0644,
			show_uint_array, store_uint_array);

static struct attribute *rqbalance_attributes[] = {
	&balance_level_attr.attr,
	&userspace_suspend_state_attr.attr,
	&up_delay_attr.attr,
	&down_delay_attr.attr,
	&load_sample_rate_attr.attr,
	&idle_bottom_freq_attr.attr,
	&idle_top_freq_attr.attr,
	&nr_down_run_thresholds_attr.attr,
	&nr_run_thresholds_attr.attr,
	NULL,
};

static const struct sysfs_ops rqbalance_sysfs_ops = {
	.show = cpuquiet_auto_sysfs_show,
	.store = cpuquiet_auto_sysfs_store,
};

static struct kobj_type ktype_rqbalance = {
	.sysfs_ops = &rqbalance_sysfs_ops,
	.default_attrs = rqbalance_attributes,
};

static int rqbalance_sysfs(void)
{
	int err;

	rqbalance_kobject = kzalloc(sizeof(*rqbalance_kobject),
				GFP_KERNEL);

	if (!rqbalance_kobject)
		return -ENOMEM;

	err = cpuquiet_kobject_init(rqbalance_kobject, &ktype_rqbalance,
				"rqbalance");

	if (err)
		kfree(rqbalance_kobject);

	return err;
}

static void rqbalance_kickstart(void)
{
	struct cpufreq_freqs initial_freq;

	/*FIXME: Kick start the state machine by faking a freq notification*/
	initial_freq.new = cpufreq_get(0);
	if (initial_freq.new != 0)
		balanced_cpufreq_transition(NULL, CPUFREQ_RESUMECHANGE,
						&initial_freq);
}

int rqbalance_pm_notify(struct notifier_block *notify_block,
				unsigned long mode, void *unused)
{
	switch (mode) {
	case PM_HIBERNATION_PREPARE:
	case PM_SUSPEND_PREPARE:
		rqbalance_state = IDLE;
		cancel_delayed_work_sync(&rqbalance_work);
		break;
	case PM_POST_SUSPEND:
	case PM_POST_HIBERNATION:
	case PM_POST_RESTORE:
		cpuquiet_wake_cpu(1, false);
		cpuquiet_wake_cpu(2, false);
		cpuquiet_wake_cpu(3, false);
		rqbalance_state = UP;
		rqbalance_kickstart();
		break;
	}

	return NOTIFY_OK;
}

/*
 * Retrieves frequency informations from CPUFreq.
 * This is needed for RQBalance's hotplugging decider
 * algorithms.
 *
 * \return Returns success (0) or negative errno.
 */
static int rqbalance_get_package_info(void)
{
	struct cpufreq_frequency_table *table;
	int count, i, prev_cluster, cur_cluster;

	/* Paranoid initialization is needed in some conditions */
	num_of_cores[CLUSTER_LITTLE] = 0;
	num_of_cores[CLUSTER_BIG] = 0;

	for_each_possible_cpu(i) {
		cur_cluster = topology_physical_package_id(i);
		num_of_cores[cur_cluster]++;
		if (cur_cluster == prev_cluster)
			continue;

		prev_cluster = cur_cluster;
		available_clusters++;

		/*
		 * Get CPUFreq frequency table. RQBALANCE only works with
		 * tables composed of at least 4 frequency entries.
		 * This requirement has to be fullfilled for ALL clusters.
		 */
		table = cpufreq_frequency_get_table(i);
		if (!table)
			return -EINVAL;

		for (count = 0;
		     table[count].frequency != CPUFREQ_TABLE_END;
		     count++);

		if (count < 4)
			return -EINVAL;

		idle_top_freq[cur_cluster] =
				table[(count / 2) - 1].frequency;
		idle_bottom_freq[cur_cluster] =
				table[(count / 2) - 2].frequency;
	};

	/* TODO: Check if we are effectively using HMP!!! This is NOT OK!!! */
	soc_is_hmp = (available_clusters > 1) ? true : false;

	return 0;
}

static void rqbalance_stop(void)
{
	/*
	   first unregister the notifiers. This ensures the governor state
	   can't be modified by a cpufreq transition
	*/
	cpufreq_unregister_notifier(&balanced_cpufreq_nb,
		CPUFREQ_TRANSITION_NOTIFIER);

	unregister_pm_notifier(&pm_notifier_block);

	/* now we can force the governor to be idle */
	rqbalance_state = IDLE;
	cancel_delayed_work_sync(&rqbalance_work);
	destroy_workqueue(rqbalance_wq);
	del_timer(&load_timer);

	stop_rq_work();
	kfree(rq_data);

	kobject_put(rqbalance_kobject);
}

static int rqbalance_start(void)
{
	int err, i, max_cpu_id = 0;

	err = rqbalance_sysfs();
	if (err)
		return err;

	rqbalance_wq = alloc_workqueue("cpuquiet-rqbalance",
			WQ_UNBOUND | WQ_FREEZABLE, 1);
	if (!rqbalance_wq)
		return -ENOMEM;

	INIT_DELAYED_WORK(&rqbalance_work, rqbalance_work_func);

	up_delay = msecs_to_jiffies(UPCORE_DELAY_MS);
	down_delay = msecs_to_jiffies(DNCORE_DELAY_MS);

	err = rqbalance_get_package_info();
	if (err)
		return err;

	for_each_possible_cpu(i)
		max_cpu_id++;

	/* Set value as target MAX on-line number of CPUs */
	if (nr_run_thresholds[max_cpu_id] != UINT_MAX)
		nr_run_thresholds[max_cpu_id] = UINT_MAX;

	/* HACK: Adjust dual-core thresholds for non-HMP SoCs */
	if (!soc_is_hmp) {
		nr_run_thresholds[0] = 250;
		nr_down_run_thresholds[1] = 110;
	}

	cpufreq_register_notifier(&balanced_cpufreq_nb,
		CPUFREQ_TRANSITION_NOTIFIER);

	init_timer(&load_timer);
	load_timer.function = calculate_load_timer;

	rqbalance_kickstart();

	rq_data = kzalloc(sizeof(struct runqueue_data), GFP_KERNEL);
	if (rq_data == NULL) {
		pr_err("%s cannot allocate memory\n", __func__);
		return -ENOMEM;
	}
	spin_lock_init(&rq_data->lock);
	rq_data->nr_run_sample_head = (uint8_t)-1;
	rq_data->nr_run_sample_tail = (uint8_t)-1;
	rq_data->nr_run_sample_count = 0;
	rq_data->update_rate = (RQ_AVG_TIMER_RATE_NS / 1000000);
	INIT_DEFERRABLE_WORK(&rq_data->work, rq_work_fn);

	start_rq_work();

	pm_notifier_block.notifier_call = rqbalance_pm_notify;
	register_pm_notifier(&pm_notifier_block);

	return 0;
}

struct cpuquiet_governor rqbalance_governor = {
	.name		= "rqbalance",
	.start		= rqbalance_start,
	.stop		= rqbalance_stop,
	.owner		= THIS_MODULE,
};

static int __init init_rqbalance(void)
{
	return cpuquiet_register_governor(&rqbalance_governor);
}

static void __exit exit_rqbalance(void)
{
	cpuquiet_unregister_governor(&rqbalance_governor);
}

MODULE_LICENSE("GPL");
#ifdef CONFIG_CPU_QUIET_DEFAULT_GOV_RQBALANCE
fs_initcall(init_rqbalance);
#else
module_init(init_rqbalance);
#endif
module_exit(exit_rqbalance);
