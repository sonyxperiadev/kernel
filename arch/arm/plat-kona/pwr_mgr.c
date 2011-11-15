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
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/clkdev.h>
#include <asm/io.h>

#include <plat/pi_mgr.h>
#include <mach/pwr_mgr.h>
#include <plat/pwr_mgr.h>
#include <mach/io_map.h>
#include <mach/clock.h>
#include <plat/clock.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#ifndef PWRMGR_I2C_VAR_DATA_REG
#define PWRMGR_I2C_VAR_DATA_REG 6
#endif /*PWRMGR_I2C_VAR_DATA_REG*/


#ifdef CONFIG_DEBUG_FS
#ifndef PWRMGR_EVENT_ID_TO_STR
static char* pwr_mgr_event2str(int event)
{
	static char str[10] ;
	sprintf(str,"event_%d",event);
	return str;
}
#define PWRMGR_EVENT_ID_TO_STR(e) pwr_mgr_event2str(e)
#endif /*PWRMGR_EVENT_ID_TO_STR*/
#endif /*CONFIG_DEBUG_FS*/


#define I2C_CMD0_DATA_SHIFT \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND_DATA__01_0_SHIFT
#define I2C_CMD0_DATA_MASK \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND_DATA__01_0_MASK
#define I2C_CMD1_DATA_SHIFT \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND_DATA__01_1_SHIFT
#define I2C_CMD1_DATA_MASK \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND_DATA__01_1_MASK

#define I2C_CMD0_SHIFT \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND__01_0_SHIFT
#define I2C_CMD0_MASK \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND__01_0_MASK
#define I2C_CMD1_SHIFT \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND__01_1_SHIFT
#define I2C_CMD1_MASK \
		PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_I2C_COMMAND__01_1_MASK

#define I2C_COMMAND_WORD(cmd1,cmd1_data,cmd0,cmd0_data) \
			( ((((u32)(cmd0)) << I2C_CMD0_SHIFT) & I2C_CMD0_MASK ) |\
				((((u32)(cmd0_data)) << I2C_CMD0_DATA_SHIFT) & I2C_CMD0_DATA_MASK) |\
				((((u32)(cmd1)) << I2C_CMD1_SHIFT) & I2C_CMD1_MASK) |\
				((((u32)(cmd1_data))  << I2C_CMD1_DATA_SHIFT)& I2C_CMD1_DATA_MASK) )


static int pwr_debug = 0;
/* global spinlock for pwr mgr API */
static DEFINE_SPINLOCK(pwr_mgr_lock);

struct pwr_mgr_event
{
	void (*pwr_mgr_event_cb)(u32 event_id,void* param);
	void* param;
};

struct pwr_mgr
{
	struct pwr_mgr_info* info;
	u32 i2c_sem_val;
	struct pwr_mgr_event event_cb[PWR_MGR_NUM_EVENTS];

};

static struct pwr_mgr pwr_mgr;

#define PWR_MGR_REG_ADDR(offset) (pwr_mgr.info->base_addr+(offset))
#define PWR_MGR_PI_EVENT_POLICY_ADDR(pi_offset,event_offset) (\
				pwr_mgr.info->base_addr+(pi->pi_info.pi_offset)+(event_offset))
#define PWR_MGR_PI_ADDR(pi_offset) (\
				pwr_mgr.info->base_addr+(pi->pi_info.pi_offset))



int	pwr_mgr_event_trg_enable(int event_id,int event_trg_type)
{
    u32 reg_val = 0;
	u32 reg_offset;
	pwr_dbg("%s:event_id: %d, trg : %d\n",
		__func__,event_id,event_trg_type);
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	reg_offset = event_id * 4;

    reg_val = readl(PWR_MGR_REG_ADDR(reg_offset));

	/*clear both pos & neg edge bits */
    reg_val &= ~PWRMGR_EVENT_NEGEDGE_CONDITION_ENABLE_MASK;
    reg_val &= ~PWRMGR_EVENT_POSEDGE_CONDITION_ENABLE_MASK;

	if(event_trg_type & PM_TRIG_POS_EDGE)
		reg_val |= PWRMGR_EVENT_POSEDGE_CONDITION_ENABLE_MASK;
    if(event_trg_type & PM_TRIG_NEG_EDGE)
	reg_val |= PWRMGR_EVENT_NEGEDGE_CONDITION_ENABLE_MASK;

    writel(reg_val, PWR_MGR_REG_ADDR(reg_offset));
	pwr_dbg("%s:reg_addr:%x value = %x\n",__func__,PWR_MGR_REG_ADDR(reg_offset),reg_val);
	spin_unlock(&pwr_mgr_lock);

	return 0;
}
EXPORT_SYMBOL(pwr_mgr_event_trg_enable);

int	pwr_mgr_get_event_trg_type(int event_id)
{
	u32 reg_val = 0;
	int trig_type = PM_TRIG_NONE;
	u32 reg_offset;
	pwr_dbg("%s:event_id: %d\n",
		__func__,event_id);
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return -EPERM;
	}
	reg_offset = event_id * 4;

    reg_val = readl(PWR_MGR_REG_ADDR(reg_offset));

	if(reg_val & PWRMGR_EVENT_NEGEDGE_CONDITION_ENABLE_MASK)
		trig_type |= PM_TRIG_NEG_EDGE;

	if(reg_val & PWRMGR_EVENT_POSEDGE_CONDITION_ENABLE_MASK)
		trig_type |= PM_TRIG_POS_EDGE;

	return trig_type;

}
EXPORT_SYMBOL(pwr_mgr_get_event_trg_type);

