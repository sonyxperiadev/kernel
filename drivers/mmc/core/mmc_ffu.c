/*
 * * ffu.c
 *
 * Copyright 2007-2008 Pierre Ossman
 *
 * Modified by SanDisk Corp., Copyright (c) 2013 SanDisk Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program includes bug.h, card.h, host.h, mmc.h, scatterlist.h,
 * slab.h, ffu.h & swap.h header files
 * The original, unmodified version of this program a the mmc_test.c
 * file a is obtained under the GPL v2.0 license that is available via
 * http://www.gnu.org/licenses/,
 * or http://www.opensource.org/licenses/gpl-2.0.php
*/
/*
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are Copyright (c) 2015 Sony Mobile Communications Inc,
 * and licensed under the license of the file.
 */

#include <linux/bug.h>
#include <linux/errno.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/firmware.h>

#define RESULT_OK		0
#define RESULT_FAIL		1
#define RESULT_UNSUP_HOST	2

#define MMC_FFU_EXT_CSD_CMD_SET_NORMAL	0

/**
 * struct mmc_ffu_pages - pages allocated by 'alloc_pages()'.
 * @page: first page in the allocation
 * @order: order of the number of pages allocated
 */
struct mmc_ffu_pages {
	struct page *page;
	unsigned int order;
};

/**
 * struct mmc_ffu_mem - allocated memory.
 * @arr: array of allocations
 * @cnt: number of allocations
 */
struct mmc_ffu_mem {
	struct mmc_ffu_pages *arr;
	unsigned int cnt;
};

struct mmc_ffu_area {
	unsigned long max_sz;
	unsigned int max_tfr;
	unsigned int max_segs;
	unsigned int max_seg_sz;
	unsigned int blocks;
	unsigned int sg_len;
	struct mmc_ffu_mem mem;
	struct sg_table sgtable;
};

static void mmc_ffu_prepare_mrq(struct mmc_card *card,
	struct mmc_request *mrq, struct scatterlist *sg, unsigned int sg_len,
	u32 arg, unsigned int blocks, unsigned int blksz, int write)
{
	BUG_ON(!mrq || !mrq->cmd || !mrq->data || !mrq->stop);

	if (blocks > 1) {
		mrq->cmd->opcode = write ?
			MMC_WRITE_MULTIPLE_BLOCK : MMC_READ_MULTIPLE_BLOCK;
	} else {
		mrq->cmd->opcode = write ? MMC_WRITE_BLOCK :
			MMC_READ_SINGLE_BLOCK;
	}

	mrq->cmd->arg = arg;
	if (!mmc_card_blockaddr(card))
		mrq->cmd->arg <<= 9;

	mrq->cmd->flags = MMC_RSP_R1 | MMC_CMD_ADTC;

	if (blocks == 1) {
		mrq->stop = NULL;
	} else {
		mrq->stop->opcode = MMC_STOP_TRANSMISSION;
		mrq->stop->arg = 0;
		mrq->stop->flags = MMC_RSP_R1B | MMC_CMD_AC;
	}

	mrq->data->blksz = blksz;
	mrq->data->blocks = blocks;
	mrq->data->flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
	mrq->data->sg = sg;
	mrq->data->sg_len = sg_len;

	mmc_set_data_timeout(mrq->data, card);
}

/*
 * Checks that a normal transfer didn't have any errors
 */
static int mmc_ffu_check_result(struct mmc_request *mrq)
{
	int ret;

	BUG_ON(!mrq || !mrq->cmd || !mrq->data);

	ret = 0;

	if (!ret && mrq->cmd->error)
		ret = mrq->cmd->error;
	if (!ret && mrq->data->error)
		ret = mrq->data->error;
	if (!ret && mrq->stop && mrq->stop->error)
		ret = mrq->stop->error;
	if (!ret && mrq->data->bytes_xfered !=
		mrq->data->blocks * mrq->data->blksz)
		ret = RESULT_FAIL;

	if (ret == -EINVAL)
		ret = RESULT_UNSUP_HOST;

	return ret;
}

