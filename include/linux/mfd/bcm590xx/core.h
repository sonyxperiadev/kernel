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
#include <linux/platform_device.h>

struct bcm590xx;

enum SLAVE_ID {
    SLAVE_ID0 = 0 ,
    SLAVE_ID1 = 1 , 
} ; 

/**
 * Data to be supplied by the platform to initialise the BCM590XX.
 *
 * @init: Function called during driver initialisation.  Should be
 *        used by the platform to configure GPIO functions and similar.
 */
struct bcm590xx_platform_data {
    int (*init)(struct bcm590xx *bcm590xx);
    struct bcm590xx_battery_pdata *battery_pdata ;
    int slave ; 
};

struct bcm590xx {
    struct device *dev;
    struct i2c_client *i2c_client_0;
    struct i2c_client *i2c_client_1;
    s32 (*read_dev)(struct bcm590xx *bcm590xx, enum SLAVE_ID slave, u8 reg );
    s32 (*write_dev)(struct bcm590xx *bcm590xx, enum SLAVE_ID slave, u8 reg, u8 val);
	
    /* Interrupt handling */
    struct mutex list_lock;
    int irq;
    struct list_head irq_handlers;
    struct work_struct work;

    /* Client devices */
    struct bcm590xx_pmic pmic;
    struct bcm590xx_platform_data  *pdata ;
};

/*
 * BCM590XX device initialisation and exit.
 */
int bcm590xx_device_init(struct bcm590xx *bcm590xx, int irq, struct bcm590xx_platform_data *pdata);
void bcm590xx_device_exit(struct bcm590xx *bcm590xx);

/*
 * BCM590XX device IO
 */
int bcm590xx_request_irq(struct bcm590xx *bcm590xx, int irq, bool enable_irq,
			 void (*handler) (int, void *), void *data) ;
int bcm590xx_free_irq(struct bcm590xx *bcm590xx, int irq);

s32 bcm590xx_reg_read(enum SLAVE_ID slave , u8 reg);
s32 bcm590xx_reg_write(enum SLAVE_ID slave , u8 reg, u8 val);

int bcm59055_initialize_charging( struct bcm590xx *bcm59055 ) ;
int bcm59055_start_charging(struct bcm590xx *bcm59055 ) ;

void bcm590xx_shutdown(void);

#endif
