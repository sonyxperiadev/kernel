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

static struct clk *clk;

#define ENTER() printk(KERN_INFO "%s: line %d\n", __func__, __LINE__)

const char *part_probes[] = { "cmdlinepart", NULL };

/*
 * The RS oob is 38 bytes free at the start, but RS is deprecated,
 * so we use the 8/16/32 byte format from BCH for everything.
 */
static struct nand_ecclayout nand_hw_eccoob_2048 = {
	.oobavail = 8,
	.oobfree = {{.offset = 0,.length = 8}}
};

static struct nand_ecclayout nand_hw_eccoob_4096 = {
	.oobavail = 16,
	.oobfree = {{.offset = 0,.length = 16}}
};

static struct nand_ecclayout nand_hw_eccoob_8192 = {
	.oobavail = 32,
	.oobfree = {{.offset = 0,.length = 32}}
};

static int check_offs_len(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct bcmnand_chip *chip = mtd->priv;
	int ret = 0;
	ENTER();

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
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Past end of device\n", __func__);
		ret = -EINVAL;
	}

	return ret;
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

/**
 * nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start
 */
static int nand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	int ret;
	uint8_t is_bad;
	struct bcmnand_chip *chip = mtd->priv;
	int bank = (int)(ofs >> chip->chip_shift);
	int block = (int)(ofs >> chip->phys_erase_shift);

	/* Check for invalid offset */
	if (ofs > mtd->size)
		return -EINVAL;

	nand_get_device(chip, mtd, FL_READING);
	ret = chal_nand_block_isbad(&chip->chal_nand, bank, block, &is_bad);
	nand_release_device(mtd);

	if (ret != CHAL_NAND_RC_SUCCESS) {
		printk(KERN_INFO "%s ofs=0x%llx is_bad=%d\n", __func__, ofs, is_bad);
		nandPrintError(ret);
		return -EIO;
	}

	return is_bad == 1;
}

/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct bcmnand_chip *chip = mtd->priv;
	int bank = (int)(ofs >> chip->chip_shift);
	int block = (int)(ofs >> chip->phys_erase_shift);
	int ret;

	ENTER();
	if ((ret = nand_block_isbad(mtd, ofs))) {
		/* If it was bad already, return success and do nothing. */
		if (ret > 0)
			return 0;
		return ret;
	}

	nand_get_device(chip, mtd, FL_WRITING);
	ret = chal_nand_block_markbad(&chip->chal_nand, bank, block);
	nand_release_device(mtd);

	if (ret != CHAL_NAND_RC_SUCCESS) {
		nandPrintError(ret);
		ret = -EIO;
	}
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

	ENTER();
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

#if 0				// fixme
	/* Check, if it is write protected */
	if (nand_check_wp(mtd)) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: Device is write protected!!!\n",
		      __func__);
		instr->state = MTD_ERASE_FAILED;
		goto erase_exit;
	}
