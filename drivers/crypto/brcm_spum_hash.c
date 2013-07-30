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
/*#define DEBUG 1*/
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

/*#define  SPUM_PIO_MODE*/
#define	OP_UPDATE	1
#define	OP_FINAL	2
#define OP_INIT		3

#define SHA224_INT_DIGEST_SIZE 32

#define FLAGS_FINUP	1

#define SPUM_HASH_BLOCK_SIZE	64
#define SPUM_MAX_PAYLOAD_PER_CMD	(SZ_64K - SZ_64)

#define DWORD_ALIGNED	__attribute__((aligned(sizeof(u64))))

#define b_host_2_be32(__buf, __size)       \
do {                                        \
	u32 *b = (u32 *)(__buf);            \
	u32 i;                              \
	for (i = 0; i < __size; i++, b++) { \
		*b = cpu_to_be32(*b);               \
	}                                         \
} while (0)

struct spum_hash_local_buff {
	u32 bufcnt;
	u32 buflen;
	u8 lbuff[PAGE_SIZE] DWORD_ALIGNED;
};

struct spum_request_context {
	u32 op;
	u8 *result;
	u8 index;
	u32 total;
	u32 digestsize;
	u32 flags;

	u32 offset;
	u32 partial;
	u32 rx_len;
	u32 blk_cnt;
	u32 total_payload;
	struct scatterlist *sg_updt;

	u32 tlen;
	struct scatterlist *tsg;

	struct scatterlist *sg;
	struct spum_hash_local_buff buff[2];

	u32 hash_init;
	u8 digest[SHA512_DIGEST_SIZE];	/* Max. digest size in bytes */

	struct scatterlist spum_data_sgl[3];
	struct scatterlist spum_in_cmd_hdr;
	struct scatterlist spum_out_cmd_hdr;
	struct scatterlist spum_in_cmd_stat;
	struct scatterlist spum_out_cmd_stat;
	struct scatterlist spum_local_data;

	u8 spum_in_hdr[128] DWORD_ALIGNED;
	u8 spum_out_hdr[128] DWORD_ALIGNED;
	u8 spum_in_stat[4] DWORD_ALIGNED;
	u8 spum_out_stat[4] DWORD_ALIGNED;
};

struct spum_hash_context {
	struct spum_hash_device *dd;
};

struct brcm_spum_driver {
	struct list_head dev_list;
	spinlock_t lock;
};

static struct brcm_spum_driver spum_drv = {
	.dev_list = LIST_HEAD_INIT(spum_drv.dev_list),
	.lock = __SPIN_LOCK_UNLOCKED(spum_drv.lock),
};

static int spum_hash_finish(struct spum_hash_device *dd, int err);
static int spum_hash_final(struct ahash_request *req);
static int spum_dma_setup(struct spum_hash_device *dd);

struct tasklet_struct dma_tasklet;

#ifdef DEBUG
static void hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
		       16, 1, buf, len, false);
}
#endif

void dump_sg_list(struct scatterlist *sg)
{
	struct scatterlist *sgl = sg;

	while (sgl) {
		pr_debug("%s: %d\n", __func__, sgl->length);
		sgl = sg_next(sgl);
	}
}

static inline void spum_sg_chain(struct scatterlist *prv,
		 unsigned int prv_nents, struct scatterlist *sgl)
{
	/*
	* offset and length are unused for chain entry.  Clear them.
	*/
	prv[prv_nents - 1].offset = 0;
	prv[prv_nents - 1].length = 0;

	/*
	* Set lowest bit to indicate a link pointer, and make sure to clear
	* the termination bit if it happens to be set.
	*/
	prv[prv_nents - 1].page_link = ((unsigned long) sgl | 0x01) & ~0x02;
}

static unsigned int get_sg_list_len(struct scatterlist *sg)
{
	struct scatterlist *sgl = sg;
	unsigned int cnt = 0;

	while (sgl) {
		sgl = sg_next(sgl);
		cnt++;
	}
	return cnt;
}

/* API to map/unmap sg nodes. */
static int spum_map_sgl(struct spum_hash_device *dd, struct scatterlist *sgl,
			u32 sg_len, enum dma_data_direction dir)
{
	struct scatterlist *sg;
	int len = 0;

	for_each_sg(sgl, sg, sg_len, len) {
		if (!dma_map_sg(dd->dev, sg, 1, dir)) {
			pr_err("%s: dma_map_sg() error\n", __func__);
			len = -EINVAL;
			break;
		}
	}

	return len;
}

static void spum_unmap_sgl(struct spum_hash_device *dd,
			 struct scatterlist *sgl, u32 sg_len,
			 enum dma_data_direction dir)
{
	struct scatterlist *sg;
	int len = 0;

	for_each_sg(sgl, sg, sg_len, len) {
		dma_unmap_sg(dd->dev, sg, 1, dir);
	}
}

/* Mark the sg list for DMA xfer. */
static u32 get_sg_list(struct spum_request_context *rctx,
		       struct scatterlist *sgl, u32 *length)
{
	struct scatterlist *sg = sgl;
	u32 count = 0, len = 0, sglen;

