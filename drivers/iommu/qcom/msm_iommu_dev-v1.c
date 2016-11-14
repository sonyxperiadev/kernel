/* Copyright (c) 2012-2014, The Linux Foundation. All rights reserved.
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
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>

#include "msm_iommu_hw-v1.h"
#include "qcom_iommu.h"
#include <linux/qcom_scm.h>
#include "msm_iommu_perfmon.h"
#include "msm_iommu_priv.h"

static const struct of_device_id msm_iommu_ctx_match_table[];

#ifdef CONFIG_IOMMU_LPAE
static const char *BFB_REG_NODE_NAME = "qcom,iommu-lpae-bfb-regs";
static const char *BFB_DATA_NODE_NAME = "qcom,iommu-lpae-bfb-data";
#else
static const char *BFB_REG_NODE_NAME = "qcom,iommu-bfb-regs";
static const char *BFB_DATA_NODE_NAME = "qcom,iommu-bfb-data";
#endif

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
	return 0;
}

static void __put_bus_vote_client(struct msm_iommu_drvdata *drvdata)
{
	drvdata->bus_client = 0;
}

/*
 * CONFIG_IOMMU_NON_SECURE allows us to override the secure
 * designation of SMMUs in device tree. With this config enabled
 * all SMMUs will be programmed by this driver.
 */
#ifdef CONFIG_IOMMU_NON_SECURE
static inline void get_secure_id(struct device_node *node,
				 struct msm_iommu_drvdata *drvdata)
{
}

static inline void get_secure_ctx(struct device_node *node,
				  struct msm_iommu_drvdata *iommu_drvdata,
				  struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	ctx_drvdata->secure_context = 0;
}
#else
static void get_secure_id(struct device_node *node,
			  struct msm_iommu_drvdata *drvdata)
{
	if (msm_iommu_get_scm_call_avail())
		of_property_read_u32(node, "qcom,iommu-secure-id",
				     &drvdata->sec_id);
}

static void get_secure_ctx(struct device_node *node,
			   struct msm_iommu_drvdata *iommu_drvdata,
			   struct msm_iommu_ctx_drvdata *ctx_drvdata)
{
	u32 secure_ctx = 0;

	if (msm_iommu_get_scm_call_avail())
		secure_ctx = of_property_read_bool(node, "qcom,secure-context");

	ctx_drvdata->secure_context = secure_ctx;
}
#endif

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

static int msm_iommu_pmon_parse_dt(struct platform_device *pdev,
				   struct iommu_pmon *pmon_info)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	unsigned int cls_prop_size;
	int ret, irq;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0 && irq == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (irq <= 0) {
		pmon_info->iommu.evt_irq = -1;
		return irq;
	}

	pmon_info->iommu.evt_irq = irq;

	ret = of_property_read_u32(np, "qcom,iommu-pmu-ngroups",
				   &pmon_info->num_groups);
	if (ret) {
		dev_err(dev, "Error reading qcom,iommu-pmu-ngroups\n");
		return ret;
	}

	ret = of_property_read_u32(np, "qcom,iommu-pmu-ncounters",
				   &pmon_info->num_counters);
	if (ret) {
		dev_err(dev, "Error reading qcom,iommu-pmu-ncounters\n");
		return ret;
	}

	if (!of_get_property(np, "qcom,iommu-pmu-event-classes",
			     &cls_prop_size)) {
		dev_err(dev, "Error reading qcom,iommu-pmu-event-classes\n");
		return -EINVAL;
	}

	pmon_info->event_cls_supported = devm_kzalloc(dev, cls_prop_size,
						      GFP_KERNEL);
	if (!pmon_info->event_cls_supported) {
		dev_err(dev, "Unable to get memory for event class array\n");
		return -ENOMEM;
	}

	pmon_info->nevent_cls_supported = cls_prop_size / sizeof(u32);

	ret = of_property_read_u32_array(np, "qcom,iommu-pmu-event-classes",
					 pmon_info->event_cls_supported,
					 pmon_info->nevent_cls_supported);
	if (ret) {
		dev_err(dev, "Error reading qcom,iommu-pmu-event-classes\n");
		return ret;
	}

	return 0;
}
#if 0
#define SCM_SVC_MP		0xc
#define MAXIMUM_VIRT_SIZE	(300 * SZ_1M)
#define MAKE_VERSION(major, minor, patch) \
	(((major & 0x3FF) << 22) | ((minor & 0x3FF) << 12) | (patch & 0xFFF))