#endif

	/* Loop through the pages */
	len = instr->len;

	instr->state = MTD_ERASING;

	while (len) {
		int block, bank;
		loff_t addr = ((loff_t) page) << chip->page_shift;
		/*
		 * heck if we have a bad block, we do not erase bad blocks !
		 */
		if (nand_block_isbad(mtd, addr)) {
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
		bank = (int)(addr >> chip->chip_shift);

		ret = chal_nand_block_erase(&chip->chal_nand, bank, block);
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
	int bank = (int)(to >> chip->chip_shift);

	ENTER();
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
		ret =
		    chal_nand_oob_write(&chip->chal_nand, bank,
					to / mtd->writesize, ops->oobbuf);
	else
		ret =
		    chal_nand_page_write(&chip->chal_nand, bank,
					 to / mtd->writesize, ops->datbuf);

	if (ret != CHAL_NAND_RC_SUCCESS) {
		nandPrintError(ret);
		ret = -EIO;
	}

 out:
	nand_release_device(mtd);
	return ret;
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
	int bank = (int)(from >> chip->chip_shift);

	//ENTER();

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
		ret =
		    chal_nand_oob_read(&chip->chal_nand, bank,
				       from / mtd->writesize, ops->oobbuf);
	else
		ret =
		    chal_nand_page_read(&chip->chal_nand, bank,
					from / mtd->writesize, ops->datbuf);

	if (ret != CHAL_NAND_RC_SUCCESS) {
#if 1 // REMOVE LATER
		printk("mode=%d len=%d ooblen=%d ooboffs=%d datbuf=%p oobbuf=%p\n",
			ops->mode,ops->len,ops->ooblen,ops->ooboffs,ops->datbuf,ops->oobbuf);
		printk("&chip->chal_nand=%p bank=%d from=0x%x, page=0x%x\n", 
			&chip->chal_nand, bank, (int)from, (int)((int)from/mtd->writesize));
#endif
		nandPrintError(ret);
		ret = -EIO;
	}

 out:
	nand_release_device(mtd);

	if (!ops->datbuf)
		ops->oobretlen = ops->ooblen;
	else
		ops->retlen = ops->len;

	return ret;
}

#define NOTALIGNED(x)	(x & (chip->subpagesize - 1)) != 0
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
	int realpage, page;
	struct bcmnand_chip *chip = mtd->priv;
	uint32_t writelen = ops->len;
	uint8_t *buf = ops->datbuf;
	int ret;

	ENTER();
	ops->retlen = 0;
	if (!writelen)
		return 0;

	/* reject writes, which are not page aligned */
	if (NOTALIGNED(to) || NOTALIGNED(ops->len)) {
		printk(KERN_NOTICE "%s: Attempt to write not "
		       "page aligned data\n", __func__);
		return -EINVAL;
	}

#if 0
	/* Check, if it is write protected */
	if (nand_check_wp(mtd))
		return -EIO;
#endif

	realpage = (int)(to >> chip->page_shift);
	page = realpage & chip->pagemask;

	/* Invalidate the page cache, when we write to the cached page */
	if (to <= (chip->pagebuf << chip->page_shift) &&
	    (chip->pagebuf << chip->page_shift) < (to + ops->len))
		chip->pagebuf = -1;

	while (1) {
		int bytes = mtd->writesize;
		int bank = (int)(to >> chip->chip_shift);

		ret =
		    chal_nand_page_write(&chip->chal_nand, bank, page,
					 (uint8_t *) buf);
		if (ret != CHAL_NAND_RC_SUCCESS) {
			nandPrintError(ret);
			ret = -EIO;
			break;
		}

		writelen -= bytes;
		if (!writelen)
			break;

		buf += bytes;
		to += bytes;
		realpage++;
		page = realpage & chip->pagemask;
	}

	ops->retlen = ops->len - writelen;

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

	ENTER();
	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size)
		return -EINVAL;
	if (!len)
		return 0;

	nand_get_device(chip, mtd, FL_WRITING);

	chip->ops.len = len;
	chip->ops.datbuf = (uint8_t *) buf;
	chip->ops.oobbuf = NULL;

	ret = nand_do_write_ops(mtd, to, &chip->ops);

	*retlen = chip->ops.retlen;

	nand_release_device(mtd);

	return ret;
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
	uint8_t *bufpoi, *buf;
	int toterrs = 0;

	stats = mtd->ecc_stats;

	chipnr = (int)(from >> chip->chip_shift);
	realpage = (int)(from >> chip->page_shift);
	page = realpage & chip->pagemask;

	col = (int)(from & (mtd->writesize - 1));

	buf = ops->datbuf;
#if 0 // FIXME REMOVE LATER
	printk("%s line %d from=0x%llx, chip->chip_shift=%d, chipnr=%d, realpage=0x%x, page=0x%x, col=0x%x, buf=%p\n", 
		__func__, __LINE__, from, chip->chip_shift, chipnr, realpage, page, col, buf);
