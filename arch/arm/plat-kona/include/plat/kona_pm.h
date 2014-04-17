/****************************************************************************
*
* Copyright 2010 --2011 Broadcom Corporation.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/

#ifndef __KONA_PM_H__
#define __KONA_PM_H__

/* Additional control parameters */
#define CTRL_PARAMS_FLAG_XTAL_ON	(1 << 0)
#define CTRL_PARAMS_ENTER_SUSPEND	(1 << 1)
#define CTRL_PARAMS_OFFLINE_CORE	(1 << 2)
#define CTRL_PARAMS_CSTATE_DISABLED	(1 << 3)
#define CTRL_PARAMS_CLUSTER_ACTIVE	(1 << 4)

/* Additional dormant traces for idle and suspend */
#define DORMANT_IDLE_PATH_ENTRY		0xA0A0
#define DORMANT_IDLE_PATH_EXIT		0xB0B0
#define DORMANT_SUSPEND_PATH_ENTRY	0xE0E0
#define DORMANT_SUSPEND_PATH_EXIT	0xF0F0

#define CSTATE_ALL 0xffffffff

enum {
	CSTATE_ENTER,
	CSTATE_EXIT,
};

struct kona_idle_state {
	char *name;
	char *desc;
	u32 flags;
	const u32 params;
	u32 num_cpu_in_state;
	u32 state;
	int	power_usage;
	u32 latency;		/* in uS */
	u32 target_residency;	/* in uS */
	int (*enter) (struct kona_idle_state *state, u32 ctrl_params);
	int disable_cnt;
};
struct pm_init_param {
	struct kona_idle_state *states;
	u32 num_states;
	u32 suspend_state;
};

int __init kona_pm_init(struct pm_init_param *ip);
extern void instrument_idle_entry(void);
extern void instrument_idle_exit(void);
int kona_pm_disable_idle_state(int state, bool disable);
int kona_pm_set_suspend_state(int state_inx);
int kona_pm_cpu_lowpower(void);
int kona_pm_get_num_cstates(void);
char *kona_pm_get_cstate_name(int state_inx);
int cstate_notifier_register(struct notifier_block *nb);
int cstate_notifier_unregister(struct notifier_block *nb);
int pm_is_forced_sleep(void);

#endif /*__KONA_PM_H__*/
