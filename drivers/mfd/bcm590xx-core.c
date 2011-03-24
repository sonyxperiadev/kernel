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
#include <linux/irq.h>
#include <asm/gpio.h> 
#include <linux/mfd/bcm590xx/bcm59055_A0.h>

#define IRQ_TO_REG_INX(irq)  ((irq)/8)
#define IRQ_TO_REG_BIT(irq)  ((irq) % 8)
#define PMU_IRQ_PIN           10

static void bcm59055_rd_cl_dis_intrs( struct bcm590xx *bcm590xx )
{
    unsigned int i = 0 ;
    unsigned int temp = 0 ;
    /*Read & clear all interrupts */
    for (i = 0; i < BCM59055_MAX_INT_REGS; i++) {
        // temp = bcm590xx_reg_read(bcm590xx, (BCM59055_REG_INT1 + i));
        temp = bcm590xx_reg_read(SLAVE_ID0, (BCM59055_REG_INT1 + i));
#ifdef CONFIG_BATTERY_BCM59055
        // Start charging if charger is plugged in at the time of booting.
        if ( ( i == 1 ) && ( temp & 1 ) ) {
            bcm59055_initialize_charging(bcm590xx ) ;
            bcm59055_start_charging(bcm590xx ) ;
        }
#endif
    }
    /*mask all interrupts */
    for (i = 0; i <= BCM59055_MAX_INTMASK_REGS; i++) 
        bcm590xx_reg_write(SLAVE_ID0, (BCM59055_REG_INT1MSK + i ), 0xFF );
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

    if (irq < 0 || irq > BCM59055_TOTAL_IRQ)
        return -EINVAL;
    regInx = IRQ_TO_REG_INX(irq);

    st = bcm590xx_reg_read(SLAVE_ID0, regInx + BCM59055_INT_MASK_REG_BASE);
    if (st < 0) {
        printk(KERN_ERR "%s bcm590xx_disable_irq : PMU reg read error %d !!!\n", __FUNCTION__, (regInx + BCM59055_INT_MASK_REG_BASE));
        return st;
    }
    reg_val = st ;
    reg_val |= (1 << IRQ_TO_REG_BIT(irq));
    
    st = bcm590xx_reg_write(SLAVE_ID0, regInx + BCM59055_INT_MASK_REG_BASE, reg_val);
    if (st < 0) {
        printk(KERN_ERR "%s bcm590xx_disable_irq : PMU reg write error %d !!!\n", __FUNCTION__, ( regInx + BCM59055_INT_MASK_REG_BASE) );
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

    printk(KERN_INFO " Inside %s, enabling irq %d \n", __FUNCTION__, irq );

    if (irq < 0 || irq > BCM59055_TOTAL_IRQ)
        return -EINVAL;

    regInx = IRQ_TO_REG_INX(irq);

    st = bcm590xx_reg_read(SLAVE_ID0, regInx + BCM59055_INT_MASK_REG_BASE);
    if (st < 0) {
        printk( KERN_ERR "%s bcm590xx_enable_irq : PMU reg read error %d !!!\n", __FUNCTION__, (regInx + BCM59055_INT_MASK_REG_BASE) );
        return st;
    }
    reg_val = st ;
    reg_val &= ~(1 << IRQ_TO_REG_BIT(irq));

    st = bcm590xx_reg_write(SLAVE_ID0, regInx + BCM59055_INT_MASK_REG_BASE, reg_val);
    if (st < 0) {
        printk( KERN_ERR "%s bcm590xx_enable_irq : PMU reg write error %d  !!!\n", __FUNCTION__, (regInx + BCM59055_INT_MASK_REG_BASE) );
        return st;
    }

    handler = bcm590xx_find_irq_handler(bcm590xx, irq);
    if (!handler) 
        printk( KERN_ERR "%s bcm590xx_enable_irq : Enabling PMU irq %d without registering handler!!!\n", __FUNCTION__, irq );
    else 
        handler->irq_enabled = true;
    return 0;
}
EXPORT_SYMBOL(bcm590xx_enable_irq);

int bcm590xx_request_irq(struct bcm590xx *bcm590xx, int irq, bool enable_irq, void (*handler) (int, void *), void *data)
{
    struct bcm_pmu_irq *irq_info;
    printk(KERN_INFO " Inside %s Interrupt no. 0x%x\n", __FUNCTION__, irq);
    if (irq < 0 || irq >= BCM59055_TOTAL_IRQ || !handler)
        return -EINVAL;
    if (WARN_ON(bcm590xx_find_irq_handler(bcm590xx, irq))) {
        printk(KERN_ERR  " %s: handler for irq : %x already registered !!!\n", __FUNCTION__, irq);
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
   printk(KERN_INFO " Inside %s\n", __FUNCTION__);
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

    /* Read all interrupt status registers. All interrupt status registers are R&C */
    for (i = 0; i < BCM59055_MAX_INT_REGS; i++) {
        intStatus[i] = bcm590xx_reg_read (SLAVE_ID0, BCM59055_INT_REG_BASE + i) ;
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

void bcm590xx_shutdown(void)
{
    int host_ctrl1;
	
    host_ctrl1 = bcm590xx_reg_read(SLAVE_ID0, BCM59055_REG_HOSTCTRL1);
    host_ctrl1 |= 1 << BCM59055_REG_HOSTCTRL1_SHDWN_OFFSET;
    bcm590xx_reg_write(SLAVE_ID0, BCM59055_REG_HOSTCTRL1, (u8)host_ctrl1);
}

EXPORT_SYMBOL_GPL(bcm590xx_shutdown);


int bcm590xx_device_init(struct bcm590xx *bcm590xx, int irq,
		       struct bcm590xx_platform_data *pdata)
{
    int ret = 0 ;

    printk(KERN_INFO "%s REG: bcm590xx_device_init called \n", __FUNCTION__ ) ;

    // All PMU functionality ( MBCCTRL, INTRs, LDOs is present on slave 0
    // So initialize regulators, isrs etc here.
    ret = bcm590xx_reg_read(SLAVE_ID0, BCM59055_REG_PMUID);
    if (ret < 0) {
        printk(KERN_ERR "%s Failed to read ID: %d\n", __FUNCTION__, ret);
        goto err;
    }
    else
        printk(KERN_INFO "%s BCM590XX: Chip Version [0x%x]\n", __FUNCTION__, ret);

    if (pdata && pdata->init) {
        ret = pdata->init(bcm590xx);
        if (ret != 0) {
            printk(KERN_ERR "%s Platform init() failed: %d\n", __FUNCTION__, ret);
            goto err;
        }
    }

    INIT_LIST_HEAD(&bcm590xx->irq_handlers);
    INIT_WORK(&bcm590xx->work, bcm590xx_irq_workq);
    mutex_init(&bcm590xx->list_lock);

    // Setup GPIO properties for interrupt from PMU.
    ret = set_irq_type(irq, IRQ_TYPE_EDGE_FALLING);
    if (ret < 0) {
        printk(KERN_ERR "%s set_irq_type failed with irq %d\n", __FUNCTION__, irq);
        return ret ;
    }
    ret = gpio_request(PMU_IRQ_PIN, "pmu_pen_down");
    if (ret < 0) {
        printk(KERN_ERR "%s unable to request GPIO pin %d\n", __FUNCTION__, PMU_IRQ_PIN);
        return ret ;
    }
    gpio_direction_input(PMU_IRQ_PIN);

    // Register IRQ.
    if (irq > 0) {
        ret = request_irq(irq, pmu_irq_handler, IRQF_TRIGGER_FALLING, "pmu 59055", bcm590xx);
        if (ret) {
            printk(KERN_ERR "%s can't get IRQ %d, ret %d\n", __FUNCTION__, irq, ret);
            goto err;
        }
    }

    disable_irq(irq);

    /* Read, clear, and disable all the interrupts. */
    bcm59055_rd_cl_dis_intrs(bcm590xx);

    enable_irq(irq);

    // Register battery device, so that battery probe function will be called.
    ret = bcm590xx_client_dev_register(bcm590xx, "bcm59055-battery") ;
    if (ret < 0) {
        printk(KERN_ERR "%s client_dev_register for bcm59055-battery failed %d\n", __FUNCTION__, ret);
        return ret ;
    }

    /* Register PowerOnKey device */
    ret = bcm590xx_client_dev_register(bcm590xx, "bcm59055-onkey") ;
    if (ret < 0) {
        printk(KERN_ERR "%s client_dev_register for bcm59055-battery failed %d\n", __FUNCTION__, ret);
        return ret ;
    }

    return 0;

err:
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
