/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/crypto/brcm_spum_hash.c
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
//#define DEBUG 1
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <asm/cacheflush.h>
#include <asm/sizes.h>
#include <plat/clock.h>
#include <linux/broadcom/mobcom_types.h>
#include <mach/hardware.h>
#include <mach/dma.h>
#include <linux/crypto.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <crypto/md5.h>
#include <crypto/algapi.h>
#include <crypto/internal/hash.h>
#include <crypto/scatterwalk.h>
#include <mach/rdb/brcm_rdb_spum_apb.h>
#include <mach/rdb/brcm_rdb_spum_axi.h>
#include "brcm_spum.h" 


#define	OP_UPDATE	1
#define	OP_FINAL	2
#define OP_INIT		3

#define FLAGS_FINUP	1

#define FLAGS_BUSY	1
#define FLAGS_RBUSY	2
#define	FLAGS_TBUSY	4

#define SPUM_HASH_BLOCK_SIZE	64
#define SPUM_HASH_QUEUE_LENGTH	10
#define SPUM_MAX_PAYLOAD_PER_CMD	(SZ_64K - SZ_64)

#define WORD_ALIGNED	__attribute__((aligned(sizeof(u32))))

#define b_host_2_be32( __buf, __size)       \
do {                                        \
	u32 *b = (u32 *)(__buf);            \
	u32 i;                              \
	for (i = 0; i < __size; i++, b++) { \
	*b = cpu_to_be32(*b);               \
  }                                         \
}while (0)

struct brcm_spum_device {
	struct list_head        list;
	struct device           *dev;
	void __iomem            *io_apb_base;
	void __iomem            *io_axi_base;
	dma_addr_t		dma_addr;
	u32 			*dma_virt;
	u32			dma_len;
	spinlock_t              lock;
	u32                     rx_dma_chan;
	u32                     tx_dma_chan;
	ulong			flags;
	struct crypto_queue     queue;
	struct ahash_request	*req;
	struct clk		*spum_open_clk;
	struct tasklet_struct	queue_task;
};

struct spum_request_context {
	u32                     op;
	struct scatterlist      *sg;
	u8			*result;
	u32                     total;
	u32                     digestsize;
	u32			flags;

	u32			bufcnt;
	u32			buflen;
	u8			buff[PAGE_SIZE] WORD_ALIGNED;

	u8			buffer[512] WORD_ALIGNED;
	struct scatterlist	spum_hdr[3];
	u32			offset;
	u32			partial;
	u32			rx_len;
	u32			tx_len;
	u32			blk_cnt;
};

struct spum_hash_context {
	u32	hash_init;
	u8	digest[SHA512_DIGEST_SIZE]; /* Max. digest size in bytes*/
	struct brcm_spum_device	*dd;
	struct crypto_shash	*fallback;
};

struct brcm_spum_driver {
	struct list_head	dev_list;
	spinlock_t		lock;
};

static struct brcm_spum_driver spum_drv = {
	.dev_list = LIST_HEAD_INIT(spum_drv.dev_list),
	.lock = __SPIN_LOCK_UNLOCKED(spum_drv.lock),
};

static int spum_hash_handle_queue(struct brcm_spum_device *dd,
                                        struct ahash_request *req);
static int spum_hash_process_request(struct brcm_spum_device *dd);
static void spum_hash_finish(struct brcm_spum_device *dd, int err);
static int spum_hash_final(struct ahash_request *req);

struct tasklet_struct	dma_tasklet;
static u32      total_payload = 0;
struct scatterlist *sg_updt;

#ifdef DEBUG
static void hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}

void show_sglist(struct scatterlist *sg)
{
	struct scatterlist *sgl = sg;

	pr_debug("%s: Show sg list\n",__func__);
	while(sgl) {
		pr_debug("length %d\n",sgl->length);
		sgl = scatterwalk_sg_next(sgl);
	}	

}
#endif

/* Mark the sg list for DMA xfer. */
static u32 get_sg_list(struct spum_request_context *rctx, 
			struct scatterlist *sgl, u32 *length)
{
	struct scatterlist *sg = sgl;
	u32 count = 0, len = 0,sglen;

	while(sg) {
		sglen = min(rctx->rx_len, sg->length);
		if(!(sglen%64) && (sglen > 64) &&
			 ((len+sglen)<=rctx->rx_len)) {
			count++;
			len += sglen;
		}
		else {
			break;
		}
		sg = scatterwalk_sg_next(sg);
	}

	*length = len;

	return count;
}

/*
 * Get the sg list for update command when total payload
 * greater than 64kb.
 */
static u32 get_sg_update(struct scatterlist *sgl)
{
        struct scatterlist *sg;
        u32 length = 0;

        while(sgl) {
                if((length+sgl->length) <= SPUM_MAX_PAYLOAD_PER_CMD) {
                        length += sgl->length;
                        sg = sgl;
                        sgl = scatterwalk_sg_next(sgl);
                }
                else {
                        sg_updt = sgl;
                        sg_mark_end(sg);
			break;
                }
        }

        total_payload -= length;
        return length;
}

static u32 spum_append_buffer(struct spum_request_context *rctx,
			const u8 *data, u32 length)
{
	u32 count = min(length, rctx->buflen - rctx->bufcnt);

	pr_debug("%s: entry\n",__func__);

	count = min(count, rctx->total);
	if (count <= 0)
		return 0;
	memcpy(rctx->buff + rctx->bufcnt, data, count);
	rctx->bufcnt += count;

	return count;
}

static void spum_append_sg(struct spum_request_context *rctx)
{
	u32 count;

	pr_debug("%s: entry\n",__func__);

	while (rctx->sg) {
		count = spum_append_buffer(rctx,
				sg_virt(rctx->sg) + rctx->offset,
				rctx->sg->length - rctx->offset);
		if (!count)
			break;
		rctx->offset += count;
		if(rctx->offset == min(rctx->sg->length, rctx->total)) {
			rctx->offset = 0;
			rctx->sg = scatterwalk_sg_next(rctx->sg);
		}
		rctx->total -= count;
	}
}

