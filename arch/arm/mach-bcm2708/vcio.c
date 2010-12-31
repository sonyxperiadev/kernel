/*
 *  linux/arch/arm/mach-bcm2708/vcio.c
 *
 *  Copyright (C) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This device provides a shared mechanism for writing to the mailboxes,
 * semaphores, doorbells etc. that are shared between the ARM and the VideoCore
 * processor
 */

#if defined(CONFIG_SERIAL_BCM_MBOX_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/console.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <asm/io.h>

#include <mach/vcio.h>

#define DRIVER_NAME BCM_VCIO_DRIVER_NAME

/* ----------------------------------------------------------------------
 *      Mailbox
 * -------------------------------------------------------------------- */

// offsets from a mail box base address
#define MAIL_WRT  0x00 // write - and next 4 words
#define MAIL_RD   0x00 // read - and next 4 words
#define MAIL_POL  0x10 // read without popping the fifo
#define MAIL_SND  0x14 // sender ID (bottom two bits)
#define MAIL_STA  0x18 // status
#define MAIL_CNF  0x1C // configuration

#define MBOX_MSG(chan, data24) (((data24) & ~0xff) | ((chan) & 0xff))

#define MBOX_MAGIC 0xd0d0c0de

typedef struct mailbox_s
{
   struct device *dev;     // parent device (in case it is useful one day)
   void __iomem *status;
   void __iomem *write;
   uint32_t magic;
} MAILBOX_T;

static void mbox_init(MAILBOX_T *mbox_out, struct device *dev,
                      uint32_t addr_mbox)
{
        mbox_out->dev    = dev;
        mbox_out->status = __io_address(addr_mbox + MAIL_STA);
        mbox_out->write  = __io_address(addr_mbox + MAIL_WRT);
        mbox_out->magic  = MBOX_MAGIC;
}


static int mbox_write(MAILBOX_T *mbox, unsigned chan, uint32_t data25)
{
        int rc;

        if (mbox->magic != MBOX_MAGIC)
                rc = -EINVAL;
        else
        {
                // wait for the mailbox FIFO to have some space in it
                while (0 != (readl(mbox->status) & (1<<31)))
                        continue;

                writel(MBOX_MSG(chan, data25), mbox->write);
                rc = 0;
        }
        return rc;
}



/* ----------------------------------------------------------------------
 *      Mailbox Methods
 * -------e------------------------------------------------------------- */

static struct device *mbox_dev = NULL; // we assume there's only one!

static int dev_mbox_write(struct device *dev, unsigned chan, uint32_t data25)
{
        int rc;

        if (mbox_dev == NULL)
                rc = -ENODEV;
        else
        {
                MAILBOX_T *mailbox = dev_get_drvdata(dev);
                device_lock(dev);
                rc = mbox_write(mailbox, chan, data25);
                device_unlock(dev);
        }
        return rc;
}

extern int bcm_mailbox_write(unsigned chan, uint32_t data25)
{
        return dev_mbox_write(mbox_dev, chan, data25);
}

EXPORT_SYMBOL_GPL(bcm_mailbox_write);


static void dev_mbox_register(const char *dev_name, struct device *dev)
{
        mbox_dev = dev;
}


/* ----------------------------------------------------------------------
 *      Platform Device for Mailbox
 * -------e------------------------------------------------------------- */

static int bcm_vcio_probe(struct platform_device *pdev)
{
	int ret = 0;
	MAILBOX_T *mailbox;

	mailbox = kzalloc(sizeof(*mailbox), GFP_KERNEL);
	if (NULL == mailbox) {
                printk(KERN_ERR DRIVER_NAME ": failed to allocate "
                       "mailbox memory\n");
		ret = -ENOMEM;
        } else
        {
        	struct resource *res;
                
                res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
                if (res == NULL) {
                        printk(KERN_ERR DRIVER_NAME ": failed to obtain memory "
                               "resource\n");
                        ret = -ENODEV;
                        kfree(mailbox);
                } else
                {
                        // should be based on the registers from res really
                        mbox_init(mailbox, &pdev->dev, ARM_0_MAIL1_WRT);

                        platform_set_drvdata(pdev, mailbox);
                        dev_mbox_register(DRIVER_NAME, &pdev->dev);
                        printk(KERN_INFO DRIVER_NAME": mailbox at %p\n",
                               __io_address(ARM_0_MAIL1_WRT));
                }
        }
	return ret;
}

static int bcm_vcio_remove(struct platform_device *pdev)
{
        MAILBOX_T *mailbox = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	kfree(mailbox);

	return 0;
}

static struct platform_driver bcm_mbox_driver = {
	.probe		= bcm_vcio_probe,
	.remove		= bcm_vcio_remove,

	.driver		= {
                .name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init bcm_mbox_init(void)
{
	int ret;

	printk(KERN_INFO "mailbox: Broadcom VideoCore Mailbox driver\n");

        ret = platform_driver_register(&bcm_mbox_driver);
        if (ret != 0) {
                printk(KERN_ERR DRIVER_NAME ": failed to register "
                       "on platform\n");
        } else
                printk(KERN_INFO DRIVER_NAME ": registered mailbox\n");

        return ret;
}

static void __exit bcm_mbox_exit(void)
{
	platform_driver_unregister(&bcm_mbox_driver);
}


//module_init(bcm_mbox_init);
arch_initcall(bcm_mbox_init); // Initialize early
module_exit(bcm_mbox_exit);

MODULE_AUTHOR("Gray Girling");
MODULE_DESCRIPTION("ARM I/O to VideoCore processor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcm-mbox");

