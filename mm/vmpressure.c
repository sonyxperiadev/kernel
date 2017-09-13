/*
 * Linux VM pressure
 *
 * Copyright 2012 Linaro Ltd.
 *		  Anton Vorontsov <anton.vorontsov@linaro.org>
 *
 * Based on ideas from Andrew Morton, David Rientjes, KOSAKI Motohiro,
 * Leonid Moiseichuk, Mel Gorman, Minchan Kim and Pekka Enberg.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/cgroup.h>
#include <linux/fs.h>
#include <linux/log2.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/vmstat.h>
#include <linux/eventfd.h>
#include <linux/swap.h>
#include <linux/printk.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/vmpressure.h>

/*
 * The window size (vmpressure_win) is the number of scanned pages before
 * we try to analyze scanned/reclaimed ratio. So the window is used as a
 * rate-limit tunable for the "low" level notification, and also for
 * averaging the ratio for medium/critical levels. Using small window
 * sizes can cause lot of false positives, but too big window size will
 * delay the notifications.
 *
 * As the vmscan reclaimer logic works with chunks which are multiple of
 * SWAP_CLUSTER_MAX, it makes sense to use it for the window size as well.
 *
 * TODO: Make the window size depend on machine size, as we do for vmstat
 * thresholds. Currently we set it to 512 pages (2MB for 4KB pages).
 */
static const unsigned long vmpressure_win = SWAP_CLUSTER_MAX * 16;

/*
 * These thresholds are used when we account memory pressure through
 * scanned/reclaimed ratio. The current values were chosen empirically. In
 * essence, they are percents: the higher the value, the more number
 * unsuccessful reclaims there were.
 */
static const unsigned int vmpressure_level_med = 60;
static const unsigned int vmpressure_level_critical = 95;

static unsigned long vmpressure_scale_max = 100;
module_param_named(vmpressure_scale_max, vmpressure_scale_max,
			ulong, S_IRUGO | S_IWUSR);

/* vmpressure values >= this will be scaled based on allocstalls */
static unsigned long allocstall_threshold = 70;
module_param_named(allocstall_threshold, allocstall_threshold,
			ulong, S_IRUGO | S_IWUSR);

static struct vmpressure global_vmpressure;
BLOCKING_NOTIFIER_HEAD(vmpressure_notifier);

int vmpressure_notifier_register(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&vmpressure_notifier, nb);
}

int vmpressure_notifier_unregister(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&vmpressure_notifier, nb);
}

void vmpressure_notify(unsigned long pressure)
{
	blocking_notifier_call_chain(&vmpressure_notifier, pressure, NULL);
}

/*
 * When there are too little pages left to scan, vmpressure() may miss the
 * critical pressure as number of pages will be less than "window size".
 * However, in that case the vmscan priority will raise fast as the
 * reclaimer will try to scan LRUs more deeply.
 *
 * The vmscan logic considers these special priorities:
 *
 * prio == DEF_PRIORITY (12): reclaimer starts with that value
 * prio <= DEF_PRIORITY - 2 : kswapd becomes somewhat overwhelmed
 * prio == 0                : close to OOM, kernel scans every page in an lru
 *
 * Any value in this range is acceptable for this tunable (i.e. from 12 to
 * 0). Current value for the vmpressure_level_critical_prio is chosen
 * empirically, but the number, in essence, means that we consider
 * critical level when scanning depth is ~10% of the lru size (vmscan
 * scans 'lru_size >> prio' pages, so it is actually 12.5%, or one
 * eights).
 */
static const unsigned int vmpressure_level_critical_prio = ilog2(100 / 10);

static struct vmpressure *work_to_vmpressure(struct work_struct *work)
{
	return container_of(work, struct vmpressure, work);
}

#ifdef CONFIG_MEMCG
static struct vmpressure *cg_to_vmpressure(struct cgroup *cg)
{
	return css_to_vmpressure(cgroup_subsys_state(cg, mem_cgroup_subsys_id));
}

static struct vmpressure *vmpressure_parent(struct vmpressure *vmpr)
{
	struct cgroup *cg = vmpressure_to_css(vmpr)->cgroup;
	struct mem_cgroup *memcg = mem_cgroup_from_cont(cg);

