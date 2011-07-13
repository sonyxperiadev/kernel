
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/err.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/plist.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/pm_qos_params.h>

#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/pwr_mgr.h>


enum
{
	NODE_ADD,
	NODE_DELETE,
	NODE_UPDATE
};

static DEFINE_SPINLOCK(pi_mgr_lock);
static DEFINE_SPINLOCK(pi_mgr_list_lock);

struct pi_mgr_qos_node
{
	char* name;
	struct plist_node list;
	u32 latency;
	u32 pi_id;
};

struct pi_mgr_dfs_node
{
	char* name;
	struct plist_node list;
	u32 opp;
	u32 pi_id;
};

struct pi_mgr_dfs_object
{
	u32 pi_id;
	u32 default_opp;
	struct blocking_notifier_head notifiers;
	struct plist_head requests;
};

struct pi_mgr_qos_object
{
	u32 pi_id;
	u32 default_latency;
	struct blocking_notifier_head notifiers;
	struct plist_head requests;
};

struct pi_mgr
{
	int pi_count;
	int init;
	struct pi* pi_list[PI_MGR_PI_ID_MAX];
	struct pi_mgr_qos_object qos[PI_MGR_PI_ID_MAX];
	struct pi_mgr_dfs_object dfs[PI_MGR_PI_ID_MAX];
};

static struct pi_mgr pi_mgr;

#ifndef CONFIG_CHANGE_POLICY_FOR_DFS
static int pi_set_ccu_freq(struct pi *pi, u32 policy, u16 freq)
{
	struct clk* clk;
	int inx;
	int res = 0;

	for(inx =0; pi->ccu_id[inx];inx++)
	{
		pr_info("%s:pi:%s clock str:%s\n",__func__,pi->name,
				pi->ccu_id[inx]);

		clk = clk_get(NULL,pi->ccu_id[inx]);
		BUG_ON(clk == 0 || IS_ERR(clk));
		pr_info("%s:%s clock %x policy freq => %d\n",__func__,
				clk->name,policy,freq);

		if((res = ccu_set_freq_policy(to_ccu_clk(clk),CCU_POLICY(policy),freq)) != 0)
		{
			pr_info("%s:ccu_set_freq_policy failed\n",__func__);

		}
	}
	return res;
}

#endif

static int pi_set_policy(struct pi *pi, u32 policy)
{
	struct pm_policy_cfg cfg;
	int res;
	/*TBD - ac & atl should not be hardcoded -- NEED tO REVISIT*/
	cfg.ac = 1;
	cfg.atl  = 0;
	cfg.policy = policy;
	pr_info("%s:%s: %d event policy => %x \n",
				__func__, pi->name, pi->sw_event_id,cfg.policy);
	res =  pwr_mgr_event_set_pi_policy(pi->sw_event_id,pi->id,&cfg);

	if(!res)
	{
		pi_change_notify(pi,cfg.policy);
	}
	return res;
}

static int pi_def_init(struct pi *pi)
{
	struct pi_mgr_qos_object* qos;
	struct pi_mgr_dfs_object* dfs;
	int inx;
	struct clk* clk;

	pr_info("%s:%s\n",__func__,pi->name);
	if(pi->init)
		return 0;
	spin_lock(&pi_mgr_lock);
	pi->init = 1;

	/* Make sure that CCUs are initialized*/
	for(inx =0; pi->ccu_id[inx];inx++)
	{
		clk = clk_get(NULL,pi->ccu_id[inx]);
		BUG_ON(clk == 0 || IS_ERR(clk));
		clk_init(clk);
	}

	if((pi->flags & PI_NO_QOS) == 0)
	{
		qos = &pi_mgr.qos[pi->id];
		BLOCKING_INIT_NOTIFIER_HEAD(&qos->notifiers);
		plist_head_init(&qos->requests,&pi_mgr_list_lock);
		for(inx = 0; inx < PI_MGR_MAX_STATE_ALLOWED &&
				pi->pi_state[inx].id != PI_MGR_STATE_UNSUPPORTED; inx++);
		BUG_ON(inx == PI_MGR_MAX_STATE_ALLOWED);
		qos->default_latency = pi->pi_state[inx].hw_wakeup_latency;
	}

	if((pi->flags & PI_NO_DFS) == 0)
	{
		dfs = &pi_mgr.dfs[pi->id];
		BLOCKING_INIT_NOTIFIER_HEAD(&dfs->notifiers);
		plist_head_init(&dfs->requests,&pi_mgr_list_lock);

		BUG_ON(pi->opp_active >= PI_OPP_MAX);
		dfs->default_opp = pi->opp_active;

#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
		pi_set_policy(pi, pi->opp[pi->opp_active]);
		pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy = pi->opp[pi->opp_active];
#else
		pi_set_ccu_freq(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
						pi->opp[pi->opp_active]);
#endif
	}

	if(pi->pi_state[PI_MGR_ACTIVE_STATE_INX].id != PI_MGR_STATE_UNSUPPORTED)
	{
		if(pi->flags & PI_DISABLE_ON_INIT)
		{
			pr_info("%s: calling pi_set_policy-- policy = %d\n",__func__,pi->pi_state[pi->state_allowed].state_policy);
			pi_set_policy(pi,pi->pi_state[pi->state_allowed].state_policy);
		}
		else
		{
			pr_info("%s: calling pi_set_policy-- policy = %d\n",__func__,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy);
			pi_set_policy(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy);
		}
		pwr_mgr_pi_set_wakeup_override(pi->id,true/*clear*/);
	}
	spin_unlock(&pi_mgr_lock);
	return 0;
}

