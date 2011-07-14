/*****************************************************************************
*
* Kona generic clock framework
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <asm/clkdev.h>
#include <plat/clock.h>
#include <asm/io.h>
#include<plat/pi_mgr.h>

#ifdef CONFIG_SMP
#include <asm/cpu.h>
#endif

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

/* global spinlock for clock API */
static DEFINE_SPINLOCK(clk_lock);

int clk_debug = 1;

int clk_init(struct clk *clk)
{
	int ret = 0;
	unsigned long flags;

	if(!clk || !clk->ops)
		return -EINVAL;

	clk_dbg("%s - %s\n", __func__, clk->name);
	if(clk->ops->init)
	{
		clk_dbg("%s - calling init\n", __func__);
		spin_lock_irqsave(&clk_lock, flags);
		ret = clk->ops->init(clk);
		spin_unlock_irqrestore(&clk_lock, flags);
	}

	return ret;
}

int clk_register(struct clk_lookup *clk_lkup,int num_clks)
{
    int ret = 0;
	int i;

	for(i = 0; i < num_clks; i++)
	{
		clkdev_add(&clk_lkup[i]);
		clk_dbg("clock registered - %s\n",clk_lkup[i].clk->name);
	}
	for(i = 0; i < num_clks; i++)
		ret |= clk_init(clk_lkup[i].clk);
	return ret;
}

static int __clk_enable(struct clk *clk)
{
	int ret = 0;

	if(!clk)
		return -EINVAL;

	if(!clk->ops || !clk->ops->enable)
		return -EINVAL;
	clk_dbg("%s - %s\n",__func__, clk->name);
	ret = clk->ops->enable(clk, 1);

	return ret;
}

int clk_enable(struct clk *clk)
{
	int ret;
	unsigned long flags;

	if(IS_ERR_OR_NULL(clk))
		return -EINVAL;

	spin_lock_irqsave(&clk_lock, flags);
	ret = __clk_enable(clk);
	spin_unlock_irqrestore(&clk_lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_enable);

static void __clk_disable(struct clk *clk)
{
	if(!clk)
		return;
	if(!clk->ops || !clk->ops->enable)
		return;

	clk->ops->enable(clk, 0);
}

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	if(IS_ERR_OR_NULL(clk))
		return;

	spin_lock_irqsave(&clk_lock, flags);
	__clk_disable(clk);
	spin_unlock_irqrestore(&clk_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long flags, rate;

	if(IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->get_rate)
		return -EINVAL;

	spin_lock_irqsave(&clk_lock, flags);
	rate = clk->ops->get_rate(clk);
	spin_unlock_irqrestore(&clk_lock, flags);
	return rate;
}
EXPORT_SYMBOL(clk_get_rate);

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags, actual;

	if(IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->round_rate)
		return -EINVAL;

	spin_lock_irqsave(&clk_lock, flags);
	actual = clk->ops->round_rate(clk, rate);
	spin_unlock_irqrestore(&clk_lock, flags);

	return actual;
}
EXPORT_SYMBOL(clk_round_rate);

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	int ret;

	if(IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->set_rate)
		return -EINVAL;

	if(clk->use_cnt)
	{
		printk("%s is in enabled state. Disable before calling set_rate\n", clk->name);
		return -EBUSY;
	}

	spin_lock_irqsave(&clk_lock, flags);
	ret = clk->ops->set_rate(clk, rate);
	spin_unlock_irqrestore(&clk_lock, flags);

	return 0;
}
EXPORT_SYMBOL(clk_set_rate);



/*CCU access functions */
int ccu_write_access_enable(struct ccu_clk* ccu_clk, int enable)
{
	u32 reg_val = 0;


	reg_val = CLK_WR_ACCESS_PASSWORD << CLK_WR_PASSWORD_SHIFT;

	if(enable)
	{
		if(ccu_clk->write_access_en_count++ != 0)
			return 0;
		reg_val |= CLK_WR_ACCESS_EN;
	}
	else if(ccu_clk->write_access_en_count == 0 || --ccu_clk->write_access_en_count != 0)
		return 0;
	writel(reg_val, CCU_WR_ACCESS_REG(ccu_clk));

	return 0;
}
EXPORT_SYMBOL(ccu_write_access_enable);

int ccu_policy_engine_resume(struct ccu_clk* ccu_clk, int load_type)
{
	u32 reg_val = 0;

	if(ccu_clk->pol_engine_dis_cnt == 0)
		return 0; /*Already in running state ??*/

	else if(--ccu_clk->pol_engine_dis_cnt != 0)
		return 0; /*disable count is non-zero */

	/*Set trigger*/
	reg_val = readl(CCU_POLICY_CTRL_REG(ccu_clk));
	if(load_type == CCU_LOAD_ACTIVE)
		reg_val |= CCU_POLICY_CTL_ATL << CCU_POLICY_CTL_GO_ATL_SHIFT;
	reg_val |= CCU_POLICY_CTL_GO_TRIG << CCU_POLICY_CTL_GO_SHIFT;

	writel(reg_val,CCU_POLICY_CTRL_REG(ccu_clk));
	while(readl(CCU_POLICY_CTRL_REG(ccu_clk)) & CCU_POLICY_CTL_GO_MASK);

	reg_val = readl(CCU_POLICY_CTRL_REG(ccu_clk));
	if((load_type == CCU_LOAD_TARGET) && (reg_val & CCU_POLICY_CTL_TGT_VLD_MASK))
	{
		reg_val |= CCU_POLICY_CTL_TGT_VLD << CCU_POLICY_CTL_TGT_VLD_SHIFT;
		writel(reg_val,CCU_POLICY_CTRL_REG(ccu_clk));
	}

	return 0;
}
EXPORT_SYMBOL(ccu_policy_engine_resume);

int ccu_policy_engine_stop(struct ccu_clk* ccu_clk)
{
	u32 reg_val = 0;

	if(ccu_clk->pol_engine_dis_cnt++ != 0)
		return 0; /*Already in disabled state */

	reg_val = (CCU_POLICY_OP_EN < CCU_POLICY_CONFIG_EN_SHIFT);
	writel(reg_val,CCU_LVM_EN_REG(ccu_clk));
	while(readl(CCU_LVM_EN_REG(ccu_clk)) & CCU_POLICY_CONFIG_EN_MASK);

	return 0;
}
EXPORT_SYMBOL(ccu_policy_engine_stop);

int ccu_set_policy_ctrl(struct ccu_clk* ccu_clk, int pol_ctrl_id, int action)
{
	u32 reg_val = 0;
	u32 shift;

	switch(pol_ctrl_id)
	{
	case POLICY_CTRL_GO:
		shift = CCU_POLICY_CTL_GO_SHIFT;
		break;
	case POLICY_CTRL_GO_AC:
		shift = CCU_POLICY_CTL_GO_AC_SHIFT;
		break;
	case POLICY_CTRL_GO_ATL:
		shift = CCU_POLICY_CTL_GO_ATL_SHIFT;
		break;
	case POLICY_CTRL_TGT_VLD:
		shift = CCU_POLICY_CTL_TGT_VLD_SHIFT;
		break;
	default:
		return -EINVAL;
	}
	reg_val = readl(CCU_POLICY_CTRL_REG(ccu_clk));
	reg_val |= action << shift;

	writel(reg_val,CCU_POLICY_CTRL_REG(ccu_clk));
	return 0;
}
EXPORT_SYMBOL(ccu_set_policy_ctrl);

int ccu_int_enable(struct ccu_clk* ccu_clk, int int_type, int enable)
{
	u32 reg_val = 0;
	u32 shift;

	if(int_type == ACT_INT)
		shift = CCU_ACT_INT_SHIFT;
	else if(int_type == TGT_INT)
		shift = CCU_TGT_INT_SHIFT;
	else
		return -EINVAL;

	reg_val = readl(CCU_INT_EN_REG(ccu_clk));

	if(enable)
		reg_val |= (CCU_INT_EN << shift);
	else
		reg_val &= ~(CCU_INT_EN << shift);

	writel(reg_val,CCU_INT_EN_REG(ccu_clk));
	return 0;
}
EXPORT_SYMBOL(ccu_int_enable);

