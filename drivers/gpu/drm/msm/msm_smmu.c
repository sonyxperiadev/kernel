/*
 * Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>
#include <linux/msm_dma_iommu_mapping.h>

#include <asm/dma-iommu.h>
#include <soc/qcom/secure_buffer.h>

#include "msm_drv.h"
#include "msm_gem.h"
#include "msm_mmu.h"
#include "sde_dbg.h"

#ifndef SZ_4G
#define SZ_4G	(((size_t) SZ_1G) * 4)
#endif

#ifndef SZ_2G
#define SZ_2G	(((size_t) SZ_1G) * 2)
#endif

struct msm_smmu_client {
	struct device *dev;
	struct dma_iommu_mapping *mmu_mapping;
	bool domain_attached;
	bool secure;
};

struct msm_smmu {
	struct msm_mmu base;
	struct device *client_dev;
	struct msm_smmu_client *client;
};

struct msm_smmu_domain {
	const char *label;
	size_t va_start;
	size_t va_size;
	bool secure;
};

#define to_msm_smmu(x) container_of(x, struct msm_smmu, base)
#define msm_smmu_to_client(smmu) (smmu->client)

static int _msm_smmu_create_mapping(struct msm_smmu_client *client,
	const struct msm_smmu_domain *domain);

static int msm_smmu_attach(struct msm_mmu *mmu, const char * const *names,
									int cnt)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	struct iommu_group *grp = NULL;
	int rc = 0;

	if (!client) {
		pr_err("undefined smmu client\n");
		return -EINVAL;
	}

	/* domain attach only once */
	if (client->domain_attached)
		return 0;

	if (!client->dev->iommu_group) {
		 grp = iommu_group_get_for_dev(client->dev);
		 if (IS_ERR_OR_NULL(grp))
			  return PTR_ERR(grp);
	}

	rc = arm_iommu_attach_device(client->dev,
			client->mmu_mapping);
	if (rc) {
		dev_err(client->dev, "iommu attach dev failed (%d)\n",
				rc);
		return rc;
	}

	client->domain_attached = true;

	dev_dbg(client->dev, "iommu domain attached\n");

	return 0;
}

static void msm_smmu_detach(struct msm_mmu *mmu, const char * const *names,
									int cnt)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);

	if (!client) {
		pr_err("undefined smmu client\n");
		return;
	}

	if (!client->domain_attached)
		return;

	arm_iommu_detach_device(client->dev);
	client->domain_attached = false;
	dev_dbg(client->dev, "iommu domain detached\n");
}

static int msm_smmu_set_attribute(struct msm_mmu *mmu,
		enum iommu_attr attr, void *data)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	struct iommu_domain *domain;
	int ret = 0;

	if (!client || !client->mmu_mapping)
		return -ENODEV;

	domain = client->mmu_mapping->domain;
	if (!domain) {
		DRM_ERROR("Invalid domain ret:%d\n", ret);
		return -EINVAL;
	}

	ret = iommu_domain_set_attr(domain, attr, data);
	if (ret)
		DRM_ERROR("set domain attribute failed:%d\n", ret);

	return ret;
}

static int msm_smmu_one_to_one_unmap(struct msm_mmu *mmu,
				uint32_t dest_address, uint32_t size)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	struct iommu_domain *domain;
	int ret = 0;

	if (!client || !client->mmu_mapping)
		return -ENODEV;

	domain = client->mmu_mapping->domain;
	if (!domain)
		return -EINVAL;

	ret = iommu_unmap(domain, dest_address, size);
	if (ret != size)
		pr_err("smmu unmap failed\n");

	return 0;
}

static int msm_smmu_one_to_one_map(struct msm_mmu *mmu, uint32_t iova,
		uint32_t dest_address, uint32_t size, int prot)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	struct iommu_domain *domain;
	int ret = 0;

	if (!client || !client->mmu_mapping)
		return -ENODEV;

	domain = client->mmu_mapping->domain;
	if (!domain)
		return -EINVAL;

	ret = iommu_map(domain, dest_address, dest_address, size, prot);
	if (ret)
		pr_err("smmu map failed\n");

	return ret;
}

static int msm_smmu_map(struct msm_mmu *mmu, uint32_t iova,
		struct sg_table *sgt, int prot)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	struct iommu_domain *domain;
	struct scatterlist *sg;
	unsigned int da = iova;
	unsigned int i, j;
	int ret;

	if (!client)
		return -ENODEV;

	domain = client->mmu_mapping->domain;
	if (!domain || !sgt)
		return -EINVAL;

	for_each_sg(sgt->sgl, sg, sgt->nents, i) {
		u32 pa = sg_phys(sg) - sg->offset;
		size_t bytes = sg->length + sg->offset;

		VERB("map[%d]: %08x %08x(%zx)", i, iova, pa, bytes);

		ret = iommu_map(domain, da, pa, bytes, prot);
		if (ret)
			goto fail;

		da += bytes;
	}

	return 0;

