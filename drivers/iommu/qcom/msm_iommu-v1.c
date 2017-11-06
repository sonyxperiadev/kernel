/* Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/iommu.h>
#include <linux/clk.h>
#include <linux/scatterlist.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_iommu.h>
#include <linux/regulator/consumer.h>
#include <linux/notifier.h>
#include <linux/iopoll.h>
#include <asm/sizes.h>
#include <linux/dma-iommu.h>

#include <linux/amba/bus.h>
#include <soc/qcom/secure_buffer.h>
#include <linux/msm-bus.h>

#include "qcom_iommu.h"
#include "msm_iommu_hw-v1.h"
#include "msm_iommu_priv.h"
#include "msm_iommu_perfmon.h"
#include "msm_iommu_pagetable.h"

#ifdef CONFIG_IOMMU_LPAE
/* bitmap of the page sizes currently supported */
#define MSM_IOMMU_PGSIZES	(SZ_4K | SZ_64K | SZ_2M | SZ_32M | SZ_1G)
#define IS_CB_FORMAT_LONG	1
#else
/* bitmap of the page sizes currently supported */
#define MSM_IOMMU_PGSIZES	(SZ_4K | SZ_64K | SZ_1M | SZ_16M)
#define IS_CB_FORMAT_LONG	0
#endif

#define IOMMU_USEC_STEP		10
#define IOMMU_USEC_TIMEOUT	500

/* Max ASID width is 8-bit */
#define MAX_ASID	0xff

struct msm_iommu_master {
	struct list_head list;
	unsigned int ctx_num;
	struct device *dev;
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
};

static LIST_HEAD(iommu_masters);

static DEFINE_MUTEX(msm_iommu_lock);
static DEFINE_SPINLOCK(msm_iommu_spin_lock);

struct dump_regs_tbl_entry dump_regs_tbl[MAX_DUMP_REGS];

static int __enable_regulators(struct msm_iommu_drvdata *drvdata)
{
	++drvdata->powered_on;

	return 0;
}

static void __disable_regulators(struct msm_iommu_drvdata *drvdata)
{
	--drvdata->powered_on;
}

static int apply_bus_vote(struct msm_iommu_drvdata *drvdata, unsigned int vote)
{
	int ret = 0;

	if (drvdata->bus_client) {
		ret = msm_bus_scale_client_update_request(drvdata->bus_client,
							  vote);
		if (ret)
			pr_err("%s: Failed to vote for bus: %d\n", __func__,
				vote);
	}
	return ret;
}

int __enable_clocks(struct msm_iommu_drvdata *drvdata)
{
	int ret;

	ret = clk_enable(drvdata->iface);
	if (ret)
		return ret;

	ret = clk_enable(drvdata->core);
	if (ret)
		goto err;

	return 0;

err:
	clk_disable(drvdata->iface);
	return ret;
}

void __disable_clocks(struct msm_iommu_drvdata *drvdata)
{
	clk_disable(drvdata->core);
	clk_disable(drvdata->iface);
}

static void _iommu_lock_acquire(unsigned int need_extra_lock)
{
	mutex_lock(&msm_iommu_lock);
}

static void _iommu_lock_release(unsigned int need_extra_lock)
{
	mutex_unlock(&msm_iommu_lock);
}

struct iommu_access_ops iommu_access_ops_v1 = {
	.iommu_power_on = __enable_regulators,
	.iommu_power_off = __disable_regulators,
	.iommu_bus_vote = apply_bus_vote,
	.iommu_clk_on = __enable_clocks,
	.iommu_clk_off = __disable_clocks,
	.iommu_lock_acquire = _iommu_lock_acquire,
	.iommu_lock_release = _iommu_lock_release,
};

static ATOMIC_NOTIFIER_HEAD(msm_iommu_notifier_list);

void msm_iommu_register_notify(struct notifier_block *nb)
{
	atomic_notifier_chain_register(&msm_iommu_notifier_list, nb);
}
EXPORT_SYMBOL(msm_iommu_register_notify);

#ifdef CONFIG_MSM_IOMMU_VBIF_CHECK

#define VBIF_XIN_HALT_CTRL0 0x200
#define VBIF_XIN_HALT_CTRL1 0x204
#define VBIF_AXI_HALT_CTRL0 0x208
#define VBIF_AXI_HALT_CTRL1 0x20C

static void __halt_vbif_xin(void __iomem *vbif_base)
{
	pr_err("Halting VBIF_XIN\n");
	writel_relaxed(0xFFFFFFFF, vbif_base + VBIF_XIN_HALT_CTRL0);
}

static void __dump_vbif_state(void __iomem *base, void __iomem *vbif_base)
{
	unsigned int reg_val;

	reg_val = readl_relaxed(base + MICRO_MMU_CTRL);
	pr_err("Value of SMMU_IMPLDEF_MICRO_MMU_CTRL = 0x%x\n", reg_val);

	reg_val = readl_relaxed(vbif_base + VBIF_XIN_HALT_CTRL0);
	pr_err("Value of VBIF_XIN_HALT_CTRL0 = 0x%x\n", reg_val);
	reg_val = readl_relaxed(vbif_base + VBIF_XIN_HALT_CTRL1);
	pr_err("Value of VBIF_XIN_HALT_CTRL1 = 0x%x\n", reg_val);
	reg_val = readl_relaxed(vbif_base + VBIF_AXI_HALT_CTRL0);
	pr_err("Value of VBIF_AXI_HALT_CTRL0 = 0x%x\n", reg_val);
	reg_val = readl_relaxed(vbif_base + VBIF_AXI_HALT_CTRL1);
	pr_err("Value of VBIF_AXI_HALT_CTRL1 = 0x%x\n", reg_val);
}

static int __check_vbif_state(struct msm_iommu_drvdata const *drvdata)
{
	int ret = 0;

	if (drvdata->vbif_base) {
		__dump_vbif_state(drvdata->base, drvdata->vbif_base);
		__halt_vbif_xin(drvdata->vbif_base);
		__dump_vbif_state(drvdata->base, drvdata->vbif_base);
	} else {
		pr_err("%s: failed to get vbif state\n", __func__);
		ret = -ENOMEM;
	}
	return ret;
}

static void check_halt_state(struct msm_iommu_drvdata const *drvdata)
{
	int res;
	unsigned int val;
	void __iomem *base = drvdata->base;
	char const *name = drvdata->name;

	pr_err("Timed out waiting for IOMMU halt to complete for %s\n", name);
	res = __check_vbif_state(drvdata);
	if (res)
		BUG();

	pr_err("Checking if IOMMU halt completed for %s\n", name);

	res = readl_poll_timeout_atomic(GLB_REG(MICRO_MMU_CTRL, base), val,
			(val & MMU_CTRL_IDLE) == MMU_CTRL_IDLE, 0, 5, 1000000);

	if (res) {
		pr_err("Timed out (again) waiting for IOMMU halt to complete for %s\n",
			name);
	} else {
		pr_err("IOMMU halt completed. VBIF FIFO most likely not getting drained by master\n");
	}
	BUG();
}