int	pwr_mgr_event_clear_events(u32 event_start, u32 event_end)
{
	u32 reg_val = 0;
	int inx;
	pwr_dbg("%s\n",__func__);
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}
	if(event_end == EVENT_ID_ALL)
	{
		event_end = PWR_MGR_NUM_EVENTS-1;
	}

	if(event_start == EVENT_ID_ALL)
	{
		event_start = 0;
	}

	if(unlikely(event_end >= PWR_MGR_NUM_EVENTS ||
			event_start > event_end))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	for(inx = event_start; inx <= event_end;inx++)
	{
		reg_val = readl(PWR_MGR_REG_ADDR(inx*4));
		if(reg_val & PWRMGR_EVENT_CONDITION_ACTIVE_MASK)
		{
			reg_val &= ~PWRMGR_EVENT_CONDITION_ACTIVE_MASK;
			writel(reg_val,PWR_MGR_REG_ADDR(inx*4));
		}
	}
	spin_unlock(&pwr_mgr_lock);
	return 0;

}
EXPORT_SYMBOL(pwr_mgr_event_clear_events);

bool pwr_mgr_is_event_active(int event_id)
{
	u32 reg_val = 0;
	pwr_dbg("%s : event_id = %d\n",__func__,event_id);
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return false;
	}

	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return false;
	}
	reg_val = readl(PWR_MGR_REG_ADDR(event_id*4));
	return !!(reg_val & PWRMGR_EVENT_CONDITION_ACTIVE_MASK);

}
EXPORT_SYMBOL(pwr_mgr_is_event_active);

int	pwr_mgr_event_set(int event_id, int event_state)
{
	u32 reg_val = 0;
	pwr_dbg("%s : event_id = %d : enable = %d\n",__func__,event_id,!!event_state);
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	reg_val = readl(PWR_MGR_REG_ADDR(event_id*4));
	if(event_state)
		reg_val |= PWRMGR_EVENT_CONDITION_ACTIVE_MASK;
	else
		reg_val &= ~PWRMGR_EVENT_CONDITION_ACTIVE_MASK;
	writel(reg_val,PWR_MGR_REG_ADDR(event_id*4));
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_event_set);

int pwr_mgr_event_set_pi_policy(int event_id,int pi_id,const struct pm_policy_cfg* pm_policy_cfg)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : event_id = %d : pi_id = %d, ac : %d, ATL : %d, policy: %d\n",
		__func__,event_id,pi_id,pm_policy_cfg->ac, pm_policy_cfg->atl, pm_policy_cfg->policy);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS || pi_id >= pwr_mgr.info->num_pi))
	{
		pwr_dbg("%s:invalid group/pi id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);

	reg_val = readl(PWR_MGR_PI_EVENT_POLICY_ADDR(policy_reg_offset,event_id*4));

	if(pm_policy_cfg->ac)
		reg_val |= (1 << pi->pi_info.ac_shift);
	else
		reg_val &= ~(1 << pi->pi_info.ac_shift);

	if(pm_policy_cfg->atl)
		reg_val |= (1 << pi->pi_info.atl_shift);
	else
		reg_val &= ~(1 << pi->pi_info.atl_shift);

	reg_val &= ~(PM_POLICY_MASK << pi->pi_info.pm_policy_shift);
	reg_val |= (pm_policy_cfg->policy & PM_POLICY_MASK) <<
								pi->pi_info.pm_policy_shift;

	pwr_dbg("%s:reg val %08x shift val: %08x\n",__func__,reg_val, pi->pi_info.pm_policy_shift);

	writel(reg_val,PWR_MGR_PI_EVENT_POLICY_ADDR(policy_reg_offset,event_id*4));
	pwr_dbg("%s : event_id = %d : pi_id = %d, ac : %d, ATL : %d, policy: %d\n",
		__func__,event_id,pi_id,pm_policy_cfg->ac, pm_policy_cfg->atl, pm_policy_cfg->policy);


	pwr_dbg("%s:reg val %08x written to register: %08x\n",
		__func__,reg_val, PWR_MGR_PI_EVENT_POLICY_ADDR(policy_reg_offset,event_id*4));

	spin_unlock(&pwr_mgr_lock);

	return 0;
}
EXPORT_SYMBOL(pwr_mgr_event_set_pi_policy);

int pwr_mgr_event_get_pi_policy(int event_id,int pi_id,struct pm_policy_cfg* pm_policy_cfg)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : event_id = %d : pi_id = %d\n",
				__func__,event_id, pi_id);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS || pi_id >= pwr_mgr.info->num_pi))
	{
		pwr_dbg("%s:invalid event/pi id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);

	reg_val = readl(PWR_MGR_PI_EVENT_POLICY_ADDR(policy_reg_offset,event_id*4));

	pm_policy_cfg->ac = !!(reg_val & (1 << pi->pi_info.ac_shift));
	pm_policy_cfg->atl = !!(reg_val & (1 << pi->pi_info.atl_shift));

	pm_policy_cfg->policy = (reg_val >> pi->pi_info.pm_policy_shift) & PM_POLICY_MASK;
	spin_unlock(&pwr_mgr_lock);

	return 0;
}
EXPORT_SYMBOL(pwr_mgr_event_get_pi_policy);

int pwr_mgr_set_pi_fixed_volt_map(int pi_id,bool activate)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : pi_id = %d\n",
				__func__,pi_id);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(pi_id >= pwr_mgr.info->num_pi))
	{
		pwr_dbg("%s:invalid event/pi id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);

	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_FIXED_VOLTAGE_MAP_OFFSET));
	if(activate)
		reg_val |= pi->pi_info.fixed_vol_map_mask;
	else
		reg_val &= ~pi->pi_info.fixed_vol_map_mask;
	writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_FIXED_VOLTAGE_MAP_OFFSET));
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_set_pi_fixed_volt_map);

