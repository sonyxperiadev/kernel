#/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
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
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>

#include <mach/hardware.h>
#include <asm/hardware/coresight.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb_A0/brcm_rdb_etb.h>

MODULE_LICENSE("GPL");

#define	HUB_FUNNEL_OUT	(1 << 0)
#define	ARM_FUNNEL_OUT	(1 << 1)
#define	FAB_FUNNEL_OUT	(1 << 2)
#define	MM_SUBSYS_OUT	(1 << 3)
#define	COMM_FUNNEL_OUT	(1 << 4)
#define	PTM0_TRACE	(1 << 0)
#define	PTM1_TRACE	(1 << 1)
#define	AXITRACE_1	(1 << 2)
#define	AXITRACE_4	(1 << 3)
#define	GIC_TRACE	(1 << 4)
#define	AXITRACE_16	(1 << 5)
#define	SWSTM_OUT	(1 << 6)
#define	ARM_FUNNEL	KONA_FUNNEL_VA

static void configure_fin_funnel(unsigned int funnel)
{
	unsigned int value;
	value = readl(KONA_FIN_FUNNEL_VA);
	writel(value | funnel, KONA_FIN_FUNNEL_VA);
}

static void configure_arm_funnel(unsigned int funnel)
{
	unsigned int value;
	value = readl(ARM_FUNNEL);
	writel(value | funnel, ARM_FUNNEL);
}

static int configure_ptm_funnel(void)
{
	unsigned int value;

	configure_fin_funnel(ARM_FUNNEL_OUT);
	configure_arm_funnel(PTM0_TRACE | PTM1_TRACE);

	/* ETB SRAM Select
	 * 1 Internal 4KByte SRAM selected for ETB tracing
	 * 0 1MByte DDR selected for ETB tracing
	 */
	value = readl(KONA_CHIPREG_VA + CHIPREG_SPARE_CONTROL0_OFFSET);
	value |= 0x1;
	writel(value, KONA_CHIPREG_VA + CHIPREG_SPARE_CONTROL0_OFFSET);

	return 0;
}

static struct etb_platform_data etb_data = {
	.tpiu_clk_name = "tpiu_clk",
	.pti_clk_name = "pti_clk",
	.config_funnels = configure_ptm_funnel,
};

static AMBA_APB_DEVICE(hawaii_etb, "etb", 0x000bb907,
			ETB_BASE_ADDR, { }, &etb_data);
static AMBA_APB_DEVICE(hawaii_etm0, "etm0", 0x102bb921,
			A9PTM0_BASE_ADDR, { }, NULL);
static AMBA_APB_DEVICE(hawaii_etm1, "etm1", 0x102bb950,
			A9PTM1_BASE_ADDR, { }, NULL);

static void *ddr_virt_addr;
static	dma_addr_t ddr_dma_addr;

static ssize_t sysfs_send_to_ddr_store(struct device *dev,
				struct device_attribute *devattr,
				const char *buf, size_t count)
{
	unsigned int *ddr_ptr;
	int total_words, i;
	int wr_ptr;

	void *__iomem etb_base_vptr = ioremap(ETB_BASE_ADDR, SZ_4K);
	if (!etb_base_vptr) {
		pr_err("cannot map ETB base addr\n");
		return -EFAULT;
	}

	/* ETB must be disabled before reading */
	writel(0, etb_base_vptr + ETBR_CTRL);

	mdelay(300);

	/* Set the read pointer to zero */
	writel(0, etb_base_vptr + ETBR_READADDR);

	ddr_ptr = ddr_virt_addr;
	pr_info("DDR virtual address = 0x%x\n", (unsigned int)ddr_ptr);

	wr_ptr = readl(etb_base_vptr + ETBR_WRITEADDR);

	/* we have only 1024 words of SRAM memory. But the pointer wraps only
	 * after 1MB */
	wr_ptr %= 1024;
	pr_info("wr ptr = 0x%x\n", wr_ptr);
	pr_info("rd ptr = 0x%x\n", readl(etb_base_vptr + ETBR_READADDR));

	total_words = wr_ptr;

	/* During reset write pointer will be lost
	 * in that case we will dump the whole 4K buffer */
	if (total_words == 0) {
		total_words = 1024;
		pr_info(KERN_INFO "Dumping full buffer. Total words =  %d\n",
						total_words);
	}

	pr_info("total trace data = %d\n", total_words);
	for (i = 0 ; i < total_words ; i++)
		*(ddr_ptr + i) = readl(etb_base_vptr + ETBR_READMEM);

	iounmap(etb_base_vptr);
	return count;
}

static ssize_t sysfs_ddr_phys_addr_show(struct device *dev,
					struct device_attribute *devattr,
					char *buf)
{
	return sprintf(buf, "%08x\n", ddr_dma_addr);
}

static DEVICE_ATTR(ddr_phys_addr, 0644, sysfs_ddr_phys_addr_show, NULL);
static DEVICE_ATTR(send_to_ddr, 0644, NULL, sysfs_send_to_ddr_store);
static struct attribute *etb_attrs[] = {
	&dev_attr_send_to_ddr.attr,
	&dev_attr_ddr_phys_addr.attr,
	NULL,
};

static struct attribute_group etb_attr_group = {
	.attrs = etb_attrs,
};

static int __init ptm_init(void)
{
	int ret;

	if (amba_device_register(&hawaii_etb_device, &iomem_resource))
		pr_err("etb device registration failed\n");
	if (amba_device_register(&hawaii_etm0_device, &iomem_resource))
		pr_err("ptm0 device registration failed\n");
	if (amba_device_register(&hawaii_etm1_device, &iomem_resource))
		pr_err("ptm1 device registration failed\n");

	ret = sysfs_create_group(&hawaii_etb_device.dev.kobj, &etb_attr_group);
	if (ret) {
		pr_err("sysfs create group failed\n");
		return ret;
	}

	ddr_virt_addr = dma_alloc_coherent(NULL, SZ_4K, &ddr_dma_addr, GFP_DMA);
	if (ddr_virt_addr == NULL) {
		pr_err("DMA alloc coherent failed\n");
		return (int)ddr_virt_addr;
	}

	pr_info("DDR address for TRACE %x\n", ddr_dma_addr);
	memset(phys_to_virt(ddr_dma_addr), 0xAA , SZ_4K);

	return 0;
}
subsys_initcall(ptm_init);