static void check_tlb_sync_state(struct msm_iommu_drvdata const *drvdata,
				int ctx, struct msm_iommu_priv *priv)
{
	int res;
	unsigned int val;
	void __iomem *base = drvdata->cb_base;
	char const *name = drvdata->name;

	pr_err("Timed out waiting for TLB SYNC to complete for %s (client: %s)\n",
		name, priv->client_name);
	atomic_notifier_call_chain(&msm_iommu_notifier_list, TLB_SYNC_TIMEOUT,
				(void *) priv->client_name);
	res = __check_vbif_state(drvdata);
	if (res)
		BUG();

	pr_err("Checking if TLB sync completed for %s\n", name);

	res = readl_poll_timeout_atomic(CTX_REG(CB_TLBSTATUS, base, ctx), val,
				(val & CB_TLBSTATUS_SACTIVE) == 0, 5, 1000000);
	if (res) {
		pr_err("Timed out (again) waiting for TLB SYNC to complete for %s\n",
			name);
	} else {
		pr_err("TLB Sync completed. VBIF FIFO most likely not getting drained by master\n");
	}
	BUG();
}

#else

/*
 * For targets without VBIF or for targets with the VBIF check disabled
 * we directly just crash to capture the issue
 */
static void check_halt_state(struct msm_iommu_drvdata const *drvdata)
{
	BUG();
}

static void check_tlb_sync_state(struct msm_iommu_drvdata const *drvdata,
				int ctx, struct msm_iommu_priv *priv)
{
	BUG();
}

#endif

void iommu_halt(struct msm_iommu_drvdata const *iommu_drvdata)
{
	void __iomem *base = iommu_drvdata->base;
	unsigned int val;
	int res;

	if (!iommu_drvdata->halt_enabled)
		return;

	SET_MICRO_MMU_CTRL_HALT_REQ(base, 1);
	res = readl_poll_timeout(GLB_REG(MICRO_MMU_CTRL, base), val,
			(val & MMU_CTRL_IDLE) == MMU_CTRL_IDLE, 0, 5000000);
	if (res)
		check_halt_state(iommu_drvdata);

	/* Ensure device is idle before continuing */
	mb();
}

void iommu_resume(const struct msm_iommu_drvdata *iommu_drvdata)
{
	if (!iommu_drvdata->halt_enabled)
		return;

	/* Ensure transactions have completed before releasing the halt */
	mb();

	SET_MICRO_MMU_CTRL_HALT_REQ(iommu_drvdata->base, 0);

	/*
	 * Ensure write is complete before continuing to ensure
	 * we don't turn off clocks while transaction is still
	 * pending.
	 */
	mb();
}

static inline bool is_domain_dynamic(struct msm_iommu_priv *priv)
{
	return (priv->attributes & (1 << DOMAIN_ATTR_DYNAMIC));
}

static void __sync_tlb(struct msm_iommu_drvdata *iommu_drvdata, int ctx,
		       struct msm_iommu_priv *priv)
{
	void __iomem *base = iommu_drvdata->cb_base;
	unsigned int val;
	unsigned int res;

	SET_TLBSYNC(base, ctx, 0);
	/* No barrier needed due to read dependency */

	res = readl_poll_timeout_atomic(CTX_REG(CB_TLBSTATUS, base, ctx), val,
				(val & CB_TLBSTATUS_SACTIVE) == 0, 5, 1000000);
	if (res)
		check_tlb_sync_state(iommu_drvdata, ctx, priv);
}

static int __flush_iotlb(struct iommu_domain *domain)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	struct msm_iommu_priv *base_priv;
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	int ret = 0;

	/*
	 * Context banks are properly attached to base domain and not dynamic
	 * domains. So, we must get the base domain and CBs attached to it
	 * for TLB invalidation.
	 */
	if (is_domain_dynamic(priv)) {
		if (!priv->base)
			return 0;

		base_priv = to_msm_priv(priv->base);
	} else {
		base_priv = priv;
	}

	list_for_each_entry(ctx_drvdata, &base_priv->list_attached, attached_elm) {
		BUG_ON(!ctx_drvdata->pdev || !ctx_drvdata->pdev->dev.parent);

		iommu_drvdata = dev_get_drvdata(ctx_drvdata->pdev->dev.parent);
		BUG_ON(!iommu_drvdata);

		ret = __enable_clocks(iommu_drvdata);
		if (ret)
			goto fail;

		SET_TLBIASID(iommu_drvdata->cb_base, ctx_drvdata->num,
			     priv->asid);
		__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);
		__disable_clocks(iommu_drvdata);
	}
fail:
	return ret;
}

/*
 * May only be called for non-secure iommus
 */
static void __reset_iommu(struct msm_iommu_drvdata *iommu_drvdata)
{
	void __iomem *base = iommu_drvdata->base;
	int i, smt_size, res;
	unsigned long val;

	/* SMMU_ACR is an implementation defined register.
	 * Resetting is not required for some implementation.
	 */
	if (iommu_drvdata->model != MMU_500)
		SET_ACR(base, 0);
	SET_CR2(base, 0);
	SET_GFAR(base, 0);
	SET_GFSRRESTORE(base, 0);

	/* Invalidate the entire non-secure TLB */
	SET_TLBIALLNSNH(base, 0);
	SET_TLBGSYNC(base, 0);
	res = readl_poll_timeout(GLB_REG(TLBGSTATUS, base), val,
				(val & TLBGSTATUS_GSACTIVE) == 0, 0, 5000000);
	if (res)
		BUG();

	smt_size = GET_IDR0_NUMSMRG(base);

	for (i = 0; i < smt_size; i++)
		SET_SMR_VALID(base, i, 0);

	mb();
}

static void __reset_iommu_secure(struct msm_iommu_drvdata *iommu_drvdata)
{
	void __iomem *base = iommu_drvdata->base;

	if (iommu_drvdata->model != MMU_500)
		SET_NSACR(base, 0);
	SET_NSCR2(base, 0);
	SET_NSGFAR(base, 0);
	SET_NSGFSRRESTORE(base, 0);
	mb();
}

static void __program_iommu_secure(struct msm_iommu_drvdata *iommu_drvdata)
{
	void __iomem *base = iommu_drvdata->base;

	if (iommu_drvdata->model == MMU_500) {
		SET_NSACR_SMTNMC_BPTLBEN(base, 1);
		SET_NSACR_MMUDIS_BPTLBEN(base, 1);
		SET_NSACR_S2CR_BPTLBEN(base, 1);
	}
	SET_NSCR0_SMCFCFG(base, 1);
	SET_NSCR0_USFCFG(base, 1);
	SET_NSCR0_STALLD(base, 1);
	SET_NSCR0_GCFGFIE(base, 1);
	SET_NSCR0_GCFGFRE(base, 1);
	SET_NSCR0_GFIE(base, 1);
	SET_NSCR0_GFRE(base, 1);
	SET_NSCR0_CLIENTPD(base, 0);
}

/*
 * May only be called for non-secure iommus
 */
static void __program_iommu(struct msm_iommu_drvdata *drvdata)
{
	__reset_iommu(drvdata);

	if (!msm_iommu_get_scm_call_avail())
		__reset_iommu_secure(drvdata);

	if (drvdata->model == MMU_500) {
		SET_ACR_SMTNMC_BPTLBEN(drvdata->base, 1);
		SET_ACR_MMUDIS_BPTLBEN(drvdata->base, 1);
		SET_ACR_S2CR_BPTLBEN(drvdata->base, 1);
	}
	SET_CR0_SMCFCFG(drvdata->base, 1);
	SET_CR0_USFCFG(drvdata->base, 1);
	SET_CR0_STALLD(drvdata->base, 1);
	SET_CR0_GCFGFIE(drvdata->base, 1);
	SET_CR0_GCFGFRE(drvdata->base, 1);
	SET_CR0_GFIE(drvdata->base, 1);
	SET_CR0_GFRE(drvdata->base, 1);
	SET_CR0_CLIENTPD(drvdata->base, 0);

	if (!msm_iommu_get_scm_call_avail())
		__program_iommu_secure(drvdata);

	if (drvdata->smmu_local_base)
		writel_relaxed(0xFFFFFFFF,
			       drvdata->smmu_local_base + SMMU_INTR_SEL_NS);

	mb(); /* Make sure writes complete before returning */
}

