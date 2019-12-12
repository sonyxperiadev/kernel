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


#include <linux/notifier.h>
#include <linux/oom_score_notifier.h>

#ifdef CONFIG_OOM_SCORE_NOTIFIER
ATOMIC_NOTIFIER_HEAD(oom_score_notifier);

int oom_score_notifier_register(struct notifier_block *n)
{
	return atomic_notifier_chain_register(&oom_score_notifier, n);
}
EXPORT_SYMBOL_GPL(oom_score_notifier_register);

int oom_score_notifier_unregister(struct notifier_block *n)
{
	return atomic_notifier_chain_unregister(&oom_score_notifier, n);
}
EXPORT_SYMBOL_GPL(oom_score_notifier_unregister);

int oom_score_notify_free(struct task_struct *tsk)
{
	struct oom_score_notifier_struct osns;

	osns.tsk = tsk;
	return notifier_to_errno(atomic_notifier_call_chain(
		&oom_score_notifier, OSN_FREE, &osns));
}
EXPORT_SYMBOL_GPL(oom_score_notify_free);

int oom_score_notify_new(struct task_struct *tsk)
{
	struct oom_score_notifier_struct osns;

	osns.tsk = tsk;
	return notifier_to_errno(atomic_notifier_call_chain(
		&oom_score_notifier, OSN_NEW, &osns));
}
EXPORT_SYMBOL_GPL(oom_score_notify_new);

int oom_score_notify_update(struct task_struct *tsk, int old_score)
{
	struct oom_score_notifier_struct osns;

	osns.tsk = tsk;
	osns.old_score = old_score;
	return notifier_to_errno(atomic_notifier_call_chain(&oom_score_notifier,
							    OSN_UPDATE, &osns));
}
EXPORT_SYMBOL_GPL(oom_score_notify_update);
#endif
