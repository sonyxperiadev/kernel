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
	int					(*enable)(struct clk *c, int enable);
	int                 (*set_rate)(struct clk *c, unsigned long rate);
	unsigned long       (*get_rate)(struct clk *c);
	unsigned long       (*round_rate)(struct clk *c, unsigned long rate);
	int                 (*set_parent)(struct clk *c, struct clk *parent);
};

/**
 * struct clk_src - clock source
 * @total: number of clock sources, 0 means root clock, no parent
 * @parents: array of parents - source
 */
struct clk_src {
	int			total;
	struct clk 	**parents;
};

struct clk {
	struct list_head	list;
	struct module		*owner;
	struct clk			*parent;
	const char			*name;
	int					id;
	int					use_cnt;

	unsigned long		rate;		/* in HZ */
	unsigned long		div;		/* programmable divider. 0 means fixed ratio to parent clock */

	struct clk_src		*src;
	struct clk_ops		*ops;
};

struct proc_clock {
	struct clk		clk;
	unsigned long	proc_clk_mgr_base;
};

struct peri_clock {
	struct clk		clk;
};

struct ccu_clock {
	struct clk	clk;
};

struct ref_clock {
	struct clk	clk;
};

static inline int is_same_clock(struct clk *a, struct clk *b)
{
	return a==b;
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

static inline struct ref_clock *to_ref_clk(struct clk *clock)
{
	return container_of(clock, struct ref_clock, clk);
}

extern struct clk_ops proc_clk_ops;
extern struct clk_ops peri_clk_ops;
extern struct clk_ops ccu_clk_ops;
extern struct clk_ops ref_clk_ops;

#ifdef CONFIG_DEBUG_FS
int clock_debug_init(void);
int clock_debug_add_clock(struct clk *c);
#else
#define	clock_debug_init() do {} while(0)
#define	clock_debug_add_clock(clk) do {} while(0)
#endif

int __init clock_init(void);
int __init clock_late_init(void);

#define	CLK_WR_ACCESS_PASSWORD	0x00A5A501
#define	CLOCK_1K				1000
#define	CLOCK_1M				(CLOCK_1K * 1000)

#if defined(DEBUG)
#define	clk_dbg printk
#else
#define clk_dbg(format...) do{} while(0)
#endif
