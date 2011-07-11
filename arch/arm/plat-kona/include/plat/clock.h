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
#ifndef __ARM_ARCH_KONA_CLOCK_H
#define __ARM_ARCH_KONA_CLOCK_H

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <asm/clkdev.h>
#include <linux/list.h>
#include <mach/clock.h>

#define GET_BIT_USING_MASK(reg_val, mask)	(!!((reg_val) & (mask)))
#define SET_BIT_USING_MASK(reg_val, mask)	((reg_val) | (mask))
#define RESET_BIT_USING_MASK(reg_val, mask)	((reg_val) & ~(mask))
#define GET_VAL_USING_MASK_SHIFT(reg_val, mask, shift)	(((reg_val) & (mask)) >> (shift))
#define SET_VAL_USING_MASK_SHIFT(reg_val, mask, shift, val)	(((reg_val) & ~(mask)) | (((val) << (shift)) & (mask)))

#define IS_REGBITS_VALID(reg_bits) ((reg_bits).reg_bit_mask != 0)

#define SET_BITS_FROM_REGBITS(reg_bits,reg_val,val) (((reg_val) & ~((reg_bits).reg_bit_mask)) \
	| (((val) << (reg_bits).reg_bit_pos) & (reg_bits).reg_bit_mask) )


#define GET_BITS_FROM_REGBITS(reg_bits,reg_val)	 (((reg_val) &  \
		(reg_bits).reg_bit_mask) >> (reg_bits).reg_bit_pos)

#define GET_PERI_SRC_CLK(clock)		((clock)->src_clk.clk[(clock)->src_clk.src_inx])
#define PERI_SRC_CLK_INX(clock)		((clock)->src_clk.src_inx)
#define PERI_SRC_CLK_COUNT(clock)	((clock)->src_clk.count)
#define PERI_SRC_CLK_VALID(clock)	((clock)->src_clk.count)

#define	CLOCK_1K		1000
#define	CLOCK_1M		(CLOCK_1K * 1000)

#define	MAX_DEP_CLKS			4
#define	CLK_RATE_MAX_DIFF		99
#define	MAX_CCU_FREQ_COUNT		8
#define	MAX_CCU_POLICY_COUNT 		4
#define	MAX_CCU_PERI_VLT_COUNT		2

#define CLK_WR_PASSWORD_SHIFT	8
#define CLK_WR_ACCESS_EN	1

#define CCU_POLICY_CONFIG_EN_SHIFT	0
#define CCU_POLICY_CONFIG_EN_MASK	0x00000001
#define CCU_POLICY_OP_EN			1
#define CCU_POLICY_OP_OK			0

#define CCU_POLICY_CTL_GO_SHIFT		 0
#define CCU_POLICY_CTL_GO_AC_SHIFT	 1
#define CCU_POLICY_CTL_GO_ATL_SHIFT	 2
#define CCU_POLICY_CTL_TGT_VLD_SHIFT 3

#define CCU_POLICY_CTL_GO_MASK		0x00000001
#define CCU_POLICY_CTL_GO_AC_MASK	0x00000002
#define CCU_POLICY_CTL_GO_ATL_MASK	0x00000004
#define CCU_POLICY_CTL_TGT_VLD_MASK 	0x00000008

#define CCU_POLICY_CTL_GO_TRIG			1
#define CCU_POLICY_CTL_ATL			1
#define CCU_POLICY_CTL_TGT_VLD			1
#define CCU_POLICY_RESUME_OK			0
#define CCU_POLICY_TGT_VLD_CLR			1

#define CCU_FREQ_POLICY0_SHIFT	0
#define CCU_FREQ_POLICY1_SHIFT	3
#define CCU_FREQ_POLICY2_SHIFT	6
#define CCU_FREQ_POLICY3_SHIFT	9
#define CCU_FREQ_POLICY_MASK	7

#define CCU_ACT_INT_SHIFT	1
#define CCU_TGT_INT_SHIFT	0
#define CCU_INT_EN			1
#define CCU_INT_STATUS_CLR	1

#define CCU_PERI_VLT_NORM_SHIFT  0
#define CCU_PERI_VLT_HIGH_SHIFT  8
#define CCU_PERI_VLT_MASK		  0xF

