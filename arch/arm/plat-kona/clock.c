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
#include <asm/clkdev.h>
#include <mach/clock.h>
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
		div = c->src->parents[i].rate / rate;
		new_rate = c->src->parents[i].rate/div;
		/* get the min diff */
		if(new_rate-rate < diff) {
			diff = new_rate-rate;
			ind = i;
		}
	}
	return c->src->parents[ind].rate;
}

static int common_set_rate(struct clk *c, unsigned long rate)
{
	int i, ind = 0;
	unsigned long diff;

	diff = rate;

	if (!c->parent) {
		return c->rate;
	}

	BUG_ON(!c->src || !c->src->total);

	for (i=0; i<c->src->total;i++) {
		unsigned long new_rate, div;
		/* round to the new rate */		
		div = c->src->parents[i].rate / rate;
		new_rate = c->src->parents[i].rate/div;
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
		if (is_same_clock(parent, &c->src->parents[i]))
			return 0;
	}
	return -EINVAL;
}

/* Proc clocks */
static int proc_clk_enable(struct clk *c, int enable)
{
	int ret=0;
	return ret;
}
static int proc_clk_set_rate(struct clk *c, unsigned long rate)
{
	int ret=0;
	c->rate = rate;
	return ret;
}
static unsigned long proc_clk_get_rate(struct clk *c)
{
	unsigned int ret = 0;
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
	struct clk *clock = p;

	if(clock->parent)
		seq_printf(seq, "parent->%s\n", clock->parent->name);
	else
		seq_printf(seq, "parent-> NULL\n");
	seq_printf(seq, "name->%s\n", clock->name);
	return 0;
}

static int fops_parent_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_parent_show, NULL);
}

static const struct file_operations clock_parent_fops = {
	.open			= fops_parent_open,
	.read			= seq_read,
	.llseek 		= seq_lseek,
	.release		= single_release,
};

static int clk_source_show(struct seq_file *seq, void *p)
{
	struct clk *clock = p;

	if (clock->src) {
		int i;
		seq_printf(seq, "clock source for %s\n", clock->name);
		for (i=0; i<clock->src->total; i++) {
			seq_printf(seq, "%d	%s\n", i, clock->src->parents->name);
		}
	}
	else
		seq_printf(seq, "no source for %s\n", clock->name);
	return 0;
}

static int fops_source_open(struct inode *inode, struct file *file)
{
	return single_open(file, clk_source_show, NULL);
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

err:
	debugfs_remove(dent_rate);
	debugfs_remove(dent_div);
	debugfs_remove(dent_usr_cnt);
	debugfs_remove(dent_id);
	debugfs_remove(dent_parent);
	debugfs_remove(dent_source);
	debugfs_remove(dent_clk_dir);
	return 0;
}
#endif
