/*
 *  drivers/cpufreq/cpufreq_stats.c
 *
 *  Copyright (C) 2003-2004 Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>.
 *  (C) 2004 Zou Nan hai <nanhai.zou@intel.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/atomic.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/cputime.h>
#include <linux/hashtable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/proc_fs.h>
#include <linux/profile.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/sort.h>

#define UID_HASH_BITS 10

DECLARE_HASHTABLE(uid_hash_table, UID_HASH_BITS);

static spinlock_t cpufreq_stats_lock;

static DEFINE_SPINLOCK(task_time_in_state_lock); /* task->time_in_state */
static DEFINE_RT_MUTEX(uid_lock); /* uid_hash_table */

struct uid_entry {
	uid_t uid;
	unsigned int dead_max_state;
	unsigned int alive_max_state;
	u64 *dead_time_in_state;
	u64 *alive_time_in_state;
	struct hlist_node hash;
};

struct cpufreq_stats {
	unsigned int total_trans;
	unsigned long long last_time;
	unsigned int max_state;
	int prev_states;
	atomic_t curr_state;
	u64 *time_in_state;
	unsigned int *freq_table;
};

static int cpufreq_max_state;
static int cpufreq_last_max_state;
static unsigned int *cpufreq_states;
static bool cpufreq_stats_initialized;

struct cpufreq_stats_attribute {
	struct attribute attr;

	ssize_t (*show)(struct cpufreq_stats *, char *);
};

/* Caller must hold uid lock */
static struct uid_entry *find_uid_entry(uid_t uid)
{
	struct uid_entry *uid_entry;

	hash_for_each_possible(uid_hash_table, uid_entry, hash, uid) {
		if (uid_entry->uid == uid)
			return uid_entry;
	}
	return NULL;
}

/* Caller must hold uid lock */
static struct uid_entry *find_or_register_uid(uid_t uid)
{
	struct uid_entry *uid_entry;

	uid_entry = find_uid_entry(uid);
	if (uid_entry)
		return uid_entry;

	uid_entry = kzalloc(sizeof(struct uid_entry), GFP_ATOMIC);
	if (!uid_entry)
		return NULL;

	uid_entry->uid = uid;

	hash_add(uid_hash_table, &uid_entry->hash, uid);

	return uid_entry;
}


static int uid_time_in_state_show(struct seq_file *m, void *v)
{
	struct uid_entry *uid_entry;
	struct task_struct *task, *temp;
	unsigned long bkt, flags;
	struct cpufreq_policy *last_policy = NULL;
	int i;

	if (!cpufreq_stats_initialized)
		return 0;

	seq_puts(m, "uid:");
	for_each_possible_cpu(i) {
		struct cpufreq_frequency_table *table, *pos;
		struct cpufreq_policy *policy;

		policy = cpufreq_cpu_get(i);
		if (!policy)
			continue;
		table = cpufreq_frequency_get_table(i);

		/* Assumes cpus are colocated within a policy */
		if (table && last_policy != policy) {
			last_policy = policy;
			cpufreq_for_each_valid_entry(pos, table)
				seq_printf(m, " %d", pos->frequency);
		}
		cpufreq_cpu_put(policy);
	}
	seq_putc(m, '\n');

	rt_mutex_lock(&uid_lock);

	rcu_read_lock();
	do_each_thread(temp, task) {

		uid_entry = find_or_register_uid(from_kuid_munged(
			current_user_ns(), task_uid(task)));
		if (!uid_entry)
			continue;

		if (uid_entry->alive_max_state < task->max_state) {
			uid_entry->alive_time_in_state = krealloc(
				uid_entry->alive_time_in_state,
				task->max_state *
				sizeof(uid_entry->alive_time_in_state[0]),
				GFP_ATOMIC);
			memset(uid_entry->alive_time_in_state +
				uid_entry->alive_max_state,
				0, (task->max_state -
				uid_entry->alive_max_state) *
				sizeof(uid_entry->alive_time_in_state[0]));
			uid_entry->alive_max_state = task->max_state;
		}

		spin_lock_irqsave(&task_time_in_state_lock, flags);
		if (task->time_in_state) {
			for (i = 0; i < task->max_state; ++i) {
				uid_entry->alive_time_in_state[i] +=
					atomic_read(&task->time_in_state[i]);
			}
		}
		spin_unlock_irqrestore(&task_time_in_state_lock, flags);

	} while_each_thread(temp, task);
	rcu_read_unlock();

	hash_for_each(uid_hash_table, bkt, uid_entry, hash) {
		int max_state = uid_entry->dead_max_state;

		if (uid_entry->alive_max_state > max_state)
			max_state = uid_entry->alive_max_state;
		if (max_state)
			seq_printf(m, "%d:", uid_entry->uid);
		for (i = 0; i < max_state; ++i) {
			u64 total_time_in_state = 0;

			if (uid_entry->dead_time_in_state &&
				i < uid_entry->dead_max_state) {
				total_time_in_state =
					uid_entry->dead_time_in_state[i];
			}
			if (uid_entry->alive_time_in_state &&
				i < uid_entry->alive_max_state) {
				total_time_in_state +=
					uid_entry->alive_time_in_state[i];
			}
			seq_printf(m, " %lu", (unsigned long)
				cputime_to_clock_t(total_time_in_state));
		}
		if (max_state)
			seq_putc(m, '\n');

		kfree(uid_entry->alive_time_in_state);
		uid_entry->alive_time_in_state = NULL;
		uid_entry->alive_max_state = 0;
	}

	rt_mutex_unlock(&uid_lock);
	return 0;
}

