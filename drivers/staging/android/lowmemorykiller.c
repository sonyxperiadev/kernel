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

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/swap.h>
#include <linux/rcupdate.h>
#include <linux/notifier.h>
#include <linux/lowmemorykiller.h>

/*
 * See Documentation/trace/postprocess/trace-almk-postprocess.pl
 * for decoding ALMK traces.
 */
#define CREATE_TRACE_POINTS
#include <trace/events/almk.h>

static uint32_t lowmem_debug_level = 1;
static short lowmem_adj[6] = {
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

#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			pr_info(x);			\
	} while (0)

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
	int tasksize;
	int i;
	short min_score_adj = OOM_SCORE_ADJ_MAX + 1;
	int minfree = 0;
	int selected_tasksize = 0;
	short selected_oom_score_adj;
	int cma_free = 0;
	int cma_file = 0;
	int array_size = ARRAY_SIZE(lowmem_adj);
	int other_free = global_page_state(NR_FREE_PAGES) - totalreserve_pages;
	int other_file = global_page_state(NR_FILE_PAGES) -
						global_page_state(NR_SHMEM);

#ifdef CONFIG_CMA
	/*
	 * We can't count CMA free pages as free pages
	 * because CMA area can't serve unmovable
	 * allocations. Consider a situation where CMA size is
	 * considerable and device enters a state
	 * where we have a lot of CMA free pages and less
	 * "non-cma" free pages. LMK will trigger but
	 * will not kill any task. This finally leads to
	 * unmovable page allocation failures.
	 * At the same time we can't put a condition here to
	 * consider cma free depending on the allocation type
	 * (whether MIGRATE_MOVABLE or not).
	 * Because that will again lead to unmovable allocation
	 * failure in a scenario like:
	 * 1) A series of movable allocations.
	 * 2) Not much of CMA allocations, i.e.
	 *    there are CMA free pages.
	 * 3) LMK does not trigger, as all are movable
	 *    allocations, and there are CMA pages >
	 *    LMK threshold.
	 * 4) This results in pages for unmovable
	 *    allocations reducing in number.
	 * 5) Being in this situation when an unmovable
	 *    allocation comes in, it fails and the LMK
	 *    fails to kill at rate, to be at par with
	 *    memory drain.
	 *
	 * Tests show that this works well when the CMA
	 * size is not huge compared to available memory.
	 */
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
		minfree = lowmem_minfree[i];
		if (other_free < minfree && other_file < minfree) {
			min_score_adj = lowmem_adj[i];
			break;
		}
	}
	if (sc->nr_to_scan > 0)
		lowmem_print(3, "lowmem_shrink %lu, %x, ofree %d %d, ma %hd\n",
				sc->nr_to_scan, sc->gfp_mask, other_free,
				other_file, min_score_adj);
	active_anon = global_page_state(NR_ACTIVE_ANON);
	inactive_anon = global_page_state(NR_INACTIVE_ANON);
	active_file = global_page_state(NR_ACTIVE_FILE);
	inactive_file = global_page_state(NR_INACTIVE_FILE);
	rem = active_anon + inactive_anon + active_file + inactive_file;

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
			active_file, inactive_file, rem);

	if (sc->nr_to_scan <= 0 || min_score_adj == OOM_SCORE_ADJ_MAX + 1) {
		lowmem_print(5, "lowmem_shrink %lu, %x, return %d\n",
			     sc->nr_to_scan, sc->gfp_mask, rem);
		return rem;
	}
	selected_oom_score_adj = min_score_adj;

	rcu_read_lock();
	for_each_process(tsk) {
		struct task_struct *p;
		short oom_score_adj;

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
		lowmem_print(2, "select '%s' (%d), adj %hd, size %d, to kill\n",
			     p->comm, p->pid, oom_score_adj, tasksize);
	}
	if (selected) {
		lowmem_print(1, "Killing '%s' (%d), adj %hd,\n" \
				"   to free %ldkB on behalf of '%s' (%d) because\n" \
				"   cache %ldkB is below limit %ldkB for oom_score_adj %hd\n" \
				"   Free memory is %ldkB above reserved\n" \
				"   cma free: %ldkB, cma file: %ldkB\n",
			     selected->comm, selected->pid,
			     selected_oom_score_adj,
			     selected_tasksize * (long)(PAGE_SIZE / 1024),
			     current->comm, current->pid,
			     other_file * (long)(PAGE_SIZE / 1024),
			     minfree * (long)(PAGE_SIZE / 1024),
			     min_score_adj,
			     other_free * (long)(PAGE_SIZE / 1024),
			     cma_free * (long)(PAGE_SIZE / 1024),
			     cma_file * (long)(PAGE_SIZE / 1024));
		lowmem_deathpending_timeout = jiffies + HZ;
		send_sig(SIGKILL, selected, 0);
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

static int __init lowmem_init(void)
{
	register_shrinker(&lowmem_shrinker);
	return 0;
}

static void __exit lowmem_exit(void)
{
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
static short lowmem_oom_adj_to_oom_score_adj(short oom_adj)
{
	if (oom_adj == OOM_ADJUST_MAX)
		return OOM_SCORE_ADJ_MAX;
	else
		return (oom_adj * OOM_SCORE_ADJ_MAX) / -OOM_DISABLE;
}

static void lowmem_autodetect_oom_adj_values(void)
{
	int i;
	short oom_adj;
	short oom_score_adj;
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
	.ops = &param_ops_short,
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
__MODULE_PARM_TYPE(adj, "array of short");
#else
module_param_array_named(adj, lowmem_adj, short, &lowmem_adj_size,
			 S_IRUGO | S_IWUSR);
#endif
module_param_array_named(minfree, lowmem_minfree, uint, &lowmem_minfree_size,
			 S_IRUGO | S_IWUSR);
module_param_named(debug_level, lowmem_debug_level, uint, S_IRUGO | S_IWUSR);

module_init(lowmem_init);
module_exit(lowmem_exit);

MODULE_LICENSE("GPL");

