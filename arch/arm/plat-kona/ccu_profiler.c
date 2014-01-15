/*****************************************************************************
*
* @file ccu_profiler.c
*
* Kona CCU Profiler
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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <asm/io.h>
#include <linux/mutex.h>
#include <plat/clock.h>
#include <plat/profiler.h>
#include <plat/ccu_profiler.h>
#include <mach/kona_timer.h>
#include <mach/timex.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>

static struct dentry *dentry_root_dir;
static struct dentry *dentry_ccu_dir;
static bool init;

char *get_prof_name(enum ccu_profiling_counter prof_type)
{
	switch (prof_type) {
	case CCU_PROF_NONE:		return "None";
	case CCU_PROF_PLL_PWRDWN:	return "PLL Power Down";
	case CCU_PROF_AUTOGATING:	return "Clk_Idle";
	case CCU_PROF_POLICY:		return "Policy";
	case CCU_PROF_CLK_REQ:		return "Clk_Req";
	case CCU_PROF_ALWAYS_ON:	return "Always ON";
	default:			return "Invalid";
	}
}

static int ccu_prof_check_params(struct ccu_profiler *ccu_profiler)
{
	struct ccu_clk *ccu_clk;
	u32 reg;
	u32 autogate_sel0 = 0;
	u32 autogate_sel1 = 0;
	u32 clkreq_sel0 = 0;
	u32 clkreq_sel1 = 0;
	int err = 0;
	enum ccu_profiling_counter cnt_type = CCU_PROF_NONE;
	enum ccu_prof_policy_sel policy;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);

	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	cnt_type = ((reg & ccu_profiler->cntrl_counter_mask) >>
			ccu_profiler->cntrl_counter_shift);
	if (cnt_type <= CCU_PROF_NONE ||
			cnt_type > CCU_PROF_ALWAYS_ON) {
		pr_err("ccu profiling type not set\n");
		err = -ENODEV;
		goto out_err;
	} else if (cnt_type == CCU_PROF_AUTOGATING) {
		autogate_sel0 =
			readl(CCU_REG_ADDR(ccu_clk,
						ccu_profiler->
						auto_gate_sel0_offset));
		if (ccu_profiler->auto_gate_sel1_offset != 0)
			autogate_sel1 =
				readl(CCU_REG_ADDR(ccu_clk,
							ccu_profiler->
							auto_gate_sel1_offset));
		if (!autogate_sel0 && !autogate_sel1) {
			pr_err("autogate select1 not set\n");
			err = -ENODEV;
			goto out_err;
		}
	} else if (cnt_type == CCU_PROF_POLICY) {
		policy =
			(readl(CCU_REG_ADDR(ccu_clk,
					    ccu_profiler->policy_sel_offset))
			 & ccu_profiler->policy_sel_mask);
		if (!policy) {
			pr_err("ccu profiler policy select not set\n");
			err = -ENODEV;
			goto out_err;
		}
	} else if (cnt_type == CCU_PROF_CLK_REQ) {
		clkreq_sel0 =
			(readl(CCU_REG_ADDR(ccu_clk,
					    ccu_profiler->
					    clk_req_sel0_offset)) &
			 ccu_profiler->clk_req_sel0_mask);
		if (ccu_profiler->clk_req_sel1_offset)
			clkreq_sel1 =
				(readl(CCU_REG_ADDR(ccu_clk,
						    ccu_profiler->
						    clk_req_sel1_offset)) &
				 ccu_profiler->clk_req_sel1_mask);
		if (!clkreq_sel0 && !clkreq_sel1) {
			pr_err("clk_req_sel is not set\n");
			err = -ENODEV;
			goto out_err;
		}
	}
out_err:
	return err;
}
static int ccu_prof_init(struct profiler *profiler)
{
	struct ccu_profiler *ccu_profiler = container_of(profiler,
				struct ccu_profiler, profiler);
	struct ccu_clk *ccu_clk;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;
	ccu_clk = ccu_profiler->ccu_clk;
	ccu_reset_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	reg &= ~ccu_profiler->cntr_start_mask;
	writel(reg, CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	ccu_reset_write_access_enable(ccu_clk, false);

	return 0;
}

static int ccu_prof_start(struct profiler *profiler, int start)
{
	struct ccu_profiler *ccu_profiler = container_of(profiler,
		struct ccu_profiler, profiler);
	struct ccu_clk *ccu_clk;
	u32 reg;
	int err = 0;

	profiler_dbg("%s\n", __func__);
	if (!ccu_profiler)
		return -EINVAL;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	if (start == 1) {
		err = ccu_prof_check_params(ccu_profiler);
		if (err < 0)
			goto out_err;
	/* To support restart, clear and then start the counter */
		reg &= ~ccu_profiler->cntr_start_mask;
		writel(reg, CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
		reg |= (ccu_profiler->cntr_start_mask <<
				ccu_profiler->cntrl_start_shift);
		profiler->flags |= PROFILER_RUNNING;
	} else if (start == 0) {
		reg &= ~ccu_profiler->cntr_start_mask;
		profiler->flags &= ~PROFILER_RUNNING;
	}
	profiler_dbg("writing regiter value %x\n", reg);
	writel(reg, CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));

