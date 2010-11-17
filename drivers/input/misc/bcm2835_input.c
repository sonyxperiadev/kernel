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
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>

#include <asm/uaccess.h>

#include <mach/memory.h>
#include <mach/irqs.h>
#include <mach/ipc.h>
#include <mach/input_ipc_cfg.h>

#define DEBUG_BCM2835_INPUT_DRIVER 0

#define IPC_BASE        __io_address(IPC_BLOCK_BASE)
#define DRIVER_NAME "bcm2835_INPT"

struct bcm_input_device {
	struct input_dev 	*input_dev;
	u32			dev_type;
	u32			irq;
	void __iomem		*base_addr;
	u32			fifo_offset;
};

static struct bcm_input_device *g_bcm_input = NULL; /* purely for debug consumption. */

static struct proc_dir_entry *input_create_proc_entry( const char * const name,
                                                     read_proc_t *read_proc,
                                                     write_proc_t *write_proc )
{
   struct proc_dir_entry *ret = NULL;

   ret = create_proc_entry( name, 0644, NULL);

   if (ret == NULL)
   {
      remove_proc_entry( name, NULL);
      printk(KERN_ALERT "could not initialize %s", name );
   }
   else
   {
      ret->read_proc  = read_proc;
      ret->write_proc = write_proc;
      ret->mode           = S_IFREG | S_IRUGO;
      ret->uid    = 0;
      ret->gid    = 0;
      ret->size           = 37;
   }
   return ret;
}

static int input_dummy_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   int len = 0;

   if (offset > 0)
   {
      *eof = 1;
      return 0;
   }

   *eof = 1;

   return len;
}

#define INPUT_MAX_INPUT_STR_LENGTH   256

static int input_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char *init_string = NULL;
	volatile input_fifo_t *fifo_p =
		(volatile input_fifo_t *)((u32)g_bcm_input->base_addr + g_bcm_input->fifo_offset);
	u32 fifo_size = fifo_p->element_count;
	u32 i, key_array[] = 
		{KEY_HOME, KEY_HOME, KEY_BACK, KEY_BACK, KEY_DOWN, KEY_BACK, KEY_OK, KEY_OK, KEY_RIGHT, KEY_UP, KEY_FORWARD, KEY_PLAYPAUSE, KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_RESERVED};

   init_string = vmalloc(INPUT_MAX_INPUT_STR_LENGTH);

   if(NULL == init_string)
      return -EFAULT;

   memset(init_string, 0, INPUT_MAX_INPUT_STR_LENGTH);

   count = (count > INPUT_MAX_INPUT_STR_LENGTH) ? INPUT_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(init_string, buffer, count))
   {
      return -EFAULT;
   }
   init_string[ INPUT_MAX_INPUT_STR_LENGTH  - 1 ] = 0;

   i = 0;
   while (((fifo_p->index_in + 1) % fifo_size) != fifo_p->index_out) {
		fifo_p->data[fifo_p->index_in].type = EV_KEY;
		fifo_p->data[fifo_p->index_in].code = key_array[i];
		fifo_p->index_in = (fifo_p->index_in + 1) % fifo_size;
		if (key_array[i] == KEY_RESERVED)
			break;
		i++;
  }

   writel(0xff, IPC_BASE + IPC_VC_ARM_INTERRUPT_OFFSET);

   writel(0x1, IO_ADDRESS(ARM_0_BELL0));

   vfree(init_string);

   return count;
}

static irqreturn_t bcm2835_input_isr(int irq, void *dev_id)
{
	struct bcm_input_device *bcm_input = (struct bcm_input_device *)dev_id;
	
	volatile input_fifo_t *fifo_p = 
		(volatile input_fifo_t *)((u32)bcm_input->base_addr + bcm_input->fifo_offset);
	u32 fifo_size = fifo_p->element_count;

	while (fifo_p->index_in != fifo_p->index_out) {

#if DEBUG_BCM2835_INPUT_DRIVER	
		printk(KERN_ERR DRIVER_NAME ":get the key with type:%d code: %d\n", fifo_p->data[fifo_p->index_out].type, fifo_p->data[fifo_p->index_out].code);
#endif
		
		/*
		 * Two things considered here:
		 * 	1. simulate the key press and release action with each event passed from VC side.
		 * 	2. VC side is passing KEY_REP type and is ignored by this driver.
		 */
		input_report_key(bcm_input->input_dev, fifo_p->data[fifo_p->index_out].code, 1);
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
	if (readl(bcm_input->base_addr) != ((INPUT_MAGIC_HEADER << 16) + INPUT_MAGIC_BASE)) {
		printk(KERN_ERR DRIVER_NAME ": Input server is not ready yet!\n");
		ret = -EIO;
		goto no_inputdev_mem;
	}

	bcm_input->fifo_offset = ((input_directory_t *)bcm_input->base_addr)->entry[0].offset;
	BUG_ON(((input_directory_t *)bcm_input->base_addr)->entry[0].ident != INPUT_REMOTE_ID);

	BUG_ON(((input_fifo_t *)((u32)bcm_input->base_addr + bcm_input->fifo_offset))->magic
						!= ((INPUT_MAGIC_HEADER<<16) + INPUT_REMOTE_ID));

        set_bit(EV_KEY, bcm_input->input_dev->evbit);
	/*
 	 * Since the IR hardwrae on VC side is doing the auto-repeat handling, we do not enable
	 * the software auto-repeat capability.
 	 */
        /* set_bit(EV_REP, bcm_input->input_dev->evbit); */

	/* Create the keymap for this device, right now we will just enable every key value */
	for (cnt = 0; cnt <= KEY_MAX; cnt++)
		set_bit(cnt, bcm_input->input_dev->keybit);

	bcm_input->input_dev->name = " Broadcom IR driver";
	bcm_input->input_dev->phys = "ir";
        bcm_input->input_dev->id.bustype = BUS_VIRTUAL;
	bcm_input->input_dev->id.vendor = 0x0001;
	bcm_input->input_dev->id.product = 0x0001;
	bcm_input->input_dev->id.version = 0x0100;

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

#if DEBUG_BCM2835_INPUT_DRIVER
	printk(KERN_ERR DRIVER_NAME "The device has been probed\n");
#endif
        input_create_proc_entry("bcm2835_inpt", input_dummy_read, input_proc_write);
	g_bcm_input = bcm_input;

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
                printk(DRIVER_NAME ": is registered\n");

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
