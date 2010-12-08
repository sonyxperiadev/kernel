/*
 *  linux/arch/arm/mach-bcm2708/ipc.c
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
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include <asm/uaccess.h>

#include <mach/memory.h>
#include <mach/irqs.h>
#include <mach/ipc.h>

#define DEBUG_IPC_MODULE   0

#define PLAT_DEV_NAME_SIZE_MAX 256

static void __iomem *ipc_base_virt;
static phys_addr_t ipc_base_phys;

#define ipc_phys_to_virt(x)	((x) - ipc_base_phys + ipc_base_virt)

/* Struct to store the user info in */
typedef struct IPC_BLOCK_USER_INFO
{
   uint32_t four_cc;
   uint32_t block_base_address;
   uint32_t interrupt_number_in_ipc;
   const void *funcs;

} IPC_BLOCK_USER_INFO_T;

typedef struct ipc_client_db {
	u32	num_registered_client;		
	struct {
		uint32_t four_cc;
		uint32_t irq;
	} client_info[IPC_BLOCK_MAX_NUM_USERS];
} ipc_client_db_t;	

static ipc_client_db_t	g_ipc_client_db;

static void ipc_mask_irq(unsigned int irq)
{
	return;
}

static void ipc_unmask_irq(unsigned int irq)
{
	return;
}

static struct irq_chip ipc_irq_chip = {
	.name           = "IPC_VIC",
	.ack            = ipc_mask_irq,
	.mask           = ipc_mask_irq,
	.unmask         = ipc_unmask_irq,
};

typedef enum {
	IPC_SEMAPHORE_ID_MIN	= 0,
	IPC_SEMAPHORE_VC_TO_ARM	= IPC_SEMAPHORE_ID_MIN,
	IPC_SEMAPHORE_ARM_TO_VC = IPC_SEMAPHORE_ID_MIN,
	IPC_SEMAPHORE_ID_2,
	IPC_SEMAPHORE_ID_3,
	IPC_SEMAPHORE_ID_4,
	IPC_SEMAPHORE_ID_5,
	IPC_SEMAPHORE_ID_6,
	IPC_SEMAPHORE_ID_7,
	IPC_SEMAPHORE_ID_MAX	= IPC_SEMAPHORE_ID_7,
} IPC_SEMAPHORE_ID_T;

typedef enum {
	IPC_SEMAPHORE_OWNER_VC = 0,
        IPC_SEMAPHORE_OWNER_ARM = 1,
} IPC_SEMAPHORE_OWNER_T;

static void ipc_spin_lock(IPC_SEMAPHORE_ID_T lock_id, IPC_SEMAPHORE_OWNER_T owner)
{
	while (0x0 != readl(IO_ADDRESS(MCORESYNC_BASE) + 4 * 6));
}


static void ipc_spin_unlock(IPC_SEMAPHORE_ID_T lock_id, IPC_SEMAPHORE_OWNER_T owner)
{
	writel(0x1, IO_ADDRESS(MCORESYNC_BASE) + 4 * 6);
}

int ipc_notify_vc_event(int irq_num)
{
	u32 vc_irq_status;
	u32 ipc_id = IRQ_TO_IPC(irq_num);
        unsigned long irq_flags;

	local_irq_save(irq_flags);
	mb();

	//ipc_spin_lock(IPC_SEMAPHORE_ARM_TO_VC, IPC_SEMAPHORE_OWNER_ARM);
	vc_irq_status = readl(ipc_base_virt + IPC_ARM_VC_INTERRUPT_OFFSET);
	vc_irq_status |= (0x1 << ipc_id);
	writel(vc_irq_status, ipc_base_virt + IPC_ARM_VC_INTERRUPT_OFFSET);

	//ipc_spin_unlock(IPC_SEMAPHORE_ARM_TO_VC, IPC_SEMAPHORE_OWNER_ARM);
	mb();

	local_irq_restore(irq_flags);

	mb();

	writel(0x1, IO_ADDRESS(ARM_0_BELL2));

	return 0;
}
EXPORT_SYMBOL(ipc_notify_vc_event);