static int cpufreq_stats_update(struct cpufreq_stats *stats)
{
	unsigned long long cur_time = get_jiffies_64();

	spin_lock(&cpufreq_stats_lock);
	stats->time_in_state[atomic_read(&stats->curr_state)] +=
		cur_time - stats->last_time;
	stats->last_time = cur_time;
	spin_unlock(&cpufreq_stats_lock);
	return 0;
}

void cpufreq_task_stats_init(struct task_struct *p)
{
	size_t alloc_size;
	void *temp;
	unsigned long flags;

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	p->time_in_state = NULL;
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	WRITE_ONCE(p->max_state, 0);

	if (!cpufreq_stats_initialized)
		return;

	/* We use one array to avoid multiple allocs per task */
	WRITE_ONCE(p->max_state, cpufreq_max_state);

	alloc_size = p->max_state * sizeof(p->time_in_state[0]);
	temp = kzalloc(alloc_size, GFP_KERNEL);

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	p->time_in_state = temp;
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
}

void cpufreq_task_stats_exit(struct task_struct *p)
{
	unsigned long flags;
	void *temp;

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	temp = p->time_in_state;
	p->time_in_state = NULL;
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	kfree(temp);
}

int proc_time_in_state_show(struct seq_file *m, struct pid_namespace *ns,
	struct pid *pid, struct task_struct *p)
{
	int i;
	cputime_t cputime;
	unsigned long flags;

	if (!cpufreq_stats_initialized || !p->time_in_state)
		return 0;

	spin_lock(&cpufreq_stats_lock);
	for (i = 0; i < p->max_state; ++i) {
		cputime = 0;
		spin_lock_irqsave(&task_time_in_state_lock, flags);
		if (p->time_in_state)
			cputime = atomic_read(&p->time_in_state[i]);
		spin_unlock_irqrestore(&task_time_in_state_lock, flags);

		seq_printf(m, "%d %lu\n", cpufreq_states[i],
			(unsigned long)cputime_to_clock_t(cputime));
	}
	spin_unlock(&cpufreq_stats_lock);

	return 0;
}

static ssize_t show_total_trans(struct cpufreq_policy *policy, char *buf)
{
	return sprintf(buf, "%d\n", policy->stats->total_trans);
}

static ssize_t show_time_in_state(struct cpufreq_policy *policy, char *buf)
{
	struct cpufreq_stats *stats = policy->stats;
	ssize_t len = 0;
	int i;

	cpufreq_stats_update(stats);
	for (i = 0; i < stats->max_state; i++) {
		len += sprintf(buf + len, "%u %llu\n", stats->freq_table[i],
			(unsigned long long)
			jiffies_64_to_clock_t(stats->time_in_state[i]));
	}
	return len;
}

/* Called without cpufreq_stats_lock held */
void acct_update_power(struct task_struct *task, cputime_t cputime)
{
	struct cpufreq_stats *stats;
	struct cpufreq_policy *policy;
	unsigned int cpu_num;
	unsigned int state;
	unsigned long flags;

	if (!task)
		return;

	cpu_num = task_cpu(task);
	policy = cpufreq_cpu_get(cpu_num);
	if (!policy)
		return;

	stats = policy->stats;
	if (!stats) {
		cpufreq_cpu_put(policy);
		return;
	}

	state = stats->prev_states + atomic_read(&policy->stats->curr_state);

	/* This function is called from a different context
	 * Interruptions in between reads/assignements are ok
	 */
	if (cpufreq_stats_initialized &&
		!(task->flags & PF_EXITING) &&
		state < READ_ONCE(task->max_state)) {
		spin_lock_irqsave(&task_time_in_state_lock, flags);
		if (task->time_in_state)
			atomic64_add(cputime, &task->time_in_state[state]);
		spin_unlock_irqrestore(&task_time_in_state_lock, flags);
	}
	cpufreq_cpu_put(policy);

}
EXPORT_SYMBOL_GPL(acct_update_power);

