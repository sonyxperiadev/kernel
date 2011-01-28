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

#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/mfd/core.h>

#ifdef CONFIG_REGULATOR_BCM_PMU59055_A0
#include <linux/mfd/bcm590xx/bcm59055_A0.h>
#endif

#define IRQ_TO_REG_INX(irq)  ((irq)/8)
#define IRQ_TO_REG_BIT(irq)  ((irq) % 8)

/*
 * BCM590XX Device IO
 */
static DEFINE_MUTEX(io_mutex);

struct bcm590xx *g_bcm590xx_slave1 ; 

static int bcm590xx_read(struct bcm590xx *bcm590xx, u8 reg)
{
	return bcm590xx->read_dev(bcm590xx, reg);
}

static int bcm590xx_write(struct bcm590xx *bcm590xx, u8 reg, int num_regs, u16 val)
{
	/* Actually write it out */
	return bcm590xx->write_dev(bcm590xx, reg, num_regs, (char)val);
}

int bcm590xx_reg_read_slave1(int reg)
{
    return(bcm590xx_reg_read(g_bcm590xx_slave1, reg) ) ;
}

int bcm590xx_reg_write_slave1(int reg, u16 val)
{
    return(bcm590xx_reg_write(g_bcm590xx_slave1, reg, val) ) ;
}

int bcm590xx_reg_read(struct bcm590xx *bcm590xx, int reg)
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

static	void bcm59055_rd_cl_dis_intrs( struct bcm590xx *bcm590xx )
{
    unsigned int i = 0 ;
    unsigned int temp = 0 ;
    /*Read & clear all interrupts */
    for (i = 0; i < BCM59055_MAX_INT_REGS; i++) {
        temp = bcm590xx_reg_read(bcm590xx, (BCM59055_REG_INT1 + i));
		// Start charging if charger is plugged in at the time of booting.
        if ( ( i == 1 ) && ( temp & 1 ) )
		{
           bcm59055_initialize_charging(bcm590xx ) ;
           bcm59055_start_charging(bcm590xx ) ;
		}
    }
    /*mask all interrupts */
    for (i = 0; i <= BCM59055_MAX_INTMASK_REGS; i++) {
        bcm590xx_reg_write(bcm590xx, (BCM59055_REG_INT1MSK + i ), 0xFF );
    }
}

static struct bcm_pmu_irq *bcm590xx_find_irq_handler(struct bcm590xx *bcm590xx, int irq)
{
	struct bcm_pmu_irq *p;
	struct bcm_pmu_irq *match = NULL;

	list_for_each_entry(p, &bcm590xx->irq_handlers, node) {
		if (p->irq == irq) {
			match = p;
			break;
		}
	}
	return match;
}

int bcm590xx_disable_irq(struct bcm590xx *bcm590xx, int irq)
{
	int regInx;
	int st;
	u8 reg_val;
	struct bcm_pmu_irq *handler;

	printk(" Inside %s\n", __FUNCTION__);

	if (irq < 0 || irq > BCM59055_TOTAL_IRQ)
		return -EINVAL;

	regInx = IRQ_TO_REG_INX(irq);

	st = bcm590xx_reg_read(bcm590xx, regInx + BCM59055_INT_MASK_REG_BASE);
	if (reg_val < 0) {
		printk("bcm590xx_disable_irq : PMU reg read error !!!\n");
		return st;
	}
	reg_val = st ;
	reg_val |= (1 << IRQ_TO_REG_BIT(irq));
	st = bcm590xx_reg_write(bcm590xx, regInx + BCM59055_INT_MASK_REG_BASE, reg_val);
	if (st < 0) {
		printk( "bcm590xx_disable_irq : PMU reg write error !!!\n");
		return st;
	}

	handler = bcm590xx_find_irq_handler(bcm590xx, irq);
	if (handler) {
		handler->irq_enabled = false; 
	}
	return 0;
}

EXPORT_SYMBOL(bcm590xx_disable_irq);

int bcm590xx_enable_irq(struct bcm590xx *bcm590xx, int irq)
{
	int regInx;
	int st;
	u8 reg_val;
	struct bcm_pmu_irq *handler;

	printk(" Inside %s, enabling irq %d \n", __FUNCTION__, irq );

	if (irq < 0 || irq > BCM59055_TOTAL_IRQ)
		return -EINVAL;

	regInx = IRQ_TO_REG_INX(irq);

	st = bcm590xx_reg_read(bcm590xx, regInx + BCM59055_INT_MASK_REG_BASE);
	if (reg_val < 0) {
		printk( "bcm590xx_enable_irq : PMU reg read error !!!\n");
		return st;
	}
	reg_val = st ;
	reg_val &= ~(1 << IRQ_TO_REG_BIT(irq));
	st = bcm590xx_reg_write(bcm590xx, regInx + BCM59055_INT_MASK_REG_BASE, reg_val);
	if (st < 0) {
		printk( "bcm590xx_enable_irq : PMU reg write error !!!\n");
		return st;
	}

	handler = bcm590xx_find_irq_handler(bcm590xx, irq);
	if (!handler) {
		printk( "bcm590xx_enable_irq : Enabling PMU irq without registering handler!!!\n");
	} else {
		handler->irq_enabled = true;
	}
	return 0;
}
EXPORT_SYMBOL(bcm590xx_enable_irq);

