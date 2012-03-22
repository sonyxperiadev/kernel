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
#include <linux/pm_qos_params.h>

#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/pwr_mgr.h>

#ifdef CONFIG_DEBUG_FS
#include <asm/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#ifndef DEBUGFS_PM_CLIENT_NAME
#define DEBUGFS_PM_CLIENT_NAME		"pi_client"
#endif /*DEBUGFS_PM_CLIENT_NAME*/

#ifndef PI_MGR_DEBUG_CLIENT_MAX
#define PI_MGR_DEBUG_CLIENT_MAX		10
#endif /*PI_MGR_DEBUG_CLIENT_MAX*/

#ifndef PI_MGR_DFS_WEIGHTAGE_BASE
#define PI_MGR_DFS_WEIGHTAGE_BASE 100
#endif

#define pi_is_enabled(pi) (!!(pi)->usg_cnt)

struct debug_qos_client
{
    struct pi *pi;
    char client_name[20];
    int req;
    struct dentry *dent_client;
    struct pi_mgr_qos_node debugfs_qos_node;
};

struct debug_dfs_client
{
    struct pi *pi;
    char client_name[20];
    int req;
    struct dentry *dent_client;
    struct pi_mgr_dfs_node debugfs_dfs_node;
};

struct debug_dfs_client debug_dfs_client[PI_MGR_DEBUG_CLIENT_MAX];
struct debug_qos_client debug_qos_client[PI_MGR_DEBUG_CLIENT_MAX];

struct debugfs_info
{
    u32 pi_id;
    struct dentry *qos_dir;
    struct dentry *dfs_dir;
};
struct debugfs_info debugfs_info[PI_MGR_PI_ID_MAX];

static char debug_fs_buf[3000];

#endif /*CONFIG_DEBUG_FS*/

static int pi_debug = 0;

/*PI init state*/
enum
{
	PI_INIT_NONE,
	PI_INIT_BASE,
	PI_INIT_COMPLETE
};

enum
{
	NODE_ADD,
	NODE_DELETE,
	NODE_UPDATE,
	NODE_RECALC,
};

static DEFINE_SPINLOCK(pi_mgr_lock);


struct pi_mgr_dfs_object
{
	u32 pi_id;
	u32 default_opp;
	struct atomic_notifier_head notifiers;
	struct plist_head requests;
};

struct pi_mgr_qos_object
{
	u32 pi_id;
	u32 default_latency;
	struct atomic_notifier_head notifiers;
	struct plist_head requests;
};

struct pi_mgr
{
	int pi_count;
	int init;
	struct pi* pi_list[PI_MGR_PI_ID_MAX];
	struct pi_mgr_qos_object qos[PI_MGR_PI_ID_MAX];
	struct pi_mgr_dfs_object dfs[PI_MGR_PI_ID_MAX];
	struct atomic_notifier_head pol_chg_notifier[PI_MGR_PI_ID_MAX];
};

static struct pi_mgr pi_mgr;

static int pi_change_notify(int pi_id, u32 type, u32 old_val,
						u32 new_val, u32 state)
{
	struct pi_notify_param param = {pi_id, old_val, new_val};

	switch(type)
	{
	case PI_NOTIFY_DFS_CHANGE:
		return atomic_notifier_call_chain(
						&pi_mgr.dfs[pi_id].notifiers,
					     state,
					     &param);

	case PI_NOTIFY_QOS_CHANGE:
		return atomic_notifier_call_chain(
						&pi_mgr.qos[pi_id].notifiers,
					     state,
					     &param);

	case PI_NOTIFY_POLICY_CHANGE:
		 return atomic_notifier_call_chain(
						&pi_mgr.pol_chg_notifier[pi_id],
					     state,
					     &param);
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
							1 /*save*/);

		pi->state_saved = 1;
	} else {
		BUG_ON(pi->state_saved == 0);
		for (inx = 0; inx < pi->num_ccu_id; inx++)
			ccu_save_state(to_ccu_clk(pi->pi_ccu[inx]),
							0 /*restore*/);

		pi->state_saved = 0;
	}
	pi->usg_cnt--;
	return 0;
}

int __pi_enable(struct pi *pi)
{
	int inx;
	int ret = 0;
	struct pi* dep_pi;

	pi_dbg("%s: pi_name:%s, usageCount:%d\n",__func__,pi->name,pi->usg_cnt);

	/*Enable dependent PIs, if any*/
	for(inx =0; inx < pi->num_dep_pi;inx++)
	{
		dep_pi = pi_mgr_get(pi->dep_pi[inx]);
		BUG_ON(dep_pi == NULL);
		__pi_enable(dep_pi);
	}

	/*increment usg_cnt. Return if already enabled */
	if(pi->usg_cnt++ == 0)
	{
		if(pi->init == PI_INIT_COMPLETE && pi->ops &&
			((pi->flags & NO_POLICY_CHANGE) == 0) && pi->ops->enable)
		{
			ret = pi->ops->enable(pi,1);
			/*Restore the context if state was saved*/
			if(pi->state_saved)
				pi_save_state(pi, 0/*restore*/);
		}
	}
	return ret;
}

