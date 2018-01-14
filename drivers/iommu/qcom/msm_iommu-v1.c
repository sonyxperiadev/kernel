/*
 * MSM Secure IOMMU v1/v2 Driver
 * Copyright (C) 2017-2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * May contain portions of code (c) 2012-2014, The Linux Foundation.
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
#include <soc/qcom/scm.h>
#include <soc/qcom/secure_buffer.h>
#include <linux/msm-bus.h>

#include "../io-pgtable.h"

#include "qcom_iommu.h"
#include "msm_iommu_hw-v1.h"
#include "msm_iommu_priv.h"

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

struct dump_regs_tbl_entry dump_regs_tbl[MAX_DUMP_REGS];
static struct iommu_ops msm_iommu_ops;

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

static inline bool is_domain_dynamic(struct msm_iommu_priv *priv)
{
	return (priv->attributes & (1 << DOMAIN_ATTR_DYNAMIC));
}

static int msm_iommu_enable_clocks(struct iommu_domain *domain)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	struct msm_iommu_priv *base_priv;
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	int ret = -EINVAL;

	if (is_domain_dynamic(priv)) {
		if (!priv->base)
			return -EINVAL;

		base_priv = to_msm_priv(priv->base);
	} else {
		base_priv = priv;
	}

	/* Find a hook to get a valid structure where to get clocks */
	list_for_each_entry(ctx_drvdata, &base_priv->list_attached, attached_elm) {
		if (!ctx_drvdata->pdev)
			continue;

		iommu_drvdata = dev_get_drvdata(ctx_drvdata->pdev->dev.parent);
		if (!iommu_drvdata)
			continue;

		ret = __enable_clocks(iommu_drvdata);
		if (ret == 0)
			goto end;
	}
end:
	return 0;
}

static void msm_iommu_disable_clocks(struct iommu_domain *domain)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	struct msm_iommu_priv *base_priv;
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	int ret = -EINVAL;

	if (is_domain_dynamic(priv)) {
		if (!priv->base)
			goto end;

		base_priv = to_msm_priv(priv->base);
	} else {
		base_priv = priv;
	}

	/* Find a hook to get a valid structure where to get clocks */
	list_for_each_entry(ctx_drvdata, &base_priv->list_attached, attached_elm) {
		if (!ctx_drvdata->pdev)
			continue;

		iommu_drvdata = dev_get_drvdata(ctx_drvdata->pdev->dev.parent);
		if (!iommu_drvdata)
			continue;

		__disable_clocks(iommu_drvdata);
	}
end:
	if (ret)
		pr_err("%s: Whoops! Cannot disable clocks!!\n", __func__);
	return;
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
		BUG();

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

static void __sync_tlb(struct msm_iommu_drvdata *iommu_drvdata, int ctx,
		       struct msm_iommu_priv *priv)
{
	void __iomem *base = iommu_drvdata->cb_base;
	unsigned int val;
	unsigned int res;

	SET_TLBSYNC(base, ctx, 0);
	/* No barrier needed due to read dependency */

	res = readl_poll_timeout_atomic(CTX_REG(CB_TLBSTATUS, base, ctx), val,
				(val & CB_TLBSTATUS_SACTIVE) == 0, 0, 1000000);
	if (res)
		BUG();
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

static void msm_iommu_tlb_sync(void *cookie)
{
	struct iommu_domain *domain = cookie;
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
		if (!priv->base) {
			pr_err("NULL priv->base in tlb sync!!!\n");
			return;
		}

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

		__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);

		__disable_clocks(iommu_drvdata);
	}
fail:
	if (ret)
		pr_err("%s: ERROR %d !!\n", __func__, ret);
	return;
}

static void msm_iommu_tlb_flush_all(void *cookie)
{
	struct iommu_domain *domain = cookie;

	__flush_iotlb(domain);
}