int ccu_int_status_clear(struct ccu_clk* ccu_clk,int int_type)
{
	u32 reg_val = 0;
	u32 shift;

	if(int_type == ACT_INT)
		shift = CCU_ACT_INT_SHIFT;
	else if(int_type == TGT_INT)
		shift = CCU_TGT_INT_SHIFT;
	else
		return -EINVAL;

	reg_val = readl(CCU_INT_STATUS_REG(ccu_clk));
	reg_val |= (CCU_INT_STATUS_CLR << shift);
	writel(reg_val,CCU_INT_STATUS_REG(ccu_clk));

	return 0;
}
EXPORT_SYMBOL(ccu_int_status_clear);

int ccu_set_freq_policy(struct ccu_clk* ccu_clk, int policy_id, int freq_id)
{
	u32 reg_val = 0;
	u32 shift;

	if(freq_id >= ccu_clk->freq_count)
		return -EINVAL;

	switch(policy_id)
	{
	case CCU_POLICY0:
		shift = CCU_FREQ_POLICY0_SHIFT;
		break;
	case CCU_POLICY1:
		shift = CCU_FREQ_POLICY1_SHIFT;
		break;
	case CCU_POLICY2:
		shift = CCU_FREQ_POLICY2_SHIFT;
		break;
	case CCU_POLICY3:
		shift = CCU_FREQ_POLICY3_SHIFT;
		break;
	default:
		return -EINVAL;
	}
	reg_val = readl(CCU_POLICY_FREQ_REG(ccu_clk));
	clk_dbg("%s: reg_val:%08x shift:%d\n",__func__, reg_val, shift);
	reg_val &= ~(CCU_FREQ_POLICY_MASK << shift);

	reg_val |= freq_id << shift;
	writel(reg_val, CCU_POLICY_FREQ_REG(ccu_clk));

	return 0;
}
EXPORT_SYMBOL(ccu_set_freq_policy);

int ccu_get_freq_policy(struct ccu_clk * ccu_clk, int policy_id)
{
	u32 shift, reg_val;

	switch(policy_id)
	{
	case CCU_POLICY0:
		shift = CCU_FREQ_POLICY0_SHIFT;
		break;
	case CCU_POLICY1:
		shift = CCU_FREQ_POLICY1_SHIFT;
		break;
	case CCU_POLICY2:
		shift = CCU_FREQ_POLICY2_SHIFT;
		break;
	case CCU_POLICY3:
		shift = CCU_FREQ_POLICY3_SHIFT;
		break;
	default:
		return CCU_FREQ_INVALID;

	}
	reg_val = readl(CCU_POLICY_FREQ_REG(ccu_clk));
	clk_dbg("%s: reg_val:%08x shift:%d\n",__func__, reg_val, shift);

	return ((reg_val >> shift) & CCU_FREQ_POLICY_MASK);
}
EXPORT_SYMBOL(ccu_get_freq_policy);

int ccu_set_peri_voltage(struct ccu_clk * ccu_clk, int peri_volt_id, u8 voltage)
{

	u32 shift, reg_val;

	if(peri_volt_id == VLT_NORMAL)
	{
		shift = CCU_PERI_VLT_NORM_SHIFT;
		ccu_clk->volt_peri[0] = voltage & CCU_PERI_VLT_MASK;
	}
	else if(peri_volt_id == VLT_HIGH)
	{
		shift = CCU_PERI_VLT_HIGH_SHIFT;
		ccu_clk->volt_peri[1] = voltage & CCU_PERI_VLT_MASK;
	}
	else
		return -EINVAL;

	reg_val = readl(CCU_VLT_PERI_REG(ccu_clk));
	reg_val  = (reg_val & ~(CCU_PERI_VLT_MASK << shift)) |
			   ((voltage & CCU_PERI_VLT_MASK) << shift);

	writel(reg_val,CCU_VLT_PERI_REG(ccu_clk));

	return 0;
}
EXPORT_SYMBOL(ccu_set_peri_voltage);

int ccu_set_voltage(struct ccu_clk * ccu_clk, int volt_id, u8 voltage)
{
	u32 shift, reg_val;
	u32 reg_addr;

	if(volt_id >= ccu_clk->freq_count)
		return -EINVAL;

	ccu_clk->freq_volt[volt_id] = voltage & CCU_VLT_MASK;
	switch(volt_id)
	{
	case CCU_VLT0:
		shift = CCU_VLT0_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT1:
		shift = CCU_VLT1_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT2:
		shift = CCU_VLT2_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT3:
		shift = CCU_VLT3_SHIFT;
		reg_addr = CCU_VLT0_3_REG(ccu_clk);
		break;
	case CCU_VLT4:
		shift = CCU_VLT4_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT5:
		shift = CCU_VLT5_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT6:
		shift = CCU_VLT6_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	case CCU_VLT7:
		shift = CCU_VLT7_SHIFT;
		reg_addr = CCU_VLT4_7_REG(ccu_clk);
		break;
	default:
		return -EINVAL;
	}
	reg_val =  readl(reg_addr);
	reg_val = (reg_val & ~(CCU_VLT_MASK << shift)) |
			  ((voltage & CCU_VLT_MASK) << shift);

	writel(reg_val, reg_addr);

	return 0;
}
EXPORT_SYMBOL(ccu_set_voltage);

void ccu_set_active_policy(struct ccu_clk * ccu_clk, u32 policy)
{
	ccu_clk->active_policy = policy;
}
EXPORT_SYMBOL(ccu_set_active_policy);

int ccu_get_active_policy(struct ccu_clk * ccu_clk)
{
	return ccu_clk->active_policy;
}
EXPORT_SYMBOL(ccu_get_active_policy);

/*Generic ccu ops functions*/

static int ccu_clk_enable(struct clk *clk, int enable)
{
	int ret = 0;
	struct ccu_clk * ccu_clk;

	if(clk->clk_type != CLK_TYPE_CCU)
		return -EPERM;

	ccu_clk = to_ccu_clk(clk);

	clk_dbg("%s enable: %d, ccu name:%s\n",__func__, enable, clk->name);


	if(enable)
	{
		clk->use_cnt++;
	}
	else
	{
		//BUG_ON(clk->use_cnt == 0);
		if(!clk->use_cnt)
			return 0;
		clk->use_cnt--;
	}

	if(ccu_clk->pi_id != -1)
	{
#ifndef CONFIG_ARCH_SAMOA
		struct pi* pi = pi_mgr_get(ccu_clk->pi_id);

		BUG_ON(!pi);
		pi_enable(pi,enable);
#endif
	}
	return ret;
}