#endif

	while (1) {
		bytes = min(mtd->writesize - col, readlen);
		aligned = (bytes == mtd->writesize);

		/* Is the current page in the buffer ? */
		if (realpage != chip->pagebuf) {
			uint32_t ecc_stats;
			int i;
			bufpoi = aligned ? buf : chip->buffers->databuf;

			/* Now read the page into the buffer */
			ret =
			    chal_nand_page_read(&chip->chal_nand, chipnr, page,
						bufpoi);
			if (ret != CHAL_NAND_RC_SUCCESS) {
				nandPrintError(ret);
				if (ret == CHAL_NAND_RC_ECC_RS_ERROR) {
					mtd->ecc_stats.failed++;
				}
				ret = -EIO;
				break;
			}
			/* Sum nibbles of error counts */
			ecc_stats = (CHAL_NAND_ECC_STATS(&chip->chal_nand))[0];
			for (i = 0;
			     i <
			     (CHAL_NAND_PAGE_SIZE(&chip->chal_nand) /
			      CHAL_NAND_RS_ECC_BLOCK_SIZE); i++) {
				toterrs += ecc_stats & 0xF;
				ecc_stats >>= 4;
			}
			mtd->ecc_stats.corrected += toterrs;

			/* Transfer not aligned data */
			if (!aligned) {
				chip->pagebuf = realpage;
				memcpy(buf, chip->buffers->databuf + col,
				       bytes);
			}
			buf += bytes;
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
#if 0 // FIXME REMOVE LATER
	printk("%s line %d ops->retlen=0x%x ops->len=0x%x readlen=0x%x\n",
		__func__, __LINE__, ops->retlen, ops->len, readlen);
	if (mtd->ecc_stats.failed != stats.failed)
		printk("%s line %d mtd->ecc_stats.failed=0x%x stats.failed=0x%x\n",
			__func__, __LINE__, mtd->ecc_stats.failed, stats.failed);

	if (mtd->ecc_stats.corrected != stats.corrected)
		printk("%s line %d mtd->ecc_stats.corrected=0x%x stats.corrected=0x%x\n",
			__func__, __LINE__, mtd->ecc_stats.corrected, stats.corrected);
#endif

	if (ret)
		return ret;

	if (mtd->ecc_stats.failed - stats.failed)
		return -EBADMSG;

	return mtd->ecc_stats.corrected - stats.corrected ? -EUCLEAN : 0;
}

/**
 * nand_read - [MTD Interface] MTD compability function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
 * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		     size_t * retlen, uint8_t * buf)
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

#if 0
	printk("%s from=0x%llx len=0x%x retlen=0x%x ret=%d %02x%02x%02x%02x%02x%02x%02x%02x\n", 
		__func__, from, len, *retlen, ret, 
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] );
#endif

	return ret;
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

	ENTER();
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
	int ret = -ENOENT;
	struct bcmnand_info *info;
#if 0				// FIXME - add resources later if appropriate
	struct bcmnand_platform_data *pdata = pdev->dev.platform_data;
	struct resource *res = pdev->resource;
	unsigned long size = resource_size(res);
#endif
	struct mtd_info *mtd;
	struct bcmnand_chip *chip;
	chal_nand_info_t *pni;

	ENTER();

	clk = clk_get(&pdev->dev, "nand_clk");
	clk_enable(clk);

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		return -ENOMEM;
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

#if 0
	/* Don't use this since this is relevant to RS only right now */
	mtd->oobavail = CHAL_NAND_AUX_DATA_SIZE(pni);
#endif
	mtd->name = "Island NAND";
	mtd->priv = pni;

	mtd->write = nand_write;
	mtd->read = nand_read;
	mtd->erase = nand_erase;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;

	/* Now that we know the page size, we can setup the ECC layout */

	switch (mtd->writesize) {	/* writesize is the pagesize */
	case 8192:
		mtd->oobavail = 32;
		mtd->ecclayout = &nand_hw_eccoob_8192;
		break;
	case 4096:
		mtd->oobavail = 16;
		mtd->ecclayout = &nand_hw_eccoob_4096;
		break;
	case 2048:
		mtd->oobavail = 8;
		mtd->ecclayout = &nand_hw_eccoob_2048;
		break;
	default:
		{
			printk(KERN_ERR "NAND - Unrecognized pagesize: %d\n",
			       mtd->writesize);
			return -EINVAL;
		}
	}
	printk("mtd->oobavail=%d, mtd->ecclayout=%p\n", mtd->oobavail, mtd->ecclayout);

	mtd->subpage_sft = 0;	/* subpages not supported */

	chip->buffers = kmalloc(sizeof(*chip->buffers), GFP_KERNEL);
	if (!chip->buffers)
		return -ENOMEM;

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
			goto out_free_info;
		}
		mtd_device_register(mtd, partition_info, nr_partitions);
	}

	platform_set_drvdata(pdev, info);

	return 0;

 out_free_info:
	kfree(info);

	return ret;
}

static int __devexit bcmnand_remove(struct platform_device *pdev)
{
	struct bcmnand_info *info = platform_get_drvdata(pdev);
#if 0				// FIXME - maybe add resources later if appropriate
	struct resource *res = pdev->resource;
	unsigned long size = resource_size(res);
#endif

	ENTER();
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
