/*
 *  drivers/misc/ipc.c
 *
 *  Copyright (C) 2011 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
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
#include <linux/spinlock.h>
#include <linux/dma-mapping.h>
#include <linux/ipc/ipc_chip.h>
#include <linux/ipc/ipc.h>

#include <asm/uaccess.h>
#include <asm/mach/irq.h>

#include <mach/irqs.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_ipcopen.h>

#define IPC_MODULE_DEBUG	0
#define IPC_VC4_FIRMWARE_READY	1
#define IPC_MODULE_DEBUG_ISR	0

#define IPC_DRIVER_NAME "Inter Processor Comm"

#define ipc_error(format, arg...) \
	printk(KERN_ERR IPC_DRIVER_NAME ": %s" format, __func__, ##arg) 

#if IPC_MODULE_DEBUG
#define ipc_dbg(format, arg...) ipc_error(format, ##arg)
#else
#define ipc_dbg(format, arg...) do {} while (0)
#endif

/* TODO: 
 * the following constants can be actually passed from the device platform data.
 */
#define IPC_BLOCK_MAGIC			0xCAFEBABE

#define IPC_BLOCK_MAGIC_OFFSET		0x000

#define IPC_BLOCK_INFO_OFFSET		0x200

#define IPC_VC_ARM_INTERRUPT_OFFSET	0x800  /* VC  -> ARM */
#define IPC_ARM_VC_INTERRUPT_OFFSET	0x804  /* ARM -> VC */

#define IPC_BLOCK_MAX_NUM_USERS		32 /* currently we tie the interrupt number to the user slot */

#define IPC_BLOCK_SIZE			SZ_2M

#define IPC_IRQNUM_NONE			-1

#define PLAT_DEV_NAME_SIZE_MAX 256

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

struct ipc_data {
	void __iomem		*phys_base;
	u32			mem_size;
	void			*virt_base;
	int			irq;
	spinlock_t		gpu_to_host_lock;
	spinlock_t		host_to_gpu_lock;
	struct ipc_chip 	*ipc_chip_p;
};

static struct ipc_data *g_ipc_data_p = NULL;

#if 0
static inline u32 IPC_TO_IRQ(u32 ipc_id)	
{
	if (g_ipc_data_p)
		return ipc_id + g_ipc_data_p->ipc_chip_p->irq_base;
	else {
		ipc_error("IPC driver is not set up yet!");
		return 0;
	}
}

static inline u32 IRQ_TO_IPC(u32 irq_num)
{
	if (g_ipc_data_p)
		return  irq_num - g_ipc_data_p->ipc_chip_p->irq_base;
	else {
		ipc_error("IPC driver is not set up yet!");
		return 0;
	}
}
#endif

static void ipc_mask_irq(struct irq_data *data)
{
	return;
}

static void ipc_unmask_irq(struct irq_data *data)
{
	return;
}

static struct irq_chip ipc_irq_chip = {
	.name           = "IPC_VIC",
	.irq_ack            = ipc_mask_irq,
	.irq_mask           = ipc_mask_irq,
	.irq_unmask         = ipc_unmask_irq,
};

#define IPC_VC_ARM_LOCK_BASE_OFFSET     (IPC_VC_ARM_INTERRUPT_OFFSET + 0x20)
#define IPC_VC_ARM_FLAG0_OFFSET         0x0
#define IPC_VC_ARM_FLAG1_OFFSET         0x4
#define IPC_VC_ARM_TURN_OFFSET          0x8

typedef enum {
        IPC_VC_ARM_LOCK0 = 0x0,
        IPC_VC_ARM_LOCK1 = 0x1,
} IPC_VC_ARM_LOCK_T;

