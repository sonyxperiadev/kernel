/*
 *  linux/include/linux/mtd/bcmnand.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_MTD_BCMNAND_H
#define __LINUX_MTD_BCMNAND_H

#include <chal/chal_nand.h>

/**
 * struct bcmnand_hw_control - Control structure for hardware controller (e.g ECC generator) shared among independent devices
 * @lock:               protection lock
 * @active:		the mtd device which holds the controller currently
 * @wq:			wait queue to sleep on if a NAND operation is in progress
 *                      used instead of the per chip wait queue when a hw controller is available
 */
struct bcmnand_hw_control {
	spinlock_t	 lock;
	struct bcmnand_chip *active;
	wait_queue_head_t wq;
};


/**
 * struct bcmnand_chip - BCMNAND Private Flash Chip Data
 */
struct bcmnand_chip {
	chal_nand_info_t chal_nand;
	int page_shift;
	int phys_erase_shift;
	int chip_shift;
	int numchips;
	uint64_t chipsize;
	int pagemask;
	int pagebuf;
	int subpagesize;

	flstate_t state;

	uint8_t *oob_poi;
	struct bcmnand_hw_control  *controller;

	struct nand_ecc_ctrl ecc;
	struct nand_buffers *buffers;
#ifdef CONFIG_MTD_BCMNAND_VERIFY_WRITE
	char *verifybuf;
#endif
	struct mtd_oob_ops ops;
	void		*priv;
};

#endif	/* __LINUX_MTD_BCMNAND_H */
