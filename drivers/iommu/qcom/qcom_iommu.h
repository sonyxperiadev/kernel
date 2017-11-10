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

extern struct bus_type platform_bus_type;
static inline struct bus_type *msm_iommu_get_bus(struct device *dev)
{
	return &platform_bus_type;
}

#endif