static void msm_iommu_tlb_flush_range_nosync(unsigned long iova, size_t size,
					bool leaf, void *cookie)
{
	struct iommu_domain *domain = cookie;
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	struct msm_iommu_priv *base_priv;
	struct msm_iommu_drvdata *iommu_drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	unsigned long va = iova;
	int ret = 0;

	/*
	 * Context banks are properly attached to base domain and not dynamic
	 * domains. So, we must get the base domain and CBs attached to it
	 * for TLB invalidation.
	 */
	if (is_domain_dynamic(priv)) {
		if (!priv->base) {
			pr_err("NULL priv->base in flush range!!!\n");
			return;
		}

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

		va &= ~12UL;
		va |= priv->asid;

		if (leaf) {
			SET_TLBIVAL(iommu_drvdata->cb_base,
						ctx_drvdata->num, va);
		} else {
			SET_TLBIVA(iommu_drvdata->cb_base,
						ctx_drvdata->num, va);
		}

		__disable_clocks(iommu_drvdata);
	}
fail:
	if (ret)
		pr_err("%s: ERROR %d !!\n", __func__, ret);
	return;
}

static const struct iommu_gather_ops msm_iommu_gather_ops = {
	.tlb_flush_all	= msm_iommu_tlb_flush_all,
	.tlb_add_flush	= msm_iommu_tlb_flush_range_nosync,
	.tlb_sync	= msm_iommu_tlb_sync,
};

static void msm_iommu_tlbi_domain(struct iommu_domain *domain)
{
	__flush_iotlb(domain);
}

static void program_iommu_bfb_settings(void __iomem *base,
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

int msm_iommu_sec_program_iommu(struct msm_iommu_drvdata *drvdata,
			struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	int ret;
	u64 scm_ret;

	if (drvdata->smmu_local_base) {
		writel_relaxed(0xFFFFFFFF,
			       drvdata->smmu_local_base + SMMU_INTR_SEL_NS);
		mb();
	}

	ret = scm_restore_sec_cfg(drvdata->sec_id, ctx_drvdata->num, &scm_ret);
	if (ret || scm_ret) {
		pr_err("scm call IOMMU_SECURE_CFG failed\n");
		return ret ? ret : -EINVAL;
	}

	return ret;
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

#if 0
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
#endif

static void __program_context(struct msm_iommu_drvdata *iommu_drvdata,
			      struct msm_iommu_ctx_drvdata *ctx_drvdata,
			      struct msm_iommu_priv *priv)
{
	void __iomem *cb_base = iommu_drvdata->cb_base;
	unsigned int ctx = ctx_drvdata->num;

	__reset_context(iommu_drvdata, ctx);

	priv->asid = ctx_drvdata->num;
	SET_TTBR0(iommu_drvdata->cb_base, ctx_drvdata->num,
			priv->pgtbl_cfg.arm_lpae_s1_cfg.ttbr[0] |
			((u64)priv->asid << CB_TTBR0_ASID_SHIFT));

	SET_TTBR1(iommu_drvdata->cb_base, ctx_drvdata->num,
			priv->pgtbl_cfg.arm_lpae_s1_cfg.ttbr[1] |
			((u64)priv->asid << CB_TTBR1_ASID_SHIFT));

	SET_CB_TCR2_SEP(iommu_drvdata->cb_base, ctx_drvdata->num,
			priv->pgtbl_cfg.arm_lpae_s1_cfg.tcr >> 32);

	SET_CB_TTBCR_EAE(iommu_drvdata->cb_base, ctx_drvdata->num, 1);

	SET_CB_MAIR0(iommu_drvdata->cb_base, ctx_drvdata->num,
			priv->pgtbl_cfg.arm_lpae_s1_cfg.mair[0]);
	SET_CB_MAIR1(iommu_drvdata->cb_base, ctx_drvdata->num,
			priv->pgtbl_cfg.arm_lpae_s1_cfg.mair[1]);

	/* Ensure that ASID assignment has completed before we use
	 * ASID for TLB invalidation. Here, mb() is required because
	 * both these registers are separated by more than 1KB. */
	mb();

	/* If requested, disable stall on this context bank */
	if (priv->attributes & (1 << DOMAIN_ATTR_CB_STALL_DISABLE)) {
		SET_CB_SCTLR_CFCFG(cb_base, ctx, 0);
		SET_CB_SCTLR_HUPCF(cb_base, ctx, 1);
	}

	SET_CB_SCTLR_CFIE(cb_base, ctx, 1);
	SET_CB_SCTLR_CFRE(cb_base, ctx, 1);
	SET_CB_SCTLR_AFE(cb_base, ctx, 1);
	SET_CB_SCTLR_TRE(cb_base, ctx, 1);
	SET_CB_SCTLR_ASIDPNE(cb_base, ctx, 1);
	SET_CB_SCTLR_M(cb_base, ctx, 1);

	mb();
}

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

	/*
	 * Note: for secure mapping, the IOMMU_DOMAIN_DMA is not allowed,
	 * hence only UNMANAGED is allowed. TODO: Find a way to differentiate
	 * between secure mapping ONLY and insecure pagetable allocation.
	 */
	if (type != IOMMU_DOMAIN_UNMANAGED && type != IOMMU_DOMAIN_DMA)
		return NULL;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return NULL;

	INIT_LIST_HEAD(&priv->list_attached);
	mutex_init(&priv->init_mutex);
	spin_lock_init(&priv->pgtbl_lock);

	if (type == IOMMU_DOMAIN_DMA) {
		ret = iommu_get_dma_cookie(&priv->domain);
		if (ret)
			goto err;
	}

	return &priv->domain;

err:
	kfree(priv);
	return NULL;
}

static void msm_iommu_domain_free(struct iommu_domain *domain)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);

	if (!priv) {
		pr_err("OUCH!\n");
		return;
	}

	iommu_put_dma_cookie(domain);

	free_io_pgtable_ops(priv->pgtbl_ops);

	kfree(priv);
}

