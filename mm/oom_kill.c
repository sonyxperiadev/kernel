/*
 *  linux/mm/oom_kill.c
 * 
 *  Copyright (C)  1998,2000  Rik van Riel
 *	Thanks go out to Claus Fischer for some serious inspiration and
 *	for goading me into coding this file...
 *  Copyright (C)  2010  Google, Inc.
 *	Rewritten by David Rientjes
 *
 *  The routines in this file are used to kill a process when
 *  we're seriously out of memory. This gets called from __alloc_pages()
 *  in mm/page_alloc.c when we really run out of memory.
 *
 *  Since we won't call these routines often (on a well-configured
 *  machine) this file will double as a 'coding guide' and a signpost
 *  for newbie kernel hackers. It features several pointers to major
 *  kernel subsystems and hints as to where to find out what things do.
 */

#include <linux/oom.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/gfp.h>
#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/sched/coredump.h>
#include <linux/sched/task.h>
#include <linux/swap.h>
#include <linux/timex.h>
#include <linux/jiffies.h>
#include <linux/cpuset.h>
#include <linux/export.h>
#include <linux/notifier.h>
#include <linux/memcontrol.h>
#include <linux/mempolicy.h>
#include <linux/security.h>
#include <linux/ptrace.h>
#include <linux/freezer.h>
#include <linux/ftrace.h>
#include <linux/ratelimit.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/mmu_notifier.h>
#include <linux/memory_hotplug.h>
#include <linux/show_mem_notifier.h>
#include <linux/psi.h>

#include <asm/tlb.h>
#include "internal.h"
#include "slab.h"

#define CREATE_TRACE_POINTS
#include <trace/events/oom.h>

int sysctl_panic_on_oom =
IS_ENABLED(CONFIG_DEBUG_PANIC_ON_OOM) ? 2 : 0;
int sysctl_oom_kill_allocating_task;
int sysctl_oom_dump_tasks = 1;
int sysctl_reap_mem_on_sigkill = 1;

static int panic_on_adj_zero;
module_param(panic_on_adj_zero, int, 0644);

/*
 * Serializes oom killer invocations (out_of_memory()) from all contexts to
 * prevent from over eager oom killing (e.g. when the oom killer is invoked
 * from different domains).
 *
 * oom_killer_disable() relies on this lock to stabilize oom_killer_disabled
 * and mark_oom_victim
 */
DEFINE_MUTEX(oom_lock);
/* Serializes oom_score_adj and oom_score_adj_min updates */
DEFINE_MUTEX(oom_adj_mutex);

/*
 * If ULMK has killed a process recently,
 * we are making progress.
 */

#ifdef CONFIG_HAVE_USERSPACE_LOW_MEMORY_KILLER

/* The maximum amount of time to loop in should_ulmk_retry() */
#define ULMK_TIMEOUT (20 * HZ)
#define ULMK_EMERG_TRIG_TIMEOUT (ULMK_TIMEOUT + 10 * HZ)

#define ULMK_DBG_POLICY_TRIGGER (BIT(0))
#define ULMK_DBG_POLICY_WDOG (BIT(1))
#define ULMK_DBG_POLICY_POSITIVE_ADJ (BIT(2))
#define ULMK_DBG_POLICY_ALL (BIT(3) - 1)
static unsigned int ulmk_dbg_policy;
module_param(ulmk_dbg_policy, uint, 0644);

static atomic64_t ulmk_wdog_expired = ATOMIC64_INIT(0);
static atomic64_t ulmk_kill_jiffies = ATOMIC64_INIT(INITIAL_JIFFIES);
static atomic64_t ulmk_watchdog_pet_jiffies = ATOMIC64_INIT(INITIAL_JIFFIES);
static unsigned long psi_emergency_jiffies = INITIAL_JIFFIES;
static unsigned long psi_emerg_trigger_jiffies = INITIAL_JIFFIES;
/* Prevents contention on the mutex_trylock in psi_emergency_jiffies */
static DEFINE_MUTEX(ulmk_retry_lock);

static bool __maybe_unused ulmk_kill_possible(void)
{
	struct task_struct *tsk;
	bool ret = false;

	rcu_read_lock();
	for_each_process(tsk) {
		if (tsk->flags & PF_KTHREAD)
			continue;

		task_lock(tsk);
		if (tsk->signal->oom_score_adj >= 0) {
			ret = true;
			task_unlock(tsk);
			break;
		}
		task_unlock(tsk);
	}
	rcu_read_unlock();

	return ret;
}

/*
 * If CONFIG_DEBUG_PANIC_ON_OOM is enabled, attempt to determine *why*
 * we are in this state.
 * 1) No events were sent by PSI to userspace
 * 2) PSI sent an event to userspace, but userspace was not able to
 * receive the event. Possible causes of this include waiting for a
 * mutex which is held by a process in direct relcaim. Or the userspace
 * component has crashed.
 * 3) Userspace received the event, but decided not to kill anything.
 */
