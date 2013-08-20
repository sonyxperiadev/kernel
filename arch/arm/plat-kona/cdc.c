 /*****************************************************************************
 * Copyright 2013 Broadcom Corporation.  All rights reserved.
 *
 *	@file	arch/arm/plat-kona/cdc.c
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif

#include <plat/cdc.h>
#include <mach/rdb/brcm_rdb_cdc.h>
#include <mach/memory.h>
#include <mach/io_map.h>

#define cdc_dbg(level, args...) \
	do { \
		if (dbg_mask & level) { \
			pr_info(args); \
		} \
	} while (0)

#define CDC_CMD_REG_OFFSET(cpu) (CDC_CORE0_COMMAND_OFFSET + 4*(cpu))
#define CDC_CMD_STATUS_REG_OFFSET(cpu) \
	(CDC_CORE0_COMMAND_STATUS_OFFSET + 4*(cpu))

/*Should be remove once RDB is updated*/
#define DEBUG_BUS_STATUS_SHIFT	16
#define DEBUG_BUS_STATUS_MASK	(0xFFFF << 16)

static const u32 valid_cmd_status_map[CDC_CMD_MAX] = {
	[CDC_CMD_RED] = CDC_STATUS_RUN | CDC_STATUS_NRFD,
	[CDC_CMD_REDCAN] = CDC_STATUS_RFD | CDC_STATUS_CENE | CDC_STATUS_RFDLC,
	[CDC_CMD_CDCE] = CDC_STATUS_RFD | CDC_STATUS_RFDLC,
	[CDC_CMD_FDCE] = CDC_STATUS_RFDLC,
	[CDC_CMD_MDEC] = CDC_STATUS_RESFDM_SHORT | CDC_STATUS_RESFDM,
	[CDC_CMD_SDEC] = CDC_STATUS_RESFDS | CDC_STATUS_RESDFS_SHORT |
							CDC_STATUS_RESCD,
};

enum {
	CDC_LOG_ERR  = 1 << 0,
	CDC_LOG_WARN = 1 << 1,
	CDC_LOG_INIT = 1 << 2,
	CDC_LOG_FLOW = 1 << 3,
	CDC_LOG_INFO = 1 << 4,
};

static int dbg_mask = CDC_LOG_ERR | CDC_LOG_WARN | CDC_LOG_INIT;
module_param_named(dbg_mask, dbg_mask, int, S_IRUGO | S_IWUSR
			| S_IWGRP);



struct cdc {
	u32 flags;
	u32 nr_cpus;
	u32 base;
	spinlock_t lock;
	bool acp_active;
	bool timer_active;
#ifdef CONFIG_DEBUG_FS
	struct dentry *cdc_dir;
#endif
};

static struct cdc *cdc;

static bool cdc_is_cmd_allowed(u32 cmd, int cpu)
{

	if (cmd < CDC_CMD_RED || cmd >= CDC_CMD_MAX)
		return false;
	return !!(valid_cmd_status_map[cmd] & cdc_get_status_for_core(cpu));
}

int cdc_send_cmd_early(u32 cmd, int cpu)
{
	writel_relaxed(cmd, KONA_CDC_VA + CDC_CMD_REG_OFFSET(cpu));
	return 0;
}

int cdc_send_cmd(u32 cmd)
{
	int ret;
	int cpu = get_cpu();
	ret = cdc_send_cmd_for_core(cmd, cpu);
	if (ret)
		goto done;
	ret = cdc_get_status_for_core(cpu);
done:
	put_cpu();
	return ret;
}

int cdc_send_cmd_for_core(u32 cmd, int cpu)
{
	u32 val;
	if (!cdc ||
		cdc->nr_cpus <= cpu ||
		!cdc_is_cmd_allowed(cmd, cpu))
		return -EINVAL;

	val = (cmd << CDC_CORE0_COMMAND_CORE0_COMMAND_SHIFT) &
			CDC_CORE0_COMMAND_CORE0_COMMAND_MASK;
	writel_relaxed(val, cdc->base + CDC_CMD_REG_OFFSET(cpu));
	cdc_dbg(CDC_LOG_FLOW, "%s: %x wrtten to %x. CPU %d\n", __func__,
		val, cdc->base + CDC_CMD_REG_OFFSET(cpu), cpu);
	return 0;
}

