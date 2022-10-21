/*
 * Copyright (C) STMicroelectronics SA 2017
 * Author: Fabien Dessenne <fabien.dessenne@st.com>
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/bitrev.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <crypto/internal/hash.h>

#include <asm/unaligned.h>

#define DRIVER_NAME             "stm32-crc32"
#define CHKSUM_DIGEST_SIZE      4
#define CHKSUM_BLOCK_SIZE       1

/* Registers */
#define CRC_DR                  0x00000000
#define CRC_CR                  0x00000008
#define CRC_INIT                0x00000010
#define CRC_POL                 0x00000014

/* Registers values */
#define CRC_CR_RESET            BIT(0)
#define CRC_CR_REVERSE          (BIT(7) | BIT(6) | BIT(5))
#define CRC_INIT_DEFAULT        0xFFFFFFFF

/* Polynomial reversed */
#define POLY_CRC32              0xEDB88320
#define POLY_CRC32C             0x82F63B78

struct stm32_crc {
	struct list_head list;
	struct device    *dev;
	void __iomem     *regs;
	struct clk       *clk;
	u8               pending_data[sizeof(u32)];
	size_t           nb_pending_bytes;
};

struct stm32_crc_list {
	struct list_head dev_list;
	spinlock_t       lock; /* protect dev_list */
};

static struct stm32_crc_list crc_list = {
	.dev_list = LIST_HEAD_INIT(crc_list.dev_list),
	.lock     = __SPIN_LOCK_UNLOCKED(crc_list.lock),
};

struct stm32_crc_ctx {
	u32 key;
	u32 poly;
};

struct stm32_crc_desc_ctx {
	u32    partial; /* crc32c: partial in first 4 bytes of that struct */
	struct stm32_crc *crc;
};

static int stm32_crc32_cra_init(struct crypto_tfm *tfm)
{
	struct stm32_crc_ctx *mctx = crypto_tfm_ctx(tfm);

	mctx->key = CRC_INIT_DEFAULT;
	mctx->poly = POLY_CRC32;
	return 0;
}

static int stm32_crc32c_cra_init(struct crypto_tfm *tfm)
{
	struct stm32_crc_ctx *mctx = crypto_tfm_ctx(tfm);

	mctx->key = CRC_INIT_DEFAULT;
	mctx->poly = POLY_CRC32C;
	return 0;
}