bool should_ulmk_retry(gfp_t gfp_mask)
{
	unsigned long now, last_kill, last_wdog_pet;
	bool ret = true;
	bool wdog_expired, trigger_active;

	struct oom_control oc = {
		.zonelist = node_zonelist(first_memory_node, gfp_mask),
		.nodemask = NULL,
		.memcg = NULL,
		.gfp_mask = gfp_mask,
		.order = 0,
		/* Also causes check_panic_on_oom not to panic */
		.only_positive_adj = true,
	};

	if (!sysctl_panic_on_oom)
		return false;

	if (gfp_mask & __GFP_RETRY_MAYFAIL)
		return false;

	/* Someone else is already checking. */
	if (!mutex_trylock(&ulmk_retry_lock))
		return true;

	now = jiffies;
	last_kill = atomic64_read(&ulmk_kill_jiffies);
	last_wdog_pet = atomic64_read(&ulmk_watchdog_pet_jiffies);
	wdog_expired = atomic64_read(&ulmk_wdog_expired);
	trigger_active = psi_is_trigger_active();

	/*
	 * Returning True causes direct reclaim retry and false
	 * causes to take OOM path.
	 * Conditions check is as below:
	 * a) If there is a kill after the previous update of
	 *    psi_emergency_jiffies, then system kills are happening
	 *    properly. Thus update the psi_emergency_jiffies with the
	 *    current time and return true.
	 *
	 * b) If no kill have had happened in the last ULMK_TIMEOUT and
	 *    LMKD also stuck for the last ULMK_TIMEOUT despite an
	 *    emergency trigger in the last ULMK_EMERG_TRIG_TIMEOUT, which
	 *    then means that system kill logic is not responding despite
	 *    PSI events sent from kernel. Return false.
	 *
	 * c) Cond1: trigger = !active && wdog_expired = false:
	 *    Then give a chance to the ULMK by raising emergnecy trigger
	 *    which also registers a watchdog timer with timeout of
	 *    2 * trigger's ->win_size. And thus further process entering
	 *    gets returned with true.
	 *
	 *    Cond2: trigger = active && wdog_expired = true:
	 *    This represents that the previously raised event is not
	 *    consumed by ULMK in 2*HZ timeout. Under this condition we rely
	 *    on OOM killer to select the positive adj task and kill. If
	 *    the OOM killer fails to find a +ve adj task, we return false.
	 *
	 *    Cond3: trigger = !active && wdog_expired = true:
	 *    This is a case of previous events to previous are yet to be
	 *    consumed by ULMK, if triggered, thus only this process is
	 *    asked to raise the trigger and the subsequent ones in the
	 *    triggers ->win.size fall back to OOM.
	 *
	 *    Cond4: trigger = !active && wdog_expired = false:
	 *    ULMK is perfectly working fine.
	 */
	if (time_after(last_kill, psi_emergency_jiffies)) {
		psi_emergency_jiffies = now;
		ret = true;
	} else if (time_after(now, psi_emergency_jiffies + ULMK_TIMEOUT) &&
		   time_after(now, last_wdog_pet + ULMK_TIMEOUT) &&
		   time_after(psi_emerg_trigger_jiffies,
				now - ULMK_EMERG_TRIG_TIMEOUT)) {
		ret = false;
	} else if (!trigger_active) {
		BUG_ON(ulmk_dbg_policy & ULMK_DBG_POLICY_TRIGGER);
		psi_emergency_trigger();
		psi_emerg_trigger_jiffies = now;
		ret = true;
	} else if (wdog_expired) {
		mutex_lock(&oom_lock);
		ret = out_of_memory(&oc);
		mutex_unlock(&oom_lock);
		BUG_ON(!ret && ulmk_dbg_policy & ULMK_DBG_POLICY_POSITIVE_ADJ);
	}

	mutex_unlock(&ulmk_retry_lock);
	return ret;
}

void ulmk_watchdog_fn(struct timer_list *t)
{
	atomic64_set(&ulmk_wdog_expired, 1);
	BUG_ON(ulmk_dbg_policy & ULMK_DBG_POLICY_WDOG);
}

void ulmk_watchdog_pet(struct timer_list *t)
{
	del_timer_sync(t);
	atomic64_set(&ulmk_wdog_expired, 0);
	atomic64_set(&ulmk_watchdog_pet_jiffies, jiffies);
}

void ulmk_update_last_kill(void)
{
	atomic64_set(&ulmk_kill_jiffies, jiffies);
}
#endif

#ifdef CONFIG_NUMA
/**
 * has_intersects_mems_allowed() - check task eligiblity for kill
 * @start: task struct of which task to consider
 * @mask: nodemask passed to page allocator for mempolicy ooms
 *
 * Task eligibility is determined by whether or not a candidate task, @tsk,
 * shares the same mempolicy nodes as current if it is bound by such a policy
 * and whether or not it has the same set of allowed cpuset nodes.
 */
static bool has_intersects_mems_allowed(struct task_struct *start,
					const nodemask_t *mask)
{
	struct task_struct *tsk;
	bool ret = false;

	rcu_read_lock();
	for_each_thread(start, tsk) {
		if (mask) {
			/*
			 * If this is a mempolicy constrained oom, tsk's
			 * cpuset is irrelevant.  Only return true if its
			 * mempolicy intersects current, otherwise it may be
			 * needlessly killed.
			 */
			ret = mempolicy_nodemask_intersects(tsk, mask);
		} else {
			/*
			 * This is not a mempolicy constrained oom, so only
			 * check the mems of tsk's cpuset.
			 */
			ret = cpuset_mems_allowed_intersects(current, tsk);
		}
		if (ret)
			break;
	}
	rcu_read_unlock();

	return ret;
}
#else
static bool has_intersects_mems_allowed(struct task_struct *tsk,
					const nodemask_t *mask)
{
	return true;
}
#endif /* CONFIG_NUMA */

/*
 * The process p may have detached its own ->mm while exiting or through
 * use_mm(), but one or more of its subthreads may still have a valid
 * pointer.  Return p, or any of its subthreads with a valid ->mm, with
 * task_lock() held.
 */
struct task_struct *find_lock_task_mm(struct task_struct *p)
{
	struct task_struct *t;

	rcu_read_lock();

	for_each_thread(p, t) {
		task_lock(t);
		if (likely(t->mm))
			goto found;
		task_unlock(t);
	}
	t = NULL;
found:
	rcu_read_unlock();

	return t;
}

/*
 * order == -1 means the oom kill is required by sysrq, otherwise only
 * for display purposes.
 */
static inline bool is_sysrq_oom(struct oom_control *oc)
{
	return oc->order == -1;
}

static inline bool is_memcg_oom(struct oom_control *oc)
{
	return oc->memcg != NULL;
}

/* return true if the task is not adequate as candidate victim task. */
static bool oom_unkillable_task(struct task_struct *p,
		struct mem_cgroup *memcg, const nodemask_t *nodemask)
{
	if (is_global_init(p))
		return true;
	if (p->flags & PF_KTHREAD)
		return true;