#define CCU_VLT0_SHIFT	0
#define CCU_VLT1_SHIFT	8
#define CCU_VLT2_SHIFT	16
#define CCU_VLT3_SHIFT	24
#define CCU_VLT4_SHIFT	0
#define CCU_VLT5_SHIFT	8
#define CCU_VLT6_SHIFT	16
#define CCU_VLT7_SHIFT	24
#define CCU_VLT_MASK	0xF


#define CLK_OFF_DELAY_EN	1

#define CLK_DIV_TRIGGER		1
#define CLK_DIV_TRIG_OK		0

#define CCU_POLICY_MASK_ENABLE_ALL_MASK	0x7FFFFFFF

#define CCU_REG_ADDR(ccu,offset)	((u32)(ccu->ccu_clk_mgr_base + offset))
#define CCU_WR_ACCESS_REG(ccu)		CCU_REG_ADDR(ccu, ccu->wr_access_offset)
#define CCU_LVM_EN_REG(ccu)		CCU_REG_ADDR(ccu, ccu->lvm_en_offset)
#define CCU_POLICY_FREQ_REG(ccu)	CCU_REG_ADDR(ccu, ccu->policy_freq_offset)
#define CCU_POLICY_CTRL_REG(ccu)	CCU_REG_ADDR(ccu, ccu->policy_ctl_offset)
#define CCU_POLICY_MASK1_REG(ccu)	CCU_REG_ADDR(ccu, ccu->policy_mask1_offset)
#define CCU_POLICY_MASK2_REG(ccu)	CCU_REG_ADDR(ccu, ccu->policy_mask2_offset)
#define CCU_INT_EN_REG(ccu)			CCU_REG_ADDR(ccu, ccu->inten_offset)
#define CCU_INT_STATUS_REG(ccu)		CCU_REG_ADDR(ccu, ccu->intstat_offset)
#define CCU_VLT_PERI_REG(ccu)		CCU_REG_ADDR(ccu, ccu->vlt_peri_offset)
#define CCU_VLT0_3_REG(ccu)			CCU_REG_ADDR(ccu, ccu->vlt0_3_offset)
#define CCU_VLT4_7_REG(ccu)			CCU_REG_ADDR(ccu, ccu->vlt4_7_offset)


#define CCU_REG_ADDR_FROM_REGBITS(ccu, register_bits)	CCU_REG_ADDR(ccu, \
(register_bits).reg_offset)

/*Helper Macros*/
#define CLK_FLG_ENABLED(clk,flg) ((clk)->flags & flg)
#define GET_CLK_NAME_STR(x) ((x)->clk.name)
#define CLK_NAME(x) clk_##x
#define CLK_PTR(x)  (&(clk_##x).clk)
#define BRCM_REGISTER_CLK(con, dev, clock)	\
	{\
		.con_id = con,\
		.dev_id = dev,\
		.clk = CLK_PTR(clock),\
	}

#define DEFINE_ARRAY_ARGS(...) {__VA_ARGS__}

#define FREQ_MHZ(x) ((x)*1000*1000)
#define FREQ_KHZ(x) ((x)*1000)

/* CCU Policy ids*/
enum
{
	CCU_POLICY0,
	CCU_POLICY1,
	CCU_POLICY2,
	CCU_POLICY3
};

/* CCU freq ids*/
enum
{
	CCU_FREQ0,
	CCU_FREQ1,
	CCU_FREQ2,
	CCU_FREQ3,
	CCU_FREQ4,
	CCU_FREQ5,
	CCU_FREQ6,
	CCU_FREQ7,
	CCU_FREQ_INVALID = 0xFF
};

/*Policy ctrl ids*/
enum
{
	POLICY_CTRL_GO,
	POLICY_CTRL_GO_AC,
	POLICY_CTRL_GO_ATL,
	POLICY_CTRL_TGT_VLD,
};

/*POLICY_CTRL_GO_ATL types*/
enum
{
	CCU_LOAD_TARGET,
	CCU_LOAD_ACTIVE
};

/*POLICY_CTRL_GO_AC flgs*/
enum
{
	CCU_AUTOCOPY_OFF,
	CCU_AUTOCOPY_ON
};



