/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* 	@file	drivers/crypto/brcm_spum_aes.c
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
#include <plat/mobcom_types.h>
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


#ifdef DEBUG
static void hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			8, 4,
			buf, len, false);
}
#endif

#define WORD_ALIGNED	__attribute__((aligned(sizeof(u32))))

#define	FLAGS_BUSY	1
#define SPUM_AES_QUEUE_LENGTH	10 

#define AES_XTS_MIN_KEY_SIZE	32
#define AES_XTS_MAX_KEY_SIZE	64
#define AES_KEYSIZE_512		64


static LIST_HEAD(spum_drv_list);
static DEFINE_SPINLOCK(spum_drv_lock);

struct spum_aes_device {
	spinlock_t		lock;
	u32			rx_dma_chan;
	u32			tx_dma_chan;
	ulong			flags;
	void __iomem		*io_apb_base;
	void __iomem		*io_axi_base;
	struct list_head	list;
	struct device		*dev;
	struct clk		*spum_open_clk;
	struct crypto_alg	*algo;
	struct tasklet_struct	done_task;
	struct tasklet_struct	queue_task;
	struct ablkcipher_request	*req;
	struct crypto_queue	queue;
};

struct spum_aes_context {
	u32	key_enc[AES_MAX_KEYLENGTH_U32];
	u32	key_len;
	struct spum_aes_device	*dd;
};

struct spum_request_context {
	u32	rx_len;
	u32	tx_len;
	u32	tx_offset;
	u8	spum_hdr[512] WORD_ALIGNED;
};

static int spum_aes_dma_init(struct spum_aes_device *dd);

static int spum_aes_cpu_xfer(struct spum_aes_device *dd, u32 *in_buff,
				u32 *out_buff, u32 length)
{
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	u32 i = 0,k = 0,status, out_fifo[32];

	pr_debug("%s entry\n",__func__);

	length /= sizeof(u32);
	if(!out_buff)
		out_buff = &out_fifo[0];

	spum_dma_init(dd->io_axi_base);

	while(i<length) {
		status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
		if(status & SPUM_AXI_FIFO_STAT_IFIFO_RDY_MASK) {
			writel(*in_buff++, dd->io_axi_base + SPUM_AXI_FIFO_IN_OFFSET );
			i++;
		}
	}

	mdelay(1);

	status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
	while((status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK)) {
		out_buff[k] = readl(dd->io_axi_base + SPUM_AXI_FIFO_OUT_OFFSET);
		k++;
		status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
	}

	rctx->rx_len -= length*sizeof(u32);
	rctx->tx_len -= k*sizeof(u32);
	rctx->tx_offset += k*sizeof(u32);

	return 0;
}