int cdc_get_status_for_core(int cpu)
{
	int val;
	if (!cdc)
		return -EINVAL;
	val = readl_relaxed(cdc->base + CDC_CMD_STATUS_REG_OFFSET(cpu));
	cdc_dbg(CDC_LOG_FLOW, "%s: cpu%d status -  %x\n", __func__,
			cpu, val);
	return (val & CDC_CORE0_COMMAND_STATUS_CORE0_COMMAND_STATUS_MASK) >>
		CDC_CORE0_COMMAND_STATUS_CORE0_COMMAND_STATUS_SHIFT;
}


int cdc_acp_active(bool active)
{
	u32 val = !!active;
	if (!cdc)
		return -EINVAL;
	spin_lock(&cdc->lock);
	writel_relaxed(val, cdc->base + CDC_ACP_IS_ACTIVE_OFFSET);
	cdc->acp_active = val;
	spin_unlock(&cdc->lock);
	return 0;
}

int cdc_core_timer_in_use(bool in_use)
{
	u32 val = !!in_use;
	if (!cdc)
		return -EINVAL;
	spin_lock(&cdc->lock);
	writel_relaxed(val, cdc->base + CDC_CORE_TIMER_IN_USE_OFFSET);
	cdc->timer_active = val;
	spin_unlock(&cdc->lock);
	return 0;
}

int cdc_get_status()
{
	int ret;
	int cpu = get_cpu();
	ret = cdc_get_status_for_core(cpu);
	put_cpu();
	return ret;
}

int cdc_get_pwr_status()
{
	u32 val;
	if (!cdc)
		return -EINVAL;
	val = readl_relaxed(cdc->base + CDC_CDC_POWER_STATUS_OFFSET);
	return (val & CDC_CDC_POWER_STATUS_PWRCTLI_MASK) >>
		CDC_CDC_POWER_STATUS_PWRCTLI_SHIFT;
}

int cdc_set_pwr_status(u32 status)
{
	u32 val;
	if (!cdc)
		return -EINVAL;
	BUG_ON(status == CDC_PWR_RSVD ||
		status > CDC_PWR_DRMNT_L2_OFF);
	spin_lock(&cdc->lock);
	val = (status << CDC_CDC_POWER_STATUS_PWRCTLO_SHIFT) &
		CDC_CDC_POWER_STATUS_PWRCTLO_MASK;
	writel_relaxed(val, cdc->base + CDC_CDC_POWER_STATUS_OFFSET);
	spin_unlock(&cdc->lock);
	return 0;
}

int cdc_get_usr_reg()
{
	if (!cdc)
		return -EINVAL;
	return readl_relaxed(cdc->base + CDC_USER_REGISTER_OFFSET);
}

int cdc_set_usr_reg(u32 set_mask)
{
	if (!cdc)
		return -EINVAL;
	spin_lock(&cdc->lock);
	writel_relaxed(set_mask, cdc->base + CDC_USER_REGISTER_SET_OFFSET);
	spin_unlock(&cdc->lock);
	return 0;
}
int cdc_clr_usr_reg(u32 clr_mask)
{
	if (!cdc)
		return -EINVAL;
	spin_lock(&cdc->lock);
	writel_relaxed(clr_mask, cdc->base + CDC_USER_REGISTER_CLR_OFFSET);
	spin_unlock(&cdc->lock);
	return 0;

}