static ssize_t show_all_time_in_state(struct kobject *kobj,
	struct kobj_attribute *attr, char *buf)
{
	ssize_t len = 0;
	unsigned int i, cpu, freq;
	struct cpufreq_policy *policy;
	struct cpufreq_stats *stats;

	len += scnprintf(buf + len, PAGE_SIZE - len, "freq\t\t");
	for_each_possible_cpu(cpu) {
		policy = cpufreq_cpu_get(cpu);
		if (!policy)
			continue;
		stats = policy->stats;
		len += scnprintf(buf + len, PAGE_SIZE - len, "cpu%u\t\t", cpu);
		cpufreq_stats_update(stats);
		cpufreq_cpu_put(policy);
	}

	if (!cpufreq_stats_initialized)
		goto out;
	for (i = 0; i < cpufreq_max_state; i++) {
		freq = cpufreq_states[i];
		len += scnprintf(buf + len, PAGE_SIZE - len, "\n%u\t\t", freq);
		for_each_possible_cpu(cpu) {
			policy = cpufreq_cpu_get(cpu);
			if (!policy)
				continue;
			stats = policy->stats;
			if (i >= stats->prev_states &&
				i < stats->prev_states + stats->max_state) {
				len += scnprintf(buf + len, PAGE_SIZE - len,
					"%lu\t\t", (unsigned long)
					cputime64_to_clock_t(
						stats->time_in_state[i -
							stats->prev_states]));
			} else {
				len += scnprintf(buf + len, PAGE_SIZE - len,
						"N/A\t\t");
			}
			cpufreq_cpu_put(policy);
		}
	}

out:
	len += scnprintf(buf + len, PAGE_SIZE - len, "\n");
	return len;
}

cpufreq_freq_attr_ro(total_trans);
cpufreq_freq_attr_ro(time_in_state);

static struct attribute *default_attrs[] = {
	&total_trans.attr,
	&time_in_state.attr,
	NULL
};
static struct attribute_group stats_attr_group = {
	.attrs = default_attrs,
	.name = "stats"
};

static struct kobj_attribute _attr_all_time_in_state = __ATTR(all_time_in_state,
		0444, show_all_time_in_state, NULL);


static int freq_table_get_index(struct cpufreq_stats *stats, unsigned int freq)
{
	int index;
	for (index = 0; index < stats->max_state; index++)
		if (stats->freq_table[index] == freq)
			return index;
	return -1;
}

static void __cpufreq_stats_free_table(struct cpufreq_policy *policy)
{
	struct cpufreq_stats *stats = policy->stats;

	if (!stats)
		return;

	pr_debug("%s: Free stats table\n", __func__);

	sysfs_remove_group(&policy->kobj, &stats_attr_group);
	kfree(stats->time_in_state);
	kfree(stats);
	policy->stats = NULL;
	/* cpufreq_last_max_state is always incrementing, not changed here */
}

static void cpufreq_stats_free_table(unsigned int cpu)
{
	struct cpufreq_policy *policy;

	policy = cpufreq_cpu_get(cpu);
	if (!policy)
		return;

	if (cpufreq_frequency_get_table(policy->cpu))
		__cpufreq_stats_free_table(policy);

	cpufreq_cpu_put(policy);
}


static int cpufreq_stats_create_all_table(void)
{
	struct cpufreq_policy *last_policy = NULL;
	struct cpufreq_policy *policy;
	struct cpufreq_stats *stats;
	int cpu, i;

	cpufreq_states = kcalloc(cpufreq_max_state, sizeof(unsigned int),
		GFP_KERNEL);
	if (cpufreq_states == NULL)
		return -ENOMEM;

	for_each_possible_cpu(cpu) {
		policy = cpufreq_cpu_get(cpu);
		if (!policy)
			continue;
		stats = policy->stats;
		if (policy != last_policy) {
			for (i = 0; i < stats->max_state; ++i)
				cpufreq_states[stats->prev_states + i]
					= stats->freq_table[i];
			last_policy = policy;
		}
		cpufreq_cpu_put(policy);
	}
	return 0;
}