static int spum_aes_dma_xfer(struct spum_aes_device *dd)
{
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	struct ablkcipher_request *req = dd->req;
	u32 cfg_rx, cfg_tx, rx_fifo, tx_fifo, length;
	int err = -EINPROGRESS;

	pr_debug("%s: entry \n",__func__);

	if (!test_bit(FLAGS_BUSY, &dd->flags)) {
		pr_err("%s: Device is busy!!!",__func__);
		BUG();
	}

	cfg_rx = DMA_CFG_SRC_ADDR_INCREMENT | DMA_CFG_DST_ADDR_FIXED |
			DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_2;
	cfg_tx = DMA_CFG_SRC_ADDR_FIXED | DMA_CFG_DST_ADDR_INCREMENT |
			DMA_CFG_BURST_SIZE_4 | DMA_CFG_BURST_LENGTH_2;

	rx_fifo = (u32)HW_IO_VIRT_TO_PHYS(dd->io_axi_base) + SPUM_AXI_FIFO_IN_OFFSET;
	tx_fifo = (u32)HW_IO_VIRT_TO_PHYS(dd->io_axi_base) + SPUM_AXI_FIFO_OUT_OFFSET;

	if(!dma_map_sg(dd->dev, req->src, 1, DMA_TO_DEVICE)) {
		pr_err("%s: dma_map_sg() error\n",__func__);
		return -EINVAL;
	}

	if(!dma_map_sg(dd->dev, req->dst, 1, DMA_FROM_DEVICE)) {
		pr_err("%s: dma_map_sg() error\n",__func__);
		dma_unmap_sg(dd->dev, req->src, 1, DMA_TO_DEVICE);
		return -EINVAL;
	}

	length = min(rctx->rx_len, sg_dma_len(req->src));
	length = min(length, sg_dma_len(req->dst));
	
	/* Rx setup */
	if(dma_setup_transfer(dd->rx_dma_chan, sg_dma_address(req->src), rx_fifo, length,
			DMA_DIRECTION_MEM_TO_DEV_FLOW_CTRL_PERI, cfg_rx)) {
		pr_err("Rx dma_setup_transfer failed %d\n",err);
		err = -EIO;
		goto err_xfer;
	}

	/* Tx setup */
	if(dma_setup_transfer(dd->tx_dma_chan, tx_fifo, sg_dma_address(req->dst),length-64,/*workaround for DMA issue!!!*/
			DMA_DIRECTION_DEV_TO_MEM_FLOW_CTRL_PERI, cfg_tx)) {
		pr_err("Tx dma_setup_transfer failed %d\n",err);
		err = -EIO;
		goto err_xfer;
	}

	spum_dma_init(dd->io_axi_base);

	/* Rx start xfer */
	if(dma_start_transfer(dd->rx_dma_chan)) {
		pr_err("Rx dma transfer failed.\n");
		err = -EIO;
		goto err_xfer;
	}

	/* Tx start xfer */
	if(dma_start_transfer(dd->tx_dma_chan)) {
		pr_err("Tx dma transfer failed.\n");
		dma_stop_transfer(dd->rx_dma_chan);
		err = -EIO;
		goto err_xfer;
	}

	rctx->rx_len -= sg_dma_len(req->src);
	rctx->tx_len -= sg_dma_len(req->dst)-64;
	rctx->tx_offset += sg_dma_len(req->dst)-64;

	pr_debug("%s: exit %d\n",__func__,err);
	return err;

	err_xfer:
		dma_unmap_sg(dd->dev, req->src, 1, DMA_TO_DEVICE);
		dma_unmap_sg(dd->dev, req->dst, 1, DMA_FROM_DEVICE);
		return err;
}

static int spum_aes_finish_req(struct spum_aes_device *dd, int err)
{
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);

	pr_debug("%s: entry tx_len %d\n",__func__,rctx->tx_len);

	dd->req->base.complete(&dd->req->base, err);
	dd->req = NULL;
	rctx->tx_offset = 0;
	clear_bit(FLAGS_BUSY, &dd->flags);
	clk_disable(dd->spum_open_clk);

	return 0;
}

static int spum_aes_process_data(struct ablkcipher_request *req)
{
	struct spum_aes_context *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct spum_request_context *rctx = ablkcipher_request_ctx(req);
	int ret = 0;

	pr_debug("%s: entry\n",__func__);
	if(rctx->rx_len <= 64)
		spum_aes_cpu_xfer(aes_ctx->dd, sg_virt(req->src),
			(sg_virt(req->dst) + rctx->tx_offset), rctx->rx_len);

	/* Do dma_xfer(data) */
	if(rctx->rx_len)
		ret = spum_aes_dma_xfer(aes_ctx->dd);

	return ret;
}

static int spum_aes_process_req(struct ablkcipher_request *req)
{
	struct spum_aes_context *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct spum_request_context *rctx = ablkcipher_request_ctx(req);
	struct spum_aes_device *dd = aes_ctx->dd;
	struct spum_hw_context *spum_cmd = 
				(struct spum_hw_context *)rctx->spum_hdr;
	int ret = 0;

	pr_debug("%s : entry \n",__func__);

	/* Do cpu_xfer(hdr) */
	if((spum_cmd->crypto_mode&SPUM_CMD_CMODE_MASK) == SPUM_CRYPTO_MODE_XTS) {
		spum_aes_cpu_xfer(dd, (u32 *)rctx->spum_hdr, NULL, (rctx->rx_len - 16 - req->nbytes));
		spum_aes_cpu_xfer(dd, (u32 *)req->info,
			NULL, crypto_ablkcipher_ivsize(crypto_ablkcipher_reqtfm(req)));
	}
	else {
		spum_aes_cpu_xfer(dd, (u32 *)rctx->spum_hdr, NULL, (rctx->rx_len - req->nbytes));
	}
	rctx->tx_offset = 0;

	ret = spum_aes_process_data(req);

	pr_debug("%s : exit %d\n",__func__,ret);

	return ret;
}

static int spum_aes_handle_queue(struct spum_aes_device *dd,
				struct ablkcipher_request *req)
{
	struct crypto_async_request *async_req, *backlog;
	struct spum_request_context *rctx;
	unsigned long flags;
	int ret = 0, err;

