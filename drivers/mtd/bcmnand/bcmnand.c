/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/*
 *  drivers/mtd/bcmnand/bcmnand.c
 *
 *  Overview:
 *   This is the generic MTD driver for BCMNAND flash devices. 
 *   It borrows some code from nand_base.c, but since the generic
 *   nand code is too low level for this microcode interface,
 *   a new driver was created here.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/flashchip.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/bcmnand.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <chal/chal_nand.h>
#include <linux/clk.h>
#include <linux/delay.h>

static int nand_get_device(struct bcmnand_chip *chip, struct mtd_info *mtd,
			   int new_state);

static int nand_do_write_oob(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops);

static int nand_block_isbad(struct mtd_info *mtd, loff_t ofs);

static struct clk *clk;

#define TRACE() printk(KERN_INFO "%s: line %d\n", __func__, __LINE__)

const char *part_probes[] = { "cmdlinepart", NULL };

static void nandPrintError(uint32_t rc)
{
	const char *msg;

	uint8_t err = rc & 0xFF;
	uint32_t line = rc >> 8;

	switch (err) {
	case CHAL_NAND_RC_SUCCESS:
		msg = "Success";
		break;
	case CHAL_NAND_RC_FAILURE:
		msg = "Generic failure";
		break;
	case CHAL_NAND_RC_NOMEM:
		msg = "UC memory not available";
		break;
	case CHAL_NAND_RC_NOCMD:
		msg = "UC command not defined";
		break;
	case CHAL_NAND_RC_CMD_TOUT:
		msg = "UC command complete timeout";
		break;
	case CHAL_NAND_RC_RB_TOUT:
		msg = "Wait for ready interrupt timeout";
		break;
	case CHAL_NAND_RC_PAGE_SIZE_ERR:
		msg = "Unsupported page size";
		break;
	case CHAL_NAND_RC_BB_NOERASE:
		msg = "Erase bad block not alowed";
		break;
	case CHAL_NAND_RC_DMA_ACTIVE_TOUT:
		msg = "DMA active timeout";
		break;
	case CHAL_NAND_RC_DMA_CMPL_TOUT:
		msg = "DMA transfer complete interrupt timeout";
		break;
	case CHAL_NAND_RC_BANK_CMPL_TOUT:
		msg = "Bank operation complete interrupt timeout";
		break;
	case CHAL_NAND_RC_ECC_RS_ERROR:
		msg = "Uncorrectable RS ECC error";
		break;
	case CHAL_NAND_RC_ECC_HM_ERROR:
		msg = "Uncorrectable HM ECC error";
		break;
	case CHAL_NAND_RC_BANK_CFG_ERR:
		msg = "Unsupported bank configuration";
		break;
	default:
		msg = "Unknown return code";
	};

	printk("rc=%d line=%d : %s\n", err, line, msg);
}

static struct nand_ecclayout nand_hw_eccoob;

static int check_offs_len(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct bcmnand_chip *chip = mtd->priv;
	int ret = 0;

	/* Start address must align on block boundary */
	if (ofs & ((1 << chip->phys_erase_shift) - 1)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Unaligned address\n", __func__);
		ret = -EINVAL;
	}

	/* Length must align on block boundary */
	if (len & ((1 << chip->phys_erase_shift) - 1)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Length not block aligned\n",
					__func__);
		ret = -EINVAL;
	}

	/* Do not allow past end of device */
	if (ofs + len > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Past end of device\n",
					__func__);
		ret = -EINVAL;
	}

	return ret;
}

/**
 * nand_release_device - [GENERIC] release chip
 * @mtd:	MTD device structure
 *
 * Deselect, release chip lock and wake up anyone waiting on the device
 */
static void nand_release_device(struct mtd_info *mtd)
{
	struct bcmnand_chip *chip = mtd->priv;

	/* Release the controller and the chip */
	spin_lock(&chip->controller->lock);
	chip->controller->active = NULL;
	chip->state = FL_READY;
	wake_up(&chip->controller->wq);
	spin_unlock(&chip->controller->lock);
}

/**
 * nand_block_bad - [DEFAULT] Read bad block marker from the chip
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 *
 * Check, if the block is bad.
 */
static int nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int chipnr, ret;
	struct bcmnand_chip *chip = mtd->priv;
	int block;
	uint8_t is_bad = 0;

	/* Check for invalid offset */
	if (ofs > mtd->size)
		return -EINVAL;

	chipnr = (int)(ofs >> chip->chip_shift);
	block = (int)(ofs >> chip->phys_erase_shift);

	if (getchip) {
		nand_get_device(chip, mtd, FL_READING);
	}

	ret = chal_nand_block_isbad(&chip->chal_nand, chipnr, block, &is_bad);

	if (getchip)
		nand_release_device(mtd);

	if (ret != CHAL_NAND_RC_SUCCESS) {
		printk(KERN_INFO "%s ofs=0x%llx is_bad=%d\n", __func__, ofs,
		       is_bad);
		nandPrintError(ret);
		/* If low level function fails, block may be good or bad - assume good */
	}
	return is_bad == 1;
}

/**
 * nand_check_wp - [GENERIC] check if the chip is write protected
 * @mtd:	MTD device structure
 * Check, if the device is write protected
 *
 * The function expects, that the device is already selected
 */
