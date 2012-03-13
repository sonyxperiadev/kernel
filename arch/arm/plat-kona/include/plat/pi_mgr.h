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

#ifndef __POWER_ISLAND_MGR_H__
#define __POWER_ISLAND_MGR_H__

#include <linux/plist.h>
#include <linux/version.h>
#include <linux/notifier.h>
#include <linux/pm_qos_params.h>
#include <mach/pi_mgr.h>

#define PI_MGR_QOS_DEFAULT_VALUE 		0xFFFFFFFF
#define PI_MGR_DFS_MIN_VALUE 			0xFFFFFFFF
#define PI_MGR_DFS_WIEGHTAGE_DEFAULT 	0xFFFFFFFF
#define PI_MGR_DFS_WIEGHTAGE_NONE		0
#define PI_OPP_UNSUPPORTED		0xFFFF

#define PI_MGR_ACTIVE_STATE_INX		0

#ifndef PI_MGR_MAX_STATE_ALLOWED
#define PI_MGR_MAX_STATE_ALLOWED 10
#endif

#ifndef MAX_CCU_PER_PI
#define MAX_CCU_PER_PI 3
#endif

#if defined(DEBUG)
#define pi_dbg printk
#else
#define pi_dbg(format...)              \
	do {                            \
	    if (pi_debug)          	\
		printk(format); 	\
	} while(0)
#endif

struct clk;
struct pi_ops;
struct pi_mgr_qos_node;
struct pi_mgr_dfs_node;

enum {
	SUB_DOMAIN_0,
	SUB_DOMAIN_1,
	SUB_DOMAIN_BOTH,
};
enum {
	POLICY_QOS = (1 << 0),
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
	POLICY_DFS = (1 << 1),
	POLICY_BOTH = (POLICY_QOS | POLICY_DFS),
#endif
};

enum {
	PI_ENABLE_ON_INIT = (1 << 0),
	PI_ARM_CORE = (1 << 1),
	PI_NO_QOS = (1 << 2),
	PI_NO_DFS = (1 << 3),
	UPDATE_PM_QOS = (1 << 5),
	NO_POLICY_CHANGE = (1 << 6),
	DFS_LIMIT_CHECK_EN = (1 << 7),
};

struct pm_pi_info {
	u32 policy_reg_offset;
	u32 ac_shift;
	u32 atl_shift;
	u32 pm_policy_shift;

	u32 fixed_vol_map_mask;
	u32 vi_to_vOx_map_mask;
	u32 wakeup_overide_mask;

	u32 counter_reg_offset;
	u32 rtn_clmp_dis_mask;

	char *reset_mgr_ccu_name;
	u32 pd_soft_reset_offset;
	u32 pd_reset_mask0;
	u32 pd_reset_mask1;
};

/*PI state flags*/
enum {
	/* Save/restore CCUs context on enter/exit this tate */
	PI_STATE_SAVE_CONTEXT = (1 << 0),
};

struct pi_state {
	u32 id;
	u32 flags;
	u32 state_policy;
	u32 hw_wakeup_latency;
};

struct pi_opp {
	u32 opp[PI_OPP_MAX];
};

struct pi {
	char *name;
	u32 flags;
	char **ccu_id;
	u32 num_ccu_id;
	struct clk *pi_ccu[MAX_CCU_PER_PI];
	u32 id;
	u32 state_allowed;
	u32 state_saved;
	int init;
	u32 usg_cnt;
	u32 opp_lmt_max;
	u32 opp_lmt_min;
	u32 opp_active;
	u32 qos_sw_event_id;
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
	u32 dfs_sw_event_id;
#endif				/*CONFIG_CHANGE_POLICY_FOR_DFS */
	struct pi_opp *pi_opp;
	u32 opp_def_weightage[PI_OPP_MAX];
	u32 num_opp;
	struct pi_state *pi_state;
	u32 num_states;
	struct pm_pi_info pi_info;
	u32 *dep_pi;
	u32 num_dep_pi;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
	struct pm_qos_request_list *pm_qos;
#else
	struct pm_qos_request_list pm_qos;
#endif
	struct pi_ops *ops;
	spinlock_t lock;
};

struct pi_mgr_qos_node {
	char *name;
	struct plist_node list;
	u32 latency;
	u32 pi_id;
	bool valid;
};

struct pi_mgr_dfs_node {
	char *name;
	struct plist_node list;
	u32 opp;
	u32 weightage;
	u32 req_active;
	u32 pi_id;
	bool valid;
};

/*change_notify state*/
enum {
	PI_PRECHANGE,
	PI_POSTCHANGE,
};

