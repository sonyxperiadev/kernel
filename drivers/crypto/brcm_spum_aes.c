/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*	@file	drivers/crypto/brcm_spum_aes.c
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>
#include <asm/sizes.h>
#include <linux/broadcom/mobcom_types.h>
#include <mach/hardware.h>
#include <mach/dma.h>
#include <linux/crypto.h>
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/internal/hash.h>
#include <crypto/scatterwalk.h>
#include <mach/rdb/brcm_rdb_spum_apb.h>
#include <mach/rdb/brcm_rdb_spum_axi.h>
#include "brcm_spum.h"

/* Enable to run in PIO mode */
/* #define SPUM_PIO_MODE */

#ifdef DEBUG
static void hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
		       8, 4, buf, len, false);
}
#endif

#define FREQ_MHZ(x) ((x)*1000*1000)

#define DWORD_ALIGNED	__attribute__((aligned(sizeof(u64))))

#define AES_XTS_MIN_KEY_SIZE	32
#define AES_XTS_MAX_KEY_SIZE	64
#define AES_KEYSIZE_512		64

static LIST_HEAD(spum_drv_list);
static DEFINE_SPINLOCK(spum_drv_lock);

struct spum_aes_context {
	u32 key_enc[AES_MAX_KEYLENGTH_U32];
	u32 key_len;
	struct spum_aes_device *dd;
};

struct spum_request_context {
	u32 rx_len;
	u32 tx_len;
	u32 tx_offset;
	struct scatterlist spum_in_cmd_hdr;
	struct scatterlist spum_out_cmd_hdr;
	struct scatterlist spum_in_cmd_tweak;
	struct scatterlist spum_in_cmd_stat;
	struct scatterlist spum_out_cmd_stat;
	u8 spum_in_hdr[128] DWORD_ALIGNED;
	u8 spum_out_hdr[128] DWORD_ALIGNED;
	u8 spum_in_stat[4] DWORD_ALIGNED;
	u8 spum_out_stat[4] DWORD_ALIGNED;
	struct scatterlist *src_sg, *dst_sg;
	u32 src_change, dst_change;
	u32 src_len, dst_len;
	u32 src_dst_mod;
};

static int spum_aes_dma_init(struct spum_aes_device *dd);
/*
 * To modify the src and dst sg list in case the nbytes and the sg->length
 * differ
 */
static void modify_src_dst_list(struct scatterlist *src,
		struct scatterlist *dst, int nbytes,
		struct spum_request_context *rctx)
{
	int list_len = 0, sg_counter = 0;

	while (list_len < nbytes) {
		if ((list_len + src->length) == nbytes) {
			sg_mark_end(src);
			rctx->src_sg = sg_next(src);
			break;
		} else if ((list_len + src->length) > nbytes) {
			rctx->src_change = sg_counter;
			rctx->src_len = src->length;
			src->length = nbytes - list_len;
			rctx->src_sg = sg_next(src);
			sg_mark_end(src);
			break;
		}
		list_len += src->length;
		src = sg_next(src);
		sg_counter++;
	}

	list_len = 0;
	sg_counter = 0;

	while (list_len < nbytes) {
		if ((list_len + dst->length) == nbytes) {
			sg_mark_end(dst);
			rctx->dst_sg = sg_next(dst);
			break;
		} else if ((list_len + dst->length) > nbytes) {
			rctx->dst_change = sg_counter;
			rctx->dst_len = dst->length;
			dst->length = nbytes - list_len;
			rctx->dst_sg = sg_next(dst);
			sg_mark_end(dst);
			break;
		}
		list_len += dst->length;
		dst = sg_next(dst);
		sg_counter++;
	}

}

static void dump_sg_list(struct scatterlist *sg)
{
	struct scatterlist *sgl = sg;

	while (sgl) {
		pr_debug("%s: %d", __func__, sgl->length);
		sgl = sg_next(sgl);
	}
}

static unsigned int get_sg_list_len(struct scatterlist *sg)
{
	struct scatterlist *sgl = sg;
	unsigned int cnt = 0;

	while (sgl) {
		cnt += sgl->length;
		sgl = sg_next(sgl);
	}
	return cnt;
}

static unsigned int get_sg_list_cnt(struct scatterlist *sg)
{
	struct scatterlist *sgl = sg;
	unsigned int cnt = 0;

	while (sgl) {
		sgl = sg_next(sgl);
		cnt++;
	}
	return cnt;
}

static int spum_map_sgl(struct spum_aes_device *dd,
		 struct scatterlist *sgl, enum dma_data_direction dir)
{
	struct scatterlist *sg = sgl;
	int len = 0;

	while (sg) {
		if (!dma_map_sg(dd->dev, sg, 1, dir)) {
			pr_err("%s: dma_map_sg() error\n", __func__);
			len = -EINVAL;
			break;
		}
		sg = sg_next(sg);
		len++;
	}

