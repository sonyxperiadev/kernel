
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/err.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/plist.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/pm_qos_params.h>

#include <plat/clock.h>
#include <plat/pi_mgr.h>
#include <plat/pwr_mgr.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#ifndef DEBUGFS_PM_CLIENT_NAME
#define DEBUGFS_PM_CLIENT_NAME		"pi_client"
#endif /*DEBUGFS_PM_CLIENT_NAME*/

#ifndef PI_MGR_DEBUG_CLIENT_MAX
#define PI_MGR_DEBUG_CLIENT_MAX		10
#endif /*PI_MGR_DEBUG_CLIENT_MAX*/

struct debug_qos_client
{
    struct pi *pi;
    char client_name[20];
    int req;
    struct dentry *dent_client;
    struct pi_mgr_qos_node* debugfs_qos_node;
};

struct debug_dfs_client
{
    struct pi *pi;
    char client_name[20];
    int req;
    struct dentry *dent_client;
    struct pi_mgr_dfs_node* debugfs_dfs_node;
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



enum
{
	NODE_ADD,
	NODE_DELETE,
	NODE_UPDATE
};

static DEFINE_SPINLOCK(pi_mgr_lock);

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
static int pi_set_ccu_freq(struct pi *pi, u32 policy, u32 opp_inx)
{
	struct clk* clk;
	int inx;
	int res = 0;
	u32 freq;

	BUG_ON(opp_inx >= pi->num_opp);

	for(inx =0; inx < pi->num_ccu_id;inx++)
	{
		pi_dbg("%s:pi:%s clock str:%s opp_inx = %d\n",__func__,pi->name,
				pi->ccu_id[inx],opp_inx);

		clk = clk_get(NULL,pi->ccu_id[inx]);
		BUG_ON(clk == 0 || IS_ERR(clk));

		freq = pi->pi_opp[inx].opp[opp_inx];

		pi_dbg("%s:%s clock %x policy freq => %d\n",__func__,
				clk->name,policy,freq);

		if((res = ccu_set_freq_policy(to_ccu_clk(clk),CCU_POLICY(policy),freq)) != 0)
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
	/*TBD - ac & atl should not be hardcoded -- NEED tO REVISIT*/
	cfg.ac = 1;
	cfg.atl  = 0;
	cfg.policy = policy;
	pi_dbg("%s:%s: %d event policy => %x \n",
			__func__, pi->name, type,cfg.policy);

	switch(type)
	{
	case POLICY_QOS:

		res =  pwr_mgr_event_set_pi_policy(pi->qos_sw_event_id,pi->id,&cfg);
		break;
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS
	case POLICY_DFS:
		res =  pwr_mgr_event_set_pi_policy(pi->dfs_qos_sw_event_id,pi->id,&cfg);
		break;

	case POLICY_BOTH:
		res =  pwr_mgr_event_set_pi_policy(pi->qos_sw_event_id,pi->id,&cfg);

		if(pi->dfs_qos_sw_event_id != pi->qos_sw_event_id)
			res |=  pwr_mgr_event_set_pi_policy(pi->dfs_qos_sw_event_id,pi->id,&cfg);
		break;
#endif /*CONFIG_CHANGE_POLICY_FOR_DFS*/
	default:

		res = -EINVAL;
		break;
	}
	if(!res)
	{
		pi_change_notify((struct pi *)pi,cfg.policy);
	}
	return res;
}
EXPORT_SYMBOL_GPL(pi_set_policy);

static int pi_def_init(struct pi *pi)
{
	struct pi_mgr_qos_object* qos;
	struct pi_mgr_dfs_object* dfs;
	struct clk* clk;
	struct pi* dep_pi;
	int inx;

	pi_dbg("%s:%s\n",__func__,pi->name);
	if(pi->init)
		return 0;

	spin_lock(&pi_mgr_lock);
	pi->init = 1;
	pi->usg_cnt = 0;
	/* Make sure that CCUs are initialized*/
	for(inx =0; inx < pi->num_ccu_id;inx++)
	{
		clk = clk_get(NULL,pi->ccu_id[inx]);
		BUG_ON(clk == 0 || IS_ERR(clk));
		clk_init(clk);
	}

	/*Make sure that dependent PI are initialized, if any*/
	for(inx =0; inx < pi->num_dep_pi;inx++)
	{
		dep_pi = pi_mgr_get(pi->dep_pi[inx]);
		BUG_ON(dep_pi == NULL);
		pi_init(dep_pi);
	}

	if((pi->flags & PI_NO_QOS) == 0)
	{
		qos = &pi_mgr.qos[pi->id];
		BLOCKING_INIT_NOTIFIER_HEAD(&qos->notifiers);
		plist_head_init(&qos->requests);
		BUG_ON(pi->num_states > PI_MGR_MAX_STATE_ALLOWED);

		qos->default_latency = pi->pi_state[pi->num_states-1].hw_wakeup_latency;
		pi->state_allowed = pi->num_states-1;
		pi_dbg( "qos->default_latency = %d state_allowed = %d\n", qos->default_latency,pi->state_allowed );
		if(pi->flags & UPDATE_PM_QOS)
		{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
			pi->pm_qos	= pm_qos_add_request(PM_QOS_CPU_DMA_LATENCY,PM_QOS_DEFAULT_VALUE);
#else
			pm_qos_add_request(&pi->pm_qos, PM_QOS_CPU_DMA_LATENCY,PM_QOS_DEFAULT_VALUE);
#endif
		}
	}

	if((pi->flags & PI_NO_DFS) == 0)
	{
		pi_dbg( "pi->opp_active = %d\n",pi->opp_active);
		dfs = &pi_mgr.dfs[pi->id];
		BLOCKING_INIT_NOTIFIER_HEAD(&dfs->notifiers);
		plist_head_init(&dfs->requests);

		dfs->default_opp = 0;
		BUG_ON(pi->num_opp && pi->pi_opp == NULL);

#ifdef CONFIG_CHANGE_POLICY_FOR_DFS

		pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy = pi->pi_opp[0].opp[pi->opp_active];
		pi_set_policy(pi, pi->pi_opp[0].opp[pi->opp_active],POLICY_DFS);
#else
		pi_set_ccu_freq(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
						pi->opp_active);
#endif
	}

	if(pi->num_states)
	{
		pi_dbg("%s: %s %s on init\n",__func__,pi->name,pi->flags & PI_ENABLE_ON_INIT ? "enable" : "disable" );

		pi_enable(pi,pi->flags & PI_ENABLE_ON_INIT);
	}
	spin_unlock(&pi_mgr_lock);
	printk(KERN_INFO " %s: count = %d\n",__func__,pi->usg_cnt);
	return 0;
}

static int pi_def_enable(struct pi *pi, int enable)
{
	u32 policy;
	int inx;
	struct pi* dep_pi;
	pi_dbg("%s: pi_name:%s, enable:%d usageCount:%d\n",__func__,pi->name,enable,pi->usg_cnt);
	spin_lock(&pi_mgr_lock);
	if(enable)
	{
		if(pi->usg_cnt++ != 0)
			goto done;
		policy = pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy;
		pi_dbg("%s: policy = %d -- PI to be enabled\n",__func__,policy);

		/*Make sure that dependent PI are enabled, if any*/
		for(inx =0; inx < pi->num_dep_pi;inx++)
		{
			dep_pi = pi_mgr_get(pi->dep_pi[inx]);
			BUG_ON(dep_pi == NULL);
			pi_enable(dep_pi,1);
		}

	}
	else
	{
		if(pi->usg_cnt == 0 || --pi->usg_cnt != 0)
			goto done;
		policy = pi->pi_state[pi->state_allowed].state_policy;
		pi_dbg("%s: policy = %d pi->state_allowed = %d\n",__func__,policy,pi->state_allowed);

		/*Make sure that dependent PI are disabled, if any*/
		for(inx =0; inx < pi->num_dep_pi;inx++)
		{
			dep_pi = pi_mgr_get(pi->dep_pi[inx]);
			BUG_ON(dep_pi == NULL);
			pi_enable(dep_pi,0);
		}
	}
	pi_dbg("%s: calling pi_set_policy\n",__func__);
	pi_set_policy(pi,policy,POLICY_QOS);
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
	default:
		BUG();
		break;
	}
	new_val = pi_mgr_dfs_get_opp(dfs);
	pi_dbg("%s:pi_id= %d oldval = %d new val = %d\n",__func__,pi_id,old_val,new_val);
	spin_unlock(&pi_mgr_lock);

	if(old_val != new_val)
	{
		blocking_notifier_call_chain(&dfs->notifiers,
					     (unsigned long)new_val,
					     NULL);

		BUG_ON(new_val >= pi->num_opp);
		pi->opp_active = new_val;
#ifdef CONFIG_CHANGE_POLICY_FOR_DFS

		pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy = pi->pi_opp[0].opp[new_val];
		if(pi_is_enabled(pi))
			pi_set_policy(pi, pi->pi_opp[0].opp[new_val],POLICY_QOS);
		if(pi->dfs_qos_sw_event_id != pi->qos_sw_event_id)
			pi_set_policy(pi, pi->pi_opp[0].opp[new_val],POLICY_DFS);
#else
		pi_set_ccu_freq(pi,pi->pi_state[PI_MGR_ACTIVE_STATE_INX].state_policy,
					new_val);
#endif
	}

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
	u32 old_policy;
	u32 new_policy;
	struct pi* dep_pi;
	int found = 0;
	struct pi_mgr_qos_object* qos = &pi_mgr.qos[pi_id];
	struct pi *pi = pi_mgr.pi_list[pi_id];

	spin_lock(&pi_mgr_lock);
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
	spin_unlock(&pi_mgr_lock);

	if(old_val != new_val)
	{
		old_state = pi->state_allowed;
		blocking_notifier_call_chain(&qos->notifiers,
					     (unsigned long)new_val,
					     NULL);

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
		if(!pi_is_enabled(pi) && pi->state_allowed != old_state)
		{
			pi_set_policy(pi, pi->pi_state[pi->state_allowed].state_policy,POLICY_QOS);
		}
		old_policy = pi->pi_state[old_state].state_policy;
		new_policy = pi->pi_state[pi->state_allowed].state_policy;

		if(!IS_ACTIVE_POLICY(old_policy) && IS_ACTIVE_POLICY(new_policy))
		{
			/*Make sure that dependent PI are enabled, if any*/
			for(i =0; i < pi->num_dep_pi;i++)
			{
				dep_pi = pi_mgr_get(pi->dep_pi[i]);
				BUG_ON(dep_pi == NULL);
				pi_enable(dep_pi,1);
			}

		}
		else if(IS_ACTIVE_POLICY(old_policy) && !IS_ACTIVE_POLICY(new_policy))
		{
			/*Make sure that dependent PI are disabled, if any*/
			for(i =0; i < pi->num_dep_pi;i++)
			{
				dep_pi = pi_mgr_get(pi->dep_pi[i]);
				BUG_ON(dep_pi == NULL);
				pi_enable(dep_pi,0);
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

	}
	pi_dbg("%s:%s state allowed = %d\n",__func__,pi->name,pi->state_allowed);

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
	if(pi)
		ret = pi->opp_active;
	return ret;
}
EXPORT_SYMBOL(pi_get_active_opp);




int pi_mgr_register(struct pi* pi)
{
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
	pi_dbg("%s:client = %s,pi_id = %d, lat_val = %d\n",__func__, client_name,
			pi_id,lat_value);
	if(unlikely(!pi_mgr.init))
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return NULL;
	}
	if(unlikely(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL))
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return NULL;
	}

	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_QOS))
	{
		pi_dbg("%s:ERROR - QOS not supported for this PI\n",__func__);
		return NULL;
	}

	node = kzalloc(sizeof(struct pi_mgr_qos_node), GFP_KERNEL);
	if(!node)
	{
		pi_dbg("%s:ERROR - kzalloc failed\n",__func__);
		return NULL;
	}
	if(lat_value == PI_MGR_QOS_DEFAULT_VALUE)
	{
		lat_value = pi_mgr.qos[pi_id].default_latency;
		pi_dbg("%s:lat_value = PI_MGR_QOS_DEFAULT_VALUE, def_at = %d\n",__func__,lat_value);
	}

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
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	pi_dbg("%s: exisiting req = %d new_req = %d\n",__func__,node->latency,lat_value);
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
	pi_dbg("%s:name = %s, req = %d\n",__func__,node->name,node->latency);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
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
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
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
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
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
	pi_dbg("%s:client = %s pi = %d opp = %d\n",__func__,client_name, pi_id, opp);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return NULL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || (pi = pi_mgr.pi_list[pi_id]) == NULL)
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return NULL;
	}
	if(unlikely(pi->flags & PI_NO_DFS))
	{
		pi_dbg("%s:ERROR - DFS not supported for this PI\n",__func__);
		return NULL;
	}

	if(opp >= pi->num_opp)
	{
		__WARN();
		pi_dbg("%s:ERROR - %d:unsupported opp \n",__func__,opp);
		return NULL;
	}

	node = kzalloc(sizeof(struct pi_mgr_dfs_node), GFP_KERNEL);
	if(!node)
	{
		pi_dbg("%s:ERROR - kzalloc failed\n",__func__);
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
	struct pi* pi =  pi_mgr.pi_list[node->pi_id];
	BUG_ON(pi==NULL);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(opp >= pi->num_opp)
	{
		__WARN();
		pi_dbg("%s:ERROR - %d:unsupported opp \n",__func__,opp);
		return -EINVAL;
	}
	pi_dbg("%s:client = %s pi = %d opp = %d opp_new = %d\n",__func__,
			node->name, node->pi_id, opp,node->opp);
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
	pi_dbg("%s:name = %s, req = %d\n",__func__,node->name,node->opp);
	if(!pi_mgr.init)
	{
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
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
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}

	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS))
	{
		pi_dbg("%s:ERROR - DFS not supported for this PI\n",__func__);
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
		pi_dbg("%s:ERROR - pi mgr not initialized\n",__func__);
		return -EINVAL;
	}
	if(pi_id >= PI_MGR_PI_ID_MAX || pi_mgr.pi_list[pi_id] == NULL)
	{
		pi_dbg("%s:ERROR - invalid pid\n",__func__);
		return -EINVAL;
	}
	if(unlikely(pi_mgr.pi_list[pi_id]->flags & PI_NO_DFS))
	{
		pi_dbg("%s:ERROR - DFS not supported for this PI\n",__func__);
		return -EINVAL;;
	}


	return blocking_notifier_chain_unregister(
			&pi_mgr.dfs[pi_id].notifiers, notifier);

}
EXPORT_SYMBOL_GPL(pi_mgr_dfs_remove_notifier);



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
	spin_lock_init(&pi_mgr_lock);
	memset(&pi_mgr,0,sizeof(pi_mgr));
	pi_mgr.init = 1;
	return 0;
}
EXPORT_SYMBOL(pi_mgr_init);

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
	BUG_ON(debug_qos_client[inx].debugfs_qos_node == NULL);
	ret = pi_mgr_qos_request_update(debug_qos_client[inx].debugfs_qos_node
			, val);
	if(ret == 0)
		debug_qos_client[inx].req = (int)val;
	return ret;
}
static int pi_debug_get_qos(void *data, u64* val)
{
	u32 inx = (u32)data;
	BUG_ON(debug_qos_client[inx].debugfs_qos_node == NULL);
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

    debug_qos_client[val].debugfs_qos_node =
	pi_mgr_qos_add_request(debug_qos_client[val].client_name, pi->id, PI_MGR_QOS_DEFAULT_VALUE);

    debug_qos_client[val].dent_client = dent_client;
	debug_qos_client[val].pi = pi;
	debug_qos_client[val].req = PI_MGR_QOS_DEFAULT_VALUE;

    return 0;
err:
    debug_qos_client[val].debugfs_qos_node = NULL;
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
		BUG_ON(!debug_qos_client[val].debugfs_qos_node ||
				!debug_qos_client[val].dent_client);
	    ret = pi_mgr_qos_request_remove(debug_qos_client[val].debugfs_qos_node);
	    if(ret)
			pi_dbg("Failed to remove node\n");
	    debugfs_remove(debug_qos_client[val].dent_client);

	    debug_qos_client[val].pi = NULL;
	    debug_qos_client[val].client_name[0] = '\0';
	    debug_qos_client[val].dent_client = NULL;
	    debug_qos_client[val].debugfs_qos_node = NULL;
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


static int pi_debug_set_dfs(void *data, u64 val)
{
	u32 inx = (u32)data;
	int ret;
	BUG_ON(debug_dfs_client[inx].debugfs_dfs_node == NULL);
	ret = pi_mgr_dfs_request_update(debug_dfs_client[inx].debugfs_dfs_node
			, val);
	if(ret == 0)
		debug_dfs_client[inx].req = (int)val;
	return ret;
}
static int pi_debug_get_dfs(void *data, u64* val)
{
	u32 inx = (u32)data;
	BUG_ON(debug_dfs_client[inx].debugfs_dfs_node == NULL);
	*val = debug_dfs_client[inx].req;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_dfs_client_fops, pi_debug_get_dfs, pi_debug_set_dfs, "%llu\n");

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

    debug_dfs_client[val].debugfs_dfs_node =
		pi_mgr_dfs_add_request(debug_dfs_client[val].client_name, pi->id, 0 /*zero is the min opp*/);

    dent_client = debugfs_create_file(debug_dfs_client[val].client_name, S_IWUSR|S_IRUSR, dfs_dir,
	    (void*)val, &pi_dfs_client_fops);
    if(!dent_client)
		goto err;
    debug_dfs_client[val].dent_client = dent_client;
	debug_dfs_client[val].pi = pi;
	debug_dfs_client[val].req = 0;

    return 0;
err:
    debug_dfs_client[val].debugfs_dfs_node = NULL;
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
		BUG_ON(!debug_dfs_client[val].debugfs_dfs_node ||
				!debug_dfs_client[val].dent_client );
	    ret = pi_mgr_dfs_request_remove(debug_dfs_client[val].debugfs_dfs_node);
	    if(ret)
			pi_dbg("Failed to remove node\n");
	    debugfs_remove(debug_dfs_client[val].dent_client);

	    debug_dfs_client[val].pi = NULL;
	    debug_dfs_client[val].client_name[0] = '\0';
	    debug_dfs_client[val].dent_client = NULL;
	    debug_dfs_client[val].debugfs_dfs_node = NULL;
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
			"PI: %s (Id:%d) \t\t Client:%s \t\t DFS request:%u \n", pi->name, dfs_node->pi_id, dfs_node->name, dfs_node->opp);
    }
	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf, len);
}