	while (sg) {
		sglen = min(rctx->rx_len, (sg->length - rctx->offset));
		if (!(sglen % sizeof(u32)) &&
		    ((len + sglen) <= rctx->rx_len)) {
			count++;
			len += sglen;
		} else {
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
static u32 get_sg_update(struct spum_request_context *rctx,
			struct scatterlist *sgl)
{
	struct scatterlist *sg;
	u32 length = 0;

	while (sgl) {
		if ((length + sgl->length) <= SPUM_MAX_PAYLOAD_PER_CMD) {
			length += sgl->length;
			sg = sgl;
			sgl = scatterwalk_sg_next(sgl);
		} else {
			rctx->sg_updt = sgl;
			sg_mark_end(rctx->sg_updt);
			break;
		}
	}

	rctx->total_payload -= length;
	return length;
}


static u32 spum_append_buffer(struct spum_request_context *rctx,
			      const u8 *data, u32 length, u32 index)
{
	u32 count = min(length, rctx->buff[index].buflen
				 - rctx->buff[index].bufcnt);

	count = min(count, rctx->total);
	if (count <= 0)
		return 0;

	memcpy(rctx->buff[index].lbuff + rctx->buff[index].bufcnt, data, count);
	rctx->buff[index].bufcnt += count;
	return count;
}

static void spum_append_sg(struct spum_request_context *rctx, u32 index)
{
	u32 count;

	while (rctx->sg) {
		count = spum_append_buffer(rctx,
				sg_virt(rctx->sg) + rctx->offset,
				rctx->sg->length - rctx->offset, index);
		if (!count)
			break;
		rctx->offset += count;

		if (rctx->offset == rctx->sg->length) {
			rctx->offset = 0;
			rctx->sg = scatterwalk_sg_next(rctx->sg);
		}
		rctx->total -= count;
	}
}

static int spum_dma_xfer(struct spum_hash_device *dd,
			struct scatterlist *sgl, u32 sg_len, u32 length)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	struct list_head head_in, head_out;
	struct dma_transfer_list_sg lli_in_hdr, lli_in_data, lli_in_stat;
	struct dma_transfer_list_sg lli_out_hdr, lli_out_data, lli_out_stat;
	u32 cfg_rx, cfg_tx, rx_fifo, tx_fifo;
	unsigned long flags;
	int err = -EINPROGRESS;

	if (!test_bit(FLAGS_BUSY, &spum_dev->flags)) {
		pr_err("%s: Device is busy!!!", __func__);
		BUG();
	}

	/* Only this configuration works. */
	cfg_rx = DMA_CFG_SRC_ADDR_INCREMENT | DMA_CFG_DST_ADDR_FIXED |
		DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_16 |
		PERIPHERAL_FLUSHP_END | DMA_PERI_REQ_ALWAYS_BURST;
	cfg_tx = DMA_CFG_SRC_ADDR_FIXED | DMA_CFG_DST_ADDR_INCREMENT |
		DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_16 |
		PERIPHERAL_FLUSHP_END | DMA_PERI_REQ_ALWAYS_BURST;

	rx_fifo =
	(u32)HW_IO_VIRT_TO_PHYS(dd->io_axi_base) + SPUM_AXI_FIFO_IN_OFFSET;
	tx_fifo =
	(u32)HW_IO_VIRT_TO_PHYS(dd->io_axi_base) + SPUM_AXI_FIFO_OUT_OFFSET;

	/* Prepare LLI list for DMA xfer. */
	INIT_LIST_HEAD(&head_in);
	INIT_LIST_HEAD(&head_out);

	/* Create IN lli */
	/* Header */
	lli_in_hdr.sgl = &rctx->spum_in_cmd_hdr;
	lli_in_hdr.sg_len = get_sg_list_len(&rctx->spum_in_cmd_hdr);
	spum_map_sgl(dd, &rctx->spum_in_cmd_hdr, lli_in_hdr.sg_len,
			 DMA_TO_DEVICE);

	/* Payload */
	lli_in_data.sgl = sgl;
	lli_in_data.sg_len = sg_len;
	spum_map_sgl(dd, sgl, sg_len, DMA_TO_DEVICE);

	/* status */
	lli_in_stat.sgl = &rctx->spum_in_cmd_stat;
	lli_in_stat.sg_len = get_sg_list_len(&rctx->spum_in_cmd_stat);
	spum_map_sgl(dd, &rctx->spum_in_cmd_stat, lli_in_stat.sg_len,
			 DMA_TO_DEVICE);

	pr_debug("%s: dumping IN HDR sg: \n", __func__);
	dump_sg_list(&rctx->spum_in_cmd_hdr);

	pr_debug("%s: dumping IN payload sg: \n", __func__);
	dump_sg_list(sgl);

	pr_debug("%s: dumping IN STATUS sg: \n", __func__);
	dump_sg_list(&rctx->spum_in_cmd_stat);

	/* Create OUT lli */
	/* Header */
	lli_out_hdr.sgl = &rctx->spum_out_cmd_hdr;
	lli_out_hdr.sg_len = get_sg_list_len(&rctx->spum_out_cmd_hdr);
	spum_map_sgl(dd, &rctx->spum_out_cmd_hdr, lli_out_hdr.sg_len,
			 DMA_FROM_DEVICE);

	/* Payload */
	lli_out_data.sgl = sgl;
	lli_out_data.sg_len = sg_len;
	spum_map_sgl(dd, sgl, sg_len, DMA_FROM_DEVICE);

	/* status */
	lli_out_stat.sgl = &rctx->spum_out_cmd_stat;
	lli_out_stat.sg_len = get_sg_list_len(&rctx->spum_out_cmd_stat);
	spum_map_sgl(dd, &rctx->spum_out_cmd_stat, lli_out_stat.sg_len,
			 DMA_FROM_DEVICE);

	/*  IN Linked list */
	list_add_tail(&lli_in_hdr.next, &head_in);
	list_add_tail(&lli_in_data.next, &head_in);
	list_add_tail(&lli_in_stat.next, &head_in);

	/*  OUT Linked List */
	list_add_tail(&lli_out_hdr.next, &head_out);
	list_add_tail(&lli_out_data.next, &head_out);
	list_add_tail(&lli_out_stat.next, &head_out);

	if (spum_dma_setup(dd)) {
		pr_err("%s: DMA channel callback setup failed.\n", __func__);
		err = -EIO;
		goto err;
	}

	/* Rx setup */
	if (dma_setup_transfer_list_multi_sg
		(spum_dev->rx_dma_chan, &head_in, rx_fifo,
			DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI, cfg_rx)) {
		pr_err("Rx dma_setup_transfer failed %d\n", err);
		err = -EIO;
		goto err;
	}

	/* Tx setup */
	if (dma_setup_transfer_list_multi_sg
		(spum_dev->tx_dma_chan, &head_out, tx_fifo,
		DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI, cfg_tx)) {
		pr_err("Tx dma_setup_transfer failed %d\n", err);
		err = -EIO;
		goto err;
	}

	/* Set the DMA channel busy flags. */
	spin_lock_irqsave(&spum_dev->lock, flags);
	set_bit(FLAGS_TBUSY, &spum_dev->flags);
	set_bit(FLAGS_RBUSY, &spum_dev->flags);
	spin_unlock_irqrestore(&spum_dev->lock, flags);


	/* Rx start xfer */
	if (dma_start_transfer(spum_dev->rx_dma_chan)) {
		pr_err("Rx dma transfer failed.\n");
		err = -EIO;
		goto err;
	}

	/* Tx start xfer */
	if (dma_start_transfer(spum_dev->tx_dma_chan)) {
		pr_err("Tx dma transfer failed.\n");
		err = -EIO;
		goto err;
	}

	/* Enable SPUM DMA interface. */
	spum_dma_init(dd->io_axi_base);

	pr_debug("%s: exit\n", __func__);

err:
	return err;
}

/* Generate SPUM header from the given crypto request. */
static int spum_generate_hdr(struct spum_hash_device *dd, u32 length)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	struct spum_hw_context spum_hw_hash_ctx;
	u32 cmd_len_bytes, rx_len, tx_len;

	memset((void *)&spum_hw_hash_ctx, 0, sizeof(spum_hw_hash_ctx));

	spum_hw_hash_ctx.operation = SPUM_CRYPTO_ENCRYPTION;
	spum_hw_hash_ctx.crypto_algo = SPUM_CRYPTO_ALGO_NULL;
	spum_hw_hash_ctx.auth_mode = SPUM_AUTH_MODE_HASH;
	spum_hw_hash_ctx.auth_order = SPUM_CMD_AUTH_FIRST;
	spum_hw_hash_ctx.key_type = SPUM_KEY_OPEN;

	spum_hw_hash_ctx.data_attribute.mac_length = length;
	spum_hw_hash_ctx.data_attribute.data_length = length;
	spum_hw_hash_ctx.data_attribute.mac_offset = 0;
	spum_hw_hash_ctx.auth_key = rctx->hash_init ?
			(void *)(&rctx->digest) : 0;

	switch (rctx->digestsize) {
	case SHA1_DIGEST_SIZE:
		spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA1;
		spum_hw_hash_ctx.icv_len = (SHA1_DIGEST_SIZE / sizeof(u32));
		spum_hw_hash_ctx.auth_type =
			rctx->
			hash_init ? SPUM_AUTH_TYPE_SHA1_UPDATE :
			SPUM_AUTH_TYPE_SHA1_INIT;
		spum_hw_hash_ctx.auth_key_len =
			rctx->hash_init ? (SHA1_DIGEST_SIZE / sizeof(u32)) : 0;
		break;

	case SHA224_DIGEST_SIZE:
		spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA224;
		/*SHA224 intermediate digest size is 256 bits*/
		spum_hw_hash_ctx.icv_len = (SHA224_DIGEST_SIZE / sizeof(u32))
						+ 1;
		spum_hw_hash_ctx.auth_type =
			rctx->
			hash_init ? SPUM_AUTH_TYPE_SHA1_UPDATE :
			SPUM_AUTH_TYPE_SHA1_INIT;
		spum_hw_hash_ctx.auth_key_len =
			rctx->hash_init ?
				((SHA224_DIGEST_SIZE / sizeof(u32)) + 1) : 0;
		break;

	case SHA256_DIGEST_SIZE:
		spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_SHA256;
		spum_hw_hash_ctx.icv_len = (SHA256_DIGEST_SIZE / sizeof(u32));
		spum_hw_hash_ctx.auth_type =
			rctx->
			hash_init ? SPUM_AUTH_TYPE_SHA1_UPDATE :
			SPUM_AUTH_TYPE_SHA1_INIT;
		spum_hw_hash_ctx.auth_key_len =
			rctx->hash_init ?
				(SHA256_DIGEST_SIZE / sizeof(u32)) : 0;
		break;

	case MD5_DIGEST_SIZE:
		spum_hw_hash_ctx.auth_algo = SPUM_AUTH_ALGO_MD5;
		spum_hw_hash_ctx.icv_len = (MD5_DIGEST_SIZE / sizeof(u32));
		spum_hw_hash_ctx.auth_type =
			rctx->
			hash_init ? SPUM_AUTH_TYPE_MD5_UPDATE :
			SPUM_AUTH_TYPE_MD5_INIT;
		spum_hw_hash_ctx.auth_key_len =
			rctx->hash_init ? (MD5_DIGEST_SIZE / sizeof(u32)) : 0;
		break;

	default:
		break;
	}

	if (!rctx->hash_init)
		rctx->hash_init = 1;

	cmd_len_bytes = spum_format_command(&spum_hw_hash_ctx,
					 &rctx->spum_in_hdr);

	/* Set sg node for in/out header and status word. */
	sg_init_one(&rctx->spum_in_cmd_hdr, rctx->spum_in_hdr, cmd_len_bytes);
	sg_init_one(&rctx->spum_out_cmd_hdr, rctx->spum_out_hdr,
			SPUM_OUTPUT_HEADER_LEN);
	sg_init_one(&rctx->spum_in_cmd_stat, rctx->spum_in_stat,
			SPUM_OUTPUT_STATUS_LEN);
	sg_init_one(&rctx->spum_out_cmd_stat, rctx->spum_out_stat,
			(SPUM_OUTPUT_STATUS_LEN +
			(spum_hw_hash_ctx.icv_len * sizeof(u32))));

	/* Calculate the in/out DMA size and configure into SPUM register. */
	tx_len = SPUM_OUTPUT_HEADER_LEN +
		spum_hw_hash_ctx.data_attribute.data_length +
		(spum_hw_hash_ctx.icv_len * sizeof(u32)) +
		 SPUM_OUTPUT_STATUS_LEN;

	rx_len = spum_hw_hash_ctx.data_attribute.data_length + cmd_len_bytes +
		SPUM_INPUT_STATUS_LEN;

	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	spum_set_pkt_length(dd->io_axi_base, rx_len, tx_len);

	return 0;
}

#ifdef SPUM_PIO_MODE
static int spum_hash_cpu_xfer(struct spum_hash_device *dd,
		struct scatterlist *in_sg, struct scatterlist *out_sg,
		int len, int type)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	struct spum_hw_context *spum_cmd =
				(struct spum_hw_context *)rctx->spum_in_hdr;
	u32 status, in_len, out_len;
	u32 *in_buff, *out_buff;
	u32 result_fifo[32];
	int ret = 0, j = 0;
	u32 i = 0;
	spum_dma_init(dd->io_axi_base);
	in_buff = (u32 *)sg_virt(in_sg);
	out_buff = (u32 *)sg_virt(out_sg);