	pr_debug("%s : entry \n",__func__);

	spin_lock_irqsave(&dd->lock, flags);
	if (req)
		ret = ablkcipher_enqueue_request(&dd->queue, req);
	if (test_bit(FLAGS_BUSY, &dd->flags)) {
		spin_unlock_irqrestore(&dd->lock, flags);
		return ret;
	}
	backlog = crypto_get_backlog(&dd->queue);
	async_req = crypto_dequeue_request(&dd->queue);
	if (async_req)
		set_bit(FLAGS_BUSY, &dd->flags);
	spin_unlock_irqrestore(&dd->lock, flags);

	if (!async_req)
		return ret;

	if (backlog)
		backlog->complete(backlog, -EINPROGRESS);

	req = ablkcipher_request_cast(async_req);
	rctx = ablkcipher_request_ctx(req);
	dd->req = req;
	rctx->tx_offset = 0;

	clk_enable(dd->spum_open_clk);
	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	spum_set_pkt_length(dd->io_axi_base, rctx->rx_len, rctx->tx_len);

	err = spum_aes_process_req(req);

	if(err != -EINPROGRESS) {
		spum_aes_finish_req(dd, err);
		tasklet_schedule(&dd->queue_task);
	}

	pr_debug("%s : exit\n",__func__);

	return ret;
}

static int spum_aes_crypt(struct ablkcipher_request *req, spum_crypto_algo algo,
				spum_crypto_mode mode, spum_crypto_op op)
{
	struct spum_aes_context *aes_ctx = crypto_tfm_ctx(req->base.tfm);
	struct spum_request_context *rctx = ablkcipher_request_ctx(req);
	struct spum_hw_context  spum_hw_aes_ctx;
	u32 cmd_len_bytes;
	char *iv     = "\x00\x00\x00\x00\x00\x00\x00\x00"
			"\x00\x00\x00\x00\x00\x00\x00\x00";

	pr_debug("%s : entry \n",__func__);

	memset((void*)&spum_hw_aes_ctx, 0, sizeof(spum_hw_aes_ctx));
	memset((void *)&rctx->spum_hdr[0], 0, ARRAY_SIZE(rctx->spum_hdr));

	spum_hw_aes_ctx.operation	=	op;
	spum_hw_aes_ctx.crypto_algo	=	algo;
	spum_hw_aes_ctx.crypto_mode	=	mode;
	spum_hw_aes_ctx.auth_algo	=	SPUM_AUTH_ALGO_NULL;
	spum_hw_aes_ctx.key_type	=	SPUM_KEY_OPEN;

	spum_hw_aes_ctx.data_attribute.crypto_length
					=	req->nbytes;
	spum_hw_aes_ctx.data_attribute.data_length
					=	(((req->nbytes + 3)
						/ sizeof(u32)) * sizeof(u32));
	spum_hw_aes_ctx.data_attribute.crypto_offset  = 0;

	spum_hw_aes_ctx.crypto_key = (void *)aes_ctx->key_enc;
	spum_hw_aes_ctx.crypto_key_len = aes_ctx->key_len/sizeof(u32);

	spum_hw_aes_ctx.init_vector_len	=	crypto_ablkcipher_ivsize(
						crypto_ablkcipher_reqtfm(req))/sizeof(u32);

	if(mode == SPUM_CRYPTO_MODE_XTS) {
		spum_hw_aes_ctx.init_vector =	iv;
		spum_hw_aes_ctx.data_attribute.crypto_length += 
				spum_hw_aes_ctx.init_vector_len*sizeof(u32);
		spum_hw_aes_ctx.data_attribute.data_length +=
				spum_hw_aes_ctx.init_vector_len*sizeof(u32);
		aes_ctx->key_len /= 2;
	}
	else {
		spum_hw_aes_ctx.init_vector =	req->info;
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

	cmd_len_bytes = spum_format_command(&spum_hw_aes_ctx, rctx->spum_hdr);

	rctx->rx_len = cmd_len_bytes + spum_hw_aes_ctx.data_attribute.data_length;
	rctx->tx_len = SPUM_OUTPUT_HEADER_LEN +
		spum_hw_aes_ctx.data_attribute.data_length +
		SPUM_OUTPUT_STATUS_LEN;

	pr_debug("%s : exit \n",__func__);

	return spum_aes_handle_queue(aes_ctx->dd, req);
}

static int spum_aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key, u32 key_len)
{
	struct spum_aes_context *aes_ctx = crypto_ablkcipher_ctx(tfm);
	struct ablkcipher_alg *cipher = crypto_ablkcipher_alg(tfm);
	const __le32 *key = (const __le32 *)in_key;
	int ret = 0;

        pr_debug("%s:Entry.%d\n",__FUNCTION__,key_len);

	if((key_len < cipher->min_keysize) || (key_len > cipher->max_keysize)) {
                crypto_ablkcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
                ret = -EINVAL;
        }
        else {
                memcpy((u32 *)(aes_ctx->key_enc), key, key_len);
                aes_ctx->key_len = key_len;
        }

        pr_debug("%s:Exit.\n",__FUNCTION__);
        return ret;
}

