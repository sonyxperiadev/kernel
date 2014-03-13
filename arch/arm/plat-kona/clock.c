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
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/clkdev.h>
#include <asm/clkdev.h>
#include <plat/clock.h>
#include <asm/io.h>
#include <plat/pi_mgr.h>
#ifdef CONFIG_SMP
#include <asm/cpu.h>
#include <linux/smp.h>
#endif

/* global spinlock for clock API */
static DEFINE_SPINLOCK(clk_gen_lock);
static DEFINE_SPINLOCK(gen_access_lock);

int clk_debug = 0;
u32 gen_lock_flag;

/* Added for panic handler to know AP CCU information */
struct ccu_clk *ccu_clk_list[MAX_CCU_COUNT];
/* num_ccu is used because mach can override the max_ccu_count */
static u32 num_ccu;

/*clk_dfs_request_update -  action*/
enum {
	CLK_STATE_CHANGE,	/*param = 1 => enable. param =0 => disable */
	CLK_RATE_CHANGE		/*param = rate */
};

#ifdef CONFIG_KONA_CLK_TRACE
static u32 *clk_trace_v;
static dma_addr_t clk_trace_p;
#endif

/*fwd declarations....*/
static int __pll_clk_enable(struct clk *clk);
static int __pll_chnl_clk_enable(struct clk *clk);
static int peri_clk_set_voltage_lvl(struct peri_clk *peri_clk, int voltage_lvl);
#ifdef CONFIG_KONA_PI_MGR
static int clk_dfs_request_update(struct clk *clk, u32 action, u32 param);
#endif
static int ccu_init_state_save_buf(struct ccu_clk *ccu_clk);

static int ccu_access_lock(struct ccu_clk *ccu_clk, unsigned long *flags);
static int ccu_access_unlock(struct ccu_clk *ccu_clk, unsigned long *flags);

#define clk_use_cnt(clk)	((clk)->use_cnt)

static inline int clk_use_cnt_incr_post(struct clk *clk)
{
	int use_cnt = clk->use_cnt++;
#ifdef CONFIG_KONA_CLK_TRACE
	if (clk_trace_v)
		clk_trace_v[clk->id] = clk->use_cnt;
#endif
	return use_cnt;
}

static inline int clk_use_cnt_decr_pre(struct clk *clk)
{
	int use_cnt = --clk->use_cnt;
#ifdef CONFIG_KONA_CLK_TRACE
	if (clk_trace_v)
		clk_trace_v[clk->id] = clk->use_cnt;
#endif
	return use_cnt;
}

static int __ccu_clk_init(struct clk *clk)
{
	struct ccu_clk *ccu_clk;
	int ret = 0;
	ccu_clk = to_ccu_clk(clk);

	clk_dbg("%s - %s\n", __func__, clk->name);

	CCU_ACCESS_EN(ccu_clk, 1);

	INIT_LIST_HEAD(&ccu_clk->clk_list);
	/*
	   Initilize CCU context save buf if CCU state save parameters
	   are defined for this CCU.
	 */
	if (ccu_clk->ccu_state_save)
		ccu_init_state_save_buf(ccu_clk);

	if (clk->ops && clk->ops->init)
		ret = clk->ops->init(clk);

	if (clk->flags & CCU_KEEP_UNLOCKED) {
		ccu_clk->write_access_en_count = 0;
		/* enable write access */
		ccu_write_access_enable(ccu_clk, true);
	}
	ccu_write_access_enable(ccu_clk, true);
	/*Keep CCU intr disabled by default*/
	ccu_int_enable(ccu_clk, ACT_INT, false);
	ccu_int_enable(ccu_clk, TGT_INT, false);
	ccu_write_access_enable(ccu_clk, false);

	CCU_ACCESS_EN(ccu_clk, 0);

	return ret;
}

static int __peri_clk_init(struct clk *clk)
{
	struct peri_clk *peri_clk;
	int ret = 0;

	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	peri_clk = to_peri_clk(clk);
	BUG_ON(peri_clk->ccu_clk == NULL);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);

	/*Add DFS request */
#ifdef CONFIG_KONA_PI_MGR
	if (peri_clk->clk_dfs) {
		BUG_ON(peri_clk->ccu_clk->pi_id == -1);
		ret = pi_mgr_dfs_add_request_ex(&peri_clk->clk_dfs->dfs_node,
						(char *)clk->name,
						peri_clk->ccu_clk->pi_id,
						PI_MGR_DFS_MIN_VALUE,
						PI_MGR_DFS_WIEGHTAGE_NONE);
		if (ret)
			clk_dbg
			    ("%s: failed to add dfs node for the clock: %s\n",
			     __func__, clk->name);
	}
#endif

	if (clk->ops && clk->ops->init)
		ret = clk->ops->init(clk);

	INIT_LIST_HEAD(&clk->list);
	list_add(&clk->list, &peri_clk->ccu_clk->clk_list);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);
	return ret;
}

static int __bus_clk_init(struct clk *clk)
{
	int ret = 0;
	struct bus_clk *bus_clk;

	bus_clk = to_bus_clk(clk);
	BUG_ON(bus_clk->ccu_clk == NULL);

	clk_dbg("%s - %s\n", __func__, clk->name);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 1);

	/*Add DSF request */
#ifdef CONFIG_KONA_PI_MGR
	if (bus_clk->clk_dfs) {
		BUG_ON(bus_clk->ccu_clk->pi_id == -1);
		ret = pi_mgr_dfs_add_request_ex(&bus_clk->clk_dfs->dfs_node,
						(char *)clk->name,
						bus_clk->ccu_clk->pi_id,
						PI_MGR_DFS_MIN_VALUE,
						PI_MGR_DFS_WIEGHTAGE_NONE);
		if (ret)
			clk_dbg
			    ("%s: failed to add dfs node for the clock: %s\n",
			     __func__, clk->name);
	}
#endif

	if (clk->ops && clk->ops->init)
		ret = clk->ops->init(clk);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 0);

	INIT_LIST_HEAD(&clk->list);
	list_add(&clk->list, &bus_clk->ccu_clk->clk_list);

	return ret;
}

static int __ref_clk_init(struct clk *clk)
{
	struct ref_clk *ref_clk;
	int ret = 0;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);
	BUG_ON(ref_clk->ccu_clk == NULL);

	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->init)
		ret = clk->ops->init(clk);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 0);
	INIT_LIST_HEAD(&clk->list);
	list_add(&clk->list, &ref_clk->ccu_clk->clk_list);

	return ret;
}

static int __pll_clk_init(struct clk *clk)
{
	struct pll_clk *pll_clk;
	int ret = 0;

	clk_dbg("%s, clock name: %s\n", __func__, clk->name);
	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);
	BUG_ON(pll_clk->ccu_clk == NULL);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->init)
		ret = clk->ops->init(clk);

	if (CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)) {
		__pll_clk_enable(clk);
	}

	else if (CLK_FLG_ENABLED(clk, DISABLE_ON_INIT)) {
		if (clk->ops && clk->ops->enable)
			clk->ops->enable(clk, 0);
	}
	INIT_LIST_HEAD(&clk->list);
	list_add(&clk->list, &pll_clk->ccu_clk->clk_list);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);
	return ret;
}

static int __pll_chnl_clk_init(struct clk *clk)
{
	struct pll_chnl_clk *pll_chnl_clk;
	int ret = 0;

	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);
	BUG_ON(pll_chnl_clk->ccu_clk == NULL);

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->init)
		ret = clk->ops->init(clk);

	INIT_LIST_HEAD(&clk->list);
	list_add(&clk->list, &pll_chnl_clk->ccu_clk->clk_list);

	if (CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)) {
		__pll_chnl_clk_enable(clk);
	}

	else if (CLK_FLG_ENABLED(clk, DISABLE_ON_INIT)) {
		if (clk->ops && clk->ops->enable)
			clk->ops->enable(clk, 0);
	}

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);
	return ret;
}

static int __misc_clk_init(struct clk *clk)
{
	int ret = 0;

	if (clk->ops && clk->ops->init)
		ret = clk->ops->init(clk);
	return ret;
}

static int __clk_init(struct clk *clk)
{
	int ret = 0;
	clk_dbg("%s - %s clk->init = %d clk->clk_type = %d\n", __func__,
		clk->name, clk->init, clk->clk_type);
	if (!clk->init) {
		switch (clk->clk_type) {
		case CLK_TYPE_CCU:
			ret = __ccu_clk_init(clk);
			break;

		case CLK_TYPE_PERI:
			ret = __peri_clk_init(clk);
			break;

		case CLK_TYPE_BUS:
			ret = __bus_clk_init(clk);
			break;

		case CLK_TYPE_REF:
			ret = __ref_clk_init(clk);
			break;

		case CLK_TYPE_PLL:
			ret = __pll_clk_init(clk);
			break;

		case CLK_TYPE_PLL_CHNL:
			ret = __pll_chnl_clk_init(clk);
			break;
		case CLK_TYPE_MISC:
			ret = __misc_clk_init(clk);
			break;
		default:
			clk_dbg("%s - %s: unknown clk_type\n", __func__,
				clk->name);
			if (clk->ops && clk->ops->init)
				ret = clk->ops->init(clk);
			break;
		}
		clk->init = 1;
	}
	return ret;
}

static struct ccu_clk *get_ccu_clk(struct clk *clk)
{
	struct ccu_clk *ccu_clk = NULL;
	switch (clk->clk_type) {

	case CLK_TYPE_CCU:
		ccu_clk = to_ccu_clk(clk);
		break;

	case CLK_TYPE_PERI:
		ccu_clk = to_peri_clk(clk)->ccu_clk;
		BUG_ON(ccu_clk == NULL);
		break;

	case CLK_TYPE_BUS:
		ccu_clk = to_bus_clk(clk)->ccu_clk;
		BUG_ON(ccu_clk == NULL);
		break;

	case CLK_TYPE_REF:
		ccu_clk = to_ref_clk(clk)->ccu_clk;
		BUG_ON(ccu_clk == NULL);
		break;

	case CLK_TYPE_PLL:
		ccu_clk = to_pll_clk(clk)->ccu_clk;
		BUG_ON(ccu_clk == NULL);
		break;

	case CLK_TYPE_PLL_CHNL:
		ccu_clk = to_pll_chnl_clk(clk)->ccu_clk;
		BUG_ON(ccu_clk == NULL);
		break;

	case CLK_TYPE_CORE:
		ccu_clk = to_core_clk(clk)->ccu_clk;
		BUG_ON(ccu_clk == NULL);
		break;
	}
	return ccu_clk;
}

/*This function is used to lock the CCU lock of dependent clock
Does nothing if dependent clock is also under same CCU.
ccu_clk can be NULL
*/

/* Using flags after acquiring clk_lock. If a core has got a spinlock,
   then it sets the bit corresponding to its cpu_id in the flag variable.
   If the core again tries to acquire the clk_lock, it results in a deadlock.
   To detect that we are adding a BUG(). The concept is, only the core
   that has got the lock can release it, and if it has already got a lock,
   it shouldn't try to acquire it again thereby creating a freeze.
   Also, we use access locks, to prevent the corruption of the flag variable.
   For clocks belonging to ccu, we have a dedicated access_lock and flag.
   And for misc clocks, there is a global access lock(gen_access_clk)
   and flag(gen_lock_flag).
*/

static int dep_clk_lock(struct ccu_clk *ccu_clk, struct clk *dep_clk,
	unsigned long *flags)
{
	struct ccu_clk *dep_ccu_clk = get_ccu_clk(dep_clk);
	int cpu_id;
	unsigned long access_flags;
	if (dep_ccu_clk == ccu_clk)
		return 0;
	clk_dbg("%s:ccu_clk =  %s : dep_ccu_clk->name = %s\n", __func__,
			ccu_clk ? ccu_clk->clk.name : "NULL",
			dep_ccu_clk ? dep_ccu_clk->clk.name : "NULL");
	cpu_id = 1 << get_cpu();
	if (dep_ccu_clk) {
		if (dep_ccu_clk->lock_flag & cpu_id) {
			printk(KERN_ALERT "same core getting lock again");
			printk(KERN_ALERT "%s:ccu_clk =  %s : dep_ccu = %s\n",
				__func__, ccu_clk->clk.name,
				dep_ccu_clk->clk.name);
			printk(KERN_ALERT "cpu=%d, flag=%d", cpu_id,
					 dep_ccu_clk->lock_flag);
			BUG();
		}

	/* Acquire clk_lock, update the flag immediately with access lock */

		/*  Allow nesting of spin_locks below to get rid of lockdep
		 warnings,because it checks for the class type and not the
		exact objects structure	embedding clk_lock, it warns if the
		two objects are of same class type, this is a valid case here
		For ex: root_ccu and aon_ccu belongs to same ccu_clk class */

		spin_lock_irqsave_nested(&dep_ccu_clk->clk_lock, *flags, 1);
		ccu_access_lock(dep_ccu_clk, &access_flags);
		dep_ccu_clk->lock_flag |= cpu_id;
		ccu_access_unlock(dep_ccu_clk, &access_flags);
	} else {
		if (gen_lock_flag & cpu_id) {
			printk(KERN_ALERT "same core getting lock again");
			printk(KERN_ALERT "%s:ccu_clk =  %s : dep_clk = %s\n",
				__func__, ccu_clk->clk.name, dep_clk->name);
			printk(KERN_ALERT "cpu=%d, genlockflag=%d", cpu_id,
					 gen_lock_flag);
			BUG();
		}
		spin_lock_irqsave(&clk_gen_lock, *flags);
		spin_lock_irqsave(&gen_access_lock, access_flags);
		gen_lock_flag |= cpu_id;
		spin_unlock_irqrestore(&gen_access_lock, access_flags);
	}
	put_cpu();
	return 0;
}

/*This function is used to unlock the CCU lock of dependent clock
Does nothing if dependent clock is also under same CCU.
ccu_clk can be NULL
*/
static int dep_clk_unlock(struct ccu_clk *ccu_clk, struct clk *dep_clk,
	unsigned long *flags)

{
	struct ccu_clk *dep_ccu_clk = get_ccu_clk(dep_clk);
	int cpu_id;
	unsigned long access_flags;
	if (dep_ccu_clk == ccu_clk)
		return 0;
	clk_dbg("%s:ccu_clk =  %s : dep_ccu_clk->name = %s\n", __func__,
			ccu_clk ? ccu_clk->clk.name : "NULL",
			dep_ccu_clk ? dep_ccu_clk->clk.name : "NULL");
	cpu_id = 1 << get_cpu();
	if (dep_ccu_clk) {
		if (cpu_id & dep_ccu_clk->lock_flag) {
			/* Clear the flag and release the clk_lock */
			ccu_access_lock(dep_ccu_clk, &access_flags);
			dep_ccu_clk->lock_flag &= ~cpu_id;
			ccu_access_unlock(dep_ccu_clk, &access_flags);
			spin_unlock_irqrestore(&dep_ccu_clk->clk_lock, *flags);
		} else {
			printk(KERN_ALERT "Has not acquired lock previously");
			printk(KERN_ALERT "%s:ccu_clk =  %s : dep_ccu = %s\n",
				__func__, ccu_clk->clk.name,
				dep_ccu_clk->clk.name);
			printk(KERN_ALERT "cpu=%d, flag=%d", cpu_id,
					 dep_ccu_clk->lock_flag);
			BUG();
		}
	} else {
		if (cpu_id & gen_lock_flag) {
			spin_lock_irqsave(&gen_access_lock, access_flags);
			gen_lock_flag &= ~cpu_id;
			spin_unlock_irqrestore(&gen_access_lock, access_flags);
			spin_unlock_irqrestore(&clk_gen_lock, *flags);
		} else {
			printk(KERN_ALERT "Has not acquired lock previously");
			printk(KERN_ALERT "%s:ccu_clk =  %s : dep_ccu = %s\n",
				__func__, ccu_clk->clk.name, dep_clk->name);
			printk(KERN_ALERT "cpu=%d, genlockflag=%d", cpu_id,
					 gen_lock_flag);
			BUG();
		}
	}
	put_cpu();
	return 0;
}

static int clk_lock(struct clk *clk, unsigned long *flags)
{
	struct ccu_clk *ccu_clk = get_ccu_clk(clk);
	int cpu_id;
	unsigned long access_flags;
	clk_dbg("%s:ccu_clk =  %s, clk = %s\n", __func__,
			ccu_clk ? ccu_clk->clk.name : "NIL",
			clk ? clk->name : "NIL");
	cpu_id = 1 << get_cpu();
	if (ccu_clk) {
		if (ccu_clk->lock_flag & cpu_id) {
			printk(KERN_ALERT "same core getting lock again");
			printk(KERN_ALERT "%s:ccu_clk =  %s, clk = %s\n",
				__func__, ccu_clk->clk.name, clk->name);
			printk(KERN_ALERT "cpu=%d, flag=%d", cpu_id,
					ccu_clk->lock_flag);
			BUG();
		}
		spin_lock_irqsave(&ccu_clk->clk_lock, *flags);
		ccu_access_lock(ccu_clk, &access_flags);
		ccu_clk->lock_flag |= cpu_id;
		ccu_access_unlock(ccu_clk, &access_flags);
	} else {
		if (gen_lock_flag & cpu_id) {
			printk(KERN_ALERT "same core getting lock again");
			printk(KERN_ALERT "%s: clk - %s\n",
				__func__, clk->name);
			printk(KERN_ALERT "cpu=%d, genlockflag=%d", cpu_id,
					 gen_lock_flag);
			BUG();
		}
		spin_lock_irqsave(&clk_gen_lock, *flags);
		spin_lock_irqsave(&gen_access_lock, access_flags);
		gen_lock_flag |= cpu_id;
		spin_unlock_irqrestore(&gen_access_lock, access_flags);
	}
	put_cpu();
	return 0;
}

static int clk_unlock(struct clk *clk, unsigned long *flags)
{
	struct ccu_clk *ccu_clk = get_ccu_clk(clk);
	int cpu_id;
	unsigned long access_flags;
	clk_dbg("%s:ccu_clk =  %s, clk = %s\n", __func__,
			ccu_clk ? ccu_clk->clk.name : "NIL",
			clk ? clk->name : "NIL");
	cpu_id = 1 << get_cpu();
	if (ccu_clk) {
		if (ccu_clk->lock_flag & cpu_id) {
			ccu_access_lock(ccu_clk, &access_flags);
			ccu_clk->lock_flag &= ~cpu_id;
			ccu_access_unlock(ccu_clk, &access_flags);
			spin_unlock_irqrestore(&ccu_clk->clk_lock, *flags);
		} else {
			printk(KERN_ALERT "Has not acquired lock previously");
			printk(KERN_ALERT "%s:ccu_clk =  %s, clk = %s\n",
				__func__, ccu_clk->clk.name, clk->name);
			printk(KERN_ALERT "cpu=%d, flag=%d", cpu_id,
					 ccu_clk->lock_flag);
			BUG();
		}
	} else {
		if (gen_lock_flag & cpu_id) {
			spin_lock_irqsave(&gen_access_lock, access_flags);
			gen_lock_flag &= ~cpu_id;
			spin_unlock_irqrestore(&gen_access_lock, access_flags);
			spin_unlock_irqrestore(&clk_gen_lock, *flags);
		} else {
			printk(KERN_ALERT "Has not acquired lock previously");
			printk(KERN_ALERT "%s: clk = %s\n",
				__func__, clk->name);
			printk(KERN_ALERT "cpu=%d, genlockflag=%d", cpu_id,
					 gen_lock_flag);
			BUG();
		}
	}
	put_cpu();
	return 0;
}

static int ccu_access_lock(struct ccu_clk *ccu_clk, unsigned long *flags)
{
		clk_dbg("%s:ccu_clk =  %s\n", __func__,
			ccu_clk ? ccu_clk->clk.name : "NULL");

	spin_lock_irqsave(&ccu_clk->access_lock, *flags);
	return 0;
}

static int ccu_access_unlock(struct ccu_clk *ccu_clk, unsigned long *flags)
{
	clk_dbg("%s:ccu_clk =  %s\n", __func__,
			ccu_clk ? ccu_clk->clk.name : "NULL");

	spin_unlock_irqrestore(&ccu_clk->access_lock, *flags);
	return 0;
}


