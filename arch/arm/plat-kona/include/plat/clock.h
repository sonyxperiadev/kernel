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

#ifndef __ARM_ARCH_KONA_CLOCK_H
#define __ARM_ARCH_KONA_CLOCK_H

#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/list.h>
#include <plat/pi_mgr.h>
#include <mach/clock.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#endif

#ifndef MAX_CCU_COUNT
#define MAX_CCU_COUNT	6
#endif

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

#define GET_PERI_SRC_CLK(clock) (\
		(clock)->src.list[(clock)->src.src_inx].clk)
#define PERI_SRC_CLK_INX(clock)		((clock)->src.src_inx)
#define PERI_SRC_CLK_COUNT(clock)	((clock)->src.count)
#define PERI_SRC_CLK_VALID(clock)	((clock)->src.count)

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
#define CCU_FREQ_POLICY1_SHIFT	8
#define CCU_FREQ_POLICY2_SHIFT	16
#define CCU_FREQ_POLICY3_SHIFT	24
#define CCU_FREQ_POLICY_MASK	7

#define CCU_POLICY_FREQ_REG_INIT	0xFFFFFFFF

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

#define CCU_POLICY_DBG_FREQ_MASK	7
#define CCU_POLICY_DBG_POLICY_MASK	3

#define CLK_OFF_DELAY_EN	1

#define CLK_DIV_TRIGGER		1
#define CLK_DIV_TRIG_OK		0

#define CCU_DBG_BUS_STATUS_MASK		0xFFFF
#define CCU_DBG_BUS_STATUS_SHIFT	0
#define CCU_DBG_BUS_SEL_MASK		(0x1F << 16)
#define CCU_DBG_BUS_SEL_SHIFT		16

#define POLICY_RESUME_INS_COUNT	20000
#define CLK_EN_INS_COUNT	1000

#define CCU_POLICY_MASK_ENABLE_ALL_MASK	0x7FFFFFFF

#define CCU_REG_ADDR(ccu, offset)	(ccu->ccu_clk_mgr_base + offset)
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
#define CCU_DBG_BUS_REG(ccu)		CCU_REG_ADDR(ccu, ccu->dbg_bus_offset)

#define CCU_REG_ADDR_FROM_REGBITS(ccu, register_bits)	CCU_REG_ADDR(ccu, \
(register_bits).reg_offset)

/*Helper Macros*/
#define CLK_FLG_ENABLED(clk, flg) (!!((clk)->flags & flg))
#define GET_CLK_NAME_STR(x) ((x)->clk.name)
#define CLK_NAME(x) clk_##x
#define CLK_PTR(x)  (&(clk_##x).clk)
#define BRCM_REGISTER_CLK(con, dev, clock)	\
	{\
		.con_id = con,\
		.dev_id = dev,\
		.clk = CLK_PTR(clock),\
	}
#define BRCM_REGISTER_CLK_DIRECT(con, dev, clock)	\
	{\
		.con_id = con,\
		.dev_id = dev,\
		.clk = clock,\
	}

#define DEFINE_ARRAY_ARGS(...) {__VA_ARGS__}

#define FREQ_MHZ(x) ((x)*1000*1000)
#define FREQ_KHZ(x) ((x)*1000)

#ifdef CONFIG_KONA_PI_MGR
#define CCU_ACCESS_EN(ccu, en) \
		if (CLK_FLG_ENABLED(&(ccu)->clk, CCU_ACCESS_ENABLE) &&\
						(ccu)->pi_id != -1) {\
			struct pi *pi = pi_mgr_get((ccu)->pi_id);\
			BUG_ON(pi == NULL);\
			if (en)\
				pi_enable(pi, 1);\
			else\
				pi_enable(pi, 0);\
		}
#else
#define CCU_ACCESS_EN(ccu, en)	{}
#endif

#define PLL_VCO_RATE_MAX	0xFFFFFFFF

#define INIT_SRC_CLK(c, sel) {\
		.clk = CLK_PTR(c),\
		.val = sel,\
}

