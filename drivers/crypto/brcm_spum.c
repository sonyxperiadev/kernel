/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/crypto/brcm_spum.c
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/scatterlist.h>
#include <mach/dma.h>
#include <mach/rdb/brcm_rdb_spum_apb.h>
#include <mach/rdb/brcm_rdb_spum_axi.h>
#include <crypto/hash.h>
#include <crypto/internal/hash.h>
#include "brcm_spum.h"

struct brcm_spum_device *spum_dev;

void spum_init_device(void __iomem *io_apb_base, void __iomem *io_axi_base)
{
        writel(SPUM_APB_CTRL_SOFT_RST_MASK,
                        io_apb_base + SPUM_APB_CTRL_OFFSET);

	/* Wait until soft reset completes */
	while(readl(io_apb_base + SPUM_APB_CTRL_OFFSET)&
		SPUM_APB_CTRL_SOFT_RST_MASK);

        writel((SPUM_APB_CTRL_PEN_MODE_MASK|SPUM_APB_CTRL_IN_ENDIAN_MASK|
                SPUM_APB_CTRL_OUT_ENDIAN_MASK), io_apb_base + SPUM_APB_CTRL_OFFSET);

	writel(SPUM_AXI_DMA_STAT_PKT_CLEAR_MASK,
		io_axi_base + SPUM_AXI_DMA_STAT_OFFSET);

	writel(0, io_axi_base + SPUM_AXI_DMA_STAT_OFFSET);

        writel(SPUM_AXI_AXI_ACCESS_ACCESS_MASK,
                io_axi_base + SPUM_AXI_AXI_ACCESS_OFFSET);

        writel((SPUM_AXI_DMA_CTL_IN_EXTFLOW_MASK|SPUM_AXI_DMA_CTL_OUT_EXTFLOW_MASK),
                io_axi_base + SPUM_AXI_DMA_CTL_OFFSET);
}

void spum_dma_init(void __iomem *io_axi_base)
{
	writel(SPUM_AXI_DMA_STAT_PKT_START_MASK,
		io_axi_base + SPUM_AXI_DMA_STAT_OFFSET);
}

void spum_set_pkt_length(void __iomem *io_axi_base,
				 u32 rx_len, u32 tx_len)
{
	writel((rx_len+3)/4, io_axi_base + SPUM_AXI_IN_DMA_SIZE_OFFSET);
        writel((tx_len+3)/4, io_axi_base + SPUM_AXI_OUT_DMA_SIZE_OFFSET);
}

int spum_processing_done(void __iomem *io_axi_base)
{
	return (readl(io_axi_base + SPUM_AXI_DMA_STAT_OFFSET) 
			& SPUM_AXI_DMA_STAT_PKT_DONE_MASK);
}

int spum_format_command(struct spum_hw_context *spum_ctx,
			void *spum_cmd)
{
	u32	*cmd = (u32 *)spum_cmd;
	spum_sctx_hdr	*sctx_hdr;
	u32	*data_attr;
	u32	idx = 0;
	u32	len = 0;
	u32	i = 0;

	pr_debug("%s: entry",__func__);

	/* Set Message Header */
	cmd[0] = (SCTX_PRESENT | BDESC_PRESENT | BD_PRESENT);
	/* Set Extended Message Header */
	cmd[1] = spum_ctx->emh;

	/* Set SCTX Header */
	sctx_hdr = (spum_sctx_hdr *)&cmd[2];

	sctx_hdr->secondWord = (spum_ctx->auth_order |
                           spum_ctx->operation  |
                           spum_ctx->crypto_algo |
                           spum_ctx->crypto_mode |
                           spum_ctx->crypto_type |
                           spum_ctx->auth_algo   |
                           spum_ctx->auth_mode   |
                           spum_ctx->auth_type   |
                           spum_ctx->key_update);

	if(spum_ctx->icv_len) {
		sctx_hdr->thirdWord = 
			(spum_ctx->icv_len << SPUM_CMD_SCTX_ICV_SIZE_SHIFT) &
			SPUM_CMD_SCTX_ICV_SIZE_MASK;
	}
	else {
		sctx_hdr->thirdWord = 0;
	}

	if(spum_ctx->key_type == SPUM_KEY_PROTECTED) {
		spum_ctx->auth_key_len = 0;
		spum_ctx->crypto_key_len = 0;
		sctx_hdr->thirdWord |= SPUM_CMD_SCTX_KEY_PROTECT |
				(((( spum_ctx->key_handle + 1)*4) <<
				SPUM_CMD_SCTX_KEY_HANDLE_SHIFT) & 
				SPUM_CMD_SCTX_KEY_HANDLE_MASK);
	}
	else {
		/* Set Authentication key */
		len = spum_ctx->auth_key_len;
		for(idx = 0; idx < len; idx++) {
			((u32*)&sctx_hdr->extended)[idx] = ((u32*)spum_ctx->auth_key)[idx];
		}

		/* Set crypto key */
		len = spum_ctx->auth_key_len + spum_ctx->crypto_key_len;
		for(i = 0; idx<len; idx++, i++) {
			((u32*)&sctx_hdr->extended)[idx] = ((u32*)spum_ctx->crypto_key)[i];
		}
	}

	if(spum_ctx->init_vector_len) {
		sctx_hdr->thirdWord |= SPUM_CMD_SCTX_IV_CONTEXT;
		/* Set initialize vector */
		len = spum_ctx->auth_key_len + spum_ctx->crypto_key_len + spum_ctx->init_vector_len;
		for(i = 0 ; idx < len; idx++, i++) {
			((u32*)&sctx_hdr->extended)[idx] = ((u32*)spum_ctx->init_vector)[i];
		}
	}

	sctx_hdr->firstWord = SCTX_TYPE_GENERIC | (SCTX_SIZE_MASK & (SCTX_SIZE_MIN +
				 spum_ctx->auth_key_len + spum_ctx->crypto_key_len +
				 spum_ctx->init_vector_len));

	/* Include data info */
	data_attr = (u32*)((void*)sctx_hdr + (SCTX_SIZE_MIN + spum_ctx->auth_key_len +
			spum_ctx->crypto_key_len + spum_ctx->init_vector_len) * sizeof(u32));

	/* Updating Buffer Descriptor header */
	data_attr[0] = (spum_ctx->data_attribute.mac_offset << 16) | 
				spum_ctx->data_attribute.mac_length;
	data_attr[1] = (spum_ctx->data_attribute.crypto_offset << 16) |
				spum_ctx->data_attribute.crypto_length;

	if(spum_ctx->data_attribute.aes_gcm_auth_length) {
		/* Use offset IV to store aesGcmAuthLength */
		data_attr[2] = spum_ctx->data_attribute.aes_gcm_auth_length;
	}
	else {
		data_attr[2] = 0;  /* Overwriting Offset ICV and IV */
	}

	/* Updating Buffer Data header */
	data_attr[3] = (spum_ctx->data_attribute.data_length << 16) |
					spum_ctx->data_attribute.prev_length;

	return ((void*)data_attr + (sizeof(u32)*4)) - spum_cmd;
}

