/*
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
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <mach/input_server_cfg.h>

#define DRIVER_NAME "BCM2835 Input Driver" 

struct bcm_input_device {
	struct input_dev 	*input_dev;
	u32			dev_type;
	u32			irq;
	void __iomem		*base_addr;
	u32			fifo_offset;
};

static irqreturn_t bcm2835_input_isr(int irq, void *dev_id)
{
	struct bcm_input_device *bcm_input = (struct bcm_input_device *)dev_id;
	
	volatile input_fifo_t *fifo_p = 
				(volatile input_fifo_t *)((u32)bcm_input->base_addr + bcm_input->fifo_offset);
	u32			fifo_size = fifo_p->element_count;	

	BUG_ON(fifo_p->magic != ((INPUT_MAGIC_HEADER<<16) + INPUT_REMOTE_ID));

	while (fifo_p->index_in != fifo_p->index_out) {
		
		printk(KERN_ERR DRIVER_NAME ":get the key with type:%d code: %d\n",
			 fifo_p->data[fifo_p->index_out].type, fifo_p->data[fifo_p->index_out].code);
 
		input_report_key(bcm_input->input_dev, fifo_p->data[fifo_p->index_out].code, 0);

		fifo_p->index_out = (fifo_p->index_out + 1) % fifo_size;
	}
	
	return IRQ_HANDLED;
}	

static int bcm2835_input_probe(struct platform_device *pdev)
{
	int ret = 0, cnt;
	struct bcm_input_device *bcm_input;
	struct resource *iomem, *irq;

	BUG_ON(pdev == NULL);

	bcm_input = kzalloc(sizeof(*bcm_input), GFP_KERNEL);
	if (NULL == bcm_input) {
                printk(KERN_ERR DRIVER_NAME ": failed to allocate "
                       "input device memory\n");
		return -ENOMEM;
        }
        platform_set_drvdata(pdev, bcm_input);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (iomem == NULL) {
		printk(KERN_ERR DRIVER_NAME ": failed to obtain platform io memory resource\n");
                ret = -ENODEV;
		goto no_io_resource;
        } 
	bcm_input->base_addr = (void __iomem *)iomem->start;

	irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (irq == NULL) {
               printk(KERN_ERR DRIVER_NAME ": failed to obtain platform irq resource\n");
               ret = -ENODEV;
               goto no_io_resource;
        } 
	bcm_input->irq = irq->start;
        
	if (NULL == (bcm_input->input_dev = input_allocate_device())) {
		printk(KERN_ERR DRIVER_NAME ": failed to allocate input device\n");
		ret = -ENOMEM;
		goto no_inputdev_mem;
	}
	
	/* Get the input device info from the server side on VC */
	if (readl(bcm_input->base_addr) != ((INPUT_MAGIC_HEADER << 16) + INPUT_MAGIC_BASE) ) {
		printk(KERN_ERR DRIVER_NAME ": Input server is not ready yet!\n");
		ret = -EIO;
		goto no_inputdev_mem;
	}

	bcm_input->fifo_offset = ((input_directory_t *)bcm_input->base_addr)->entry[0].offset;
	BUG_ON(((input_directory_t *)bcm_input->base_addr)->entry[0].ident != INPUT_REMOTE_ID);

        set_bit(EV_KEY, bcm_input->input_dev->evbit);
        set_bit(EV_REP, bcm_input->input_dev->evbit);

	/* Create the keymap for this device */
	for (cnt = 0; cnt <= KEY_MAX; cnt++)
		set_bit(cnt, bcm_input->input_dev->keybit);

	bcm_input->input_dev->name = "ir";
	bcm_input->input_dev->phys = "ir/input0";		 	  
        bcm_input->input_dev->id.bustype = BUS_VIRTUAL;
	bcm_input->input_dev->id.vendor = 0x0001;
	bcm_input->input_dev->id.product = 0x0001;
	bcm_input->input_dev->id.version = 0x0100;
#if 0
	bcm_input->input_dev->keycodesize = sizeof(unsigned char);
	bcm_input->input_dev->keycodemax = KEY_MAX;
#endif

	ret = request_irq(bcm_input->irq, bcm2835_input_isr, IRQF_DISABLED, "input", bcm_input);
	if (ret) {
		printk(KERN_ERR DRIVER_NAME ":Unable to register ISR\n");
		goto fail_to_irq;
	} 

	ret = input_register_device(bcm_input->input_dev);
	if (ret) {
		printk(KERN_ERR DRIVER_NAME ":Unable to register input device");
		goto fail_to_register_input;
	}		 

	return 0;

fail_to_register_input:
	free_irq(bcm_input->irq, (void *)bcm_input);
fail_to_irq:
	input_free_device(bcm_input->input_dev);
no_inputdev_mem:
no_io_resource:
	kfree(bcm_input);
	return ret;
}

static int bcm2835_input_remove(struct platform_device *pdev)
{
        struct bcm_input_device *bcm_input = platform_get_drvdata(pdev);

        free_irq(bcm_input->irq, (void *)bcm_input);
	platform_set_drvdata(pdev, NULL);
        input_unregister_device(bcm_input->input_dev);
        input_free_device(bcm_input->input_dev);
	kfree(bcm_input);

	return 0;
}

static struct platform_driver bcm2835_input_driver = {
	.probe		= bcm2835_input_probe,
	.remove		= bcm2835_input_remove,
	.suspend	= NULL,
	.resume		= NULL,
	.driver		= {
                .name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init bcm2835_input_init(void)
{
	int ret;

        ret = platform_driver_register(&bcm2835_input_driver);
        if (ret != 0) {
                printk(KERN_ERR DRIVER_NAME ": failed to register "
                       "on platform\n");
        } else
                printk(KERN_INFO DRIVER_NAME ": is registered\n");

        return ret;
}

static void __exit bcm2835_input_exit(void)
{
	platform_driver_unregister(&bcm2835_input_driver);
}


module_init(bcm2835_input_init);
module_exit(bcm2835_input_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("BCM2835 Input Device");
MODULE_LICENSE("GPL");