static int ccu_clk_init(struct clk* clk)
{
	struct ccu_clk * ccu_clk;
	int inx;
	u32 reg_val;
	if(clk->clk_type != CLK_TYPE_CCU)
		return -EPERM;

	if(clk->init)
		return 0;

	ccu_clk = to_ccu_clk(clk);

	BUG_ON (ccu_clk->freq_count > MAX_CCU_FREQ_COUNT);
	clk_dbg("%s - %s\n",__func__, clk->name);

	ccu_clk->write_access_en_count = 0;

	/* enable write access*/
	ccu_write_access_enable(ccu_clk,true);
	/*stop policy engine */
	ccu_policy_engine_stop(ccu_clk);

	/*Enabel ALl policy mask by default --  TBD- SHOULD WE DO THIS ????*/
	reg_val = CCU_POLICY_MASK_ENABLE_ALL_MASK;

	for(inx = CCU_POLICY0; inx <= CCU_POLICY3; inx++)
	{
		if(ccu_clk->policy_mask1_offset)
			writel(reg_val,(CCU_POLICY_MASK1_REG(ccu_clk) +  4 * inx));

		if(ccu_clk->policy_mask2_offset)
			writel(reg_val,(CCU_POLICY_MASK2_REG(ccu_clk) +  4 * inx));
	}


	/*Init voltage table */
	for (inx = 0; inx < ccu_clk->freq_count; inx++)
	{
		ccu_set_voltage(ccu_clk,inx,ccu_clk->freq_volt[inx]);
	}
	/*Init peri voltage table  */
	for (inx = 0; inx < MAX_CCU_PERI_VLT_COUNT; inx++)
	{
		ccu_set_peri_voltage(ccu_clk,inx,ccu_clk->volt_peri[inx]);
	}

	/*Init freq policy */
	for (inx = 0; inx < MAX_CCU_POLICY_COUNT; inx++)
	{
		BUG_ON(ccu_clk->freq_policy[inx] >= ccu_clk->freq_count);
		ccu_set_freq_policy(ccu_clk,inx,ccu_clk->freq_policy[inx]);
	}
	/*Set ATL & AC */
	if(clk->flags & CCU_TARGET_LOAD)
	{
		if(clk->flags & CCU_TARGET_AC)
			ccu_set_policy_ctrl(ccu_clk, POLICY_CTRL_GO_AC, CCU_AUTOCOPY_ON);
		ccu_policy_engine_resume(ccu_clk, CCU_LOAD_TARGET);
	}
	else
		ccu_policy_engine_resume(ccu_clk, CCU_LOAD_ACTIVE);
	/* disable write access*/
	ccu_write_access_enable(ccu_clk, false);

	clk->init = 1;

	if(ccu_clk->pi_id != -1)
	{
#ifndef CONFIG_ARCH_SAMOA

		struct pi* pi = pi_mgr_get(ccu_clk->pi_id);
		BUG_ON(!pi);
		pi_init(pi);
#endif
	}

	return 0;
}

struct gen_clk_ops gen_ccu_clk_ops =
{
	.init		= 	ccu_clk_init,
	.enable		=	ccu_clk_enable,
};


int peri_clk_set_policy_mask(struct peri_clk * peri_clk, int policy_id, int mask)
{
	u32 reg_val;
	u32 policy_offset = 0;

	clk_dbg("%s\n",__func__);
	if(!peri_clk->ccu_clk)
	{
		BUG_ON(1);
		return -EINVAL;
	}
	if(peri_clk->mask_set == 1)
	{
		policy_offset = peri_clk->ccu_clk->policy_mask1_offset;
	}
	else if(peri_clk->mask_set == 2)
	{
		policy_offset = peri_clk->ccu_clk->policy_mask2_offset;
	}
	else
		return -EINVAL;

	if(!policy_offset)
		return -EINVAL;


	policy_offset = policy_offset + (4 * policy_id);

	clk_dbg("%s offset: %08x, mask: %08x, bit_mask: %d\n",__func__, policy_offset, mask, peri_clk->policy_bit_mask);
	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, policy_offset));
	if(mask)
		reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->policy_bit_mask);
	else
		reg_val = RESET_BIT_USING_MASK(reg_val, peri_clk->policy_bit_mask);

	clk_dbg("%s writing %08x to %08x\n",__func__, reg_val,	peri_clk->ccu_clk->ccu_clk_mgr_base + policy_offset);
	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, policy_offset));

	return 0;
}
EXPORT_SYMBOL(peri_clk_set_policy_mask);

static int peri_clk_get_policy_mask(struct peri_clk * peri_clk, int policy_id)
{
	u32 policy_offset = 0;
	u32 reg_val;

	if(!peri_clk->ccu_clk)
	{
		BUG_ON(1);
		return -EINVAL;
	}
	BUG_ON(policy_id < CCU_POLICY0 || policy_id > CCU_POLICY3);
	if(peri_clk->mask_set == 1)
	{
		if(!peri_clk->ccu_clk->policy_mask1_offset)
			return -EINVAL;
		policy_offset = peri_clk->ccu_clk->policy_mask1_offset;
	}
	else if(peri_clk->mask_set == 2)
	{
		if(!peri_clk->ccu_clk->policy_mask2_offset)
			return -EINVAL;
		policy_offset = peri_clk->ccu_clk->policy_mask2_offset;
	}
	else
		return -EINVAL;

	policy_offset = policy_offset + 4 * policy_id;

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, policy_offset));
	return GET_BIT_USING_MASK(reg_val, peri_clk->policy_bit_mask);

}
EXPORT_SYMBOL(peri_clk_get_policy_mask);

static int peri_clk_get_gating_ctrl(struct peri_clk * peri_clk)
{
	u32 reg_val;

	if(!peri_clk->clk_gate_offset || !peri_clk->gating_sel_mask)
		return -EINVAL;

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,	peri_clk->clk_gate_offset));

	return GET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);
}
EXPORT_SYMBOL(peri_clk_get_gating_ctrl);

static int peri_clk_set_gating_ctrl(struct peri_clk * peri_clk, int  gating_ctrl)
{
	u32 reg_val;

	if(gating_ctrl != CLK_GATING_AUTO && gating_ctrl != CLK_GATING_SW)
		return -EINVAL;
	if(!peri_clk->clk_gate_offset || !peri_clk->gating_sel_mask)
		return -EINVAL;

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,	peri_clk->clk_gate_offset));
	if(gating_ctrl == CLK_GATING_SW)
		reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);
	else
		reg_val = RESET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);

	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	return 0;
}
EXPORT_SYMBOL(peri_clk_set_gating_ctrl);

static int peri_clk_get_pll_select(struct peri_clk * peri_clk)
{
	u32 reg_val;

	if(!peri_clk->clk_div.pll_select_offset || !peri_clk->clk_div.pll_select_mask)
		return -EINVAL;

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_div.pll_select_offset));

	return GET_VAL_USING_MASK_SHIFT(reg_val,
									peri_clk->clk_div.pll_select_mask, peri_clk->clk_div.pll_select_shift);
}
EXPORT_SYMBOL(peri_clk_get_pll_select);

static int peri_clk_set_pll_select(struct peri_clk * peri_clk, int source)
{
	u32 reg_val;
	if(!peri_clk->clk_div.pll_select_offset ||
			!peri_clk->clk_div.pll_select_mask || source >= peri_clk->src_clk.count)
		return -EINVAL;

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_div.pll_select_offset));
	reg_val = SET_VAL_USING_MASK_SHIFT(reg_val, peri_clk->clk_div.pll_select_mask,
									   peri_clk->clk_div.pll_select_shift, source);
	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk,	peri_clk->clk_div.pll_select_offset));

	return 0;
}
EXPORT_SYMBOL(peri_clk_set_pll_select);

static int peri_clk_hyst_enable(struct peri_clk * peri_clk, int enable, int delay)
{
	u32 reg_val;

	if(!peri_clk->clk_gate_offset || !peri_clk->hyst_val_mask || !peri_clk->hyst_en_mask)
		return -EINVAL;

	if(enable)
	{
		if(delay != CLK_HYST_LOW && delay != CLK_HYST_HIGH)
			return -EINVAL;
	}

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	if(enable)
	{
		reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->hyst_en_mask);
		if(delay == CLK_HYST_HIGH)
			reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->hyst_val_mask);
		else
			reg_val = RESET_BIT_USING_MASK(reg_val, peri_clk->hyst_val_mask);
	}
	else
		reg_val = RESET_BIT_USING_MASK(reg_val, peri_clk->hyst_en_mask);

	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	return 0;
}
EXPORT_SYMBOL(peri_clk_hyst_enable);

static int peri_clk_get_gating_status(struct peri_clk * peri_clk)
{
	u32 reg_val;

	if(!peri_clk->clk_gate_offset || !peri_clk->stprsts_mask)
		return -EINVAL;
	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	return GET_BIT_USING_MASK(reg_val, peri_clk->stprsts_mask);
}
EXPORT_SYMBOL(peri_clk_get_gating_status);

static int peri_clk_set_voltage_lvl(struct peri_clk * peri_clk, int voltage_lvl)
{
	u32 reg_val;

	if(!peri_clk->clk_gate_offset || !peri_clk->volt_lvl_mask)
		return -EINVAL;
	if(voltage_lvl != VLT_NORMAL && voltage_lvl != VLT_HIGH)
		return -EINVAL;

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	if(voltage_lvl == VLT_HIGH)
		reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->volt_lvl_mask);
	else
		reg_val = RESET_BIT_USING_MASK(reg_val, peri_clk->volt_lvl_mask);

	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk,	peri_clk->clk_gate_offset));

	return 0;
}
EXPORT_SYMBOL(peri_clk_set_voltage_lvl);

