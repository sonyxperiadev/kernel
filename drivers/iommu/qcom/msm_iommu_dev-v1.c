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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/iommu.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/msm-bus.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_iommu.h>

#include <asm/cacheflush.h>

#include "../io-pgtable.h"
#include "msm_iommu_hw-v1.h"
#include "msm_iommu_priv.h"
#include <linux/qcom_iommu.h>
#include <soc/qcom/scm.h>

static const struct of_device_id msm_iommu_ctx_match_table[];
static struct iommu_access_ops *iommu_access_ops;
static int is_secure;

static DEFINE_MUTEX(iommu_list_lock);
static LIST_HEAD(iommu_list);

/* Support LPAE only */
static const char *BFB_REG_NODE_NAME = "qcom,iommu-lpae-bfb-regs";
static const char *BFB_DATA_NODE_NAME = "qcom,iommu-lpae-bfb-data";

extern struct iommu_access_ops iommu_access_ops_v1;

static void msm_iommu_add_drv(struct msm_iommu_drvdata *drv)
{
	mutex_lock(&iommu_list_lock);
	list_add(&drv->list, &iommu_list);
	mutex_unlock(&iommu_list_lock);
}

static void msm_iommu_remove_drv(struct msm_iommu_drvdata *drv)
{
	mutex_lock(&iommu_list_lock);
	list_del(&drv->list);
	mutex_unlock(&iommu_list_lock);
}

static int find_iommu_ctx(struct device *dev, void *data)
{
	struct msm_iommu_ctx_drvdata *c;

	c = dev_get_drvdata(dev);
	if (!c || !c->name)
		return 0;

	return !strcmp(data, c->name);
}

static struct device *msm_iommu_find_context(
			struct device *dev, const char *name)
{
	return device_find_child(dev, (void *)name, find_iommu_ctx);
}

struct device *msm_iommu_get_ctx(const char *ctx_name)
{
	struct msm_iommu_drvdata *drv;
	struct device *dev = NULL;

	mutex_lock(&iommu_list_lock);
	list_for_each_entry(drv, &iommu_list, list) {
		dev = msm_iommu_find_context(drv->dev, ctx_name);
		if (dev)
			break;
	}
	mutex_unlock(&iommu_list_lock);

	put_device(dev);

	if (!dev || !dev_get_drvdata(dev)) {
		pr_debug("Could not find context <%s>\n", ctx_name);
		dev = ERR_PTR(-EPROBE_DEFER);
	}

	return dev;
}
EXPORT_SYMBOL(msm_iommu_get_ctx);

static int msm_iommu_parse_bfb_settings(struct platform_device *pdev,
					struct msm_iommu_drvdata *drvdata)
{
	struct msm_iommu_bfb_settings *bfb_settings;
	u32 nreg, nval;
	int ret;

	/*
	 * It is not valid for a device to have the BFB_REG_NODE_NAME
	 * property but not the BFB_DATA_NODE_NAME property, and vice versa.
	 */
	if (!of_get_property(pdev->dev.of_node, BFB_REG_NODE_NAME, &nreg)) {
		if (of_get_property(pdev->dev.of_node, BFB_DATA_NODE_NAME,
				    &nval))
			return -EINVAL;
		return 0;
	}

	if (!of_get_property(pdev->dev.of_node, BFB_DATA_NODE_NAME, &nval))
		return -EINVAL;

	if (nreg >= sizeof(bfb_settings->regs))
		return -EINVAL;

	if (nval >= sizeof(bfb_settings->data))
		return -EINVAL;

	if (nval != nreg)
		return -EINVAL;

	bfb_settings = devm_kzalloc(&pdev->dev, sizeof(*bfb_settings),
				    GFP_KERNEL);
	if (!bfb_settings)
		return -ENOMEM;

	ret = of_property_read_u32_array(pdev->dev.of_node,
					 BFB_REG_NODE_NAME,
					 bfb_settings->regs,
					 nreg / sizeof(*bfb_settings->regs));
	if (ret)
		return ret;

	ret = of_property_read_u32_array(pdev->dev.of_node,
					 BFB_DATA_NODE_NAME,
					 bfb_settings->data,
					 nval / sizeof(*bfb_settings->data));
	if (ret)
		return ret;

	bfb_settings->length = nreg / sizeof(*bfb_settings->regs);

	drvdata->bfb_settings = bfb_settings;

	return 0;
}

