/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*	@file	arch/arm/plat-kona/kona_cpufreq.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <plat/kona_cpufreq_drv.h>
#include <linux/smp.h>
#include <plat/pi_mgr.h>
#include <mach/pwr_mgr.h>
#include <asm/cpu.h>
#include <asm/div64.h>
#include <plat/kona_pm.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#endif

#if defined(DEBUG)
#define kcf_dbg printk
#else
#define kcf_dbg(format...)              \
	do {                            \
	    if (kcf_debug)          	\
		printk(format); 	\
	} while(0)
#endif

#define PLL_VAL_FOR_TURBO_1P2G	1200000
#define PLL_VAL_FOR_TURBO_1G	999999

#ifdef CONFIG_KONA_TMON
#include <linux/broadcom/kona_tmon.h>
#define INVALID_INX 0xFFFFFFFF
#define INIT_WORK_DELAY 20
#endif

#define CPU_ACTIVE	0xFFFF

static int kcf_debug = 0;

struct kona_freq_map {
	u32 cpu_freq;
	int opp;		/* Operating point eg: ECONOMY, NORMAL, TURBO */
};

enum {
	FREQ_LMT_NODE_ADD,
	FREQ_LMT_NODE_DEL,
	FREQ_LMT_NODE_UPDATE,
};

struct kona_cpufreq_stats {
	int stats_en;
	unsigned long long start_time;
	u64 *stats[CONFIG_NR_CPUS];
	u32 cpu_in_idle[CONFIG_NR_CPUS];
	u32 act_freq_inx;
	u64 last_time[CONFIG_NR_CPUS];
};

struct kona_cpufreq {
	int pi_id;
	struct pi_mgr_dfs_node dfs_node;
	struct cpufreq_frequency_table *kona_freqs_table;
	struct kona_freq_map *freq_map;
	int no_of_opps;
	int num_cstates;
	struct cpufreq_policy *policy;
	struct kona_cpufreq_drv_pdata *pdata;
	spinlock_t kcf_lock;
	struct plist_head min_lmt_list;
	struct plist_head max_lmt_list;
	int active_min_lmt;
	int active_max_lmt;
#ifdef CONFIG_SMP
	unsigned long l_p_j_ref;
	unsigned int l_p_j_ref_freq;
#endif
#ifdef CONFIG_KONA_TMON
	struct notifier_block tmon_nb;
	int r_inx;
	int f_inx;
	struct cpufreq_lmt_node tmon_node;
	struct delayed_work init_work;
#endif
	struct kona_cpufreq_stats stats;
};

static struct kona_cpufreq *kona_cpufreq;

static struct cpufreq_lmt_node usr_min_lmt_node = {
	.name = "usr_min_lmt",
};

static struct cpufreq_lmt_node usr_max_lmt_node = {
	.name = "usr_max_lmt",
};

static int kona_cpufreq_freq_stats_update(int new_freq_inx)
{
	struct kona_cpufreq_stats *stats;
	int cpu, num_cstates;
	unsigned long long cur_time;

	BUG_ON(!kona_cpufreq);
	num_cstates = kona_cpufreq->num_cstates;
	spin_lock(&kona_cpufreq->kcf_lock);
	stats = &kona_cpufreq->stats;
	cur_time = get_jiffies_64();
	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
		if (stats->cpu_in_idle[cpu] == CPU_ACTIVE) {
			stats->stats[cpu][stats->act_freq_inx] +=
				cur_time - stats->last_time[cpu];
			stats->last_time[cpu] = cur_time;
		}
	}
	stats->act_freq_inx = new_freq_inx;
	spin_unlock(&kona_cpufreq->kcf_lock);
	return 0;
}

static int kona_cpufreq_cstate_stats_update(int cpu, int state, int enter)
{
	struct kona_cpufreq_stats *stats;
	unsigned long long cur_time;

	BUG_ON(!kona_cpufreq);
	stats = &kona_cpufreq->stats;
	cur_time = get_jiffies_64();
	spin_lock(&kona_cpufreq->kcf_lock);
	if (enter) {
		stats->stats[cpu][stats->act_freq_inx] +=
				cur_time - stats->last_time[cpu];
	} else {
		stats->stats[cpu][kona_cpufreq->no_of_opps + state] +=
			cur_time - stats->last_time[cpu];
	}
	stats->last_time[cpu] = cur_time;
	spin_unlock(&kona_cpufreq->kcf_lock);
	return 0;
}

static int cpufreq_cstate_notify_handler(struct notifier_block *nb,
					     unsigned long val,
					     void *data)
{
	struct kona_cpufreq_stats *stats;
	int cpu = get_cpu();
	int state = *(int *)data;

	BUG_ON(!kona_cpufreq);

	stats = &kona_cpufreq->stats;
	switch (val) {
	case CSTATE_ENTER:
		stats->cpu_in_idle[cpu] = state;
		if (stats->stats_en)
			kona_cpufreq_cstate_stats_update(cpu, state, 1);
		break;
	case CSTATE_EXIT:
		stats->cpu_in_idle[cpu] = CPU_ACTIVE;
		if (stats->stats_en)
			kona_cpufreq_cstate_stats_update(cpu, state, 0);
		break;
	}
	put_cpu();
	return 0;
}

static struct notifier_block kona_cpufreq_cstate_nb = {
	.notifier_call = cpufreq_cstate_notify_handler,
};