int clk_init(struct clk *clk)
{
	int ret = 0;
	unsigned long flags;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	clk_dbg("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	ret = __clk_init(clk);
	clk_unlock(clk, &flags);

	return ret;
}

EXPORT_SYMBOL(clk_init);

static int __clk_reset(struct clk *clk)
{
	int ret = 0;

	if (!clk)
		return -EINVAL;

	if (!clk->ops || !clk->ops->reset)
		return -EINVAL;
	clk_dbg("%s - %s\n", __func__, clk->name);
	ret = clk->ops->reset(clk);

	return ret;
}

int clk_reset(struct clk *clk)
{
	int ret;
	unsigned long flags;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;
	 clk_dbg("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	ret = __clk_reset(clk);
	clk_unlock(clk, &flags);

	return ret;
}

EXPORT_SYMBOL(clk_reset);

static int __ccu_clk_enable(struct clk *clk)
{
	int ret = 0;
	int inx;
	struct ccu_clk *ccu_clk;
	unsigned long flgs;
	clk_dbg("%s ccu name:%s\n", __func__, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	ccu_clk = to_ccu_clk(clk);
	/*Make sure that all dependent & src clks are enabled/disabled*/
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s, Enabling dependant clock %s\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(ccu_clk, clk->dep_clks[inx], &flgs);
		__clk_enable(clk->dep_clks[inx]);
		dep_clk_unlock(ccu_clk, clk->dep_clks[inx], &flgs);
	}
	/*enable PI */
#ifdef CONFIG_KONA_PI_MGR
	if (ccu_clk->pi_id != -1) {
		struct pi *pi = pi_mgr_get(ccu_clk->pi_id);

		BUG_ON(!pi);
		pi_enable(pi, 1);
	}
#endif
	if (clk_use_cnt_incr_post(clk) == 0) {
		if (clk->ops && clk->ops->enable) {
			ret = clk->ops->enable(clk, 1);
		}
	}
	return ret;
}

static int __peri_clk_enable(struct clk *clk)
{
	struct peri_clk *peri_clk;
	int inx;
	int ret = 0;
	struct clk *src_clk;
	unsigned long flgs;

	clk_dbg("%s clock name: %s\n", __func__, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);
	peri_clk = to_peri_clk(clk);
	BUG_ON(!peri_clk->ccu_clk);

	if (!(clk->flags & DONOT_NOTIFY_STATUS_TO_CCU)
	    && !(clk->flags & AUTO_GATE)) {
		clk_dbg("%s: peri clock %s enable CCU\n", __func__, clk->name);
		__ccu_clk_enable(&peri_clk->ccu_clk->clk);
	}

	/*Make sure that all dependent & src clks are enabled/disabled */
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s, Enabling dependant clock %s\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(peri_clk->ccu_clk, clk->dep_clks[inx], &flgs);
		__clk_enable(clk->dep_clks[inx]);
		dep_clk_unlock(peri_clk->ccu_clk, clk->dep_clks[inx], &flgs);
	}

	/*Enable src clock, if valid */
	if (PERI_SRC_CLK_VALID(peri_clk)) {
		src_clk = GET_PERI_SRC_CLK(peri_clk);
		BUG_ON(!src_clk);
		clk_dbg("%s, after enabling source clock %s\n", __func__,
			src_clk->name);
		dep_clk_lock(peri_clk->ccu_clk, src_clk, &flgs);
		__clk_enable(src_clk);
		dep_clk_unlock(peri_clk->ccu_clk, src_clk, &flgs);
	}

	clk_dbg("%s:%s use count = %d\n", __func__, clk->name,
		peri_clk->clk.use_cnt);

	/*Increment usage count... return if already enabled */
	if (clk_use_cnt_incr_post(clk) == 0) {
		CCU_ACCESS_EN(peri_clk->ccu_clk, 1);
		/*Update DFS request before enabling the clock */
#ifdef CONFIG_KONA_PI_MGR
		if (peri_clk->clk_dfs) {
			clk_dfs_request_update(clk, CLK_STATE_CHANGE, 1);
		}
#endif
		if (CLK_FLG_ENABLED(clk, ENABLE_HVT))
			peri_clk_set_voltage_lvl(peri_clk, VLT_HIGH);

		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 1);

		CCU_ACCESS_EN(peri_clk->ccu_clk, 0);
	}

	return ret;
}

static int __bus_clk_enable(struct clk *clk)
{
	struct bus_clk *bus_clk;
	int inx;
	int ret = 0;
	unsigned long flgs;

	BUG_ON(clk->clk_type != CLK_TYPE_BUS);

	clk_dbg("%s : %s use cnt= %d\n", __func__, clk->name, clk->use_cnt);

	bus_clk = to_bus_clk(clk);

	BUG_ON(bus_clk->ccu_clk == NULL);

	if (!(clk->flags & AUTO_GATE) && (clk->flags & NOTIFY_STATUS_TO_CCU)) {
		clk_dbg("%s: bus clock %s incrementing CCU count\n", __func__,
			clk->name);
		__ccu_clk_enable(&bus_clk->ccu_clk->clk);
	}

	/*Make sure that all dependent & src clks are enabled/disabled */
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s, Enabling dependant clock %s\n", __func__,
			clk->dep_clks[inx]->name);

		dep_clk_lock(bus_clk->ccu_clk, clk->dep_clks[inx], &flgs);
		__clk_enable(clk->dep_clks[inx]);
		dep_clk_unlock(bus_clk->ccu_clk, clk->dep_clks[inx], &flgs);

	}

	if (bus_clk->src_clk) {
		clk_dbg("%s src clock %s enable\n", __func__,
			bus_clk->src_clk->name);
		dep_clk_lock(bus_clk->ccu_clk, bus_clk->src_clk, &flgs);
		__clk_enable(bus_clk->src_clk);
		dep_clk_unlock(bus_clk->ccu_clk, bus_clk->src_clk, &flgs);
	}

	/*Increment usage count... return if already enabled */
	if (clk_use_cnt_incr_post(clk) == 0) {
		CCU_ACCESS_EN(bus_clk->ccu_clk, 1);
		/*Update DFS request before enabling the clock */
#ifdef CONFIG_KONA_PI_MGR
		if (bus_clk->clk_dfs) {
			clk_dfs_request_update(clk, CLK_STATE_CHANGE, 1);
		}
#endif
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 1);

		CCU_ACCESS_EN(bus_clk->ccu_clk, 0);
	}
	return ret;

}

static int __ref_clk_enable(struct clk *clk)
{
	int ret = 0;
	struct ref_clk *ref_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);

	if (clk_use_cnt_incr_post(clk) == 0) {
		CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 1);

		CCU_ACCESS_EN(ref_clk->ccu_clk, 0);
	}
	return ret;
}

static int __pll_clk_enable(struct clk *clk)
{
	int ret = 0;
	struct pll_clk *pll_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	if (clk_use_cnt_incr_post(clk) == 0) {
		CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 1);

		CCU_ACCESS_EN(pll_clk->ccu_clk, 0);
	}
	return ret;
}

static int __pll_chnl_clk_enable(struct clk *clk)
{
	int ret = 0;
	struct pll_chnl_clk *pll_chnl_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);

	__pll_clk_enable(&pll_chnl_clk->pll_clk->clk);

	if (clk_use_cnt_incr_post(clk) == 0) {
		CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 1);

		CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);
	}
	return ret;
}
static int __misc_clk_enable(struct clk *clk)
{
	int ret = 0;
	int inx;
	unsigned long flgs;

	/*Make sure that all dependent clks are enabled*/
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s, Enabling dependant clock %s\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(get_ccu_clk(clk), clk->dep_clks[inx], &flgs);
		__clk_enable(clk->dep_clks[inx]);
		dep_clk_unlock(get_ccu_clk(clk), clk->dep_clks[inx], &flgs);

	}
	if (clk_use_cnt_incr_post(clk) == 0) {
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 1);
	}
	return ret;

}

int __clk_enable(struct clk *clk)
{
	int ret = 0;
	clk_dbg("%s - %s\n", __func__, clk->name);
	switch (clk->clk_type) {
	case CLK_TYPE_CCU:
		ret = __ccu_clk_enable(clk);
		break;

	case CLK_TYPE_PERI:
		ret = __peri_clk_enable(clk);
		break;

	case CLK_TYPE_BUS:
		ret = __bus_clk_enable(clk);
		break;

	case CLK_TYPE_REF:
		ret = __ref_clk_enable(clk);
		break;

	case CLK_TYPE_PLL:
		ret = __pll_clk_enable(clk);
		break;

	case CLK_TYPE_PLL_CHNL:
		ret = __pll_chnl_clk_enable(clk);
		break;
	case CLK_TYPE_MISC:
		ret = __misc_clk_enable(clk);
		break;
	default:
		clk_dbg("%s - %s: unknown clk_type\n", __func__, clk->name);
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 1);
		else {
			clk_dbg
			    ("%s - %s: unknown clk_type & func ptr == NULL\n",
			     __func__, clk->name);
		}
		break;
	}
	return ret;
}

int clk_enable(struct clk *clk)
{
	int ret;
	unsigned long flags;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;
	 clk_dbg("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	ret = __clk_enable(clk);
	clk_unlock(clk, &flags);

	return ret;
}

EXPORT_SYMBOL(clk_enable);

static int __ccu_clk_disable(struct clk *clk)
{
	int ret = 0;
	int inx;
	struct ccu_clk *ccu_clk;
	unsigned long flgs;

	clk_dbg("%s ccu name:%s\n", __func__, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	ccu_clk = to_ccu_clk(clk);

	if (clk_use_cnt(clk) && clk_use_cnt_decr_pre(clk) == 0) {
		if (clk->ops && clk->ops->enable) {
/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
			ret = clk->ops->enable(clk, 0);
#endif
		}
	}
	/*disable PI */
#ifdef CONFIG_KONA_PI_MGR
	if (ccu_clk->pi_id != -1) {
		struct pi *pi = pi_mgr_get(ccu_clk->pi_id);

		BUG_ON(!pi);
		pi_enable(pi, 0);
	}
#endif
	/*Make sure that all dependent & src clks are disabled*/
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s, Disabling dependant clock %s\n", __func__,
				clk->dep_clks[inx]->name);
		dep_clk_lock(ccu_clk, clk->dep_clks[inx], &flgs);
		__clk_disable(clk->dep_clks[inx]);
		dep_clk_unlock(ccu_clk, clk->dep_clks[inx], &flgs);

	}

	return ret;
}

static int __peri_clk_disable(struct clk *clk)
{
	struct peri_clk *peri_clk;
	int inx;
	struct clk *src_clk;
	int ret = 0;
	unsigned long flgs;

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);

	peri_clk = to_peri_clk(clk);
	clk_dbg("%s: clock name: %s\n", __func__, clk->name);

	BUG_ON(!peri_clk->ccu_clk);

	/*decrment usage count... return if already disabled or usage count is non-zero */
	if (clk_use_cnt(clk) && clk_use_cnt_decr_pre(clk) == 0) {
		CCU_ACCESS_EN(peri_clk->ccu_clk, 1);

/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 0);
#endif
		if (clk->flags & ENABLE_HVT)
			peri_clk_set_voltage_lvl(peri_clk, VLT_NORMAL);

		/*update DFS request */

#ifdef CONFIG_KONA_PI_MGR
		if (peri_clk->clk_dfs) {
			clk_dfs_request_update(clk, CLK_STATE_CHANGE, 0);
		}
#endif

		CCU_ACCESS_EN(peri_clk->ccu_clk, 0);
	}

	/*Make sure that all dependent & src clks are disabled */
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s, Disabling dependant clock %s\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(peri_clk->ccu_clk, clk->dep_clks[inx], &flgs);
		__clk_disable(clk->dep_clks[inx]);
		dep_clk_unlock(peri_clk->ccu_clk, clk->dep_clks[inx], &flgs);

	}

	if (PERI_SRC_CLK_VALID(peri_clk)) {
		src_clk = GET_PERI_SRC_CLK(peri_clk);
		BUG_ON(!src_clk);
		clk_dbg("%s, disabling source clock\n", __func__);
		dep_clk_lock(peri_clk->ccu_clk, src_clk, &flgs);
		__clk_disable(src_clk);
		dep_clk_unlock(peri_clk->ccu_clk, src_clk, &flgs);
	}

	if (!(clk->flags & DONOT_NOTIFY_STATUS_TO_CCU)
	    && !(clk->flags & AUTO_GATE)) {
		clk_dbg("%s: peri clock %s decrementing CCU count\n", __func__,
			clk->name);
		__ccu_clk_disable(&peri_clk->ccu_clk->clk);
	}
	return ret;
}

static int __bus_clk_disable(struct clk *clk)
{
	struct bus_clk *bus_clk;
	int ret = 0;
	int inx;
	unsigned long flgs;

	BUG_ON(clk->clk_type != CLK_TYPE_BUS);

	clk_dbg("%s : %s use cnt= %d\n", __func__, clk->name, clk->use_cnt);

	bus_clk = to_bus_clk(clk);
	BUG_ON(bus_clk->ccu_clk == NULL);

	/*decrment usage count... return if already disabled or usage count is non-zero */
	if (clk_use_cnt(clk) && clk_use_cnt_decr_pre(clk) == 0) {
		CCU_ACCESS_EN(bus_clk->ccu_clk, 1);
/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 0);
#endif

		/*update DFS request */
#ifdef CONFIG_KONA_PI_MGR
		if (bus_clk->clk_dfs) {
			clk_dfs_request_update(clk, CLK_STATE_CHANGE, 0);
		}
#endif

		CCU_ACCESS_EN(bus_clk->ccu_clk, 0);
	}

	/*Disable dependent & src clks */
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s, Disabling dependant clock %s\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(bus_clk->ccu_clk, clk->dep_clks[inx], &flgs);
		__clk_disable(clk->dep_clks[inx]);
		dep_clk_unlock(bus_clk->ccu_clk, clk->dep_clks[inx], &flgs);
	}

	if (bus_clk->src_clk) {
		clk_dbg("%s src clock %s disable\n", __func__,
			bus_clk->src_clk->name);
		dep_clk_lock(bus_clk->ccu_clk, bus_clk->src_clk, &flgs);
		__clk_disable(bus_clk->src_clk);
		dep_clk_unlock(bus_clk->ccu_clk, bus_clk->src_clk, &flgs);
	}

	if (!(clk->flags & AUTO_GATE) && (clk->flags & NOTIFY_STATUS_TO_CCU)) {
		clk_dbg("%s: bus clock %s decrementing CCU count\n", __func__,
			clk->name);
		__ccu_clk_disable(&bus_clk->ccu_clk->clk);
	}

	return ret;
}

static int __ref_clk_disable(struct clk *clk)
{
	int ret = 0;
	struct ref_clk *ref_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);

	if (clk_use_cnt(clk) && clk_use_cnt_decr_pre(clk) == 0) {
		CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 0);
#endif

		CCU_ACCESS_EN(ref_clk->ccu_clk, 0);
	}
	return ret;
}

static int __pll_clk_disable(struct clk *clk)
{
	int ret = 0;
	struct pll_clk *pll_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	if (clk_use_cnt(clk) && clk_use_cnt_decr_pre(clk) == 0) {
		CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 0);
#endif

		CCU_ACCESS_EN(pll_clk->ccu_clk, 0);
	}
	return ret;
}

static int __pll_chnl_clk_disable(struct clk *clk)
{
	int ret = 0;
	struct pll_chnl_clk *pll_chnl_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);

	if (clk_use_cnt(clk) && clk_use_cnt_decr_pre(clk) == 0) {
		CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);
/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 0);
#endif

		CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);
	}
	__pll_clk_disable(&pll_chnl_clk->pll_clk->clk);
	return ret;
}

static int __misc_clk_disable(struct clk *clk)
{
	int inx, ret = 0;
	unsigned long flgs;
	if (clk_use_cnt(clk) && clk_use_cnt_decr_pre(clk) == 0) {
		/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 0);
#endif
	}
	/*Make sure that all dependent clks are disabled*/
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx];
		inx++) {
		clk_dbg("%s, Disabling dependant clock %s\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(get_ccu_clk(clk), clk->dep_clks[inx], &flgs);
		__clk_disable(clk->dep_clks[inx]);
		dep_clk_unlock(get_ccu_clk(clk), clk->dep_clks[inx], &flgs);
	}

	return ret;
}

void __clk_disable(struct clk *clk)
{
	int ret = 0;
	clk_dbg("%s - %s\n", __func__, clk->name);
	/**Return if the clk is already in disabled state*/
	if (clk_use_cnt(clk) == 0)
		return;

	switch (clk->clk_type) {
	case CLK_TYPE_CCU:
		ret = __ccu_clk_disable(clk);
		break;

	case CLK_TYPE_PERI:
		ret = __peri_clk_disable(clk);
		break;

	case CLK_TYPE_BUS:
		ret = __bus_clk_disable(clk);
		break;

	case CLK_TYPE_REF:
		ret = __ref_clk_disable(clk);
		break;

	case CLK_TYPE_PLL:
		ret = __pll_clk_disable(clk);
		break;

	case CLK_TYPE_PLL_CHNL:
		ret = __pll_chnl_clk_disable(clk);
		break;
	case CLK_TYPE_MISC:
		ret = __misc_clk_disable(clk);
		break;

	default:
		clk_dbg("%s - %s: unknown clk_type\n", __func__, clk->name);
/*Debug interface to avoid clk disable*/
#ifndef CONFIG_KONA_PM_NO_CLK_DISABLE
		if (clk->ops && clk->ops->enable)
			ret = clk->ops->enable(clk, 0);
		else {
			clk_dbg
			    ("%s - %s: unknown clk_type & func ptr == NULL\n",
			     __func__, clk->name);
		}
#endif /*CONFIG_KONA_PM_NO_CLK_DISABLE */
		break;
	}
}

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	if (IS_ERR_OR_NULL(clk))
		return;
	 clk_dbg("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	__clk_disable(clk);
	clk_unlock(clk, &flags);
}

EXPORT_SYMBOL(clk_disable);

static unsigned long __ccu_clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	struct ccu_clk *ccu_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	ccu_clk = to_ccu_clk(clk);

	CCU_ACCESS_EN(ccu_clk, 1);
	if (clk->ops && clk->ops->get_rate)
		rate = clk->ops->get_rate(clk);

	CCU_ACCESS_EN(ccu_clk, 0);

	return rate;
}

static unsigned long __peri_clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	struct peri_clk *peri_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);
	peri_clk = to_peri_clk(clk);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->get_rate)
		rate = clk->ops->get_rate(clk);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);

	return rate;
}

static unsigned long __bus_clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	struct bus_clk *bus_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_BUS);
	bus_clk = to_bus_clk(clk);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->get_rate)
		rate = clk->ops->get_rate(clk);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 0);

	return rate;
}

static unsigned long __ref_clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	struct ref_clk *ref_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->get_rate)
		rate = clk->ops->get_rate(clk);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 0);

	return rate;
}

static unsigned long __pll_clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	struct pll_clk *pll_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->get_rate)
		rate = clk->ops->get_rate(clk);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return rate;
}

static unsigned long __pll_chnl_clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	struct pll_chnl_clk *pll_chnl_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->get_rate)
		rate = clk->ops->get_rate(clk);

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);

	return rate;
}

static unsigned long __clk_get_rate(struct clk *clk)
{
	unsigned long rate = 0;
	clk_dbg("%s - %s\n", __func__, clk->name);
	switch (clk->clk_type) {
	case CLK_TYPE_CCU:
		rate = __ccu_clk_get_rate(clk);
		break;

	case CLK_TYPE_PERI:
		rate = __peri_clk_get_rate(clk);
		break;

	case CLK_TYPE_BUS:
		rate = __bus_clk_get_rate(clk);
		break;

	case CLK_TYPE_REF:
		rate = __ref_clk_get_rate(clk);
		break;

	case CLK_TYPE_PLL:
		rate = __pll_clk_get_rate(clk);
		break;

	case CLK_TYPE_PLL_CHNL:
		rate = __pll_chnl_clk_get_rate(clk);
		break;

	default:
		clk_dbg("%s - %s: unknown clk_type\n", __func__, clk->name);
		if (clk->ops && clk->ops->get_rate)
			rate = clk->ops->get_rate(clk);
		else {
			clk_dbg
			    ("%s - %s: unknown clk_type & func ptr == NULL\n",
			     __func__, clk->name);
		}
		break;
	}

	return rate;
}

unsigned long clk_get_rate(struct clk *clk)
{
	unsigned long flags, rate;

	if (IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->get_rate)
		return -EINVAL;
	clk_dbg("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	rate = __clk_get_rate(clk);
	clk_unlock(clk, &flags);
	return rate;
}

EXPORT_SYMBOL(clk_get_rate);

static long __ccu_clk_round_rate(struct clk *clk, unsigned long rate)
{
	long actual = 0;
	struct ccu_clk *ccu_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	ccu_clk = to_ccu_clk(clk);

	CCU_ACCESS_EN(ccu_clk, 1);
	if (clk->ops && clk->ops->round_rate)
		actual = clk->ops->round_rate(clk, rate);

	CCU_ACCESS_EN(ccu_clk, 0);

	return actual;
}

static long __peri_clk_round_rate(struct clk *clk, unsigned long rate)
{
	long actual = 0;
	struct peri_clk *peri_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);
	peri_clk = to_peri_clk(clk);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->round_rate)
		actual = clk->ops->round_rate(clk, rate);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);

	return actual;
}

static long __bus_clk_round_rate(struct clk *clk, unsigned long rate)
{
	long actual = 0;
	struct bus_clk *bus_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_BUS);
	bus_clk = to_bus_clk(clk);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->round_rate)
		actual = clk->ops->round_rate(clk, rate);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 0);

	return actual;
}

static unsigned long __ref_clk_round_rate(struct clk *clk, unsigned long rate)
{
	long actual = 0;
	struct ref_clk *ref_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->round_rate)
		actual = clk->ops->round_rate(clk, rate);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 0);

	return actual;
}

static unsigned long __pll_clk_round_rate(struct clk *clk, unsigned long rate)
{
	long actual = 0;
	struct pll_clk *pll_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->round_rate)
		actual = clk->ops->round_rate(clk, rate);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return actual;
}

static unsigned long __pll_chnl_clk_round_rate(struct clk *clk,
					       unsigned long rate)
{
	long actual = 0;
	struct pll_chnl_clk *pll_chnl_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->round_rate)
		actual = clk->ops->round_rate(clk, rate);

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);

	return actual;
}