int __pi_disable(struct pi *pi)
{
	int inx;
	int ret = 0;
	struct pi* dep_pi;

	/*decrement usg_cnt */
	if(pi->usg_cnt && --pi->usg_cnt == 0)
	{
		if(pi->init == PI_INIT_COMPLETE && pi->ops &&
			((pi->flags & NO_POLICY_CHANGE) == 0) && pi->ops->enable)
		{
			/*Save Context if state_allowed will cause the CCUs to shutdown*/
			if(pi->pi_state[pi->state_allowed].flags & PI_STATE_SAVE_CONTEXT)
				pi_save_state(pi, 1 /*save*/);

			ret = pi->ops->enable(pi,0);
		}
	}
	/*disable dependent PIs, if any, only state_allowed*/
	for(inx =0; inx < pi->num_dep_pi;inx++)
	{
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
	if(enable)
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
	struct pi* dep_pi;
	int inx;

	pi_dbg("%s:%s\n",__func__,pi->name);
	if(pi->init != PI_INIT_NONE)
		return 0;

	BUG_ON(pi->id >= PI_MGR_PI_ID_MAX);

	/*Make sure that dependent PIs are initialized, if any*/
	for(inx =0; inx < pi->num_dep_pi;inx++)
	{
		dep_pi = pi_mgr_get(pi->dep_pi[inx]);
		BUG_ON(dep_pi == NULL);
		__pi_init(dep_pi);
	}

	if(pi->ops && pi->ops->init)
		ret = pi->ops->init(pi);

	pi->init = PI_INIT_BASE;
	/*make sure that PI is at wakeup policy*/
	pwr_mgr_pi_set_wakeup_override(pi->id,false);

	pi_dbg("%s: %s %s during init\n", __func__, pi->name,
		pi->flags & PI_ENABLE_ON_INIT ? "enable" : "disable");

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


static int __pi_init_state(struct pi *pi)
{
	int ret = 0;
	int inx;
	unsigned long flgs;

	pi_dbg("%s:%s count:%d\n", __func__, pi->name, pi->usg_cnt);
	BUG_ON(pi->init == PI_INIT_NONE);

	if(pi->init == PI_INIT_BASE)
	{
		/*PI_STATE_SAVE_CONTEXT should not be defined for active state*/
		BUG_ON(pi->pi_state[PI_MGR_ACTIVE_STATE_INX].flags &
					PI_STATE_SAVE_CONTEXT);
		BUG_ON(pi->num_ccu_id > MAX_CCU_PER_PI);
		for(inx =0; inx < pi->num_ccu_id;inx++)
		{
			pi->pi_ccu[inx] = clk_get(NULL,pi->ccu_id[inx]);
			BUG_ON(pi->pi_ccu[inx] == 0 || IS_ERR(pi->pi_ccu[inx]));
		}
		spin_lock_irqsave(&pi->lock, flgs);
		if(pi->ops && pi->ops->init_state)
			ret = pi->ops->init_state(pi);

		pi->init = PI_INIT_COMPLETE;

		if(pi->num_states)
		{
			pi_dbg("%s: %s usage_cnt on init_state (late_init)\n",
				__func__, pi->name, pi->usg_cnt);

			if (pi->usg_cnt && pi->ops && pi->ops->enable)
				pi->ops->enable(pi, 1);
			else
			{
			/*Save Context if state_allowed will cause the CCUs to shutdown*/
				if (pi->pi_state[pi->state_allowed].flags &
						PI_STATE_SAVE_CONTEXT)
					pi_save_state(pi, 1 /*save*/);
				if (pi->ops && pi->ops->enable)
					pi->ops->enable(pi, 0);
			}
		}
		if((pi->flags & NO_POLICY_CHANGE) == 0)
		{
			struct pm_policy_cfg cfg;
			pwr_mgr_event_get_pi_policy(SOFTWARE_0_EVENT,pi->id,&cfg);
			pi_dbg("%s: pi-%s cnt = %d  policy =%d\n",__func__,
					pi->name,pi->usg_cnt,cfg.policy);
			pwr_mgr_pi_set_wakeup_override(pi->id,true /*clear*/);
		}
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
	u32 freq;
	BUG_ON(opp_inx >= pi->num_opp);
	for(inx =0; inx < pi->num_ccu_id;inx++)
	{
		freq = pi->pi_opp[inx].opp[opp_inx];

		pi_dbg("%s:%s clock %x policy freq => %ul\n",__func__,
				pi->pi_ccu[inx]->name,policy,freq);

		if((res = ccu_set_freq_policy(to_ccu_clk(pi->pi_ccu[inx]),CCU_POLICY(policy),freq)) != 0)
		{
			pi_dbg("%s:ccu_set_freq_policy failed\n",__func__);

		}
	}
	return res;
}

#endif

int pi_set_policy(const struct pi *pi, u32 policy,int type)
{
	struct pm_policy_cfg cfg;
	int res ;
	u32 old_pol;

	switch(type)
	{
	case POLICY_QOS:
		res = pwr_mgr_event_get_pi_policy(pi->qos_sw_event_id, pi->id, &cfg);
		if(res != 0 || policy == cfg.policy)
			break;
		old_pol = cfg.policy;
		cfg.policy = policy;

		pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol,policy, PI_PRECHANGE);
		res =  pwr_mgr_event_set_pi_policy(pi->qos_sw_event_id,pi->id, &cfg);
		if(!res)
		{
			pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol, policy, PI_POSTCHANGE);
		}

		break;

#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
	case POLICY_DFS:

		res = pwr_mgr_event_get_pi_policy(pi->dfs_sw_event_id, pi->id, &cfg);
		if(res != 0 || policy == cfg.policy)
			break;
		old_pol = cfg.policy;
		cfg.policy = policy;

		pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol, policy, PI_PRECHANGE);

		res =  pwr_mgr_event_set_pi_policy(pi->dfs_sw_event_id,pi->id,&cfg);
		if(!res)
		{
			pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol, policy, PI_POSTCHANGE);
		}
		break;

	case POLICY_BOTH:
		res = pwr_mgr_event_get_pi_policy(pi->qos_sw_event_id, pi->id, &cfg);
		if(res != 0 || policy == cfg.policy)
			break;
		old_pol = cfg.policy;
		cfg.policy = policy;

		pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol, policy, PI_PRECHANGE);

		res =  pwr_mgr_event_set_pi_policy(pi->qos_sw_event_id,pi->id,&cfg);
		if(!res)
		{
			pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol, policy, PI_POSTCHANGE);
		}
		if(res == 0 && pi->dfs_sw_event_id != pi->qos_sw_event_id)
		{
			res = pwr_mgr_event_get_pi_policy(pi->dfs_sw_event_id, pi->id, &cfg);
			if(res != 0 || policy == cfg.policy)
				break;
			old_pol = cfg.policy;
			cfg.policy = policy;

			pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol, policy, PI_PRECHANGE);

			res =  pwr_mgr_event_set_pi_policy(pi->dfs_sw_event_id,pi->id,&cfg);
			if(!res)
			{
				pi_change_notify(pi->id,PI_NOTIFY_POLICY_CHANGE,
					old_pol, policy, PI_POSTCHANGE);
			}
		}
		break;
#endif /*CONFIG_CHANGE_POLICY_FOR_DFS*/
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
					pi->pi_opp[0].opp[pi->opp_active];
	pi_set_policy(pi, pi->pi_opp[0].opp[pi->opp_active],POLICY_DFS);
#else
	pi_set_ccu_freq(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
				pi->opp_active);
#endif
	return 0;
}

static int pi_def_init(struct pi *pi)
{
	struct pi_mgr_qos_object* qos;
	struct pi_mgr_dfs_object* dfs;

	pi_dbg("%s:%s\n",__func__,pi->name);

	if ((pi->flags & PI_NO_QOS) == 0) {
		qos = &pi_mgr.qos[pi->id];
		qos->pi_id = pi->id;
		ATOMIC_INIT_NOTIFIER_HEAD(&qos->notifiers);
		plist_head_init(&qos->requests);
		BUG_ON(pi->num_states > PI_MGR_MAX_STATE_ALLOWED);

		qos->default_latency =
			pi->pi_state[pi->num_states-1].hw_wakeup_latency;
		pi->state_allowed = pi->num_states-1;
		pi_dbg("qos->default_latency = %d state_allowed = %d\n",
				qos->default_latency, pi->state_allowed);
		if (pi->flags & UPDATE_PM_QOS)	{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
			pi->pm_qos =
			pm_qos_add_request(PM_QOS_CPU_DMA_LATENCY,
						PM_QOS_DEFAULT_VALUE);
#else
			pm_qos_add_request(&pi->pm_qos, PM_QOS_CPU_DMA_LATENCY,
						PM_QOS_DEFAULT_VALUE);
#endif
		}
	}

	if((pi->flags & PI_NO_DFS) == 0)
	{
		pi_dbg( "pi->opp_active = %d\n",pi->opp_active);
		dfs = &pi_mgr.dfs[pi->id];
		ATOMIC_INIT_NOTIFIER_HEAD(&dfs->notifiers);
		plist_head_init(&dfs->requests);
		dfs->pi_id = pi->id;
		dfs->default_opp = 0;
		BUG_ON(pi->num_opp && pi->pi_opp == NULL);
		if (pi->flags & DFS_LIMIT_CHECK_EN)
			BUG_ON(pi->num_opp <= pi->opp_lmt_max ||
				pi->opp_lmt_max < pi->opp_lmt_min);

	}

	/*Init PI Policy change notifier*/
	ATOMIC_INIT_NOTIFIER_HEAD(&pi_mgr.pol_chg_notifier[pi->id]);

	printk(KERN_INFO " %s: count = %d\n",__func__,pi->usg_cnt);
	return 0;
}

static int pi_def_enable(struct pi *pi, int enable)
{
	u32 policy;
	pi_dbg("%s: pi_name:%s, enable:%d usageCount:%d\n",__func__,pi->name,enable,pi->usg_cnt);
	if(enable)
	{
		policy = pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy;
		pi_dbg("%s: policy = %d -- PI to be enabled\n",__func__,policy);
	}
	else
	{
		policy = pi->pi_state[pi->state_allowed].state_policy;
		pi_dbg("%s: policy = %d pi->state_allowed = %d\n",__func__,policy,pi->state_allowed);

	}
	pi_dbg("%s: calling pi_set_policy\n",__func__);
	return pi_set_policy(pi,policy,POLICY_QOS);
}