void program_iommu_bfb_settings(void __iomem *base,
			const struct msm_iommu_bfb_settings *bfb_settings)
{
	unsigned int i;

	if (bfb_settings)
		for (i = 0; i < bfb_settings->length; i++)
			SET_GLOBAL_REG(base, bfb_settings->regs[i],
				       bfb_settings->data[i]);

	/* Make sure writes complete before returning */
	mb();
}

static void __reset_context(struct msm_iommu_drvdata *iommu_drvdata, int ctx)
{
	void __iomem *base = iommu_drvdata->cb_base;

	/* Don't set ACTLR to zero because if context bank is in
	 * bypass mode (say after iommu_detach), still this ACTLR
	 * value matters for micro-TLB caching.
	 */
	if (iommu_drvdata->model != MMU_500)
		SET_ACTLR(base, ctx, 0);
	SET_FAR(base, ctx, 0);
	SET_FSRRESTORE(base, ctx, 0);
	SET_NMRR(base, ctx, 0);
	SET_PAR(base, ctx, 0);
	SET_PRRR(base, ctx, 0);
	SET_SCTLR(base, ctx, 0);
	SET_TTBCR(base, ctx, 0);
	SET_TTBR0(base, ctx, 0);
	SET_TTBR1(base, ctx, 0);
	mb();
}

static void __release_smg(void __iomem *base)
{
	int i, smt_size;
	smt_size = GET_IDR0_NUMSMRG(base);

	/* Invalidate all SMGs */
	for (i = 0; i < smt_size; i++)
		if (GET_SMR_VALID(base, i))
			SET_SMR_VALID(base, i, 0);
}

#ifdef CONFIG_IOMMU_LPAE
static void msm_iommu_set_ASID(void __iomem *base, unsigned int ctx_num,
			       unsigned int asid)
{
	SET_CB_TTBR0_ASID(base, ctx_num, asid);
}
#else
static void msm_iommu_set_ASID(void __iomem *base, unsigned int ctx_num,
			       unsigned int asid)
{
	SET_CB_CONTEXTIDR_ASID(base, ctx_num, asid);
}
#endif

static void msm_iommu_assign_ASID(const struct msm_iommu_drvdata *iommu_drvdata,
				  struct msm_iommu_ctx_drvdata *curr_ctx,
				  struct msm_iommu_priv *priv)
{
	void __iomem *cb_base = iommu_drvdata->cb_base;

	/*
	 * Domain also keeps the ASID info separately. This is because with
	 * dynamic domain, each domain will have different ASID but their
	 * attached CB is the same
	 */
	priv->asid = curr_ctx->num;
	msm_iommu_set_ASID(cb_base, curr_ctx->num, curr_ctx->asid);
}

#ifdef CONFIG_IOMMU_LPAE
static void msm_iommu_setup_ctx(void __iomem *base, unsigned int ctx)
{
	SET_CB_TTBCR_EAE(base, ctx, 1); /* Extended Address Enable (EAE) */
}

static void msm_iommu_setup_memory_remap(void __iomem *base, unsigned int ctx)
{
	SET_CB_MAIR0(base, ctx, msm_iommu_get_mair0());
	SET_CB_MAIR1(base, ctx, msm_iommu_get_mair1());
}

static void msm_iommu_setup_pg_l2_redirect(void __iomem *base, unsigned int ctx)
{
	/*
	 * Configure page tables as inner-cacheable and shareable to reduce
	 * the TLB miss penalty.
	 */
	SET_CB_TTBCR_SH0(base, ctx, 3);		/* Inner shareable */
	SET_CB_TTBCR_ORGN0(base, ctx, 1);	/* outer cachable*/
	SET_CB_TTBCR_IRGN0(base, ctx, 1);	/* inner cachable*/
	SET_CB_TTBCR_T0SZ(base, ctx, 0);	/* 0GB-4GB */


	SET_CB_TTBCR_SH1(base, ctx, 3);		/* Inner shareable */
	SET_CB_TTBCR_ORGN1(base, ctx, 1);	/* outer cachable*/
	SET_CB_TTBCR_IRGN1(base, ctx, 1);	/* inner cachable*/
	SET_CB_TTBCR_T1SZ(base, ctx, 0);	/* TTBR1 not used */
}

static u64 get_full_ttbr0(struct msm_iommu_priv *priv)
{
	return (virt_to_phys(priv->pt.fl_table) |
			(priv->asid << CB_TTBR0_ASID_SHIFT));
}

#else

static void msm_iommu_setup_ctx(void __iomem *base, unsigned int ctx)
{
	/* Turn on TEX Remap */
	SET_CB_SCTLR_TRE(base, ctx, 1);
}

static void msm_iommu_setup_memory_remap(void __iomem *base, unsigned int ctx)
{
	SET_PRRR(base, ctx, msm_iommu_get_prrr());
	SET_NMRR(base, ctx, msm_iommu_get_nmrr());
}

static void msm_iommu_setup_pg_l2_redirect(void __iomem *base, unsigned int ctx)
{
	/* Configure page tables as inner-cacheable and shareable to reduce
	 * the TLB miss penalty.
	 */
	SET_CB_TTBR0_S(base, ctx, 1);
	SET_CB_TTBR0_NOS(base, ctx, 1);
	SET_CB_TTBR0_IRGN1(base, ctx, 0); /* WB, WA */
	SET_CB_TTBR0_IRGN0(base, ctx, 1);
	SET_CB_TTBR0_RGN(base, ctx, 1);   /* WB, WA */
}

static u64 get_full_ttbr0(struct msm_iommu_priv *priv)
{
	return virt_to_phys(priv->pt.fl_table);
}

#endif

static int program_m2v_table(struct device *dev, void __iomem *base)
{
	struct msm_iommu_ctx_drvdata *ctx_drvdata = dev_get_drvdata(dev);
	u32 *sids = ctx_drvdata->sids;
	u32 *sid_mask = ctx_drvdata->sid_mask;
	unsigned int ctx = ctx_drvdata->num;
	int num = 0, i, smt_size;
	int len = ctx_drvdata->nsid;

	smt_size = GET_IDR0_NUMSMRG(base);

	/* Program the M2V tables for this context */
	for (i = 0; i < len / sizeof(*sids); i++) {
		for (; num < smt_size; num++)
			if (GET_SMR_VALID(base, num) == 0)
				break;
		BUG_ON(num >= smt_size);

		SET_SMR_VALID(base, num, 1);
		SET_SMR_MASK(base, num, sid_mask[i]);
		SET_SMR_ID(base, num, sids[i]);

		SET_S2CR_N(base, num, 0);
		SET_S2CR_CBNDX(base, num, ctx);
		SET_S2CR_MEMATTR(base, num, 0x0A);
		/* Set security bit override to be Non-secure */
		SET_S2CR_NSCFG(base, num, 3);
	}

	return 0;
}

static void program_all_m2v_tables(struct msm_iommu_drvdata *iommu_drvdata)
{
	device_for_each_child(iommu_drvdata->dev, iommu_drvdata->base,
			      program_m2v_table);
}