int	pwr_mgr_set_pi_vmap(int pi_id,int vset, bool activate)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : vset = %d : pi_id = %d\n",
				__func__,vset, pi_id);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(pi_id >= pwr_mgr.info->num_pi ||
		vset < VOLT0 || vset > VOLT2))
	{
		pwr_dbg("%s:invalid param\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);

	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_VI_TO_VO0_MAP_OFFSET+4*vset));
	if(activate)
		reg_val |= pi->pi_info.vi_to_vOx_map_mask;
	else
		reg_val &= ~pi->pi_info.vi_to_vOx_map_mask;

	writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_VI_TO_VO0_MAP_OFFSET+4*vset));
	spin_unlock(&pwr_mgr_lock);
	return 0;

}
EXPORT_SYMBOL(pwr_mgr_set_pi_vmap);

int	pwr_mgr_pi_set_wakeup_override(int pi_id, bool clear)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : clear = %d : pi_id = %d\n",
				__func__,clear, pi_id);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(pi_id >= pwr_mgr.info->num_pi))
	{
		pwr_dbg("%s:invalid param\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);

	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));
	if(clear)
		reg_val &= ~pi->pi_info.wakeup_overide_mask;
	else
		reg_val |= pi->pi_info.wakeup_overide_mask;
	pr_info("%s:just before reg update...\n",__func__);
	writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));
	spin_unlock(&pwr_mgr_lock);
	return 0;

}
EXPORT_SYMBOL(pwr_mgr_pi_set_wakeup_override);

int pwr_mgr_set_pc_sw_override(int pc_pin, bool enable, int value)
{
	u32 reg_val = 0;
	u32 value_mask, enable_mask;

	pwr_dbg("%s : pc_pin = %d : enable = %d,value = %d\n",
				__func__,pc_pin, enable,value);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	spin_lock(&pwr_mgr_lock);

	switch (pc_pin)
    {
    case PC3:
    	value_mask = PWRMGR_PC3_SW_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC3_SW_OVERRIDE_ENABLE_MASK;
    	break;
    case PC2:
    	value_mask = PWRMGR_PC2_SW_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC2_SW_OVERRIDE_ENABLE_MASK;
		break;
    case PC1:
    	value_mask = PWRMGR_PC1_SW_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC1_SW_OVERRIDE_ENABLE_MASK;
		break;
	case PC0:
    	value_mask = PWRMGR_PC0_SW_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC0_SW_OVERRIDE_ENABLE_MASK;
	break;
    default:
		pwr_dbg("%s:invalid param\n",__func__);
    	return -EINVAL;
    }
    reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));
    if(enable)
		reg_val = reg_val | (value_mask | enable_mask);
    else
		reg_val = reg_val & (~enable_mask);
	 writel(reg_val, PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_set_pc_sw_override);

int	pwr_mgr_set_pc_clkreq_override(int pc_pin, bool enable, int value)
{
	u32 reg_val = 0;
	u32 value_mask, enable_mask;

	pwr_dbg("%s : pc_pin = %d : enable = %d,value = %d\n",
				__func__,pc_pin, enable,value);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	spin_lock(&pwr_mgr_lock);

	switch (pc_pin)
    {
    case PC3:
    	value_mask = PWRMGR_PC3_CLKREQ_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC3_CLKREQ_OVERRIDE_ENABLE_MASK;
    	break;
    case PC2:
    	value_mask = PWRMGR_PC2_CLKREQ_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC2_CLKREQ_OVERRIDE_ENABLE_MASK;
		break;
    case PC1:
    	value_mask = PWRMGR_PC1_CLKREQ_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC1_CLKREQ_OVERRIDE_ENABLE_MASK;
		break;
	case PC0:
    	value_mask = PWRMGR_PC0_CLKREQ_OVERRIDE_VALUE_MASK;
		enable_mask = PWRMGR_PC0_CLKREQ_OVERRIDE_ENABLE_MASK;
	break;
    default:
		pwr_dbg("%s:invalid param\n",__func__);
    	return -EINVAL;
    }
    reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));
    if(enable)
		reg_val = reg_val | (value_mask | enable_mask);
    else
		reg_val = reg_val & (~enable_mask);
	writel(reg_val, PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_set_pc_clkreq_override);

int pm_get_pc_value(int pc_pin)
{
	u32 reg_val = 0;
	u32 value;

	pwr_dbg("%s : pc_pin = %d \n",
				__func__,pc_pin);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));
	switch(pc_pin)
    {
    case PC3:
    	value = reg_val & PWRMGR_PC3_CURRENT_VALUE_MASK;
    	break;
    case PC2:
    	value = reg_val & PWRMGR_PC2_CURRENT_VALUE_MASK;
		break;
    case PC1:
    	value = reg_val & PWRMGR_PC1_CURRENT_VALUE_MASK;
		break;
    case PC0:
    	value = reg_val & PWRMGR_PC0_CURRENT_VALUE_MASK;
		break;
    default:
		pwr_dbg("%s:invalid param\n",__func__);
    	return -EINVAL;
    	break;
    }

    return (!!value);
}
EXPORT_SYMBOL(pm_get_pc_value);