static int pi_reset(struct pi *pi, int sub_domain)
{
	u32 reg_val;
	struct clk* clk;
	struct ccu_clk *ccu_clk;
	unsigned long flgs;

	pi_dbg("%s: pi_name:%s, usageCount:%d\n", __func__,
					pi->name, pi->usg_cnt);
	if (pi->pi_info.reset_mgr_ccu_name == NULL ||
					!pi->pi_info.pd_soft_reset_offset)
		return -EPERM;
	if ((sub_domain == SUB_DOMAIN_0 && !pi->pi_info.pd_reset_mask0) ||
	    (sub_domain == SUB_DOMAIN_1 && !pi->pi_info.pd_reset_mask1) ||
	    (sub_domain == SUB_DOMAIN_BOTH && (!pi->pi_info.pd_reset_mask0 ||
			!pi->pi_info.pd_reset_mask1)))
		return -EPERM;

	spin_lock_irqsave(&pi->lock, flgs);
	pi_dbg("%s:pi:%s reset ccu str:%s\n", __func__, pi->name,
				pi->pi_info.reset_mgr_ccu_name);
	clk = clk_get(NULL, pi->pi_info.reset_mgr_ccu_name);
	BUG_ON(clk == 0 || IS_ERR(clk));

	ccu_clk = to_ccu_clk(clk);

	ccu_reset_write_access_enable(ccu_clk, true);
	reg_val = readl(ccu_clk->ccu_reset_mgr_base +
					pi->pi_info.pd_soft_reset_offset);
	pi_dbg("reset offset: %08x, reg_val: %08x\n",
		(ccu_clk->ccu_reset_mgr_base +
				pi->pi_info.pd_soft_reset_offset), reg_val);
	switch(sub_domain) {
	case SUB_DOMAIN_0:
	    reg_val = reg_val & ~pi->pi_info.pd_reset_mask0;
	    break;
	case SUB_DOMAIN_1:
	    reg_val = reg_val & ~pi->pi_info.pd_reset_mask1;
	    break;
	case SUB_DOMAIN_BOTH:
	    reg_val = reg_val & ~pi->pi_info.pd_reset_mask0;
	    reg_val = reg_val & ~pi->pi_info.pd_reset_mask1;
	    break;
	default:
		spin_unlock_irqrestore(&pi->lock, flgs);
		return -EINVAL;
	}
	pi_dbg("writing reset value: %08x\n", reg_val);
	writel(reg_val, ccu_clk->ccu_reset_mgr_base +
					pi->pi_info.pd_soft_reset_offset);
	udelay(10);

	switch(sub_domain) {
	case SUB_DOMAIN_0:
	    reg_val = reg_val | pi->pi_info.pd_reset_mask0;
	    break;
	case SUB_DOMAIN_1:
	    reg_val = reg_val | pi->pi_info.pd_reset_mask1;
	    break;
	case SUB_DOMAIN_BOTH:
	    reg_val = reg_val | pi->pi_info.pd_reset_mask0;
	    reg_val = reg_val | pi->pi_info.pd_reset_mask1;
	    break;
	default:
		spin_unlock_irqrestore(&pi->lock, flgs);
		return -EINVAL;
	}
	pi_dbg("writing reset release value: %08x\n", reg_val);
	writel(reg_val, ccu_clk->ccu_reset_mgr_base + pi->pi_info.pd_soft_reset_offset);

	ccu_reset_write_access_enable(ccu_clk , false);
	spin_unlock_irqrestore(&pi->lock, flgs);
	return 0;
}

struct pi_ops gen_pi_ops = {
	.init = pi_def_init,
	.init_state = pi_def_init_state,
	.enable = pi_def_enable,
	.reset = pi_reset,
};


static u32 pi_mgr_dfs_get_opp(const struct pi_mgr_dfs_object* dfs)
{
	u32 opp = dfs->default_opp;
	int i;
	int sum[PI_OPP_MAX - 1] = {0};
	struct pi_mgr_dfs_node *dfs_node;
	struct pi *pi = pi_mgr.pi_list[dfs->pi_id];

	if(!plist_head_empty(&dfs->requests))
	{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
		opp =  list_entry(dfs->requests.node_list.prev,
			  struct plist_node, plist.node_list)->prio;
#else
		opp = plist_last(&dfs->requests)->prio;
#endif
		if(opp == (pi->num_opp - 1)) /*Highest OPP ??*/
			return opp;

		plist_for_each_entry(dfs_node, &dfs->requests, list)
		{
			if(dfs_node->req_active && dfs_node->opp < (pi->num_opp - 1))
			{
				sum[dfs_node->opp] +=  dfs_node->weightage;
			}
		}

		for(i = opp ; i < pi->num_opp - 1; i++)
		{
			if(sum[i]/PI_MGR_DFS_WEIGHTAGE_BASE)
				opp++;
		}

		if(opp >= pi->num_opp)
			opp = pi->num_opp - 1;
		pi_dbg("%s:pi :%s opp = %d\n",__func__,pi->name,opp);
	}

	return opp;
}

static u32 check_dfs_limit(struct pi *pi, u32 opp)
{
	u32 act_opp = opp;

	if (pi->flags & DFS_LIMIT_CHECK_EN) {

		BUG_ON(pi->opp_lmt_min > pi->opp_lmt_max);
		if (opp < pi->opp_lmt_min)
			act_opp = pi->opp_lmt_min;
		else if (opp > pi->opp_lmt_max)
			act_opp = pi->opp_lmt_max;
	}
	return act_opp;
}

static u32 pi_mgr_dfs_update(struct pi_mgr_dfs_node *node,
						u32 pi_id, int action)
{
	u32 old_val, new_val;
	struct pi_mgr_dfs_object* dfs = &pi_mgr.dfs[pi_id];
	struct pi *pi = pi_mgr.pi_list[pi_id];
	unsigned long flgs;

	spin_lock_irqsave(&pi->lock, flgs);

	old_val = pi->opp_active;
	switch(action)
	{
	case NODE_ADD:
		pi_dbg("%s:NODE_ADD -> opp req = %d\n",__func__,node->opp);
		plist_node_init(&node->list,node->opp);
		plist_add(&node->list, &dfs->requests);
		break;
	case NODE_DELETE:
		pi_dbg("%s:NODE_DELETE -> opp req = %d\n",__func__,node->opp);
		plist_del(&node->list, &dfs->requests);
		break;
	case NODE_UPDATE:
		pi_dbg("%s:NODE_UPDATE -> opp req = %d\n",__func__,node->opp);
		plist_del(&node->list, &dfs->requests);
		plist_node_init(&node->list,node->opp);
		plist_add(&node->list, &dfs->requests);
		break;

	case NODE_RECALC:
		pi_dbg("%s:NODE_RECALC\n", __func__);
		break;

	default:
		BUG();
		break;
	}
	new_val = check_dfs_limit(pi, pi_mgr_dfs_get_opp(dfs));
	pi_dbg("%s:pi_id= %d oldval = %d new val = %d\n",__func__,pi_id,old_val,new_val);

	if(old_val != new_val)
	{
		BUG_ON(new_val >= pi->num_opp);
		pi->opp_active = new_val;
		if(pi->init == PI_INIT_COMPLETE && ((pi->flags & NO_POLICY_CHANGE) == 0))
		{
			pi_change_notify(pi->id,PI_NOTIFY_DFS_CHANGE,
					old_val, new_val, PI_PRECHANGE);
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
			pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy = pi->pi_opp[0].opp[new_val];
			if(pi_is_enabled(pi))
				pi_set_policy(pi, pi->pi_opp[0].opp[new_val],POLICY_QOS);
			if(pi->dfs_sw_event_id != pi->qos_sw_event_id)
				pi_set_policy(pi, pi->pi_opp[0].opp[new_val],POLICY_DFS);
#else
			pi_set_ccu_freq(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
						new_val);
#endif
			pi_change_notify(pi->id,PI_NOTIFY_DFS_CHANGE,
					old_val, new_val, PI_POSTCHANGE);
		}
	}
	spin_unlock_irqrestore(&pi->lock, flgs);

	return new_val;
}

static u32 pi_mgr_qos_get_value(const struct pi_mgr_qos_object* qos)
{
	if (plist_head_empty(&qos->requests))
		return qos->default_latency;
	return plist_first(&qos->requests)->prio;
}