/*PLL OFFSET register offsets and masks*/
#define PLL_OFFSET_NDIV_MASK	(0x1FF << PLL_OFFSET_NDIV_SHIFT)
#define PLL_OFFSET_NDIV_SHIFT	20
#define PLL_OFFSET_NDIV_F_MASK	(0xFFFFF << PLL_OFFSET_NDIV_F_SHIFT)
#define PLL_OFFSET_NDIV_F_SHIFT	0
#define PLL_OFFSET_MODE_MASK	(1 << 28)
#define PLL_OFFSET_SW_CTRL_MASK	(1 << 29)


/* CCU Policy ids*/
enum {
	CCU_POLICY0,
	CCU_POLICY1,
	CCU_POLICY2,
	CCU_POLICY3
};

/* CCU freq ids*/
enum {
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
enum {
	POLICY_CTRL_GO,
	POLICY_CTRL_GO_AC,
	POLICY_CTRL_GO_ATL,
	POLICY_CTRL_TGT_VLD,
};

/*POLICY_CTRL_GO_ATL types*/
enum {
	CCU_LOAD_TARGET,
	CCU_LOAD_ACTIVE
};

/*POLICY_CTRL_GO_AC flgs*/
enum {
	CCU_AUTOCOPY_OFF,
	CCU_AUTOCOPY_ON
};

/*CCU interrupt types*/
enum {
	ACT_INT,
	TGT_INT
};

/*CCU Peri voltage ids*/
enum {
	VLT_NORMAL,
	VLT_HIGH
};

/*Voltage ids */
enum {
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
enum {
	CLK_STOPPED,
	CLK_RUNNING
};

/*CLK - hyst*/
enum {
	CLK_HYST_LOW,
	CLK_HYST_HIGH
};

/*clock gating ctrl*/
enum {
	CLK_GATING_AUTO,
	CLK_GATING_SW
};

/* Clock flags */
enum {
	HYST_ENABLE = (1 << 0),
	HYST_HIGH = (1 << 1),
	AUTO_GATE = (1 << 2),
	INVERT_ENABLE = (1 << 3),
	ENABLE_ON_INIT = (1 << 4),
	DISABLE_ON_INIT = (1 << 5),
	ENABLE_HVT = (1 << 6),
	/*used for peri ...clk set/get rate functions uses .rate field */
	RATE_FIXED = (1 << 7),
	NOTIFY_STATUS_TO_CCU = (1 << 8),
	DONOT_NOTIFY_STATUS_TO_CCU = (1 << 9),
	UPDATE_LPJ = (1 << 10),	/*used for core clock */

	/* CCU specific flags */
	CCU_TARGET_LOAD = (1 << 16),
	CCU_TARGET_AC = (1 << 17),
	CCU_ACCESS_ENABLE = (1 << 18),
	CCU_KEEP_UNLOCKED = (1 << 19),
	CCU_DBG_BUS_EN = (1 << 20),

	/*Ref clk flags */
	CLK_RATE_FIXED = (1 << 24),

