/*
 *  lowmemorykiller_oom
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

/* add fake print format with original module name */
#define pr_fmt(fmt) "lowmemorykiller: " fmt

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/oom.h>

#ifdef LMK_TNG_ENABLE_TRACE
#include <trace/events/lmk.h>
#endif

#include "lowmemorykiller.h"
#include "lowmemorykiller_tng.h"
#include "lowmemorykiller_stats.h"
#include "lowmemorykiller_tasks.h"

/**
 * lowmemorykiller_oom_notify - OOM notifier
 * @self:	notifier block struct
 * @notused:	not used
 * @parm:	returned - number of pages freed
 *
 * Return value:
 *	NOTIFY_OK
 **/

static int lowmemorykiller_oom_notify(struct notifier_block *self,
				      unsigned long notused, void *param)
{
	struct lmk_rb_watch *lrw;
	unsigned long *nfreed = param;

	lowmem_print(2, "oom notify event\n");
	*nfreed = 0;
	lmk_inc_stats(LMK_OOM_COUNT);
	spin_lock_bh(&lmk_task_lock);
	lrw = __lmk_task_first();
	if (lrw) {
		struct task_struct *selected = lrw->tsk;
		struct lmk_death_pending_entry *ldpt;

		if (!task_trylock_lmk(selected)) {
			lmk_inc_stats(LMK_ERROR);
			lowmem_print(1, "Failed to lock task.\n");
			lmk_inc_stats(LMK_BUSY);
			goto unlock_out;
		}

		get_task_struct(selected);
		/* move to kill pending set */
		ldpt = kmem_cache_alloc(lmk_dp_cache, GFP_ATOMIC);
		/* if we fail to alloc we ignore the death pending list */
		if (ldpt) {
			ldpt->tsk = selected;
			__lmk_death_pending_add(ldpt);
		} else {
			WARN_ON(1);
			lmk_inc_stats(LMK_MEM_ERROR);
#ifdef LMK_TNG_ENABLE_TRACE
			trace_lmk_sigkill(selected->pid, selected->comm,
					  LMK_TRACE_MEMERROR, *nfreed, 0);
#endif
		}
		if (!__lmk_task_remove(selected, lrw->key))
			WARN_ON(1);

		spin_unlock_bh(&lmk_task_lock);
		*nfreed = get_task_rss(selected);
		send_sig(SIGKILL, selected, 0);

		LMK_TAG_TASK_DIE(selected);
#ifdef LMK_TNG_ENABLE_TRACE
		trace_lmk_sigkill(selected->pid, selected->comm,
				  LMK_TRACE_OOMKILL, *nfreed,
				  0);
#endif

		task_unlock(selected);
		put_task_struct(selected);
		lmk_inc_stats(LMK_OOM_KILL_COUNT);
		goto out;
	}
unlock_out:
	spin_unlock_bh(&lmk_task_lock);
out:
	return NOTIFY_OK;
}

static struct notifier_block lowmemorykiller_oom_nb = {
	.notifier_call = lowmemorykiller_oom_notify
};

int __init lowmemorykiller_register_oom_notifier(void)
{
	register_oom_notifier(&lowmemorykiller_oom_nb);
	return 0;
}
