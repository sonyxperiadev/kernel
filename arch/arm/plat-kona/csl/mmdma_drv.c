/*
 * arch/arm/plat-kona/csl/mmdma_drv.c
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

#define pr_fmt(fmt) "mmdma csl: " fmt

#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/broadcom/mobcom_types.h>
#include <linux/broadcom/msconsts.h>
#include <linux/module.h>
#include <plat/osabstract/ostypes.h>
#include <plat/osabstract/ostask.h>
#include <plat/osabstract/ossemaphore.h>
#include <plat/osabstract/osqueue.h>
#include <plat/osabstract/osinterrupt.h>
#include <mach/irqs.h>
#include <mach/memory.h>
#include <linux/kernel.h>
#include <mach/io_map.h>
#include <plat/dma_drv.h>
#include <linux/platform_device.h>
#include <plat/csl/csl_dma_vc4lite.h>

static struct mutex lock;

void mmdma_callback (DMA_VC4LITE_CALLBACK_STATUS status)
{
	pr_debug("MMDma Completed[%d] \n", status);
	mutex_unlock(&lock);
}

int mmdma_execute(unsigned int srcAddr, unsigned int dstAddr, unsigned int size)
{

	DMA_VC4LITE_CHANNEL_INFO_t dmaChInfo;
	DMA_VC4LITE_XFER_DATA_t dmaData;
	Int32 dmaCh;
	int ret = 0;

	/* Reserve Channel */
	dmaCh = csl_dma_vc4lite_obtain_channel(DMA_VC4LITE_CLIENT_MEMORY, DMA_VC4LITE_CLIENT_MEMORY);
	if (dmaCh == -1) {
		pr_err("%s: ERR Reserving DMA Ch \n", __func__);
		return -ENODEV;
	}
	pr_debug("%s: Got DmaCh[%ld] \n",__func__, dmaCh);

	/* Configure Channel */
	dmaChInfo.autoFreeChan = 1;
	dmaChInfo.srcID = DMA_VC4LITE_CLIENT_MEMORY;
	dmaChInfo.dstID = DMA_VC4LITE_CLIENT_MEMORY;

	/*dmaChInfo.burstLen     = DMA_VC4LITE_BURST_LENGTH_4; */
	dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_8;
	dmaChInfo.xferMode = DMA_VC4LITE_XFER_MODE_LINERA;
	dmaChInfo.dstStride = 0;
	dmaChInfo.srcStride = 0;
	dmaChInfo.waitResponse = 0;
	dmaChInfo.callback = mmdma_callback;

	if (csl_dma_vc4lite_config_channel(dmaCh, &dmaChInfo)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		pr_err("%s: ERR Configure DMA Ch \n", __func__);
		ret = -EINVAL;
		goto release_channel;
	}

	/* Add the DMA transfer data */
#ifdef CONFIG_ARCH_HAWAII
	dmaData.burstWriteEnable32 = 1;
#endif
	dmaData.srcAddr = srcAddr;
	dmaData.dstAddr = dstAddr;
	dmaData.xferLength = size;

	if (csl_dma_vc4lite_add_data(dmaCh, &dmaData)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		pr_err("[CSL DSI] %s: ERR add DMA transfer data \n", __func__);
		ret = -EINVAL;
		goto release_channel;
	}

	/* start DMA transfer */
	if (csl_dma_vc4lite_start_transfer(dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		pr_err("[CSL DSI] %s: ERR start DMA data transfer \n", __func__);
		ret = -EINVAL;
		goto release_channel;
	}
	mb();

	mutex_lock(&lock);
	return ret;

release_channel:
	csl_dma_vc4lite_release_channel(dmaCh);
	return ret;
}
EXPORT_SYMBOL(mmdma_execute);

static __init int init_mmdma(void)
{
	pr_debug("init \n");
	if (csl_dma_vc4lite_init() != DMA_VC4LITE_STATUS_SUCCESS)
	{
		pr_err("csl_dma_vc4lite_init Failed \n");
		return (-1);
	}
	mutex_init(&lock);
	mutex_lock(&lock);
	return 0;
}

static __exit void exit_mmdma(void)
{
	pr_info("exit \n");
}

module_init(init_mmdma);
module_exit(exit_mmdma);

