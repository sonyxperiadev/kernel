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
#include <mach/clock.h>
#include <mach/io_map.h>
#include <mach/brcm_ccu_clk_mgr_reg.h>
#include <asm/io.h>
#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>
#if (defined(CONFIG_ARCH_ISLAND) || defined(CONFIG_ARCH_HANA))
#include <mach/rdb/brcm_rdb_iroot_clk_mgr_reg.h>
#else
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#endif

#ifdef CONFIG_SMP
#include <asm/cpu.h>
#endif

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

/* global spinlock for clock API */
static DEFINE_SPINLOCK(clk_lock);

int clk_debug = 0;

static int clk_init(struct clk *clk)
{
    int ret = 0;
    unsigned long flags;

    if (!clk || !clk->ops)
	return -EINVAL;

    if (clk->ops->init) {
	spin_lock_irqsave(&clk_lock, flags);
	ret = clk->ops->init(clk);
	spin_unlock_irqrestore(&clk_lock, flags);
    }

    return ret;
}

int clk_register(struct clk_lookup *clk_lkup)
{
    int ret = 0;

    ret = clk_init(clk_lkup->clk);
    clkdev_add(clk_lkup);
    return ret;
}

static int __clk_enable(struct clk *clk)
{
	int ret = 0;

	if (!clk)
		return -EINVAL;

	if (!clk->ops || !clk->ops->enable)
		return -EINVAL;

	/* enable parent clock first */
	if (clk->parent)
		ret = __clk_enable(clk->parent);

	if (ret)
		return ret;

	if (clk->use_cnt++ == 0) {
		ret = clk->ops->enable(clk, 1);
	}

	return ret;
}

int clk_enable(struct clk *clk)
{
	int ret;
	unsigned long flags;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	spin_lock_irqsave(&clk_lock, flags);
	ret = __clk_enable(clk);
	spin_unlock_irqrestore(&clk_lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_enable);

static void __clk_disable(struct clk *clk)
{
	if (!clk->ops || !clk->ops->enable)
		return;

	if (clk->use_cnt == 0) {
	    clk_dbg("%s already disabled\n", clk->name);
	    return;
	}
	if (--clk->use_cnt == 0)
		clk->ops->enable(clk, 0);

	/* disable parent */
	if (clk->parent)
		__clk_disable(clk->parent);
}

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	if (IS_ERR_OR_NULL(clk))
		return;

	spin_lock_irqsave(&clk_lock, flags);
	__clk_disable(clk);
	spin_unlock_irqrestore(&clk_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long flags, rate;

	if (IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->get_rate)
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

	if (IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->round_rate)
		return -EINVAL;

	if (clk->use_cnt)
		return -EBUSY;

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

	if (IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->set_rate)
		return -EINVAL;

	if (clk->use_cnt)
		return -EBUSY;

	spin_lock_irqsave(&clk_lock, flags);
	ret = clk->ops->set_rate(clk, rate);
	spin_unlock_irqrestore(&clk_lock, flags);

	return 0;
}
EXPORT_SYMBOL(clk_set_rate);

struct clk *clk_get_parent(struct clk *clk)
{
	struct clk *parent;
	unsigned long flags;

	if (IS_ERR_OR_NULL(clk))
		return NULL;

	spin_lock_irqsave(&clk_lock, flags);
	parent = clk->parent;
	spin_unlock_irqrestore(&clk_lock, flags);
	return parent;
}
EXPORT_SYMBOL(clk_get_parent);

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	unsigned long flags;
	struct clk *old_parent;

	if (IS_ERR_OR_NULL(clk) || !parent || !clk->ops || !clk->ops->set_parent)
		return -EINVAL;

	/* if more than one user, parent is not allowed */
	if (clk->use_cnt > 1)
		return -EBUSY;

	if (clk->parent == parent)
		return 0;

	spin_lock_irqsave(&clk_lock, flags);
	old_parent = clk->parent;
	clk->ops->set_parent(clk, parent);

	/* if clock is active */
	if (clk->use_cnt != 0) {
		clk->use_cnt--;
		/* enable clock with the new parent */
		__clk_enable(clk);
		/* disable the old parent */
		__clk_disable(old_parent);
	}
	spin_unlock_irqrestore(&clk_lock, flags);

	return 0;
}
EXPORT_SYMBOL(clk_set_parent);

static unsigned long common_get_rate(struct clk *c)
{
	if (c->parent && c->div)
		c->rate = c->parent->ops->get_rate(c->parent) / c->div;

	return c->rate;
}

static unsigned long common_round_rate(struct clk *c, unsigned long rate)
{
	int i, ind = 0;
	unsigned long diff;

	diff = rate;

	if (!c->parent)
		return c->rate;

	BUG_ON(!c->src || !c->src->total);

	for (i=0; i<c->src->total;i++) {
		unsigned long new_rate, div;
		/* round to the new rate */
		div = c->src->parents[i]->rate / rate;
		if(div == 0)
			div = 1;
		new_rate = c->src->parents[i]->rate/div;
		/* get the min diff */
		if(abs(new_rate-rate) < diff) {
			diff = abs(new_rate-rate);
			ind = i;
		}
	}
	return c->src->parents[ind]->rate;
}

static int common_set_rate(struct clk *c, unsigned long rate)
{
	int i, ind = 0;
	unsigned long diff;

	diff = rate;

	if (!c->parent) {
		return c->rate;
	}

	BUG_ON( !c->src || !c->src->total);

	for (i=0; i<c->src->total;i++) {
		unsigned long new_rate, div;
		/* round to the new rate */
		div = c->src->parents[i]->rate / rate;
		new_rate = c->src->parents[i]->rate/div;
		/* get the min diff */
		if(new_rate-rate < diff) {
			diff = new_rate-rate;
			ind = i;
			c->div = div;
			c->rate = new_rate;
		}
	}
	return 0;
}