static int msm_iommu_dynamic_attach(struct iommu_domain *domain, struct device *dev,
				struct msm_iommu_drvdata *iommu_drvdata,
				struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	int ret;
	struct msm_iommu_priv *priv;
	struct io_pgtable_ops *pgtbl_ops;

	priv = to_msm_priv(domain);

	/* Check if the domain is already attached or not */
	if (priv->asid < MAX_ASID && priv->asid > 0)
		return -EBUSY;

	/* Make sure the domain is initialized */
	domain->geometry.aperture_end = (1ULL << priv->pgtbl_cfg.ias) - 1;
	priv->pgtbl_cfg = (struct io_pgtable_cfg) {
		.pgsize_bitmap	= msm_iommu_ops.pgsize_bitmap,
		.ias		= 32,
		.oas		= 40,
		.tlb		= &msm_iommu_gather_ops,
		.iommu_dev	= dev,
		.iova_base	= domain->geometry.aperture_start,
		.iova_end	= domain->geometry.aperture_end,
	};
	domain->geometry.force_aperture = true;


	pgtbl_ops = alloc_io_pgtable_ops(ARM_32_LPAE_S1, &priv->pgtbl_cfg, domain);
	if (!pgtbl_ops) {
		pr_err("failed to allocate pagetable ops\n");
		return -ENOMEM;
	}

	ret = idr_alloc_cyclic(&iommu_drvdata->asid_idr, priv,
			iommu_drvdata->ncb + 2, MAX_ASID + 1, GFP_KERNEL);
	if (ret < 0) {
		pr_err("Failed to allocate idr for dynamic domain.\n");
		free_io_pgtable_ops(pgtbl_ops);
		return -ENOSPC;
	}

	priv->asid = ret;
	priv->base = ctx_drvdata->attached_domain;

	priv->pgtbl_ops = pgtbl_ops;

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
	struct io_pgtable_ops *pgtbl_ops;
	int ret = 0;
	int is_secure;
	bool secure_ctx;
	unsigned long flags;

	priv = to_msm_priv(domain);
	if (!priv || !dev) {
		return -EINVAL;
	}

	mutex_lock(&priv->init_mutex);

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
		if (ctx_drvdata->needs_secure_map) {
			pr_err("%s: BUG! IOMMU requires secure mapping!"
				" This context cannot be dynamic!\n",
				__func__);
			BUG();
		}
		ret = msm_iommu_dynamic_attach(domain, dev,
				iommu_drvdata, ctx_drvdata);
		mutex_unlock(&priv->init_mutex);
		return ret;
	}

	++ctx_drvdata->attach_count;

	if (ctx_drvdata->attach_count > 1)
		goto unlock;

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	if (!list_empty(&ctx_drvdata->attached_elm)) {
		ret = -EBUSY;
		spin_unlock_irqrestore(&priv->pgtbl_lock, flags);
		goto unlock;
	}

	list_for_each_entry(tmp_drvdata, &priv->list_attached, attached_elm)
		if (tmp_drvdata == ctx_drvdata) {
			ret = -EBUSY;
			spin_unlock_irqrestore(&priv->pgtbl_lock, flags);
			goto unlock;
		}

	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);

	is_secure = iommu_drvdata->sec_id != -1;

	ret = apply_bus_vote(iommu_drvdata, 1);
	if (ret)
		goto unlock;

	ret = __enable_clocks(iommu_drvdata);
	if (ret)
		goto unlock;

	/* We can only do this once */
	if (!iommu_drvdata->ctx_attach_count) {
		ret = msm_iommu_sec_program_iommu(iommu_drvdata, ctx_drvdata);
		if (ret)
			goto unlock;

		/* bfb settings are always programmed by HLOS */
		program_iommu_bfb_settings(iommu_drvdata->base,
					   iommu_drvdata->bfb_settings);
	}


	/* Make sure the domain is initialized */
	domain->geometry.aperture_end = (1ULL << priv->pgtbl_cfg.ias) - 1;
	priv->pgtbl_cfg = (struct io_pgtable_cfg) {
		.pgsize_bitmap	= msm_iommu_ops.pgsize_bitmap,
		.ias		= 32,
		.oas		= 40,
		.tlb		= &msm_iommu_gather_ops,
		.arm_msm_secure_cfg = {
			.sec_id = iommu_drvdata->sec_id,
			.cbndx  = ctx_drvdata->num,
		},
		.iommu_dev	= iommu_drvdata->dev,
		.iova_base	= domain->geometry.aperture_start,
		.iova_end	= domain->geometry.aperture_end,
	};
	domain->geometry.force_aperture = true;

	pgtbl_ops = alloc_io_pgtable_ops(ARM_32_LPAE_S1, &priv->pgtbl_cfg, domain);
	if (!pgtbl_ops) {
		pr_err("failed to allocate pagetable ops\n");
		ret = -ENOMEM;
		goto unlock;
	}

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	priv->pgtbl_ops = pgtbl_ops;
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);

	secure_ctx = !!(ctx_drvdata->secure_context > 0);
	if (secure_ctx) {
		dev_dbg(dev, "Detected secure context.\n");
		goto add_domain;
	}

	iommu_halt(iommu_drvdata);

	__program_context(iommu_drvdata, ctx_drvdata, priv);

	iommu_resume(iommu_drvdata);

	/* Ensure TLB is clear */
	if (iommu_drvdata->model != MMU_500) {
		SET_TLBIASID(iommu_drvdata->cb_base, ctx_drvdata->num,
			     ctx_drvdata->asid);
		__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);
	}

	__disable_clocks(iommu_drvdata);