static int __get_bus_vote_client(struct platform_device *pdev,
				 struct msm_iommu_drvdata *drvdata)
{
	int ret = 0;
	struct msm_bus_scale_pdata *bs_table;
	const char *dummy;

	/* Check whether bus scaling has been specified for this node */
	ret = of_property_read_string(pdev->dev.of_node, "qcom,msm-bus,name",
				      &dummy);
	if (ret)
		return 0;

	bs_table = msm_bus_cl_get_pdata(pdev);

	if (bs_table) {
		drvdata->bus_client = msm_bus_scale_register_client(bs_table);
		if (IS_ERR(&drvdata->bus_client)) {
			pr_err("%s(): Bus client register failed.\n", __func__);
			ret = -EINVAL;
		}
	}
	return ret;
}

static void __put_bus_vote_client(struct msm_iommu_drvdata *drvdata)
{
	msm_bus_scale_unregister_client(drvdata->bus_client);
	drvdata->bus_client = 0;
}

static void msm_iommu_check_scm_call_avail(void)
{
	is_secure = scm_is_call_available(SCM_SVC_MP, IOMMU_SECURE_CFG);
}

static int msm_iommu_get_scm_call_avail(void)
{
	return is_secure;
}

static void get_secure_id(struct device_node *node,
			  struct msm_iommu_drvdata *drvdata)
{
	if (!msm_iommu_get_scm_call_avail())
		return;

	of_property_read_u32(node, "qcom,iommu-secure-id",
				     &drvdata->sec_id);
}

static void get_secure_ctx(struct device_node *node,
			   struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	if (!msm_iommu_get_scm_call_avail())
		return;

	ctx_drvdata->secure_context = of_property_read_bool(node,
				"qcom,secure-context");
}

static void get_secure_mapping(struct device_node *node,
			   struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	if (!msm_iommu_get_scm_call_avail())
		return;

	ctx_drvdata->needs_secure_map = of_property_read_bool(node,
				"qcom,require-tz-mapping");
}

static int msm_iommu_parse_dt(struct platform_device *pdev,
			      struct msm_iommu_drvdata *drvdata)
{
	struct device_node *child;
	int ret;

	drvdata->dev = &pdev->dev;

	ret = __get_bus_vote_client(pdev, drvdata);
	if (ret)
		goto fail;

	ret = msm_iommu_parse_bfb_settings(pdev, drvdata);
	if (ret)
		goto fail;

	for_each_available_child_of_node(pdev->dev.of_node, child)
		drvdata->ncb++;

	ret = of_property_read_string(pdev->dev.of_node, "label",
				      &drvdata->name);
	if (ret)
		goto fail;

	drvdata->sec_id = -1;
	get_secure_id(pdev->dev.of_node, drvdata);

	drvdata->halt_enabled = of_property_read_bool(pdev->dev.of_node,
						      "qcom,iommu-enable-halt");

	msm_iommu_add_drv(drvdata);

	return 0;

fail:
	__put_bus_vote_client(drvdata);
	return ret;
}

static int iommu_scm_set_pool_size(void)
{
	struct scm_desc desc = {0};
	int ret;

	desc.args[0] = MAXIMUM_VIRT_SIZE;
	desc.args[1] = 0;
	desc.arginfo = SCM_ARGS(2);

	ret = scm_call2(SCM_SIP_FNID(SCM_SVC_MP, IOMMU_SET_CP_POOL_SIZE),
			 &desc);
	if (ret)
		pr_err("%s: Failed to set CP pool size in secure env\n",
			__func__);

	return ret;
}

static int iommu_scm_secure_ptbl_init(u64 addr, u32 sz, u32 spare)
{
	struct scm_desc desc = {0};
	int ret;

	desc.args[0] = addr;
	desc.args[1] = sz;
	desc.args[2] = spare;
	desc.arginfo = SCM_ARGS(3, SCM_RW, SCM_VAL, SCM_VAL);

	ret = scm_call2(SCM_SIP_FNID(SCM_SVC_MP, IOMMU_SECURE_PTBL_INIT),
			&desc);
	if (ret == -EPERM)
		return 0;

	if (ret) {
		pr_err("%s: Failed to initialize secure partition table\n",
			__func__);
		return ret;
	}

	if (desc.ret[0]) {
		pr_err("%s: TZ Error while initializing secure"
			" partition table", __func__);
		ret = desc.ret[0];
	}

	return ret;
}