	/* When mem_cgroup_out_of_memory() and p is not member of the group */
	if (memcg && !task_in_mem_cgroup(p, memcg))
		return true;

	/* p may not have freeable memory in nodemask */
	if (!has_intersects_mems_allowed(p, nodemask))
		return true;

	return false;
}

/*
 * Print out unreclaimble slabs info when unreclaimable slabs amount is greater
 * than all user memory (LRU pages)
 */
static bool is_dump_unreclaim_slabs(void)
{
	unsigned long nr_lru;

	nr_lru = global_node_page_state(NR_ACTIVE_ANON) +
		 global_node_page_state(NR_INACTIVE_ANON) +
		 global_node_page_state(NR_ACTIVE_FILE) +
		 global_node_page_state(NR_INACTIVE_FILE) +
		 global_node_page_state(NR_ISOLATED_ANON) +
		 global_node_page_state(NR_ISOLATED_FILE) +
		 global_node_page_state(NR_UNEVICTABLE);

	return (global_node_page_state(NR_SLAB_UNRECLAIMABLE) > nr_lru);
}

/**
 * oom_badness - heuristic function to determine which candidate task to kill
 * @p: task struct of which task we should calculate
 * @totalpages: total present RAM allowed for page allocation
 * @memcg: task's memory controller, if constrained
 * @nodemask: nodemask passed to page allocator for mempolicy ooms
 *
 * The heuristic for determining which task to kill is made to be as simple and
 * predictable as possible.  The goal is to return the highest value for the
 * task consuming the most memory to avoid subsequent oom failures.
 */
unsigned long oom_badness(struct task_struct *p, struct mem_cgroup *memcg,
			  const nodemask_t *nodemask, unsigned long totalpages,
			  bool only_positive_adj)
{
	long points;
	long adj;

	if (oom_unkillable_task(p, memcg, nodemask))
		return 0;

	p = find_lock_task_mm(p);
	if (!p)
		return 0;

	/*
	 * Do not even consider tasks which are explicitly marked oom
	 * unkillable or have been already oom reaped or the are in
	 * the middle of vfork
	 */
	adj = (long)p->signal->oom_score_adj;
	if (adj == OOM_SCORE_ADJ_MIN ||
			(only_positive_adj && adj < 0) ||
			test_bit(MMF_OOM_SKIP, &p->mm->flags) ||
			in_vfork(p)) {
		task_unlock(p);
		return 0;
	}

	/*
	 * The baseline for the badness score is the proportion of RAM that each
	 * task's rss, pagetable and swap space use.
	 */
	points = get_mm_rss(p->mm) + get_mm_counter(p->mm, MM_SWAPENTS) +
		mm_pgtables_bytes(p->mm) / PAGE_SIZE;
	task_unlock(p);

	/* Normalize to oom_score_adj units */
	adj *= totalpages / 1000;
	points += adj;

	/*
	 * Never return 0 for an eligible task regardless of the root bonus and
	 * oom_score_adj (oom_score_adj can't be OOM_SCORE_ADJ_MIN here).
	 */
	return points > 0 ? points : 1;
}

enum oom_constraint {
	CONSTRAINT_NONE,
	CONSTRAINT_CPUSET,
	CONSTRAINT_MEMORY_POLICY,
	CONSTRAINT_MEMCG,
};

/*
 * Determine the type of allocation constraint.
 */
static enum oom_constraint constrained_alloc(struct oom_control *oc)
{
	struct zone *zone;
	struct zoneref *z;
	enum zone_type high_zoneidx = gfp_zone(oc->gfp_mask);
	bool cpuset_limited = false;
	int nid;

	if (is_memcg_oom(oc)) {
		oc->totalpages = mem_cgroup_get_max(oc->memcg) ?: 1;
		return CONSTRAINT_MEMCG;
	}

	/* Default to all available memory */
	oc->totalpages = totalram_pages + total_swap_pages;

	if (!IS_ENABLED(CONFIG_NUMA))
		return CONSTRAINT_NONE;

	if (!oc->zonelist)
		return CONSTRAINT_NONE;
	/*
	 * Reach here only when __GFP_NOFAIL is used. So, we should avoid
	 * to kill current.We have to random task kill in this case.
	 * Hopefully, CONSTRAINT_THISNODE...but no way to handle it, now.
	 */
	if (oc->gfp_mask & __GFP_THISNODE)
		return CONSTRAINT_NONE;

	/*
	 * This is not a __GFP_THISNODE allocation, so a truncated nodemask in
	 * the page allocator means a mempolicy is in effect.  Cpuset policy
	 * is enforced in get_page_from_freelist().
	 */
	if (oc->nodemask &&
	    !nodes_subset(node_states[N_MEMORY], *oc->nodemask)) {
		oc->totalpages = total_swap_pages;
		for_each_node_mask(nid, *oc->nodemask)
			oc->totalpages += node_spanned_pages(nid);
		return CONSTRAINT_MEMORY_POLICY;
	}

	/* Check this allocation failure is caused by cpuset's wall function */
	for_each_zone_zonelist_nodemask(zone, z, oc->zonelist,
			high_zoneidx, oc->nodemask)
		if (!cpuset_zone_allowed(zone, oc->gfp_mask))
			cpuset_limited = true;

	if (cpuset_limited) {
		oc->totalpages = total_swap_pages;
		for_each_node_mask(nid, cpuset_current_mems_allowed)
			oc->totalpages += node_spanned_pages(nid);
		return CONSTRAINT_CPUSET;
	}
	return CONSTRAINT_NONE;
}