	memcg = parent_mem_cgroup(memcg);
	if (!memcg)
		return NULL;
	return memcg_to_vmpressure(memcg);
}
#else
static struct vmpressure *cg_to_vmpressure(struct cgroup *cg)
{
	return NULL;
}

static struct vmpressure *vmpressure_parent(struct vmpressure *vmpr)
{
	return NULL;
}
#endif

enum vmpressure_levels {
	VMPRESSURE_LOW = 0,
	VMPRESSURE_MEDIUM,
	VMPRESSURE_CRITICAL,
	VMPRESSURE_NUM_LEVELS,
};

static const char * const vmpressure_str_levels[] = {
	[VMPRESSURE_LOW] = "low",
	[VMPRESSURE_MEDIUM] = "medium",
	[VMPRESSURE_CRITICAL] = "critical",
};

static enum vmpressure_levels vmpressure_level(unsigned long pressure)
{
	if (pressure >= vmpressure_level_critical)
		return VMPRESSURE_CRITICAL;
	else if (pressure >= vmpressure_level_med)
		return VMPRESSURE_MEDIUM;
	return VMPRESSURE_LOW;
}

static unsigned long vmpressure_calc_pressure(unsigned long scanned,
						    unsigned long reclaimed)
{
	unsigned long scale = scanned + reclaimed;
	unsigned long pressure = 0;

	/*
	 * reclaimed can be greater than scanned in cases
	 * like THP, where the scanned is 1 and reclaimed
	 * could be 512
	 */
	if (reclaimed >= scanned)
		goto out;
	/*
	 * We calculate the ratio (in percents) of how many pages were
	 * scanned vs. reclaimed in a given time frame (window). Note that
	 * time is in VM reclaimer's "ticks", i.e. number of pages
	 * scanned. This makes it possible to set desired reaction time
	 * and serves as a ratelimit.
	 */
	pressure = scale - (reclaimed * scale / scanned);
	pressure = pressure * 100 / scale;

out:
	pr_debug("%s: %3lu  (s: %lu  r: %lu)\n", __func__, pressure,
		 scanned, reclaimed);

	return pressure;
}

static unsigned long vmpressure_account_stall(unsigned long pressure,
				unsigned long stall, unsigned long scanned)
{
	unsigned long scale;

	if (pressure < allocstall_threshold)
		return pressure;

	scale = ((vmpressure_scale_max - pressure) * stall) / scanned;

	return pressure + scale;
}

struct vmpressure_event {
	struct eventfd_ctx *efd;
	enum vmpressure_levels level;
	struct list_head node;
};

static bool vmpressure_event(struct vmpressure *vmpr,
			     unsigned long scanned, unsigned long reclaimed)
{
	struct vmpressure_event *ev;
	enum vmpressure_levels level;
	unsigned long pressure;
	bool signalled = false;

	pressure = vmpressure_calc_pressure(scanned, reclaimed);
	level = vmpressure_level(pressure);

	mutex_lock(&vmpr->events_lock);

	list_for_each_entry(ev, &vmpr->events, node) {
		if (level >= ev->level) {
			eventfd_signal(ev->efd, 1);
			signalled = true;
		}
	}

	mutex_unlock(&vmpr->events_lock);

	return signalled;
}

static void vmpressure_work_fn(struct work_struct *work)
{
	struct vmpressure *vmpr = work_to_vmpressure(work);
	unsigned long scanned;
	unsigned long reclaimed;

	/*
	 * Several contexts might be calling vmpressure(), so it is
	 * possible that the work was rescheduled again before the old
	 * work context cleared the counters. In that case we will run
	 * just after the old work returns, but then scanned might be zero
	 * here. No need for any locks here since we don't care if
	 * vmpr->reclaimed is in sync.
	 */
	if (!vmpr->scanned)
		return;

	mutex_lock(&vmpr->sr_lock);
	scanned = vmpr->scanned;
	reclaimed = vmpr->reclaimed;
	vmpr->scanned = 0;
	vmpr->reclaimed = 0;
	mutex_unlock(&vmpr->sr_lock);

	do {
		if (vmpressure_event(vmpr, scanned, reclaimed))
			break;
		/*
		 * If not handled, propagate the event upward into the
		 * hierarchy.
		 */
	} while ((vmpr = vmpressure_parent(vmpr)));
}