static long __clk_round_rate(struct clk *clk, unsigned long rate)
{
	long actual = 0;
	clk_dbg("%s - %s\n", __func__, clk->name);
	switch (clk->clk_type) {
	case CLK_TYPE_CCU:
		actual = __ccu_clk_round_rate(clk, rate);
		break;

	case CLK_TYPE_PERI:
		actual = __peri_clk_round_rate(clk, rate);
		break;

	case CLK_TYPE_BUS:
		actual = __bus_clk_round_rate(clk, rate);
		break;

	case CLK_TYPE_REF:
		actual = __ref_clk_round_rate(clk, rate);
		break;

	case CLK_TYPE_PLL:
		actual = __pll_clk_round_rate(clk, rate);
		break;

	case CLK_TYPE_PLL_CHNL:
		actual = __pll_chnl_clk_round_rate(clk, rate);
		break;

	default:
		clk_dbg("%s - %s: unknown clk_type\n", __func__, clk->name);
		if (clk->ops && clk->ops->round_rate)
			actual = clk->ops->round_rate(clk, rate);
		else {
			clk_dbg
			    ("%s - %s: unknown clk_type & func ptr == NULL\n",
			     __func__, clk->name);
		}
		break;
	}

	return actual;
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags, actual;

	if (IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->round_rate)
		return -EINVAL;
	 clk_dbg("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	actual = __clk_round_rate(clk, rate);
	clk_unlock(clk, &flags);

	return actual;
}

EXPORT_SYMBOL(clk_round_rate);

static long __ccu_clk_set_rate(struct clk *clk, unsigned long rate)
{
	long ret = 0;
	struct ccu_clk *ccu_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	ccu_clk = to_ccu_clk(clk);

	CCU_ACCESS_EN(ccu_clk, 1);
	if (clk->ops && clk->ops->set_rate)
		ret = clk->ops->set_rate(clk, rate);

	CCU_ACCESS_EN(ccu_clk, 0);

	return ret;
}

static long __peri_clk_set_rate(struct clk *clk, unsigned long rate)
{
	long ret = 0;
	struct peri_clk *peri_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);
	peri_clk = to_peri_clk(clk);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);

	if (clk->ops && clk->ops->set_rate)
		ret = clk->ops->set_rate(clk, rate);

#ifdef CONFIG_KONA_PI_MGR
	if (peri_clk->clk_dfs) {
		clk_dfs_request_update(clk, CLK_RATE_CHANGE,
				       __clk_get_rate(clk));
	}
#endif

	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);

	return ret;
}

static long __bus_clk_set_rate(struct clk *clk, unsigned long rate)
{
	long ret = 0;
	struct bus_clk *bus_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_BUS);
	bus_clk = to_bus_clk(clk);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->set_rate)
		ret = clk->ops->set_rate(clk, rate);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 0);

	return ret;
}

static unsigned long __ref_clk_set_rate(struct clk *clk, unsigned long rate)
{
	long ret = 0;
	struct ref_clk *ref_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->set_rate)
		ret = clk->ops->set_rate(clk, rate);

	CCU_ACCESS_EN(ref_clk->ccu_clk, 0);

	return ret;
}

static unsigned long __pll_clk_set_rate(struct clk *clk, unsigned long rate)
{
	long ret = 0;
	struct pll_clk *pll_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->set_rate)
		ret = clk->ops->set_rate(clk, rate);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return ret;
}

static unsigned long __pll_chnl_clk_set_rate(struct clk *clk,
					     unsigned long rate)
{
	long ret = 0;
	struct pll_chnl_clk *pll_chnl_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);
	if (clk->ops && clk->ops->set_rate)
		ret = clk->ops->set_rate(clk, rate);

	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);

	return ret;
}

static long __clk_set_rate(struct clk *clk, unsigned long rate)
{
	long ret = 0;
	clk_dbg("%s - %s\n", __func__, clk->name);
	switch (clk->clk_type) {
	case CLK_TYPE_CCU:
		ret = __ccu_clk_set_rate(clk, rate);
		break;

	case CLK_TYPE_PERI:
		ret = __peri_clk_set_rate(clk, rate);
		break;

	case CLK_TYPE_BUS:
		ret = __bus_clk_set_rate(clk, rate);
		break;

	case CLK_TYPE_REF:
		ret = __ref_clk_set_rate(clk, rate);
		break;

	case CLK_TYPE_PLL:
		ret = __pll_clk_set_rate(clk, rate);
		break;

	case CLK_TYPE_PLL_CHNL:
		ret = __pll_chnl_clk_set_rate(clk, rate);
		break;

	default:
		clk_dbg("%s - %s: unknown clk_type\n", __func__, clk->name);
		if (clk->ops && clk->ops->set_rate)
			ret = clk->ops->set_rate(clk, rate);
		else {
			clk_dbg
			    ("%s - %s: unknown clk_type & func ptr == NULL\n",
			     __func__, clk->name);
		}
		break;
	}
	return ret;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	int ret;

	if (IS_ERR_OR_NULL(clk) || !clk->ops || !clk->ops->set_rate)
		return -EINVAL;
	 clk_dbg("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	ret = __clk_set_rate(clk, rate);
	clk_unlock(clk, &flags);

	return ret;
}

EXPORT_SYMBOL(clk_set_rate);

static int clk_is_enabled(struct clk *clk)
{
	return (!!clk->use_cnt);
}

#ifdef CONFIG_KONA_PI_MGR
int clk_dfs_request_update(struct clk *clk, u32 action, u32 param)
{
	struct clk_dfs *clk_dfs;
	struct pi_mgr_dfs_node *dfs_node = NULL;
	struct dfs_rate_thold *thold = NULL;

	if (clk->clk_type == CLK_TYPE_PERI) {
		struct peri_clk *peri_clk;
		peri_clk = to_peri_clk(clk);
		clk_dfs = peri_clk->clk_dfs;
		if (clk_dfs)
			dfs_node = &peri_clk->clk_dfs->dfs_node;
	} else if (clk->clk_type == CLK_TYPE_BUS) {
		struct bus_clk *bus_clk;
		bus_clk = to_bus_clk(clk);
		clk_dfs = bus_clk->clk_dfs;
		if (clk_dfs)
			dfs_node = &bus_clk->clk_dfs->dfs_node;
	} else
		BUG();

	BUG_ON(!clk_dfs || !dfs_node);

	switch (clk_dfs->dfs_policy) {
	case CLK_DFS_POLICY_STATE:
		if (action == CLK_STATE_CHANGE) {
			if (param) {	/*enable ? */

				pi_mgr_dfs_request_update_ex(dfs_node,
							     clk_dfs->
							     policy_param,
							     (clk_dfs->
							      policy_param ==
							      PI_MGR_DFS_MIN_VALUE)
							     ?
							     PI_MGR_DFS_WIEGHTAGE_NONE
							     : clk_dfs->
							     opp_weightage
							     [clk_dfs->
							      policy_param]);
			} else {
				pi_mgr_dfs_request_update_ex(dfs_node,
							     PI_MGR_DFS_MIN_VALUE,
							     PI_MGR_DFS_WIEGHTAGE_NONE);
			}
		}
		break;

	case CLK_DFS_POLICY_RATE:
		if (action == CLK_STATE_CHANGE && param == 0) {
			pi_mgr_dfs_request_update_ex(dfs_node,
						     PI_MGR_DFS_MIN_VALUE,
						     PI_MGR_DFS_WIEGHTAGE_NONE);

		} else {
			u32 rate = 0, inx;
			if (action == CLK_STATE_CHANGE)	/*enable */
				rate = __clk_get_rate(clk);
			else if (action == CLK_RATE_CHANGE) {
				if (!clk_is_enabled(clk))
					return 0;
				rate = param;
			} else
				BUG();

			thold = (struct dfs_rate_thold *)clk_dfs->policy_param;
			for (inx = 0; inx < PI_OPP_MAX; inx++) {
				if (rate <= thold[inx].rate_thold ||
				    thold[inx].rate_thold == -1) {
					break;
				}

			}
			BUG_ON(inx == PI_OPP_MAX);
			pi_mgr_dfs_request_update_ex(dfs_node, thold[inx].opp,
						     (thold[inx].opp ==
						      PI_MGR_DFS_MIN_VALUE) ?
						     PI_MGR_DFS_WIEGHTAGE_NONE :
						     clk_dfs->
						     opp_weightage[thold[inx].
								   opp]);

		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

#endif /*CONFIG_KONA_PI_MGR */

int ccu_init_state_save_buf(struct ccu_clk *ccu_clk)
{
	int ret = 0;
	int i;

	struct ccu_state_save *ccu_state_save = ccu_clk->ccu_state_save;
	if (!ccu_state_save)
		return ret;
	ccu_state_save->num_reg = 0;
	for (i = 0; i < ccu_state_save->reg_set_count; i++) {
		BUG_ON(ccu_state_save->reg_save[i].offset_end <
		       ccu_state_save->reg_save[i].offset_start);
		ccu_state_save->num_reg +=
		    (ccu_state_save->reg_save[i].offset_end -
		     ccu_state_save->reg_save[i].offset_start +
		     sizeof(u32)) / sizeof(u32);
	}
	clk_dbg("%s:num_reg = %d\n", __func__, ccu_state_save->num_reg);

	/*Set save flag to false by default */
	if (!ret)
		ccu_state_save->save_buf[ccu_state_save->num_reg] = 0;
	return ret;
}

static int clock_panic_event(struct notifier_block *this, unsigned long event,
		void *ptr)
{
	static int has_panicked;
	struct ccu_clk *ccu_clk;
	struct pi *pi;
	int i;
	if (has_panicked)
		return 0;

	pr_info("CCU panic handler\n-----------------\n");
	for (i = 0; i < num_ccu; i++) {
		ccu_clk = ccu_clk_list[i];
		if (ccu_clk_list[i]->clk.flags & CCU_ACCESS_ENABLE) {
			pi = pi_mgr_get(ccu_clk->pi_id);
			BUG_ON(!pi);
			if (!pi->usg_cnt)
				continue;
		}
		pr_info("%s information\n", ccu_clk->clk.name);
		pr_info("Policy: %d, FID: %d, VLT0-3: 0x%08x, VLT4-7: 0x%08x\n",
			ccu_policy_dbg_get_act_policy(ccu_clk),
			ccu_policy_dbg_get_act_freqid(ccu_clk),
			readl(CCU_VLT0_3_REG(ccu_clk)),
			readl(CCU_VLT4_7_REG(ccu_clk)));
	}
	has_panicked = 1;
	return 0;
}

static struct notifier_block panic_block = {
	.notifier_call	= clock_panic_event,
	.next		= NULL,
	.priority	= 200	/* priority: INT_MAX >= x >= 0 */
};


int clk_register(struct clk_lookup *clk_lkup, int num_clks)
{
	int ret = 0;
	int i;

	for (i = 0; i < num_clks; i++) {
		clkdev_add(&clk_lkup[i]);
		clk_dbg("clock registered - %s\n", clk_lkup[i].clk->name);
	}
	for (i = 0; i < num_clks; i++) {
		/*Init per-ccu spin lock */
		if (clk_lkup[i].clk->clk_type == CLK_TYPE_CCU) {
			struct ccu_clk *ccu_clk = to_ccu_clk(clk_lkup[i].clk);
			spin_lock_init(&ccu_clk->clk_lock);
			spin_lock_init(&ccu_clk->access_lock);
			if (ccu_clk->pi_id != -1) {
				ccu_clk_list[num_ccu] = ccu_clk;
				num_ccu++;
				BUG_ON(num_ccu > MAX_CCU_COUNT);
			}
		}
		ret |= clk_init(clk_lkup[i].clk);
		if (ret)
			pr_info("%s: clk %s init failed !!!\n", __func__,
				clk_lkup[i].clk->name);
	}
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);
	return ret;
}

EXPORT_SYMBOL(clk_register);

int ccu_reset_write_access_enable(struct ccu_clk *ccu_clk, int enable)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->rst_write_access)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret =  ccu_clk->ccu_ops->rst_write_access(ccu_clk, enable);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;
}

EXPORT_SYMBOL(ccu_reset_write_access_enable);

/*CCU reset access functions */
static int ccu_rst_write_access_enable(struct ccu_clk *ccu_clk, int enable)
{
	u32 reg_val = 0;

	reg_val = CLK_WR_ACCESS_PASSWORD << CLK_WR_PASSWORD_SHIFT;
	if (enable) {
		if (ccu_clk->rst_write_access_en_count++ != 0)
			return 0;
		reg_val |= CLK_WR_ACCESS_EN;
	} else if (ccu_clk->rst_write_access_en_count == 0
		   || --ccu_clk->rst_write_access_en_count != 0)
		return 0;
	writel(reg_val,
	       ccu_clk->ccu_reset_mgr_base + ccu_clk->reset_wr_access_offset);

	reg_val =
	    readl(ccu_clk->ccu_reset_mgr_base +
		  ccu_clk->reset_wr_access_offset);
	clk_dbg("rst mgr access %s: reg value: %08x\n",
		enable ? "enabled" : "disabled", reg_val);

	return 0;
}

int ccu_write_access_enable(struct ccu_clk *ccu_clk, int enable)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->write_access)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->write_access(ccu_clk, enable);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;
}
EXPORT_SYMBOL(ccu_write_access_enable);

int ccu_policy_engine_resume(struct ccu_clk *ccu_clk, int load_type)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops ||
	    !ccu_clk->ccu_ops->policy_engine_resume)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->policy_engine_resume(ccu_clk, load_type);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;
}

EXPORT_SYMBOL(ccu_policy_engine_resume);

int ccu_policy_engine_stop(struct ccu_clk *ccu_clk)
{
	unsigned long flags;
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops ||
	    !ccu_clk->ccu_ops->policy_engine_stop)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->policy_engine_stop(ccu_clk);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;

}

EXPORT_SYMBOL(ccu_policy_engine_stop);

int ccu_set_policy_ctrl(struct ccu_clk *ccu_clk, int pol_ctrl_id, int action)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops ||
	    !ccu_clk->ccu_ops->set_policy_ctrl)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->set_policy_ctrl(ccu_clk, pol_ctrl_id, action);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;

}

EXPORT_SYMBOL(ccu_set_policy_ctrl);

int ccu_int_enable(struct ccu_clk *ccu_clk, int int_type, int enable)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops ||
	    !ccu_clk->ccu_ops->int_enable)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->int_enable(ccu_clk, int_type, enable);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;
}

EXPORT_SYMBOL(ccu_int_enable);

int ccu_int_status_clear(struct ccu_clk *ccu_clk, int int_type)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops ||
	    !ccu_clk->ccu_ops->int_status_clear)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->int_status_clear(ccu_clk, int_type);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;
}

EXPORT_SYMBOL(ccu_int_status_clear);

int ccu_set_freq_policy(struct ccu_clk *ccu_clk, int policy_id,
			struct opp_info *opp_info)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->set_freq_policy)
		return -EINVAL;
	ret = ccu_clk->ccu_ops->set_freq_policy(ccu_clk,
			policy_id, opp_info);
	return ret;

}
EXPORT_SYMBOL(ccu_set_freq_policy);

int ccu_get_freq_policy(struct ccu_clk *ccu_clk, int policy_id)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->get_freq_policy)
		return -EINVAL;
	ret = ccu_clk->ccu_ops->get_freq_policy(ccu_clk, policy_id);
	return ret;
}

EXPORT_SYMBOL(ccu_get_freq_policy);

int ccu_set_peri_voltage(struct ccu_clk *ccu_clk, int peri_volt_id, u8 voltage)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->set_peri_voltage)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->set_peri_voltage(ccu_clk,
		peri_volt_id, voltage);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;

}

EXPORT_SYMBOL(ccu_set_peri_voltage);

int ccu_set_voltage(struct ccu_clk *ccu_clk, int volt_id, u8 voltage)
{
	int ret;
	unsigned long flags;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->set_voltage)
		return -EINVAL;
	ccu_access_lock(ccu_clk, &flags);
	ret = ccu_clk->ccu_ops->set_voltage(ccu_clk, volt_id, voltage);
	ccu_access_unlock(ccu_clk, &flags);
	return ret;

}

EXPORT_SYMBOL(ccu_set_voltage);

int ccu_get_voltage(struct ccu_clk *ccu_clk, int freq_id)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->get_voltage)
		return -EINVAL;
	ret = ccu_clk->ccu_ops->get_voltage(ccu_clk, freq_id);
	return ret;

}

EXPORT_SYMBOL(ccu_get_voltage);

int ccu_set_active_policy(struct ccu_clk *ccu_clk, u32 policy)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->set_active_policy)
		return -EINVAL;
	ret = ccu_clk->ccu_ops->set_active_policy(ccu_clk, policy);
	return ret;

}

EXPORT_SYMBOL(ccu_set_active_policy);

int ccu_get_active_policy(struct ccu_clk *ccu_clk)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->get_active_policy)
		return -EINVAL;
	ret = ccu_clk->ccu_ops->get_active_policy(ccu_clk);
	return ret;
}

EXPORT_SYMBOL(ccu_get_active_policy);

/*caller should make sure that PI is in enabled state */
int ccu_save_state(struct ccu_clk *ccu_clk, int save)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->save_state)
		return -EINVAL;
	ret = ccu_clk->ccu_ops->save_state(ccu_clk, save);
	return ret;
}
EXPORT_SYMBOL(ccu_save_state);

int ccu_get_dbg_bus_status(struct ccu_clk *ccu_clk)
{
	int ret;

	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->get_dbg_bus_status
		|| !CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN))
		return -EINVAL;
	ret = ccu_clk->ccu_ops->get_dbg_bus_status(ccu_clk);
	return ret;
}
EXPORT_SYMBOL(ccu_get_dbg_bus_status);

int ccu_set_dbg_bus_sel(struct ccu_clk *ccu_clk, u32 sel)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->set_dbg_bus_sel
		|| !CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN))
		return -EINVAL;
	ret = ccu_clk->ccu_ops->set_dbg_bus_sel(ccu_clk, sel);
	return ret;
}
EXPORT_SYMBOL(ccu_set_dbg_bus_sel);

int ccu_get_dbg_bus_sel(struct ccu_clk *ccu_clk)
{
	int ret;
	if (IS_ERR_OR_NULL(ccu_clk) || !ccu_clk->ccu_ops
	    || !ccu_clk->ccu_ops->get_dbg_bus_sel
		|| !CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN))
		return -EINVAL;

	ret = ccu_clk->ccu_ops->get_dbg_bus_sel(ccu_clk);
	return ret;
}
EXPORT_SYMBOL(ccu_get_dbg_bus_sel);

int ccu_clk_get_freq_id_from_opp(struct ccu_clk *ccu_clk, int opp_id)
{
	struct pi *pi;
	struct pi_opp *pi_opp;
	struct opp_info *opp_info;
	int i, ccu_number = 0;

	/* There are no pi_opp structures for Modem, DSP and root CCU */
	if (ccu_clk->pi_id == -1) {
		pr_err("Freq list unavailable for %s\n", ccu_clk->clk.name);
		return -1;
	}

	pi = pi_mgr_get(ccu_clk->pi_id);
	BUG_ON(pi == NULL);
	pi_opp = pi->pi_opp;
	if (opp_id > pi_opp->num_opp)
		return -1;

	for (i = 0; i < pi->num_ccu_id; i++)
		if (strcmp(pi->ccu_id[i], ccu_clk->clk.name) == 0) {
			ccu_number = i;
			break;
		}
	opp_info = pi_opp->opp_info[ccu_number];
	BUG_ON(opp_info == NULL);
	return opp_info[opp_id].freq_id;
}

u32 ccu_clk_get_rate(struct clk *clk, int opp_id)
{
	struct ccu_clk *ccu_clk;
	int freq_id;
	BUG_ON(clk == NULL);
	ccu_clk = to_ccu_clk(clk);
	freq_id = ccu_clk_get_freq_id_from_opp(ccu_clk, opp_id);
	if (freq_id <= 0)
		return 0;

	if (ccu_clk->freq_tbl_size)
		return ccu_clk->freq_tbl[freq_id][0];
	else
		return 0;
}

int ccu_print_sleep_prevent_clks(struct clk *clk)
{
	struct ccu_clk *ccu_clk;
	struct clk *clk_iter;
	int use_cnt;
	int sleep_prevent = 0;
	int num_active = 0;
	unsigned long flags;

	if (!clk || (clk->clk_type != CLK_TYPE_CCU))
		return -EINVAL;
	ccu_clk = to_ccu_clk(clk);
	list_for_each_entry(clk_iter, &ccu_clk->clk_list, list) {
		clk_lock(clk_iter, &flags);
		use_cnt = clk_iter->use_cnt;
		clk_unlock(clk_iter, &flags);
		switch (clk_iter->clk_type) {
		case CLK_TYPE_REF:
			/**
			 * ignore reference clks as they dont prevent
			 * retention
			 */
			sleep_prevent = 0;
			break;
		case CLK_TYPE_PLL:
		case CLK_TYPE_PLL_CHNL:
		case CLK_TYPE_CORE:
			if (use_cnt && !CLK_FLG_ENABLED(clk_iter, AUTO_GATE))
				sleep_prevent = 1;
			break;
		case CLK_TYPE_BUS:
			sleep_prevent = (CLK_FLG_ENABLED(clk_iter,
					NOTIFY_STATUS_TO_CCU) &&
				!CLK_FLG_ENABLED(clk_iter, AUTO_GATE));
			break;
		case CLK_TYPE_PERI:
			sleep_prevent = !CLK_FLG_ENABLED(clk_iter,
					DONOT_NOTIFY_STATUS_TO_CCU);
			break;
		default:
			break;
		}
		if (use_cnt && sleep_prevent) {
			pr_info("%20s %10d", clk_iter->name, clk_iter->use_cnt);
			num_active++;
		}
	}
	return num_active;
}
EXPORT_SYMBOL(ccu_print_sleep_prevent_clks);

