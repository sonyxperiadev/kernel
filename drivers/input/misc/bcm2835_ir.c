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
#include <linux/miscdevice.h>
#include <linux/ipc/ipc.h>

#include <asm/uaccess.h>

#include <mach/memory.h>
#include <mach/irqs.h>
#include <mach/input_ipc_cfg.h>

#include "bcm2835_ir_remote.h"

#define BCM2835_INPUT_DEBUG 0

#define BCM2835_IR_DRIVER_NAME "bcm2835_INPT"
#define BCM2835_IR_MISC_DRIVER_NAME "bcm2835_ir_remote"

#define bcm2835_ir_error(format, arg...) \
        printk(KERN_ERR BCM2835_IR_DRIVER_NAME ": %s" format, __func__, ##arg)

#if BCM2835_INPUT_DEBUG
#define bcm2835_ir_dbg(format, arg...) bcm2835_ir_error(format, ##arg)
#else
#define bcm2835_ir_dbg(format, arg...) printk(KERN_DEBUG BCM2835_IR_DRIVER_NAME ": %s" format, __func__, ##arg)
#endif

struct bcm_input_device {
	u32			irq;
	void __iomem		*base_addr;
	u32			fifo_offset;
	spinlock_t		fifo_lock;
	struct semaphore	fifo_sem;
	u8			ioctl_cmd_buf[MAX_BCM2835_IR_REMOTE_IOCTL_CMD_SIZE];
};

static struct bcm_input_device *g_bcm_input = NULL;

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
	u32 ipc_base_phys, ipc_base;

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

  ipc_base_phys = readl(IO_ADDRESS(ARM_0_MAIL0_RD));
  ipc_base = (u32)ipc_bus_to_virt(ipc_base_phys);

  printk(KERN_ERR"ipc_phy_base and ipc_base = 0x%08x and 0x%08x\n", ipc_base_phys, ipc_base);

#if 0
   writel(0xff, ipc_base + IPC_VC_ARM_INTERRUPT_OFFSET);

   writel(0x1, IO_ADDRESS(ARM_0_BELL0));
#endif

   vfree(init_string);

   return count;
}

static irqreturn_t bcm2835_input_isr(int irq, void *dev_id)
{
	struct bcm_input_device *bcm_input = (struct bcm_input_device *)dev_id;

	up(&bcm_input->fifo_sem);
	
	return IRQ_HANDLED;
}

static int ir_open( struct inode *inode, struct file *file_id)
{
        return 0;
}

static int ir_release( struct inode *inode, struct file *file_id )
{
        int ret = 0;

        return ret;
}

static ssize_t ir_read( struct file *file, char *buffer, size_t count, loff_t *ppos )
{
        return -EINVAL;
}

static ssize_t ir_write( struct file *file, const char *buffer, size_t count, loff_t *ppos )
{
        return -EINVAL;
}

static inline int input_fifo_empty(volatile input_fifo_t *fifo_p)
{
	return (fifo_p->index_in == fifo_p->index_out);
}

static inline int input_fifo_get(volatile input_fifo_t *fifo_p, bcm2835_ir_remote_event_t *element)
{
	if (!input_fifo_empty(fifo_p)) {

		element->type = fifo_p->data[fifo_p->index_out].type;
		element->code = fifo_p->data[fifo_p->index_out].code;
		fifo_p->index_out = (fifo_p->index_out + 1) % fifo_p->element_count;

		return 0;	
	} else {
		return -1;
	}	
}

static int read_event(bcm2835_ir_remote_event_t *entry)
{
	unsigned long flags;
	int ret;
	volatile input_fifo_t *fifo_p = (volatile input_fifo_t *)((u32)g_bcm_input->base_addr + g_bcm_input->fifo_offset);

	do {
                spin_lock_irqsave(&g_bcm_input->fifo_lock, flags);
                if (!input_fifo_empty(fifo_p)) {
                        ret = input_fifo_get(fifo_p, entry);
			bcm2835_ir_dbg("entry got type=0x%08x code=0x%08x\n", 
				entry->type, entry->code);
                        BUG_ON(-1 == ret);
                        spin_unlock_irqrestore(&g_bcm_input->fifo_lock, flags);
			break;
                } else {
                        spin_unlock_irqrestore(&g_bcm_input->fifo_lock, flags);
                        ret = down_interruptible(&g_bcm_input->fifo_sem);
                        if (ret < 0)
                                return -ERESTARTSYS;
                        else {
                                continue;
                        }
                }
	} while (1);

	return 0;
}

static int ir_ioctl( struct inode *inode, struct file *file_id, unsigned int cmd, unsigned long arg )
{
        int ret = 0;
        unsigned long uncopied;

        BUG_ON(MAX_BCM2835_IR_REMOTE_IOCTL_CMD_SIZE < _IOC_SIZE(cmd));

        bcm2835_ir_dbg("the ret value =%d and the direct is %s\n", ret, (IOC_OUT == _IOC_DIR(cmd))?"OUT":"IN");


        if ((_IOC_WRITE & _IOC_DIR(cmd)) && (0 != _IOC_SIZE(cmd))) {
                uncopied =
                        copy_from_user(g_bcm_input->ioctl_cmd_buf, (void *)arg, _IOC_SIZE(cmd));
                if (uncopied != 0)
                        return -EFAULT;
        }

        switch (cmd) {
        case IOCTL_IR_REMOTE_GET_EV:
                ret = read_event((bcm2835_ir_remote_event_t *)g_bcm_input->ioctl_cmd_buf);
		bcm2835_ir_dbg("the size of cmd is =0x%08x, and type= 0x%08x code = 0x%08x\n",
			_IOC_SIZE(cmd),
			((bcm2835_ir_remote_event_t *)g_bcm_input->ioctl_cmd_buf)->type,
			 ((bcm2835_ir_remote_event_t *)g_bcm_input->ioctl_cmd_buf)->code);
                break;

        default:
                bcm2835_ir_error("Wrong IOCTL cmd\n");
                ret = -EFAULT;
                break;
        }

	bcm2835_ir_dbg("the ret value =%d and the direct is %s\n", ret, (IOC_OUT == _IOC_DIR(cmd))?"OUT":"IN");

	if (ret < 0) {
		return ret;
	}
	else {
		if ((_IOC_READ & _IOC_DIR(cmd)) && (0 != _IOC_SIZE(cmd))) {
                	uncopied = 
                        	copy_to_user((void *)arg, g_bcm_input->ioctl_cmd_buf, _IOC_SIZE(cmd));
                	if (uncopied != 0)
                        	return -EFAULT;
        	}
	} 

        return ret;
}

