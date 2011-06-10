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

#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_ipcopen.h>

#include "island.h"

#define VC4_IPC_PHYS_ADDR_BASE	0x60040000UL
#define ARM_IPC_PHYS_ADDR_BASE	0x34040000UL

#define ISLAND_VC_IPC_BLOCK_PHYS_BASE		((ARM_IPC_PHYS_ADDR_BASE) + 0x0)
#define ISLAND_VC_IPC_BLOCK_SIZE		(SZ_128K)

static struct resource ipc_resource[] = {
   [0] = {
      .start = ISLAND_VC_IPC_BLOCK_PHYS_BASE,
      .end =  (ISLAND_VC_IPC_BLOCK_PHYS_BASE) + (ISLAND_VC_IPC_BLOCK_SIZE) - 1,
      .flags = IORESOURCE_MEM,
   },
   [1] = {
      .start = BCM_INT_ID_IPC_OPEN,
      .end = BCM_INT_ID_IPC_OPEN,
      .flags = IORESOURCE_IRQ,
   },
};


static u32 gpu_to_host_phys_addr(u32 gpu_addr)
{
	WARN_ON(gpu_addr < (VC4_IPC_PHYS_ADDR_BASE));

	return gpu_addr - (VC4_IPC_PHYS_ADDR_BASE) + (ARM_IPC_PHYS_ADDR_BASE);
}

static void clear_doorbell_irq(void)
{

}

static void ring_doorbell_irq(void)
{

}

static u32 read_and_ack_req_atomic(u32 virt_base)
{
	u32 req;

	(void)virt_base;
	req = readl((KONA_IPC_NS_VA) + (IPCOPEN_IPCASTATUS_OFFSET));
	writel(req, (KONA_IPC_NS_VA) + (IPCOPEN_IPCACLR_OFFSET));
	return req;
}

static void send_req_atomic(u32 req, u32 virt_base)
{
	(void)virt_base;
	writel(req, (KONA_IPC_NS_VA) + (IPCOPEN_IPCASET_OFFSET));
}

static struct ipc_chip ipc_chip = {
	32,
	IRQ_IPC_0,
	&gpu_to_host_phys_addr,
	&clear_doorbell_irq,
	&ring_doorbell_irq,
	&read_and_ack_req_atomic,
	&send_req_atomic,
	NULL,
	NULL,
};

struct platform_device island_ipc_device = {
	.name = "IPC",
	.resource = ipc_resource,
	.num_resources = ARRAY_SIZE(ipc_resource),
	.dev = {
		.platform_data = &ipc_chip,
	},
};