	return len;
}

static void spum_unmap_sgl(struct spum_aes_device *dd,
		 struct scatterlist *sgl, enum dma_data_direction dir)
{
	struct scatterlist *sg = sgl;

	while (sg) {
		dma_unmap_sg(dd->dev, sg, 1, dir);
		sg = sg_next(sg);
	}
}

static int spum_aes_finish_req(struct spum_aes_device *dd, int err)
{
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	unsigned long flags;
	struct scatterlist *src, *dst;

	src = dd->req->src;
	dst = dd->req->dst;

	pr_debug("%s: entry tx_len %d\n", __func__, rctx->tx_len);
	/* To restore the src and dst sg's back to original
	 * state before returning */
	while (rctx->src_dst_mod == 1) {
		int src_change = rctx->src_change;
		int dst_change = rctx->dst_change;

		if (!rctx->src_change) {
			src->length = rctx->src_len;
			if (rctx->src_sg)
				sg_chain(src, rctx->src_change, rctx->src_sg);
			rctx->src_dst_mod = 0;
		} else {
			src = sg_next(src);
			src_change--;
		}

		if (!rctx->dst_change) {
			dst->length = rctx->dst_len;
			if (rctx->dst_sg)
				sg_chain(dst, rctx->dst_change, rctx->dst_sg);
			rctx->src_dst_mod = 0;
		} else {
			dst = sg_next(dst);
			dst_change--;
		}

		if (!(src_change || dst_change))
			break;
	}

	dd->req->base.complete(&dd->req->base, err);
	dd->req = NULL;
	spin_lock_irqsave(&spum_dev->lock, flags);
	clear_bit(FLAGS_BUSY, &spum_dev->flags);
	spin_unlock_irqrestore(&spum_dev->lock, flags);

	return 0;
}

#ifdef SPUM_PIO_MODE
static int spum_aes_cpu_xfer(struct spum_aes_device *dd,
		struct scatterlist *in_sg, struct scatterlist *out_sg)
{
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	struct spum_hw_context *spum_cmd =
				(struct spum_hw_context *)rctx->spum_in_hdr;
	u32 status, in_len, out_len;
	u32 *in_buff, *out_buff;
	int ret = 0, i = 0, j = 0;


	spum_dma_init(dd->io_axi_base);

	if (((spum_cmd->crypto_mode & SPUM_CMD_CMODE_MASK) ==
		SPUM_CRYPTO_MODE_XTS) && (out_sg == NULL)) {
		/* Copy the parameter i to in FIFO */

		in_buff = (u32 *)sg_virt(in_sg);
		in_len = (get_sg_list_len(in_sg) + 3) / sizeof(u32);

		while (i < in_len) {
			status = readl(dd->io_axi_base +
				SPUM_AXI_FIFO_STAT_OFFSET);
			if (status & SPUM_AXI_FIFO_STAT_IFIFO_RDY_MASK) {
				writel(*in_buff++, dd->io_axi_base +
					SPUM_AXI_FIFO_IN_OFFSET);
				i++;
			}
		}

		status = readl(dd->io_axi_base +
			SPUM_AXI_FIFO_STAT_OFFSET);

		while (status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK) {
			u32 out;
			out = readl(dd->io_axi_base +
				SPUM_AXI_FIFO_OUT_OFFSET);
			status = readl(dd->io_axi_base +
				SPUM_AXI_FIFO_STAT_OFFSET);
		}
		return ret;
	}

	in_buff = (u32 *)sg_virt(in_sg);
	out_buff = (u32 *)sg_virt(out_sg);
	in_len = (get_sg_list_len(in_sg) + 3) / sizeof(u32);
	out_len = (get_sg_list_len(out_sg) + 3) / sizeof(u32);

	while (j < out_len) {
		status = readl(dd->io_axi_base +
			SPUM_AXI_FIFO_STAT_OFFSET);
		if ((status & SPUM_AXI_FIFO_STAT_IFIFO_RDY_MASK)
			&& (i < in_len)) {
			writel(*in_buff++, dd->io_axi_base +
				SPUM_AXI_FIFO_IN_OFFSET);
			i++;
		}
		if (status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK) {
			*out_buff++ = readl(dd->io_axi_base +
				SPUM_AXI_FIFO_OUT_OFFSET);
			j++;
		}
	}

	return ret;
}