static int peri_clk_enable(struct clk* clk, int enable)
{
	u32 reg_val;
	u32 bit_val;
	struct peri_clk * peri_clk;
	int inx;
	struct clk* src_clk;

	if(clk->clk_type != CLK_TYPE_PERI)
	{
		BUG_ON(1);
		return -EPERM;
	}

	peri_clk = to_peri_clk(clk);
	clk_dbg("%s:%d, clock name: %s \n",__func__,enable, clk->name);

	BUG_ON(!peri_clk->ccu_clk || (peri_clk->clk_gate_offset == 0));
	BUG_ON( !peri_clk->clk_en_mask && !CLK_FLG_ENABLED(clk,AUTO_GATE));

	if((enable) && !(clk->flags & DONOT_NOTIFY_STATUS_TO_CCU))
	    peri_clk->ccu_clk->clk.ops->enable(&peri_clk->ccu_clk->clk, 1);

	/*Make sure that all dependent & src clks are enabled/disabled*/
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++)
	{
		if(clk->dep_clks[inx]->ops && clk->dep_clks[inx]->ops->enable)
		{
			clk_dbg("%s, %s dependant clock %s \n",__func__,
				enable?"Enabling":"Disabling",clk->dep_clks[inx]->name);
			clk->dep_clks[inx]->ops->enable(clk->dep_clks[inx],enable);
		}
	}

	if(PERI_SRC_CLK_VALID(peri_clk))
	{
		src_clk = GET_PERI_SRC_CLK(peri_clk);
		BUG_ON(!src_clk);
		if(src_clk->ops && src_clk->ops->enable)
			src_clk->ops->enable(src_clk,enable);
		clk_dbg("%s, after %s source clock \n",__func__,
		enable?"enabling":"disabling");
	}
	/* enable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, true);

	clk_dbg("%s:%s use count = %d\n",__func__,clk->name,peri_clk->clk.use_cnt);
	if(enable)
	{
		/*Increment usage count... return if already enabled*/
		if(peri_clk->clk.use_cnt++ != 0)
			goto enable_done;
		bit_val = 1;
		/*TBD - MAY NEED TO REVISIT*/
		if(clk->flags & ENABLE_HVT)
			peri_clk_set_voltage_lvl(peri_clk,VLT_HIGH);
	}
	else
	{
		/*decrment usage count... return if already disabled or usage count is non-zero*/
		if(peri_clk->clk.use_cnt == 0 || --peri_clk->clk.use_cnt != 0)
			goto enable_done;

		/*MAY NEED TO REVISIT*/
		if(clk->flags & ENABLE_HVT)
			peri_clk_set_voltage_lvl(peri_clk,VLT_NORMAL);
		bit_val = 0;
	}

	if(clk->flags & AUTO_GATE)
	{
		clk_dbg("%s:%s: is auto gated\n",__func__, clk->name);
		goto enable_done;
	}

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,peri_clk->clk_gate_offset));
	clk_dbg("%s, Before change clk_gate reg value: %08x  \n",__func__, reg_val);
	if(bit_val)
		reg_val = reg_val | peri_clk->clk_en_mask;
	else
		reg_val = reg_val & ~peri_clk->clk_en_mask;
	clk_dbg("%s, writing %08x to clk_gate reg\n",__func__, reg_val);
	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	clk_dbg("%s:%s clk before stprsts start\n",__func__,clk->name);
	if(enable)
		while(!(readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset))
				& peri_clk->stprsts_mask));
	else
		while((readl(CCU_REG_ADDR(peri_clk->ccu_clk,
								  peri_clk->clk_gate_offset))) & peri_clk->stprsts_mask);
	clk_dbg("%s:%s clk after stprsts start\n",__func__,clk->name);
	clk_dbg("%s, %s is %s..! \n",__func__, clk->name, enable?"enabled":"disabled");

enable_done:
	/* disable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk,false);
	if(!enable && !(clk->flags & DONOT_NOTIFY_STATUS_TO_CCU))
	    peri_clk->ccu_clk->clk.ops->enable(&peri_clk->ccu_clk->clk, 0);
	return 0;
}

static u32 compute_rate(u32 rate, u32 div, u32 dither,u32 max_dither,u32 pre_div)
{
	u32 res  = rate;
	res /= (pre_div+1);
	/* rate/(X + 1 + Y / 2^n)
	= (rate*2^n) / (2^n(X+1) + Y)
	==> 2^n = max_dither+1 */
	clk_dbg("%s:src_rate = %d,div = %d, dither = %d,Max_dither = %d,pre_div = %d\n",
		__func__, rate, div, dither, max_dither, pre_div);
	res = ((res/100)*(max_dither + 1))/ (((max_dither + 1)*(div+1)) + dither);
	clk_dbg("%s:result = %d\n",__func__,res);
	return res*100;

}


static u32 peri_clk_calculate_div(struct peri_clk * peri_clk, u32 rate, u32* div,int* pre_div, int * src_clk_inx)
{
	u32 s,src = 0;
	u32 d,div_int = 0;
	u32 d_d,div_frac = 0;
	u32 pd, pre_d = 0;
	u32 diff = 0xFFFFFFFF;
	u32 temp_rate, temp_diff;
	u32 new_rate = 0;
	u32 src_clk_rate;

	u32 max_div = 0;
	u32 max_diether = 0;
	u32 max_pre_div = 0;
	struct clk_div* clk_div = &peri_clk->clk_div;
	struct src_clk* src_clk = &peri_clk->src_clk;


	if(clk_div->div_offset && clk_div->div_mask)
		max_div = clk_div->div_mask >> clk_div->div_shift;
	max_diether = ~(0xFFFFFFFF << clk_div->diether_bits);

	if(clk_div->pre_div_offset && clk_div->pre_div_mask)
		max_pre_div = clk_div->pre_div_mask >> clk_div->pre_div_shift;

	for(s = 0;s < src_clk->count; s++)
	{
		d	= 0;
		d_d	= 0;
		pd	= 0;

		BUG_ON(src_clk->clk[s]->ops == NULL ||
			src_clk->clk[s]->ops->get_rate == NULL);
		src_clk_rate =  src_clk->clk[s]->ops->get_rate(src_clk->clk[s]);

		if(rate > src_clk_rate)
			continue;

		else if(src_clk_rate == rate)
		{
			src = s;
			div_int = d;
			div_frac = d_d;
			pre_d = pd;
			new_rate = rate;
			diff = 0;
			break;
		}

		for(; d <= max_div; d++)
		{
			d_d = 0;
			pd = 0;

			temp_rate = compute_rate(src_clk_rate, d, d_d, max_diether, pd);
			if(temp_rate == rate)
			{
				src = s;
				div_int = d;
				div_frac = d_d;
				pre_d = pd;
				new_rate = rate;
				diff = 0;
				goto exit;
			}

			temp_diff = abs(temp_rate - rate);
			if(temp_diff < diff)
			{
				diff = temp_diff;
				src = s;
				div_int = d;
				div_frac = d_d;
				pre_d = pd;
				new_rate = temp_rate;
			}
			else if(temp_rate < rate && temp_diff > diff)
				break;

			for(; pd <= max_pre_div; pd++)
			{
				d_d = 0;

				temp_rate = compute_rate(src_clk_rate, d, d_d, max_diether, pd);
				if(temp_rate == rate)
				{
					src = s;
					div_int = d;
					div_frac = d_d;
					pre_d = pd;
					new_rate = rate;
					diff = 0;
					goto exit;
				}

				temp_diff = abs(temp_rate - rate);
				if(temp_diff < diff)
				{
					diff = temp_diff;
					src = s;
					div_int = d;
					div_frac = d_d;
					pre_d = pd;
					new_rate = temp_rate;
				}
				else if(temp_rate < rate && temp_diff > diff)
					break;

				for(d_d = 1; d_d <= max_diether; d_d++)
				{
					temp_rate = compute_rate(src_clk_rate, d, d_d, max_diether, pd);
					if(temp_rate == rate)
					{
						src = s;
						div_int = d;
						div_frac = d_d;
						pre_d = pd;
						new_rate = rate;
						diff = 0;
						goto exit;
					}

					temp_diff = abs(temp_rate - rate);
					if(temp_diff < diff)
					{
						diff = temp_diff;
						src = s;
						div_int = d;
						div_frac = d_d;
						pre_d = pd;
						new_rate = temp_rate;
					}
					else if(temp_rate < rate && temp_diff > diff)
						break;

				}

			}

		}
	}

exit:

	if(div)
	{
		*div = div_int;
		clk_dbg("div: %08x\n", *div);
		if(max_diether)
			*div = ((*div) << clk_div->diether_bits) | div_frac;
		clk_dbg("div: %08x, div_frac:%08x\n", *div, div_frac);
	}
	if(pre_div)
		*pre_div = pre_d;

	if(src_clk_inx)
		*src_clk_inx = src;

	return new_rate;

}