	pr_debug("%s: entry\n", __func__);

	/*Calculating length to be sent and received*/
	if (!type) {
		in_len = (in_sg->length+3)/sizeof(u32);
		out_len = ((out_sg->length+3)/sizeof(u32));
	} else {
		if (in_sg->length > len) {
			in_len = len/sizeof(u32);
			out_len = len/sizeof(u32);
		} else {
			in_len = (in_sg->length+3)/sizeof(u32);
			out_len = ((out_sg->length+3)/sizeof(u32));

		}
	}

	pr_debug("in_len=%d, out_len=%d\n", in_len, out_len);
	/*Send and Receive data*/
	while (j < out_len) {
		status = readl(dd->io_axi_base +
			SPUM_AXI_FIFO_STAT_OFFSET);
		if ((status & SPUM_AXI_FIFO_STAT_IFIFO_RDY_MASK)
			&& (i < in_len)) {
			writel(*in_buff++, dd->io_axi_base +
					SPUM_AXI_FIFO_IN_OFFSET);
			i++;
		}
		status = readl(dd->io_axi_base +
			SPUM_AXI_FIFO_STAT_OFFSET);
		if (status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK) {
			*out_buff++ = readl(dd->io_axi_base +
				SPUM_AXI_FIFO_OUT_OFFSET);
			j++;

		}
	}

