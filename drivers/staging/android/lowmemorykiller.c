/* drivers/misc/lowmemorykiller.c
 *
 * The lowmemorykiller driver lets user-space specify a set of memory thresholds
 * where processes with a range of oom_score_adj values will get killed. Specify
 * the minimum oom_score_adj values in
 * /sys/module/lowmemorykiller/parameters/adj and the number of free pages in
 * /sys/module/lowmemorykiller/parameters/minfree. Both files take a comma
 * separated list of numbers in ascending order.
 *
 * For example, write "0,8" to /sys/module/lowmemorykiller/parameters/adj and
 * "1024,4096" to /sys/module/lowmemorykiller/parameters/minfree to kill
 * processes with a oom_score_adj value of 8 or higher when the free memory
 * drops below 4096 pages and kill processes with a oom_score_adj value of 0 or
 * higher when the free memory drops below 1024 pages.
 *
 * The driver considers memory used for caches to be free, but if a large
 * percentage of the cached memory is locked this can be very inaccurate
 * and processes may not get killed until the normal oom killer is triggered.
 *
 * Copyright (C) 2007-2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/swap.h>
#include <linux/rcupdate.h>
#include <linux/notifier.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/lowmemorykiller.h>

#define CREATE_TRACE_POINTS
#include <trace/events/almk.h>

static uint32_t lowmem_debug_level = 1;
static int lowmem_adj[6] = {
	0,
	1,
	6,
	12,
};
static int lowmem_adj_size = 4;
static int lowmem_minfree[6] = {
	3 * 512,	/* 6MB */
	2 * 1024,	/* 8MB */
	4 * 1024,	/* 16MB */
	16 * 1024,	/* 64MB */
};
static int lowmem_minfree_size = 4;

static unsigned long lowmem_deathpending_timeout;

static LIST_HEAD(lmk_reg_list);
static DECLARE_RWSEM(lmk_reg_rwsem);

static struct dentry *lmk_debug_dir;

/* Light weight accounting of LMK stats.
 * At present, kill count alone.
 */
struct lmk_stat {
	unsigned long kill_count;
};

DEFINE_PER_CPU(struct lmk_stat, lmk_stats);

#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			printk(x);			\
	} while (0)

static int lowmem_oom_score_adj_to_oom_adj(int oom_score_adj);