static int spum_cpu_xfer(struct brcm_spum_device *dd, u8 *in_buff, u32 length)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	u32 i=0,status,tx_len, out_fifo[32];
	static int k = 0;
	u32 *buff = (u32 *)in_buff;

	pr_debug("%s: entry length %d\n",__func__,length);

	spum_dma_init(dd->io_axi_base);

	length = ((length+3)/sizeof(u32));

	tx_len = rctx->tx_len/4;
	i=0;k=0;
	while(i<length) {
		status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
		if(status & SPUM_AXI_FIFO_STAT_IFIFO_RDY_MASK) {
			writel(*buff++, dd->io_axi_base + SPUM_AXI_FIFO_IN_OFFSET );
            		i++;
		}
		if((status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK) && tx_len) {
			out_fifo[k%32] = readl(dd->io_axi_base + SPUM_AXI_FIFO_OUT_OFFSET);
			k++; tx_len--;
		}
	}
	status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
	while((status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK) && tx_len) {
		out_fifo[k%32] = readl(dd->io_axi_base + SPUM_AXI_FIFO_OUT_OFFSET);
		k++; tx_len--;
		status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
	}

	length *= sizeof(u32);
        if(rctx->rx_len >= length) {
                rctx->rx_len -= length;
        }
        else {
                length -= rctx->rx_len;

                if(rctx->partial > length)
                        rctx->partial -= length;
                else
                        rctx->partial = 0;

                rctx->rx_len = 0;
        }

	rctx->tx_len -= k*4;
	
	if((rctx->sg->length - rctx->offset) == length) { //rctx->offset) {
		rctx->sg = scatterwalk_sg_next(rctx->sg);
		rctx->offset = 0;
	}
	pr_debug("%s: exit\n",__func__);

	return 0;
}

static int spum_dma_xfer(struct brcm_spum_device *dd, u32 dma_len, u32 length)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	u32 cfg_rx, cfg_tx, rx_fifo, tx_fifo;
	int err = -EINPROGRESS;

	pr_debug("%s: entry\n",__func__);

	/* Only this configuration works. */
	cfg_rx = DMA_CFG_SRC_ADDR_INCREMENT | DMA_CFG_DST_ADDR_FIXED |
            DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_16;
	cfg_tx = DMA_CFG_SRC_ADDR_FIXED | DMA_CFG_DST_ADDR_INCREMENT |
	    DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_16;

	rx_fifo = (u32)HW_IO_VIRT_TO_PHYS(dd->io_axi_base) + SPUM_AXI_FIFO_IN_OFFSET;
	tx_fifo = (u32)HW_IO_VIRT_TO_PHYS(dd->io_axi_base) + SPUM_AXI_FIFO_OUT_OFFSET;

	if(dma_len > 1) {
		/* Rx setup */
		if(dma_setup_transfer_sg(dd->rx_dma_chan, rctx->sg, dma_len, rx_fifo,
					DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI, cfg_rx)) {
			pr_err("%s: Rx dma_setup_transfer failed 0x%x\n",__func__,(u32)dd->flags);
			err = -EIO;
			goto err;
		}
		/* Tx setup */
		if(dma_setup_transfer_sg(dd->tx_dma_chan, rctx->sg, dma_len, tx_fifo,
					DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI, cfg_tx)) {
			pr_err("%s: Tx dma_setup_transfer failed 0x%x\n",__func__,(u32)dd->flags);
			err = -EIO;
			goto err;
		}
	}
	else {
		/* Rx setup */
		if(dma_setup_transfer(dd->rx_dma_chan, (sg_dma_address(rctx->sg)+rctx->offset), rx_fifo, length,
				DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI, cfg_rx)) {
			pr_err("Rx dma_setup_transfer failed %d\n",err);
			err = -EIO;
			goto err;
		}
		/* Tx setup */
		if(dma_setup_transfer(dd->tx_dma_chan, tx_fifo, (sg_dma_address(rctx->sg)+rctx->offset)/*dd->dma_addr*/, length,
				DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI, cfg_tx)) {
			pr_err("Tx dma_setup_transfer failed %d\n",err);
			err = -EIO;
			goto err;
		}
	}

	spum_dma_init(dd->io_axi_base);

	set_bit(FLAGS_TBUSY, &dd->flags);
	set_bit(FLAGS_RBUSY, &dd->flags);

	if(rctx->tx_len >= length)
		rctx->tx_len -= length;
	else
		rctx->tx_len = 0;

	if(rctx->rx_len >= length)
		rctx->rx_len -= length;
	else {
		length -= rctx->rx_len;
		if(rctx->partial >= length)
			rctx->partial -= length;
		else
			rctx->partial = 0;
		rctx->rx_len = 0;
	}

	/* Rx start xfer */
	if(dma_start_transfer(dd->rx_dma_chan)) {
		pr_err("Rx dma transfer failed.\n");
		err = -EIO;
		goto err;
	}

	/* Tx start xfer */
	if(dma_start_transfer(dd->tx_dma_chan)) {
		pr_err("Tx dma transfer failed.\n");
		err = -EIO;
		goto err;
	}

	pr_debug("%s: exit \n",__func__);

	err:
		return err;
}