static int nand_check_wp(struct mtd_info *mtd)
{
#if 0				// Enable for Capri - not supported on BI
	struct bcmnand_chip *chip = mtd->priv;

	/* broken xD cards report WP despite being writable */
	if (chip->options & NAND_BROKEN_XD)
		return 0;

	/* Check the WP bit */
	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	return (chip->read_byte(mtd) & NAND_STATUS_WP) ? 0 : 1;
#endif
	return 0;
}

/**
 * nand_block_checkbad - [GENERIC] Check if a block is marked bad
 * @mtd:        MTD device structure
 * @ofs:        offset from device start
 * @getchip:    0, if the chip is already selected
 * @allowbbt:   1, if its allowed to access the bbt area
 *
 * Check, if the block is bad. Either by reading the bad block table or
 * calling of the scan function.
 */
static int nand_block_checkbad(struct mtd_info *mtd, loff_t ofs, int getchip,
			       int allowbbt)
{
	(void)allowbbt;
	return nand_block_bad(mtd, ofs, getchip);
}

/**
 * panic_nand_get_device - [GENERIC] Get chip for selected access
 * @chip:	the nand chip descriptor
 * @mtd:	MTD device structure
 * @new_state:	the state which is requested
 *
 * Used when in panic, no locks are taken.
 */
static void panic_nand_get_device(struct bcmnand_chip *chip,
				  struct mtd_info *mtd, int new_state)
{
	/* Hardware controller shared among independend devices */
	chip->controller->active = chip;
	chip->state = new_state;
}

/**
 * nand_get_device - [GENERIC] Get chip for selected access
 * @chip:	the nand chip descriptor
 * @mtd:	MTD device structure
 * @new_state:	the state which is requested
 *
 * Get the device and lock it for exclusive access
 */
static int
nand_get_device(struct bcmnand_chip *chip, struct mtd_info *mtd, int new_state)
{
	spinlock_t *lock = &chip->controller->lock;
	wait_queue_head_t *wq = &chip->controller->wq;
	DECLARE_WAITQUEUE(wait, current);
retry:
	spin_lock(lock);

	/* Hardware controller shared among independent devices */
	if (!chip->controller->active)
		chip->controller->active = chip;

	if (chip->controller->active == chip && chip->state == FL_READY) {
		chip->state = new_state;
		spin_unlock(lock);
		return 0;
	}
	if (new_state == FL_PM_SUSPENDED) {
		if (chip->controller->active->state == FL_PM_SUSPENDED) {
			chip->state = FL_PM_SUSPENDED;
			spin_unlock(lock);
			return 0;
		}
	}
	set_current_state(TASK_UNINTERRUPTIBLE);
	add_wait_queue(wq, &wait);
	spin_unlock(lock);
	schedule();
	remove_wait_queue(wq, &wait);
	goto retry;
}

/**
 * panic_nand_wait - [GENERIC]  wait until the command is done
 * @mtd:	MTD device structure
 * @chip:	NAND chip structure
 * @timeo:	Timeout
 *
 * Wait for command done. This is a helper function for nand_wait used when
 * we are in interrupt context. May happen when in panic and trying to write
 * an oops through mtdoops.
 */
static void panic_nand_wait(struct mtd_info *mtd, struct bcmnand_chip *chip,
			    unsigned long timeo)
{
#if 0				// maybe can do this in Capri
	int i;
	for (i = 0; i < timeo; i++) {
		if (chip->dev_ready) {
			if (chip->dev_ready(mtd))
				break;
		} else {
			if (chip->read_byte(mtd) & NAND_STATUS_READY)
				break;
		}
		mdelay(1);
	}
#endif
}

/**
 * nand_transfer_oob - [Internal] Transfer oob to client buffer
 * @chip:	nand chip structure
 * @oob:	oob destination address
 * @ops:	oob ops structure
 * @len:	size of oob to transfer
 */
static uint8_t *nand_transfer_oob(struct bcmnand_chip *chip, uint8_t *oob,
				  struct mtd_oob_ops *ops, size_t len)
{
	switch (ops->mode) {

	case MTD_OOB_PLACE:
	case MTD_OOB_RAW:
		memcpy(oob, chip->oob_poi + ops->ooboffs, len);
		return oob + len;

	case MTD_OOB_AUTO: {
		struct nand_oobfree *free = chip->ecc.layout->oobfree;
		uint32_t boffs = 0, roffs = ops->ooboffs;
		size_t bytes = 0;

		for (; free->length && len; free++, len -= bytes) {
			/* Read request not from offset 0 ? */
			if (unlikely(roffs)) {
				if (roffs >= free->length) {
					roffs -= free->length;
					continue;
				}
				boffs = free->offset + roffs;
				bytes = min_t(size_t, len,
					      (free->length - roffs));
				roffs = 0;
			} else {
				bytes = min_t(size_t, len, free->length);
				boffs = free->offset;
			}
			memcpy(oob, chip->oob_poi + boffs, bytes);
			oob += bytes;
		}
		return oob;
	}
	default:
		BUG();
	}
	return NULL;
}

/**
 * nand_do_read_ops - [Internal] Read data with ECC
 *
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob ops structure
 *
 * Internal function. Called with chip held.
 */