static int iommu_scm_get_secure_ptbl_size(u32 spare, size_t *size)
{
	struct scm_desc desc = {0};
	int ret;

	desc.args[0] = spare;
	desc.arginfo = SCM_ARGS(1);

	ret = scm_call2(SCM_SIP_FNID(SCM_SVC_MP, IOMMU_SECURE_PTBL_SIZE),
			&desc);
	if (ret) {
		pr_err("%s: Failed to get secure partition table size\n",
			__func__);
		return ret;
	}

	if (desc.ret[1]) {
		pr_err("%s: TZ Error while getting secure partition table\n",
			__func__);
		return -EINVAL;
	}

	if (size)
		*size = desc.ret[0];

	return desc.ret[1];
}

static int msm_iommu_sec_ptbl_init(struct device *dev)
{
	size_t psize;
	unsigned int spare = 0;
	int ret;
	void *cpu_addr;
	dma_addr_t paddr;
	static bool allocated = false;

	if (allocated)
		return 0;

	ret = scm_get_feat_version(SCM_SVC_MP);
	if (ret < 0) {
		pr_err("ERROR: Cannot get SCM version!\n");
		return -EINVAL;
	}

	if (ret >= MAKE_VERSION(1, 1, 1)) {
		ret = iommu_scm_set_pool_size();
		if (ret) {
			dev_err(dev, "failed setting max virtual size (%d)\n",
				ret);
			return ret;
		}
	}

	ret = iommu_scm_get_secure_ptbl_size(spare, &psize);
	if (ret) {
		dev_err(dev, "failed to get iommu secure pgtable size (%d)\n",
			ret);
		return ret;
	}

	dev_info(dev, "iommu sec: pgtable size: %zu\n", psize);

	cpu_addr = dma_alloc_attrs(dev, psize, &paddr, GFP_KERNEL,
			DMA_ATTR_NO_KERNEL_MAPPING);
	if (!cpu_addr) {
		dev_err(dev, "failed to allocate %zu bytes for pgtable\n",
			psize);
		return -ENOMEM;
	}

	ret = iommu_scm_secure_ptbl_init(paddr, psize, spare);
	if (ret) {
		dev_err(dev, "failed to init iommu pgtable (%d)\n", ret);
		goto free_mem;
	}

	allocated = true;

	return 0;

free_mem:
	dma_free_attrs(dev, psize, cpu_addr, paddr,
		DMA_ATTR_NO_KERNEL_MAPPING);
	return ret;
}

static int msm_iommu_dump_fault_regs(int smmu_id, int cb_num,
				struct msm_scm_fault_regs_dump *regs)
{
	int ret;
	struct scm_desc desc = {0};

	struct msm_scm_fault_regs_dump_req {
		uint32_t id;
		uint32_t cb_num;
		uint32_t buff;
		uint32_t len;
	} req_info;

	desc.args[0] = req_info.id = smmu_id;
	desc.args[1] = req_info.cb_num = cb_num;
	/* virt_to_phys(regs) may be greater than 4GB */
	req_info.buff = virt_to_phys(regs);
	desc.args[2] =  virt_to_phys(regs);
	desc.args[3] = req_info.len = sizeof(*regs);
	desc.arginfo = SCM_ARGS(4, SCM_VAL, SCM_VAL, SCM_RW, SCM_VAL);

	//__dma_flush_area(regs, sizeof(*regs));
	dmac_clean_range(regs, regs + 1);

	ret = scm_call2(SCM_SIP_FNID(SCM_SVC_UTIL,
			IOMMU_DUMP_SMMU_FAULT_REGS), &desc);

	dmac_inv_range(regs, regs + 1);
	//__dma_flush_area(regs, sizeof(*regs));

	return ret;
}