static int lowmem_shrink(struct shrinker *s, struct shrink_control *sc)
{
	struct task_struct *tsk;
	struct task_struct *selected = NULL;
	struct reg_lmk *reg_lmk;
	int rem = 0;
	int active_anon;
	int inactive_anon;
	int active_file;
	int inactive_file;
	int contig_pages;
	int tasksize;
	int i;
	int min_score_adj = OOM_SCORE_ADJ_MAX + 1;
	int minfree = INT_MAX;
	int selected_tasksize = 0;
	int selected_oom_score_adj;
	int array_size = ARRAY_SIZE(lowmem_adj);
	int other_free = global_page_state(NR_FREE_PAGES) - totalreserve_pages;
	int other_file = global_page_state(NR_FILE_PAGES) -
						global_page_state(NR_SHMEM) -
						total_swapcache_pages;

	int cma_free = INT_MAX;
	int cma_file = INT_MAX;
	/*
	 * If CMA is enabled, then do not count free pages
	 * from CMA region and also ignore CMA pages that are
	 * allocated for files.
	 */
#ifdef CONFIG_CMA
	cma_free = global_page_state(NR_FREE_CMA_PAGES);
	cma_file = global_page_state(NR_CMA_INACTIVE_FILE)
			+ global_page_state(NR_CMA_ACTIVE_FILE);

	other_free -= cma_free;
	other_file -= cma_file;
#endif
	if (lowmem_adj_size < array_size)
		array_size = lowmem_adj_size;
	if (lowmem_minfree_size < array_size)
		array_size = lowmem_minfree_size;
	for (i = 0; i < array_size; i++) {
		if (other_free < lowmem_minfree[i] &&
		    other_file < lowmem_minfree[i]) {
			min_score_adj = lowmem_adj[i];
			minfree = lowmem_minfree[i];
			break;
		}
	}
	if (sc->nr_to_scan > 0)
#ifndef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
		lowmem_print(3, "lowmem_shrink %lu, %x, ofree %d %d, ma %d\n",
				sc->nr_to_scan, sc->gfp_mask, other_free,
				other_file, min_score_adj);
#else
		lowmem_print(3, "lowmem_shrink %lu, %x, ofree %d %d, msa %d"
				" ma %d\n",
				sc->nr_to_scan, sc->gfp_mask, other_free,
				other_file, min_score_adj,
				lowmem_oom_score_adj_to_oom_adj(min_score_adj));
#endif
	active_anon = global_page_state(NR_ACTIVE_ANON);
	inactive_anon = global_page_state(NR_INACTIVE_ANON);
	active_file = global_page_state(NR_ACTIVE_FILE);
	inactive_file = global_page_state(NR_INACTIVE_FILE);
	rem = active_anon + inactive_anon + active_file + inactive_file;

	/*
	 * If CMA is enabled, We will also free up contiguous
	 * allocations done by processes (We cannot free up DMA
	 * allocations that go from CMA region, but we can't count
	 * DMA and PMEM allocations separately right now, so we take
	 * the total.
	 */
#ifdef CONFIG_CMA
	contig_pages = global_page_state(NR_CONTIG_PAGES);
	rem += contig_pages;
#endif

	down_read(&lmk_reg_rwsem);
	list_for_each_entry(reg_lmk, &lmk_reg_list, list) {
		int ret;
		struct lmk_op op = {
			.op = 0,
		};

		ret = reg_lmk->cbk(reg_lmk, &op);
		if (!WARN_ONCE(ret < 0, "invalid rem: %p, %d\n", reg_lmk, ret))
			rem += ret;
	}
	up_read(&lmk_reg_rwsem);

	trace_almk_start(sc->nr_to_scan, sc->gfp_mask, current->comm,
			min_score_adj, minfree, other_free, other_file,
			cma_free, cma_file, active_anon, inactive_anon,
			active_file, inactive_file, contig_pages, rem);

	if (sc->nr_to_scan <= 0 || min_score_adj == OOM_SCORE_ADJ_MAX + 1) {
		lowmem_print(5, "lowmem_shrink %lu, %x, return %d\n",
			     sc->nr_to_scan, sc->gfp_mask, rem);
		return rem;
	}
	selected_oom_score_adj = min_score_adj;

	rcu_read_lock();
	for_each_process(tsk) {
		struct task_struct *p;
		int oom_score_adj;

		if (tsk->flags & PF_KTHREAD)
			continue;

		p = find_lock_task_mm(tsk);
		if (!p)
			continue;

		if (test_tsk_thread_flag(p, TIF_MEMDIE) &&
		    time_before_eq(jiffies, lowmem_deathpending_timeout)) {
			task_unlock(p);
			rcu_read_unlock();
			trace_almk_end(-2, 0, current->comm, minfree,
					other_free, other_file, cma_free,
					cma_file, sc->gfp_mask);
			return 0;
		}
		oom_score_adj = p->signal->oom_score_adj;
		if (oom_score_adj < min_score_adj) {
			task_unlock(p);
			continue;
		}
		tasksize = get_mm_rss(p->mm);
		task_unlock(p);
		if (tasksize <= 0)
			continue;
		if (selected) {
			if (oom_score_adj < selected_oom_score_adj)
				continue;
			if (oom_score_adj == selected_oom_score_adj &&
			    tasksize <= selected_tasksize)
				continue;
		}
		selected = p;
		selected_tasksize = tasksize;
		selected_oom_score_adj = oom_score_adj;
#ifndef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
		lowmem_print(2, "select %d (%s), adj %d, size %d, to kill\n",
			     p->pid, p->comm, oom_score_adj, tasksize);
#else
		lowmem_print(2, "select %d (%s), score_adj %d, adj %d,"
				"size %d, to kill\n",
			     p->pid, p->comm, oom_score_adj,
			     lowmem_oom_score_adj_to_oom_adj(oom_score_adj),
			     tasksize);
#endif
	}
	if (selected) {
#ifndef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
		lowmem_print(1, "send sigkill to %d (%s), adj %d, size %d"
				" with ofree %d %d, cfree %d %d ma %d\n",
			     selected->pid, selected->comm,
			     selected_oom_score_adj, selected_tasksize,
			     other_free, other_file, cma_free, cma_file,
			     min_score_adj);
#else
		lowmem_print(1, "send sigkill to %d (%s), score_adj %d,"
				"adj %d, size %d with ofree %d %d, cfree %d %d"
				" msa %d ma %d\n",
		selected->pid, selected->comm,
		selected_oom_score_adj,
		lowmem_oom_score_adj_to_oom_adj(selected_oom_score_adj),
		selected_tasksize,
		other_free, other_file, cma_free, cma_file,
		min_score_adj,
		lowmem_oom_score_adj_to_oom_adj(min_score_adj));
#endif
		lowmem_deathpending_timeout = jiffies + HZ;
		send_sig(SIGKILL, selected, 0);
		this_cpu_inc(lmk_stats.kill_count);
		set_tsk_thread_flag(selected, TIF_MEMDIE);
		rem -= selected_tasksize;
		trace_almk_end(selected_oom_score_adj, selected_tasksize,
				current->comm, minfree, other_free,
				other_file, cma_free,
				cma_file, sc->gfp_mask);
	} else {
		trace_almk_end(-1, 0, current->comm, minfree,
				other_free, other_file,
				cma_free, cma_file, sc->gfp_mask);
	}

	lowmem_print(4, "lowmem_shrink %lu, %x, return %d\n",
		     sc->nr_to_scan, sc->gfp_mask, rem);
	rcu_read_unlock();
	return rem;
}

