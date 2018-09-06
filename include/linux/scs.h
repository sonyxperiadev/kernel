/*
 * Shadow Call Stack support.
 *
 * Copyright (C) 2018 Google LLC
 */

#ifndef _LINUX_SCS_H
#define _LINUX_SCS_H

#ifdef CONFIG_SHADOW_CALL_STACK

#include <linux/gfp.h>
#include <linux/sched.h>
#include <asm/page.h>

#ifdef CONFIG_SHADOW_CALL_STACK_VMAP
# define SCS_SIZE		PAGE_SIZE
#else
# define SCS_SIZE		1024
#endif

#define SCS_GFP			(GFP_KERNEL | __GFP_ZERO)

extern unsigned long init_shadow_call_stack[];

static inline void *task_scs(struct task_struct *tsk)
{
	return task_thread_info(tsk)->shadow_call_stack;
}

static inline void task_set_scs(struct task_struct *tsk, void *s)
{
	task_thread_info(tsk)->shadow_call_stack = s;
}

extern void scs_init(void);
extern void scs_set_init_magic(struct task_struct *tsk);
extern void scs_task_init(struct task_struct *tsk);
extern void scs_task_reset(struct task_struct *tsk);
extern int scs_prepare(struct task_struct *tsk, int node);
extern bool scs_corrupted(struct task_struct *tsk);
extern void scs_release(struct task_struct *tsk);

#else /* CONFIG_SHADOW_CALL_STACK */

static inline void *task_scs(struct task_struct *tsk)
{
	return 0;
}

static inline void task_set_scs(struct task_struct *tsk, void *s)
{
}

static inline void scs_init(void)
{
}

static inline void scs_set_init_magic(struct task_struct *tsk)
{
}

static inline void scs_task_init(struct task_struct *tsk)
{
}

static inline void scs_task_reset(struct task_struct *tsk)
{
}

static inline int scs_prepare(struct task_struct *tsk, int node)
{
	return 0;
}

static inline bool scs_corrupted(struct task_struct *tsk)
{
	return false;
}

static inline void scs_release(struct task_struct *tsk)
{
}

#endif /* CONFIG_SHADOW_CALL_STACK */

#endif /* _LINUX_SCS_H */