static int msm_iommu_reg_dump_to_regs(
	struct msm_iommu_context_reg ctx_regs[],
	struct msm_scm_fault_regs_dump *dump, struct msm_iommu_drvdata *drvdata,
	struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	int i, j, ret = 0;
	const uint32_t nvals = (dump->dump_size / sizeof(uint32_t));
	uint32_t *it = (uint32_t *) dump->dump_data;
	const uint32_t * const end = ((uint32_t *) dump) + nvals;
	phys_addr_t phys_base = drvdata->phys_base;
	int ctx = ctx_drvdata->num;

	if (!nvals)
		return -EINVAL;

	for (i = 1; it < end; it += 2, i += 2) {
		unsigned int reg_offset;
		uint32_t addr	= *it;
		uint32_t val	= *(it + 1);
		struct msm_iommu_context_reg *reg = NULL;
		if (addr < phys_base) {
			pr_err("Bogus-looking register (0x%x) for Iommu with base at %pa. Skipping.\n",
				addr, &phys_base);
			continue;
		}
		reg_offset = addr - phys_base;

		for (j = 0; j < MAX_DUMP_REGS; ++j) {
			struct dump_regs_tbl_entry dump_reg = dump_regs_tbl[j];
			void *test_reg;
			unsigned int test_offset;
			switch (dump_reg.dump_reg_type) {
			case DRT_CTX_REG:
				test_reg = CTX_REG(dump_reg.reg_offset,
					drvdata->cb_base, ctx);
				break;
			case DRT_GLOBAL_REG:
				test_reg = GLB_REG(
					dump_reg.reg_offset, drvdata->glb_base);
				break;
			case DRT_GLOBAL_REG_N:
				test_reg = GLB_REG_N(
					drvdata->glb_base, ctx,
					dump_reg.reg_offset);
				break;
			default:
				pr_err("Unknown dump_reg_type: 0x%x\n",
					dump_reg.dump_reg_type);
				BUG();
				break;
			}
			test_offset = test_reg - drvdata->glb_base;
			if (test_offset == reg_offset) {
				reg = &ctx_regs[j];
				break;
			}
		}

		if (reg == NULL) {
			pr_debug("Unknown register in secure CB dump: %x\n",
				addr);
			continue;
		}

		if (reg->valid) {
			WARN(1, "Invalid (repeated?) register in CB dump: %x\n",
				addr);
			continue;
		}

		reg->val = val;
		reg->valid = true;
	}

	if (i != nvals) {
		pr_err("Invalid dump! %d != %d\n", i, nvals);
		ret = 1;
	}

	for (i = 0; i < MAX_DUMP_REGS; ++i) {
		if (!ctx_regs[i].valid) {
			if (dump_regs_tbl[i].must_be_present) {
				pr_err("Register missing from dump for ctx %d: %s, 0x%x\n",
					ctx,
					dump_regs_tbl[i].name,
					dump_regs_tbl[i].reg_offset);
				ret = 1;
			}
			ctx_regs[i].val = 0xd00dfeed;
		}
	}

	return ret;
}

irqreturn_t msm_iommu_secure_fault_handler_v2(int irq, void *dev_id)
{
	struct platform_device *pdev = dev_id;
	struct msm_iommu_drvdata *drvdata;
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	struct msm_scm_fault_regs_dump *regs;
	int tmp, ret = IRQ_HANDLED;

	iommu_access_ops->iommu_lock_acquire(0);

	BUG_ON(!pdev);

	drvdata = dev_get_drvdata(pdev->dev.parent);
	BUG_ON(!drvdata);

	ctx_drvdata = dev_get_drvdata(&pdev->dev);
	BUG_ON(!ctx_drvdata);

	regs = kzalloc(sizeof(*regs), GFP_ATOMIC);
	if (!regs) {
		pr_err("%s: Couldn't allocate memory\n", __func__);
		goto lock_release;
	}

	if (!drvdata->ctx_attach_count) {
		pr_err("Unexpected IOMMU page fault from secure context bank!\n");
		pr_err("name = %s\n", drvdata->name);
		pr_err("Power is OFF. Unable to read page fault information\n");
		/*
		 * We cannot determine which context bank caused the issue so
		 * we just return handled here to ensure IRQ handler code is
		 * happy
		 */
		goto free_regs;
	}

	iommu_access_ops->iommu_clk_on(drvdata);
	tmp = msm_iommu_dump_fault_regs(drvdata->sec_id,
					ctx_drvdata->num, regs);

	if (tmp) {
		pr_err("%s: Couldn't dump fault registers (%d) %s, ctx: %d\n",
			__func__, tmp, drvdata->name, ctx_drvdata->num);
		goto free_regs;
	} else {
		struct msm_iommu_context_reg ctx_regs[MAX_DUMP_REGS];
		memset(ctx_regs, 0, sizeof(ctx_regs));
		tmp = msm_iommu_reg_dump_to_regs(
			ctx_regs, regs, drvdata, ctx_drvdata);
		if (tmp < 0) {
			ret = IRQ_NONE;
			pr_err("Incorrect response from secure environment\n");
			goto free_regs;
		}

		if (ctx_regs[DUMP_REG_FSR].val) {
			if (tmp)
				pr_err("Incomplete fault register dump. Printout will be incomplete.\n");
			if (!ctx_drvdata->attached_domain) {
				pr_err("Bad domain in interrupt handler\n");
				tmp = -ENOSYS;
			} else {
				tmp = report_iommu_fault(
					ctx_drvdata->attached_domain,
					&ctx_drvdata->pdev->dev,
					COMBINE_DUMP_REG(
						ctx_regs[DUMP_REG_FAR1].val,
						ctx_regs[DUMP_REG_FAR0].val),
					0);
			}

			/* if the fault wasn't handled by someone else: */
			if (tmp == -ENOSYS) {
				pr_err("Unexpected IOMMU page fault from secure context bank!\n");
				pr_err("name = %s\n", drvdata->name);
				pr_err("context = %s (%d)\n", ctx_drvdata->name,
					ctx_drvdata->num);
				pr_err("Interesting registers:\n");
				print_ctx_regs(ctx_regs);
			}
		} else {
			ret = IRQ_NONE;
		}
	}
free_regs:
	iommu_access_ops->iommu_clk_off(drvdata);
	kfree(regs);
lock_release:
	iommu_access_ops->iommu_lock_release(0);
	return ret;
}