out_err:
	ccu_write_access_enable(ccu_clk, false);
	return err;
}

static int ccu_prof_status(struct profiler *profiler)
{
	struct ccu_profiler *ccu_profiler = container_of(profiler,
		struct ccu_profiler, profiler);
	struct ccu_clk *ccu_clk;
	u32 reg;

	if (!ccu_profiler)
		return -EINVAL;
	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	profiler_dbg("%s: reg %x\n", __func__, reg);
	ccu_write_access_enable(ccu_clk, false);
	return reg & ccu_profiler->cntr_start_mask;
}

static int ccu_prof_get_counter(struct profiler *profiler,
				unsigned long *counter,
				int *overflow)
{
	struct ccu_profiler *ccu_profiler = container_of(profiler,
		struct ccu_profiler, profiler);
	struct ccu_clk *ccu_clk;
	u32 reg;

	if (!ccu_profiler)
		return -EINVAL;
	*overflow = 0;
	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->counter_offset));
	if (reg & CCU_CNT_OVERFLOW_MASK) {
		profiler_dbg("Counter overflowed\n");
		*overflow = 1;
	}
	reg = ((reg & ccu_profiler->counter_mask) >>
		ccu_profiler->counter_shift);
	ccu_write_access_enable(ccu_clk, false);
	*counter = reg;
	return 0;
}

static int ccu_prof_print(struct profiler *profiler)
{
	unsigned long counter = 0;
	int err;
	int len_name, len_raw_cnt, len_ms_cnt, overflow;
	u8 buffer[32];
	u8 str_raw_cnt[] = "RAW_CNT:";
	u8 str_ms_cnt[] = "MS_CNT:";

	profiler_dbg("%s\n", __func__);
	err = ccu_prof_get_counter(profiler, &counter, &overflow);
	if (err < 0)
		return err;

	len_name = strlen(profiler->name);
	len_raw_cnt = strlen(str_raw_cnt);
	len_ms_cnt = strlen(str_ms_cnt);

	memcpy(buffer, profiler->name, len_name);
	memcpy(buffer + len_name, str_raw_cnt, len_raw_cnt);
	memcpy(buffer + len_name + len_raw_cnt, str_ms_cnt, len_ms_cnt);

	profiler_print(buffer);
	return 0;
}

int ccu_prof_set_prof_type(struct ccu_profiler *ccu_profiler,
		enum ccu_profiling_counter counter_type)
{
	struct ccu_clk *ccu_clk;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;
	if ((counter_type <= CCU_PROF_NONE) ||
		(counter_type > CCU_PROF_ALWAYS_ON))
		return -EINVAL;

	profiler_dbg("set counter type %d\n", counter_type);

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	reg &= ~ccu_profiler->cntrl_counter_mask;
	reg |= ((counter_type << ccu_profiler->cntrl_counter_shift)&
		ccu_profiler->cntrl_counter_mask);
	profiler_dbg("set cntrl reg %x\n", reg);
	writel(reg, CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	ccu_write_access_enable(ccu_clk, false);
	return 0;
}

int ccu_prof_get_prof_type(struct ccu_profiler *ccu_profiler)
{
	struct ccu_clk *ccu_clk;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->ctrl_offset));
	reg = ((reg & ccu_profiler->cntrl_counter_mask)>>
		ccu_profiler->cntrl_counter_shift);
	ccu_write_access_enable(ccu_clk, false);
	return reg;
}

