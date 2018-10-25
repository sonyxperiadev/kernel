/*
 * Copyright (c) 2016-2018, The Linux Foundation. All rights reserved.
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
 *
 * Window Assisted Load Tracking (WALT) implementation credits:
 * Srivatsa Vaddagiri, Steve Muckle, Syed Rameez Mustafa, Joonwoo Park,
 * Pavan Kumar Kondeti, Olav Haugan
 *
 * 2016-03-06: Integration with EAS/refactoring by Vikram Mulukutla
 *             and Todd Kjos
 */

#include <linux/syscore_ops.h>
#include <linux/cpufreq.h>
#include <linux/list_sort.h>
#include <linux/jiffies.h>
#include <linux/sched/core_ctl.h>
#include <trace/events/sched.h>
#include "sched.h"
#include "walt.h"

#include <trace/events/sched.h>

const char *task_event_names[] = {"PUT_PREV_TASK", "PICK_NEXT_TASK",
				  "TASK_WAKE", "TASK_MIGRATE", "TASK_UPDATE",
				"IRQ_UPDATE"};

const char *migrate_type_names[] = {"GROUP_TO_RQ", "RQ_TO_GROUP",
					 "RQ_TO_RQ", "GROUP_TO_GROUP"};

#define SCHED_FREQ_ACCOUNT_WAIT_TIME 0
#define SCHED_ACCOUNT_WAIT_TIME 1

#define EARLY_DETECTION_DURATION 9500000

static ktime_t ktime_last;
static bool sched_ktime_suspended;
static struct cpu_cycle_counter_cb cpu_cycle_counter_cb;
static bool use_cycle_counter;
DEFINE_MUTEX(cluster_lock);
static atomic64_t walt_irq_work_lastq_ws;
u64 walt_load_reported_window;

static struct irq_work walt_cpufreq_irq_work;
static struct irq_work walt_migration_irq_work;

u64 sched_ktime_clock(void)
{
	if (unlikely(sched_ktime_suspended))
		return ktime_to_ns(ktime_last);
	return ktime_get_ns();
}

static void sched_resume(void)
{
	sched_ktime_suspended = false;
}

static int sched_suspend(void)
{
	ktime_last = ktime_get();
	sched_ktime_suspended = true;
	return 0;
}

static struct syscore_ops sched_syscore_ops = {
	.resume	= sched_resume,
	.suspend = sched_suspend
};

static int __init sched_init_ops(void)
{
	register_syscore_ops(&sched_syscore_ops);
	return 0;
}
late_initcall(sched_init_ops);

static void acquire_rq_locks_irqsave(const cpumask_t *cpus,
				     unsigned long *flags)
{
	int cpu;

	local_irq_save(*flags);
	for_each_cpu(cpu, cpus)
		raw_spin_lock(&cpu_rq(cpu)->lock);
}

static void release_rq_locks_irqrestore(const cpumask_t *cpus,
					unsigned long *flags)
{
	int cpu;

	for_each_cpu(cpu, cpus)
		raw_spin_unlock(&cpu_rq(cpu)->lock);
	local_irq_restore(*flags);
}

#ifdef CONFIG_HZ_300
/*
 * Tick interval becomes to 3333333 due to
 * rounding error when HZ=300.
 */
#define MIN_SCHED_RAVG_WINDOW (3333333 * 6)
#else
/* Min window size (in ns) = 20ms */
#define MIN_SCHED_RAVG_WINDOW 20000000
#endif

/* Max window size (in ns) = 1s */
#define MAX_SCHED_RAVG_WINDOW 1000000000

/* 1 -> use PELT based load stats, 0 -> use window-based load stats */
unsigned int __read_mostly walt_disabled = 0;

__read_mostly unsigned int sysctl_sched_cpu_high_irqload = (10 * NSEC_PER_MSEC);

unsigned int sysctl_sched_walt_rotate_big_tasks;
unsigned int walt_rotation_enabled;

/*
 * sched_window_stats_policy and sched_ravg_hist_size have a 'sysctl' copy
 * associated with them. This is required for atomic update of those variables
 * when being modifed via sysctl interface.
 *
 * IMPORTANT: Initialize both copies to same value!!
 */

__read_mostly unsigned int sched_ravg_hist_size = 5;
__read_mostly unsigned int sysctl_sched_ravg_hist_size = 5;

static __read_mostly unsigned int sched_io_is_busy = 1;

__read_mostly unsigned int sched_window_stats_policy =
	WINDOW_STATS_MAX_RECENT_AVG;
__read_mostly unsigned int sysctl_sched_window_stats_policy =
	WINDOW_STATS_MAX_RECENT_AVG;

/* Window size (in ns) */
__read_mostly unsigned int sched_ravg_window = MIN_SCHED_RAVG_WINDOW;

/*
 * A after-boot constant divisor for cpu_util_freq_walt() to apply the load
 * boost.
 */
__read_mostly unsigned int walt_cpu_util_freq_divisor;

/* Initial task load. Newly created tasks are assigned this load. */
unsigned int __read_mostly sysctl_sched_init_task_load_pct = 15;

/*
 * Maximum possible frequency across all cpus. Task demand and cpu
 * capacity (cpu_power) metrics are scaled in reference to it.
 */
unsigned int max_possible_freq = 1;

/*
 * Minimum possible max_freq across all cpus. This will be same as
 * max_possible_freq on homogeneous systems and could be different from
 * max_possible_freq on heterogenous systems. min_max_freq is used to derive
 * capacity (cpu_power) of cpus.
 */
unsigned int min_max_freq = 1;

unsigned int max_capacity = 1024; /* max(rq->capacity) */
unsigned int min_capacity = 1024; /* min(rq->capacity) */
unsigned int max_possible_capacity = 1024; /* max(rq->max_possible_capacity) */
unsigned int
min_max_possible_capacity = 1024; /* min(rq->max_possible_capacity) */

/* Temporarily disable window-stats activity on all cpus */
unsigned int __read_mostly sched_disable_window_stats;

/*
 * Task load is categorized into buckets for the purpose of top task tracking.
 * The entire range of load from 0 to sched_ravg_window needs to be covered
 * in NUM_LOAD_INDICES number of buckets. Therefore the size of each bucket
 * is given by sched_ravg_window / NUM_LOAD_INDICES. Since the default value
 * of sched_ravg_window is MIN_SCHED_RAVG_WINDOW, use that to compute
 * sched_load_granule.
 */
__read_mostly unsigned int sched_load_granule =
			MIN_SCHED_RAVG_WINDOW / NUM_LOAD_INDICES;
/* Size of bitmaps maintained to track top tasks */
static const unsigned int top_tasks_bitmap_size =
		BITS_TO_LONGS(NUM_LOAD_INDICES + 1) * sizeof(unsigned long);

/*
 * This governs what load needs to be used when reporting CPU busy time
 * to the cpufreq governor.
 */
__read_mostly unsigned int sysctl_sched_freq_reporting_policy;

static int __init set_sched_ravg_window(char *str)
{
	unsigned int window_size;

	get_option(&str, &window_size);

	if (window_size < MIN_SCHED_RAVG_WINDOW ||
			window_size > MAX_SCHED_RAVG_WINDOW) {
		WARN_ON(1);
		return -EINVAL;
	}

	sched_ravg_window = window_size;
	return 0;
}

early_param("sched_ravg_window", set_sched_ravg_window);

static int __init set_sched_predl(char *str)
{
	unsigned int predl;

	get_option(&str, &predl);
	sched_predl = !!predl;
	return 0;
}
early_param("sched_predl", set_sched_predl);

void inc_rq_walt_stats(struct rq *rq, struct task_struct *p)
{
	inc_nr_big_task(&rq->walt_stats, p);
	walt_inc_cumulative_runnable_avg(rq, p);
}

void dec_rq_walt_stats(struct rq *rq, struct task_struct *p)
{
	dec_nr_big_task(&rq->walt_stats, p);
	walt_dec_cumulative_runnable_avg(rq, p);
}

void fixup_walt_sched_stats_common(struct rq *rq, struct task_struct *p,
				   u32 new_task_load, u32 new_pred_demand)
{
	s64 task_load_delta = (s64)new_task_load - task_load(p);
	s64 pred_demand_delta = PRED_DEMAND_DELTA;

	fixup_cumulative_runnable_avg(&rq->walt_stats, task_load_delta,
				      pred_demand_delta);

	walt_fixup_cum_window_demand(rq, task_load_delta);
}

/*
 * Demand aggregation for frequency purpose:
 *
 * CPU demand of tasks from various related groups is aggregated per-cluster and
 * added to the "max_busy_cpu" in that cluster, where max_busy_cpu is determined
 * by just rq->prev_runnable_sum.
 *
 * Some examples follow, which assume:
 *	Cluster0 = CPU0-3, Cluster1 = CPU4-7
 *	One related thread group A that has tasks A0, A1, A2
 *
 *	A->cpu_time[X].curr/prev_sum = counters in which cpu execution stats of
 *	tasks belonging to group A are accumulated when they run on cpu X.
 *
 *	CX->curr/prev_sum = counters in which cpu execution stats of all tasks
 *	not belonging to group A are accumulated when they run on cpu X
 *
 * Lets say the stats for window M was as below:
 *
 *	C0->prev_sum = 1ms, A->cpu_time[0].prev_sum = 5ms
 *		Task A0 ran 5ms on CPU0
 *		Task B0 ran 1ms on CPU0
 *
 *	C1->prev_sum = 5ms, A->cpu_time[1].prev_sum = 6ms
 *		Task A1 ran 4ms on CPU1
 *		Task A2 ran 2ms on CPU1
 *		Task B1 ran 5ms on CPU1
 *
 *	C2->prev_sum = 0ms, A->cpu_time[2].prev_sum = 0
 *		CPU2 idle
 *
 *	C3->prev_sum = 0ms, A->cpu_time[3].prev_sum = 0
 *		CPU3 idle
 *
 * In this case, CPU1 was most busy going by just its prev_sum counter. Demand
 * from all group A tasks are added to CPU1. IOW, at end of window M, cpu busy
 * time reported to governor will be:
 *
 *
 *	C0 busy time = 1ms
 *	C1 busy time = 5 + 5 + 6 = 16ms
 *
 */
__read_mostly int sched_freq_aggregate_threshold;

static u64
update_window_start(struct rq *rq, u64 wallclock, int event)
{
	s64 delta;
	int nr_windows;
	u64 old_window_start = rq->window_start;

	delta = wallclock - rq->window_start;
	BUG_ON(delta < 0);
	if (delta < sched_ravg_window)
		return old_window_start;

	nr_windows = div64_u64(delta, sched_ravg_window);
	rq->window_start += (u64)nr_windows * (u64)sched_ravg_window;

	rq->cum_window_demand = rq->walt_stats.cumulative_runnable_avg;

	return old_window_start;
}

int register_cpu_cycle_counter_cb(struct cpu_cycle_counter_cb *cb)
{
	mutex_lock(&cluster_lock);
	if (!cb->get_cpu_cycle_counter) {
		mutex_unlock(&cluster_lock);
		return -EINVAL;
	}

	cpu_cycle_counter_cb = *cb;
	use_cycle_counter = true;
	mutex_unlock(&cluster_lock);

	return 0;
}

/*
 * Assumes rq_lock is held and wallclock was recorded in the same critical
 * section as this function's invocation.
 */
static inline u64 read_cycle_counter(int cpu, u64 wallclock)
{
	struct rq *rq = cpu_rq(cpu);

	if (rq->last_cc_update != wallclock) {
		rq->cycles = cpu_cycle_counter_cb.get_cpu_cycle_counter(cpu);
		rq->last_cc_update = wallclock;
	}

	return rq->cycles;
}

static void update_task_cpu_cycles(struct task_struct *p, int cpu,
				   u64 wallclock)
{
	if (use_cycle_counter)
		p->cpu_cycles = read_cycle_counter(cpu, wallclock);
}

void clear_ed_task(struct task_struct *p, struct rq *rq)
{
	if (p == rq->ed_task)
		rq->ed_task = NULL;
}

bool early_detection_notify(struct rq *rq, u64 wallclock)
{
	struct task_struct *p;
	int loop_max = 10;

	if ((!walt_rotation_enabled && sched_boost_policy() ==
			SCHED_BOOST_NONE) || !rq->cfs.h_nr_running)
		return 0;

	rq->ed_task = NULL;
	list_for_each_entry(p, &rq->cfs_tasks, se.group_node) {
		if (!loop_max)
			break;

		if (wallclock - p->last_wake_ts >= EARLY_DETECTION_DURATION) {
			rq->ed_task = p;
			return 1;
		}

		loop_max--;
	}

	return 0;
}

void sched_account_irqstart(int cpu, struct task_struct *curr, u64 wallclock)
{
	struct rq *rq = cpu_rq(cpu);

	if (!rq->window_start || sched_disable_window_stats)
		return;

	/*
	 * We don’t have to note down an irqstart event when cycle
	 * counter is not used.
	 */
	if (!use_cycle_counter)
		return;

	if (is_idle_task(curr)) {
		/* We're here without rq->lock held, IRQ disabled */
		raw_spin_lock(&rq->lock);
		update_task_cpu_cycles(curr, cpu, sched_ktime_clock());
		raw_spin_unlock(&rq->lock);
	}
}

/*
 * Return total number of tasks "eligible" to run on highest capacity cpu
 *
 * This is simply nr_big_tasks for cpus which are not of max_capacity and
 * nr_running for cpus of max_capacity
 */
