#ifndef ASM_DMA_CONTIGUOUS_H
#define ASM_DMA_CONTIGUOUS_H

#ifdef __KERNEL__

#include <linux/device.h>
#include <linux/dma-contiguous.h>

#ifdef CONFIG_CMA

static inline struct cma *get_dev_cma_area(struct device *dev)
{
	if (dev && dev->cma_area)
		return dev->cma_area;
	return dma_contiguous_default_area;
}

static inline void set_dev_cma_area(struct device *dev, struct cma *cma)
{
	if (dev)
		dev->cma_area = cma;
	dma_contiguous_default_area = cma;
}

#endif
#endif
#endif