int ccu_prof_set_policy(struct ccu_profiler *ccu_profiler,
				 enum ccu_prof_policy_sel policy)
{
	struct ccu_clk *ccu_clk;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;
	if (!(policy & (CCU_PROF_POLICY_4|
		CCU_PROF_POLICY_5|
		CCU_PROF_POLICY_6|
		CCU_PROF_POLICY_7)))
		return -EINVAL;

	profiler_dbg("Set policy to %d\n", policy);

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->policy_sel_offset));
	reg &= ~ccu_profiler->policy_sel_mask;
	reg |= ((policy << ccu_profiler->policy_sel_shift)&
		ccu_profiler->policy_sel_mask);
	profiler_dbg("set policy select reg %x\n", reg);
	writel(reg, CCU_REG_ADDR(ccu_clk, ccu_profiler->policy_sel_offset));
	ccu_write_access_enable(ccu_clk, false);
	return 0;
}

int ccu_prof_get_policy(struct ccu_profiler *ccu_profiler)
{
	struct ccu_clk *ccu_clk;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	reg = readl(CCU_REG_ADDR(ccu_clk, ccu_profiler->policy_sel_offset));
	reg = ((reg & ccu_profiler->policy_sel_mask)>>
		ccu_profiler->policy_sel_shift);
	ccu_write_access_enable(ccu_clk, false);
	return reg;
}

int ccu_prof_set_autogate_sel(struct ccu_profiler *ccu_profiler,
					u32 select, u32 value)
{
	struct ccu_clk *ccu_clk;
	int err = 0;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	if (select == 0) {
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->auto_gate_sel0_offset));
		reg &= ~ccu_profiler->auto_gate_sel0_mask;
		reg |= value & ccu_profiler->auto_gate_sel0_mask;
		profiler_dbg("set autogate sel0 reg %x\n", reg);
		writel(reg, CCU_REG_ADDR(ccu_clk,
				ccu_profiler->auto_gate_sel0_offset));
	} else if (select == 1) {
		if (ccu_profiler->auto_gate_sel1_offset == 0) {
			profiler_dbg("autogate select1 not supported by ccu\n");
			err = -ENODEV;
			goto out;
		}
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->auto_gate_sel1_offset));
		reg &= ~ccu_profiler->auto_gate_sel1_mask;
		reg |= value & ccu_profiler->auto_gate_sel1_mask;
		writel(reg, CCU_REG_ADDR(ccu_clk,
					ccu_profiler->auto_gate_sel1_offset));
		profiler_dbg("set autogate sel1 reg %x\n", reg);
	} else
		err = -EINVAL;
out:
	ccu_write_access_enable(ccu_clk, false);
	return err;
}

int ccu_prof_get_autogate_sel(struct ccu_profiler *ccu_profiler,
					u32 select, u32 *value)
{
	struct ccu_clk *ccu_clk;
	int err = 0;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	if (select == 0) {
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->auto_gate_sel0_offset));
		reg &= ccu_profiler->auto_gate_sel0_mask;
	} else if (select == 1) {
		if (ccu_profiler->auto_gate_sel1_offset == 0) {
			profiler_dbg("autogate select1 not supported by ccu\n");
			err = -ENODEV;
			goto out;
		}
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->auto_gate_sel1_offset));
		reg &= ccu_profiler->auto_gate_sel1_mask;
	} else
		err = -EINVAL;
	if (!err)
		*value = reg;
out:
	ccu_write_access_enable(ccu_clk, false);
	return err;
}

int ccu_prof_set_clkreq_sel(struct ccu_profiler *ccu_profiler,
				      u32 select, u32 value)
{
	struct ccu_clk *ccu_clk;
	int err = 0;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	if (select == 0) {
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->clk_req_sel0_offset));
		reg &= ~ccu_profiler->clk_req_sel0_mask;
		reg |= value & ccu_profiler->clk_req_sel0_mask;
		profiler_dbg("set clkreq sel0 reg %x\n", reg);
		writel(reg, CCU_REG_ADDR(ccu_clk,
				ccu_profiler->clk_req_sel0_offset));
	} else if (select == 1) {
		if (ccu_profiler->clk_req_sel1_offset == 0) {
			profiler_dbg("clkreq select 1 not supported by ccu\n");
			err = -ENODEV;
			goto out;
		}
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->clk_req_sel1_offset));
		reg &= ~ccu_profiler->clk_req_sel1_mask;
		reg |= value & ccu_profiler->clk_req_sel1_mask;
		profiler_dbg("set clkreq sel1 reg %x\n", reg);
		writel(reg, CCU_REG_ADDR(ccu_clk,
				ccu_profiler->clk_req_sel1_offset));
	} else
		err = -EINVAL;
