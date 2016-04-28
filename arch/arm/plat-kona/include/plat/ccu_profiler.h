/*****************************************************************************
*
* Kona profiler framework
*
* Copyright 2012 Broadcom Corporation.  All rights reserved.
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
#ifndef _CCU_PROFILER_H
#define _CCU_PROFILER_H

#include "profiler.h"

#define DEFINE_CCU_PROFILER(name)	\
	static struct ccu_profiler DEFINE_PROFILER(ccu, name)

#define CCU_PROFILER(name)			DEFINE_PROFILER(ccu, name)
#define PROFILER_CCU_DIR_NAME			("ccu")

#define CCU_CNT_OVERFLOW_MASK		(0x1<<30)
#define CCU_CNT_OVERLOW_VAL		(0x1<<30)

enum ccu_profiling_counter {
	CCU_PROF_NONE = 0,
	CCU_PROF_PLL_PWRDWN,
	CCU_PROF_AUTOGATING,
	CCU_PROF_POLICY,
	CCU_PROF_CLK_REQ,
	CCU_PROF_ALWAYS_ON,
};

enum ccu_prof_policy_sel {
	CCU_PROF_POLICY_4 = (1<<0),
	CCU_PROF_POLICY_5 = (1<<1),
	CCU_PROF_POLICY_6 = (1<<2),
	CCU_PROF_POLICY_7 = (1<<3),
};

struct ccu_profiler;

struct gen_ccu_prof_ops {
	int (*set_prof_type)(struct ccu_profiler *ccu_profiler,
		enum ccu_profiling_counter counter_type);
	int (*get_prof_type)(struct ccu_profiler *ccu_profiler);
	int (*set_prof_policy)(struct ccu_profiler *ccu_profiler,
		enum ccu_prof_policy_sel policy);
	int (*get_prof_policy)(struct ccu_profiler *ccu_profiler);
	int (*set_autogate_sel)(struct ccu_profiler *ccu_profiler,
		u32 select, u32 value);
	int (*get_autogate_sel)(struct ccu_profiler *ccu_profiler,
		u32 select, u32 *value);
	int (*set_clkreq_sel)(struct ccu_profiler *ccu_profiler,
		u32 select, u32 value);
	int (*get_clkreq_sel)(struct ccu_profiler *ccu_profiler,
		u32 select, u32 *value);
};

/**
 * Kona CCU Profiler
 */

struct ccu_prof_parameter {
	enum ccu_profiling_counter count_type;
	enum ccu_prof_policy_sel policy;
	int val;
	int sel;
};

struct ccu_profiler {
	struct profiler profiler;

	enum ccu_prof_policy_sel policy_sel;
	u32 auto_gate_sel0;
	u32 auto_gate_sel1;
	u32 clk_req_sel0;
	u32 clk_req_sel1;
	/**
	 * Kona ccu clock device name to access the CCU registers
	 */
	const char *clk_dev_id;

	/**
	 * CCU Profiler register offsets and masks
	 */
	u32 ctrl_offset;
	u32 policy_sel_offset;
	u32 auto_gate_sel0_offset;
	u32 auto_gate_sel1_offset;
	u32 clk_req_sel0_offset;
	u32 clk_req_sel1_offset;
	u32 counter_offset;

	u32 cntr_start_mask;
	u32 cntrl_counter_mask;
	u32 policy_sel_mask;
	u32 auto_gate_sel0_mask;
	u32 auto_gate_sel1_mask;
	u32 clk_req_sel0_mask;
	u32 clk_req_sel1_mask;
	u32 counter_mask;

	u32 cntrl_start_shift;
	u32 cntrl_counter_shift;
	u32 policy_sel_shift;
	u32 auto_gate_sel0_shift;
	u32 auto_gate_sel1_shift;
	u32 clk_req_sel0_shift;
	u32 clk_req_sel1_shift;
	u32 counter_shift;
	struct ccu_clk *ccu_clk;
	struct gen_ccu_prof_ops *ccu_gen_prof_ops;
};

int ccu_profiler_register(struct ccu_profiler *ccu_profiler);
int ccu_profiler_unregister(struct ccu_profiler *ccu_profiler);
int ccu_profiler_init(struct dentry *prof_root_dir);
#endif
