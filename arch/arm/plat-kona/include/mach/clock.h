/*****************************************************************************
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

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <asm/clkdev.h>

struct clk;

/**
 * struct clk_ops - standard clock operations
 * @enable: enable/disable clock, see clk_enable() and clk_disable()
 * @set_rate: set the clock rate, see clk_set_rate().
 * @get_rate: get the clock rate, see clk_get_rate().
 * @round_rate: round a given clock rate, see clk_round_rate().
 * @set_parent: set the clock's parent, see clk_set_parent().
 *
 * Group the common clock implementations together so that we
 * don't have to keep setting the same fiels again. We leave
 * enable in struct clk.
 *
 */
struct clk_ops {
	int		(*enable)(struct clk *c, int enable);
	int		(*set_rate)(struct clk *c, unsigned long rate);
	unsigned long	(*get_rate)(struct clk *c);
	unsigned long	(*round_rate)(struct clk *c, unsigned long rate);
	int		(*set_parent)(struct clk *c, struct clk *parent);
};

/**
 * struct clk_src - clock source
 * @total: number of clock sources, 0 means root clock, no parent
 * @parents: array of parents - source
 */
struct clk_src {
	int		total;
	int		sel;
	struct clk 	**parents;
};

struct clk {
	struct list_head	list;
	struct module	*owner;
	struct clk	*parent;
	const char	*name;
	int		id;
	int		use_cnt;

	unsigned long	rate;		/* in HZ */
	unsigned long	div;		/* programmable divider. 0 means fixed ratio to parent clock */

	struct clk_src	*src;
	struct clk_ops	*ops;
};

struct proc_clock {
	struct clk	clk;
	unsigned long	proc_clk_mgr_base;
};

struct peri_clock {
	struct clk	clk;

	unsigned long	ccu_clk_mgr_base;
	unsigned long	wr_access_offset;
	unsigned long	clkgate_offset;
	unsigned long	div_offset;
	unsigned long	div_trig_offset;

	unsigned long	stprsts_mask;
	unsigned long	hw_sw_gating_mask;
	unsigned long	clk_en_mask;
	unsigned long	div_mask;
	int		div_shift;
	int		div_dithering;		/* dithering franctional bit(s) */
	unsigned long	pll_select_mask;
	int		pll_select_shift;
	unsigned long	trigger_mask;
};

struct ccu_clock {
	struct clk	clk;

	unsigned long	ccu_clk_mgr_base;
	unsigned long	wr_access_offset;
	unsigned long	policy_freq_offset;
	int		freq_bit_shift;		/* 8 for most CCU. MM in Rhea is special with 3 */
	unsigned long	policy_ctl_offset;
	unsigned long	policy0_mask_offset;
	unsigned long	policy1_mask_offset;
	unsigned long	policy2_mask_offset;
	unsigned long	policy3_mask_offset;
	unsigned long	lvm_en_offset;

	int		freq_id;
	unsigned long	freq_tbl[8];
};

struct bus_clock {
	struct clk	clk;

	unsigned long	ccu_clk_mgr_base;
	unsigned long	wr_access_offset;
	unsigned long	clkgate_offset;

	unsigned long	stprsts_mask;
	unsigned long	hw_sw_gating_mask;
	unsigned long	clk_en_mask;

	unsigned long	freq_tbl[8];
};

struct ref_clock {
	struct clk	clk;
};

static inline int is_same_clock(struct clk *a, struct clk *b)
{
	return (a==b);
}

#define	to_clk(p) (&((p)->clk))
#define	name_to_clk(name) (&((name##_clk).clk))
/* declare a struct clk_lookup */
#define	CLK_LK(name) {.con_id=__stringify(name##_clk), .clk=name_to_clk(name),}

static inline struct proc_clock *to_proc_clk(struct clk *clock)
{
	return container_of(clock, struct proc_clock, clk);
}

static inline struct peri_clock *to_peri_clk(struct clk *clock)
{
	return container_of(clock, struct peri_clock, clk);
}

static inline struct ccu_clock *to_ccu_clk(struct clk *clock)
{
	return container_of(clock, struct ccu_clock, clk);
}

static inline struct bus_clock *to_bus_clk(struct clk *clock)
{
	return container_of(clock, struct bus_clock, clk);
}

static inline struct ref_clock *to_ref_clk(struct clk *clock)
{
	return container_of(clock, struct ref_clock, clk);
}

extern struct clk_ops proc_clk_ops;
extern struct clk_ops peri_clk_ops;
extern struct clk_ops ccu_clk_ops;
extern struct clk_ops bus_clk_ops;
extern struct clk_ops ref_clk_ops;

extern int clk_debug;

#ifdef CONFIG_DEBUG_FS
int clock_debug_init(void);
int clock_debug_add_clock(struct clk *c);
#else
#define	clock_debug_init() do {} while(0)
#define	clock_debug_add_clock(clk) do {} while(0)
#endif

int __init clock_init(void);
int __init clock_late_init(void);

unsigned long clock_get_xtal(void);

#define	CLK_WR_ACCESS_PASSWORD	0x00A5A501
#define	CLOCK_1K		1000
#define	CLOCK_1M		(CLOCK_1K * 1000)