out:
	ccu_write_access_enable(ccu_clk, false);
	return 0;
}

int ccu_prof_get_clkreq_sel(struct ccu_profiler *ccu_profiler,
				      u32 select, u32 *value)
{
	struct ccu_clk *ccu_clk;
	int err = 0;
	u32 reg;

	profiler_dbg("%s\n", __func__);

	if (!ccu_profiler)
		return -EINVAL;

	ccu_clk = ccu_profiler->ccu_clk;
	ccu_write_access_enable(ccu_clk, true);
	if (select == 0) {
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->clk_req_sel0_offset));
		reg &= ccu_profiler->clk_req_sel0_mask;
	} else if (select == 1) {
		if (ccu_profiler->clk_req_sel1_offset == 0) {
			profiler_dbg("clkreq select 1 not supported by ccu\n");
			err = -ENODEV;
			goto out;
		}
		reg = readl(CCU_REG_ADDR(ccu_clk,
				ccu_profiler->clk_req_sel1_offset));
		reg &= ccu_profiler->clk_req_sel1_mask;
	} else
		err = -EINVAL;
	if (!err)
		*value = reg;
out:
	ccu_write_access_enable(ccu_clk, false);
	return 0;
}

static int set_ccu_prof_start(void *data, u64 start)
{
	struct ccu_profiler *ccu_profiler = data;
	int err = 0;
	if (start == 1) {
		err = ccu_profiler->profiler.ops->start(&ccu_profiler->profiler,
				1);
		if (err < 0) {
			pr_err("Failed to start ccu profiler\n");
			return err;
		}
		ccu_profiler->profiler.start_time = ktime_to_ms(ktime_get());
		ccu_profiler->profiler.stop_time = 0;
		ccu_profiler->profiler.overflow = 0;
		ccu_profiler->profiler.running_time = 0;
		ccu_profiler->profiler.ops->get_counter(
				&ccu_profiler->profiler,
				&ccu_profiler->profiler.running_time,
				&ccu_profiler->profiler.overflow);
		if (ccu_profiler->profiler.overflow)
			ccu_profiler->profiler.flags |= PROFILER_OVERFLOW;
	/* Overflow bit once set cannot be flushed, due to known ASIC issue
		So, we have to keep track of it for the next cycles */
	} else if (start == 0) {
		if (ccu_profiler->profiler.ops->status(&ccu_profiler->profiler)
				== 0)
			return 0;
		ccu_profiler->profiler.stop_time = ktime_to_ms(ktime_get());
		ccu_profiler->profiler.ops->get_counter(
				&ccu_profiler->profiler,
				&ccu_profiler->profiler.running_time,
				&ccu_profiler->profiler.overflow);
	/* Read everything before clearing the counters	*/
		err = ccu_profiler->profiler.ops->start(
				&ccu_profiler->profiler, 0);
	}
	return err;
}

static int get_ccu_prof_start(void *data, u64 *is_running)
{
	struct ccu_profiler *ccu_profiler = data;
	*is_running = ccu_profiler->profiler.ops->status(
			&ccu_profiler->profiler);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_prof_start_fops, get_ccu_prof_start,
		set_ccu_prof_start, "%llu\n");

static int set_ccu_prof_type(void *data, u64 counter_type)
{
	int err;
	struct ccu_profiler *ccu_profiler = data;

	if (ccu_profiler->profiler.ops->status(&ccu_profiler->profiler)) {
		profiler_dbg(" Stop the profiler first\n");
		return -EBUSY;
	}

	err = ccu_profiler->ccu_gen_prof_ops->set_prof_type(ccu_profiler,
			counter_type);
	if (err < 0)
		pr_err("Failed to set profiling counter type\n");
	return err;
}

static int get_ccu_prof_type(void *data, u64 *counter_type)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	*counter_type = 0;
	err = ccu_profiler->ccu_gen_prof_ops->get_prof_type(ccu_profiler);
	if (err < 0)
		pr_err("Failed to get profiling counter type\n");
	else
		*counter_type = err;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_prof_counter_type_fops,
	get_ccu_prof_type, set_ccu_prof_type, "%llu\n");

