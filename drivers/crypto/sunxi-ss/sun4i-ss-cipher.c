/*
 * sun4i-ss-cipher.c - hardware cryptographic accelerator for Allwinner A20 SoC
 *
 * Copyright (C) 2013-2015 Corentin LABBE <clabbe.montjoie@gmail.com>
 *
 * This file add support for AES cipher with 128,192,256 bits
 * keysize in CBC and ECB mode.
 * Add support also for DES and 3DES in CBC and ECB mode.
 *
 * You could find the datasheet in Documentation/arm/sunxi/README
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "sun4i-ss.h"

static int sun4i_ss_opti_poll(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_ss_ctx *ss = op->ss;
	unsigned int ivsize = crypto_skcipher_ivsize(tfm);
	struct sun4i_cipher_req_ctx *ctx = skcipher_request_ctx(areq);
	u32 mode = ctx->mode;
	/* when activating SS, the default FIFO space is SS_RX_DEFAULT(32) */
	u32 rx_cnt = SS_RX_DEFAULT;
	u32 tx_cnt = 0;
	u32 spaces;
	u32 v;
	int err = 0;
	unsigned int i;
	unsigned int ileft = areq->cryptlen;
	unsigned int oleft = areq->cryptlen;
	unsigned int todo;
	unsigned long pi = 0, po = 0; /* progress for in and out */
	bool miter_err;
	struct sg_mapping_iter mi, mo;
	unsigned int oi, oo; /* offset for in and out */
	unsigned long flags;

	if (!areq->cryptlen)
		return 0;

	if (!areq->iv) {
		dev_err_ratelimited(ss->dev, "ERROR: Empty IV\n");
		return -EINVAL;
	}

	if (!areq->src || !areq->dst) {
		dev_err_ratelimited(ss->dev, "ERROR: Some SGs are NULL\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&ss->slock, flags);

	for (i = 0; i < op->keylen / 4; i++)
		writesl(ss->base + SS_KEY0 + i * 4, &op->key[i], 1);

	if (areq->iv) {
		for (i = 0; i < 4 && i < ivsize / 4; i++) {
			v = *(u32 *)(areq->iv + i * 4);
			writesl(ss->base + SS_IV0 + i * 4, &v, 1);
		}
	}
	writel(mode, ss->base + SS_CTL);


	ileft = areq->cryptlen / 4;
	oleft = areq->cryptlen / 4;
	oi = 0;
	oo = 0;
	do {
		if (ileft) {
			sg_miter_start(&mi, areq->src, sg_nents(areq->src),
					SG_MITER_FROM_SG | SG_MITER_ATOMIC);
			if (pi)
				sg_miter_skip(&mi, pi);
			miter_err = sg_miter_next(&mi);
			if (!miter_err || !mi.addr) {
				dev_err_ratelimited(ss->dev, "ERROR: sg_miter return null\n");
				err = -EINVAL;
				goto release_ss;
			}
			todo = min(rx_cnt, ileft);
			todo = min_t(size_t, todo, (mi.length - oi) / 4);
			if (todo) {
				ileft -= todo;
				writesl(ss->base + SS_RXFIFO, mi.addr + oi, todo);
				oi += todo * 4;
			}
			if (oi == mi.length) {
				pi += mi.length;
				oi = 0;
			}
			sg_miter_stop(&mi);
		}

		spaces = readl(ss->base + SS_FCSR);
		rx_cnt = SS_RXFIFO_SPACES(spaces);
		tx_cnt = SS_TXFIFO_SPACES(spaces);

		sg_miter_start(&mo, areq->dst, sg_nents(areq->dst),
			       SG_MITER_TO_SG | SG_MITER_ATOMIC);
		if (po)
			sg_miter_skip(&mo, po);
		miter_err = sg_miter_next(&mo);
		if (!miter_err || !mo.addr) {
			dev_err_ratelimited(ss->dev, "ERROR: sg_miter return null\n");
			err = -EINVAL;
			goto release_ss;
		}
		todo = min(tx_cnt, oleft);
		todo = min_t(size_t, todo, (mo.length - oo) / 4);
		if (todo) {
			oleft -= todo;
			readsl(ss->base + SS_TXFIFO, mo.addr + oo, todo);
			oo += todo * 4;
		}
		if (oo == mo.length) {
			oo = 0;
			po += mo.length;
		}
		sg_miter_stop(&mo);
	} while (oleft);

	if (areq->iv) {
		for (i = 0; i < 4 && i < ivsize / 4; i++) {
			v = readl(ss->base + SS_IV0 + i * 4);
			*(u32 *)(areq->iv + i * 4) = v;
		}
	}

release_ss:
	writel(0, ss->base + SS_CTL);
	spin_unlock_irqrestore(&ss->slock, flags);
	return err;
}

/* Generic function that support SG with size not multiple of 4 */
static int sun4i_ss_cipher_poll(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_ss_ctx *ss = op->ss;
	int no_chunk = 1;
	struct scatterlist *in_sg = areq->src;
	struct scatterlist *out_sg = areq->dst;
	unsigned int ivsize = crypto_skcipher_ivsize(tfm);
	struct sun4i_cipher_req_ctx *ctx = skcipher_request_ctx(areq);
	u32 mode = ctx->mode;
	/* when activating SS, the default FIFO space is SS_RX_DEFAULT(32) */
	u32 rx_cnt = SS_RX_DEFAULT;
	u32 tx_cnt = 0;
	u32 v;
	u32 spaces;
	int err = 0;
	unsigned int i;
	unsigned int ileft = areq->cryptlen;
	unsigned int oleft = areq->cryptlen;
	unsigned int todo;
	struct sg_mapping_iter mi, mo;
	unsigned long pi = 0, po = 0; /* progress for in and out */
	bool miter_err;
	unsigned int oi, oo;	/* offset for in and out */
	char buf[4 * SS_RX_MAX];/* buffer for linearize SG src */
	char bufo[4 * SS_TX_MAX]; /* buffer for linearize SG dst */
	unsigned int ob = 0;	/* offset in buf */
	unsigned int obo = 0;	/* offset in bufo*/
	unsigned int obl = 0;	/* length of data in bufo */
	unsigned long flags;

	if (!areq->cryptlen)
		return 0;

	if (!areq->iv) {
		dev_err_ratelimited(ss->dev, "ERROR: Empty IV\n");
		return -EINVAL;
	}

	if (!areq->src || !areq->dst) {
		dev_err_ratelimited(ss->dev, "ERROR: Some SGs are NULL\n");
		return -EINVAL;
	}

	/*
	 * if we have only SGs with size multiple of 4,
	 * we can use the SS optimized function
	 */
	while (in_sg && no_chunk == 1) {
		if ((in_sg->length | in_sg->offset) & 3u)
			no_chunk = 0;
		in_sg = sg_next(in_sg);
	}
	while (out_sg && no_chunk == 1) {
		if ((out_sg->length | out_sg->offset) & 3u)
			no_chunk = 0;
		out_sg = sg_next(out_sg);
	}

	if (no_chunk == 1)
		return sun4i_ss_opti_poll(areq);

	spin_lock_irqsave(&ss->slock, flags);

	for (i = 0; i < op->keylen / 4; i++)
		writesl(ss->base + SS_KEY0 + i * 4, &op->key[i], 1);

	if (areq->iv) {
		for (i = 0; i < 4 && i < ivsize / 4; i++) {
			v = *(u32 *)(areq->iv + i * 4);
			writesl(ss->base + SS_IV0 + i * 4, &v, 1);
		}
	}
	writel(mode, ss->base + SS_CTL);

	ileft = areq->cryptlen;
	oleft = areq->cryptlen;
	oi = 0;
	oo = 0;

	while (oleft) {
		if (ileft) {
			sg_miter_start(&mi, areq->src, sg_nents(areq->src),
				       SG_MITER_FROM_SG | SG_MITER_ATOMIC);
			if (pi)
				sg_miter_skip(&mi, pi);
			miter_err = sg_miter_next(&mi);
			if (!miter_err || !mi.addr) {
				dev_err_ratelimited(ss->dev, "ERROR: sg_miter return null\n");
				err = -EINVAL;
				goto release_ss;
			}
			/*
			 * todo is the number of consecutive 4byte word that we
			 * can read from current SG
			 */
			todo = min(rx_cnt, ileft / 4);
			todo = min_t(size_t, todo, (mi.length - oi) / 4);
			if (todo && !ob) {
				writesl(ss->base + SS_RXFIFO, mi.addr + oi,
					todo);
				ileft -= todo * 4;
				oi += todo * 4;
			} else {
				/*
				 * not enough consecutive bytes, so we need to
				 * linearize in buf. todo is in bytes
				 * After that copy, if we have a multiple of 4
				 * we need to be able to write all buf in one
				 * pass, so it is why we min() with rx_cnt
				 */
				todo = min(rx_cnt * 4 - ob, ileft);
				todo = min_t(size_t, todo, mi.length - oi);
				memcpy(buf + ob, mi.addr + oi, todo);
				ileft -= todo;
				oi += todo;
				ob += todo;
				if (!(ob % 4)) {
					writesl(ss->base + SS_RXFIFO, buf,
						ob / 4);
					ob = 0;
				}
			}
			if (oi == mi.length) {
				pi += mi.length;
				oi = 0;
			}
			sg_miter_stop(&mi);
		}

		spaces = readl(ss->base + SS_FCSR);
		rx_cnt = SS_RXFIFO_SPACES(spaces);
		tx_cnt = SS_TXFIFO_SPACES(spaces);

		if (!tx_cnt)
			continue;
		sg_miter_start(&mo, areq->dst, sg_nents(areq->dst),
			       SG_MITER_TO_SG | SG_MITER_ATOMIC);
		if (po)
			sg_miter_skip(&mo, po);
		miter_err = sg_miter_next(&mo);
		if (!miter_err || !mo.addr) {
			dev_err_ratelimited(ss->dev, "ERROR: sg_miter return null\n");
			err = -EINVAL;
			goto release_ss;
		}
		/* todo in 4bytes word */
		todo = min(tx_cnt, oleft / 4);
		todo = min_t(size_t, todo, (mo.length - oo) / 4);

		if (todo) {
			readsl(ss->base + SS_TXFIFO, mo.addr + oo, todo);
			oleft -= todo * 4;
			oo += todo * 4;
			if (oo == mo.length) {
				po += mo.length;
				oo = 0;
			}
		} else {
			/*
			 * read obl bytes in bufo, we read at maximum for
			 * emptying the device
			 */
			readsl(ss->base + SS_TXFIFO, bufo, tx_cnt);
			obl = tx_cnt * 4;
			obo = 0;
			do {
				/*
				 * how many bytes we can copy ?
				 * no more than remaining SG size
				 * no more than remaining buffer
				 * no need to test against oleft
				 */
				todo = min_t(size_t,
					     mo.length - oo, obl - obo);
				memcpy(mo.addr + oo, bufo + obo, todo);
				oleft -= todo;
				obo += todo;
				oo += todo;
				if (oo == mo.length) {
					po += mo.length;
					sg_miter_next(&mo);
					oo = 0;
				}
			} while (obo < obl);
			/* bufo must be fully used here */
		}
		sg_miter_stop(&mo);
	}
	if (areq->iv) {
		for (i = 0; i < 4 && i < ivsize / 4; i++) {
			v = readl(ss->base + SS_IV0 + i * 4);
			*(u32 *)(areq->iv + i * 4) = v;
		}
	}

release_ss:
	writel(0, ss->base + SS_CTL);
	spin_unlock_irqrestore(&ss->slock, flags);

	return err;
}

/* CBC AES */
int sun4i_ss_cbc_aes_encrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_AES | SS_CBC | SS_ENABLED | SS_ENCRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

int sun4i_ss_cbc_aes_decrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_AES | SS_CBC | SS_ENABLED | SS_DECRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

/* ECB AES */
int sun4i_ss_ecb_aes_encrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_AES | SS_ECB | SS_ENABLED | SS_ENCRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

int sun4i_ss_ecb_aes_decrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_AES | SS_ECB | SS_ENABLED | SS_DECRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

/* CBC DES */
int sun4i_ss_cbc_des_encrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_DES | SS_CBC | SS_ENABLED | SS_ENCRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

int sun4i_ss_cbc_des_decrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_DES | SS_CBC | SS_ENABLED | SS_DECRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

/* ECB DES */
int sun4i_ss_ecb_des_encrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_DES | SS_ECB | SS_ENABLED | SS_ENCRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

int sun4i_ss_ecb_des_decrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_DES | SS_ECB | SS_ENABLED | SS_DECRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

/* CBC 3DES */
int sun4i_ss_cbc_des3_encrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_3DES | SS_CBC | SS_ENABLED | SS_ENCRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

int sun4i_ss_cbc_des3_decrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_3DES | SS_CBC | SS_ENABLED | SS_DECRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

/* ECB 3DES */
int sun4i_ss_ecb_des3_encrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_3DES | SS_ECB | SS_ENABLED | SS_ENCRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

int sun4i_ss_ecb_des3_decrypt(struct skcipher_request *areq)
{
	struct crypto_skcipher *tfm = crypto_skcipher_reqtfm(areq);
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_cipher_req_ctx *rctx = skcipher_request_ctx(areq);

	rctx->mode = SS_OP_3DES | SS_ECB | SS_ENABLED | SS_DECRYPTION |
		op->keymode;
	return sun4i_ss_cipher_poll(areq);
}

int sun4i_ss_cipher_init(struct crypto_tfm *tfm)
{
	struct sun4i_tfm_ctx *op = crypto_tfm_ctx(tfm);
	struct sun4i_ss_alg_template *algt;

	memset(op, 0, sizeof(struct sun4i_tfm_ctx));

	algt = container_of(tfm->__crt_alg, struct sun4i_ss_alg_template,
			    alg.crypto.base);
	op->ss = algt->ss;

	crypto_skcipher_set_reqsize(__crypto_skcipher_cast(tfm),
				    sizeof(struct sun4i_cipher_req_ctx));

	return 0;
}

/* check and set the AES key, prepare the mode to be used */
int sun4i_ss_aes_setkey(struct crypto_skcipher *tfm, const u8 *key,
			unsigned int keylen)
{
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_ss_ctx *ss = op->ss;

	switch (keylen) {
	case 128 / 8:
		op->keymode = SS_AES_128BITS;
		break;
	case 192 / 8:
		op->keymode = SS_AES_192BITS;
		break;
	case 256 / 8:
		op->keymode = SS_AES_256BITS;
		break;
	default:
		dev_err(ss->dev, "ERROR: Invalid keylen %u\n", keylen);
		crypto_skcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	op->keylen = keylen;
	memcpy(op->key, key, keylen);
	return 0;
}

/* check and set the DES key, prepare the mode to be used */
int sun4i_ss_des_setkey(struct crypto_skcipher *tfm, const u8 *key,
			unsigned int keylen)
{
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_ss_ctx *ss = op->ss;
	u32 flags;
	u32 tmp[DES_EXPKEY_WORDS];
	int ret;

	if (unlikely(keylen != DES_KEY_SIZE)) {
		dev_err(ss->dev, "Invalid keylen %u\n", keylen);
		crypto_skcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	flags = crypto_skcipher_get_flags(tfm);

	ret = des_ekey(tmp, key);
	if (unlikely(!ret) && (flags & CRYPTO_TFM_REQ_WEAK_KEY)) {
		crypto_skcipher_set_flags(tfm, CRYPTO_TFM_RES_WEAK_KEY);
		dev_dbg(ss->dev, "Weak key %u\n", keylen);
		return -EINVAL;
	}

	op->keylen = keylen;
	memcpy(op->key, key, keylen);
	return 0;
}

/* check and set the 3DES key, prepare the mode to be used */
int sun4i_ss_des3_setkey(struct crypto_skcipher *tfm, const u8 *key,
			 unsigned int keylen)
{
	struct sun4i_tfm_ctx *op = crypto_skcipher_ctx(tfm);
	struct sun4i_ss_ctx *ss = op->ss;

	if (unlikely(keylen != 3 * DES_KEY_SIZE)) {
		dev_err(ss->dev, "Invalid keylen %u\n", keylen);
		crypto_skcipher_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	op->keylen = keylen;
	memcpy(op->key, key, keylen);
	return 0;
}