	j = 0;

	/*If data is payload reading the hash value*/
	if (type == 1) {
		status = readl(dd->io_axi_base +
			SPUM_AXI_FIFO_STAT_OFFSET);
		while ((status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK)
					&& j < 8) {
			result_fifo[j % 32] = readl(dd->io_axi_base +
					SPUM_AXI_FIFO_OUT_OFFSET);
			j++;
			status = readl(dd->io_axi_base +
				SPUM_AXI_FIFO_STAT_OFFSET);
		}
	}

	/*Copy the rssult hash to send back*/
	/*SHA224 intermediate digest size is 256 bits*/
	if (rctx->digestsize == SHA224_DIGEST_SIZE && rctx->op == OP_UPDATE)
		memcpy(&rctx->digest, result_fifo, SHA224_INT_DIGEST_SIZE);
	else
		memcpy(&rctx->digest, result_fifo, rctx->digestsize);

	pr_debug("%s: exit\n", __func__);
	return ret;

}

static int spum_hash_pio_xfer(struct spum_hash_device *dd,
			struct scatterlist *sg, int length)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	struct spum_hw_context *spum_cmd =
				(struct spum_hw_context *)rctx->spum_in_hdr;
	struct scatterlist *in_sg;
	struct scatterlist *out_sg;
	u32 locallength = 0, len = length;
	int ret;
	/*Sending header*/
	pr_debug("%s: entry\n", __func__);
	spum_hash_cpu_xfer(dd, &rctx->spum_in_cmd_hdr,
			&rctx->spum_out_cmd_hdr, 0, 0);

	in_sg = sg;
	out_sg = sg;
	pr_debug("header sent and received in func %s", __func__);
	/*Sending payload*/
	while (in_sg && out_sg) {
		ret = spum_hash_cpu_xfer(dd, in_sg, out_sg, len, 1);
		if (in_sg->length > len)
			break;
		else
			len -= in_sg->length;

		in_sg = sg_next(in_sg);
		out_sg = sg_next(out_sg);
	}
	pr_debug("payload sent and received in func %s", __func__);
	/*Sending status*/
	/*spum_hash_cpu_xfer(dd, &rctx->spum_in_cmd_stat,
			&rctx->spum_out_cmd_stat, 0);*/

	/*Resoring the original SG*/
	if ((rctx->rx_len == 0)
		&& (rctx->partial != 0)) {
		while (!sg_is_last(sg)) {
			if (sg == rctx->tsg)
				break;
			sg = sg_next(sg);
		}
		sg->length = rctx->tlen;
		pr_debug("sg length reverted back to %d in func %s",
			sg->length, __func__);
	}
	rctx->buff[rctx->index].bufcnt = 0;
	ret = spum_hash_finish(dd, ret);
	if (rctx->flags & FLAGS_FINUP && (ret != -EINPROGRESS))
		ret = spum_hash_final(dd->req);
	if (ret != -EINPROGRESS) {
		if (spum_dev->spum_queue.qlen)
			spum_queue_task((unsigned long)&spum_dev);
	}
	pr_debug("%s: exit\n", __func__);
	return 0;

}
#endif

