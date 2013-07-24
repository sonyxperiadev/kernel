/*
 * arch/arm/plat-<soc>/include/plat/bcm_iommu.h
 *
 * Copyright (C) 2012 Broadcom, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _BCM_IOMMU_H_
#define _BCM_IOMMU_H_

#include <linux/iommu.h>
#include <linux/scatterlist.h>

/* These APIs are taken directly from 3.6 dma mapping code so that
 * hooking to dma-mapping APIs becomes simpler if at all needed
 **/
#define DMA_ERROR_CODE (~0)

struct dma_iommu_mapping;

/* Platform data of iommu - if device-tree not used */
struct bcm_iommu_pdata {
	char *name;
	u32 iova_begin;
	u32 iova_size;
	u32 errbuf_size;
	u32 skip_enable;	/* Skip enabling of iommu during probe */
};

/* Platform data of iovmm - if device-tree not used */
struct bcm_iovmm_pdata {
	char *name;
	u32 base;
	u32 size;
	u32 order;
};

struct dma_iommu_mapping *arm_iommu_create_mapping(struct bus_type *bus,
		dma_addr_t base, size_t size, int order);

void arm_iommu_release_mapping(struct dma_iommu_mapping *mapping);

int arm_iommu_attach_device(struct device *dev,
		struct dma_iommu_mapping *mapping);

int arm_iommu_map_sgt(struct device *dev, struct sg_table *sgt, u32 align);

int arm_iommu_unmap(struct device *dev, dma_addr_t iova, size_t size);

/* API to enable M4U if not already enabled during probe.
 * This could be needed to enable display subsystem iommu if u-boot shows
 *  splash screen in video mode and wants to continue without interruption
 *  during kernel boot
 **/
int bcm_iommu_enable(struct device *dev);

#endif /* _BCM_IOMMU_H_ */