/*CCU interrupt types*/
enum
{
	ACT_INT,
	TGT_INT
};


/*CCU Peri voltage ids*/
enum
{
	VLT_NORMAL,
	VLT_HIGH
};


/*Voltage ids */
enum
{
	CCU_VLT0,
	CCU_VLT1,
	CCU_VLT2,
	CCU_VLT3,
	CCU_VLT4,
	CCU_VLT5,
	CCU_VLT6,
	CCU_VLT7,
	CCU_VLT_INVALID = 0xFF
};



/*clk gating status */
enum
{
	CLK_STOPPED,
	CLK_RUNNING
};

/*CLK - hyst*/
enum
{
	CLK_HYST_LOW,
	CLK_HYST_HIGH
};

/*clock gating ctrl*/
enum
{
	CLK_GATING_AUTO,
	CLK_GATING_SW
};

/* Clock flags */
enum {
    HYST_ENABLE 		= (1 << 0),
    HYST_HIGH 			= (1 << 1),
    AUTO_GATE 			= (1 << 2),
    INVERT_ENABLE		= (1 << 3),
    ENABLE_ON_INIT		= (1 << 4),
    DISABLE_ON_INIT		= (1 << 5),
    ENABLE_HVT			= (1 << 6),
    RATE_FIXED			= (1 << 7), /*used for peri ...clk set/get rate functions uses .rate field*/
    NOTIFY_STATUS_TO_CCU	= (1 << 8),
    DONOT_NOTIFY_STATUS_TO_CCU	= (1 << 9),

    /* CCU specific flags */
    CCU_TARGET_LOAD		= (1 << 16),
    CCU_TARGET_AC		= (1 << 17),

    /*Ref clk flags*/
    CLK_RATE_FIXED		= (1<<24),


};

/*clk type*/
enum
{
	CLK_TYPE_CCU = 1,
	CLK_TYPE_PERI,
	CLK_TYPE_BUS,
	CLK_TYPE_REF,
	CLK_TYPE_PLL
};

struct clk;

/**
 * struct gen_clk_ops - standard clock operations
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
struct gen_clk_ops {
	int	(*init)(struct clk *c);
	int	(*enable)(struct clk *c, int enable);
	int	(*set_parent)(struct clk *c, struct clk *parent);
	int	(*set_rate)(struct clk *c, u32 rate);
	unsigned long	(*get_rate)(struct clk *c);
	unsigned long	(*round_rate)(struct clk *c, unsigned long rate);

};

struct peri_clk_ops
{
};


struct bus_clk_ops
{
};

struct ccu_clk_ops
{
};

struct clk_div
{
	u32 div_offset;
	u32 div_mask;
	u32 div_shift;
	u32 pre_div_offset;
	u32 pre_div_mask;
	u32 pre_div_shift;
	u32 div_trig_offset;
	u32 div_trig_mask;
	u32 prediv_trig_offset;
	u32 prediv_trig_mask;
	u8  diether_bits;
	u32 pll_select_offset;
	u32 pll_select_mask;
	u32 pll_select_shift;

};

/**
 * struct clk_src - clock source
 * @total: number of clock sources, 0 means root clock, no parent
 * @clk: array of source clocks
 */


struct src_clk
{
	u8		count;
	u8		src_inx;
	struct clk 	**clk;
};


struct clk
{
	struct list_head 	list;
	struct module		*owner;
	const char	*name;
	int			id;
	int 		init;
	int			use_cnt;
	u32			flags;
	u32			rate;
	int 		clk_type;
	struct clk* dep_clks[MAX_DEP_CLKS];

	struct gen_clk_ops 	*ops;

};
/* to be revisited and tuned according to A9 CCU clock */
struct proc_clock {
	struct clk	clk;
	unsigned long	proc_clk_mgr_base;
};

struct ccu_clk {
	struct clk	clk;
	int pi_id;
	struct list_head peri_list;
	struct list_head bus_list;

	u32 pol_engine_dis_cnt;
	u32 write_access_en_count;

	u32	ccu_clk_mgr_base;

	u32	wr_access_offset;
	u32 policy_mask1_offset;
	u32 policy_mask2_offset;