add_domain:
	if (ctx_drvdata->needs_secure_map)
		dev_dbg(dev, "Attaching secure domain %s (%d)\n",
			ctx_drvdata->name, ctx_drvdata->num);

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	list_add(&(ctx_drvdata->attached_elm), &priv->list_attached);
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);

	ctx_drvdata->attached_domain = domain;
	++iommu_drvdata->ctx_attach_count;

unlock:
	mutex_unlock(&priv->init_mutex);
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
	int ret;
	unsigned long flags;

	if (!dev)
		return;

	priv = to_msm_priv(domain);
	if (!priv)
		return;

	mutex_lock(&priv->init_mutex);

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
		if (ctx_drvdata->needs_secure_map) {
			pr_err("%s: BUG! IOMMU requires secure mapping!"
				" This context cannot be dynamic!\n",
				__func__);
			BUG();
		}
		msm_iommu_dynamic_detach(domain,
				iommu_drvdata, ctx_drvdata);
		mutex_unlock(&priv->init_mutex);
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

	if (iommu_drvdata->model == MMU_500) {
		SET_TLBIASID(iommu_drvdata->cb_base, ctx_drvdata->num,
			     ctx_drvdata->asid);
		__sync_tlb(iommu_drvdata, ctx_drvdata->num, priv);
	}

	ctx_drvdata->asid = -1;

	__reset_context(iommu_drvdata, ctx_drvdata->num);

	__disable_clocks(iommu_drvdata);

	apply_bus_vote(iommu_drvdata, 0);

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	list_del_init(&ctx_drvdata->attached_elm);
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);

	ctx_drvdata->attached_domain = NULL;
	BUG_ON(iommu_drvdata->ctx_attach_count == 0);
	--iommu_drvdata->ctx_attach_count;