static int oom_evaluate_task(struct task_struct *task, void *arg)
{
	struct oom_control *oc = arg;
	unsigned long points;

	if (oom_unkillable_task(task, NULL, oc->nodemask))
		goto next;

	/*
	 * This task already has access to memory reserves and is being killed.
	 * Don't allow any other task to have access to the reserves unless
	 * the task has MMF_OOM_SKIP because chances that it would release
	 * any memory is quite low.
	 */
	if (!is_sysrq_oom(oc) && tsk_is_oom_victim(task)) {
		if (test_bit(MMF_OOM_SKIP, &task->signal->oom_mm->flags))
			goto next;
		goto abort;
	}

	/*
	 * If task is allocating a lot of memory and has been marked to be
	 * killed first if it triggers an oom, then select it.
	 */
	if (oom_task_origin(task)) {
		points = ULONG_MAX;
		goto select;
	}

	points = oom_badness(task, NULL, oc->nodemask, oc->totalpages,
				oc->only_positive_adj);
	if (!points || points < oc->chosen_points)
		goto next;

	/* Prefer thread group leaders for display purposes */
	if (points == oc->chosen_points && thread_group_leader(oc->chosen))
		goto next;
select:
	if (oc->chosen)
		put_task_struct(oc->chosen);
	get_task_struct(task);
	oc->chosen = task;
	oc->chosen_points = points;
next:
	return 0;
abort:
	if (oc->chosen)
		put_task_struct(oc->chosen);
	oc->chosen = (void *)-1UL;
	return 1;
}

/*
 * Simple selection loop. We choose the process with the highest number of
 * 'points'. In case scan was aborted, oc->chosen is set to -1.
 */
static void select_bad_process(struct oom_control *oc)
{
	if (is_memcg_oom(oc))
		mem_cgroup_scan_tasks(oc->memcg, oom_evaluate_task, oc);
	else {
		struct task_struct *p;

		rcu_read_lock();
		for_each_process(p)
			if (oom_evaluate_task(p, oc))
				break;
		rcu_read_unlock();
	}

	oc->chosen_points = oc->chosen_points * 1000 / oc->totalpages;
}

/**
 * dump_tasks - dump current memory state of all system tasks
 * @memcg: current's memory controller, if constrained
 * @nodemask: nodemask passed to page allocator for mempolicy ooms
 *
 * Dumps the current memory state of all eligible tasks.  Tasks not in the same
 * memcg, not in the same cpuset, or bound to a disjoint set of mempolicy nodes
 * are not shown.
 * State information includes task's pid, uid, tgid, vm size, rss,
 * pgtables_bytes, swapents, oom_score_adj value, and name.
 */
void dump_tasks(struct mem_cgroup *memcg, const nodemask_t *nodemask)
{
	struct task_struct *p;
	struct task_struct *task;

	pr_info("Tasks state (memory values in pages):\n");
	pr_info("[  pid  ]   uid  tgid total_vm      rss pgtables_bytes swapents oom_score_adj name\n");
	rcu_read_lock();
	for_each_process(p) {
		if (oom_unkillable_task(p, memcg, nodemask))
			continue;

		task = find_lock_task_mm(p);
		if (!task) {
			/*
			 * This is a kthread or all of p's threads have already
			 * detached their mm's.  There's no need to report
			 * them; they can't be oom killed anyway.
			 */
			continue;
		}

		pr_info("[%7d] %5d %5d %8lu %8lu %8ld %8lu         %5hd %s\n",
			task->pid, from_kuid(&init_user_ns, task_uid(task)),
			task->tgid, task->mm->total_vm, get_mm_rss(task->mm),
			mm_pgtables_bytes(task->mm),
			get_mm_counter(task->mm, MM_SWAPENTS),
			task->signal->oom_score_adj, task->comm);
		task_unlock(task);
	}
	rcu_read_unlock();
}

static void dump_header(struct oom_control *oc, struct task_struct *p)
{
	pr_warn("%s invoked oom-killer: gfp_mask=%#x(%pGg), nodemask=%*pbl, order=%d, oom_score_adj=%hd\n",
		current->comm, oc->gfp_mask, &oc->gfp_mask,
		nodemask_pr_args(oc->nodemask), oc->order,
			current->signal->oom_score_adj);
	if (!IS_ENABLED(CONFIG_COMPACTION) && oc->order)
		pr_warn("COMPACTION is disabled!!!\n");

	cpuset_print_current_mems_allowed();
	dump_stack();
	if (is_memcg_oom(oc))
		mem_cgroup_print_oom_info(oc->memcg, p);
	else {
		show_mem(SHOW_MEM_FILTER_NODES, oc->nodemask);
		if (is_dump_unreclaim_slabs())
			dump_unreclaimable_slab();

		show_mem_call_notifiers();
	}

	if (sysctl_oom_dump_tasks)
		dump_tasks(oc->memcg, oc->nodemask);
}

/*
 * Number of OOM victims in flight
 */
static atomic_t oom_victims = ATOMIC_INIT(0);
static DECLARE_WAIT_QUEUE_HEAD(oom_victims_wait);

static bool oom_killer_disabled __read_mostly;

#define K(x) ((x) << (PAGE_SHIFT-10))

/*
 * task->mm can be NULL if the task is the exited group leader.  So to
 * determine whether the task is using a particular mm, we examine all the
 * task's threads: if one of those is using this mm then this task was also
 * using it.
 */
bool process_shares_mm(struct task_struct *p, struct mm_struct *mm)
{
	struct task_struct *t;

	for_each_thread(p, t) {
		struct mm_struct *t_mm = READ_ONCE(t->mm);
		if (t_mm)
			return t_mm == mm;
	}
	return false;
}

#ifdef CONFIG_MMU
/*
 * OOM Reaper kernel thread which tries to reap the memory used by the OOM
 * victim (if that is possible) to help the OOM killer to move on.
 */
static struct task_struct *oom_reaper_th;
static DECLARE_WAIT_QUEUE_HEAD(oom_reaper_wait);
static struct task_struct *oom_reaper_list;
static DEFINE_SPINLOCK(oom_reaper_lock);