int cdc_set_fsm_ctrl(u32 fsm_ctrl)
{
	if (!cdc)
		return -EINVAL;
	spin_lock(&cdc->lock);
	fsm_ctrl &= 0xF;
	writel_relaxed(fsm_ctrl, cdc->base + CDC_FSM_CONTROL_OFFSET);
	spin_unlock(&cdc->lock);
	return 0;
}
int cdc_get_override(u32 type)
{

#if 0
	u32 shift;
	u32 mask;

	if (!cdc)
		return -EINVAL;
	switch (type) {
	case WAIT_IDLE_TIMEOUT:
	case WAIT_IDLE_TIMEOUT_STATUS:
	case STBYWFIL2_STATUS:
	case STBYWFIL2_OVERRIDE:
	case ARMSYSIDLE_TIMER:
	case IS_IDLE_OVERRIDE:
		mask = CDC_IS_IDLE_OVERRIDE_IS_IDLE_OVERRIDE_MASK;
		shift = CDC_IS_IDLE_OVERRIDE_IS_IDLE_OVERRIDE_SHIFT;
		break;

	default:
		return -EINVAL;
	};
#endif
	return -EINVAL;
}

int cdc_set_override(u32 type, u32 val)
{
	u32 reg;
	u32 mask;
	u32 shift;

	if (!cdc)
		return -EINVAL;
	switch (type) {
	case WAIT_IDLE_TIMEOUT:
		mask = CDC_IS_IDLE_OVERRIDE_WAIT_IDLE_TIMEOUT_MASK;
		shift = CDC_IS_IDLE_OVERRIDE_WAIT_IDLE_TIMEOUT_SHIFT;
		break;
	case STBYWFIL2_OVERRIDE:
		mask = CDC_IS_IDLE_OVERRIDE_STBYWFIL2_OVERRIDE_MASK;
		shift = CDC_IS_IDLE_OVERRIDE_STBYWFIL2_OVERRIDE_SHIFT;
		break;
	case ARMSYSIDLE_TIMER:
		mask = CDC_IS_IDLE_OVERRIDE_ARMSYSIDLE_TIMER_MASK;
		shift = CDC_IS_IDLE_OVERRIDE_ARMSYSIDLE_TIMER_SHIFT;
		break;
	case IS_IDLE_OVERRIDE:
		mask = CDC_IS_IDLE_OVERRIDE_IS_IDLE_OVERRIDE_MASK;
		shift = CDC_IS_IDLE_OVERRIDE_IS_IDLE_OVERRIDE_SHIFT;
		break;

	default:
		return -EINVAL;
	};

	spin_lock(&cdc->lock);
	reg = readl_relaxed(cdc->base +
					CDC_IS_IDLE_OVERRIDE_OFFSET);
	reg &= ~mask;
	reg |= (val << shift) & mask;
	writel_relaxed(reg,
			cdc->base + CDC_IS_IDLE_OVERRIDE_OFFSET);
	spin_unlock(&cdc->lock);

	return 0;
}

int cdc_dbg_bus_sel(u32 sel)
{
	u32 reg;
	if (!cdc)
		return -EINVAL;
	reg = readl_relaxed(cdc->base + CDC_CONFIG_OFFSET);
	reg &= ~CDC_CONFIG_DEBUG_BUS_SELECTION_MASK;
	reg |= (sel << CDC_CONFIG_DEBUG_BUS_SELECTION_SHIFT) &
			CDC_CONFIG_DEBUG_BUS_SELECTION_MASK;
	writel_relaxed(reg, cdc->base + CDC_CONFIG_OFFSET);
	return 0;
}

int cdc_get_dbg_bus_val()
{
	u32 reg;
	if (!cdc)
		return -EINVAL;
	reg = readl_relaxed(cdc->base + CDC_CONFIG_OFFSET);
	reg &= DEBUG_BUS_STATUS_MASK;
	reg >>= DEBUG_BUS_STATUS_SHIFT;
	return (int)reg;
}

int cdc_assert_reset_in_state(u32 states)
{
	u32 reg;
	if (!cdc)
		return -EINVAL;

	spin_lock(&cdc->lock);
	reg = readl_relaxed(cdc->base +
		CDC_RESET_STATE_ENABLE_OFFSET);
	reg |= states;
	writel_relaxed(states,
	cdc->base + CDC_RESET_STATE_ENABLE_OFFSET);
	spin_unlock(&cdc->lock);

	return 0;
}