static int msm_iommu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct msm_iommu_drvdata *drvdata;
	struct resource *res;
	resource_size_t ioaddr;
	int ret;
	int global_cfg_irq, global_client_irq;
	u32 temp;
	unsigned long rate;

	drvdata = devm_kzalloc(dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	drvdata->dev = dev;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "iommu_base");
	drvdata->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(drvdata->base))
		return PTR_ERR(drvdata->base);

	drvdata->glb_base = drvdata->base;
	drvdata->phys_base = ioaddr = res->start;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					   "smmu_local_base");
	drvdata->smmu_local_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(drvdata->smmu_local_base) &&
	    PTR_ERR(drvdata->smmu_local_base) != -EPROBE_DEFER)
		drvdata->smmu_local_base = NULL;

	if (of_device_is_compatible(np, "qcom,msm-mmu-500"))
		drvdata->model = MMU_500;

	drvdata->iface = devm_clk_get(dev, "iface_clk");
	if (IS_ERR(drvdata->iface))
		return PTR_ERR(drvdata->iface);

	ret = clk_prepare(drvdata->iface);
	if (ret)
		return ret;

	drvdata->core = devm_clk_get(dev, "core_clk");
	if (IS_ERR(drvdata->core)) {
		clk_unprepare(drvdata->iface);
		return PTR_ERR(drvdata->core);
	}

	ret = clk_prepare(drvdata->core);
	if (ret)
		return ret;

	if (!of_property_read_u32(np, "qcom,cb-base-offset", &temp))
		drvdata->cb_base = drvdata->base + temp;
	else
		drvdata->cb_base = drvdata->base + 0x8000;

	rate = clk_get_rate(drvdata->core);
	if (!rate) {
		rate = clk_round_rate(drvdata->core, 1000);
		clk_set_rate(drvdata->core, rate);
	}

	dev_info(&pdev->dev, "iface: %lu, core: %lu\n",
		 clk_get_rate(drvdata->iface), clk_get_rate(drvdata->core));

	ret = msm_iommu_parse_dt(pdev, drvdata);
	if (ret)
		return ret;

	dev_info(dev, "device %s (model: %d) mapped at %p, with %d ctx banks\n",
		 drvdata->name, drvdata->model, drvdata->base, drvdata->ncb);

	if (drvdata->sec_id != -1) {
		ret = msm_iommu_sec_ptbl_init(dev);
		if (ret)
			return ret;
	}

	platform_set_drvdata(pdev, drvdata);

	global_cfg_irq = platform_get_irq_byname(pdev, "global_cfg_NS_irq");
	if (global_cfg_irq < 0 && global_cfg_irq == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (global_cfg_irq > 0) {
		ret = devm_request_threaded_irq(dev, global_cfg_irq,
						NULL,
						msm_iommu_global_fault_handler,
						IRQF_ONESHOT | IRQF_SHARED,
						"msm_iommu_global_cfg_irq",
						pdev);
		if (ret < 0)
			dev_err(dev, "Request Global CFG IRQ %d failed with ret=%d\n",
				global_cfg_irq, ret);
	}

	global_client_irq =
			platform_get_irq_byname(pdev, "global_client_NS_irq");
	if (global_client_irq < 0 && global_client_irq == -EPROBE_DEFER)
		return -EPROBE_DEFER;

	if (global_client_irq > 0) {
		ret = devm_request_threaded_irq(dev, global_client_irq,
						NULL,
						msm_iommu_global_fault_handler,
						IRQF_ONESHOT | IRQF_SHARED,
						"msm_iommu_global_client_irq",
						pdev);
		if (ret < 0)
			dev_err(dev, "Request Global Client IRQ %d failed with ret=%d\n",
				global_client_irq, ret);
	}

	INIT_LIST_HEAD(&drvdata->masters);

	idr_init(&drvdata->asid_idr);

	ret = of_platform_populate(np, msm_iommu_ctx_match_table, NULL, dev);
	if (ret) {
		dev_err(dev, "Failed to create iommu context device\n");
		return ret;
	}

	ret = __enable_clocks(drvdata);
	if (ret) {
		dev_err(dev, "Failed to enable clocks\n");
		return ret;
	}

	ret = iommu_device_sysfs_add(&drvdata->iommu, dev, NULL,
				     "msm-iommu.%pa", &ioaddr);
	if (ret) {
		dev_err(dev, "Cannot add msm-iommu.%pa to sysfs\n", &ioaddr);
		return ret;
	}

	return msm_iommu_init(drvdata);
}