static int spum_hash_update_data(struct brcm_spum_device *dd)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	u32     length;
	struct scatterlist *sg;
	int     err = 0,sg_cnt;

	pr_debug("%s: entry\n",__func__);

	if(!(rctx->rx_len+ rctx->partial))
		return 0;

	/* Here check for DMA burst size alignment.
	 * If not, Do length -= (length%dma_burst_size)
	 * Seting rctx->offset = length should take care the remaining data tx.
	 */
	if(rctx->rx_len > SPUM_HASH_BLOCK_SIZE && (!((u32)sg_virt(rctx->sg)%4))) {
		dd->dma_len = get_sg_list(rctx, rctx->sg, &length);

		if(dd->dma_len > 1) {
			sg_cnt = dma_map_sg(dd->dev, rctx->sg, dd->dma_len, DMA_TO_DEVICE);
			if(sg_cnt) {
				return spum_dma_xfer(dd, sg_cnt, length);
			}
			else {
				pr_err("%s:dma map error\n",__func__);
			}
		}
		else if(dd->dma_len && (!(length%64)) && 
				(!((u32)(sg_virt(rctx->sg)+rctx->offset)%4))) {
                        sg = rctx->sg;
                        if(sg_is_last(sg)) {
				u32 offset = rctx->offset;
                                rctx->offset += length;
                                spum_append_sg(rctx);
                                rctx->offset = offset;
                                rctx->partial -= rctx->bufcnt;
                                rctx->sg = sg;
                                if (!length)
                                        return 0;
                        }
                        sg_cnt = dma_map_sg(dd->dev, rctx->sg, dd->dma_len, DMA_TO_DEVICE);
			if(sg_cnt) {
				return spum_dma_xfer(dd, sg_cnt, length);
			}
			else {
				pr_err("%s:dma map error\n",__func__);
			}
		}
	}

	length = min((rctx->rx_len+rctx->partial), (rctx->sg->length - rctx->offset));
	sg = rctx->sg;
	if(!length)
		return 0;

	pr_debug("%s: length %d offset %d\n",__func__,length,rctx->offset);

	if (sg_virt(sg) != rctx->buffer) { /* If it's not header. */
                if(sg_is_last(sg)) {
                        if(rctx->bufcnt) {
                                spum_append_sg(rctx);
                                if(rctx->bufcnt==64) {
                                        err = spum_cpu_xfer(dd, rctx->buff, rctx->bufcnt);
                                        rctx->bufcnt = 0;
                                }
                                if(rctx->sg)
                                        return spum_hash_process_request(dd);
                                else
                                        return 0;
                        }
                        else {
				u32 offset = rctx->offset;
                                length -= rctx->partial;
                                rctx->offset += length;
                                spum_append_sg(rctx);
                                rctx->offset = offset;
                                rctx->partial -= rctx->bufcnt;
                                rctx->sg = sg;
                                if (!length)
                                        return 0;
                        }
                }
		else if(length%4) {
			if(sg_virt(sg) == rctx->buff) {
				rctx->sg = scatterwalk_sg_next(rctx->sg);
				rctx->bufcnt = length;
				if(rctx->sg)
					return spum_hash_process_request(dd);
				else
					return 0;
			}
			else {
				spum_append_sg(rctx);
				if(rctx->bufcnt==64) {
					err = spum_cpu_xfer(dd, rctx->buff, rctx->bufcnt);
					rctx->bufcnt = 0;
				}
				if(rctx->sg) 
					return spum_hash_process_request(dd);
				else
					return 0;
			}
		}
		else if(rctx->bufcnt) {
			spum_append_sg(rctx);
			if(rctx->bufcnt==64) {
				err = spum_cpu_xfer(dd, rctx->buff, rctx->bufcnt);
				rctx->bufcnt = 0;
			}
			if(rctx->sg)
				return spum_hash_process_request(dd);
			else
				return 0;
		}
	}

	err = spum_cpu_xfer(dd, (sg_virt(sg)+rctx->offset), length);
	return spum_hash_process_request(dd);
}

static int spum_hash_update_final(struct brcm_spum_device *dd)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	u32 length;
	int err = 0;

	if(!rctx->rx_len)
		return 0;
	length = min(rctx->rx_len, (rctx->sg->length - rctx->offset));

	if((length <= SPUM_HASH_BLOCK_SIZE) || 
		(!IS_ALIGNED(rctx->sg->offset, sizeof(u32))) || (!(IS_ALIGNED(length,16)))) {
			err = spum_cpu_xfer(dd, (sg_virt(rctx->sg)), length);
			return spum_hash_process_request(dd);
	}
	else {
		dd->dma_len = get_sg_list(rctx, rctx->sg, &length);

		if(!dma_map_sg(dd->dev, rctx->sg, dd->dma_len, DMA_TO_DEVICE)) {
			pr_err("%s:dma map error\n",__func__);
			return -EINVAL;
		}
		err = spum_dma_xfer(dd,dd->dma_len, length);
	}

	return err;
}

static int spum_hash_process_request(struct brcm_spum_device *dd)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	int err = 0;

	pr_debug("%s: entry \n",__func__);

	if(rctx->op == OP_UPDATE) {
		err = spum_hash_update_data(dd);
	}
	else if(rctx->op == OP_FINAL) {
		err = spum_hash_update_final(dd);
	}

	return err;
}

static int spum_hash_handle_queue(struct brcm_spum_device *dd,
					struct ahash_request *req)
{
	struct crypto_async_request *async_req, *backlog;
	struct spum_request_context *rctx;
	unsigned long flags;
	int ret = 0;

	pr_debug("%s: entry \n",__func__);

	spin_lock_irqsave(&dd->lock, flags);
	if (test_bit(FLAGS_BUSY, &dd->flags)) {
		spin_unlock_irqrestore(&dd->lock, flags);
		return -EBUSY;
	}

	backlog = crypto_get_backlog(&dd->queue);
	async_req = crypto_dequeue_request(&dd->queue);
	if (async_req)
		set_bit(FLAGS_BUSY, &dd->flags);
	spin_unlock_irqrestore(&dd->lock, flags);

	if (!async_req) {
		return ret;
	}

	if (backlog)
		backlog->complete(backlog, -EINPROGRESS);

	req = ahash_request_cast(async_req);
	dd->req = req;
	rctx = ahash_request_ctx(req);

