#ifndef _BP_LINUX_INTERRUPT_H
#define _BP_LINUX_INTERRUPT_H
#include <linux/version.h>
#include_next <linux/interrupt.h>
#include <linux/ktime.h>

#if LINUX_VERSION_IS_LESS(4,10,0)
static inline void backport_hrtimer_start(struct hrtimer *timer, s64 time,
					  const enum hrtimer_mode mode)
{
	ktime_t _time = { .tv64 = time };
	hrtimer_start(timer, _time, mode);
}
#define hrtimer_start LINUX_BACKPORT(hrtimer_start)
#endif

#endif /* _BP_LINUX_INTERRUPT_H */
