/*
 *  lowmemorykiller_tng
 *
 *  Author: Peter Enderborg <peter.enderborg@sonymobile.com>
 *  co Author: Snild Dolkow
 *  co Author: Björn Davidsson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
/*
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
/* This file contain the logic for new lowmemorykiller (TNG). Parts
 * of the calculations is taken from the original lowmemorykiller
 */

/* add fake print format with original module name */
#define pr_fmt(fmt) "lowmemorykiller: " fmt

#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/mm.h>

#include <linux/oom_score_notifier.h>
#include "lowmemorykiller_tasks.h"
#include "lowmemorykiller_oom.h"
#include "lowmemorykiller_tng.h"

#ifdef LMK_TNG_ENABLE_TRACE
#include <trace/events/lmk.h>
#endif
#ifdef CONFIG_ZCACHE
#include <linux/zcache.h>
#else
/*zcache.h has incorrect definition here.*/
static inline u64 zcache_pages(void) { return 0; }
#endif
#define LMK_ZOMBIE_SIZE (PAGE_SIZE)
static int seek_count;
static unsigned long lowmem_count_tng(struct shrinker *s,
				      struct shrink_control *sc);
static unsigned long lowmem_scan_tng(struct shrinker *s,
				     struct shrink_control *sc);

void __init lowmem_init_tng(struct shrinker *shrinker)
{
	lmk_dp_cache = KMEM_CACHE(lmk_death_pending_entry, 0);
	lmk_task_cache = KMEM_CACHE(lmk_rb_watch, 0);
	oom_score_notifier_register(&lmk_oom_score_nb);
	lowmemorykiller_register_oom_notifier();
	shrinker->count_objects = lowmem_count_tng;
	shrinker->scan_objects = lowmem_scan_tng;
}

ssize_t get_task_rss(struct task_struct *tsk)
{
	unsigned long rss = 0;
	struct mm_struct *mm;

	mm = READ_ONCE(tsk->mm);
	if (mm)
		rss = get_mm_rss(mm) << PAGE_SHIFT;
	if (rss < LMK_ZOMBIE_SIZE)
		rss = LMK_ZOMBIE_SIZE;
	return rss;
}

static inline long zone_threshold_check(struct zone *zone, int zone_type,
					int threshold)
{
	int order;
	unsigned long flags;
	unsigned long tot = 0;

	spin_lock_irqsave(&zone->lock, flags);
	for (order = 0; order < MAX_ORDER; ++order) {
		struct free_area *area;
		struct list_head *curr;

		area = &zone->free_area[order];
		list_for_each(curr, &area->free_list[zone_type]) {
			tot += 1 << order;
			if (tot >= threshold)
				goto out;
		}
	}
out:
	spin_unlock_irqrestore(&zone->lock, flags);
	return tot;
}

/* can_swap check if we have memory resources to be able to swap.
 * When we have a pressure to get more atomic or movable pages
 * it is sent through the shrinker as gfp. We can not trace it back
 * to who the consumer is.
 */
static int can_swap(gfp_t mask, int atomic_threshold, int movable_threshold)
{
	struct zonelist *zonelist;
	enum zone_type high_zoneidx;
	struct zone *zone;
	struct zoneref *zoneref;
	int zone_idx;
	struct zone *preferred_zone;
	struct zoneref *zref;
	int sum_atomic = 0;
	int sum_movable = 0;

	/* if there is no need for movable or atomic we are safe */

	if (atomic_threshold == 0 && movable_threshold == 0)
		return 1;

	zonelist = node_zonelist(0, 0);

	high_zoneidx = gfp_zone(mask);
	zref = first_zones_zonelist(zonelist, high_zoneidx, NULL);
	preferred_zone = zref->zone;

	/* Watermark for the zone can handle 64 pages of order 0
	 * we assume we are safe.
	 */
	if (zone_watermark_ok(preferred_zone, 0, 64, high_zoneidx, mask))
		return 1;

	/* this is scan is heuristic based on qualcomm hardware. */
	/* we can swap if there is movable pages */
	for_each_zone_zonelist(zone, zoneref, zonelist, MAX_NR_ZONES) {
		zone_idx = zonelist_zone_idx(zoneref);
		if (zone_idx == ZONE_NORMAL) {
			sum_atomic +=
				zone_threshold_check(zone,
						     MIGRATE_HIGHATOMIC,
						     atomic_threshold);

			if (sum_movable >= movable_threshold &&
			    sum_atomic >= atomic_threshold)
				return 1;

			sum_movable +=
				zone_threshold_check(zone,
						     MIGRATE_MOVABLE,
						     movable_threshold);

			if (sum_movable >= movable_threshold &&
			    sum_atomic >= atomic_threshold)
				return 1;
		}
	}
	return 0;
}

