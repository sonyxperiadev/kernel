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

#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/err.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/plist.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/ctype.h>

#include <plat/kona_pm_dbg.h>
#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/pwr_mgr.h>
#include <asm/cputime.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#ifndef DEBUGFS_PM_CLIENT_NAME
#define DEBUGFS_PM_CLIENT_NAME		"pi_client"
#endif /*DEBUGFS_PM_CLIENT_NAME */

#define DEBUGFS_PM_CLIENT_LEN	12

#ifndef PI_MGR_DEBUG_CLIENT_MAX
#define PI_MGR_DEBUG_CLIENT_MAX		10
#endif /*PI_MGR_DEBUG_CLIENT_MAX */

#ifndef PI_MGR_DFS_WEIGHTAGE_BASE
#define PI_MGR_DFS_WEIGHTAGE_BASE 100
#endif

#define pi_is_enabled(pi) (!!(pi)->usg_cnt)

struct debug_qos_client {
	struct pi *pi;
	char client_name[20];
	int req;
	struct dentry *dent_client;
	struct pi_mgr_qos_node debugfs_qos_node;
};

struct debug_dfs_client {
	struct pi *pi;
	char client_name[20];
	int req;
	struct dentry *dent_client;
	struct pi_mgr_dfs_node debugfs_dfs_node;
};

struct debug_dfs_client debug_dfs_client[PI_MGR_DEBUG_CLIENT_MAX];
struct debug_qos_client debug_qos_client[PI_MGR_DEBUG_CLIENT_MAX];

struct debugfs_info {
	u32 pi_id;
	struct dentry *qos_dir;
	struct dentry *dfs_dir;
};
struct debugfs_info debugfs_info[PI_MGR_PI_ID_MAX];

static char debug_fs_buf[3000];

#endif /*CONFIG_DEBUG_FS */

int pi_debug;

/*PI init state*/
enum {
	PI_INIT_NONE,
	PI_INIT_BASE,
	PI_INIT_COMPLETE
};

enum {
	NODE_ADD,
	NODE_DELETE,
	NODE_UPDATE,
	NODE_RECALC,
};

static DEFINE_SPINLOCK(pi_mgr_lock);

struct pi_mgr_dfs_object {
	u32 pi_id;
	u32 default_opp;
	struct atomic_notifier_head notifiers;
	struct plist_head requests;
};

struct pi_mgr_qos_object {
	u32 pi_id;
	u32 default_latency;
	struct atomic_notifier_head notifiers;
	struct plist_head requests;
};

struct pi_mgr {
	int pi_count;
	int init;
	struct pi *pi_list[PI_MGR_PI_ID_MAX];
	struct pi_mgr_qos_object qos[PI_MGR_PI_ID_MAX];
	struct pi_mgr_dfs_object dfs[PI_MGR_PI_ID_MAX];
	struct atomic_notifier_head pol_chg_notifier[PI_MGR_PI_ID_MAX];
};

static struct pi_mgr pi_mgr;
static int pi_set_ccu_freq(struct pi *pi, u32 policy, u32 opp_inx);

static int pi_change_notify(int pi_id, u32 type, u32 old_val,
			    u32 new_val, u32 state)
{
	struct pi_notify_param param = { pi_id, old_val, new_val };

	switch (type) {
	case PI_NOTIFY_DFS_CHANGE:
		return atomic_notifier_call_chain(&pi_mgr.dfs[pi_id].notifiers,
						  state, &param);

	case PI_NOTIFY_QOS_CHANGE:
		return atomic_notifier_call_chain(&pi_mgr.qos[pi_id].notifiers,
						  state, &param);

	case PI_NOTIFY_POLICY_CHANGE:
		return atomic_notifier_call_chain(&pi_mgr.
						  pol_chg_notifier[pi_id],
						  state, &param);
	default:
		BUG();
	}
	return -EINVAL;
}

static int pi_save_state(struct pi *pi, int save)
{
	int inx;

	/*ccu_save_state may call pi_enable again
	   increment usg_cnt to avoid lockup
	 */
	pi->usg_cnt++;
	if (save) {
		for (inx = 0; inx < pi->num_ccu_id; inx++)
			ccu_save_state(to_ccu_clk(pi->pi_ccu[inx]),
				1/*save */);

		pi->state_saved = 1;
	} else {
		BUG_ON(pi->state_saved == 0);
		for (inx = 0; inx < pi->num_ccu_id; inx++)
			ccu_save_state(to_ccu_clk(pi->pi_ccu[inx]),
				0/*restore */);

		pi->state_saved = 0;
	}
	pi->usg_cnt--;
	return 0;
}

static inline int get_opp_inx(u32 id, u32 map)
{
	u32 i;
	int cnt = 0;
	if (id >= PI_OPP_MAX || !IS_SUPPORTED_OPP(map, id))
		return -EINVAL;

	for (i = 0; i <= id; i++) {
		if (map & OPP_ID_MASK(i))
			cnt++;
	}
	return cnt-1;
}

static inline int opp_inx_to_id(struct pi_opp *pi_opp, u32 inx)
{
	if (inx >= pi_opp->num_opp)
		return -EINVAL;
	return pi_opp->opp_info[0][inx].opp_id;
}

u32 pi_get_dfs_lmt(u32 pi_id, bool max)
{
	struct pi *pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);
	BUG_ON(pi->pi_opp == NULL);
	if (max)
		return opp_inx_to_id(pi->pi_opp, pi->opp_lmt_max);
	else
		return opp_inx_to_id(pi->pi_opp, pi->opp_lmt_min);
}

int __pi_enable(struct pi *pi)
{
	int inx;
	int ret = 0;
	struct pi *dep_pi;

	pi_dbg(pi->id, PI_LOG_EN_DIS, "%s: pi_name:%s, usageCount:%d\n",
	       __func__, pi->name, pi->usg_cnt);

	/*Enable dependent PIs, if any */
	for (inx = 0; inx < pi->num_dep_pi; inx++) {
		dep_pi = pi_mgr_get(pi->dep_pi[inx]);
		BUG_ON(dep_pi == NULL);
		__pi_enable(dep_pi);
	}

	/*increment usg_cnt. Return if already enabled */
	if (pi->usg_cnt++ == 0) {
		if (pi->init == PI_INIT_COMPLETE && pi->ops &&
		    ((pi->flags & NO_POLICY_CHANGE) == 0) && pi->ops->enable) {
			ret = pi->ops->enable(pi, 1);
			/*Restore the context if state was saved */
			if (pi->state_saved)
				pi_save_state(pi, 0/*restore*/);
			/*Update freq if a new freq was set while
			PI was in disabled state*/
			pi_set_ccu_freq(pi,
			pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
				pi->opp_inx_act);

		}
	}
	return ret;
}

int __pi_disable(struct pi *pi)
{
	int inx;
	int ret = 0;
	struct pi *dep_pi;

	/*decrement usg_cnt */
	if (pi->usg_cnt && --pi->usg_cnt == 0) {
		if (pi->init == PI_INIT_COMPLETE && pi->ops &&
		    ((pi->flags & NO_POLICY_CHANGE) == 0) && pi->ops->enable) {
			/*Save Context if state_allowed will cause the CCUs to
				shutdown */
			if (pi->pi_state[pi->state_allowed].
			    flags & PI_STATE_SAVE_CONTEXT)
				pi_save_state(pi, 1/*save*/);

			ret = pi->ops->enable(pi, 0);
		}
	}
	/*disable dependent PIs, if any, only state_allowed */
	for (inx = 0; inx < pi->num_dep_pi; inx++) {
		dep_pi = pi_mgr_get(pi->dep_pi[inx]);
		BUG_ON(dep_pi == NULL);
		__pi_disable(dep_pi);
	}

	return ret;
}

int pi_enable(struct pi *pi, int enable)
{
	int ret;
	unsigned long flgs;

	spin_lock_irqsave(&pi->lock, flgs);
	if (enable)
		ret = __pi_enable(pi);
	else
		ret = __pi_disable(pi);
	spin_unlock_irqrestore(&pi->lock, flgs);
	return ret;
}
EXPORT_SYMBOL(pi_enable);

static int __pi_init(struct pi *pi)
{
	int ret = 0;
	struct pi *dep_pi;
	int inx;

	pi_dbg(pi->id, PI_LOG_INIT, "%s:%s\n", __func__, pi->name);
	if (pi->init != PI_INIT_NONE)
		return 0;

	BUG_ON(pi->id >= PI_MGR_PI_ID_MAX);

	/*Make sure that dependent PIs are initialized, if any */
	for (inx = 0; inx < pi->num_dep_pi; inx++) {
		dep_pi = pi_mgr_get(pi->dep_pi[inx]);
		BUG_ON(dep_pi == NULL);
		__pi_init(dep_pi);
	}

	if (pi->ops && pi->ops->init)
		ret = pi->ops->init(pi);

	pi->init = PI_INIT_BASE;
	/*make sure that PI is at wakeup policy */
	pwr_mgr_pi_set_wakeup_override(pi->id, false);

	pi_dbg(pi->id, PI_LOG_INIT, "%s: %s %s during init\n", __func__,
	       pi->name, pi->flags & PI_ENABLE_ON_INIT ? "enable" : "disable");

	if (pi->flags & PI_ENABLE_ON_INIT)
		pi->usg_cnt++;

	return ret;
}

int pi_init(struct pi *pi)
{
	int ret;
	unsigned long flgs;

	spin_lock_irqsave(&pi->lock, flgs);
	ret = __pi_init(pi);
	spin_unlock_irqrestore(&pi->lock, flgs);
	return ret;
}
EXPORT_SYMBOL(pi_init);

#ifdef CONFIG_KONA_PI_DFS_STATS
static int pi_dfs_stats_update(struct pi *pi)
{
	u64 cur_time;
	unsigned long flgs;
	if (!pi)
		return -EPERM;

	cur_time = get_jiffies_64();
	spin_lock_irqsave(&pi->lock, flgs);

	if (pi->pi_dfs_stats.dfs_pi_active || pi->usg_cnt) {
		if (pi->pi_dfs_stats.time_in_state)
			pi->pi_dfs_stats.
			time_in_state[pi->pi_dfs_stats.last_index] +=
				cur_time - pi->pi_dfs_stats.last_time;
	}
	pi->pi_dfs_stats.last_time = cur_time;

	spin_unlock_irqrestore(&pi->lock, flgs);
	return 0;
}

static int pi_freq_chg_notifier(struct notifier_block *self, unsigned long
event, void *data)
{
	struct pi_notify_param *p = data;
	struct pi *pi = NULL;
	unsigned long flgs;

	if (event != PI_POSTCHANGE)
		return 0;

	pi = pi_mgr_get(p->pi_id);
	BUG_ON(pi == NULL);
	pi_dfs_stats_update(pi);
	spin_lock_irqsave(&pi->lock, flgs);

	pi->pi_dfs_stats.last_index = p->new_value;
	if (pi->pi_dfs_stats.trans_table)
		pi->pi_dfs_stats.trans_table[p->old_value *
			pi->pi_opp->num_opp + p->new_value]++;
	pi->pi_dfs_stats.total_trans++;
	spin_unlock_irqrestore(&pi->lock, flgs);

	return 0;
}

static int pi_state_chg_notifier(struct notifier_block *self, unsigned long
event, void *data)
{
	struct pi_notify_param *p = data;
	struct pi *dfs_dep_pi = NULL;
	struct pi_dfs_stats *pi_dfs_stats = NULL;

	unsigned long flgs;

	if (event != PI_POSTCHANGE)
		return 0;

	pi_dfs_stats = container_of(self, struct pi_dfs_stats,
						pi_state_notify_blk);
	dfs_dep_pi = container_of(pi_dfs_stats, struct pi, pi_dfs_stats);
	pi_dfs_stats_update(dfs_dep_pi);

	spin_lock_irqsave(&dfs_dep_pi->lock, flgs);

	if (!IS_ACTIVE_POLICY(p->new_value) && IS_ACTIVE_POLICY(p->old_value))
		dfs_dep_pi->pi_dfs_stats.dfs_pi_active = 0;
	else if (IS_ACTIVE_POLICY(p->new_value)
				&& !IS_ACTIVE_POLICY(p->old_value))
		dfs_dep_pi->pi_dfs_stats.dfs_pi_active = 1;

	spin_unlock_irqrestore(&dfs_dep_pi->lock, flgs);

	return 0;
}
static int __pi_dfs_stats_clear(struct pi *pi)
{
	int i;

	if (!pi)
		return -EPERM;

	for (i = pi->pi_opp->num_opp - 1; i >= 0; i--)
		pi->pi_dfs_stats.time_in_state[i] = 0;
	for (i = (pi->pi_opp->num_opp * pi->pi_opp->num_opp) - 1; i >= 0; i--)
		pi->pi_dfs_stats.trans_table[i] = 0;
	pi->pi_dfs_stats.total_trans = 0;
	pi->pi_dfs_stats.last_time = get_jiffies_64();
	pi->pi_dfs_stats.last_index = pi->opp_inx_act;

	return 0;
}
static int pi_dfs_stats_clear(struct pi *pi)
{
	unsigned long flgs;
	if (!pi)
		return -EPERM;
	spin_lock_irqsave(&pi->lock, flgs);
	__pi_dfs_stats_clear(pi);
	spin_unlock_irqrestore(&pi->lock, flgs);
	return 0;
}