static void ipc_vc_arm_get_lock(IPC_VC_ARM_LOCK_T lock_num)
{
        writel(1, g_ipc_data_p->virt_base + IPC_VC_ARM_LOCK_BASE_OFFSET + lock_num * 0x20 + IPC_VC_ARM_FLAG1_OFFSET);
        writel(0, g_ipc_data_p->virt_base + IPC_VC_ARM_LOCK_BASE_OFFSET + lock_num * 0x20 + IPC_VC_ARM_TURN_OFFSET);

        mb();

        /* busy wait until VC has given it up ... */
        while ((readl(g_ipc_data_p->virt_base + IPC_VC_ARM_LOCK_BASE_OFFSET + lock_num * 0x20 + IPC_VC_ARM_FLAG0_OFFSET) == 1) && (readl(g_ipc_data_p->virt_base + IPC_VC_ARM_LOCK_BASE_OFFSET + lock_num * 0x20 + IPC_VC_ARM_TURN_OFFSET) == 0));

}

static void ipc_vc_arm_put_lock(IPC_VC_ARM_LOCK_T lock_num)
{
        writel(0, g_ipc_data_p->virt_base + IPC_VC_ARM_LOCK_BASE_OFFSET + lock_num * 0x20 + IPC_VC_ARM_FLAG1_OFFSET);
        mb();
}

int ipc_notify_vc_event(int irq_num)
{
	u32 req = 0x1 << (irq_num - g_ipc_data_p->ipc_chip_p->irq_base);
        unsigned long irq_flags;

	spin_lock_irqsave(&g_ipc_data_p->host_to_gpu_lock, irq_flags);
	mb();

	if (g_ipc_data_p->ipc_chip_p->send_req_atomic)
		g_ipc_data_p->ipc_chip_p->send_req_atomic(req, (u32)g_ipc_data_p->virt_base);
	else {
		ipc_vc_arm_get_lock(IPC_VC_ARM_LOCK1);
		g_ipc_data_p->ipc_chip_p->send_req(req, (u32)g_ipc_data_p->virt_base);
		ipc_vc_arm_put_lock(IPC_VC_ARM_LOCK1);
	}
	mb();

	spin_unlock_irqrestore(&g_ipc_data_p->host_to_gpu_lock, irq_flags);

	g_ipc_data_p->ipc_chip_p->ring_doorbell_irq();

	return 0;
}
EXPORT_SYMBOL(ipc_notify_vc_event);

#if 0
static inline u32 __bus_to_phys(u32 bus_addr)
{
	if (g_ipc_data_p)
		return g_ipc_data_p->ipc_chip_p->gpu_to_host_phys_addr(bus_addr);
	else {
		ipc_error("IPC driver is not set up yet!");
		return 0;
	}	
}
#endif

void *ipc_bus_to_virt(uint32_t bus_addr)
{
	if (g_ipc_data_p)
		return (void *)(g_ipc_data_p->ipc_chip_p->gpu_to_host_phys_addr(bus_addr) - 
			(u32)g_ipc_data_p->phys_base + (u32)g_ipc_data_p->virt_base);
	else
		return 0;

}
EXPORT_SYMBOL(ipc_bus_to_virt);

static void ipc_isr_handler( unsigned int irq, struct irq_desc *desc )
{
	u32 vc_irq_status, ipc_id;
	unsigned long irq_flags;
	struct irq_chip *chip = irq_desc_get_chip(desc);

	chained_irq_enter(chip, desc);

	g_ipc_data_p->ipc_chip_p->clear_doorbell_irq();

	spin_lock_irqsave(&g_ipc_data_p->gpu_to_host_lock, irq_flags);
	mb();

	if (g_ipc_data_p->ipc_chip_p->read_and_ack_req_atomic)
		vc_irq_status = g_ipc_data_p->ipc_chip_p->read_and_ack_req_atomic((u32)g_ipc_data_p->virt_base);
	else {
		ipc_vc_arm_get_lock(IPC_VC_ARM_LOCK0);
	
		vc_irq_status = g_ipc_data_p->ipc_chip_p->read_and_ack_req((u32)g_ipc_data_p->virt_base);

		ipc_vc_arm_put_lock(IPC_VC_ARM_LOCK0);
	}
	mb();

	spin_unlock_irqrestore(&g_ipc_data_p->gpu_to_host_lock, irq_flags);

	ipc_dbg("IPC req has value 0x%08x\n", vc_irq_status);

	for (ipc_id = 0; ipc_id < 32; ipc_id++) {
		if (vc_irq_status & (0x1 << ipc_id))
			generic_handle_irq(ipc_id + g_ipc_data_p->ipc_chip_p->irq_base);
	}

	chained_irq_exit(chip, desc);
}