static int spum_hash_update_data(struct spum_hash_device *dd)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	u32 length;
	int err = 0;

	pr_debug("%s: entry rctx->rx_len %d\n", __func__, rctx->rx_len);

	if (!rctx->rx_len) {
		return 0;
	}

	/* If current sg node partially processed. */
	if (rctx->offset) {
		spum_append_sg(rctx, rctx->index);
		if (!(rctx->buff[rctx->index].bufcnt%sizeof(u32))) {
			sg_init_one(&rctx->spum_local_data,
				rctx->buff[rctx->index].lbuff,
				rctx->buff[rctx->index].bufcnt);
			rctx->rx_len -= rctx->buff[rctx->index].bufcnt;
			dd->sg = &rctx->spum_local_data;
			dd->dma_len = 1;
			spum_generate_hdr(dd, rctx->buff[rctx->index].bufcnt);
			#ifdef SPUM_PIO_MODE
				return  spum_hash_pio_xfer(dd,
					&rctx->spum_local_data,
					 rctx->buff[rctx->index].bufcnt);
			#else
				return spum_dma_xfer(dd, &rctx->spum_local_data,
					1, rctx->buff[rctx->index].bufcnt);
			#endif
		} else
			return -ENOSR;
	}

	/* Get the sg list which can be DMA xfer. */
	dd->dma_len = get_sg_list(rctx, rctx->sg, &length);
	if (dd->dma_len) {
		rctx->rx_len -= length;
		dd->sg = rctx->sg;
		spum_generate_hdr(dd, length);
		#ifdef SPUM_PIO_MODE
			return spum_hash_pio_xfer(dd, dd->sg, length);
		#else
			return spum_dma_xfer(dd, dd->sg,
				dd->dma_len, length);
		#endif
	}

	/* Unaligned size sg node. Handle individually. */
	if ((rctx->sg->length - rctx->offset)%sizeof(u32)) {
		if (sg_virt(rctx->sg) == rctx->buff[rctx->index].lbuff) {
			rctx->sg = sg_next(rctx->sg);
			spum_append_sg(rctx, rctx->index);
			if (!(rctx->buff[rctx->index].bufcnt%sizeof(u32))) {
				sg_init_one(&rctx->spum_local_data,
					 rctx->buff[rctx->index].lbuff,
					 rctx->buff[rctx->index].bufcnt);
				rctx->rx_len -= rctx->buff[rctx->index].bufcnt;
				dd->sg = &rctx->spum_local_data;
				dd->dma_len = 1;
				spum_generate_hdr(dd,
					 rctx->buff[rctx->index].bufcnt);

				#ifdef SPUM_PIO_MODE
					return spum_hash_pio_xfer(dd, dd->sg,
					 rctx->buff[rctx->index].bufcnt);
				#else
					return spum_dma_xfer(dd, dd->sg, 1,
					 rctx->buff[rctx->index].bufcnt);
				#endif
			} else
				return -ENOSR;
		} else {
			spum_append_sg(rctx, rctx->index);
			if (!(rctx->buff[rctx->index].bufcnt%sizeof(u32))) {
				sg_init_one(&rctx->spum_local_data,
					rctx->buff[rctx->index].lbuff,
					rctx->buff[rctx->index].bufcnt);
				rctx->rx_len -= rctx->buff[rctx->index].bufcnt;
				dd->sg = &rctx->spum_local_data;
				dd->dma_len = 1;
				spum_generate_hdr(dd,
					 rctx->buff[rctx->index].bufcnt);
				#ifdef SPUM_PIO_MODE
					return spum_hash_pio_xfer(dd,
					&rctx->spum_local_data,
					rctx->buff[rctx->index].bufcnt);
				#else
					return spum_dma_xfer(dd,
					 &rctx->spum_local_data, 1,
					 rctx->buff[rctx->index].bufcnt);
				#endif
			} else
				return -ENOSR;
		}
	}

	return err;
}

int spum_hash_process_request(struct spum_hash_device *dd)
{
	return spum_hash_update_data(dd);
}

static int spum_hash_update(struct ahash_request *req)
{
	struct spum_request_context *rctx = ahash_request_ctx(req);
	struct scatterlist *sg;
	u32 len = 0;
	int ret = 0;

	pr_debug("%s: entry\n", __func__);
	pr_debug("%s: req->nbytes: %d\n", __func__, req->nbytes);

	if (!req->nbytes)
		return ret;

	rctx->total_payload =
	    req->nbytes > SPUM_MAX_PAYLOAD_PER_CMD ? req->nbytes : 0;

	if (req->nbytes > SPUM_MAX_PAYLOAD_PER_CMD)
		req->nbytes = get_sg_update(rctx, req->src);

	rctx->sg = req->src;
	rctx->total = req->nbytes;
	rctx->result = req->result;

	if (rctx->buff[rctx->index].bufcnt + req->nbytes
			 <= SPUM_HASH_BLOCK_SIZE) {
		pr_debug("%s: Appending to local buff; "
			"bufcnt: %d, req->nbytes: %d\n",
			__func__,
			rctx->buff[rctx->index].bufcnt,
			req->nbytes);
		spum_append_sg(rctx, rctx->index);
		return ret;
	}
	rctx->partial = (rctx->buff[rctx->index].bufcnt + rctx->total) %
			 SPUM_HASH_BLOCK_SIZE;
	rctx->rx_len = (rctx->buff[rctx->index].bufcnt + rctx->total) -
			 rctx->partial;
	rctx->blk_cnt += rctx->rx_len;

	pr_debug("%s: rctx->partial: %d; rctx->rx_len: %d, "
		"rctx->blk_cnt: %d\n", __func__, rctx->partial,
		rctx->rx_len, rctx->blk_cnt);

	/* Strip the partial length payload from the end
	*  of the input sglist and copy into local buffer.
	*/
	if (rctx->partial) {
		struct scatterlist *sgl;

		sg = rctx->sg;
		len += rctx->buff[rctx->index].bufcnt;
		while ((!sg_is_last(rctx->sg)) &&
			((len+rctx->sg->length) <= rctx->rx_len)) {
			len += rctx->sg->length;
			rctx->sg = sg_next(rctx->sg);
		}

		if (sg_is_last(rctx->sg))
			rctx->offset = rctx->sg->length - rctx->partial;
		else
			rctx->offset = (req->nbytes +
					 rctx->buff[rctx->index].bufcnt) -
					 (len + rctx->partial);

		/* Note down the sg being modified */
		sgl = rctx->sg;
		rctx->tsg = sgl;
		rctx->tlen = sgl->length;

		spum_append_sg(rctx, (0x1 ^ rctx->index));
		if (sg_is_last(sgl))
			sgl->length -= rctx->partial;
		else
			sgl->length = (req->nbytes +
					 rctx->buff[rctx->index].bufcnt) -
					 (len + rctx->partial);

		rctx->offset = 0;
		rctx->sg = sg;

	}

	if (rctx->buff[rctx->index].bufcnt) {
		sg_set_buf(&rctx->spum_data_sgl[0],
				 &rctx->buff[rctx->index].lbuff,
				 rctx->buff[rctx->index].bufcnt);
		spum_sg_chain(&rctx->spum_data_sgl[0], 2, req->src);
		rctx->sg = &rctx->spum_data_sgl[0];
	}

	rctx->op = OP_UPDATE;
	ret = spum_enqueue_request(&req->base);

	if (ret == -EINPROGRESS)
		spum_queue_task((unsigned long)&spum_dev);

	return ret;
}