static int __pi_dfs_stats_enable(struct pi *pi, int enable)
{
	struct pi *dfs_pi = NULL;

	dfs_pi = pi_mgr_get(pi->pi_dfs_stats.qos_pi_id);
	if (dfs_pi == NULL)
		return -EPERM;
	if (enable) {
		pi->pi_dfs_stats.last_time = get_jiffies_64();
		pi->pi_dfs_stats.last_index = pi->opp_inx_act;
		pi_mgr_register_notifier(pi->id, &pi->
			pi_dfs_stats.pi_dfs_notify_blk, PI_NOTIFY_DFS_CHANGE);
		if (dfs_pi->usg_cnt)
			pi->pi_dfs_stats.dfs_pi_active = 1;
		pi_mgr_register_notifier(pi->pi_dfs_stats.qos_pi_id,
			&pi->pi_dfs_stats.pi_state_notify_blk,
			PI_NOTIFY_POLICY_CHANGE);
		pi->flags |= ENABLE_DFS_STATS;
	} else {
		pi_mgr_unregister_notifier(pi->id,
			&pi->pi_dfs_stats.pi_dfs_notify_blk,
					PI_NOTIFY_DFS_CHANGE);
		pi_mgr_unregister_notifier(pi->pi_dfs_stats.qos_pi_id,
			&pi->pi_dfs_stats.pi_state_notify_blk,
			PI_NOTIFY_POLICY_CHANGE);
		pi_dfs_stats_clear(pi);
		pi->flags &= ~ENABLE_DFS_STATS;
	}
	return 0;
}

static int pi_dfs_stats_enable(struct pi *pi, int enable)
{
	int ret = 0;
	unsigned long flgs;
	if (!pi)
		return -EPERM;
	if (((pi->flags & ENABLE_DFS_STATS) && enable) ||
				(!(pi->flags & ENABLE_DFS_STATS) && !enable))
		return 0;

	if (!(pi->flags & ENABLE_DFS_STATS) && enable) {
		spin_lock_irqsave(&pi->lock, flgs);
		ret  = __pi_dfs_stats_enable(pi, 1);
		spin_unlock_irqrestore(&pi->lock, flgs);
	} else if ((pi->flags & ENABLE_DFS_STATS) && !enable) {
		ret  = __pi_dfs_stats_enable(pi, 0);
	}
	return ret;
}
#endif

static int __pi_init_state(struct pi *pi)
{
	int ret = 0;
	int inx;
	unsigned long flgs;

	pi_dbg(pi->id, PI_LOG_INIT, "%s:%s count:%d\n",
	       __func__, pi->name, pi->usg_cnt);
	BUG_ON(pi->init == PI_INIT_NONE);

	if (pi->init == PI_INIT_BASE) {
		/*PI_STATE_SAVE_CONTEXT should not be defined for
			active state*/
		BUG_ON(pi->pi_state[PI_MGR_ACTIVE_STATE_INX].flags &
			   PI_STATE_SAVE_CONTEXT);
		BUG_ON(pi->num_ccu_id > MAX_CCU_PER_PI);
		for (inx = 0; inx < pi->num_ccu_id; inx++) {
			pi->pi_ccu[inx] = clk_get(NULL, pi->ccu_id[inx]);
			BUG_ON(IS_ERR_OR_NULL(pi->pi_ccu[inx]));
		}
		spin_lock_irqsave(&pi->lock, flgs);
		if (pi->ops && pi->ops->init_state) {
			ret = pi->ops->init_state(pi);
			if (ret) {
				spin_unlock_irqrestore(&pi->lock, flgs);
				return ret;
			}
		}

		pi->init = PI_INIT_COMPLETE;

		if (pi->num_states) {
			pi_dbg(pi->id, PI_LOG_INIT,
			       "%s: %s usage_cnt on init_state (late_init):%d\n",
			       __func__, pi->name, pi->usg_cnt);

			if (pi->usg_cnt && pi->ops && pi->ops->enable)
				pi->ops->enable(pi, 1);
			else {
				/*Save Context if state_allowed will cause
				the CCUs to shutdown */
				if (pi->pi_state[pi->state_allowed].flags &
				    PI_STATE_SAVE_CONTEXT)
					pi_save_state(pi, 1 /*save */);
				if (pi->ops && pi->ops->enable)
					pi->ops->enable(pi, 0);
			}
		}
		if ((pi->flags & NO_POLICY_CHANGE) == 0) {
			struct pm_policy_cfg cfg;
			ret = pwr_mgr_event_get_pi_policy(SOFTWARE_0_EVENT,
					pi->id, &cfg);
			pi_dbg(pi->id, PI_LOG_INIT,
			       "%s: pi-%s cnt = %d  policy =%d\n", __func__,
			       pi->name, pi->usg_cnt, cfg.policy);
			pwr_mgr_pi_set_wakeup_override(pi->id,
				true/*clear*/);
		}
#ifdef CONFIG_KONA_PI_DFS_STATS
		pi->pi_dfs_stats.pi_dfs_notify_blk.notifier_call =
					pi_freq_chg_notifier;
		pi->pi_dfs_stats.pi_state_notify_blk.notifier_call =
					pi_state_chg_notifier;
		if (pi->flags & ENABLE_DFS_STATS)
			__pi_dfs_stats_enable(pi, 1);
#endif
		spin_unlock_irqrestore(&pi->lock, flgs);
	}
	return ret;
}

int pi_init_state(struct pi *pi)
{
	int ret;

	ret = __pi_init_state(pi);
	return ret;
}
EXPORT_SYMBOL(pi_init_state);

#ifndef CONFIG_CHANGE_POLICY_FOR_DFS
static int pi_set_ccu_freq(struct pi *pi, u32 policy, u32 opp_inx)
{
	int inx;
	int res = 0;
	struct opp_info *opp_info;
	struct pi_opp *pi_opp;

	BUG_ON(opp_inx >= pi->pi_opp->num_opp);
	pi_dbg(pi->id, PI_LOG_DFS,
			"%s:pi->num_ccu_id = %d\n",
			__func__, pi->num_ccu_id);
	pi_opp = pi->pi_opp;
	for (inx = 0; inx < pi->num_ccu_id; inx++) {
		opp_info = &pi_opp->opp_info[inx][opp_inx];
		pi_dbg(pi->id, PI_LOG_DFS,
			"%s:%s  pi->opp_inx_act = %d, opp_inx = %d\n",
			__func__, pi->pi_ccu[inx]->name, pi->opp_inx_act,
			opp_inx);
		if (pi->opp_inx_act !=  opp_inx ||
			ccu_get_freq_policy(to_ccu_clk(pi->pi_ccu[inx]),
			CCU_POLICY(policy)) !=  opp_info->freq_id) {

			pi_dbg(pi->id, PI_LOG_DFS,
			"%s:%s  policy => %x freq_id => %d\n",
			__func__, pi->pi_ccu[inx]->name,
			policy, opp_info->freq_id);
#ifdef CONFIG_KONA_CPU_PM_HANDLER
			log_pm(num_dbg_args[DBG_MSG_PI_SET_FREQ_OPP],
				DBG_MSG_PI_SET_FREQ_OPP, pi->id,
					opp_inx, opp_info->freq_id);
#endif
			res = ccu_set_freq_policy(to_ccu_clk
				(pi->pi_ccu[inx]),
				CCU_POLICY(policy), opp_info);

			}
	}
	return res;
}

#endif

int pi_set_policy(const struct pi *pi, u32 policy, int type)
{
	struct pm_policy_cfg cfg;
	int res;
	u32 old_pol;

	switch (type) {
	case POLICY_QOS:
		res =
		    pwr_mgr_event_get_pi_policy(pi->qos_sw_event_id, pi->id,
						&cfg);
		if (res != 0 || policy == cfg.policy)
			break;
		old_pol = cfg.policy;
		cfg.policy = policy;

		pi_change_notify(pi->id, PI_NOTIFY_POLICY_CHANGE,
				 old_pol, policy, PI_PRECHANGE);
#ifdef CONFIG_KONA_CPU_PM_HANDLER
		log_pm(num_dbg_args[DBG_MSG_PI_SET_FREQ_POLICY],
			DBG_MSG_PI_SET_FREQ_POLICY, pi->id, old_pol, policy);
#endif
		res =
		    pwr_mgr_event_set_pi_policy(pi->qos_sw_event_id, pi->id,
						&cfg);
		if (!res) {
			pi_change_notify(pi->id, PI_NOTIFY_POLICY_CHANGE,
					 old_pol, policy, PI_POSTCHANGE);
		}

		break;

#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
	case POLICY_DFS:

		res =
		    pwr_mgr_event_get_pi_policy(pi->dfs_sw_event_id, pi->id,
						&cfg);
		if (res != 0 || policy == cfg.policy)
			break;
		old_pol = cfg.policy;
		cfg.policy = policy;

		pi_change_notify(pi->id, PI_NOTIFY_POLICY_CHANGE,
				 old_pol, policy, PI_PRECHANGE);
#ifdef CONFIG_KONA_CPU_PM_HANDLER
		log_pm(num_dbg_args[DBG_MSG_PI_SET_FREQ_POLICY],
			DBG_MSG_PI_SET_FREQ_POLICY, pi->id, old_pol, policy);
#endif
		res =
		    pwr_mgr_event_set_pi_policy(pi->dfs_sw_event_id, pi->id,
						&cfg);
		if (!res) {
			pi_change_notify(pi->id, PI_NOTIFY_POLICY_CHANGE,
					 old_pol, policy, PI_POSTCHANGE);
		}
		break;

	case POLICY_BOTH:
		res =
		    pwr_mgr_event_get_pi_policy(pi->qos_sw_event_id, pi->id,
						&cfg);
		if (res != 0 || policy == cfg.policy)
			break;
		old_pol = cfg.policy;
		cfg.policy = policy;

		pi_change_notify(pi->id, PI_NOTIFY_POLICY_CHANGE,
				 old_pol, policy, PI_PRECHANGE);

		res =
		    pwr_mgr_event_set_pi_policy(pi->qos_sw_event_id, pi->id,
						&cfg);
		if (!res) {
			pi_change_notify(pi->id, PI_NOTIFY_POLICY_CHANGE,
					 old_pol, policy, PI_POSTCHANGE);
		}
		if (res == 0 && pi->dfs_sw_event_id != pi->qos_sw_event_id) {
			res =
			    pwr_mgr_event_get_pi_policy(pi->dfs_sw_event_id,
							pi->id, &cfg);
			if (res != 0 || policy == cfg.policy)
				break;
			old_pol = cfg.policy;
			cfg.policy = policy;

			pi_change_notify(pi->id, PI_NOTIFY_POLICY_CHANGE,
					 old_pol, policy, PI_PRECHANGE);

			res =
			    pwr_mgr_event_set_pi_policy(pi->dfs_sw_event_id,
							pi->id, &cfg);
			if (!res) {
				pi_change_notify(pi->id,
						 PI_NOTIFY_POLICY_CHANGE,
						 old_pol, policy,
						 PI_POSTCHANGE);
			}
		}
		break;
#endif /*CONFIG_CHANGE_POLICY_FOR_DFS */
	default:

		BUG();
		break;
	}
	return res;
}
EXPORT_SYMBOL_GPL(pi_set_policy);

