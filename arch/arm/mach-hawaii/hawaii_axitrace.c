/************************************************************************/
/*                                                                      */
/*  Copyright 2013  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software.  The special   exception does not apply to any            */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/amba/bus.h>
#include <linux/ioport.h>
#include <mach/axitrace.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/io_map.h>

static struct funnel_map trace1_funnel[] = {
	AXI_FUNNEL(KONA_FUNNEL_VA, 2, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 1, 1),
	{NULL}
};

static struct funnel_map trace4_funnel[] = {
	AXI_FUNNEL(KONA_FUNNEL_VA, 3, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 1, 1),
	{NULL}
};

static struct funnel_map trace16_funnel[] = {
	AXI_FUNNEL(KONA_FUNNEL_VA, 5, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 1, 1),
	{NULL}
};


static struct funnel_map trace17_funnel[] = {
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 3, 1),
	{NULL}
};

static struct funnel_map trace2_funnel[] = {
	AXI_FUNNEL(KONA_FAB_FUNNEL1_VA, 0, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 2, 1),
	{NULL}
};

static struct funnel_map trace3_funnel[] = {
	AXI_FUNNEL(KONA_FAB_FUNNEL1_VA, 1, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 2, 1),
	{NULL}
};

static struct funnel_map trace11_funnel[] = {
	AXI_FUNNEL(KONA_HUB_FUNNEL_VA, 1, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 0, 1),
	{NULL}
};

static struct funnel_map trace12_funnel[] = {
	AXI_FUNNEL(KONA_HUB_FUNNEL_VA, 2, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 0, 1),
	{NULL}
};

static struct funnel_map trace19_funnel[] = {
	AXI_FUNNEL(KONA_HUB_FUNNEL_VA, 0, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 0, 1),
	{NULL}
};

static struct axi_master mm_masters[] = {
	AXI_MASTER("CSI",	0),
	AXI_MASTER("DMAC",	1),
	AXI_MASTER("V3D",	2),
	AXI_MASTER("VCE",	3),
	AXI_MASTER("ISP",	4),
	AXI_MASTER("Hub_Sw",	5),
	AXI_MASTER("H264",	6),
	AXI_MASTER("AXIPV",	7),
	/* For "ALL", id is dont care because driver will
	   set the id_mask to 0 */
	AXI_MASTER("ALL",	7),
	{NULL}
};

static struct axi_master axitrace19_masters[] = {
	AXI_MASTER("AHB/AXI",  0),
	AXI_MASTER("APB/AXI",  1),
	AXI_MASTER("ETB/AXI",  2),
	AXI_MASTER("CAPH/AXI", 3),
	AXI_MASTER("ALL",      3),
	{NULL}
};

static struct axi_master comms_masters[] = {
	AXI_MASTER("DSP0",	0),
	AXI_MASTER("DSP1",	1),
	AXI_MASTER("R4",	3),
	AXI_MASTER("Modem_Sw",	4),
	AXI_MASTER("ALL",	4),
	{NULL}
};

static struct axi_slave axitrace17_slaves[] = {
	AXI_SLAVE("Sys_EMI", 0x80000000, 0xFFFFFFFF),
	{NULL}
};

