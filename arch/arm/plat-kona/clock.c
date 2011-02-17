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
#include <asm/io.h>
#include <mach/rdb/brcm_rdb_kproc_clk_mgr_reg.h>

#ifdef CONFIG_SMP
#include <asm/cpu.h>
#endif

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

/* global spinlock for clock API */
static DEFINE_SPINLOCK(clk_lock);

static int __clk_enable(struct clk *clk)
{
	int ret = 0;

	if (!clk->ops || !clk->ops->enable)
		return -EINVAL;

	/* enable parent clock first */
	if (clk->parent)
		ret = __clk_enable(clk->parent);

	if (ret)
		return ret;

	if (clk->use_cnt++ == 0)
		ret = clk->ops->enable(clk, 1);

	return ret;
}

int clk_enable(struct clk *clk)
{
	int ret;
	unsigned long flags;

	if (!clk)
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

	if (--clk->use_cnt == 0)
		clk->ops->enable(clk, 0);

	/* disable parent */
	if (clk->parent)
		__clk_disable(clk->parent);
}

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	if (!clk)
		return;

	spin_lock_irqsave(&clk_lock, flags);
	__clk_disable(clk);
	spin_unlock_irqrestore(&clk_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long flags, rate;

	if (!clk || !clk->ops || !clk->ops->get_rate)
		return 0;

	spin_lock_irqsave(&clk_lock, flags);
	rate = clk->ops->get_rate(clk);
	spin_unlock_irqrestore(&clk_lock, flags);
	return rate;
}
EXPORT_SYMBOL(clk_get_rate);

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags, actual;

	if (!clk || !clk->ops || !clk->ops->round_rate)
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

	if (!clk || !clk->ops || !clk->ops->set_rate)
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

	if (!clk)
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

	if (!clk || !parent || !clk->ops || !clk->ops->set_parent)
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
		new_rate = c->src->parents[i]->rate/div;
		/* get the min diff */
		if(new_rate-rate < diff) {
			diff = new_rate-rate;
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

	clk_dbg ("xtal %d, int %d, frac %d, vco %d\n", xtal, ndiv_int, ndiv_frac, vco_rate);
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
	.enable		=	proc_clk_enable,
	.set_rate	=	proc_clk_set_rate,
	.get_rate	=	proc_clk_get_rate,
	.round_rate	=	proc_clk_round_rate,
	.set_parent	=	common_set_parent,
};

static int peri_clk_enable(struct clk *c, int enable)
{
	int ret=0;
	return ret;
}

struct clk_ops peri_clk_ops = {
	.enable		=	peri_clk_enable,
	.set_rate	=	common_set_rate,
	.get_rate	=	common_get_rate,
	.round_rate	=	common_round_rate,
	.set_parent	=	common_set_parent,
};

static int ccu_clk_enable(struct clk *c, int enable)
{
	int ret=0;
	return ret;
}

struct clk_ops ccu_clk_ops = {
	.enable		=	ccu_clk_enable,
	.set_rate	=	common_set_rate,
	.get_rate	=	common_get_rate,
	.round_rate	=	common_round_rate,
	.set_parent	=	common_set_parent,
};

/* reference clocks */
static int ref_clk_enable(struct clk *c, int enable)
{
	return 0;
}

struct clk_ops ref_clk_ops = {
	.enable		=	ref_clk_enable,
	.set_rate	=	common_set_rate,
	.get_rate	=	common_get_rate,
	.round_rate	=	common_round_rate,
	.set_parent	=	common_set_parent,
};

#ifdef CONFIG_DEBUG_FS
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
	return dent_clk_root_dir?-ENOMEM:0;
}

int __init clock_debug_add_clock(struct clk *c)
{
	struct dentry *dent_clk_dir=0, *dent_rate=0, *dent_div=0, *dent_usr_cnt=0, *dent_id=0,
		      *dent_parent=0, *dent_source=0;
	BUG_ON(!dent_clk_root_dir);

	/* create root clock dir /clock/clk_a */
	dent_clk_dir	=	debugfs_create_dir(c->name, dent_clk_root_dir);
	if(!dent_clk_dir)
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