	u32 policy_freq_offset;
	u32 policy_ctl_offset;
	u32 inten_offset;
	u32 intstat_offset;
	u32 vlt_peri_offset;
	u32 lvm_en_offset;
	u32 lvm0_3_offset;
	u32 vlt0_3_offset;
	u32 vlt4_7_offset;

	u8 freq_volt[MAX_CCU_FREQ_COUNT];
	const u8 freq_count;
	u8 volt_peri[MAX_CCU_PERI_VLT_COUNT];
	u8 freq_policy[MAX_CCU_POLICY_COUNT];

	struct ccu_clk_ops* ccu_ops;
	u8 active_policy;
	u32*	freq_tbl[MAX_CCU_FREQ_COUNT];

};

struct peri_clk {
	struct clk	clk;

	struct ccu_clk*	ccu_clk;
	int mask_set;
	u32 policy_bit_mask;
	u8 policy_mask_init[4];
	u32 clk_gate_offset;
	u32 clk_en_mask;
	u32 gating_sel_mask;
	u32 hyst_val_mask;
	u32 hyst_en_mask;
	u32 stprsts_mask;
	u32 volt_lvl_mask;

	struct peri_clk_ops* peri_ops;

	struct clk_div  clk_div;

	struct src_clk	src_clk;
};

struct bus_clk {
	struct clk	clk;

	struct ccu_clk*	ccu_clk;
	u32 clk_gate_offset;
	u32 clk_en_mask;
	u32 gating_sel_mask;
	u32 hyst_val_mask;
	u32 hyst_en_mask;
	u32 stprsts_mask;
	int freq_tbl_index;
	struct clk* src_clk;
	struct bus_clk_ops* bus_ops;
};

struct ref_clk {
	struct clk	clk;
	struct ccu_clk*	ccu_clk;
	u32 clk_gate_offset;
	u32 clk_en_mask;
	u32 gating_sel_mask;
	u32 hyst_val_mask;
	u32 hyst_en_mask;
	u32 stprsts_mask;
	struct clk_div  clk_div;
	struct src_clk	src_clk;

};



#define	to_clk(p) (&((p)->clk))
#define	name_to_clk(name) (&((name##_clk).clk))
/* declare a struct clk_lookup */
#define	CLK_LK(name) {.con_id=__stringify(name##_clk), .clk=name_to_clk(name),}

static inline struct proc_clock *to_proc_clk(struct clk *clock)
{
	return container_of(clock, struct proc_clock, clk);
}

static inline struct peri_clk *to_peri_clk(struct clk *clock)
{
	return container_of(clock, struct peri_clk, clk);
}

static inline struct ccu_clk *to_ccu_clk(struct clk *clock)
{
	return container_of(clock, struct ccu_clk, clk);
}

static inline struct bus_clk *to_bus_clk(struct clk *clock)
{
	return container_of(clock, struct bus_clk, clk);
}

static inline struct ref_clk *to_ref_clk(struct clk *clock)
{
	return container_of(clock, struct ref_clk, clk);
}

static inline int is_same_clock(struct clk *a, struct clk *b)
{
	return (a==b);
}

extern struct gen_clk_ops gen_ref_clk_ops;
extern struct gen_clk_ops gen_bus_clk_ops;
extern struct gen_clk_ops gen_ccu_clk_ops;
extern struct gen_clk_ops gen_peri_clk_ops;

extern int clk_debug;
int __init clock_init(void);
int __init clock_late_init(void);
unsigned long clock_get_xtal(void);

#ifdef CONFIG_DEBUG_FS
int clock_debug_init(void);
int clock_debug_add_clock(struct clk *c);
#else
#define	clock_debug_init() do {} while(0)
#define	clock_debug_add_clock(clk) do {} while(0)
#endif

int clk_register(struct clk_lookup *clk_lkup);
int ccu_set_freq_policy(struct ccu_clk* ccu_clk, int policy_id, int freq_id);

#if defined(DEBUG)
#define	clk_dbg printk
#else
#define	clk_dbg(format...)		\
	do {				\
		if (clk_debug) 		\
			printk(format);	\
	} while(0)
#endif


#endif /*__ARM_ARCH_KONA_CLOCK_H*/