static void __program_context(struct msm_iommu_drvdata *iommu_drvdata,
			      struct msm_iommu_ctx_drvdata *ctx_drvdata,
			      struct msm_iommu_priv *priv, bool is_secure,
			      bool program_m2v)
{
	phys_addr_t pn;
	void __iomem *base = iommu_drvdata->base;
	void __iomem *cb_base = iommu_drvdata->cb_base;
	unsigned int ctx = ctx_drvdata->num;
	phys_addr_t pgtable = __pa(priv->pt.fl_table);

	__reset_context(iommu_drvdata, ctx);
	msm_iommu_setup_ctx(cb_base, ctx);

	if (priv->pt.redirect)
		msm_iommu_setup_pg_l2_redirect(cb_base, ctx);

	msm_iommu_setup_memory_remap(cb_base, ctx);

	pn = pgtable >> CB_TTBR0_ADDR_SHIFT;
	SET_CB_TTBR0_ADDR(cb_base, ctx, pn);

	/* Enable context fault interrupt */
	SET_CB_SCTLR_CFIE(cb_base, ctx, 1);

	if (iommu_drvdata->model != MMU_500) {
		/* Redirect all cacheable requests to L2 slave port. */
		SET_CB_ACTLR_BPRCISH(cb_base, ctx, 1);
		SET_CB_ACTLR_BPRCOSH(cb_base, ctx, 1);
		SET_CB_ACTLR_BPRCNSH(cb_base, ctx, 1);
	}

	/* Enable private ASID namespace */
	SET_CB_SCTLR_ASIDPNE(cb_base, ctx, 1);

	if (!is_secure) {
		if (program_m2v)
			program_all_m2v_tables(iommu_drvdata);

		SET_CBAR_N(base, ctx, 0);

		/* Stage 1 Context with Stage 2 bypass */
		SET_CBAR_TYPE(base, ctx, 1);

		/* Route page faults to the non-secure interrupt */
		SET_CBAR_IRPTNDX(base, ctx, 1);

		/* Set VMID to non-secure HLOS */
		SET_CBAR_VMID(base, ctx, 3);

		/* Bypass is treated as inner-shareable */
		SET_CBAR_BPSHCFG(base, ctx, 2);

		/* Do not downgrade memory attributes */
		SET_CBAR_MEMATTR(base, ctx, 0x0A);

	}

	msm_iommu_assign_ASID(iommu_drvdata, ctx_drvdata, priv);

	/* Ensure that ASID assignment has completed before we use
	 * ASID for TLB invalidation. Here, mb() is required because
	 * both these registers are separated by more than 1KB. */
	mb();

	SET_TLBIASID(iommu_drvdata->cb_base, ctx_drvdata->num,
					ctx_drvdata->asid);
	__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);

	/* Enable the MMU */
	SET_CB_SCTLR_M(cb_base, ctx, 1);
	mb();
}

#ifdef CONFIG_IOMMU_PGTABLES_L2
#define INITIAL_REDIRECT_VAL	1
#else
#define INITIAL_REDIRECT_VAL	0
#endif

static struct msm_iommu_master *msm_iommu_find_master(struct device *dev)
{
	struct msm_iommu_master *master;
	bool found = false;

	list_for_each_entry(master, &iommu_masters, list) {
		if (master && master->dev == dev) {
			found = true;
			break;
		}
	}

	if (found) {
		dev_dbg(dev, "found master %s with ctx:%d\n",
			dev_name(master->dev),
			master->ctx_num);
		return master;
	}

	return ERR_PTR(-ENODEV);
}

static struct iommu_domain *msm_iommu_domain_alloc(unsigned type)
{
	struct msm_iommu_priv *priv;
	int ret;

	if (type != IOMMU_DOMAIN_UNMANAGED && type != IOMMU_DOMAIN_DMA)
		return NULL;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return NULL;

	priv->pt.redirect = INITIAL_REDIRECT_VAL;

	INIT_LIST_HEAD(&priv->list_attached);

	ret = msm_iommu_pagetable_alloc(&priv->pt);
	if (ret) {
		kfree(priv);
		return NULL;
	}

	if (type == IOMMU_DOMAIN_DMA) {
		ret = iommu_get_dma_cookie(&priv->domain);
		if (ret)
			goto err;
	}

	return &priv->domain;

err:
	msm_iommu_pagetable_free(&priv->pt);
	kfree(priv);
	return NULL;
}

static void msm_iommu_domain_free(struct iommu_domain *domain)
{
	struct msm_iommu_priv *priv;

	mutex_lock(&msm_iommu_lock);
	priv = to_msm_priv(domain);
	if (priv)
		msm_iommu_pagetable_free(&priv->pt);

	if (domain->type == IOMMU_DOMAIN_DMA)
		iommu_put_dma_cookie(domain);

	kfree(priv);
	mutex_unlock(&msm_iommu_lock);
}

static int msm_iommu_dynamic_attach(struct iommu_domain *domain,
				struct msm_iommu_drvdata *iommu_drvdata,
				struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	int ret;
	struct msm_iommu_priv *priv;

	priv = to_msm_priv(domain);

	/* Check if the domain is already attached or not */
	if (priv->asid < MAX_ASID && priv->asid > 0)
		return -EBUSY;

	ret = idr_alloc_cyclic(&iommu_drvdata->asid_idr, priv,
			iommu_drvdata->ncb + 2, MAX_ASID + 1, GFP_KERNEL);

	if (ret < 0)
		return -ENOSPC;

	priv->asid = ret;
	priv->base = ctx_drvdata->attached_domain;

	/* Once the CB is dynamic, it is always dynamic */
	ctx_drvdata->dynamic = true;

	return 0;
}

static int msm_iommu_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	struct msm_iommu_priv *priv;
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	struct msm_iommu_ctx_drvdata *tmp_drvdata;
	struct msm_iommu_master *master;
	int ret = 0;
	int is_secure;
	bool secure_ctx;
	bool set_m2v = false;
	unsigned long flags;

	mutex_lock(&msm_iommu_lock);

	priv = to_msm_priv(domain);
	if (!priv || !dev) {
		ret = -EINVAL;
		goto unlock;
	}

	if (!(priv->client_name))
		priv->client_name = dev_name(dev);

	master = msm_iommu_find_master(dev);
	if (IS_ERR(master)) {
		/* if error use legacy api */
		iommu_drvdata = dev_get_drvdata(dev->parent);
		ctx_drvdata = dev_get_drvdata(dev);
	} else {
		iommu_drvdata = master->iommu_drvdata;
		ctx_drvdata = master->ctx_drvdata;
	}

	if (!iommu_drvdata || !ctx_drvdata) {
		ret = -EINVAL;
		goto unlock;
	}

	if (is_domain_dynamic(priv)) {
		ret = msm_iommu_dynamic_attach(domain,
				iommu_drvdata, ctx_drvdata);
		mutex_unlock(&msm_iommu_lock);
		return ret;
	}


	++ctx_drvdata->attach_count;

	if (ctx_drvdata->attach_count > 1)
		goto already_attached;

	spin_lock_irqsave(&msm_iommu_spin_lock, flags);
	if (!list_empty(&ctx_drvdata->attached_elm)) {
		ret = -EBUSY;
		spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);
		goto unlock;
	}

	list_for_each_entry(tmp_drvdata, &priv->list_attached, attached_elm)
		if (tmp_drvdata == ctx_drvdata) {
			ret = -EBUSY;
			spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);
			goto unlock;
		}

	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);

	is_secure = iommu_drvdata->sec_id != -1;

	ret = __enable_regulators(iommu_drvdata);
	if (ret)
		goto unlock;

	ret = apply_bus_vote(iommu_drvdata, 1);
	if (ret)
		goto unlock;

	ret = __enable_clocks(iommu_drvdata);
	if (ret) {
		__disable_regulators(iommu_drvdata);
		goto unlock;
	}

	/* We can only do this once */
	if (!iommu_drvdata->ctx_attach_count) {
		if (!is_secure) {
			iommu_halt(iommu_drvdata);
			__program_iommu(iommu_drvdata);
			iommu_resume(iommu_drvdata);
		} else {
			ret = msm_iommu_sec_program_iommu(iommu_drvdata,
							  ctx_drvdata);
			if (ret) {
				__disable_regulators(iommu_drvdata);
				goto unlock;
			}
		}
		program_iommu_bfb_settings(iommu_drvdata->base,
					   iommu_drvdata->bfb_settings);
		set_m2v = true;
	}

	secure_ctx = !!(ctx_drvdata->secure_context > 0);
	if (secure_ctx) {
		dev_dbg(dev, "Detected secure context.\n");
		goto add_domain;
	}

	iommu_halt(iommu_drvdata);

	__program_context(iommu_drvdata, ctx_drvdata, priv, is_secure, set_m2v);

	iommu_resume(iommu_drvdata);

	/* Ensure TLB is clear */
	if (iommu_drvdata->model != MMU_500) {
		SET_TLBIASID(iommu_drvdata->cb_base, ctx_drvdata->num,
			     ctx_drvdata->asid);
		__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);
	}

	__disable_clocks(iommu_drvdata);