int bcm590xx_request_irq(struct bcm590xx *bcm590xx, int irq, bool enable_irq,
			 void (*handler) (int, void *), void *data)
{
	struct bcm_pmu_irq *irq_info;
	 printk(" Inside %s Interrupt no. 0x%x\n", __FUNCTION__, irq);
	if (irq < 0 || irq >= BCM59055_TOTAL_IRQ || !handler)
		return -EINVAL;
	if (WARN_ON(bcm590xx_find_irq_handler(bcm590xx, irq))) {
		printk( " %s: handler for irq : %x already registered !!!\n",
			__FUNCTION__, irq);
		return -EBUSY;
	}

	irq_info = kzalloc(sizeof(struct bcm_pmu_irq), GFP_KERNEL);
	if (!irq_info)
		return -ENOMEM;

	irq_info->handler = handler;
	irq_info->data = data;
	irq_info->irq_enabled = enable_irq;
	irq_info->irq = irq;

	mutex_lock(&bcm590xx->list_lock);
	list_add(&irq_info->node, &bcm590xx->irq_handlers);
	mutex_unlock(&bcm590xx->list_lock);

	enable_irq ? bcm590xx_enable_irq(bcm590xx, irq) : bcm590xx_disable_irq(bcm590xx, irq);
	return 0;
}
EXPORT_SYMBOL(bcm590xx_request_irq);

int bcm590xx_free_irq(struct bcm590xx *bcm590xx, int irq)
{
	struct bcm_pmu_irq *irq_info;
	printk(" Inside %s\n", __FUNCTION__);
	if (irq < 0 || irq >= BCM59055_TOTAL_IRQ)
		return -EINVAL;
	irq_info = bcm590xx_find_irq_handler(bcm590xx, irq);
	if (irq_info) {
		mutex_lock(&bcm590xx->list_lock);
		list_del(&irq_info->node);
		mutex_unlock(&bcm590xx->list_lock);
		kfree(irq_info);
	}

	/* disalbe IRQ as there is no handler */
	bcm590xx_disable_irq(bcm590xx, irq);
	return 0;
}
EXPORT_SYMBOL(bcm590xx_free_irq);

static void bcm590xx_irq_workq(struct work_struct *work)
{
	struct bcm590xx *bcm590xx = container_of(work, struct bcm590xx, work);
	int i;
	u8 intStatus[BCM59055_MAX_INT_REGS];
	struct bcm_pmu_irq *handler;

	// printk(" Inside %s\n", __FUNCTION__);
	/* Read all interrupt status registers. All interrupt status registers are R&C */
	for (i = 0; i < BCM59055_MAX_INT_REGS; i++) {
            intStatus[i] = bcm590xx_reg_read (bcm590xx, BCM59055_INT_REG_BASE + i) ;
	}

	mutex_lock(&bcm590xx->list_lock);
	list_for_each_entry(handler, &bcm590xx->irq_handlers, node) {
		if (handler->irq_enabled &&
		    (intStatus[IRQ_TO_REG_INX(handler->irq)] &
		     (1 << IRQ_TO_REG_BIT(handler->irq)))) {
			handler->handler(handler->irq, handler->data);
		}
	}
	mutex_unlock(&bcm590xx->list_lock);

	enable_irq(bcm590xx->irq);
}

static irqreturn_t pmu_irq_handler(int irq, void *dev_id)
{
	struct bcm590xx *bcm590xx = dev_id;

	// printk(" Inside %s\n", __FUNCTION__);
	/* Read all interrupt status registers. All interrupt status registers are R&C */
	disable_irq_nosync(irq);
	schedule_work(&bcm590xx->work);
	return IRQ_HANDLED;
}

static int bcm590xx_client_dev_register(struct bcm590xx *bcm590xx, const char *name)
{
	struct mfd_cell cell = { };

	cell.name = name;
    return mfd_add_devices(bcm590xx->dev, -1, &cell, 1, NULL, 0);
}

int bcm590xx_device_init(struct bcm590xx *bcm590xx, int irq,
		       struct bcm590xx_platform_data *pdata)
{
	int ret;

	printk("REG: bcm590xx_device_init called \n") ;

    if ( pdata->slave == 0 )
	{
		// All PMU functionality ( MBCCTRL, INTRs, LDOs is present on slave 0
		// So initialize regulators, isrs etc here.
    	/* get BCM590XX revision and config mode */
    	ret = bcm590xx_reg_read(bcm590xx, BCM59055_REG_PMUID);

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

        INIT_LIST_HEAD(&bcm590xx->irq_handlers);
    	INIT_WORK(&bcm590xx->work, bcm590xx_irq_workq);
    	mutex_init(&bcm590xx->list_lock);

    	// Register IRQ.
    	if (irq > 0) {
    		ret = request_irq(irq, pmu_irq_handler, IRQF_TRIGGER_FALLING, "pmu 59055", bcm590xx);
    		if (ret) {
    			printk("can't get IRQ %d, ret %d\n", irq, ret);
    			goto err;
    		}
    	}

        /* init 59055 chip by reading and clearing INT registers */
    	disable_irq(irq);

    	/* Read, clear, and disable all the interrupts. */
    	bcm59055_rd_cl_dis_intrs(bcm590xx);
	
    	enable_irq(irq);
 
    	// Register battery device, so that battery probe function will be called.
        bcm590xx_client_dev_register(bcm590xx, "bcm59055-battery") ;
    }
	else
	{
        // Save i2c details to read and write from slave1 ( 0x0C ).
        g_bcm590xx_slave1 = bcm590xx ;
	}

	return 0;

err:
	// kfree(bcm590xx->reg_cache);
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