static int kona_cpufreq_freq_stats_enable(int en)
{
	struct kona_cpufreq_stats *stats;
	int i, cpu, num_cstates;
	int freq_inx = -1;
	unsigned long long cur_time;
	struct cpufreq_policy *pol;

	BUG_ON(!kona_cpufreq);
	stats = &kona_cpufreq->stats;
	num_cstates = kona_cpufreq->num_cstates;
	cur_time = get_jiffies_64();
	if (en && !stats->stats_en) {
		pol = cpufreq_cpu_get(get_cpu());
		for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
			stats->last_time[cpu] = cur_time;
			for (i = 0; i < kona_cpufreq->no_of_opps + num_cstates;
					i++) {
				stats->stats[cpu][i] = 0;
				if (i < kona_cpufreq->no_of_opps && pol &&
				pol->cur == kona_cpufreq->freq_map[i].cpu_freq)
					freq_inx = i;
			}
		}
		BUG_ON(freq_inx == -1);
		stats->start_time = cur_time;
		stats->act_freq_inx = (u32)freq_inx;
		cpufreq_cpu_put(pol);
		put_cpu();
		stats->stats_en = 1;
		kona_cpufreq_freq_stats_update(freq_inx);
		cstate_notifier_register(&kona_cpufreq_cstate_nb);
	} else if (!en && stats->stats_en) {
		cstate_notifier_unregister(&kona_cpufreq_cstate_nb);
		stats->stats_en = 0;
	}
	return 0;
}

static u32 cpufreq_stats_percentage(unsigned long long cur_time,
		unsigned long long opp_time)
{
	unsigned long long total_time;
	struct kona_cpufreq_stats *stats;
	stats = &kona_cpufreq->stats;
	total_time = cur_time - stats->start_time;
	opp_time *= 100;
	do_div(opp_time, total_time);
	return (u32)opp_time;
}

