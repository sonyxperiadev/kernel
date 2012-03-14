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
#include <plat/kona_cpufreq_drv.h>

#include <plat/pi_mgr.h>
#include <asm/cpu.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
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

static int kcf_debug = 0;

struct kona_freq_map {
	u32 cpu_freq;
	int opp;		/* Operating point eg: ECONOMY, NORMAL, TURBO */
};

struct kona_cpufreq {
	int pi_id;
	struct pi_mgr_dfs_node dfs_node;
	struct cpufreq_frequency_table *kona_freqs_table;
	struct kona_freq_map *freq_map;
	int no_of_opps;
	struct cpufreq_policy *policy;
	struct kona_cpufreq_drv_pdata *pdata;
#ifdef CONFIG_SMP
	unsigned long l_p_j_ref;
	unsigned int l_p_j_ref_freq;
#endif

};
static struct kona_cpufreq *kona_cpufreq;

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

static int kona_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	int ret = -EINVAL;

	if (kona_cpufreq->kona_freqs_table)
		ret = cpufreq_frequency_table_verify(policy,
						     kona_cpufreq->
						     kona_freqs_table);

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
	u32 opp = PI_OPP_NORMAL;
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

	if (freqs.old == freqs.new)
		return 0;

	kcf_dbg("%s: cpu freq change: %u --> %u\n", __func__, freqs.old,
		freqs.new);

	for_each_online_cpu(freqs.cpu)
	    cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
	local_irq_disable();

	for (i = 0; i < kona_cpufreq->no_of_opps; i++) {
		if (freqs.new == kona_cpufreq->freq_map[i].cpu_freq) {
			opp = kona_cpufreq->freq_map[i].opp;
			break;
		}
	}
	ret = pi_mgr_dfs_request_update(&kona_cpufreq->dfs_node, opp);

	if (unlikely(ret)) {
		kcf_dbg("%s: cpu freq change failed : %d\n", __func__, ret);
	}

	local_irq_enable();
	for_each_online_cpu(freqs.cpu)
	    cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

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
	struct kona_cpufreq_drv_pdata *pdata = pdev->dev.platform_data;
	int i;
	int ret = -1;

	kcf_dbg("%s\n", __func__);
	BUG_ON(pdata == NULL);
	/* allocate memory for per-cpu data for all cpus */
	kona_cpufreq = kzalloc(sizeof(struct kona_cpufreq), GFP_KERNEL);
	if (!kona_cpufreq) {
		kcf_dbg("%s: kzalloc failed for kona_cpufreq\n", __func__);
		return -ENOMEM;
	}
	memset(kona_cpufreq, 0, sizeof(struct kona_cpufreq));
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

	return cpufreq_register_driver(&kona_cpufreq_driver);

}

static int __devexit cpufreq_drv_remove(struct platform_device *pdev)
{
	int ret = 0;
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
	.remove = __devexit_p(cpufreq_drv_remove),
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
	return 0;

}

late_initcall(kona_cpufreq_debug_init);

#endif

MODULE_ALIAS("platform:kona_cpufreq_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CPU Frequency Driver for Broadcom Kona Chipsets");