static struct file_operations pi_dfs_request_list_fops =
{
	.open =         pi_debugfs_open,
	.read =         read_get_dfs_request_list,
};


static int pi_debug_set_enable(void *data, u64 val)
{
    struct pi *pi = data;

    if(pi && pi->ops && pi->ops->enable)
	{
		if(val == 1)
			pi->ops->enable(pi, 1);
		else if(val == 0)
			pi->ops->enable(pi, 0);
		else
			pi_dbg("write 1 to enable; 0 to disable\n");
    }

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pi_enable_fops, NULL, pi_debug_set_enable, "%llu\n");

static int pi_debug_get_count(void *data, u64 *val)
{
    struct pi *pi = data;
    *val = pi->usg_cnt;
    if (*val >= 0)
	 pi_dbg("%s count is %llu \n", pi->name, *val);

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


static struct dentry *dent_pi_root_dir;
int __init pi_debug_init(void)
{
    struct dentry *dent_all_requests = 0;
    dent_pi_root_dir = debugfs_create_dir("power_domains", 0);
    if(!dent_pi_root_dir)
		return -ENOMEM;
    if(!debugfs_create_u32("debug", S_IRUSR|S_IWUSR, dent_pi_root_dir, (int *)&pi_debug))
		return -ENOMEM;

    dent_all_requests = debugfs_create_file("all_requests", S_IRUSR, dent_pi_root_dir, NULL, &all_req_fops);
    if(!dent_all_requests)
		pi_dbg("Erro registering all_requests with debugfs\n");
    return 0;

}

int __init pi_debug_add_pi(struct pi *pi)
{
    struct dentry *dent_pi_dir=0, *dent_count=0, *dent_enable=0,
    *dent_dfs_dir=0, *dent_dfs=0, *dent_register_qos_client=0,
    *dent_remove_qos_client=0, *dent_remove_dfs_client=0, *dent_register_dfs_client=0,
    *dent_request_dfs=0, *dent_qos_dir=0, *dent_qos=0, *dent_request_qos=0, *dent_state,
	*dent_opp;


    BUG_ON(!dent_pi_root_dir);

    pi_dbg("%s: adding %s to pi debugfs \n", __func__, pi->name);

    dent_pi_dir = debugfs_create_dir(pi->name, dent_pi_root_dir);
    if(!dent_pi_dir)
		goto err;

	dent_enable = debugfs_create_file("enable", S_IWUSR|S_IRUSR, dent_pi_dir, pi, &pi_enable_fops);
	if(!dent_enable)
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

    dent_qos_dir = debugfs_create_dir("qos", dent_pi_dir);
    if(!dent_qos_dir)
		goto err;

	dent_dfs_dir = debugfs_create_dir("dfs", dent_pi_dir);
	if(!dent_dfs_dir)
		goto err;

    debugfs_info[pi->id].pi_id = pi->id;
    debugfs_info[pi->id].qos_dir = dent_qos_dir;
    debugfs_info[pi->id].dfs_dir = dent_dfs_dir;

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

    return 0;

err:
    debugfs_remove(dent_enable);
    debugfs_remove(dent_count);

    return -ENOMEM;

}

#endif /* CONFIG_DEBUG_FS  */