/*CCU access functions */
static int ccu_clk_write_access_enable(struct ccu_clk *ccu_clk, int enable)
{
	u32 reg_val = 0;

	reg_val = CLK_WR_ACCESS_PASSWORD << CLK_WR_PASSWORD_SHIFT;

	if (enable) {
		if (ccu_clk->write_access_en_count++ != 0)
			return 0;
		reg_val |= CLK_WR_ACCESS_EN;
	} else if (ccu_clk->write_access_en_count == 0
		   || --ccu_clk->write_access_en_count != 0)
		return 0;
	writel_relaxed(reg_val, CCU_WR_ACCESS_REG(ccu_clk));

	return 0;
}

static int ccu_clk_policy_engine_resume(struct ccu_clk *ccu_clk, int load_type)
{
	u32 reg_val = 0;
	u32 insurance = POLICY_RESUME_INS_COUNT;

	if (ccu_clk->pol_engine_dis_cnt == 0)
		return 0;	/*Already in running state ?? */

	else if (--ccu_clk->pol_engine_dis_cnt != 0)
		return 0;	/*disable count is non-zero */

	/*Set trigger */
	reg_val = readl(CCU_POLICY_CTRL_REG(ccu_clk));
	if (load_type == CCU_LOAD_ACTIVE)
		reg_val |= CCU_POLICY_CTL_ATL << CCU_POLICY_CTL_GO_ATL_SHIFT;
	else
		reg_val &= ~(CCU_POLICY_CTL_ATL << CCU_POLICY_CTL_GO_ATL_SHIFT);
	reg_val |= CCU_POLICY_CTL_GO_TRIG << CCU_POLICY_CTL_GO_SHIFT;

	writel(reg_val, CCU_POLICY_CTRL_REG(ccu_clk));

	while ((readl(CCU_POLICY_CTRL_REG(ccu_clk)) & CCU_POLICY_CTL_GO_MASK)
	       && insurance) {
		udelay(1);
		insurance--;
	}
	BUG_ON(insurance == 0);

	reg_val = readl(CCU_POLICY_CTRL_REG(ccu_clk));
	if ((load_type == CCU_LOAD_TARGET)
	    && (reg_val & CCU_POLICY_CTL_TGT_VLD_MASK)) {
		reg_val |=
		    CCU_POLICY_CTL_TGT_VLD << CCU_POLICY_CTL_TGT_VLD_SHIFT;
		writel(reg_val, CCU_POLICY_CTRL_REG(ccu_clk));
	}

	return 0;
}

static int ccu_clk_policy_engine_stop(struct ccu_clk *ccu_clk)
{
	u32 reg_val = 0;
	u32 insurance = POLICY_RESUME_INS_COUNT;

	if (ccu_clk->pol_engine_dis_cnt++ != 0)
		return 0;	/*Already in disabled state */

	reg_val = (CCU_POLICY_OP_EN << CCU_POLICY_CONFIG_EN_SHIFT);
	writel(reg_val, CCU_LVM_EN_REG(ccu_clk));

	while ((readl(CCU_LVM_EN_REG(ccu_clk)) & CCU_POLICY_CONFIG_EN_MASK) &&
	       insurance) {
		udelay(1);
		insurance--;
	}
	BUG_ON(insurance == 0);

	return 0;
}

static int ccu_clk_set_policy_ctrl(struct ccu_clk *ccu_clk, int pol_ctrl_id,
				   int action)
{
	u32 reg_val = 0;
	u32 shift;

	switch (pol_ctrl_id) {
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

	writel(reg_val, CCU_POLICY_CTRL_REG(ccu_clk));
	return 0;
}

static int ccu_clk_int_enable(struct ccu_clk *ccu_clk, int int_type, int enable)
{
	u32 reg_val = 0;
	u32 shift;

	if (int_type == ACT_INT)
		shift = CCU_ACT_INT_SHIFT;
	else if (int_type == TGT_INT)
		shift = CCU_TGT_INT_SHIFT;
	else
		return -EINVAL;

	reg_val = readl(CCU_INT_EN_REG(ccu_clk));

	if (enable)
		reg_val |= (CCU_INT_EN << shift);
	else
		reg_val &= ~(CCU_INT_EN << shift);

	writel(reg_val, CCU_INT_EN_REG(ccu_clk));
	return 0;
}

static int ccu_clk_int_status_clear(struct ccu_clk *ccu_clk, int int_type)
{
	u32 reg_val = 0;
	u32 shift;

	if (int_type == ACT_INT)
		shift = CCU_ACT_INT_SHIFT;
	else if (int_type == TGT_INT)
		shift = CCU_TGT_INT_SHIFT;
	else
		return -EINVAL;

	reg_val = readl(CCU_INT_STATUS_REG(ccu_clk));
	reg_val |= (CCU_INT_STATUS_CLR << shift);
	writel(reg_val, CCU_INT_STATUS_REG(ccu_clk));

	return 0;
}

static int ccu_clk_set_freq_policy(struct ccu_clk *ccu_clk, int policy_id,
				   struct opp_info *opp_info)
{
	u32 reg_val = 0;
	u32 shift;
	unsigned long flags;
	clk_dbg("%s:%s ccu , freq_id = %d policy_id = %d\n", __func__,
		ccu_clk->clk.name, opp_info->freq_id, policy_id);

	if (opp_info->freq_id >= ccu_clk->freq_count)
		return -EINVAL;

	switch (policy_id) {
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
	clk_dbg("%s: reg_val:%08x shift:%d\n", __func__, reg_val, shift);
	reg_val &= ~(CCU_FREQ_POLICY_MASK << shift);

	reg_val |= opp_info->freq_id << shift;

	ccu_write_access_enable(ccu_clk, true);
	ccu_policy_engine_stop(ccu_clk);
	ccu_access_lock(ccu_clk, &flags);
	writel(reg_val, CCU_POLICY_FREQ_REG(ccu_clk));
	ccu_access_unlock(ccu_clk, &flags);
	ccu_policy_engine_resume(ccu_clk,
				 ccu_clk->clk.
				 flags & CCU_TARGET_LOAD ? CCU_LOAD_TARGET :
				 CCU_LOAD_ACTIVE);
	ccu_write_access_enable(ccu_clk, false);
	clk_dbg("%s:%s ccu OK\n", __func__, ccu_clk->clk.name);
	return 0;
}

static int ccu_clk_get_freq_policy(struct ccu_clk *ccu_clk, int policy_id)
{
	u32 shift, reg_val;

	switch (policy_id) {
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
	clk_dbg("%s: reg_val:%08x shift:%d\n", __func__, reg_val, shift);

	return ((reg_val >> shift) & CCU_FREQ_POLICY_MASK);
}

static int ccu_clk_set_peri_voltage(struct ccu_clk *ccu_clk, int peri_volt_id,
				    u8 voltage)
{

	u32 shift, reg_val;

	if (peri_volt_id == VLT_NORMAL) {
		shift = CCU_PERI_VLT_NORM_SHIFT;
		ccu_clk->volt_peri[0] = voltage & CCU_PERI_VLT_MASK;
	} else if (peri_volt_id == VLT_HIGH) {
		shift = CCU_PERI_VLT_HIGH_SHIFT;
		ccu_clk->volt_peri[1] = voltage & CCU_PERI_VLT_MASK;
	} else
		return -EINVAL;

	reg_val = readl(CCU_VLT_PERI_REG(ccu_clk));
	reg_val = (reg_val & ~(CCU_PERI_VLT_MASK << shift)) |
	    ((voltage & CCU_PERI_VLT_MASK) << shift);

	writel(reg_val, CCU_VLT_PERI_REG(ccu_clk));

	return 0;
}

static int ccu_clk_set_voltage(struct ccu_clk *ccu_clk, int volt_id, u8 voltage)
{
	u32 shift, reg_val;
	void __iomem *reg_addr;

	if (volt_id >= ccu_clk->freq_count)
		return -EINVAL;

	ccu_clk->freq_volt[volt_id] = voltage & CCU_VLT_MASK;
	switch (volt_id) {
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
	reg_val = readl(reg_addr);
	reg_val = (reg_val & ~(CCU_VLT_MASK << shift)) |
	    ((voltage & CCU_VLT_MASK) << shift);

	writel(reg_val, reg_addr);

	return 0;
}

static int ccu_clk_get_voltage(struct ccu_clk *ccu_clk, int freq_id)
{
	u32 shift, reg_val;
	 void __iomem *reg_addr;
	int volt_id;

	/*Ideally we should compare against ccu_clk->freq_count,
	   but anyways allowing read for all 8 freq Ids. */
	if (freq_id >= 8)
		return -EINVAL;

	switch (freq_id) {
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
	reg_val = readl(reg_addr);
	volt_id = (reg_val & (CCU_VLT_MASK << shift)) >> shift;

	return volt_id;
}

int ccu_policy_dbg_get_act_freqid(struct ccu_clk *ccu_clk)
{
	u32 reg_val;

	reg_val = readl(ccu_clk->ccu_clk_mgr_base + ccu_clk->policy_dbg_offset);
	reg_val =
	    (reg_val >> ccu_clk->
	     policy_dbg_act_freq_shift) & CCU_POLICY_DBG_FREQ_MASK;

	return (int)reg_val;
}

int ccu_policy_dbg_get_act_policy(struct ccu_clk *ccu_clk)
{
	u32 reg_val;

	reg_val = readl(ccu_clk->ccu_clk_mgr_base + ccu_clk->policy_dbg_offset);
	reg_val =
	    (reg_val >> ccu_clk->
	     policy_dbg_act_policy_shift) & CCU_POLICY_DBG_POLICY_MASK;

	return (int)reg_val;
}

static int ccu_clk_set_active_policy(struct ccu_clk *ccu_clk, u32 policy)
{
	ccu_clk->active_policy = policy;
	return 0;
}

static int ccu_clk_get_active_policy(struct ccu_clk *ccu_clk)
{
#ifdef CONFIG_DEBUG_FS
	return ccu_policy_dbg_get_act_policy(ccu_clk);
#else
	return ccu_clk->active_policy;
#endif

}

/*Default function to save/restore CCU state
Caller should make sure that PI is in enabled state */
static int ccu_clk_save_state(struct ccu_clk *ccu_clk, int save)
{
	int ret = 0;
	int i, j;
	struct reg_save *reg_save;
	u32 buf_inx = 0;
	u32 reg_val;
	struct clk *clk = &ccu_clk->clk;
	struct ccu_state_save *ccu_state_save = ccu_clk->ccu_state_save;

	clk_dbg("%s: CCU: %s save = %d\n", __func__, clk->name, save);
	BUG_ON(!ccu_state_save);
	reg_save = ccu_state_save->reg_save;

	if (save) {
		for (i = 0; i < ccu_state_save->reg_set_count; i++, reg_save++) {

			for (j = reg_save->offset_start;
			     j <= reg_save->offset_end; j += 4) {
				reg_val = readl(CCU_REG_ADDR(ccu_clk, j));
				clk_dbg("%s:save - off = %x,val = %x\n",
					__func__, j, reg_val);
				ccu_state_save->save_buf[buf_inx++] = reg_val;
			}
		}
		BUG_ON(buf_inx != ccu_state_save->num_reg);
		/*Set save_buf[num_reg] to 1 to indicate that buf entries are valid */
		ccu_state_save->save_buf[buf_inx] = 1;
	} else {		/*Restore */

		/*Error if the contxt buffer is not having valid data */
		BUG_ON(ccu_state_save->save_buf[ccu_state_save->num_reg] == 0);

		/* enable write access */
		ccu_write_access_enable(ccu_clk, true);
		/*stop policy engine */
		ccu_policy_engine_stop(ccu_clk);

		/*Re-init CCU */
		if (clk->ops && clk->ops->init)
			ret = clk->ops->init(clk);

		for (i = 0; i < ccu_state_save->reg_set_count; i++, reg_save++) {
			for (j = reg_save->offset_start;
			     j <= reg_save->offset_end; j += 4) {
				reg_val = ccu_state_save->save_buf[buf_inx++];
				clk_dbg("%s:restore - off = %x,val = %x\n",
					__func__, j, reg_val);
				writel(reg_val, CCU_REG_ADDR(ccu_clk, j));
				clk_dbg("%s:restore - off = %x,nweval = %x\n",
					__func__, j,
					readl(CCU_REG_ADDR(ccu_clk, j)));
			}
		}
		BUG_ON(buf_inx != ccu_state_save->num_reg);
		/*Set save_buf[num_reg] to 0 to indicate that buf entries are restored */
		ccu_state_save->save_buf[buf_inx] = 0;
		/*Resume polic engine */
		ccu_policy_engine_resume(ccu_clk,
					 ccu_clk->clk.
					 flags & CCU_TARGET_LOAD ?
					 CCU_LOAD_TARGET : CCU_LOAD_ACTIVE);
		/* disable write access */
		ccu_write_access_enable(ccu_clk, false);
	}
	clk_dbg("%s: done\n", __func__);
	return ret;
}


static int ccu_clk_get_dbg_bus_status(struct ccu_clk *ccu_clk)
{
	u32 reg;
	BUG_ON(!ccu_clk ||
			!CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN));
	reg = readl(CCU_DBG_BUS_REG(ccu_clk));
	return (reg & CCU_DBG_BUS_STATUS_MASK) >>
				CCU_DBG_BUS_STATUS_SHIFT;
}
static int ccu_clk_set_dbg_bus_sel(struct ccu_clk *ccu_clk, u32 sel)
{
	u32 reg;
	BUG_ON(!ccu_clk ||
			!CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN));
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_DBG_BUS_REG(ccu_clk));
	reg &= ~CCU_DBG_BUS_SEL_MASK;
	reg |= (sel << CCU_DBG_BUS_SEL_SHIFT) &
				CCU_DBG_BUS_SEL_MASK;
	writel(reg, CCU_DBG_BUS_REG(ccu_clk));
	ccu_write_access_enable(ccu_clk, false);
	return 0;
}

static int ccu_clk_get_dbg_bus_sel(struct ccu_clk *ccu_clk)
{
	u32 reg;
	BUG_ON(!ccu_clk ||
			!CLK_FLG_ENABLED(&ccu_clk->clk, CCU_DBG_BUS_EN));
	reg = readl(CCU_DBG_BUS_REG(ccu_clk));
	reg &= CCU_DBG_BUS_SEL_MASK;
	return (int)((reg & CCU_DBG_BUS_SEL_MASK) >>
					CCU_DBG_BUS_SEL_SHIFT);
}

static int __ccu_volt_id_update_for_freqid(struct clk *clk, u8 freq_id,
					   u8 volt_id)
{
	struct ccu_clk *ccu_clk;
	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	ccu_clk = to_ccu_clk(clk);

	if (freq_id > 8 || volt_id > 0xF)
		return -EINVAL;
	if (freq_id > (ccu_clk->freq_count - 1))
		pr_info("invalid freq_id for this CCU\n");

	/* enable write access */
	ccu_write_access_enable(ccu_clk, true);
	/*stop policy engine */
	ccu_policy_engine_stop(ccu_clk);
	ccu_set_voltage(ccu_clk, freq_id, volt_id);

	/*Set ATL & AC */
	if (clk->flags & CCU_TARGET_LOAD) {
		if (clk->flags & CCU_TARGET_AC)
			ccu_set_policy_ctrl(ccu_clk, POLICY_CTRL_GO_AC,
					    CCU_AUTOCOPY_ON);
		ccu_policy_engine_resume(ccu_clk, CCU_LOAD_TARGET);
	} else
		ccu_policy_engine_resume(ccu_clk, CCU_LOAD_ACTIVE);

	/* disable write access */
	ccu_write_access_enable(ccu_clk, false);

	return 0;
}


int ccu_volt_id_update_for_freqid(struct clk *clk, u8 freq_id, u8 volt_id)
{
	int ret = 0;
	unsigned long flags;
	struct ccu_clk *ccu_clk;

	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;
	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	ccu_clk = to_ccu_clk(clk);

	pr_info("%s - %s\n", __func__, clk->name);
	clk_lock(clk, &flags);
	CCU_ACCESS_EN(ccu_clk, 1);
	ret = __ccu_volt_id_update_for_freqid(clk, freq_id, volt_id);
	CCU_ACCESS_EN(ccu_clk, 0);
	clk_unlock(clk, &flags);

	return ret;
}

int ccu_volt_tbl_display(struct clk *clk, u8 *volt_tbl)
{
	int ret = 0;
	int freq_id;
	unsigned long flags;
	struct ccu_clk *ccu_clk;

	if (volt_tbl == NULL)
		return -EINVAL;
	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;
	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	 clk_dbg("%s - %s\n", __func__, clk->name);
	ccu_clk = to_ccu_clk(clk);
	clk_lock(clk, &flags);
	CCU_ACCESS_EN(ccu_clk, 1);

	if (!ccu_clk->freq_count)
		return -EINVAL;
	for (freq_id = 0; freq_id < ccu_clk->freq_count; freq_id++) {
		/*if (freq_id > (ccu_clk->freq_count - 1))
		   pr_info("invalid freq_id for this CCU\n"); */
		volt_tbl[freq_id] = ccu_get_voltage(ccu_clk, freq_id);
	}

	CCU_ACCESS_EN(ccu_clk, 0);
	clk_unlock(clk, &flags);

	return ret;
}

struct ccu_clk_ops gen_ccu_ops = {
	.write_access = ccu_clk_write_access_enable,
	.rst_write_access = ccu_rst_write_access_enable,
	.policy_engine_resume = ccu_clk_policy_engine_resume,
	.policy_engine_stop = ccu_clk_policy_engine_stop,
	.set_policy_ctrl = ccu_clk_set_policy_ctrl,
	.int_enable = ccu_clk_int_enable,
	.int_status_clear = ccu_clk_int_status_clear,
	.set_freq_policy = ccu_clk_set_freq_policy,
	.get_freq_policy = ccu_clk_get_freq_policy,
	.set_peri_voltage = ccu_clk_set_peri_voltage,
	.set_voltage = ccu_clk_set_voltage,
	.get_voltage = ccu_clk_get_voltage,
	.set_active_policy = ccu_clk_set_active_policy,
	.get_active_policy = ccu_clk_get_active_policy,
	.save_state = ccu_clk_save_state,
	.get_dbg_bus_status = ccu_clk_get_dbg_bus_status,
	.set_dbg_bus_sel = ccu_clk_set_dbg_bus_sel,
	.get_dbg_bus_sel = ccu_clk_get_dbg_bus_sel,
};

/*Generic ccu ops functions*/

static int ccu_clk_enable(struct clk *clk, int enable)
{
	int ret = 0;
	clk_dbg("%s enable: %d, ccu name:%s\n", __func__, enable, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_CCU);
	return ret;
}

static int ccu_clk_init(struct clk *clk)
{
	struct ccu_clk *ccu_clk;
	int inx;
	u32 reg_val;
	struct opp_info opp_info;

	clk_dbg("%s - %s\n", __func__, clk->name);
	BUG_ON(clk->clk_type != CLK_TYPE_CCU);

	ccu_clk = to_ccu_clk(clk);

	/* enable write access */
	ccu_write_access_enable(ccu_clk, true);
	/*stop policy engine */
	ccu_policy_engine_stop(ccu_clk);

	/*Enabel ALL policy mask by default --  TBD- SHOULD WE DO THIS ???? */
	reg_val = CCU_POLICY_MASK_ENABLE_ALL_MASK;

	for (inx = CCU_POLICY0; inx <= CCU_POLICY3; inx++) {
		if (ccu_clk->policy_mask1_offset)
			writel(reg_val,
			       (CCU_POLICY_MASK1_REG(ccu_clk) + 4 * inx));

		if (ccu_clk->policy_mask2_offset)
			writel(reg_val,
			       (CCU_POLICY_MASK2_REG(ccu_clk) + 4 * inx));
	}

	BUG_ON(ccu_clk->freq_count > MAX_CCU_FREQ_COUNT);
	/*Init voltage table */
	for (inx = 0; inx < ccu_clk->freq_count; inx++) {
		ccu_set_voltage(ccu_clk, inx, ccu_clk->freq_volt[inx]);
	}
	/*PROC ccu doea not have the PERI voltage registers */
	if (ccu_clk->vlt_peri_offset != 0) {
		/*Init peri voltage table  */
		for (inx = 0; inx < MAX_CCU_PERI_VLT_COUNT; inx++) {
			ccu_set_peri_voltage(ccu_clk, inx,
					     ccu_clk->volt_peri[inx]);
		}
	}

	opp_info.ctrl_prms = CCU_POLICY_FREQ_REG_INIT;
	if (ccu_clk->policy_freq_offset != 0) {
		/*Init freq policy */
		for (inx = 0; inx < MAX_CCU_POLICY_COUNT; inx++) {
			BUG_ON(ccu_clk->freq_policy[inx] >=
			       ccu_clk->freq_count);
			opp_info.freq_id = ccu_clk->freq_policy[inx];
			ccu_set_freq_policy(ccu_clk, inx, &opp_info);
		}
	}
	/*Set ATL & AC */
	if (clk->flags & CCU_TARGET_LOAD) {
		if (clk->flags & CCU_TARGET_AC)
			ccu_set_policy_ctrl(ccu_clk, POLICY_CTRL_GO_AC,
					    CCU_AUTOCOPY_ON);
		ccu_policy_engine_resume(ccu_clk, CCU_LOAD_TARGET);
	} else
		ccu_policy_engine_resume(ccu_clk, CCU_LOAD_ACTIVE);
	/* disable write access */
	ccu_write_access_enable(ccu_clk, false);

	return 0;
}

struct gen_clk_ops gen_ccu_clk_ops = {
	.init = ccu_clk_init,
	.enable = ccu_clk_enable,
};

int peri_clk_set_policy_mask(struct peri_clk *peri_clk, int policy_id, int mask)
{
	u32 reg_val;
	u32 policy_offset = 0;

	clk_dbg("%s\n", __func__);
	if (!peri_clk->ccu_clk) {
		BUG_ON(1);
		return -EINVAL;
	}
	if (peri_clk->mask_set == 1) {
		policy_offset = peri_clk->ccu_clk->policy_mask1_offset;
	} else if (peri_clk->mask_set == 2) {
		policy_offset = peri_clk->ccu_clk->policy_mask2_offset;
	} else
		return -EINVAL;

	if (!policy_offset)
		return -EINVAL;

	policy_offset = policy_offset + (4 * policy_id);

	clk_dbg("%s offset: %08x, mask: %08x, bit_mask: %d\n", __func__,
		policy_offset, mask, peri_clk->policy_bit_mask);
	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, policy_offset));
	if (mask)
		reg_val =
		    SET_BIT_USING_MASK(reg_val, peri_clk->policy_bit_mask);
	else
		reg_val =
		    RESET_BIT_USING_MASK(reg_val, peri_clk->policy_bit_mask);

	clk_dbg("%s writing %08x to %08x\n", __func__, reg_val,
		(u32) peri_clk->ccu_clk->ccu_clk_mgr_base + policy_offset);
	writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk, policy_offset));

	return 0;
}
EXPORT_SYMBOL(peri_clk_set_policy_mask);