	DELAYED_PLL_LOCK = (1 << 28),
};

/*clk type*/
enum {
	CLK_TYPE_CCU = 1,
	CLK_TYPE_PERI,
	CLK_TYPE_BUS,
	CLK_TYPE_REF,
	CLK_TYPE_CORE,
	CLK_TYPE_PLL,
	CLK_TYPE_PLL_CHNL,
	CLK_TYPE_MISC,
};

enum {
	MONITOR_CAMCS_PIN,
	MONITOR_DEBUG_BUS_GPIO
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
	int (*init) (struct clk * c);
	int (*enable) (struct clk * c, int enable);
	int (*set_parent) (struct clk * c, struct clk * parent);
	int (*set_rate) (struct clk * c, u32 rate);
	unsigned long (*get_rate) (struct clk * c);
	unsigned long (*round_rate) (struct clk * c, unsigned long rate);
	int (*reset) (struct clk * c);

};

struct peri_clk_ops {
};

struct bus_clk_ops {
};
struct ccu_clk;
struct ccu_clk_ops {
	int (*write_access) (struct ccu_clk * ccu_clk, int enable);
	int (*rst_write_access) (struct ccu_clk * ccu_clk, int enable);
	int (*policy_engine_resume) (struct ccu_clk * ccu_clk, int load_type);
	int (*policy_engine_stop) (struct ccu_clk * ccu_clk);
	int (*set_policy_ctrl) (struct ccu_clk * ccu_clk, int pol_ctrl_id,
				int action);
	int (*int_enable) (struct ccu_clk * ccu_clk, int int_type, int enable);
	int (*int_status_clear) (struct ccu_clk * ccu_clk, int int_type);
	int (*set_freq_policy) (struct ccu_clk * ccu_clk, int policy_id,
				struct opp_info *opp_info);
	int (*get_freq_policy) (struct ccu_clk * ccu_clk, int policy_id);
	int (*set_peri_voltage) (struct ccu_clk * ccu_clk, int peri_volt_id,
				 u8 voltage);
	int (*set_voltage) (struct ccu_clk * ccu_clk, int volt_id, u8 voltage);
	int (*get_voltage) (struct ccu_clk * ccu_clk, int freq_id);
	int (*set_active_policy) (struct ccu_clk * ccu_clk, u32 policy);
	int (*get_active_policy) (struct ccu_clk * ccu_clk);
	int (*save_state) (struct ccu_clk * ccu_clk, int save);
	int (*get_dbg_bus_status) (struct ccu_clk *ccu_clk);
	int (*set_dbg_bus_sel) (struct ccu_clk *ccu_clk, u32 sel);
	int (*get_dbg_bus_sel) (struct ccu_clk *ccu_clk);
};

struct clk_div {
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
	u8 diether_bits;
	u32 pll_select_offset;
	u32 pll_select_mask;
	u32 pll_select_shift;

};

/**
 * struct reg_save - Structure to specify the set of consecutive CCU registers
 * that needs to be saved when CCU is shutdown
 * @offset_start: consecutive CCU register set start register offset
 * @offset_end: consecutive CCU register set end register offset - can be same as offset_start
 */
struct reg_save {
	u32 offset_start;
	u32 offset_end;
};

/**
 * struct ccu_state_save - This struct holds required info to save CCU
 * context during CCU shutdown
 * @reg_save: List of registers to be saved/restored
 * @reg_set_count: Number of enteries in reg_save list
 * @num_reg : Total no of registers to be saved. Clock manager initializes this.
	No need to pass from mach
 * @save_buf : Buffer to save the context - Can be NULL. Clock manager
 * will allocate the buffer if NULL.save_buf size should be num_reg + 1
 */

struct ccu_state_save {
	struct reg_save *reg_save;
	u32 reg_set_count;
	u32 num_reg;
	u32 *save_buf;
};

/**
 * struct src_clk - clock source
 * @total: number of clock sources, 0 means root clock, no parent
 * @clk: array of source clocks
 */

struct clock_source {
	struct clk *clk;
	u32 val;
};

struct src_clk {
	u8 count;
	u8 src_inx;
	struct clock_source *list;
};

#ifdef CONFIG_KONA_PI_MGR

enum clk_dfs_policy {
	CLK_DFS_POLICY_NONE,
	CLK_DFS_POLICY_STATE,
	CLK_DFS_POLICY_RATE
};

struct dfs_rate_thold {
	u32 rate_thold;
	u32 opp;
};
struct clk_dfs {
	u32 dfs_policy;
	u32 policy_param;
	u32 opp_weightage[PI_OPP_MAX];
	struct pi_mgr_dfs_node dfs_node;

};

#endif

struct clk {
	struct list_head list;
	struct module *owner;
	const char *name;
	int id;
	int init;
	int use_cnt;
	u32 flags;
	u32 rate;
	int clk_type;
	struct clk *dep_clks[MAX_DEP_CLKS];