static int spum_aes_pio_xfer(struct spum_aes_device *dd)
{
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	struct spum_hw_context *spum_cmd =
				(struct spum_hw_context *)rctx->spum_in_hdr;
	struct scatterlist *in_sg;
	struct scatterlist *out_sg;
	u32 *out_stat_buff;
	int err = 0;

	if (!test_bit(FLAGS_BUSY, &spum_dev->flags)) {
		pr_err("%s: Device is busy!!!", __func__);
		BUG();
	}

	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	spum_set_pkt_length(dd->io_axi_base, rctx->rx_len, rctx->tx_len);

	/* Process header */
	spum_aes_cpu_xfer(dd, &rctx->spum_in_cmd_hdr,
			&rctx->spum_out_cmd_hdr);

	pr_debug("%s: dumping IN HDR sg: \n", __func__);
	dump_sg_list(&rctx->spum_in_cmd_hdr);

	/* If xts mode then pass tweak value, Parameter i */
	if ((spum_cmd->crypto_mode & SPUM_CMD_CMODE_MASK) ==
		SPUM_CRYPTO_MODE_XTS) {
		spum_aes_cpu_xfer(dd, &rctx->spum_in_cmd_tweak,
				NULL);
	}

	/* Process payload */
	in_sg = dd->req->src;
	out_sg = dd->req->dst;

	pr_debug("%s: dumping IN payload sg: \n", __func__);
	dump_sg_list(in_sg);

	while (in_sg && out_sg) {
		spum_aes_cpu_xfer(dd, in_sg, out_sg);
		in_sg = sg_next(in_sg);
		out_sg = sg_next(out_sg);
	}

#if 0 /* Status word read gets stuck for some reason. Debug later */
	/* Process status word */
	pr_debug("%s: dumping IN STATUS sg: \n", __func__);
	dump_sg_list(&rctx->spum_in_cmd_stat);

	spum_aes_cpu_xfer(dd, &rctx->spum_in_cmd_stat,
			&rctx->spum_out_cmd_stat);

	/* Check status for success */
	out_stat_buff = (u32 *)sg_virt(&rctx->spum_out_cmd_stat);
	hexdump((unsigned char *)out_stat_buff, 4);
	if (*out_stat_buff & SPUM_CMD_CRYPTO_STATUS_ERROR) {
		pr_err("%s: SPU-M h/w returned ERROR in status field\n",
			__func__);
		err = -1;
	}
#endif

	spum_aes_finish_req(dd, err);

	if (spum_dev->spum_queue.qlen)
		spum_queue_task((unsigned long)&spum_dev);

	return 0;
}

#endif