unsigned int nr_eligible_big_tasks(int cpu)
{
	struct rq *rq = cpu_rq(cpu);

	if (!is_max_capacity_cpu(cpu))
		return rq->walt_stats.nr_big_tasks;

	return rq->nr_running;
}

void clear_walt_request(int cpu)
{
	struct rq *rq = cpu_rq(cpu);
	unsigned long flags;

	clear_boost_kick(cpu);
	clear_reserved(cpu);
	if (rq->push_task) {
		struct task_struct *push_task = NULL;

		raw_spin_lock_irqsave(&rq->lock, flags);
		if (rq->push_task) {
			clear_reserved(rq->push_cpu);
			push_task = rq->push_task;
			rq->push_task = NULL;
		}
		rq->active_balance = 0;
		raw_spin_unlock_irqrestore(&rq->lock, flags);
		if (push_task)
			put_task_struct(push_task);
	}
}

void sched_account_irqtime(int cpu, struct task_struct *curr,
				 u64 delta, u64 wallclock)
{
	struct rq *rq = cpu_rq(cpu);
	unsigned long flags, nr_windows;
	u64 cur_jiffies_ts;

	raw_spin_lock_irqsave(&rq->lock, flags);

	/*
	 * cputime (wallclock) uses sched_clock so use the same here for
	 * consistency.
	 */
	delta += sched_clock() - wallclock;
	cur_jiffies_ts = get_jiffies_64();

	if (is_idle_task(curr))
		update_task_ravg(curr, rq, IRQ_UPDATE, sched_ktime_clock(),
				 delta);

	nr_windows = cur_jiffies_ts - rq->irqload_ts;

	if (nr_windows) {
		if (nr_windows < 10) {
			/* Decay CPU's irqload by 3/4 for each window. */
			rq->avg_irqload *= (3 * nr_windows);
			rq->avg_irqload = div64_u64(rq->avg_irqload,
						    4 * nr_windows);
		} else {
			rq->avg_irqload = 0;
		}
		rq->avg_irqload += rq->cur_irqload;
		rq->cur_irqload = 0;
	}

	rq->cur_irqload += delta;
	rq->irqload_ts = cur_jiffies_ts;
	raw_spin_unlock_irqrestore(&rq->lock, flags);
}

/*
 * Special case the last index and provide a fast path for index = 0.
 * Note that sched_load_granule can change underneath us if we are not
 * holding any runqueue locks while calling the two functions below.
 */
static u32  top_task_load(struct rq *rq)
{
	int index = rq->prev_top;
	u8 prev = 1 - rq->curr_table;

	if (!index) {
		int msb = NUM_LOAD_INDICES - 1;

		if (!test_bit(msb, rq->top_tasks_bitmap[prev]))
			return 0;
		else
			return sched_load_granule;
	} else if (index == NUM_LOAD_INDICES - 1) {
		return sched_ravg_window;
	} else {
		return (index + 1) * sched_load_granule;
	}
}

u64 freq_policy_load(struct rq *rq)
{
	unsigned int reporting_policy = sysctl_sched_freq_reporting_policy;
	int freq_aggr_thresh = sched_freq_aggregate_threshold;
	struct sched_cluster *cluster = rq->cluster;
	u64 aggr_grp_load = cluster->aggr_grp_load;
	u64 load, tt_load = 0;

	if (rq->ed_task != NULL) {
		load = sched_ravg_window;
		goto done;
	}

	if (aggr_grp_load > freq_aggr_thresh)
		load = rq->prev_runnable_sum + aggr_grp_load;
	else
		load = rq->prev_runnable_sum + rq->grp_time.prev_runnable_sum;

	tt_load = top_task_load(rq);
	switch (reporting_policy) {
	case FREQ_REPORT_MAX_CPU_LOAD_TOP_TASK:
		load = max_t(u64, load, tt_load);
		break;
	case FREQ_REPORT_TOP_TASK:
		load = tt_load;
		break;
	case FREQ_REPORT_CPU_LOAD:
		break;
	default:
		break;
	}

done:
	trace_sched_load_to_gov(rq, aggr_grp_load, tt_load, freq_aggr_thresh,
				load, reporting_policy, walt_rotation_enabled);
	return load;
}

/*
 * In this function we match the accumulated subtractions with the current
 * and previous windows we are operating with. Ignore any entries where
 * the window start in the load_subtraction struct does not match either
 * the curent or the previous window. This could happen whenever CPUs
 * become idle or busy with interrupts disabled for an extended period.
 */
static inline void account_load_subtractions(struct rq *rq)
{
	u64 ws = rq->window_start;
	u64 prev_ws = ws - sched_ravg_window;
	struct load_subtractions *ls = rq->load_subs;
	int i;

	for (i = 0; i < NUM_TRACKED_WINDOWS; i++) {
		if (ls[i].window_start == ws) {
			rq->curr_runnable_sum -= ls[i].subs;
			rq->nt_curr_runnable_sum -= ls[i].new_subs;
		} else if (ls[i].window_start == prev_ws) {
			rq->prev_runnable_sum -= ls[i].subs;
			rq->nt_prev_runnable_sum -= ls[i].new_subs;
		}

		ls[i].subs = 0;
		ls[i].new_subs = 0;
	}

	BUG_ON((s64)rq->prev_runnable_sum < 0);
	BUG_ON((s64)rq->curr_runnable_sum < 0);
	BUG_ON((s64)rq->nt_prev_runnable_sum < 0);
	BUG_ON((s64)rq->nt_curr_runnable_sum < 0);
}

static inline void create_subtraction_entry(struct rq *rq, u64 ws, int index)
{
	rq->load_subs[index].window_start = ws;
	rq->load_subs[index].subs = 0;
	rq->load_subs[index].new_subs = 0;
}

static int get_top_index(unsigned long *bitmap, unsigned long old_top)
{
	int index = find_next_bit(bitmap, NUM_LOAD_INDICES, old_top);

	if (index == NUM_LOAD_INDICES)
		return 0;

	return NUM_LOAD_INDICES - 1 - index;
}

static bool get_subtraction_index(struct rq *rq, u64 ws)
{
	int i;
	u64 oldest = ULLONG_MAX;
	int oldest_index = 0;

	for (i = 0; i < NUM_TRACKED_WINDOWS; i++) {
		u64 entry_ws = rq->load_subs[i].window_start;

		if (ws == entry_ws)
			return i;

		if (entry_ws < oldest) {
			oldest = entry_ws;
			oldest_index = i;
		}
	}

	create_subtraction_entry(rq, ws, oldest_index);
	return oldest_index;
}

static void update_rq_load_subtractions(int index, struct rq *rq,
					u32 sub_load, bool new_task)
{
	rq->load_subs[index].subs +=  sub_load;
	if (new_task)
		rq->load_subs[index].new_subs += sub_load;
}

void update_cluster_load_subtractions(struct task_struct *p,
					int cpu, u64 ws, bool new_task)
{
	struct sched_cluster *cluster = cpu_cluster(cpu);
	struct cpumask cluster_cpus = cluster->cpus;
	u64 prev_ws = ws - sched_ravg_window;
	int i;

	cpumask_clear_cpu(cpu, &cluster_cpus);
	raw_spin_lock(&cluster->load_lock);

	for_each_cpu(i, &cluster_cpus) {
		struct rq *rq = cpu_rq(i);
		int index;

		if (p->ravg.curr_window_cpu[i]) {
			index = get_subtraction_index(rq, ws);
			update_rq_load_subtractions(index, rq,
				p->ravg.curr_window_cpu[i], new_task);
			p->ravg.curr_window_cpu[i] = 0;
		}

		if (p->ravg.prev_window_cpu[i]) {
			index = get_subtraction_index(rq, prev_ws);
			update_rq_load_subtractions(index, rq,
				p->ravg.prev_window_cpu[i], new_task);
			p->ravg.prev_window_cpu[i] = 0;
		}
	}

	raw_spin_unlock(&cluster->load_lock);
}

static inline void inter_cluster_migration_fixup
	(struct task_struct *p, int new_cpu, int task_cpu, bool new_task)
{
	struct rq *dest_rq = cpu_rq(new_cpu);
	struct rq *src_rq = cpu_rq(task_cpu);

	if (same_freq_domain(new_cpu, task_cpu))
		return;

	p->ravg.curr_window_cpu[new_cpu] = p->ravg.curr_window;
	p->ravg.prev_window_cpu[new_cpu] = p->ravg.prev_window;

	dest_rq->curr_runnable_sum += p->ravg.curr_window;
	dest_rq->prev_runnable_sum += p->ravg.prev_window;

	src_rq->curr_runnable_sum -=  p->ravg.curr_window_cpu[task_cpu];
	src_rq->prev_runnable_sum -=  p->ravg.prev_window_cpu[task_cpu];

	if (new_task) {
		dest_rq->nt_curr_runnable_sum += p->ravg.curr_window;
		dest_rq->nt_prev_runnable_sum += p->ravg.prev_window;

		src_rq->nt_curr_runnable_sum -=
				p->ravg.curr_window_cpu[task_cpu];
		src_rq->nt_prev_runnable_sum -=
				p->ravg.prev_window_cpu[task_cpu];
	}

	p->ravg.curr_window_cpu[task_cpu] = 0;
	p->ravg.prev_window_cpu[task_cpu] = 0;

	update_cluster_load_subtractions(p, task_cpu,
			src_rq->window_start, new_task);

	BUG_ON((s64)src_rq->prev_runnable_sum < 0);
	BUG_ON((s64)src_rq->curr_runnable_sum < 0);
	BUG_ON((s64)src_rq->nt_prev_runnable_sum < 0);
	BUG_ON((s64)src_rq->nt_curr_runnable_sum < 0);
}

static int load_to_index(u32 load)
{
	if (load < sched_load_granule)
		return 0;
	else if (load >= sched_ravg_window)
		return NUM_LOAD_INDICES - 1;
	else
		return load / sched_load_granule;
}

static void
migrate_top_tasks(struct task_struct *p, struct rq *src_rq, struct rq *dst_rq)
{
	int index;
	int top_index;
	u32 curr_window = p->ravg.curr_window;
	u32 prev_window = p->ravg.prev_window;
	u8 src = src_rq->curr_table;
	u8 dst = dst_rq->curr_table;
	u8 *src_table;
	u8 *dst_table;

	if (curr_window) {
		src_table = src_rq->top_tasks[src];
		dst_table = dst_rq->top_tasks[dst];
		index = load_to_index(curr_window);
		src_table[index] -= 1;
		dst_table[index] += 1;

		if (!src_table[index])
			__clear_bit(NUM_LOAD_INDICES - index - 1,
				src_rq->top_tasks_bitmap[src]);

		if (dst_table[index] == 1)
			__set_bit(NUM_LOAD_INDICES - index - 1,
				dst_rq->top_tasks_bitmap[dst]);

		if (index > dst_rq->curr_top)
			dst_rq->curr_top = index;

		top_index = src_rq->curr_top;
		if (index == top_index && !src_table[index])
			src_rq->curr_top = get_top_index(
				src_rq->top_tasks_bitmap[src], top_index);
	}

	if (prev_window) {
		src = 1 - src;
		dst = 1 - dst;
		src_table = src_rq->top_tasks[src];
		dst_table = dst_rq->top_tasks[dst];
		index = load_to_index(prev_window);
		src_table[index] -= 1;
		dst_table[index] += 1;

		if (!src_table[index])
			__clear_bit(NUM_LOAD_INDICES - index - 1,
				src_rq->top_tasks_bitmap[src]);

		if (dst_table[index] == 1)
			__set_bit(NUM_LOAD_INDICES - index - 1,
				dst_rq->top_tasks_bitmap[dst]);

		if (index > dst_rq->prev_top)
			dst_rq->prev_top = index;

		top_index = src_rq->prev_top;
		if (index == top_index && !src_table[index])
			src_rq->prev_top = get_top_index(
				src_rq->top_tasks_bitmap[src], top_index);
	}
}