add_domain:
	spin_lock_irqsave(&msm_iommu_spin_lock, flags);
	list_add(&(ctx_drvdata->attached_elm), &priv->list_attached);
	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);

	ctx_drvdata->attached_domain = domain;
	++iommu_drvdata->ctx_attach_count;

already_attached:
	mutex_unlock(&msm_iommu_lock);

	msm_iommu_attached(dev->parent);
	return ret;
unlock:
	mutex_unlock(&msm_iommu_lock);
	return ret;
}

static void msm_iommu_dynamic_detach(struct iommu_domain *domain,
				struct msm_iommu_drvdata *iommu_drvdata,
				struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	int ret;
	struct msm_iommu_priv *priv;

	priv = to_msm_priv(domain);
	if (ctx_drvdata->attach_count > 0) {
		ret = __enable_clocks(iommu_drvdata);
		if (ret)
			return;

		SET_TLBIASID(iommu_drvdata->cb_base, ctx_drvdata->num,
			     priv->asid);
		__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);
		__disable_clocks(iommu_drvdata);
	}

	BUG_ON(priv->asid == -1);

	idr_remove(&iommu_drvdata->asid_idr, priv->asid);

	priv->asid = (-1);
	priv->base = NULL;
}

static void msm_iommu_detach_dev(struct iommu_domain *domain,
				 struct device *dev)
{
	struct msm_iommu_priv *priv;
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	struct msm_iommu_master *master;
	int is_secure;
	int ret;
	unsigned long flags;

	if (!dev)
		return;

	msm_iommu_detached(dev->parent);

	mutex_lock(&msm_iommu_lock);
	priv = to_msm_priv(domain);
	if (!priv)
		goto unlock;

	master = msm_iommu_find_master(dev);
	if (IS_ERR(master)) {
		ret = PTR_ERR(master);
		goto unlock;
	}

	iommu_drvdata = master->iommu_drvdata;
	ctx_drvdata = master->ctx_drvdata;

	if (!iommu_drvdata || !ctx_drvdata)
		goto unlock;

	if (is_domain_dynamic(priv)) {
		msm_iommu_dynamic_detach(domain,
				iommu_drvdata, ctx_drvdata);
		mutex_unlock(&msm_iommu_lock);
		return;
	}

	if (!ctx_drvdata->attached_domain)
		goto unlock;

	--ctx_drvdata->attach_count;
	BUG_ON(ctx_drvdata->attach_count < 0);

	if (ctx_drvdata->attach_count > 0)
		goto unlock;

	ret = __enable_clocks(iommu_drvdata);
	if (ret)
		goto unlock;

	is_secure = iommu_drvdata->sec_id != -1;

	if (iommu_drvdata->model == MMU_500) {
		SET_TLBIASID(iommu_drvdata->cb_base, ctx_drvdata->num,
			     ctx_drvdata->asid);
		__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);
	}

	ctx_drvdata->asid = -1;

	__reset_context(iommu_drvdata, ctx_drvdata->num);

	/*
	 * Only reset the M2V tables on the very last detach */
	if (!is_secure && iommu_drvdata->ctx_attach_count == 1) {
		iommu_halt(iommu_drvdata);
		__release_smg(iommu_drvdata->base);
		iommu_resume(iommu_drvdata);
	}

	__disable_clocks(iommu_drvdata);

	apply_bus_vote(iommu_drvdata, 0);

	__disable_regulators(iommu_drvdata);

	spin_lock_irqsave(&msm_iommu_spin_lock, flags);
	list_del_init(&ctx_drvdata->attached_elm);
	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);

	ctx_drvdata->attached_domain = NULL;
	BUG_ON(iommu_drvdata->ctx_attach_count == 0);
	--iommu_drvdata->ctx_attach_count;
unlock:
	mutex_unlock(&msm_iommu_lock);
}

static int msm_iommu_map(struct iommu_domain *domain, unsigned long va,
			 phys_addr_t pa, size_t len, int prot)
{
	struct msm_iommu_priv *priv;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&msm_iommu_spin_lock, flags);

	priv = to_msm_priv(domain);
	if (!priv) {
		ret = -EINVAL;
		goto fail;
	}

	ret = msm_iommu_pagetable_map(&priv->pt, va, pa, len, prot);
	if (ret)
		goto fail;

fail:
	mutex_unlock(&msm_iommu_lock);
	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);
	return ret;
}

static size_t msm_iommu_unmap(struct iommu_domain *domain, unsigned long va,
			      size_t len)
{
	struct msm_iommu_priv *priv;
	int ret = -ENODEV;
	unsigned long flags;

	spin_lock_irqsave(&msm_iommu_spin_lock, flags);

	priv = to_msm_priv(domain);
	if (!priv)
		goto fail;

	ret = msm_iommu_pagetable_unmap(&priv->pt, va, len);
	if (ret < 0)
		goto fail;

	ret = __flush_iotlb(domain);

	msm_iommu_pagetable_free_tables(&priv->pt, va, len);
fail:
	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);

	/* the IOMMU API requires us to return how many bytes were unmapped */
	len = ret ? 0 : len;
	return len;
}

static size_t msm_iommu_map_sg(struct iommu_domain *domain, unsigned long iova,
			       struct scatterlist *sg, unsigned int nents,
			       int prot)
{
	struct msm_iommu_priv *priv;
	struct scatterlist *tmp;
	unsigned int len = 0;
	int ret, i;
	unsigned long flags;

	spin_lock_irqsave(&msm_iommu_spin_lock, flags);

	priv = to_msm_priv(domain);
	if (!priv) {
		ret = -EINVAL;
		goto fail;
	}

	for_each_sg(sg, tmp, nents, i)
		len += tmp->length;

	ret = msm_iommu_pagetable_map_range(&priv->pt, iova, sg, len, prot);
	if (ret < 0)
		goto fail;

	ret = len;

fail:
	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);
	return ret;
}
/*
static int msm_iommu_unmap_range(struct iommu_domain *domain, unsigned int va,
				 unsigned int len)
{
	struct msm_iommu_priv *priv;
	unsigned long flags;

	spin_lock_irqsave(&msm_iommu_spin_lock, flags);

	priv = to_msm_priv(domain);
	msm_iommu_pagetable_unmap_range(&priv->pt, va, len);

	__flush_iotlb(domain);

	msm_iommu_pagetable_free_tables(&priv->pt, va, len);
	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);

	return 0;
}
*/