void *ipc_bus_to_virt(uint32_t bus_addr)
{
	if (ipc_base_virt && ipc_base_phys)
		return (void *)ipc_phys_to_virt(__bus_to_phys(bus_addr));
	else
		return NULL;

}
EXPORT_SYMBOL(ipc_bus_to_virt);

#if 0

static nt ipc_lookup_service_resource(u32 four_cc, vc_service_resource_t *res)
{	
	u32 blk_num;
	u32 fcc_offset = ipc_base_virt + IPC_BLOCK_INFO_OFFSET;
	int ret = -EINVAL;

	if (!res)
		return ret;

	for (blk_num = 0; blk_num < IPC_BLOCK_MAX_NUM_USERS; blk_num++) {
		if (four_cc ==  readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, four_cc))) {
			res->irq = 
				IPC_TO_IRQ(readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, interrupt_number_in_ipc)));
			res->block_base = 
                                readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, block_base_address));
			res->block_base = IO_ADDRESS(__bus_to_phys(res->block_base));
			return 0;
		}
		fcc_offset +=  sizeof(IPC_BLOCK_USER_INFO_T);
	}
	
	return ret;
}
#endif

static void ipc_isr_handler( unsigned int irq, struct irq_desc *desc )
{
	u32 vc_irq_status, ipc_id;
	unsigned long irq_flags;

#if DEBUG_IPC_MODULE
	printk(KERN_ERR "we got interrupt from VC side\n");
#endif
	/* 
	 * Clear the doorbell first.
	 *
	 * Read and clear the interrupt 32bit with semaphore held
	 *
	 * Scan the 32bit and invoke specific second-level IRQ handler
	 *
	 */
	(void)readl(IO_ADDRESS(ARM_0_BELL0));

	local_irq_save(irq_flags);
	mb();
	//ipc_spin_lock(IPC_SEMAPHORE_VC_TO_ARM, IPC_SEMAPHORE_OWNER_ARM);
	vc_irq_status = readl(ipc_base_virt + IPC_VC_ARM_INTERRUPT_OFFSET);
	//writel(0x0, ipc_base_virt + IPC_VC_ARM_INTERRUPT_OFFSET);
	//ipc_spin_unlock(IPC_SEMAPHORE_VC_TO_ARM, IPC_SEMAPHORE_OWNER_ARM);
        mb();
	local_irq_restore(irq_flags);

#if DEBUG_IPC_MODULE
	printk(KERN_ERR "the int offset has value 0x%08x\n", vc_irq_status);
#endif

	for (ipc_id = 0; ipc_id < 32; ipc_id++) {
		if (vc_irq_status & (0x1 << ipc_id))
			generic_handle_irq(IPC_TO_IRQ(ipc_id));
	}

}

typedef struct
{
   /* proc entries */
   struct proc_dir_entry *init;
} IPC_STATE_T;


static struct proc_dir_entry *ipc_create_proc_entry( const char * const name,
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

static int ipc_dummy_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
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

#define IPC_MAX_INPUT_STR_LENGTH   256

static int ipc_init_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
   char *init_string = NULL;

   init_string = vmalloc(IPC_MAX_INPUT_STR_LENGTH);

   if(NULL == init_string)
      return -EFAULT;

   memset(init_string, 0, IPC_MAX_INPUT_STR_LENGTH);

   count = (count > IPC_MAX_INPUT_STR_LENGTH) ? IPC_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(init_string, buffer, count))
   {
      return -EFAULT;
   }

   init_string[ IPC_MAX_INPUT_STR_LENGTH  - 1 ] = 0;

   writel(0xff, ipc_base_virt + IPC_VC_ARM_INTERRUPT_OFFSET);

   writel(0x1, IO_ADDRESS(ARM_0_BELL0));  

   vfree(init_string);

   return count;
}