	struct gen_clk_ops *ops;

};
/* to be revisited and tuned according to A9 CCU clock */
struct proc_clock {
	struct clk clk;
	unsigned long proc_clk_mgr_base;
};

struct ccu_clk {
	struct clk clk;
	int pi_id;
	struct list_head clk_list;
	u32 lock_flag;
	u32 pol_engine_dis_cnt;
	u32 write_access_en_count;

	void __iomem *ccu_clk_mgr_base;
	void __iomem *ccu_reset_mgr_base;
	u32 reset_wr_access_offset;
	u32 rst_write_access_en_count;

	u32 wr_access_offset;
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
	u32 dbg_bus_offset;

	u8 freq_volt[MAX_CCU_FREQ_COUNT];
	const u8 freq_count;
	u8 volt_peri[MAX_CCU_PERI_VLT_COUNT];
	u8 freq_policy[MAX_CCU_POLICY_COUNT];

	struct ccu_clk_ops *ccu_ops;
	u8 active_policy;
	u32 *freq_tbl[MAX_CCU_FREQ_COUNT];
	int freq_tbl_size;
	struct ccu_state_save *ccu_state_save;
	spinlock_t clk_lock;
	spinlock_t access_lock;
	u32 policy_dbg_offset;
	u32 policy_dbg_act_freq_shift;
	u32 policy_dbg_act_policy_shift;
#ifdef CONFIG_DEBUG_FS
	struct dentry *dent_ccu_dir;
	u32 clk_mon_offset;
#endif

};

struct peri_clk {
	struct clk clk;

	struct ccu_clk *ccu_clk;
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
	struct peri_clk_ops *peri_ops;

	struct clk_div clk_div;
	struct src_clk src;
	/*Reset offset and bit fields */
	u32 soft_reset_offset;
	u32 clk_reset_mask;

#ifdef CONFIG_KONA_PI_MGR
	struct clk_dfs *clk_dfs;
#endif

};

struct bus_clk {
	struct clk clk;

	struct ccu_clk *ccu_clk;
	u32 clk_gate_offset;
	u32 clk_en_mask;
	u32 gating_sel_mask;
	u32 hyst_val_mask;
	u32 hyst_en_mask;
	u32 stprsts_mask;
	int freq_tbl_index;
	struct clk *src_clk;
	struct bus_clk_ops *bus_ops;
#ifdef CONFIG_KONA_PI_MGR
	struct clk_dfs *clk_dfs;
#endif

	/*Reset offset and bit fields */
	u32 soft_reset_offset;
	u32 clk_reset_mask;
};

struct ref_clk {
	struct clk clk;
	struct ccu_clk *ccu_clk;
	u32 clk_gate_offset;
	u32 clk_en_mask;
	u32 gating_sel_mask;
	u32 hyst_val_mask;
	u32 hyst_en_mask;
	u32 stprsts_mask;
	struct clk_div clk_div;
	struct src_clk src_clk;

};

struct pll_cfg_ctrl_info {
	u32 pll_cfg_ctrl_offset;
	u32 pll_cfg_ctrl_mask;
	u32 pll_cfg_ctrl_shift;

	u32 *vco_thold;
	u32 *pll_config_value;
	u32 thold_count;
};

/*PLL desense adjust params*/
enum {
	PLL_OFFSET_EN = 1,
	PLL_OFFSET_NDIV = 1 << 1,
	PLL_OFFSET_NDIV_FRAC = 1 << 2,
	PLL_OFFSET_SW_MODE = 1 << 3,

};
struct pll_desense {
	u32 flags;
	u32 pll_offset_offset;
	int def_delta;
};

struct pll_clk {
	struct clk clk;
	struct ccu_clk *ccu_clk;

	u32 pll_ctrl_offset;
	u32 soft_post_resetb_offset;
	u32 soft_post_resetb_mask;
	u32 soft_resetb_offset;
	u32 soft_resetb_mask;
	u32 pwrdwn_offset;
	u32 pwrdwn_mask;
	u32 idle_pwrdwn_sw_ovrride_mask;
	u32 ndiv_pdiv_offset;
	u32 ndiv_int_mask;
	u32 ndiv_int_shift;
	u32 ndiv_int_max;
	u32 pdiv_mask;
	u32 pdiv_shift;
	u32 pdiv_max;
	u32 pll_lock_offset;
	u32 pll_lock;