static int spum_aes_xts_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key, u32 key_len)
{
	struct spum_aes_context *aes_ctx = crypto_ablkcipher_ctx(tfm);
	struct ablkcipher_alg *cipher = crypto_ablkcipher_alg(tfm);
        const __le32 *key = (const __le32 *)in_key;
        int ret = 0;

	pr_debug("%s:Entry.%d\n",__FUNCTION__,key_len);

	if((key_len < cipher->min_keysize) || (key_len > cipher->max_keysize)) {
		crypto_ablkcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		ret = -EINVAL;
	}
	else {
		memcpy((u32 *)(aes_ctx->key_enc), ((u8 *)key + key_len/2), key_len/2);
		memcpy((u32 *)((u8 *)aes_ctx->key_enc+key_len/2), key, key_len/2);
		aes_ctx->key_len = key_len;
	}

	pr_debug("%s:Exit.\n",__FUNCTION__);
	return ret;
}

static int spum_aes_ecb_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_ECB, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_ecb_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_ECB, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_cbc_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_CBC, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_cbc_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_CBC, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_ctr_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_CTR, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_ctr_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_CTR, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_xts_encrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_XTS, SPUM_CRYPTO_ENCRYPTION);
}

static int spum_aes_xts_decrypt(struct ablkcipher_request *req)
{
	pr_debug("%s : entry \n",__func__);
	return spum_aes_crypt(req, SPUM_CRYPTO_ALGO_AES,
			SPUM_CRYPTO_MODE_XTS, SPUM_CRYPTO_DECRYPTION);
}

static int spum_aes_cra_init(struct crypto_tfm *tfm)
{
	struct spum_aes_device *dd = NULL;
	struct spum_aes_context *ctx = crypto_tfm_ctx(tfm);

	pr_debug("%s: entry \n",__func__);

	spin_lock_bh(&spum_drv_lock);
	list_for_each_entry(dd, &spum_drv_list, list) {
			break;
	}
	spin_unlock_bh(&spum_drv_lock);

	ctx->dd = dd;
	tfm->crt_ablkcipher.reqsize = sizeof(struct spum_request_context);

	if(spum_aes_dma_init(dd))
		return -1;

	return 0;
}

static void spum_aes_cra_exit(struct crypto_tfm *tfm)
{
	struct spum_aes_context *ctx = crypto_tfm_ctx(tfm);
	struct spum_aes_device *dd = ctx->dd;

	pr_debug("%s: exit\n",__func__);

	dma_free_callback(dd->rx_dma_chan);
	dma_free_callback(dd->tx_dma_chan);
	dma_free_chan(dd->rx_dma_chan);
	dma_free_chan(dd->tx_dma_chan);
	ctx->dd = NULL;
}