fail:
	da = iova;

	for_each_sg(sgt->sgl, sg, i, j) {
		size_t bytes = sg->length + sg->offset;

		iommu_unmap(domain, da, bytes);
		da += bytes;
	}
	return ret;
}

static int msm_smmu_map_sg(struct msm_mmu *mmu, struct sg_table *sgt,
		enum dma_data_direction dir)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	int ret;

	ret = dma_map_sg(client->dev, sgt->sgl, sgt->nents, dir);
	if (ret != sgt->nents)
		return -ENOMEM;

	return 0;
}

static void msm_smmu_unmap_sg(struct msm_mmu *mmu, struct sg_table *sgt,
		enum dma_data_direction dir)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);

	dma_unmap_sg(client->dev, sgt->sgl, sgt->nents, dir);
}

static int msm_smmu_unmap(struct msm_mmu *mmu, uint32_t iova,
		struct sg_table *sgt)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	struct iommu_domain *domain;
	struct scatterlist *sg;
	unsigned int da = iova;
	int i;

	if (!client)
		return -ENODEV;

	domain = client->mmu_mapping->domain;
	if (!domain || !sgt)
		return -EINVAL;

	for_each_sg(sgt->sgl, sg, sgt->nents, i) {
		size_t bytes = sg->length + sg->offset;
		size_t unmapped;

		unmapped = iommu_unmap(domain, da, bytes);
		if (unmapped < bytes)
			return unmapped;

		VERB("unmap[%d]: %08x(%zx)", i, iova, bytes);

		WARN_ON(!PAGE_ALIGNED(bytes));

		da += bytes;
	}

	return 0;
}

static void msm_smmu_destroy(struct msm_mmu *mmu)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct platform_device *pdev = to_platform_device(smmu->client_dev);

	if (smmu->client_dev)
		platform_device_unregister(pdev);

	if (smmu)
		kfree(smmu);
}

static int msm_smmu_map_dma_buf(struct msm_mmu *mmu, struct sg_table *sgt,
			struct dma_buf *dma_buf, int dir, u32 flags)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);
	unsigned long attrs = 0x0;
	int ret;

	if (!sgt || !client) {
		DRM_ERROR("sg table is invalid\n");
		return -ENOMEM;
	}

	if (flags & MSM_BO_KEEPATTRS)
		attrs |= DMA_ATTR_IOMMU_USE_UPSTREAM_HINT;

	ret = msm_dma_map_sg_attrs(client->dev, sgt->sgl, sgt->nents, dir,
			dma_buf, attrs);
	if (ret != sgt->nents) {
		DRM_ERROR("dma map sg failed\n");
		return -ENOMEM;
	}

	if (sgt && sgt->sgl) {
		DRM_DEBUG("%pad/0x%x/0x%x/0x%lx\n", &sgt->sgl->dma_address,
				sgt->sgl->dma_length, dir, attrs);
		SDE_EVT32(sgt->sgl->dma_address, sgt->sgl->dma_length,
				dir, attrs, client->secure);
	}

	return 0;
}


static void msm_smmu_unmap_dma_buf(struct msm_mmu *mmu, struct sg_table *sgt,
			struct dma_buf *dma_buf, int dir)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);

	if (!sgt || !client) {
		DRM_ERROR("sg table is invalid\n");
		return;
	}

	if (sgt && sgt->sgl) {
		DRM_DEBUG("%pad/0x%x/0x%x\n", &sgt->sgl->dma_address,
				sgt->sgl->dma_length, dir);
		SDE_EVT32(sgt->sgl->dma_address, sgt->sgl->dma_length, dir,
			client->secure);
	}

	msm_dma_unmap_sg(client->dev, sgt->sgl, sgt->nents, dir, dma_buf);
}

static bool msm_smmu_is_domain_secure(struct msm_mmu *mmu)
{
	struct msm_smmu *smmu = to_msm_smmu(mmu);
	struct msm_smmu_client *client = msm_smmu_to_client(smmu);

	return client->secure;
}

static const struct msm_mmu_funcs funcs = {
	.attach = msm_smmu_attach,
	.detach = msm_smmu_detach,
	.map = msm_smmu_map,
	.map_sg = msm_smmu_map_sg,
	.unmap_sg = msm_smmu_unmap_sg,
	.unmap = msm_smmu_unmap,
	.map_dma_buf = msm_smmu_map_dma_buf,
	.unmap_dma_buf = msm_smmu_unmap_dma_buf,
	.destroy = msm_smmu_destroy,
	.is_domain_secure = msm_smmu_is_domain_secure,
	.set_attribute = msm_smmu_set_attribute,
	.one_to_one_map = msm_smmu_one_to_one_map,
	.one_to_one_unmap = msm_smmu_one_to_one_unmap,
};