int peri_clk_get_policy_mask(struct peri_clk *peri_clk, int policy_id)
{
	u32 policy_offset = 0;
	u32 reg_val;

	if (!peri_clk->ccu_clk) {
		BUG_ON(1);
		return -EINVAL;
	}
	BUG_ON(policy_id < CCU_POLICY0 || policy_id > CCU_POLICY3);
	if (peri_clk->mask_set == 1) {
		if (!peri_clk->ccu_clk->policy_mask1_offset)
			return -EINVAL;
		policy_offset = peri_clk->ccu_clk->policy_mask1_offset;
	} else if (peri_clk->mask_set == 2) {
		if (!peri_clk->ccu_clk->policy_mask2_offset)
			return -EINVAL;
		policy_offset = peri_clk->ccu_clk->policy_mask2_offset;
	} else
		return -EINVAL;

	policy_offset = policy_offset + 4 * policy_id;

	reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk, policy_offset));
	return GET_BIT_USING_MASK(reg_val, peri_clk->policy_bit_mask);

}
EXPORT_SYMBOL(peri_clk_get_policy_mask);

int peri_clk_get_gating_ctrl(struct peri_clk *peri_clk)
{
	u32 reg_val;

	if (!peri_clk->clk_gate_offset || !peri_clk->gating_sel_mask)
		return -EINVAL;

	BUG_ON(!peri_clk->ccu_clk);
	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);
}

static int peri_clk_set_gating_ctrl(struct peri_clk *peri_clk, int gating_ctrl)
{
	u32 reg_val;

	if (gating_ctrl != CLK_GATING_AUTO && gating_ctrl != CLK_GATING_SW)
		return -EINVAL;
	if (!peri_clk->clk_gate_offset || !peri_clk->gating_sel_mask)
		return -EINVAL;

	reg_val =
	    readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	if (gating_ctrl == CLK_GATING_SW)
		reg_val =
		    SET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);
	else
		reg_val =
		    RESET_BIT_USING_MASK(reg_val, peri_clk->gating_sel_mask);

	writel(reg_val,
	       CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	return 0;
}

int peri_clk_set_hw_gating_ctrl(struct clk *clk, int gating_ctrl)
{
	int ret = 0;
	struct peri_clk *peri_clk;
	if (clk->clk_type != CLK_TYPE_PERI) {
		BUG_ON(1);
		return -EPERM;
	}
	peri_clk = to_peri_clk(clk);
	ret = peri_clk_set_gating_ctrl(peri_clk, gating_ctrl);

	return (ret);
}
EXPORT_SYMBOL(peri_clk_set_hw_gating_ctrl);

int peri_clk_get_pll_select(struct peri_clk *peri_clk)
{
	u32 reg_val;

	if (!peri_clk->clk_div.pll_select_offset
	    || !peri_clk->clk_div.pll_select_mask)
		return -EINVAL;

	reg_val =
	    readl(CCU_REG_ADDR
		  (peri_clk->ccu_clk, peri_clk->clk_div.pll_select_offset));

	return GET_VAL_USING_MASK_SHIFT(reg_val,
					peri_clk->clk_div.pll_select_mask,
					peri_clk->clk_div.pll_select_shift);
}
EXPORT_SYMBOL(peri_clk_get_pll_select);

int peri_clk_set_pll_select(struct peri_clk *peri_clk, int source)
{
	u32 reg_val;
	if (!peri_clk->clk_div.pll_select_offset ||
	    !peri_clk->clk_div.pll_select_mask
	    || source >= peri_clk->src.count)
		return -EINVAL;

	reg_val =
	    readl(CCU_REG_ADDR
		  (peri_clk->ccu_clk, peri_clk->clk_div.pll_select_offset));
	reg_val =
	    SET_VAL_USING_MASK_SHIFT(reg_val, peri_clk->clk_div.pll_select_mask,
				     peri_clk->clk_div.pll_select_shift,
				     source);
	writel(reg_val,
	       CCU_REG_ADDR(peri_clk->ccu_clk,
			    peri_clk->clk_div.pll_select_offset));

	return 0;
}
EXPORT_SYMBOL(peri_clk_set_pll_select);

int peri_clk_hyst_enable(struct peri_clk *peri_clk, int enable, int delay)
{
	u32 reg_val;

	if (!peri_clk->clk_gate_offset || !peri_clk->hyst_val_mask
	    || !peri_clk->hyst_en_mask)
		return -EINVAL;

	if (enable) {
		if (delay != CLK_HYST_LOW && delay != CLK_HYST_HIGH)
			return -EINVAL;
	}

	reg_val =
	    readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	if (enable) {
		reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->hyst_en_mask);
		if (delay == CLK_HYST_HIGH)
			reg_val =
			    SET_BIT_USING_MASK(reg_val,
					       peri_clk->hyst_val_mask);
		else
			reg_val =
			    RESET_BIT_USING_MASK(reg_val,
						 peri_clk->hyst_val_mask);
	} else
		reg_val = RESET_BIT_USING_MASK(reg_val, peri_clk->hyst_en_mask);

	writel(reg_val,
	       CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	return 0;
}

EXPORT_SYMBOL(peri_clk_hyst_enable);

int peri_clk_get_gating_status(struct peri_clk *peri_clk)
{
	u32 reg_val;

	BUG_ON(!peri_clk->ccu_clk);
	if (!peri_clk->clk_gate_offset || !peri_clk->stprsts_mask)
		return -EINVAL;
	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, peri_clk->stprsts_mask);
}

int peri_clk_get_enable_bit(struct peri_clk *peri_clk)
{
	u32 reg_val;

	BUG_ON(!peri_clk->ccu_clk);
	if (!peri_clk->clk_gate_offset || !peri_clk->clk_en_mask)
		return -EINVAL;
	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, peri_clk->clk_en_mask);
}

static int peri_clk_set_voltage_lvl(struct peri_clk *peri_clk, int voltage_lvl)
{
	u32 reg_val;

	if (!peri_clk->clk_gate_offset || !peri_clk->volt_lvl_mask)
		return -EINVAL;
	if (voltage_lvl != VLT_NORMAL && voltage_lvl != VLT_HIGH)
		return -EINVAL;

	reg_val =
	    readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	if (voltage_lvl == VLT_HIGH)
		reg_val = SET_BIT_USING_MASK(reg_val, peri_clk->volt_lvl_mask);
	else
		reg_val =
		    RESET_BIT_USING_MASK(reg_val, peri_clk->volt_lvl_mask);

	writel(reg_val,
	       CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	return 0;
}

static int peri_clk_enable(struct clk *clk, int enable)
{
	u32 reg_val;
	struct peri_clk *peri_clk;
	int insurance;
	int ret = 0;
	clk_dbg("%s:%d, clock name: %s\n", __func__, enable, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);
	peri_clk = to_peri_clk(clk);

	BUG_ON(!peri_clk->ccu_clk || (peri_clk->clk_gate_offset == 0));

	if (clk->flags & AUTO_GATE || !peri_clk->clk_en_mask) {
		clk_dbg("%s:%s: is auto gated or no enable bit\n",
				__func__, clk->name);
		goto err;
	}

	/*enable write access */
	ccu_write_access_enable(peri_clk->ccu_clk, true);

	reg_val =
	    readl(CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));
	clk_dbg("%s, Before change clk_gate reg value: %08x\n", __func__,
		reg_val);
	if (enable)
		reg_val = reg_val | peri_clk->clk_en_mask;
	else
		reg_val = reg_val & ~peri_clk->clk_en_mask;
	clk_dbg("%s, writing %08x to clk_gate reg\n", __func__, reg_val);
	writel(reg_val,
	       CCU_REG_ADDR(peri_clk->ccu_clk, peri_clk->clk_gate_offset));

	clk_dbg("%s:%s clk before stprsts start\n", __func__, clk->name);
	insurance = 0;
	if (enable) {
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (peri_clk->ccu_clk,
				   peri_clk->clk_gate_offset));
			insurance++;
		} while (!(GET_BIT_USING_MASK(reg_val, peri_clk->stprsts_mask))
			 && insurance < CLK_EN_INS_COUNT);
	} else {
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (peri_clk->ccu_clk,
				   peri_clk->clk_gate_offset));
			insurance++;
		} while ((GET_BIT_USING_MASK(reg_val, peri_clk->stprsts_mask))
			 && insurance < CLK_EN_INS_COUNT);
	}
	if (insurance >= CLK_EN_INS_COUNT) {
		__WARN();
		ret = -EINVAL;
	}
	clk_dbg("%s:%s clk after stprsts start\n", __func__, clk->name);

	/* disable write access */
	ccu_write_access_enable(peri_clk->ccu_clk, false);
err:
	if (!ret)
		clk_dbg("****%s: peri clock %s count after %s : %d ****\n",
		     __func__, clk->name, enable ? "enable" : "disable",
			clk->use_cnt);
	else
		clk_dbg("%s, %s : Operation Unsuccesful!\n",
				__func__, clk->name);
	return ret;
}

static u32 compute_rate(u32 rate, u32 div, u32 dither, u32 max_dither,
			u32 pre_div)
{
	u32 res = rate;
	res /= (pre_div + 1);
	/* rate/(X + 1 + Y / 2^n)
	   = (rate*2^n) / (2^n(X+1) + Y)
	   ==> 2^n = max_dither+1 */
	clk_dbg
	    ("%s:src_rate = %d,div = %d, dither = %d,Max_dither = %d,pre_div = %d\n",
	     __func__, rate, div, dither, max_dither, pre_div);
	res =
	    ((res / 100) * (max_dither + 1)) / (((max_dither + 1) * (div + 1)) +
						dither);
	clk_dbg("%s:result = %d\n", __func__, res);
	return res * 100;

}

static u32 peri_clk_calculate_div(struct peri_clk *peri_clk, u32 rate, u32 *div,
				  int *pre_div, int *src_clk_inx)
{
	u32 s, src = 0;
	u32 d, div_int = 0;
	u32 d_d, div_frac = 0;
	u32 pd, pre_d = 0;
	u32 diff = 0xFFFFFFFF;
	u32 temp_rate, temp_diff;
	u32 new_rate = 0;
	u32 src_clk_rate;

	u32 max_div = 0;
	u32 max_diether = 0;
	u32 max_pre_div = 0;
	struct clk_div *clk_div = &peri_clk->clk_div;
	struct src_clk *src_clk = &peri_clk->src;

	if (clk_div->div_offset && clk_div->div_mask)
		max_div = clk_div->div_mask >> clk_div->div_shift;
	max_div = max_div >> clk_div->diether_bits;
	max_diether = ~(0xFFFFFFFF << clk_div->diether_bits);

	if (clk_div->pre_div_offset && clk_div->pre_div_mask)
		max_pre_div = clk_div->pre_div_mask >> clk_div->pre_div_shift;

	for (s = 0; s < src_clk->count; s++) {
		d = 0;
		d_d = 0;
		pd = 0;

		BUG_ON(src_clk->list[s].clk->ops == NULL ||
		       src_clk->list[s].clk->ops->get_rate == NULL);
		src_clk_rate =
			src_clk->list[s].clk->ops->get_rate(
				src_clk->list[s].clk);

		if (rate > src_clk_rate)
			continue;

		else if (src_clk_rate == rate) {
			src = s;
			div_int = d;
			div_frac = d_d;
			pre_d = pd;
			new_rate = rate;
			diff = 0;
			break;
		}

		for (; d <= max_div; d++) {
			d_d = 0;
			pd = 0;

			temp_rate =
			    compute_rate(src_clk_rate, d, d_d, max_diether, pd);
			if (temp_rate == rate) {
				src = s;
				div_int = d;
				div_frac = d_d;
				pre_d = pd;
				new_rate = rate;
				diff = 0;
				goto exit;
			}

			temp_diff = abs(temp_rate - rate);
			if (temp_diff < diff) {
				diff = temp_diff;
				src = s;
				div_int = d;
				div_frac = d_d;
				pre_d = pd;
				new_rate = temp_rate;
			} else if (temp_rate < rate && temp_diff > diff)
				break;

			for (; pd <= max_pre_div; pd++) {
				d_d = 0;

				temp_rate =
				    compute_rate(src_clk_rate, d, d_d,
						 max_diether, pd);
				if (temp_rate == rate) {
					src = s;
					div_int = d;
					div_frac = d_d;
					pre_d = pd;
					new_rate = rate;
					diff = 0;
					goto exit;
				}

				temp_diff = abs(temp_rate - rate);
				if (temp_diff < diff) {
					diff = temp_diff;
					src = s;
					div_int = d;
					div_frac = d_d;
					pre_d = pd;
					new_rate = temp_rate;
				} else if (temp_rate < rate && temp_diff > diff)
					break;

				for (d_d = 1; d_d <= max_diether; d_d++) {
					temp_rate =
					    compute_rate(src_clk_rate, d, d_d,
							 max_diether, pd);
					if (temp_rate == rate) {
						src = s;
						div_int = d;
						div_frac = d_d;
						pre_d = pd;
						new_rate = rate;
						diff = 0;
						goto exit;
					}

					temp_diff = abs(temp_rate - rate);
					if (temp_diff < diff) {
						diff = temp_diff;
						src = s;
						div_int = d;
						div_frac = d_d;
						pre_d = pd;
						new_rate = temp_rate;
					} else if (temp_rate < rate
						   && temp_diff > diff)
						break;

				}

			}

		}
	}

exit:

	if (div) {
		*div = div_int;
		clk_dbg("div: %08x\n", *div);
		if (max_diether)
			*div = ((*div) << clk_div->diether_bits) | div_frac;
		clk_dbg("div: %08x, div_frac:%08x\n", *div, div_frac);
	}
	if (pre_div)
		*pre_div = pre_d;

	if (src_clk_inx)
		*src_clk_inx = src;

	return new_rate;

}

static int peri_clk_set_rate(struct clk *clk, u32 rate)
{
	struct peri_clk *peri_clk;
	u32 new_rate, reg_val;
	u32 div, pre_div, src;
	struct clk_div *clk_div;
	int insurance;
	int ret = 0;
	if (clk->clk_type != CLK_TYPE_PERI) {
		ret = -EPERM;
		goto err1;
	}

	peri_clk = to_peri_clk(clk);

	clk_dbg("%s : %s\n", __func__, clk->name);

	if (CLK_FLG_ENABLED(clk, RATE_FIXED)) {
		clk_dbg("%s : %s - fixed rate...rate can't be changed\n",
			__func__, clk->name);
		ret = -EINVAL;
		goto err1;

	}
	/*Clock should be in enabled state to set the rate,.
	   trigger won't work otherwise
	 */
	__peri_clk_enable(clk);
	clk_div = &peri_clk->clk_div;
	new_rate = peri_clk_calculate_div(peri_clk, rate, &div, &pre_div, &src);
	if (abs(rate - new_rate) > CLK_RATE_MAX_DIFF) {
		pr_info("%s : %s - rate(%d) not supported; nearest: %d\n",
			__func__, clk->name, rate, new_rate);
		/* Disable clock to compensate enable call before set rate */
		__peri_clk_disable(clk);
		ret = -EINVAL;
		goto err1;
	}
	clk_dbg
	    ("%s clock name %s, src_rate %u sel %d div %u pre_div %u new_rate %u\n",
	     __func__, clk->name, peri_clk->src.list[src].clk->
		rate, src, div, pre_div, new_rate);

	/* enable write access */
	ccu_write_access_enable(peri_clk->ccu_clk, true);

	if (clk_div->div_offset) {
		/*Write DIV */
		reg_val = readl(CCU_REG_ADDR(peri_clk->ccu_clk,
					clk_div->div_offset));
		reg_val =
			SET_VAL_USING_MASK_SHIFT(reg_val, clk_div->div_mask,
					clk_div->div_shift, div);
		clk_dbg("reg: 0x%x, offset:%x, mask:%x, shift:%x, div:%x\n",
			 reg_val, clk_div->div_offset, clk_div->div_mask,
			 clk_div->div_shift, div);
		writel(reg_val, CCU_REG_ADDR(peri_clk->ccu_clk,
					clk_div->div_offset));
	}

	if (clk_div->pre_div_offset && clk_div->pre_div_mask) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (peri_clk->ccu_clk, clk_div->pre_div_offset));
		reg_val =
		    SET_VAL_USING_MASK_SHIFT(reg_val, clk_div->pre_div_mask,
					     clk_div->pre_div_shift, pre_div);
		writel(reg_val,
		       CCU_REG_ADDR(peri_clk->ccu_clk,
				    clk_div->pre_div_offset));
	}
	/*set the source clock selected */
	peri_clk_set_pll_select(peri_clk, peri_clk->
				src.list[src].val);

	clk_dbg("Before trigger clock\n");
	if (clk_div->div_trig_offset && clk_div->div_trig_mask) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (peri_clk->ccu_clk, clk_div->div_trig_offset));
		clk_dbg
		    ("DIV: tigger offset: %08x, reg_value: %08x trig_mask:%08x\n",
		     clk_div->div_trig_offset, reg_val, clk_div->div_trig_mask);
		reg_val = SET_BIT_USING_MASK(reg_val, clk_div->div_trig_mask);
		writel(reg_val,
		       CCU_REG_ADDR(peri_clk->ccu_clk,
				    clk_div->div_trig_offset));
		insurance = 0;
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (peri_clk->ccu_clk,
				   clk_div->div_trig_offset));
			clk_dbg("reg_val: %08x, trigger bit: %08x\n", reg_val,
				GET_BIT_USING_MASK(reg_val,
						   clk_div->div_trig_mask));
			insurance++;
		}
		while ((GET_BIT_USING_MASK(reg_val, clk_div->div_trig_mask))
		       && insurance < CLK_EN_INS_COUNT);
		if (insurance >= CLK_EN_INS_COUNT) {
			__WARN();
			ret = -EINVAL;
			goto err;
		}
	}
	if (clk_div->prediv_trig_offset && clk_div->prediv_trig_mask) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (peri_clk->ccu_clk, clk_div->prediv_trig_offset));
		clk_dbg("PREDIV: trig offset: %x, reg_value: %x trig_mask:%x\n",
			clk_div->prediv_trig_offset, reg_val,
			clk_div->prediv_trig_mask);
		reg_val =
		    SET_BIT_USING_MASK(reg_val, clk_div->prediv_trig_mask);
		writel(reg_val,
		       CCU_REG_ADDR(peri_clk->ccu_clk,
				    clk_div->prediv_trig_offset));
		insurance = 0;
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (peri_clk->ccu_clk,
				   clk_div->prediv_trig_offset));
			clk_dbg("reg_val: %08x, trigger bit: %08x\n", reg_val,
				GET_BIT_USING_MASK(reg_val,
						   clk_div->prediv_trig_mask));
			insurance++;
		}
		while ((GET_BIT_USING_MASK(reg_val, clk_div->prediv_trig_mask))
		       && insurance < CLK_EN_INS_COUNT);
		if (insurance >= CLK_EN_INS_COUNT) {
			__WARN();
			ret = -EINVAL;
			goto err;
		}
	}
	/* disable write access */
err:
	ccu_write_access_enable(peri_clk->ccu_clk, false);
	/* Disable clock to compensate enable call before set rate */
	__peri_clk_disable(clk);
err1:
	if (!ret)
		clk_dbg("clock set rate done\n");
	else
		clk_dbg("clock set rate not done\n");
	return ret;
}