static int nand_do_read_ops(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops)
{
	int chipnr, page, realpage, col, bytes, aligned;
	struct bcmnand_chip *chip = mtd->priv;
	struct mtd_ecc_stats stats;
	int ret = 0;
	uint32_t readlen = ops->len;
	uint32_t oobreadlen = ops->ooblen;
	uint32_t max_oobsize = ops->mode == MTD_OOB_AUTO ?
		mtd->oobavail : mtd->oobsize;

	uint8_t *bufpoi, *oob, *buf;

	int toterrs;

	stats = mtd->ecc_stats;

	chipnr = (int)(from >> chip->chip_shift);
	realpage = (int)(from >> chip->page_shift);
	page = realpage & chip->pagemask;

	col = (int)(from & (mtd->writesize - 1));

	buf = ops->datbuf;
	oob = ops->oobbuf;

	while (1) {
		bytes = min(mtd->writesize - col, readlen);
		aligned = (bytes == mtd->writesize);

		/* Is the current page in the buffer ? */
		if (realpage != chip->pagebuf || oob) {
			bufpoi = aligned ? buf : chip->buffers->databuf;

			/* Now read the page into the buffer */
			ret =
			    chal_nand_page_read(&chip->chal_nand, chipnr, page,
						bufpoi);
			if (ret != CHAL_NAND_RC_SUCCESS) {
				nandPrintError(ret);
				mtd->ecc_stats.failed++;
			}
			{
				uint32_t ecc_stats;
				int i;

				/* Sum nibbles of error counts */
				toterrs = 0;
				ecc_stats = (CHAL_NAND_ECC_STATS(&chip->chal_nand))[0];
				for (i = 0; i <(CHAL_NAND_PAGE_SIZE(&chip->chal_nand) /
			      		CHAL_NAND_RS_ECC_BLOCK_SIZE); i++) {
					toterrs += ecc_stats & 0xF;
					ecc_stats >>= 4;
				}
			}
			mtd->ecc_stats.corrected += toterrs;

			ret =
			    chal_nand_oob_read(&chip->chal_nand, chipnr, page,
					       bufpoi + mtd->writesize);
			if (ret != CHAL_NAND_RC_SUCCESS) {
				nandPrintError(ret);
			}

			/* Transfer not aligned data */
			if (!aligned) {
				if (!NAND_SUBPAGE_READ(chip) && !oob &&
				    !(mtd->ecc_stats.failed - stats.failed))
					chip->pagebuf = realpage;
				memcpy(buf, chip->buffers->databuf + col, bytes);
			}

			buf += bytes;

			if (unlikely(oob)) {

				int toread = min(oobreadlen, max_oobsize);

				if (toread) {
					oob = nand_transfer_oob(chip,
						oob, ops, toread);
					oobreadlen -= toread;
				}
			}
		} else {
			memcpy(buf, chip->buffers->databuf + col, bytes);
			buf += bytes;
		}

		readlen -= bytes;

		if (!readlen)
			break;

		/* For subsequent reads align to page boundary. */
		col = 0;
		/* Increment page address */
		realpage++;

		page = realpage & chip->pagemask;
		/* Check, if we cross a chip boundary */
		if (!page) {
			chipnr++;
		}
	}

	ops->retlen = ops->len - (size_t) readlen;
	if (oob)
		ops->oobretlen = ops->ooblen - oobreadlen;

	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;

	return  mtd->ecc_stats.corrected - stats.corrected ? -EUCLEAN : 0;
}

/**
 * nand_read - [MTD Interface] MTD compatibility function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
 * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, uint8_t *buf)
{
	struct bcmnand_chip *chip = mtd->priv;
	int ret;

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	nand_get_device(chip, mtd, FL_READING);

	chip->ops.len = len;
	chip->ops.datbuf = buf;
	chip->ops.oobbuf = NULL;

	ret = nand_do_read_ops(mtd, from, &chip->ops);

	*retlen = chip->ops.retlen;

	nand_release_device(mtd);

	return ret;
}

/**
 * nand_do_read_oob - [Intern] NAND read out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operations description structure
 *
 * NAND read out-of-band data from the spare area
 */
static int nand_do_read_oob(struct mtd_info *mtd, loff_t from,
			    struct mtd_oob_ops *ops)
{
	int page, realpage, chipnr;
	struct bcmnand_chip *chip = mtd->priv;
	int readlen = ops->ooblen;
	int len;
	uint8_t *buf = ops->oobbuf;

	DEBUG(MTD_DEBUG_LEVEL3, "%s: from = 0x%08Lx, len = %i\n",
			__func__, (unsigned long long)from, readlen);

	if (ops->mode == MTD_OOB_AUTO)
		len = chip->ecc.layout->oobavail;
	else
		len = mtd->oobsize;

	if (unlikely(ops->ooboffs >= len)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt to start read "
					"outside oob\n", __func__);
		return -EINVAL;
	}

	/* Do not allow reads past end of device */
	if (unlikely(from >= mtd->size ||
		     ops->ooboffs + readlen > ((mtd->size >> chip->page_shift) -
					(from >> chip->page_shift)) * len)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt read beyond end "
					"of device\n", __func__);
		return -EINVAL;
	}

	chipnr = (int)(from >> chip->chip_shift);

	/* Shift to get page */
	realpage = (int)(from >> chip->page_shift);
	page = realpage & chip->pagemask;

	while (1) {
		int ret;

		ret = chal_nand_oob_read(&chip->chal_nand, chipnr, page, buf);
		if (ret != CHAL_NAND_RC_SUCCESS) {
			nandPrintError(ret);
		}

		len = min(len, readlen);
		buf = nand_transfer_oob(chip, buf, ops, len);

		readlen -= len;
		if (readlen <= 0) {
			readlen = 0;
			break;
		}

		/* Increment page address */
		realpage++;

		page = realpage & chip->pagemask;
		/* Check, if we cross a chip boundary */
		if (!page) {
			chipnr++;
		}
	}

	ops->oobretlen = ops->ooblen;
	return 0;
}