static int ir_mmap(struct file *filp, struct vm_area_struct *vma)
{
        return 0;
}

static struct file_operations ir_file_ops =
{
    owner:      THIS_MODULE,
    open:       ir_open,
    release:    ir_release,
    read:       ir_read,
    write:      ir_write,
    ioctl:      ir_ioctl,
    mmap:       ir_mmap,
};

struct miscdevice ir_misc_dev = {
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     BCM2835_IR_MISC_DRIVER_NAME,
    .fops =     &ir_file_ops
};


static int bcm2835_input_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct bcm_input_device *bcm_input;
	struct resource *iomem, *irq;
	input_directory_t *input_dir;
	int num_input_src;

	BUG_ON(pdev == NULL);

	bcm_input = kzalloc(sizeof(*bcm_input), GFP_KERNEL);
	if (NULL == bcm_input) {
                bcm2835_ir_error(": failed to allocate input device memory\n");
		return -ENOMEM;
        }
        platform_set_drvdata(pdev, bcm_input);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (NULL == iomem) {
		bcm2835_ir_error(": failed to obtain platform io memory resource\n");
                ret = -ENODEV;
		goto no_io_resource;
        } 
	bcm_input->base_addr = (void __iomem *)iomem->start;

	irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (NULL == irq) {
               bcm2835_ir_error(": failed to obtain platform irq resource\n");
               ret = -ENODEV;
               goto no_io_resource;
        } 
	bcm_input->irq = irq->start;
       
        ret = misc_register(&ir_misc_dev);
        if (ret < 0) {
                bcm2835_ir_error("failed to register misc device\n");
                goto err_reg_chrdev;
        }
	dev_set_drvdata(ir_misc_dev.this_device, bcm_input);	
 
        /* Get the input device info from the server side on VC */
        if (readl(bcm_input->base_addr) != ((INPUT_MAGIC_HEADER << 16) + INPUT_MAGIC_BASE)) {
                bcm2835_ir_error(": Input server is not ready yet!\n");
                ret = -EIO;
                goto no_vc_inputdev;
        }

	input_dir = (input_directory_t *)bcm_input->base_addr;
	num_input_src = input_dir->entry_count;
	for (num_input_src--; num_input_src >= 0; num_input_src--) {
		if (input_dir->entry[num_input_src].ident == INPUT_RAW_REMOTE_ID) { 
		        bcm_input->fifo_offset = input_dir->entry[num_input_src].offset;
			bcm2835_ir_dbg("the offset is 0x%08x\n", bcm_input->fifo_offset);
			break;
		}
	}
	if (num_input_src < 0) {
                bcm2835_ir_error(": No raw remote input on VC side!\n");
                ret = -EIO;
                goto no_vc_inputdev;
        }
	 
        BUG_ON(((input_fifo_t *)((u32)bcm_input->base_addr + bcm_input->fifo_offset))->magic
                                                != ((INPUT_MAGIC_HEADER<<16) + INPUT_RAW_REMOTE_ID));

        ret = request_irq(bcm_input->irq, bcm2835_input_isr, IRQF_DISABLED, "input", bcm_input);
        if (ret) {
                bcm2835_ir_error(":Unable to register ISR\n");
                goto fail_to_req_irq;
        }

        bcm2835_ir_dbg("The device has been probed successfully\n");
	input_create_proc_entry(BCM2835_IR_MISC_DRIVER_NAME, input_dummy_read, input_proc_write);

	spin_lock_init(&bcm_input->fifo_lock);
	sema_init(&bcm_input->fifo_sem, 0);
        g_bcm_input = bcm_input;

        return 0;

fail_to_req_irq:
no_vc_inputdev:
	misc_deregister(&ir_misc_dev);
err_reg_chrdev:
no_io_resource:
        kfree(bcm_input);
        return ret;
}

static int __devexit bcm2835_input_remove(struct platform_device *pdev)
{
        struct bcm_input_device *bcm_input = platform_get_drvdata(pdev);

        free_irq(bcm_input->irq, (void *)bcm_input);
	platform_set_drvdata(pdev, NULL);
	misc_deregister(&ir_misc_dev);
	kfree(bcm_input);

	return 0;
}

static struct platform_driver bcm2835_input_driver = {
	.probe		= bcm2835_input_probe,
	.remove		= __devexit_p(bcm2835_input_remove),
	.suspend	= NULL,
	.resume		= NULL,
	.driver		= {
                .name	= BCM2835_IR_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init bcm2835_input_init(void)
{
	int ret;

        ret = platform_driver_register(&bcm2835_input_driver);
        if (ret != 0) {
                printk(KERN_ERR BCM2835_IR_DRIVER_NAME ": failed to register "
                       "on platform\n");
        } else
                printk(BCM2835_IR_DRIVER_NAME ": is registered\n");

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