static int peri_clk_set_rate(struct clk* clk, u32 rate)
{
	struct peri_clk * peri_clk;
	u32 new_rate, reg_val;
	u32 div, pre_div, src;
	struct clk_div* clk_div;

	if(clk->clk_type != CLK_TYPE_PERI)
		return -EPERM;

	peri_clk = to_peri_clk(clk);

	clk_dbg("%s : %s\n",
			__func__, clk->name);

	if(CLK_FLG_ENABLED(clk,RATE_FIXED))
	{
		clk_dbg("%s : %s - fixed rate clk...rate cannot be changed\n",
			__func__, clk->name);
		return -EINVAL;

	}

	clk_div = &peri_clk->clk_div;

	new_rate = peri_clk_calculate_div(peri_clk,rate,&div,&pre_div,&src);

	if(abs(rate - new_rate) > CLK_RATE_MAX_DIFF)
		return -EINVAL;
	clk_dbg("%s src_rate %u sel %d div %u pre_div %u new_rate %u\n",
			__func__, peri_clk->src_clk.clk[src]->rate, src, div, pre_div, new_rate);

	/* enable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, true);

	/*Write DIV*/
	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_offset));
	reg_val = SET_VAL_USING_MASK_SHIFT(reg_val, clk_div->div_mask, clk_div->div_shift, div);
	clk_dbg("reg_val: %08x, div_offset:%08x, div_mask:%08x, div_shift:%08x, div:%08x\n",
			reg_val, clk_div->div_offset, clk_div->div_mask, clk_div->div_shift, div);
	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_offset));

	if(clk_div->pre_div_offset && clk_div->pre_div_mask)
	{
		reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->pre_div_offset));
		reg_val = SET_VAL_USING_MASK_SHIFT(reg_val, clk_div->pre_div_mask, clk_div->pre_div_shift, pre_div);
		writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->pre_div_offset));
	}
	/*set the source clock selected */
	peri_clk_set_pll_select(peri_clk, src);

	clk_dbg("Before trigger clock \n");
	if(clk_div->div_trig_offset && clk_div->div_trig_mask)
	{
		reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_trig_offset));
		clk_dbg("DIV: tigger offset: %08x, reg_value: %08x trig_mask:%08x\n",
				clk_div->div_trig_offset, reg_val, clk_div->div_trig_mask);
		reg_val = SET_BIT_USING_MASK(reg_val, clk_div->div_trig_mask);
		writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_trig_offset));
		do
		{
			reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_trig_offset));
			clk_dbg("reg_val: %08x, trigger bit: %08x\n", reg_val, GET_BIT_USING_MASK(reg_val, clk_div->div_trig_mask));
		}
		while(!(GET_BIT_USING_MASK(reg_val, clk_div->div_trig_mask)));
	}
	if(clk_div->prediv_trig_offset && clk_div->prediv_trig_mask)
	{
		reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->prediv_trig_offset));
		clk_dbg("PERDIV: tigger offset: %08x, reg_value: %08x trig_mask:%08x\n",
				clk_div->div_trig_offset, reg_val, clk_div->div_trig_mask);
		reg_val = SET_BIT_USING_MASK(reg_val, clk_div->prediv_trig_mask);
		writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->prediv_trig_offset));
		do
		{
			reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,	clk_div->prediv_trig_offset));
			clk_dbg("reg_val: %08x, trigger bit: %08x\n", reg_val, GET_BIT_USING_MASK(reg_val, clk_div->prediv_trig_mask));
		}
		while(!(GET_BIT_USING_MASK(reg_val, clk_div->prediv_trig_mask)));
	}
	/* disable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk,false);

	clk_dbg("clock set rate done \n");
	return 0;
}

static int peri_clk_init(struct clk* clk)
{
	struct peri_clk * peri_clk;
	struct src_clk * src_clks;
	unsigned int need_status_update = 0;
	int inx;

	if(clk->clk_type != CLK_TYPE_PERI)
		return -EPERM;

	if(clk->init)
		return 0;

	peri_clk = to_peri_clk(clk);
	BUG_ON(peri_clk->ccu_clk == NULL);

	clk_dbg("%s, clock name: %s \n",__func__, clk->name);
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++)
	{
		if(clk->dep_clks[inx]->ops && clk->dep_clks[inx]->ops->init)
			clk->dep_clks[inx]->ops->init(clk->dep_clks[inx]);
	}
	/*Init source clocks */
	/*enable/disable src clk*/
	BUG_ON(!PERI_SRC_CLK_VALID(peri_clk) && peri_clk->clk_div.pll_select_offset);

	/* enable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, true);
	if(peri_clk_get_gating_status(peri_clk) == 1)
	    clk->use_cnt = 1;

	if(PERI_SRC_CLK_VALID(peri_clk))
	{
		src_clks = &peri_clk->src_clk;
		for(inx =0; inx < src_clks->count; inx++)
		{
			if(src_clks->clk[inx]->ops && src_clks->clk[inx]->ops->init)
				src_clks->clk[inx]->ops->init(src_clks->clk[inx]);
		}
		/*set the default src clock*/
		BUG_ON(peri_clk->src_clk.src_inx >= peri_clk->src_clk.count);
		peri_clk_set_pll_select(peri_clk,peri_clk->src_clk.src_inx);
	}


	peri_clk_set_voltage_lvl(peri_clk,VLT_NORMAL);
	peri_clk_hyst_enable(peri_clk,HYST_ENABLE & clk->flags,
						 (clk->flags & HYST_HIGH) ? CLK_HYST_HIGH: CLK_HYST_LOW);

	if(clk->flags & AUTO_GATE)
		peri_clk_set_gating_ctrl(peri_clk, CLK_GATING_AUTO);
	else
		peri_clk_set_gating_ctrl(peri_clk, CLK_GATING_SW);

	clk_dbg("%s: before setting the mask\n",__func__);
	/*This is temporary, if PM initializes the policy mask of each clock then
	* this can be removed. */
	/*stop policy engine */
	ccu_policy_engine_stop(peri_clk->ccu_clk);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY0, peri_clk->policy_mask_init[0]);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY1,	peri_clk->policy_mask_init[1]);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY2,	peri_clk->policy_mask_init[2]);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY3,	peri_clk->policy_mask_init[3]);
	/*start policy engine */
	ccu_policy_engine_resume(peri_clk->ccu_clk, CCU_LOAD_ACTIVE);

	BUG_ON(CLK_FLG_ENABLED(clk,ENABLE_ON_INIT) && CLK_FLG_ENABLED(clk,DISABLE_ON_INIT));

	if(CLK_FLG_ENABLED(clk,ENABLE_ON_INIT))
	{
		if(clk->ops && clk->ops->enable)
		{
			clk->ops->enable(clk, 1);
		}
	}

	else if(CLK_FLG_ENABLED(clk,DISABLE_ON_INIT))
	{
		if(clk->ops->enable)
		{
			clk->ops->enable(clk, 0);
		}
	}else
	    need_status_update = 1;

	if(peri_clk_get_gating_status(peri_clk) == 1) {
	    clk->use_cnt = 1;
	    if (need_status_update && !(clk->flags & DONOT_NOTIFY_STATUS_TO_CCU))
		peri_clk->ccu_clk->clk.ops->enable(&peri_clk->ccu_clk->clk, 1);
	}
	/* Disable write access*/
	ccu_write_access_enable(peri_clk->ccu_clk, false);
	clk->init = 1;

	return 0;
}