static int pi_def_init_state(struct pi *pi)
{
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
	pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy =
		pi->pi_opp->opp_info[0][pi->opp_inx_act].freq_id;
	pi_set_policy(pi, pi->pi_opp->opp_info[0][pi->opp_inx_act].freq_id,
				POLICY_DFS);
#else
	pi_set_ccu_freq(pi, pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
			pi->opp_inx_act);
#endif
	return 0;
}

static int pi_def_init(struct pi *pi)
{
	struct pi_mgr_qos_object *qos;
	struct pi_mgr_dfs_object *dfs;

	pi_dbg(pi->id, PI_LOG_INIT, "%s:%s\n", __func__, pi->name);

	if ((pi->flags & PI_NO_QOS) == 0) {
		qos = &pi_mgr.qos[pi->id];
		qos->pi_id = pi->id;
		ATOMIC_INIT_NOTIFIER_HEAD(&qos->notifiers);
		plist_head_init(&qos->requests);
		BUG_ON(pi->num_states > PI_MGR_MAX_STATE_ALLOWED);

		qos->default_latency =
		    pi->pi_state[pi->num_states - 1].hw_wakeup_latency;
		pi->state_allowed = pi->num_states - 1;
		pi_dbg(pi->id, PI_LOG_INIT,
		       "qos->default_latency = %d state_allowed = %d\n",
		       qos->default_latency, pi->state_allowed);
		if (pi->flags & UPDATE_PM_QOS) {
			pm_qos_add_request(&pi->pm_qos, PM_QOS_CPU_DMA_LATENCY,
					   PM_QOS_DEFAULT_VALUE);
		}
	}

	if ((pi->flags & PI_NO_DFS) == 0) {
		pi_dbg(pi->id, PI_LOG_INIT, "pi->opp_inx_act = %d\n",
		       pi->opp_inx_act);
		dfs = &pi_mgr.dfs[pi->id];
		ATOMIC_INIT_NOTIFIER_HEAD(&dfs->notifiers);
		plist_head_init(&dfs->requests);
		dfs->pi_id = pi->id;
		dfs->default_opp = 0;
		BUG_ON(pi->pi_opp->num_opp && (pi->pi_opp == NULL ||
			pi->pi_opp->opp_info == NULL));
		if (pi->flags & DFS_LIMIT_CHECK_EN)
			BUG_ON(pi->pi_opp->num_opp <= pi->opp_lmt_max ||
			       pi->opp_lmt_max < pi->opp_lmt_min);

	}

	/*Init PI Policy change notifier */
	ATOMIC_INIT_NOTIFIER_HEAD(&pi_mgr.pol_chg_notifier[pi->id]);

	printk(KERN_INFO " %s: count = %d\n", __func__, pi->usg_cnt);
	return 0;
}

static int pi_def_enable(struct pi *pi, int enable)
{
	u32 policy;
	pi_dbg(pi->id, PI_LOG_EN_DIS,
	       "%s: pi_name:%s, enable:%d usageCount:%d\n",
	       __func__, pi->name, enable, pi->usg_cnt);
#ifdef CONFIG_KONA_CPU_PM_HANDLER
	log_pm(num_dbg_args[DBG_MSG_PI_ENABLE],
		DBG_MSG_PI_ENABLE, pi->id, enable);
#endif
	if (enable) {
		policy = pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy;
		pi_dbg(pi->id, PI_LOG_EN_DIS,
		       "%s: policy = %d -- PI to be enabled\n",
		       __func__, policy);
	} else {
		policy = pi->pi_state[pi->state_allowed].state_policy;
		pi_dbg(pi->id, PI_LOG_EN_DIS,
		       "%s: policy = %d pi->state_allowed = %d\n",
		       __func__, policy, pi->state_allowed);

	}
	pi_dbg(pi->id, PI_LOG_EN_DIS, "%s: calling pi_set_policy\n", __func__);
	return pi_set_policy(pi, policy, POLICY_QOS);
}

static int pi_reset(struct pi *pi, int sub_domain)
{
	u32 reg_val;
	u32 mask;
	struct clk *clk;
	struct ccu_clk *ccu_clk;
	unsigned long flgs;

	pi_dbg(pi->id, PI_LOG_RESET, "%s:pi_name:%s,usageCount:%d\n", __func__,
	       pi->name, pi->usg_cnt);
	if (pi->pi_info.reset_mgr_ccu_name == NULL ||
	    !pi->pi_info.pd_soft_reset_offset)
		return -EPERM;
	if ((sub_domain == SUB_DOMAIN_0 && !pi->pi_info.pd_reset_mask0) ||
	    (sub_domain == SUB_DOMAIN_1 && !pi->pi_info.pd_reset_mask1) ||
	    (sub_domain == SUB_DOMAIN_BOTH && (!pi->pi_info.pd_reset_mask0 ||
					       !pi->pi_info.pd_reset_mask1)))
		return -EPERM;

	clk = clk_get(NULL, pi->pi_info.reset_mgr_ccu_name);
	BUG_ON(IS_ERR_OR_NULL(clk));

	spin_lock_irqsave(&pi->lock, flgs);
	pi_dbg(pi->id, PI_LOG_RESET, "%s:pi:%s reset ccu str:%s\n",
	       __func__, pi->name, pi->pi_info.reset_mgr_ccu_name);

	ccu_clk = to_ccu_clk(clk);

	ccu_reset_write_access_enable(ccu_clk, true);
	reg_val = readl(ccu_clk->ccu_reset_mgr_base +
			pi->pi_info.pd_soft_reset_offset);
	pi_dbg(pi->id, PI_LOG_RESET, "reset offset: %08x, reg_val: %08x\n",
	       (u32) (ccu_clk->ccu_reset_mgr_base +
		pi->pi_info.pd_soft_reset_offset), reg_val);

	switch (sub_domain) {
	case SUB_DOMAIN_0:
		mask = pi->pi_info.pd_reset_mask0;
		break;
	case SUB_DOMAIN_1:
		mask = pi->pi_info.pd_reset_mask1;
		break;
	case SUB_DOMAIN_BOTH:
		mask = pi->pi_info.pd_reset_mask0 | pi->pi_info.pd_reset_mask1;
		break;
	default:
		spin_unlock_irqrestore(&pi->lock, flgs);
		return -EINVAL;
	}
	reg_val &= ~mask;
	pi_dbg(pi->id, PI_LOG_RESET, "writing reset value: %08x\n", reg_val);
	writel(reg_val, ccu_clk->ccu_reset_mgr_base +
	       pi->pi_info.pd_soft_reset_offset);
	udelay(10);
	reg_val |= mask;
	pi_dbg(pi->id, PI_LOG_RESET, "writing reset release value: %08x\n",
	       reg_val);
	writel(reg_val,
	       ccu_clk->ccu_reset_mgr_base + pi->pi_info.pd_soft_reset_offset);

	ccu_reset_write_access_enable(ccu_clk, false);
	spin_unlock_irqrestore(&pi->lock, flgs);
	return 0;
}

struct pi_ops gen_pi_ops = {
	.init = pi_def_init,
	.init_state = pi_def_init_state,
	.enable = pi_def_enable,
	.reset = pi_reset,
};

static u32 pi_mgr_dfs_get_opp_inx(const struct pi_mgr_dfs_object *dfs)
{
	u32 opp_inx = dfs->default_opp;
	int i;
	int sum[PI_OPP_MAX - 1] = { 0 };
	struct pi_mgr_dfs_node *dfs_node;
	struct pi *pi = pi_mgr.pi_list[dfs->pi_id];
	struct pi_opp *pi_opp = pi->pi_opp;

	if (!plist_head_empty(&dfs->requests)) {

		opp_inx = plist_last(&dfs->requests)->prio;
		if (opp_inx == (pi_opp->num_opp - 1))	/*Highest OPP ?? */
			return opp_inx;

		plist_for_each_entry(dfs_node, &dfs->requests, list) {
			if (dfs_node->req_active
			    && dfs_node->opp_inx < (pi_opp->num_opp - 1)) {
				sum[dfs_node->opp_inx] += dfs_node->weightage;
			}
		}

		for (i = opp_inx; i < pi_opp->num_opp - 1; i++) {
			if (sum[i] / PI_MGR_DFS_WEIGHTAGE_BASE)
				opp_inx++;
		}

		if (opp_inx >= pi_opp->num_opp)
			opp_inx = pi->pi_opp->num_opp - 1;
		pi_dbg(pi->id, PI_LOG_DFS, "%s:pi :%s opp = %d\n",
		       __func__, pi->name, opp_inx);
	}

	return opp_inx;
}

static u32 check_dfs_limit(struct pi *pi, u32 opp_inx)
{
	u32 act_opp_inx = opp_inx;

	if (pi->flags & DFS_LIMIT_CHECK_EN) {

		BUG_ON(pi->opp_lmt_min > pi->opp_lmt_max);
		if (opp_inx < pi->opp_lmt_min)
			act_opp_inx = pi->opp_lmt_min;
		else if (opp_inx > pi->opp_lmt_max)
			act_opp_inx = pi->opp_lmt_max;
	}
	return act_opp_inx;
}

static u32 pi_mgr_dfs_update(struct pi_mgr_dfs_node *node,
			     u32 pi_id, int action)
{
	u32 old_inx, new_inx;
	u32 old_opp, new_opp;
	struct pi_mgr_dfs_object *dfs = &pi_mgr.dfs[pi_id];
	struct pi *pi = pi_mgr.pi_list[pi_id];
	struct pi_opp *pi_opp = pi->pi_opp;
	unsigned long flgs;
	spin_lock_irqsave(&pi->lock, flgs);

	old_inx = pi->opp_inx_act;
	switch (action) {
	case NODE_ADD:
		pi_dbg(pi->id, PI_LOG_DFS, "%s:NODE_ADD-> opp_inx req = %d\n",
		       __func__, node->opp_inx);
		plist_node_init(&node->list, node->opp_inx);
		plist_add(&node->list, &dfs->requests);
		break;
	case NODE_DELETE:
		pi_dbg(pi->id, PI_LOG_DFS, "%s:NODE_DEL-> opp_inx req = %d\n",
		       __func__, node->opp_inx);
		plist_del(&node->list, &dfs->requests);
		break;
	case NODE_UPDATE:
		pi_dbg(pi->id, PI_LOG_DFS, "%s:NODE_UPD-> opp_inx req = %d\n",
		       __func__, node->opp_inx);
		plist_del(&node->list, &dfs->requests);
		plist_node_init(&node->list, node->opp_inx);
		plist_add(&node->list, &dfs->requests);
		break;

	case NODE_RECALC:
		pi_dbg(pi->id, PI_LOG_DFS, "%s:NODE_RECALC\n", __func__);
		break;

	default:
		BUG();
		break;
	}
	new_inx = check_dfs_limit(pi, pi_mgr_dfs_get_opp_inx(dfs));
	pi_dbg(pi->id, PI_LOG_OPP_CHANGE,
				"%s:pi_id= %d old_inx = %d => new_inx = %d\n",
				__func__, pi_id, old_inx, new_inx);

	if (old_inx != new_inx) {
		BUG_ON(new_inx >= pi_opp->num_opp);
		old_opp = opp_inx_to_id(pi_opp, old_inx);
		new_opp = opp_inx_to_id(pi_opp, new_inx);
		if (pi->init == PI_INIT_COMPLETE
		    && ((pi->flags & NO_POLICY_CHANGE) == 0)) {
			pi_change_notify(pi->id, PI_NOTIFY_DFS_CHANGE,
				old_opp, new_opp, PI_PRECHANGE);
			pi_dbg(pi->id, PI_LOG_OPP_CHANGE,
				"%s:pi_id= %d old_opp = %d => new_opp = %d\n",
				__func__, pi_id, old_opp, new_opp);
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
			pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy =
				pi_opp->opp_info[0][new_inx].freq_id;
			if (pi_is_enabled(pi))
				pi_set_policy(pi,
					pi->pi_opp[0]->opp[new_inx].freq_id,
					POLICY_QOS);
			if (pi->dfs_sw_event_id != pi->qos_sw_event_id)
				pi_set_policy(pi,
					pi->pi_opp[0]->opp[new_inx].freq_id,
					POLICY_DFS);
#else
			/*Update freq if in enabled state
			pi_enable function will set the new freq otherwise*/
			if (pi->flags & DEFER_DFS_UPDATE) {
				if (pi_is_enabled(pi))
					pi_set_ccu_freq(pi,
					pi->pi_state[PI_MGR_ACTIVE_STATE_INX].
					state_policy, new_inx);
			} else
				pi_set_ccu_freq(pi,
					pi->pi_state[PI_MGR_ACTIVE_STATE_INX].
					state_policy, new_inx);

#endif
			pi_change_notify(pi->id, PI_NOTIFY_DFS_CHANGE,
					 old_opp, new_opp, PI_POSTCHANGE);
		}
		pi->opp_inx_act = new_inx;
	}
	spin_unlock_irqrestore(&pi->lock, flgs);

	return new_inx;
}