static int msm_iommu_sec_ptbl_init(struct device *dev)
{
	int psize[2] = {0, 0};
	unsigned int spare = 0;
	int ret;
	int version;
	void *cpu_addr;
	dma_addr_t paddr;
	DEFINE_DMA_ATTRS(attrs);
	static bool allocated = false;

	if (allocated)
		return 0;

	version = qcom_scm_get_feat_version(SCM_SVC_MP);

	if (version >= MAKE_VERSION(1, 1, 1)) {
		ret = qcom_scm_iommu_set_cp_pool_size(MAXIMUM_VIRT_SIZE, 0);
		if (ret) {
			dev_err(dev, "failed setting max virtual size (%d)\n",
				ret);
			return ret;
		}
	}

	ret = qcom_scm_iommu_secure_ptbl_size(spare, psize);
	if (ret) {
		dev_err(dev, "failed to get iommu secure pgtable size (%d)\n",
			ret);
		return ret;
	}

	if (psize[1]) {
		dev_err(dev, "failed to get iommu secure pgtable size (%d)\n",
			ret);
		return psize[1];
	}

	dev_info(dev, "iommu sec: pgtable size: %d\n", psize[0]);

	dma_set_attr(DMA_ATTR_NO_KERNEL_MAPPING, &attrs);

	cpu_addr = dma_alloc_attrs(dev, psize[0], &paddr, GFP_KERNEL, &attrs);
	if (!cpu_addr) {
		dev_err(dev, "failed to allocate %d bytes for pgtable\n",
			psize[0]);
		return -ENOMEM;
	}

	ret = qcom_scm_iommu_secure_ptbl_init(paddr, psize[0], spare);
	if (ret) {
		dev_err(dev, "failed to init iommu pgtable (%d)\n", ret);
		goto free_mem;
	}

	allocated = true;

	return 0;

free_mem:
	dma_free_attrs(dev, psize[0], cpu_addr, paddr, &attrs);
	return ret;
}
#endif

static int msm_iommu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct iommu_pmon *pmon_info;
	struct msm_iommu_drvdata *drvdata;
	struct resource *res;
	int ret;
	int global_cfg_irq, global_client_irq;
	u32 temp;
	unsigned long rate;

	if (!qcom_scm_is_available())
		return -EPROBE_DEFER;

	msm_iommu_check_scm_call_avail();
	msm_set_iommu_access_ops(&iommu_access_ops_v1);
	msm_iommu_sec_set_access_ops(&iommu_access_ops_v1);

	drvdata = devm_kzalloc(dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	drvdata->dev = dev;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "iommu_base");
	drvdata->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(drvdata->base))
		return PTR_ERR(drvdata->base);

	drvdata->glb_base = drvdata->base;
	drvdata->phys_base = res->start;

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

	drvdata->core = devm_clk_get(dev, "core_clk");
	if (IS_ERR(drvdata->core))
		return PTR_ERR(drvdata->core);

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

#if 0
	if (drvdata->sec_id != -1) {
		ret = msm_iommu_sec_ptbl_init(dev);
		if (ret)
			return ret;
	}
#endif
	platform_set_drvdata(pdev, drvdata);

	pmon_info = msm_iommu_pm_alloc(dev);
	if (pmon_info) {
		ret = msm_iommu_pmon_parse_dt(pdev, pmon_info);
		if (ret) {
			msm_iommu_pm_free(dev);
			dev_info(dev, "%s: pmon not available\n",
				 drvdata->name);
		} else {
			pmon_info->iommu.base = drvdata->base;
			pmon_info->iommu.ops = msm_get_iommu_access_ops();
			pmon_info->iommu.hw_ops = iommu_pm_get_hw_ops_v1();
			pmon_info->iommu.iommu_name = drvdata->name;
			ret = msm_iommu_pm_iommu_register(pmon_info);
			if (ret) {
				dev_err(dev, "%s iommu register fail\n",
					drvdata->name);
				msm_iommu_pm_free(dev);
			} else {
				dev_dbg(dev, "%s iommu registered for pmon\n",
					pmon_info->iommu.iommu_name);
			}
		}
	}

	global_cfg_irq = platform_get_irq_byname(pdev, "global_cfg_NS_irq");
	if (global_cfg_irq < 0 && global_cfg_irq == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (global_cfg_irq > 0) {
		ret = devm_request_threaded_irq(dev, global_cfg_irq,
						NULL,
						msm_iommu_global_fault_handler,
						IRQF_ONESHOT | IRQF_SHARED /*|
						IRQF_TRIGGER_RISING*/,
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
						IRQF_ONESHOT | IRQF_SHARED /*|
						IRQF_TRIGGER_RISING*/,
						"msm_iommu_global_client_irq",
						pdev);
		if (ret < 0)
			dev_err(dev, "Request Global Client IRQ %d failed with ret=%d\n",
				global_client_irq, ret);
	}

	INIT_LIST_HEAD(&drvdata->masters);

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

	return msm_iommu_init(&pdev->dev);
}

static int msm_iommu_remove(struct platform_device *pdev)
{
	struct msm_iommu_drvdata *drv;

	msm_iommu_pm_iommu_unregister(&pdev->dev);
	msm_iommu_pm_free(&pdev->dev);

	drv = platform_get_drvdata(pdev);
	if (drv) {
		__disable_clocks(drv);
		__put_bus_vote_client(drv);
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

	get_secure_ctx(pdev->dev.of_node, drvdata, ctx_drvdata);

	if (ctx_drvdata->secure_context) {
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
		memset(ctx_drvdata->sid_mask, 0, MAX_NUM_SMR);
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

	if (!qcom_scm_is_available())
		return -EPROBE_DEFER;

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

static int __init msm_iommu_driver_init(void)
{
	int ret;

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

static void __exit msm_iommu_driver_exit(void)
{
	platform_driver_unregister(&msm_iommu_ctx_driver);
	platform_driver_unregister(&msm_iommu_driver);
}
subsys_initcall(msm_iommu_driver_init);
module_exit(msm_iommu_driver_exit);

MODULE_LICENSE("GPL v2");