static int peri_clk_init(struct clk *clk)
{
	struct peri_clk *peri_clk;
	struct src_clk *src_clks;
	int inx;
	unsigned long flgs;

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);
	peri_clk = to_peri_clk(clk);

	BUG_ON(peri_clk->ccu_clk == NULL);

	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	/* enable write access */
	ccu_write_access_enable(peri_clk->ccu_clk, true);

	/*Init dependent clocks .... */
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s dep clock %s init\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(peri_clk->ccu_clk, clk->dep_clks[inx],
			&flgs);
		__clk_init(clk->dep_clks[inx]);
		dep_clk_unlock(peri_clk->ccu_clk, clk->dep_clks[inx],
			&flgs);
	}
	/*Init source clocks */
	/*enable/disable src clk */
	BUG_ON(!PERI_SRC_CLK_VALID(peri_clk)
	       && peri_clk->clk_div.pll_select_offset);

	if (PERI_SRC_CLK_VALID(peri_clk)) {
		src_clks = &peri_clk->src;
		for (inx = 0; inx < src_clks->count; inx++) {
			clk_dbg("%s src clock %s init\n", __func__,
				src_clks->list[inx].clk->name);
			dep_clk_lock(peri_clk->ccu_clk,
				src_clks->list[inx].clk, &flgs);
			__clk_init(src_clks->list[inx].clk);
			dep_clk_unlock(peri_clk->ccu_clk,
				src_clks->list[inx].clk, &flgs);
		}
		/*set the default src clock */
		BUG_ON(peri_clk->src.src_inx >= peri_clk->src.count);
		peri_clk_set_pll_select(peri_clk,
			src_clks->list[src_clks->src_inx].val);
	}

	peri_clk_set_voltage_lvl(peri_clk, VLT_NORMAL);
	peri_clk_hyst_enable(peri_clk, HYST_ENABLE & clk->flags,
			     (clk->
			      flags & HYST_HIGH) ? CLK_HYST_HIGH :
			     CLK_HYST_LOW);

	if (clk->flags & AUTO_GATE)
		peri_clk_set_gating_ctrl(peri_clk, CLK_GATING_AUTO);
	else
		peri_clk_set_gating_ctrl(peri_clk, CLK_GATING_SW);

	clk_dbg("%s: before setting the mask\n", __func__);
	/*This is temporary, if PM initializes the policy mask of each clock then
	 * this can be removed. */
	/*stop policy engine */
	ccu_policy_engine_stop(peri_clk->ccu_clk);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY0,
				 peri_clk->policy_mask_init[0]);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY1,
				 peri_clk->policy_mask_init[1]);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY2,
				 peri_clk->policy_mask_init[2]);
	peri_clk_set_policy_mask(peri_clk, CCU_POLICY3,
				 peri_clk->policy_mask_init[3]);
	/*start policy engine */
	ccu_policy_engine_resume(peri_clk->ccu_clk, CCU_LOAD_ACTIVE);

	BUG_ON(CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)
	       && CLK_FLG_ENABLED(clk, DISABLE_ON_INIT));

	if (CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)) {
		__peri_clk_enable(clk);
	}

	else if (CLK_FLG_ENABLED(clk, DISABLE_ON_INIT)) {
		if (clk->ops->enable) {
			clk->ops->enable(clk, 0);
		}
	}

	/* Disable write access */
	ccu_write_access_enable(peri_clk->ccu_clk, false);

	clk_dbg
	    ("*************%s: peri clock %s count after init %d **************\n",
	     __func__, clk->name, clk->use_cnt);

	return 0;
}

static unsigned long peri_clk_round_rate(struct clk *clk, unsigned long rate)
{
	u32 new_rate;
	struct peri_clk *peri_clk;
	struct clk_div *clk_div = NULL;

	if (clk->clk_type != CLK_TYPE_PERI)
		return -EPERM;

	peri_clk = to_peri_clk(clk);

	clk_div = &peri_clk->clk_div;
	if (clk_div == NULL)
		return -EPERM;

	new_rate = peri_clk_calculate_div(peri_clk, rate, NULL, NULL, NULL);
	clk_dbg("%s:rate = %d\n", __func__, new_rate);

	return new_rate;
}

static unsigned long peri_clk_get_rate(struct clk *clk)
{
	struct peri_clk *peri_clk;
	int sel = -1;
	u32 div = 0, pre_div = 0;
	u32 reg_val = 0;
	u32 max_diether;
	struct clk_div *clk_div;
	u32 parent_rate, dither = 0;
	struct clk *src_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_PERI);

	peri_clk = to_peri_clk(clk);

	if (CLK_FLG_ENABLED(clk, RATE_FIXED)) {
		clk_dbg("%s : %s - fixed rate clk...\n", __func__, clk->name);
		return clk->rate;

	}

	clk_div = &peri_clk->clk_div;
	if (clk_div->div_offset && clk_div->div_mask) {
		reg_val =
		    readl(CCU_REG_ADDR(peri_clk->ccu_clk, clk_div->div_offset));
		clk_dbg("div_offset:%08x reg_val:%08x\n", clk_div->div_offset,
			reg_val);
		div =
		    GET_VAL_USING_MASK_SHIFT(reg_val, clk_div->div_mask,
					     clk_div->div_shift);
	}
	if (clk_div->diether_bits) {
		clk_dbg("div:%u, dither mask: %08x", div,
			~(0xFFFFFFFF << clk_div->diether_bits));
		dither = div & ~(0xFFFFFFFF << clk_div->diether_bits);
		div = div >> clk_div->diether_bits;
		clk_dbg("dither: %u div : %u\n", dither, div);
	}
	if (clk_div->pre_div_offset && clk_div->pre_div_mask) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (peri_clk->ccu_clk, clk_div->pre_div_offset));
		pre_div =
		    GET_VAL_USING_MASK_SHIFT(reg_val, clk_div->pre_div_mask,
					     clk_div->pre_div_shift);
		clk_dbg("pre div : %u\n", pre_div);
	}
	if (clk_div->pll_select_offset && clk_div->pll_select_mask) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (peri_clk->ccu_clk, clk_div->pll_select_offset));
		sel =
		    GET_VAL_USING_MASK_SHIFT(reg_val, clk_div->pll_select_mask,
					     clk_div->pll_select_shift);
		clk_dbg("pll_sel : %u\n", sel);
	}

	BUG_ON(sel >= peri_clk->src.count);
	/*For clocks which doesnt have PLL select value, sel will be -1 */
	if (sel >= 0)
		peri_clk->src.src_inx = sel;
	src_clk = GET_PERI_SRC_CLK(peri_clk);
	BUG_ON(!src_clk || !src_clk->ops || !src_clk->ops->get_rate);
	parent_rate = src_clk->ops->get_rate(src_clk);
	max_diether = ~(0xFFFFFFFF << clk_div->diether_bits);

	clk->rate =
	    compute_rate(parent_rate, div, dither, max_diether, pre_div);

	clk_dbg
	    ("%s clock name %s, src_rate %u sel %d div %u pre_div %u dither %u rate %u\n",
	     __func__, clk->name, peri_clk->src.list[sel].clk->
		rate, sel, div, pre_div, dither, clk->rate);
	return clk->rate;
}

static int peri_clk_reset(struct clk *clk)
{
	u32 reg_val;
	struct peri_clk *peri_clk;

	if (clk->clk_type != CLK_TYPE_PERI) {
		BUG_ON(1);
		return -EPERM;
	}

	peri_clk = to_peri_clk(clk);
	clk_dbg("%s -- %s to be reset\n", __func__, clk->name);

	BUG_ON(!peri_clk->ccu_clk);
	if (!peri_clk->soft_reset_offset || !peri_clk->clk_reset_mask)
		return -EPERM;

	CCU_ACCESS_EN(peri_clk->ccu_clk, 1);

	/* enable write access */
	ccu_reset_write_access_enable(peri_clk->ccu_clk, true);

	reg_val =
	    readl(peri_clk->ccu_clk->ccu_reset_mgr_base +
		  peri_clk->soft_reset_offset);
	clk_dbg("reset offset: %08x, reg_val: %08x\n",
		(u32) (peri_clk->ccu_clk->ccu_reset_mgr_base +
		 peri_clk->soft_reset_offset), reg_val);
	reg_val = reg_val & ~peri_clk->clk_reset_mask;
	clk_dbg("writing reset value: %08x\n", reg_val);
	writel(reg_val,
	       peri_clk->ccu_clk->ccu_reset_mgr_base +
	       peri_clk->soft_reset_offset);

	udelay(10);

	reg_val = reg_val | peri_clk->clk_reset_mask;
	clk_dbg("writing reset release value: %08x\n", reg_val);

	writel(reg_val,
	       peri_clk->ccu_clk->ccu_reset_mgr_base +
	       peri_clk->soft_reset_offset);

	ccu_reset_write_access_enable(peri_clk->ccu_clk, false);

	CCU_ACCESS_EN(peri_clk->ccu_clk, 0);

	return 0;
}

struct gen_clk_ops gen_peri_clk_ops = {
	.init = peri_clk_init,
	.enable = peri_clk_enable,
	.set_rate = peri_clk_set_rate,
	.get_rate = peri_clk_get_rate,
	.round_rate = peri_clk_round_rate,
	.reset = peri_clk_reset,
};

int bus_clk_get_gating_ctrl(struct bus_clk *bus_clk)
{
	u32 reg_val;

	if (!bus_clk->clk_gate_offset || !bus_clk->gating_sel_mask)
		return -EINVAL;

	BUG_ON(!bus_clk->ccu_clk);
	reg_val =
	    readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	return GET_BIT_USING_MASK(reg_val, bus_clk->gating_sel_mask);
}

EXPORT_SYMBOL(bus_clk_get_gating_ctrl);

static int bus_clk_set_gating_ctrl(struct bus_clk *bus_clk, int gating_ctrl)
{
	u32 reg_val;

	if (!bus_clk->clk_gate_offset || !bus_clk->gating_sel_mask)
		return -EINVAL;

	if (gating_ctrl != CLK_GATING_AUTO && gating_ctrl != CLK_GATING_SW)
		return -EINVAL;

	reg_val =
	    readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));
	if (CLK_GATING_SW == gating_ctrl)
		reg_val = SET_BIT_USING_MASK(reg_val, bus_clk->gating_sel_mask);
	else
		reg_val =
		    RESET_BIT_USING_MASK(reg_val, bus_clk->gating_sel_mask);
	writel(reg_val,
	       CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	return 0;
}

int bus_clk_get_gating_status(struct bus_clk *bus_clk)
{
	u32 reg_val;

	BUG_ON(!bus_clk->ccu_clk);
	if (!bus_clk->clk_gate_offset || !bus_clk->stprsts_mask)
		return -EINVAL;
	CCU_ACCESS_EN(bus_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));
	CCU_ACCESS_EN(bus_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, bus_clk->stprsts_mask);
}

int bus_clk_get_enable_bit(struct bus_clk *bus_clk)
{
	u32 reg_val;

	BUG_ON(!bus_clk->ccu_clk);
	if (!bus_clk->clk_gate_offset || !bus_clk->clk_en_mask)
		return -EINVAL;
	reg_val =
	    readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	return GET_BIT_USING_MASK(reg_val, bus_clk->clk_en_mask);
}

static int bus_clk_hyst_enable(struct bus_clk *bus_clk, int enable, int delay)
{
	u32 reg_val;

	if (!bus_clk->clk_gate_offset || !bus_clk->hyst_val_mask
	    || !bus_clk->hyst_en_mask)
		return -EINVAL;

	if (enable) {
		if (delay != CLK_HYST_LOW && delay != CLK_HYST_HIGH)
			return -EINVAL;
	}

	reg_val =
	    readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	if (enable) {
		reg_val = SET_BIT_USING_MASK(reg_val, bus_clk->hyst_en_mask);
		if (delay == CLK_HYST_HIGH)
			reg_val =
			    SET_BIT_USING_MASK(reg_val, bus_clk->hyst_val_mask);
		else
			reg_val =
			    RESET_BIT_USING_MASK(reg_val,
						 bus_clk->hyst_val_mask);
	} else
		reg_val = RESET_BIT_USING_MASK(reg_val, bus_clk->hyst_en_mask);

	writel(reg_val,
	       CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));
	return 0;
}

/* bus clocks */
static int bus_clk_enable(struct clk *clk, int enable)
{
	struct bus_clk *bus_clk;
	u32 reg_val;
	int insurance;
	int ret = 0;

	BUG_ON(clk->clk_type != CLK_TYPE_BUS);

	clk_dbg("%s -- %s to be %s\n", __func__, clk->name,
		enable ? "enabled" : "disabled");

	bus_clk = to_bus_clk(clk);

	if ((bus_clk->clk_gate_offset == 0) || (bus_clk->clk_en_mask == 0)) {
		ret = -EPERM;
		goto err;
	}

	if (clk->flags & AUTO_GATE) {
		clk_dbg("%s:%s: is auto gated\n", __func__, clk->name);
		goto err;
	}

	/* enable write access */
	ccu_write_access_enable(bus_clk->ccu_clk, true);

	reg_val =
	    readl(CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));
	clk_dbg("gate offset: %08x, reg_val: %08x, enable:%u\n",
		bus_clk->clk_gate_offset, reg_val, enable);
	if (enable)
		reg_val = reg_val | bus_clk->clk_en_mask;
	else
		reg_val = reg_val & ~bus_clk->clk_en_mask;
	clk_dbg("%s, writing %08x to clk_gate reg %08x\n", __func__, reg_val,
		(u32) (bus_clk->ccu_clk->ccu_clk_mgr_base +
		 bus_clk->clk_gate_offset));
	writel(reg_val,
	       CCU_REG_ADDR(bus_clk->ccu_clk, bus_clk->clk_gate_offset));

	clk_dbg("%s:%s clk before stprsts start\n", __func__, clk->name);
	insurance = 0;
	if (enable) {
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (bus_clk->ccu_clk, bus_clk->clk_gate_offset));
			insurance++;
		} while (!(GET_BIT_USING_MASK(reg_val, bus_clk->stprsts_mask))
			 && insurance < CLK_EN_INS_COUNT);

	} else {
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (bus_clk->ccu_clk, bus_clk->clk_gate_offset));
			insurance++;
		} while ((GET_BIT_USING_MASK(reg_val, bus_clk->stprsts_mask))
			 && insurance < CLK_EN_INS_COUNT);
	}
	if (insurance >= CLK_EN_INS_COUNT) {
		__WARN();
		ret = -EINVAL;
	}

	clk_dbg("%s:%s clk after stprsts start\n", __func__, clk->name);

	/* disable write access */
	ccu_write_access_enable(bus_clk->ccu_clk, false);
err:
	if (!ret)
		clk_dbg("****%s: bus clock %s count after %s : %d****\n",
			__func__, clk->name, enable ? "enable" : "disable",
			clk->use_cnt);
	else
		clk_dbg("%s : %s : Operation Unsuccessful\n",
				__func__, clk->name);

	return ret;
}

static unsigned long bus_clk_get_rate(struct clk *c)
{
	struct bus_clk *bus_clk = to_bus_clk(c);
	struct ccu_clk *ccu_clk;
	int current_policy;
	int freq_id;

	BUG_ON(!bus_clk->ccu_clk);
	ccu_clk = bus_clk->ccu_clk;

	if (bus_clk->freq_tbl_index == -1) {
		if (!bus_clk->src_clk || !bus_clk->src_clk->ops
		    || !bus_clk->src_clk->ops->get_rate) {
			clk_dbg
			    ("This bus clock freq depends on internal dividers\n");
			c->rate = 0;
			goto ret;
		}
		c->rate = bus_clk->src_clk->ops->get_rate(bus_clk->src_clk);
		goto ret;
	}
	current_policy = ccu_get_active_policy(ccu_clk);

	freq_id = ccu_get_freq_policy(ccu_clk, current_policy);
	if (freq_id < 0)
		return 0;
	BUG_ON(freq_id >= MAX_CCU_FREQ_COUNT);

	clk_dbg("current_policy: %d freq_id %d freq_tbl_index :%d\n",
		current_policy, freq_id, bus_clk->freq_tbl_index);
	c->rate = ccu_clk->freq_tbl[freq_id][bus_clk->freq_tbl_index];
ret:
	clk_dbg("clock rate: %ld\n", (long int)c->rate);

	return c->rate;
}

static int bus_clk_init(struct clk *clk)
{
	struct bus_clk *bus_clk;
	int inx;
	unsigned long flgs;

	BUG_ON(clk->clk_type != CLK_TYPE_BUS);

	bus_clk = to_bus_clk(clk);
	BUG_ON(bus_clk->ccu_clk == NULL);

	clk_dbg("%s - %s\n", __func__, clk->name);

	/* Enable write access */
	ccu_write_access_enable(bus_clk->ccu_clk, true);

	clk_dbg("%s init dep clks -- %s\n", __func__, clk->name);
	/*Init dependent clocks, if any */
	for (inx = 0; inx < MAX_DEP_CLKS && clk->dep_clks[inx]; inx++) {
		clk_dbg("%s Dependant clock %s init\n", __func__,
			clk->dep_clks[inx]->name);
		dep_clk_lock(bus_clk->ccu_clk, clk->dep_clks[inx],
			&flgs);
		__clk_init(clk->dep_clks[inx]);
		dep_clk_unlock(bus_clk->ccu_clk, clk->dep_clks[inx],
			&flgs);
	}

	clk_dbg("%s init src clks -- %s\n", __func__, clk->name);
	/*Init src clk, if any */
	if (bus_clk->src_clk) {
		clk_dbg("%s src clock %s init\n", __func__,
			bus_clk->src_clk->name);
		dep_clk_lock(bus_clk->ccu_clk, bus_clk->src_clk,
			&flgs);
		__clk_init(bus_clk->src_clk);
		dep_clk_unlock(bus_clk->ccu_clk, bus_clk->src_clk,
			&flgs);
	}

	if (bus_clk->hyst_val_mask)
		bus_clk_hyst_enable(bus_clk, HYST_ENABLE & clk->flags,
				    (clk->
				     flags & HYST_HIGH) ? CLK_HYST_HIGH :
				    CLK_HYST_LOW);

	if (clk->flags & AUTO_GATE)
		bus_clk_set_gating_ctrl(bus_clk, CLK_GATING_AUTO);
	else
		bus_clk_set_gating_ctrl(bus_clk, CLK_GATING_SW);

	BUG_ON(CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)
	       && CLK_FLG_ENABLED(clk, DISABLE_ON_INIT));

	if (CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)) {
		__bus_clk_enable(clk);
	}
	if (CLK_FLG_ENABLED(clk, DISABLE_ON_INIT)) {
		if (clk->ops->enable) {
			clk->ops->enable(clk, 0);
		}
	}

	/* Disable write access */
	ccu_write_access_enable(bus_clk->ccu_clk, false);
	clk_dbg("%s init complete\n", clk->name);
	clk_dbg
	    ("*************%s: bus clock %s count after init %d ***************\n",
	     __func__, clk->name, clk->use_cnt);

	return 0;
}

static int bus_clk_reset(struct clk *clk)
{
	struct bus_clk *bus_clk;
	u32 reg_val;

	if (clk->clk_type != CLK_TYPE_BUS) {
		BUG_ON(1);
		return -EPERM;
	}
	clk_dbg("%s -- %s to be reset\n", __func__, clk->name);

	bus_clk = to_bus_clk(clk);

	BUG_ON(!bus_clk->ccu_clk);
	if (!bus_clk->soft_reset_offset || !bus_clk->clk_reset_mask)
		return -EPERM;

	CCU_ACCESS_EN(bus_clk->ccu_clk, 1);

	/* enable write access */
	ccu_reset_write_access_enable(bus_clk->ccu_clk, true);

	reg_val =
	    readl(bus_clk->ccu_clk->ccu_reset_mgr_base +
		  bus_clk->soft_reset_offset);

	clk_dbg("reset offset: %08x, reg_val: %08x\n",
		(u32) (bus_clk->ccu_clk->ccu_clk_mgr_base +
		 bus_clk->soft_reset_offset), reg_val);
	reg_val = reg_val & ~bus_clk->clk_reset_mask;
	clk_dbg("writing reset val: %08x\n", reg_val);
	writel(reg_val,
	       bus_clk->ccu_clk->ccu_reset_mgr_base +
	       bus_clk->soft_reset_offset);

	udelay(10);

	reg_val = reg_val | bus_clk->clk_reset_mask;
	clk_dbg("writing reset release val: %08x\n", reg_val);
	writel(reg_val,
	       bus_clk->ccu_clk->ccu_reset_mgr_base +
	       bus_clk->soft_reset_offset);

	/* disable write access */
	ccu_reset_write_access_enable(bus_clk->ccu_clk, false);

	CCU_ACCESS_EN(bus_clk->ccu_clk, 0);

	return 0;
}

struct gen_clk_ops gen_bus_clk_ops = {
	.init = bus_clk_init,
	.enable = bus_clk_enable,
	.get_rate = bus_clk_get_rate,
	.reset = bus_clk_reset,
};

int ref_clk_get_gating_status(struct ref_clk *ref_clk)
{
	u32 reg_val;

	BUG_ON(!ref_clk->ccu_clk);
	if (!ref_clk->clk_gate_offset || !ref_clk->stprsts_mask)
		return -EINVAL;
	CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));
	CCU_ACCESS_EN(ref_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, ref_clk->stprsts_mask);
}

int ref_clk_get_enable_bit(struct ref_clk *ref_clk)
{
	u32 reg_val;

	BUG_ON(!ref_clk->ccu_clk);
	if (!ref_clk->clk_gate_offset || !ref_clk->clk_en_mask)
		return -EINVAL;
	CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));
	CCU_ACCESS_EN(ref_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, ref_clk->clk_en_mask);
}

int ref_clk_get_gating_ctrl(struct ref_clk *ref_clk)
{
	u32 reg_val;

	if (!ref_clk->clk_gate_offset || !ref_clk->gating_sel_mask)
		return -EINVAL;

	BUG_ON(!ref_clk->ccu_clk);
	CCU_ACCESS_EN(ref_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));
	CCU_ACCESS_EN(ref_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, ref_clk->gating_sel_mask);
}

static int ref_clk_set_gating_ctrl(struct ref_clk *ref_clk, int gating_ctrl)
{
	u32 reg_val;

	if (gating_ctrl != CLK_GATING_AUTO && gating_ctrl != CLK_GATING_SW)
		return -EINVAL;
	if (!ref_clk->clk_gate_offset || !ref_clk->gating_sel_mask)
		return -EINVAL;

	reg_val =
	    readl(CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));
	if (gating_ctrl == CLK_GATING_SW)
		reg_val = SET_BIT_USING_MASK(reg_val, ref_clk->gating_sel_mask);
	else
		reg_val =
		    RESET_BIT_USING_MASK(reg_val, ref_clk->gating_sel_mask);

	writel(reg_val,
	       CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));

	return 0;
}

