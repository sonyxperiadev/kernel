/*
 * Copyright (C) 2018 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */
#ifndef __LOWMEMORYKILLER_H
#define __LOWMEMORYKILLER_H

#include <linux/version.h>

/* The lowest score LMK is using */
#define LMK_SCORE_THRESHOLD 0
#define LMK_TRACE_OOMKILL (-1)
#define LMK_TRACE_MEMERROR (-2)

extern u32 lowmem_debug_level;

#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			pr_info(x);			\
	} while (0)

int __init lowmemorykiller_register_oom_notifier(void);
struct calculated_params {
	long selected_tasksize;
	long minfree;
	int other_file;
	int other_free;
	int margin;
	int dynamic_max_queue_len;
	short selected_oom_score_adj;
	short min_score_adj;
	int kill_reason;
};

int kill_needed(int level, gfp_t mask,
		struct calculated_params *cp);
void print_obituary(struct task_struct *doomed,
		    struct calculated_params *cp,
		    gfp_t gfp_mask);

ssize_t get_task_rss(struct task_struct *tsk);

/* kernel does not have a task_trylock and
 * to make it more obvious what the code do
 * we create a help function for it.
 * see sched.h for task_lock and task_unlock
 */
static inline int task_trylock_lmk(struct task_struct *p)
{
	return spin_trylock(&p->alloc_lock);
}

/* maybe not exact version, we need something betwwen 3.18 and 4.4.
 * using LINUX_VERSION_CODE like this will give a warning.
 * it it not OK for mainline but for multiple kernel version patches
 * I think it is OK.
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 0, 0)
#define LMK_TAG_TASK_DIE(x) set_tsk_thread_flag(x, TIF_MEMDIE)
#elseif LINUX_VERSION_CODE <= KERNEL_VERSION(4, 5, 0)
#define LMK_TAG_TASK_DIE(x) mark_oom_victim(x)
#else
#define LMK_TAG_TASK_DIE(x)			\
	do {					\
		task_set_lmk_waiting(x);	\
		if (x->mm) {			\
			if (!test_bit(MMF_OOM_SKIP, &x->mm->flags) && \
			    oom_reaper) { \
				mark_lmk_victim(x); \
				wake_oom_reaper(x);\
			} \
		} \
	} while (0)

#endif

#endif