static int mmc_ffu_busy(struct mmc_command *cmd)
{
	return !(cmd->resp[0] & R1_READY_FOR_DATA) ||
		(R1_CURRENT_STATE(cmd->resp[0]) == R1_STATE_PRG);
}

static int mmc_ffu_wait_busy(struct mmc_card *card)
{
	int ret, busy = 0;
	struct mmc_command cmd = {0};

	memset(&cmd, 0, sizeof(struct mmc_command));
	cmd.opcode = MMC_SEND_STATUS;
	cmd.arg = card->rca << 16;
	cmd.flags = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_AC;

	do {
		ret = mmc_wait_for_cmd(card->host, &cmd, 0);
		if (ret)
			break;

		if (!busy && mmc_ffu_busy(&cmd)) {
			busy = 1;
			if (card->host->caps & MMC_CAP_WAIT_WHILE_BUSY) {
				pr_warn("%s: Warning: Host did not "
					"wait for busy state to end.\n",
					mmc_hostname(card->host));
			}
		}

	} while (mmc_ffu_busy(&cmd));

	return ret;
}

/*
 * transfer with certain parameters
 */
static int mmc_ffu_simple_transfer(struct mmc_card *card,
	struct scatterlist *sg, unsigned int sg_len, u32 arg,
	 unsigned int blocks, unsigned int blksz, int write)
{
	struct mmc_request mrq = {0};
	struct mmc_command cmd = {0};
	struct mmc_command stop = {0};
	struct mmc_data data = {0};

	mrq.cmd = &cmd;
	mrq.data = &data;
	mrq.stop = &stop;

	mmc_ffu_prepare_mrq(card, &mrq, sg, sg_len, arg, blocks, blksz,
		write);

	mmc_wait_for_req(card->host, &mrq);

	mmc_ffu_wait_busy(card);

	return mmc_ffu_check_result(&mrq);
}

/*
 * Map memory into a scatterlist.
 */
static unsigned int mmc_ffu_map_sg(struct mmc_ffu_mem *mem, int size,
	struct scatterlist *sglist)
{
	struct scatterlist *sg = sglist;
	unsigned int i;
	unsigned long sz = size;
	unsigned int sctr_len = 0;
	unsigned long len;

	for (i = 0; i < mem->cnt && sz; i++, sz -= len) {
		len = PAGE_SIZE << mem->arr[i].order;

		if (len > sz) {
			len = sz;
			sz = 0;
		}

		sg_set_page(sg, mem->arr[i].page, len, 0);
		sg = sg_next(sg);
		sctr_len++;
	}

	return sctr_len;
}

static void mmc_ffu_free_mem(struct mmc_ffu_mem *mem)
{
	if (!mem)
		return;

	while (mem->cnt) {
		mem->cnt--;
		__free_pages(mem->arr[mem->cnt].page,
				mem->arr[mem->cnt].order);
	}

	kfree(mem->arr);
}

/*
 * Cleanup struct mmc_ffu_area.
 */
static int mmc_ffu_area_cleanup(struct mmc_ffu_area *area)
{
	sg_free_table(&area->sgtable);
	mmc_ffu_free_mem(&area->mem);
	return 0;
}

/*
 * Allocate a lot of memory, preferably max_sz but at least min_sz. In case
 * there isn't much memory do not exceed 1/16th total low mem pages. Also do
 * not exceed a maximum number of segments and try not to make segments much
 * bigger than maximum segment size.
 */
