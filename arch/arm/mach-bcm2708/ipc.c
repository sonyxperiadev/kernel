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
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>


#include <mach/irqs.h>
#include <mach/ipc.h>

/******************************************************************************
Private macros and constants
******************************************************************************/
#if 0

#define IPC_BLOCK_MAGIC			0xCAFEBABE

#define IPC_BLOCK_MAGIC_OFFSET		0x000

#define IPC_BLOCK_INFO_OFFSET		0x200

#define IPC_VC_ARM_INTERRUPT_OFFSET	0x800  /* VC  -> ARM */
#define IPC_ARM_VC_INTERRUPT_OFFSET	0x804  /* ARM -> VC */

//currently we tie the interrupt number to the user slot
#define IPC_BLOCK_MAX_NUM_USERS		32

#define	IPC_BLOCK_BASE_PHY		0x07E00000
#define IPC_BLOCK_SIZE			SIZE_2M

#endif

#define IPC_BASE	IO_ADDRESS(IPC_BLOCK_BASE_PHY)

/******************************************************************************
 Private Types
 *****************************************************************************/

/* Struct to store the user info in */
typedef struct IPC_BLOCK_USER_INFO
{
   const void *funcs;

   uint32_t four_cc;
   uint32_t block_base_address;
   uint32_t interrupt_number_in_ipc;

} IPC_BLOCK_USER_INFO_T;

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
	IPC_SEMAPHORE_ID_0	= IPC_SEMAPHORE_ID_MIN,
	IPC_SEMAPHORE_ID_1,
	IPC_SEMAPHORE_ID_2,
	IPC_SEMAPHORE_ID_3,
	IPC_SEMAPHORE_ID_4,
	IPC_SEMAPHORE_ID_5,
	IPC_SEMAPHORE_ID_6,
	IPC_SEMAPHORE_ID_7,
	IPC_SEMAPHORE_ID_MAX	= IPC_SEMAPHORE_ID_7,
} IPC_SEMAPHORE_ID_T;


static void ipc_spin_lock(IPC_SEMAPHORE_ID_T lock_id)
{
	while (0x0 != readl(IO_ADDRESS(ARM_0_SEM0) + (lock_id << 2)));
}


static void ipc_spin_unlock(IPC_SEMAPHORE_ID_T lock_id)
{
	BUG_ON(0x0 == readl(IO_ADDRESS(ARM_0_SEM0) + (lock_id << 2)));
	writel(0x1, IO_ADDRESS(ARM_0_SEM0) + (lock_id << 2));
}

int ipc_notify_vc_event(int ipc_id)
{
	u32 vc_irq_status;

	ipc_spin_lock(IPC_SEMAPHORE_ID_0);
	vc_irq_status = readl(IPC_BASE + IPC_ARM_VC_INTERRUPT_OFFSET);
	vc_irq_status |= (0x1 << ipc_id);
	writel(vc_irq_status, IPC_BASE + IPC_ARM_VC_INTERRUPT_OFFSET);
	ipc_spin_unlock(IPC_SEMAPHORE_ID_0);

	writel(0x1, IO_ADDRESS(ARM_0_BELL0));

	return 0;
}
EXPORT_SYMBOL(ipc_notify_vc_event);


int ipc_lookup_irqnum(u32 four_cc)
{	
	u32 blk_num;
	u32 four_cc_offset = IPC_BASE + IPC_BLOCK_INFO_OFFSET;

	for (blk_num = 0; blk_num < IPC_BLOCK_MAX_NUM_USERS; blk_num++) {
		if (four_cc ==  readl(four_cc_offset + offsetof(IPC_BLOCK_USER_INFO_T, four_cc)))
			return readl(four_cc_offset + offsetof(IPC_BLOCK_USER_INFO_T, interrupt_number_in_ipc));

		four_cc_offset +=  sizeof(IPC_BLOCK_USER_INFO_T);
	}
	
	return IPC_IRQNUM_NONE;
}
EXPORT_SYMBOL(ipc_lookup_irqnum);

static void ipc_isr_handler( unsigned int irq, struct irq_desc *desc )
{
	u32 vc_irq_status, ipc_id;
	/* 
	 * Clear the doorbell first.
	 *
	 * Read and clear the interrupt 32bit with semaphore held
	 *
	 * Scan the 32bit and invoke specific second-level IRQ handler
	 *
	 */
	(void)readl(IO_ADDRESS(ARM_0_BELL1));
	
	ipc_spin_lock(IPC_SEMAPHORE_ID_1);
	vc_irq_status = readl(IPC_BASE + IPC_VC_ARM_INTERRUPT_OFFSET);
	writel(0x0, IPC_BASE + IPC_VC_ARM_INTERRUPT_OFFSET);
	ipc_spin_unlock(IPC_SEMAPHORE_ID_1);

	for (ipc_id = 0; ipc_id < 32; ipc_id++) {
		if (vc_irq_status & (0x1 << ipc_id))
			generic_handle_irq(IPC_TO_IRQ(ipc_id));
	}

}


static int __init bcm_ipc_init(void)
{
	u32 i;

	/* 
	 * First sanity-check if the IPC block is setup by VC; and after that,
	 * we will set up the ipc_irq_chip for interrupts from VC to ARM and also expose 
	 * APIs to ring doorbell from ARM to VC side.
	 */
	//ipc_base = ioremap(IPC_BLOCK_BASE_PHY, 	IPC_BLOCK_SIZE);

	if (IPC_BLOCK_MAGIC != readl(IPC_BASE +  IPC_BLOCK_MAGIC_OFFSET)) {
		printk(KERN_ERR "The VC has not initialized the IPC block yet!");
		return  -1;
	}

	/* Enable the doorbell interrupt from VC side and set up the irq chip. */
	for (i = IPC_TO_IRQ(0); i <= IPC_TO_IRQ(31); i++) {
		set_irq_chip(i, &ipc_irq_chip);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID);
	}
	set_irq_chained_handler(IRQ_ARM_DOORBELL_1, ipc_isr_handler);

	return 0;
};

arch_initcall(bcm_ipc_init);