static u32 pi_mgr_qos_get_value(const struct pi_mgr_qos_object *qos)
{
	if (plist_head_empty(&qos->requests))
		return qos->default_latency;
	return plist_first(&qos->requests)->prio;
}

static u32 pi_mgr_qos_update(struct pi_mgr_qos_node *node, u32 pi_id,
			     int action)
{
	u32 old_val, new_val;
	u32 old_state;
	int i;
	int found = 0;
	unsigned long flgs;
	struct pi *dep_pi;
	struct pi_mgr_qos_object *qos = &pi_mgr.qos[pi_id];
	struct pi *pi = pi_mgr.pi_list[pi_id];

	spin_lock_irqsave(&pi->lock, flgs);
	old_val = pi_mgr_qos_get_value(qos);

	switch (action) {
	case NODE_ADD:
		pi_dbg(pi->id, PI_LOG_QOS, "%s:NODE_ADD -> lat req = %d\n",
		       __func__, node->latency);
		plist_node_init(&node->list, node->latency);
		plist_add(&node->list, &qos->requests);
		break;
	case NODE_DELETE:
		pi_dbg(pi->id, PI_LOG_QOS, "%s:NODE_DELETE -> lat req = %d\n",
		       __func__, node->latency);
		plist_del(&node->list, &qos->requests);
		break;
	case NODE_UPDATE:
		pi_dbg(pi->id, PI_LOG_QOS, "%s:NODE_UPDATE -> lat req = %d\n",
		       __func__, node->latency);
		plist_del(&node->list, &qos->requests);
		plist_node_init(&node->list, node->latency);
		plist_add(&node->list, &qos->requests);
		break;
	default:
		BUG();
		break;
	}
	new_val = pi_mgr_qos_get_value(qos);
	pi_dbg(pi->id, PI_LOG_QOS, "%s:pi_id= %d oldval = %d new val = %d\n",
	       __func__, pi_id, old_val, new_val);

	if (old_val != new_val) {
		old_state = pi->state_allowed;

		pi_change_notify(pi->id, PI_NOTIFY_QOS_CHANGE,
				 old_val, new_val, PI_PRECHANGE);

		if (new_val <= pi->pi_state[0].hw_wakeup_latency)
			pi->state_allowed = 0;
		else {
			for (i = 1; i < PI_MGR_MAX_STATE_ALLOWED &&
			     i < pi->num_states; i++) {
				if (new_val >=
				    pi->pi_state[i - 1].hw_wakeup_latency
				    && new_val <
				    pi->pi_state[i].hw_wakeup_latency) {
					pi->state_allowed = i - 1;
					found = 1;
					break;
				}
			}
			BUG_ON(i == PI_MGR_MAX_STATE_ALLOWED);
			if (!found)
				pi->state_allowed = i - 1;
		}
		/*Disabling LPM through QoS ? */
		if (!IS_ACTIVE_POLICY(pi->pi_state[old_state].state_policy) &&
		    IS_ACTIVE_POLICY(pi->pi_state[pi->state_allowed].
				     state_policy)) {
			/*Enable dependent PIs, if any */
			for (i = 0; i < pi->num_dep_pi; i++) {
				dep_pi = pi_mgr_get(pi->dep_pi[i]);
				BUG_ON(dep_pi == NULL);
				__pi_enable(dep_pi);
			}

		}
		/*re-enabling LPM PI through QoS ? */
		else if (IS_ACTIVE_POLICY(pi->pi_state[old_state].state_policy)
			 && !IS_ACTIVE_POLICY(pi->pi_state[pi->state_allowed].
					      state_policy)) {
			/*disable dependent PIs, if any */
			for (i = 0; i < pi->num_dep_pi; i++) {
				dep_pi = pi_mgr_get(pi->dep_pi[i]);
				BUG_ON(dep_pi == NULL);
				__pi_disable(dep_pi);
			}

		}

		if (pi->init == PI_INIT_COMPLETE) {
			if (!pi_is_enabled(pi)
			    && pi->state_allowed != old_state) {
				/*Do pi_enable and pi_disable call to switch to
				the right state. Also needed to handle
				state save/restore properly
				 */
				__pi_enable(pi);
				__pi_disable(pi);
			}
		}

		pi_change_notify(pi->id, PI_NOTIFY_QOS_CHANGE,
				 old_val, new_val, PI_POSTCHANGE);

	}
	pi_dbg(pi->id, PI_LOG_QOS, "%s:%s state allowed = %d\n", __func__,
	       pi->name, pi->state_allowed);
	spin_unlock_irqrestore(&pi->lock, flgs);
	/*We can't invoke pm_qos_update_request from intr disabled
	context*/
	if (pi->flags & UPDATE_PM_QOS && old_val != new_val)
			pm_qos_update_request(&pi->pm_qos, new_val);
	return new_val;
}

int pi_state_allowed(int pi_id)
{
	int ret = -EINVAL;
	struct pi *pi = pi_mgr_get(pi_id);
	if (pi)
		ret = pi->state_allowed;
	return ret;
}
EXPORT_SYMBOL(pi_state_allowed);

int pi_get_use_count(int pi_id)
{
	int ret = -EINVAL;
	struct pi *pi = pi_mgr_get(pi_id);

	if (pi)
		ret = pi->usg_cnt;

	return ret;
}
EXPORT_SYMBOL(pi_get_use_count);

u32 pi_get_active_qos(int pi_id)
{
	struct pi_mgr_qos_object *qos;
	BUG_ON(pi_id >= PI_MGR_PI_ID_MAX);
	qos = &pi_mgr.qos[pi_id];
	return pi_mgr_qos_get_value(qos);
}
EXPORT_SYMBOL(pi_get_active_qos);

u32 pi_get_active_opp(int pi_id)
{
	int ret = -EINVAL;
	struct pi *pi = pi_mgr_get(pi_id);
	/*Before PI init is complete, PI policy is set to wake up ploicy, 7.
	   Policy 7 freq is initialized to turbo mode freq.
	   Hence, this function should return the max opp number if the init is
	   not complete. */
	if (pi && pi->pi_opp) {
		if (pi->init == PI_INIT_COMPLETE)
			ret = opp_inx_to_id(pi->pi_opp, pi->opp_inx_act);
		else
			return opp_inx_to_id(pi->pi_opp,
					pi->pi_opp->num_opp - 1);
	}
	return ret;
}
EXPORT_SYMBOL(pi_get_active_opp);

int pi_mgr_register(struct pi *pi)
{
	unsigned long flgs;

	pi_dbg(pi->id, PI_LOG_INIT, "%s:name:%s id:%d\n", __func__,
	       pi->name, pi->id);
	if (!pi_mgr.init) {
		pi_dbg(pi->id, PI_LOG_ERR,
		       "%s:ERROR - pi mgr not initialized\n", __func__);
		return -EPERM;
	}
	if (pi->id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi->id]) {
		pi_dbg(pi->id, PI_LOG_ERR,
		       "%s:pi already registered or invalid id\n", __func__);
		return -EPERM;
	}
	spin_lock_irqsave(&pi_mgr_lock, flgs);
	spin_lock_init(&pi->lock);
	pi_mgr.pi_list[pi->id] = pi;
	pi_mgr.pi_count++;
	spin_unlock_irqrestore(&pi_mgr_lock, flgs);

	return 0;
}
EXPORT_SYMBOL(pi_mgr_register);

int pi_mgr_qos_add_request(struct pi_mgr_qos_node *node, char *client_name,
			   u32 pi_id, u32 lat_value)
{
	pi_dbg(pi_id, PI_LOG_QOS, "%s:client = %s,pi_id = %d, lat_val = %d\n",
	       __func__, client_name, pi_id, lat_value);
	if (unlikely(!pi_mgr.init)) {
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR - pi mgr not initialized\n", __func__);
		return -EPERM;
	}
	if (node == NULL) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR Invalid node\n", __func__);
		return -EINVAL;
	}
	if (node->valid) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR node already added\n",
		       __func__);
		return -EINVAL;
	}
	if (unlikely
	    (pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR -invalid pid\n", __func__);
		return -EINVAL;
	}

	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS)) {
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR -QOS not supported for this PI\n", __func__);
		return -EPERM;
	}
	if (lat_value == PI_MGR_QOS_DEFAULT_VALUE) {
		lat_value = pi_mgr.qos[pi_id].default_latency;
		pi_dbg(pi_id, PI_LOG_QOS,
		       "%s:lat_value =PI_MGR_QOS_DEFAULT_VALUE, def_at =%d\n",
		       __func__, lat_value);
	}

	node->name = client_name;
	node->latency = lat_value;
	node->pi_id = pi_id;
	pi_mgr_qos_update(node, pi_id, NODE_ADD);
	node->valid = 1;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_add_request);

int pi_mgr_qos_request_update(struct pi_mgr_qos_node *node, u32 lat_value)
{
	if (!pi_mgr.init) {
		pi_dbg(node->pi_id, PI_LOG_ERR,
		       "%s:ERROR -pi mgr not initialized\n", __func__);
		return -EINVAL;
	}
	BUG_ON(node->valid == 0);
	pi_dbg(node->pi_id, PI_LOG_QOS, "%s: exisiting req = %d new_req = %d\n",
	       __func__, node->latency, lat_value);
	if (lat_value == PI_MGR_QOS_DEFAULT_VALUE)
		lat_value = pi_mgr.qos[node->pi_id].default_latency;

	if (node->latency != lat_value) {
		node->latency = lat_value;
		pi_mgr_qos_update(node, node->pi_id, NODE_UPDATE);
	}

	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_request_update);

int pi_mgr_qos_request_remove(struct pi_mgr_qos_node *node)
{
	pi_dbg(node->pi_id, PI_LOG_QOS, "%s:name = %s, req = %d\n", __func__,
	       node->name, node->latency);
	BUG_ON(node->valid == 0);
	if (!pi_mgr.init) {
		pi_dbg(node->pi_id, PI_LOG_ERR,
		       "%s:ERROR -pi mgr not initialized\n", __func__);
		return -EINVAL;
	}
	pi_mgr_qos_update(node, node->pi_id, NODE_DELETE);
	node->name = NULL;
	node->valid = 0;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_request_remove);

int pi_mgr_dfs_add_request(struct pi_mgr_dfs_node *node, char *client_name,
			   u32 pi_id, u32 opp)
{
	return pi_mgr_dfs_add_request_ex(node, client_name, pi_id, opp,
					 PI_MGR_DFS_WIEGHTAGE_DEFAULT);
}
EXPORT_SYMBOL(pi_mgr_dfs_add_request);

int pi_mgr_dfs_request_update(struct pi_mgr_dfs_node *node, u32 opp)
{
	return pi_mgr_dfs_request_update_ex(node, opp,
					    PI_MGR_DFS_WIEGHTAGE_DEFAULT);
}
EXPORT_SYMBOL(pi_mgr_dfs_request_update);