static struct shrinker lowmem_shrinker = {
	.shrink = lowmem_shrink,
	.seeks = DEFAULT_SEEKS * 16
};

static int lmk_kill_count_show(struct seq_file *m, void *v)
{
	int cpu;
	unsigned long sum = 0;

	for_each_possible_cpu(cpu) {
		struct lmk_stat *this = &per_cpu(lmk_stats, cpu);
		sum += this->kill_count;
		seq_printf(m, "%d:%lu\n", cpu, this->kill_count);
	}

	seq_printf(m, "%lu\n", sum);
	return 0;
}

static int lmk_kill_count_open(struct inode *inode, struct file *file)
{
	return single_open(file, lmk_kill_count_show, NULL);
}

static const struct file_operations lmk_kill_count_fops = {
	.open           = lmk_kill_count_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int __init lowmem_init(void)
{
	struct dentry *fentry;

	lmk_debug_dir = debugfs_create_dir("almk", NULL);
	if (!lmk_debug_dir) {
		pr_err("almk: failed to create debugfs dir\n");
		goto skip;
	}

	fentry = debugfs_create_file("kill_count", S_IRUSR,
			lmk_debug_dir, NULL,
			&lmk_kill_count_fops);
	if (!fentry)
		pr_err("almk: failed to create debugfs file\n");
skip:
	register_shrinker(&lowmem_shrinker);
	return 0;
}

static void __exit lowmem_exit(void)
{
	debugfs_remove_recursive(lmk_debug_dir);
	unregister_shrinker(&lowmem_shrinker);
}

/*
 * Register callback to LMK.
 * This may serve different purposes. But now it is used to get the
 * pages mapped/used to/by user space, other than the kernel accounted
 * anon and file pages. For example pages allocated by a memory allocator
 * like ION. The register function and the callback will remain the same
 * for other purposes too, and will differ only in the argument passed
 * to the callbacks when invoked by LMK.
 */
void register_lmk(struct reg_lmk *reg_lmk)
{
	down_write(&lmk_reg_rwsem);
	list_add_tail(&reg_lmk->list, &lmk_reg_list);
	up_write(&lmk_reg_rwsem);
}
EXPORT_SYMBOL(register_lmk);

void unregister_lmk(struct reg_lmk *reg_lmk)
{
	down_write(&lmk_reg_rwsem);
	list_del(&reg_lmk->list);
	up_write(&lmk_reg_rwsem);
}
EXPORT_SYMBOL(unregister_lmk);

#ifdef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
static int lowmem_oom_adj_to_oom_score_adj(int oom_adj)
{
	if (oom_adj == OOM_ADJUST_MAX)
		return OOM_SCORE_ADJ_MAX;
	else
		return (oom_adj * OOM_SCORE_ADJ_MAX) / -OOM_DISABLE;
}

static int lowmem_oom_score_adj_to_oom_adj(int oom_score_adj)
{
	if (oom_score_adj == OOM_SCORE_ADJ_MAX)
		return OOM_ADJUST_MAX;
	else
		return (oom_score_adj * -OOM_DISABLE) / OOM_SCORE_ADJ_MAX;
}

static void lowmem_autodetect_oom_adj_values(void)
{
	int i;
	int oom_adj;
	int oom_score_adj;
	int array_size = ARRAY_SIZE(lowmem_adj);

	if (lowmem_adj_size < array_size)
		array_size = lowmem_adj_size;

	if (array_size <= 0)
		return;

	oom_adj = lowmem_adj[array_size - 1];
	if (oom_adj > OOM_ADJUST_MAX)
		return;

	oom_score_adj = lowmem_oom_adj_to_oom_score_adj(oom_adj);
	if (oom_score_adj <= OOM_ADJUST_MAX)
		return;

	lowmem_print(1, "lowmem_shrink: convert oom_adj to oom_score_adj:\n");
	for (i = 0; i < array_size; i++) {
		oom_adj = lowmem_adj[i];
		oom_score_adj = lowmem_oom_adj_to_oom_score_adj(oom_adj);
		lowmem_adj[i] = oom_score_adj;
		lowmem_print(1, "oom_adj %d => oom_score_adj %d\n",
			     oom_adj, oom_score_adj);
	}
}

static int lowmem_adj_array_set(const char *val, const struct kernel_param *kp)
{
	int ret;

	ret = param_array_ops.set(val, kp);

	/* HACK: Autodetect oom_adj values in lowmem_adj array */
	lowmem_autodetect_oom_adj_values();

	return ret;
}

static int lowmem_adj_array_get(char *buffer, const struct kernel_param *kp)
{
	return param_array_ops.get(buffer, kp);
}

static void lowmem_adj_array_free(void *arg)
{
	param_array_ops.free(arg);
}

static struct kernel_param_ops lowmem_adj_array_ops = {
	.set = lowmem_adj_array_set,
	.get = lowmem_adj_array_get,
	.free = lowmem_adj_array_free,
};

static const struct kparam_array __param_arr_adj = {
	.max = ARRAY_SIZE(lowmem_adj),
	.num = &lowmem_adj_size,
	.ops = &param_ops_int,
	.elemsize = sizeof(lowmem_adj[0]),
	.elem = lowmem_adj,
};
#endif

module_param_named(cost, lowmem_shrinker.seeks, int, S_IRUGO | S_IWUSR);
#ifdef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
__module_param_call(MODULE_PARAM_PREFIX, adj,
		    &lowmem_adj_array_ops,
		    .arr = &__param_arr_adj,
		    S_IRUGO | S_IWUSR, -1);
__MODULE_PARM_TYPE(adj, "array of int");
#else
module_param_array_named(adj, lowmem_adj, int, &lowmem_adj_size,
			 S_IRUGO | S_IWUSR);
#endif
module_param_array_named(minfree, lowmem_minfree, uint, &lowmem_minfree_size,
			 S_IRUGO | S_IWUSR);
module_param_named(debug_level, lowmem_debug_level, uint, S_IRUGO | S_IWUSR);

module_init(lowmem_init);
module_exit(lowmem_exit);

MODULE_LICENSE("GPL");