static u32 pi_mgr_qos_update(struct pi_mgr_qos_node* node, u32 pi_id, int action)
{
	u32 old_val, new_val;
	u32 old_state;
	int i;
	int found = 0;
	unsigned long flgs;
	struct pi* dep_pi;
	struct pi_mgr_qos_object* qos = &pi_mgr.qos[pi_id];
	struct pi *pi = pi_mgr.pi_list[pi_id];

	spin_lock_irqsave(&pi->lock, flgs);
	old_val = pi_mgr_qos_get_value(qos);

	switch(action)
	{
	case NODE_ADD:
		pi_dbg("%s:NODE_ADD -> lat req = %d\n",__func__,node->latency);
		plist_node_init(&node->list,node->latency);
		plist_add(&node->list, &qos->requests);
		break;
	case NODE_DELETE:
		pi_dbg("%s:NODE_DELETE -> lat req = %d\n",__func__,node->latency);
		plist_del(&node->list, &qos->requests);
		break;
	case NODE_UPDATE:
		pi_dbg("%s:NODE_UPDATE -> lat req = %d\n",__func__,node->latency);
		plist_del(&node->list, &qos->requests);
		plist_node_init(&node->list,node->latency);
		plist_add(&node->list, &qos->requests);
		break;
	default:
		BUG();
		break;
	}
	new_val = pi_mgr_qos_get_value(qos);
	pi_dbg("%s:pi_id= %d oldval = %d new val = %d\n",__func__,pi_id,old_val,new_val);

	if(old_val != new_val)
	{
		old_state = pi->state_allowed;

		pi_change_notify(pi->id,PI_NOTIFY_QOS_CHANGE,
					old_val, new_val, PI_PRECHANGE);


		if(new_val <= pi->pi_state[0].hw_wakeup_latency)
			pi->state_allowed = 0;
		else
		{
			for(i = 1; i < PI_MGR_MAX_STATE_ALLOWED &&
				i < pi->num_states; i++)
			{
				if(new_val >= pi->pi_state[i-1].hw_wakeup_latency &&
					new_val < pi->pi_state[i].hw_wakeup_latency)
				{
					pi->state_allowed = i-1;
					found = 1;
					break;
				}
			}
			BUG_ON(i == PI_MGR_MAX_STATE_ALLOWED);
			if(!found)
				pi->state_allowed = i-1;
		}
		/*Disabling LPM through QoS ?*/
		if(!IS_ACTIVE_POLICY(pi->pi_state[old_state].state_policy) &&
			IS_ACTIVE_POLICY(pi->pi_state[pi->state_allowed].state_policy))
		{
				/*Enable dependent PIs, if any*/
			for(i =0; i < pi->num_dep_pi;i++)
			{
				dep_pi = pi_mgr_get(pi->dep_pi[i]);
				BUG_ON(dep_pi == NULL);
				__pi_enable(dep_pi);
			}

		}
		/*re-enabling LPM PI through QoS ?*/
		else if(IS_ACTIVE_POLICY(pi->pi_state[old_state].state_policy) &&
			!IS_ACTIVE_POLICY(pi->pi_state[pi->state_allowed].state_policy))
		{
				/*disable dependent PIs, if any*/
			for(i =0; i < pi->num_dep_pi;i++)
			{
				dep_pi = pi_mgr_get(pi->dep_pi[i]);
				BUG_ON(dep_pi == NULL);
				__pi_disable(dep_pi);
			}

		}

		if(pi->init == PI_INIT_COMPLETE)
		{
			if(!pi_is_enabled(pi) && pi->state_allowed != old_state)
			{
				/*Do pi_enable and pi_disable call to switch to the right state
				Also needed to handle state save/restore properly
				*/
				__pi_enable(pi);
				__pi_disable(pi);
			}

		}

		if(pi->flags & UPDATE_PM_QOS)
		{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
			BUG_ON(pi->pm_qos == NULL);
			pm_qos_update_request(pi->pm_qos, new_val);
#else
			pm_qos_update_request(&pi->pm_qos, new_val);
#endif

		}
		pi_change_notify(pi->id,PI_NOTIFY_QOS_CHANGE,
					old_val, new_val, PI_POSTCHANGE);

	}
	pi_dbg("%s:%s state allowed = %d\n",__func__,pi->name,pi->state_allowed);
	spin_unlock_irqrestore(&pi->lock, flgs);
	return new_val;
}
int pi_state_allowed(int pi_id)
{
	int ret = -EINVAL;
	struct pi* pi = pi_mgr_get(pi_id);
	if(pi)
		ret = pi->state_allowed;
	return ret;
}
EXPORT_SYMBOL(pi_state_allowed);

u32 pi_get_active_qos(int pi_id)
{
	struct pi_mgr_qos_object* qos = &pi_mgr.qos[pi_id];
	if(qos)
		return pi_mgr_qos_get_value(qos);
	else
		pi_dbg("%s:invalid param \n",__func__);
	return 0;
}
EXPORT_SYMBOL(pi_get_active_qos);

u32 pi_get_active_opp(int pi_id)
{
	int ret = -EINVAL;
	struct pi* pi = pi_mgr_get(pi_id);
	/*Before PI init is complete, PI policy is set to wake up ploicy, 7.
	Policy 7 freq is initialized to turbo mode freq.
	Hence, this function should return the max opp number if the init is
	not complete.*/
	if (pi) {
		if (pi->init == PI_INIT_COMPLETE)
			ret = pi->opp_active;
		else
			return pi->num_opp-1;
	}
	return ret;
}
EXPORT_SYMBOL(pi_get_active_opp);




int pi_mgr_register(struct pi* pi)
{
	unsigned long flgs;

	pi_dbg("%s:name:%s id:%d\n",__func__,pi->name,pi->id);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EPERM;
	}
	if(pi->id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi->id])
	{
		pi_dbg("%s:pi already registered or invalid id \n",__func__);
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
	pi_dbg("%s:client = %s,pi_id = %d, lat_val = %d\n", __func__,
		client_name, pi_id, lat_value);
	if(unlikely(!pi_mgr.init))
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n", __func__);
		return -EPERM;
	}
	if(node == NULL)
	{
	    pi_dbg("%s:ERROR Invalid node\n", __func__);
	    return -EINVAL;
	}
	if(node->valid)
	{
	    pi_dbg("%s:ERROR node already added\n", __func__);
	    return -EINVAL;
	}
	if(unlikely(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL))
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}

	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS))
	{
		pi_dbg("%s:ERROR - QOS not supported for this PI\n",__func__);
		return -EPERM;
	}
	if(lat_value == PI_MGR_QOS_DEFAULT_VALUE)
	{
		lat_value = pi_mgr.qos[pi_id].default_latency;
		pi_dbg("%s:lat_value = PI_MGR_QOS_DEFAULT_VALUE, def_at = %d\n",__func__,lat_value);
	}

	node->name = client_name;
	node->latency = lat_value;
	node->pi_id = pi_id;
	pi_mgr_qos_update(node, pi_id, NODE_ADD);
	node->valid = 1;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_add_request);

int pi_mgr_qos_request_update(struct pi_mgr_qos_node* node, u32 lat_value)
{
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	BUG_ON(node->valid == 0);
	pi_dbg("%s: exisiting req = %d new_req = %d\n",__func__,node->latency,lat_value);
	if(lat_value == PI_MGR_QOS_DEFAULT_VALUE)
		lat_value = pi_mgr.qos[node->pi_id].default_latency;

	if(node->latency != lat_value)
	{
		node->latency = lat_value;
		pi_mgr_qos_update(node, node->pi_id, NODE_UPDATE);
	}

	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_request_update);

int pi_mgr_qos_request_remove(struct pi_mgr_qos_node* node)
{
	pi_dbg("%s:name = %s, req = %d\n",__func__,node->name,node->latency);
	BUG_ON(node->valid == 0);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	pi_mgr_qos_update(node,node->pi_id,NODE_DELETE);
	node->name = NULL;
	node->valid = 0;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_request_remove);

