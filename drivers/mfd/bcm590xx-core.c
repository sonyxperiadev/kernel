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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#include <linux/mfd/bcm590xx/core.h>
#include <linux/mfd/bcm590xx/pmic.h>

#ifdef CONFIG_REGULATOR_BCM_PMU59055_A0
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#endif

/*
 * BCM590XX Device IO
 */
static DEFINE_MUTEX(io_mutex);

static int bcm590xx_read(struct bcm590xx *bcm590xx, u8 reg)
{
	return bcm590xx->read_dev(bcm590xx, reg);
}

static int bcm590xx_write(struct bcm590xx *bcm590xx, u8 reg, int num_regs, u16 val)
{
	/* Actually write it out */
	return bcm590xx->write_dev(bcm590xx, reg, num_regs, (char)val);
}

u16 bcm590xx_reg_read(struct bcm590xx *bcm590xx, int reg)
{
	int err;
	mutex_lock(&io_mutex);
	err = bcm590xx_read(bcm590xx, reg);
	// printk("RRRRRRR Read to regi_addr = 0x%x , return = 0x%x \n", reg, err ) ;
	mutex_unlock(&io_mutex);
	return err ;

}
EXPORT_SYMBOL_GPL(bcm590xx_reg_read);

int bcm590xx_reg_write(struct bcm590xx *bcm590xx, int reg, u16 val)
{
	int ret;

	mutex_lock(&io_mutex);
	ret = bcm590xx_write(bcm590xx, reg, 1, val);
	// printk("Wrote to regi_addr = 0x%x value is = 0x%x , return = %d \n", reg, val, ret ) ;
	if (ret)
		dev_err(bcm590xx->dev, "write to reg R%d failed\n", reg);
	mutex_unlock(&io_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(bcm590xx_reg_write);

int bcm590xx_device_init(struct bcm590xx *bcm590xx, int irq,
		       struct bcm590xx_platform_data *pdata)
{
	int ret;

	printk("REG: bcm590xx_device_init called \n") ;

	/* get BCM590XX revision and config mode */
	ret = bcm590xx->read_dev(bcm590xx, BCM59055_REG_PMUID);


	if (ret < 0) {
		dev_err(bcm590xx->dev, "Failed to read ID: %d\n", ret);
		goto err;
	}
	else
	{
        printk("BCM590XX: Chip Version [0x%x]\n", ret);
	}

	if (pdata && pdata->init) {
		ret = pdata->init(bcm590xx);
		if (ret != 0) {
			dev_err(bcm590xx->dev, "Platform init() failed: %d\n",
				ret);
			goto err;
		}
	}

	return 0;

err:
	kfree(bcm590xx->reg_cache);
	return ret;
}
EXPORT_SYMBOL_GPL(bcm590xx_device_init);

void bcm590xx_device_exit(struct bcm590xx *bcm590xx)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(bcm590xx->pmic.pdev); i++)
		platform_device_unregister(bcm590xx->pmic.pdev[i]);

}
EXPORT_SYMBOL_GPL(bcm590xx_device_exit);

MODULE_DESCRIPTION("BCM590XX PMIC core driver");
MODULE_LICENSE("GPL");