static struct crypto_alg spum_algos[] = {
{
	.cra_name		= "ecb(aes)",
	.cra_driver_name	= "spum-ecb-aes",
	.cra_priority		= 300,
	.cra_flags		= (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC),
	.cra_blocksize		= AES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct spum_aes_context),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_ablkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_init		= spum_aes_cra_init,
	.cra_exit		= spum_aes_cra_exit,
	.cra_u.ablkcipher = {
		.min_keysize	= AES_MIN_KEY_SIZE,
		.max_keysize	= AES_MAX_KEY_SIZE,
		.setkey		= spum_aes_setkey,
		.encrypt	= spum_aes_ecb_encrypt,
		.decrypt	= spum_aes_ecb_decrypt,
	}
},
{
	.cra_name		= "cbc(aes)",
	.cra_driver_name	= "spum-cbc-aes",
	.cra_priority		= 300,
	.cra_flags		= (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC),
	.cra_blocksize		= AES_BLOCK_SIZE,
	.cra_ctxsize		= sizeof(struct spum_aes_context),
	.cra_alignmask		= 0,
	.cra_type		= &crypto_ablkcipher_type,
	.cra_module		= THIS_MODULE,
	.cra_init		= spum_aes_cra_init,
	.cra_exit		= spum_aes_cra_exit,
	.cra_u.ablkcipher = {
		.min_keysize	= AES_MIN_KEY_SIZE,
		.max_keysize	= AES_MAX_KEY_SIZE,
		.ivsize		= AES_BLOCK_SIZE,
		.setkey		= spum_aes_setkey,
		.encrypt	= spum_aes_cbc_encrypt,
		.decrypt	= spum_aes_cbc_decrypt,
	}
},
{
	.cra_name               = "ctr(aes)",
	.cra_driver_name        = "spum-ctr-aes",
	.cra_priority           = 300,
	.cra_flags              = (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC),
	.cra_blocksize          = AES_BLOCK_SIZE,
	.cra_ctxsize            = sizeof(struct spum_aes_context),
	.cra_alignmask          = 0,
	.cra_type               = &crypto_ablkcipher_type,
	.cra_module             = THIS_MODULE,
	.cra_init               = spum_aes_cra_init,
	.cra_exit               = spum_aes_cra_exit,
	.cra_u.ablkcipher = {
		.min_keysize    = AES_MIN_KEY_SIZE,
		.max_keysize    = AES_MAX_KEY_SIZE,
		.ivsize         = AES_BLOCK_SIZE,
		.setkey         = spum_aes_setkey,
		.encrypt        = spum_aes_ctr_encrypt,
		.decrypt        = spum_aes_ctr_decrypt,
	}
},
{
	.cra_name               = "xts(aes)",
	.cra_driver_name        = "spum-xts-aes",
	.cra_priority           = 300,
	.cra_flags              = (CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC),
	.cra_blocksize          = AES_BLOCK_SIZE,
	.cra_ctxsize            = sizeof(struct spum_aes_context),
	.cra_alignmask          = 0,
	.cra_type               = &crypto_ablkcipher_type,
	.cra_module             = THIS_MODULE,
	.cra_init               = spum_aes_cra_init,
	.cra_exit               = spum_aes_cra_exit,
	.cra_u.ablkcipher = {
		.min_keysize    = AES_XTS_MIN_KEY_SIZE,
		.max_keysize    = AES_XTS_MAX_KEY_SIZE,
		.ivsize         = AES_BLOCK_SIZE,
		.setkey         = spum_aes_xts_setkey,
		.encrypt        = spum_aes_xts_encrypt,
		.decrypt        = spum_aes_xts_decrypt,
	}
},
};

static void spum_aes_done_task(unsigned long data)
{
	struct spum_aes_device *dd = (struct spum_aes_device *)data;
	struct spum_request_context *rctx = ablkcipher_request_ctx(dd->req);
	u32 status;
	int err;

	pr_debug("%s: entry\n",__func__);

	err = dma_stop_transfer(dd->rx_dma_chan);
	if(err) {
		pr_err("%s: Rx transfer stop failed %d\n",__func__,err);
	}

	err = dma_stop_transfer(dd->tx_dma_chan);
	if(err) {
		pr_err("%s: Tx transfer stop failed %d\n",__func__,err);
	}

	dma_unmap_sg(dd->dev, dd->req->src, 1, DMA_TO_DEVICE);
	dma_unmap_sg(dd->dev, dd->req->dst, 1, DMA_FROM_DEVICE);

	while(rctx->tx_len-4) {
		status = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);
		if((status & SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK)) {
			*(u32 *)(sg_virt(dd->req->dst)+rctx->tx_offset) =
					readl(dd->io_axi_base + SPUM_AXI_FIFO_OUT_OFFSET);
			rctx->tx_len -=4;
			rctx->tx_offset +=4;
		}

	}

	if(rctx->rx_len && !err) {
		err = spum_aes_process_data(dd->req);
		return;
	}

	spum_aes_finish_req(dd, err);

	spum_aes_handle_queue(dd, NULL);
}

static void spum_aes_queue_task(unsigned long data)
{
	struct spum_aes_device *dd = (struct spum_aes_device *)data;

	spum_aes_handle_queue(dd, NULL);
}