int cdc_enable_isolation_in_state(u32 states)
{
	u32 reg;
	if (!cdc)
		return -EINVAL;

	spin_lock(&cdc->lock);
	reg = readl_relaxed(cdc->base +
		CDC_ISOLATION_STATE_ENABLE_OFFSET);
	reg |= states;
	writel_relaxed(states,
		cdc->base + CDC_ISOLATION_STATE_ENABLE_OFFSET);
	spin_unlock(&cdc->lock);

	return 0;
}

int cdc_assert_cdcbusy_in_state(u32 states)
{
	u32 reg;
	if (!cdc)
		return -EINVAL;

	spin_lock(&cdc->lock);
	reg = readl_relaxed(cdc->base +
		CDC_CDCBUSY_STATE_ENABLE_OFFSET);
	reg |= states;
	writel_relaxed(states,
		cdc->base + CDC_CDCBUSY_STATE_ENABLE_OFFSET);
	spin_unlock(&cdc->lock);

	return 0;
}

int cdc_set_reset_counter(int type, u32 val)
{
	u32 reg;
	u32 mask;
	u32 shift;

	if (!cdc)
		return -EINVAL;

	switch (type) {
	case FD_RESET_TIMER:
		shift = CDC_RESET_COUNTER_VALUES_FD_RESET_TIMER_SHIFT;
		mask = CDC_RESET_COUNTER_VALUES_FD_RESET_TIMER_MASK;
		break;

	case CD_RESET_TIMER:
		shift = CDC_RESET_COUNTER_VALUES_CD_RESET_TIMER_SHIFT;
		mask = CDC_RESET_COUNTER_VALUES_CD_RESET_TIMER_MASK;
		break;

	default:
		BUG();
	}

	spin_lock(&cdc->lock);
	reg = readl_relaxed(cdc->base +
		CDC_RESET_COUNTER_VALUES_OFFSET);
	reg &= ~mask;
	reg |= (val << shift) & mask;
	writel_relaxed(reg,
		cdc->base + CDC_RESET_COUNTER_VALUES_OFFSET);

	spin_unlock(&cdc->lock);

	return 0;
}

int cdc_set_switch_counter(int type, u32 val)
{
	u32 reg;
	u32 mask;
	u32 shift;

	if (!cdc)
		return -EINVAL;

	switch (type) {
	case WEAK_SWITCH_TIMER:
		shift = CDC_SWITCH_COUNTER_VALUES_WEAK_SWITCH_TIMER_SHIFT;
		mask = CDC_SWITCH_COUNTER_VALUES_WEAK_SWITCH_TIMER_MASK;
		break;

	case STRONG_SWITCH_TIMER:
		shift = CDC_SWITCH_COUNTER_VALUES_STRONG_SWITCH_TIMER_SHIFT;
		mask = CDC_SWITCH_COUNTER_VALUES_STRONG_SWITCH_TIMER_MASK;
		break;

	default:
		BUG();
	}

	spin_lock(&cdc->lock);
	reg = readl_relaxed(cdc->base +
		CDC_SWITCH_COUNTER_VALUES_OFFSET);
	reg &= ~mask;
	reg |= (val << shift) & mask;
	writel_relaxed(reg,
		cdc->base + CDC_SWITCH_COUNTER_VALUES_OFFSET);
	spin_unlock(&cdc->lock);
	return 0;
}

int cdc_master_clk_gating_en(bool en)
{
	u32 reg;
	u32 mask;
	if (!cdc)
		return -EINVAL;

	spin_lock(&cdc->lock);
	reg = readl_relaxed(cdc->base +
		CDC_ENABLE_MASTER_CLK_GATING_OFFSET);
	mask = CDC_ENABLE_MASTER_CLK_GATING_ENABLE_MASTER_CLK_GATING_MASK;
	if (en)
		reg |= mask;
	else
		reg &= ~mask;
	writel_relaxed(reg,
		cdc->base + CDC_ENABLE_MASTER_CLK_GATING_OFFSET);
	spin_unlock(&cdc->lock);
	return 0;
}



#ifdef CONFIG_DEBUG_FS