static int msm_iommu_remove(struct platform_device *pdev)
{
	struct msm_iommu_drvdata *drv;

	drv = platform_get_drvdata(pdev);
	if (drv) {
		idr_destroy(&drv->asid_idr);
		__disable_clocks(drv);
		__put_bus_vote_client(drv);
		clk_unprepare(drv->iface);
		clk_unprepare(drv->core);
		msm_iommu_remove_drv(drv);
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

static int msm_iommu_ctx_parse_dt(struct platform_device *pdev,
				  struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	struct resource *r, rp;
	int irq = 0, ret = 0;
	struct msm_iommu_drvdata *drvdata;
	u32 nsid;
	u32 n_sid_mask;
	unsigned long cb_offset;

	drvdata = dev_get_drvdata(pdev->dev.parent);
	if (!drvdata)
		return -EPROBE_DEFER;

	get_secure_ctx(pdev->dev.of_node, ctx_drvdata);
	get_secure_mapping(pdev->dev.of_node, ctx_drvdata);

	if (drvdata->sec_id != -1) {
		irq = platform_get_irq(pdev, 1);
		if (irq < 0 && irq == -EPROBE_DEFER)
			return -EPROBE_DEFER;

		if (irq > 0) {
			ret = devm_request_threaded_irq(&pdev->dev, irq, NULL,
					msm_iommu_secure_fault_handler_v2,
					IRQF_ONESHOT | IRQF_SHARED,
					"msm_iommu_secure_irq", pdev);
			if (ret) {
				pr_err("Request IRQ %d failed with ret=%d\n",
					irq, ret);
				return ret;
			}
		}
	} else {
		irq = platform_get_irq(pdev, 0);
		if (irq < 0 && irq == -EPROBE_DEFER)
			return -EPROBE_DEFER;

		if (irq > 0) {
			ret = devm_request_threaded_irq(&pdev->dev, irq, NULL,
					msm_iommu_fault_handler_v2,
					IRQF_ONESHOT | IRQF_SHARED,
					"msm_iommu_nonsecure_irq", pdev);
			if (ret) {
				pr_err("Request IRQ %d failed with ret=%d\n",
					irq, ret);
				goto out;
			}
		}
	}

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		ret = -EINVAL;
		goto out;
	}

	ret = of_address_to_resource(pdev->dev.parent->of_node, 0, &rp);
	if (ret)
		goto out;

	/* Calculate the context bank number using the base addresses.
	 * Typically CB0 base address is 0x8000 pages away if the number
	 * of CBs are <=8. So, assume the offset 0x8000 until mentioned
	 * explicitely.
	 */
	cb_offset = drvdata->cb_base - drvdata->base;
	ctx_drvdata->num = (r->start - rp.start - cb_offset) >> CTX_SHIFT;

	if (of_property_read_string(pdev->dev.of_node, "label",
				    &ctx_drvdata->name))
		ctx_drvdata->name = dev_name(&pdev->dev);

	if (!of_get_property(pdev->dev.of_node, "qcom,iommu-ctx-sids", &nsid)) {
		ret = -EINVAL;
		goto out;
	}

	if (nsid >= sizeof(ctx_drvdata->sids)) {
		ret = -EINVAL;
		goto out;
	}

	if (of_property_read_u32_array(pdev->dev.of_node, "qcom,iommu-ctx-sids",
				       ctx_drvdata->sids,
				       nsid / sizeof(*ctx_drvdata->sids))) {
		ret = -EINVAL;
		goto out;
	}

	ctx_drvdata->nsid = nsid;
	ctx_drvdata->asid = -1;

	if (!of_get_property(pdev->dev.of_node, "qcom,iommu-sid-mask",
			     &n_sid_mask)) {
		memset(ctx_drvdata->sid_mask, 0, sizeof(*ctx_drvdata->sid_mask));
		goto out;
	}

	if (n_sid_mask != nsid) {
		ret = -EINVAL;
		goto out;
	}

	if (of_property_read_u32_array(pdev->dev.of_node, "qcom,iommu-sid-mask",
				ctx_drvdata->sid_mask,
				n_sid_mask / sizeof(*ctx_drvdata->sid_mask))) {
		ret = -EINVAL;
		goto out;
	}

	ctx_drvdata->n_sid_mask = n_sid_mask;

out:
	return ret;
}

static int msm_iommu_ctx_probe(struct platform_device *pdev)
{
	struct msm_iommu_ctx_drvdata *ctx_drvdata;
	int ret;

	if (!pdev->dev.parent)
		return -EINVAL;

	ctx_drvdata = devm_kzalloc(&pdev->dev, sizeof(*ctx_drvdata),
				   GFP_KERNEL);
	if (!ctx_drvdata)
		return -ENOMEM;

	ctx_drvdata->pdev = pdev;
	INIT_LIST_HEAD(&ctx_drvdata->attached_elm);

	ret = msm_iommu_ctx_parse_dt(pdev, ctx_drvdata);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, ctx_drvdata);

	dev_info(&pdev->dev, "context %s using bank %d\n",
		 ctx_drvdata->name, ctx_drvdata->num);

	return 0;
}