#if DEBUG_IPC_MODULE 
static irqreturn_t ipc_isr(int irq, void *dev_id)
{
   (void) dev_id;
	
   printk(KERN_ERR "service with fcc=%d got interrupt from VC!", irq);

   return IRQ_HANDLED;
}
#endif

static int __init ipc_add_service_devices(void)
{
        u32 blk_num, four_cc;
        u32 fcc_offset = (uint32_t )((char *)ipc_base_virt + IPC_BLOCK_INFO_OFFSET);
	char *dev_name = NULL;
	struct resource *dev_resource = NULL;
	struct platform_device *plat_dev = NULL;	
	resource_size_t start;
	int ret;

	/*
 	 * Scan through the IPC info block and register the appropriate platform device
 	 * according to the information in the block for each service.
 	 */
	for (blk_num = 0; blk_num < IPC_BLOCK_MAX_NUM_USERS; blk_num++) {
		start = 0;
		four_cc = readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, four_cc));
		if (0 == four_cc)
			break;

		plat_dev =  kzalloc(sizeof(struct platform_device), GFP_KERNEL);
		if (NULL == plat_dev) {
			printk(KERN_ERR "ipc failed to allocate mem\n");
			return -ENOMEM;
		}

		dev_name = kzalloc(PLAT_DEV_NAME_SIZE_MAX, GFP_KERNEL);
                if (NULL == dev_name) {
                        printk(KERN_ERR "ipc failed to allocate mem\n");
                        return -ENOMEM;
                }

		sprintf(dev_name, "%s%c%c%c%c", "bcm2835_", (four_cc & 0xff), ((four_cc >> 8) & 0xff),
						((four_cc >> 16) & 0xff), ((four_cc >> 24) & 0xff));

		plat_dev->name		= (const char *)dev_name;
		plat_dev->id 		= -1;

                dev_resource = kzalloc(sizeof(struct resource)*2, GFP_KERNEL);
                if (NULL == dev_resource) {
                        printk(KERN_ERR "ipc failed to allocate mem\n");
                        return -ENOMEM;
                }

		start = readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, block_base_address));
		printk(KERN_INFO"%s: Adding (%s) @ bus(0x%08x), phys(0x%08x) address\n", __func__, dev_name, start, __bus_to_phys(start));
		start = __bus_to_phys(start);

		dev_resource->start	= (resource_size_t) ipc_phys_to_virt(start);
                dev_resource->end	= dev_resource->start + SZ_8K - 1;
                dev_resource->flags     = IORESOURCE_MEM;

		printk(KERN_INFO"%s: Adding (%s) from 0x%08x-0x%08x Virt address range\n", __func__, dev_name, dev_resource->start,dev_resource->end);
                (dev_resource+1)->start	= 
				IPC_TO_IRQ(readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, interrupt_number_in_ipc)));
                (dev_resource+1)->end	= (dev_resource+1)->start;
                (dev_resource+1)->flags	= IORESOURCE_IRQ;
		
		plat_dev->resource	= dev_resource;
		plat_dev->num_resources = 2;
		plat_dev->dev.coherent_dma_mask	= DMA_BIT_MASK(32);

		ret = platform_device_register(plat_dev);
		if (ret) {
                        printk(KERN_ERR "ipc failed to register platform device\n");
                        return ret;
                }
#if DEBUG_IPC_MODULE
		printk(KERN_ERR "IPC just added plat dev with name=%s irq=%d\n", dev_name, (dev_resource+1)->start);
#endif

		g_ipc_client_db.client_info[blk_num].four_cc	= four_cc;
                g_ipc_client_db.client_info[blk_num].irq	= (dev_resource+1)->start; 

		fcc_offset +=  sizeof(IPC_BLOCK_USER_INFO_T);
	}

	g_ipc_client_db.num_registered_client = blk_num;

	return 0;
}