static ssize_t kona_cpufreq_log_stats(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct kona_cpufreq_stats *stats;
	int i, cpu, num_cstates;
	unsigned long long cur_time;
	unsigned long long act_time;
	static char buf[2048];
	int len = 0;

	BUG_ON(!kona_cpufreq);
	stats = &kona_cpufreq->stats;
	if (!stats->stats_en)
		return -EINVAL;

	num_cstates = kona_cpufreq->num_cstates;
	for (i = 0; i < kona_cpufreq->no_of_opps; i++) {
		len += snprintf(buf+len, sizeof(buf)-len,
			"%10uKHz   ", kona_cpufreq->freq_map[i].cpu_freq);
	}
	for (i = 0; i < num_cstates; i++)
		len += snprintf(buf+len, sizeof(buf)-len, "%9s\t",
				kona_pm_get_cstate_name(i));
	len += snprintf(buf+len, sizeof(buf)-len, "\n");

	cur_time = get_jiffies_64();
	for (cpu = 0; cpu < CONFIG_NR_CPUS; cpu++) {
		for (i = 0; i < kona_cpufreq->no_of_opps + num_cstates; i++) {
			if (i >= kona_cpufreq->no_of_opps &&
				stats->cpu_in_idle[cpu] == (i -
					kona_cpufreq->no_of_opps)) {
				cur_time = get_jiffies_64();
				act_time = stats->stats[cpu][i] +
					(cur_time - stats->last_time[cpu]);
				goto percentage;
			} else if (i == stats->act_freq_inx) {
				if (stats->cpu_in_idle[cpu] == CPU_ACTIVE) {
					cur_time = get_jiffies_64();
					act_time = stats->stats[cpu][i] +
					(cur_time - stats->last_time[cpu]);
					goto percentage;
				}
			}
			act_time = stats->stats[cpu][i];
percentage:
			len += snprintf(buf+len, sizeof(buf)-len, "%8llu"\
				"(%u%c)\t", act_time, cpufreq_stats_percentage(
				cur_time, act_time), '%');
		}
		len += snprintf(buf+len, sizeof(buf)-len, "\n");
	}
	len += snprintf(buf+len, sizeof(buf)-len, "Total Time: %llu\n",
			cur_time - stats->start_time);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

#ifdef CONFIG_KONA_TMON
static int cpufreq_set_init_thold_freq(struct kona_cpufreq *kona_cpufreq)
{
	int i, j;
	long max_temp, curr_temp;
	struct kona_cpufreq_drv_pdata *pdata;
	pdata = kona_cpufreq->pdata;

	curr_temp = tmon_get_current_temp(CELCIUS, true);

	for (i = pdata->num_freqs - 1; i > 0; i--) {
		if (pdata->freq_tbl[i].max_temp != TEMP_DONT_CARE) {
			kona_cpufreq->r_inx = i;
			break;
		}
	}

	kona_cpufreq->f_inx = INVALID_INX;
	for (i = 0; i < pdata->num_freqs; i++) {
		max_temp = pdata->freq_tbl[i].max_temp;
		if (curr_temp >= max_temp &&
				TEMP_DONT_CARE != max_temp) {
			BUG_ON(i == 0);
			cpufreq_update_lmt_req(&kona_cpufreq->tmon_node,
				pdata->freq_tbl[i-1].cpu_freq);
			for (j = i - 1; j > 0; j--) {
				max_temp = pdata->freq_tbl[j].max_temp;
				if (max_temp != TEMP_DONT_CARE) {
					kona_cpufreq->r_inx = j;
					break;
				}
			}
			if (!j)
				kona_cpufreq->r_inx = INVALID_INX;
			kona_cpufreq->f_inx = i;
			break;
		}
	}
	return 0;
}

static void cpufreq_tmon_init_work(struct work_struct *ws)
{
	struct kona_cpufreq_drv_pdata *pdata;
	struct kona_cpufreq *kona_cpufreq;

	kona_cpufreq = container_of((struct delayed_work *)ws,
			struct kona_cpufreq, init_work);
	pdata = kona_cpufreq->pdata;

	if (cpufreq_add_lmt_req(&kona_cpufreq->tmon_node,
			"tmon", DEFAULT_LIMIT, MAX_LIMIT)) {
		printk(KERN_ALERT "TMON REGN CPUFREQ FAILED\n");
		goto out;
	}
	cpufreq_set_init_thold_freq(kona_cpufreq);
	tmon_register_notifier(&kona_cpufreq->tmon_nb);
out:
	kona_cpufreq = NULL;
}

static int cpufreq_tmon_notify_handler(struct notifier_block *nb,
		unsigned long curr_temp, void *dev)
{
	int i;
	struct kona_cpufreq *kona_cpufreq = container_of(nb,
			struct kona_cpufreq, tmon_nb);
	struct kona_cpufreq_drv_pdata *pdata = kona_cpufreq->pdata;
	int f_inx = kona_cpufreq->f_inx;
	int r_inx = kona_cpufreq->r_inx;

	if ((f_inx != INVALID_INX) &&
		curr_temp < pdata->freq_tbl[f_inx].max_temp) {
		cpufreq_update_lmt_req(&kona_cpufreq->tmon_node,
				pdata->freq_tbl[f_inx].cpu_freq);

		printk(KERN_ALERT "CPU FREQ: BBIC, Threshold Crossed: %ld\n",
			pdata->freq_tbl[f_inx].max_temp);
		printk(KERN_ALERT "CPU FREQ: Dir: Fall, Action:Freq : %u\n",
			pdata->freq_tbl[f_inx].cpu_freq);

		kona_cpufreq->r_inx = f_inx;
		for (i = f_inx + 1; i < pdata->num_freqs; i++) {
			if (pdata->freq_tbl[i].max_temp != TEMP_DONT_CARE) {
				kona_cpufreq->f_inx = i;
				break;
			}
		}
		if (i == pdata->num_freqs)
			kona_cpufreq->f_inx = INVALID_INX;
	} else if ((r_inx != INVALID_INX) &&
			curr_temp >= pdata->freq_tbl[r_inx].max_temp) {
		BUG_ON(r_inx == 0);
		cpufreq_update_lmt_req(&kona_cpufreq->tmon_node,
				pdata->freq_tbl[r_inx - 1].cpu_freq);

		printk(KERN_ALERT "CPU FREQ: BBIC, Threshold Crossed: %ld\n",
			pdata->freq_tbl[r_inx].max_temp);
		printk(KERN_ALERT "CPU FREQ: Dir: Rise, Action:Freq : %u\n",
			pdata->freq_tbl[r_inx - 1].cpu_freq);

		kona_cpufreq->f_inx = r_inx;
		for (i = r_inx - 1; i > 0; i--) {
			if (pdata->freq_tbl[i].max_temp != TEMP_DONT_CARE) {
				kona_cpufreq->r_inx = i;
				break;
			}
		}
		if (!i)
			kona_cpufreq->r_inx = INVALID_INX;
	}
	return 0;
}
#endif

/*********************************************************************
 *                   CPUFREQ TABLE MANIPULATION                      *
 *********************************************************************/

/* Create and populate cpu freqs table. The memory for the table must
 * be statically allocated.
 */
static int kona_create_cpufreqs_table(struct cpufreq_policy *policy,
				      struct cpufreq_frequency_table *t)
{
	struct kona_cpufreq_drv_pdata *pdata = kona_cpufreq->pdata;
	int i, num;

	num = pdata->num_freqs;
	kcf_dbg("%s: num_freqs: %d\n", __func__, num);

	for (i = 0; i < num; i++) {
		t[i].index = i;
		t[i].frequency = pdata->freq_tbl[i].cpu_freq;
		kcf_dbg("%s: index: %d, freq: %u\n", __func__, t[i].index,
			t[i].frequency);
	}
	t[num].index = i;
	t[num].frequency = CPUFREQ_TABLE_END;

	return 0;
}

/*********************************************************************
 *                       CPUFREQ CORE INTERFACE                      *
 *********************************************************************/

static unsigned int kona_cpufreq_get_speed(unsigned int cpu)
{
	int opp;
	int i;

	opp = pi_get_active_opp(kona_cpufreq->pi_id);
	kcf_dbg("%s: opp = %d\n", __func__, opp);

	if (opp < 0)
		return 0;
	for (i = 0; i < kona_cpufreq->no_of_opps; i++) {
		if (kona_cpufreq->freq_map[i].opp == opp) {
			kcf_dbg("opp found, return corresponding freq %u\n",
				kona_cpufreq->freq_map[i].cpu_freq);
			return kona_cpufreq->freq_map[i].cpu_freq;
		}
	}
	kcf_dbg
	    ("Since the table is setup with all OPP, ctrl shouldnt reach here\n");
	BUG();
	return 0;
}

static int get_cpufreq_lmt(int max)
{
	if (max && !plist_head_empty(&kona_cpufreq->max_lmt_list))
		return plist_first(&kona_cpufreq->max_lmt_list)->prio;
	else if (!max && !plist_head_empty(&kona_cpufreq->min_lmt_list))
		return plist_last(&kona_cpufreq->min_lmt_list)->prio;
	return 0;
}

static int kona_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	int ret = -EINVAL;
	int max_lmt, min_lmt;

	if (kona_cpufreq->kona_freqs_table)
		ret = cpufreq_frequency_table_verify(policy,
						     kona_cpufreq->
						     kona_freqs_table);
	min_lmt = get_cpufreq_lmt(0);
	max_lmt = get_cpufreq_lmt(1);
	if (max_lmt && policy->max > max_lmt)
		policy->max = max_lmt;
	else if (min_lmt && policy->min < min_lmt)
		policy->min = min_lmt;

	kcf_dbg("%s: after cpufreq verify: min:%d->max:%d kHz\n",
		__func__, policy->min, policy->max);

	return ret;
}

