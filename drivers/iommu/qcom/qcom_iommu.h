/*
 * Copyright (C) 2017-2018, AngeloGioacchino Del Regno <kholk11@gmail.com>
 *
 * May contain portions of code (c) 2010-2015, The Linux Foundation.
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

#ifndef MSM_IOMMU_H
#define MSM_IOMMU_H

#define DOMAIN_ATTR_QCOM_COHERENT_HTW_DISABLE	DOMAIN_ATTR_MAX

#ifdef CONFIG_QCOM_IOMMU_V1
void *msm_iommu_lock_initialize(void);
void msm_iommu_mutex_lock(void);
void msm_iommu_mutex_unlock(void);
void msm_set_iommu_access_ops(struct iommu_access_ops *ops);
struct iommu_access_ops *msm_get_iommu_access_ops(void);
#else
static inline void *msm_iommu_lock_initialize(void)
{
	return NULL;
}
static inline void msm_iommu_mutex_lock(void) { }
static inline void msm_iommu_mutex_unlock(void) { }
static inline void msm_set_iommu_access_ops(struct iommu_access_ops *ops)
{

}
static inline struct iommu_access_ops *msm_get_iommu_access_ops(void)
{
	return NULL;
}
#endif

#ifdef CONFIG_QCOM_IOMMU_V1
/*
 * Look up an IOMMU context device by its context name. NULL if none found.
 * Useful for testing and drivers that do not yet fully have IOMMU stuff in
 * their platform devices.
 */
struct device *msm_iommu_get_ctx(const char *ctx_name);
#else
static inline struct device *msm_iommu_get_ctx(const char *ctx_name)
{
	return NULL;
}
#endif

#ifdef CONFIG_MSM_IOMMU_V0
static inline int msm_soc_version_supports_iommu_v0(void)
{
	static int soc_supports_v0 = -1;
#ifdef CONFIG_OF
	struct device_node *node;
#endif

	if (soc_supports_v0 != -1)
		return soc_supports_v0;

#ifdef CONFIG_OF
	node = of_find_compatible_node(NULL, NULL, "qcom,msm-smmu-v0");
	if (node) {
		soc_supports_v0 = 1;
		of_node_put(node);
		return 1;
	}
#endif
	if (cpu_is_msm8960() &&
	    SOCINFO_VERSION_MAJOR(socinfo_get_version()) < 2) {
		soc_supports_v0 = 0;
		return 0;
	}

	if (cpu_is_msm8x60() &&
	    (SOCINFO_VERSION_MAJOR(socinfo_get_version()) != 2 ||
	    SOCINFO_VERSION_MINOR(socinfo_get_version()) < 1))	{
		soc_supports_v0 = 0;
		return 0;
	}

	soc_supports_v0 = 1;
	return 1;
}
#else
static inline int msm_soc_version_supports_iommu_v0(void)
{
	return 0;
}
#endif

extern struct bus_type platform_bus_type;
+static inline struct bus_type *msm_iommu_get_bus(struct device *dev)
{

	return &platform_bus_type;
}


#endif