static void calc_params(struct calculated_params *cp, gfp_t mask)
{
	int i;
	int array_size;
	long free_swap = get_nr_swap_pages();
	long irp = global_node_page_state(NR_INDIRECTLY_RECLAIMABLE_BYTES)
	  >> PAGE_SHIFT;
	long usable_free_swap = 0;
	long tot_usable = 0;
	int start;
	int cs = 1; /* we can swap unless the checks says no */
	int movc = 0;
	int atomicc = 0;

	if (free_swap > 64) {
		if (mask & __GFP_MOVABLE)
			movc = (1 << (PAGE_ALLOC_COSTLY_ORDER + 2));

		if (mask & __GFP_ATOMIC)
			atomicc = (1 << PAGE_ALLOC_COSTLY_ORDER);

		cs = can_swap(mask, atomicc, movc);
		if (cs) {
			if (free_swap > 0x800)
				usable_free_swap = free_swap;
			else
				usable_free_swap = free_swap / 2;

		} else {
			cp->kill_reason |= LMK_CANT_SWAP;
		}
	}

	tot_usable = usable_free_swap + irp;
	/* Free CMA is part of NR_FREE_PAGES, but sometimes not usable. */
	cp->margin = global_zone_page_state(NR_FREE_PAGES) - totalreserve_pages
	  - global_zone_page_state(NR_FREE_CMA_PAGES);
	cp->other_free = cp->margin + tot_usable;

	if (global_node_page_state(NR_SHMEM) + total_swapcache_pages() +
	    global_node_page_state(NR_UNEVICTABLE) <
	    global_node_page_state(NR_FILE_PAGES))
		cp->other_file = global_node_page_state(NR_FILE_PAGES) -
		  global_node_page_state(NR_SHMEM) -
		  global_node_page_state(NR_UNEVICTABLE) -
		  total_swapcache_pages();
	else
		cp->other_file = 0;

	cp->min_score_adj = SHRT_MAX;
	array_size = lowmem_min_param_size();
	start = 0;

	/* be extra careful with vmpressure to kill perceptible stuff */
	if (tot_usable > 128 && cp->kill_reason == LMK_VMPRESSURE)
		start = 3;

	for (i = start; i < array_size; i++) {
		cp->minfree = lowmem_minfree[i];
		if (cp->other_free < cp->minfree &&
		    cp->other_file < cp->minfree) {
			cp->min_score_adj = lowmem_adj[i];
			break;
		}
	}
	cp->dynamic_max_queue_len = (array_size - i) / 2 + 1;

	/* If there is a lot of reclaimable we dont kill more
	 *  than one at the time.
	 */
	if (tot_usable > 128)
		cp->dynamic_max_queue_len = 1;

	if (cp->margin <= 0) {
		/* If have used more 5/6 our total reserve
		 * we need to take a action and kill something even if
		 * we have reclaimable memory and lot of free swap
		 * but can not swap.
		 */
		if (cp->kill_reason & LMK_CANT_SWAP) {
			if (-6 * (cp->margin) > 5 * totalreserve_pages) {
				cp->kill_reason |= LMK_LOW_RESERVE;
				cp->dynamic_max_queue_len = 1;
				cp->min_score_adj = 0;
			}
		}
	}
}