static struct msm_smmu_domain msm_smmu_domains[MSM_SMMU_DOMAIN_MAX] = {
	[MSM_SMMU_DOMAIN_UNSECURE] = {
		.label = "mdp_ns",
		.va_start = SZ_2G,
		.va_size = SZ_4G - SZ_2G,
		.secure = false,
	},
	[MSM_SMMU_DOMAIN_SECURE] = {
		.label = "mdp_s",
		.va_start = SZ_2G,
		.va_size = SZ_4G - SZ_2G,
		.secure = true,
	},
	[MSM_SMMU_DOMAIN_NRT_UNSECURE] = {
		.label = "rot_ns",
		.va_start = SZ_2G,
		.va_size = SZ_4G - SZ_2G,
		.secure = false,
	},
	[MSM_SMMU_DOMAIN_NRT_SECURE] = {
		.label = "rot_s",
		.va_start = SZ_2G,
		.va_size = SZ_4G - SZ_2G,
		.secure = true,
	},
};

static const struct of_device_id msm_smmu_dt_match[] = {
	{ .compatible = "qcom,smmu_sde_unsec",
		.data = &msm_smmu_domains[MSM_SMMU_DOMAIN_UNSECURE] },
	{ .compatible = "qcom,smmu_sde_sec",
		.data = &msm_smmu_domains[MSM_SMMU_DOMAIN_SECURE] },
	{ .compatible = "qcom,smmu_sde_nrt_unsec",
		.data = &msm_smmu_domains[MSM_SMMU_DOMAIN_NRT_UNSECURE] },
	{ .compatible = "qcom,smmu_sde_nrt_sec",
		.data = &msm_smmu_domains[MSM_SMMU_DOMAIN_NRT_SECURE] },
	{}
};
MODULE_DEVICE_TABLE(of, msm_smmu_dt_match);

static struct device *msm_smmu_device_create(struct device *dev,
		enum msm_mmu_domain_type domain,
		struct msm_smmu *smmu)
{
	struct device_node *child;
	struct platform_device *pdev;
	int i;
	const char *compat = NULL;

	for (i = 0; i < ARRAY_SIZE(msm_smmu_dt_match); i++) {
		if (msm_smmu_dt_match[i].data == &msm_smmu_domains[domain]) {
			compat = msm_smmu_dt_match[i].compatible;
			break;
		}
	}

	if (!compat) {
		DRM_DEBUG("unable to find matching domain for %d\n", domain);
		return ERR_PTR(-ENOENT);
	}
	DRM_DEBUG("found domain %d compat: %s\n", domain, compat);

	if (domain == MSM_SMMU_DOMAIN_UNSECURE) {
		int rc;

		smmu->client = devm_kzalloc(dev,
				sizeof(struct msm_smmu_client), GFP_KERNEL);
		if (!smmu->client)
			return ERR_PTR(-ENOMEM);

		smmu->client->dev = dev;

		rc = _msm_smmu_create_mapping(msm_smmu_to_client(smmu),
			msm_smmu_dt_match[i].data);
		if (rc) {
			devm_kfree(dev, smmu->client);
			smmu->client = NULL;
			return ERR_PTR(rc);
		}

		return NULL;
	}

	child = of_find_compatible_node(dev->of_node, NULL, compat);
	if (!child) {
		DRM_DEBUG("unable to find compatible node for %s\n", compat);
		return ERR_PTR(-ENODEV);
	}

	pdev = of_platform_device_create(child, NULL, dev);
	if (!pdev) {
		DRM_ERROR("unable to create smmu platform dev for domain %d\n",
				domain);
		return ERR_PTR(-ENODEV);
	}

	smmu->client = platform_get_drvdata(pdev);

	return &pdev->dev;
}

struct msm_mmu *msm_smmu_new(struct device *dev,
		enum msm_mmu_domain_type domain)
{
	struct msm_smmu *smmu;
	struct device *client_dev;

	smmu = kzalloc(sizeof(*smmu), GFP_KERNEL);
	if (!smmu)
		return ERR_PTR(-ENOMEM);

	client_dev = msm_smmu_device_create(dev, domain, smmu);
	if (IS_ERR(client_dev)) {
		kfree(smmu);
		return (void *)client_dev ? : ERR_PTR(-ENODEV);
	}

	smmu->client_dev = client_dev;
	msm_mmu_init(&smmu->base, dev, &funcs);

