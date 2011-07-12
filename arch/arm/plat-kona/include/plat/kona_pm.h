#ifndef __KONA_PM_H__
#define __KONA_PM_H__

#ifdef CONFIG_CPU_IDLE

struct kona_idle_state_info
{
	char	name[CPUIDLE_NAME_LEN];
	bool	valid;
	u32		flags;
	u32 	latency; /* in US */
	u32		target_residency; /* in US */
};
#endif /*CONFIG_CPU_IDLE*/

#endif /*__KONA_PM_H__*/