/**
 * nand_read_oob - [MTD Interface] NAND read data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 *
 * NAND read data and/or out-of-band data
 */
static int nand_read_oob(struct mtd_info *mtd, loff_t from,
			 struct mtd_oob_ops *ops)
{
	struct bcmnand_chip *chip = mtd->priv;
	int ret = -ENOTSUPP;

	ops->retlen = 0;

	/* Do not allow reads past end of device */
	if (ops->datbuf && (from + ops->len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt read "
				"beyond end of device\n", __func__);
		return -EINVAL;
	}

	nand_get_device(chip, mtd, FL_READING);

	switch (ops->mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_AUTO:
	case MTD_OOB_RAW:
		break;

	default:
		goto out;
	}

	if (!ops->datbuf)
		ret = nand_do_read_oob(mtd, from, ops);
	else
		ret = nand_do_read_ops(mtd, from, ops);

out:
	nand_release_device(mtd);
	return ret;
}

/**
 * nand_fill_oob - [Internal] Transfer client buffer to oob
 * @chip:	nand chip structure
 * @oob:	oob data buffer
 * @len:	oob data write length
 * @ops:	oob ops structure
 */
static uint8_t *nand_fill_oob(struct bcmnand_chip *chip, uint8_t *oob, size_t len,
						struct mtd_oob_ops *ops)
{
	switch (ops->mode) {

	case MTD_OOB_PLACE:
	case MTD_OOB_RAW:
		memcpy(chip->oob_poi + ops->ooboffs, oob, len);
		return oob + len;

	case MTD_OOB_AUTO: {
		struct nand_oobfree *free = chip->ecc.layout->oobfree;
		uint32_t boffs = 0, woffs = ops->ooboffs;
		size_t bytes = 0;

		for (; free->length && len; free++, len -= bytes) {
			/* Write request not from offset 0 ? */
			if (unlikely(woffs)) {
				if (woffs >= free->length) {
					woffs -= free->length;
					continue;
				}
				boffs = free->offset + woffs;
				bytes = min_t(size_t, len,
					      (free->length - woffs));
				woffs = 0;
			} else {
				bytes = min_t(size_t, len, free->length);
				boffs = free->offset;
			}
			memcpy(chip->oob_poi + boffs, oob, bytes);
			oob += bytes;
		}
		return oob;
	}
	default:
		BUG();
	}
	return NULL;
}

#define NOTALIGNED(x)	((x & (chip->subpagesize - 1)) != 0)

/**
 * nand_do_write_ops - [Internal] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operations description structure
 *
 * NAND write with ECC
 */
static int nand_do_write_ops(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	int chipnr, realpage, page, blockmask, column;
	struct bcmnand_chip *chip = mtd->priv;
	uint32_t writelen = ops->len;

	uint32_t oobwritelen = ops->ooblen;
	uint32_t oobmaxlen = ops->mode == MTD_OOB_AUTO ?
				mtd->oobavail : mtd->oobsize;

	uint8_t *oob = ops->oobbuf;
	uint8_t *buf = ops->datbuf;
	int ret, subpage;

	ops->retlen = 0;
	if (!writelen)
		return 0;

	/* reject writes, which are not page aligned */
	if (NOTALIGNED(to) || NOTALIGNED(ops->len)) {
		printk(KERN_NOTICE "%s: Attempt to write not "
				"page aligned data\n", __func__);
		return -EINVAL;
	}

	column = to & (mtd->writesize - 1);
	subpage = column || (writelen & (mtd->writesize - 1));

	if (subpage && oob)
		return -EINVAL;

	chipnr = (int)(to >> chip->chip_shift);

	/* Check, if it is write protected */
	if (nand_check_wp(mtd))
		return -EIO;

	realpage = (int)(to >> chip->page_shift);
	page = realpage & chip->pagemask;
	blockmask = (1 << (chip->phys_erase_shift - chip->page_shift)) - 1;

	/* Invalidate the page cache, when we write to the cached page */
	if (to <= (chip->pagebuf << chip->page_shift) &&
	    (chip->pagebuf << chip->page_shift) < (to + ops->len))
		chip->pagebuf = -1;

	/* If we're not given explicit OOB data, let it be 0xFF */
	if (likely(!oob))
		memset(chip->oob_poi, 0xff, mtd->oobsize);

	/* Don't allow multipage oob writes with offset */
	if (oob && ops->ooboffs && (ops->ooboffs + ops->ooblen > oobmaxlen))
		return -EINVAL;

	while (1) {
		int bytes = mtd->writesize;
		int cached = writelen > bytes && page != blockmask;
		uint8_t *wbuf = buf;

		/* Partial page write ? */
		if (unlikely(column || writelen < (mtd->writesize - 1))) {
			cached = 0;
			bytes = min_t(int, bytes - column, (int) writelen);
			chip->pagebuf = -1;
			memset(chip->buffers->databuf, 0xff, mtd->writesize);
			memcpy(&chip->buffers->databuf[column], buf, bytes);
			wbuf = chip->buffers->databuf;
		}

		if (unlikely(oob)) {
			size_t len = min(oobwritelen, oobmaxlen);
			oob = nand_fill_oob(chip, oob, len, ops);
			oobwritelen -= len;
		}


		ret = chal_nand_page_write(&chip->chal_nand, chipnr, page,
					   (uint8_t *) buf);
		if (ret != CHAL_NAND_RC_SUCCESS) {
			nandPrintError(ret);
			ret = -EIO;
			break;
		}

		if (unlikely(oob)) {
			ret =
			    chal_nand_oob_write(&chip->chal_nand, chipnr, page,
						chip->oob_poi);
			if (ret != CHAL_NAND_RC_SUCCESS) {
				nandPrintError(ret);
				ret = -EIO;
				break;
			}
		}

#ifdef CONFIG_MTD_BCMNAND_VERIFY_WRITE
		ret = chal_nand_page_read(&chip->chal_nand, chipnr, page,
					  (uint8_t *) chip->verifybuf);
		if (ret != CHAL_NAND_RC_SUCCESS) {
			nandPrintError(ret);
			ret = -EIO;
			break;
		}
		if (memcmp(buf, chip->verifybuf, writelen)) {
			printk(KERN_NOTICE "%s: verify failed\n", __func__);
			ret = -EFAULT;
			break;
		}
		//printk(KERN_NOTICE "%s: verify succeeded at page 0x%x\n", __func__, page);
#endif

		writelen -= bytes;
		if (!writelen)
			break;

		column = 0;
		buf += bytes;
		realpage++;

		page = realpage & chip->pagemask;
		/* Check, if we cross a chip boundary */
		if (!page) {
			chipnr++;
		}
	}

	ops->retlen = ops->len - writelen;
	if (unlikely(oob))
		ops->oobretlen = ops->ooblen;
	return ret;
}