/*Struct to pass PI
 notifier info*/
struct pi_notify_param {
	int pi_id;
	u32 old_value;
	u32 new_value;
};

/*Notifier types*/
enum {
	PI_NOTIFY_DFS_CHANGE,
	PI_NOTIFY_QOS_CHANGE,
	PI_NOTIFY_POLICY_CHANGE,
};

struct pi_ops {
	int (*init) (struct pi * pi);
	int (*init_state) (struct pi * pi);
	int (*reset) (struct pi * pi, int sub_domain);
	int (*enable) (struct pi * pi, int enable);
};

extern struct pi_ops gen_pi_ops;

#ifdef CONFIG_KONA_PI_MGR
struct pi *pi_mgr_get(int pi_id);
int pi_mgr_qos_add_request(struct pi_mgr_qos_node *node, char *client_name,
			   u32 pi_id, u32 lat_value);
int pi_mgr_qos_request_update(struct pi_mgr_qos_node *node, u32 lat_value);
int pi_mgr_qos_request_remove(struct pi_mgr_qos_node *node);
int pi_set_policy(const struct pi *pi, u32 policy, int type);

int pi_mgr_disable_policy_change(int pi_id, int disable);

int pi_mgr_dfs_add_request(struct pi_mgr_dfs_node *node, char *client_name,
			   u32 pi_id, u32 opp);
int pi_mgr_dfs_request_update(struct pi_mgr_dfs_node *node, u32 opp);
int pi_mgr_dfs_add_request_ex(struct pi_mgr_dfs_node *node, char *client_name,
			      u32 pi_id, u32 opp, u32 opp_weightage);
int pi_mgr_dfs_request_update_ex(struct pi_mgr_dfs_node *node, u32 opp,
				 u32 opp_weightage);
int pi_mgr_dfs_request_remove(struct pi_mgr_dfs_node *node);
int pi_mgr_set_dfs_opp_limit(int pi_id, int min, int max);

int pi_mgr_register_notifier(u32 pi_id, struct notifier_block *notifier,
			     u32 type);
int pi_mgr_unregister_notifier(u32 pi_id, struct notifier_block *notifier,
			       u32 type);

int pi_state_allowed(int pi_id);
int pi_mgr_register(struct pi *pi);
int pi_mgr_init(void);
u32 pi_get_active_qos(int pi_id);
u32 pi_get_active_opp(int pi_id);

int __pi_enable(struct pi *pi);
int __pi_disable(struct pi *pi);

int pi_enable(struct pi *pi, int enable);
int pi_init(struct pi *pi);
int pi_init_state(struct pi *pi);

#define pi_get_name(pi)	(pi)->name
#else
static inline struct pi *pi_mgr_get(int pi_id)
{
	return NULL;
}
static inline int pi_mgr_qos_add_request(struct pi_mgr_qos_node *node,
					 char *client_name, u32 pi_id,
					 u32 lat_value)
{
	return NULL;
}
static inline int pi_mgr_qos_request_update(struct pi_mgr_qos_node *node, u32
					    lat_value)
{
	return 0;
}
static inline int pi_mgr_qos_request_remove(struct pi_mgr_qos_node *node)
{
	return 0;
}
static inline int pi_set_policy(const struct pi *pi, u32 policy, int type)
{
	return 0;
}
static inline int pi_mgr_disable_policy_change(int pi_id, int disable)
{
	return 0;
}

static inline int pi_mgr_dfs_add_request(struct pi_mgr_qos_node *node,
					 char *client_name, u32 pi_id, u32 opp)
{
	return NULL;
}
static inline int pi_mgr_dfs_request_update(struct pi_mgr_dfs_node *node,
					    u32 opp)
{
	return 0;
}
static inline int pi_mgr_dfs_request_remove(struct pi_mgr_dfs_node *node)
{
	return 0;
}
static inline int pi_mgr_set_dfs_opp_limit(int pi_id, int min, int max)
{
	return 0;
}
static inline int pi_mgr_register_notifier(u32 pi_id,
					   struct notifier_block *notifier,
					   u32 type)
{
	return 0;
}
static inline int pi_mgr_unregister_notifier(u32 pi_id,
					     struct notifier_block *notifier,
					     u32 type)
{
	return 0;
}

#endif

#ifdef CONFIG_DEBUG_FS
int __init pi_debug_init(void);
int __init pi_debug_add_pi(struct pi *pi);
#endif /* CONFIG_DEBUG_FS */

#endif /*__POWER_ISLAND_MGR_H__*/