static int pi_def_enable(struct pi *pi, int enable)
{
	u32 policy;

	pr_info("%s: enable:%d usageCount:%d\n",__func__,enable,pi->usg_cnt);
	spin_lock(&pi_mgr_lock);
	if(enable)
	{
		if(pi->usg_cnt++ != 0)
			goto done;
		policy = pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy;
	}
	else
	{
		if(pi->usg_cnt && --pi->usg_cnt != 0)
			goto done;
		policy = pi->pi_state[pi->state_allowed].state_policy;
	}
	pr_info("%s: calling pi_set_policy-- policy = %d pi->state_allowed = %d\n",__func__,policy,pi->state_allowed);
	pi_set_policy(pi,policy);
done:
	spin_unlock(&pi_mgr_lock);
	return 0;
}

struct pi_ops gen_pi_ops = {
	.init = pi_def_init,
	.enable = pi_def_enable,
	.change_notify = NULL,
};


static u32 pi_mgr_dfs_get_opp(const struct pi_mgr_dfs_object* dfs)
{
	if (plist_head_empty(&dfs->requests))
		return dfs->default_opp;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
	return list_entry(dfs->requests.node_list.prev,
			  struct plist_node, plist.node_list)->prio;
#else
	return plist_last(&dfs->requests)->prio;
#endif

}

static u32 pi_mgr_dfs_update(struct pi_mgr_dfs_node* node, u32 pi_id, int action)
{
	u32 old_val, new_val;
	struct pi_mgr_dfs_object* dfs = &pi_mgr.dfs[pi_id];
	struct pi *pi = pi_mgr.pi_list[pi_id];

	spin_lock(&pi_mgr_lock);
	old_val = pi_mgr_dfs_get_opp(dfs);
	switch(action)
	{
	case NODE_ADD:
		plist_node_init(&node->list,node->opp);
		plist_add(&node->list, &dfs->requests);
		break;
	case NODE_DELETE:
		plist_del(&node->list, &dfs->requests);
		break;
	case NODE_UPDATE:
		plist_del(&node->list, &dfs->requests);
		plist_node_init(&node->list,node->opp);
		plist_add(&node->list, &dfs->requests);
		break;
	default:
		BUG();
		break;
	}
	new_val = pi_mgr_dfs_get_opp(dfs);
	spin_unlock(&pi_mgr_lock);

	if(old_val != new_val)
	{
		blocking_notifier_call_chain(&dfs->notifiers,
					     (unsigned long)new_val,
					     NULL);

		BUG_ON(new_val >= PI_OPP_MAX);
		pi->opp_active = new_val;
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
		pi_set_policy(pi, pi->opp[new_val]);
		pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy = pi->opp[new_val];
#else
		pi_set_ccu_freq(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
					pi->opp[new_val]);
#endif
	}

	return new_val;
}