bool __oom_reap_task_mm(struct mm_struct *mm)
{
	struct vm_area_struct *vma;
	bool ret = true;

	/*
	 * Tell all users of get_user/copy_from_user etc... that the content
	 * is no longer stable. No barriers really needed because unmapping
	 * should imply barriers already and the reader would hit a page fault
	 * if it stumbled over a reaped memory.
	 */
	set_bit(MMF_UNSTABLE, &mm->flags);

	for (vma = mm->mmap ; vma; vma = vma->vm_next) {
		if (!can_madv_dontneed_vma(vma))
			continue;

		/*
		 * Only anonymous pages have a good chance to be dropped
		 * without additional steps which we cannot afford as we
		 * are OOM already.
		 *
		 * We do not even care about fs backed pages because all
		 * which are reclaimable have already been reclaimed and
		 * we do not want to block exit_mmap by keeping mm ref
		 * count elevated without a good reason.
		 */
		if (vma_is_anonymous(vma) || !(vma->vm_flags & VM_SHARED)) {
			const unsigned long start = vma->vm_start;
			const unsigned long end = vma->vm_end;
			struct mmu_gather tlb;

			tlb_gather_mmu(&tlb, mm, start, end);
			if (mmu_notifier_invalidate_range_start_nonblock(mm, start, end)) {
				tlb_finish_mmu(&tlb, start, end);
				ret = false;
				continue;
			}
			unmap_page_range(&tlb, vma, start, end, NULL);
			mmu_notifier_invalidate_range_end(mm, start, end);
			tlb_finish_mmu(&tlb, start, end);
		}
	}

	return ret;
}

/*
 * Reaps the address space of the give task.
 *
 * Returns true on success and false if none or part of the address space
 * has been reclaimed and the caller should retry later.
 */
static bool oom_reap_task_mm(struct task_struct *tsk, struct mm_struct *mm)
{
	bool ret = true;

	if (!down_read_trylock(&mm->mmap_sem)) {
		trace_skip_task_reaping(tsk->pid);
		return false;
	}

	/*
	 * MMF_OOM_SKIP is set by exit_mmap when the OOM reaper can't
	 * work on the mm anymore. The check for MMF_OOM_SKIP must run
	 * under mmap_sem for reading because it serializes against the
	 * down_write();up_write() cycle in exit_mmap().
	 */
	if (test_bit(MMF_OOM_SKIP, &mm->flags)) {
		trace_skip_task_reaping(tsk->pid);
		goto out_unlock;
	}

	trace_start_task_reaping(tsk->pid);

	/* failed to reap part of the address space. Try again later */
	ret = __oom_reap_task_mm(mm);
	if (!ret)
		goto out_finish;

	pr_info("oom_reaper: reaped process %d (%s), now anon-rss:%lukB, file-rss:%lukB, shmem-rss:%lukB\n",
			task_pid_nr(tsk), tsk->comm,
			K(get_mm_counter(mm, MM_ANONPAGES)),
			K(get_mm_counter(mm, MM_FILEPAGES)),
			K(get_mm_counter(mm, MM_SHMEMPAGES)));
out_finish:
	trace_finish_task_reaping(tsk->pid);
out_unlock:
	up_read(&mm->mmap_sem);

	return ret;
}

#define MAX_OOM_REAP_RETRIES 10
static void oom_reap_task(struct task_struct *tsk)
{
	int attempts = 0;
	struct mm_struct *mm = tsk->signal->oom_mm;

	/* Retry the down_read_trylock(mmap_sem) a few times */
	while (attempts++ < MAX_OOM_REAP_RETRIES && !oom_reap_task_mm(tsk, mm))
		schedule_timeout_idle(HZ/10);

	if (attempts <= MAX_OOM_REAP_RETRIES ||
	    test_bit(MMF_OOM_SKIP, &mm->flags))
		goto done;

	pr_info("oom_reaper: unable to reap pid:%d (%s)\n",
		task_pid_nr(tsk), tsk->comm);
	debug_show_all_locks();

done:
	tsk->oom_reaper_list = NULL;

	/*
	 * Hide this mm from OOM killer because it has been either reaped or
	 * somebody can't call up_write(mmap_sem).
	 */
	set_bit(MMF_OOM_SKIP, &mm->flags);

	/* Drop a reference taken by wake_oom_reaper */
	put_task_struct(tsk);
}

static int oom_reaper(void *unused)
{
	while (true) {
		struct task_struct *tsk = NULL;

		wait_event_freezable(oom_reaper_wait, oom_reaper_list != NULL);
		spin_lock(&oom_reaper_lock);
		if (oom_reaper_list != NULL) {
			tsk = oom_reaper_list;
			oom_reaper_list = tsk->oom_reaper_list;
		}
		spin_unlock(&oom_reaper_lock);

		if (tsk)
			oom_reap_task(tsk);
	}

	return 0;
}

static void wake_oom_reaper(struct task_struct *tsk)
{
	/*
	 * Move the lock here to avoid scenario of queuing
	 * the same task by both OOM killer and any other SIGKILL
	 * path.
	 */
	spin_lock(&oom_reaper_lock);

	/* mm is already queued? */
	if (test_and_set_bit(MMF_OOM_REAP_QUEUED, &tsk->signal->oom_mm->flags)) {
		spin_unlock(&oom_reaper_lock);
		return;
	}

	get_task_struct(tsk);

	tsk->oom_reaper_list = oom_reaper_list;
	oom_reaper_list = tsk;
	spin_unlock(&oom_reaper_lock);
	trace_wake_reaper(tsk->pid);
	wake_up(&oom_reaper_wait);
}

static int __init oom_init(void)
{
	oom_reaper_th = kthread_run(oom_reaper, NULL, "oom_reaper");
	return 0;
}
subsys_initcall(oom_init)
#else
static inline void wake_oom_reaper(struct task_struct *tsk)
{
}
#endif /* CONFIG_MMU */

static void __mark_oom_victim(struct task_struct *tsk)
{
	struct mm_struct *mm = tsk->mm;

	if (!cmpxchg(&tsk->signal->oom_mm, NULL, mm)) {
		mmgrab(tsk->signal->oom_mm);
		set_bit(MMF_OOM_VICTIM, &mm->flags);
	}
}