static int common_set_parent(struct clk *c, struct clk *parent)
{
	int i;

	if(!c->parent)
		return -EINVAL;
	BUG_ON(!c->src || !c->src->total);

	for (i=0; i<c->src->total; i++) {
		if (is_same_clock(parent, c->src->parents[i]))
			return 0;
	}
	return -EINVAL;
}

static inline void __proc_clk_enable_access (void __iomem *base, int enable)
{
	if (enable)
		writel(CLK_WR_ACCESS_PASSWORD, base + KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET);
	else
		writel(0, base + KPROC_CLK_MGR_REG_WR_ACCESS_OFFSET);
}

/* post divider for policy 6 and 7*/
static inline void __proc_clk_set_policy_div(void __iomem *base, int policy, int div)
{
	unsigned long offset;

	if (policy==6)
		offset = KPROC_CLK_MGR_REG_PLLARMC_OFFSET;
	else if( policy==7)
		offset = KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET;

	clk_dbg ("policy %d div %d\n", policy, div);
	writel(div, base+offset);
	writel((div<<KPROC_CLK_MGR_REG_PLLARMC_PLLARM_MDIV_SHIFT)|
		KPROC_CLK_MGR_REG_PLLARMC_PLLARM_LOAD_EN_MASK,
		base+offset);
	writel(div, base+offset);
}

