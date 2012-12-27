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

static struct funnel_map trace4_funnel[] = {
	AXI_FUNNEL(KONA_FUNNEL_VA, 3, 1),
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 1, 1),
	{NULL}
};

static struct funnel_map trace17_funnel[] = {
	AXI_FUNNEL(KONA_FIN_FUNNEL_VA, 3, 1),
	{NULL}
};

static struct axi_master axitrace17_masters[] = {
	AXI_MASTER("CSI",	0),
	AXI_MASTER("DMAC",	1),
	AXI_MASTER("V3D",	2),
	AXI_MASTER("VCE",	3),
	AXI_MASTER("ISP",	4),
	AXI_MASTER("Hub_Sw",	5),
	AXI_MASTER("H264",	6),
	AXI_MASTER("AXIPV",	7),
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

static struct axi_master axitrace4_masters[] = {
	AXI_MASTER("A9", 0),
	{NULL}
};

static struct axi_slave axitrace4_slaves[] = {
	AXI_SLAVE("Sys_EMI", 0x80000000, 0xFFFFFFFF),
	{NULL}
};

static struct resource axitrace4_resource = {
	.start	= AXITRACE4_BASE_ADDR,
	.end	= AXITRACE4_BASE_ADDR + SZ_4K,
	.flags	= IORESOURCE_MEM,
};

static struct axitrace_source trace_sources[] = {
	{
		.name		= "axitrace17",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= axitrace17_masters,
		.slaves		= axitrace17_slaves,
		.axi_id_mask	= 7,
		.resource	= &axitrace17_resource,
		.map		= trace17_funnel,
	},
	{
		.name		= "axitrace4",
		.cap		= AXITRACE_FULL_CAP,
		.filters	= AXITRACE_ALL_FILTERS,
		.filter_count	= 2,
		.masters	= axitrace4_masters,
		.slaves		= axitrace4_slaves,
		.axi_id_mask	= 0,
		.resource	= &axitrace4_resource,
		.map		= trace4_funnel,
	},
	{ 0 }, /* NULL Termination */
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