static int spum_hash_init(struct ahash_request *req)
{
	struct crypto_ahash *tfm = crypto_ahash_reqtfm(req);
	struct spum_request_context *rctx = ahash_request_ctx(req);
	int err = 0;

	pr_debug("%s: entry\n", __func__);

	rctx->hash_init = 0;

	rctx->index = 0;
	rctx->buff[0].bufcnt = 0;
	rctx->buff[1].bufcnt = 0;
	rctx->buff[0].buflen = PAGE_SIZE;
	rctx->buff[1].buflen = PAGE_SIZE;

	rctx->flags = 0;
	rctx->rx_len = 0;
	rctx->offset = 0;
	rctx->blk_cnt = 0;
	rctx->digestsize = crypto_ahash_digestsize(tfm);
	rctx->total_payload =
	    req->nbytes > SPUM_MAX_PAYLOAD_PER_CMD ? req->nbytes : 0;

	pr_debug("%s: exit\n", __func__);

	return err;
}

static int spum_hash_finish(struct spum_hash_device *dd, int err)
{
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	int ret = 0;
	unsigned long flags;

	pr_debug("%s: entry\n", __func__);

	spin_lock_irqsave(&spum_dev->lock, flags);
	clear_bit(FLAGS_BUSY, &spum_dev->flags);
	spin_unlock_irqrestore(&spum_dev->lock, flags);

	rctx->offset = 0;
	if (!rctx->buff[rctx->index].bufcnt)
		rctx->index ^= 0x1;

	if ((rctx->op & OP_FINAL)
	    || ((rctx->op & OP_UPDATE) && (!rctx->flags & FLAGS_FINUP))) {

		/* Copy back hash value to request if buffer pointer provided */
		if (dd->req->result)
			memcpy(dd->req->result, &rctx->digest,
						rctx->digestsize);

		if (rctx->digestsize == MD5_DIGEST_SIZE)
			b_host_2_be32(dd->req->result,
				      rctx->digestsize / sizeof(u32));

		if (rctx->total_payload) {
			dd->req->nbytes = rctx->total_payload;
			dd->req->src = rctx->sg_updt;
			ret = spum_hash_update(dd->req);
		} else if (dd->req->base.complete) {
			dd->req->base.complete(&dd->req->base, err);
			pr_debug("%s: ACK to client offset %d\n", __func__,
				 rctx->offset);
		}
	}

	pr_debug("%s: exit\n", __func__);

	return ret;
}

static int spum_hash_final(struct ahash_request *req)
{
	struct spum_request_context *rctx = ahash_request_ctx(req);
	static const u8 padding[64] = { 0x80, };
	u32 index, padlen;
	int ret = 0;
	__be64 bits;

	pr_debug("%s: entry\n", __func__);

	rctx->flags &= ~FLAGS_FINUP;
	rctx->offset = 0;

	/* Pad the last chunk of payload to make it
	*  multiple of hash block size.
	*/
	index = rctx->buff[rctx->index].bufcnt;
	padlen = ((index%64) < 56) ? (56 - (index%64)) :
			 ((64 + 56) - (index%64));

	if (rctx->digestsize == MD5_DIGEST_SIZE) {
		*(u32 *)&rctx->buff[rctx->index].lbuff[index + padlen] =
		    (rctx->buff[rctx->index].bufcnt + rctx->blk_cnt) << 3;
		*(u32 *)&rctx->buff[rctx->index].lbuff[index + padlen + 4] =
		    (rctx->buff[rctx->index].bufcnt + rctx->blk_cnt) >> 29;
	} else {
		bits = cpu_to_be64((rctx->buff[rctx->index].bufcnt +
					 rctx->blk_cnt) << 3);
		memcpy(&rctx->buff[rctx->index].lbuff[index + padlen],
			 (const u8 *)&bits, sizeof(bits));
	}

	memcpy(&rctx->buff[rctx->index].lbuff[index], &padding[0], padlen);
	rctx->buff[rctx->index].bufcnt = index + padlen + sizeof(bits);

	rctx->rx_len = rctx->buff[rctx->index].bufcnt;

	sg_init_one(&rctx->spum_local_data, rctx->buff[rctx->index].lbuff,
			 rctx->buff[rctx->index].bufcnt);

	rctx->sg = &rctx->spum_local_data;
	rctx->buff[rctx->index].bufcnt = 0;

	/* Queue the crypto request. */
	rctx->op = OP_FINAL;
	ret = spum_enqueue_request(&req->base);

	if (ret == -EINPROGRESS)
		spum_queue_task((unsigned long)&spum_dev);

	return ret;
}

static int spum_hash_finup(struct ahash_request *req)
{
	struct spum_request_context *rctx = ahash_request_ctx(req);
	int err1, err2;

	pr_debug("%s: entry\n", __func__);

	rctx->flags |= FLAGS_FINUP;

	err1 = spum_hash_update(req);
	if (err1 == -EINPROGRESS)
		return err1;

	err2 = spum_hash_final(req);

	return err1 ? : err2;
}

static int spum_hash_digest(struct ahash_request *req)
{
	pr_debug("%s: entry\n", __func__);
	return spum_hash_init(req) ? : spum_hash_finup(req);
}

static int spum_hash_cra_init(struct crypto_tfm *tfm)
{
	struct spum_hash_context *ctx = crypto_tfm_ctx(tfm);
	struct spum_hash_device *dd = NULL;
	int err = 0;

	pr_debug("%s: entry\n", __func__);

	list_for_each_entry(dd, &spum_drv.dev_list, list) {
		break;
	}
	ctx->dd = dd;

	crypto_ahash_set_reqsize(__crypto_ahash_cast(tfm),
				 sizeof(struct spum_request_context));

	clk_enable(ctx->dd->spum_open_clk);

	return err;
}

static void spum_hash_cra_exit(struct crypto_tfm *tfm)
{
	struct spum_hash_context *ctx = crypto_tfm_ctx(tfm);

	pr_debug("%s: entry\n", __func__);

	clk_disable(ctx->dd->spum_open_clk);

	pr_debug("%s: exit\n", __func__);
}

