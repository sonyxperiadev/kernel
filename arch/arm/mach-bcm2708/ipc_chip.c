/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#include <linux/ipc/ipc_chip.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <asm/bug.h>

#include <mach/memory.h>
#include <mach/ipc.h>
#include <mach/irqs.h> 

#define BCM2835_VC_IPC_BLOCK_SIZE		(SZ_2M)

void __init bcm2835_get_ipc_base(u32 *ipc_base_phys)
{
	*ipc_base_phys = 0;
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
	}

	*ipc_base_phys = readl(IO_ADDRESS(ARM_0_MAIL0_RD));
	printk(KERN_ERR"Received ipc_base = 0x%08x\n", *ipc_base_phys);
	*ipc_base_phys = __bus_to_phys(*ipc_base_phys);
	printk(KERN_ERR"Real ipc_base = 0x%08x\n", *ipc_base_phys);
}

static struct resource ipc_resource[] = {
   [0] = {
      .start = 0,
      .end = 0 + (BCM2835_VC_IPC_BLOCK_SIZE) - 1,
      .flags = IORESOURCE_MEM,
   },
   [1] = {
      .start = IRQ_ARM_DOORBELL_0,
      .end = IRQ_ARM_DOORBELL_0,
      .flags = IORESOURCE_IRQ,
   },
};


static u32 gpu_to_host_phys_addr(u32 gpu_addr)
{
	return __bus_to_phys(gpu_addr);
}

static void clear_doorbell_irq(void)
{
	(void)readl(IO_ADDRESS(ARM_0_BELL0));
}

static void ring_doorbell_irq(void)
{

	writel(0x1, IO_ADDRESS(ARM_0_BELL2));
}

static u32 read_and_ack_req(u32 ipc_virt_base)
{
	u32 vc_irq_status = readl(ipc_virt_base + IPC_VC_ARM_INTERRUPT_OFFSET);
	writel(0x0, ipc_virt_base + IPC_VC_ARM_INTERRUPT_OFFSET);

	return vc_irq_status; 
}

static void send_req(u32 req, u32 ipc_virt_base)
{
	u32 vc_irq_status = readl(ipc_virt_base + IPC_ARM_VC_INTERRUPT_OFFSET);
	vc_irq_status |= (0x1 << req);
	writel(vc_irq_status, ipc_virt_base + IPC_ARM_VC_INTERRUPT_OFFSET);
}

static struct ipc_chip ipc_chip = {
	32,
	IPC_TO_IRQ(0),
	&gpu_to_host_phys_addr,
	&clear_doorbell_irq,
	&ring_doorbell_irq,
	NULL,
	NULL,
	&read_and_ack_req,
	&send_req,
};

struct platform_device bcm2835_ipc_device = {
	.name = "IPC",
	.resource = ipc_resource,
	.num_resources = ARRAY_SIZE(ipc_resource),
	.dev = {
		.platform_data = &ipc_chip,
	},
};