static int __cpufreq_stats_create_table(struct cpufreq_policy *policy,
	struct cpufreq_frequency_table *table, int count)
{
	unsigned int i, ret = 0;
	struct cpufreq_stats *stats;
	unsigned int alloc_size;
	struct cpufreq_frequency_table *pos;

	if (policy->stats)
		return -EBUSY;

	stats = kzalloc(sizeof(*stats), GFP_KERNEL);
	if (stats == NULL)
		return -ENOMEM;

	ret = sysfs_create_group(&policy->kobj, &stats_attr_group);
	if (ret)
		pr_warn("Cannot create stats attr group\n");

	alloc_size = count * sizeof(u64) + count * sizeof(unsigned int);

	stats->time_in_state = kzalloc(alloc_size, GFP_KERNEL);
	if (!stats->time_in_state) {
		ret = -ENOMEM;
		goto error_alloc;
	}
	stats->freq_table = (unsigned int *)(stats->time_in_state + count);

	i = 0;
	cpufreq_for_each_valid_entry(pos, table)
		if (freq_table_get_index(stats, pos->frequency) == -1)
			stats->freq_table[i++] = pos->frequency;

	cpufreq_last_max_state = cpufreq_max_state;
	stats->prev_states = cpufreq_last_max_state;
	stats->max_state = count;
	cpufreq_max_state += count;

	spin_lock(&cpufreq_stats_lock);
	stats->last_time = get_jiffies_64();
	atomic_set(&stats->curr_state,
		freq_table_get_index(stats, policy->cur));
	spin_unlock(&cpufreq_stats_lock);
	policy->stats = stats;
	return 0;
error_alloc:
	sysfs_remove_group(&policy->kobj, &stats_attr_group);
	kfree(stats);
	policy->stats = NULL;
	return ret;
}

static void cpufreq_stats_create_table(struct cpufreq_policy *policy)
{
	struct cpufreq_frequency_table *table, *pos;
	int count = 0;

	table = cpufreq_frequency_get_table(policy->cpu);
	if (likely(table)) {
		cpufreq_for_each_valid_entry(pos, table)
			count++;

		__cpufreq_stats_create_table(policy, table, count);
	}
}

void cpufreq_task_stats_remove_uids(uid_t uid_start, uid_t uid_end)
{
	struct uid_entry *uid_entry;
	struct hlist_node *tmp;

	rt_mutex_lock(&uid_lock);

	for (; uid_start <= uid_end; uid_start++) {
		hash_for_each_possible_safe(uid_hash_table, uid_entry, tmp,
			hash, uid_start) {
			if (uid_start == uid_entry->uid) {
				hash_del(&uid_entry->hash);
				kfree(uid_entry->dead_time_in_state);
				kfree(uid_entry);
			}
		}
	}

	rt_mutex_unlock(&uid_lock);
}

static int cpufreq_stat_notifier_policy(struct notifier_block *nb,
	unsigned long val, void *data)
{
	int ret = 0, count = 0;
	struct cpufreq_policy *policy = data;
	struct cpufreq_frequency_table *table, *pos;

	table = cpufreq_frequency_get_table(policy->cpu);
	if (!table)
		return 0;

	cpufreq_for_each_valid_entry(pos, table)
		count++;

	if (val == CPUFREQ_CREATE_POLICY)
		ret = __cpufreq_stats_create_table(policy, table, count);
	else if (val == CPUFREQ_REMOVE_POLICY)
		__cpufreq_stats_free_table(policy);

	return ret;
}

static int cpufreq_stat_notifier_trans(struct notifier_block *nb,
	unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	struct cpufreq_stats *stat;
	struct cpufreq_policy *policy;

	if (val != CPUFREQ_POSTCHANGE)
		return 0;

	policy = cpufreq_cpu_get(freq->cpu);
	if (!policy)
		return 0;

	stat = policy->stats;
	if (!stat) {
		cpufreq_cpu_put(policy);
		return 0;
	}

	cpufreq_stats_update(policy->stats);
	spin_lock(&cpufreq_stats_lock);
	atomic_set(&stat->curr_state, freq_table_get_index(stat, freq->new));
	stat->total_trans++;
	spin_unlock(&cpufreq_stats_lock);
	cpufreq_cpu_put(policy);
	return 0;
}