unlock:
	mutex_unlock(&priv->init_mutex);
}

static int msm_iommu_map(struct iommu_domain *domain, unsigned long va,
			 phys_addr_t pa, size_t len, int prot)
{
	struct msm_iommu_priv *priv;
	int ret = 0;
	unsigned long flags;

	priv = to_msm_priv(domain);
	if (!priv) {
		ret = -EINVAL;
		goto fail;
	}

	if (!priv->pgtbl_ops)
		return -ENODEV;

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	ret = priv->pgtbl_ops->map(priv->pgtbl_ops, va, pa, len, prot);
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);
fail:
	return ret;
}

static size_t msm_iommu_unmap(struct iommu_domain *domain, unsigned long va,
			      size_t len)
{
	struct msm_iommu_priv *priv;
	int ret = -ENODEV;
	unsigned long flags;

	priv = to_msm_priv(domain);
	if (!priv)
		goto fail;

	if (!priv->pgtbl_ops)
		return 0;

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	ret = priv->pgtbl_ops->unmap(priv->pgtbl_ops, va, len);
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);
fail:
	return ret;
}

static size_t msm_iommu_map_sg(struct iommu_domain *domain, unsigned long va,
			   struct scatterlist *sg, unsigned int nents, int prot)
{
	struct msm_iommu_priv *priv;
	int ret = 0;
	size_t size;
	unsigned long flags;

	priv = to_msm_priv(domain);
	if (!priv) {
		ret = -EINVAL;
		goto fail;
	}

	if (!priv->pgtbl_ops)
		return -ENODEV;

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	ret = priv->pgtbl_ops->map_sg(priv->pgtbl_ops,
		va, sg, nents, prot, &size);
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);
fail:
	return ret;
}

static phys_addr_t msm_iommu_iova_to_phys(struct iommu_domain *domain,
					  phys_addr_t va)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	phys_addr_t ret = 0;
	unsigned long flags;

	/*
	 * NOTE: The iova_to_phys for secure mapping ONLY is
	 *       NEVER supported. Though, this should not give
	 *       us any problem here, since we always support
	 *       also the insecure pagetable mapping. Always.
	 */
	if (!priv)
		return -EINVAL;

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	ret = priv->pgtbl_ops->iova_to_phys(priv->pgtbl_ops, va);
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);

	return ret;
}

static bool msm_iommu_is_iova_coherent(struct iommu_domain *domain,
					 dma_addr_t va)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	bool ret = 0;
	unsigned long flags;

	if (!priv)
		return -EINVAL;

	spin_lock_irqsave(&priv->pgtbl_lock, flags);
	ret = priv->pgtbl_ops->iova_to_phys(priv->pgtbl_ops, va);
	spin_unlock_irqrestore(&priv->pgtbl_lock, flags);

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

static inline void print_ctx_mem_attr_regs(struct msm_iommu_context_reg regs[])
{
	pr_err("MAIR0   = %08x    MAIR1   = %08x\n",
		 regs[DUMP_REG_MAIR0].val, regs[DUMP_REG_MAIR1].val);
}

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
	u64 faulty_iova = 0;

	mutex_lock(&msm_iommu_lock);

	BUG_ON(!pdev);

	drvdata = dev_get_drvdata(pdev->dev.parent);
	BUG_ON(!drvdata);

	ctx_drvdata = dev_get_drvdata(&pdev->dev);
	BUG_ON(!ctx_drvdata);

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