static int spum_aes_dma_xfer(struct spum_aes_device *dd)
{
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	struct ablkcipher_request *req = dd->req;
	u32 cfg_rx, cfg_tx, rx_fifo, tx_fifo;
	struct list_head head_in, head_out;
	int err = -EINPROGRESS;
	struct dma_transfer_list_sg lli_in_hdr, lli_in_tweak;
	struct dma_transfer_list_sg lli_in_data, lli_in_stat;
	struct dma_transfer_list_sg lli_out_hdr, lli_out_data, lli_out_stat;
	struct spum_hw_context *spum_cmd =
				(struct spum_hw_context *)rctx->spum_in_hdr;


	pr_debug("%s: entry\n", __func__);

	if (!test_bit(FLAGS_BUSY, &spum_dev->flags)) {
		pr_err("%s: Device is busy!!!", __func__);
		BUG();
	}

	cfg_rx = DMA_CFG_SRC_ADDR_INCREMENT | DMA_CFG_DST_ADDR_FIXED |
	    DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_16 |
	    PERIPHERAL_FLUSHP_END | DMA_PERI_REQ_ALWAYS_BURST;
	cfg_tx = DMA_CFG_SRC_ADDR_FIXED | DMA_CFG_DST_ADDR_INCREMENT |
	    DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_16 |
	    PERIPHERAL_FLUSHP_END | DMA_PERI_REQ_ALWAYS_BURST;

	rx_fifo =
		HW_IO_VIRT_TO_PHYS(dd->io_axi_base) +
		SPUM_AXI_FIFO_IN_OFFSET;
	tx_fifo =
		HW_IO_VIRT_TO_PHYS(dd->io_axi_base) +
		SPUM_AXI_FIFO_OUT_OFFSET;

	INIT_LIST_HEAD(&head_in);
	INIT_LIST_HEAD(&head_out);

	pr_debug("%s: dumping IN HDR sg: \n", __func__);
	dump_sg_list(&rctx->spum_in_cmd_hdr);

	pr_debug("%s: dumping IN payload sg: \n", __func__);
	dump_sg_list(dd->req->src);

	pr_debug("%s: dumping IN STATUS sg: \n", __func__);
	dump_sg_list(&rctx->spum_in_cmd_stat);

	/* DMA mapping of input/output sg list. */ /* TODO error condition. */
	err = dma_map_sg(dd->dev, &rctx->spum_in_cmd_hdr, 1, DMA_TO_DEVICE);
	if (err) {
		pr_err("%s failed to map in_cmd_hdr sgl - %d\n", __func__, err);
		goto err_xfer;
	}
	err = dma_map_sg(dd->dev, &rctx->spum_out_cmd_hdr, 1, DMA_FROM_DEVICE);
	if (err) {
		pr_err("%s failed to map in_cmd_hdr sgl - %d\n", __func__, err);
		goto err_xfer;
	}
	err = dma_map_sg(dd->dev, &rctx->spum_in_cmd_stat, 1, DMA_TO_DEVICE);
	if (err) {
		pr_err("%s failed to map in_cmd_hdr sgl - %d\n", __func__, err);
		goto err_xfer;
	}
	err = dma_map_sg(dd->dev, &rctx->spum_out_cmd_stat, 1, DMA_FROM_DEVICE);
	if (err) {
		pr_err("%s failed to map in_cmd_hdr sgl - %d\n", __func__, err);
		goto err_xfer;
	}
	spum_map_sgl(dd, dd->req->src, DMA_TO_DEVICE);
	spum_map_sgl(dd, dd->req->dst, DMA_FROM_DEVICE);

	/* Create IN lli */
	/* Header */
	lli_in_hdr.sgl = &rctx->spum_in_cmd_hdr;
	lli_in_hdr.sg_len = 1;
	/* If XTS mode. Set tweak value. */
	if ((spum_cmd->crypto_mode&SPUM_CMD_CMODE_MASK) ==
					 SPUM_CRYPTO_MODE_XTS) {
		lli_in_tweak.sgl = &rctx->spum_in_cmd_tweak;
		lli_in_tweak.sg_len = 1;
	}
	/* Payload */
	lli_in_data.sgl = dd->req->src;
	lli_in_data.sg_len = get_sg_list_cnt(dd->req->src);
	/* status */
	lli_in_stat.sgl = &rctx->spum_in_cmd_stat;
	lli_in_stat.sg_len = 1;

	/* Create OUT lli */
	/* Header */
	lli_out_hdr.sgl = &rctx->spum_out_cmd_hdr;
	lli_out_hdr.sg_len = 1;
	/* Payload */
	lli_out_data.sgl = dd->req->dst;
	lli_out_data.sg_len = get_sg_list_cnt(dd->req->dst);
	/* status */
	lli_out_stat.sgl = &rctx->spum_out_cmd_stat;
	lli_out_stat.sg_len = 1;

	/*  IN Linked list */
	list_add_tail(&lli_in_hdr.next, &head_in);
	if ((spum_cmd->crypto_mode&SPUM_CMD_CMODE_MASK) ==
					 SPUM_CRYPTO_MODE_XTS) {
		list_add_tail(&lli_in_tweak.next, &head_in);
	}
	list_add_tail(&lli_in_data.next, &head_in);
	list_add_tail(&lli_in_stat.next, &head_in);

	/*  OUT Linked List */
	list_add_tail(&lli_out_hdr.next, &head_out);
	list_add_tail(&lli_out_data.next, &head_out);
	list_add_tail(&lli_out_stat.next, &head_out);

	spum_set_pkt_length(dd->io_axi_base, rctx->rx_len, rctx->tx_len);

	/* Rx setup */
	if (dma_setup_transfer_list_multi_sg
			(spum_dev->rx_dma_chan, &head_in, rx_fifo,
			DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI, cfg_rx)) {
		pr_err("Rx dma_setup_transfer failed %d\n", err);
		err = -EIO;
		goto err_xfer;
	}

	/* Tx setup */
	if (dma_setup_transfer_list_multi_sg
		(spum_dev->tx_dma_chan, &head_out, tx_fifo,
		DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI, cfg_tx)) {
		pr_err("Tx dma_setup_transfer failed %d\n", err);
		err = -EIO;
		goto err_xfer;
	}

	rctx->tx_offset += sg_dma_len(req->dst);

	/* Rx start xfer */
	if (dma_start_transfer(spum_dev->rx_dma_chan)) {
		pr_err("Rx dma transfer failed.\n");
		err = -EIO;
		goto err_xfer;
	}

	/* Tx start xfer */
	if (dma_start_transfer(spum_dev->tx_dma_chan)) {
		pr_err("Tx dma transfer failed.\n");
		dma_stop_transfer(spum_dev->rx_dma_chan);
		err = -EIO;
		goto err_xfer;
	}

	spum_dma_init(dd->io_axi_base);

	pr_debug("%s: exit %d\n", __func__, err);
	return err;

err_xfer:
	return err;
}

int spum_aes_process_request(struct spum_aes_device *dd)
{
	int ret = 0;
#ifdef SPUM_PIO_MODE
	ret = spum_aes_pio_xfer(dd);
#else
	ret = spum_aes_dma_xfer(dd);
#endif

	return ret;
}