static int ref_clk_hyst_enable(struct ref_clk *ref_clk, int enable, int delay)
{
	u32 reg_val;

	if (!ref_clk->clk_gate_offset || !ref_clk->hyst_val_mask
	    || !ref_clk->hyst_en_mask)
		return -EINVAL;

	if (enable) {
		if (delay != CLK_HYST_LOW && delay != CLK_HYST_HIGH)
			return -EINVAL;
	}

	reg_val =
	    readl(CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));

	if (enable) {
		reg_val = SET_BIT_USING_MASK(reg_val, ref_clk->hyst_en_mask);
		if (delay == CLK_HYST_HIGH)
			reg_val =
			    SET_BIT_USING_MASK(reg_val, ref_clk->hyst_val_mask);
		else
			reg_val =
			    RESET_BIT_USING_MASK(reg_val,
						 ref_clk->hyst_val_mask);
	} else
		reg_val = RESET_BIT_USING_MASK(reg_val, ref_clk->hyst_en_mask);

	writel(reg_val,
	       CCU_REG_ADDR(ref_clk->ccu_clk, ref_clk->clk_gate_offset));
	return 0;
}

/* reference clocks */
unsigned long ref_clk_get_rate(struct clk *clk)
{
	if (clk->clk_type != CLK_TYPE_REF)
		return -EPERM;
	return clk->rate;
}

static int ref_clk_init(struct clk *clk)
{
	struct ref_clk *ref_clk;

	BUG_ON(clk->clk_type != CLK_TYPE_REF);
	ref_clk = to_ref_clk(clk);
	BUG_ON(ref_clk->ccu_clk == NULL);

	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	/* enable write access */
	ccu_write_access_enable(ref_clk->ccu_clk, true);
	if (ref_clk_get_gating_status(ref_clk) == 1)
		clk->use_cnt = 1;

	ref_clk_hyst_enable(ref_clk, HYST_ENABLE & clk->flags,
			    (clk->
			     flags & HYST_HIGH) ? CLK_HYST_HIGH : CLK_HYST_LOW);

	if (clk->flags & AUTO_GATE)
		ref_clk_set_gating_ctrl(ref_clk, CLK_GATING_AUTO);
	else
		ref_clk_set_gating_ctrl(ref_clk, CLK_GATING_SW);

	clk_dbg("%s: before setting the mask\n", __func__);

	BUG_ON(CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)
	       && CLK_FLG_ENABLED(clk, DISABLE_ON_INIT));
	if (CLK_FLG_ENABLED(clk, ENABLE_ON_INIT)) {
		__ref_clk_enable(clk);
	}

	else if (CLK_FLG_ENABLED(clk, DISABLE_ON_INIT)) {
		if (clk->ops->enable) {
			clk->ops->enable(clk, 0);
		}
	}

	/* Disable write access */
	ccu_write_access_enable(ref_clk->ccu_clk, false);

	return 0;
}

static int ref_clk_enable(struct clk *c, int enable)
{
	return 0;
}

struct gen_clk_ops gen_ref_clk_ops = {
	.init = ref_clk_init,
	.enable = ref_clk_enable,
	.get_rate = ref_clk_get_rate,
};

static u32 compute_pll_vco_rate(u32 ndiv_int, u32 nfrac, u32 frac_div, u32 pdiv)
{
	unsigned long xtal = clock_get_xtal();
	u64 temp;
	/*
	   vco_rate = 26Mhz*(ndiv_int + ndiv_frac/frac_div)/pdiv
	   = 26(*ndiv_int*frac_div + ndiv_frac)/(pdiv*frac_div)
	 */
	//clk_dbg("%s:pdiv = %x, nfrac = %x ndiv_int = %x\n", __func__, pdiv, nfrac, ndiv_int);
	temp = ((u64)(ndiv_int * frac_div + nfrac) * xtal);

	//clk_dbg("%s: temp = %llu\n",__func__,temp);
	do_div(temp, pdiv * frac_div);

	//clk_dbg("%s: after div temp = %llu\n",__func__,temp);
	return (unsigned long)temp;
}

static unsigned long compute_pll_vco_div(struct pll_clk *pll_clk, u32 rate,
					 u32 *pdiv, u32 *ndiv_int, u32 *nfrac)
{
	u32 max_ndiv;
	u32 frac_div;
	u32 _pdiv = 1;
	u32 _ndiv_int, _nfrac;
	u32 temp_rate;
	u32 new_rate;
	unsigned long xtal = clock_get_xtal();
	u64 temp_frac;

	max_ndiv = pll_clk->ndiv_int_max;
	frac_div = 1 + (pll_clk->ndiv_frac_mask >> pll_clk->ndiv_frac_shift);

	_ndiv_int = rate / xtal;	/*pdiv = 1 */

	if (_ndiv_int > max_ndiv)
		_ndiv_int = max_ndiv;

	temp_frac = ((u64)rate - (u64)_ndiv_int * xtal) * frac_div;
	do_div(temp_frac, xtal);

	_nfrac = (u32)temp_frac;

	_nfrac &= (frac_div - 1);

	temp_rate = compute_pll_vco_rate(_ndiv_int, _nfrac, frac_div, _pdiv);

	if (temp_rate != rate) {
		for (; _nfrac < frac_div; _nfrac++) {
			temp_rate =
			    compute_pll_vco_rate(_ndiv_int, _nfrac, frac_div,
						 _pdiv);
			if (temp_rate > rate) {
				u32 temp =
				    compute_pll_vco_rate(_ndiv_int, _nfrac - 1,
							 frac_div, _pdiv);
				if (abs(temp_rate - rate) > abs(rate - temp))
					_nfrac--;
				break;
			}
		}
	}

	new_rate = compute_pll_vco_rate(_ndiv_int, _nfrac, frac_div, _pdiv);

	if (_ndiv_int == max_ndiv)
		_ndiv_int = 0;

	if (ndiv_int)
		*ndiv_int = _ndiv_int;

	if (nfrac)
		*nfrac = _nfrac;

	if (pdiv)
		*pdiv = _pdiv;
	return new_rate;
}


int __pll_set_desense_offset(struct clk *clk, int offset)
{
	struct pll_clk *pll_clk;
	struct pll_desense *des;
	unsigned long rate;
	int off_rate;
	u32 new_rate;
	u32 reg;
	u32 ndiv_off, nfrac_off;
	u32 ndiv, nfrac;
	int err = 0;

	clk_dbg("%s - %s\n", __func__, clk->name);
	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
/* enable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, true);
	des = pll_clk->desense;
	if (!des || !(des->flags & PLL_OFFSET_EN)) {
		err = -EINVAL;
		goto ret;
	}
	rate = __pll_clk_get_rate(clk);

	off_rate = (int)rate + offset;
	new_rate = compute_pll_vco_div(pll_clk, (u32)off_rate, NULL,
		&ndiv_off, &nfrac_off);
	clk_dbg("%s- compute_pll_vco_div: %u %u %x %x\n", __func__,
		new_rate, (u32)off_rate,
		ndiv_off, nfrac_off);
	if (abs(new_rate - (u32)off_rate) > 100) {
		clk_dbg("%s : %s - rate(%lu) not supported\n",
			__func__, clk->name, rate);
		err = -EINVAL;
		goto ret;
	}

	reg =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_frac_offset));
	reg &= pll_clk->ndiv_frac_mask;
	nfrac = reg >> pll_clk->ndiv_frac_shift;

	reg =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_pdiv_offset));
	reg &= pll_clk->ndiv_int_mask;
	ndiv =  reg >> pll_clk->ndiv_int_shift;

	clk_dbg("%s: ndiv =  %x, frac = %x\n", __func__, ndiv, nfrac);

	reg =  readl(CCU_REG_ADDR(pll_clk->ccu_clk, des->pll_offset_offset));
	if (des->flags & PLL_OFFSET_NDIV) {
		reg &= ~PLL_OFFSET_NDIV_MASK;
		reg |= (ndiv_off << PLL_OFFSET_NDIV_SHIFT) &
				PLL_OFFSET_NDIV_MASK;
	} else if (ndiv != ndiv_off) {
		pr_info("%s - ndiv != ndiv_off. PLL_OFFSET_NDIV not eanbled\n",
			__func__);
		err = -EINVAL;
		goto ret;
	}

	if (des->flags & PLL_OFFSET_NDIV_FRAC) {
		reg &= ~PLL_OFFSET_NDIV_F_MASK;
		reg |= (nfrac_off << PLL_OFFSET_NDIV_F_SHIFT) &
				PLL_OFFSET_NDIV_F_MASK;
	} else if (nfrac_off != nfrac) {
		pr_info("%s - ndiv != ndiv_off. PLL_OFFSET_NDIV not eanbled\n",
			__func__);
		err = -EINVAL;
		goto ret;
	}
	clk_dbg("%s %x written to %x\n", __func__, reg,
		(u32) CCU_REG_ADDR(pll_clk->ccu_clk, des->pll_offset_offset));
	writel(reg, CCU_REG_ADDR(pll_clk->ccu_clk, des->pll_offset_offset));
ret:
	ccu_write_access_enable(pll_clk->ccu_clk, false);
	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);
	return err;
}

int pll_set_desense_offset(struct clk *clk, int offset)
{
	int ret;
	unsigned long flgs;
	clk_lock(clk, &flgs);
	ret = __pll_set_desense_offset(clk, offset);
	clk_unlock(clk, &flgs);
	return ret;
}
EXPORT_SYMBOL(pll_set_desense_offset);

int pll_get_desense_offset(struct clk *clk)
{
	struct pll_clk *pll_clk;
	unsigned long flgs;
	int off_rate = 0;
	u32 pll_rate;
	u32 ndiv_int, nfrac, pdiv;
	u32 frac_div;
	u32 reg;
	u32 off_reg;
	struct pll_desense *des;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);

	clk_lock(clk, &flgs);
	pll_clk = to_pll_clk(clk);
	des = pll_clk->desense;
	if (!des || !(des->flags & PLL_OFFSET_EN))
		goto ret;
	pll_rate = __pll_clk_get_rate(clk);
	off_reg = readl(CCU_REG_ADDR(pll_clk->ccu_clk,
		des->pll_offset_offset));
	if (des->flags & PLL_OFFSET_NDIV_FRAC) {
		nfrac = off_reg & PLL_OFFSET_NDIV_F_MASK;
		nfrac >>= PLL_OFFSET_NDIV_F_SHIFT;
	} else {
		reg =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk,
			pll_clk->ndiv_frac_offset));
		nfrac = (reg & pll_clk->ndiv_frac_mask) >>
					pll_clk->ndiv_frac_shift;
	}
	if (des->flags & PLL_OFFSET_NDIV) {
		ndiv_int = off_reg & PLL_OFFSET_NDIV_MASK;
		ndiv_int >>= PLL_OFFSET_NDIV_SHIFT;
	} else {
		reg =
			readl(CCU_REG_ADDR(pll_clk->ccu_clk,
				pll_clk->ndiv_pdiv_offset));
		ndiv_int =
			(reg & pll_clk->ndiv_int_mask) >>
					pll_clk->ndiv_int_shift;
	}
	reg =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk,
			pll_clk->ndiv_pdiv_offset));
	pdiv = (reg & pll_clk->pdiv_mask) >> pll_clk->pdiv_shift;
	if (pdiv == 0)
		pdiv = pll_clk->pdiv_max;
	if (ndiv_int == 0)
		ndiv_int = pll_clk->ndiv_int_max;

	frac_div = 1 + (pll_clk->ndiv_frac_mask >> pll_clk->ndiv_frac_shift);
	 off_rate = (int)compute_pll_vco_rate(ndiv_int, nfrac,
						frac_div, pdiv) - (int)pll_rate;
ret:
	clk_unlock(clk, &flgs);
	return off_rate;
}
EXPORT_SYMBOL(pll_get_desense_offset);

int pll_desense_enable(struct clk *clk, int enable)
{
	struct pll_clk *pll_clk;
	struct pll_desense *des;
	unsigned long flgs;
	u32 reg;
	int err = 0;

	clk_dbg("%s - %s\n", __func__, clk->name);
	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	clk_lock(clk, &flgs);
/* enable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, true);
	des = pll_clk->desense;
	if (!des || !(des->flags & PLL_OFFSET_EN) ||
		!(des->flags & PLL_OFFSET_SW_MODE)) {
		err = -EINVAL;
		goto ret;
	}
	reg = readl(CCU_REG_ADDR(pll_clk->ccu_clk, des->pll_offset_offset));
	if (enable)
		reg |= PLL_OFFSET_SW_CTRL_MASK;
	else
		reg &= PLL_OFFSET_SW_CTRL_MASK;
	writel(reg,
		CCU_REG_ADDR(pll_clk->ccu_clk, des->pll_offset_offset));
ret:
	ccu_write_access_enable(pll_clk->ccu_clk, false);
	clk_unlock(clk, &flgs);
	return err;
}
EXPORT_SYMBOL(pll_desense_enable);

static unsigned long pll_clk_round_rate(struct clk *clk, unsigned long rate)
{
	u32 new_rate;
	struct pll_clk *pll_clk;

	if (clk->clk_type != CLK_TYPE_PLL)
		return -EPERM;

	pll_clk = to_pll_clk(clk);

	new_rate = compute_pll_vco_div(pll_clk, rate, NULL, NULL, NULL);

	clk_dbg("%s:rate = %d\n", __func__, new_rate);

	return new_rate;
}

static unsigned long pll_clk_get_rate(struct clk *clk)
{
	struct pll_clk *pll_clk;
	u32 reg_val = 0;
	u32 ndiv_int, nfrac, pdiv;
	u32 frac_div;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);

	pll_clk = to_pll_clk(clk);

	if (CLK_FLG_ENABLED(clk, RATE_FIXED)) {
		clk_dbg("%s : %s - fixed rate clk...\n", __func__, clk->name);
		return clk->rate;
	}
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_frac_offset));
	nfrac = (reg_val & pll_clk->ndiv_frac_mask) >> pll_clk->ndiv_frac_shift;

	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_pdiv_offset));
	pdiv = (reg_val & pll_clk->pdiv_mask) >> pll_clk->pdiv_shift;
	ndiv_int =
	    (reg_val & pll_clk->ndiv_int_mask) >> pll_clk->ndiv_int_shift;

	if (pdiv == 0)
		pdiv = pll_clk->pdiv_max;
	if (ndiv_int == 0)
		ndiv_int = pll_clk->ndiv_int_max;

	frac_div = 1 + (pll_clk->ndiv_frac_mask >> pll_clk->ndiv_frac_shift);
	return compute_pll_vco_rate(ndiv_int, nfrac, frac_div, pdiv);
}

static int pll_clk_set_rate(struct clk *clk, u32 rate)
{
	struct pll_clk *pll_clk;
	u32 new_rate, reg_val;
	u32 pll_cfg_ctrl = 0;
	int insurance;
	int ret = 0;
	u32 ndiv_int, nfrac, pdiv;
	int inx;
	struct pll_cfg_ctrl_info *cfg_ctrl;
	if (clk->clk_type != CLK_TYPE_PLL) {
		ret = -EPERM;
		goto err;
	}

	pll_clk = to_pll_clk(clk);

	clk_dbg("%s : %s\n", __func__, clk->name);

	if (CLK_FLG_ENABLED(clk, RATE_FIXED)) {
		clk_dbg("%s : %s - fixed rate...can't be changed\n",
			__func__, clk->name);
		ret = -EINVAL;
		goto err;
	}
	new_rate = compute_pll_vco_div(pll_clk, rate, &pdiv, &ndiv_int, &nfrac);

	if (abs(new_rate - rate) > 100) {
		clk_dbg("%s : %s - rate(%d) not supported\n",
			__func__, clk->name, rate);
		ret = -EINVAL;
		goto err;
	}

	/* enable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, true);

	/*Write pll_cfg_ctrl */
	if (pll_clk->cfg_ctrl_info && pll_clk->cfg_ctrl_info->thold_count) {
		cfg_ctrl = pll_clk->cfg_ctrl_info;
		for (inx = 0; inx < cfg_ctrl->thold_count; inx++) {
			if (cfg_ctrl->vco_thold[inx] == PLL_VCO_RATE_MAX
			    || new_rate < cfg_ctrl->vco_thold[inx]) {
				pll_cfg_ctrl = cfg_ctrl->pll_config_value[inx];
				pll_cfg_ctrl <<= cfg_ctrl->pll_cfg_ctrl_shift;
				pll_cfg_ctrl &= cfg_ctrl->pll_cfg_ctrl_mask;
				break;
			}
		}
		if (inx != pll_clk->cfg_ctrl_info->thold_count) {
			writel(pll_cfg_ctrl,
			       CCU_REG_ADDR(pll_clk->ccu_clk,
					    cfg_ctrl->pll_cfg_ctrl_offset));
		}
	}
	/*Write nfrac */
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_frac_offset));
	reg_val &= ~pll_clk->ndiv_frac_mask;
	reg_val |= nfrac << pll_clk->ndiv_frac_shift;
	writel(reg_val,
	       CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_frac_offset));

	/*write nint & pdiv */
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_pdiv_offset));
	reg_val &= ~(pll_clk->pdiv_mask | pll_clk->ndiv_int_mask);
	reg_val |= (pdiv << pll_clk->pdiv_shift)
	    | (ndiv_int << pll_clk->ndiv_int_shift);
	writel(reg_val,
	       CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_pdiv_offset));

	reg_val =
	    readl(CCU_REG_ADDR
		  (pll_clk->ccu_clk, pll_clk->soft_post_resetb_offset));
	reg_val |= pll_clk->soft_post_resetb_mask;
	writel(reg_val,
	       CCU_REG_ADDR(pll_clk->ccu_clk,
			    pll_clk->soft_post_resetb_offset));

	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->soft_resetb_offset));
	reg_val |= pll_clk->soft_resetb_mask;
	writel(reg_val,
	       CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->soft_resetb_offset));

	/*Loop for lock bit only if the
	   - PLL is AUTO GATED or
	   - PLL is enabled */
	reg_val = readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->pwrdwn_offset));
	if (clk->flags & AUTO_GATE || ((reg_val & pll_clk->pwrdwn_mask) == 0)) {
		insurance = 0;
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (pll_clk->ccu_clk, pll_clk->pll_lock_offset));
			insurance++;
		} while (!(GET_BIT_USING_MASK(reg_val, pll_clk->pll_lock))
			 && insurance < CLK_EN_INS_COUNT);
		if (insurance >= CLK_EN_INS_COUNT &&
				!(clk->flags & DELAYED_PLL_LOCK)) {
			__WARN();
			ret = -EINVAL;
		}
	}

	/* disable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, false);
err:
	if (!ret)
		clk_dbg("clock set rate done\n");
	else
		clk_dbg("clock set rate not done\n");

	return ret;
}

static int pll_clk_enable(struct clk *clk, int enable)
{
	u32 reg_val;
	struct pll_clk *pll_clk;
	int insurance;
	int ret = 0;
	clk_dbg("%s:%d, clock name: %s\n", __func__, enable, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	BUG_ON(!pll_clk->ccu_clk || (pll_clk->pll_ctrl_offset == 0));

	if (clk->flags & AUTO_GATE || !pll_clk->pwrdwn_mask) {
		clk_dbg("%s:%s: is auto gated or no enable bit\n",
				__func__, clk->name);
		goto err;
	}

	/*enable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, true);

	if (pll_clk->idle_pwrdwn_sw_ovrride_mask != 0) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_clk->ccu_clk, pll_clk->pll_ctrl_offset));
		clk_dbg("%s, Before change pll_ctrl reg value: %08x\n",
			__func__, reg_val);
		/*Return if sw_override bit is set */
		if (GET_BIT_USING_MASK
		    (reg_val, pll_clk->idle_pwrdwn_sw_ovrride_mask))
			goto auto_gated;
	}
	if (enable) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_clk->ccu_clk, pll_clk->pwrdwn_offset));
		reg_val &= ~pll_clk->pwrdwn_mask;
		clk_dbg("%s, writing %08x to pwrdwn reg\n", __func__, reg_val);
		writel(reg_val,
		       CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->pwrdwn_offset));

		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_clk->ccu_clk, pll_clk->soft_post_resetb_offset));
		reg_val |= pll_clk->soft_post_resetb_mask;
		clk_dbg("%s, writing %08x to soft_post_resetb reg\n", __func__,
			reg_val);
		writel(reg_val,
		       CCU_REG_ADDR(pll_clk->ccu_clk,
				    pll_clk->soft_post_resetb_offset));

		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_clk->ccu_clk, pll_clk->soft_resetb_offset));
		reg_val |=
		    pll_clk->soft_post_resetb_mask | pll_clk->soft_resetb_mask;
		clk_dbg("%s, writing %08x to soft_resetb reg\n", __func__,
			reg_val);
		writel(reg_val,
		       CCU_REG_ADDR(pll_clk->ccu_clk,
				    pll_clk->soft_resetb_offset));

	} else {
		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_clk->ccu_clk, pll_clk->pwrdwn_offset));
		reg_val = reg_val | pll_clk->pwrdwn_mask;
		clk_dbg("%s, writing %08x to pwrdwn reg\n", __func__, reg_val);
		writel(reg_val,
		       CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->pwrdwn_offset));
	}

	if (enable) {
		insurance = 0;
		do {
			udelay(1);
			reg_val =
			    readl(CCU_REG_ADDR
				  (pll_clk->ccu_clk, pll_clk->pll_lock_offset));
			insurance++;
		} while (!(GET_BIT_USING_MASK(reg_val, pll_clk->pll_lock))
			 && insurance < CLK_EN_INS_COUNT);
		if (insurance >= 1000) {
			__WARN();
			ret = -EINVAL;
		}
	}