int pwr_mgr_pi_counter_enable(int pi_id, bool enable)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : pi_id = %d enable = %d\n",
				__func__, pi_id,enable);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(pi_id >= pwr_mgr.info->num_pi))
	{
		pwr_dbg("%s:invalid param\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);
	reg_val = readl(PWR_MGR_PI_ADDR(counter_reg_offset));
	pwr_dbg("%s:counter reg val = %x\n",__func__,reg_val);

	if(enable)
		reg_val |= PWRMGR_PI_ARM_CORE_ON_COUNTER_PI_ARM_CORE_ON_COUNTER_ENABLE_MASK;
	else
		reg_val &= ~PWRMGR_PI_ARM_CORE_ON_COUNTER_PI_ARM_CORE_ON_COUNTER_ENABLE_MASK;
	writel(reg_val,PWR_MGR_PI_ADDR(counter_reg_offset));
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_pi_counter_enable);

int pwr_mgr_pi_counter_read(int pi_id,bool* over_flow)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : pi_id = %d\n",
				__func__, pi_id);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return 0;
	}

	if(unlikely(pi_id >= pwr_mgr.info->num_pi))
	{
		pwr_dbg("%s:invalid param\n",__func__);
		return 0;
	}
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);
	reg_val = readl(PWR_MGR_PI_ADDR(counter_reg_offset));
	pwr_dbg("%s:counter reg val = %x\n",__func__,reg_val);

	if(over_flow)
		*over_flow = !!(reg_val &
						PWRMGR_PI_ARM_CORE_ON_COUNTER_PI_ARM_CORE_ON_COUNTER_OVERFLOW_MASK);
	return ((reg_val & PWRMGR_PI_ARM_CORE_ON_COUNTER_PI_ARM_CORE_ON_COUNTER_MASK)
					>> PWRMGR_PI_ARM_CORE_ON_COUNTER_PI_ARM_CORE_ON_COUNTER_SHIFT);
}
EXPORT_SYMBOL(pwr_mgr_pi_counter_read);

int pwr_mgr_pm_i2c_sem_lock()
{
	u32 value, read_val,write_val;
	u32 max_val;
	u32 ret = 0;
	u32 insurance = 1000;
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EINVAL;
	}

	max_val = 1 + (PWRMGR_I2C_HARDWARE_SEMAPHORE_WRITE_I2C_HARDWARE_SEMAPHORE_WRITE_VALUE_MASK >>
				PWRMGR_I2C_HARDWARE_SEMAPHORE_WRITE_I2C_HARDWARE_SEMAPHORE_WRITE_VALUE_SHIFT);

	spin_lock(&pwr_mgr_lock);
	pwr_mgr.i2c_sem_val = (pwr_mgr.i2c_sem_val+1)% max_val;
	if(pwr_mgr.i2c_sem_val == 0)
		pwr_mgr.i2c_sem_val++;

	value = pwr_mgr.i2c_sem_val;
	write_val = (value << PWRMGR_I2C_HARDWARE_SEMAPHORE_WRITE_I2C_HARDWARE_SEMAPHORE_WRITE_VALUE_SHIFT)
				& PWRMGR_I2C_HARDWARE_SEMAPHORE_WRITE_I2C_HARDWARE_SEMAPHORE_WRITE_VALUE_MASK;
	pwr_dbg("%s: value = %x max_val = %x\n",__func__,value,max_val);
	do
	{
		writel(write_val,
			PWR_MGR_REG_ADDR(PWRMGR_I2C_HARDWARE_SEMAPHORE_WRITE_OFFSET));
		udelay(1);
		read_val = readl(PWR_MGR_REG_ADDR(PWRMGR_I2C_HARDWARE_SEMAPHORE_READ_OFFSET));
		read_val &= PWRMGR_I2C_HARDWARE_SEMAPHORE_READ_I2C_HARDWARE_SEMAPHORE_READ_VALUE_MASK;
		read_val >>= PWRMGR_I2C_HARDWARE_SEMAPHORE_READ_I2C_HARDWARE_SEMAPHORE_READ_VALUE_SHIFT;
		insurance--;
	}while(read_val != value && insurance);
	spin_unlock(&pwr_mgr_lock);

	if(read_val != value)
	{
		pr_info("%s: failed to acquire PMU I2C HW sem !!\n",__func__);
		ret = -EAGAIN;
	}
	return ret;
}
EXPORT_SYMBOL(pwr_mgr_pm_i2c_sem_lock);

int pwr_mgr_pm_i2c_sem_unlock()
{
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EINVAL;
	}
	spin_lock(&pwr_mgr_lock);
	writel(0,
			PWR_MGR_REG_ADDR(PWRMGR_I2C_HARDWARE_SEMAPHORE_WRITE_OFFSET));
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_pm_i2c_sem_unlock);

int	pwr_mgr_pm_i2c_enable(bool enable)
{
	u32 reg_val;
	pwr_dbg("%s:enable = %d\n",__func__,enable);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EINVAL;
	}
	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_POWER_MANAGER_I2C_ENABLE_OFFSET));
	if(enable)
		reg_val |= PWRMGR_POWER_MANAGER_I2C_ENABLE_POWER_MANAGER_I2C_ENABLE_MASK;
	else
		reg_val &= ~PWRMGR_POWER_MANAGER_I2C_ENABLE_POWER_MANAGER_I2C_ENABLE_MASK;
	writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_POWER_MANAGER_I2C_ENABLE_OFFSET));
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_pm_i2c_enable);