static unsigned long peri_clk_round_rate(struct clk *clk, unsigned long rate)
{
	u32 new_rate;
	struct peri_clk *peri_clk;
	struct clk_div *clk_div;

	if(clk->clk_type != CLK_TYPE_PERI)
		return -EPERM;

	peri_clk = to_peri_clk(clk);

	clk_div = &peri_clk->clk_div;

	new_rate = peri_clk_calculate_div(peri_clk,rate,NULL,NULL,NULL);
	clk_dbg("%s:rate = %d\n", __func__, new_rate);

	return new_rate;
}

static unsigned long peri_clk_get_rate(struct clk *clk)
{
	struct peri_clk *peri_clk;
	int sel = 0;
	u32 div = 0, pre_div = 0;
	u32 reg_val = 0;
	u32 max_diether;
	struct clk_div *clk_div;
	u32 parent_rate, dither = 0;
	struct clk* src_clk;

	if(clk->clk_type != CLK_TYPE_PERI)
		return -EPERM;
	peri_clk = to_peri_clk(clk);

	if(CLK_FLG_ENABLED(clk,RATE_FIXED))
	{
		clk_dbg("%s : %s - fixed rate clk...\n",
			__func__, clk->name);
		return clk->rate;

	}

	clk_div = &peri_clk->clk_div;
	if(clk_div->div_offset && clk_div->div_mask)
	{
		reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_offset));
		clk_dbg("div_offset:%08x reg_val:%08x \n", clk_div->div_offset, reg_val);
		div = GET_VAL_USING_MASK_SHIFT(reg_val, clk_div->div_mask, clk_div->div_shift);
	}
	if(clk_div->diether_bits)
	{
		clk_dbg("div:%u, dither mask: %08x", div, ~(0xFFFFFFFF << clk_div->diether_bits));
		dither = div & ~(0xFFFFFFFF << clk_div->diether_bits);
		div = div >> clk_div->diether_bits;
		clk_dbg("dither: %u div : %u\n", dither, div);
	}
	if(clk_div->pre_div_offset && clk_div->pre_div_mask)
	{
		reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->pre_div_offset));
		pre_div = GET_VAL_USING_MASK_SHIFT(reg_val, clk_div->pre_div_mask, clk_div->pre_div_shift);
		clk_dbg("pre div : %u\n", pre_div);
	}
	if(clk_div->pll_select_offset && clk_div->pll_select_mask)
	{
		reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_offset));
		sel = GET_VAL_USING_MASK_SHIFT(reg_val, clk_div->pll_select_mask, clk_div->pll_select_shift);
		clk_dbg("pll_sel : %u\n", sel);
	}

	BUG_ON(sel >= peri_clk->src_clk.count);
	peri_clk->src_clk.src_inx = sel;
	src_clk = GET_PERI_SRC_CLK(peri_clk);
	BUG_ON(!src_clk || !src_clk->ops || !src_clk->ops->get_rate);
	parent_rate = src_clk->ops->get_rate(src_clk);
	max_diether = ~(0xFFFFFFFF << clk_div->diether_bits);

	clk->rate = compute_rate(parent_rate, div, dither, max_diether, pre_div);

	clk_dbg("%s src_rate %u sel %d div %u pre_div %u dither %u rate %u\n",__func__,
			peri_clk->src_clk.clk[sel]->rate, sel, div, pre_div, dither, clk->rate);

	return clk->rate;
}

struct gen_clk_ops gen_peri_clk_ops =
{
	.init		= 	peri_clk_init,
	.enable		=	peri_clk_enable,
	.set_rate	=	peri_clk_set_rate,
	.get_rate	=	peri_clk_get_rate,
	.round_rate	=	peri_clk_round_rate,
};

static int ref_clk_get_gating_status(struct ref_clk *ref_clk)
{
	u32 reg_val;

	if(!ref_clk->clk_gate_offset || !ref_clk->stprsts_mask)
		return -EINVAL;
	reg_val = readl(CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));

	return GET_BIT_USING_MASK(reg_val, ref_clk->stprsts_mask);
}
EXPORT_SYMBOL(ref_clk_get_gating_status);


int bus_clk_get_gating_ctrl(struct bus_clk * bus_clk)
{
	u32 reg_val;

	if(!bus_clk->clk_gate_offset || !bus_clk->gating_sel_mask)
		return -EINVAL;

	reg_val = readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	return GET_BIT_USING_MASK(reg_val, bus_clk->gating_sel_mask);
}
EXPORT_SYMBOL(bus_clk_get_gating_ctrl);

static int bus_clk_set_gating_ctrl(struct bus_clk * bus_clk, int  gating_ctrl)
{
	u32 reg_val;

	if(!bus_clk->clk_gate_offset || !bus_clk->gating_sel_mask)
		return -EINVAL;

	if(gating_ctrl != CLK_GATING_AUTO && gating_ctrl != CLK_GATING_SW)
		return -EINVAL;

	reg_val = readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));
	if(CLK_GATING_SW)
		reg_val = SET_BIT_USING_MASK(reg_val, bus_clk->gating_sel_mask);
	else
		reg_val = RESET_BIT_USING_MASK(reg_val, bus_clk->gating_sel_mask);
	writel(reg_val, CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	return 0;
}
EXPORT_SYMBOL(bus_clk_set_gating_ctrl);

static int bus_clk_get_gating_status(struct bus_clk *bus_clk)
{
	u32 reg_val;

	if(!bus_clk->clk_gate_offset || !bus_clk->stprsts_mask)
		return -EINVAL;
	reg_val = readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	return GET_BIT_USING_MASK(reg_val, bus_clk->stprsts_mask);
}
EXPORT_SYMBOL(bus_clk_get_gating_status);

static int bus_clk_hyst_enable(struct bus_clk * bus_clk, int enable, int delay)
{
	u32 reg_val;

	if(!bus_clk->clk_gate_offset || !bus_clk->hyst_val_mask || !bus_clk->hyst_en_mask)
		return -EINVAL;

	if(enable)
	{
		if(delay != CLK_HYST_LOW && delay != CLK_HYST_HIGH)
			return -EINVAL;
	}

	reg_val = readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	if(enable)
	{
		reg_val = SET_BIT_USING_MASK(reg_val, bus_clk->hyst_en_mask);
		if(delay == CLK_HYST_HIGH)
			reg_val = SET_BIT_USING_MASK(reg_val, bus_clk->hyst_val_mask);
		else
			reg_val = RESET_BIT_USING_MASK(reg_val, bus_clk->hyst_val_mask);
	}
	else
		reg_val = RESET_BIT_USING_MASK(reg_val, bus_clk->hyst_en_mask);

	writel(reg_val, CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));
	return 0;
}
EXPORT_SYMBOL(bus_clk_hyst_enable);