int pi_mgr_dfs_add_request(struct pi_mgr_dfs_node *node, char* client_name, u32 pi_id, u32 opp)
{
	return pi_mgr_dfs_add_request_ex(node, client_name, pi_id, opp, PI_MGR_DFS_WIEGHTAGE_DEFAULT);
}
EXPORT_SYMBOL(pi_mgr_dfs_add_request);

int pi_mgr_dfs_request_update(struct pi_mgr_dfs_node* node, u32 opp)
{
	return pi_mgr_dfs_request_update_ex(node,opp,PI_MGR_DFS_WIEGHTAGE_DEFAULT);
}
EXPORT_SYMBOL(pi_mgr_dfs_request_update);

int pi_mgr_dfs_add_request_ex(struct pi_mgr_dfs_node *node, char* client_name, u32 pi_id, u32 opp, u32 weightage)
{
	struct pi* pi;
	pi_dbg("%s:client = %s pi = %d opp = %d\n",__func__,client_name, pi_id, opp);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EPERM;
	}
	if(node == NULL)
	{
	    pi_dbg("%s:ERROR Invalid node\n",__func__);
	    return -EINVAL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || (pi = pi_mgr.pi_list[pi_id]) == NULL)
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}
	if(unlikely(pi->flags & PI_NO_DFS))
	{
		pi_dbg("%s:ERROR - DFS not supported for this PI\n",__func__);
		return -EPERM;
	}

	if(opp >= pi->num_opp && opp != PI_MGR_DFS_MIN_VALUE)
	{
		__WARN();
		pi_dbg("%s:ERROR - %d:unsupported opp \n",__func__,opp);
		return -EINVAL;
	}

	if(PI_MGR_DFS_WIEGHTAGE_DEFAULT != weightage && weightage >= PI_MGR_DFS_WEIGHTAGE_BASE)
	{
		__WARN();
		pi_dbg("%s:ERROR - %d:unsupported weightage \n",__func__,weightage);
		return -EINVAL;
	}

	node->name = client_name;
	node->pi_id = pi_id;

	if(opp == PI_MGR_DFS_MIN_VALUE)
	{
		node->req_active = 0;
		node->opp = 0;
	}
	else
	{
		node->opp = opp;
		node->req_active = 1;
	}
	if(weightage == PI_MGR_DFS_WIEGHTAGE_DEFAULT)
		node->weightage = pi->opp_def_weightage[node->opp];
	else
		node->weightage = weightage;

	BUG_ON(node->weightage >= PI_MGR_DFS_WEIGHTAGE_BASE);

	pi_mgr_dfs_update(node,pi_id,NODE_ADD);
	node->valid = 1;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_dfs_add_request_ex);

int pi_mgr_dfs_request_update_ex(struct pi_mgr_dfs_node* node, u32 opp, u32 weightage)
{
	struct pi* pi =  pi_mgr.pi_list[node->pi_id];
	BUG_ON(pi==NULL);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	BUG_ON(node->valid == 0);
	if(opp >= pi->num_opp && opp != PI_MGR_DFS_MIN_VALUE)
	{
		__WARN();
		pi_dbg("%s:ERROR - %d:unsupported opp \n",__func__,opp);
		return -EINVAL;
	}

	if(PI_MGR_DFS_WIEGHTAGE_DEFAULT != weightage && weightage >= PI_MGR_DFS_WEIGHTAGE_BASE)
	{
		__WARN();
		pi_dbg("%s:ERROR - %d:unsupported weightage \n",__func__,weightage);
		return -EINVAL;
	}

	pi_dbg("%s:client = %s pi = %d opp = %d opp_new = %d weightage = %d\n",__func__,
			node->name, node->pi_id, opp,node->opp,weightage);

	if(node->opp != opp || weightage != node->weightage)
	{
		if(opp == PI_MGR_DFS_MIN_VALUE)
		{
			node->req_active = 0;
			node->opp = 0;
		}
		else
		{
			node->opp = opp;
			node->req_active = 1;
		}
		if(weightage == PI_MGR_DFS_WIEGHTAGE_DEFAULT)
			node->weightage = pi->opp_def_weightage[node->opp];
		else
			node->weightage = weightage;

		BUG_ON(node->weightage >= PI_MGR_DFS_WEIGHTAGE_BASE);

		pi_mgr_dfs_update(node,node->pi_id,NODE_UPDATE);
	}

	return 0;
}
EXPORT_SYMBOL(pi_mgr_dfs_request_update_ex);

int pi_mgr_dfs_request_remove(struct pi_mgr_dfs_node* node)
{
	pi_dbg("%s:name = %s, req = %d\n",__func__,node->name,node->opp);
	BUG_ON(node->valid == 0);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
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
	bool update = false;
	BUG_ON(pi == NULL);
	if ((pi->flags & DFS_LIMIT_CHECK_EN) == 0)
		return -EINVAL;
	if (min >= 0 && (u32)min != pi->opp_lmt_min) {
		pi->opp_lmt_min = (u32)min;
		update = true;
	}

	if (max >= 0 && (u32)max != pi->opp_lmt_max) {
		if ((u32)max >= pi->num_opp)
			return -EINVAL;
		pi->opp_lmt_max = (u32)max;
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
	struct pi* pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);

	if(disable && ((pi->flags & NO_POLICY_CHANGE) == 0))
	{
		pr_info("%s : dis pol change\n",__func__);
		pi->flags |= NO_POLICY_CHANGE;
		pwr_mgr_pi_set_wakeup_override(pi->id,false);
	}
	else if(!disable && (pi->flags & NO_POLICY_CHANGE))
	{
		pr_info("%s : enable pol change\n",__func__);
		pi->flags &= ~NO_POLICY_CHANGE;

		/*Update PI DFS freq based on opp_active value*/
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
		pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy =
					pi->pi_opp[0].opp[pi->opp_active];
		pi_set_policy(pi, pi->pi_opp[0].opp[pi->opp_active],POLICY_DFS);
#else
		pi_set_ccu_freq(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
				pi->opp_active);
#endif

		if(pi->usg_cnt)
			 pi->ops->enable(pi,1);
		else
		{
		/*Save Context if state_allowed will cause the CCUs to shutdown*/
			if(pi->pi_state[pi->state_allowed].flags & PI_STATE_SAVE_CONTEXT)
				pi_save_state(pi, 1 /*save*/);
			 pi->ops->enable(pi,0);
		}

		pwr_mgr_pi_set_wakeup_override(pi->id,true);
	}
	return 	0;
}
EXPORT_SYMBOL(pi_mgr_disable_policy_change);

static int pi_mgr_dfs_add_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS))	{
		pi_dbg("%s:ERROR - DFS not supported for this PI\n", __func__);
		return -EINVAL;;
	}

	return atomic_notifier_chain_register(
			&pi_mgr.dfs[pi_id].notifiers, notifier);
}

static int pi_mgr_dfs_remove_notifier(u32 pi_id,
					struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS))	{
		pi_dbg("%s:ERROR - DFS not supported for this PI\n", __func__);
		return -EINVAL;;
	}


	return atomic_notifier_chain_unregister(
			&pi_mgr.dfs[pi_id].notifiers, notifier);

}

static int pi_mgr_qos_add_notifier(u32 pi_id,
					struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS))	{
		pi_dbg("%s:ERROR - QOS not supported for this PI\n", __func__);
		return -EINVAL;;
	}

	return atomic_notifier_chain_register(
			&pi_mgr.qos[pi_id].notifiers, notifier);
}

static int pi_mgr_qos_remove_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if (unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS))	{
		pi_dbg("%s:ERROR - QOS not supported for this PI\n", __func__);
		return -EINVAL;;
	}

	return atomic_notifier_chain_unregister(
			&pi_mgr.qos[pi_id].notifiers, notifier);

}