static inline void __proc_clk_dump_register (void __iomem *base)
{
#if defined(CONFIG_SMP) && defined(DEBUG)
	unsigned int cpu = get_cpu();
	clk_dbg("current cpu %d\n", cpu);
	put_cpu();
#endif
	clk_dbg ("policy freq      0x%08x\n", readl(base+KPROC_CLK_MGR_REG_POLICY_FREQ_OFFSET));
	clk_dbg ("policy_ctrl      0x%08x\n", readl(base+KPROC_CLK_MGR_REG_POLICY_CTL_OFFSET));
	clk_dbg ("arma             0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMA_OFFSET));
	clk_dbg ("armb             0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMB_OFFSET));
	clk_dbg ("armc             0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMC_OFFSET));
	clk_dbg ("armctrl3         0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMCTRL3_OFFSET));
	clk_dbg ("armctrl5         0x%08x\n", readl(base+KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET));
	clk_dbg ("lvm_en           0x%08x\n", readl(base+KPROC_CLK_MGR_REG_LVM_EN_OFFSET));
	clk_dbg ("arm_div          0x%08x\n", readl(base+KPROC_CLK_MGR_REG_ARM_DIV_OFFSET));
}

/* Proc clocks */
static int proc_clk_enable(struct clk *c, int enable)
{
	int ret=0;
	return ret;
}

#ifndef CONFIG_CPU_FREQ
static void __recalc_loops_per_jiffy(unsigned long old_rate, unsigned long new_rate)
{
#if !defined(CONFIG_SMP)
	extern unsigned long loops_per_jiffy;
#endif
	u64 prod;
	u32 r;

#if defined(CONFIG_SMP)
	int i;
	clk_dbg ("recal jiffy - SMP\n");
	for_each_online_cpu(i) {
		clk_dbg ("recal for cpu %d\n", i);
		prod = (u64)per_cpu(cpu_data, i).loops_per_jiffy * (u64)new_rate;
		per_cpu(cpu_data, i).loops_per_jiffy = div_u64_rem(prod, (u32)old_rate, &r);
	}
#else
	clk_dbg ("recal jiffy - UP\n");
	prod = (u64)loops_per_jiffy * (u64)new_rate;
	loops_per_jiffy = (unsigned long) div_u64_rem(prod, (u32)old_rate, &r);
#endif

#endif
}

#ifdef CONFIG_HAVE_ARM_TWD
static void __recalc_twd_rate(unsigned long old_rate, unsigned long new_rate)
{
extern unsigned long twd_timer_rate;
extern void __iomem *twd_base;

	u64 prod;
	u32 r, load;

	clk_dbg ("reacl twd_rate\n");
	prod = (u64)twd_timer_rate * (u64)new_rate;
	twd_timer_rate = (unsigned long) div_u64_rem(prod, (u32)old_rate, &r);

	load = twd_timer_rate / HZ;
	writel (load, twd_base + 0);
}
#endif

static unsigned int __proc_clk_get_vco_rate(void __iomem *base)
{
	unsigned long xtal = clock_get_xtal();
	unsigned int ndiv_int, ndiv_frac, vco_rate;

	ndiv_int = (readl(base + KPROC_CLK_MGR_REG_PLLARMA_OFFSET)&KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_MASK)
		>> KPROC_CLK_MGR_REG_PLLARMA_PLLARM_NDIV_INT_SHIFT;
	ndiv_frac = (readl(base + KPROC_CLK_MGR_REG_PLLARMB_OFFSET) & KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_MASK)
		>> KPROC_CLK_MGR_REG_PLLARMB_PLLARM_NDIV_FRAC_SHIFT;

	vco_rate = ndiv_int * xtal;

	vco_rate += (unsigned long) (u64) (((u64)ndiv_frac * (u64)xtal) >> 20);

	clk_dbg ("xtal %d, int %d, frac %d, vco %d\n", (int)xtal, ndiv_int, ndiv_frac, vco_rate);
	return vco_rate;
}

static unsigned int __proc_clk_get_rate(void __iomem *base)
{
	unsigned int vco_rate = __proc_clk_get_vco_rate (base);
	int div = (readl(base+KPROC_CLK_MGR_REG_PLLARMCTRL5_OFFSET)& KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_MASK)
		>> KPROC_CLK_MGR_REG_PLLARMCTRL5_PLLARM_H_MDIV_SHIFT;

	return vco_rate /div;
}

static int proc_clk_set_rate(struct clk *c, unsigned long rate)
{
	struct proc_clock *proc_clk = to_proc_clk(c);
	unsigned long old_rate, vco_rate;
	void __iomem *base;
	int ret = 0, div = 2;

	base = ioremap (proc_clk->proc_clk_mgr_base, SZ_4K);
	if(!base)
		return -ENOMEM;

	/* enable  access */
	__proc_clk_enable_access (base, 1);


	old_rate = __proc_clk_get_rate(base);

	vco_rate = __proc_clk_get_vco_rate (base);

	/* to get minimium clock >= desired_rate */
	div = vco_rate/rate;
	div = min (max (2, div), 255);

	c->rate = vco_rate / div;

	/* switch to normal freq of policy 7 */
	__proc_clk_set_policy_div (base, 7, div);

	if (ret)
		goto err;

	__proc_clk_dump_register (base);

	__proc_clk_enable_access (base, 0);
	iounmap (base);

#ifndef CONFIG_CPU_FREQ
	__recalc_loops_per_jiffy(old_rate, c->rate);
#endif
#ifdef CONFIG_HAVE_ARM_TWD
	__recalc_twd_rate(old_rate, c->rate);
#endif

	return ret;
err:
	__proc_clk_enable_access (base, 0);
	iounmap (base);
	return ret;
}
static unsigned long proc_clk_get_rate(struct clk *c)
{
	unsigned int ret = 0;
	struct proc_clock *proc_clk = to_proc_clk(c);
	void __iomem *base;

	base = ioremap (proc_clk->proc_clk_mgr_base, SZ_4K);
	if (!base)
		return -ENOMEM;
	c->rate = __proc_clk_get_rate(base);
	iounmap (base);
	ret = c->rate;
	return ret;
}

static unsigned long proc_clk_round_rate(struct clk *c, unsigned long rate)
{
	unsigned long ret = rate;
	return ret;
}

struct clk_ops proc_clk_ops = {
    	.init		= 	NULL,
	.enable		=	proc_clk_enable,
	.set_rate	=	proc_clk_set_rate,
	.get_rate	=	proc_clk_get_rate,
	.round_rate	=	proc_clk_round_rate,
	.set_parent	=	common_set_parent,
};

static int peri_clk_enable(struct clk *c, int enable)
{
	int ret=0, reg;
	struct peri_clock *peri_clk = to_peri_clk(c);
	void __iomem *base;

	if (c->flags & AUTO_GATE)
	    return -EPERM;

	base = ioremap (peri_clk->ccu_clk_mgr_base, SZ_4K);
	if (!base)
		return -ENOMEM;

	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + peri_clk->wr_access_offset);

	if (enable) {
		clk_dbg("%s %s rate %lu div %lu ind %d parent_rate %lu\n", __func__, c->name,
			c->rate, c->div, c->src->sel, c->parent->rate);

		/* clkgate */
		reg = readl(base + peri_clk->clkgate_offset);
		clk_dbg ("Before %s, %s gating reg(%p): 0x%08x\n",
			enable?"enable":"disable", c->name, (base + peri_clk->clkgate_offset), reg);
		reg |= peri_clk->clk_en_mask;
		writel(reg, base + peri_clk->clkgate_offset);

		/* div and pll select */
		if (!peri_clk->div_mask)
			BUG_ON (c->div != 1);

		reg = ((c->div-1) << (peri_clk->div_shift + peri_clk->div_dithering));

		if (peri_clk->div_dithering > 0) {
			reg = ((c->div-1) << (peri_clk->div_shift +
			peri_clk->div_dithering)) | (c->fraction << peri_clk->div_shift);
		} else {
			reg = ((c->div-1) << (peri_clk->div_shift + peri_clk->div_dithering));
		}
		
		if (peri_clk->pre_div_mask) {
			reg |= (c->pre_div-1) << peri_clk->pre_div_shift;
		}

		reg |= (c->src->sel << peri_clk->pll_select_shift);

		writel(reg, base + peri_clk->div_offset);

		/* trigger */
		writel(peri_clk->trigger_mask, base + peri_clk->div_trig_offset);
		while(readl(base + peri_clk->div_trig_offset) & peri_clk->trigger_mask);

		/* wait for running */
		while(! (readl(base + peri_clk->clkgate_offset) & peri_clk->stprsts_mask));
	}
	else {
		/* clkgate */
		reg = readl(base + peri_clk->clkgate_offset);
		clk_dbg ("Before %s, %s gating reg(%p): 0x%08x\n",
			enable?"enable":"disable", c->name, (base + peri_clk->clkgate_offset), reg);
		reg &= ~peri_clk->clk_en_mask;
		writel(reg, base + peri_clk->clkgate_offset);

		/* wait for stop */
		while((readl(base + peri_clk->clkgate_offset) & peri_clk->stprsts_mask));
	}
	/* disable access */
	writel(0, base + peri_clk->wr_access_offset);

	iounmap (base);

	return ret;
}


static unsigned long calc_pll_divisor(struct peri_clock *peri_clk, unsigned long
req_rate, struct clk_rate_div *clk_rate_div)
{
    struct clk *c = &peri_clk->clk;
    int i, ind = 0;
    unsigned long diff = 0x7fffffff;
    unsigned long div=0, pre_div=0, k, l;
    unsigned long new_rate;

    clk_dbg("%s clk:%s req_rate:%lu \n",__func__, c->name, req_rate);

    for(ind=0;ind<c->src->total;ind++ )
    {
        if (c->src->parents[ind]->rate == req_rate)
        {
            clk_rate_div->sel = ind;
            clk_rate_div->pre_div = 0;
            clk_rate_div->div = 0;
            return c->src->parents[ind]->rate;
        }
    }
    for (i=0;i<c->src->total;i++) {
	k = 1;
	l = 1;
	while(1)
	{
	    new_rate = c->src->parents[i]->rate/(k*l);
	    if (new_rate < req_rate && abs(new_rate - req_rate) >= diff) {
		break;
	    }
	    if (abs(new_rate - req_rate) < diff)
	    {
		diff = abs(new_rate - req_rate);
		ind = i;
		pre_div = k-1;
		div = l-1;
	    }
	    if(peri_clk->pre_div_max == k && peri_clk->div_max == l)
		break;
	    if(peri_clk->pre_div_max == k && peri_clk->div_max > l)
	    {
		k = 1;
		l++;
	    } else
	    	k++;
	}
    }
    clk_dbg("%s After calc clk:%s div:%lu ; pre_div: %lu sel: %d\n",__func__, c->name, div, pre_div, ind);
    clk_rate_div->sel = ind;
    clk_rate_div->pre_div = pre_div;
    clk_rate_div->div = div;
    return (c->src->parents[ind]->rate/((div+1)*(pre_div+1)));
}

static unsigned long calc_pll_fractional_divisor(struct peri_clock *peri_clk, unsigned
                long req_rate, struct clk_rate_div *clk_rate_div)
{
    struct clk *c = &peri_clk->clk;
    unsigned long calc_rate;
    unsigned long i, max_fraction;
    unsigned long diff = 0x7fffffff;
    unsigned long fraction = 0;

    calc_rate = calc_pll_divisor(peri_clk, req_rate, clk_rate_div);

    if (calc_rate < req_rate && clk_rate_div->pre_div > 0)
	clk_rate_div->pre_div = clk_rate_div->pre_div -1;
    else if (calc_rate < req_rate && clk_rate_div->div > 0)
	clk_rate_div->div = clk_rate_div->div -1;
    else if (calc_rate < req_rate)
    {
	clk_rate_div->fraction = 0;
	return (c->src->parents[clk_rate_div->sel]->rate/
		((clk_rate_div->div+1)*(clk_rate_div->pre_div+1)));
    }
    if (peri_clk->div_dithering <= 0)
    {
	clk_rate_div->fraction = 0;
	return 	(c->src->parents[clk_rate_div->sel]->rate/
	((clk_rate_div->div+1)*(clk_rate_div->pre_div+1)));
    }
    max_fraction = ~(0xFFFFFFFF << peri_clk->div_dithering) - 1;
    for (i = 0; i <= max_fraction; i++)
    {
	calc_rate = (((c->src->parents[clk_rate_div->sel]->rate/100)*(max_fraction+1))/
		((clk_rate_div->div+1)*(max_fraction+1)+i))*100;
	if (calc_rate >= req_rate && (calc_rate-req_rate) < diff)
	{
	    diff = calc_rate - req_rate;
	    fraction = i;
	}
	else
	    break;
    }

    clk_rate_div->fraction = fraction;
    return ((((c->src->parents[clk_rate_div->sel]->rate/100)*(max_fraction+1))/
	((clk_rate_div->div+1)*(max_fraction+1)+ fraction))*100);

}


static int peri_clk_set_rate(struct clk *c, unsigned long rate)
{
	int ret = 0, reg;
	unsigned long diff;
	unsigned long temp_rate=0, temp_div=1, temp_prediv=0, temp_fraction=0;
	struct peri_clock *peri_clk = to_peri_clk(c);
	unsigned int div_ctrl_val = 0;
	void __iomem *base;
	struct clk_rate_div clk_rate_div;

	diff = rate;
	clk_dbg("%s clk:%s rate to set:%lu \n",__func__, c->name, rate);

	BUG_ON(!c->parent);
	BUG_ON(!c->src || !c->src->total);

	memset(&clk_rate_div, 0, sizeof(struct clk_rate_div));
	if (peri_clk->div_dithering > 0)
	    temp_rate = calc_pll_fractional_divisor(peri_clk, rate, &clk_rate_div);
	else
	    temp_rate = calc_pll_divisor(peri_clk, rate, &clk_rate_div);

	c->src->sel = clk_rate_div.sel;
	c->parent = c->src->parents[c->src->sel];
	temp_div = clk_rate_div.div;
	temp_prediv = clk_rate_div.pre_div;
	temp_fraction = clk_rate_div.fraction;

	clk_dbg("%s After calc clk:%s sel:%d parent_rate:%lu temp_div:%lu \
	temp_prediv:%lu temp_fraction:%lu\n",__func__, c->name, c->src->sel,
	c->src->parents[c->src->sel]->rate, temp_div, temp_prediv, temp_fraction);

	if (peri_clk->div_mask) {
	    if (peri_clk->div_dithering > 0) {
	        div_ctrl_val = (temp_div << (peri_clk->div_shift +
	       	peri_clk->div_dithering)) | (temp_fraction << peri_clk->div_shift);
	    } else {
	       div_ctrl_val = temp_div << peri_clk->div_shift;
	    }
	}
	if (peri_clk->pre_div_mask) {
	    div_ctrl_val = div_ctrl_val |
	    	(temp_prediv << peri_clk->pre_div_shift);
	}
	if (peri_clk->pll_select_mask)
	    div_ctrl_val |= (c->src->sel << peri_clk->pll_select_shift);
	base = ioremap (peri_clk->ccu_clk_mgr_base, SZ_4K);
	writel(CLK_WR_ACCESS_PASSWORD, base + peri_clk->wr_access_offset);
	if (!base)
		return -ENOMEM;
	writel(div_ctrl_val, base + peri_clk->div_offset);

	if (peri_clk->pre_trigger_mask)
	    writel(peri_clk->pre_trigger_mask, base + peri_clk->div_trig_offset);
	if (peri_clk->trigger_mask) {
	    writel(peri_clk->trigger_mask, base + peri_clk->div_trig_offset);
	    reg = readl(base + peri_clk->clkgate_offset);
	    if (reg & peri_clk->stprsts_mask)
		while(readl(base + peri_clk->div_trig_offset) &	peri_clk->trigger_mask);
	}
	/* disable access */
	writel(0, base + peri_clk->wr_access_offset);
	iounmap (base);

	c->rate = temp_rate;
	c->div = temp_div + 1;
	c->pre_div = temp_prediv + 1;
	c->fraction = temp_fraction;

	clk_dbg("At %s exit %s set rate %lu div %lu sel %d parent %lu\n", __func__, c->name,
		c->rate, c->div, c->src->sel, c->parent->rate);
	return ret;
}

static unsigned long peri_clk_get_rate(struct clk *c)
{
	struct peri_clock *peri_clk = to_peri_clk(c);
	void __iomem *base;
	int sel, div;

	base = ioremap (peri_clk->ccu_clk_mgr_base, SZ_4K);
	if (!base)
		return -ENOMEM;
	sel = (readl(base + peri_clk->div_offset) & peri_clk->pll_select_mask)
		>> peri_clk->pll_select_shift;

	div = ((readl(base + peri_clk->div_offset) & peri_clk->div_mask)
		>> peri_clk->div_shift);
	div = (div >> peri_clk->div_dithering) + 1;

	if (!peri_clk->div_mask)
		BUG_ON (div != 1);

	BUG_ON (sel >= c->src->total);
	c->src->sel = sel;
	c->parent = c->src->parents[sel];
	c->div = div;
	c->rate = c->parent->rate / c->div;
	clk_dbg("%s src %lu sel %d div %d rate %lu\n",__func__, c->parent->rate, sel, div, c->rate);

	iounmap (base);
	return c->rate;
}

static int peri_clk_init(struct clk *clk)
{
    int ret=0, reg;
    struct peri_clock *peri_clk = to_peri_clk(clk);
    void __iomem *base;
    int clk_status = 0;

    base = ioremap (peri_clk->ccu_clk_mgr_base, SZ_4K);
    if (!base) {
	printk (KERN_INFO "%s ioremap error\n", __func__);
	return -ENOMEM;
    }
    /* enable access */
    writel(CLK_WR_ACCESS_PASSWORD, base + peri_clk->wr_access_offset);
    /* clkgate */
    reg = readl(base + peri_clk->clkgate_offset);
    clk_status = !!(reg & peri_clk->stprsts_mask);
    clk_dbg ("%s entry: %s  %s \n", __func__, clk->name, clk_status?"enabled":"disabled");
    /*If the clock is already enabled by ASIC/bootloader/early kernel inits,
     * make usei_cnt = 1 */
    if (clk_status)
	clk->use_cnt = clk->use_cnt + 1;

    if (clk->flags & AUTO_GATE)
	reg &= ~peri_clk->hw_sw_gating_mask;
    else
	reg |= peri_clk->hw_sw_gating_mask;

    writel(reg, base + peri_clk->clkgate_offset);

    /* disable access */
    writel(0, base + peri_clk->wr_access_offset);
    iounmap (base);

    if ((clk_status == 0) && (clk->flags & ENABLE_ON_INIT)) {
	clk->ops->enable(clk, 1);
	clk->use_cnt = 1;
    }
    if((clk_status == 1) && (clk->flags & DISABLE_ON_INIT)) {
	clk->ops->enable(clk, 0);
	clk->use_cnt = 0;
    }

    return ret;
}

struct clk_ops peri_clk_ops = {
    	.init		= 	peri_clk_init,
	.enable		=	peri_clk_enable,
	.set_rate	=	peri_clk_set_rate,
	.get_rate	=	peri_clk_get_rate,
	.round_rate	=	common_round_rate,
	.set_parent	=	common_set_parent,
};

static int ccu_clk_enable(struct clk *c, int enable)
{
    clk_dbg("%s enable: %d, ccu name:%s\n",__func__, enable, c->name);
    if (c->ccu_id == BCM2165x_ROOT_CCU)
	return 0;

    /* Add any CCU enable code here if needed */
    return 0;
}

static unsigned long ccu_clk_get_rate(struct clk *c)
{
	struct ccu_clock *ccu_clk = to_ccu_clk(c);

	if (c->ccu_id == BCM2165x_ROOT_CCU)
	    return 0;
	c->rate = ccu_clk->freq_tbl[ccu_clk->freq_id];
	return 	c->rate;
}

#if 0	/* FIXME: Unused for now */
static int trigger_active_load(struct clk *clk, void __iomem  *base)
{
    int val;
    clk_dbg("%s\n", __func__);

    if(clk == NULL || !base)
	return -EINVAL;
    /* Software update enable for policy related data */
    writel (CCU_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK, base + CCU_CLK_MGR_REG_LVM_EN_OFFSET);
    do {
	val = readl (base + CCU_CLK_MGR_REG_LVM_EN_OFFSET);
    } while (val & CCU_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK);

    val = CCU_CLK_MGR_REG_POLICY_CTL_GO_MASK | CCU_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK;
    writel (val, base + CCU_CLK_MGR_REG_POLICY_CTL_OFFSET);
    /*polling ctrl go bit till its back to 0 */
    do {
	val = readl (base + CCU_CLK_MGR_REG_POLICY_CTL_OFFSET);
    } while (val & CCU_CLK_MGR_REG_POLICY_CTL_GO_MASK);

    return 0;
}
#endif

static int root_ccu_init(struct clk *clk)
{
    void __iomem *base;

    base = (void __iomem *)KONA_ROOT_CLK_VA;
    writel(CLK_WR_ACCESS_PASSWORD, base + CCU_CLK_MGR_REG_WR_ACCESS_OFFSET);

    // HWRHEA-877: var_312m_clk and var_96m_clk in rootCCU have wrong default
    // pll_select vaules, SW should program rootccu VAR_312M_DIV/VAR_48M_DIV
    // to use PLL1 clock instead of default PLL0i
#if (defined(CONFIG_ARCH_ISLAND) || defined(CONFIG_ARCH_HANA))
    writel (0x1, base  + IROOT_CLK_MGR_REG_VAR_312M_DIV_OFFSET);
    writel (0x1, base + IROOT_CLK_MGR_REG_VAR_48M_DIV_OFFSET);
    writel (0x5, base + IROOT_CLK_MGR_REG_REFCLK_SEG_TRG_OFFSET);
    while(readl(base + IROOT_CLK_MGR_REG_REFCLK_SEG_TRG_OFFSET));
#else
    writel (0x1, base  + ROOT_CLK_MGR_REG_VAR_312M_DIV_OFFSET);
    writel (0x1, base + ROOT_CLK_MGR_REG_VAR_48M_DIV_OFFSET);
    writel (0x5, base + ROOT_CLK_MGR_REG_REFCLK_SEG_TRG_OFFSET);
    while(readl(base + ROOT_CLK_MGR_REG_REFCLK_SEG_TRG_OFFSET));
#endif

    writel(0, base + CCU_CLK_MGR_REG_WR_ACCESS_OFFSET);

    return 0;
}

static int ccu_common_init(struct clk *clk, void __iomem *base)
{
    struct ccu_clock *ccu_clk = to_ccu_clk(clk);
    int reg, ret = 0;
    clk_dbg("%s \n",__func__);

    /* enable access */
    writel(CLK_WR_ACCESS_PASSWORD, base + ccu_clk->wr_access_offset);

    /* config enable for policy engine */
    writel(1, base + ccu_clk->lvm_en_offset);
    while (readl(base + ccu_clk->lvm_en_offset) & 1);

    /* freq ID */
    if (!ccu_clk->freq_bit_shift)
	ccu_clk->freq_bit_shift = 8;

    reg = ccu_clk->freq_id |
    	(ccu_clk->freq_id << (ccu_clk->freq_bit_shift)) |
	(ccu_clk->freq_id << (ccu_clk->freq_bit_shift * 2)) |
	(ccu_clk->freq_id << (ccu_clk->freq_bit_shift * 3));

    writel(reg, base + ccu_clk->policy_freq_offset);

    /* enable all clock mask */
    writel(0x7fffffff, base + ccu_clk->policy0_mask_offset);
    writel(0x7fffffff, base + ccu_clk->policy1_mask_offset);
    writel(0x7fffffff, base + ccu_clk->policy2_mask_offset);
    writel(0x7fffffff, base + ccu_clk->policy3_mask_offset);

    if (clk->ccu_id == BCM2165x_HUB_CCU) {
	writel(0x7fffffff, base + ccu_clk->policy0_mask1_offset);
	writel(0x7fffffff, base + ccu_clk->policy1_mask1_offset);
	writel(0x7fffffff, base + ccu_clk->policy2_mask1_offset);
	writel(0x7fffffff, base + ccu_clk->policy3_mask1_offset);
    }
    /* start policy engine */
    reg = readl(base + ccu_clk->policy_ctl_offset);
    reg |= 5;
    writel(reg, base + ccu_clk->policy_ctl_offset);
    while (readl(base + ccu_clk->policy_ctl_offset) & 1);
    /* disable access */
    writel(0, base + ccu_clk->wr_access_offset);

    return ret;
}

static int hub_ccu_init(struct clk *clk)
{
    int ret;
    void __iomem *base;

    clk_dbg("%s \n",__func__);
    base = (void __iomem *)KONA_HUB_CLK_VA;
    ret = ccu_common_init(clk, base);

    return 0;
}

static int aon_ccu_init(struct clk *clk)
{
    int ret;
    void __iomem *base;

    clk_dbg("%s \n",__func__);
    base = (void __iomem *)KONA_AON_CLK_VA;
    ret = ccu_common_init(clk, base);

    return 0;
}

#if !(defined(CONFIG_ARCH_ISLAND) || defined(CONFIG_ARCH_HANA))
static int mm_ccu_init(struct clk *clk)
{
    int ret;
    void __iomem *base;

    clk_dbg("%s \n",__func__);
    base = (void __iomem *)KONA_MM_CLK_VA;
    ret = ccu_common_init(clk, base);

    return 0;
}
#endif /* !(defined(CONFIG_ARCH_ISLAND)) */

static int proc_ccu_init(struct clk *clk)
{
    return 0;
}

/* CCU specific initialization should be done here.
 * This is called during CCU init and should be before any clocks usage.
 * 1. Set the CCU specific frequency policy
 * 2. Intial gating policies
 */
static int kona_master_ccu_init(struct clk *clk)
{
    int ret;
    void __iomem *base;

    clk_dbg("%s \n",__func__);
    base = (void __iomem *)KONA_KPM_CLK_VA;
    ret = ccu_common_init(clk, base);

    return 0;
}

static int kona_slave_ccu_init(struct clk *clk)
{
    int ret;
    void __iomem *base;

    clk_dbg("%s \n",__func__);
    base = (void __iomem *)KONA_KPS_CLK_VA;
    ret = ccu_common_init(clk, base);

    return 0;
}

static int ccu_init(struct clk *c)
{
    int ret = 0;

    clk_dbg ("%s %s\n", __func__, c->name);
    switch(c->ccu_id) {
    case BCM2165x_ROOT_CCU:
	ret = root_ccu_init(c);
	break;
    case BCM2165x_HUB_CCU:
	#ifndef CONFIG_ARCH_HANA
	ret = hub_ccu_init(c);
	#endif
	break;
    case BCM2165x_AON_CCU:
	ret = aon_ccu_init(c);
	break;
    case BCM2165x_MM_CCU:
	#if !(defined(CONFIG_ARCH_ISLAND) || defined(CONFIG_ARCH_HANA))
	ret = mm_ccu_init(c);
	#endif
	break;
    case BCM2165x_KONA_MST_CCU:
	ret = kona_master_ccu_init(c);
	break;
    case BCM2165x_PROC_CCU:
	ret = proc_ccu_init(c);
	break;
    case BCM2165x_KONA_SLV_CCU:
	ret = kona_slave_ccu_init(c);
	break;
    default:
	clk_dbg("Invalid CCU ID\n");
	ret = -EINVAL;
    }

    return ret;
}

struct clk_ops ccu_clk_ops = {
    	.init		= 	ccu_init,
	.enable		=	ccu_clk_enable,
	.get_rate	=	ccu_clk_get_rate,
};

/* bus clocks */
static int bus_clk_enable(struct clk *c, int enable)
{
	struct bus_clock *bus_clk = to_bus_clk(c);
	void __iomem *base;
	int reg, ret = 0;

	base = ioremap(bus_clk->ccu_clk_mgr_base, SZ_4K);
	if(!base)
		return -ENOMEM;

	/* enable access */
	writel(CLK_WR_ACCESS_PASSWORD, base + bus_clk->wr_access_offset);

	/* enable gating */
	reg = readl(base + bus_clk->clkgate_offset);
	clk_dbg ("Before %s, %s gating reg(%p): 0x%08x\n", enable?"enable":"disable\n", c->name, (base + bus_clk->clkgate_offset), reg);
	clk_dbg ("status_mask: 0x%08lx \n", bus_clk->stprsts_mask);
	if (!!(reg&bus_clk->stprsts_mask) == !!enable)
		clk_dbg ("%s already %s\n", c->name, enable?"enabled":"disabled");
	else if (enable) {
	//	reg |= bus_clk->hw_sw_gating_mask;
		reg |= bus_clk->clk_en_mask;
		writel(reg, base + bus_clk->clkgate_offset);
		while(! (readl(base + bus_clk->clkgate_offset) & bus_clk->stprsts_mask));
	}
	else {
	//	reg |= bus_clk->hw_sw_gating_mask;
		reg &= ~bus_clk->clk_en_mask;
		writel(reg, base + bus_clk->clkgate_offset);
		while(readl(base + bus_clk->clkgate_offset) & bus_clk->stprsts_mask);
	}

	/* disable access */
	writel(0, base + bus_clk->wr_access_offset);

	iounmap(base);
	return ret;
}

static unsigned long bus_clk_get_rate(struct clk *c)
{
	struct bus_clock *bus_clk = to_bus_clk(c);
	struct ccu_clock *ccu_clk;

	BUG_ON(!c->parent);
	ccu_clk= to_ccu_clk(c->parent);

	c->rate = bus_clk->freq_tbl[ccu_clk->freq_id];
	c->div = ccu_clk->freq_tbl[ccu_clk->freq_id]/c->rate;
	return c->rate;
}

static int bus_clk_init(struct clk *clk)
{
    int ret=0, reg;
    struct bus_clock *bus_clk = to_bus_clk(clk);
    void __iomem *base;
    int clk_status = 0;

    if ((clk->flags & SW_GATE) && (clk->flags & AUTO_GATE))
	return -EINVAL;
    base = ioremap (bus_clk->ccu_clk_mgr_base, SZ_4K);
    if (!base) {
	printk (KERN_INFO "%s ioremap error\n", __func__);
	return -ENOMEM;
    }
    /* enable access */
    writel(CLK_WR_ACCESS_PASSWORD, base + bus_clk->wr_access_offset);
    /* clkgate */
    reg = readl(base + bus_clk->clkgate_offset);
    clk_status = !!(reg & bus_clk->stprsts_mask);
    clk_dbg ("%s entry: %s  %s \n", __func__, clk->name, clk_status?"enabled":"disabled");
    /*If the clock is already enabled by ASIC/bootloader/early kernel inits,
     * make use_cnt = 1 */
    if (clk_status)
	 clk->use_cnt = clk->use_cnt + 1;

    if(clk->flags & SW_GATE) {
	reg |= bus_clk->hw_sw_gating_mask;
    }else if (clk->flags & AUTO_GATE)
	reg &= ~bus_clk->hw_sw_gating_mask;

    writel(reg, base + bus_clk->clkgate_offset);

    /* disable access */
    writel(0, base + bus_clk->wr_access_offset);
    iounmap (base);

    if ((clk_status == 0) && (clk->flags & ENABLE_ON_INIT)) {
	clk->ops->enable(clk, 1);
	clk->use_cnt = 1;
    }
    if((clk_status == 1) && (clk->flags & DISABLE_ON_INIT)) {
	clk->ops->enable(clk, 0);
	clk->use_cnt = 0;
    }

    return ret;
}


struct clk_ops bus_clk_ops = {
    	.init		= 	bus_clk_init,
	.enable		=	bus_clk_enable,
	.get_rate	=	bus_clk_get_rate,
};

/* reference clocks */
static int ref_clk_enable(struct clk *c, int enable)
{
	return 0;
}

struct clk_ops ref_clk_ops = {
    	.init		= 	NULL,
	.enable		=	ref_clk_enable,
	.set_rate	=	common_set_rate,
	.get_rate	=	common_get_rate,
	.round_rate	=	common_round_rate,
	.set_parent	=	common_set_parent,
};

#ifdef CONFIG_DEBUG_FS

static int test_func_invoke(void *data, u64 val)
{
    if (val == 1) {
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
    struct peri_clock *peri_clk;
    struct bus_clock *bus_clk;
    int val;
    int enabled = 0;
    void __iomem *base ;


	clk_dbg("%s clock id:: %d, clock end: %d \n", __func__, c->id, BCM2165x_CLK_END);
    if(c->id < BCM2165x_CLK_END) {
	peri_clk = to_peri_clk(c);
	base = ioremap (peri_clk->ccu_clk_mgr_base, SZ_4K);
	if (!base)
	    return -ENOMEM;
	val = readl(base + peri_clk->clkgate_offset);
	clk_dbg("%s clock gate_reg: %p \n", c->name, (base + peri_clk->clkgate_offset));
	enabled = val & peri_clk->stprsts_mask;
	clk_dbg("status_mask: 0x%lx, clock enabled/disabled:%d \n", peri_clk->stprsts_mask, !!enabled);
    } else {
    	bus_clk = to_bus_clk(c);
	base = ioremap (bus_clk->ccu_clk_mgr_base, SZ_4K);
	if (!base)
	    return -ENOMEM;
	val = readl(base + bus_clk->clkgate_offset);
	clk_dbg("%s bus clock gate_reg: %p \n", c->name, (base + bus_clk->clkgate_offset));
	enabled = val & bus_clk->stprsts_mask;
	clk_dbg("status_mask: 0x%lx, clock enabled/disabled:%d \n", bus_clk->stprsts_mask, !!enabled);
    }
    iounmap (base);
    return !!enabled;
}

static int clk_debug_get_status(void *data, u64 *val)
{
    struct clk *clock = data;
    *val = _get_clk_status(clock);
    clk_dbg("%s is %s \n", clock->name, *val?"enabled":"disabled");

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_status_fops, clk_debug_get_status, NULL, "%llu\n");



static int clk_debug_set_enable(void *data, u64 val)
{
    struct clk *clock = data;
    if (val == 1)
	clk_enable(clock);
    else if (val == 0)
	clk_disable(clock);
    else
	clk_dbg("Invalid value \n");

    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_enable_fops, NULL, clk_debug_set_enable, "%llu\n");

static int clk_parent_show(struct seq_file *seq, void *p)
{
	struct clk *clock = seq->private;

	seq_printf(seq, "name   -- %s\n", clock->name);
	if(clock->parent)
		seq_printf(seq, "parent -- %s\n", clock->parent->name);
	else
		seq_printf(seq, "parent -- NULL\n");

	return 0;
}

static int fops_parent_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_parent_show, inode->i_private);
}

static const struct file_operations clock_parent_fops = {
	.open			= fops_parent_open,
	.read			= seq_read,
	.llseek 		= seq_lseek,
	.release		= single_release,
};

static int clk_source_show(struct seq_file *seq, void *p)
{
	struct clk *clock = seq->private;

	if (clock->src) {
		int i;
		seq_printf(seq, "clock source for %s\n", clock->name);
		for (i=0; i<clock->src->total; i++) {
			seq_printf(seq, "%d	%s\n", i, clock->src->parents[i]->name);
		}
	}
	else
		seq_printf(seq, "no source for %s\n", clock->name);
	return 0;
}

static int fops_source_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_source_show, inode->i_private);
}