static int process_notifier(struct notifier_block *self,
	unsigned long cmd, void *v)
{
	struct task_struct *task = v;
	struct uid_entry *uid_entry;
	unsigned long flags;
	uid_t uid;
	int i;

	if (!task)
		return NOTIFY_OK;

	rt_mutex_lock(&uid_lock);

	uid = from_kuid_munged(current_user_ns(), task_uid(task));
	uid_entry = find_or_register_uid(uid);
	if (!uid_entry) {
		rt_mutex_unlock(&uid_lock);
		pr_err("%s: failed to find uid %d\n", __func__, uid);
		return NOTIFY_OK;
	}

	if (uid_entry->dead_max_state < task->max_state) {
		uid_entry->dead_time_in_state = krealloc(
			uid_entry->dead_time_in_state,
			task->max_state *
			sizeof(uid_entry->dead_time_in_state[0]),
			GFP_ATOMIC);
		memset(uid_entry->dead_time_in_state +
			uid_entry->dead_max_state,
			0, (task->max_state - uid_entry->dead_max_state) *
			sizeof(uid_entry->dead_time_in_state[0]));
		uid_entry->dead_max_state = task->max_state;
	}

	spin_lock_irqsave(&task_time_in_state_lock, flags);
	if (task->time_in_state) {
		for (i = 0; i < task->max_state; ++i) {
			uid_entry->dead_time_in_state[i] +=
				atomic_read(&task->time_in_state[i]);
		}
	}
	spin_unlock_irqrestore(&task_time_in_state_lock, flags);

	rt_mutex_unlock(&uid_lock);
	return NOTIFY_OK;
}

static int uid_time_in_state_open(struct inode *inode, struct file *file)
{
	return single_open(file, uid_time_in_state_show, PDE_DATA(inode));
}

static const struct file_operations uid_time_in_state_fops = {
	.open		= uid_time_in_state_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct notifier_block notifier_policy_block = {
	.notifier_call = cpufreq_stat_notifier_policy
};

static struct notifier_block notifier_trans_block = {
	.notifier_call = cpufreq_stat_notifier_trans
};

static struct notifier_block process_notifier_block = {
	.notifier_call	= process_notifier,
};

static int __init cpufreq_stats_init(void)
{
	int ret;
	unsigned int cpu;
	struct cpufreq_policy *policy;
	struct cpufreq_policy *last_policy = NULL;

	spin_lock_init(&cpufreq_stats_lock);
	ret = cpufreq_register_notifier(&notifier_policy_block,
				CPUFREQ_POLICY_NOTIFIER);
	if (ret)
		return ret;

	get_online_cpus();
	for_each_online_cpu(cpu) {
		policy = cpufreq_cpu_get(cpu);
		if (!policy)
			continue;
		if (policy != last_policy) {
			cpufreq_stats_create_table(policy);
			last_policy = policy;
		}
		cpufreq_cpu_put(policy);
	}
	put_online_cpus();

	/* XXX TODO task support for time_in_state doesn't update freq
	 * info for tasks already initialized, so tasks initialized early
	 * (before cpufreq_stat_init is done) do not get time_in_state data
	 * and CPUFREQ_TRANSITION_NOTIFIER does not update freq info for
	 * tasks already created
	 */
	ret = cpufreq_register_notifier(&notifier_trans_block,
				CPUFREQ_TRANSITION_NOTIFIER);
	if (ret) {
		cpufreq_unregister_notifier(&notifier_policy_block,
				CPUFREQ_POLICY_NOTIFIER);
		get_online_cpus();
		for_each_online_cpu(cpu)
			cpufreq_stats_free_table(cpu);
		put_online_cpus();
		return ret;
	}
	ret = sysfs_create_file(cpufreq_global_kobject,
			&_attr_all_time_in_state.attr);
	if (ret)
		pr_warn("Cannot create sysfs file for cpufreq stats\n");

	proc_create_data("uid_time_in_state", 0444, NULL,
		&uid_time_in_state_fops, NULL);

	profile_event_register(PROFILE_TASK_EXIT, &process_notifier_block);

	ret = cpufreq_stats_create_all_table();
	if (ret)
		pr_warn("Cannot create cpufreq all freqs table\n");

	cpufreq_stats_initialized = true;
	return 0;
}
static void __exit cpufreq_stats_exit(void)
{
	unsigned int cpu;

	cpufreq_unregister_notifier(&notifier_policy_block,
			CPUFREQ_POLICY_NOTIFIER);
	cpufreq_unregister_notifier(&notifier_trans_block,
			CPUFREQ_TRANSITION_NOTIFIER);
	for_each_online_cpu(cpu)
		cpufreq_stats_free_table(cpu);
}

MODULE_AUTHOR("Zou Nan hai <nanhai.zou@intel.com>");
MODULE_DESCRIPTION("Export cpufreq stats via sysfs");
MODULE_LICENSE("GPL");

module_init(cpufreq_stats_init);
module_exit(cpufreq_stats_exit);