static int mmc_ffu_alloc_mem(struct mmc_ffu_area *area, unsigned long min_sz)
{
	unsigned long max_page_cnt = DIV_ROUND_UP(area->max_tfr, PAGE_SIZE);
	unsigned long min_page_cnt = DIV_ROUND_UP(min_sz, PAGE_SIZE);
	unsigned long max_seg_page_cnt =
	DIV_ROUND_UP(area->max_seg_sz, PAGE_SIZE);
	unsigned long page_cnt = 0;
	/* we divide by 16 to ensure we will not allocate a big amount
	 * of unnecessary pages */
	unsigned long limit = nr_free_buffer_pages() >> 4;

	gfp_t flags = GFP_KERNEL | GFP_DMA | __GFP_NOWARN | __GFP_NORETRY;

	if (max_page_cnt > limit) {
		max_page_cnt = limit;
		area->max_tfr = max_page_cnt * PAGE_SIZE;
	}

	if (min_page_cnt > max_page_cnt)
		min_page_cnt = max_page_cnt;

	if (area->max_segs * max_seg_page_cnt > max_page_cnt)
		area->max_segs = DIV_ROUND_UP(max_page_cnt, max_seg_page_cnt);

	area->mem.arr = kzalloc(sizeof(struct mmc_ffu_pages) * area->max_segs,
						GFP_KERNEL);
	area->mem.cnt = 0;
	if (!area->mem.arr)
		goto out_free;

	while (max_page_cnt) {
		struct page *page;
		unsigned int order;

		order = get_order(max_seg_page_cnt << PAGE_SHIFT);

		do {
			page = alloc_pages(flags, order);
		} while (!page && order--);

		if (!page)
			goto out_free;

		area->mem.arr[area->mem.cnt].page = page;
		area->mem.arr[area->mem.cnt].order = order;
		area->mem.cnt++;
		page_cnt += 1UL << order;
		if (max_page_cnt <= (1UL << order))
			break;
		max_page_cnt -= 1UL << order;
	}

	if (page_cnt < min_page_cnt)
		goto out_free;

	return 0;

out_free:
	mmc_ffu_free_mem(&area->mem);
	return -ENOMEM;
}

/*
 * Initialize an area for data transfers.
 * Copy the data to the allocated pages.
 */
static int mmc_ffu_area_init(struct mmc_ffu_area *area, struct mmc_card *card,
	const u8 *data)
{
	int ret;
	int i;
	unsigned int length = 0, page_length;

	ret = mmc_ffu_alloc_mem(area, 1);
	for (i = 0; i < area->mem.cnt; i++) {
		if (length > area->max_tfr) {
			ret = -EINVAL;
			goto out_free;
		}
		page_length = PAGE_SIZE << area->mem.arr[i].order;
		memcpy(page_address(area->mem.arr[i].page), data + length,
		min(area->max_tfr - length, page_length));
		length += page_length;
	}

	ret = sg_alloc_table(&area->sgtable, area->mem.cnt, GFP_KERNEL);
	if (ret)
		goto out_free;

	area->sg_len = mmc_ffu_map_sg(&area->mem, area->max_tfr,
	area->sgtable.sgl);


	return 0;

out_free:
	mmc_ffu_free_mem(&area->mem);
	return ret;
}

static int mmc_ffu_write(struct mmc_card *card, const u8 *src, u32 arg,
	int size)
{
	int rc;
	struct mmc_ffu_area area = {0};
	int block_size = card->ext_csd.data_sector_size;

	area.max_segs = card->host->max_segs;
	area.max_seg_sz = card->host->max_seg_size & ~(block_size - 1);

	do {
		area.max_tfr = size;
		if (area.max_tfr >> 9 > card->host->max_blk_count)
			area.max_tfr = card->host->max_blk_count << 9;
		if (area.max_tfr > card->host->max_req_size)
			area.max_tfr = card->host->max_req_size;
		if (DIV_ROUND_UP(area.max_tfr, area.max_seg_sz) > area.max_segs)
			area.max_tfr = area.max_segs * area.max_seg_sz;

		rc = mmc_ffu_area_init(&area, card, src);
		if (rc != 0)
			goto exit;

		rc = mmc_ffu_simple_transfer(card, area.sgtable.sgl,
					area.sg_len, arg,
					area.max_tfr / block_size,
					block_size, 1);
		mmc_ffu_area_cleanup(&area);
		if (rc != 0) {
			pr_err("%s mmc_ffu_simple_transfer %d\n", __func__, rc);
			goto exit;
		}
		src += area.max_tfr;
		size -= area.max_tfr;

	} while (size > 0);

exit:
	return rc;
}