static void spum_aes_done_task(unsigned long data)
{
	struct spum_aes_device *dd = (struct spum_aes_device *)data;
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	int err = 0;

	pr_debug("%s\n", __func__);

	err = dma_stop_transfer(spum_dev->rx_dma_chan);
	if (err)
		pr_err("%s: Rx transfer stop failed %d\n", __func__, err);

	err = dma_stop_transfer(spum_dev->tx_dma_chan);
	if (err)
		pr_err("%s: Tx transfer stop failed %d\n", __func__, err);

	dma_unmap_sg(dd->dev, &rctx->spum_in_cmd_hdr, 1, DMA_TO_DEVICE);
	dma_unmap_sg(dd->dev, &rctx->spum_out_cmd_hdr, 1, DMA_FROM_DEVICE);
	dma_unmap_sg(dd->dev, &rctx->spum_in_cmd_stat, 1, DMA_TO_DEVICE);
	dma_unmap_sg(dd->dev, &rctx->spum_out_cmd_stat, 1, DMA_FROM_DEVICE);
	spum_unmap_sgl(dd, dd->req->src, DMA_TO_DEVICE);
	spum_unmap_sgl(dd, dd->req->dst, DMA_FROM_DEVICE);

	spum_aes_finish_req(dd, err);

	if (spum_dev->spum_queue.qlen)
		spum_queue_task((unsigned long)&spum_dev);
}

static void spum_aes_dma_callback(void *data, enum pl330_xfer_status status)
{
	struct spum_aes_device *dd = NULL;
	u32 *dma_chan = (u32 *)data;

	pr_debug("%s: dma_chan: %d\n", __func__, *dma_chan);

	list_for_each_entry(dd, &spum_drv_list, list) {
		break;
	}

	if (*dma_chan == spum_dev->rx_dma_chan)
		return;

	tasklet_hi_schedule(&dd->done_task);
}

static int spum_aes_dma_init(struct spum_aes_device *dd)
{

	/* Register DMA callback */
	if (dma_register_callback(spum_dev->tx_dma_chan, spum_aes_dma_callback,
				  &spum_dev->tx_dma_chan) != 0) {
		pr_err("%s: Tx dma_register_callback failed\n", __func__);
		goto err1;
	}
	if (dma_register_callback(spum_dev->rx_dma_chan, spum_aes_dma_callback,
				  &spum_dev->rx_dma_chan) != 0) {
		pr_err("%s: Rx dma_register_callback failed\n", __func__);
		goto err1;
	}
	return 0;
err1:
	return -EIO;
}