        if((test_bit(FLAGS_TBUSY, &dd->flags))|| (test_bit(FLAGS_RBUSY, &dd->flags)))
                pr_err("%s: DMA Busy 0x%x\n",__func__,(u32)dd->flags);

	clk_enable(dd->spum_open_clk);
	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	spum_set_pkt_length(dd->io_axi_base, rctx->rx_len, rctx->tx_len);
	/* Don't want to pop digest and status words from out FIFO. finish() routine will do that. */
	rctx->tx_len -= (SPUM_OUTPUT_STATUS_LEN + rctx->digestsize);

	ret = spum_hash_process_request(dd);

	if(ret != -EINPROGRESS) {
		spum_hash_finish(dd, ret);
		if(rctx->flags&FLAGS_FINUP)
			ret = spum_hash_final(dd->req);
		tasklet_schedule(&dd->queue_task);
	}

	return ret;
}

static int spum_hash_enqueue(struct ahash_request *req, u32 op)
{
	struct spum_request_context *rctx = ahash_request_ctx(req);
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct spum_hash_context *ctx = crypto_ahash_ctx(tfm);
	unsigned long flags;
	int ret;

	pr_debug("%s: entry\n",__func__);

	rctx->op = op;

	spin_lock_irqsave(&ctx->dd->lock, flags);
	ret = ahash_enqueue_request(&ctx->dd->queue, req);
	spin_unlock_irqrestore(&ctx->dd->lock, flags);
	tasklet_schedule(&ctx->dd->queue_task);

	return ret;
}