static int mmc_ffu_switch_mode(struct mmc_card *card , int mode)
{
	int err = 0;
	int offset;

	switch (mode) {
	case MMC_FFU_MODE_SET:
	case MMC_FFU_MODE_NORMAL:
		offset = EXT_CSD_MODE_CONFIG;
		break;
	case MMC_FFU_INSTALL_SET:
		offset = EXT_CSD_MODE_OPERATION_CODES;
		mode = 0x1;
		break;
	default:
		err = -EINVAL;
		break;
	}

	if (err == 0) {
		err = mmc_switch(card, MMC_FFU_EXT_CSD_CMD_SET_NORMAL,
					offset, mode,
					card->ext_csd.generic_cmd6_time);
	}

	return err;
}

u8 mmc_ffu_check_compatibility(struct mmc_card *card, const char *name)
{
	u8 ret = false;
	char *buf;
	unsigned int rev;

	buf = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!buf)
		return false;

	/* manfid + prod_name */
	snprintf(buf, PATH_MAX, "/emmc/%02X-%02X%02X%02X%02X%02X%02X-",
			card->cid.manfid,
			card->cid.prod_name[0], card->cid.prod_name[1],
			card->cid.prod_name[2], card->cid.prod_name[3],
			card->cid.prod_name[4], card->cid.prod_name[5]);

	if (strncmp(name, buf, strnlen(buf, PATH_MAX)))
		goto exit;

	name += strnlen(buf, PATH_MAX);

	/* fwrev */
	if (strncmp(name, "XX", 2)) {
		if (sscanf(name, "%x", &rev) < 1 ||
			rev <= card->ext_csd.fwrev[0]) {
			pr_warn("FFU: %s: The target firmware is older or the "
				"same (current=0x%02x:target=0x%02x)\n",
				mmc_hostname(card->host),
				card->ext_csd.fwrev[0], rev);
			goto exit;
		}
	}
	name += 2;

	/* file extension */
	if (strncmp(name, ".bin", 4))
		goto exit;

	name += 4;
	if (*name)
		goto exit;

	ret = true;
exit:
	kfree(buf);
	return ret;
}

static int mmc_ffu_install(struct mmc_card *card, u8 *ext_csd)
{
	int err;
	u32 timeout;

	/* check mode operation */
	if (!card->ext_csd.ffu_mode_op) {
		/* host switch back to work in normal MMC Read/Write commands */
		err = mmc_ffu_switch_mode(card, MMC_FFU_MODE_NORMAL);
		if (err) {
			pr_err("FFU: %s: switch to normal mode error %d:\n",
				mmc_hostname(card->host), err);
			return err;
		}
	} else {
		timeout = ext_csd[EXT_CSD_OPERATION_CODE_TIMEOUT];
		if (timeout == 0 || timeout > 0x17) {
			timeout = 0x17;
			pr_warn("FFU: %s: operation code timeout is out "
				"of range. Using maximum timeout.\n",
				mmc_hostname(card->host));
		}

		/* timeout is at millisecond resolution */
		timeout = DIV_ROUND_UP((100 * (1 << timeout)), 1000);

		/* set ext_csd to install mode */
		err = mmc_ffu_switch_mode(card, MMC_FFU_INSTALL_SET);
		if (err) {
			pr_err("FFU: %s: error %d setting install mode\n",
				mmc_hostname(card->host), err);
			return err;
		}
	}

	return 0;
}

int mmc_ffu_check_status(struct mmc_card *card)
{
	int err;

	err = card->cached_ext_csd[EXT_CSD_FFU_STATUS];
	if (err) {
		pr_err("FFU: %s: error %d FFU install:\n",
			mmc_hostname(card->host), err);
		return -EINVAL;
	}

	return err;
}