int pi_mgr_dfs_add_request_ex(struct pi_mgr_dfs_node *node, char *client_name,
			      u32 pi_id, u32 opp, u32 weightage)
{
	struct pi *pi;
	struct pi_opp *pi_opp;
	int inx;
	pi = pi_mgr_get(pi_id);
	if (pi)
		pi_opp = pi->pi_opp;
	else
		return -EINVAL;
	inx = (opp == PI_MGR_DFS_MIN_VALUE) ? 0 :
		get_opp_inx(opp, pi_opp->opp_map);

	pi_dbg(pi_id, PI_LOG_DFS, "%s:client = %s pi = %d opp = %d\n",
	       __func__, client_name, pi_id, opp);
	if (!pi_mgr.init) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR -pi mgr not initialized\n",
		       __func__);
		return -EPERM;
	}
	if (node == NULL) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR Invalid node\n", __func__);
		return -EINVAL;
	}
	if (pi_id >= PI_MGR_PI_ID_MAX ||
		pi_mgr.pi_list[pi_id] == NULL) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR-invalid pid\n", __func__);
		return -EINVAL;
	}
	pi = pi_mgr.pi_list[pi_id];
	if (unlikely(pi->flags & PI_NO_DFS)) {
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR -DFS not supported for this PI\n", __func__);
		return -EPERM;
	}

	if (inx < 0) {
		__WARN();
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR - %d:unsupp opp, err:%d\n",
				__func__, opp, inx);
		return -EINVAL;
	}

	if (PI_MGR_DFS_WIEGHTAGE_DEFAULT != weightage
	    && weightage >= PI_MGR_DFS_WEIGHTAGE_BASE) {
		__WARN();
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR - %d:unsupported weightage\n",
		       __func__, weightage);
		return -EINVAL;
	}
	node->name = client_name;
	node->pi_id = pi_id;

	if (opp == PI_MGR_DFS_MIN_VALUE) {
		node->req_active = 0;
		node->opp_inx = 0;
	} else {
		node->req_active = 1;
		node->opp_inx = (u32)inx;
	}
	pi_opp = pi->pi_opp;
	BUG_ON(!pi->pi_opp ||
		node->opp_inx >= pi_opp->num_opp);
	if (weightage == PI_MGR_DFS_WIEGHTAGE_DEFAULT) {
		if (pi_opp->def_weightage)
			node->weightage = pi_opp->def_weightage[node->opp_inx];
		else
			node->weightage = 0;
	}
	else
		node->weightage = weightage;

	BUG_ON(node->weightage >= PI_MGR_DFS_WEIGHTAGE_BASE);

	pi_mgr_dfs_update(node, pi_id, NODE_ADD);
	node->valid = 1;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_dfs_add_request_ex);

int pi_mgr_dfs_request_update_ex(struct pi_mgr_dfs_node *node, u32 opp,
				 u32 weightage)
{
	struct pi *pi = pi_mgr.pi_list[node->pi_id];
	struct pi_opp *pi_opp = pi->pi_opp;
	int inx = (opp == PI_MGR_DFS_MIN_VALUE) ? 0 :
				get_opp_inx(opp, pi_opp->opp_map);

	BUG_ON(!pi || !pi->pi_opp);

	if (!pi_mgr.init) {
		pi_dbg(node->pi_id, PI_LOG_ERR,
		       "%s:ERROR -pi mgr not initialized\n", __func__);
		return -EINVAL;
	}
	BUG_ON(node->valid == 0);
	if (inx < 0) {
		__WARN();
		pi_dbg(node->pi_id, PI_LOG_ERR,
		       "%s:ERROR - %d:unsupported opp, error val: %d\n",
		       __func__, opp, inx);
		return -EINVAL;
	}

	if (PI_MGR_DFS_WIEGHTAGE_DEFAULT != weightage
	    && weightage >= PI_MGR_DFS_WEIGHTAGE_BASE) {
		__WARN();
		pi_dbg(node->pi_id, PI_LOG_ERR,
		       "%s:ERROR - %d:unsupported weightage\n",
		       __func__, weightage);
		return -EINVAL;
	}

	pi_dbg(node->pi_id, PI_LOG_DFS,
		"%s:client = %s pi= %d opp= %d opp_new= %d weightage= %d\n",
		__func__, node->name, node->pi_id,
		opp_inx_to_id(pi_opp, node->opp_inx), opp, weightage);

	if (node->opp_inx != (u32)inx || weightage != node->weightage) {
		if (opp == PI_MGR_DFS_MIN_VALUE) {
			node->req_active = 0;
			node->opp_inx = 0;
		} else {
			node->opp_inx = (u32)inx;
			node->req_active = 1;
		}
		BUG_ON(node->opp_inx >= pi_opp->num_opp);

		if (weightage == PI_MGR_DFS_WIEGHTAGE_DEFAULT) {
			if (pi_opp->def_weightage)
				node->weightage =
					pi_opp->def_weightage[node->opp_inx];
			else
				node->weightage = 0;
		} else
			node->weightage = weightage;

		BUG_ON(node->weightage >= PI_MGR_DFS_WEIGHTAGE_BASE);

		pi_mgr_dfs_update(node, node->pi_id, NODE_UPDATE);
	}

	return 0;
}
EXPORT_SYMBOL(pi_mgr_dfs_request_update_ex);

int pi_mgr_dfs_request_remove(struct pi_mgr_dfs_node *node)
{
	struct pi *pi = pi_mgr_get(node->pi_id);
	BUG_ON(pi == NULL);
	pi_dbg(node->pi_id, PI_LOG_DFS, "%s:name = %s, req = %d\n", __func__,
	       node->name, opp_inx_to_id(pi->pi_opp, node->opp_inx));
	BUG_ON(node->valid == 0);
	if (!pi_mgr.init) {
		pi_dbg(node->pi_id, PI_LOG_ERR,
		       "%s:ERROR - pi mgr not initialized\n", __func__);
		return -EINVAL;
	}
	pi_mgr_dfs_update(node, node->pi_id, NODE_DELETE);
	node->name = NULL;
	node->valid = 0;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_dfs_request_remove);

int pi_mgr_set_dfs_opp_limit(int pi_id, int min, int max)
{
	struct pi *pi = pi_mgr_get(pi_id);
	struct pi_opp *pi_opp;
	bool update = false;
	int min_inx, max_inx;

	BUG_ON(pi == NULL);
	pi_opp = pi->pi_opp;
	if (!pi_opp || ((pi->flags & DFS_LIMIT_CHECK_EN) == 0))
		return -EINVAL;
	min_inx = get_opp_inx(min, pi_opp->opp_map);
	max_inx = get_opp_inx(max, pi_opp->opp_map);

	if (min_inx >= 0 && (u32) min_inx != pi->opp_lmt_min) {
		pi->opp_lmt_min = (u32) min_inx;
		update = true;
	}

	if (max_inx >= 0 && (u32) max_inx != pi->opp_lmt_max) {
		if ((u32) max_inx >= pi_opp->num_opp)
			return -EINVAL;
		pi->opp_lmt_max = (u32) max_inx;
		pi_dbg(pi_id, PI_LOG_POLICY,
			"%s: pi->opp_lmt_max = %d\n", __func__,
			pi->opp_lmt_max);
		update = true;
	}
	if (update)
		pi_mgr_dfs_update(NULL, pi_id, NODE_RECALC);
	return 0;
}
EXPORT_SYMBOL(pi_mgr_set_dfs_opp_limit);

/*Interface function to PI disable policy change
 PI policy will be set to 7, and PI policies won't be updated
*/
int pi_mgr_disable_policy_change(int pi_id, int disable)
{
	struct pi *pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);

	if (disable && ((pi->flags & NO_POLICY_CHANGE) == 0)) {
		pi_dbg(pi_id, PI_LOG_POLICY, "%s :dis pol change\n", __func__);
		pi->flags |= NO_POLICY_CHANGE;
		pwr_mgr_pi_set_wakeup_override(pi->id, false);
	} else if (!disable && (pi->flags & NO_POLICY_CHANGE)) {
		pi_dbg(pi_id, PI_LOG_POLICY,
		       "%s :enable pol change\n", __func__);
		pi->flags &= ~NO_POLICY_CHANGE;

		/*Update PI DFS freq based on opp_inx_act value */
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
		pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy =
			pi->pi_opp->opp_info[0][pi->opp_inx_act].freq_id;
		pi_set_policy(pi,
			pi->pi_opp->opp_info[0][pi->opp_inx_act].freq_id,
			      POLICY_DFS);
#else
		pi_set_ccu_freq(pi,
				pi->pi_state[PI_MGR_ACTIVE_STATE_INX].
				state_policy, pi->opp_inx_act);
#endif

		if (pi->usg_cnt)
			pi->ops->enable(pi, 1);
		else {
			/*Save Context if state_allowed will cause
			the CCUs to shutdown */
			if (pi->pi_state[pi->state_allowed].flags &
						PI_STATE_SAVE_CONTEXT)
				pi_save_state(pi, 1/*save */);
			pi->ops->enable(pi, 0);
		}

		pwr_mgr_pi_set_wakeup_override(pi->id, true);
	}
	return 0;
}
EXPORT_SYMBOL(pi_mgr_disable_policy_change);

static int pi_mgr_dfs_add_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS)) {
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR - DFS not supported for this PI\n", __func__);
		return -EINVAL;
	}

	return atomic_notifier_chain_register(&pi_mgr.dfs[pi_id].notifiers,
					      notifier);
}

static int pi_mgr_dfs_remove_notifier(u32 pi_id,
				      struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS)) {
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR -DFS not supported for this PI\n", __func__);
		return -EINVAL;
	}

	return atomic_notifier_chain_unregister(&pi_mgr.dfs[pi_id].notifiers,
						notifier);

}

static int pi_mgr_qos_add_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS)) {
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR -QOS not supported for this PI\n", __func__);
		return -EINVAL;
	}

	return atomic_notifier_chain_register(&pi_mgr.qos[pi_id].notifiers,
					      notifier);
}

static int pi_mgr_qos_remove_notifier(u32 pi_id,
				      struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS)) {
		pi_dbg(pi_id, PI_LOG_ERR,
		       "%s:ERROR -QOS not supported for this PI\n", __func__);
		return -EINVAL;
	}

	return atomic_notifier_chain_unregister(&pi_mgr.qos[pi_id].notifiers,
						notifier);

}

int pi_mgr_register_notifier(u32 pi_id, struct notifier_block *notifier,
			     u32 type)
{
	if (!pi_mgr.init) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR -pi mgr not initialized\n",
		       __func__);
		return -EINVAL;
	}
	if (pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR - invalid pid\n", __func__);
		return -EINVAL;
	}
	switch (type) {
	case PI_NOTIFY_DFS_CHANGE:
		return pi_mgr_dfs_add_notifier(pi_id, notifier);

	case PI_NOTIFY_QOS_CHANGE:
		return pi_mgr_qos_add_notifier(pi_id, notifier);

	case PI_NOTIFY_POLICY_CHANGE:
		return atomic_notifier_chain_register(&pi_mgr.
						      pol_chg_notifier[pi_id],
						      notifier);
	}
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(pi_mgr_register_notifier);

int pi_mgr_unregister_notifier(u32 pi_id, struct notifier_block *notifier,
			       u32 type)
{
	if (!pi_mgr.init) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR -pi mgr not initialized\n",
		       __func__);
		return -EINVAL;
	}
	if (pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR -invalid pid\n", __func__);
		return -EINVAL;
	}
	switch (type) {
	case PI_NOTIFY_DFS_CHANGE:
		return pi_mgr_dfs_remove_notifier(pi_id, notifier);

	case PI_NOTIFY_QOS_CHANGE:
		return pi_mgr_qos_remove_notifier(pi_id, notifier);

	case PI_NOTIFY_POLICY_CHANGE:
		return atomic_notifier_chain_unregister(&pi_mgr.
							pol_chg_notifier[pi_id],
							notifier);
	}
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(pi_mgr_unregister_notifier);

struct pi *pi_mgr_get(int pi_id)
{
	if (!pi_mgr.init) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:ERROR -pi mgr not initialized\n",
		       __func__);
		return NULL;
	}
	if (pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL) {
		pi_dbg(pi_id, PI_LOG_ERR, "%s:invalid pi_id id\n", __func__);
		return NULL;
	}
	return pi_mgr.pi_list[pi_id];
}
EXPORT_SYMBOL(pi_mgr_get);

int pi_mgr_initialized(void)
{
	return pi_mgr.init;
}
EXPORT_SYMBOL(pi_mgr_initialized);

int pi_mgr_init()
{
	memset(&pi_mgr, 0, sizeof(pi_mgr));
	pi_mgr.init = 1;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_init);

__weak int chip_reset(void)
{
	return 0;
}

__weak int get_state_dep_pi_id(u32 pi_id)
{
	return -1;
}

__weak char *get_opp_name(int opp)
{
	return NULL;
}

__weak u32 get_opp_from_name(char *name)
{
	return -EINVAL;
}