/**
 * mark_oom_victim - mark the given task as OOM victim
 * @tsk: task to mark
 *
 * Has to be called with oom_lock held and never after
 * oom has been disabled already.
 *
 * tsk->mm has to be non NULL and caller has to guarantee it is stable (either
 * under task_lock or operate on the current).
 */
static void mark_oom_victim(struct task_struct *tsk)
{
	WARN_ON(oom_killer_disabled);
	/* OOM killer might race with memcg OOM */
	if (test_and_set_tsk_thread_flag(tsk, TIF_MEMDIE))
		return;

	/* oom_mm is bound to the signal struct life time. */
	__mark_oom_victim(tsk);

	/*
	 * Make sure that the task is woken up from uninterruptible sleep
	 * if it is frozen because OOM killer wouldn't be able to free
	 * any memory and livelock. freezing_slow_path will tell the freezer
	 * that TIF_MEMDIE tasks should be ignored.
	 */
	__thaw_task(tsk);
	atomic_inc(&oom_victims);
	trace_mark_victim(tsk->pid);
}

/**
 * exit_oom_victim - note the exit of an OOM victim
 */
void exit_oom_victim(void)
{
	clear_thread_flag(TIF_MEMDIE);

	if (!atomic_dec_return(&oom_victims))
		wake_up_all(&oom_victims_wait);
}

/**
 * oom_killer_enable - enable OOM killer
 */
void oom_killer_enable(void)
{
	oom_killer_disabled = false;
	pr_info("OOM killer enabled.\n");
}

/**
 * oom_killer_disable - disable OOM killer
 * @timeout: maximum timeout to wait for oom victims in jiffies
 *
 * Forces all page allocations to fail rather than trigger OOM killer.
 * Will block and wait until all OOM victims are killed or the given
 * timeout expires.
 *
 * The function cannot be called when there are runnable user tasks because
 * the userspace would see unexpected allocation failures as a result. Any
 * new usage of this function should be consulted with MM people.
 *
 * Returns true if successful and false if the OOM killer cannot be
 * disabled.
 */
bool oom_killer_disable(signed long timeout)
{
	signed long ret;

	/*
	 * Make sure to not race with an ongoing OOM killer. Check that the
	 * current is not killed (possibly due to sharing the victim's memory).
	 */
	if (mutex_lock_killable(&oom_lock))
		return false;
	oom_killer_disabled = true;
	mutex_unlock(&oom_lock);

	ret = wait_event_interruptible_timeout(oom_victims_wait,
			!atomic_read(&oom_victims), timeout);
	if (ret <= 0) {
		oom_killer_enable();
		return false;
	}
	pr_info("OOM killer disabled.\n");

	return true;
}

static inline bool __task_will_free_mem(struct task_struct *task)
{
	struct signal_struct *sig = task->signal;

	/*
	 * A coredumping process may sleep for an extended period in exit_mm(),
	 * so the oom killer cannot assume that the process will promptly exit
	 * and release memory.
	 */
	if (sig->flags & SIGNAL_GROUP_COREDUMP)
		return false;

	if (sig->flags & SIGNAL_GROUP_EXIT)
		return true;

	if (thread_group_empty(task) && (task->flags & PF_EXITING))
		return true;

	return false;
}

/*
 * Checks whether the given task is dying or exiting and likely to
 * release its address space. This means that all threads and processes
 * sharing the same mm have to be killed or exiting.
 * Caller has to make sure that task->mm is stable (hold task_lock or
 * it operates on the current).
 */
static bool task_will_free_mem(struct task_struct *task)
{
	struct mm_struct *mm = task->mm;
	struct task_struct *p;
	bool ret = true;

	/*
	 * Skip tasks without mm because it might have passed its exit_mm and
	 * exit_oom_victim. oom_reaper could have rescued that but do not rely
	 * on that for now. We can consider find_lock_task_mm in future.
	 */
	if (!mm)
		return false;

	if (!__task_will_free_mem(task))
		return false;

	/*
	 * This task has already been drained by the oom reaper so there are
	 * only small chances it will free some more
	 */
	if (test_bit(MMF_OOM_SKIP, &mm->flags))
		return false;

	if (atomic_read(&mm->mm_users) <= 1)
		return true;

	/*
	 * Make sure that all tasks which share the mm with the given tasks
	 * are dying as well to make sure that a) nobody pins its mm and
	 * b) the task is also reapable by the oom reaper.
	 */
	rcu_read_lock();
	for_each_process(p) {
		if (!process_shares_mm(p, mm))
			continue;
		if (same_thread_group(task, p))
			continue;
		ret = __task_will_free_mem(p);
		if (!ret)
			break;
	}
	rcu_read_unlock();

	return ret;
}