void fixup_busy_time(struct task_struct *p, int new_cpu)
{
	struct rq *src_rq = task_rq(p);
	struct rq *dest_rq = cpu_rq(new_cpu);
	u64 wallclock;
	u64 *src_curr_runnable_sum, *dst_curr_runnable_sum;
	u64 *src_prev_runnable_sum, *dst_prev_runnable_sum;
	u64 *src_nt_curr_runnable_sum, *dst_nt_curr_runnable_sum;
	u64 *src_nt_prev_runnable_sum, *dst_nt_prev_runnable_sum;
	bool new_task;
	struct related_thread_group *grp;

	if (!p->on_rq && p->state != TASK_WAKING)
		return;

	if (exiting_task(p)) {
		clear_ed_task(p, src_rq);
		return;
	}

	if (p->state == TASK_WAKING)
		double_rq_lock(src_rq, dest_rq);

	if (sched_disable_window_stats)
		goto done;

	wallclock = sched_ktime_clock();

	update_task_ravg(task_rq(p)->curr, task_rq(p),
			 TASK_UPDATE,
			 wallclock, 0);
	update_task_ravg(dest_rq->curr, dest_rq,
			 TASK_UPDATE, wallclock, 0);

	update_task_ravg(p, task_rq(p), TASK_MIGRATE,
			 wallclock, 0);

	update_task_cpu_cycles(p, new_cpu, wallclock);

	/*
	 * When a task is migrating during the wakeup, adjust
	 * the task's contribution towards cumulative window
	 * demand.
	 */
	if (p->state == TASK_WAKING && p->last_sleep_ts >=
				       src_rq->window_start) {
		walt_fixup_cum_window_demand(src_rq, -(s64)p->ravg.demand);
		walt_fixup_cum_window_demand(dest_rq, p->ravg.demand);
	}

	new_task = is_new_task(p);
	/* Protected by rq_lock */
	grp = p->grp;

	/*
	 * For frequency aggregation, we continue to do migration fixups
	 * even for intra cluster migrations. This is because, the aggregated
	 * load has to reported on a single CPU regardless.
	 */
	if (grp) {
		struct group_cpu_time *cpu_time;

		cpu_time = &src_rq->grp_time;
		src_curr_runnable_sum = &cpu_time->curr_runnable_sum;
		src_prev_runnable_sum = &cpu_time->prev_runnable_sum;
		src_nt_curr_runnable_sum = &cpu_time->nt_curr_runnable_sum;
		src_nt_prev_runnable_sum = &cpu_time->nt_prev_runnable_sum;

		cpu_time = &dest_rq->grp_time;
		dst_curr_runnable_sum = &cpu_time->curr_runnable_sum;
		dst_prev_runnable_sum = &cpu_time->prev_runnable_sum;
		dst_nt_curr_runnable_sum = &cpu_time->nt_curr_runnable_sum;
		dst_nt_prev_runnable_sum = &cpu_time->nt_prev_runnable_sum;

		if (p->ravg.curr_window) {
			*src_curr_runnable_sum -= p->ravg.curr_window;
			*dst_curr_runnable_sum += p->ravg.curr_window;
			if (new_task) {
				*src_nt_curr_runnable_sum -=
							p->ravg.curr_window;
				*dst_nt_curr_runnable_sum +=
							p->ravg.curr_window;
			}
		}

		if (p->ravg.prev_window) {
			*src_prev_runnable_sum -= p->ravg.prev_window;
			*dst_prev_runnable_sum += p->ravg.prev_window;
			if (new_task) {
				*src_nt_prev_runnable_sum -=
							p->ravg.prev_window;
				*dst_nt_prev_runnable_sum +=
							p->ravg.prev_window;
			}
		}
	} else {
		inter_cluster_migration_fixup(p, new_cpu,
						task_cpu(p), new_task);
	}

	migrate_top_tasks(p, src_rq, dest_rq);

	if (!same_freq_domain(new_cpu, task_cpu(p))) {
		src_rq->notif_pending = true;
		dest_rq->notif_pending = true;
		irq_work_queue(&walt_migration_irq_work);
	}

	if (p == src_rq->ed_task) {
		src_rq->ed_task = NULL;
		dest_rq->ed_task = p;
	}

done:
	if (p->state == TASK_WAKING)
		double_rq_unlock(src_rq, dest_rq);
}

void set_window_start(struct rq *rq)
{
	static int sync_cpu_available;

	if (likely(rq->window_start))
		return;

	if (!sync_cpu_available) {
		rq->window_start = 1;
		sync_cpu_available = 1;
		atomic64_set(&walt_irq_work_lastq_ws, rq->window_start);
		walt_load_reported_window =
					atomic64_read(&walt_irq_work_lastq_ws);

	} else {
		struct rq *sync_rq = cpu_rq(cpumask_any(cpu_online_mask));

		raw_spin_unlock(&rq->lock);
		double_rq_lock(rq, sync_rq);
		rq->window_start = sync_rq->window_start;
		rq->curr_runnable_sum = rq->prev_runnable_sum = 0;
		rq->nt_curr_runnable_sum = rq->nt_prev_runnable_sum = 0;
		raw_spin_unlock(&sync_rq->lock);
	}

	rq->curr->ravg.mark_start = rq->window_start;
}

unsigned int max_possible_efficiency = 1;
unsigned int min_possible_efficiency = UINT_MAX;

#define INC_STEP 8
#define DEC_STEP 2
#define CONSISTENT_THRES 16
#define INC_STEP_BIG 16
/*
 * bucket_increase - update the count of all buckets
 *
 * @buckets: array of buckets tracking busy time of a task
 * @idx: the index of bucket to be incremented
 *
 * Each time a complete window finishes, count of bucket that runtime
 * falls in (@idx) is incremented. Counts of all other buckets are
 * decayed. The rate of increase and decay could be different based
 * on current count in the bucket.
 */
static inline void bucket_increase(u8 *buckets, int idx)
{
	int i, step;

	for (i = 0; i < NUM_BUSY_BUCKETS; i++) {
		if (idx != i) {
			if (buckets[i] > DEC_STEP)
				buckets[i] -= DEC_STEP;
			else
				buckets[i] = 0;
		} else {
			step = buckets[i] >= CONSISTENT_THRES ?
						INC_STEP_BIG : INC_STEP;
			if (buckets[i] > U8_MAX - step)
				buckets[i] = U8_MAX;
			else
				buckets[i] += step;
		}
	}
}

static inline int busy_to_bucket(u32 normalized_rt)
{
	int bidx;

	bidx = mult_frac(normalized_rt, NUM_BUSY_BUCKETS, max_task_load());
	bidx = min(bidx, NUM_BUSY_BUCKETS - 1);

	/*
	 * Combine lowest two buckets. The lowest frequency falls into
	 * 2nd bucket and thus keep predicting lowest bucket is not
	 * useful.
	 */
	if (!bidx)
		bidx++;

	return bidx;
}

/*
 * get_pred_busy - calculate predicted demand for a task on runqueue
 *
 * @rq: runqueue of task p
 * @p: task whose prediction is being updated
 * @start: starting bucket. returned prediction should not be lower than
 *         this bucket.
 * @runtime: runtime of the task. returned prediction should not be lower
 *           than this runtime.
 * Note: @start can be derived from @runtime. It's passed in only to
 * avoid duplicated calculation in some cases.
 *
 * A new predicted busy time is returned for task @p based on @runtime
 * passed in. The function searches through buckets that represent busy
 * time equal to or bigger than @runtime and attempts to find the bucket to
 * to use for prediction. Once found, it searches through historical busy
 * time and returns the latest that falls into the bucket. If no such busy
 * time exists, it returns the medium of that bucket.
 */
static u32 get_pred_busy(struct rq *rq, struct task_struct *p,
				int start, u32 runtime)
{
	int i;
	u8 *buckets = p->ravg.busy_buckets;
	u32 *hist = p->ravg.sum_history;
	u32 dmin, dmax;
	u64 cur_freq_runtime = 0;
	int first = NUM_BUSY_BUCKETS, final;
	u32 ret = runtime;

	/* skip prediction for new tasks due to lack of history */
	if (unlikely(is_new_task(p)))
		goto out;

	/* find minimal bucket index to pick */
	for (i = start; i < NUM_BUSY_BUCKETS; i++) {
		if (buckets[i]) {
			first = i;
			break;
		}
	}
	/* if no higher buckets are filled, predict runtime */
	if (first >= NUM_BUSY_BUCKETS)
		goto out;

	/* compute the bucket for prediction */
	final = first;

	/* determine demand range for the predicted bucket */
	if (final < 2) {
		/* lowest two buckets are combined */
		dmin = 0;
		final = 1;
	} else {
		dmin = mult_frac(final, max_task_load(), NUM_BUSY_BUCKETS);
	}
	dmax = mult_frac(final + 1, max_task_load(), NUM_BUSY_BUCKETS);

	/*
	 * search through runtime history and return first runtime that falls
	 * into the range of predicted bucket.
	 */
	for (i = 0; i < sched_ravg_hist_size; i++) {
		if (hist[i] >= dmin && hist[i] < dmax) {
			ret = hist[i];
			break;
		}
	}
	/* no historical runtime within bucket found, use average of the bin */
	if (ret < dmin)
		ret = (dmin + dmax) / 2;
	/*
	 * when updating in middle of a window, runtime could be higher
	 * than all recorded history. Always predict at least runtime.
	 */
	ret = max(runtime, ret);
out:
	trace_sched_update_pred_demand(rq, p, runtime,
		mult_frac((unsigned int)cur_freq_runtime, 100,
			  sched_ravg_window), ret);
	return ret;
}

static inline u32 calc_pred_demand(struct rq *rq, struct task_struct *p)
{
	if (p->ravg.pred_demand >= p->ravg.curr_window)
		return p->ravg.pred_demand;

	return get_pred_busy(rq, p, busy_to_bucket(p->ravg.curr_window),
			     p->ravg.curr_window);
}

/*
 * predictive demand of a task is calculated at the window roll-over.
 * if the task current window busy time exceeds the predicted
 * demand, update it here to reflect the task needs.
 */
void update_task_pred_demand(struct rq *rq, struct task_struct *p, int event)
{
	u32 new, old;

	if (!sched_predl)
		return;

	if (is_idle_task(p) || exiting_task(p))
		return;

	if (event != PUT_PREV_TASK && event != TASK_UPDATE &&
			(!SCHED_FREQ_ACCOUNT_WAIT_TIME ||
			 (event != TASK_MIGRATE &&
			 event != PICK_NEXT_TASK)))
		return;

	/*
	 * TASK_UPDATE can be called on sleeping task, when its moved between
	 * related groups
	 */
	if (event == TASK_UPDATE) {
		if (!p->on_rq && !SCHED_FREQ_ACCOUNT_WAIT_TIME)
			return;
	}

	new = calc_pred_demand(rq, p);
	old = p->ravg.pred_demand;

	if (old >= new)
		return;

	if (task_on_rq_queued(p) && (!task_has_dl_policy(p) ||
				!p->dl.dl_throttled))
		p->sched_class->fixup_walt_sched_stats(rq, p,
				p->ravg.demand,
				new);

	p->ravg.pred_demand = new;
}

void clear_top_tasks_bitmap(unsigned long *bitmap)
{
	memset(bitmap, 0, top_tasks_bitmap_size);
	__set_bit(NUM_LOAD_INDICES, bitmap);
}

static void update_top_tasks(struct task_struct *p, struct rq *rq,
		u32 old_curr_window, int new_window, bool full_window)
{
	u8 curr = rq->curr_table;
	u8 prev = 1 - curr;
	u8 *curr_table = rq->top_tasks[curr];
	u8 *prev_table = rq->top_tasks[prev];
	int old_index, new_index, update_index;
	u32 curr_window = p->ravg.curr_window;
	u32 prev_window = p->ravg.prev_window;
	bool zero_index_update;

	if (old_curr_window == curr_window && !new_window)
		return;

	old_index = load_to_index(old_curr_window);
	new_index = load_to_index(curr_window);

	if (!new_window) {
		zero_index_update = !old_curr_window && curr_window;
		if (old_index != new_index || zero_index_update) {
			if (old_curr_window)
				curr_table[old_index] -= 1;
			if (curr_window)
				curr_table[new_index] += 1;
			if (new_index > rq->curr_top)
				rq->curr_top = new_index;
		}

		if (!curr_table[old_index])
			__clear_bit(NUM_LOAD_INDICES - old_index - 1,
				rq->top_tasks_bitmap[curr]);

		if (curr_table[new_index] == 1)
			__set_bit(NUM_LOAD_INDICES - new_index - 1,
				rq->top_tasks_bitmap[curr]);

		return;
	}

	/*
	 * The window has rolled over for this task. By the time we get
	 * here, curr/prev swaps would has already occurred. So we need
	 * to use prev_window for the new index.
	 */
	update_index = load_to_index(prev_window);

	if (full_window) {
		/*
		 * Two cases here. Either 'p' ran for the entire window or
		 * it didn't run at all. In either case there is no entry
		 * in the prev table. If 'p' ran the entire window, we just
		 * need to create a new entry in the prev table. In this case
		 * update_index will be correspond to sched_ravg_window
		 * so we can unconditionally update the top index.
		 */
		if (prev_window) {
			prev_table[update_index] += 1;
			rq->prev_top = update_index;
		}

		if (prev_table[update_index] == 1)
			__set_bit(NUM_LOAD_INDICES - update_index - 1,
				rq->top_tasks_bitmap[prev]);
	} else {
		zero_index_update = !old_curr_window && prev_window;
		if (old_index != update_index || zero_index_update) {
			if (old_curr_window)
				prev_table[old_index] -= 1;

			prev_table[update_index] += 1;

			if (update_index > rq->prev_top)
				rq->prev_top = update_index;

			if (!prev_table[old_index])
				__clear_bit(NUM_LOAD_INDICES - old_index - 1,
						rq->top_tasks_bitmap[prev]);

			if (prev_table[update_index] == 1)
				__set_bit(NUM_LOAD_INDICES - update_index - 1,
						rq->top_tasks_bitmap[prev]);
		}
	}

	if (curr_window) {
		curr_table[new_index] += 1;

		if (new_index > rq->curr_top)
			rq->curr_top = new_index;

		if (curr_table[new_index] == 1)
			__set_bit(NUM_LOAD_INDICES - new_index - 1,
				rq->top_tasks_bitmap[curr]);
	}
}

static void rollover_top_tasks(struct rq *rq, bool full_window)
{
	u8 curr_table = rq->curr_table;
	u8 prev_table = 1 - curr_table;
	int curr_top = rq->curr_top;

	clear_top_tasks_table(rq->top_tasks[prev_table]);
	clear_top_tasks_bitmap(rq->top_tasks_bitmap[prev_table]);

	if (full_window) {
		curr_top = 0;
		clear_top_tasks_table(rq->top_tasks[curr_table]);
		clear_top_tasks_bitmap(
				rq->top_tasks_bitmap[curr_table]);
	}

	rq->curr_table = prev_table;
	rq->prev_top = curr_top;
	rq->curr_top = 0;
}