int pi_mgr_register_notifier(u32 pi_id, struct notifier_block *notifier, u32 type)
{
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}
	switch(type)
	{
	case PI_NOTIFY_DFS_CHANGE:
		return pi_mgr_dfs_add_notifier(pi_id, notifier);

	case PI_NOTIFY_QOS_CHANGE:
		return pi_mgr_qos_add_notifier(pi_id, notifier);

	case PI_NOTIFY_POLICY_CHANGE:
		return atomic_notifier_chain_register(
			&pi_mgr.pol_chg_notifier[pi_id], notifier);
	}
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(pi_mgr_register_notifier);

int pi_mgr_unregister_notifier(u32 pi_id, struct notifier_block *notifier, u32 type)
{
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}
	switch(type)
	{
	case PI_NOTIFY_DFS_CHANGE:
		return pi_mgr_dfs_remove_notifier(pi_id, notifier);

	case PI_NOTIFY_QOS_CHANGE:
		return pi_mgr_qos_remove_notifier(pi_id, notifier);

	case PI_NOTIFY_POLICY_CHANGE:
		return atomic_notifier_chain_unregister(
			&pi_mgr.pol_chg_notifier[pi_id], notifier);
	}
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(pi_mgr_unregister_notifier);

struct pi* pi_mgr_get(int pi_id)
{
	pi_dbg("%s: id:%d\n",__func__,pi_id);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return NULL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pi_dbg("%s:invalid pi_id id \n",__func__);
		return NULL;
	}
	return pi_mgr.pi_list[pi_id];
}
EXPORT_SYMBOL(pi_mgr_get);

int pi_mgr_init()
{
	memset(&pi_mgr,0,sizeof(pi_mgr));
	pi_mgr.init = 1;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_init);

__weak int chip_reset(void)
{
    return 0;
}

#ifdef CONFIG_DEBUG_FS

static int pi_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int pi_debug_set_qos(void *data, u64 val)
{
	u32 inx = (u32)data;
	int ret;
	BUG_ON(debug_qos_client[inx].pi == NULL);
	ret = pi_mgr_qos_request_update(&debug_qos_client[inx].debugfs_qos_node
			, val);
	if(ret == 0)
		debug_qos_client[inx].req = (int)val;
	return ret;
}
static int pi_debug_get_qos(void *data, u64* val)
{
	u32 inx = (u32)data;
	BUG_ON(debug_qos_client[inx].pi == NULL);
	*val = debug_qos_client[inx].req;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_qos_client_fops, pi_debug_get_qos, pi_debug_set_qos, "%llu\n");

static int pi_debug_get_active_qos(void *data, u64 *val)
{
	struct pi *pi = data;
	*val = pi_get_active_qos(pi->id);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_qos_fops, pi_debug_get_active_qos, NULL, "%llu\n");

static int pi_debug_register_qos_client(void *data, u64 value)
{
    struct pi *pi = data;
	u32 val = (u32)value;
    struct dentry *qos_dir=0, *dent_client=0;
    int ret = 0;

    if(val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

    pi_dbg("%s: client_name  %s_%d \n", __func__, DEBUGFS_PM_CLIENT_NAME, val);

    if(debug_qos_client[val].pi)
	{
		pi_dbg("%s: client in use by %s\n",__func__,pi->name);
		return 0;
	}

    qos_dir = debugfs_info[pi->id].qos_dir;
    sprintf(debug_qos_client[val].client_name, "%s_%d", DEBUGFS_PM_CLIENT_NAME, val);

    dent_client = debugfs_create_file(debug_qos_client[val].client_name, S_IWUSR|S_IRUSR, qos_dir,
						(void*)val, &pi_qos_client_fops);
    if(!dent_client)
		goto err;

    ret =
	pi_mgr_qos_add_request(&debug_qos_client[val].debugfs_qos_node, debug_qos_client[val].client_name, pi->id, PI_MGR_QOS_DEFAULT_VALUE);
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
DEFINE_SIMPLE_ATTRIBUTE(pi_qos_register_client_fops, NULL, pi_debug_register_qos_client, "%llu\n");

static int pi_debug_remove_qos_client(void *data, u64 value)
{
    struct pi *pi = data;
    int ret;
	u32 val = (u32)value;

    if(val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

    pi_dbg("%s: client_name %s_%d \n", __func__, DEBUGFS_PM_CLIENT_NAME, val);

    if(debug_qos_client[val].pi &&
		debug_qos_client[val].pi->id == pi->id)
	{
		BUG_ON(!debug_qos_client[val].pi ||
				!debug_qos_client[val].dent_client);
	    ret = pi_mgr_qos_request_remove(&debug_qos_client[val].debugfs_qos_node);
	    if(ret)
			pi_dbg("Failed to remove node\n");
	    debugfs_remove(debug_qos_client[val].dent_client);

	    debug_qos_client[val].pi = NULL;
	    debug_qos_client[val].client_name[0] = '\0';
	    debug_qos_client[val].dent_client = NULL;
	    pi_dbg("This client registration removed for this PI\n");
	}
	else
		pi_dbg("%s: client not registered\n",__func__);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(pi_qos_remove_client_fops, NULL, pi_debug_remove_qos_client, "%llu\n");

static ssize_t read_get_qos_request_list(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	u32 len = 0;
    struct pi *pi = (struct pi *)file->private_data;
    struct pi_mgr_qos_object* qos = &pi_mgr.qos[pi->id];
    struct pi_mgr_qos_node *qos_node;

    len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
	"*************************** PI qos requests **************************** \n");
    plist_for_each_entry(qos_node, &qos->requests, list)
	{
		len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
		"PI: %s (Id:%d) \t\t Client: %s \t\t Latency_request: %u \n",
			pi->name, qos_node->pi_id, qos_node->name, qos_node->latency);
    }
	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf, len);
}
static struct file_operations pi_qos_request_list_fops =
{
	.open =         pi_debugfs_open,
	.read =         read_get_qos_request_list,
};

static ssize_t pi_debug_set_dfs_client_opp(struct file *file, char const __user *buf,
					size_t count, loff_t *offset)
{
	u32 len = 0;
	u32 opp = PI_MGR_DFS_MIN_VALUE;
	u32 weightage = PI_MGR_DFS_WIEGHTAGE_DEFAULT;
	char input_str[100];
    u32 inx = (u32)file->private_data;
	if (count > 100)
		len = 100;
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
    sscanf(input_str, "%u%u", &opp, &weightage);
	pi_mgr_dfs_request_update_ex(&debug_dfs_client[inx].debugfs_dfs_node,opp,weightage);
	return count;
}

static ssize_t pi_debug_get_dfs_client_opp(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	u32 len = 0;
    u32 inx = (u32)file->private_data;

    len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
		"DFS Req:%u Request active:%u Request Weightage:%u \n",
			debug_dfs_client[inx].debugfs_dfs_node.opp,
			debug_dfs_client[inx].debugfs_dfs_node.req_active,
			debug_dfs_client[inx].debugfs_dfs_node.weightage);
    return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf, len);
}

static struct file_operations pi_dfs_client_fops =
{
	.open = pi_debugfs_open,
	.read = pi_debug_get_dfs_client_opp,
	.write = pi_debug_set_dfs_client_opp,
};

static int pi_debug_get_active_dfs(void *data, u64 *val)
{

    struct pi *pi = data;

    *val = pi_get_active_opp(pi->id);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_fops, pi_debug_get_active_dfs, NULL, "%llu\n");