typedef struct
{
	struct proc_dir_entry *init;
} IPC_STATE_T;


static struct proc_dir_entry *ipc_create_proc_entry(const char * const name,
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

	if (offset > 0) {
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

	if(copy_from_user(init_string, buffer, count)) {
		return -EFAULT;
	}

	init_string[ IPC_MAX_INPUT_STR_LENGTH  - 1 ] = 0;

#if 0
	writel(0xff, g_ipc_data_p->virt_base + IPC_VC_ARM_INTERRUPT_OFFSET);

	writel(0x1, IO_ADDRESS(ARM_0_BELL0));  

	writel(0xff, (KONA_IPC_NS_VA) + (IPCOPEN_IPCVSET_OFFSET)); 
#endif

	writel(0xff, (KONA_IPC_NS_VA) + (IPCOPEN_IPCVSET_OFFSET)); 

	vfree(init_string);

	return count;
}


#if	IPC_MODULE_DEBUG_ISR
static irqreturn_t ipc_isr(int irq, void *dev_id)
{
	(void) dev_id;
	
	ipc_dbg("service with fcc=%d got interrupt from VC!", irq);

	return IRQ_HANDLED;
}
#endif /* IPC_MODULE_DEBUG */

static int __init ipc_add_service_devices(void)
{
        u32 blk_num, four_cc;
        u32 fcc_offset = (uint32_t )((char *)g_ipc_data_p->virt_base + IPC_BLOCK_INFO_OFFSET);
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
		printk(KERN_INFO"%s: Adding (%s) @ bus(0x%08x), phys(0x%08x) address\n", 
			__func__, dev_name, start, g_ipc_data_p->ipc_chip_p->gpu_to_host_phys_addr(start));
		dev_resource->start	= (resource_size_t) ipc_bus_to_virt(start);
                dev_resource->end	= dev_resource->start + SZ_8K - 1;
                dev_resource->flags     = IORESOURCE_MEM;

		printk(KERN_INFO"%s: Adding (%s) from 0x%08x-0x%08x Virt address range\n", __func__, dev_name, dev_resource->start,dev_resource->end);
                (dev_resource+1)->start	= 
				readl(fcc_offset + offsetof(IPC_BLOCK_USER_INFO_T, interrupt_number_in_ipc)) + g_ipc_data_p->ipc_chip_p->irq_base;
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
		ipc_dbg("IPC just added plat dev with name=%s irq=%d\n", dev_name, (dev_resource+1)->start);

		g_ipc_client_db.client_info[blk_num].four_cc	= four_cc;
                g_ipc_client_db.client_info[blk_num].irq	= (dev_resource+1)->start; 

		fcc_offset +=  sizeof(IPC_BLOCK_USER_INFO_T);
	}

	g_ipc_client_db.num_registered_client = blk_num;

	return 0;
}