static struct resource axitrace17_resource = {
	.start	= AXITRACE17_BASE_ADDR,
	.end	= AXITRACE17_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct axi_master axitrace4_1_masters[] = {
	AXI_MASTER("A9", 0),
	{NULL}
};

static struct axi_master axitrace2_masters[] = {
	AXI_MASTER("CHAN0", 0),
	AXI_MASTER("CHAN1", 1),
	AXI_MASTER("CHAN2", 2),
	AXI_MASTER("CHAN3", 3),
	AXI_MASTER("CHAN4", 4),
	AXI_MASTER("CHAN5", 5),
	AXI_MASTER("CHAN6", 6),
	AXI_MASTER("CHAN7", 7),
	AXI_MASTER("PL330_DMAC_MGR", 8),
	AXI_MASTER("ALL", 0xF),
	{NULL}
};

static struct axi_master axitrace3_masters[] = {
	AXI_MASTER("USB_OTG_CS1",	2),
	AXI_MASTER("USB_OTG_CS2",	3),
	AXI_MASTER("FSHOST",		6),
	AXI_MASTER("FSHOST_CTRL",	7),
	AXI_MASTER("SDIO1",		8),
	AXI_MASTER("SDIO2",		9),
	AXI_MASTER("SDIO3",		10),
	AXI_MASTER("SDIO4",		11),
	AXI_MASTER("ALL",		0),
	{NULL},
};

static struct axi_slave axitrace4_slaves[] = {
	AXI_SLAVE("Sys_EMI", 0x80000000, 0xFFFFFFFF),
	{NULL}
};

static struct axi_slave axitrace_all_slaves[] = {
	AXI_SLAVE("ALL", 0x0, 0xFFFFFFFF),
	{NULL}
};

static struct resource axitrace4_resource = {
	.start	= AXITRACE4_BASE_ADDR,
	.end	= AXITRACE4_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct resource axitrace1_resource = {
	.start	= AXITRACE1_BASE_ADDR,
	.end	= AXITRACE1_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct resource axitrace16_resource = {
	.start	= AXITRACE16_BASE_ADDR,
	.end	= AXITRACE16_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct resource axitrace2_resource = {
	.start	= AXITRACE2_BASE_ADDR,
	.end	= AXITRACE2_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct resource axitrace3_resource = {
	.start	= AXITRACE3_BASE_ADDR,
	.end	= AXITRACE3_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct resource axitrace11_resource = {
	.start	= AXITRACE11_BASE_ADDR,
	.end	= AXITRACE11_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct resource axitrace12_resource = {
	.start	= AXITRACE12_BASE_ADDR,
	.end	= AXITRACE12_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct resource axitrace19_resource = {
	.start	= AXITRACE19_BASE_ADDR,
	.end	= AXITRACE19_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct axitrace_source trace_sources[] = {
	{	/* MM to SYS_EMI */
		.name		= "axitrace17",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= mm_masters,
		.slaves		= axitrace17_slaves,
		.axi_id_mask	= 7,
		.resource	= &axitrace17_resource,
		.map		= trace17_funnel,
	},
	{	/* ARM core to Fabric */
		.name		= "axitrace1",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= axitrace4_1_masters,
		.slaves		= axitrace_all_slaves,
		.axi_id_mask	= 0,
		.resource	= &axitrace1_resource,
		.map		= trace1_funnel,
	},
	{	/* ARM core to SYS_EMI */
		.name		= "axitrace4",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= axitrace4_1_masters,
		.slaves		= axitrace4_slaves,
		.axi_id_mask	= 0,
		.resource	= &axitrace4_resource,
		.map		= trace4_funnel,
	},
	{	/* ACP access */
		.name		= "axitrace16",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= mm_masters,
		.slaves		= axitrace_all_slaves,
		.axi_id_mask	= 0x7,
		.resource	= &axitrace16_resource,
		.map		= trace16_funnel,
	},
	{	/* PL330 */
		.name		= "axitrace2",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= axitrace2_masters,
		.slaves		= axitrace_all_slaves,
		.axi_id_mask	= 0xF,
		.resource	= &axitrace2_resource,
		.map		= trace2_funnel,
	},
	{	/* USB, SDIO and FSHOST */
		.name		= "axitrace3",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= axitrace3_masters,
		.slaves		= axitrace_all_slaves,
		.axi_id_mask	= 0xF,
		.resource	= &axitrace3_resource,
		.map		= trace3_funnel,
	},
	{	/* MM to Hub */
		.name		= "axitrace11",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= mm_masters,
		.slaves		= axitrace_all_slaves,
		.axi_id_mask	= 0x7,
		.resource	= &axitrace11_resource,
		.map		= trace11_funnel,
	},
	{	/* COMMs to Hub */
		.name		= "axitrace12",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= comms_masters,
		.slaves		= axitrace_all_slaves,
		.axi_id_mask	= 0x7,
		.resource	= &axitrace12_resource,
		.map		= trace12_funnel,
	},
	{	/* AON Hub, ETB2AXI, CAPH to Hub Switch */
		.name		= "axitrace19",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= axitrace19_masters,
		.slaves		= axitrace_all_slaves,
		.axi_id_mask	= 0x7,
		.resource	= &axitrace19_resource,
		.map		= trace19_funnel,
	},
	{ NULL }, /* NULL Termination */
};

static struct axitrace_platform_data axitrace_pdata = {
	.sources	= trace_sources,
};

AMBA_APB_DEVICE(hawaii_axitrace, "axitrace", 0x0003c921, 0, { },
					&axitrace_pdata);

static int __init hawaii_axitrace_init(void)
{
	int retval;

	retval = amba_device_register(&hawaii_axitrace_device,
						&iomem_resource);
	if (retval)
		pr_err("axitrace device registratin failed\n");

	return retval;
}
subsys_initcall(hawaii_axitrace_init);

static void  __exit hawaii_axitrace_exit(void)
{
	amba_device_unregister(&hawaii_axitrace_device);
}
module_exit(hawaii_axitrace_exit);
