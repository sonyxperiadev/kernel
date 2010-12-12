/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

#ifndef __LINUX_MFD_BCM590XX_CORE_H_
#define __LINUX_MFD_BCM590XX_CORE_H_

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/mfd/bcm590xx/pmic.h>

struct bcm590xx;

/* Define irq structure here. */

struct bcm590xx {
	struct device *dev;

    struct i2c_client *i2c_client;
	int (*read_dev)(struct bcm590xx *bcm590xx, char reg );
	int (*write_dev)(struct bcm590xx *bcm590xx, char reg, int size, u8 val);
	u16 *reg_cache;

	/* Interrupt handling */

	/* Client devices */
	struct bcm590xx_pmic pmic;
};

/**
 * Data to be supplied by the platform to initialise the BCM590XX.
 *
 * @init: Function called during driver initialisation.  Should be
 *        used by the platform to configure GPIO functions and similar.
 */
struct bcm590xx_platform_data {
	int (*init)(struct bcm590xx *bcm590xx);
};


/*
 * BCM590XX device initialisation and exit.
 */
int bcm590xx_device_init(struct bcm590xx *bcm590xx, int irq, struct bcm590xx_platform_data *pdata);
void bcm590xx_device_exit(struct bcm590xx *bcm590xx);

/*
 * BCM590XX device IO
 */
u16 bcm590xx_reg_read(struct bcm590xx *bcm590xx, int reg);
int bcm590xx_reg_write(struct bcm590xx *bcm590xx, int reg, u16 val);
#endif