int mmc_ffu_invoke(struct mmc_card *card, const char *name)
{
	u8 ext_csd[512];
	int err;
	u32 arg;
	u32 fw_prog_bytes;
	const struct firmware *fw;
	int block_size = card->ext_csd.data_sector_size;

	WARN_ON(!card->host->claimed);

	/* Check if FFU is supported */
	if (!card->ext_csd.ffu_capable) {
		pr_err("FFU: %s: error FFU is not supported %d rev %d\n",
			mmc_hostname(card->host), card->ext_csd.ffu_capable,
			card->ext_csd.rev);
		return -EOPNOTSUPP;
	}

	if (strnlen(name, PATH_MAX) > 512) {
		pr_err("FFU: %s: %.20s is not a valid argument\n",
			mmc_hostname(card->host), name);
		return -EINVAL;
	}

	if (!mmc_ffu_check_compatibility(card, name))
		return -EINVAL;

	pr_info("FFU: %s: Start FFU operation name=%s current=0x%02x\n",
		mmc_hostname(card->host), name, card->ext_csd.fwrev[0]);

	/* setup FW data buffer */
	err = request_firmware(&fw, name, NULL);
	if (err) {
		pr_err("FFU: %s: Firmware request failed %d\n",
			mmc_hostname(card->host), err);
		return -EINVAL;
	}
	if ((fw->size % block_size)) {
		pr_warn("FFU: %s: Warning %zd firmware data size "
			"is not aligned!!!\n", mmc_hostname(card->host),
			fw->size);
	}

	/* Read the EXT_CSD */
	err = mmc_send_ext_csd(card, ext_csd);
	if (err) {
		pr_err("FFU: %s: error %d sending ext_csd\n",
			mmc_hostname(card->host), err);
		goto exit;
	}

	/* set CMD ARG */
	arg = ext_csd[EXT_CSD_FFU_ARG] |
	ext_csd[EXT_CSD_FFU_ARG + 1] << 8 |
	ext_csd[EXT_CSD_FFU_ARG + 2] << 16 |
	ext_csd[EXT_CSD_FFU_ARG + 3] << 24;

	/* set device to FFU mode */
	err = mmc_ffu_switch_mode(card, MMC_FFU_MODE_SET);
	if (err) {
		pr_err("FFU: %s: error %d FFU is not supported\n",
			mmc_hostname(card->host), err);
		goto exit;
	}

	err = mmc_ffu_write(card, fw->data, arg, fw->size);
	if (err) {
		pr_err("FFU: %s: write error %d\n",
			mmc_hostname(card->host), err);
		goto exit;
	}
	/* payload will be checked only in op_mode supported */
	if (card->ext_csd.ffu_mode_op) {
		/* Read the EXT_CSD */
		err = mmc_send_ext_csd(card, ext_csd);
		if (err) {
			pr_err("FFU: %s: error %d sending ext_csd\n",
				mmc_hostname(card->host), err);
			goto exit;
		}

		/* check that the eMMC has received the payload */
		fw_prog_bytes = ext_csd[EXT_CSD_NUM_OF_FW_SEC_PROG] |
		ext_csd[EXT_CSD_NUM_OF_FW_SEC_PROG + 1] << 8 |
		ext_csd[EXT_CSD_NUM_OF_FW_SEC_PROG + 2] << 16 |
		ext_csd[EXT_CSD_NUM_OF_FW_SEC_PROG + 3] << 24;

		/* convert sectors to bytes: multiply by -512B or 4KB as
		 * required by the card */
		fw_prog_bytes *=
			block_size << (ext_csd[EXT_CSD_DATA_SECTOR_SIZE] * 3);
		if (fw_prog_bytes != fw->size) {
			err = -EINVAL;
			pr_err("FFU: %s: error %d number of programmed fw "
				"sector incorrect %d %zd\n", __func__, err,
				fw_prog_bytes, fw->size);
			goto exit;
		}
	}

	err = mmc_ffu_install(card, ext_csd);
	if (err) {
		pr_err("FFU: %s: error firmware install %d\n",
			mmc_hostname(card->host), err);
		goto exit;
	}

exit:
	if (err != 0) {
		/* host switch back to work in normal MMC
		 * Read/Write commands */
		mmc_ffu_switch_mode(card, MMC_FFU_MODE_NORMAL);
		err = -EIO;
	}
	release_firmware(fw);

	return err;
}