int	pwr_mgr_set_v0x_specific_i2c_cmd_ptr(int v0x, const struct v0x_spec_i2c_cmd_ptr* cmd_ptr)
{
	u32 reg_val;
	u32 offset;
	pwr_dbg("%s:v0x = %d\n",__func__,v0x);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EINVAL;
	}

	if(unlikely(v0x < VOLT0 || v0x > VOLT2))
	{
		pwr_dbg("%s:ERROR - invalid param\n",__func__);
		return -EINVAL;
	}
	spin_lock(&pwr_mgr_lock);
	offset = PWRMGR_VO0_SPECIFIC_I2C_COMMAND_POINTER_OFFSET +4*v0x;

	reg_val = (cmd_ptr->set2_val << PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_VALUE_SHIFT)
    				& PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_VALUE_MASK;
    reg_val |= (cmd_ptr->set2_ptr << PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_SHIFT)
    				& PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET2_MASK;
    reg_val |= (cmd_ptr->set1_val << PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_VALUE_SHIFT)
    				& PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_VALUE_MASK;
    reg_val |= (cmd_ptr->set1_ptr << PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_SHIFT)
    				& PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_SET1_MASK;
    reg_val |= (cmd_ptr->zerov_ptr << PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_ZEROV_SHIFT)
    				& PWRMGR_VO_SPECIFIC_I2C_COMMAND_PTR_ZEROV_MASK;
    reg_val |= (cmd_ptr->other_ptr << PWRMGR_VO_SPECIFIC_I2C_COMMAND_POINTER_SHIFT)
    				& PWRMGR_VO_SPECIFIC_I2C_COMMAND_POINTER_MASK;
    writel(reg_val, PWR_MGR_REG_ADDR(offset));

	pwr_dbg("%s: %x set to %x register\n",__func__,reg_val,PWR_MGR_REG_ADDR(offset));
	spin_unlock(&pwr_mgr_lock);
	return 0;

}
int pwr_mgr_pm_i2c_cmd_write(const struct i2c_cmd* i2c_cmd , u32 num_cmds)
{
	u32 inx;
	u32 reg_val;
	u8 cmd0,cmd1;
	u8 cmd0_data,cmd1_data;

	pwr_dbg("%s:num_cmds = %d\n",__func__,num_cmds);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EINVAL;
	}

	if(unlikely((pwr_mgr.info->flags & PM_PMU_I2C) == 0))
	{
		pwr_dbg("%s:ERROR - invalid param\n",__func__);
		return -EINVAL;
	}
	if(unlikely(num_cmds > PM_I2C_CMD_MAX))
	{
		pwr_dbg("%s:ERROR - invalid param(num_cmds > PM_I2C_CMD_MAX)\n",__func__);
		return -EINVAL;
	}

	spin_lock(&pwr_mgr_lock);
	for(inx=0; inx < (num_cmds+1)/2; inx++)
	{
		cmd0 = i2c_cmd[inx*2].cmd;
		cmd0_data = i2c_cmd[inx*2].cmd_data;

		if((2*inx+1) < num_cmds)
		{
			cmd1 = i2c_cmd[inx*2+1].cmd;
			cmd1_data = i2c_cmd[inx*2+1].cmd_data;
		}
		else
		{
			reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET + inx*4));
			cmd1 = (reg_val & I2C_CMD1_MASK) >> I2C_CMD1_SHIFT;
			cmd1_data = (reg_val & I2C_CMD1_DATA_MASK) >> I2C_CMD1_DATA_SHIFT;
		}
		pwr_dbg("%s:cmd0 = %x cmd0_data = %x cmd1 = %x cmd1_data = %x",__func__, cmd0,cmd0_data,cmd1,cmd1_data);
		reg_val = I2C_COMMAND_WORD(cmd1,cmd1_data,cmd0,cmd0_data);
		writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET + inx*4));
		pwr_dbg("%s: %x set to %x register\n",__func__,reg_val,PWR_MGR_REG_ADDR(PWRMGR_POWER_MANAGER_I2C_COMMAND_DATA_LOCATION_01_OFFSET + inx*4));
	}
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_pm_i2c_cmd_write);

int pwr_mgr_pm_i2c_var_data_write(const u8* var_data,int count)
{
	u32 inx;
	u32 reg_val;

	pwr_dbg("%s:\n",__func__);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EINVAL;
	}

	if(unlikely((pwr_mgr.info->flags & PM_PMU_I2C) == 0 || (count > PWRMGR_I2C_VAR_DATA_REG*4)))
	{
		pwr_dbg("%s:ERROR - invalid param or not supported\n",__func__);
		return -EINVAL;
	}
	spin_lock(&pwr_mgr_lock);

	for(inx=0; inx < count/4; inx++)
	{
		reg_val = (var_data[inx*4] <<PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_00_SHIFT) &
			PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_00_MASK;
		reg_val |= (var_data[inx*4+1] <<PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_01_SHIFT) &
			PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_01_MASK;
		reg_val |= (var_data[inx*4+2] <<PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_02_SHIFT) &
			PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_02_MASK;
		reg_val |= (var_data[inx*4+3] <<PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_03_SHIFT) &
			PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_I2C_VARIABLE_DATA_03_MASK;

		writel(reg_val, PWR_MGR_REG_ADDR(PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_OFFSET + inx*4));
		pwr_dbg("%s: %x set to %x register\n",__func__,
		reg_val,PWR_MGR_REG_ADDR(PWRMGR_POWER_MANAGER_I2C_VARIABLE_DATA_LOCATION_01_OFFSET + inx*4));
	}

	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_pm_i2c_var_data_write);