#if defined(DEBUG)
#define	clk_dbg printk
#else
#define	clk_dbg(format...)		\
	do {				\
		if (clk_debug) 		\
			printk(format);	\
	} while(0)
#endif

/* declare a reference clock */
#define	DECLARE_REF_CLK(clk_name, clk_rate, clk_div, clk_parent)		\
	static struct proc_clock clk_name##_clk = {				\
		.clk	=	{						\
			.name	=	__stringify(clk_name##_clk),		\
			.parent	=	clk_parent,				\
			.rate	=	clk_rate,				\
			.div	=	clk_div,				\
			.id	=	-1,					\
			.ops	=	&ref_clk_ops,				\
		},								\
	}

/* declare c CCU clock */
#define	DECLARE_CCU_CLK(clk_name, id, ccu, pfx, ... )						\
	static struct ccu_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.ops	=	&ccu_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.policy_freq_offset	=	pfx##_CLK_MGR_REG_POLICY_FREQ_OFFSET,		\
		.freq_bit_shift 	=	pfx##_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT,	\
		.policy_ctl_offset	=	pfx##_CLK_MGR_REG_POLICY_CTL_OFFSET,		\
		.policy0_mask_offset	=	pfx##_CLK_MGR_REG_POLICY0_MASK_OFFSET,		\
		.policy1_mask_offset	=	pfx##_CLK_MGR_REG_POLICY1_MASK_OFFSET,		\
		.policy2_mask_offset	=	pfx##_CLK_MGR_REG_POLICY2_MASK_OFFSET,		\
		.policy3_mask_offset	=	pfx##_CLK_MGR_REG_POLICY3_MASK_OFFSET,		\
		.lvm_en_offset		=	pfx##_CLK_MGR_REG_LVM_EN_OFFSET,		\
		.freq_id	=	id,							\
		.freq_tbl	=	{__VA_ARGS__},						\
	}

/* declare a bus clock*/
#define	DECLARE_BUS_CLK(clk_name, NAME1, NAME2, clk_parent, ccu, pfx, ... )			\
	static struct bus_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.ops	=	&bus_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,	\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_HW_SW_GATING_SEL_SHIFT,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,	\
		.freq_tbl	=	{	__VA_ARGS__	},				\
	}

/* declare a bus clock no H/W S/W gating control */
#define	DECLARE_BUS_CLK_NO_GATING(clk_name, NAME1, NAME2, clk_parent, ccu, pfx, ... )		\
	static struct bus_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.ops	=	&bus_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_OFFSET,	\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_STPRSTS_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##NAME1##_CLKGATE_##NAME2##_CLK_EN_MASK,	\
		.freq_tbl	=	{	__VA_ARGS__	},				\
	}

/* declare a peripheral clock */
#define	DECLARE_PERI_CLK(clk_name, CLK_NAME, clk_parent, clk_rate, clk_div, ccu, pfx, dthr)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	clk_div,						\
			.id	=	-1,							\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_DIV_TRIG_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_CLK_EN_MASK,		\
		.div_mask		=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_DIV_MASK,		\
		.div_shift		=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_DIV_SHIFT,		\
		.div_dithering		=	dthr,									\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_DIV_TRIG_##CLK_NAME##_TRIGGER_MASK,	\
	}

/* declare a peripheral clock without divider */
#define	DECLARE_PERI_CLK_NO_DIV(clk_name, CLK_NAME, clk_parent, clk_rate, clk_div, ccu, pfx)	\
	static struct peri_clock clk_name##_clk = {						\
		.clk	=	{								\
			.name	=	__stringify(clk_name##_clk),				\
			.parent =	name_to_clk(clk_parent),				\
			.rate	=	clk_rate,						\
			.div	=	clk_div,						\
			.id	=	-1,							\
			.src	=	&clk_name##_clk_src,					\
			.ops	=	&peri_clk_ops,						\
		},										\
		.ccu_clk_mgr_base	=	ccu##_CLK_BASE_ADDR,				\
		.wr_access_offset	=	pfx##_CLK_MGR_REG_WR_ACCESS_OFFSET,		\
		.clkgate_offset 	=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_OFFSET,	\
		.div_offset		=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_OFFSET,	\
		.div_trig_offset	=	pfx##_CLK_MGR_REG_DIV_TRIG_OFFSET,		\
		.stprsts_mask		=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_STPRSTS_MASK,		\
		.hw_sw_gating_mask	=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_HW_SW_GATING_SEL_MASK,	\
		.clk_en_mask		=	pfx##_CLK_MGR_REG_##CLK_NAME##_CLKGATE_##CLK_NAME##_CLK_EN_MASK,		\
		.pll_select_mask	=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_PLL_SELECT_MASK,	\
		.pll_select_shift	=	pfx##_CLK_MGR_REG_##CLK_NAME##_DIV_##CLK_NAME##_PLL_SELECT_SHIFT,	\
		.trigger_mask		=	pfx##_CLK_MGR_REG_DIV_TRIG_##CLK_NAME##_TRIGGER_MASK,		\
	}