/**
 * panic_nand_write - [MTD Interface] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * NAND write with ECC. Used when performing writes in interrupt context, this
 * may for example be called by mtdoops when writing an oops while in panic.
 */
static int panic_nand_write(struct mtd_info *mtd, loff_t to, size_t len,
			    size_t *retlen, const uint8_t *buf)
{
	struct bcmnand_chip *chip = mtd->priv;
	int ret;

	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	/* Wait for the device to get ready.  */
	panic_nand_wait(mtd, chip, 400);

	/* Grab the device.  */
	panic_nand_get_device(chip, mtd, FL_WRITING);

	chip->ops.len = len;
	chip->ops.datbuf = (uint8_t *)buf;
	chip->ops.oobbuf = NULL;

	ret = nand_do_write_ops(mtd, to, &chip->ops);

	*retlen = chip->ops.retlen;
	return ret;
}

/**
 * nand_write - [MTD Interface] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * NAND write with ECC
 * subpages not supported.
 */
static int nand_write(struct mtd_info *mtd, loff_t to, size_t len,
		      size_t * retlen, const uint8_t * buf)
{
	struct bcmnand_chip *chip = mtd->priv;
	int ret;

	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	nand_get_device(chip, mtd, FL_WRITING);

	chip->ops.len = len;
	chip->ops.datbuf = (uint8_t *)buf;
	chip->ops.oobbuf = NULL;

	ret = nand_do_write_ops(mtd, to, &chip->ops);

	*retlen = chip->ops.retlen;

	nand_release_device(mtd);

	return ret;
}

/**
 * nand_do_write_oob - [MTD Interface] NAND write out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 *
 * NAND write out-of-band
 */
static int nand_do_write_oob(struct mtd_info *mtd, loff_t to,
			     struct mtd_oob_ops *ops)
{
	int chipnr, page, status, len;
	struct bcmnand_chip *chip = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3, "%s: to = 0x%08x, len = %i\n",
			 __func__, (unsigned int)to, (int)ops->ooblen);

	if (ops->mode == MTD_OOB_AUTO)
		len = chip->ecc.layout->oobavail;
	else
		len = mtd->oobsize;

	/* Do not allow write past end of page */
	if ((ops->ooboffs + ops->ooblen) > len) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt to write "
				"past end of page\n", __func__);
		return -EINVAL;
	}

	if (unlikely(ops->ooboffs >= len)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt to start "
				"write outside oob\n", __func__);
		return -EINVAL;
	}

	/* Do not allow write past end of device */
	if (unlikely(to >= mtd->size ||
		     ops->ooboffs + ops->ooblen >
			((mtd->size >> chip->page_shift) -
			 (to >> chip->page_shift)) * len)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt write beyond "
				"end of device\n", __func__);
		return -EINVAL;
	}

	chipnr = (int)(to >> chip->chip_shift);

	/* Shift to get page */
	page = (int)(to >> chip->page_shift);

#if 0
	/*
	 * Reset the chip. Some chips (like the Toshiba TC5832DC found in one
	 * of my DiskOnChip 2000 test units) will clear the whole data page too
	 * if we don't do this. I have no clue why, but I seem to have 'fixed'
	 * it in the doc2000 driver in August 1999.  dwmw2.
	 */
	chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