int	pwr_mgr_arm_core_dormant_enable(bool enable)
{
	u32 reg_val = 0;
	pwr_dbg("%s : enable = %d\n",
				__func__,enable);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EINVAL;
	}
	spin_lock(&pwr_mgr_lock);
	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));
	if(enable)
		reg_val &= ~PWRMGR_PI_DEFAULT_POWER_STATE_ARM_CORE_DORMANT_DISABLE_MASK;
	else
		reg_val |= PWRMGR_PI_DEFAULT_POWER_STATE_ARM_CORE_DORMANT_DISABLE_MASK;
	writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));
	spin_unlock(&pwr_mgr_lock);

	return 0;
}
EXPORT_SYMBOL(pwr_mgr_arm_core_dormant_enable);

int	pwr_mgr_pi_retn_clamp_enable(int pi_id,bool enable)
{
	u32 reg_val = 0;
	const struct pi* pi;
	pwr_dbg("%s : pi_id = %d enable = %d\n",
				__func__, pi_id,enable);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(unlikely(pi_id >= pwr_mgr.info->num_pi))
	{
		pwr_dbg("%s:invalid param\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pi = pi_mgr_get(pi_id);
	BUG_ON(pi == NULL);
	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));
	if(enable)
		reg_val &= ~pi->pi_info.rtn_clmp_dis_mask;
	else
		reg_val |= pi->pi_info.rtn_clmp_dis_mask;
	writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));

	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_pi_retn_clamp_enable);

int pwr_mgr_ignore_power_ok_signal(bool ignore)
{
	u32 reg_val = 0;
	pwr_dbg("%s :ignore = %d\n",
				__func__, ignore);

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);

	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));

	if(ignore)
		reg_val |= PWRMGR_PI_DEFAULT_POWER_STATE_POWER_OK_MASK_MASK;
	else
		reg_val &= ~PWRMGR_PI_DEFAULT_POWER_STATE_POWER_OK_MASK_MASK;

	writel(reg_val,PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));
	spin_unlock(&pwr_mgr_lock);

	return 0;
}
EXPORT_SYMBOL(pwr_mgr_ignore_power_ok_signal);

int pwr_mgr_ignore_dap_powerup_request(bool ignore)
{
	u32 reg_val = 0;
	pwr_dbg("%s :ignore = %d\n",
				__func__, ignore);

	if (unlikely(!pwr_mgr.info)) {
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n", __func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));

	if (ignore)
		reg_val |= \
	    PWRMGR_PI_DEFAULT_POWER_STATE_IGNORE_DAP_POWERUPREQ_MASK;
	else
		reg_val &= \
	    ~PWRMGR_PI_DEFAULT_POWER_STATE_IGNORE_DAP_POWERUPREQ_MASK;

	writel(reg_val, PWR_MGR_REG_ADDR(PWRMGR_PI_DEFAULT_POWER_STATE_OFFSET));

	spin_unlock(&pwr_mgr_lock);

	return 0;
}
EXPORT_SYMBOL(pwr_mgr_ignore_dap_powerup_request);

int pwr_mgr_register_event_handler(u32 event_id, void (*pwr_mgr_event_cb)(u32 event_id,void* param),
											void* param)
{
	int ret = 0;
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}
	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	if(likely(!pwr_mgr.event_cb[event_id].pwr_mgr_event_cb))
	{
		pwr_mgr.event_cb[event_id].pwr_mgr_event_cb = pwr_mgr_event_cb;
		pwr_mgr.event_cb[event_id].param = param;
	}
	else
	{
		ret = -EINVAL;
		pwr_dbg("%s:Handler already registered for event id: %d\n",__func__, event_id);
	}
	spin_unlock(&pwr_mgr_lock);
	return ret;
}
EXPORT_SYMBOL(pwr_mgr_register_event_handler);

int pwr_mgr_unregister_event_handler(u32 event_id)
{
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}
	if(unlikely(event_id >= PWR_MGR_NUM_EVENTS))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return -EPERM;
	}
	spin_lock(&pwr_mgr_lock);
	pwr_mgr.event_cb[event_id].pwr_mgr_event_cb = NULL;
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_unregister_event_handler);

int pwr_mgr_process_events(u32 event_start, u32 event_end, int clear_event)
{
	u32 reg_val = 0;
	int inx;
	pwr_dbg("%s\n",__func__);
	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	if(event_end == EVENT_ID_ALL)
	{
		event_end = PWR_MGR_NUM_EVENTS-1;
	}

	if(event_start == EVENT_ID_ALL)
	{
		event_start = 0;
	}

	if(unlikely(event_end >= PWR_MGR_NUM_EVENTS ||
			event_start > event_end))
	{
		pwr_dbg("%s:invalid event id\n",__func__);
		return -EPERM;
	}

	spin_lock(&pwr_mgr_lock);
	for(inx = event_start; inx <= event_end;inx++)
	{
		reg_val = readl(PWR_MGR_REG_ADDR(inx*4));
		if(reg_val & PWRMGR_EVENT_CONDITION_ACTIVE_MASK)
		{
			pwr_dbg("%s:event id : %x\n",__func__,inx);
			if(pwr_mgr.event_cb[inx].pwr_mgr_event_cb)
				pwr_mgr.event_cb[inx].pwr_mgr_event_cb(inx,pwr_mgr.event_cb[inx].param);
			if(clear_event)
			{
				reg_val &= ~PWRMGR_EVENT_CONDITION_ACTIVE_MASK;
				writel(reg_val,PWR_MGR_REG_ADDR(inx*4));

			}
		}
	}
	spin_unlock(&pwr_mgr_lock);
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_process_events);

int pwr_mgr_init(struct pwr_mgr_info* info)
{
	pwr_mgr.info = info;
	pwr_mgr.i2c_sem_val = 0;
	return 0;
}
EXPORT_SYMBOL(pwr_mgr_init);