static u32 empty_windows[NR_CPUS];

static void rollover_task_window(struct task_struct *p, bool full_window)
{
	u32 *curr_cpu_windows = empty_windows;
	u32 curr_window;
	int i;

	/* Rollover the sum */
	curr_window = 0;

	if (!full_window) {
		curr_window = p->ravg.curr_window;
		curr_cpu_windows = p->ravg.curr_window_cpu;
	}

	p->ravg.prev_window = curr_window;
	p->ravg.curr_window = 0;

	/* Roll over individual CPU contributions */
	for (i = 0; i < nr_cpu_ids; i++) {
		p->ravg.prev_window_cpu[i] = curr_cpu_windows[i];
		p->ravg.curr_window_cpu[i] = 0;
	}
}

void sched_set_io_is_busy(int val)
{
	sched_io_is_busy = val;
}

static inline int cpu_is_waiting_on_io(struct rq *rq)
{
	if (!sched_io_is_busy)
		return 0;

	return atomic_read(&rq->nr_iowait);
}

static int account_busy_for_cpu_time(struct rq *rq, struct task_struct *p,
				     u64 irqtime, int event)
{
	if (is_idle_task(p)) {
		/* TASK_WAKE && TASK_MIGRATE is not possible on idle task! */
		if (event == PICK_NEXT_TASK)
			return 0;

		/* PUT_PREV_TASK, TASK_UPDATE && IRQ_UPDATE are left */
		return irqtime || cpu_is_waiting_on_io(rq);
	}

	if (event == TASK_WAKE)
		return 0;

	if (event == PUT_PREV_TASK || event == IRQ_UPDATE)
		return 1;

	/*
	 * TASK_UPDATE can be called on sleeping task, when its moved between
	 * related groups
	 */
	if (event == TASK_UPDATE) {
		if (rq->curr == p)
			return 1;

		return p->on_rq ? SCHED_FREQ_ACCOUNT_WAIT_TIME : 0;
	}

	/* TASK_MIGRATE, PICK_NEXT_TASK left */
	return SCHED_FREQ_ACCOUNT_WAIT_TIME;
}

#define DIV64_U64_ROUNDUP(X, Y) div64_u64((X) + (Y - 1), Y)

static inline u64 scale_exec_time(u64 delta, struct rq *rq)
{
	u32 freq;

	freq = cpu_cycles_to_freq(rq->cc.cycles, rq->cc.time);
	delta = DIV64_U64_ROUNDUP(delta * freq, max_possible_freq);
	delta *= rq->cluster->exec_scale_factor;
	delta >>= 10;

	return delta;
}

/* Convert busy time to frequency equivalent
 * Assumes load is scaled to 1024
 */
static inline unsigned int load_to_freq(struct rq *rq, unsigned int load)
{
	return mult_frac(cpu_max_possible_freq(cpu_of(rq)), load,
			 (unsigned int) capacity_orig_of(cpu_of(rq)));
}

bool do_pl_notif(struct rq *rq)
{
	u64 prev = rq->old_busy_time;
	u64 pl = rq->walt_stats.pred_demands_sum;
	int cpu = cpu_of(rq);

	/* If already at max freq, bail out */
	if (capacity_orig_of(cpu) == capacity_curr_of(cpu))
		return false;

	prev = max(prev, rq->old_estimated_time);

	pl = div64_u64(pl, sched_ravg_window >> SCHED_CAPACITY_SHIFT);

	/* 400 MHz filter. */
	return (pl > prev) && (load_to_freq(rq, pl - prev) > 400000);
}

static void rollover_cpu_window(struct rq *rq, bool full_window)
{
	u64 curr_sum = rq->curr_runnable_sum;
	u64 nt_curr_sum = rq->nt_curr_runnable_sum;
	u64 grp_curr_sum = rq->grp_time.curr_runnable_sum;
	u64 grp_nt_curr_sum = rq->grp_time.nt_curr_runnable_sum;

	if (unlikely(full_window)) {
		curr_sum = 0;
		nt_curr_sum = 0;
		grp_curr_sum = 0;
		grp_nt_curr_sum = 0;
	}

	rq->prev_runnable_sum = curr_sum;
	rq->nt_prev_runnable_sum = nt_curr_sum;
	rq->grp_time.prev_runnable_sum = grp_curr_sum;
	rq->grp_time.nt_prev_runnable_sum = grp_nt_curr_sum;

	rq->curr_runnable_sum = 0;
	rq->nt_curr_runnable_sum = 0;
	rq->grp_time.curr_runnable_sum = 0;
	rq->grp_time.nt_curr_runnable_sum = 0;
}

/*
 * Account cpu activity in its busy time counters (rq->curr/prev_runnable_sum)
 */
static void update_cpu_busy_time(struct task_struct *p, struct rq *rq,
				 int event, u64 wallclock, u64 irqtime)
{
	int new_window, full_window = 0;
	int p_is_curr_task = (p == rq->curr);
	u64 mark_start = p->ravg.mark_start;
	u64 window_start = rq->window_start;
	u32 window_size = sched_ravg_window;
	u64 delta;
	u64 *curr_runnable_sum = &rq->curr_runnable_sum;
	u64 *prev_runnable_sum = &rq->prev_runnable_sum;
	u64 *nt_curr_runnable_sum = &rq->nt_curr_runnable_sum;
	u64 *nt_prev_runnable_sum = &rq->nt_prev_runnable_sum;
	bool new_task;
	struct related_thread_group *grp;
	int cpu = rq->cpu;
	u32 old_curr_window = p->ravg.curr_window;

	new_window = mark_start < window_start;
	if (new_window) {
		full_window = (window_start - mark_start) >= window_size;
		if (p->ravg.active_windows < USHRT_MAX)
			p->ravg.active_windows++;
	}

	new_task = is_new_task(p);

	/*
	 * Handle per-task window rollover. We don't care about the idle
	 * task or exiting tasks.
	 */
	if (!is_idle_task(p) && !exiting_task(p)) {
		if (new_window)
			rollover_task_window(p, full_window);
	}

	if (p_is_curr_task && new_window) {
		rollover_cpu_window(rq, full_window);
		rollover_top_tasks(rq, full_window);
	}

	if (!account_busy_for_cpu_time(rq, p, irqtime, event))
		goto done;

	grp = p->grp;
	if (grp) {
		struct group_cpu_time *cpu_time = &rq->grp_time;

		curr_runnable_sum = &cpu_time->curr_runnable_sum;
		prev_runnable_sum = &cpu_time->prev_runnable_sum;

		nt_curr_runnable_sum = &cpu_time->nt_curr_runnable_sum;
		nt_prev_runnable_sum = &cpu_time->nt_prev_runnable_sum;
	}

	if (!new_window) {
		/*
		 * account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. No rollover
		 * since we didn't start a new window. An example of this is
		 * when a task starts execution and then sleeps within the
		 * same window.
		 */

		if (!irqtime || !is_idle_task(p) || cpu_is_waiting_on_io(rq))
			delta = wallclock - mark_start;
		else
			delta = irqtime;
		delta = scale_exec_time(delta, rq);
		*curr_runnable_sum += delta;
		if (new_task)
			*nt_curr_runnable_sum += delta;

		if (!is_idle_task(p) && !exiting_task(p)) {
			p->ravg.curr_window += delta;
			p->ravg.curr_window_cpu[cpu] += delta;
		}

		goto done;
	}

	if (!p_is_curr_task) {
		/*
		 * account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. A new window
		 * has also started, but p is not the current task, so the
		 * window is not rolled over - just split up and account
		 * as necessary into curr and prev. The window is only
		 * rolled over when a new window is processed for the current
		 * task.
		 *
		 * Irqtime can't be accounted by a task that isn't the
		 * currently running task.
		 */

		if (!full_window) {
			/*
			 * A full window hasn't elapsed, account partial
			 * contribution to previous completed window.
			 */
			delta = scale_exec_time(window_start - mark_start, rq);
			if (!exiting_task(p)) {
				p->ravg.prev_window += delta;
				p->ravg.prev_window_cpu[cpu] += delta;
			}
		} else {
			/*
			 * Since at least one full window has elapsed,
			 * the contribution to the previous window is the
			 * full window (window_size).
			 */
			delta = scale_exec_time(window_size, rq);
			if (!exiting_task(p)) {
				p->ravg.prev_window = delta;
				p->ravg.prev_window_cpu[cpu] = delta;
			}
		}

		*prev_runnable_sum += delta;
		if (new_task)
			*nt_prev_runnable_sum += delta;

		/* Account piece of busy time in the current window. */
		delta = scale_exec_time(wallclock - window_start, rq);
		*curr_runnable_sum += delta;
		if (new_task)
			*nt_curr_runnable_sum += delta;

		if (!exiting_task(p)) {
			p->ravg.curr_window = delta;
			p->ravg.curr_window_cpu[cpu] = delta;
		}

		goto done;
	}

	if (!irqtime || !is_idle_task(p) || cpu_is_waiting_on_io(rq)) {
		/*
		 * account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. A new window
		 * has started and p is the current task so rollover is
		 * needed. If any of these three above conditions are true
		 * then this busy time can't be accounted as irqtime.
		 *
		 * Busy time for the idle task or exiting tasks need not
		 * be accounted.
		 *
		 * An example of this would be a task that starts execution
		 * and then sleeps once a new window has begun.
		 */

		if (!full_window) {
			/*
			 * A full window hasn't elapsed, account partial
			 * contribution to previous completed window.
			 */
			delta = scale_exec_time(window_start - mark_start, rq);
			if (!is_idle_task(p) && !exiting_task(p)) {
				p->ravg.prev_window += delta;
				p->ravg.prev_window_cpu[cpu] += delta;
			}
		} else {
			/*
			 * Since at least one full window has elapsed,
			 * the contribution to the previous window is the
			 * full window (window_size).
			 */
			delta = scale_exec_time(window_size, rq);
			if (!is_idle_task(p) && !exiting_task(p)) {
				p->ravg.prev_window = delta;
				p->ravg.prev_window_cpu[cpu] = delta;
			}
		}

		/*
		 * Rollover is done here by overwriting the values in
		 * prev_runnable_sum and curr_runnable_sum.
		 */
		*prev_runnable_sum += delta;
		if (new_task)
			*nt_prev_runnable_sum += delta;

		/* Account piece of busy time in the current window. */
		delta = scale_exec_time(wallclock - window_start, rq);
		*curr_runnable_sum += delta;
		if (new_task)
			*nt_curr_runnable_sum += delta;

		if (!is_idle_task(p) && !exiting_task(p)) {
			p->ravg.curr_window = delta;
			p->ravg.curr_window_cpu[cpu] = delta;
		}

		goto done;
	}

	if (irqtime) {
		/*
		 * account_busy_for_cpu_time() = 1 so busy time needs
		 * to be accounted to the current window. A new window
		 * has started and p is the current task so rollover is
		 * needed. The current task must be the idle task because
		 * irqtime is not accounted for any other task.
		 *
		 * Irqtime will be accounted each time we process IRQ activity
		 * after a period of idleness, so we know the IRQ busy time
		 * started at wallclock - irqtime.
		 */

		BUG_ON(!is_idle_task(p));
		mark_start = wallclock - irqtime;

		/*
		 * Roll window over. If IRQ busy time was just in the current
		 * window then that is all that need be accounted.
		 */
		if (mark_start > window_start) {
			*curr_runnable_sum = scale_exec_time(irqtime, rq);
			return;
		}

		/*
		 * The IRQ busy time spanned multiple windows. Process the
		 * busy time preceding the current window start first.
		 */
		delta = window_start - mark_start;
		if (delta > window_size)
			delta = window_size;
		delta = scale_exec_time(delta, rq);
		*prev_runnable_sum += delta;

		/* Process the remaining IRQ busy time in the current window. */
		delta = wallclock - window_start;
		rq->curr_runnable_sum = scale_exec_time(delta, rq);

		return;
	}

done:
	if (!is_idle_task(p) && !exiting_task(p))
		update_top_tasks(p, rq, old_curr_window,
					new_window, full_window);
}


static inline u32 predict_and_update_buckets(struct rq *rq,
			struct task_struct *p, u32 runtime) {

	int bidx;
	u32 pred_demand;

	if (!sched_predl)
		return 0;

	bidx = busy_to_bucket(runtime);
	pred_demand = get_pred_busy(rq, p, bidx, runtime);
	bucket_increase(p->ravg.busy_buckets, bidx);

	return pred_demand;
}

static int
account_busy_for_task_demand(struct rq *rq, struct task_struct *p, int event)
{
	/*
	 * No need to bother updating task demand for exiting tasks
	 * or the idle task.
	 */
	if (exiting_task(p) || is_idle_task(p))
		return 0;

	/*
	 * When a task is waking up it is completing a segment of non-busy
	 * time. Likewise, if wait time is not treated as busy time, then
	 * when a task begins to run or is migrated, it is not running and
	 * is completing a segment of non-busy time.
	 */
	if (event == TASK_WAKE || (!SCHED_ACCOUNT_WAIT_TIME &&
			 (event == PICK_NEXT_TASK || event == TASK_MIGRATE)))
		return 0;

	/*
	 * TASK_UPDATE can be called on sleeping task, when its moved between
	 * related groups
	 */
	if (event == TASK_UPDATE) {
		if (rq->curr == p)
			return 1;

		return p->on_rq ? SCHED_ACCOUNT_WAIT_TIME : 0;
	}

	return 1;
}