static int set_ccu_prof_policy(void *data, u64 policy)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;
	err = ccu_profiler->ccu_gen_prof_ops->set_prof_policy(ccu_profiler,
		policy);
	if (err < 0)
		pr_err("Failed to set profiler policy\n");

	return err;
}

static int get_ccu_prof_policy(void *data, u64 *policy)
{
	struct ccu_profiler *ccu_profiler = data;
	int err = 0;
	err = ccu_profiler->ccu_gen_prof_ops->get_prof_policy(ccu_profiler);
	if (err < 0)
		pr_err("Failed to get profiler policy\n");
	else
		*policy = err;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_prof_policy_fops,
	get_ccu_prof_policy, set_ccu_prof_policy, "%llu\n");


static int set_ccu_prof_autogate_sel0(void *data, u64 select0)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;
	err = ccu_profiler->ccu_gen_prof_ops->set_autogate_sel(ccu_profiler,
		0, select0);
	if (err < 0)
		pr_err("Failed to set autogate select0\n");

	return err;
}

static int get_ccu_prof_autogate_sel0(void *data, u64 *select0)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	*select0 = 0;
	err = ccu_profiler->ccu_gen_prof_ops->get_autogate_sel(ccu_profiler,
		0, (u32 *)select0);
	if (err < 0)
		pr_err("Failed to get autogate select0\n");

	return err;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_prof_autogate_sel0_fops,
	get_ccu_prof_autogate_sel0, set_ccu_prof_autogate_sel0, "%llu\n");

static int set_ccu_prof_autogate_sel1(void *data, u64 select1)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	err = ccu_profiler->ccu_gen_prof_ops->set_autogate_sel(ccu_profiler,
		1, select1);
	if (err < 0)
		pr_err("Failed to set autogate select1\n");

	return err;
}

static int get_ccu_prof_autogate_sel1(void *data, u64 *select1)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	*select1 = 0;
	err = ccu_profiler->ccu_gen_prof_ops->get_autogate_sel(ccu_profiler,
		1, (u32 *)select1);
	if (err < 0)
		pr_err("Failed to get autogate select1\n");

	return err;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_prof_autogate_sel1_fops,
	get_ccu_prof_autogate_sel1, set_ccu_prof_autogate_sel1, "%llu\n");

static int set_ccu_prof_clkreq_sel0(void *data, u64 select0)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	err = ccu_profiler->ccu_gen_prof_ops->set_clkreq_sel(ccu_profiler,
		0, select0);
	if (err < 0)
		pr_err("Failed to set clkreq select0\n");

	return err;
}

static int get_ccu_prof_clkreq_sel0(void *data, u64 *select0)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	*select0 = 0;
	err = ccu_profiler->ccu_gen_prof_ops->get_clkreq_sel(ccu_profiler,
		0, (u32 *)select0);
	if (err < 0)
		pr_err("Failed to set clkreq select0\n");

	return err;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_prof_clkreq_sel0_fops,
	get_ccu_prof_clkreq_sel0, set_ccu_prof_clkreq_sel0, "%llu\n");

static int set_ccu_prof_clkreq_sel1(void *data, u64 select1)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	err = ccu_profiler->ccu_gen_prof_ops->set_clkreq_sel(ccu_profiler,
		1, select1);
	if (err < 0)
		pr_err("Failed to set clkreq select1\n");

	return err;
}

static int get_ccu_prof_clkreq_sel1(void *data, u64 *select1)
{
	struct ccu_profiler *ccu_profiler = data;
	int err;

	*select1 = 0;
	err = ccu_profiler->ccu_gen_prof_ops->get_clkreq_sel(ccu_profiler,
		1, (u32 *)select1);
	if (err < 0)
		pr_err("Failed to set clkreq select1\n");

	return err;
}

DEFINE_SIMPLE_ATTRIBUTE(ccu_prof_clkreq_sel1_fops,
	get_ccu_prof_clkreq_sel1, set_ccu_prof_clkreq_sel1, "%llu\n");