#ifdef CONFIG_DEBUG_FS

static u32 bmdm_pwr_mgr_base;

__weak void pwr_mgr_mach_debug_fs_init(int type)
{
}

static int set_pm_mgr_dbg_bus(void *data, u64 val)
{
    u32 reg_val = 0;

    pwr_dbg("%s: val: %lld\n", __func__, val);
    pwr_mgr_mach_debug_fs_init(0);
    if(val > 0xA)
	return -EINVAL;
    reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));
    reg_val &= ~(0xF << 20);
    reg_val |= (val & 0x0F) << 20;
    pwr_dbg("reg_val to be written %08x\n", reg_val);
    writel(reg_val, PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));

    reg_val = readl(PWR_MGR_REG_ADDR(PWRMGR_PC_PIN_OVERRIDE_CONTROL_OFFSET));
    pwr_dbg("PC_PIN_OVERRIDE_CONTROL Register: %08x\n", reg_val);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(set_pm_dbg_bus_fops, NULL, set_pm_mgr_dbg_bus, "%llu\n");

static int set_bmdm_mgr_dbg_bus(void *data, u64 val)
{
    u32 reg_val = 0;
	u32 reg_addr = bmdm_pwr_mgr_base+BMDM_PWRMGR_DEBUG_AND_COUNTER_CONTROL_OFFSET;
    pwr_dbg("%s: val: %lld\n", __func__, val);
    pwr_mgr_mach_debug_fs_init(1);
    if(val > 0xA)
	return -EINVAL;
    reg_val = readl(reg_addr);
    reg_val &= ~(BMDM_PWRMGR_DEBUG_AND_COUNTER_CONTROL_DEBUG_BUS_SELECT_MASK);
    reg_val |= (val << BMDM_PWRMGR_DEBUG_AND_COUNTER_CONTROL_DEBUG_BUS_SELECT_SHIFT) &
					BMDM_PWRMGR_DEBUG_AND_COUNTER_CONTROL_DEBUG_BUS_SELECT_MASK;
    pwr_dbg("reg_val to be written %08x\n", reg_val);
    writel(reg_val, reg_addr);

    reg_val = readl(reg_addr);
    pwr_dbg("BMDM_PWRMGR_DEBUG_AND_COUNTER_CONTROL Register: %08x\n", reg_val);

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(set_bmdm_dbg_bus_fops, NULL, set_bmdm_mgr_dbg_bus, "%llu\n");


static int pwr_mgr_dbg_event_get_active(void *data, u64* val)
{
	u32 event_id = (u32)data;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS);

	*val = pwr_mgr_is_event_active(event_id);
	return 0;
}

static int pwr_mgr_dbg_event_set_active(void *data, u64 val)
{
	u32 event_id = (u32)data;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS);

	return pwr_mgr_event_set(event_id,(int)val);
}

DEFINE_SIMPLE_ATTRIBUTE(pwr_mgr_dbg_active_fops, pwr_mgr_dbg_event_get_active,
		pwr_mgr_dbg_event_set_active, "%llu\n");


static int pwr_mgr_dbg_event_get_trig(void *data, u64* val)
{
	u32 event_id = (u32)data;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS);

	*val = (u64)pwr_mgr_get_event_trg_type(event_id);
	return 0;
}

static int pwr_mgr_dbg_event_set_trig(void *data, u64 val)
{
	u32 event_id = (u32)data;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS);

	return pwr_mgr_event_trg_enable(event_id,(int)val);
}

DEFINE_SIMPLE_ATTRIBUTE(pwr_mgr_dbg_trig_fops, pwr_mgr_dbg_event_get_trig,
		pwr_mgr_dbg_event_set_trig, "%llu\n");

static int pwr_mgr_dbg_event_get_atl(void *data, u64* val)
{
	u32 event_id = ((u32)data & 0xFFFF0000) >> 16;
	u32 pid = (u32)data & 0xFFFF;
	int ret;
	struct pm_policy_cfg pm_policy_cfg;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS ||  pid >= pwr_mgr.info->num_pi);

	ret = pwr_mgr_event_get_pi_policy(event_id,pid,&pm_policy_cfg);

	if(!ret)
		*val = pm_policy_cfg.atl;
	return ret;
}
static int pwr_mgr_dbg_event_set_atl(void *data, u64 val)
{
	u32 event_id = ((u32)data & 0xFFFF0000) >> 16;
	u32 pid = (u32)data & 0xFFFF;
	int ret;
	struct pm_policy_cfg pm_policy_cfg;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS ||  pid >= pwr_mgr.info->num_pi);

	ret = pwr_mgr_event_get_pi_policy(event_id,pid,&pm_policy_cfg);
	if(!ret)
	{
		pm_policy_cfg.atl = (u32)val;
		ret = pwr_mgr_event_set_pi_policy(event_id,pid,&pm_policy_cfg);
	}

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(pwr_mgr_dbg_atl_ops, pwr_mgr_dbg_event_get_atl,
		pwr_mgr_dbg_event_set_atl, "%llu\n");