static int pi_debug_register_dfs_client(void *data, u64 value)
{
    struct pi *pi = data;
    int ret = -1;
	u32 val = (u32)value;
    struct dentry *dfs_dir=0, *dent_client=0;

    if(val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

    pi_dbg("%s: client_name  %s_%d \n", __func__, DEBUGFS_PM_CLIENT_NAME, val);

    if(debug_dfs_client[val].pi)
	{
		pi_dbg("%s: client in use by %s\n",__func__,pi->name);
		return 0;
	}

    dfs_dir = debugfs_info[pi->id].dfs_dir;
    sprintf(debug_dfs_client[val].client_name, "%s_%d", DEBUGFS_PM_CLIENT_NAME, val);

    ret = pi_mgr_dfs_add_request(&debug_dfs_client[val].debugfs_dfs_node, debug_dfs_client[val].client_name, pi->id, PI_MGR_DFS_MIN_VALUE);
    if (ret)
	pi_dbg("%s: DFS add request failed for %s\n",__func__,pi->name);

    dent_client = debugfs_create_file(debug_dfs_client[val].client_name, S_IWUSR|S_IRUSR, dfs_dir,
	    (void*)val, &pi_dfs_client_fops);
    if(!dent_client)
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
DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_register_client_fops, NULL, pi_debug_register_dfs_client, "%llu\n");

static int pi_debug_remove_dfs_client(void *data, u64 value)
{
    struct pi *pi = data;
    int val = (int)value;
	int ret;

    if(val < 0 || val >= PI_MGR_DEBUG_CLIENT_MAX)
		return -EINVAL;

    pi_dbg("%s: client_name %s_%d \n", __func__, DEBUGFS_PM_CLIENT_NAME, val);

    if(debug_dfs_client[val].pi &&
		debug_dfs_client[val].pi->id == pi->id)
	{
		BUG_ON(!debug_dfs_client[val].dent_client );
	    ret = pi_mgr_dfs_request_remove(&debug_dfs_client[val].debugfs_dfs_node);
	    if(ret)
			pi_dbg("Failed to remove node\n");
	    debugfs_remove(debug_dfs_client[val].dent_client);

	    debug_dfs_client[val].pi = NULL;
	    debug_dfs_client[val].client_name[0] = '\0';
	    debug_dfs_client[val].dent_client = NULL;
	    debug_dfs_client[val].debugfs_dfs_node.name = NULL;
	    pi_dbg("This client registration removed for this PI\n");
	}
	else
		pi_dbg("%s: client not registered\n",__func__);

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_remove_client_fops, NULL, pi_debug_remove_dfs_client, "%llu\n");


static ssize_t read_get_dfs_request_list(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	u32 len = 0;
    struct pi *pi = (struct pi *)file->private_data;
    struct pi_mgr_dfs_object* dfs = &pi_mgr.dfs[pi->id];
    struct pi_mgr_dfs_node *dfs_node;

    len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
	"*************************** PI DFS requests **************************** \n");
    plist_for_each_entry(dfs_node, &dfs->requests, list)
	{
		len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
			"PI: %s (Id:%d) \t\t Client:%s \t\t DFS request:%u request_active:%u request_weightage: %u\n", pi->name, dfs_node->pi_id, dfs_node->name, dfs_node->opp,dfs_node->req_active,
					dfs_node->weightage);
    }
	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf, len);
}

static const struct file_operations pi_dfs_request_list_fops = {
	.open =         pi_debugfs_open,
	.read =         read_get_dfs_request_list,
};

static int pi_opp_set_min_lmt(void *data, u64 val)
{
	struct pi *pi = data;
	BUG_ON(pi == NULL);

	if (unlikely(val > pi->opp_lmt_max)) {
		pr_info("%s: min > max\n", __func__);
		return 0;
	}
	return pi_mgr_set_dfs_opp_limit(pi->id, (int)val, -1);
}

static int pi_opp_get_min_lmt(void *data, u64 *val)
{
	struct pi *pi = data;
	BUG_ON(pi == NULL);
	*val = pi->opp_lmt_min;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_opp_min_lmt_fops, pi_opp_get_min_lmt,
					pi_opp_set_min_lmt, "%llu\n");

static int pi_opp_set_max_lmt(void *data, u64 val)
{
	struct pi *pi = data;
	BUG_ON(pi == NULL);

	if (unlikely(val < pi->opp_lmt_min ||
			val >= pi->num_opp)) {
		pr_info("%s: min > max or max > max supported opp\n",
				__func__);
		return 0;
	}
	return pi_mgr_set_dfs_opp_limit(pi->id, -1, (int)val);
}

static int pi_opp_get_max_lmt(void *data, u64 *val)
{
	struct pi *pi = data;
	BUG_ON(pi == NULL);
	*val = pi->opp_lmt_max;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_opp_max_lmt_fops, pi_opp_get_max_lmt,
					pi_opp_set_max_lmt, "%llu\n");

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

    if(pi && pi->ops && pi->ops->reset) {
	if(val >= 0 && val <= 2)
	    pi->ops->reset(pi, val);
	else
	    pi_dbg("write 0 to reset SUB_DOMAIN0, 1 to reset SUB_DOMAIN1 and 2 to reset both \n");
    }
    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_reset_fops, NULL, pi_debug_reset, "%llu\n");

static int pi_debug_set_enable(void *data, u64 val)
{
    struct pi *pi = data;

    if(pi && pi->ops && pi->ops->enable)
	{
		if(val == 1)
			pi_enable(pi, 1);
		else if(val == 0)
			pi_enable(pi, 0);
		else
			pi_dbg("write 1 to enable; 0 to disable\n");
    }

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_enable_fops, NULL, pi_debug_set_enable, "%llu\n");

static int pi_debug_pol_change_disable_set(void *data, u64 val)
{
    struct pi *pi = data;
    BUG_ON(pi == NULL);
	pr_info("%s: val = %d\n",__func__,(u32)val);
	pi_mgr_disable_policy_change(pi->id, val != 0);
    return 0;
}

static int pi_debug_pol_change_disable_get(void *data, u64 *val)
{
    struct pi *pi = data;
    BUG_ON(pi == NULL);

	if(pi->flags & NO_POLICY_CHANGE)
		*val = 1;
	else
		*val = 0;

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_pol_change_disable_fops, pi_debug_pol_change_disable_get,
			pi_debug_pol_change_disable_set, "%llu\n");


static int pi_debug_get_count(void *data, u64 *val)
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
    struct pi_mgr_dfs_object* dfs;
    struct pi_mgr_qos_object* qos;
    struct pi_mgr_dfs_node *dfs_node;
    struct pi_mgr_qos_node *qos_node;

    len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
				"**************************All active Requests*********************\n\n");

	len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
			"************************DFS requests ************************\n");
    for(i=0;i<PI_MGR_PI_ID_MAX;i++)
	{
		pi = pi_mgr.pi_list[i];
		if(pi == NULL || pi->flags & PI_NO_DFS)
			continue;
		dfs = &pi_mgr.dfs[i];
		BUG_ON(!dfs);
		if(!plist_head_empty(&dfs->requests))
		{
			plist_for_each_entry(dfs_node, &dfs->requests, list)
			{
				len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
				"PI: %s (Id:%d) \t\t Client: %s \t\t OPP_FREQ request: %u\n",
				pi->name, dfs_node->pi_id, dfs_node->name, dfs_node->opp);
			}
	    }
		else
			len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
				"No DFS requests for PI: %s \t (id: %d)\n", pi->name, i);
    }

    len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
		"\n************************PI QOS requests *********************\n");
    for(i=0;i<PI_MGR_PI_ID_MAX;i++)
	{
		pi = pi_mgr.pi_list[i];
		if(pi == NULL || pi->flags & PI_NO_QOS)
			continue;
		qos = &pi_mgr.qos[i];
		BUG_ON(!qos);
		if(!plist_head_empty(&qos->requests))
		{
			plist_for_each_entry(qos_node, &qos->requests, list)
			{
				len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
				"PI:%s (Id:%d) \t\t Client: %s \t\t Latency_request: %u\n",
				pi->name, qos_node->pi_id, qos_node->name, qos_node->latency);
			}
		}
		else
			len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
				"No QOS requests for PI: %s \t (id: %d)\n", pi->name, i);
    }

    return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf, len);
}
static struct file_operations all_req_fops =
{
	.open =         pi_debugfs_open,
	.read =         read_file_all_req,
};