static int get_counter_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}
static int get_counter_read(struct file *file, char __user *buf, size_t len,
		loff_t *ppos)
{
	struct ccu_profiler *ccu_profiler = file->private_data;
	int err = 0;
	int count = 0;
	int overflow = 0;
	int history = 0;
	unsigned long cnt = 0;
	unsigned long duration = 0;
	unsigned curr_time = 0;
	u8 buffer[128];
	unsigned long start_time = 0;
	int status = ccu_profiler->profiler.ops->status
				(&ccu_profiler->profiler);
	int type = ccu_profiler->ccu_gen_prof_ops->get_prof_type
				(ccu_profiler);
	if (status) {
		curr_time = ktime_to_ms(ktime_get());
		err = ccu_profiler->profiler.ops->get_counter(
				&ccu_profiler->profiler,
				&cnt, &overflow);
		start_time = ccu_profiler->profiler.start_time;
		duration = curr_time - start_time;
		if (ccu_profiler->profiler.flags & PROFILER_OVERFLOW)
			history = 1;
	} else if (ccu_profiler->profiler.stop_time) {
		curr_time = ccu_profiler->profiler.stop_time;
		start_time = ccu_profiler->profiler.start_time;
		duration = curr_time - start_time;
		overflow = ccu_profiler->profiler.overflow;
		cnt = ccu_profiler->profiler.running_time;
		if (ccu_profiler->profiler.flags & PROFILER_OVERFLOW)
			history = 1;
	}
	if (status) {
		count = snprintf(buffer, 100, "Profile: %s\t Status: Running"\
				"\tDuration = %lums\t cnt_raw = %lu\t" \
				"cnt = %lums %s\n", get_prof_name(type),
				duration, cnt, COUNTER_TO_MS(cnt),
				((!overflow) ? " " : ((history == overflow)
				? ((duration < 330000) ? " " : "*") :
				(duration < 330000) ? " " : "*")));
	} else if (ccu_profiler->profiler.stop_time) {
		count = snprintf(buffer, 100, "Profile: %s\t Status: Stopped"\
				"\tDuration = %lums\t cnt_raw = %lu\t" \
				"cnt = %lums %s\n", get_prof_name(type),
				duration, cnt, COUNTER_TO_MS(cnt),
				((!overflow) ? " " : ((history == overflow)
				? ((duration < 330000) ? " " : "*") :
				(duration < 330000) ? " " : "*")));
	} else
		count = snprintf(buffer, 30, "Status: Yet to begin\n");
	/* If overflow bit isn't set, then value is proper. If it is
	set, it is compared with history and till 165 sec, the value
	is guaranteed to be correct */
	count = simple_read_from_buffer(buf, len, ppos, buffer, count);
	return count;
}

static const struct file_operations ccu_prof_counter_fops = {
	.open = get_counter_open,
	.read = get_counter_read,
};

static int ccu_profiler_start(struct profiler *profiler, void *data)
{
	int ret = 0;
	struct ccu_profiler *ccu_profiler;
	struct ccu_prof_parameter *param;
	if (!profiler) {
		ret = -EINVAL;
		goto err;
	}
	ccu_profiler = container_of(profiler, struct ccu_profiler,
						profiler);
	param =	(struct ccu_prof_parameter *)data;

	switch (param->count_type) {
	case CCU_PROF_NONE:
	case CCU_PROF_PLL_PWRDWN:
				goto err;

	case CCU_PROF_AUTOGATING:
				ccu_profiler->ccu_gen_prof_ops->
					set_prof_type(ccu_profiler,
						param->count_type);
				ccu_profiler->ccu_gen_prof_ops->
					set_autogate_sel(ccu_profiler,
						param->sel, param->val);
				break;

	case CCU_PROF_POLICY:
				ccu_profiler->ccu_gen_prof_ops->
					set_prof_type(ccu_profiler,
						param->count_type);
				ccu_profiler->ccu_gen_prof_ops->
					set_prof_policy(ccu_profiler,
						param->val);
				break;

	case CCU_PROF_CLK_REQ:
				ccu_profiler->ccu_gen_prof_ops->
					set_prof_type(ccu_profiler,
						param->count_type);
				ccu_profiler->ccu_gen_prof_ops->
					set_clkreq_sel(ccu_profiler,
						param->sel, param->val);
				break;

	case CCU_PROF_ALWAYS_ON:
				ccu_profiler->ccu_gen_prof_ops->
					set_prof_type(ccu_profiler,
						param->count_type);
				break;

	default:
				profiler_dbg("Wrong type");
				ret = -EINVAL;
				goto err;
		}
	ret = profiler->ops->start(profiler, 1);
	profiler->start_time = kona_hubtimer_get_counter();
	profiler->ops->get_counter(profiler,
				&profiler->running_time,
				&profiler->overflow);
	if (profiler->overflow)
		profiler->flags |= PROFILER_OVERFLOW;
err:
	return ret;
}