static int kona_cpufreq_set_speed(struct cpufreq_policy *policy,
				  unsigned int target_freq,
				  unsigned int relation)
{
	struct cpufreq_freqs freqs;
	int i, index;
	int ret = 0;
	int freq_inx = -1;
	u32 opp = PI_OPP_NORMAL;
	struct kona_cpufreq_stats *stats;
#ifdef CONFIG_SMP
	struct kona_cpufreq_drv_pdata *pdata = kona_cpufreq->pdata;
#endif
	/* Lookup the next frequency */
	if (cpufreq_frequency_table_target
	    (policy, kona_cpufreq->kona_freqs_table, target_freq, relation,
	     &index)) {
		return -EINVAL;
	}
	freqs.old = kona_cpufreq_get_speed(policy->cpu);
	freqs.new = kona_cpufreq->kona_freqs_table[index].frequency;
	freqs.cpu = policy->cpu;

	if (freqs.old == freqs.new)
		return 0;

	kcf_dbg("%s: cpu freq change: %u --> %u\n", __func__, freqs.old,
		freqs.new);

	cpufreq_notify_transition(policy, &freqs, CPUFREQ_PRECHANGE);
	local_irq_disable();

	for (i = 0; i < kona_cpufreq->no_of_opps; i++) {
		if (freqs.new == kona_cpufreq->freq_map[i].cpu_freq) {
			opp = kona_cpufreq->freq_map[i].opp;
			freq_inx = i;
			break;
		}
	}
	ret = pi_mgr_dfs_request_update(&kona_cpufreq->dfs_node, opp);

	if (unlikely(ret)) {
		kcf_dbg("%s: cpu freq change failed : %d\n", __func__, ret);
	}

	stats = &kona_cpufreq->stats;
	BUG_ON(freq_inx  == -1);
	if (stats->stats_en)
		kona_cpufreq_freq_stats_update(freq_inx);

	local_irq_enable();

#ifdef CONFIG_SMP
	if (pdata->flags & KONA_CPUFREQ_UPDATE_LPJ) {
		int i;
		for_each_online_cpu(i) {
			per_cpu(cpu_data, i).loops_per_jiffy =
			    cpufreq_scale(kona_cpufreq->l_p_j_ref,
					  kona_cpufreq->l_p_j_ref_freq,
					  freqs.new);
		}
	}
#endif
	cpufreq_notify_transition(policy, &freqs, CPUFREQ_POSTCHANGE);

	return ret;
}

static int kona_cpufreq_init(struct cpufreq_policy *policy)
{
	struct kona_cpufreq_drv_pdata *pdata = kona_cpufreq->pdata;
	int ret, i;

	kcf_dbg("%s\n", __func__);

	/* Get handle to cpu private data */

	/* Set default policy and cpuinfo */
	policy->cur = kona_cpufreq_get_speed(policy->cpu);
	pr_info("%s:policy->cur = %d\n", __func__, policy->cur);

	policy->cpuinfo.transition_latency = pdata->latency;

	for (i = 0; i < pdata->num_freqs; i++) {
		kcf_dbg("index: %d, freq: %u opp:%d\n",
			i, pdata->freq_tbl[i].cpu_freq, pdata->freq_tbl[i].opp);
	}

	ret =
	    kona_create_cpufreqs_table(policy, kona_cpufreq->kona_freqs_table);
	if (ret) {
		kcf_dbg("%s: setup_cpufreqs_table failed: %d\n", __func__, ret);
		goto err_cpufreqs_table;
	}

	ret =
	    cpufreq_frequency_table_cpuinfo(policy,
					    kona_cpufreq->kona_freqs_table);
	if (ret) {
		kcf_dbg("%s: cpufreq_frequency_table_cpuinfo failed\n",
			__func__);
		goto err_cpufreqs_table;
	}
	cpufreq_frequency_table_get_attr(kona_cpufreq->kona_freqs_table,
					 policy->cpu);
	policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
	cpumask_copy(policy->related_cpus, cpu_possible_mask);

	kona_cpufreq->policy = policy;

#ifdef CONFIG_SMP
	/*
	 * Multi-core processors where
	 * the frequency cannot be set independently for each core.
	 * Each cpu is bound to the same speed.
	 * So the affected cpu is all of the cpus.
	 */
	cpumask_setall(policy->cpus);

	if (kona_cpufreq->l_p_j_ref == 0
	    && (pdata->flags & KONA_CPUFREQ_UPDATE_LPJ)) {
		kona_cpufreq->l_p_j_ref = per_cpu(cpu_data, 0).loops_per_jiffy;
		kona_cpufreq->l_p_j_ref_freq = policy->cur;
	}
#endif

	return 0;

      err_cpufreqs_table:
	return ret;
}

static int kona_cpufreq_exit(struct cpufreq_policy *policy)
{
	kcf_dbg("%s\n", __func__);

	cpufreq_frequency_table_put_attr(policy->cpu);

	return 0;
}

u32 get_cpu_freq_from_opp(int opp)
{
	int i, ret = 0;
	struct cpufreq_policy policy;
	int cpu = get_cpu();
	ret = cpufreq_get_policy(&policy, cpu);
	put_cpu();

	if (ret) {
		pr_err("%s:cpufreq not initialized yet\n", __func__);
		ret = 0;
		goto over;
	}

	if (opp < 0) {
		pr_err("%s: Invalid OPP: %d", __func__, opp);
		goto over;
	}

	for (i = 0; i < kona_cpufreq->no_of_opps; i++)
		if (kona_cpufreq->freq_map[i].opp == opp) {
			ret = kona_cpufreq->freq_map[i].cpu_freq;
			break;
		}
over:
	return ret;
}

int get_cpufreq_limit(unsigned int *val, int limit_type)
{
	int ret = 0;
	struct cpufreq_policy policy;
	int cpu	= get_cpu();
	ret = cpufreq_get_policy(&policy, cpu);
	put_cpu();

	if (ret)
		goto over;
	switch (limit_type) {
	case MAX_LIMIT:
		*val = policy.max;
		break;
	case MIN_LIMIT:
		*val = policy.min;
		break;
	case CURRENT_FREQ:
		*val = policy.cur;
		break;
	default:
		ret = -EINVAL;
	}
over:
	return ret;
}

int set_cpufreq_limit(unsigned int val, int limit_type)
{
	struct cpufreq_policy *policy;
	int ret = 0;
	int cpu = get_cpu();

	if (limit_type != MAX_LIMIT && limit_type != MIN_LIMIT) {
		pr_err("%s: Invalid Limit Type", __func__);
		ret = -EINVAL;
		goto over;
	}
	policy = cpufreq_cpu_get(cpu);
	if (!policy) {
		pr_err("%s:cpufreq not initialized yet\n", __func__);
		ret = -EINVAL;
		goto over;
	}
	kcf_dbg("%s: val:%u  limit_type: %s\n", __func__, val,
				limit_type ? "Max" : "Min");
	if (limit_type == MAX_LIMIT)
		policy->user_policy.max = val;
	else
		policy->user_policy.min = val;

	cpufreq_cpu_put(policy);
	cpufreq_update_policy(cpu);
over:
	put_cpu();
	return ret;
}