static struct ahash_alg spum_algo[] = {
	{
	 .init = spum_hash_init,
	 .update = spum_hash_update,
	 .final = spum_hash_final,
	 .finup = spum_hash_finup,
	 .digest = spum_hash_digest,
	 .halg.digestsize = SHA1_DIGEST_SIZE,
	 .halg.base = {
		       .cra_name = "sha1",
		       .cra_driver_name = "spum-sha1",
		       .cra_priority = 110,
		       .cra_flags = CRYPTO_ALG_TYPE_AHASH |
			CRYPTO_ALG_ASYNC | CRYPTO_TFM_REQ_MAY_BACKLOG,
		       .cra_blocksize = SHA1_BLOCK_SIZE,
		       .cra_ctxsize = sizeof(struct spum_hash_context),
		       .cra_alignmask = 0,
		       .cra_module = THIS_MODULE,
		       .cra_init = spum_hash_cra_init,
		       .cra_exit = spum_hash_cra_exit,
		       },
	 },
	{
	 .init = spum_hash_init,
	 .update = spum_hash_update,
	 .final = spum_hash_final,
	 .finup = spum_hash_finup,
	 .digest = spum_hash_digest,
	 .halg.digestsize = SHA224_DIGEST_SIZE,
	 .halg.base = {
		       .cra_name = "sha224",
		       .cra_driver_name = "spum-sha224",
		       .cra_priority = 110,
		       .cra_flags = CRYPTO_ALG_TYPE_AHASH |
			CRYPTO_ALG_ASYNC | CRYPTO_TFM_REQ_MAY_BACKLOG,
		       .cra_blocksize = SHA224_BLOCK_SIZE,
		       .cra_ctxsize = sizeof(struct spum_hash_context),
		       .cra_alignmask = 0,
		       .cra_module = THIS_MODULE,
		       .cra_init = spum_hash_cra_init,
		       .cra_exit = spum_hash_cra_exit,
		       },
	 },
	{
	 .init = spum_hash_init,
	 .update = spum_hash_update,
	 .final = spum_hash_final,
	 .finup = spum_hash_finup,
	 .digest = spum_hash_digest,
	 .halg.digestsize = SHA256_DIGEST_SIZE,
	 .halg.base = {
		       .cra_name = "sha256",
		       .cra_driver_name = "spum-sha256",
		       .cra_priority = 110,
		       .cra_flags = CRYPTO_ALG_TYPE_AHASH |
			CRYPTO_ALG_ASYNC | CRYPTO_TFM_REQ_MAY_BACKLOG,
		       .cra_blocksize = SHA256_BLOCK_SIZE,
		       .cra_ctxsize = sizeof(struct spum_hash_context),
		       .cra_alignmask = 0,
		       .cra_module = THIS_MODULE,
		       .cra_init = spum_hash_cra_init,
		       .cra_exit = spum_hash_cra_exit,
		       },
	 },
	{
	 .init = spum_hash_init,
	 .update = spum_hash_update,
	 .final = spum_hash_final,
	 .finup = spum_hash_finup,
	 .digest = spum_hash_digest,
	 .halg.digestsize = MD5_DIGEST_SIZE,
	 .halg.base = {
		       .cra_name = "md5",
		       .cra_driver_name = "spum-md5",
		       .cra_priority = 110,
		       .cra_flags = CRYPTO_ALG_TYPE_AHASH |
			CRYPTO_ALG_ASYNC | CRYPTO_TFM_REQ_MAY_BACKLOG,
		       .cra_blocksize = SPUM_HASH_BLOCK_SIZE,
		       .cra_ctxsize = sizeof(struct spum_hash_context),
		       .cra_alignmask = 0,
		       .cra_module = THIS_MODULE,
		       .cra_init = spum_hash_cra_init,
		       .cra_exit = spum_hash_cra_exit,
		       },
	 }
};

static void spum_dma_tasklet(unsigned long data)
{
	struct spum_hash_device *dd = (struct spum_hash_device *)data;
	struct spum_request_context *rctx = ahash_request_ctx(dd->req);
	int err = -EINPROGRESS;
	struct scatterlist *sg = dd->req->src;

	err = dma_stop_transfer(spum_dev->tx_dma_chan);
	if (err)
		pr_err("%s: Tx transfer stop failed %d\n", __func__, err);

	err = dma_stop_transfer(spum_dev->rx_dma_chan);
	if (err)
		pr_err("%s: Rx transfer stop failed %d\n", __func__, err);

	/* Free dma callback. */
	dma_free_callback(spum_dev->rx_dma_chan);
	dma_free_callback(spum_dev->tx_dma_chan);

	/* Unmap the payload and header buffer. */
	spum_unmap_sgl(dd, dd->sg, dd->dma_len, DMA_TO_DEVICE);
	spum_unmap_sgl(dd, dd->sg, dd->dma_len, DMA_FROM_DEVICE);
	dma_unmap_sg(dd->dev, &rctx->spum_in_cmd_hdr, 1, DMA_TO_DEVICE);
	dma_unmap_sg(dd->dev, &rctx->spum_out_cmd_hdr, 1, DMA_FROM_DEVICE);
	dma_unmap_sg(dd->dev, &rctx->spum_in_cmd_stat, 1, DMA_TO_DEVICE);
	dma_unmap_sg(dd->dev, &rctx->spum_out_cmd_stat, 1, DMA_FROM_DEVICE);

	/* Collect the intermediate hash value. */
	if (rctx->digestsize == SHA224_DIGEST_SIZE && rctx->op == OP_UPDATE)
		memcpy(&rctx->digest, (u8 *)sg_virt(&rctx->spum_out_cmd_stat),
			SHA224_INT_DIGEST_SIZE);
	else
		memcpy(&rctx->digest, (u8 *)sg_virt(&rctx->spum_out_cmd_stat),
		 rctx->digestsize);

	rctx->buff[rctx->index].bufcnt = 0;

	/* Restore the src sg list. */
	if ((rctx->rx_len == 0) && (rctx->flags&OP_UPDATE)
		&& (rctx->partial != 0)) {
		while (!sg_is_last(sg)) {
			if (sg == rctx->tsg)
				break;
			sg = sg_next(sg);
		}

		sg->length = rctx->tlen;
	}

	err = spum_hash_update_data(dd);

	if (err != -EINPROGRESS) {
		err = spum_hash_finish(dd, err);
		if (rctx->flags & FLAGS_FINUP && (err != -EINPROGRESS))
			err = spum_hash_final(dd->req);
		if (err != -EINPROGRESS) {
			if (spum_dev->spum_queue.qlen)
				spum_queue_task((unsigned long)&spum_dev);
		}
	}
}