auto_gated:
	/* disable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, false);
err:
	if (!ret)
		clk_dbg("****%s: pll clock %s count after %s : %d****\n",
			__func__, clk->name, enable ? "enable" : "disable",
			clk->use_cnt);
	else
		clk_dbg("%s, %s is : Operation Unsuccesful!\n",
				__func__, clk->name);
	return ret;
}

static int pll_clk_init(struct clk *clk)
{
	struct pll_clk *pll_clk;
	u32 reg_val;
	struct pll_desense *des;

	BUG_ON(clk->clk_type != CLK_TYPE_PLL);
	pll_clk = to_pll_clk(clk);

	BUG_ON(pll_clk->ccu_clk == NULL);

	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	/* enable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, true);

	if (pll_clk->idle_pwrdwn_sw_ovrride_mask != 0) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_clk->ccu_clk, pll_clk->pll_ctrl_offset));
		if (clk->flags & AUTO_GATE) {
			reg_val |= pll_clk->idle_pwrdwn_sw_ovrride_mask;
		} else {
			reg_val &= ~pll_clk->idle_pwrdwn_sw_ovrride_mask;
		}
		writel(reg_val,
		       CCU_REG_ADDR(pll_clk->ccu_clk,
				    pll_clk->pll_ctrl_offset));
	}
	des = pll_clk->desense;
	if (des) {

		if (des->flags & PLL_OFFSET_EN) {
			reg_val = readl(CCU_REG_ADDR(pll_clk->ccu_clk,
					des->pll_offset_offset));
			if (des->flags & PLL_OFFSET_SW_MODE)
				reg_val |= PLL_OFFSET_MODE_MASK;
			else
				reg_val &= PLL_OFFSET_MODE_MASK;
			writel(reg_val,
				CCU_REG_ADDR(pll_clk->ccu_clk,
						des->pll_offset_offset));
			__pll_set_desense_offset(clk, des->def_delta);
		} else {
			reg_val = PLL_OFFSET_MODE_MASK;
			writel(reg_val,
				CCU_REG_ADDR(pll_clk->ccu_clk,
					des->pll_offset_offset));
		}
	}
	/* Disable write access */
	ccu_write_access_enable(pll_clk->ccu_clk, false);

	clk_dbg
	    ("*************%s: peri clock %s count after init %d **************\n",
	     __func__, clk->name, clk->use_cnt);

	return 0;
}

int pll_clk_get_lock_status(struct pll_clk *pll_clk)
{
	u32 reg_val;

	BUG_ON(!pll_clk->ccu_clk);
	if (!pll_clk->pll_lock_offset || !pll_clk->pll_lock)
		return -EINVAL;
	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->pll_lock_offset));
	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, pll_clk->pll_lock);
}

int pll_clk_get_pdiv(struct pll_clk *pll_clk)
{
	u32 reg_val;

	BUG_ON(!pll_clk->ccu_clk);
	if (!pll_clk->ndiv_pdiv_offset || !pll_clk->pdiv_mask)
		return -EINVAL;
	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_pdiv_offset));
	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return GET_VAL_USING_MASK_SHIFT(reg_val, pll_clk->pdiv_mask,
					pll_clk->pdiv_shift);
}

int pll_clk_get_ndiv_int(struct pll_clk *pll_clk)
{
	u32 reg_val;

	BUG_ON(!pll_clk->ccu_clk);
	if (!pll_clk->ndiv_pdiv_offset || !pll_clk->ndiv_int_mask)
		return -EINVAL;
	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_pdiv_offset));
	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return GET_VAL_USING_MASK_SHIFT(reg_val, pll_clk->ndiv_int_mask,
					pll_clk->ndiv_int_shift);
}

int pll_clk_get_ndiv_frac(struct pll_clk *pll_clk)
{
	u32 reg_val;

	BUG_ON(!pll_clk->ccu_clk);
	if (!pll_clk->ndiv_frac_offset || !pll_clk->ndiv_frac_mask)
		return -EINVAL;
	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->ndiv_frac_offset));
	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return GET_VAL_USING_MASK_SHIFT(reg_val, pll_clk->ndiv_frac_mask,
					pll_clk->ndiv_frac_shift);
}

int pll_clk_get_idle_pwrdwn_sw_ovrride(struct pll_clk *pll_clk)
{
	u32 reg_val;

	BUG_ON(!pll_clk->ccu_clk);
	if (!pll_clk->pll_ctrl_offset || !pll_clk->idle_pwrdwn_sw_ovrride_mask)
		return -EINVAL;
	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->pll_ctrl_offset));
	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val,
				  pll_clk->idle_pwrdwn_sw_ovrride_mask);
}

int pll_clk_get_pwrdwn(struct pll_clk *pll_clk)
{
	u32 reg_val;

	BUG_ON(!pll_clk->ccu_clk);
	if (!pll_clk->pwrdwn_offset || !pll_clk->pwrdwn_mask)
		return -EINVAL;
	CCU_ACCESS_EN(pll_clk->ccu_clk, 1);
	reg_val = readl(CCU_REG_ADDR(pll_clk->ccu_clk, pll_clk->pwrdwn_offset));
	CCU_ACCESS_EN(pll_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, pll_clk->pwrdwn_mask);
}

struct gen_clk_ops gen_pll_clk_ops = {
	.init = pll_clk_init,
	.enable = pll_clk_enable,
	.set_rate = pll_clk_set_rate,
	.get_rate = pll_clk_get_rate,
	.round_rate = pll_clk_round_rate,
};

int pll_chnl_clk_enable(struct clk *clk, int enable)
{
	u32 reg_val;
	struct pll_chnl_clk *pll_chnl_clk;
	clk_dbg("%s:%d, clock name: %s\n", __func__, enable, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);

	/*enable write access */
	ccu_write_access_enable(pll_chnl_clk->ccu_clk, true);

	if (enable) {
		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_chnl_clk->ccu_clk,
			   pll_chnl_clk->pll_enableb_offset));
		reg_val &= ~pll_chnl_clk->out_en_mask;
		clk_dbg("%s, writing %08x to pll_enableb_offset reg\n",
			__func__, reg_val);
		writel(reg_val,
		       CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				    pll_chnl_clk->pll_enableb_offset));

		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_chnl_clk->ccu_clk,
			   pll_chnl_clk->pll_load_ch_en_offset));
		reg_val |= pll_chnl_clk->load_en_mask;
		clk_dbg("%s, writing %08x to pll_load_ch_en_offset reg\n",
			__func__, reg_val);
		writel(reg_val,
		       CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				    pll_chnl_clk->pll_load_ch_en_offset));

	} else {
		reg_val =
		    readl(CCU_REG_ADDR
			  (pll_chnl_clk->ccu_clk,
			   pll_chnl_clk->pll_enableb_offset));
		reg_val = reg_val | pll_chnl_clk->out_en_mask;
		clk_dbg("%s, writing %08x to pll_enableb_offset reg\n",
			__func__, reg_val);
		writel(reg_val,
		       CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				    pll_chnl_clk->pll_enableb_offset));
	}

	/* disable write access */
	ccu_write_access_enable(pll_chnl_clk->ccu_clk, false);

	clk_dbg
	    ("*************%s: pll channrl clock %s count after %s : %d ***************\n",
	     __func__, clk->name, enable ? "enable" : "disable", clk->use_cnt);

	return 0;
}

static unsigned long pll_chnl_clk_round_rate(struct clk *clk,
					     unsigned long rate)
{
	u32 new_rate;
	u32 vco_rate;
	u32 mdiv;
	struct pll_chnl_clk *pll_chnl_clk;

	if (clk->clk_type != CLK_TYPE_PLL_CHNL)
		return 0;

	pll_chnl_clk = to_pll_chnl_clk(clk);

	vco_rate = __pll_clk_get_rate(&pll_chnl_clk->pll_clk->clk);

	if (vco_rate < rate || rate == 0)
		return 0;

	mdiv = vco_rate / rate;

	if (mdiv > pll_chnl_clk->mdiv_max)
		mdiv = pll_chnl_clk->mdiv_max;
	new_rate = vco_rate / mdiv;

	return new_rate;
}

static unsigned long pll_chnl_clk_get_rate(struct clk *clk)
{
	struct pll_chnl_clk *pll_chnl_clk;
	u32 mdiv;
	u32 reg_val;
	u32 vco_rate;
	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);

	pll_chnl_clk = to_pll_chnl_clk(clk);

	reg_val =
	    readl(CCU_REG_ADDR
		  (pll_chnl_clk->ccu_clk, pll_chnl_clk->cfg_reg_offset));
	mdiv = (reg_val & pll_chnl_clk->mdiv_mask) >> pll_chnl_clk->mdiv_shift;
	if (mdiv == 0)
		mdiv = pll_chnl_clk->mdiv_max;
	vco_rate = __pll_clk_get_rate(&pll_chnl_clk->pll_clk->clk);
	return (vco_rate / mdiv);
}

static int pll_chnl_clk_set_rate(struct clk *clk, u32 rate)
{
	u32 reg_val;
	u32 vco_rate;
	u32 mdiv;
	struct pll_chnl_clk *pll_chnl_clk;

	if (clk->clk_type != CLK_TYPE_PLL_CHNL)
		return 0;

	clk_dbg("%s : %s\n", __func__, clk->name);

	pll_chnl_clk = to_pll_chnl_clk(clk);

	vco_rate = __pll_clk_get_rate(&pll_chnl_clk->pll_clk->clk);

	if (vco_rate < rate || rate == 0) {
		clk_dbg("%s : invalid rate : %d\n", __func__, rate);
		return -EINVAL;
	}

	mdiv = vco_rate / rate;

	if (mdiv == 0)
		mdiv++;
	if (abs(rate - vco_rate / mdiv) > abs(rate - vco_rate / (mdiv + 1)))
		mdiv++;
	if (mdiv > pll_chnl_clk->mdiv_max)
		mdiv = pll_chnl_clk->mdiv_max;

	if (abs(rate - vco_rate / mdiv) > 100) {
		clk_dbg("%s : invalid rate : %d\n", __func__, rate);
		return -EINVAL;
	}

	/* enable write access */
	ccu_write_access_enable(pll_chnl_clk->ccu_clk, true);

	/*Write mdiv */
	if (mdiv == pll_chnl_clk->mdiv_max)
		mdiv = 0;

	reg_val =
	    readl(CCU_REG_ADDR
		  (pll_chnl_clk->ccu_clk, pll_chnl_clk->cfg_reg_offset));
	reg_val &= ~pll_chnl_clk->mdiv_mask;
	reg_val |= mdiv << pll_chnl_clk->mdiv_shift;
	writel(reg_val,
	       CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
			    pll_chnl_clk->cfg_reg_offset));

/* Write the mdiv value, clear and set load_en_mask(trigger) */
	reg_val = readl(CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				pll_chnl_clk->pll_load_ch_en_offset));
	reg_val &= ~pll_chnl_clk->load_en_mask;
	writel(reg_val, CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
			    pll_chnl_clk->pll_load_ch_en_offset));

	reg_val =
	    readl(CCU_REG_ADDR
		  (pll_chnl_clk->ccu_clk, pll_chnl_clk->pll_load_ch_en_offset));
	reg_val |= pll_chnl_clk->load_en_mask;
	writel(reg_val,
	       CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
			    pll_chnl_clk->pll_load_ch_en_offset));

	/* disable write access */
	ccu_write_access_enable(pll_chnl_clk->ccu_clk, false);

	clk_dbg("clock set rate done\n");
	return 0;
}

static int pll_chnl_clk_init(struct clk *clk)
{
	struct pll_chnl_clk *pll_chnl_clk;
	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_PLL_CHNL);
	pll_chnl_clk = to_pll_chnl_clk(clk);

	BUG_ON(!pll_chnl_clk->ccu_clk ||
	       !pll_chnl_clk->pll_clk || !pll_chnl_clk->cfg_reg_offset);
	return 0;
}

int pll_chnl_clk_get_mdiv(struct pll_chnl_clk *pll_chnl_clk)
{
	u32 reg_val;

	BUG_ON(!pll_chnl_clk->ccu_clk);
	if (!pll_chnl_clk->cfg_reg_offset || !pll_chnl_clk->mdiv_mask)
		return -EINVAL;
	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);
	reg_val = readl(CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				     pll_chnl_clk->cfg_reg_offset));
	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);

	return GET_VAL_USING_MASK_SHIFT(reg_val, pll_chnl_clk->mdiv_mask,
					pll_chnl_clk->mdiv_shift);
}

int pll_chnl_clk_get_enb_clkout(struct pll_chnl_clk *pll_chnl_clk)
{
	u32 reg_val;

	BUG_ON(!pll_chnl_clk->ccu_clk);
	if (!pll_chnl_clk->pll_enableb_offset || !pll_chnl_clk->out_en_mask)
		return -EINVAL;
	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 1);
	reg_val = readl(CCU_REG_ADDR(pll_chnl_clk->ccu_clk,
				     pll_chnl_clk->pll_enableb_offset));
	CCU_ACCESS_EN(pll_chnl_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, pll_chnl_clk->out_en_mask);
}

struct gen_clk_ops gen_pll_chnl_clk_ops = {
	.init = pll_chnl_clk_init,
	.enable = pll_chnl_clk_enable,
	.set_rate = pll_chnl_clk_set_rate,
	.get_rate = pll_chnl_clk_get_rate,
	.round_rate = pll_chnl_clk_round_rate,
};

static unsigned long core_clk_get_rate(struct clk *clk)
{
	struct core_clk *core_clk;
	u32 freq_id;
	u32 pll_chnl;
	BUG_ON(clk->clk_type != CLK_TYPE_CORE);

	core_clk = to_core_clk(clk);
	 /**/
	    freq_id =
	    ccu_get_freq_policy(core_clk->ccu_clk, core_clk->active_policy);

	if (freq_id < core_clk->num_pre_def_freq) {
		return core_clk->pre_def_freq[freq_id];
	}
	pll_chnl = freq_id - core_clk->num_pre_def_freq;

	BUG_ON(pll_chnl >= core_clk->num_chnls);

	return __pll_chnl_clk_get_rate(&core_clk->pll_chnl_clk[pll_chnl]->clk);
}

/**
 * core_clk_freq_scale - "old * mult / div" calculation for large values (32-bit-arch safe)
 * @old:   old value
 * @div:   divisor
 * @mult:  multiplier
 *
 *
 *    new = old * mult / div
 */
static inline unsigned long core_clk_freq_scale(unsigned long old, u_int div,
						u_int mult)
{
#if BITS_PER_LONG == 32

	u64 result = ((u64)old) * ((u64)mult);
	do_div(result, div);
	return (unsigned long)result;

#elif BITS_PER_LONG == 64

	unsigned long result = old * ((u64)mult);
	result /= div;
	return result;

#endif
};

static int core_clk_set_rate(struct clk *clk, u32 rate)
{
	u32 vco_rate;
	u32 div;
	struct core_clk *core_clk;
	static u32 l_p_j_ref = 0;
	static u32 l_p_j_ref_freq = 0;
	int ret;

	if (clk->clk_type != CLK_TYPE_CORE || rate == 0)
		return -EINVAL;

	clk_dbg("%s : %s\n", __func__, clk->name);

	core_clk = to_core_clk(clk);
	vco_rate = __pll_clk_get_rate(&core_clk->pll_clk->clk);
	div = vco_rate / rate;
	if (l_p_j_ref == 0 && (clk->flags & UPDATE_LPJ)) {
		BUG_ON(!clk->ops->get_rate);
#ifdef CONFIG_SMP
		l_p_j_ref = per_cpu(cpu_data, 0).loops_per_jiffy;
#else
		l_p_j_ref = loops_per_jiffy;
#endif
		l_p_j_ref_freq = clk->ops->get_rate(clk) / 1000;
		clk_dbg("l_p_j_ref = %d, l_p_j_ref_freq = %x\n", l_p_j_ref,
			l_p_j_ref_freq);
	}

	if (div < 2 || rate * div != vco_rate) {
		__pll_clk_set_rate(&core_clk->pll_clk->clk, rate * 2);
		vco_rate = __pll_clk_get_rate(&core_clk->pll_clk->clk);
		div = vco_rate / rate;
	}

	ret =
	    __pll_chnl_clk_set_rate(&core_clk->
				    pll_chnl_clk[core_clk->num_chnls - 1]->clk,
				    rate);
	if (!ret && (clk->flags & UPDATE_LPJ)) {
#ifdef CONFIG_SMP
		int i;
		for_each_online_cpu(i) {
			per_cpu(cpu_data, i).loops_per_jiffy =
			    core_clk_freq_scale(l_p_j_ref, l_p_j_ref_freq,
						rate / 1000);
		}
#endif
		loops_per_jiffy = core_clk_freq_scale(l_p_j_ref,
						      l_p_j_ref_freq,
						      rate / 1000);
		clk_dbg("loops_per_jiffy = %lu, rate = %u\n", loops_per_jiffy,
			rate);
	}
	return ret;
}

static int core_clk_init(struct clk *clk)
{
	struct core_clk *core_clk;
	clk_dbg("%s, clock name: %s\n", __func__, clk->name);

	BUG_ON(clk->clk_type != CLK_TYPE_CORE);
	core_clk = to_core_clk(clk);

	BUG_ON(!core_clk->ccu_clk ||
	       !core_clk->pll_clk || !core_clk->num_chnls);
	INIT_LIST_HEAD(&clk->list);
	list_add(&clk->list, &core_clk->ccu_clk->clk_list);

	return 0;
}

static int core_clk_reset(struct clk *clk)
{
	u32 reg_val;
	struct core_clk *core_clk;

	if (clk->clk_type != CLK_TYPE_CORE) {
		BUG_ON(1);
		return -EPERM;
	}
	core_clk = to_core_clk(clk);
	clk_dbg("%s -- %s to be reset\n", __func__, clk->name);

	BUG_ON(!core_clk->ccu_clk);
	if (!core_clk->soft_reset_offset || !core_clk->clk_reset_mask)
		return -EPERM;

	/* enable write access */
	ccu_reset_write_access_enable(core_clk->ccu_clk, true);

	reg_val =
	    readl(core_clk->ccu_clk->ccu_reset_mgr_base +
		  core_clk->soft_reset_offset);
	clk_dbg("reset offset: %08x, reg_val: %08x\n",
		(u32) (core_clk->ccu_clk->ccu_reset_mgr_base +
		 core_clk->soft_reset_offset), reg_val);
	reg_val = reg_val & ~core_clk->clk_reset_mask;
	clk_dbg("writing reset value: %08x\n", reg_val);
	writel(reg_val,
	       core_clk->ccu_clk->ccu_reset_mgr_base +
	       core_clk->soft_reset_offset);
	//core should have reset here. below code wont be executed.
	udelay(10);

	reg_val = reg_val | core_clk->clk_reset_mask;
	clk_dbg("writing reset release value: %08x\n", reg_val);

	writel(reg_val,
	       core_clk->ccu_clk->ccu_reset_mgr_base +
	       core_clk->soft_reset_offset);

	ccu_reset_write_access_enable(core_clk->ccu_clk, false);

	return 0;
}

int core_clk_get_gating_status(struct core_clk *core_clk)
{
	u32 reg_val;

	BUG_ON(!core_clk->ccu_clk);
	if (!core_clk->clk_gate_offset || !core_clk->stprsts_mask)
		return -EINVAL;
	CCU_ACCESS_EN(core_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(core_clk->ccu_clk, core_clk->clk_gate_offset));
	CCU_ACCESS_EN(core_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, core_clk->stprsts_mask);
}

int core_clk_get_gating_ctrl(struct core_clk *core_clk)
{
	u32 reg_val;

	if (!core_clk->clk_gate_offset || !core_clk->gating_sel_mask)
		return -EINVAL;

	BUG_ON(!core_clk->ccu_clk);
	CCU_ACCESS_EN(core_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(core_clk->ccu_clk, core_clk->clk_gate_offset));
	CCU_ACCESS_EN(core_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, core_clk->gating_sel_mask);
}

int core_clk_get_enable_bit(struct core_clk *core_clk)
{
	u32 reg_val;

	BUG_ON(!core_clk->ccu_clk);
	if (!core_clk->clk_gate_offset || !core_clk->clk_en_mask)
		return -EINVAL;
	CCU_ACCESS_EN(core_clk->ccu_clk, 1);
	reg_val =
	    readl(CCU_REG_ADDR(core_clk->ccu_clk, core_clk->clk_gate_offset));
	CCU_ACCESS_EN(core_clk->ccu_clk, 0);

	return GET_BIT_USING_MASK(reg_val, core_clk->clk_en_mask);
}

struct gen_clk_ops gen_core_clk_ops = {
	.init = core_clk_init,
	.set_rate = core_clk_set_rate,
	.get_rate = core_clk_get_rate,
	.reset = core_clk_reset,
};

int __init clk_trace_init(unsigned int count)
{
#ifdef CONFIG_KONA_CLK_TRACE
	void *virt;

	virt = dma_zalloc_coherent(NULL, count * sizeof(u32),
						&clk_trace_p, GFP_ATOMIC);
	if (!virt) {
		pr_info("%s: dma allocation failed\n", __func__);
		return -ENOMEM;
	}
	clk_trace_v = (u32 *)virt;
	pr_info("%s: virtual: %p physical: %#lx",
		__func__, clk_trace_v, (long)clk_trace_p);
#endif
	return 0;
}