static int cpufreq_min_lmt_update(struct cpufreq_lmt_node *lmt_node, int action)
{
	int new_val;
	int ret = 0;
	struct cpufreq_policy *policy;
	int cpu = get_cpu();
	policy = cpufreq_cpu_get(cpu);
	put_cpu();

	if (!policy) {
		pr_err("%s:cpufreq not initialized yet\n", __func__);
		ret = -EINVAL;
		goto over;
	}
	if (lmt_node->lmt == DEFAULT_LIMIT)
		lmt_node->lmt = (int)policy->cpuinfo.min_freq;
	cpufreq_cpu_put(policy);

	spin_lock(&kona_cpufreq->kcf_lock);
	switch (action) {
	case FREQ_LMT_NODE_ADD:
		plist_node_init(&lmt_node->node, lmt_node->lmt);
		plist_add(&lmt_node->node, &kona_cpufreq->min_lmt_list);
		break;
	case FREQ_LMT_NODE_DEL:
		plist_del(&lmt_node->node, &kona_cpufreq->min_lmt_list);
		break;
	case FREQ_LMT_NODE_UPDATE:
		plist_del(&lmt_node->node, &kona_cpufreq->min_lmt_list);
		plist_node_init(&lmt_node->node, lmt_node->lmt);
		plist_add(&lmt_node->node, &kona_cpufreq->min_lmt_list);
		break;
	default:
		BUG();
	}
	new_val = plist_last(&kona_cpufreq->min_lmt_list)->prio;
		if (new_val != kona_cpufreq->active_min_lmt) {
			ret = set_cpufreq_limit(new_val, MIN_LIMIT);
			if (!ret)
				kona_cpufreq->active_min_lmt = new_val;
	}
	spin_unlock(&kona_cpufreq->kcf_lock);
over:
	return ret;
}

static int cpufreq_max_lmt_update(struct cpufreq_lmt_node *lmt_node, int action)
{
	int new_val;
	int ret = 0;
	struct cpufreq_policy *policy;
	int cpu = get_cpu();
	policy = cpufreq_cpu_get(cpu);
	put_cpu();

	if (!policy) {
		pr_err("%s:cpufreq not initialized yet\n", __func__);
		ret = -EINVAL;
		goto over;
	}
	if (lmt_node->lmt == DEFAULT_LIMIT)
		lmt_node->lmt = (int)policy->cpuinfo.max_freq;
	cpufreq_cpu_put(policy);

	spin_lock(&kona_cpufreq->kcf_lock);
	switch (action) {
	case FREQ_LMT_NODE_ADD:
		plist_node_init(&lmt_node->node, lmt_node->lmt);
		plist_add(&lmt_node->node, &kona_cpufreq->max_lmt_list);
		break;
	case FREQ_LMT_NODE_DEL:
		plist_del(&lmt_node->node, &kona_cpufreq->max_lmt_list);
		break;
	case FREQ_LMT_NODE_UPDATE:
		plist_del(&lmt_node->node, &kona_cpufreq->max_lmt_list);
		plist_node_init(&lmt_node->node, lmt_node->lmt);
		plist_add(&lmt_node->node, &kona_cpufreq->max_lmt_list);
		break;
	default:
		BUG();
	}
	new_val = plist_first(&kona_cpufreq->max_lmt_list)->prio;
	if (new_val != kona_cpufreq->active_max_lmt) {
		ret = set_cpufreq_limit(new_val, MAX_LIMIT);
		if (!ret)
			kona_cpufreq->active_max_lmt = new_val;
	}
	spin_unlock(&kona_cpufreq->kcf_lock);
over:
	return ret;
}

int cpufreq_add_lmt_req(struct cpufreq_lmt_node *lmt_node,
		char *client_name, int lmt, int lmt_typ)
{
	BUG_ON(lmt_node->valid);

	if (lmt_typ != MAX_LIMIT && lmt_typ != MIN_LIMIT)
		return -EINVAL;

	lmt_node->lmt = lmt;
	lmt_node->name = client_name;
	lmt_node->lmt_typ = lmt_typ;
	lmt_node->valid = 1;

	if (lmt_typ == MAX_LIMIT)
		return cpufreq_max_lmt_update(lmt_node, FREQ_LMT_NODE_ADD);
	else
		return cpufreq_min_lmt_update(lmt_node, FREQ_LMT_NODE_ADD);

}
EXPORT_SYMBOL(cpufreq_add_lmt_req);

int cpufreq_del_lmt_req(struct cpufreq_lmt_node *lmt_node)
{
	int ret;
	BUG_ON(lmt_node->valid == 0);

	if (lmt_node->lmt_typ == MIN_LIMIT)
		ret = cpufreq_min_lmt_update(lmt_node, FREQ_LMT_NODE_DEL);
	else
		ret = cpufreq_max_lmt_update(lmt_node, FREQ_LMT_NODE_DEL);

	lmt_node->valid = 0;
	lmt_node->name = NULL;
	return ret;
}
EXPORT_SYMBOL(cpufreq_del_lmt_req);

int cpufreq_update_lmt_req(struct cpufreq_lmt_node *lmt_node, int lmt)
{
	int ret = 0;
	BUG_ON(lmt_node->valid == 0);

	if (lmt_node->lmt != lmt) {
		lmt_node->lmt = lmt;
		if (lmt_node->lmt_typ == MIN_LIMIT)
			ret =
			cpufreq_min_lmt_update(lmt_node, FREQ_LMT_NODE_UPDATE);
		else
			ret =
			cpufreq_max_lmt_update(lmt_node, FREQ_LMT_NODE_UPDATE);
	}
	return ret;
}
EXPORT_SYMBOL(cpufreq_update_lmt_req);