static u32 pi_mgr_qos_get_value(const struct pi_mgr_qos_object* qos)
{
	if (plist_head_empty(&qos->requests))
		return qos->default_latency;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
	return list_entry(qos->requests.node_list.prev,
			  struct plist_node, plist.node_list)->prio;
#else
	return plist_last(&qos->requests)->prio;
#endif

}

static u32 pi_mgr_qos_update(struct pi_mgr_qos_node* node, u32 pi_id, int action)
{
	u32 old_val, new_val;
	int i;
	int found = 0;
	struct pi_mgr_qos_object* qos = &pi_mgr.qos[pi_id];
	struct pi *pi = pi_mgr.pi_list[pi_id];

	spin_lock(&pi_mgr_lock);
	old_val = pi_mgr_qos_get_value(qos);
	switch(action)
	{
	case NODE_ADD:
		plist_node_init(&node->list,node->latency);
		plist_add(&node->list, &qos->requests);
		break;
	case NODE_DELETE:
		plist_del(&node->list, &qos->requests);
		break;
	case NODE_UPDATE:
		plist_del(&node->list, &qos->requests);
		plist_node_init(&node->list,node->latency);
		plist_add(&node->list, &qos->requests);
		break;
	default:
		BUG();
		break;
	}
	new_val = pi_mgr_qos_get_value(qos);
	spin_unlock(&pi_mgr_lock);

	if(old_val != new_val)
	{
		blocking_notifier_call_chain(&qos->notifiers,
					     (unsigned long)new_val,
					     NULL);

		if(new_val <= pi->pi_state[0].hw_wakeup_latency)
			pi->state_allowed = 0;
		else
		{
			for(i = 1; i < PI_MGR_MAX_STATE_ALLOWED &&
				pi->pi_state[i].id != PI_MGR_STATE_UNSUPPORTED; i++)
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
		/*TODO - Notify pm_qos for ARM core CCU*/
	}
	pr_info("%s:%s state allowed = %d\n",__func__,pi->name,pi->state_allowed);

	return new_val;
}

int pi_mgr_register(struct pi* pi)
{
	pr_info("%s:name:%s id:%d\n",__func__,pi->name,pi->id);
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EPERM;
	}
	if(pi->id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi->id])
	{
		pr_info("%s:pi already registered or invalid id \n",__func__);
		return -EPERM;
	}
	spin_lock(&pi_mgr_lock);
	pi_mgr.pi_list[pi->id] = pi;
	pi_mgr.pi_count++;

	spin_unlock(&pi_mgr_lock);

	return 0;
}
EXPORT_SYMBOL(pi_mgr_register);

struct pi_mgr_qos_node* pi_mgr_qos_add_request(char* client_name, u32 pi_id, u32 lat_value)
{
	struct pi_mgr_qos_node* node;
	if(unlikely(!pi_mgr.init))
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return NULL;
	}
	if(unlikely(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL))
	{
		pr_info("%s:ERROR - invalid pid\n",__func__);
		return NULL;
	}

	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS))
	{
		pr_info("%s:ERROR - QOS not supported for this PI\n",__func__);
		return NULL;
	}

	node = kzalloc(sizeof(struct pi_mgr_qos_node), GFP_KERNEL);
	if(!node)
	{
		pr_info("%s:ERROR - kzalloc failed\n",__func__);
		return NULL;
	}
	if(lat_value == PI_MGR_QOS_DEFAULT_VALUE)
		lat_value = pi_mgr.qos[pi_id].default_latency;

	node->name = client_name;
	node->latency = lat_value;
	node->pi_id = pi_id;
	pi_mgr_qos_update(node,pi_id,NODE_ADD);
	return node;
}
EXPORT_SYMBOL(pi_mgr_qos_add_request);

int pi_mgr_qos_request_update(struct pi_mgr_qos_node* node, u32 lat_value)
{
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(lat_value == PI_MGR_QOS_DEFAULT_VALUE)
		lat_value = pi_mgr.qos[node->pi_id].default_latency;

	if(node->latency != lat_value)
	{
		node->latency = lat_value;
		pi_mgr_qos_update(node,node->pi_id,NODE_UPDATE);
	}

	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_request_update);

int pi_mgr_qos_request_remove(struct pi_mgr_qos_node* node)
{
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	pi_mgr_qos_update(node,node->pi_id,NODE_DELETE);
	kfree(node);
	return 0;
}
EXPORT_SYMBOL(pi_mgr_qos_request_remove);