int kill_needed(int level, gfp_t mask,
		struct calculated_params *cp)
{
	calc_params(cp, mask);
	cp->selected_oom_score_adj = level;

	if (level >= cp->min_score_adj)
		return 1;
	return 0;
}

void print_obituary(struct task_struct *doomed,
		    struct calculated_params *cp,
		    gfp_t gfp_mask)
{
	long cache_size = cp->other_file * (long)(PAGE_SIZE / 1024);
	long cache_limit = cp->minfree * (long)(PAGE_SIZE / 1024);
	long free = cp->other_free * (long)(PAGE_SIZE / 1024);

	lowmem_print(1, "Killing '%s' (%d), adj %hd,\n"
		     "   to free %ldkB on behalf of '%s' (%d) because\n"
		     "   cache %ldkB is below limit %ldkB for oom_score_adj %hd\n"
		     "   Free memory is %ldkB above reserved.\n"
		     "   Free CMA is %ldkB\n"
		     "   Total reserve is %ldkB\n"
		     "   Total free pages is %ldkB\n"
		     "   Total file cache is %ldkB\n"
		     "   Slab Reclaimable is %ldkB\n"
		     "   Slab UnReclaimable is %ldkB\n"
		     "   Total Slab is %ldkB\n"
		     "   GFP mask is 0x%x\n"
		     "   Indirect Reclaimable is %zdkB\n"
		     "   Free Swap %ldkB\n"
		     "   queue len is %d of max %d reason:0x%x margin:%d\n",
		     doomed->comm, doomed->pid,
		     cp->selected_oom_score_adj,
		     cp->selected_tasksize / 1024,
		     current->comm, current->pid,
		     cache_size, cache_limit,
		     cp->min_score_adj,
		     free,
		     global_zone_page_state(NR_FREE_CMA_PAGES) *
		     (long)(PAGE_SIZE / 1024),
		     totalreserve_pages * (long)(PAGE_SIZE / 1024),
		     global_zone_page_state(NR_FREE_PAGES) *
		     (long)(PAGE_SIZE / 1024),
		     global_node_page_state(NR_FILE_PAGES) *
		     (long)(PAGE_SIZE / 1024),
		     global_node_page_state(NR_SLAB_RECLAIMABLE) *
		     (long)(PAGE_SIZE / 1024),
		     global_node_page_state(NR_SLAB_UNRECLAIMABLE) *
		     (long)(PAGE_SIZE / 1024),
		     global_node_page_state(NR_SLAB_RECLAIMABLE) *
		     (long)(PAGE_SIZE / 1024) +
		     global_node_page_state(NR_SLAB_UNRECLAIMABLE) *
		     (long)(PAGE_SIZE / 1024),
		     gfp_mask,
		     global_node_page_state(NR_INDIRECTLY_RECLAIMABLE_BYTES)
		     / 1024,
		     get_nr_swap_pages() * (long)(PAGE_SIZE / 1024),
		     death_pending_len,
		     cp->dynamic_max_queue_len,
		     cp->kill_reason, cp->margin);
}

static unsigned long lowmem_count_tng(struct shrinker *s,
				      struct shrink_control *sc)
{
	struct lmk_rb_watch *lrw;
	struct calculated_params cp;
	short score;

	if (!(sc->gfp_mask & __GFP_MOVABLE)) {
		if (current_is_kswapd()) {
			if (seek_count++ < (s->seeks * 4))
				return 0;
			seek_count = 0;
		}
	}

	lmk_inc_stats(LMK_COUNT);
	cp.selected_tasksize = 0;
	cp.kill_reason = LMK_SHRINKER_COUNT;
	spin_lock(&lmk_task_lock);
	lrw = __lmk_task_first();
	if (lrw) {
		ssize_t rss = get_task_rss(lrw->tsk);

		score = lrw->tsk->signal->oom_score_adj;
		spin_unlock(&lmk_task_lock);
		if (kill_needed(score, sc->gfp_mask, &cp)) {
			if (death_pending_len < cp.dynamic_max_queue_len)
				cp.selected_tasksize = rss;
			else if (lmk_death_pending_morgue())
				if (death_pending_len <
				    cp.dynamic_max_queue_len)
					cp.selected_tasksize = rss;
		}
	} else {
		spin_unlock(&lmk_task_lock);
		lowmem_print(2, "Empty task list in count");
	}
	if (cp.selected_tasksize == 0)
		lmk_inc_stats(LMK_ZERO_COUNT);