#endif

	/* Check, if it is write protected */
	if (nand_check_wp(mtd))
		return -EROFS;

	/* Invalidate the page cache, if we write to the cached page */
	if (page == chip->pagebuf)
		chip->pagebuf = -1;

	memset(chip->oob_poi, 0xff, mtd->oobsize);
	nand_fill_oob(chip, ops->oobbuf, ops->ooblen, ops);

	status = chal_nand_oob_write(&chip->chal_nand, chipnr, page, chip->oob_poi);
	if (status) {
		nandPrintError(status);
		return status;
	}

	ops->oobretlen = ops->ooblen;

	return 0;
}

/**
 * nand_write_oob - [MTD Interface] NAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 */
static int nand_write_oob(struct mtd_info *mtd, loff_t to,
			  struct mtd_oob_ops *ops)
{
	struct bcmnand_chip *chip = mtd->priv;
	int ret = -ENOTSUPP;

	ops->retlen = 0;

	/* Do not allow writes past end of device */
	if (ops->datbuf && (to + ops->len) > mtd->size) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Attempt write beyond "
				"end of device\n", __func__);
		return -EINVAL;
	}

	nand_get_device(chip, mtd, FL_WRITING);

	switch (ops->mode) {
	case MTD_OOB_PLACE:
	case MTD_OOB_AUTO:
	case MTD_OOB_RAW:
		break;

	default:
		goto out;
	}

	if (!ops->datbuf)
		ret = nand_do_write_oob(mtd, to, ops);
	else
		ret = nand_do_write_ops(mtd, to, ops);

out:
	nand_release_device(mtd);
	return ret;
}

/**
 * nand_erase - [MTD Interface] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 *
 * Erase one ore more blocks
 */
static int nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int page, pages_per_block, ret, chipnr;
	//int status;
	struct bcmnand_chip *chip = mtd->priv;
	loff_t len;

	DEBUG(MTD_DEBUG_LEVEL3, "%s: start = 0x%012llx, len = %llu\n",
				__func__, (unsigned long long)instr->addr,
				(unsigned long long)instr->len);

	if (check_offs_len(mtd, instr->addr, instr->len))
		return -EINVAL;

	instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;

	/* Grab the lock and see if the device is available */
	nand_get_device(chip, mtd, FL_ERASING);

	/* Shift to get first page */
	page = (int)(instr->addr >> chip->page_shift);
	chipnr = (int)(instr->addr >> chip->chip_shift);

	/* Calculate pages in each block */
	pages_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);

	/* Check, if it is write protected */
	if (nand_check_wp(mtd)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Device is write protected!!!\n",
					__func__);
		instr->state = MTD_ERASE_FAILED;
		goto erase_exit;
	}

	/* Loop through the pages */
	len = instr->len;

	instr->state = MTD_ERASING;

	while (len) {
		int block, chipnr;
		loff_t addr = ((loff_t) page) << chip->page_shift;
		/*
		 * heck if we have a bad block, we do not erase bad blocks !
		 */
		if (nand_block_checkbad(mtd, addr, 0, 0)) {
			printk(KERN_WARNING "%s: attempt to erase a bad block "
			       "at page 0x%08x\n", __func__, page);
			instr->state = MTD_ERASE_FAILED;
			goto erase_exit;
		}

		/*
		 * Invalidate the page cache, if we erase the block which
		 * contains the current cached page
		 */
		if (page <= chip->pagebuf && chip->pagebuf <
		    (page + pages_per_block))
			chip->pagebuf = -1;

		block = (int)(addr >> chip->phys_erase_shift);
		chipnr = (int)(addr >> chip->chip_shift);

		ret = chal_nand_block_erase(&chip->chal_nand, chipnr, block);
		if (ret != CHAL_NAND_RC_SUCCESS) {
			nandPrintError(ret);
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = ((loff_t) page << chip->page_shift);
			goto erase_exit;
		}

		/* Increment page address and decrement length */
		len -= (1 << chip->phys_erase_shift);
		page += pages_per_block;
	}
	instr->state = MTD_ERASE_DONE;

erase_exit:

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;

	/* Deselect and wake up anyone waiting on the device */
	nand_release_device(mtd);

	/* Do call back function */
	if (!ret)
		mtd_erase_callback(instr);

	/* Return more or less happy */
	return ret;
}

/**
 * nand_sync - [MTD Interface] sync
 * @mtd:	MTD device structure
 *
 * Sync is actually a wait for chip ready function
 */
static void nand_sync(struct mtd_info *mtd)
{
	struct bcmnand_chip *chip = mtd->priv;

	DEBUG(MTD_DEBUG_LEVEL3, "%s: called\n", __func__);

	/* Grab the lock and see if the device is available */
	nand_get_device(chip, mtd, FL_SYNCING);
	/* Release it and go back */
	nand_release_device(mtd);
}

/**
 * nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start
 */
static int nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	/* Check for invalid offset */
	if (offs > mtd->size)
		return -EINVAL;

	return nand_block_checkbad(mtd, offs, 1, 0);
}