int pi_mgr_qos_add_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if(unlikely(!pi_mgr.init))
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(unlikely(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL))
	{
		pr_info("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}
	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS))
	{
		pr_info("%s:ERROR - QOS not supported for this PI\n",__func__);
		return -EINVAL;;
	}


	return blocking_notifier_chain_register(
			&pi_mgr.qos[pi_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(pi_mgr_qos_add_notifier);

int pi_mgr_qos_remove_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if(unlikely(!pi_mgr.init))
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(unlikely(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL))
	{
		pr_info("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}
	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS))
	{
		pr_info("%s:ERROR - QOS not supported for this PI\n",__func__);
		return -EINVAL;;
	}

	return blocking_notifier_chain_unregister(
			&pi_mgr.qos[pi_id].notifiers, notifier);

}
EXPORT_SYMBOL_GPL(pi_mgr_qos_remove_notifier);


struct pi_mgr_dfs_node* pi_mgr_dfs_add_request(char* client_name, u32 pi_id, u32 opp)
{
	struct pi_mgr_dfs_node* node;
	struct pi* pi;
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return NULL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || (pi = pi_mgr.pi_list[pi_id]) == NULL)
	{
		pr_info("%s:ERROR - invalid pid\n",__func__);
		return NULL;
	}
	if(unlikely(pi->flags & PI_NO_DFS))
	{
		pr_info("%s:ERROR - DFS not supported for this PI\n",__func__);
		return NULL;
	}

	if(opp >= PI_OPP_MAX || pi->opp[opp] == PI_OPP_UNSUPPORTED)
	{
		pr_info("%s:ERROR - %d:unsupported opp \n",__func__,opp);
		return NULL;
	}

	node = kzalloc(sizeof(struct pi_mgr_dfs_node), GFP_KERNEL);
	if(!node)
	{
		pr_info("%s:ERROR - kzalloc failed\n",__func__);
		return NULL;
	}

	node->name = client_name;
	node->opp = opp;
	node->pi_id = pi_id;
	pi_mgr_dfs_update(node,pi_id,NODE_ADD);
	return node;
}
EXPORT_SYMBOL(pi_mgr_dfs_add_request);

int pi_mgr_dfs_request_update(struct pi_mgr_dfs_node* node, u32 opp)
{
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(opp >= PI_OPP_MAX || pi_mgr.pi_list[node->pi_id]->opp[opp] == PI_OPP_UNSUPPORTED)
	{
		pr_info("%s:ERROR - %d:unsupported opp \n",__func__,opp);
		return -EINVAL;
	}

	if(node->opp != opp)
	{
		node->opp = opp;
		pi_mgr_dfs_update(node,node->pi_id,NODE_UPDATE);
	}

	return 0;
}
EXPORT_SYMBOL(pi_mgr_dfs_request_update);

int pi_mgr_dfs_request_remove(struct pi_mgr_dfs_node* node)
{
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	pi_mgr_dfs_update(node,node->pi_id,NODE_DELETE);
	kfree(node);
	return 0;
}
EXPORT_SYMBOL(pi_mgr_dfs_request_remove);

int pi_mgr_dfs_add_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pr_info("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}

	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS))
	{
		pr_info("%s:ERROR - DFS not supported for this PI\n",__func__);
		return -EINVAL;;
	}

	return blocking_notifier_chain_register(
			&pi_mgr.dfs[pi_id].notifiers, notifier);
}
EXPORT_SYMBOL_GPL(pi_mgr_dfs_add_notifier);


int pi_mgr_dfs_remove_notifier(u32 pi_id, struct notifier_block *notifier)
{
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pr_info("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}
	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS))
	{
		pr_info("%s:ERROR - DFS not supported for this PI\n",__func__);
		return -EINVAL;;
	}


	return blocking_notifier_chain_unregister(
			&pi_mgr.dfs[pi_id].notifiers, notifier);

}
EXPORT_SYMBOL_GPL(pi_mgr_dfs_remove_notifier);



struct pi* pi_mgr_get(int pi_id)
{
	pr_info("%s: id:%d\n",__func__,pi_id);
	if(!pi_mgr.init)
	{
		pr_info("%s:ERROR - pi mgr not initialized\n",__func__);
		return NULL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pr_info("%s:invalid pi_id id \n",__func__);
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