static const struct file_operations clock_source_fops = {
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
	if (!dent_clk_root_dir)
		return -ENOMEM;

	if (!debugfs_create_u32("debug", 0644, dent_clk_root_dir, (int*)&clk_debug))
		return -ENOMEM;

	if (!debugfs_create_file("debug_func", 0644, dent_clk_root_dir, NULL, &test_func_fops))
		return -ENOMEM;

	return 0;
}

int __init clock_debug_add_clock(struct clk *c)
{
	struct dentry *dent_clk_dir=0, *dent_rate=0, *dent_enable=0,
		*dent_status=0, *dent_div=0, *dent_usr_cnt=0, *dent_id=0,
		*dent_parent=0, *dent_source=0;
	BUG_ON(!dent_clk_root_dir);

	/* create root clock dir /clock/clk_a */
	dent_clk_dir	=	debugfs_create_dir(c->name, dent_clk_root_dir);
	if(!dent_clk_dir)
		goto err;

	/* file /clock/clk_a/enable */
	dent_enable       =       debugfs_create_file("enable", 0644, dent_clk_dir, c, &clock_enable_fops);
	if(!dent_enable)
		goto err;

	/* file /clock/clk_a/status */
	dent_status     =       debugfs_create_file("status", 0644, dent_clk_dir, c, &clock_status_fops);
	if(!dent_status)
	     goto err;

	/* file /clock/clk_a/rate */
	dent_rate	=	debugfs_create_file("rate", 0644, dent_clk_dir, c, &clock_rate_fops);
	if(!dent_rate)
		goto err;

	/* file /clock/clk_a/div */
	dent_div	=	debugfs_create_u32("div", 0444, dent_clk_dir, (unsigned int*)&c->div);
	if(!dent_div)
		goto err;

	/* file /clock/clk_a/usr_cnt */
	dent_usr_cnt	=	debugfs_create_u32("usr_cnt", 0444, dent_clk_dir, (unsigned int*)&c->use_cnt);
	if(!dent_usr_cnt)
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
	debugfs_remove(dent_usr_cnt);
	debugfs_remove(dent_id);
	debugfs_remove(dent_parent);
	debugfs_remove(dent_source);
	debugfs_remove(dent_clk_dir);
	return -ENOMEM;
}
#endif