	u32 ndiv_frac_offset;
	u32 ndiv_frac_mask;
	u32 ndiv_frac_shift;
	struct pll_desense *desense;
	struct pll_cfg_ctrl_info *cfg_ctrl_info;
};

struct pll_chnl_clk {
	struct clk clk;
	struct ccu_clk *ccu_clk;
	struct pll_clk *pll_clk;

	u32 cfg_reg_offset;
	u32 pll_hold_ch_offset;
	u32 pll_load_ch_en_offset;
	u32 pll_enableb_offset;
	u32 mdiv_mask;
	u32 mdiv_shift;
	u32 mdiv_max;

	u32 out_en_mask;

	u32 load_en_mask;

	u32 hold_en_mask;
};

struct core_clk {
	struct clk clk;
	struct ccu_clk *ccu_clk;
	struct pll_clk *pll_clk;
	struct pll_chnl_clk **pll_chnl_clk;
	u32 num_chnls;
	u32 active_policy;
	u32 *pre_def_freq;
	u32 num_pre_def_freq;

	u32 policy_bit_mask;
	u8 policy_mask_init[4];
	u32 clk_gate_offset;
	u32 clk_en_mask;
	u32 gating_sel_mask;
	u32 hyst_val_mask;
	u32 hyst_en_mask;
	u32 stprsts_mask;