static int spum_aes_crypt(struct ablkcipher_request *req, spum_crypto_algo algo,
			  spum_crypto_mode mode, spum_crypto_op op)
{
	struct spum_aes_context *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct spum_request_context *rctx = ablkcipher_request_ctx(req);
	struct spum_hw_context spum_hw_aes_ctx;
	u32 cmd_len_bytes;
	int ret = 0;
	char *iv = "\x00\x00\x00\x00\x00\x00\x00\x00"
	    "\x00\x00\x00\x00\x00\x00\x00\x00";

	pr_debug("%s : entry. nbytes: %d, mode: %d, op: %d\n",
		__func__, req->nbytes, mode, op);

	memset((void *)&spum_hw_aes_ctx, 0, sizeof(spum_hw_aes_ctx));
	memset((void *)&rctx->spum_in_hdr[0], 0, ARRAY_SIZE(rctx->spum_in_hdr));

	spum_hw_aes_ctx.operation = op;
	spum_hw_aes_ctx.crypto_algo = algo;
	spum_hw_aes_ctx.crypto_mode = mode;
	spum_hw_aes_ctx.auth_algo = SPUM_AUTH_ALGO_NULL;
	spum_hw_aes_ctx.key_type = SPUM_KEY_OPEN;

	spum_hw_aes_ctx.data_attribute.crypto_length = req->nbytes;
	spum_hw_aes_ctx.data_attribute.data_length = (((req->nbytes + 3)
						       / sizeof(u32)) *
						      sizeof(u32));
	spum_hw_aes_ctx.data_attribute.crypto_offset = 0;

	spum_hw_aes_ctx.crypto_key = (void *)aes_ctx->key_enc;
	spum_hw_aes_ctx.crypto_key_len = aes_ctx->key_len / sizeof(u32);

	spum_hw_aes_ctx.init_vector_len =
	    crypto_ablkcipher_ivsize(crypto_ablkcipher_reqtfm(req)) /
	    sizeof(u32);

	if (mode == SPUM_CRYPTO_MODE_XTS) {
		spum_hw_aes_ctx.init_vector = iv;
		spum_hw_aes_ctx.data_attribute.crypto_length +=
		    spum_hw_aes_ctx.init_vector_len * sizeof(u32);
		spum_hw_aes_ctx.data_attribute.data_length +=
		    spum_hw_aes_ctx.init_vector_len * sizeof(u32);
		aes_ctx->key_len /= 2;
	} else {
		spum_hw_aes_ctx.init_vector = req->info;
	}

	switch (aes_ctx->key_len) {
	case AES_KEYSIZE_128:
		spum_hw_aes_ctx.crypto_type = SPUM_CRYPTO_TYPE_AES_K128;
		break;
	case AES_KEYSIZE_192:
		spum_hw_aes_ctx.crypto_type = SPUM_CRYPTO_TYPE_AES_K192;
		break;
	case AES_KEYSIZE_256:
		spum_hw_aes_ctx.crypto_type = SPUM_CRYPTO_TYPE_AES_K256;
		break;
	case AES_KEYSIZE_512:
		spum_hw_aes_ctx.crypto_type = SPUM_CRYPTO_TYPE_AES_K256;
		break;
	default:
		return -EPERM;
	}

	cmd_len_bytes = spum_format_command(&spum_hw_aes_ctx,
						rctx->spum_in_hdr);
	rctx->src_dst_mod = 0;
	rctx->rx_len = cmd_len_bytes +
			    spum_hw_aes_ctx.data_attribute.data_length +
			    SPUM_INPUT_STATUS_LEN;
	rctx->tx_len = SPUM_OUTPUT_HEADER_LEN +
			    spum_hw_aes_ctx.data_attribute.data_length +
			    SPUM_OUTPUT_STATUS_LEN;

	pr_debug("%s: cmd_len_bytes: %d, total rx_len: %d, total tx_len: %d\n",
		__func__, cmd_len_bytes, rctx->rx_len, rctx->tx_len);

	/* Preparing sg node for SPUM input/output message commands. */
	sg_init_one(&rctx->spum_in_cmd_hdr, rctx->spum_in_hdr, cmd_len_bytes);
	sg_init_one(&rctx->spum_out_cmd_hdr, rctx->spum_out_hdr,
			 SPUM_OUTPUT_HEADER_LEN);
	sg_init_one(&rctx->spum_in_cmd_stat, rctx->spum_in_stat,
			SPUM_INPUT_STATUS_LEN);
	sg_init_one(&rctx->spum_out_cmd_stat, rctx->spum_out_stat,
			SPUM_OUTPUT_STATUS_LEN);

	if (mode == SPUM_CRYPTO_MODE_XTS) {
		sg_init_one(&rctx->spum_in_cmd_tweak, req->info,
		crypto_ablkcipher_ivsize(crypto_ablkcipher_reqtfm(req)));
	}
	/* Check if sg->length is greater than nbytes */
	if ((req->nbytes < get_sg_list_len(req->src)) ||
		(req->nbytes < get_sg_list_len(req->dst))) {
		rctx->src_dst_mod = 1;
		modify_src_dst_list(req->src, req->dst, req->nbytes, rctx);
	}
	ret = spum_enqueue_request(&req->base);

	if (ret == -EINPROGRESS)
		spum_queue_task((unsigned long)&spum_dev);

	pr_debug("%s : exit\n", __func__);

	return ret;
}

static int spum_aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
			   u32 key_len)
{
	struct spum_aes_context *aes_ctx = crypto_ablkcipher_ctx(tfm);
	struct ablkcipher_alg *cipher = crypto_ablkcipher_alg(tfm);
	const __le32 *key = (const __le32 *)in_key;
	int ret = 0;

	pr_debug("%s: entry. key_len: %d\n", __func__, key_len);

	if ((key_len < cipher->min_keysize) ||
		(key_len > cipher->max_keysize)) {
		crypto_ablkcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		ret = -EINVAL;
	} else {
		memcpy((u32 *)(aes_ctx->key_enc), key, key_len);
		aes_ctx->key_len = key_len;
	}

	pr_debug("%s: exit\n", __func__);
	return ret;
}

static int spum_aes_xts_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
			       u32 key_len)
{
	struct spum_aes_context *aes_ctx = crypto_ablkcipher_ctx(tfm);
	struct ablkcipher_alg *cipher = crypto_ablkcipher_alg(tfm);
	const __le32 *key = (const __le32 *)in_key;
	int ret = 0;

	pr_debug("%s: entry. key_len: %d\n", __func__, key_len);

	if ((key_len < cipher->min_keysize) ||
		(key_len > cipher->max_keysize)) {
		crypto_ablkcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		ret = -EINVAL;
	} else {
		memcpy((u32 *)(aes_ctx->key_enc), ((u8 *)key + key_len / 2),
		       key_len / 2);
		memcpy((u32 *)((u8 *)aes_ctx->key_enc + key_len / 2), key,
		       key_len / 2);
		aes_ctx->key_len = key_len;
	}

	pr_debug("%s: exit\n", __func__);
	return ret;
}