/* bus clocks */
static int bus_clk_enable(struct clk *clk, int enable)
{
	struct bus_clk *bus_clk;
	u32 reg_val;
	u32 bit_val;
	int inx;

	if(clk->clk_type != CLK_TYPE_BUS)
	{
		BUG_ON(1);
		return -EPERM;
	}
	clk_dbg("%s -- %s to be %s\n", __func__, clk->name, enable?"enabled":"disabled");

	bus_clk = to_bus_clk(clk);

	if(enable && !(clk->flags & AUTO_GATE) && (clk->flags & NOTIFY_STATUS_TO_CCU))
	    bus_clk->ccu_clk->clk.ops->enable(&bus_clk->ccu_clk->clk, 1);
	if((bus_clk->clk_gate_offset == 0) || (bus_clk->clk_en_mask == 0))
			return -EPERM;
			/*Make sure that all dependent & src clks are enabled/disabled*/
			for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++)
	{
		if(clk->dep_clks[inx]->ops && clk->dep_clks[inx]->ops->enable)
				clk->dep_clks[inx]->ops->enable(clk->dep_clks[inx],enable);
		}

	clk_dbg("%s : %s use cnt= %d\n",__func__,clk->name,	clk->use_cnt);
	if(enable)
{
	/*Increment usage count... return if already enabled*/
	if(clk->use_cnt++ != 0)
			goto enable_done;
		bit_val =  1;
	}
	else
	{
		/*decrment usage count... return if already disabled or usage count is non-zero*/
		if(clk->use_cnt == 0 || --clk->use_cnt != 0)
			goto enable_done;
		bit_val =  0;
	}

	if(clk->flags & AUTO_GATE)
	{
		clk_dbg("%s:%s: is auto gated\n",__func__, clk->name);
		goto enable_done;
	}

	/* enable write access*/
	ccu_write_access_enable(bus_clk->ccu_clk, true);


	reg_val = readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));
	clk_dbg("gate offset: %08x, reg_val: %08x, bit_val:%u\n", bus_clk->clk_gate_offset, reg_val, bit_val);
	if(bit_val)
	reg_val = reg_val | bus_clk->clk_en_mask;
	else
		reg_val = reg_val & ~bus_clk->clk_en_mask;
	clk_dbg("%s, writing %08x to clk_gate reg %08x\n",__func__, reg_val,
				(bus_clk->ccu_clk->ccu_clk_mgr_base+ bus_clk->clk_gate_offset));
	writel(reg_val, CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	clk_dbg("%s:%s clk before stprsts start\n",__func__,clk->name);
	if(enable)
		while(!(readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset))
					& bus_clk->stprsts_mask));
	else
		while((readl(CCU_REG_ADDR(bus_clk->ccu_clk,
							bus_clk->clk_gate_offset))) & bus_clk->stprsts_mask);

	clk_dbg("%s:%s clk after stprsts start\n",__func__,clk->name);
	clk_dbg("%s -- %s is %s\n", __func__, clk->name, enable?"enabled":"disabled");
						/* disable write access*/
	ccu_write_access_enable(bus_clk->ccu_clk, false);
enable_done:
	if(!enable && !(clk->flags & AUTO_GATE) && (clk->flags & NOTIFY_STATUS_TO_CCU))
	    bus_clk->ccu_clk->clk.ops->enable(&bus_clk->ccu_clk->clk, 0);
	return 0;
}

static unsigned long bus_clk_get_rate(struct clk *c)
{
	struct bus_clk *bus_clk = to_bus_clk(c);
	struct ccu_clk *ccu_clk;
	int current_policy;
	int freq_id;

	BUG_ON(!bus_clk->ccu_clk);
	ccu_clk= bus_clk->ccu_clk;
	if(bus_clk->freq_tbl_index == -1)
	{
		if(!bus_clk->src_clk || !bus_clk->src_clk->ops || !bus_clk->src_clk->ops->get_rate)
		{
		    clk_dbg("This bus clock freq depends on internal dividers\n");
		    c->rate = 0;
		}
		c->rate =  bus_clk->src_clk->ops->get_rate(bus_clk->src_clk);
		return c->rate;
	}
	current_policy = ccu_get_active_policy(ccu_clk);

	freq_id = ccu_get_freq_policy(ccu_clk, current_policy);
	clk_dbg("current_policy: %d freq_id %d freq_tbl_index :%d\n",
			current_policy, freq_id, bus_clk->freq_tbl_index);
	c->rate = ccu_clk->freq_tbl[freq_id][bus_clk->freq_tbl_index];
	clk_dbg("clock rate: %ld\n", (long int)c->rate);
	return c->rate;
}

static int bus_clk_init(struct clk *clk)
{
	struct bus_clk * bus_clk;
	int inx;
	unsigned int need_status_update = 0;

	if(clk->clk_type != CLK_TYPE_BUS)
		return -EPERM;
	if(clk->init)
		return 0;

	bus_clk = to_bus_clk(clk);
	BUG_ON(bus_clk->ccu_clk == NULL);

	clk_dbg("%s - %s\n", __func__, clk->name);
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++)
	{
		if(clk->dep_clks[inx]->ops->init)
			clk_dbg("%s Dependant clock %s init \n", __func__, clk->dep_clks[inx]->name);
		clk->dep_clks[inx]->ops->init(clk->dep_clks[inx]);
	}
	/* Enable write access*/
	ccu_write_access_enable(bus_clk->ccu_clk, true);
	if(bus_clk_get_gating_status(bus_clk) == 1)
	    clk->use_cnt = 1;
	if(bus_clk->hyst_val_mask)
		bus_clk_hyst_enable(bus_clk,HYST_ENABLE & clk->flags,
						 (clk->flags & HYST_HIGH) ? CLK_HYST_HIGH: CLK_HYST_LOW);

	/*Init source clocks */
	if(clk->flags & AUTO_GATE)
		bus_clk_set_gating_ctrl(bus_clk, CLK_GATING_AUTO);
	else
		bus_clk_set_gating_ctrl(bus_clk, CLK_GATING_SW);

	BUG_ON(CLK_FLG_ENABLED(clk,ENABLE_ON_INIT) && CLK_FLG_ENABLED(clk,DISABLE_ON_INIT));

	if(CLK_FLG_ENABLED(clk,ENABLE_ON_INIT))
	{
		if(clk->ops->enable)
		{
			clk->ops->enable(clk, 1);
		}
	}
	if(CLK_FLG_ENABLED(clk,DISABLE_ON_INIT))
	{
		if(clk->ops->enable)
			clk->ops->enable(clk, 0);
	} else
		need_status_update = 1;

	if(bus_clk_get_gating_status(bus_clk) == 1) {
	    clk->use_cnt = 1;
	    if (need_status_update && !(clk->flags & AUTO_GATE) &&(clk->flags & NOTIFY_STATUS_TO_CCU))
		bus_clk->ccu_clk->clk.ops->enable(&bus_clk->ccu_clk->clk, 1);
	}
	/* Disable write access*/
	ccu_write_access_enable(bus_clk->ccu_clk, false);
	clk->init = 1;
	clk_dbg("%s init complete\n", clk->name);

	return 0;
}

struct gen_clk_ops gen_bus_clk_ops =
{
	.init		= 	bus_clk_init,
	.enable		=	bus_clk_enable,
	.get_rate	=	bus_clk_get_rate,
};

/* reference clocks */
unsigned long ref_clk_get_rate(struct clk *clk)
{
	if(clk->clk_type != CLK_TYPE_REF)
		return -EPERM;
	return clk->rate;
}

static int ref_clk_enable(struct clk *c, int enable)
{
	return 0;
}

struct gen_clk_ops gen_ref_clk_ops =
{
	.init		= 	NULL,
	.enable		=	ref_clk_enable,
	.get_rate		=	ref_clk_get_rate,
};

#ifdef CONFIG_DEBUG_FS