/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct bcmnand_chip *chip = mtd->priv;
	int chipnr = (int)(ofs >> chip->chip_shift);
	int block = (int)(ofs >> chip->phys_erase_shift);
	int ret;

	ret = nand_block_isbad(mtd, ofs);
	if (ret) {
		/* If it was bad already, return success and do nothing. */
		if (ret > 0)
			return 0;
		return ret;
	}

	nand_get_device(chip, mtd, FL_WRITING);
	ret = chal_nand_block_markbad(&chip->chal_nand, chipnr, block);
	nand_release_device(mtd);

	if (ret != CHAL_NAND_RC_SUCCESS) {
		nandPrintError(ret);
		ret = -EIO;
	}
	return ret;
}

/**
 * nand_suspend - [MTD Interface] Suspend the NAND flash
 * @mtd:	MTD device structure
 */
static int nand_suspend(struct mtd_info *mtd)
{
	struct bcmnand_chip *chip = mtd->priv;

	return nand_get_device(chip, mtd, FL_PM_SUSPENDED);
}

/**
 * nand_resume - [MTD Interface] Resume the NAND flash
 * @mtd:	MTD device structure
 */
static void nand_resume(struct mtd_info *mtd)
{
	struct bcmnand_chip *chip = mtd->priv;

	if (chip->state == FL_PM_SUSPENDED)
		nand_release_device(mtd);
	else
		printk(KERN_ERR "%s called for a chip which is not "
		       "in suspended state\n", __func__);
}

struct bcmnand_info {
	/* mtd and nand framework related */
	struct mtd_info mtd;
	struct bcmnand_chip chip;
	struct bcmnand_hw_control controller;
};

static int bcmnand_init_chip(struct bcmnand_info *info)
{
	uint32_t rc;
	uint32_t dma = 1;
	uint32_t ecc = 0;
	uint8_t flags;
	chal_nand_info_t *pni = &info->chip.chal_nand;

	/* Initialize hardware. */

	/* Note - it is assumed that the pinmux and clocks are already setup by the 
	 * platform initialization code. */

	/* Initialize CHAL */
	memset((void *)pni, 0x0, sizeof(*pni));

	flags = CHAL_NAND_FLAG_FORCE;
	if (dma) {
		flags |= CHAL_NAND_FLAG_DMA;
		/* set the DMA setup callback */
		CHAL_NAND_DMA_SETUP_CB(pni) = chal_nand_dma_setup_callback;
	}
	if (ecc)
		flags |= CHAL_NAND_FLAG_ECC;

	if ((rc = chal_nand_init(pni, flags)) != CHAL_NAND_RC_SUCCESS) {
		nandPrintError(rc);
		printk(KERN_ERR "Failed to initialize.\n");
		return -EINVAL;
	}

	printk(KERN_INFO "\nNAND %d bit: %s %s\n"
	       "ID %02X%02X%02X%02X%02X%02X%02X%02X\n"
	       "banks %d\n"
	       "bank size %d MB\n"
	       "page size %d KB\n"
	       "block size %d KB\n"
	       "oob size %d bytes\n"
	       "aux data size %d bytes\n",
	       CHAL_NAND_BUS_WIDTH(pni),
	       (flags & CHAL_NAND_FLAG_DMA) ? "DMA" : "",
	       (flags & CHAL_NAND_FLAG_ECC) ? "ECC" : "",
	       (CHAL_NAND_ID(pni))[0],
	       (CHAL_NAND_ID(pni))[1],
	       (CHAL_NAND_ID(pni))[2],
	       (CHAL_NAND_ID(pni))[3],
	       (CHAL_NAND_ID(pni))[4],
	       (CHAL_NAND_ID(pni))[5],
	       (CHAL_NAND_ID(pni))[6],
	       (CHAL_NAND_ID(pni))[7],
	       CHAL_NAND_BANKS(pni),
	       0x1 << (CHAL_NAND_BANK_SHIFT(pni) - 20),
	       CHAL_NAND_PAGE_SIZE(pni) >> 10,
	       CHAL_NAND_BLOCK_SIZE(pni) >> 10,
	       CHAL_NAND_OOB_SIZE(pni), CHAL_NAND_AUX_DATA_SIZE(pni));

	return 0;
}

static int __devinit bcmnand_probe(struct platform_device *pdev)
{
	int ret;
	struct bcmnand_info *info;
#if 0				// FIXME - add resources later if appropriate
	struct bcmnand_platform_data *pdata = pdev->dev.platform_data;
	struct resource *res = pdev->resource;
	unsigned long size = resource_size(res);
#endif
	struct mtd_info *mtd;
	struct bcmnand_chip *chip;
	chal_nand_info_t *pni;

	clk = clk_get(&pdev->dev, "nand_clk");
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	ret = clk_enable(clk);
	if (!ret)
		goto out_put_clk;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		return -ENOMEM;
		goto out_disable_clk;
	}

	spin_lock_init(&info->controller.lock);
	init_waitqueue_head(&info->controller.wq);

#if 0				// FIXME - add resources later if appropriate
	if (!request_mem_region(res->start, size, dev_name(&pdev->dev))) {
		ret = -EBUSY;
		goto out_free_info;
	}
#endif
	mtd = &info->mtd;
	chip = &info->chip;
	pni = &chip->chal_nand;

#if 0				// FIXME - add resources later if appropriate
	/* Since current ISLAND chal_nand has a msleep polling 
	 * architecture, we don't need to use irqs at this time.
	 * When we phase this out and switch to KONA, we may 
	 * then have to use and irq to signal hardware complete
	 * events.
	 */
	info->irq = platform_get_irq(pdev, 0);