static int ipc_probe(struct platform_device *pdev)
{
        int ret = -ENXIO, i;
        struct resource *r;
	struct ipc_data *ipc_data_p = NULL;

        ipc_data_p = kzalloc(sizeof(struct ipc_data), GFP_KERNEL);
        if (NULL == ipc_data_p) {
                ipc_error("Unable to allocate ipc data structure\n");
                ret = -ENOMEM;
                goto err_alloc_failed;
        }
	g_ipc_data_p = ipc_data_p;
        platform_set_drvdata(pdev, ipc_data_p);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
        if (r == NULL) {
		ipc_error("Unable to get memory resource\n");
                ret = -ENODEV;
                goto err_no_io_base;
        }
	ipc_dbg("IPC physical addr space start-end (0x%08x)-(0x%08x)\n", r->start, r->end);
	ipc_data_p->phys_base = (void __iomem *)r->start;
	ipc_data_p->mem_size = resource_size(r);

	ipc_data_p->virt_base = ioremap((u32)ipc_data_p->phys_base, ipc_data_p->mem_size);
	if (ipc_data_p->virt_base == NULL) {
		ipc_error("Failed to ioremap ipc memory\n");
		ret = -ENOMEM;
		goto err_ioremap;
	}
	printk(KERN_INFO"%s: IPC mem is mapped at virtiual addr 0x%08x\n", __func__, (uint32_t)ipc_data_p->virt_base);

#if IPC_VC4_FIRMWARE_READY
	if (IPC_BLOCK_MAGIC != readl((u32)ipc_data_p->virt_base + IPC_BLOCK_MAGIC_OFFSET)) {
		ipc_error("The IPC block has not been initialized yet!");
		ret = -ENODEV;
		goto err_no_magic;
	}
#endif

	ipc_data_p->irq = platform_get_irq(pdev, 0);
        if(ipc_data_p->irq < 0) {
                ipc_error("Unable to get irq resource\n");
                ret = -ENODEV;
                goto err_no_irq;
        }
	ipc_data_p->ipc_chip_p = (struct ipc_chip *)pdev->dev.platform_data;

	printk(KERN_INFO"%s: IPC irq:%d and irq_base:%d\n", __func__, (uint32_t)ipc_data_p->irq,
			(uint32_t)ipc_data_p->ipc_chip_p->irq_base);

	/* Enable the doorbell interrupt from the other side and set up the irq chip. */
	for (i = 0; i < ipc_data_p->ipc_chip_p->num_channels; i++) {
		irq_set_chip_and_handler(ipc_data_p->ipc_chip_p->irq_base + i, &ipc_irq_chip, handle_simple_irq);
		set_irq_flags(ipc_data_p->ipc_chip_p->irq_base + i, IRQF_VALID);
	}
	irq_set_chained_handler(ipc_data_p->irq, ipc_isr_handler);

	if (NULL == ipc_create_proc_entry( "inter_processor_comm", ipc_dummy_read, ipc_init_write )) {
		ipc_error("Failed to register ipc proc entry\n");
                ret = -ENODEV;
		goto err_create_proc_entry;
	}

#if IPC_VC4_FIRMWARE_READY
	if (ipc_add_service_devices()) {
		ipc_error("Failed to register ipc service devices\n");
                ret = -ENODEV;
		goto err_add_services;
	}
#endif

	spin_lock_init(&ipc_data_p->gpu_to_host_lock);
	spin_lock_init(&ipc_data_p->host_to_gpu_lock);

#if IPC_MODULE_DEBUG_ISR
	for (i = 0; i <= 2; i++) {
		if (request_irq(i+ipc_data_p->ipc_chip_p->irq_base, ipc_isr, IRQF_DISABLED, "ipc", NULL)) {
			ipc_error("IPC irq request failed for irq# %d\n", i+ipc_data_p->ipc_chip_p->irq_base);
			ret = -ENODEV;
			goto err_add_services;
		}
	}
#endif

	printk(KERN_INFO"Inter-Processor Communication has been initialized successfully!");
	return 0;

err_add_services:
err_create_proc_entry:
	free_irq(ipc_data_p->irq, NULL);
err_no_irq:
err_no_magic:
	iounmap(ipc_data_p->virt_base);
err_ioremap:
err_no_io_base:
	kfree(ipc_data_p);
err_alloc_failed:
	return ret;

};

static struct platform_driver ipc_driver = {
        .probe          = ipc_probe,
        .driver = {
                .name = "IPC"
        }
};

static int __init inter_processor_comm_init(void)
{
        int ret;

        ret = platform_driver_register(&ipc_driver);
        if (ret)
                ipc_error("Unable to register ipc driver\n");

        printk(KERN_INFO IPC_DRIVER_NAME "Init %s !\n", ret ? "FAILED" : "OK");

        return ret;
}

module_init(inter_processor_comm_init);