static phys_addr_t msm_iommu_iova_to_phys(struct iommu_domain *domain,
					  phys_addr_t va)
{
	phys_addr_t ret = 0;
	unsigned long flags;

	mutex_lock(&msm_iommu_lock);

	spin_lock_irqsave(&msm_iommu_spin_lock, flags);
	ret = msm_iommu_iova_to_phys_soft(domain, va);
	spin_unlock_irqrestore(&msm_iommu_spin_lock, flags);

	mutex_unlock(&msm_iommu_lock);

	return ret;
}

static int msm_iommu_add_device(struct device *dev)
{
	struct iommu_group *group;

	group = iommu_group_get_for_dev(dev);
	if (IS_ERR(group))
		return PTR_ERR(group);

	return 0;
}

static void msm_iommu_remove_device(struct device *dev)
{
	iommu_group_remove_device(dev);
}

static struct iommu_group *msm_iommu_device_group(struct device *dev)
{
	struct msm_iommu_master *master;
	struct iommu_group *group;

	group = generic_device_group(dev);
	if (IS_ERR(group))
		return group;

	master = msm_iommu_find_master(dev);
	if (IS_ERR(master)) {
		iommu_group_put(group);
		return ERR_CAST(master);
	}

	iommu_group_set_iommudata(group, &master->ctx_drvdata, NULL);

	return group;
}

#ifdef CONFIG_IOMMU_LPAE
static inline void print_ctx_mem_attr_regs(struct msm_iommu_context_reg regs[])
{
	pr_err("MAIR0   = %08x    MAIR1   = %08x\n",
		 regs[DUMP_REG_MAIR0].val, regs[DUMP_REG_MAIR1].val);
}
#else
static inline void print_ctx_mem_attr_regs(struct msm_iommu_context_reg regs[])
{
	pr_err("PRRR   = %08x    NMRR   = %08x\n",
		 regs[DUMP_REG_PRRR].val, regs[DUMP_REG_NMRR].val);
}
#endif

void print_ctx_regs(struct msm_iommu_context_reg regs[])
{
	uint32_t fsr = regs[DUMP_REG_FSR].val;
	u64 ttbr;
	enum dump_reg iter;

	pr_err("FAR    = %016llx\n",
		COMBINE_DUMP_REG(
			regs[DUMP_REG_FAR1].val,
			regs[DUMP_REG_FAR0].val));
	pr_err("PAR    = %016llx\n",
		COMBINE_DUMP_REG(
			regs[DUMP_REG_PAR1].val,
			regs[DUMP_REG_PAR0].val));
	pr_err("FSR    = %08x [%s%s%s%s%s%s%s%s%s]\n", fsr,
			(fsr & 0x02) ? "TF " : "",
			(fsr & 0x04) ? "AFF " : "",
			(fsr & 0x08) ? "PF " : "",
			(fsr & 0x10) ? "EF " : "",
			(fsr & 0x20) ? "TLBMCF " : "",
			(fsr & 0x40) ? "TLBLKF " : "",
			(fsr & 0x80) ? "MHF " : "",
			(fsr & 0x40000000) ? "SS " : "",
			(fsr & 0x80000000) ? "MULTI " : "");

	pr_err("FSYNR0 = %08x    FSYNR1 = %08x\n",
		 regs[DUMP_REG_FSYNR0].val, regs[DUMP_REG_FSYNR1].val);

	ttbr = COMBINE_DUMP_REG(regs[DUMP_REG_TTBR0_1].val,
				regs[DUMP_REG_TTBR0_0].val);
	if (regs[DUMP_REG_TTBR0_1].valid)
		pr_err("TTBR0  = %016llx\n", ttbr);
	else
		pr_err("TTBR0  = %016llx (32b)\n", ttbr);

	ttbr = COMBINE_DUMP_REG(regs[DUMP_REG_TTBR1_1].val,
				regs[DUMP_REG_TTBR1_0].val);

	if (regs[DUMP_REG_TTBR1_1].valid)
		pr_err("TTBR1  = %016llx\n", ttbr);
	else
		pr_err("TTBR1  = %016llx (32b)\n", ttbr);

	pr_err("SCTLR  = %08x    ACTLR  = %08x\n",
		 regs[DUMP_REG_SCTLR].val, regs[DUMP_REG_ACTLR].val);
	pr_err("CBAR  = %08x    CBFRSYNRA  = %08x\n",
		regs[DUMP_REG_CBAR_N].val, regs[DUMP_REG_CBFRSYNRA_N].val);
	print_ctx_mem_attr_regs(regs);

	for (iter = DUMP_REG_FIRST; iter < MAX_DUMP_REGS; ++iter)
		if (!regs[iter].valid)
			pr_err("NOTE: Value actually unknown for %s\n",
				dump_regs_tbl[iter].name);
}

static void __print_ctx_regs(struct msm_iommu_drvdata *drvdata, int ctx,
					unsigned int fsr)
{
	void __iomem *base = drvdata->base;
	void __iomem *cb_base = drvdata->cb_base;
	bool is_secure = drvdata->sec_id != -1;

	struct msm_iommu_context_reg regs[MAX_DUMP_REGS];
	unsigned int i;
	memset(regs, 0, sizeof(regs));

	for (i = DUMP_REG_FIRST; i < MAX_DUMP_REGS; ++i) {
		struct msm_iommu_context_reg *r = &regs[i];
		unsigned long regaddr = dump_regs_tbl[i].reg_offset;
		if (is_secure &&
			dump_regs_tbl[i].dump_reg_type != DRT_CTX_REG) {
			r->valid = 0;
			continue;
		}
		r->valid = 1;
		switch (dump_regs_tbl[i].dump_reg_type) {
		case DRT_CTX_REG:
			r->val = GET_CTX_REG(regaddr, cb_base, ctx);
			break;
		case DRT_GLOBAL_REG:
			r->val = GET_GLOBAL_REG(regaddr, base);
			break;
		case DRT_GLOBAL_REG_N:
			r->val = GET_GLOBAL_REG_N(regaddr, ctx, base);
			break;
		default:
			pr_info("Unknown dump_reg_type...\n");
			r->valid = 0;
			break;
		}
	}
	print_ctx_regs(regs);
}

static void print_global_regs(void __iomem *base, unsigned int gfsr)
{
	pr_err("GFAR    = %016llx\n", GET_GFAR(base));

	pr_err("GFSR    = %08x [%s%s%s%s%s%s%s%s%s%s]\n", gfsr,
			(gfsr & 0x01) ? "ICF " : "",
			(gfsr & 0x02) ? "USF " : "",
			(gfsr & 0x04) ? "SMCF " : "",
			(gfsr & 0x08) ? "UCBF " : "",
			(gfsr & 0x10) ? "UCIF " : "",
			(gfsr & 0x20) ? "CAF " : "",
			(gfsr & 0x40) ? "EF " : "",
			(gfsr & 0x80) ? "PF " : "",
			(gfsr & 0x40000000) ? "SS " : "",
			(gfsr & 0x80000000) ? "MULTI " : "");

	pr_err("GFSYNR0	= %08x\n", GET_GFSYNR0(base));
	pr_err("GFSYNR1	= %08x\n", GET_GFSYNR1(base));
	pr_err("GFSYNR2	= %08x\n", GET_GFSYNR2(base));
}

