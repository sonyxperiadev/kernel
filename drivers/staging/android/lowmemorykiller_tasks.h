/*
 *  lowmemorykiller_tasks interface
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

#ifndef __LOWMEMORYKILLER_TASKS_H
#define __LOWMEMORYKILLER_TASKS_H

struct lmk_death_pending_entry {
	struct list_head lmk_dp_list;
	struct task_struct *tsk;
};

struct lmk_rb_watch {
	struct rb_node rb_node;
	struct task_struct *tsk;
	int key;
};

extern int death_pending_len;
extern struct kmem_cache *lmk_dp_cache;
extern struct kmem_cache *lmk_task_cache;
extern spinlock_t lmk_task_lock;
extern struct notifier_block lmk_oom_score_nb;

int __lmk_task_remove(struct task_struct *tsk, int score);
int __lmk_death_pending_add(struct lmk_death_pending_entry *lwp);
bool lmk_death_pending_morgue(void);
bool __lmk_death_pending_morgue(void);
struct lmk_rb_watch *__lmk_task_first(void);

#endif