	return cp.selected_tasksize;
}

static unsigned long lowmem_scan_tng(struct shrinker *s,
				     struct shrink_control *sc)
{
	struct task_struct *selected = NULL;
	unsigned long nr_to_scan = sc->nr_to_scan;
	struct lmk_rb_watch *lrw;
	int do_kill;
	struct calculated_params cp;

	lmk_inc_stats(LMK_SCAN);

	cp.selected_tasksize = 0;
	cp.kill_reason = LMK_SHRINKER_SCAN;
	spin_lock(&lmk_task_lock);

	lrw = __lmk_task_first();
	if (lrw) {
		cp.selected_tasksize = get_task_rss(lrw->tsk);
		do_kill = kill_needed(lrw->key, sc->gfp_mask, &cp);
		if (death_pending_len >= cp.dynamic_max_queue_len) {
			lmk_inc_stats(LMK_BUSY);
			cp.selected_tasksize = SHRINK_STOP;
			goto unlock_out;
		}

		if (do_kill) {
			struct lmk_death_pending_entry *ldpt;

			selected = lrw->tsk;

			/* there is a chance that task is locked,
			 * and the case where it locked in oom_score_adj_write
			 * we might have deadlock.
			 */
			if (!task_trylock_lmk(selected)) {
				lmk_inc_stats(LMK_ERROR);
				lowmem_print(2, "Failed to lock task.\n");
				lmk_inc_stats(LMK_BUSY);
				cp.selected_tasksize = SHRINK_STOP;
				goto unlock_out;
			}

			/* move to kill pending set */
			ldpt = kmem_cache_alloc(lmk_dp_cache, GFP_ATOMIC);
			if (!ldpt) {
				WARN_ON(1);
				lmk_inc_stats(LMK_MEM_ERROR);
				cp.selected_tasksize = SHRINK_STOP;
#ifdef LMK_TNG_ENABLE_TRACE
				trace_lmk_sigkill(selected->pid, selected->comm,
						  LMK_TRACE_MEMERROR,
						  cp.selected_tasksize,
						  sc->gfp_mask);
#endif

				goto unlock_out;
			}
			ldpt->tsk = selected;

			__lmk_death_pending_add(ldpt);
			if (!__lmk_task_remove(selected, lrw->key))
				WARN_ON(1);

			spin_unlock(&lmk_task_lock);

			send_sig(SIGKILL, selected, 0);
			LMK_TAG_TASK_DIE(selected);
#ifdef LMK_TNG_ENABLE_TRACE
			trace_lmk_sigkill(selected->pid, selected->comm,
					  cp.selected_oom_score_adj,
					  cp.selected_tasksize,
					  sc->gfp_mask);
#endif
			print_obituary(selected, &cp, sc->gfp_mask);

			task_unlock(selected);

			lmk_inc_stats(LMK_KILL);
			goto out;
		} else {
			lmk_inc_stats(LMK_WASTE);
		}
	} else {
		lmk_inc_stats(LMK_NO_KILL);
	}

unlock_out:
	spin_unlock(&lmk_task_lock);
out:
	if (cp.selected_tasksize == 0)
		lowmem_print(2, "list empty nothing to free\n");
	lowmem_print(4, "lowmem_shrink %lu, %x, return %ld\n",
		     nr_to_scan, sc->gfp_mask, cp.selected_tasksize);

	return cp.selected_tasksize;
}

void tune_lmk_param_mask(int *other_free, int *other_file, gfp_t mask)
{
	struct shrink_control fake_shrink_control;

	fake_shrink_control.gfp_mask = mask;
	tune_lmk_param(other_free, other_file, &fake_shrink_control);
}