static void __oom_kill_process(struct task_struct *victim)
{
	struct task_struct *p;
	struct mm_struct *mm;
	bool can_oom_reap = true;

	p = find_lock_task_mm(victim);
	if (!p) {
		put_task_struct(victim);
		return;
	} else if (victim != p) {
		get_task_struct(p);
		put_task_struct(victim);
		victim = p;
	}

	/* Get a reference to safely compare mm after task_unlock(victim) */
	mm = victim->mm;
	mmgrab(mm);

	/* Raise event before sending signal: task reaper must see this */
	count_vm_event(OOM_KILL);
	memcg_memory_event_mm(mm, MEMCG_OOM_KILL);

	/*
	 * We should send SIGKILL before granting access to memory reserves
	 * in order to prevent the OOM victim from depleting the memory
	 * reserves from the user space under its control.
	 */
	do_send_sig_info(SIGKILL, SEND_SIG_FORCED, victim, PIDTYPE_TGID);
	mark_oom_victim(victim);
	pr_err("Killed process %d (%s) total-vm:%lukB, anon-rss:%lukB, file-rss:%lukB, shmem-rss:%lukB oom_score_adj=%hd\n",
		task_pid_nr(victim), victim->comm, K(victim->mm->total_vm),
		K(get_mm_counter(victim->mm, MM_ANONPAGES)),
		K(get_mm_counter(victim->mm, MM_FILEPAGES)),
		K(get_mm_counter(victim->mm, MM_SHMEMPAGES)),
		p->signal->oom_score_adj);
	task_unlock(victim);

	/*
	 * Kill all user processes sharing victim->mm in other thread groups, if
	 * any.  They don't get access to memory reserves, though, to avoid
	 * depletion of all memory.  This prevents mm->mmap_sem livelock when an
	 * oom killed thread cannot exit because it requires the semaphore and
	 * its contended by another thread trying to allocate memory itself.
	 * That thread will now get access to memory reserves since it has a
	 * pending fatal signal.
	 */
	rcu_read_lock();
	for_each_process(p) {
		if (!process_shares_mm(p, mm))
			continue;
		if (same_thread_group(p, victim))
			continue;
		if (is_global_init(p)) {
			can_oom_reap = false;
			set_bit(MMF_OOM_SKIP, &mm->flags);
			pr_info("oom killer %d (%s) has mm pinned by %d (%s)\n",
					task_pid_nr(victim), victim->comm,
					task_pid_nr(p), p->comm);
			continue;
		}
		/*
		 * No use_mm() user needs to read from the userspace so we are
		 * ok to reap it.
		 */
		if (unlikely(p->flags & PF_KTHREAD))
			continue;
		do_send_sig_info(SIGKILL, SEND_SIG_FORCED, p, PIDTYPE_TGID);
	}
	rcu_read_unlock();

	if (can_oom_reap)
		wake_oom_reaper(victim);

	mmdrop(mm);
	put_task_struct(victim);
}
#undef K

/*
 * Kill provided task unless it's secured by setting
 * oom_score_adj to OOM_SCORE_ADJ_MIN.
 */
static int oom_kill_memcg_member(struct task_struct *task, void *unused)
{
	if (task->signal->oom_score_adj != OOM_SCORE_ADJ_MIN &&
	    !is_global_init(task)) {
		get_task_struct(task);
		__oom_kill_process(task);
	}
	return 0;
}

static void oom_kill_process(struct oom_control *oc, const char *message,
				bool quiet)
{
	struct task_struct *p = oc->chosen;
	unsigned int points = oc->chosen_points;
	struct task_struct *victim = p;
	struct task_struct *child;
	struct task_struct *t;
	struct mem_cgroup *oom_group;
	unsigned int victim_points = 0;
	static DEFINE_RATELIMIT_STATE(oom_rs, DEFAULT_RATELIMIT_INTERVAL,
					      DEFAULT_RATELIMIT_BURST);

	/*
	 * If the task is already exiting, don't alarm the sysadmin or kill
	 * its children or threads, just give it access to memory reserves
	 * so it can die quickly
	 */
	task_lock(p);
	if (task_will_free_mem(p)) {
		mark_oom_victim(p);
		wake_oom_reaper(p);
		task_unlock(p);
		put_task_struct(p);
		return;
	}
	task_unlock(p);

	if (!quiet && __ratelimit(&oom_rs))
		dump_header(oc, p);

	pr_err("%s: Kill process %d (%s) score %u or sacrifice child\n",
		message, task_pid_nr(p), p->comm, points);

	/*
	 * If any of p's children has a different mm and is eligible for kill,
	 * the one with the highest oom_badness() score is sacrificed for its
	 * parent.  This attempts to lose the minimal amount of work done while
	 * still freeing memory.
	 */
	read_lock(&tasklist_lock);

	/*
	 * The task 'p' might have already exited before reaching here. The
	 * put_task_struct() will free task_struct 'p' while the loop still try
	 * to access the field of 'p', so, get an extra reference.
	 */
	get_task_struct(p);
	for_each_thread(p, t) {
		list_for_each_entry(child, &t->children, sibling) {
			unsigned int child_points;

			if (process_shares_mm(child, p->mm))
				continue;
			/*
			 * oom_badness() returns 0 if the thread is unkillable
			 */
			child_points = oom_badness(child,
				oc->memcg, oc->nodemask, oc->totalpages,
				oc->only_positive_adj);
			if (child_points > victim_points) {
				put_task_struct(victim);
				victim = child;
				victim_points = child_points;
				get_task_struct(victim);
			}
		}
	}
	put_task_struct(p);
	read_unlock(&tasklist_lock);

	/*
	 * Do we need to kill the entire memory cgroup?
	 * Or even one of the ancestor memory cgroups?
	 * Check this out before killing the victim task.
	 */
	oom_group = mem_cgroup_get_oom_group(victim, oc->memcg);

	/*
	 * If ->only_positive_adj = true in oom context,
	 * consider them as kill from ulmk.
	 */
	if (oc->only_positive_adj)
		ulmk_update_last_kill();
	__oom_kill_process(victim);

	/*
	 * If necessary, kill all tasks in the selected memory cgroup.
	 */
	if (oom_group) {
		mem_cgroup_print_oom_group(oom_group);
		mem_cgroup_scan_tasks(oom_group, oom_kill_memcg_member, NULL);
		mem_cgroup_put(oom_group);
	}
}

/*
 * Determines whether the kernel must panic because of the panic_on_oom sysctl.
 */
static void check_panic_on_oom(struct oom_control *oc,
			       enum oom_constraint constraint)
{
	if (likely(!sysctl_panic_on_oom))
		return;
	if (sysctl_panic_on_oom != 2) {
		/*
		 * panic_on_oom == 1 only affects CONSTRAINT_NONE, the kernel
		 * does not panic for cpuset, mempolicy, or memcg allocation
		 * failures.
		 */
		if (constraint != CONSTRAINT_NONE)
			return;
	}
	/* Do not panic for oom kills triggered by sysrq */
	if (is_sysrq_oom(oc) || oc->only_positive_adj)
		return;
	dump_header(oc, NULL);
	panic("Out of memory: %s panic_on_oom is enabled\n",
		sysctl_panic_on_oom == 2 ? "compulsory" : "system-wide");
}