/*
 * Called when new window is starting for a task, to record cpu usage over
 * recently concluded window(s). Normally 'samples' should be 1. It can be > 1
 * when, say, a real-time task runs without preemption for several windows at a
 * stretch.
 */
static void update_history(struct rq *rq, struct task_struct *p,
			 u32 runtime, int samples, int event)
{
	u32 *hist = &p->ravg.sum_history[0];
	int ridx, widx;
	u32 max = 0, avg, demand, pred_demand;
	u64 sum = 0;
	u64 prev_demand;

	/* Ignore windows where task had no activity */
	if (!runtime || is_idle_task(p) || exiting_task(p) || !samples)
		goto done;

	prev_demand = p->ravg.demand;

	/* Push new 'runtime' value onto stack */
	widx = sched_ravg_hist_size - 1;
	ridx = widx - samples;
	for (; ridx >= 0; --widx, --ridx) {
		hist[widx] = hist[ridx];
		sum += hist[widx];
		if (hist[widx] > max)
			max = hist[widx];
	}

	for (widx = 0; widx < samples && widx < sched_ravg_hist_size; widx++) {
		hist[widx] = runtime;
		sum += hist[widx];
		if (hist[widx] > max)
			max = hist[widx];
	}

	p->ravg.sum = 0;

	if (sched_window_stats_policy == WINDOW_STATS_RECENT) {
		demand = runtime;
	} else if (sched_window_stats_policy == WINDOW_STATS_MAX) {
		demand = max;
	} else {
		avg = div64_u64(sum, sched_ravg_hist_size);
		if (sched_window_stats_policy == WINDOW_STATS_AVG)
			demand = avg;
		else
			demand = max(avg, runtime);
	}
	pred_demand = predict_and_update_buckets(rq, p, runtime);

	/*
	 * A throttled deadline sched class task gets dequeued without
	 * changing p->on_rq. Since the dequeue decrements walt stats
	 * avoid decrementing it here again.
	 *
	 * When window is rolled over, the cumulative window demand
	 * is reset to the cumulative runnable average (contribution from
	 * the tasks on the runqueue). If the current task is dequeued
	 * already, it's demand is not included in the cumulative runnable
	 * average. So add the task demand separately to cumulative window
	 * demand.
	 */
	if (!task_has_dl_policy(p) || !p->dl.dl_throttled) {
		if (task_on_rq_queued(p))
			p->sched_class->fixup_walt_sched_stats(rq, p, demand,
							       pred_demand);
		else if (rq->curr == p)
			walt_fixup_cum_window_demand(rq, demand);
	}

	p->ravg.demand = demand;
	p->ravg.coloc_demand = div64_u64(sum, sched_ravg_hist_size);
	p->ravg.pred_demand = pred_demand;

done:
	trace_sched_update_history(rq, p, runtime, samples, event);
}

static u64 add_to_task_demand(struct rq *rq, struct task_struct *p, u64 delta)
{
	delta = scale_exec_time(delta, rq);
	p->ravg.sum += delta;
	if (unlikely(p->ravg.sum > sched_ravg_window))
		p->ravg.sum = sched_ravg_window;

	return delta;
}

/*
 * Account cpu demand of task and/or update task's cpu demand history
 *
 * ms = p->ravg.mark_start;
 * wc = wallclock
 * ws = rq->window_start
 *
 * Three possibilities:
 *
 *	a) Task event is contained within one window.
 *		window_start < mark_start < wallclock
 *
 *		ws   ms  wc
 *		|    |   |
 *		V    V   V
 *		|---------------|
 *
 *	In this case, p->ravg.sum is updated *iff* event is appropriate
 *	(ex: event == PUT_PREV_TASK)
 *
 *	b) Task event spans two windows.
 *		mark_start < window_start < wallclock
 *
 *		ms   ws   wc
 *		|    |    |
 *		V    V    V
 *		-----|-------------------
 *
 *	In this case, p->ravg.sum is updated with (ws - ms) *iff* event
 *	is appropriate, then a new window sample is recorded followed
 *	by p->ravg.sum being set to (wc - ws) *iff* event is appropriate.
 *
 *	c) Task event spans more than two windows.
 *
 *		ms ws_tmp			   ws  wc
 *		|  |				   |   |
 *		V  V				   V   V
 *		---|-------|-------|-------|-------|------
 *		   |				   |
 *		   |<------ nr_full_windows ------>|
 *
 *	In this case, p->ravg.sum is updated with (ws_tmp - ms) first *iff*
 *	event is appropriate, window sample of p->ravg.sum is recorded,
 *	'nr_full_window' samples of window_size is also recorded *iff*
 *	event is appropriate and finally p->ravg.sum is set to (wc - ws)
 *	*iff* event is appropriate.
 *
 * IMPORTANT : Leave p->ravg.mark_start unchanged, as update_cpu_busy_time()
 * depends on it!
 */
static u64 update_task_demand(struct task_struct *p, struct rq *rq,
			       int event, u64 wallclock)
{
	u64 mark_start = p->ravg.mark_start;
	u64 delta, window_start = rq->window_start;
	int new_window, nr_full_windows;
	u32 window_size = sched_ravg_window;
	u64 runtime;

	new_window = mark_start < window_start;
	if (!account_busy_for_task_demand(rq, p, event)) {
		if (new_window)
			/*
			 * If the time accounted isn't being accounted as
			 * busy time, and a new window started, only the
			 * previous window need be closed out with the
			 * pre-existing demand. Multiple windows may have
			 * elapsed, but since empty windows are dropped,
			 * it is not necessary to account those.
			 */
			update_history(rq, p, p->ravg.sum, 1, event);
		return 0;
	}

	if (!new_window) {
		/*
		 * The simple case - busy time contained within the existing
		 * window.
		 */
		return add_to_task_demand(rq, p, wallclock - mark_start);
	}

	/*
	 * Busy time spans at least two windows. Temporarily rewind
	 * window_start to first window boundary after mark_start.
	 */
	delta = window_start - mark_start;
	nr_full_windows = div64_u64(delta, window_size);
	window_start -= (u64)nr_full_windows * (u64)window_size;

	/* Process (window_start - mark_start) first */
	runtime = add_to_task_demand(rq, p, window_start - mark_start);

	/* Push new sample(s) into task's demand history */
	update_history(rq, p, p->ravg.sum, 1, event);
	if (nr_full_windows) {
		u64 scaled_window = scale_exec_time(window_size, rq);

		update_history(rq, p, scaled_window, nr_full_windows, event);
		runtime += nr_full_windows * scaled_window;
	}

	/*
	 * Roll window_start back to current to process any remainder
	 * in current window.
	 */
	window_start += (u64)nr_full_windows * (u64)window_size;

	/* Process (wallclock - window_start) next */
	mark_start = window_start;
	runtime += add_to_task_demand(rq, p, wallclock - mark_start);

	return runtime;
}

static void
update_task_rq_cpu_cycles(struct task_struct *p, struct rq *rq, int event,
			  u64 wallclock, u64 irqtime)
{
	u64 cur_cycles;
	int cpu = cpu_of(rq);

	lockdep_assert_held(&rq->lock);

	if (!use_cycle_counter) {
		rq->cc.cycles = cpu_cur_freq(cpu);
		rq->cc.time = 1;
		return;
	}

	cur_cycles = read_cycle_counter(cpu, wallclock);

	/*
	 * If current task is idle task and irqtime == 0 CPU was
	 * indeed idle and probably its cycle counter was not
	 * increasing.  We still need estimatied CPU frequency
	 * for IO wait time accounting.  Use the previously
	 * calculated frequency in such a case.
	 */
	if (!is_idle_task(rq->curr) || irqtime) {
		if (unlikely(cur_cycles < p->cpu_cycles))
			rq->cc.cycles = cur_cycles + (U64_MAX - p->cpu_cycles);
		else
			rq->cc.cycles = cur_cycles - p->cpu_cycles;
		rq->cc.cycles = rq->cc.cycles * NSEC_PER_MSEC;

		if (event == IRQ_UPDATE && is_idle_task(p))
			/*
			 * Time between mark_start of idle task and IRQ handler
			 * entry time is CPU cycle counter stall period.
			 * Upon IRQ handler entry sched_account_irqstart()
			 * replenishes idle task's cpu cycle counter so
			 * rq->cc.cycles now represents increased cycles during
			 * IRQ handler rather than time between idle entry and
			 * IRQ exit.  Thus use irqtime as time delta.
			 */
			rq->cc.time = irqtime;
		else
			rq->cc.time = wallclock - p->ravg.mark_start;
		BUG_ON((s64)rq->cc.time < 0);
	}

	p->cpu_cycles = cur_cycles;

	trace_sched_get_task_cpu_cycles(cpu, event, rq->cc.cycles, rq->cc.time, p);
}

static inline void run_walt_irq_work(u64 old_window_start, struct rq *rq)
{
	u64 result;

	if (old_window_start == rq->window_start)
		return;

	result = atomic64_cmpxchg(&walt_irq_work_lastq_ws, old_window_start,
				   rq->window_start);
	if (result == old_window_start)
		irq_work_queue(&walt_cpufreq_irq_work);
}

/* Reflect task activity on its demand and cpu's busy time statistics */
void update_task_ravg(struct task_struct *p, struct rq *rq, int event,
						u64 wallclock, u64 irqtime)
{
	u64 old_window_start;

	if (!rq->window_start || sched_disable_window_stats ||
	    p->ravg.mark_start == wallclock)
		return;

	lockdep_assert_held(&rq->lock);

	old_window_start = update_window_start(rq, wallclock, event);

	if (!p->ravg.mark_start) {
		update_task_cpu_cycles(p, cpu_of(rq), wallclock);
		goto done;
	}

	update_task_rq_cpu_cycles(p, rq, event, wallclock, irqtime);
	update_task_demand(p, rq, event, wallclock);
	update_cpu_busy_time(p, rq, event, wallclock, irqtime);
	update_task_pred_demand(rq, p, event);

	if (exiting_task(p))
		goto done;

	trace_sched_update_task_ravg(p, rq, event, wallclock, irqtime,
				rq->cc.cycles, rq->cc.time, &rq->grp_time);
	trace_sched_update_task_ravg_mini(p, rq, event, wallclock, irqtime,
				rq->cc.cycles, rq->cc.time, &rq->grp_time);

done:
	p->ravg.mark_start = wallclock;

	run_walt_irq_work(old_window_start, rq);
}

u32 sched_get_init_task_load(struct task_struct *p)
{
	return p->init_load_pct;
}

int sched_set_init_task_load(struct task_struct *p, int init_load_pct)
{
	if (init_load_pct < 0 || init_load_pct > 100)
		return -EINVAL;

	p->init_load_pct = init_load_pct;

	return 0;
}

void init_new_task_load(struct task_struct *p, bool idle_task)
{
	int i;
	u32 init_load_windows;
	u32 init_load_pct;

	p->init_load_pct = 0;
	rcu_assign_pointer(p->grp, NULL);
	INIT_LIST_HEAD(&p->grp_list);
	memset(&p->ravg, 0, sizeof(struct ravg));
	p->cpu_cycles = 0;

	p->ravg.curr_window_cpu = kcalloc(nr_cpu_ids, sizeof(u32), GFP_KERNEL);
	p->ravg.prev_window_cpu = kcalloc(nr_cpu_ids, sizeof(u32), GFP_KERNEL);

	/* Don't have much choice. CPU frequency would be bogus */
	BUG_ON(!p->ravg.curr_window_cpu || !p->ravg.prev_window_cpu);

	if (idle_task)
		return;

	if (current->init_load_pct)
		init_load_pct = current->init_load_pct;
	else
		init_load_pct = sysctl_sched_init_task_load_pct;

	init_load_windows = div64_u64((u64)init_load_pct *
				(u64)sched_ravg_window, 100);

	p->ravg.demand = init_load_windows;
	p->ravg.coloc_demand = init_load_windows;
	p->ravg.pred_demand = 0;
	for (i = 0; i < RAVG_HIST_SIZE_MAX; ++i)
		p->ravg.sum_history[i] = init_load_windows;
	p->misfit = false;
}

/*
 * kfree() may wakeup kswapd. So this function should NOT be called
 * with any CPU's rq->lock acquired.
 */
void free_task_load_ptrs(struct task_struct *p)
{
	kfree(p->ravg.curr_window_cpu);
	kfree(p->ravg.prev_window_cpu);

	/*
	 * update_task_ravg() can be called for exiting tasks. While the
	 * function itself ensures correct behavior, the corresponding
	 * trace event requires that these pointers be NULL.
	 */
	p->ravg.curr_window_cpu = NULL;
	p->ravg.prev_window_cpu = NULL;
}

void reset_task_stats(struct task_struct *p)
{
	u32 sum = 0;
	u32 *curr_window_ptr = NULL;
	u32 *prev_window_ptr = NULL;

	if (exiting_task(p)) {
		sum = EXITING_TASK_MARKER;
	} else {
		curr_window_ptr =  p->ravg.curr_window_cpu;
		prev_window_ptr = p->ravg.prev_window_cpu;
		memset(curr_window_ptr, 0, sizeof(u32) * nr_cpu_ids);
		memset(prev_window_ptr, 0, sizeof(u32) * nr_cpu_ids);
	}

	memset(&p->ravg, 0, sizeof(struct ravg));

	p->ravg.curr_window_cpu = curr_window_ptr;
	p->ravg.prev_window_cpu = prev_window_ptr;

	/* Retain EXITING_TASK marker */
	p->ravg.sum_history[0] = sum;
}

