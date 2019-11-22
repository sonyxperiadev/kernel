/*
 *  oom_score_notifier interface
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

#ifndef _LINUX_OOM_SCORE_NOTIFIER_H
#define _LINUX_OOM_SCORE_NOTIFIER_H

#ifdef CONFIG_OOM_SCORE_NOTIFIER

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>

enum osn_msg_type {
	OSN_NEW,
	OSN_FREE,
	OSN_UPDATE
};

extern struct atomic_notifier_head oom_score_notifier;
extern int oom_score_notifier_register(struct notifier_block *n);
extern int oom_score_notifier_unregister(struct notifier_block *n);
extern int oom_score_notify_free(struct task_struct *tsk);
extern int oom_score_notify_new(struct task_struct *tsk);
extern int oom_score_notify_update(struct task_struct *tsk, int old_score);

struct oom_score_notifier_struct {
	struct task_struct *tsk;
	int old_score;
};

#else
static inline int oom_score_notify_free(struct task_struct *tsk) { return 0; };
static inline int oom_score_notify_new(struct task_struct *tsk) { return 0; };
static inline int oom_score_notify_update(struct task_struct *tsk,
					  int old_score)
{
	return 0;
};

#endif /* CONFIG_OOM_SCORE_NOTIFIER */

#endif /* _LINUX_OOM_SCORE_NOTIFIER_H */