void vmpressure_memcg(gfp_t gfp, struct mem_cgroup *memcg,
		unsigned long scanned, unsigned long reclaimed)
{
	struct vmpressure *vmpr = memcg_to_vmpressure(memcg);

	BUG_ON(!vmpr);

	/*
	 * Here we only want to account pressure that userland is able to
	 * help us with. For example, suppose that DMA zone is under
	 * pressure; if we notify userland about that kind of pressure,
	 * then it will be mostly a waste as it will trigger unnecessary
	 * freeing of memory by userland (since userland is more likely to
	 * have HIGHMEM/MOVABLE pages instead of the DMA fallback). That
	 * is why we include only movable, highmem and FS/IO pages.
	 * Indirect reclaim (kswapd) sets sc->gfp_mask to GFP_KERNEL, so
	 * we account it too.
	 */
	if (!(gfp & (__GFP_HIGHMEM | __GFP_MOVABLE | __GFP_IO | __GFP_FS)))
		return;

	/*
	 * If we got here with no pages scanned, then that is an indicator
	 * that reclaimer was unable to find any shrinkable LRUs at the
	 * current scanning depth. But it does not mean that we should
	 * report the critical pressure, yet. If the scanning priority
	 * (scanning depth) goes too high (deep), we will be notified
	 * through vmpressure_prio(). But so far, keep calm.
	 */
	if (!scanned)
		return;

	mutex_lock(&vmpr->sr_lock);
	vmpr->scanned += scanned;
	vmpr->reclaimed += reclaimed;
	scanned = vmpr->scanned;
	mutex_unlock(&vmpr->sr_lock);

	if (scanned < vmpressure_win || work_pending(&vmpr->work))
		return;
	schedule_work(&vmpr->work);
}

void vmpressure_global(gfp_t gfp, unsigned long scanned,
		unsigned long reclaimed)
{
	struct vmpressure *vmpr = &global_vmpressure;
	unsigned long pressure;
	unsigned long stall;

	if (!(gfp & (__GFP_HIGHMEM | __GFP_MOVABLE | __GFP_IO | __GFP_FS)))
		return;

	if (!scanned)
		return;

	mutex_lock(&vmpr->sr_lock);
	vmpr->scanned += scanned;
	vmpr->reclaimed += reclaimed;

	if (!current_is_kswapd())
		vmpr->stall += scanned;

	stall = vmpr->stall;
	scanned = vmpr->scanned;
	reclaimed = vmpr->reclaimed;
	mutex_unlock(&vmpr->sr_lock);

	if (scanned < vmpressure_win)
		return;

	mutex_lock(&vmpr->sr_lock);
	vmpr->scanned = 0;
	vmpr->reclaimed = 0;
	vmpr->stall = 0;
	mutex_unlock(&vmpr->sr_lock);

	pressure = vmpressure_calc_pressure(scanned, reclaimed);
	pressure = vmpressure_account_stall(pressure, stall, scanned);
	vmpressure_notify(pressure);
}

/**
 * vmpressure() - Account memory pressure through scanned/reclaimed ratio
 * @gfp:	reclaimer's gfp mask
 * @memcg:	cgroup memory controller handle
 * @scanned:	number of pages scanned
 * @reclaimed:	number of pages reclaimed
 *
 * This function should be called from the vmscan reclaim path to account
 * "instantaneous" memory pressure (scanned/reclaimed ratio). The raw
 * pressure index is then further refined and averaged over time.
 *
 * This function does not return any value.
 */
void vmpressure(gfp_t gfp, struct mem_cgroup *memcg,
		unsigned long scanned, unsigned long reclaimed)
{
	if (!memcg)
		vmpressure_global(gfp, scanned, reclaimed);

	if (IS_ENABLED(CONFIG_MEMCG))
		vmpressure_memcg(gfp, memcg, scanned, reclaimed);
}

/**
 * vmpressure_prio() - Account memory pressure through reclaimer priority level
 * @gfp:	reclaimer's gfp mask
 * @memcg:	cgroup memory controller handle
 * @prio:	reclaimer's priority
 *
 * This function should be called from the reclaim path every time when
 * the vmscan's reclaiming priority (scanning depth) changes.
 *
 * This function does not return any value.
 */