static int test_func_invoke(void *data, u64 val)
{
	if(val == 1)
	{
		clk_dbg("Test func invoked \n");
	}
	else
		clk_dbg("Invalid value \n");

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(test_func_fops, NULL, test_func_invoke, "%llu\n");


static int clk_debug_get_rate(void *data, u64 *val)
{
	struct clk *clock = data;
	*val = clk_get_rate(clock);
	return 0;
}

static int clk_debug_set_rate(void *data, u64 val)
{
	struct clk *clock = data;
	clk_set_rate(clock, val);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_rate_fops, clk_debug_get_rate,
						clk_debug_set_rate, "%llu\n");

static int _get_clk_status(struct clk *c)
{
	struct peri_clk *peri_clk;
	struct bus_clk *bus_clk;
	struct ref_clk *ref_clk;
	int enabled = 0;

	clk_dbg("%s clock id:: %d\n",__func__,c->id);
	if(c->clk_type == CLK_TYPE_PERI)
	{
		peri_clk = to_peri_clk(c);
		enabled = peri_clk_get_gating_status(peri_clk);
	}
	else if(c->clk_type == CLK_TYPE_BUS)
	{
		bus_clk = to_bus_clk(c);
		enabled = bus_clk_get_gating_status(bus_clk);
	}
	else if(c->clk_type == CLK_TYPE_REF)
	{
		ref_clk = to_ref_clk(c);
		enabled = ref_clk_get_gating_status(ref_clk);
	}
	if(enabled < 0)
	    clk_dbg("Status register not available for clock %s\n", c->name);
	else
	    clk_dbg("clock %s \n", enabled?"enabled":"disabled");
	return enabled;
}

static int clk_debug_get_status(void *data, u64 *val)
{
	struct clk *clock = data;
	*val = _get_clk_status(clock);
	if (*val >= 0)
	    clk_dbg("%s is %s \n", clock->name, *val?"enabled":"disabled");

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_status_fops, clk_debug_get_status, NULL, "%llu\n");

static int clk_debug_set_enable(void *data, u64 val)
{
	struct clk *clock = data;
	if(val == 1)
		clk_enable(clock);
	else if(val == 0)
		clk_disable(clock);
	else
		clk_dbg("Invalid value \n");

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_enable_fops, NULL, clk_debug_set_enable, "%llu\n");

static int clk_parent_show(struct seq_file *seq, void *p)
{
	struct clk *clock = seq->private;
	struct peri_clk *peri_clk;
	struct bus_clk *bus_clk;
	struct ref_clk *ref_clk;
	switch(clock->clk_type) {
	    case CLK_TYPE_PERI:
	    	peri_clk = to_peri_clk(clock);
		seq_printf(seq, "name   -- %s\n", clock->name);
		if((peri_clk->src_clk.count > 0) && (peri_clk->src_clk.src_inx < peri_clk->src_clk.count))
		    seq_printf(seq, "parent -- %s\n", peri_clk->src_clk.clk[peri_clk->src_clk.src_inx]->name);
		else
		    seq_printf(seq, "parent -- NULL\n");
	    	break;
	    case CLK_TYPE_BUS:
	    	bus_clk = to_bus_clk(clock);
		seq_printf(seq, "name   -- %s\n", clock->name);
		if (bus_clk->freq_tbl_index < 0)
		    seq_printf(seq, "parent -- %s\n", bus_clk->src_clk->name);
		else
		    seq_printf(seq, "parent derived from internal bus\n");
	    	break;
	    case CLK_TYPE_REF:
	    	ref_clk = to_ref_clk(clock);
		seq_printf(seq, "name   -- %s\n", clock->name);
		if ((ref_clk->src_clk.count > 0) && (ref_clk->src_clk.src_inx < ref_clk->src_clk.count))
		    seq_printf(seq, "parent -- %s\n", ref_clk->src_clk.clk[ref_clk->src_clk.src_inx]->name);
		else
		    seq_printf(seq, "Derived from %s ccu\n", ref_clk->ccu_clk->clk.name);
	    	break;
	    default:
	    	return -EINVAL;
	}
	return 0;
}

static int fops_parent_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_parent_show, inode->i_private);
}

static const struct file_operations clock_parent_fops =
{
	.open			= fops_parent_open,
	.read			= seq_read,
	.llseek 		= seq_lseek,
	.release		= single_release,
};

static int clk_source_show(struct seq_file *seq, void *p)
{
	struct clk *clock = seq->private;
	struct peri_clk *peri_clk;
	struct bus_clk *bus_clk;
	struct ref_clk *ref_clk;
	switch(clock->clk_type) {
	case CLK_TYPE_PERI:
		peri_clk = to_peri_clk(clock);
		if(peri_clk->src_clk.count > 0)
		{
		    int i;
		    seq_printf(seq, "clock source for %s\n", clock->name);
		    for (i=0; i<peri_clk->src_clk.count; i++)
		    {
			seq_printf(seq, "%d   %s\n", i, peri_clk->src_clk.clk[i]->name);
		    }
		}
		else
		    seq_printf(seq, "no source for %s\n", clock->name);
		break;
	case CLK_TYPE_BUS:
		bus_clk = to_bus_clk(clock);
		if (bus_clk->freq_tbl_index < 0)
		    seq_printf(seq, "source for %s is %s\n", clock->name, bus_clk->src_clk->name);
		else
		    seq_printf(seq, "%s derived from %s CCU\n", clock->name, bus_clk->ccu_clk->clk.name);
		break;
	case CLK_TYPE_REF:
		ref_clk = to_ref_clk(clock);
		if ((ref_clk->src_clk.count > 0) && (ref_clk->src_clk.src_inx < ref_clk->src_clk.count))
		    seq_printf(seq, "parent -- %s\n", ref_clk->src_clk.clk[ref_clk->src_clk.src_inx]->name);
		else
		    seq_printf(seq, "%s derived from %s CCU\n", clock->name,
		    ref_clk->ccu_clk->clk.name);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int fops_source_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_source_show, inode->i_private);
}

static const struct file_operations clock_source_fops =
{
	.open			= fops_source_open,
	.read			= seq_read,
	.llseek 		= seq_lseek,
	.release		= single_release,
};

static struct dentry *dent_clk_root_dir;
int __init clock_debug_init(void)
{
	/* create root clock dir /clock */
	dent_clk_root_dir = debugfs_create_dir("clock", 0);
	if(!dent_clk_root_dir)
		return -ENOMEM;

	if(!debugfs_create_u32("debug", 0644, dent_clk_root_dir, (int*)&clk_debug))
		return -ENOMEM;

	if(!debugfs_create_file("debug_func", 0644, dent_clk_root_dir, NULL, &test_func_fops))
		return -ENOMEM;

	return 0;
}

int __init clock_debug_add_clock(struct clk *c)
{
	struct dentry *dent_clk_dir=0, *dent_rate=0, *dent_enable=0,
										*dent_status=0,
										*dent_div=0,
										*dent_use_cnt=0, *dent_id=0,
												*dent_parent=0, *dent_source=0;
	BUG_ON(!dent_clk_root_dir);

	/* create root clock dir /clock/clk_a */
	dent_clk_dir	=	debugfs_create_dir(c->name, dent_clk_root_dir);
	if(!dent_clk_dir)
		goto err;

	/* file /clock/clk_a/enable */
	dent_enable	   =	   debugfs_create_file("enable", 0644, dent_clk_dir, c, &clock_enable_fops);
	if(!dent_enable)
		goto err;

	/* file /clock/clk_a/status */
	dent_status	 =	   debugfs_create_file("status", 0644, dent_clk_dir, c, &clock_status_fops);
	if(!dent_status)
		goto err;

	/* file /clock/clk_a/rate */
	dent_rate	=	debugfs_create_file("rate", 0644, dent_clk_dir, c, &clock_rate_fops);
	if(!dent_rate)
		goto err;
#if 0
	/* file /clock/clk_a/div */
	dent_div	=	debugfs_create_u32("div", 0444, dent_clk_dir, (unsigned int*)&c->div);
	if(!dent_div)
		goto err;
#endif
	/* file /clock/clk_a/use_cnt */
	dent_use_cnt	=	debugfs_create_u32("use_cnt", 0444, dent_clk_dir, (unsigned int*)&c->use_cnt);
	if(!dent_use_cnt)
		goto err;

	/* file /clock/clk_a/id */
	dent_id		=	debugfs_create_u32("id", 0444, dent_clk_dir, (unsigned int*)&c->id);
	if(!dent_id)
		goto err;

	/* file /clock/clk_a/parent */
	dent_parent	=	debugfs_create_file("parent", 0444, dent_clk_dir, c, &clock_parent_fops);
	if(!dent_parent)
		goto err;

	/* file /clock/clk_a/source */
	dent_source	=	debugfs_create_file("source", 0444, dent_clk_dir, c, &clock_source_fops);
	if(!dent_source)
		goto err;

	return 0;

err:
	debugfs_remove(dent_rate);
	debugfs_remove(dent_div);
	debugfs_remove(dent_use_cnt);
	debugfs_remove(dent_id);
	debugfs_remove(dent_parent);
	debugfs_remove(dent_source);
	debugfs_remove(dent_clk_dir);
	return -ENOMEM;
}
#endif

