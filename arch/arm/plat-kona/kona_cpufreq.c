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

struct kona_freq_map
{
    u32 cpu_freq;
    int opp;         /* Operating point eg: ECONOMY, NORMAL, TURBO */
};
/* Per-CPU private data */
struct kona_cpufreq
{
	int pi_id;
	struct pi_mgr_dfs_node* dfs_node;
	struct cpufreq_frequency_table *kona_freqs_table;
	struct kona_freq_map *freq_map;
	int no_of_opps;
	struct cpufreq_policy *policy;
	struct kona_cpufreq_drv_plat *plat;
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
	struct kona_cpufreq *b = &kona_cpufreq[policy->cpu];
	struct kona_cpu_info *info = NULL;
	int i, num;

	info = &b->plat->info[policy->cpu];
	num = info->num_freqs;
	kcf_dbg("%s: num_freqs: %d\n", __func__, num);

	for (i = 0; i < num; i++)
	{
		t[i].index = i;
		t[i].frequency = info->freq_tbl[i].cpu_freq;
		kcf_dbg("%s: index: %d, freq: %u\n", __func__, t[i].index, t[i].frequency);
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
    struct kona_cpufreq *b = &kona_cpufreq[cpu];
    int opp;
    int i;

    opp = pi_get_active_opp(b->pi_id);
	kcf_dbg("%s: opp = %d\n",__func__,opp);

	if(opp < 0)
		return 0;
	for(i=0; i<b->no_of_opps; i++)
	{
		if (b->freq_map[i].opp == opp)
		{
			kcf_dbg("opp found, return corresponding freq %u\n", b->freq_map[i].cpu_freq);
			return b->freq_map[i].cpu_freq;
		}
    }
    kcf_dbg("Since the table is setup with all OPP, ctrl shouldnt reach here\n");
    BUG();
    return 0;
}

static int kona_cpufreq_verify_speed(struct cpufreq_policy *policy)
{
	struct kona_cpufreq *b = &kona_cpufreq[policy->cpu];
	int ret = -EINVAL;

	if (b->kona_freqs_table)
		ret = cpufreq_frequency_table_verify(policy,
			b->kona_freqs_table);
	kcf_dbg("%s: after cpufreq verify: min:%d->max:%d kHz\n",
			__func__, policy->min, policy->max);


	return 0;
	return ret;
}

static int kona_cpufreq_set_speed(struct cpufreq_policy *policy,
	unsigned int target_freq,
	unsigned int relation)
{
	struct cpufreq_freqs freqs;
	struct kona_cpufreq *b = &kona_cpufreq[policy->cpu];
	//struct kona_cpu_info *info = &b->plat->info[policy->cpu];
	int i, index;
	int ret = 0;
	u32 opp = PI_OPP_NORMAL;

	/* Lookup the next frequency */
	if(cpufreq_frequency_table_target(policy, b->kona_freqs_table,
		target_freq, relation, &index))
	{
		return -EINVAL;
	}
	freqs.cpu = policy->cpu;
	freqs.old = kona_cpufreq_get_speed(policy->cpu);
	freqs.new = b->kona_freqs_table[index].frequency;

	if (freqs.old == freqs.new)
		return 0;

	kcf_dbg("%s: cpu freq change: %u --> %u\n", __func__, freqs.old,
		freqs.new);

	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
	local_irq_disable();

	for(i=0;i<b->no_of_opps;i++)
	{
	    if(freqs.new == b->freq_map[i].cpu_freq)
		{
			opp = b->freq_map[i].opp;
			break;
	    }
	}
	ret = pi_mgr_dfs_request_update(b->dfs_node, opp);

	local_irq_enable();
	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	if(unlikely(ret))
		kcf_dbg("%s: cpu freq change failed : %d\n", __func__, ret);

	return ret;
}

static int kona_cpufreq_init(struct cpufreq_policy *policy)
{
	struct kona_cpufreq *b = NULL;
	struct kona_cpu_info *info = NULL;
	int ret, i;

	kcf_dbg("%s\n", __func__);

	/* Get handle to cpu private data */
	b = &kona_cpufreq[policy->cpu];
	info = &b->plat->info[policy->cpu];

	b->pi_id = info->pi_id;
	kcf_dbg("%s: pi_id for ARM core :%d\n", __func__, b->pi_id);
	b->no_of_opps = info->num_freqs;

	for(i=0; i < b->no_of_opps; i++)
	{
	    b->freq_map[i].cpu_freq = info->freq_tbl[i].cpu_freq;
	    b->freq_map[i].opp = info->freq_tbl[i].opp;
	}
	/* Set default policy and cpuinfo */
	policy->cur = kona_cpufreq_get_speed(policy->cpu);
	pr_info("%s:policy->cur = %d\n",__func__, policy->cur);
	/* FIXME: Tune this value */
	policy->cpuinfo.transition_latency = info->kona_latency;

	for(i=0;i<info->num_freqs;i++)
	    kcf_dbg("index: %d, freq: %u opp:%d\n",
	    		i, info->freq_tbl[i].cpu_freq, info->freq_tbl[i].opp);

	ret = kona_create_cpufreqs_table(policy, (b->kona_freqs_table));
	if(ret)
	{
		kcf_dbg("%s: setup_cpufreqs_table failed: %d\n",
			__func__, ret);
		goto err_cpufreqs_table;
	}

	ret = cpufreq_frequency_table_cpuinfo(policy, b->kona_freqs_table);
	if(ret)
	{
		kcf_dbg("%s: cpufreq_frequency_table_cpuinfo failed\n",
			__func__);
		goto err_cpufreqs_table;
	}
	cpufreq_frequency_table_get_attr(b->kona_freqs_table, policy->cpu);
	b->policy = policy;

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
	.name   = "kona",
	.flags  = 0,
	.init   = kona_cpufreq_init,
	.verify = kona_cpufreq_verify_speed,
	.target = kona_cpufreq_set_speed,
	.get    = kona_cpufreq_get_speed,
	.exit   = kona_cpufreq_exit,
	.attr   = kona_cpufreq_attr,
	.owner  = THIS_MODULE,
};

static int cpufreq_drv_probe(struct platform_device *pdev)
{
	struct kona_cpufreq_drv_plat *plat = pdev->dev.platform_data;
	int i;

	kcf_dbg("%s\n", __func__);
	BUG_ON(plat == NULL);
	/* allocate memory for per-cpu data for all cpus */
	kona_cpufreq = kzalloc(plat->nr_cpus * sizeof(struct kona_cpufreq),
		GFP_KERNEL);
	if(!kona_cpufreq)
	{
		kcf_dbg("%s: kzalloc failed for kona_cpufreq\n", __func__);
		return -ENOMEM;
	}
	for(i = 0; i < plat->nr_cpus; i++)
	{
	   	kona_cpufreq[i].freq_map = kzalloc(plat->info[i].num_freqs *
						sizeof(struct kona_freq_map), GFP_KERNEL);
		if(!kona_cpufreq[i].freq_map)
		{
		    kcf_dbg("%s: kzalloc failed for freq_map\n", __func__);
		    return -ENOMEM;
		}
		kona_cpufreq[i].kona_freqs_table = kzalloc(
				(plat->info[i].num_freqs + 1) * sizeof(struct cpufreq_frequency_table), GFP_KERNEL);
		if (!kona_cpufreq[i].kona_freqs_table)
		{
		    kcf_dbg("%s: kzalloc failed for kona_freqs_table\n", __func__);
		    kfree(kona_cpufreq[i].freq_map);
		    return -ENOMEM;
		}
		kona_cpufreq[i].pi_id = plat->info[i].pi_id;
		/*Add a DFS client for ARM CCU. this client will be used later
		 * for changinf ARM freq via cpu-freq.*/
		kona_cpufreq[i].dfs_node = pi_mgr_dfs_add_request("cpu_freq", kona_cpufreq[i].pi_id,
				pi_get_active_opp(kona_cpufreq[i].pi_id));
		if (!kona_cpufreq[i].dfs_node)
		{
		    kcf_dbg("Failed add dfs request for CPU\n");
		    kfree(kona_cpufreq[i].kona_freqs_table);
		    kfree(kona_cpufreq[i].freq_map);
		    return  -ENOMEM;
		}
		kona_cpufreq[i].plat = plat;
	}
	platform_set_drvdata(pdev, kona_cpufreq);

	return cpufreq_register_driver(&kona_cpufreq_driver);

}

static int __devexit cpufreq_drv_remove(struct platform_device *pdev)
{
    struct kona_cpufreq_drv_plat *plat = pdev->dev.platform_data;
    int ret = 0;
    int i;
    if (cpufreq_unregister_driver(&kona_cpufreq_driver) != 0)
	kcf_dbg("%s: cpufreq unregister failed\n", __func__);
    for(i = 0; i < plat->nr_cpus; i++) {
	ret = pi_mgr_dfs_request_remove(kona_cpufreq[i].dfs_node);
	if(ret)
	    kcf_dbg("%s: dfs remove request failed\n", __func__);

	    kfree(kona_cpufreq[i].kona_freqs_table);
	    kfree(kona_cpufreq[i].freq_map);
    }
    kfree(kona_cpufreq);
    kona_cpufreq = NULL;

    return 0;
}

static struct platform_driver cpufreq_drv =
	{
	.probe = cpufreq_drv_probe,
	.remove = __devexit_p(cpufreq_drv_remove),
	.driver =
		{
			.name = "kona-cpufreq-drv",
		}	,
	};

static int __init cpufreq_drv_init(void)
{
	return   platform_driver_register(&cpufreq_drv);
}
module_init(cpufreq_drv_init);

static void __exit cpufreq_drv_exit(void)
{
	platform_driver_unregister(&cpufreq_drv);
}
module_exit(cpufreq_drv_exit);

#ifdef CONFIG_DEBUG_FS


static ssize_t kona_cpufreq_bogmips_get(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
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

	lpj = (1<<12);
	while ((lpj <<= 1) != 0) {
		/* wait for "start of" clock tick */
		ticks = jiffies;
		while (ticks == jiffies)
			/* nothing */;
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
			/* nothing */;
		ticks = jiffies;
		__delay(lpj);
		if (jiffies != ticks)	/* longer than 1 tick */
			lpj &= ~loopbit;
	}
	len += snprintf(buf+len, sizeof(buf)-len,
			"%lu.%02lu BogoMIPS (lpj=%lu)\n", lpj/(500000/HZ),
		(lpj/(5000/HZ)) % 100, lpj);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static struct file_operations bogo_mips_fops =
{
	.read =         kona_cpufreq_bogmips_get,
};


static struct dentry *dent_kcf_root_dir;
int __init kona_cpufreq_debug_init(void)
{
    dent_kcf_root_dir = debugfs_create_dir("kona_cpufreq", 0);
    if(!dent_kcf_root_dir)
		return -ENOMEM;
    if(!debugfs_create_u32("debug", S_IRUSR|S_IWUSR, dent_kcf_root_dir, &kcf_debug))
		return -ENOMEM;

    if(!debugfs_create_file("bogo_mips", S_IRUSR, dent_kcf_root_dir, NULL, &bogo_mips_fops))
		return -ENOMEM;
    return 0;

}

late_initcall(kona_cpufreq_debug_init);


#endif

MODULE_ALIAS("platform:kona_cpufreq_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CPU Frequency Driver for Broadcom Kona Chipsets");