void mark_task_starting(struct task_struct *p)
{
	u64 wallclock;
	struct rq *rq = task_rq(p);

	if (!rq->window_start || sched_disable_window_stats) {
		reset_task_stats(p);
		return;
	}

	wallclock = sched_ktime_clock();
	p->ravg.mark_start = p->last_wake_ts = wallclock;
	p->last_enqueued_ts = wallclock;
	p->last_switch_out_ts = 0;
	update_task_cpu_cycles(p, cpu_of(rq), wallclock);
}

static cpumask_t all_cluster_cpus = CPU_MASK_NONE;
DECLARE_BITMAP(all_cluster_ids, NR_CPUS);
struct sched_cluster *sched_cluster[NR_CPUS];
int num_clusters;

struct list_head cluster_head;

static void
insert_cluster(struct sched_cluster *cluster, struct list_head *head)
{
	struct sched_cluster *tmp;
	struct list_head *iter = head;

	list_for_each_entry(tmp, head, list) {
		if (cluster->max_power_cost < tmp->max_power_cost)
			break;
		iter = &tmp->list;
	}

	list_add(&cluster->list, iter);
}

static struct sched_cluster *alloc_new_cluster(const struct cpumask *cpus)
{
	struct sched_cluster *cluster = NULL;

	cluster = kzalloc(sizeof(struct sched_cluster), GFP_ATOMIC);
	if (!cluster) {
		__WARN_printf("Cluster allocation failed.  Possible bad scheduling\n");
		return NULL;
	}

	INIT_LIST_HEAD(&cluster->list);
	cluster->max_power_cost		=	1;
	cluster->min_power_cost		=	1;
	cluster->capacity		=	1024;
	cluster->max_possible_capacity	=	1024;
	cluster->efficiency		=	1;
	cluster->load_scale_factor	=	1024;
	cluster->cur_freq		=	1;
	cluster->max_freq		=	1;
	cluster->max_mitigated_freq	=	UINT_MAX;
	cluster->min_freq		=	1;
	cluster->max_possible_freq	=	1;
	cluster->dstate			=	0;
	cluster->dstate_wakeup_energy	=	0;
	cluster->dstate_wakeup_latency	=	0;
	cluster->freq_init_done		=	false;

	raw_spin_lock_init(&cluster->load_lock);
	cluster->cpus = *cpus;
	cluster->efficiency = arch_get_cpu_efficiency(cpumask_first(cpus));

	if (cluster->efficiency > max_possible_efficiency)
		max_possible_efficiency = cluster->efficiency;
	if (cluster->efficiency < min_possible_efficiency)
		min_possible_efficiency = cluster->efficiency;

	cluster->notifier_sent = 0;
	return cluster;
}

static void add_cluster(const struct cpumask *cpus, struct list_head *head)
{
	struct sched_cluster *cluster = alloc_new_cluster(cpus);
	int i;

	if (!cluster)
		return;

	for_each_cpu(i, cpus)
		cpu_rq(i)->cluster = cluster;

	insert_cluster(cluster, head);
	set_bit(num_clusters, all_cluster_ids);
	num_clusters++;
}

static int compute_max_possible_capacity(struct sched_cluster *cluster)
{
	int capacity = 1024;

	capacity *= capacity_scale_cpu_efficiency(cluster);
	capacity >>= 10;

	capacity *= (1024 * cluster->max_possible_freq) / min_max_freq;
	capacity >>= 10;

	return capacity;
}

static void update_min_max_capacity(void)
{
	unsigned long flags;

	acquire_rq_locks_irqsave(cpu_possible_mask, &flags);
	__update_min_max_capacity();
	release_rq_locks_irqrestore(cpu_possible_mask, &flags);
}

unsigned int max_power_cost = 1;

static int
compare_clusters(void *priv, struct list_head *a, struct list_head *b)
{
	struct sched_cluster *cluster1, *cluster2;
	int ret;

	cluster1 = container_of(a, struct sched_cluster, list);
	cluster2 = container_of(b, struct sched_cluster, list);

	/*
	 * Don't assume higher capacity means higher power. If the
	 * power cost is same, sort the higher capacity cluster before
	 * the lower capacity cluster to start placing the tasks
	 * on the higher capacity cluster.
	 */
	ret = cluster1->max_power_cost > cluster2->max_power_cost ||
		(cluster1->max_power_cost == cluster2->max_power_cost &&
		cluster1->max_possible_capacity <
				cluster2->max_possible_capacity);

	return ret;
}

static void sort_clusters(void)
{
	struct sched_cluster *cluster;
	struct list_head new_head;
	unsigned int tmp_max = 1;

	INIT_LIST_HEAD(&new_head);

	for_each_sched_cluster(cluster) {
		cluster->max_power_cost = power_cost(cluster_first_cpu(cluster),
							       true);
		cluster->min_power_cost = power_cost(cluster_first_cpu(cluster),
							       false);

		if (cluster->max_power_cost > tmp_max)
			tmp_max = cluster->max_power_cost;
	}
	max_power_cost = tmp_max;

	move_list(&new_head, &cluster_head, true);

	list_sort(NULL, &new_head, compare_clusters);
	assign_cluster_ids(&new_head);

	/*
	 * Ensure cluster ids are visible to all CPUs before making
	 * cluster_head visible.
	 */
	move_list(&cluster_head, &new_head, false);
}

int __read_mostly min_power_cpu;

void walt_sched_energy_populated_callback(void)
{
	struct sched_cluster *cluster;
	int prev_max = 0, next_min = 0;

	mutex_lock(&cluster_lock);

	if (num_clusters == 1) {
		sysctl_sched_is_big_little = 0;
		mutex_unlock(&cluster_lock);
		return;
	}

	sort_clusters();

	for_each_sched_cluster(cluster) {
		if (cluster->min_power_cost > prev_max) {
			prev_max = cluster->max_power_cost;
			continue;
		}
		/*
		 * We assume no overlap in the power curves of
		 * clusters on a big.LITTLE system.
		 */
		sysctl_sched_is_big_little = 0;
		next_min = cluster->min_power_cost;
	}

	/*
	 * Find the OPP at which the lower power cluster
	 * power is overlapping with the next cluster.
	 */
	if (!sysctl_sched_is_big_little) {
		int cpu = cluster_first_cpu(sched_cluster[0]);
		struct sched_group_energy *sge = sge_array[cpu][SD_LEVEL1];
		int i;

		for (i = 1; i < sge->nr_cap_states; i++) {
			if (sge->cap_states[i].power >= next_min) {
				sched_smp_overlap_capacity =
						sge->cap_states[i-1].cap;
				break;
			}
		}

		min_power_cpu = cpu;
	}

	mutex_unlock(&cluster_lock);
}

static void update_all_clusters_stats(void)
{
	struct sched_cluster *cluster;
	u64 highest_mpc = 0, lowest_mpc = U64_MAX;
	unsigned long flags;

	acquire_rq_locks_irqsave(cpu_possible_mask, &flags);

	for_each_sched_cluster(cluster) {
		u64 mpc;

		cluster->capacity = compute_capacity(cluster);
		mpc = cluster->max_possible_capacity =
			compute_max_possible_capacity(cluster);
		cluster->load_scale_factor = compute_load_scale_factor(cluster);

		cluster->exec_scale_factor =
			DIV_ROUND_UP(cluster->efficiency * 1024,
				     max_possible_efficiency);

		if (mpc > highest_mpc)
			highest_mpc = mpc;

		if (mpc < lowest_mpc)
			lowest_mpc = mpc;
	}

	max_possible_capacity = highest_mpc;
	min_max_possible_capacity = lowest_mpc;

	__update_min_max_capacity();
	sched_update_freq_max_load(cpu_possible_mask);
	release_rq_locks_irqrestore(cpu_possible_mask, &flags);
}

void update_cluster_topology(void)
{
	struct cpumask cpus = *cpu_possible_mask;
	const struct cpumask *cluster_cpus;
	struct list_head new_head;
	int i;

	INIT_LIST_HEAD(&new_head);

	for_each_cpu(i, &cpus) {
		cluster_cpus = cpu_coregroup_mask(i);
		cpumask_or(&all_cluster_cpus, &all_cluster_cpus, cluster_cpus);
		cpumask_andnot(&cpus, &cpus, cluster_cpus);
		add_cluster(cluster_cpus, &new_head);
	}

	assign_cluster_ids(&new_head);

	/*
	 * Ensure cluster ids are visible to all CPUs before making
	 * cluster_head visible.
	 */
	move_list(&cluster_head, &new_head, false);
	update_all_clusters_stats();
}

struct sched_cluster init_cluster = {
	.list			=	LIST_HEAD_INIT(init_cluster.list),
	.id			=	0,
	.max_power_cost		=	1,
	.min_power_cost		=	1,
	.capacity		=	1024,
	.max_possible_capacity	=	1024,
	.efficiency		=	1,
	.load_scale_factor	=	1024,
	.cur_freq		=	1,
	.max_freq		=	1,
	.max_mitigated_freq	=	UINT_MAX,
	.min_freq		=	1,
	.max_possible_freq	=	1,
	.dstate			=	0,
	.dstate_wakeup_energy	=	0,
	.dstate_wakeup_latency	=	0,
	.exec_scale_factor	=	1024,
	.notifier_sent		=	0,
	.wake_up_idle		=	0,
	.aggr_grp_load		=	0,
};

void init_clusters(void)
{
	bitmap_clear(all_cluster_ids, 0, NR_CPUS);
	init_cluster.cpus = *cpu_possible_mask;
	raw_spin_lock_init(&init_cluster.load_lock);
	INIT_LIST_HEAD(&cluster_head);
}

static unsigned long cpu_max_table_freq[NR_CPUS];

static int cpufreq_notifier_policy(struct notifier_block *nb,
		unsigned long val, void *data)
{
	struct cpufreq_policy *policy = (struct cpufreq_policy *)data;
	struct sched_cluster *cluster = NULL;
	struct cpumask policy_cluster = *policy->related_cpus;
	unsigned int orig_max_freq = 0;
	int i, j, update_capacity = 0;

	if (val != CPUFREQ_NOTIFY && val != CPUFREQ_REMOVE_POLICY &&
						val != CPUFREQ_CREATE_POLICY)
		return 0;

	if (val == CPUFREQ_REMOVE_POLICY || val == CPUFREQ_CREATE_POLICY) {
		update_min_max_capacity();
		return 0;
	}

	max_possible_freq = max(max_possible_freq, policy->cpuinfo.max_freq);
	if (min_max_freq == 1)
		min_max_freq = UINT_MAX;
	min_max_freq = min(min_max_freq, policy->cpuinfo.max_freq);
	BUG_ON(!min_max_freq);
	BUG_ON(!policy->max);

	for_each_cpu(i, &policy_cluster)
		cpu_max_table_freq[i] = policy->cpuinfo.max_freq;

	for_each_cpu(i, &policy_cluster) {
		cluster = cpu_rq(i)->cluster;
		cpumask_andnot(&policy_cluster, &policy_cluster,
						&cluster->cpus);

		orig_max_freq = cluster->max_freq;
		cluster->min_freq = policy->min;
		cluster->max_freq = policy->max;
		cluster->cur_freq = policy->cur;

		if (!cluster->freq_init_done) {
			mutex_lock(&cluster_lock);
			for_each_cpu(j, &cluster->cpus)
				cpumask_copy(&cpu_rq(j)->freq_domain_cpumask,
						policy->related_cpus);
			cluster->max_possible_freq = policy->cpuinfo.max_freq;
			cluster->max_possible_capacity =
				compute_max_possible_capacity(cluster);
			cluster->freq_init_done = true;

			sort_clusters();
			update_all_clusters_stats();
			mutex_unlock(&cluster_lock);
			continue;
		}

		update_capacity += (orig_max_freq != cluster->max_freq);
	}

	if (update_capacity)
		update_cpu_cluster_capacity(policy->related_cpus);

	return 0;
}

static struct notifier_block notifier_policy_block = {
	.notifier_call = cpufreq_notifier_policy
};

static int cpufreq_notifier_trans(struct notifier_block *nb,
		unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = (struct cpufreq_freqs *)data;
	unsigned int cpu = freq->cpu, new_freq = freq->new;
	unsigned long flags;
	struct sched_cluster *cluster;
	struct cpumask policy_cpus = cpu_rq(cpu)->freq_domain_cpumask;
	int i, j;

	if (val != CPUFREQ_POSTCHANGE)
		return NOTIFY_DONE;

	if (cpu_cur_freq(cpu) == new_freq)
		return NOTIFY_OK;

	for_each_cpu(i, &policy_cpus) {
		cluster = cpu_rq(i)->cluster;

		if (!use_cycle_counter) {
			for_each_cpu(j, &cluster->cpus) {
				struct rq *rq = cpu_rq(j);

				raw_spin_lock_irqsave(&rq->lock, flags);
				update_task_ravg(rq->curr, rq, TASK_UPDATE,
						 sched_ktime_clock(), 0);
				raw_spin_unlock_irqrestore(&rq->lock, flags);
			}
		}

		cluster->cur_freq = new_freq;
		cpumask_andnot(&policy_cpus, &policy_cpus, &cluster->cpus);
	}

	return NOTIFY_OK;
}