static int cdc_dbg_get_status(void *data, u64 *val)
{
	u32 cpu = (u32) data;
	BUG_ON(cpu >= cdc->nr_cpus);
	*val = cdc_get_status_for_core(cpu);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(cdc_dbg_get_status_ops, cdc_dbg_get_status, NULL,
			"%llu\n");

static int cdc_dbg_get_dbg_bus(void *data, u64 *val)
{
	*val = cdc_get_dbg_bus_val();
	return 0;
}

static int cdc_dbg_set_dbg_bus(void *data, u64 val)
{
	return cdc_dbg_bus_sel((u32)val);
}

DEFINE_SIMPLE_ATTRIBUTE(cdc_dbg_bus_ops, cdc_dbg_get_dbg_bus,
	cdc_dbg_set_dbg_bus, "%llx\n");


static int cdc_dbg_init(struct cdc *cdc_info)
{
	int i;
	char buf[10];
	cdc_info->cdc_dir = debugfs_create_dir("cdc", 0);

	if (!cdc_info->cdc_dir)
		return -EINVAL;

	for (i = 0; i < cdc_info->nr_cpus; i++) {
		sprintf(buf, "cpu%d_status", i);
		if (!debugfs_create_file(buf,
				S_IRUSR, cdc_info->cdc_dir, (void *)i,
				&cdc_dbg_get_status_ops))
			goto err;
	}
	if (!debugfs_create_u32("dbg_mask", S_IRUSR | S_IWUSR,
		cdc_info->cdc_dir, (u32 *)dbg_mask))
			goto err;

	if (!debugfs_create_file("dbg_bus",
			S_IRUSR | S_IWUSR , cdc_info->cdc_dir, NULL,
			&cdc_dbg_bus_ops))
		goto err;
	return 0;
err:
	debugfs_remove_recursive(cdc_info->cdc_dir);
	return -EINVAL;
}

#endif /*CONFIG_DEBUG_FS*/

static int cdc_probe(struct platform_device *pdev)
{
	int i;
	struct resource *mem_res;
	struct cdc_pdata *pdata = pdev->dev.platform_data;

	cdc_dbg(CDC_LOG_INIT, "%s\n", __func__);

	if (!pdata) {
		cdc_dbg(CDC_LOG_ERR,
				"%s : invalid paltform data !!\n", __func__);
		return -EPERM;
	}
	/* Get CDC base address*/
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_res) {
		cdc_dbg(CDC_LOG_ERR,
				"%s : IORESOURCE_MEM !!\n", __func__);
		return -ENXIO;
	}

	cdc = kzalloc(sizeof(struct cdc), GFP_KERNEL);
	if (!cdc)
		return -ENOMEM;

	cdc->flags = pdata->flags;
	cdc->nr_cpus = pdata->nr_cpus;
	cdc->base = HW_IO_PHYS_TO_VIRT(mem_res->start);
	spin_lock_init(&cdc->lock);

	for (i = 0; i < cdc->nr_cpus; i++)
		cdc_dbg(CDC_LOG_INIT, "%s: cpu%d status = %x\n", __func__,
			i, cdc_get_status_for_core(i));

#ifdef CONFIG_DEBUG_FS
	cdc_dbg_init(cdc);
#endif
	return 0;
}

static int __devexit cdc_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver cdc_driver = {
	.probe = cdc_probe,
	.remove = __devexit_p(cdc_remove),
	.driver = {.name = "brcm-cdc",},
};

static int __init cdc_init(void)
{
	return platform_driver_register(&cdc_driver);
}

subsys_initcall_sync(cdc_init);

static void __exit cdc_exit(void)
{
	platform_driver_unregister(&cdc_driver);

	if (cdc) {
#ifdef CONFIG_DEBUG_FS
		if (cdc->cdc_dir)
			debugfs_remove_recursive(cdc->cdc_dir);
#endif
		kfree(cdc);
	}
}

module_exit(cdc_exit);

MODULE_ALIAS("platform:brcm_cdc");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Broacom CDC Driver");