static BLOCKING_NOTIFIER_HEAD(oom_notify_list);

int register_oom_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&oom_notify_list, nb);
}
EXPORT_SYMBOL_GPL(register_oom_notifier);

int unregister_oom_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&oom_notify_list, nb);
}
EXPORT_SYMBOL_GPL(unregister_oom_notifier);

/**
 * out_of_memory - kill the "best" process when we run out of memory
 * @oc: pointer to struct oom_control
 *
 * If we run out of memory, we have the choice between either
 * killing a random task (bad), letting the system crash (worse)
 * OR try to be smart about which process to kill. Note that we
 * don't have to be perfect here, we just have to be good.
 */
bool out_of_memory(struct oom_control *oc)
{
	unsigned long freed = 0;
	enum oom_constraint constraint = CONSTRAINT_NONE;

	if (oom_killer_disabled)
		return false;

	if (try_online_one_block(numa_node_id())) {
		/* Got some memory back */
		WARN(1, "OOM killer had to online a memory block\n");
		return true;
	}

	if (!is_memcg_oom(oc)) {
		blocking_notifier_call_chain(&oom_notify_list, 0, &freed);
		if (freed > 0)
			/* Got some memory back in the last second. */
			return true;
	}

	/*
	 * If current has a pending SIGKILL or is exiting, then automatically
	 * select it.  The goal is to allow it to allocate so that it may
	 * quickly exit and free its memory.
	 */
	if (task_will_free_mem(current)) {
		mark_oom_victim(current);
		wake_oom_reaper(current);
		return true;
	}

	/*
	 * The OOM killer does not compensate for IO-less reclaim.
	 * pagefault_out_of_memory lost its gfp context so we have to
	 * make sure exclude 0 mask - all other users should have at least
	 * ___GFP_DIRECT_RECLAIM to get here. But mem_cgroup_oom() has to
	 * invoke the OOM killer even if it is a GFP_NOFS allocation.
	 */
	if (oc->gfp_mask && !(oc->gfp_mask & __GFP_FS) && !is_memcg_oom(oc))
		return true;

	/*
	 * Check if there were limitations on the allocation (only relevant for
	 * NUMA and memcg) that may require different handling.
	 */
	constraint = constrained_alloc(oc);
	if (constraint != CONSTRAINT_MEMORY_POLICY)
		oc->nodemask = NULL;
	check_panic_on_oom(oc, constraint);

	if (!is_memcg_oom(oc) && sysctl_oom_kill_allocating_task &&
	    current->mm && !oom_unkillable_task(current, NULL, oc->nodemask) &&
	    current->signal->oom_score_adj != OOM_SCORE_ADJ_MIN) {
		get_task_struct(current);
		oc->chosen = current;
		oom_kill_process(oc, "Out of memory (oom_kill_allocating_task)",
				 false);
		return true;
	}

	select_bad_process(oc);
	/* Found nothing?!?! */
	if (!oc->chosen) {
		dump_header(oc, NULL);
		pr_warn("Out of memory and no killable processes...\n");
		/*
		 * If we got here due to an actual allocation at the
		 * system level, we cannot survive this and will enter
		 * an endless loop in the allocator. Bail out now.
		 */
		if (!is_sysrq_oom(oc) && !is_memcg_oom(oc) &&
		    !oc->only_positive_adj)
			panic("System is deadlocked on memory\n");
	}
	if (oc->chosen && oc->chosen != (void *)-1UL)
		oom_kill_process(oc, !is_memcg_oom(oc) ? "Out of memory" :
				 "Memory cgroup out of memory",
			IS_ENABLED(CONFIG_HAVE_USERSPACE_LOW_MEMORY_KILLER));
	return !!oc->chosen;
}

/*
 * The pagefault handler calls here because it is out of memory, so kill a
 * memory-hogging task. If oom_lock is held by somebody else, a parallel oom
 * killing is already in progress so do nothing.
 */
void pagefault_out_of_memory(void)
{
	struct oom_control oc = {
		.zonelist = NULL,
		.nodemask = NULL,
		.memcg = NULL,
		.gfp_mask = 0,
		.order = 0,
	};

	if (IS_ENABLED(CONFIG_HAVE_LOW_MEMORY_KILLER) ||
	    IS_ENABLED(CONFIG_HAVE_USERSPACE_LOW_MEMORY_KILLER))
		return;

	if (mem_cgroup_oom_synchronize(true))
		return;

	if (!mutex_trylock(&oom_lock))
		return;
	out_of_memory(&oc);
	mutex_unlock(&oom_lock);
}

void add_to_oom_reaper(struct task_struct *p)
{
	static DEFINE_RATELIMIT_STATE(reaper_rs, DEFAULT_RATELIMIT_INTERVAL,
						 DEFAULT_RATELIMIT_BURST);

	if (!sysctl_reap_mem_on_sigkill)
		return;

	p = find_lock_task_mm(p);
	if (!p)
		return;

	get_task_struct(p);
	if (task_will_free_mem(p)) {
		__mark_oom_victim(p);
		wake_oom_reaper(p);
	}

	task_unlock(p);

	if (!strcmp(current->comm, ULMK_MAGIC) && __ratelimit(&reaper_rs)
			&& p->signal->oom_score_adj == 0) {
		show_mem(SHOW_MEM_FILTER_NODES, NULL);
		show_mem_call_notifiers();
	}

	put_task_struct(p);
}

/*
 * Should be called prior to sending sigkill. To guarantee that the
 * process to-be-killed is still untouched.
 */
void check_panic_on_foreground_kill(struct task_struct *p)
{
	if (unlikely(!strcmp(current->comm, ULMK_MAGIC)
			&& p->signal->oom_score_adj == 0
			&& panic_on_adj_zero)) {
		show_mem(SHOW_MEM_FILTER_NODES, NULL);
		show_mem_call_notifiers();
		panic("Attempt to kill foreground task: %s", p->comm);
	}
}