static int cpufreq_update_usr_lmt_req(int val, int lmt_typ)
{

	if (lmt_typ != MAX_LIMIT && lmt_typ != MIN_LIMIT)
		return -EINVAL;

	if (lmt_typ == MAX_LIMIT) {
		if (!usr_max_lmt_node.valid)
			cpufreq_add_lmt_req(&usr_max_lmt_node,
			usr_max_lmt_node.name, val, lmt_typ);
		else
			cpufreq_update_lmt_req(&usr_max_lmt_node, val);
	} else {
		if (!usr_min_lmt_node.valid)
			cpufreq_add_lmt_req(&usr_min_lmt_node,
			usr_min_lmt_node.name, val, lmt_typ);
		else
			cpufreq_update_lmt_req(&usr_min_lmt_node, val);
	}
	return 0;
}

#ifdef CONFIG_KONA_CPU_FREQ_LIMITS

#define kona_cpufreq_power_attr(_name, _mode)	\
static struct kobj_attribute _name##_attr = {	\
	.attr	= {				\
		.name = __stringify(_name),	\
		.mode = _mode,			\
	},					\
	.show	= _name##_show,			\
	.store	= _name##_store,		\
}

#define kona_cpufreq_show(fname, lmt_typ)			\
	static ssize_t fname##_show(struct kobject *kobj,	\
			struct kobj_attribute *attr, char *buf)	\
{								\
	ssize_t ret = -EINTR;					\
	unsigned int val;					\
	if (!get_cpufreq_limit(&val, lmt_typ))			\
		ret = scnprintf(buf, PAGE_SIZE-1, "%u\n", val);		\
	return ret;						\
}


#define kona_cpufreq_store(fname, lmt_typ)			\
static ssize_t fname##_store(struct kobject *kobj,		\
	struct kobj_attribute *attr, const char *buf, size_t n)	\
{								\
	long val;                                               \
	if (strict_strtol(buf, 10, &val))			\
		return -EINVAL;					\
	cpufreq_update_usr_lmt_req(val, lmt_typ);		\
	return n;						\
}

static ssize_t cpufreq_table_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)

{
	ssize_t count = 0;
	int num, i;
	struct kona_cpufreq_drv_pdata *pdata = kona_cpufreq->pdata;
	BUG_ON(pdata == NULL);
	num = pdata->num_freqs;
	/*List in descending order*/
	for (i = num - 1; i >= 0; i--) {
		count += scnprintf(&buf[count], (PAGE_SIZE - count - 2), "%d ",
				pdata->freq_tbl[i].cpu_freq);
	}
	count += snprintf(&buf[count], (PAGE_SIZE - count - 2), "\n");
	return count;
}

#define cpufreq_table_store	NULL
#define cpufreq_cur_store	NULL

kona_cpufreq_show(cpufreq_min_limit, MIN_LIMIT);
kona_cpufreq_show(cpufreq_cur, CURRENT_FREQ);
kona_cpufreq_show(cpufreq_max_limit, MAX_LIMIT);

kona_cpufreq_store(cpufreq_min_limit, MIN_LIMIT);
kona_cpufreq_store(cpufreq_max_limit, MAX_LIMIT);

kona_cpufreq_power_attr(cpufreq_max_limit, S_IRUGO|S_IWUSR);
kona_cpufreq_power_attr(cpufreq_min_limit, S_IRUGO|S_IWUSR);
kona_cpufreq_power_attr(cpufreq_cur, S_IRUGO);
kona_cpufreq_power_attr(cpufreq_table, S_IRUGO);

static struct attribute *_cpufreq_attr[] = {
	&cpufreq_max_limit_attr.attr,
	&cpufreq_min_limit_attr.attr,
	&cpufreq_cur_attr.attr,
	&cpufreq_table_attr.attr,
	NULL,
};

static struct attribute_group _cpufreq_attr_group = {
	.attrs = _cpufreq_attr,
};



#endif /*CONFIG_KONA_CPU_FREQ_LIMITS*/

/*********************************************************************
 *                              INIT CODE                            *
 *********************************************************************/

static struct freq_attr *kona_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver kona_cpufreq_driver = {
	.name = "kona",
	.flags = 0,
	.init = kona_cpufreq_init,
	.verify = kona_cpufreq_verify_speed,
	.target = kona_cpufreq_set_speed,
	.get = kona_cpufreq_get_speed,
	.exit = kona_cpufreq_exit,
	.attr = kona_cpufreq_attr,
	.owner = THIS_MODULE,
};