struct prof_ops ccu_prof_ops = {
	.init = ccu_prof_init,
	.start = ccu_prof_start,
	.status = ccu_prof_status,
	.get_counter = ccu_prof_get_counter,
	.print = ccu_prof_print,
	.start_profiler = ccu_profiler_start,
};

struct gen_ccu_prof_ops gen_ccu_prof_ops = {
	.set_prof_type = ccu_prof_set_prof_type,
	.get_prof_type = ccu_prof_get_prof_type,
	.set_prof_policy = ccu_prof_set_policy,
	.get_prof_policy = ccu_prof_get_policy,
	.set_autogate_sel = ccu_prof_set_autogate_sel,
	.get_autogate_sel = ccu_prof_get_autogate_sel,
	.set_clkreq_sel = ccu_prof_set_clkreq_sel,
	.get_clkreq_sel = ccu_prof_get_clkreq_sel,
};

int ccu_profiler_register(struct ccu_profiler *ccu_profiler)
{
	struct dentry *dentry_dir;
	int err = 0;

	if (!ccu_profiler)
		return -EINVAL;
	if (!init)
		return -EPERM;

	INIT_LIST_HEAD(&ccu_profiler->profiler.node);
	ccu_profiler->profiler.ops = &ccu_prof_ops;
	ccu_profiler->ccu_gen_prof_ops = &gen_ccu_prof_ops;
	err = profiler_register(&ccu_profiler->profiler);
	if (err < 0)
		return err;

	dentry_dir = debugfs_create_dir(ccu_profiler->profiler.name,
			dentry_ccu_dir);
	if (!dentry_dir)
		goto err_out;

	if (!debugfs_create_file("start", S_IRUSR | S_IWUSR, dentry_dir,
		ccu_profiler, &ccu_prof_start_fops))
		goto err_out;

	if (!debugfs_create_file("prof_type", S_IRUSR | S_IWUSR, dentry_dir,
		ccu_profiler, &ccu_prof_counter_type_fops))
		goto err_out;
	if (!debugfs_create_file("policy", S_IRUSR | S_IWUSR, dentry_dir,
		ccu_profiler, &ccu_prof_policy_fops))
		goto err_out;
	if (!debugfs_create_file("autogate_sel0", S_IRUSR | S_IWUSR, dentry_dir,
		ccu_profiler, &ccu_prof_autogate_sel0_fops))
		goto err_out;
	if (!debugfs_create_file("autogate_sel1", S_IRUSR | S_IWUSR, dentry_dir,
		ccu_profiler, &ccu_prof_autogate_sel1_fops))
		goto err_out;
	if (!debugfs_create_file("clkreq_sel0", S_IRUSR | S_IWUSR, dentry_dir,
		ccu_profiler, &ccu_prof_clkreq_sel0_fops))
		goto err_out;
	if (!debugfs_create_file("clkreq_sel1", S_IRUSR | S_IWUSR, dentry_dir,
		ccu_profiler, &ccu_prof_clkreq_sel1_fops))
		goto err_out;
	if (!debugfs_create_file("counter", S_IRUGO, dentry_dir,
		ccu_profiler, &ccu_prof_counter_fops))
		goto err_out;

	profiler_dbg("ccu registered profiler %s\n",
		ccu_profiler->profiler.name);
	return 0;

err_out:
	profiler_dbg("Failed to create directory\n");
	if (dentry_dir)
		debugfs_remove_recursive(dentry_dir);
	profiler_unregister(&ccu_profiler->profiler);
	return err;
}
EXPORT_SYMBOL(ccu_profiler_register);

int ccu_profiler_unregister(struct ccu_profiler *ccu_profiler)
{
	if (!ccu_profiler)
		return -EINVAL;
	return profiler_unregister(&ccu_profiler->profiler);
}
EXPORT_SYMBOL(ccu_profiler_unregister);

int __init ccu_profiler_init(struct dentry *prof_root_dir)
{
	dentry_root_dir = prof_root_dir;
	dentry_ccu_dir = debugfs_create_dir(PROFILER_CCU_DIR_NAME,
			prof_root_dir);
	if (!dentry_ccu_dir)
		return -ENOMEM;
	init = true;
	return 0;
}