static int spum_aes_ecb_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_ECB, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_ecb_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_ECB, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_cbc_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_CBC, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_cbc_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_CBC, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_ctr_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_CTR, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_ctr_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_CTR, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_xts_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_XTS, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_xts_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry\n", __func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			      SPUM_CRYPTO_MODE_XTS, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_cra_init(struct crypto_tfm *tfm)
{
	struct spum_aes_device *dd = NULL;
	struct spum_aes_context *ctx = crypto_tfm_ctx(tfm);

	pr_debug("%s: enter\n", __func__);

	list_for_each_entry(dd, &spum_drv_list, list) {
		break;
	}
	ctx->dd = dd;

	tfm->crt_ablkcipher.reqsize = sizeof(struct spum_request_context);

	return clk_enable(dd->spum_open_clk);
}

static void spum_aes_cra_exit(struct crypto_tfm *tfm)
{
	struct spum_aes_context *ctx = crypto_tfm_ctx(tfm);
	struct spum_aes_device *dd = ctx->dd;

	pr_debug("%s: exit\n", __func__);

	clk_disable(dd->spum_open_clk);

	ctx->dd = NULL;
}

static struct crypto_alg spum_algos[] = {
	{
	 .cra_name = "ecb(aes)",
	 .cra_driver_name = "spum-ecb-aes",
	 .cra_priority = 300,
	 .cra_flags = (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC |
			CRYPTO_TFM_REQ_MAY_BACKLOG),
	 .cra_blocksize = AES_BLOCK_SIZE,
	 .cra_ctxsize = sizeof(struct spum_aes_context),
	 .cra_alignmask = 0,
	 .cra_type = &crypto_ablkcipher_type,
	 .cra_module = THIS_MODULE,
	 .cra_init = spum_aes_cra_init,
	 .cra_exit = spum_aes_cra_exit,
	 .cra_u.ablkcipher = {
			      .min_keysize = AES_MIN_KEY_SIZE,
			      .max_keysize = AES_MAX_KEY_SIZE,
			      .setkey = spum_aes_setkey,
			      .encrypt = spum_aes_ecb_encrypt,
			      .decrypt = spum_aes_ecb_decrypt,
			      }
	 },
	{
	 .cra_name = "cbc(aes)",
	 .cra_driver_name = "spum-cbc-aes",
	 .cra_priority = 300,
	 .cra_flags = (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC |
			CRYPTO_TFM_REQ_MAY_BACKLOG),
	 .cra_blocksize = AES_BLOCK_SIZE,
	 .cra_ctxsize = sizeof(struct spum_aes_context),
	 .cra_alignmask = 0,
	 .cra_type = &crypto_ablkcipher_type,
	 .cra_module = THIS_MODULE,
	 .cra_init = spum_aes_cra_init,
	 .cra_exit = spum_aes_cra_exit,
	 .cra_u.ablkcipher = {
			      .min_keysize = AES_MIN_KEY_SIZE,
			      .max_keysize = AES_MAX_KEY_SIZE,
			      .ivsize = AES_BLOCK_SIZE,
			      .setkey = spum_aes_setkey,
			      .encrypt = spum_aes_cbc_encrypt,
			      .decrypt = spum_aes_cbc_decrypt,
			      }
	 },
	{
	 .cra_name = "ctr(aes)",
	 .cra_driver_name = "spum-ctr-aes",
	 .cra_priority = 300,
	 .cra_flags = (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC |
			CRYPTO_TFM_REQ_MAY_BACKLOG),
	 .cra_blocksize = AES_BLOCK_SIZE,
	 .cra_ctxsize = sizeof(struct spum_aes_context),
	 .cra_alignmask = 0,
	 .cra_type = &crypto_ablkcipher_type,
	 .cra_module = THIS_MODULE,
	 .cra_init = spum_aes_cra_init,
	 .cra_exit = spum_aes_cra_exit,
	 .cra_u.ablkcipher = {
			      .min_keysize = AES_MIN_KEY_SIZE,
			      .max_keysize = AES_MAX_KEY_SIZE,
			      .ivsize = AES_BLOCK_SIZE,
			      .setkey = spum_aes_setkey,
			      .encrypt = spum_aes_ctr_encrypt,
			      .decrypt = spum_aes_ctr_decrypt,
			      }
	 },
	{
	 .cra_name = "xts(aes)",
	 .cra_driver_name = "spum-xts-aes",
	 .cra_priority = 300,
	 .cra_flags = (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC |
			CRYPTO_TFM_REQ_MAY_BACKLOG),
	 .cra_blocksize = AES_BLOCK_SIZE,
	 .cra_ctxsize = sizeof(struct spum_aes_context),
	 .cra_alignmask = 0,
	 .cra_type = &crypto_ablkcipher_type,
	 .cra_module = THIS_MODULE,
	 .cra_init = spum_aes_cra_init,
	 .cra_exit = spum_aes_cra_exit,
	 .cra_u.ablkcipher = {
			      .min_keysize = AES_XTS_MIN_KEY_SIZE,
			      .max_keysize = AES_XTS_MAX_KEY_SIZE,
			      .ivsize = AES_BLOCK_SIZE,
			      .setkey = spum_aes_xts_setkey,
			      .encrypt = spum_aes_xts_encrypt,
			      .decrypt = spum_aes_xts_decrypt,
			      }
	 },
};