static int cpufreq_drv_probe(struct platform_device *pdev)
{
	struct kona_cpufreq_stats *stats;
	struct kona_cpufreq_drv_pdata *pdata = pdev->dev.platform_data;
	int i, ret = -1;

	kcf_dbg("%s\n", __func__);
	BUG_ON(pdata == NULL);
	/* allocate memory for per-cpu data for all cpus */
	kona_cpufreq = kzalloc(sizeof(struct kona_cpufreq), GFP_KERNEL);
	if (!kona_cpufreq) {
		kcf_dbg("%s: kzalloc failed for kona_cpufreq\n", __func__);
		return -ENOMEM;
	}
	memset(kona_cpufreq, 0, sizeof(struct kona_cpufreq));
	spin_lock_init(&kona_cpufreq->kcf_lock);
	plist_head_init(&kona_cpufreq->min_lmt_list);
	plist_head_init(&kona_cpufreq->max_lmt_list);


	kona_cpufreq->freq_map = kzalloc(pdata->num_freqs *
					 sizeof(struct kona_freq_map),
					 GFP_KERNEL);
	if (!kona_cpufreq->freq_map) {
		kcf_dbg("%s: kzalloc failed for freq_map\n", __func__);
		kfree(kona_cpufreq);
		return -ENOMEM;
	}
	kona_cpufreq->kona_freqs_table = kzalloc((pdata->num_freqs +
						  1) *
						 sizeof(struct
							cpufreq_frequency_table),
						 GFP_KERNEL);
	if (!kona_cpufreq->kona_freqs_table) {
		kcf_dbg("%s: kzalloc failed for kona_freqs_table\n", __func__);
		kfree(kona_cpufreq->freq_map);
		kfree(kona_cpufreq);
		return -ENOMEM;
	}
	/*Invlide init callback function if valid */
	if (pdata->cpufreq_init)
		pdata->cpufreq_init();

	kona_cpufreq->pi_id = pdata->pi_id;
	kona_cpufreq->no_of_opps = pdata->num_freqs;
	for (i = 0; i < kona_cpufreq->no_of_opps; i++) {
		kona_cpufreq->freq_map[i].cpu_freq =
		    pdata->freq_tbl[i].cpu_freq;
		kona_cpufreq->freq_map[i].opp = pdata->freq_tbl[i].opp;
	}
	kona_cpufreq->active_min_lmt = kona_cpufreq->freq_map[0].cpu_freq;
	kona_cpufreq->active_max_lmt =
		kona_cpufreq->freq_map[kona_cpufreq->no_of_opps-1].cpu_freq;

	/*Add a DFS client for ARM CCU. this client will be used later
	   for changinf ARM freq via cpu-freq. */
	ret =
	    pi_mgr_dfs_add_request(&kona_cpufreq->dfs_node, "cpu_freq",
				   kona_cpufreq->pi_id,
				   pi_get_active_opp(kona_cpufreq->pi_id));
	if (ret) {
		kcf_dbg("Failed add dfs request for CPU\n");
		kfree(kona_cpufreq->kona_freqs_table);
		kfree(kona_cpufreq->freq_map);
		kfree(kona_cpufreq);
		return -ENOMEM;
	}

	kona_cpufreq->pdata = pdata;
	platform_set_drvdata(pdev, kona_cpufreq);

#ifdef CONFIG_KONA_CPU_FREQ_LIMITS
	ret = sysfs_create_group(power_kobj, &_cpufreq_attr_group);
	if (ret) {
		pr_info("%s:sysfs_create_group failed\n", __func__);
		return ret;
	}
#endif
	ret = cpufreq_register_driver(&kona_cpufreq_driver);

#ifdef CONFIG_KONA_TMON
	if (pdata->flags & KONA_CPUFREQ_TMON) {
		INIT_DELAYED_WORK(&kona_cpufreq->init_work,
				cpufreq_tmon_init_work);
		kona_cpufreq->tmon_nb.notifier_call =
			cpufreq_tmon_notify_handler;
		schedule_delayed_work(&kona_cpufreq->init_work,
				msecs_to_jiffies(INIT_WORK_DELAY));
	}
#endif
	kona_cpufreq->num_cstates = kona_pm_get_num_cstates();
	pr_info("%s: num_cstates: %d\n", __func__, kona_cpufreq->num_cstates);
	stats = &kona_cpufreq->stats;
	stats->stats_en = 0;
	for (i = 0; i < CONFIG_NR_CPUS; i++)
		stats->stats[i] = kzalloc(sizeof(u64) * ((pdata->num_freqs) +
				kona_cpufreq->num_cstates), GFP_KERNEL);

	return ret;
}

static int cpufreq_drv_remove(struct platform_device *pdev)
{
	int ret = 0;

	if (kona_cpufreq->stats.stats_en)
		cstate_notifier_unregister(&kona_cpufreq_cstate_nb);

	if (cpufreq_unregister_driver(&kona_cpufreq_driver) != 0)
		kcf_dbg("%s: cpufreq unregister failed\n", __func__);

	ret = pi_mgr_dfs_request_remove(&kona_cpufreq->dfs_node);
	if (ret)
		kcf_dbg("%s: dfs remove request failed\n", __func__);

	kfree(kona_cpufreq->kona_freqs_table);
	kfree(kona_cpufreq->freq_map);
	kfree(kona_cpufreq);
	kona_cpufreq = NULL;

	return 0;
}

static struct platform_driver cpufreq_driver = {
	.probe = cpufreq_drv_probe,
	.remove = cpufreq_drv_remove,
	.driver = {
		   .name = "kona-cpufreq-drv",
		   },
};

static int __init cpufreq_drv_init(void)
{
	return platform_driver_register(&cpufreq_driver);
}

module_init(cpufreq_drv_init);

static void __exit cpufreq_drv_exit(void)
{
	platform_driver_unregister(&cpufreq_driver);
}

module_exit(cpufreq_drv_exit);

#ifdef CONFIG_DEBUG_FS

static int cpufreq_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t kona_cpufreq_bogmips_get(struct file *file,
					char __user *user_buf, size_t count,
					loff_t *ppos)
{

	/* re-used from init/calibrate.c
	 *
	 *  Copyright (C) 1991, 1992  Linus Torvalds
	 */
#define LPS_PREC 8

	unsigned long lpj;
	unsigned long ticks, loopbit;
	int lps_precision = LPS_PREC;
	char buf[100];
	u32 len = 0;

	lpj = (1 << 12);
	while ((lpj <<= 1) != 0) {
		/* wait for "start of" clock tick */
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */ ;
		/* Go .. */
		ticks = jiffies;
		__delay(lpj);
		ticks = jiffies - ticks;
		if (ticks)
			break;
	}

	/*
	 * Do a binary approximation to get lpj set to
	 * equal one clock (up to lps_precision bits)
	 */
	lpj >>= 1;
	loopbit = lpj;
	while (lps_precision-- && (loopbit >>= 1)) {
		lpj |= loopbit;
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */ ;
		ticks = jiffies;
		__delay(lpj);
		if (jiffies != ticks)	/* longer than 1 tick */
			lpj &= ~loopbit;
	}
	len += snprintf(buf + len, sizeof(buf) - len,
			"%lu.%02lu BogoMIPS (lpj=%lu)\n", lpj / (500000 / HZ),
			(lpj / (5000 / HZ)) % 100, lpj);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static struct file_operations bogo_mips_fops = {
	.read = kona_cpufreq_bogmips_get,
};

char debug_fs_buf[1000];

static ssize_t cpufreq_read_requests(struct file *file,
			char __user *user_buf, size_t count, loff_t *ppos)
{
	u32 len = 0;
	struct kona_cpufreq *cpufreq = (struct kona_cpufreq *)
			file->private_data;
	struct cpufreq_lmt_node *lmt_node;

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
		"***** CPUFREQ min limit requests ******\n");
	plist_for_each_entry(lmt_node, &cpufreq->min_lmt_list, node) {
		len += snprintf(debug_fs_buf + len,
				sizeof(debug_fs_buf) - len,
				"Name: %s, limit: %d, valid: %d\n",
				lmt_node->name, lmt_node->lmt,
				lmt_node->valid);
	}

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
		"***** CPUFREQ max limit requests ******\n");
	plist_for_each_entry(lmt_node, &cpufreq->max_lmt_list, node) {
		len += snprintf(debug_fs_buf + len,
				sizeof(debug_fs_buf) - len,
				"Name: %s, limit: %d, valid: %d\n",
				lmt_node->name, lmt_node->lmt,
				lmt_node->valid);
	}

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations request_list_ops = {
	.open = cpufreq_debugfs_open,
	.read = cpufreq_read_requests,
};