void spum_queue_task(unsigned long data)
{
	struct crypto_async_request *async_req = NULL, *backlog = NULL;
	unsigned long flags;

	spin_lock_irqsave(&spum_dev->lock, flags);
	if (test_bit(FLAGS_BUSY, &spum_dev->flags)) {
		spin_unlock_irqrestore(&spum_dev->lock, flags);
		return;
	}

	backlog = crypto_get_backlog(&spum_dev->spum_queue);
	async_req = crypto_dequeue_request(&spum_dev->spum_queue);
	if (async_req)
		set_bit(FLAGS_BUSY, &spum_dev->flags);
	spin_unlock_irqrestore(&spum_dev->lock, flags);

	if (!async_req)
		return;

	if (backlog)
		backlog->complete(backlog, -EINPROGRESS);

	if (async_req->tfm->__crt_alg->cra_type == &crypto_ahash_type) {
		spum_dev->hash_dev->req = ahash_request_cast(async_req);
#if defined(CONFIG_CRYPTO_DEV_BRCM_SPUM_HASH)
		spum_hash_process_request(spum_dev->hash_dev);
#endif
	} else if (async_req->tfm->__crt_alg->cra_type ==
					&crypto_ablkcipher_type) {
		spum_dev->aes_dev->req = ablkcipher_request_cast(async_req);
#if defined(CONFIG_CRYPTO_DEV_BRCM_SPUM_AES)
		spum_aes_process_request(spum_dev->aes_dev);
#endif
	} else {
		pr_err("%s: Invalid crypto request!\n", __func__);
		return;
	}

	return;
}

int spum_enqueue_request(struct crypto_async_request *req)
{
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&spum_dev->lock, flags);
	ret = crypto_enqueue_request(&spum_dev->spum_queue, req);
	spin_unlock_irqrestore(&spum_dev->lock, flags);

	return ret;
}

static int __init brcm_spum_init(void)
{
	spum_dev = (struct brcm_spum_device *)
		kzalloc(sizeof(struct brcm_spum_device), GFP_KERNEL);

	spin_lock_init(&spum_dev->lock);

	crypto_init_queue(&spum_dev->spum_queue, SPUM_QUEUE_LENGTH);

	spum_dev->flags = 0;

	/* Aquire DMA channels */
	if (dma_request_chan(&spum_dev->rx_dma_chan, "SPUM_OpenA") != 0) {
		pr_err("%s: Rx dma_request_chan failed\n", __func__);
		return -1;
	}
	if (dma_request_chan(&spum_dev->tx_dma_chan, "SPUM_OpenB") != 0) {
		pr_err("%s: Tx dma_request_chan failed\n", __func__);
		goto err;
	}

	pr_info("%s: DMA channel aquired rx %d tx %d\n", __func__,
			spum_dev->rx_dma_chan, spum_dev->tx_dma_chan);

	return 0;
err:
	dma_free_chan(spum_dev->rx_dma_chan);
	return -EIO;
}

arch_initcall(brcm_spum_init);