static ssize_t read_file_pi_count(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	int i, counter = 0;
	u32 len = 0;
	struct pi *pi;
	bool overflow;
	ktime_t _time;
	s64 ms;

	_time = ktime_get();
	ms = ktime_to_ms(_time);
	for (i = 0; i < PI_MGR_PI_ID_MAX; i++) {
		pi = pi_mgr.pi_list[i];
		if (pi == NULL)
			continue;

		counter = pwr_mgr_pi_counter_read(pi->id, &overflow);
		if (counter < 0)
			return -ENOMEM;
		len += snprintf(debug_fs_buf+len, sizeof(debug_fs_buf)-len,
				"%8s(%1d): counter:0x%08X, overflow:%d, "
				"systime:%16ld mS\n",
				pi->name, pi->id, counter, overflow,
				(long int)ms);
	}

	return simple_read_from_buffer(user_buf, count,
				ppos, debug_fs_buf, len);
}
static const struct file_operations pi_debug_count_fops = {
	.open =         pi_debugfs_open,
	.read =         read_file_pi_count,
};
static int pi_debug_count_clear(void *data, u64 val)
{
	if (val == 1) {
		pm_mgr_pi_count_clear(true);
		pm_mgr_pi_count_clear(false);
	} else
		pr_info("Invalid parm\n");
	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(pi_debug_count_clr_fops, NULL, pi_debug_count_clear,
"%llu\n");

static struct dentry *dent_pi_root_dir;
int __init pi_debug_init(void)
{
    struct dentry *dent_all_requests = 0, *dent_chip_reset = 0;
	struct dentry *dent_pi_count = 0, *dent_pi_count_clr = 0;
    dent_pi_root_dir = debugfs_create_dir("power_domains", 0);
    if(!dent_pi_root_dir)
		return -ENOMEM;
    if(!debugfs_create_u32("debug", S_IRUSR|S_IWUSR, dent_pi_root_dir, (int *)&pi_debug))
		return -ENOMEM;

    dent_all_requests = debugfs_create_file("all_requests", S_IRUSR, dent_pi_root_dir, NULL, &all_req_fops);
    if(!dent_all_requests)
		pi_dbg("Erro registering all_requests with debugfs\n");

	dent_chip_reset = debugfs_create_file("chip_reset", S_IRUSR | S_IWUSR,
			dent_pi_root_dir, NULL, &chip_reset_fops);
    if(!dent_chip_reset)
		pi_dbg("Erro registering all_requests with debugfs\n");

	dent_pi_count = debugfs_create_file("pi_count", S_IRUSR,
				dent_pi_root_dir, NULL, &pi_debug_count_fops);
	if (!dent_pi_count)
		pi_dbg("Error registering pi_count with debugfs\n");

	dent_pi_count_clr = debugfs_create_file("pi_count_clear",
	S_IRUSR|S_IWUSR, dent_pi_root_dir, NULL, &pi_debug_count_clr_fops);
	if (!dent_pi_count_clr)
		pi_dbg("Error registering pi_count_clear with debugfs\n");

    return 0;

}

int __init pi_debug_add_pi(struct pi *pi)
{
    struct dentry *dent_pi_dir=0, *dent_count=0, *dent_enable=0,
    *dent_dfs_dir=0, *dent_dfs=0, *dent_register_qos_client=0,
    *dent_remove_qos_client=0, *dent_remove_dfs_client=0, *dent_register_dfs_client=0,
    *dent_request_dfs=0, *dent_qos_dir=0, *dent_qos=0, *dent_request_qos=0,
    *dent_state=0, *dent_opp=0, *dent_reset=0, *dent_flags=0, *dent_pol_disable;


    BUG_ON(!dent_pi_root_dir);

    pi_dbg("%s: adding %s to pi debugfs \n", __func__, pi->name);

    dent_pi_dir = debugfs_create_dir(pi->name, dent_pi_root_dir);
    if(!dent_pi_dir)
	goto err;

    dent_enable = debugfs_create_file("enable", S_IWUSR|S_IRUSR, dent_pi_dir, pi, &pi_enable_fops);
	if(!dent_enable)
	    goto err;

    dent_reset = debugfs_create_file("reset", S_IWUSR|S_IRUSR, dent_pi_dir, pi, &pi_reset_fops);
	if(!dent_reset)
	    goto err;

    dent_flags = debugfs_create_u32("flags", S_IWUSR|S_IRUSR, dent_pi_dir, &pi->flags);
	if(!dent_flags)
	    goto err;

    dent_count = debugfs_create_u32("count", S_IRUSR, dent_pi_dir, &pi->usg_cnt);
    if(!dent_count)
		goto err;
	dent_state = debugfs_create_u32("state_allowed", S_IRUSR, dent_pi_dir, (unsigned int *)&pi->state_allowed);
	if(!dent_state)
		goto err;
    dent_opp = debugfs_create_u32("opp_active", S_IRUSR, dent_pi_dir, &pi->opp_active);
	if(!dent_opp)
		goto err;

    debugfs_info[pi->id].pi_id = pi->id;
    if (!(pi->flags & PI_NO_QOS)) {
	dent_qos_dir = debugfs_create_dir("qos", dent_pi_dir);
	if(!dent_qos_dir)
	    goto err;
	debugfs_info[pi->id].qos_dir = dent_qos_dir;

	dent_register_qos_client = debugfs_create_file("register_client", S_IWUSR|S_IRUSR, dent_qos_dir, pi, &pi_qos_register_client_fops);
	if(!dent_register_qos_client)
	    goto err;

	dent_remove_qos_client = debugfs_create_file("remove_client", S_IWUSR|S_IRUSR, dent_qos_dir, pi,
				&pi_qos_remove_client_fops);
	if(!dent_remove_qos_client)
	    goto err;

	dent_qos = debugfs_create_file("qos", S_IRUSR, dent_qos_dir, pi, &pi_qos_fops);
	if(!dent_qos)
	    goto err;

	dent_request_qos = debugfs_create_file("request_list", S_IRUSR, dent_qos_dir,
				pi, &pi_qos_request_list_fops);
	if(!dent_request_qos)
	    goto err;
    }

	if(((pi->flags & PI_NO_DFS) == 0) &&
		((pi->flags & PI_NO_QOS) == 0)) {

	dent_pol_disable = debugfs_create_file("no_policy_change", S_IWUSR|S_IRUSR,
			dent_pi_dir, pi, &pi_pol_change_disable_fops);
	if(!dent_pol_disable)
		goto err;

	}

    if (!(pi->flags & PI_NO_DFS)) {


	dent_dfs_dir = debugfs_create_dir("dfs", dent_pi_dir);
	if(!dent_dfs_dir)
	    goto err;
	debugfs_info[pi->id].dfs_dir = dent_dfs_dir;

	dent_dfs = debugfs_create_file("dfs", S_IRUSR, dent_dfs_dir, pi, &pi_dfs_fops);
	if(!dent_dfs)
	    goto err;

	dent_register_dfs_client = debugfs_create_file("register_client", S_IWUSR|S_IRUSR,
			dent_dfs_dir, pi, &pi_dfs_register_client_fops);
	if(!dent_register_dfs_client)
	    goto err;

	dent_remove_dfs_client = debugfs_create_file("remove_client", S_IWUSR|S_IRUSR, dent_dfs_dir, pi,
				&pi_dfs_remove_client_fops);
	if(!dent_remove_dfs_client)
	    goto err;

	dent_request_dfs = debugfs_create_file("request_list", S_IRUSR, dent_dfs_dir, pi, &pi_dfs_request_list_fops);
	if(!dent_request_dfs)
	    goto err;

	if (pi->flags & DFS_LIMIT_CHECK_EN) {

		debugfs_create_file("opp_lmt_max", S_IRUSR, dent_dfs_dir, pi,
			&pi_opp_max_lmt_fops);

		debugfs_create_file("opp_lmt_min", S_IRUSR, dent_dfs_dir, pi,
				&pi_opp_min_lmt_fops);
	}
    }

    return 0;

err:
    debugfs_remove(dent_enable);
    debugfs_remove(dent_count);

    return -ENOMEM;
}

#endif /* CONFIG_DEBUG_FS  */

