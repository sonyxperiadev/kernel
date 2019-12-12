/*
 *  lowmemorykiller_vmpressure
 *
 *  Author: Peter Enderborg <peter.enderborg@sonymobile.com>
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

/* todo: Handle vmpressure notifier directly.
 * It is there on this version since it us hooking
 * into qualcomm vmpressure extension.
 */

/* add fake print format with original module name */
#define pr_fmt(fmt) "lowmemorykiller: " fmt

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/slab.h>
#include <linux/vmpressure.h>

#ifdef LMK_TNG_ENABLE_TRACE
#include <trace/events/lmk.h>
#endif

#include "lowmemorykiller.h"
#include "lowmemorykiller_tng.h"
#include "lowmemorykiller_stats.h"
#include "lowmemorykiller_tasks.h"

static unsigned long oldvmp;

void balance_cache(unsigned long vmpressure)
{
	struct task_struct *selected = NULL;
	struct lmk_rb_watch *lrw;
	int do_kill;
	struct calculated_params cp;
	gfp_t mask = ___GFP_KSWAPD_RECLAIM |
	  ___GFP_DIRECT_RECLAIM | __GFP_FS | __GFP_IO;

	if (vmpressure < 50) {
		oldvmp = vmpressure;
		return;
	}

	if (vmpressure < oldvmp && vmpressure < 95) {
		oldvmp = vmpressure;
		return;
	}

	oldvmp = vmpressure;
	cp.selected_tasksize = 0;
	cp.dynamic_max_queue_len = 1;
	cp.kill_reason = LMK_VMPRESSURE;
	spin_lock_bh(&lmk_task_lock);

	lrw = __lmk_task_first();

	if (lrw) {
		if (lrw->tsk->mm) {
			cp.selected_tasksize = get_mm_rss(lrw->tsk->mm);
		} else {
			lowmem_print(3, "pid:%d no mem\n", lrw->tsk->pid);
			lmk_inc_stats(LMK_ERROR);
			goto unlock_out;
		}

		do_kill = kill_needed(lrw->key, mask, &cp);

		if (death_pending_len >= cp.dynamic_max_queue_len)
			__lmk_death_pending_morgue();
		if (death_pending_len >= cp.dynamic_max_queue_len) {
			lmk_inc_stats(LMK_BUSY);
			cp.selected_tasksize = SHRINK_STOP;
			lowmem_print(3, "Queue %d >= %d",
				     death_pending_len,
				     cp.dynamic_max_queue_len);
			goto unlock_out;
		}

		if (do_kill) {
			struct lmk_death_pending_entry *ldpt;

			selected = lrw->tsk;

			/* there is a chance that task is locked,
			 * and the case where it locked in oom_score_adj_write
			 * we might have deadlock. There is no macro for it
			 *  and this is the only place there is a try on
			 * the task_lock.
			 */
			if (!spin_trylock(&selected->alloc_lock)) {
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
						  0);
#endif
				goto unlock_out;
			}

			ldpt->tsk = selected;

			__lmk_death_pending_add(ldpt);
			if (!__lmk_task_remove(selected, lrw->key))
				WARN_ON(1);

			spin_unlock_bh(&lmk_task_lock);

			send_sig(SIGKILL, selected, 0);
			LMK_TAG_TASK_DIE(selected);
			print_obituary(selected, &cp, 0);
#ifdef LMK_TNG_ENABLE_TRACE
			trace_lmk_sigkill(selected->pid, selected->comm,
					  cp.selected_oom_score_adj,
					  cp.selected_tasksize,
					  0);
#endif

			task_unlock(selected);

			lmk_inc_stats(LMK_BALANCE_KILL);
			goto out;
		} else {
			lowmem_print(3, "No kill");
			lmk_inc_stats(LMK_BALANCE_WASTE);
		}
	} else {
		lowmem_print(2, "Nothing to kill");
		lmk_inc_stats(LMK_NO_KILL);
	}
unlock_out:
	spin_unlock_bh(&lmk_task_lock);
out:
	if (cp.selected_tasksize == 0)
		lowmem_print(2, "list empty nothing to free\n");
}