irqreturn_t msm_iommu_global_fault_handler(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct msm_iommu_drvdata *drvdata;
	unsigned int gfsr;
	int ret;

	mutex_lock(&msm_iommu_lock);
	BUG_ON(!pdev);

	drvdata = dev_get_drvdata(&pdev->dev);
	BUG_ON(!drvdata);

	if (!drvdata->powered_on) {
		pr_err("Unexpected IOMMU global fault !!\n");
		pr_err("name = %s\n", drvdata->name);
		pr_err("Power is OFF. Can't read global fault information\n");
		ret = IRQ_HANDLED;
		goto fail;
	}

	if (drvdata->sec_id != -1) {
		pr_err("NON-secure interrupt from secure %s\n", drvdata->name);
		ret = IRQ_HANDLED;
		goto fail;
	}

	ret = __enable_clocks(drvdata);
	if (ret) {
		ret = IRQ_NONE;
		goto fail;
	}

	gfsr = GET_GFSR(drvdata->base);
	if (gfsr) {
		pr_err("Unexpected %s global fault !!\n", drvdata->name);
		print_global_regs(drvdata->base, gfsr);
		SET_GFSR(drvdata->base, gfsr);
		ret = IRQ_HANDLED;
	} else
		ret = IRQ_NONE;

	__disable_clocks(drvdata);
fail:
	mutex_unlock(&msm_iommu_lock);

	return ret;
}

irqreturn_t msm_iommu_fault_handler_v2(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct msm_iommu_drvdata *drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	unsigned int fsr;
	int ret;

	phys_addr_t pagetable_phys;
	u64 faulty_iova = 0;

	mutex_lock(&msm_iommu_lock);

	BUG_ON(!pdev);

	drvdata = dev_get_drvdata(pdev->dev.parent);
	BUG_ON(!drvdata);

	ctx_drvdata = dev_get_drvdata(&pdev->dev);
	BUG_ON(!ctx_drvdata);

	if (!drvdata->powered_on) {
		pr_err("Unexpected IOMMU page fault!\n");
		pr_err("name = %s\n", drvdata->name);
		pr_err("Power is OFF. Unable to read page fault information\n");
		/*
		 * We cannot determine which context bank caused the issue so
		 * we just return handled here to ensure IRQ handler code is
		 * happy
		 */
		ret = IRQ_HANDLED;
		goto fail;
	}

	ret = __enable_clocks(drvdata);
	if (ret) {
		ret = IRQ_NONE;
		goto fail;
	}

	fsr = GET_FSR(drvdata->cb_base, ctx_drvdata->num);
	if (fsr) {
		if (!ctx_drvdata->attached_domain) {
			pr_err("Bad domain in interrupt handler\n");
			ret = -ENOSYS;
		} else {
			faulty_iova =
				GET_FAR(drvdata->cb_base, ctx_drvdata->num);
			ret = report_iommu_fault(ctx_drvdata->attached_domain,
				&ctx_drvdata->pdev->dev,
				faulty_iova, 0);

		}
		if (ret == -ENOSYS) {
			pr_err("Unexpected IOMMU page fault!\n");
			pr_err("name = %s\n", drvdata->name);
			pr_err("context = %s (%d)\n", ctx_drvdata->name,
							ctx_drvdata->num);
			pr_err("Interesting registers:\n");
			__print_ctx_regs(drvdata,
					ctx_drvdata->num, fsr);

			if (ctx_drvdata->attached_domain) {
				pagetable_phys = msm_iommu_iova_to_phys_soft(
					ctx_drvdata->attached_domain,
					faulty_iova);
				pr_err("Page table in DDR shows PA = %x\n",
					(unsigned int) pagetable_phys);
			}
		}

		if (ret != -EBUSY)
			SET_FSR(drvdata->cb_base, ctx_drvdata->num, fsr);
		ret = IRQ_HANDLED;
	} else
		ret = IRQ_NONE;

	__disable_clocks(drvdata);
fail:
	mutex_unlock(&msm_iommu_lock);

	return ret;
}
#if 0
static phys_addr_t msm_iommu_get_pt_base_addr(struct iommu_domain *domain)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);

	return __pa(priv->pt.fl_table);
}
#endif
#define DUMP_REG_INIT(dump_reg, cb_reg, mbp, drt)		\
	do {							\
		dump_regs_tbl[dump_reg].reg_offset = cb_reg;	\
		dump_regs_tbl[dump_reg].name = #cb_reg;		\
		dump_regs_tbl[dump_reg].must_be_present = mbp;	\
		dump_regs_tbl[dump_reg].dump_reg_type = drt;	\
	} while (0)

static void msm_iommu_build_dump_regs_table(void)
{
	DUMP_REG_INIT(DUMP_REG_FAR0,	CB_FAR,       1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_FAR1,	CB_FAR + 4,   1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_PAR0,	CB_PAR,       1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_PAR1,	CB_PAR + 4,   1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_FSR,	CB_FSR,       1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_FSYNR0,	CB_FSYNR0,    1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_FSYNR1,	CB_FSYNR1,    1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_TTBR0_0,	CB_TTBR0,     1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_TTBR0_1,	CB_TTBR0 + 4, 0, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_TTBR1_0,	CB_TTBR1,     1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_TTBR1_1,	CB_TTBR1 + 4, 0, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_SCTLR,	CB_SCTLR,     1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_ACTLR,	CB_ACTLR,     1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_PRRR,	CB_PRRR,      1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_NMRR,	CB_NMRR,      1, DRT_CTX_REG);
	DUMP_REG_INIT(DUMP_REG_CBAR_N,	CBAR,         1, DRT_GLOBAL_REG_N);
	DUMP_REG_INIT(DUMP_REG_CBFRSYNRA_N, CBFRSYNRA, 1, DRT_GLOBAL_REG_N);
}

#ifdef CONFIG_IOMMU_PGTABLES_L2
static void __do_set_redirect(struct iommu_domain *domain, void *data)
{
	struct msm_iommu_priv *priv;
	int *no_redirect = data;

	mutex_lock(&msm_iommu_lock);
	priv = to_msm_priv(domain);
	priv->pt.redirect = !(*no_redirect);
	mutex_unlock(&msm_iommu_lock);
}

static void __do_get_redirect(struct iommu_domain *domain, void *data)
{
	struct msm_iommu_priv *priv;
	int *no_redirect = data;

	mutex_lock(&msm_iommu_lock);
	priv = to_msm_priv(domain);
	*no_redirect = !priv->pt.redirect;
	mutex_unlock(&msm_iommu_lock);
}

#else

static void __do_set_redirect(struct iommu_domain *domain, void *data)
{
}

static void __do_get_redirect(struct iommu_domain *domain, void *data)
{
}
#endif

