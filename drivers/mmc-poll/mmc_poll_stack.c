/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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

#include <linux/kernel.h>
#include <linux/mmc-poll/mmc_poll.h>
#include <linux/mmc-poll/mmc_poll_stack.h>

#include "kona_mmc.h"

/* #define __MMC_POLL_INIT_DEBUG */

#ifdef __MMC_POLL_INIT_DEBUG
#define debug_print(a,b...) printk(a,##b)
#else
#define debug_print(a,b...)
#endif

int mmc_poll_stack_init (struct mmc **mmc, int *dev_num) 
{
	int status;

	if ((mmc == NULL) || (dev_num == NULL))
		return -1;

	mmc_initialize();

	/* For eMMC initilaize the instance number 2 and for SD Card use 1 */
	status = kona_mmc_init (2);
	if (status  < 0) {
		printk("rhearay_mmc_init failed \r\n");
		return -1;
	}
	debug_print("kona_mmc_init done \r\n");

	/* Use 0 for the first device registered via kona_mmc_init etc */
	*mmc = find_mmc_device (0);
	if (*mmc == NULL) {
		printk("No mmc device found\r\n");
		return -1;
	}

	if (mmc_init(*mmc) < 0) {
		printk("mmc init failed \r\n");
		return -1;
	}
	debug_print("mmc_init done \r\n");

	/* Use 0 for the first device registered via kona_mmc_init etc */
	*dev_num = 0;
	return 0;
}

static void print_mmcinfo(struct mmc *mmc)
{
	printk("Device: %s\n", mmc->name);
	printk("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	printk("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	printk("Name: %c%c%c%c%c \n", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);

	printk("Tran Speed: %d\n", mmc->tran_speed);
	printk("Rd Block Len: %d\n", mmc->read_bl_len);

	printk("%s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
			(mmc->version >> 4) & 0xf, mmc->version & 0xf);

	printk("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	printk("Capacity: %lld\n", mmc->capacity);

	printk("Bus Width: %d-bit\n", mmc->bus_width);
}