static struct notifier_block notifier_trans_block = {
	.notifier_call = cpufreq_notifier_trans
};

static int register_walt_callback(void)
{
	int ret;

	ret = cpufreq_register_notifier(&notifier_policy_block,
					CPUFREQ_POLICY_NOTIFIER);
	if (!ret)
		ret = cpufreq_register_notifier(&notifier_trans_block,
						CPUFREQ_TRANSITION_NOTIFIER);

	return ret;
}
/*
 * cpufreq callbacks can be registered at core_initcall or later time.
 * Any registration done prior to that is "forgotten" by cpufreq. See
 * initialization of variable init_cpufreq_transition_notifier_list_called
 * for further information.
 */
core_initcall(register_walt_callback);

static void transfer_busy_time(struct rq *rq, struct related_thread_group *grp,
				struct task_struct *p, int event);

/*
 * Enable colocation and frequency aggregation for all threads in a process.
 * The children inherits the group id from the parent.
 */
unsigned int __read_mostly sysctl_sched_enable_thread_grouping;

/* Maximum allowed threshold before freq aggregation must be enabled */
#define MAX_FREQ_AGGR_THRESH 1000

struct related_thread_group *related_thread_groups[MAX_NUM_CGROUP_COLOC_ID];
static LIST_HEAD(active_related_thread_groups);
DEFINE_RWLOCK(related_thread_group_lock);

unsigned int __read_mostly sysctl_sched_freq_aggregate_threshold_pct;

/*
 * Task groups whose aggregate demand on a cpu is more than
 * sched_group_upmigrate need to be up-migrated if possible.
 */
unsigned int __read_mostly sched_group_upmigrate = 20000000;
unsigned int __read_mostly sysctl_sched_group_upmigrate_pct = 100;

/*
 * Task groups, once up-migrated, will need to drop their aggregate
 * demand to less than sched_group_downmigrate before they are "down"
 * migrated.
 */
unsigned int __read_mostly sched_group_downmigrate = 19000000;
unsigned int __read_mostly sysctl_sched_group_downmigrate_pct = 95;

static int
group_will_fit(struct sched_cluster *cluster, struct related_thread_group *grp,
						u64 demand, bool group_boost)
{
	int cpu = cluster_first_cpu(cluster);
	int prev_capacity = 0;
	unsigned int threshold = sched_group_upmigrate;
	u64 load;

	if (cluster->capacity == max_capacity)
		return 1;

	if (group_boost)
		return 0;

	if (!demand)
		return 1;

	if (grp->preferred_cluster)
		prev_capacity = grp->preferred_cluster->capacity;

	if (cluster->capacity < prev_capacity)
		threshold = sched_group_downmigrate;

	load = scale_load_to_cpu(demand, cpu);
	if (load < threshold)
		return 1;

	return 0;
}

unsigned long __weak arch_get_cpu_efficiency(int cpu)
{
	return SCHED_CAPACITY_SCALE;
}

/* Return cluster which can offer required capacity for group */
static struct sched_cluster *best_cluster(struct related_thread_group *grp,
					u64 total_demand, bool group_boost)
{
	struct sched_cluster *cluster = NULL;

	for_each_sched_cluster(cluster) {
		if (group_will_fit(cluster, grp, total_demand, group_boost))
			return cluster;
	}

	return sched_cluster[0];
}

int preferred_cluster(struct sched_cluster *cluster, struct task_struct *p)
{
	struct related_thread_group *grp;
	int rc = 1;

	rcu_read_lock();

	grp = task_related_thread_group(p);
	if (grp)
		rc = (grp->preferred_cluster == cluster);

	rcu_read_unlock();
	return rc;
}

static void _set_preferred_cluster(struct related_thread_group *grp)
{
	struct task_struct *p;
	u64 combined_demand = 0;
	bool boost_on_big = sched_boost_policy() == SCHED_BOOST_ON_BIG;
	bool group_boost = false;
	u64 wallclock;

	if (list_empty(&grp->tasks))
		return;

	if (!sysctl_sched_is_big_little) {
		grp->preferred_cluster = sched_cluster[0];
		return;
	}

	wallclock = sched_ktime_clock();

	/*
	 * wakeup of two or more related tasks could race with each other and
	 * could result in multiple calls to _set_preferred_cluster being issued
	 * at same time. Avoid overhead in such cases of rechecking preferred
	 * cluster
	 */
	if (wallclock - grp->last_update < sched_ravg_window / 10)
		return;

	list_for_each_entry(p, &grp->tasks, grp_list) {
		if (boost_on_big && task_sched_boost(p)) {
			group_boost = true;
			break;
		}

		if (p->ravg.mark_start < wallclock -
		    (sched_ravg_window * sched_ravg_hist_size))
			continue;

		combined_demand += p->ravg.coloc_demand;

	}

	grp->preferred_cluster = best_cluster(grp,
			combined_demand, group_boost);
	grp->last_update = sched_ktime_clock();
	trace_sched_set_preferred_cluster(grp, combined_demand);
}

void set_preferred_cluster(struct related_thread_group *grp)
{
	raw_spin_lock(&grp->lock);
	_set_preferred_cluster(grp);
	raw_spin_unlock(&grp->lock);
}

int update_preferred_cluster(struct related_thread_group *grp,
		struct task_struct *p, u32 old_load)
{
	u32 new_load = task_load(p);

	if (!grp)
		return 0;

	/*
	 * Update if task's load has changed significantly or a complete window
	 * has passed since we last updated preference
	 */
	if (abs(new_load - old_load) > sched_ravg_window / 4 ||
		sched_ktime_clock() - grp->last_update > sched_ravg_window)
		return 1;

	return 0;
}

DEFINE_MUTEX(policy_mutex);

#define pct_to_real(tunable)	\
		(div64_u64((u64)tunable * (u64)max_task_load(), 100))

unsigned int update_freq_aggregate_threshold(unsigned int threshold)
{
	unsigned int old_threshold;

	mutex_lock(&policy_mutex);

	old_threshold = sysctl_sched_freq_aggregate_threshold_pct;

	sysctl_sched_freq_aggregate_threshold_pct = threshold;
	sched_freq_aggregate_threshold =
		pct_to_real(sysctl_sched_freq_aggregate_threshold_pct);

	mutex_unlock(&policy_mutex);

	return old_threshold;
}

#define ADD_TASK	0
#define REM_TASK	1

#define DEFAULT_CGROUP_COLOC_ID 1

static inline struct related_thread_group*
lookup_related_thread_group(unsigned int group_id)
{
	return related_thread_groups[group_id];
}

int alloc_related_thread_groups(void)
{
	int i, ret;
	struct related_thread_group *grp;

	/* groupd_id = 0 is invalid as it's special id to remove group. */
	for (i = 1; i < MAX_NUM_CGROUP_COLOC_ID; i++) {
		grp = kzalloc(sizeof(*grp), GFP_NOWAIT);
		if (!grp) {
			ret = -ENOMEM;
			goto err;
		}

		grp->id = i;
		INIT_LIST_HEAD(&grp->tasks);
		INIT_LIST_HEAD(&grp->list);
		raw_spin_lock_init(&grp->lock);

		related_thread_groups[i] = grp;
	}

	return 0;

err:
	for (i = 1; i < MAX_NUM_CGROUP_COLOC_ID; i++) {
		grp = lookup_related_thread_group(i);
		if (grp) {
			kfree(grp);
			related_thread_groups[i] = NULL;
		} else {
			break;
		}
	}

	return ret;
}

static void remove_task_from_group(struct task_struct *p)
{
	struct related_thread_group *grp = p->grp;
	struct rq *rq;
	int empty_group = 1;
	struct rq_flags rf;

	raw_spin_lock(&grp->lock);

	rq = __task_rq_lock(p, &rf);
	transfer_busy_time(rq, p->grp, p, REM_TASK);
	list_del_init(&p->grp_list);
	rcu_assign_pointer(p->grp, NULL);
	__task_rq_unlock(rq, &rf);


	if (!list_empty(&grp->tasks)) {
		empty_group = 0;
		_set_preferred_cluster(grp);
	}

	raw_spin_unlock(&grp->lock);

	/* Reserved groups cannot be destroyed */
	if (empty_group && grp->id != DEFAULT_CGROUP_COLOC_ID)
		 /*
		  * We test whether grp->list is attached with list_empty()
		  * hence re-init the list after deletion.
		  */
		list_del_init(&grp->list);
}

static int
add_task_to_group(struct task_struct *p, struct related_thread_group *grp)
{
	struct rq *rq;
	struct rq_flags rf;

	raw_spin_lock(&grp->lock);

	/*
	 * Change p->grp under rq->lock. Will prevent races with read-side
	 * reference of p->grp in various hot-paths
	 */
	rq = __task_rq_lock(p, &rf);
	transfer_busy_time(rq, grp, p, ADD_TASK);
	list_add(&p->grp_list, &grp->tasks);
	rcu_assign_pointer(p->grp, grp);
	__task_rq_unlock(rq, &rf);

	_set_preferred_cluster(grp);

	raw_spin_unlock(&grp->lock);

	return 0;
}

void add_new_task_to_grp(struct task_struct *new)
{
	unsigned long flags;
	struct related_thread_group *grp;
	struct task_struct *leader = new->group_leader;
	unsigned int leader_grp_id = sched_get_group_id(leader);

	if (!sysctl_sched_enable_thread_grouping &&
	    leader_grp_id != DEFAULT_CGROUP_COLOC_ID)
		return;

	if (thread_group_leader(new))
		return;

	if (leader_grp_id == DEFAULT_CGROUP_COLOC_ID) {
		if (!same_schedtune(new, leader))
			return;
	}

	write_lock_irqsave(&related_thread_group_lock, flags);

	rcu_read_lock();
	grp = task_related_thread_group(leader);
	rcu_read_unlock();

	/*
	 * It's possible that someone already added the new task to the
	 * group. A leader's thread group is updated prior to calling
	 * this function. It's also possible that the leader has exited
	 * the group. In either case, there is nothing else to do.
	 */
	if (!grp || new->grp) {
		write_unlock_irqrestore(&related_thread_group_lock, flags);
		return;
	}

	raw_spin_lock(&grp->lock);

	rcu_assign_pointer(new->grp, grp);
	list_add(&new->grp_list, &grp->tasks);

	raw_spin_unlock(&grp->lock);
	write_unlock_irqrestore(&related_thread_group_lock, flags);
}

static int __sched_set_group_id(struct task_struct *p, unsigned int group_id)
{
	int rc = 0;
	unsigned long flags;
	struct related_thread_group *grp = NULL;

	if (group_id >= MAX_NUM_CGROUP_COLOC_ID)
		return -EINVAL;

	raw_spin_lock_irqsave(&p->pi_lock, flags);
	write_lock(&related_thread_group_lock);

	/* Switching from one group to another directly is not permitted */
	if ((current != p && p->flags & PF_EXITING) ||
			(!p->grp && !group_id) ||
			(p->grp && group_id))
		goto done;

	if (!group_id) {
		remove_task_from_group(p);
		goto done;
	}

	grp = lookup_related_thread_group(group_id);
	if (list_empty(&grp->list))
		list_add(&grp->list, &active_related_thread_groups);

	rc = add_task_to_group(p, grp);
done:
	write_unlock(&related_thread_group_lock);
	raw_spin_unlock_irqrestore(&p->pi_lock, flags);

	return rc;
}

int sched_set_group_id(struct task_struct *p, unsigned int group_id)
{
	/* DEFAULT_CGROUP_COLOC_ID is a reserved id */
	if (group_id == DEFAULT_CGROUP_COLOC_ID)
		return -EINVAL;

	return __sched_set_group_id(p, group_id);
}

unsigned int sched_get_group_id(struct task_struct *p)
{
	unsigned int group_id;
	struct related_thread_group *grp;

	rcu_read_lock();
	grp = task_related_thread_group(p);
	group_id = grp ? grp->id : 0;
	rcu_read_unlock();

	return group_id;
}

#if defined(CONFIG_SCHED_TUNE) && defined(CONFIG_CGROUP_SCHEDTUNE)
/*
 * We create a default colocation group at boot. There is no need to
 * synchronize tasks between cgroups at creation time because the
 * correct cgroup hierarchy is not available at boot. Therefore cgroup
 * colocation is turned off by default even though the colocation group
 * itself has been allocated. Furthermore this colocation group cannot
 * be destroyted once it has been created. All of this has been as part
 * of runtime optimizations.
 *
 * The job of synchronizing tasks to the colocation group is done when
 * the colocation flag in the cgroup is turned on.
 */
static int __init create_default_coloc_group(void)
{
	struct related_thread_group *grp = NULL;
	unsigned long flags;

	grp = lookup_related_thread_group(DEFAULT_CGROUP_COLOC_ID);
	write_lock_irqsave(&related_thread_group_lock, flags);
	list_add(&grp->list, &active_related_thread_groups);
	write_unlock_irqrestore(&related_thread_group_lock, flags);

	update_freq_aggregate_threshold(MAX_FREQ_AGGR_THRESH);
	return 0;
}
late_initcall(create_default_coloc_group);

int sync_cgroup_colocation(struct task_struct *p, bool insert)
{
	unsigned int grp_id = insert ? DEFAULT_CGROUP_COLOC_ID : 0;

	return __sched_set_group_id(p, grp_id);
}
#endif