int pi_mgr_print_active_pis(void)
{
	u32 i;
	struct pi *pi;
	struct pi_mgr_qos_object *qos;
	struct pi_mgr_qos_node *qos_node;
	unsigned long flag;
	int state_policy;
	int num_active = 0;

	if (unlikely(!pi_mgr.init))
		return -EPERM;

	for (i = 0; i < PI_MGR_PI_ID_MAX; i++) {
		pi = pi_mgr.pi_list[i];

		if (!pi)
			continue;
		qos = &pi_mgr.qos[pi->id];

		if (pi_mgr.pi_list[pi->id]->flags & PI_NO_QOS)
			continue;
		spin_lock_irqsave(&pi->lock, flag);
		state_policy = pi->pi_state[pi->state_allowed].state_policy;
		if (IS_ACTIVE_POLICY(state_policy)) {
			pr_info("%s \t%d \t%d\n", pi->name,
					pi->state_allowed, pi->usg_cnt);
			pr_info("-- Qos Request List --\n");
			pr_info("client \t latency\n");
			/**
			 * print the QoS Request list
			 */
			plist_for_each_entry(qos_node, &qos->requests, list)
				pr_info("%s \t %d\n", qos_node->name,
						qos_node->latency);
			pr_info("---------------------\n");
			num_active++;
		}
		spin_unlock_irqrestore(&pi->lock, flag);
	}
	pr_info("%s, Num PIs with active QOS req: %d", __func__, num_active);
	return num_active;
}
EXPORT_SYMBOL(pi_mgr_print_active_pis);

#ifdef CONFIG_DEBUG_FS

static int pi_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int pi_debug_set_qos(void *data, u64 val)
{
	u32 inx = (u32) data;
	int ret;
	BUG_ON(debug_qos_client[inx].pi == NULL);
	ret =
	    pi_mgr_qos_request_update(&debug_qos_client[inx].debugfs_qos_node,
				      val);
	if (ret == 0)
		debug_qos_client[inx].req = (int)val;
	return ret;
}

static int pi_debug_get_qos(void *data, u64 * val)
{
	u32 inx = (u32) data;
	BUG_ON(debug_qos_client[inx].pi == NULL);
	*val = debug_qos_client[inx].req;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_qos_client_fops, pi_debug_get_qos, pi_debug_set_qos,
			"%llu\n");

static int pi_debug_get_active_qos(void *data, u64 * val)
{
	struct pi *pi = data;
	*val = pi_get_active_qos(pi->id);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_qos_fops, pi_debug_get_active_qos, NULL, "%llu\n");

static int pi_debug_register_qos_client(void *data, u64 value)
{
	struct pi *pi = data;
	u32 val = (u32) value;
	struct dentry *qos_dir = 0, *dent_client = 0;
	int ret = 0;

	if (val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

	pi_dbg(pi->id, PI_LOG_DBGFS, "%s: client_name  %s_%d\n", __func__,
	       DEBUGFS_PM_CLIENT_NAME, val);

	if (debug_qos_client[val].pi) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: client in use by %s\n",
		       __func__, pi->name);
		return 0;
	}

	qos_dir = debugfs_info[pi->id].qos_dir;
	snprintf(debug_qos_client[val].client_name, DEBUGFS_PM_CLIENT_LEN,
		"%s_%d", DEBUGFS_PM_CLIENT_NAME, val);

	dent_client =
	    debugfs_create_file(debug_qos_client[val].client_name,
				S_IWUSR | S_IRUSR, qos_dir, (void *)val,
				&pi_qos_client_fops);
	if (!dent_client)
		goto err;

	ret =
	    pi_mgr_qos_add_request(&debug_qos_client[val].debugfs_qos_node,
				   debug_qos_client[val].client_name, pi->id,
				   PI_MGR_QOS_DEFAULT_VALUE);
	if (ret)
		goto err;

	debug_qos_client[val].dent_client = dent_client;
	debug_qos_client[val].pi = pi;
	debug_qos_client[val].req = PI_MGR_QOS_DEFAULT_VALUE;

	return 0;
err:
	debug_qos_client[val].dent_client = NULL;
	return -ENOMEM;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_qos_register_client_fops, NULL,
			pi_debug_register_qos_client, "%llu\n");

static int pi_debug_remove_qos_client(void *data, u64 value)
{
	struct pi *pi = data;
	int ret;
	u32 val = (u32) value;

	if (val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

	pi_dbg(pi->id, PI_LOG_DBGFS, "%s: client_name %s_%d\n", __func__,
	       DEBUGFS_PM_CLIENT_NAME, val);

	if (debug_qos_client[val].pi &&
			debug_qos_client[val].pi->id == pi->id) {
		BUG_ON(!debug_qos_client[val].pi ||
		       !debug_qos_client[val].dent_client);
		ret =
		    pi_mgr_qos_request_remove(&debug_qos_client[val].
					      debugfs_qos_node);
		if (ret)
			pi_dbg(pi->id, PI_LOG_ERR, "Failed to remove node\n");
		debugfs_remove(debug_qos_client[val].dent_client);

		debug_qos_client[val].pi = NULL;
		debug_qos_client[val].client_name[0] = '\0';
		debug_qos_client[val].dent_client = NULL;
		pi_dbg(pi->id, PI_LOG_DBGFS,
		       "This client registration removed for this PI\n");
	} else
		pi_dbg(pi->id, PI_LOG_ERR, "%s: client not registered\n",
		       __func__);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_qos_remove_client_fops, NULL,
			pi_debug_remove_qos_client, "%llu\n");

static ssize_t read_get_qos_request_list(struct file *file,
					 char __user *user_buf, size_t count,
					 loff_t *ppos)
{
	u32 len = 0;
	struct pi *pi = (struct pi *)file->private_data;
	struct pi_mgr_qos_object *qos = &pi_mgr.qos[pi->id];
	struct pi_mgr_qos_node *qos_node;

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"************ PI qos requests *****************\n");
	plist_for_each_entry(qos_node, &qos->requests, list) {
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
				"PI: %s (Id:%d)\t\tClient: %s\t\tLatency_request: %u\n",
				pi->name, qos_node->pi_id, qos_node->name,
				qos_node->latency);
	}
	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations pi_qos_request_list_fops = {
	.open = pi_debugfs_open,
	.read = read_get_qos_request_list,
};

static ssize_t pi_debug_set_dfs_client_opp(struct file *file,
					   const char __user *buf,
					   size_t count, loff_t *offset)
{
	u32 len = 0;
	int opp = PI_MGR_DFS_MIN_VALUE;
	char opp_str[15];
	u32 weightage = PI_MGR_DFS_WIEGHTAGE_DEFAULT;
	char input_str[15];
	u32 inx = (u32) file->private_data;
	int opp_inx;
	struct pi *pi = debug_dfs_client[inx].pi;
	BUG_ON(pi == NULL);
	if (count > 15)
		len = 15;
	else
		len = count;
/* usage of sscanf and copy from user are for debugfs operations, so they will
 * not create any problems, so using coverity ignore comments */
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	sscanf(input_str, "%s%u", opp_str, &weightage);
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	if (isdigit(opp_str[0]))
		opp = (opp_str[0] - toascii('0'));
	else
		opp = get_opp_from_name(opp_str);
	if (opp < 0) {
		pr_err("%s: Invalid opp %s, opp_id %d\n",
				__func__, opp_str, opp);
		return count;
	}
	opp_inx = get_opp_inx(opp, pi->pi_opp->opp_map);
	if (opp_inx > pi->opp_lmt_max || opp_inx < 0) {
		pr_err("%s: Unsupported OPP %d, max %d\n",
			__func__, opp, opp_inx_to_id(
			pi->pi_opp, pi->opp_lmt_max));
		return count;
	}
	pi_mgr_dfs_request_update_ex(&debug_dfs_client[inx].debugfs_dfs_node,
				     opp, weightage);
	return count;
}

static ssize_t pi_debug_get_dfs_client_opp(struct file *file,
					   char __user *user_buf, size_t count,
					   loff_t *ppos)
{
	u32 len = 0;
	u32 inx = (u32) file->private_data;
	char *str;
	str = get_opp_name(opp_inx_to_id(debug_dfs_client[inx].pi->pi_opp,
			debug_dfs_client[inx].debugfs_dfs_node.opp_inx));
	if (str == NULL) {
		pr_err("%s: Invalid opp index %d\n", __func__,
			debug_dfs_client[inx].debugfs_dfs_node.opp_inx);
		return count;
	}

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"DFS Req:%s Request active:%u Request Weightage:%u\n",
			str, debug_dfs_client[inx].debugfs_dfs_node.req_active,
			debug_dfs_client[inx].debugfs_dfs_node.weightage);
	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations pi_dfs_client_fops = {
	.open = pi_debugfs_open,
	.read = pi_debug_get_dfs_client_opp,
	.write = pi_debug_set_dfs_client_opp,
};

static ssize_t pi_debug_get_active_dfs(struct file *file,
		char __user *user_buf, size_t count, loff_t *ppos)
{

	struct pi *pi = file->private_data;
	u32 len = 0;
	char *str;
	BUG_ON(pi == NULL);
	str = get_opp_name(pi_get_active_opp(pi->id));
	if (str == NULL) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid DFS\n",
				__func__);
		return count;
	}
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"Active OPP: %s\n", str);

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
			len);
}

static const struct file_operations pi_dfs_fops = {
	.open = pi_debugfs_open,
	.read = pi_debug_get_active_dfs,
};

static int pi_debug_register_dfs_client(void *data, u64 value)
{
	struct pi *pi = data;
	int ret = -1;
	u32 val = (u32) value;
	struct dentry *dfs_dir = 0, *dent_client = 0;

	if (val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

	pi_dbg(pi->id, PI_LOG_DBGFS, "%s: client_name  %s_%d\n", __func__,
	       DEBUGFS_PM_CLIENT_NAME, val);

	if (debug_dfs_client[val].pi) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s:client in use by %s\n", __func__,
		       debug_dfs_client[val].pi->name);
		return 0;
	}

	dfs_dir = debugfs_info[pi->id].dfs_dir;
	snprintf(debug_dfs_client[val].client_name, DEBUGFS_PM_CLIENT_LEN,
			"%s_%d", DEBUGFS_PM_CLIENT_NAME, val);

	ret =
	    pi_mgr_dfs_add_request(&debug_dfs_client[val].debugfs_dfs_node,
				   debug_dfs_client[val].client_name, pi->id,
				   PI_MGR_DFS_MIN_VALUE);
	if (ret)
		pi_dbg(pi->id, PI_LOG_ERR,
		       "%s: DFS add request failed for %s\n", __func__,
		       pi->name);

	dent_client =
	    debugfs_create_file(debug_dfs_client[val].client_name,
				S_IWUSR | S_IRUSR, dfs_dir, (void *)val,
				&pi_dfs_client_fops);
	if (!dent_client)
		goto err;
	debug_dfs_client[val].dent_client = dent_client;
	debug_dfs_client[val].pi = pi;
	debug_dfs_client[val].req = 0;

	return 0;
err:
	debug_dfs_client[val].debugfs_dfs_node.name = NULL;
	debug_dfs_client[val].dent_client = NULL;
	debug_dfs_client[val].pi = NULL;
	return -ENOMEM;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_register_client_fops, NULL,
			pi_debug_register_dfs_client, "%llu\n");

static int pi_debug_remove_dfs_client(void *data, u64 value)
{
	struct pi *pi = data;
	int val = (int)value;
	int ret;

	if (val < 0 || val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

	pi_dbg(pi->id, PI_LOG_DBGFS, "%s: client_name %s_%d\n", __func__,
	       DEBUGFS_PM_CLIENT_NAME, val);

	if (debug_dfs_client[val].pi &&
		debug_dfs_client[val].pi->id == pi->id) {
		BUG_ON(!debug_dfs_client[val].dent_client);
		ret =
		    pi_mgr_dfs_request_remove(&debug_dfs_client[val].
					      debugfs_dfs_node);
		if (ret)
			pi_dbg(pi->id, PI_LOG_ERR, "Failed to remove node\n");
		debugfs_remove(debug_dfs_client[val].dent_client);

		debug_dfs_client[val].pi = NULL;
		debug_dfs_client[val].client_name[0] = '\0';
		debug_dfs_client[val].dent_client = NULL;
		debug_dfs_client[val].debugfs_dfs_node.name = NULL;
		pi_dbg(pi->id, PI_LOG_DBGFS,
		       "This client registration removed for this PI\n");
	} else
		pi_dbg(pi->id, PI_LOG_ERR,
		       "%s: client not registered\n", __func__);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_remove_client_fops, NULL,
			pi_debug_remove_dfs_client, "%llu\n");

static ssize_t read_get_dfs_request_list(struct file *file,
					 char __user *user_buf, size_t count,
					 loff_t *ppos)
{
	u32 len = 0;
	struct pi *pi = (struct pi *)file->private_data;
	struct pi_mgr_dfs_object *dfs = &pi_mgr.dfs[pi->id];
	struct pi_mgr_dfs_node *dfs_node;
	char *opp_str;
	BUG_ON(pi == NULL || dfs == NULL);

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
		"*********** PI DFS requests **********************\n");
	plist_for_each_entry(dfs_node, &dfs->requests, list) {
		opp_str = get_opp_name(opp_inx_to_id(pi->pi_opp,
						dfs_node->opp_inx));
		if (opp_str == NULL) {
			pi_dbg(pi->id, PI_LOG_ERR, "%s:	Invalid opp inx: %d\n",
					__func__, dfs_node->opp_inx);
			return count;
		}
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"PI: %s (Id:%d) \t\t Client:%s \t\t DFS req:%s"\
			" request_active:%u request_weightage:%u\n",
			pi->name, dfs_node->pi_id, dfs_node->name,
			opp_str, dfs_node->req_active,
			dfs_node->weightage);
	}
	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations pi_dfs_request_list_fops = {
	.open = pi_debugfs_open,
	.read = read_get_dfs_request_list,
};