static int spum_hash_update(struct ahash_request *req)
{
	struct spum_request_context *rctx = ahash_request_ctx(req);
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct spum_hash_context *ctx = crypto_ahash_ctx(tfm);
	struct spum_hw_context  spum_hw_hash_ctx;
	u32 cmd_len_bytes;

	pr_debug("%s: entry req->nbytes %d\n",__func__,req->nbytes);

	if(!req->nbytes)
		return 0;

	total_payload = req->nbytes>SPUM_MAX_PAYLOAD_PER_CMD? req->nbytes:0;

	if(req->nbytes > SPUM_MAX_PAYLOAD_PER_CMD) {
                req->nbytes = get_sg_update(req->src);
	}

	rctx->sg = req->src;
	rctx->total = req->nbytes;
	rctx->result = req->result;

	if(rctx->bufcnt + req->nbytes <= rctx->buflen) {
		spum_append_sg(rctx);
		return 0;
	}

	rctx->partial = (rctx->bufcnt + rctx->total)%SPUM_HASH_BLOCK_SIZE;
	rctx->rx_len = (rctx->bufcnt + rctx->total) - rctx->partial;
	rctx->blk_cnt += rctx->rx_len;

	memset((void*)&spum_hw_hash_ctx, 0, sizeof(spum_hw_hash_ctx));

	spum_hw_hash_ctx.operation      =       SPUM_CRYPTO_ENCRYPTION;
	spum_hw_hash_ctx.crypto_algo    =       SPUM_CRYPTO_ALGO_NULL;
	spum_hw_hash_ctx.auth_mode      =       SPUM_AUTH_MODE_HASH;
	spum_hw_hash_ctx.auth_order     =       SPUM_CMD_AUTH_FIRST;
	spum_hw_hash_ctx.key_type       =       SPUM_KEY_OPEN;

	spum_hw_hash_ctx.data_attribute.mac_length  = rctx->rx_len;
	spum_hw_hash_ctx.data_attribute.data_length = rctx->rx_len;
	spum_hw_hash_ctx.data_attribute.mac_offset  = 0;
	spum_hw_hash_ctx.auth_key = ctx->hash_init? (void *)(&ctx->digest) : 0;

	switch(rctx->digestsize) {
		case SHA1_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA1;
			spum_hw_hash_ctx.icv_len   = (SHA1_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type =
			ctx->hash_init? SPUM_AUTH_TYPE_SHA1_UPDATE : SPUM_AUTH_TYPE_SHA1_INIT;
			spum_hw_hash_ctx.auth_key_len =
					ctx->hash_init? (SHA1_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

		case SHA224_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA224;
			spum_hw_hash_ctx.icv_len   = (SHA224_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type =
			ctx->hash_init? SPUM_AUTH_TYPE_SHA1_UPDATE : SPUM_AUTH_TYPE_SHA1_INIT;
			spum_hw_hash_ctx.auth_key_len =
					ctx->hash_init? (SHA224_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

		case SHA256_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA256;
			spum_hw_hash_ctx.icv_len   = (SHA256_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type =
			ctx->hash_init? SPUM_AUTH_TYPE_SHA1_UPDATE : SPUM_AUTH_TYPE_SHA1_INIT;
			spum_hw_hash_ctx.auth_key_len =
					ctx->hash_init? (SHA256_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

		case MD5_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_MD5;
			spum_hw_hash_ctx.icv_len   = (MD5_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type =
			ctx->hash_init? SPUM_AUTH_TYPE_MD5_UPDATE : SPUM_AUTH_TYPE_MD5_INIT;
			spum_hw_hash_ctx.auth_key_len =
					ctx->hash_init? (MD5_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

		default:
			break;
	}

	if(!ctx->hash_init)
		ctx->hash_init = 1;

	cmd_len_bytes = spum_format_command(&spum_hw_hash_ctx, &rctx->buffer);
	
	sg_init_table(rctx->spum_hdr,(rctx->bufcnt)?3:2);
	sg_set_buf(&rctx->spum_hdr[0], &rctx->buffer, cmd_len_bytes);

	if(rctx->bufcnt) {
		sg_set_buf(&rctx->spum_hdr[1], &rctx->buff, rctx->bufcnt);
		scatterwalk_sg_chain(&rctx->spum_hdr[0], 3, req->src);
	}
	else {
		scatterwalk_sg_chain(&rctx->spum_hdr[0], 2, req->src);
	}

	rctx->tx_len = SPUM_OUTPUT_HEADER_LEN +
			spum_hw_hash_ctx.data_attribute.data_length +
			(spum_hw_hash_ctx.icv_len * sizeof(u32)) +
			SPUM_OUTPUT_STATUS_LEN;

	rctx->rx_len += cmd_len_bytes;

	rctx->bufcnt = 0;
	rctx->sg = rctx->spum_hdr;

	return spum_hash_enqueue(req, OP_UPDATE);
}

static int spum_hash_init(struct ahash_request *req)
{
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct spum_hash_context *ctx = crypto_ahash_ctx(tfm);
	struct spum_request_context *rctx = ahash_request_ctx(req);
	struct brcm_spum_device	*dd;
	int    err = 0;

	pr_debug("%s: entry\n",__func__);

	spin_lock_bh(&spum_drv.lock);
	if (!ctx->dd) {
		list_for_each_entry(dd, &spum_drv.dev_list, list) {
			break;
		}
		ctx->dd = dd;
	}
	spin_unlock_bh(&spum_drv.lock);

	ctx->hash_init = 0;
	rctx->bufcnt = 0;
	rctx->buflen = SPUM_HASH_BLOCK_SIZE;
	rctx->flags = 0;
	rctx->rx_len = 0;
	rctx->tx_len = 0;
	rctx->offset = 0;
	rctx->blk_cnt = 0;
	rctx->digestsize = crypto_ahash_digestsize(tfm);
        total_payload = req->nbytes>SPUM_MAX_PAYLOAD_PER_CMD? req->nbytes:0;

	pr_debug("%s: exit\n",__func__);

	return err;
}

static void spum_hash_finish(struct brcm_spum_device *dd, int err)
{
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(dd->req);
	struct spum_hash_context *ctx = crypto_ahash_ctx(tfm);
        struct spum_request_context *rctx = ahash_request_ctx(dd->req);
        u32     tx_fifo[64] = {0, }, status, count, tx_len;
        int     i;

	pr_debug("%s: entry\n",__func__);

	count = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
	count &= SPUM_AXI_FIFO_STAT_OFIFO_LVL_MASK;
	count >>= SPUM_AXI_FIFO_STAT_OFIFO_LVL_SHIFT;
	tx_len = rctx->tx_len/4;

	/* Should also read previous leftover data on tx fifo. max(tx_len+dig, read(fifostate))*/
	count = (tx_len+(rctx->digestsize/4));
	status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
	i=0; 
	while(i<count) {
		status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
		if(  status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK ) {
			tx_fifo[i] = readl(dd->io_axi_base + SPUM_AXI_FIFO_OUT_OFFSET );
			i++;
                }
        }
	rctx->tx_len -= i*4;
	/* rctx->tx_len should be <= 0 here. */

	/* Make a local copy of the intemediate hash value. */
	memcpy(&ctx->digest, (u8 *)&tx_fifo[tx_len], rctx->digestsize);

	clear_bit(FLAGS_BUSY, &dd->flags);
	clear_bit(FLAGS_RBUSY, &dd->flags);
	clear_bit(FLAGS_TBUSY, &dd->flags);
	clk_disable(dd->spum_open_clk);

	if((rctx->op&OP_FINAL) || ((rctx->op&OP_UPDATE) && (!rctx->flags&FLAGS_FINUP)) ||
		((rctx->flags&FLAGS_FINUP) && (rctx->bufcnt == 0))) {

		/* Copy back hash value to request if buffer pointer provided. */
		if(dd->req->result)
			memcpy(dd->req->result, &ctx->digest, rctx->digestsize);

		if(rctx->digestsize == MD5_DIGEST_SIZE)
			b_host_2_be32(dd->req->result, rctx->digestsize/sizeof(u32));

		rctx->offset = 0;
		if(total_payload) {
			dd->req->nbytes = total_payload;
			dd->req->src = sg_updt;
			spum_hash_update(dd->req);
		}
		else if (dd->req->base.complete) {
			dd->req->base.complete(&dd->req->base, err);
			pr_debug("%s: ACK to client offset %d\n",__func__,rctx->offset);
		}
	}

	pr_debug("%s: exit\n",__func__);
}

static int spum_hash_final(struct ahash_request *req)
{
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct spum_hash_context *ctx = crypto_ahash_ctx(tfm);
	struct spum_request_context *rctx = ahash_request_ctx(req);
	struct spum_hw_context  spum_hw_hash_ctx;
	u32 cmd_len_bytes,index,padlen;
	int err = 0;
	static const u8 padding[64] = { 0x80, };
	__be64 bits;

	pr_debug("%s: entry\n",__func__);

	rctx->flags &= ~FLAGS_FINUP;

	index = rctx->bufcnt;
	padlen = (index < 56) ? (56 - index) : ((64+56) - index);

	if(rctx->digestsize == MD5_DIGEST_SIZE) {
		*(u32 *)&rctx->buff[index+padlen] =
			(rctx->bufcnt+rctx->blk_cnt) << 3;
		*(u32 *)&rctx->buff[index+padlen+4] =
			(rctx->bufcnt+rctx->blk_cnt) >> 29;
	}
	else {
		bits = cpu_to_be64((rctx->bufcnt+rctx->blk_cnt) << 3);
		memcpy(&rctx->buff[index+padlen], (const u8 *)&bits, sizeof(bits));
	}

	memcpy(&rctx->buff[index], &padding[0],padlen);
	rctx->bufcnt = index+padlen+sizeof(bits);

	memset((void*)&spum_hw_hash_ctx, 0, sizeof(spum_hw_hash_ctx));

	spum_hw_hash_ctx.operation      =       SPUM_CRYPTO_ENCRYPTION;
	spum_hw_hash_ctx.crypto_algo    =       SPUM_CRYPTO_ALGO_NULL;
	spum_hw_hash_ctx.auth_mode      =       SPUM_AUTH_MODE_HASH;
	spum_hw_hash_ctx.auth_order     =       SPUM_CMD_AUTH_FIRST;
	spum_hw_hash_ctx.key_type       =       SPUM_KEY_OPEN;

	spum_hw_hash_ctx.auth_key = ctx->hash_init? (void *)(&ctx->digest) : 0;

	switch(rctx->digestsize) {
		case SHA1_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA1;
			spum_hw_hash_ctx.icv_len = (SHA1_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type =
				ctx->hash_init? SPUM_AUTH_TYPE_SHA1_UPDATE : SPUM_AUTH_TYPE_SHA1_INIT;
			spum_hw_hash_ctx.auth_key_len = 
				ctx->hash_init? (SHA1_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

		case SHA224_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA224;
			spum_hw_hash_ctx.icv_len = (SHA224_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type = 
				ctx->hash_init? SPUM_AUTH_TYPE_SHA1_UPDATE : SPUM_AUTH_TYPE_SHA1_INIT;
			spum_hw_hash_ctx.auth_key_len = 
				ctx->hash_init? (SHA224_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

		case SHA256_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA256;
			spum_hw_hash_ctx.icv_len = (SHA256_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type = 
				ctx->hash_init? SPUM_AUTH_TYPE_SHA1_UPDATE : SPUM_AUTH_TYPE_SHA1_INIT;
			spum_hw_hash_ctx.auth_key_len = 
				ctx->hash_init? (SHA256_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

		case MD5_DIGEST_SIZE:
			spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_MD5;
			spum_hw_hash_ctx.icv_len = (MD5_DIGEST_SIZE/sizeof(u32));
			spum_hw_hash_ctx.auth_type = 
				ctx->hash_init? SPUM_AUTH_TYPE_MD5_UPDATE : SPUM_AUTH_TYPE_MD5_INIT;
			spum_hw_hash_ctx.auth_key_len = 
				ctx->hash_init? (MD5_DIGEST_SIZE/sizeof(u32)) : 0;
			break;

	}

	spum_hw_hash_ctx.data_attribute.mac_offset  = 0;
	spum_hw_hash_ctx.data_attribute.mac_length  = rctx->bufcnt;
	spum_hw_hash_ctx.data_attribute.data_length = (((rctx->bufcnt + 3)/sizeof(u32))*sizeof(u32));

	cmd_len_bytes = spum_format_command(&spum_hw_hash_ctx, &rctx->buffer);

	rctx->rx_len = cmd_len_bytes + spum_hw_hash_ctx.data_attribute.data_length;
	rctx->tx_len = SPUM_OUTPUT_HEADER_LEN +
		spum_hw_hash_ctx.data_attribute.data_length +
		(spum_hw_hash_ctx.icv_len * sizeof(u32)) +
		SPUM_OUTPUT_STATUS_LEN;

	sg_init_table(rctx->spum_hdr,2);
	sg_set_buf(rctx->spum_hdr, &rctx->buffer, cmd_len_bytes);
	sg_set_buf(&rctx->spum_hdr[1], &rctx->buff, spum_hw_hash_ctx.data_attribute.data_length);

	rctx->sg = rctx->spum_hdr;
	rctx->bufcnt = 0;			

	err = spum_hash_enqueue(req, OP_FINAL);

	pr_debug("%s: exit\n",__func__);

	return err;
}

static int spum_hash_finup(struct ahash_request *req)
{
        struct spum_request_context *rctx = ahash_request_ctx(req);
        int err1, err2;

        pr_debug("%s: entry\n",__func__);

	rctx->flags |= FLAGS_FINUP;

        err1 = spum_hash_update(req);
        if(err1 == -EINPROGRESS)
                return err1;

        err2 = spum_hash_final(req);

        return err1 ?: err2;
}

static int spum_hash_digest(struct ahash_request *req)
{
        pr_debug("%s: entry\n",__func__);
        return spum_hash_init(req) ?: spum_hash_finup(req);
}

static int spum_hash_cra_init(struct crypto_tfm *tfm)
{
	struct spum_hash_context *ctx = crypto_tfm_ctx(tfm);
	const char *alg_name = crypto_tfm_alg_name(tfm);

	pr_debug("%s: entry \n",__func__);

	/* Allocate fallback */
	ctx->fallback = crypto_alloc_shash(alg_name, 0,
				CRYPTO_ALG_NEED_FALLBACK);
	if (IS_ERR(ctx->fallback)) {
		pr_err("%s: fallback driver '%s' "
				"could not be loaded.\n",__func__,alg_name);
		return PTR_ERR(ctx->fallback);
	}

	crypto_ahash_set_reqsize(__crypto_ahash_cast(tfm),
				sizeof(struct spum_request_context));

	return 0;
}

static void spum_hash_cra_exit(struct crypto_tfm *tfm)
{
	struct spum_hash_context *ctx = crypto_tfm_ctx(tfm);

	pr_debug("%s: entry\n",__func__);

	crypto_free_shash(ctx->fallback);
	ctx->fallback = NULL;

	pr_debug("%s: exit\n",__func__);
}


static struct ahash_alg spum_algo[] = {
{
	.init		= spum_hash_init,
	.update		= spum_hash_update,
	.final		= spum_hash_final,
	.finup		= spum_hash_finup,
	.digest		= spum_hash_digest,
	.halg.digestsize	= SHA1_DIGEST_SIZE,
	.halg.base	= {
		.cra_name		= "sha1",
		.cra_driver_name	= "spum-sha1",
		.cra_priority		= 110,
		.cra_flags		= CRYPTO_ALG_TYPE_AHASH |
						CRYPTO_ALG_ASYNC |
						CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize		= SHA1_BLOCK_SIZE,
		.cra_ctxsize		= sizeof(struct spum_hash_context),
		.cra_alignmask		= 0,
		.cra_module		= THIS_MODULE,
		.cra_init		= spum_hash_cra_init,
		.cra_exit		= spum_hash_cra_exit,
	},
},
{	
	.init		= spum_hash_init,
	.update		= spum_hash_update,
	.final		= spum_hash_final,
	.finup		= spum_hash_finup,
	.digest		= spum_hash_digest,
	.halg.digestsize	= SHA224_DIGEST_SIZE,
	.halg.base	= {
		.cra_name		= "sha224",
		.cra_driver_name	= "spum-sha224",
		.cra_priority		= 110,
		.cra_flags		= CRYPTO_ALG_TYPE_AHASH |
						CRYPTO_ALG_ASYNC |
						CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize		= SHA224_BLOCK_SIZE,
		.cra_ctxsize		= sizeof(struct spum_hash_context),
		.cra_alignmask		= 0,
		.cra_module		= THIS_MODULE,
		.cra_init		= spum_hash_cra_init,
		.cra_exit		= spum_hash_cra_exit,
	},
},
{
	.init		= spum_hash_init,
	.update		= spum_hash_update,
	.final		= spum_hash_final,
	.finup		= spum_hash_finup,
	.digest		= spum_hash_digest,
	.halg.digestsize	= SHA256_DIGEST_SIZE,
	.halg.base	= {
		.cra_name		= "sha256",
		.cra_driver_name	= "spum-sha256",
		.cra_priority		= 110,
		.cra_flags		= CRYPTO_ALG_TYPE_AHASH |
						CRYPTO_ALG_ASYNC |
						CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize		= SHA256_BLOCK_SIZE,
		.cra_ctxsize		= sizeof(struct spum_hash_context),
		.cra_alignmask		= 0,
		.cra_module		= THIS_MODULE,
		.cra_init		= spum_hash_cra_init,
		.cra_exit		= spum_hash_cra_exit,
	},
},
{
	.init		= spum_hash_init,
	.update		= spum_hash_update,
	.final		= spum_hash_final,
	.finup		= spum_hash_finup,
	.digest		= spum_hash_digest,
	.halg.digestsize	= MD5_DIGEST_SIZE,
	.halg.base	= {
		.cra_name		= "md5",
		.cra_driver_name	= "spum-md5",
		.cra_priority		= 110,
		.cra_flags		= CRYPTO_ALG_TYPE_AHASH |
						CRYPTO_ALG_ASYNC |
						CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize		= SPUM_HASH_BLOCK_SIZE,
		.cra_ctxsize		= sizeof(struct spum_hash_context),
		.cra_alignmask		= 0,
		.cra_module		= THIS_MODULE,
		.cra_init		= spum_hash_cra_init,
		.cra_exit		= spum_hash_cra_exit,
	},
}
};

static void spum_queue_task(unsigned long data)
{
	struct brcm_spum_device *dd = (struct brcm_spum_device *)data;

	spum_hash_handle_queue(dd, NULL);
}

static void spum_dma_tasklet(unsigned long data)
{
	struct brcm_spum_device *dd = (struct brcm_spum_device *)data;
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	int 	err = -EINPROGRESS;

	pr_debug("%s: entry\n",__func__);

	err = dma_stop_transfer(dd->tx_dma_chan);
	if(err) {
		pr_err("%s: Tx transfer stop failed %d\n",__func__,err);
	}

	err = dma_stop_transfer(dd->rx_dma_chan);
	if(err) {
		pr_err("%s: Rx transfer stop failed %d\n",__func__,err);
	}

	dma_unmap_sg(dd->dev, rctx->sg, dd->dma_len, DMA_TO_DEVICE);

	while(dd->dma_len) {
		pr_debug("%s:length %d offset %d dma_len %d\n",__func__,rctx->sg->length,rctx->offset,dd->dma_len);	
		rctx->sg = scatterwalk_sg_next(rctx->sg);
		dd->dma_len--;
	}

	err = spum_hash_process_request(dd);

	if((err != -EINPROGRESS)) {
		spum_hash_finish(dd, err);
		if(rctx->flags&FLAGS_FINUP)
			err = spum_hash_final(dd->req);
		if(err != -EINPROGRESS)
			tasklet_schedule(&dd->queue_task);
	}
}

static void spum_dma_callback(void *data, enum pl330_xfer_status status)
{
	struct brcm_spum_device *dd;
	u32 *dma_chan = (u32 *)data;

	if (status != DMA_PL330_XFER_OK)
		pr_err("%s: DMA failed. err %d",__func__,status);

	list_for_each_entry(dd, &spum_drv.dev_list, list) {
		break;
	}

	if(*dma_chan == dd->rx_dma_chan) {
		clear_bit(FLAGS_RBUSY, &dd->flags);
	}
	if(*dma_chan == dd->tx_dma_chan) {
		clear_bit(FLAGS_TBUSY, &dd->flags);
	}

	if((!test_bit(FLAGS_TBUSY, &dd->flags)) && (!test_bit(FLAGS_RBUSY, &dd->flags)))
		tasklet_hi_schedule(&dma_tasklet);
}

static int spum_dma_setup(struct brcm_spum_device *dd)
{
	/* Aquire DMA channels */
	if (dma_request_chan(&dd->rx_dma_chan,
					"SPUM_OpenA") != 0) {
		pr_debug("%s: Rx dma_request_chan failed\n", __func__);
		return -1;
	}
	if (dma_request_chan(&dd->tx_dma_chan,
					"SPUM_OpenB") != 0) {
		pr_debug("%s: Tx dma_request_chan failed\n", __func__);
		goto err;
	}

	pr_debug("DMA channel aquired rx %d tx %d\n",dd->rx_dma_chan,dd->tx_dma_chan);

	/* Register DMA callback */
	if (dma_register_callback(dd->rx_dma_chan, spum_dma_callback,
				&dd->rx_dma_chan) != 0) {
		pr_debug("%s: Rx dma_register_callback failed\n", __func__);
		goto err1;
	}
	if (dma_register_callback(dd->tx_dma_chan, spum_dma_callback,
				&dd->tx_dma_chan) != 0) {
		pr_debug("%s: Tx dma_register_callback failed\n", __func__);
		goto err2;
	}
	return 0;
err2:
	dma_free_callback(dd->rx_dma_chan);
err1:
	dma_free_chan(dd->tx_dma_chan);
err:
	dma_free_chan(dd->rx_dma_chan);
	return -EIO;
}

static int __devinit brcm_spum_probe(struct platform_device *pdev)
{
	struct brcm_spum_device *dd;
	struct resource *res;
	u32 *page;
	int ret = 0,i,j;

	pr_debug("%s: entry\n",__func__);

	dd = (struct brcm_spum_device *)
			kzalloc(sizeof(struct brcm_spum_device), GFP_KERNEL);
	if(dd == NULL) {
		pr_err("%s:Failed to allocate spum structure.\n",__func__);
		return -ENOMEM;
	}

	dd->dev = &pdev->dev;
	platform_set_drvdata(pdev, dd);

	/* Initializing the clock. */
	dd->spum_open_clk = clk_get(NULL, "spum_open");
	if (!dd->spum_open_clk) {
		pr_err("%s: Clock intialization failed.\n",__func__);
		kfree(dd);
		return -ENOMEM;
	}

	if(clk_set_rate(dd->spum_open_clk, FREQ_MHZ(156)))
		pr_debug("%s: Clock set failed!!!\n",__func__);

	page = (u32 *)__get_free_page(GFP_KERNEL);
	dd->dma_addr = dma_map_single(dd->dev, page, PAGE_SIZE,
					DMA_FROM_DEVICE);

	if (dma_mapping_error(dd->dev, dd->dma_addr)) {
		pr_err("Failed to get dma addresses\n");
		free_page((unsigned long)page);
		kfree(dd);
		return -ENOMEM;
	}
	dd->dma_virt = page;

	/* Get the base address APB space. */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res) {
		pr_err("%s: Invalid resource type.\n",__func__);
		ret = -EINVAL;
		goto exit;
	}

	dd->io_apb_base = (void __iomem *)HW_IO_PHYS_TO_VIRT(res->start);
	if(!dd->io_apb_base) {
		pr_err("%s: Ioremap failed.\n",__func__);
		ret = -ENOMEM;
		goto exit;
	}

	/* Get the base address AXI space. */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if(!res) {
		pr_err("%s: Invalid resource type.\n",__func__);
		ret = -EINVAL;
		goto exit;
	}

	dd->io_axi_base = (void __iomem *)HW_IO_PHYS_TO_VIRT(res->start);
	if(!dd->io_axi_base) {
		pr_err("%s: Ioremap failed.\n",__func__);
		ret = -ENOMEM;
		goto exit;
	}

	crypto_init_queue(&dd->queue, SPUM_HASH_QUEUE_LENGTH);

	/* Initialize SPU-M block */
	clk_enable(dd->spum_open_clk);
	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	clk_disable(dd->spum_open_clk);

	ret = spum_dma_setup(dd);

	INIT_LIST_HEAD(&dd->list);
	spin_lock_init(&dd->lock);
	spin_lock(&spum_drv.lock);
	list_add_tail(&dd->list, &spum_drv.dev_list);
	spin_unlock(&spum_drv.lock);

	tasklet_init(&dma_tasklet, spum_dma_tasklet, (unsigned long)dd);
	tasklet_init(&dd->queue_task, spum_queue_task, (unsigned long)dd);

	for(i = 0; i < ARRAY_SIZE(spum_algo); i++) {
		ret = crypto_register_ahash(&spum_algo[i]);
		if(ret) {
			pr_err("%s: Crypto algorithm registration failed for %s\n",__func__,spum_algo[i].halg.base.cra_name);
			goto exit_algos;
		}
		else
			pr_info("%s: SPUM %s algorithm registered..\n",__func__,spum_algo[i].halg.base.cra_name);
	} 

	pr_info("%s: SPUM driver registered.\n",__func__);

	return ret;

	exit_algos:
		for(j = 0; j < i; j++)
			crypto_unregister_ahash(&spum_algo[i]);
	exit:
		dma_unmap_single(dd->dev, dd->dma_addr, PAGE_SIZE, DMA_FROM_DEVICE);
		free_page((unsigned long)page);
		kfree(dd);
		return ret;
}

static int __devexit brcm_spum_remove(struct platform_device *pdev)
{
	struct brcm_spum_device *dd;
	int i;

	pr_debug("%s: entry\n",__func__);

	dd = platform_get_drvdata(pdev);
	if (!dd)
		return -ENODEV;

	spin_lock(&spum_drv.lock);
	list_del(&dd->list);
	spin_unlock(&spum_drv.lock);

	for(i = 0; i < ARRAY_SIZE(spum_algo); i++)
		crypto_unregister_ahash(&spum_algo[i]);

	tasklet_kill(&dma_tasklet);
	tasklet_kill(&dd->queue_task);

	dma_unmap_single(dd->dev, dd->dma_addr, PAGE_SIZE, DMA_FROM_DEVICE);
	free_page((unsigned long)dd->dma_virt);
	clk_put(dd->spum_open_clk);
	kfree(dd);

	return 0;
}

static struct platform_driver brcm_spum_driver = {
	.probe  =       brcm_spum_probe,
	.remove =       brcm_spum_remove,
	.driver =       {
		.name   =       "brcm-spum",
		.owner  =       THIS_MODULE,
	},
};

static int __init brcm_spum_init(void)
{
	pr_info("%s: SPUM driver init.\n",__func__);
	return platform_driver_register(&brcm_spum_driver);
}

static void __init brcm_spum_exit(void)
{
	pr_info("%s: SPUM driver exit.\n",__func__);
	platform_driver_unregister(&brcm_spum_driver);
}

module_init(brcm_spum_init);
module_exit(brcm_spum_exit);

MODULE_LICENSE("GPL V2");