void vmpressure_prio(gfp_t gfp, struct mem_cgroup *memcg, int prio)
{
	/*
	 * We only use prio for accounting critical level. For more info
	 * see comment for vmpressure_level_critical_prio variable above.
	 */
	if (prio > vmpressure_level_critical_prio)
		return;

	/*
	 * OK, the prio is below the threshold, updating vmpressure
	 * information before shrinker dives into long shrinking of long
	 * range vmscan. Passing scanned = vmpressure_win, reclaimed = 0
	 * to the vmpressure() basically means that we signal 'critical'
	 * level.
	 */
	vmpressure(gfp, memcg, vmpressure_win, 0);
}

/**
 * vmpressure_register_event() - Bind vmpressure notifications to an eventfd
 * @cg:		cgroup that is interested in vmpressure notifications
 * @cft:	cgroup control files handle
 * @eventfd:	eventfd context to link notifications with
 * @args:	event arguments (used to set up a pressure level threshold)
 *
 * This function associates eventfd context with the vmpressure
 * infrastructure, so that the notifications will be delivered to the
 * @eventfd. The @args parameter is a string that denotes pressure level
 * threshold (one of vmpressure_str_levels, i.e. "low", "medium", or
 * "critical").
 *
 * This function should not be used directly, just pass it to (struct
 * cftype).register_event, and then cgroup core will handle everything by
 * itself.
 */
int vmpressure_register_event(struct cgroup *cg, struct cftype *cft,
			      struct eventfd_ctx *eventfd, const char *args)
{
	struct vmpressure *vmpr = cg_to_vmpressure(cg);
	struct vmpressure_event *ev;
	int level;

	BUG_ON(!vmpr);

	for (level = 0; level < VMPRESSURE_NUM_LEVELS; level++) {
		if (!strcmp(vmpressure_str_levels[level], args))
			break;
	}

	if (level >= VMPRESSURE_NUM_LEVELS)
		return -EINVAL;

	ev = kzalloc(sizeof(*ev), GFP_KERNEL);
	if (!ev)
		return -ENOMEM;

	ev->efd = eventfd;
	ev->level = level;

	mutex_lock(&vmpr->events_lock);
	list_add(&ev->node, &vmpr->events);
	mutex_unlock(&vmpr->events_lock);

	return 0;
}

/**
 * vmpressure_unregister_event() - Unbind eventfd from vmpressure
 * @cg:		cgroup handle
 * @cft:	cgroup control files handle
 * @eventfd:	eventfd context that was used to link vmpressure with the @cg
 *
 * This function does internal manipulations to detach the @eventfd from
 * the vmpressure notifications, and then frees internal resources
 * associated with the @eventfd (but the @eventfd itself is not freed).
 *
 * This function should not be used directly, just pass it to (struct
 * cftype).unregister_event, and then cgroup core will handle everything
 * by itself.
 */
void vmpressure_unregister_event(struct cgroup *cg, struct cftype *cft,
				 struct eventfd_ctx *eventfd)
{
	struct vmpressure *vmpr = cg_to_vmpressure(cg);
	struct vmpressure_event *ev;

	if (!vmpr)
		BUG();

	mutex_lock(&vmpr->events_lock);
	list_for_each_entry(ev, &vmpr->events, node) {
		if (ev->efd != eventfd)
			continue;
		list_del(&ev->node);
		kfree(ev);
		break;
	}
	mutex_unlock(&vmpr->events_lock);
}

/**
 * vmpressure_init() - Initialize vmpressure control structure
 * @vmpr:	Structure to be initialized
 *
 * This function should be called on every allocated vmpressure structure
 * before any usage.
 */
void vmpressure_init(struct vmpressure *vmpr)
{
	mutex_init(&vmpr->sr_lock);
	mutex_init(&vmpr->events_lock);
	INIT_LIST_HEAD(&vmpr->events);
	INIT_WORK(&vmpr->work, vmpressure_work_fn);
}

int vmpressure_global_init(void)
{
	vmpressure_init(&global_vmpressure);
	return 0;
}
late_initcall(vmpressure_global_init);
