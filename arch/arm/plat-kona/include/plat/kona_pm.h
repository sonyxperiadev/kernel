#ifndef __KONA_PM_H__
#define __KONA_PM_H__

#ifdef CONFIG_CPU_IDLE

struct kona_idle_state
{
	char*	name;
	char*	desc;
	u32		flags;
	u32 	state;
	u32 	latency; /* in uS */
	u32		target_residency; /* in uS */
	int (*enter)(struct kona_idle_state* state);
};
#endif /*CONFIG_CPU_IDLE*/

int __init kona_pm_init(void);

#endif /*__KONA_PM_H__*/
