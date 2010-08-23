/*
 *  linux/drivers/mmc/host/bcm2708_mci.c - Broadcom BCM2708 MCI driver
 *
 *  Copyright (C) 2010 Broadcom, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

struct clk;

#define BCM2708_MCI_COMMAND	0x00

#define BCM2708_MCI_READ     	(1 << 6)
#define BCM2708_MCI_WRITE    	(1 << 7)
#define BCM2708_MCI_LONGRESP 	(1 << 9)
#define BCM2708_MCI_NORESP   	(1 << 10)
#define BCM2708_MCI_BUSY     	(1 << 11)
#define BCM2708_MCI_ENABLE   	(1 << 15)

#define BCM2708_MCI_ARGUMENT	0x04

#define BCM2708_MCI_RESPONSE0	0x10
#define BCM2708_MCI_RESPONSE1	0x14
#define BCM2708_MCI_RESPONSE2	0x18
#define BCM2708_MCI_RESPONSE3	0x1c

#define BCM2708_MCI_STATUS	0x20

#define BCM2708_MCI_DATAFLAG	(1 << 0)
#define BCM2708_MCI_CMDTIMEOUT	(1 << 6)

#define BCM2708_MCI_DATA  	0x40

#define NR_SG		16

struct bcm2708_mci_host {
	void __iomem		*base;
	struct mmc_host		*mmc;

	int is_acmd;
};

static inline char *bcm2708_mci_kmap_atomic(struct scatterlist *sg, unsigned long *flags)
{
	local_irq_save(*flags);
	return kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
}

static inline void bcm2708_mci_kunmap_atomic(void *buffer, unsigned long *flags)
{
	kunmap_atomic(buffer, KM_BIO_SRC_IRQ);
	local_irq_restore(*flags);
}