static int __init bcm_ipc_init(void)
{
	u32 i;
	IPC_STATE_T  *state = NULL;
	/* 
	 * First sanity-check if the IPC block is setup by VC; and after that,
	 * we will set up the ipc_irq_chip for interrupts from VC to ARM and also expose 
	 * APIs to ring doorbell from ARM to VC side.
	 */

	
	/* Wait for data to arrive in mailbox 0 and then read ipc base address from mailbox */
	if ((readl(IO_ADDRESS(ARM_0_MAIL0_STA)) & ARM_MS_EMPTY)) {
		printk(KERN_ERR"Mailbox 0 is empty, we dont have IPC base address\n");
		printk(KERN_ERR"%s: FAILED!!! \n", __func__);
		printk(KERN_ERR"Mailbox regs  ->\n");
		printk(KERN_ERR"RD -> 0x%08x\n", readl(IO_ADDRESS(ARM_0_MAIL0_RD)));
		printk(KERN_ERR"POL -> 0x%08x\n", readl(IO_ADDRESS(ARM_0_MAIL0_POL)));
		printk(KERN_ERR"SND -> 0x%08x\n", readl(IO_ADDRESS(ARM_0_MAIL0_SND)));
		printk(KERN_ERR"STA -> 0x%08x\n", readl(IO_ADDRESS(ARM_0_MAIL0_STA)));
		printk(KERN_ERR"SNF -> 0x%08x\n", readl(IO_ADDRESS(ARM_0_MAIL0_CNF)));
		return -ENODEV;
	}

	ipc_base_phys = readl(IO_ADDRESS(ARM_0_MAIL0_RD));
	printk(KERN_ERR"Received ipc_base = 0x%08x\n", ipc_base_phys);
	ipc_base_phys = __bus_to_phys(ipc_base_phys);
	printk(KERN_ERR"Real ipc_base = 0x%08x\n", ipc_base_phys);
	printk(KERN_ERR"sema address=0x%08x\n", IO_ADDRESS(ARM_0_SEM0 + 0x100 * 1));

	ipc_base_virt = ioremap(ipc_base_phys, IPC_BLOCK_SIZE);
	if (ipc_base_virt == NULL) {
		printk(KERN_ERR"Failed to ioremap ipc memory\n");
		return -ENOMEM;
	}

	printk(KERN_INFO"%s: IPC mem is mapped at 0x%08x\n", __func__, (uint32_t)ipc_base_virt);


	if (IPC_BLOCK_MAGIC != readl(ipc_base_virt + IPC_BLOCK_MAGIC_OFFSET)) {
		printk(KERN_ERR "BCM2708 VC has not initialized the IPC block yet!");
		return  -ENODEV;
	}

	/* Enable the doorbell interrupt from VC side and set up the irq chip. */
	for (i = IPC_TO_IRQ(0); i <= IPC_TO_IRQ(31); i++) {
		set_irq_chip(i, &ipc_irq_chip);
		set_irq_handler(i, handle_simple_irq);
		set_irq_flags(i, IRQF_VALID);
	}
	set_irq_chained_handler(IRQ_ARM_DOORBELL_0, ipc_isr_handler);

	state = kmalloc( sizeof(IPC_STATE_T ), GFP_KERNEL );

	if( state != NULL ) {
		state->init = ipc_create_proc_entry( "bcm2708_ipc", ipc_dummy_read, ipc_init_write );
	}

	if (ipc_add_service_devices()) {
		printk(KERN_ERR "bcm2708 ipc failed to register ipc service device\n");
                return  -ENODEV;
	}
	printk("BCM2708 ARM/VC IPC has been initialized successfully!");

#if DEBUG_IPC_MODULE
	for (i = IPC_TO_IRQ(0); i <= IPC_TO_IRQ(2); i++) {
		if (request_irq(i,  ipc_isr, IRQF_DISABLED, "ipc", NULL))
			printk(KERN_ERR "BRCM IPC irq request failed for irq# %d\n", i);
	}
#endif

	return 0;
};

arch_initcall(bcm_ipc_init);