static int spum_aes_probe(struct platform_device *pdev)
{
	struct spum_aes_device *dd;
	struct resource *res;
	int ret = 0, i, j;

	dd = (struct spum_aes_device *)
	    kzalloc(sizeof(struct spum_aes_device), GFP_KERNEL);
	if (dd == NULL) {
		pr_err("%s:Failed to allocate spum structure.\n", __func__);
		return -ENOMEM;
	}

	spum_dev->aes_dev = dd;
	dd->dev = &pdev->dev;
	platform_set_drvdata(pdev, dd);

	/* Initializing the clock. */
	dd->spum_open_clk = clk_get(NULL, "spum_open");
	if (IS_ERR_OR_NULL(dd->spum_open_clk)) {
		pr_err("%s: Clock intialization failed.\n", __func__);
		kfree(dd);
		return -ENOMEM;
	}

	/* Get the base address APB space. */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("%s: Invalid resource type.\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	dd->io_apb_base = HW_IO_PHYS_TO_VIRT(res->start);
	if (!dd->io_apb_base) {
		pr_err("%s: Ioremap failed.\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	/* Get the base address AXI space. */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res) {
		pr_err("%s: Invalid resource type.\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	dd->io_axi_base = HW_IO_PHYS_TO_VIRT(res->start);
	if (!dd->io_axi_base) {
		pr_err("%s: Ioremap failed.\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	/* Initialize SPU-M block */
	if (clk_set_rate(dd->spum_open_clk, FREQ_MHZ(156)))
		pr_debug("%s: Clock set failed!!!\n", __func__);
	ret = clk_enable(dd->spum_open_clk);
	if (ret) {
		pr_err("%s: Failed to enable clock - %d\n", __func__, ret);
		goto exit;
	}
	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	if (spum_aes_dma_init(dd)) {
		pr_err("%s: DMA callback register failed\n", __func__);
		ret = -EIO;
		goto exit;
	}
	clk_disable(dd->spum_open_clk);

	INIT_LIST_HEAD(&dd->list);
	spin_lock(&spum_drv_lock);
	list_add_tail(&dd->list, &spum_drv_list);
	spin_unlock(&spum_drv_lock);

	tasklet_init(&dd->done_task, spum_aes_done_task, (unsigned long)dd);

	for (i = 0; i < ARRAY_SIZE(spum_algos); i++) {
		ret = crypto_register_alg(&spum_algos[i]);
		if (ret) {
			pr_err("%s: Crypto algorithm registration failed for %s\n",
			     __func__, spum_algos[i].cra_name);
			goto exit_algos;
		} else
			pr_info("%s: SPUM %s algorithm registered.\n",
				__func__, spum_algos[i].cra_name);
	}

	pr_info("%s: SPUM AES driver registered.\n", __func__);

	return ret;

exit_algos:
	for (j = 0; j < i; j++)
		crypto_unregister_alg(&spum_algos[i]);
exit:
	kfree(dd);
	return ret;
}

static int spum_aes_remove(struct platform_device *pdev)
{
	struct spum_aes_device *dd = platform_get_drvdata(pdev);
	int i;

	if (!dd)
		return -ENODEV;

	/* Free the DMA callback. */
	dma_free_callback(spum_dev->tx_dma_chan);
	dma_free_callback(spum_dev->rx_dma_chan);

	spin_lock(&spum_drv_lock);
	list_del(&dd->list);
	spin_unlock(&spum_drv_lock);

	for (i = 0; i < ARRAY_SIZE(spum_algos); i++)
		crypto_unregister_alg(&spum_algos[i]);

	tasklet_kill(&dd->done_task);
	clk_put(dd->spum_open_clk);

	kfree(dd);

	return 0;
}

static struct platform_driver spum_aes_driver = {
	.probe = spum_aes_probe,
	.remove = spum_aes_remove,
	.driver = {
		   .name = "brcm-spum-aes",
		   .owner = THIS_MODULE,
		   },
};

static int __init spum_aes_init(void)
{
	pr_info("SPUM AES driver init.\n");

	return platform_driver_register(&spum_aes_driver);
}

static void __exit spum_aes_exit(void)
{
	platform_driver_unregister(&spum_aes_driver);
}

module_init(spum_aes_init);
module_exit(spum_aes_exit);

MODULE_LICENSE("GPL v2");