static int msm_iommu_domain_set_attr(struct iommu_domain *domain,
				enum iommu_attr attr, void *data)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	struct msm_iommu_ctx_drvdata *ctx_drvdata = NULL;
	int dynamic;

	if (!list_empty(&priv->list_attached)) {
		ctx_drvdata = list_first_entry(&priv->list_attached,
			struct msm_iommu_ctx_drvdata, attached_elm);
	}

	switch (attr) {
	case DOMAIN_ATTR_QCOM_COHERENT_HTW_DISABLE:
		__do_set_redirect(domain, data);
		break;
	case DOMAIN_ATTR_SECURE_VMID:
		/*
		 * Not supported on MMU-500 driver as we are on preconfigured
		 * secure context banks where the secure VMID is already set
		 * from bootloader MMU initialization.
		 * Also, the TZ in MSM SoC using this driver will not accept
		 * hypervisor SCM calls which would be needed to change the
		 * secure VMID mapping in the IOMMU!
		 *
		 * Note: This is valid for both secure and non-secure IOMMU.
		 */
		break;
	case DOMAIN_ATTR_CONTEXT_BANK:
		/*
		 * We don't need to do anything here because CB allocation
		 * is not dynamic in this driver.
		 */
		break;
	case DOMAIN_ATTR_ATOMIC:
		/*
		 * Map / unmap in legacy driver are by default atomic. So
		 * we don't need to do anything here.
		 */
		break;
	case DOMAIN_ATTR_PROCID:
		priv->procid = *((u32 *)data);
		break;
	case DOMAIN_ATTR_DYNAMIC:
		dynamic = *((int *)data);

		if (ctx_drvdata)
			return -EBUSY;

		if (dynamic)
			priv->attributes |= 1 << DOMAIN_ATTR_DYNAMIC;
		else
			priv->attributes &= ~(1 << DOMAIN_ATTR_DYNAMIC);
		break;
	case DOMAIN_ATTR_FAST:
		/*
		 * We cannot support DOMAIN_ATTR_FAST. This is an optimization
		 * that is specific to arm_smmu implementations and MMU-500
		 * does not support this mere trick.
		 */
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int msm_iommu_domain_get_attr(struct iommu_domain *domain,
				enum iommu_attr attr, void *data)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	struct msm_iommu_ctx_drvdata *ctx_drvdata = NULL;
	u64 ttbr0;
	u32 ctxidr;

	if (!list_empty(&priv->list_attached))
		ctx_drvdata = list_first_entry(&priv->list_attached,
			struct msm_iommu_ctx_drvdata, attached_elm);

	switch (attr) {
	case DOMAIN_ATTR_QCOM_COHERENT_HTW_DISABLE:
		__do_get_redirect(domain, data);
		break;
	/* kholk TODO: PT_BASE_ADDR scheduled for removal */
	case DOMAIN_ATTR_PT_BASE_ADDR:
		*((phys_addr_t *)data) = virt_to_phys(priv->pt.fl_table);
		break;
	case DOMAIN_ATTR_SECURE_VMID:
		*((int *) data) = -VMID_INVAL;
		break;
	case DOMAIN_ATTR_CONTEXT_BANK:
		if (!ctx_drvdata)
			return -ENODEV;

		*((unsigned int *) data) = ctx_drvdata->num;
		break;
	case DOMAIN_ATTR_TTBR0:
		ttbr0 = get_full_ttbr0(priv);

		*((u64 *)data) = ttbr0;
		break;
	case DOMAIN_ATTR_CONTEXTIDR:
		if (IS_CB_FORMAT_LONG)
			ctxidr = priv->procid;
		else
			ctxidr = (priv->asid & CB_CONTEXTIDR_ASID_MASK) |
				(priv->procid << CB_CONTEXTIDR_PROCID_SHIFT);

		*((u32 *)data) = ctxidr;
		break;
	case DOMAIN_ATTR_PROCID:
		*((u32 *)data) = priv->procid;
		break;
	case DOMAIN_ATTR_DYNAMIC:
		*((int *)data) = !!(priv->attributes
					& (1 << DOMAIN_ATTR_DYNAMIC));
		break;
	case DOMAIN_ATTR_FAST:
		/*
		 * We cannot support DOMAIN_ATTR_FAST. This is an optimization
		 * that is specific to arm_smmu implementations and MMU-500
		 * does not support this mere trick.
		 */
		*((int *)data) = 0;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int msm_iommu_of_xlate(struct device *dev, struct of_phandle_args *args)
{
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	struct platform_device *pdev, *ctx_pdev;
	struct msm_iommu_master *master;
	struct device_node *child;
	bool found = false;
	u32 val;
	int ret;

	if (args->args_count > 2)
		return -EINVAL;

	dev_dbg(dev, "getting pdev for %s\n", args->np->name);

	pdev = of_find_device_by_node(args->np);
	if (!pdev) {
		dev_dbg(dev, "iommu pdev not found\n");
		return -ENODEV;
	}

	iommu_drvdata = platform_get_drvdata(pdev);
	if (!iommu_drvdata)
		return -ENODEV;

	for_each_child_of_node(args->np, child) {
		ctx_pdev = of_find_device_by_node(child);
		if (!ctx_pdev)
			return -ENODEV;

		ctx_drvdata = platform_get_drvdata(ctx_pdev);

		ret = of_property_read_u32(child, "qcom,ctx-num", &val);
		if (ret)
			return ret;

		if (val == args->args[0]) {
			found = true;
			break;
		}
	}

	if (!found)
		return -ENODEV;

	dev_dbg(dev, "found ctx data for %s (num:%d)\n",
		ctx_drvdata->name, ctx_drvdata->num);

	master = devm_kzalloc(iommu_drvdata->dev, sizeof(*master), GFP_KERNEL);
	if (!master)
		return -ENOMEM;

	INIT_LIST_HEAD(&master->list);
	master->ctx_num = args->args[0];
	master->dev = dev;
	master->iommu_drvdata = iommu_drvdata;
	master->ctx_drvdata = ctx_drvdata;

	dev_dbg(dev, "adding master for device %s\n", dev_name(dev));

	list_add_tail(&master->list, &iommu_masters);
#if 0
	if (dev->bus && dev->bus->iommu_ops) {
		ret = dev->bus->iommu_ops->add_device(dev);
		if (ret) {
			dev_err(dev, "iommu add_device failed (%d)\n", ret);
			return ret;
		}
	} else {
		dev_err(dev, "of_xlate missing iommu_ops for bus\n");
		return -ENODEV;
	}
#endif
	return 0;
}

int msm_iommu_dma_supported(struct iommu_domain *domain,
				  struct device *dev, u64 mask)
{
	return ((1ULL << 32) - 1) < mask ? 0 : 1;
}

static struct iommu_ops msm_iommu_ops = {
	.domain_alloc = msm_iommu_domain_alloc,
	.domain_free = msm_iommu_domain_free,
	.attach_dev = msm_iommu_attach_dev,
	.detach_dev = msm_iommu_detach_dev,
	.map = msm_iommu_map,
	.unmap = msm_iommu_unmap,
	.map_sg = msm_iommu_map_sg, //default_iommu_map_sg,
	.iova_to_phys = msm_iommu_iova_to_phys,
	.add_device = msm_iommu_add_device,
	.remove_device = msm_iommu_remove_device,
	.device_group = msm_iommu_device_group,
	.pgsize_bitmap = MSM_IOMMU_PGSIZES,
	.domain_set_attr = msm_iommu_domain_set_attr,
	.domain_get_attr = msm_iommu_domain_get_attr,
	.of_xlate = msm_iommu_of_xlate,
	.dma_supported = msm_iommu_dma_supported,
};

int msm_iommu_init(struct device *dev)
{
	static bool done = false;
	int ret;

	of_iommu_set_ops(dev->of_node, &msm_iommu_ops);

	if (done)
		return 0;

	msm_iommu_pagetable_init();

	ret = bus_set_iommu(&platform_bus_type, &msm_iommu_ops);
	if (ret)
		return ret;

#ifdef CONFIG_ARM_AMBA
	if (!iommu_present(&amba_bustype))
		bus_set_iommu(&amba_bustype, &msm_iommu_ops);
#endif

#ifdef CONFIG_PCI
	if (!iommu_present(&pci_bus_type))
		bus_set_iommu(&pci_bus_type, &msm_iommu_ops);
#endif

	msm_iommu_build_dump_regs_table();

	done = true;

	return 0;
}