static int kona_cpufreq_set_stats_en(void *data, u64 val)
{
	return kona_cpufreq_freq_stats_enable(!!val);
}

static int kona_cpufreq_get_stats_en(void *data, u64 *val)
{
	*val = kona_cpufreq->stats.stats_en;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(stats_en_ops, kona_cpufreq_get_stats_en,
		kona_cpufreq_set_stats_en, "%llu\n");


static ssize_t kona_cpufreq_get_stats(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct kona_cpufreq_stats *stats;
	int len = 0;
	char debug_fs_buf[100];
	stats = &kona_cpufreq->stats;
	if (!stats->stats_en) {
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"Stats not enabled\n");
		return simple_read_from_buffer(user_buf, count, ppos,
				debug_fs_buf, len);
	}
	return kona_cpufreq_log_stats(file, user_buf, count, ppos);
}

static const struct file_operations stats_ops = {
	.open = cpufreq_debugfs_open,
	.read = kona_cpufreq_get_stats,
};

#ifdef CONFIG_KONA_TMON
static ssize_t cpufreq_get_temp_tholds(struct file *file,
	char __user *user_buf, size_t count, loff_t *ppos)
{
	u32 len = 0;
	int i;
	char out_str[200];
	struct kona_cpufreq_drv_pdata *pdata = kona_cpufreq->pdata;

	memset(out_str, 0, sizeof(out_str));
	len += snprintf(out_str + len, sizeof(out_str) - len,
			"Level\t\tLimit CPUFREQ\tThreshold\n");
	for (i = pdata->num_freqs - 1; i >= 0; i--)
		len += snprintf(out_str + len, sizeof(out_str) - len,
		"Level%d:\t\t%d\t\t%ld\n", pdata->num_freqs - 1 - i,
		pdata->freq_tbl[i].cpu_freq, pdata->freq_tbl[i].max_temp);

	return simple_read_from_buffer(user_buf, count, ppos,
			out_str, len);
}

static ssize_t cpufreq_set_temp_tholds(struct file *file,
	  char const __user *buf, size_t count, loff_t *offset)
{
	u32 len = 0;
	char input_str[20];
	struct kona_cpufreq_drv_pdata *pdata = kona_cpufreq->pdata;
	int num_freqs = pdata->num_freqs;
	int next, prev, inx, thold;

	memset(input_str, 0, ARRAY_SIZE(input_str));
	if (count > ARRAY_SIZE(input_str))
		len = ARRAY_SIZE(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;

	sscanf(&input_str[0], "%d%d", &inx, &thold);

	inx = num_freqs - 1 - inx;
	/*check if inx is valid*/
	if (inx < 0 || inx >= num_freqs)
		goto exit;

	if (thold == TEMP_DONT_CARE)
		goto set_thold;

	/*checking if thold is in ascending order*/
	prev = inx - 1;
	next = inx + 1;

	if ((inx > 0) && (pdata->freq_tbl[prev].max_temp != TEMP_DONT_CARE)) {
		if (thold >= pdata->freq_tbl[prev].max_temp)
			goto exit;
	}
	if ((inx < num_freqs - 1) &&
			(pdata->freq_tbl[next].max_temp != TEMP_DONT_CARE)) {
		if (thold <= pdata->freq_tbl[next].max_temp)
			goto exit;
	}
set_thold:
	pdata->freq_tbl[inx].max_temp = thold;
	return count;
exit:
	pr_info("USAGE:\necho inx thold > temp_tholds\n");
	pr_info("inx range: [0-%d]\ntholds shd be in ascending order\n",
			num_freqs - 1);
	return count;
}

static const struct file_operations kcf_temp_tholds_ops = {
	.open = cpufreq_debugfs_open,
	.write = cpufreq_set_temp_tholds,
	.read = cpufreq_get_temp_tholds,
};
#endif

static struct dentry *dent_kcf_root_dir;
int __init kona_cpufreq_debug_init(void)
{
	dent_kcf_root_dir = debugfs_create_dir("kona_cpufreq", 0);
	if (!dent_kcf_root_dir)
		return -ENOMEM;
	if (!debugfs_create_u32
	    ("debug", S_IRUSR | S_IWUSR, dent_kcf_root_dir, &kcf_debug))
		return -ENOMEM;

	if (!debugfs_create_file
	    ("bogo_mips", S_IRUSR, dent_kcf_root_dir, NULL, &bogo_mips_fops))
		return -ENOMEM;
	if (!debugfs_create_file("read_requests", S_IRUSR,
			dent_kcf_root_dir, kona_cpufreq, &request_list_ops))
		return -ENOMEM;
#ifdef CONFIG_KONA_TMON
	if (!debugfs_create_file("temp_tholds",  S_IRUSR | S_IWUSR,
			dent_kcf_root_dir, kona_cpufreq, &kcf_temp_tholds_ops))
		return -ENOMEM;
#endif
	if (!debugfs_create_file
	    ("enable_stats", S_IWUSR, dent_kcf_root_dir, NULL,
						&stats_en_ops))
		return -ENOMEM;

	if (!debugfs_create_file
	    ("stats", S_IRUSR, dent_kcf_root_dir, NULL,
						&stats_ops))
		return -ENOMEM;
	return 0;
}

late_initcall(kona_cpufreq_debug_init);

#endif

MODULE_ALIAS("platform:kona_cpufreq_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CPU Frequency Driver for Broadcom Kona Chipsets");