static void spum_dma_callback(void *data, enum pl330_xfer_status status)
{
	struct spum_hash_device *dd;
	u32 *dma_chan = (u32 *)data;
	unsigned long flags;

	pr_debug("%s: dma_chain %d\n", __func__, *dma_chan);

	if (status != DMA_PL330_XFER_OK)
		pr_err("%s: DMA failed. err %d", __func__, status);

	list_for_each_entry(dd, &spum_drv.dev_list, list) {
		break;
	}

	if (*dma_chan == spum_dev->rx_dma_chan) {
		spin_lock_irqsave(&spum_dev->lock, flags);
		clear_bit(FLAGS_RBUSY, &spum_dev->flags);
		spin_unlock_irqrestore(&spum_dev->lock, flags);
	}

	if (*dma_chan == spum_dev->tx_dma_chan) {
		spin_lock_irqsave(&spum_dev->lock, flags);
		clear_bit(FLAGS_TBUSY, &spum_dev->flags);
		spin_unlock_irqrestore(&spum_dev->lock, flags);
	}

	if ((!test_bit(FLAGS_TBUSY, &spum_dev->flags))
	    && (!test_bit(FLAGS_RBUSY, &spum_dev->flags)))
		tasklet_hi_schedule(&dma_tasklet);
}

static int spum_dma_setup(struct spum_hash_device *dd)
{
	/* Register DMA callback */
	if (dma_register_callback(spum_dev->rx_dma_chan, spum_dma_callback,
				  &spum_dev->rx_dma_chan) != 0) {
		pr_err("%s: Rx dma_register_callback failed\n", __func__);
		goto err1;
	}
	if (dma_register_callback(spum_dev->tx_dma_chan, spum_dma_callback,
				  &spum_dev->tx_dma_chan) != 0) {
		pr_err("%s: Tx dma_register_callback failed\n", __func__);
		goto err2;
	}
	return 0;
err2:
	dma_free_callback(spum_dev->rx_dma_chan);
err1:
	return -EIO;
}

static int spum_hash_probe(struct platform_device *pdev)
{
	struct spum_hash_device *dd;
	struct resource *res;
	int ret = 0, i, j;

	pr_debug("%s: entry\n", __func__);

	dd = (struct spum_hash_device *)
	    kzalloc(sizeof(struct spum_hash_device), GFP_KERNEL);
	if (dd == NULL) {
		pr_err("%s:Failed to allocate spum structure.\n", __func__);
		return -ENOMEM;
	}

	spum_dev->hash_dev = dd;
	dd->dev = &pdev->dev;
	platform_set_drvdata(pdev, dd);

	/* Initializing the clock. */
	dd->spum_open_clk = clk_get(NULL, "spum_open");
	if (IS_ERR_OR_NULL(dd->spum_open_clk)) {
		pr_err("%s: Clock intialization failed.\n", __func__);
		kfree(dd);
		return -ENOMEM;
	}

	if (clk_set_rate(dd->spum_open_clk, FREQ_MHZ(156)))
		pr_debug("%s: Clock set failed!!!\n", __func__);

	/* Get the base address APB space. */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("%s: Invalid resource type.\n", __func__);
		ret = -EINVAL;
		goto exit;
	}

	dd->io_apb_base = (void __iomem *)HW_IO_PHYS_TO_VIRT(res->start);
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

	dd->io_axi_base = (void __iomem *)HW_IO_PHYS_TO_VIRT(res->start);
	if (!dd->io_axi_base) {
		pr_err("%s: Ioremap failed.\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	/* Initialize SPU-M block */
	clk_enable(dd->spum_open_clk);
	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	clk_disable(dd->spum_open_clk);

	INIT_LIST_HEAD(&dd->list);
	spin_lock(&spum_drv.lock);
	list_add_tail(&dd->list, &spum_drv.dev_list);
	spin_unlock(&spum_drv.lock);

	tasklet_init(&dma_tasklet, spum_dma_tasklet, (unsigned long)dd);

	for (i = 0; i < ARRAY_SIZE(spum_algo); i++) {
		ret = crypto_register_ahash(&spum_algo[i]);
		if (ret) {
			pr_err("%s: Crypto algorithm registration failed for %s\n",
			     __func__, spum_algo[i].halg.base.cra_name);
			goto exit_algos;
		} else
			pr_info("%s: SPUM %s algorithm registered..\n",
				__func__, spum_algo[i].halg.base.cra_name);
	}

	pr_info("%s: SPUM driver registered.\n", __func__);

	return ret;

exit_algos:
	for (j = 0; j < i; j++)
		crypto_unregister_ahash(&spum_algo[i]);
exit:
	kfree(dd);
	return ret;
}

static int spum_hash_remove(struct platform_device *pdev)
{
	struct spum_hash_device *dd;
	int i;

	pr_debug("%s: entry\n", __func__);

	dd = platform_get_drvdata(pdev);
	if (!dd)
		return -ENODEV;

	spin_lock(&spum_drv.lock);
	list_del(&dd->list);
	spin_unlock(&spum_drv.lock);

	for (i = 0; i < ARRAY_SIZE(spum_algo); i++)
		crypto_unregister_ahash(&spum_algo[i]);

	tasklet_kill(&dma_tasklet);

	clk_put(dd->spum_open_clk);
	kfree(dd);

	return 0;
}

static struct platform_driver spum_hash_driver = {
	.probe = spum_hash_probe,
	.remove = spum_hash_remove,
	.driver = {
		   .name = "brcm-spum",
		   .owner = THIS_MODULE,
		   },
};

static int __init brcm_hash_init(void)
{
	pr_info("%s: SPUM driver init.\n", __func__);
	return platform_driver_register(&spum_hash_driver);
}

static void __init brcm_hash_exit(void)
{
	pr_info("%s: SPUM driver exit.\n", __func__);
	platform_driver_unregister(&spum_hash_driver);
}

module_init(brcm_hash_init);
module_exit(brcm_hash_exit);

MODULE_LICENSE("GPL V2");