static int pwr_mgr_dbg_event_get_ac(void *data, u64* val)
{
	u32 event_id = ((u32)data & 0xFFFF0000) >> 16;
	u32 pid = (u32)data & 0xFFFF;
	int ret;
	struct pm_policy_cfg pm_policy_cfg;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS ||  pid >= pwr_mgr.info->num_pi);

	ret = pwr_mgr_event_get_pi_policy(event_id,pid,&pm_policy_cfg);

	if(!ret)
		*val = pm_policy_cfg.ac;
	return ret;
}
static int pwr_mgr_dbg_event_set_ac(void *data, u64 val)
{
	u32 event_id = ((u32)data & 0xFFFF0000) >> 16;
	u32 pid = (u32)data & 0xFFFF;
	int ret;
	struct pm_policy_cfg pm_policy_cfg;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS ||  pid >= pwr_mgr.info->num_pi);

	ret = pwr_mgr_event_get_pi_policy(event_id,pid,&pm_policy_cfg);
	if(!ret)
	{
		pm_policy_cfg.ac = (u32)val;
		ret = pwr_mgr_event_set_pi_policy(event_id,pid,&pm_policy_cfg);
	}

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(pwr_mgr_dbg_ac_ops, pwr_mgr_dbg_event_get_ac,
		pwr_mgr_dbg_event_set_ac, "%llu\n");


static int pwr_mgr_dbg_event_get_policy(void *data, u64* val)
{
	u32 event_id = ((u32)data & 0xFFFF0000) >> 16;
	u32 pid = (u32)data & 0xFFFF;
	int ret;
	struct pm_policy_cfg pm_policy_cfg;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS ||  pid >= pwr_mgr.info->num_pi);

	ret = pwr_mgr_event_get_pi_policy(event_id,pid,&pm_policy_cfg);

	if(!ret)
		*val = pm_policy_cfg.policy;
	return ret;
}
static int pwr_mgr_dbg_event_set_policy(void *data, u64 val)
{
	u32 event_id = ((u32)data & 0xFFFF0000) >> 16;
	u32 pid = (u32)data & 0xFFFF;
	int ret;
	struct pm_policy_cfg pm_policy_cfg;
	BUG_ON(event_id >= PWR_MGR_NUM_EVENTS ||  pid >= pwr_mgr.info->num_pi);

	ret = pwr_mgr_event_get_pi_policy(event_id,pid,&pm_policy_cfg);
	if(!ret)
	{
		pm_policy_cfg.policy = (u32)val;
		ret = pwr_mgr_event_set_pi_policy(event_id,pid,&pm_policy_cfg);
	}

	return ret;
}

DEFINE_SIMPLE_ATTRIBUTE(pwr_mgr_dbg_policy_ops, pwr_mgr_dbg_event_get_policy,
		pwr_mgr_dbg_event_set_policy, "%llu\n");

struct dentry *dent_pwr_root_dir = NULL;
int __init pwr_mgr_debug_init(u32 bmdm_pwr_base)
{
	struct dentry *dent_event_tbl;
	struct dentry *dent_pi;
	struct dentry *dent_event;
	struct pi* pi;
	int event;
	int i;

	if(unlikely(!pwr_mgr.info))
	{
		pwr_dbg("%s:ERROR - pwr mgr not initialized\n",__func__);
		return -EPERM;
	}

	dent_pwr_root_dir = debugfs_create_dir("power_mgr", 0);
	if (!dent_pwr_root_dir)
		return -ENOMEM;

	bmdm_pwr_mgr_base = bmdm_pwr_base;

    if(!debugfs_create_u32("debug", S_IWUSR|S_IRUSR, dent_pwr_root_dir, (int*)&pwr_debug))
		return -ENOMEM;
    /* Debug Bus control via Debugfs */
    if(!debugfs_create_file("pm_debug_bus", S_IWUSR|S_IRUSR, dent_pwr_root_dir, NULL, &set_pm_dbg_bus_fops))
		return -ENOMEM;
	if(!debugfs_create_file("bmdm_debug_bus", S_IWUSR|S_IRUSR, dent_pwr_root_dir, NULL, &set_bmdm_dbg_bus_fops))
		return -ENOMEM;

	dent_event_tbl = debugfs_create_dir("event_table", dent_pwr_root_dir);
    if(!dent_event_tbl)
		return -ENOMEM;
	for(event = 0; event < PWR_MGR_NUM_EVENTS;event++)
	{
		dent_event = debugfs_create_dir(PWRMGR_EVENT_ID_TO_STR(event), dent_event_tbl);
		if(!dent_event)
			return -ENOMEM;
		if(!debugfs_create_file("active", S_IWUSR|S_IRUSR, dent_event,(void*)event,&pwr_mgr_dbg_active_fops))
			return -ENOMEM;

		if(!debugfs_create_file("trig_type", S_IWUSR|S_IRUSR, dent_event,(void*)event,&pwr_mgr_dbg_trig_fops))
			return -ENOMEM;

		for(i =0; i < pwr_mgr.info->num_pi; i++)
		{
			pi = pi_mgr_get(i);
			BUG_ON(pi == NULL);
			dent_pi = debugfs_create_dir(pi_get_name(pi), dent_event);
			if(!dent_pi)
				return -ENOMEM;

			if(!debugfs_create_file("policy", S_IWUSR|S_IRUSR, dent_pi,(void*)(((u32)event << 16) | (u16)i),
							&pwr_mgr_dbg_policy_ops))
			return -ENOMEM;

			if(!debugfs_create_file("ac", S_IWUSR|S_IRUSR, dent_pi,(void*)(((u32)event << 16) | (u16)i),
				&pwr_mgr_dbg_ac_ops))
			return -ENOMEM;

			if(!debugfs_create_file("atl", S_IWUSR|S_IRUSR, dent_pi,(void*)(((u32)event << 16) | (u16)i),
				&pwr_mgr_dbg_atl_ops))
				return -ENOMEM;
		}

	}



    return 0;
}

#endif /*  CONFIG_DEBUG_FS  */