static int stm32_crc_setkey(struct crypto_shash *tfm, const u8 *key,
			    unsigned int keylen)
{
	struct stm32_crc_ctx *mctx = crypto_shash_ctx(tfm);

	if (keylen != sizeof(u32)) {
		crypto_shash_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	mctx->key = get_unaligned_le32(key);
	return 0;
}

static int stm32_crc_init(struct shash_desc *desc)
{
	struct stm32_crc_desc_ctx *ctx = shash_desc_ctx(desc);
	struct stm32_crc_ctx *mctx = crypto_shash_ctx(desc->tfm);
	struct stm32_crc *crc;

	spin_lock_bh(&crc_list.lock);
	list_for_each_entry(crc, &crc_list.dev_list, list) {
		ctx->crc = crc;
		break;
	}
	spin_unlock_bh(&crc_list.lock);

	/* Reset, set key, poly and configure in bit reverse mode */
	writel_relaxed(bitrev32(mctx->key), ctx->crc->regs + CRC_INIT);
	writel_relaxed(bitrev32(mctx->poly), ctx->crc->regs + CRC_POL);
	writel_relaxed(CRC_CR_RESET | CRC_CR_REVERSE, ctx->crc->regs + CRC_CR);

	/* Store partial result */
	ctx->partial = readl_relaxed(ctx->crc->regs + CRC_DR);
	ctx->crc->nb_pending_bytes = 0;

	return 0;
}

static int stm32_crc_update(struct shash_desc *desc, const u8 *d8,
			    unsigned int length)
{
	struct stm32_crc_desc_ctx *ctx = shash_desc_ctx(desc);
	struct stm32_crc *crc = ctx->crc;
	u32 *d32;
	unsigned int i;

	if (unlikely(crc->nb_pending_bytes)) {
		while (crc->nb_pending_bytes != sizeof(u32) && length) {
			/* Fill in pending data */
			crc->pending_data[crc->nb_pending_bytes++] = *(d8++);
			length--;
		}

		if (crc->nb_pending_bytes == sizeof(u32)) {
			/* Process completed pending data */
			writel_relaxed(*(u32 *)crc->pending_data,
				       crc->regs + CRC_DR);
			crc->nb_pending_bytes = 0;
		}
	}

	d32 = (u32 *)d8;
	for (i = 0; i < length >> 2; i++)
		/* Process 32 bits data */
		writel_relaxed(*(d32++), crc->regs + CRC_DR);

	/* Store partial result */
	ctx->partial = readl_relaxed(crc->regs + CRC_DR);

	/* Check for pending data (non 32 bits) */
	length &= 3;
	if (likely(!length))
		return 0;

	if ((crc->nb_pending_bytes + length) >= sizeof(u32)) {
		/* Shall not happen */
		dev_err(crc->dev, "Pending data overflow\n");
		return -EINVAL;
	}

	d8 = (const u8 *)d32;
	for (i = 0; i < length; i++)
		/* Store pending data */
		crc->pending_data[crc->nb_pending_bytes++] = *(d8++);

	return 0;
}

static int stm32_crc_final(struct shash_desc *desc, u8 *out)
{
	struct stm32_crc_desc_ctx *ctx = shash_desc_ctx(desc);
	struct stm32_crc_ctx *mctx = crypto_shash_ctx(desc->tfm);

	/* Send computed CRC */
	put_unaligned_le32(mctx->poly == POLY_CRC32C ?
			   ~ctx->partial : ctx->partial, out);

	return 0;
}

static int stm32_crc_finup(struct shash_desc *desc, const u8 *data,
			   unsigned int length, u8 *out)
{
	return stm32_crc_update(desc, data, length) ?:
	       stm32_crc_final(desc, out);
}

static int stm32_crc_digest(struct shash_desc *desc, const u8 *data,
			    unsigned int length, u8 *out)
{
	return stm32_crc_init(desc) ?: stm32_crc_finup(desc, data, length, out);
}

static struct shash_alg algs[] = {
	/* CRC-32 */
	{
		.setkey         = stm32_crc_setkey,
		.init           = stm32_crc_init,
		.update         = stm32_crc_update,
		.final          = stm32_crc_final,
		.finup          = stm32_crc_finup,
		.digest         = stm32_crc_digest,
		.descsize       = sizeof(struct stm32_crc_desc_ctx),
		.digestsize     = CHKSUM_DIGEST_SIZE,
		.base           = {
			.cra_name               = "crc32",
			.cra_driver_name        = "stm32-crc32-crc32",
			.cra_priority           = 200,
			.cra_flags		= CRYPTO_ALG_OPTIONAL_KEY,
			.cra_blocksize          = CHKSUM_BLOCK_SIZE,
			.cra_alignmask          = 3,
			.cra_ctxsize            = sizeof(struct stm32_crc_ctx),
			.cra_module             = THIS_MODULE,
			.cra_init               = stm32_crc32_cra_init,
		}
	},
	/* CRC-32Castagnoli */
	{
		.setkey         = stm32_crc_setkey,
		.init           = stm32_crc_init,
		.update         = stm32_crc_update,
		.final          = stm32_crc_final,
		.finup          = stm32_crc_finup,
		.digest         = stm32_crc_digest,
		.descsize       = sizeof(struct stm32_crc_desc_ctx),
		.digestsize     = CHKSUM_DIGEST_SIZE,
		.base           = {
			.cra_name               = "crc32c",
			.cra_driver_name        = "stm32-crc32-crc32c",
			.cra_priority           = 200,
			.cra_flags		= CRYPTO_ALG_OPTIONAL_KEY,
			.cra_blocksize          = CHKSUM_BLOCK_SIZE,
			.cra_alignmask          = 3,
			.cra_ctxsize            = sizeof(struct stm32_crc_ctx),
			.cra_module             = THIS_MODULE,
			.cra_init               = stm32_crc32c_cra_init,
		}
	}
};

static int stm32_crc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct stm32_crc *crc;
	struct resource *res;
	int ret;

	crc = devm_kzalloc(dev, sizeof(*crc), GFP_KERNEL);
	if (!crc)
		return -ENOMEM;

	crc->dev = dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	crc->regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(crc->regs)) {
		dev_err(dev, "Cannot map CRC IO\n");
		return PTR_ERR(crc->regs);
	}

	crc->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(crc->clk)) {
		dev_err(dev, "Could not get clock\n");
		return PTR_ERR(crc->clk);
	}

	ret = clk_prepare_enable(crc->clk);
	if (ret) {
		dev_err(crc->dev, "Failed to enable clock\n");
		return ret;
	}

	platform_set_drvdata(pdev, crc);

	spin_lock(&crc_list.lock);
	list_add(&crc->list, &crc_list.dev_list);
	spin_unlock(&crc_list.lock);

	ret = crypto_register_shashes(algs, ARRAY_SIZE(algs));
	if (ret) {
		dev_err(dev, "Failed to register\n");
		clk_disable_unprepare(crc->clk);
		return ret;
	}

	dev_info(dev, "Initialized\n");

	return 0;
}

static int stm32_crc_remove(struct platform_device *pdev)
{
	struct stm32_crc *crc = platform_get_drvdata(pdev);

	spin_lock(&crc_list.lock);
	list_del(&crc->list);
	spin_unlock(&crc_list.lock);

	crypto_unregister_shashes(algs, ARRAY_SIZE(algs));

	clk_disable_unprepare(crc->clk);

	return 0;
}

static const struct of_device_id stm32_dt_ids[] = {
	{ .compatible = "st,stm32f7-crc", },
	{},
};
MODULE_DEVICE_TABLE(of, stm32_dt_ids);

static struct platform_driver stm32_crc_driver = {
	.probe  = stm32_crc_probe,
	.remove = stm32_crc_remove,
	.driver = {
		.name           = DRIVER_NAME,
		.of_match_table = stm32_dt_ids,
	},
};

module_platform_driver(stm32_crc_driver);

MODULE_AUTHOR("Fabien Dessenne <fabien.dessenne@st.com>");
MODULE_DESCRIPTION("STMicrolectronics STM32 CRC32 hardware driver");
MODULE_LICENSE("GPL");