static int msm_iommu_domain_set_attr(struct iommu_domain *domain,
				enum iommu_attr attr, void *data)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);
	struct msm_iommu_ctx_drvdata *ctx_drvdata = NULL;
	int dynamic;

	if (!priv)
		return -EINVAL;

	if (!list_empty(&priv->list_attached)) {
		ctx_drvdata = list_first_entry(&priv->list_attached,
			struct msm_iommu_ctx_drvdata, attached_elm);
	}

	switch (attr) {
	case DOMAIN_ATTR_QCOM_COHERENT_HTW_DISABLE:
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
#ifdef ENABLE_DYNAMIC_CBNUM
		if (!is_domain_dynamic(priv)) {
			return -EINVAL;
		}
		/* try to do it anyway... */
		ctx_drvdata->num = *((unsigned int *) data);
#endif
		break;
	case DOMAIN_ATTR_ATOMIC:
		/*
		 * Map / unmap in legacy driver are by default atomic. So
		 * we don't need to do anything here.
		 */
		break;
	case DOMAIN_ATTR_PROCID:
		if (ctx_drvdata)
			return -EBUSY;

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
	case DOMAIN_ATTR_CB_STALL_DISABLE:
		if (*((int *)data))
			priv->attributes |= 1 << DOMAIN_ATTR_CB_STALL_DISABLE;
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
		break;
	/* kholk TODO: PT_BASE_ADDR scheduled for removal */
	case DOMAIN_ATTR_PT_BASE_ADDR:
		*((phys_addr_t *)data) =
			priv->pgtbl_cfg.arm_lpae_s1_cfg.ttbr[0];
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
		ttbr0 = priv->pgtbl_cfg.arm_lpae_s1_cfg.ttbr[0] |
			((u64)priv->asid << CB_TTBR0_ASID_SHIFT);

		*((u64 *)data) = ttbr0;
		break;
	case DOMAIN_ATTR_CONTEXTIDR:
		ctxidr = priv->procid;

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
	case DOMAIN_ATTR_CB_STALL_DISABLE:
		*((int *)data) = !!(priv->attributes
			& (1 << DOMAIN_ATTR_CB_STALL_DISABLE));
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

	return 0;
}

static int msm_iommu_dma_supported(struct iommu_domain *domain,
				  struct device *dev, u64 mask)
{
	return ((1ULL << 32) - 1) < mask ? 0 : 1;
}

static bool msm_iommu_capable(enum iommu_cap cap)
{
	switch (cap) {
	case IOMMU_CAP_CACHE_COHERENCY:
		return true;
	case IOMMU_CAP_NOEXEC:
		return true;
	default:
		return false;
	}
}

static unsigned long msm_iommu_get_pgsize_bitmap(struct iommu_domain *domain)
{
	struct msm_iommu_priv *priv = to_msm_priv(domain);

	/*
	 * if someone is calling map before attach just return the
	 * supported page sizes for the hardware itself.
	 */
	if (!priv->pgtbl_cfg.pgsize_bitmap)
		return msm_iommu_ops.pgsize_bitmap;
	/*
	 * otherwise return the page sizes supported by this specific page
	 * table configuration
	 */
	return priv->pgtbl_cfg.pgsize_bitmap;
}

static struct iommu_ops msm_iommu_ops = {
	.capable = msm_iommu_capable,
	.domain_alloc = msm_iommu_domain_alloc,
	.domain_free = msm_iommu_domain_free,
	.attach_dev = msm_iommu_attach_dev,
	.detach_dev = msm_iommu_detach_dev,
	.map = msm_iommu_map,
	.unmap = msm_iommu_unmap,
	.map_sg = msm_iommu_map_sg,
	.iova_to_phys = msm_iommu_iova_to_phys,
	.is_iova_coherent = msm_iommu_is_iova_coherent,
	.add_device = msm_iommu_add_device,
	.remove_device = msm_iommu_remove_device,
	.device_group = msm_iommu_device_group,
	.pgsize_bitmap = (SZ_4K | SZ_64K | SZ_2M | SZ_32M | SZ_1G),
	.get_pgsize_bitmap = msm_iommu_get_pgsize_bitmap,
	.domain_set_attr = msm_iommu_domain_set_attr,
	.domain_get_attr = msm_iommu_domain_get_attr,
	.of_xlate = msm_iommu_of_xlate,
	.dma_supported = msm_iommu_dma_supported,
	.tlbi_domain = msm_iommu_tlbi_domain,
	.enable_config_clocks	= msm_iommu_enable_clocks,
	.disable_config_clocks	= msm_iommu_disable_clocks,
};

int msm_iommu_init(struct device *dev)
{
	static bool done = false;
	int ret;

	of_iommu_set_ops(dev->of_node, &msm_iommu_ops);

	if (done)
		return 0;

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