	return &smmu->base;
}

static int msm_smmu_fault_handler(struct iommu_domain *domain,
		struct device *dev, unsigned long iova,
		int flags, void *token)
{
	struct msm_smmu_client *client;
	int rc = -EINVAL;

	if (!token) {
		DRM_ERROR("Error: token is NULL\n");
		return -EINVAL;
	}

	client = (struct msm_smmu_client *)token;

	/* see iommu.h for fault flags definition */
	SDE_EVT32(iova, flags);
	DRM_ERROR("trigger dump, iova=0x%08lx, flags=0x%x\n", iova, flags);
	DRM_ERROR("SMMU device:%s", client->dev ? client->dev->kobj.name : "");

	/* generate dump, but no panic */
	SDE_DBG_DUMP("all", "dbg_bus", "vbif_dbg_bus");

	/*
	 * return -ENOSYS to allow smmu driver to dump out useful
	 * debug info.
	 */
	return rc;
}

static int _msm_smmu_create_mapping(struct msm_smmu_client *client,
	const struct msm_smmu_domain *domain)
{
	int rc;
	int mdphtw_llc_enable = 1;

	client->mmu_mapping = arm_iommu_create_mapping(&platform_bus_type,
			domain->va_start, domain->va_size);
	if (IS_ERR(client->mmu_mapping)) {
		dev_err(client->dev,
			"iommu create mapping failed for domain=%s\n",
			domain->label);
		return PTR_ERR(client->mmu_mapping);
	}

	rc = iommu_domain_set_attr(client->mmu_mapping->domain,
			DOMAIN_ATTR_USE_UPSTREAM_HINT, &mdphtw_llc_enable);
	if (rc) {
		dev_err(client->dev, "couldn't enable mdp pagetable walks: %d\n",
			rc);
		goto error;
	}

	if (domain->secure) {
		int secure_vmid = VMID_CP_PIXEL;

		client->secure = true;
		rc = iommu_domain_set_attr(client->mmu_mapping->domain,
				DOMAIN_ATTR_SECURE_VMID, &secure_vmid);
		if (rc) {
			dev_err(client->dev, "couldn't set secure pix vmid\n");
			goto error;
		}
	}

	iommu_set_fault_handler(client->mmu_mapping->domain,
			msm_smmu_fault_handler, (void *)client);

	DRM_INFO("Created domain %s [%zx,%zx] secure=%d\n",
			domain->label, domain->va_start, domain->va_size,
			domain->secure);

	return 0;

error:
	arm_iommu_release_mapping(client->mmu_mapping);
	return rc;
}

/**
 * msm_smmu_probe()
 * @pdev: platform device
 *
 * Each smmu context acts as a separate device and the context banks are
 * configured with a VA range.
 * Registers the clks as each context bank has its own clks, for which voting
 * has to be done everytime before using that context bank.
 */
static int msm_smmu_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct msm_smmu_client *client;
	const struct msm_smmu_domain *domain;
	int rc;

	match = of_match_device(msm_smmu_dt_match, &pdev->dev);
	if (!match || !match->data) {
		dev_err(&pdev->dev, "probe failed as match data is invalid\n");
		return -EINVAL;
	}

	domain = match->data;
	if (!domain) {
		dev_err(&pdev->dev, "no matching device found\n");
		return -EINVAL;
	}

	DRM_INFO("probing device %s\n", match->compatible);

	client = devm_kzalloc(&pdev->dev, sizeof(*client), GFP_KERNEL);
	if (!client)
		return -ENOMEM;

	client->dev = &pdev->dev;

	rc = _msm_smmu_create_mapping(client, domain);
	platform_set_drvdata(pdev, client);

	return rc;
}

static int msm_smmu_remove(struct platform_device *pdev)
{
	struct msm_smmu_client *client;

	client = platform_get_drvdata(pdev);
	if (client->domain_attached) {
		arm_iommu_detach_device(client->dev);
		client->domain_attached = false;
	}
	arm_iommu_release_mapping(client->mmu_mapping);

	return 0;
}

static struct platform_driver msm_smmu_driver = {
	.probe = msm_smmu_probe,
	.remove = msm_smmu_remove,
	.driver = {
		.name = "msmdrm_smmu",
		.of_match_table = msm_smmu_dt_match,
		.suppress_bind_attrs = true,
	},
};

int __init msm_smmu_driver_init(void)
{
	int ret;

	ret = platform_driver_register(&msm_smmu_driver);
	if (ret)
		pr_err("mdss_smmu_register_driver() failed!\n");

	return ret;
}

void __exit msm_smmu_driver_cleanup(void)
{
	platform_driver_unregister(&msm_smmu_driver);
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MSM SMMU driver");