#endif

	if (bcmnand_init_chip(info)) {
		printk(KERN_ERR "NAND init failed\n");
		ret = -EINVAL;
		goto out_free_info;
	}

	chip->chipsize = 0x1 << (CHAL_NAND_BANK_SHIFT(pni));
	chip->numchips = CHAL_NAND_BANKS(pni);
	mtd->size = chip->numchips * chip->chipsize;

	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->erasesize = CHAL_NAND_BLOCK_SIZE(pni);
	mtd->writesize = CHAL_NAND_PAGE_SIZE(pni);
	mtd->oobsize = CHAL_NAND_OOB_SIZE(pni);
	mtd->oobavail = CHAL_NAND_AUX_DATA_SIZE(pni);

	mtd->name = "Island NAND";
	mtd->priv = pni;

	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->panic_write = panic_nand_write;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = nand_suspend;
	mtd->resume = nand_resume;
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;
	mtd->writebufsize = mtd->writesize;

	chip->ecc.layout = &nand_hw_eccoob;
	chip->ecc.layout->oobavail = mtd->oobavail;
	chip->ecc.layout->oobfree[0].offset = 1; 	/* Skip bad block marker */
	chip->ecc.layout->oobfree[0].length = mtd->oobavail-1;

	/* propagate ecc.layout to mtd_info */
	mtd->ecclayout = chip->ecc.layout;

	mtd->subpage_sft = 0;	/* subpages not supported */

	chip->buffers = kmalloc(sizeof(*chip->buffers), GFP_KERNEL);
	if (!chip->buffers) {
		ret = -ENOMEM;
		goto out_free_info;
	}

	/* Set the internal oob buffer location, just after the page data */
	chip->oob_poi = chip->buffers->databuf + mtd->writesize;

#ifdef CONFIG_MTD_BCMNAND_VERIFY_WRITE
	chip->verifybuf = kmalloc(mtd->writesize, GFP_KERNEL);
	if (!chip->verifybuf) {
		ret = -ENOMEM;
		goto out_free_buffers;
	}
#endif

	chip->subpagesize = mtd->writesize >> mtd->subpage_sft;

	/* Initialize state */
	chip->state = FL_READY;

	/* Invalidate the pagebuffer reference */
	chip->pagebuf = -1;

	/* Calculate the address shift from the page size */
	chip->page_shift = ffs(mtd->writesize) - 1;
	/* Convert chipsize to number of pages per chip -1. */
	chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;

	chip->phys_erase_shift = ffs(mtd->erasesize) - 1;
	if (chip->chipsize & 0xffffffff)
		chip->chip_shift = ffs((unsigned)chip->chipsize) - 1;
	else
		chip->chip_shift =
		    ffs((unsigned)(chip->chipsize >> 32)) + 32 - 1;

	chip->controller = &info->controller;
	info->mtd.name = dev_name(&pdev->dev);
	mtd->priv = &info->chip;
	mtd->owner = THIS_MODULE;

	/* Register the partitions */
	{
		int nr_partitions;
		struct mtd_partition *partition_info;

		mtd->name = "bcmnand";
		nr_partitions =
		    parse_mtd_partitions(mtd, part_probes, &partition_info, 0);

		if (nr_partitions <= 0) {
			printk(KERN_ERR "BCMNAND: Too few partitions - %d\n",
			       nr_partitions);
			ret = -EIO;
			goto out_free_verifybuf;
		}
		mtd_device_register(mtd, partition_info, nr_partitions);
	}

	platform_set_drvdata(pdev, info);

	return 0;

out_free_verifybuf:
	kfree(chip->verifybuf);
out_free_buffers:
	kfree(chip->buffers);
out_free_info:
	kfree(info);
out_disable_clk:
	clk_disable(clk);
out_put_clk:
	clk_put(clk);

	return ret;
}

static int __devexit bcmnand_remove(struct platform_device *pdev)
{
	struct bcmnand_info *info = platform_get_drvdata(pdev);
#if 0				// FIXME - maybe add resources later if appropriate
	struct resource *res = pdev->resource;
	unsigned long size = resource_size(res);
#endif

	platform_set_drvdata(pdev, NULL);

#if 0				// FIXME - maybe add resources later if appropriate
	release_mem_region(res->start, size);
#endif
	kfree(info->chip.buffers);
	kfree(info);
	clk_disable(clk);
	clk_put(clk);
	return 0;
}

#define bcmnand_suspend    NULL
#define bcmnand_resume     NULL

static struct platform_driver bcmnand_driver = {
	.probe = bcmnand_probe,
	.remove = __devexit_p(bcmnand_remove),
	.suspend = bcmnand_suspend,
	.resume = bcmnand_resume,
	.driver = {
		   .name = "bcmnand",
		   .owner = THIS_MODULE,
		   },

};

static int __init bcmnand_init(void)
{
	printk(KERN_INFO "BCMNAND Driver,(C) 2011 Broadcom Corp\n");
	return platform_driver_register(&bcmnand_driver);
}

static void __exit bcmnand_exit(void)
{
	platform_driver_unregister(&bcmnand_driver);
}

module_init(bcmnand_init);
module_exit(bcmnand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom NAND flash driver code");