static int msm_iommu_ctx_remove(struct platform_device *pdev)
{
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id msm_iommu_match_table[] = {
	{ .compatible = "qcom,msm-smmu-v1", },
	{ .compatible = "qcom,msm-smmu-v2", },
	{}
};

static struct platform_driver msm_iommu_driver = {
	.driver = {
		.name = "msm_iommu",
		.of_match_table = msm_iommu_match_table,
	},
	.probe = msm_iommu_probe,
	.remove = msm_iommu_remove,
};

static const struct of_device_id msm_iommu_ctx_match_table[] = {
	{ .compatible = "qcom,msm-smmu-v1-ctx", },
	{ .compatible = "qcom,msm-smmu-v2-ctx", },
	{}
};

static struct platform_driver msm_iommu_ctx_driver = {
	.driver = {
		.name = "msm_iommu_ctx",
		.of_match_table = msm_iommu_ctx_match_table,
	},
	.probe = msm_iommu_ctx_probe,
	.remove = msm_iommu_ctx_remove,
};

static int __init msm_iommu_driver_init(void)//struct device_node *np)
{
	int ret;

	msm_iommu_check_scm_call_avail();

	iommu_access_ops = &iommu_access_ops_v1;

	ret = platform_driver_register(&msm_iommu_driver);
	if (ret) {
		pr_err("Failed to register IOMMU driver\n");
		return ret;
	}

	ret = platform_driver_register(&msm_iommu_ctx_driver);
	if (ret) {
		pr_err("Failed to register IOMMU context driver\n");
		platform_driver_unregister(&msm_iommu_driver);
		return ret;
	}

	return 0;
}
//IOMMU_OF_DECLARE(msm_mmuv1, "qcom,msm-mmu-500", msm_iommu_driver_init);
subsys_initcall(msm_iommu_driver_init);

static void __exit msm_iommu_driver_exit(void)
{
	platform_driver_unregister(&msm_iommu_ctx_driver);
	platform_driver_unregister(&msm_iommu_driver);
}
module_exit(msm_iommu_driver_exit);

MODULE_LICENSE("GPL v2");