static ssize_t pi_opp_set_min_lmt(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct pi *pi = (struct pi *)file->private_data;
	int val;
	u32 len;
	int max_opp;
	char opp_str[15];
	char input_str[15];
	BUG_ON(pi == NULL);
	if (count > 15)
		len = 15;
	else
		len = count;
/* usage of sscanf and copy from user are for debugfs operations, so they will
 * not create any problems, so using coverity ignore comments */
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	sscanf(input_str, "%s", opp_str);
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	if (isdigit(opp_str[0]))
		val = (opp_str[0] - toascii('0'));
	else
		val = get_opp_from_name(opp_str);
	if (val < 0) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid	opp:%s\n",
				__func__, opp_str);
		return count;
	}
	max_opp = opp_inx_to_id(pi->pi_opp, pi->opp_lmt_max);
	if (max_opp < 0) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: invalid max_opp: %d\n",
				__func__, max_opp);
		return count;
	}
	if (unlikely(val > max_opp)) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: val: %d, min > max\n",
				__func__, val);
		return count;
	}
	pi_mgr_set_dfs_opp_limit(pi->id, val, -1);
	return count;
}

static ssize_t pi_opp_get_min_lmt(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct pi *pi = file->private_data;
	u32 len = 0;
	char *str;
	BUG_ON(pi == NULL);
	str = get_opp_name(opp_inx_to_id(pi->pi_opp,
					pi->opp_lmt_min));
	if (str == NULL) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid min lmt %d\n",
				__func__, pi->opp_lmt_min);
		return count;
	}
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"OPP limit min: %s\n", str);
	return simple_read_from_buffer(user_buf, count,
			ppos, debug_fs_buf, len);
}

static const struct file_operations pi_opp_min_lmt_fops = {
	.open = pi_debugfs_open,
	.read = pi_opp_get_min_lmt,
	.write = pi_opp_set_min_lmt,
};

static ssize_t pi_opp_set_max_lmt(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct pi *pi = (struct pi *)file->private_data;
	int val, opp_inx_req;
	u32 len;
	char opp_str[15];
	char input_str[15];
	BUG_ON(pi == NULL);
	if (count > 15)
		len = 15;
	else
		len = count;
/* usage of sscanf and copy from user are for debugfs operations, so they will
 * not create any problems, so using coverity ignore comments */
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	sscanf(input_str, "%s", opp_str);
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	if (isdigit(opp_str[0]))
		val = (opp_str[0] - toascii('0'));
	else
		val = get_opp_from_name(opp_str);
	if (val < 0) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid i/p:%s, id %d\n",
				__func__, opp_str, val);
		return count;
	}
	opp_inx_req = get_opp_inx(val, pi->pi_opp->opp_map);
	if (opp_inx_req < 0) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: Unsupp OPP:%d, inx = %d\n",
				__func__, val, opp_inx_req);
		return count;
	}

	if (opp_inx_req < pi->opp_lmt_min ||
		opp_inx_req >= pi->pi_opp->num_opp) {
		pi_dbg(pi->id, PI_LOG_ERR,
			"%s: val:%d, min > max or max > max supp opp\n",
			__func__, val);
		return count;
	}
	pi_mgr_set_dfs_opp_limit(pi->id, -1, (int)val);
	return count;
}

static int pi_opp_get_max_lmt(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	struct pi *pi = file->private_data;
	u32 len = 0;
	char *str;
	BUG_ON(pi == NULL);
	str = get_opp_name(opp_inx_to_id(pi->pi_opp,
					pi->opp_lmt_max));
	if (str == NULL) {
		pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid max lmt %d\n",
			__func__, pi->opp_lmt_max);
		return count;
	}

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"OPP limit max: %s\n", str);
	return simple_read_from_buffer(user_buf, count,
			ppos, debug_fs_buf, len);
}

static const struct file_operations pi_opp_max_lmt_fops = {
	.open = pi_debugfs_open,
	.read = pi_opp_get_max_lmt,
	.write = pi_opp_set_max_lmt
};

static ssize_t pi_dfs_set_opp_list(struct file *file, const char __user *buf,
		size_t count, loff_t *ppos)
{
	struct pi *pi = file->private_data;
	u32 len = 0;
	u32 i = 0xFF;
	u32 c = 0xFF;
	u32 f = 0xFF;
	char input_str[15];

	struct pi_opp *pi_opp;
	struct opp_info *opp_info;
	BUG_ON(!pi || !pi->pi_opp);
	pi_opp = pi->pi_opp;

	BUG_ON(pi == NULL);
	if (count > 15)
		len = 15;
	else
		len = count;
/* usage of sscanf and copy from user are for debugfs operations, so they will
 * not create any problems, so using coverity ignore comments */
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	/* coverity[tainted_data_argument] */
	sscanf(input_str, "%u%u%u", &c, &i, &f);

	if (c == 0xFF || i == 0xFF || f == 0xFF) {
		pr_info("invalid  param\n");
		goto ret;
	}
	if (c >= pi->num_ccu_id ||
		i >= pi_opp->num_opp) {
		pr_info("invalid  param\n");
		goto ret;
	}
	opp_info = &pi_opp->opp_info[c][i];
	opp_info->freq_id = f;
ret:
	return count;
}

static int pi_dfs_get_opp_list(struct file *file, char __user *buf,
		size_t count, loff_t *ppos)
{
	struct pi *pi = file->private_data;
	u32 len = 0;
	int i;
	int c;
	struct pi_opp *pi_opp;
	struct opp_info *opp_info;
	BUG_ON(!pi || !pi->pi_opp);
	pi_opp = pi->pi_opp;
	for (c = 0; c < pi->num_ccu_id; c++) {
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
		"****%s****\n",
		pi->pi_ccu[c]->name ? pi->pi_ccu[c]->name : "NULL");
		for (i = 0; i < pi_opp->num_opp; i++) {
			opp_info = &pi_opp->opp_info[c][i];
			len += snprintf(debug_fs_buf + len,
					sizeof(debug_fs_buf) - len,
					"%s:%d\n",
					get_opp_name(opp_info->opp_id),
					opp_info->freq_id);
		}
	}
	return simple_read_from_buffer(buf, count,
			ppos, debug_fs_buf, len);
}

static const struct file_operations pi_dfs_opp_list_fops = {
	.open = pi_debugfs_open,
	.read = pi_dfs_get_opp_list,
	.write = pi_dfs_set_opp_list,
};


#ifdef CONFIG_KONA_PI_DFS_STATS
static ssize_t pi_dfs_get_time_in_state(struct file *file,
					   char __user *user_buf, size_t count,
					   loff_t *ppos)
{
	struct pi *pi = NULL;
	u32 i, len = 0;
	char *str;
	pi = (struct pi *) file->private_data;
	BUG_ON(pi == NULL);
	if (pi->flags & ENABLE_DFS_STATS)
		pi_dfs_stats_update(pi);

	for (i = 0; i < pi->pi_opp->num_opp; i++) {
		str = get_opp_name(i);
		if (str == NULL) {
			pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid OPP %d\n",
					__func__, i);
			return count;
		}
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) -
		len, "%s %llu\n", str, (u64)
		cputime64_to_clock_t(pi->pi_dfs_stats.time_in_state[i]));
	}
	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations pi_dfs_time_in_state_fops = {
	.open = pi_debugfs_open,
	.read = pi_dfs_get_time_in_state,
};

static ssize_t pi_dfs_get_trans_table(struct file *file,
					   char __user *user_buf, size_t count,
					   loff_t *ppos)
{
	struct pi *pi = NULL;
	u32 i, j, len = 0;
	char *str;
	pi = (struct pi *) file->private_data;
	BUG_ON(pi == NULL);
	if (pi->flags & ENABLE_DFS_STATS)
		pi_dfs_stats_update(pi);

	len += snprintf(debug_fs_buf + len , sizeof(debug_fs_buf) - len,
					"   From  :    To\n");
	len += snprintf(debug_fs_buf + len , sizeof(debug_fs_buf) - len,
					"         : ");
	for (i = 0; i < pi->pi_opp->num_opp; i++) {
		if (len >= sizeof(debug_fs_buf))
			break;
		str = get_opp_name(i);
		if (str == NULL) {
			pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid OPP %d\n",
					__func__, i);
			return count;
		}
		len += snprintf(debug_fs_buf + len , sizeof(debug_fs_buf) - len,
			 "%10s", str);
	}
	if (len >= sizeof(debug_fs_buf))
		return sizeof(debug_fs_buf);
	len += snprintf(debug_fs_buf + len , sizeof(debug_fs_buf) - len, "\n");

	for (i = 0; i < pi->pi_opp->num_opp; i++) {
		if (len >= sizeof(debug_fs_buf))
			break;
		str = get_opp_name(i);
		if (str == NULL) {
			pi_dbg(pi->id, PI_LOG_ERR, "%s: Invalid OPP %d\n",
					__func__, i);
			return count;
		}
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
				"%10s: ", str);
		for (j = 0; j < pi->pi_opp->num_opp; j++) {
			if (len >= sizeof(debug_fs_buf))
				break;
			len += snprintf(debug_fs_buf + len,
				sizeof(debug_fs_buf) - len, "%9u ",
				pi->pi_dfs_stats.
				trans_table[i*pi->pi_opp->num_opp+j]);
		}
		if (len >= sizeof(debug_fs_buf))
			break;
		len += snprintf(debug_fs_buf + len ,
					sizeof(debug_fs_buf) - len, "\n");
	}
	if (len >= sizeof(debug_fs_buf))
		return sizeof(debug_fs_buf);

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations pi_dfs_trans_table_fops = {
	.open = pi_debugfs_open,
	.read = pi_dfs_get_trans_table,
};

static int pi_dfs_clear_stats(void *data, u64 val)
{
	int ret = 0;
	struct pi *pi = data;
	if (pi == NULL)
		return -EINVAL;

	if (val == 1)
		ret = pi_dfs_stats_clear(pi);

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_clear_stats_fops, NULL, pi_dfs_clear_stats,
			"%llu\n");

static int pi_dfs_enable_stats(void *data, u64 val)
{
	int ret = 0;
	struct pi *pi = data;
	if (pi == NULL)
		return -EINVAL;

	if (val == 0)
		ret = pi_dfs_stats_enable(pi, 0);
	else
		ret = pi_dfs_stats_enable(pi, 1);

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_enable_stats_fops, NULL, pi_dfs_enable_stats,
			"%llu\n");
#endif /* CONFIG_KONA_PI_DFS_STATS */

