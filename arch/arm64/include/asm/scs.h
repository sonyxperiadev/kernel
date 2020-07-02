#ifndef _ASM_SCS_H
#define _ASM_SCS_H

#ifndef __ASSEMBLY__

#include <linux/scs.h>

#ifdef CONFIG_SHADOW_CALL_STACK

extern void scs_init_irq(void);

static inline void scs_save(struct task_struct *tsk)
{
	void *s;

	asm volatile("mov %0, x18" : "=r" (s));
	task_set_scs(tsk, s);
}

static inline void scs_load(struct task_struct *tsk)
{
	asm volatile("mov x18, %0" : : "r" (task_scs(tsk)));
	task_set_scs(tsk, NULL);
}

static inline void scs_thread_switch(struct task_struct *prev,
				     struct task_struct *next)
{
	scs_save(prev);
	scs_load(next);

	if (unlikely(scs_corrupted(prev)))
		panic("corrupted shadow stack detected inside scheduler\n");
}

#else /* CONFIG_SHADOW_CALL_STACK */

static inline void scs_init_irq(void)
{
}

static inline void scs_save(struct task_struct *tsk)
{
}

static inline void scs_load(struct task_struct *tsk)
{
}

static inline void scs_thread_switch(struct task_struct *prev,
				     struct task_struct *next)
{
}

#endif /* CONFIG_SHADOW_CALL_STACK */

#endif /* __ASSEMBLY __ */

#endif /* _ASM_SCS_H */
