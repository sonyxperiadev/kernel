/*
 * Shadow Call Stack support.
 *
 * Copyright (C) 2018 Google LLC
 */

#include <linux/cpuhotplug.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/scs.h>

#include <asm/scs.h>

#define SCS_END_MAGIC	0xaf0194819b1635f6UL

static inline void *__scs_base(struct task_struct *tsk)
{
	return (void *)((uintptr_t)task_scs(tsk) & ~(SCS_SIZE - 1));
}

static inline void *scs_alloc(int node)
{
	return kmalloc(SCS_SIZE, SCS_GFP);
}

static inline void scs_free(void *s)
{
	kfree(s);
}

static inline unsigned long *scs_magic(struct task_struct *tsk)
{
	return (unsigned long *)(__scs_base(tsk) + SCS_SIZE - sizeof(long));
}

static inline void scs_set_magic(struct task_struct *tsk)
{
	*scs_magic(tsk) = SCS_END_MAGIC;
}

void scs_task_init(struct task_struct *tsk)
{
	task_set_scs(tsk, NULL);
}

void scs_set_init_magic(struct task_struct *tsk)
{
	scs_save(tsk);
	scs_set_magic(tsk);
	scs_load(tsk);
}

int scs_prepare(struct task_struct *tsk, int node)
{
	void *s;

	s = scs_alloc(node);
	if (!s)
		return -ENOMEM;

	task_set_scs(tsk, s);
	scs_set_magic(tsk);

	return 0;
}

void scs_release(struct task_struct *tsk)
{
	void *s;

	s = __scs_base(tsk);
	if (!s)
		return;

	BUG_ON(*scs_magic(tsk) != SCS_END_MAGIC);

	scs_task_init(tsk);
	scs_free(s);
}