static int debug_chip_reset(void *data, u64 val)
{
	int ret = 0;

	ret = chip_reset();

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(chip_reset_fops, NULL, debug_chip_reset, "%llu\n");

static int pi_debug_reset(void *data, u64 val)
{
	struct pi *pi = data;

	if (pi && pi->ops && pi->ops->reset) {
		if (val <= 2)
			pi->ops->reset(pi, val);
		else {
			pi_dbg(pi->id, PI_LOG_ERR,
			       "write 0 to reset SUB_DOMAIN0\n");
			pi_dbg(pi->id, PI_LOG_ERR,
			       "write 1 to reset SUB_DOMAIN1\n");
			pi_dbg(pi->id, PI_LOG_ERR, "write 2 to reset both\n");
		}
	}
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_reset_fops, NULL, pi_debug_reset, "%llu\n");

static int pi_debug_set_enable(void *data, u64 val)
{
	struct pi *pi = data;

	if (pi && pi->ops && pi->ops->enable) {
		if (val == 1)
			pi_enable(pi, 1);
		else if (val == 0)
			pi_enable(pi, 0);
		else
			pi_dbg(pi->id, PI_LOG_ERR,
			       "write 1 to enable; 0 to disable\n");
	}

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_enable_fops, NULL, pi_debug_set_enable, "%llu\n");

static int pi_debug_pol_change_disable_set(void *data, u64 val)
{
	struct pi *pi = data;
	BUG_ON(pi == NULL);
	pi_dbg(pi->id, PI_LOG_DBGFS, "%s: val = %d\n", __func__, (u32) val);
	pi_mgr_disable_policy_change(pi->id, val != 0);
	return 0;
}

static int pi_debug_pol_change_disable_get(void *data, u64 * val)
{
	struct pi *pi = data;
	BUG_ON(pi == NULL);

	if (pi->flags & NO_POLICY_CHANGE)
		*val = 1;
	else
		*val = 0;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_pol_change_disable_fops,
			pi_debug_pol_change_disable_get,
			pi_debug_pol_change_disable_set, "%llu\n");

static int pi_debug_get_count(void *data, u64 * val)
{
	struct pi *pi = data;
	*val = pi->usg_cnt;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_count_fops, pi_debug_get_count, NULL, "%llu\n");

static ssize_t read_file_all_req(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	int i;
	u32 len = 0;
	struct pi *pi;
	struct pi_mgr_dfs_object *dfs;
	struct pi_mgr_qos_object *qos;
	struct pi_mgr_dfs_node *dfs_node;
	struct pi_mgr_qos_node *qos_node;

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"**************************All active Requests*********************\n\n");

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"************************DFS requests ************************\n");
	for (i = 0; i < PI_MGR_PI_ID_MAX; i++) {
		pi = pi_mgr.pi_list[i];
		if (pi == NULL || pi->flags & PI_NO_DFS)
			continue;
		dfs = &pi_mgr.dfs[i];
		BUG_ON(!dfs);
		if (!plist_head_empty(&dfs->requests)) {
			plist_for_each_entry(dfs_node, &dfs->requests, list) {
				len +=
				    snprintf(debug_fs_buf + len,
					     sizeof(debug_fs_buf) - len,
					     "PI: %s (Id:%d) \t\t Client: %s"\
					     "\t\t OPP_FREQ request: %u\n",
					     pi->name, dfs_node->pi_id,
					     dfs_node->name,
					     dfs_node->opp_inx);
			}
		} else
			len +=
			    snprintf(debug_fs_buf + len,
				     sizeof(debug_fs_buf) - len,
				     "No DFS requests for PI: %s \t (id: %d)\n",
				     pi->name, i);
	}

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"\n************************PI QOS requests *********************\n");
	for (i = 0; i < PI_MGR_PI_ID_MAX; i++) {
		pi = pi_mgr.pi_list[i];
		if (pi == NULL || pi->flags & PI_NO_QOS)
			continue;
		qos = &pi_mgr.qos[i];
		BUG_ON(!qos);
		if (!plist_head_empty(&qos->requests)) {
			plist_for_each_entry(qos_node, &qos->requests, list) {
				len +=
				    snprintf(debug_fs_buf + len,
					     sizeof(debug_fs_buf) - len,
					     "PI:%s (Id:%d) \t\t Client: %s \t\t Latency_request: %u\n",
					     pi->name, qos_node->pi_id,
					     qos_node->name, qos_node->latency);
			}
		} else
			len +=
			    snprintf(debug_fs_buf + len,
				     sizeof(debug_fs_buf) - len,
				     "No QOS requests for PI: %s \t (id: %d)\n",
				     pi->name, i);
	}

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations all_req_fops = {
	.open = pi_debugfs_open,
	.read = read_file_all_req,
};

static struct dentry *dent_pi_root_dir;
int __init pi_debug_init(void)
{
	struct dentry *dent_all_requests = 0, *dent_chip_reset = 0;
	dent_pi_root_dir = debugfs_create_dir("power_domains", 0);
	if (!dent_pi_root_dir)
		return -ENOMEM;
	if (!debugfs_create_u32
	    ("debug", S_IRUSR | S_IWUSR, dent_pi_root_dir, (int *)&pi_debug))
		return -ENOMEM;

	dent_all_requests =
	    debugfs_create_file("all_requests", S_IRUSR, dent_pi_root_dir, NULL,
				&all_req_fops);
	if (!dent_all_requests)
		pi_dbg(0, PI_LOG_ERR,
		       "Error registering all_requests with debugfs\n");

	dent_chip_reset = debugfs_create_file("chip_reset", S_IRUSR | S_IWUSR,
					      dent_pi_root_dir, NULL,
					      &chip_reset_fops);
	if (!dent_chip_reset)
		pi_dbg(0, PI_LOG_ERR,
		       "Error registering all_requests with debugfs\n");

	return 0;

}

int __init pi_debug_add_pi(struct pi *pi)
{
	struct dentry *dent_pi_dir = 0, *dent_count = 0, *dent_enable = 0,
	    *dent_dfs_dir = 0, *dent_dfs = 0, *dent_register_qos_client = 0,
	    *dent_remove_qos_client = 0, *dent_remove_dfs_client =
	    0, *dent_register_dfs_client = 0, *dent_request_dfs =
	    0, *dent_qos_dir = 0, *dent_qos = 0, *dent_request_qos =
	    0, *dent_state = 0, *dent_opp = 0, *dent_reset = 0, *dent_flags = 0,
#ifdef CONFIG_KONA_PI_DFS_STATS
	    *dent_dfs_stats_dir = 0,
	    *dent_time_in_state = 0, *dent_total_trans = 0,
	    *dent_trans_table = 0, *dent_clear_stats = 0, *dent_en_stats = 0,
#endif
	    *dent_pol_disable = 0;

	BUG_ON(!dent_pi_root_dir);

	pi_dbg(pi->id, PI_LOG_DBGFS, "%s: adding %s to pi debugfs\n",
	       __func__, pi->name);

	dent_pi_dir = debugfs_create_dir(pi->name, dent_pi_root_dir);
	if (!dent_pi_dir)
		goto err;

	dent_enable =
	    debugfs_create_file("enable", S_IWUSR | S_IRUSR, dent_pi_dir, pi,
				&pi_enable_fops);
	if (!dent_enable)
		goto err;

	dent_reset =
	    debugfs_create_file("reset", S_IWUSR | S_IRUSR, dent_pi_dir, pi,
				&pi_reset_fops);
	if (!dent_reset)
		goto err;

	dent_flags =
	    debugfs_create_u32("flags", S_IWUSR | S_IRUSR, dent_pi_dir,
			       &pi->flags);
	if (!dent_flags)
		goto err;

	dent_count =
	    debugfs_create_u32("count", S_IRUSR, dent_pi_dir, &pi->usg_cnt);
	if (!dent_count)
		goto err;
	dent_state =
	    debugfs_create_u32("state_allowed", S_IRUSR, dent_pi_dir,
			       (unsigned int *)&pi->state_allowed);
	if (!dent_state)
		goto err;
	dent_opp =
	    debugfs_create_u32("opp_inx_act", S_IRUSR, dent_pi_dir,
			       &pi->opp_inx_act);
	if (!dent_opp)
		goto err;

	debugfs_info[pi->id].pi_id = pi->id;
	if (!(pi->flags & PI_NO_QOS)) {
		dent_qos_dir = debugfs_create_dir("qos", dent_pi_dir);
		if (!dent_qos_dir)
			goto err;
		debugfs_info[pi->id].qos_dir = dent_qos_dir;

		dent_register_qos_client =
		    debugfs_create_file("register_client", S_IWUSR | S_IRUSR,
					dent_qos_dir, pi,
					&pi_qos_register_client_fops);
		if (!dent_register_qos_client)
			goto err;

		dent_remove_qos_client =
		    debugfs_create_file("remove_client", S_IWUSR | S_IRUSR,
					dent_qos_dir, pi,
					&pi_qos_remove_client_fops);
		if (!dent_remove_qos_client)
			goto err;

		dent_qos =
		    debugfs_create_file("qos", S_IRUSR, dent_qos_dir, pi,
					&pi_qos_fops);
		if (!dent_qos)
			goto err;

		dent_request_qos =
		    debugfs_create_file("request_list", S_IRUSR, dent_qos_dir,
					pi, &pi_qos_request_list_fops);
		if (!dent_request_qos)
			goto err;
	}

	if (((pi->flags & PI_NO_DFS) == 0) && ((pi->flags & PI_NO_QOS) == 0)) {

		dent_pol_disable =
		    debugfs_create_file("no_policy_change", S_IWUSR | S_IRUSR,
					dent_pi_dir, pi,
					&pi_pol_change_disable_fops);
		if (!dent_pol_disable)
			goto err;

	}

	if (!(pi->flags & PI_NO_DFS)) {

		dent_dfs_dir = debugfs_create_dir("dfs", dent_pi_dir);
		if (!dent_dfs_dir)
			goto err;
		debugfs_info[pi->id].dfs_dir = dent_dfs_dir;

#ifdef CONFIG_KONA_PI_DFS_STATS
		dent_dfs_stats_dir = debugfs_create_dir("stats", dent_dfs_dir);
		if (!dent_dfs_stats_dir)
			goto err;

		dent_time_in_state = debugfs_create_file("time_in_state",
		S_IRUSR, dent_dfs_stats_dir, pi, &pi_dfs_time_in_state_fops);
		if (!dent_time_in_state)
			goto err;

		dent_trans_table = debugfs_create_file("trans_table",
		S_IRUSR, dent_dfs_stats_dir, pi, &pi_dfs_trans_table_fops);
		if (!dent_trans_table)
			goto err;

		dent_total_trans = debugfs_create_u32("total_trans", S_IRUSR,
		dent_dfs_stats_dir, &pi->pi_dfs_stats.total_trans);
		if (!dent_total_trans)
			goto err;

		dent_clear_stats = debugfs_create_file("clear_stats",
		S_IRUSR, dent_dfs_stats_dir, pi, &pi_dfs_clear_stats_fops);
		if (!dent_clear_stats)
			goto err;

		dent_en_stats = debugfs_create_file("enable_stats",
			S_IRUSR | S_IWUSR, dent_dfs_stats_dir, pi,
			&pi_dfs_enable_stats_fops);
		if (!dent_en_stats)
			goto err;
#endif
		dent_dfs =
		    debugfs_create_file("dfs", S_IRUSR, dent_dfs_dir, pi,
					&pi_dfs_fops);
		if (!dent_dfs)
			goto err;

		dent_register_dfs_client =
		    debugfs_create_file("register_client", S_IWUSR | S_IRUSR,
					dent_dfs_dir, pi,
					&pi_dfs_register_client_fops);
		if (!dent_register_dfs_client)
			goto err;

		dent_remove_dfs_client =
		    debugfs_create_file("remove_client", S_IWUSR | S_IRUSR,
					dent_dfs_dir, pi,
					&pi_dfs_remove_client_fops);
		if (!dent_remove_dfs_client)
			goto err;

		dent_request_dfs =
		    debugfs_create_file("request_list", S_IRUSR, dent_dfs_dir,
					pi, &pi_dfs_request_list_fops);
		if (!dent_request_dfs)
			goto err;

		if (pi->flags & DFS_LIMIT_CHECK_EN) {

			debugfs_create_file("opp_lmt_max", S_IRUSR | S_IWUSR,
				dent_dfs_dir, pi, &pi_opp_max_lmt_fops);

			debugfs_create_file("opp_lmt_min", S_IRUSR | S_IWUSR,
				dent_dfs_dir, pi, &pi_opp_min_lmt_fops);
		}
		debugfs_create_file("opp_list", S_IWUSR | S_IRUSR,
					dent_dfs_dir, pi,
					&pi_dfs_opp_list_fops);

	}

	return 0;

err:
	debugfs_remove(dent_enable);
	debugfs_remove(dent_count);

	return -ENOMEM;
}

#endif /* CONFIG_DEBUG_FS  */