void update_cpu_cluster_capacity(const cpumask_t *cpus)
{
	int i;
	struct sched_cluster *cluster;
	struct cpumask cpumask;
	unsigned long flags;

	cpumask_copy(&cpumask, cpus);
	acquire_rq_locks_irqsave(cpu_possible_mask, &flags);

	for_each_cpu(i, &cpumask) {
		cluster = cpu_rq(i)->cluster;
		cpumask_andnot(&cpumask, &cpumask, &cluster->cpus);

		cluster->capacity = compute_capacity(cluster);
		cluster->load_scale_factor = compute_load_scale_factor(cluster);
	}

	__update_min_max_capacity();

	release_rq_locks_irqrestore(cpu_possible_mask, &flags);
}

static unsigned long max_cap[NR_CPUS];
static unsigned long thermal_cap_cpu[NR_CPUS];

unsigned long thermal_cap(int cpu)
{
	return thermal_cap_cpu[cpu] ?: SCHED_CAPACITY_SCALE;
}

unsigned long do_thermal_cap(int cpu, unsigned long thermal_max_freq)
{
	struct sched_domain *sd;
	struct sched_group *sg;
	struct rq *rq = cpu_rq(cpu);
	int nr_cap_states;

	if (!max_cap[cpu]) {
		rcu_read_lock();
		sd = rcu_dereference(per_cpu(sd_ea, cpu));
		if (!sd || !sd->groups || !sd->groups->sge ||
		    !sd->groups->sge->cap_states) {
			rcu_read_unlock();
			return rq->cpu_capacity_orig;
		}
		sg = sd->groups;
		nr_cap_states = sg->sge->nr_cap_states;
		max_cap[cpu] = sg->sge->cap_states[nr_cap_states - 1].cap;
		rcu_read_unlock();
	}

	if (cpu_max_table_freq[cpu])
		return div64_ul(thermal_max_freq * max_cap[cpu],
				cpu_max_table_freq[cpu]);
	else
		return rq->cpu_capacity_orig;
}

static DEFINE_SPINLOCK(cpu_freq_min_max_lock);
void sched_update_cpu_freq_min_max(const cpumask_t *cpus, u32 fmin, u32 fmax)
{
	struct cpumask cpumask;
	struct sched_cluster *cluster;
	int i, update_capacity = 0;
	unsigned long flags;

	spin_lock_irqsave(&cpu_freq_min_max_lock, flags);
	cpumask_copy(&cpumask, cpus);

	for_each_cpu(i, &cpumask)
		thermal_cap_cpu[i] = do_thermal_cap(i, fmax);

	for_each_cpu(i, &cpumask) {
		cluster = cpu_rq(i)->cluster;
		cpumask_andnot(&cpumask, &cpumask, &cluster->cpus);
		update_capacity += (cluster->max_mitigated_freq != fmax);
		cluster->max_mitigated_freq = fmax;
	}
	spin_unlock_irqrestore(&cpu_freq_min_max_lock, flags);

	if (update_capacity)
		update_cpu_cluster_capacity(cpus);
}

void note_task_waking(struct task_struct *p, u64 wallclock)
{
	p->last_wake_ts = wallclock;
}

/*
 * Task's cpu usage is accounted in:
 *	rq->curr/prev_runnable_sum,  when its ->grp is NULL
 *	grp->cpu_time[cpu]->curr/prev_runnable_sum, when its ->grp is !NULL
 *
 * Transfer task's cpu usage between those counters when transitioning between
 * groups
 */
static void transfer_busy_time(struct rq *rq, struct related_thread_group *grp,
				struct task_struct *p, int event)
{
	u64 wallclock;
	struct group_cpu_time *cpu_time;
	u64 *src_curr_runnable_sum, *dst_curr_runnable_sum;
	u64 *src_prev_runnable_sum, *dst_prev_runnable_sum;
	u64 *src_nt_curr_runnable_sum, *dst_nt_curr_runnable_sum;
	u64 *src_nt_prev_runnable_sum, *dst_nt_prev_runnable_sum;
	int migrate_type;
	int cpu = cpu_of(rq);
	bool new_task;
	int i;

	wallclock = sched_ktime_clock();

	update_task_ravg(rq->curr, rq, TASK_UPDATE, wallclock, 0);
	update_task_ravg(p, rq, TASK_UPDATE, wallclock, 0);
	new_task = is_new_task(p);

	cpu_time = &rq->grp_time;
	if (event == ADD_TASK) {
		migrate_type = RQ_TO_GROUP;

		src_curr_runnable_sum = &rq->curr_runnable_sum;
		dst_curr_runnable_sum = &cpu_time->curr_runnable_sum;
		src_prev_runnable_sum = &rq->prev_runnable_sum;
		dst_prev_runnable_sum = &cpu_time->prev_runnable_sum;

		src_nt_curr_runnable_sum = &rq->nt_curr_runnable_sum;
		dst_nt_curr_runnable_sum = &cpu_time->nt_curr_runnable_sum;
		src_nt_prev_runnable_sum = &rq->nt_prev_runnable_sum;
		dst_nt_prev_runnable_sum = &cpu_time->nt_prev_runnable_sum;

		*src_curr_runnable_sum -= p->ravg.curr_window_cpu[cpu];
		*src_prev_runnable_sum -= p->ravg.prev_window_cpu[cpu];
		if (new_task) {
			*src_nt_curr_runnable_sum -=
					p->ravg.curr_window_cpu[cpu];
			*src_nt_prev_runnable_sum -=
					p->ravg.prev_window_cpu[cpu];
		}

		update_cluster_load_subtractions(p, cpu,
				rq->window_start, new_task);

	} else {
		migrate_type = GROUP_TO_RQ;

		src_curr_runnable_sum = &cpu_time->curr_runnable_sum;
		dst_curr_runnable_sum = &rq->curr_runnable_sum;
		src_prev_runnable_sum = &cpu_time->prev_runnable_sum;
		dst_prev_runnable_sum = &rq->prev_runnable_sum;

		src_nt_curr_runnable_sum = &cpu_time->nt_curr_runnable_sum;
		dst_nt_curr_runnable_sum = &rq->nt_curr_runnable_sum;
		src_nt_prev_runnable_sum = &cpu_time->nt_prev_runnable_sum;
		dst_nt_prev_runnable_sum = &rq->nt_prev_runnable_sum;

		*src_curr_runnable_sum -= p->ravg.curr_window;
		*src_prev_runnable_sum -= p->ravg.prev_window;
		if (new_task) {
			*src_nt_curr_runnable_sum -= p->ravg.curr_window;
			*src_nt_prev_runnable_sum -= p->ravg.prev_window;
		}

		/*
		 * Need to reset curr/prev windows for all CPUs, not just the
		 * ones in the same cluster. Since inter cluster migrations
		 * did not result in the appropriate book keeping, the values
		 * per CPU would be inaccurate.
		 */
		for_each_possible_cpu(i) {
			p->ravg.curr_window_cpu[i] = 0;
			p->ravg.prev_window_cpu[i] = 0;
		}
	}

	*dst_curr_runnable_sum += p->ravg.curr_window;
	*dst_prev_runnable_sum += p->ravg.prev_window;
	if (new_task) {
		*dst_nt_curr_runnable_sum += p->ravg.curr_window;
		*dst_nt_prev_runnable_sum += p->ravg.prev_window;
	}

	/*
	 * When a task enter or exits a group, it's curr and prev windows are
	 * moved to a single CPU. This behavior might be sub-optimal in the
	 * exit case, however, it saves us the overhead of handling inter
	 * cluster migration fixups while the task is part of a related group.
	 */
	p->ravg.curr_window_cpu[cpu] = p->ravg.curr_window;
	p->ravg.prev_window_cpu[cpu] = p->ravg.prev_window;

	trace_sched_migration_update_sum(p, migrate_type, rq);

	BUG_ON((s64)*src_curr_runnable_sum < 0);
	BUG_ON((s64)*src_prev_runnable_sum < 0);
	BUG_ON((s64)*src_nt_curr_runnable_sum < 0);
	BUG_ON((s64)*src_nt_prev_runnable_sum < 0);
}

/*
 * Runs in hard-irq context. This should ideally run just after the latest
 * window roll-over.
 */
void walt_irq_work(struct irq_work *irq_work)
{
	struct sched_cluster *cluster;
	struct rq *rq;
	int cpu;
	u64 wc;
	int flag = SCHED_CPUFREQ_WALT;
	bool is_migration = false;

	/* Am I the window rollover work or the migration work? */
	if (irq_work == &walt_migration_irq_work)
		is_migration = true;

	for_each_cpu(cpu, cpu_possible_mask)
		raw_spin_lock(&cpu_rq(cpu)->lock);

	wc = sched_ktime_clock();
	walt_load_reported_window = atomic64_read(&walt_irq_work_lastq_ws);

	for_each_sched_cluster(cluster) {
		u64 aggr_grp_load = 0;

		raw_spin_lock(&cluster->load_lock);

		for_each_cpu(cpu, &cluster->cpus) {
			rq = cpu_rq(cpu);
			if (rq->curr) {
				update_task_ravg(rq->curr, rq,
						TASK_UPDATE, wc, 0);
				account_load_subtractions(rq);
				aggr_grp_load += rq->grp_time.prev_runnable_sum;
			}
		}

		cluster->aggr_grp_load = aggr_grp_load;

		raw_spin_unlock(&cluster->load_lock);
	}

	for_each_sched_cluster(cluster) {
		for_each_cpu(cpu, &cluster->cpus) {
			int nflag = flag;

			rq = cpu_rq(cpu);

			if (is_migration) {
				if (rq->notif_pending) {
					nflag |= SCHED_CPUFREQ_INTERCLUSTER_MIG;
					rq->notif_pending = false;
				} else {
					nflag |= SCHED_CPUFREQ_FORCE_UPDATE;
				}
			}

			cpufreq_update_util(rq, nflag);
		}
	}

	for_each_cpu(cpu, cpu_possible_mask)
		raw_spin_unlock(&cpu_rq(cpu)->lock);

	if (!is_migration)
		core_ctl_check(this_rq()->window_start);
}

void walt_rotation_checkpoint(int nr_big)
{
	if (!hmp_capable())
		return;

	if (!sysctl_sched_walt_rotate_big_tasks || sched_boost() != NO_BOOST) {
		walt_rotation_enabled = 0;
		return;
	}

	walt_rotation_enabled = nr_big >= num_possible_cpus();
}

int walt_proc_update_handler(struct ctl_table *table, int write,
			     void __user *buffer, size_t *lenp,
			     loff_t *ppos)
{
	int ret;
	unsigned int *data = (unsigned int *)table->data;
	static DEFINE_MUTEX(mutex);

	mutex_lock(&mutex);
	ret = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	if (ret || !write) {
		mutex_unlock(&mutex);
		return ret;
	}

	if (data == &sysctl_sched_group_upmigrate_pct)
		sched_group_upmigrate =
		    pct_to_real(sysctl_sched_group_upmigrate_pct);
	else if (data == &sysctl_sched_group_downmigrate_pct)
		sched_group_downmigrate =
		    pct_to_real(sysctl_sched_group_downmigrate_pct);
	else
		ret = -EINVAL;
	mutex_unlock(&mutex);

	return ret;
}

void walt_sched_init(struct rq *rq)
{
	int j;

	cpumask_set_cpu(cpu_of(rq), &rq->freq_domain_cpumask);
	init_irq_work(&walt_migration_irq_work, walt_irq_work);
	init_irq_work(&walt_cpufreq_irq_work, walt_irq_work);
	walt_rotate_work_init();

	rq->walt_stats.cumulative_runnable_avg = 0;
	rq->window_start = 0;
	rq->cum_window_start = 0;
	rq->walt_stats.nr_big_tasks = 0;
	rq->walt_flags = 0;
	rq->cur_irqload = 0;
	rq->avg_irqload = 0;
	rq->irqload_ts = 0;
	rq->static_cpu_pwr_cost = 0;
	rq->cc.cycles = 1;
	rq->cc.time = 1;
	rq->cstate = 0;
	rq->wakeup_latency = 0;
	rq->wakeup_energy = 0;

	/*
	 * All cpus part of same cluster by default. This avoids the
	 * need to check for rq->cluster being non-NULL in hot-paths
	 * like select_best_cpu()
	 */
	rq->cluster = &init_cluster;
	rq->curr_runnable_sum = rq->prev_runnable_sum = 0;
	rq->nt_curr_runnable_sum = rq->nt_prev_runnable_sum = 0;
	memset(&rq->grp_time, 0, sizeof(struct group_cpu_time));
	rq->old_busy_time = 0;
	rq->old_estimated_time = 0;
	rq->old_busy_time_group = 0;
	rq->walt_stats.pred_demands_sum = 0;
	rq->ed_task = NULL;
	rq->curr_table = 0;
	rq->prev_top = 0;
	rq->curr_top = 0;
	rq->last_cc_update = 0;
	rq->cycles = 0;
	for (j = 0; j < NUM_TRACKED_WINDOWS; j++) {
		memset(&rq->load_subs[j], 0,
				sizeof(struct load_subtractions));
		rq->top_tasks[j] = kcalloc(NUM_LOAD_INDICES,
				sizeof(u8), GFP_NOWAIT);
		/* No other choice */
		BUG_ON(!rq->top_tasks[j]);
		clear_top_tasks_bitmap(rq->top_tasks_bitmap[j]);
	}
	rq->cum_window_demand = 0;
	rq->notif_pending = false;

	walt_cpu_util_freq_divisor =
	    (sched_ravg_window >> SCHED_CAPACITY_SHIFT) * 100;
}