static void spum_aes_dma_callback(void *data, enum pl330_xfer_status status)
{
	struct spum_aes_device *dd = NULL;
	u32 *dma_chan = (u32 *)data, qstatus;

	pr_debug("%s: entry %d\n",__func__,*dma_chan);

	list_for_each_entry(dd, &spum_drv_list, list) {
			break;
	}

	qstatus = readl(dd->io_axi_base + SPUM_AXI_FIFO_STAT_OFFSET);

	if(*dma_chan == dd->rx_dma_chan) {
		return;
	}

	tasklet_schedule(&dd->done_task);
}

static int spum_aes_dma_init(struct spum_aes_device *dd)
{
	/* Aquire DMA channels */
	if (dma_request_chan(&dd->tx_dma_chan,
					"SPUM_OpenB") != 0) {
		pr_debug("%s: Tx dma_request_chan failed\n", __func__);
		return -1;
	}
	if (dma_request_chan(&dd->rx_dma_chan,
					"SPUM_OpenA") != 0) {
		pr_debug("%s: Rx dma_request_chan failed\n", __func__);
		goto err;
	}

	pr_debug("DMA channel aquired rx %d tx %d\n",dd->rx_dma_chan,dd->tx_dma_chan);

	/* Register DMA callback */
	if (dma_register_callback(dd->tx_dma_chan, spum_aes_dma_callback,
			      &dd->tx_dma_chan) != 0) {
		pr_debug("%s: Tx dma_register_callback failed\n", __func__);
		goto err1;
	}
	if (dma_register_callback(dd->rx_dma_chan, spum_aes_dma_callback,
			      &dd->rx_dma_chan) != 0) {
		pr_debug("%s: Rx dma_register_callback failed\n", __func__);
		goto err2;
	}
	return 0;
err2:
	dma_free_callback(dd->tx_dma_chan);
err1:
	dma_free_chan(dd->rx_dma_chan);
err:
	dma_free_chan(dd->tx_dma_chan);
	return -EIO;
}

static int spum_aes_probe(struct platform_device *pdev)
{
	struct spum_aes_device *dd;
	struct resource *res;
	int ret = 0,i,j;

	pr_debug("%s: entry\n",__func__);

	dd = (struct spum_aes_device *)
			kzalloc(sizeof(struct spum_aes_device), GFP_KERNEL);
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

	crypto_init_queue(&dd->queue, SPUM_AES_QUEUE_LENGTH);

	/* Initialize SPU-M block */
	clk_enable(dd->spum_open_clk);
	spum_init_device(dd->io_apb_base, dd->io_axi_base);
	clk_disable(dd->spum_open_clk);

	INIT_LIST_HEAD(&dd->list);
	spin_lock_init(&dd->lock);
	spin_lock(&spum_drv_lock);
	list_add_tail(&dd->list, &spum_drv_list);
	spin_unlock(&spum_drv_lock);

	tasklet_init(&dd->done_task, spum_aes_done_task, (unsigned long)dd);
	tasklet_init(&dd->queue_task, spum_aes_queue_task, (unsigned long)dd);

	for(i = 0; i < ARRAY_SIZE(spum_algos); i++) {
		ret = crypto_register_alg(&spum_algos[i]);
		if(ret) {
			pr_err("%s: Crypto algorithm registration failed for %s\n",
						__func__,spum_algos[i].cra_name);
			goto exit_algos;
		}
		else
			pr_info("%s: SPUM %s algorithm registered.\n",
						__func__,spum_algos[i].cra_name);
	} 

	pr_info("%s: SPUM AES driver registered.\n",__func__);

	return ret;

	exit_algos:
		for(j = 0; j < i; j++)
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

	spin_lock(&spum_drv_lock);
	list_del(&dd->list);
	spin_unlock(&spum_drv_lock);

	for(i = 0; i < ARRAY_SIZE(spum_algos); i++)
		crypto_unregister_alg(&spum_algos[i]);

	tasklet_kill(&dd->queue_task);
	tasklet_kill(&dd->done_task);
	clk_put(dd->spum_open_clk);

	kfree(dd);

	return 0;
}

static struct platform_driver spum_aes_driver = {
	.probe	=	spum_aes_probe,
	.remove	=	spum_aes_remove,
	.driver	=	{
		.name	=	"brcm-spum-aes",
		.owner	=	THIS_MODULE,
	},
};

static int __init spum_aes_init(void)
{
	pr_info("SPUM AES driver init.\n");

	return  platform_driver_register(&spum_aes_driver);
}

static void __exit spum_aes_exit(void)
{
platform_driver_unregister(&spum_aes_driver);
}

module_init(spum_aes_init);
module_exit(spum_aes_exit);

MODULE_LICENSE("GPL v2");