	/*Reset offset and bit fields */
	u32 soft_reset_offset;
	u32 clk_reset_mask;
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
static inline struct pll_clk *to_pll_clk(struct clk *clock)
{
	return container_of(clock, struct pll_clk, clk);
}

static inline struct pll_chnl_clk *to_pll_chnl_clk(struct clk *clock)
{
	return container_of(clock, struct pll_chnl_clk, clk);
}

static inline struct core_clk *to_core_clk(struct clk *clock)
{
	return container_of(clock, struct core_clk, clk);
}

static inline int is_same_clock(struct clk *a, struct clk *b)
{
	return (a == b);
}

extern struct gen_clk_ops gen_ref_clk_ops;
extern struct gen_clk_ops gen_bus_clk_ops;
extern struct gen_clk_ops gen_ccu_clk_ops;
extern struct gen_clk_ops gen_peri_clk_ops;
extern struct gen_clk_ops gen_pll_clk_ops;
extern struct gen_clk_ops gen_pll_chnl_clk_ops;
extern struct gen_clk_ops gen_core_clk_ops;
extern struct ccu_clk_ops gen_ccu_ops;

extern int clk_debug;
int __init clock_init(void);
int __init clock_late_init(void);
unsigned long clock_get_xtal(void);

#ifdef CONFIG_DEBUG_FS
int clock_debug_init(void);
int clock_debug_add_clock(struct clk *c);
int __init clock_debug_add_ccu(struct clk *c, bool is_root_ccu);
#else
#define	clock_debug_init() do {} while(0)
#define	clock_debug_add_clock(clk) do {} while(0)
#define	clock_debug_add_ccu(clk) do {} while(0)
#endif

int clk_init(struct clk *clk);
int clk_reset(struct clk *clk);
int clk_register(struct clk_lookup *clk_lkup, int num_clks);
int peri_clk_set_hw_gating_ctrl(struct clk *clk, int gating_ctrl);
int peri_clk_hyst_enable(struct peri_clk *peri_clk, int enable, int delay);
int peri_clk_set_pll_select(struct peri_clk *peri_clk, int source);
int ccu_write_access_enable(struct ccu_clk *ccu_clk, int enable);
int ccu_reset_write_access_enable(struct ccu_clk *ccu_clk, int enable);
int ccu_policy_engine_resume(struct ccu_clk *ccu_clk, int load_type);
int ccu_policy_engine_stop(struct ccu_clk *ccu_clk);
int ccu_set_policy_ctrl(struct ccu_clk *ccu_clk, int pol_ctrl_id, int action);
int ccu_int_enable(struct ccu_clk *ccu_clk, int int_type, int enable);
int ccu_int_status_clear(struct ccu_clk *ccu_clk, int int_type);
int ccu_set_freq_policy(struct ccu_clk *ccu_clk, int policy_id,
				struct opp_info *opp_info);
int ccu_get_freq_policy(struct ccu_clk *ccu_clk, int policy_id);
int ccu_set_peri_voltage(struct ccu_clk *ccu_clk, int peri_volt_id, u8 voltage);
int ccu_set_voltage(struct ccu_clk *ccu_clk, int volt_id, u8 voltage);
int ccu_get_voltage(struct ccu_clk *ccu_clk, int freq_id);
int ccu_set_active_policy(struct ccu_clk *ccu_clk, u32 policy);
int ccu_get_active_policy(struct ccu_clk *ccu_clk);
int ccu_save_state(struct ccu_clk *ccu_clk, int save);
int ccu_get_dbg_bus_status(struct ccu_clk *ccu_clk);
int ccu_set_dbg_bus_sel(struct ccu_clk *ccu_clk, u32 sel);
int ccu_get_dbg_bus_sel(struct ccu_clk *ccu_clk);
int ccu_print_sleep_prevent_clks(struct clk *clk);
int ccu_volt_id_update_for_freqid(struct clk *clk, u8 freq_id, u8 volt_id);
int ccu_volt_tbl_display(struct clk *clk, u8 *volt_tbl);
int ccu_clk_get_freq_id_from_opp(struct ccu_clk *ccu_clk, int opp_id);
u32 ccu_clk_get_rate(struct clk *clk, int opp_id);

int pll_set_desense_offset(struct clk *clk, int offset);
int pll_get_desense_offset(struct clk *clk);
int pll_desense_enable(struct clk *clk, int enable);

int ccu_policy_dbg_get_act_policy(struct ccu_clk *ccu_clk);
int ccu_policy_dbg_get_act_freqid(struct ccu_clk *ccu_clk);
int ref_clk_get_gating_status(struct ref_clk *ref_clk);

int pll_clk_get_lock_status(struct pll_clk *pll_clk);
int pll_clk_get_pdiv(struct pll_clk *pll_clk);
int pll_clk_get_ndiv_int(struct pll_clk *pll_clk);
int pll_clk_get_ndiv_frac(struct pll_clk *pll_clk);
int pll_clk_get_idle_pwrdwn_sw_ovrride(struct pll_clk *pll_clk);
int pll_clk_get_pwrdwn(struct pll_clk *pll_clk);


int pll_chnl_clk_get_enb_clkout(struct pll_chnl_clk *pll_chnl_clk);
int pll_chnl_clk_get_mdiv(struct pll_chnl_clk *pll_chnl_clk);

int ref_clk_get_gating_ctrl(struct ref_clk *ref_clk);
int ref_clk_get_enable_bit(struct ref_clk *ref_clk);

int core_clk_get_gating_ctrl(struct core_clk *core_clk);
int core_clk_get_gating_status(struct core_clk *core_clk);
int core_clk_get_enable_bit(struct core_clk *core_clk);

int peri_clk_get_gating_ctrl(struct peri_clk *peri_clk);
int peri_clk_get_enable_bit(struct peri_clk *peri_clk);
int peri_clk_get_gating_status(struct peri_clk *peri_clk);

int bus_clk_get_enable_bit(struct bus_clk *bus_clk);
int bus_clk_get_gating_ctrl(struct bus_clk *bus_clk);
int bus_clk_get_gating_status(struct bus_clk *bus_clk);

int clk_trace_init(unsigned int count);



/*These clock API should only be called after
* appropriate locks are acquired*/
int __clk_enable(struct clk *clk);
void __clk_disable(struct clk *clk);




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
