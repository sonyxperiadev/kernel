 /************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
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
#include <linux/version.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <asm/mach/arch.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_mm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/memory.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <plat/clock.h>
#include <mach/clock.h>
#include <mach/pi_mgr.h>
#include <plat/pwr_mgr.h>
#include <linux/broadcom/mobcom_types.h>
#include <plat/csl/csl_dma_vc4lite.h>
#include <linux/dma-mapping.h>
#include <plat/scu.h>
#include "pm_params.h"

static void *acp_workaround_src_buffer, *acp_workaround_dest_buffer;
static u32 acp_workaround_src_buffer_phys, acp_workaround_dest_buffer_phys;

#define MM_WORKAROUND_DEBUG_ENABLE	0

#if (MM_WORKAROUND_DEBUG_ENABLE == 1)
#define mm_workaround_debug(format, args...)	printk(KERN_ERR format, ##args)
#else
#define mm_workaround_debug(format, args...)	do { } while (0)
#endif

#define ACP_WR_CMD_ALIGNMENT	8
#define ACP_RD_DATA_ALIGNMENT	16
extern void csl_dma_poll_int(int chanID);

static void acp_workaround_dma_callback(int status)
{
}

static int acp_dma_start(int dma_chan, int direction, int num_transactions)
{
	DMA_VC4LITE_CHANNEL_INFO_t dmaChInfo;
	DMA_VC4LITE_XFER_DATA_t dmaData;
	Int32 dmaCh;
	UInt32 src_buf, dest_buf;

	dmaCh = dma_chan;

	dmaChInfo.autoFreeChan = 1;
	dmaChInfo.srcID = DMA_VC4LITE_CLIENT_MEMORY;
	dmaChInfo.dstID = DMA_VC4LITE_CLIENT_MEMORY;
	dmaChInfo.burstLen = DMA_VC4LITE_BURST_LENGTH_1;
	dmaChInfo.xferMode = DMA_VC4LITE_XFER_MODE_LINERA;
	dmaChInfo.dstStride = 0;
	dmaChInfo.srcStride = 0;
	dmaChInfo.waitResponse = 0;
	dmaChInfo.callback =
	    (DMA_VC4LITE_CALLBACK_t) &acp_workaround_dma_callback;
	if (csl_dma_vc4lite_config_channel(dmaCh, &dmaChInfo)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		printk(KERN_ERR "%s %d", __func__, __LINE__);
		return -1;
	}

	if (direction) {
		src_buf =
		    ((u32) acp_workaround_src_buffer_phys & ~0x80000000) |
		    0x40000000;
		dest_buf = (u32) acp_workaround_dest_buffer_phys;
	} else {
		src_buf = (u32) acp_workaround_src_buffer_phys;
		dest_buf =
		    ((u32) acp_workaround_dest_buffer_phys & ~0x80000000) |
		    0x40000000;
	}
	dmaData.srcAddr = src_buf;
	dmaData.dstAddr = dest_buf;
	dmaData.xferLength = num_transactions * 4;
	if (csl_dma_vc4lite_add_data(dmaCh, &dmaData)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		printk(KERN_ERR "%s %d", __func__, __LINE__);
		return -1;
	}

	if (csl_dma_vc4lite_start_transfer(dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		printk(KERN_ERR "%s %d", __func__, __LINE__);
		return -1;
	}
	mb();

	return 0;
}

static void rhea_acp_workaround(void)
{
	unsigned int n_wr_cmds;
	unsigned int n_rd_data;
	unsigned int n_wr_cmd_to_do;
	unsigned int n_rd_data_to_do;

	n_wr_cmds = readl(KONA_AXITRACE16_VA + 0x10);
	n_rd_data = readl(KONA_AXITRACE16_VA + 0x5C);
	n_wr_cmd_to_do = (n_wr_cmds % ACP_WR_CMD_ALIGNMENT);
	n_rd_data_to_do = (n_rd_data % ACP_RD_DATA_ALIGNMENT);
	if (n_wr_cmd_to_do) {

		acp_dma_start(1, 0, ACP_WR_CMD_ALIGNMENT - n_wr_cmd_to_do);
		csl_dma_poll_int(1);
	}
	if (n_rd_data_to_do) {
		acp_dma_start(1, 1, ACP_RD_DATA_ALIGNMENT - n_rd_data_to_do);
		csl_dma_poll_int(1);
	}
	n_wr_cmds = readl(KONA_AXITRACE16_VA + 0x10);
	n_rd_data = readl(KONA_AXITRACE16_VA + 0x5C);
}

static int rhea_mm_memc_dma_start(int size,
				  DMA_VC4LITE_BURST_LENGTH_t burst_size,
				  int channel, int direction, int acp)
{
	DMA_VC4LITE_CHANNEL_INFO_t dmaChInfo;
	DMA_VC4LITE_XFER_DATA_t dmaData;
	Int32 dmaCh;
	UInt32 src_buf, dest_buf;

	dmaCh = channel;

	dmaChInfo.autoFreeChan = 1;
	dmaChInfo.srcID = DMA_VC4LITE_CLIENT_MEMORY;
	dmaChInfo.dstID = DMA_VC4LITE_CLIENT_MEMORY;

	dmaChInfo.burstLen = burst_size;
	dmaChInfo.xferMode = DMA_VC4LITE_XFER_MODE_LINERA;
	dmaChInfo.dstStride = 0;
	dmaChInfo.srcStride = 0;
	dmaChInfo.waitResponse = 0;
	dmaChInfo.callback =
	    (DMA_VC4LITE_CALLBACK_t) &acp_workaround_dma_callback;

	if (csl_dma_vc4lite_config_channel(dmaCh, &dmaChInfo)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		printk(KERN_ERR "%s %d", __func__, __LINE__);
		return -1;
	}

	if (direction) {
		src_buf = (u32) 0x34040000;
		dest_buf = (u32) acp_workaround_dest_buffer_phys;
	} else {
		src_buf = (u32) acp_workaround_src_buffer_phys;
		dest_buf = (u32) 0x34040000;
	}

	if (acp) {
		if (direction) {
			src_buf = (u32) acp_workaround_src_buffer_phys;
			dest_buf =
			    ((u32) acp_workaround_dest_buffer_phys &
			     ~0x80000000) | 0x40000000;
		} else {
			src_buf =
			    ((u32) acp_workaround_src_buffer_phys & ~0x80000000)
			    | 0x40000000;
			dest_buf = (u32) acp_workaround_dest_buffer_phys;
		}
	}

	dmaData.srcAddr = src_buf;
	dmaData.dstAddr = dest_buf;
	dmaData.xferLength = size;

	if (csl_dma_vc4lite_add_data(dmaCh, &dmaData)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		printk(KERN_ERR "%s %d", __func__, __LINE__);
		return -1;
	}

	if (csl_dma_vc4lite_start_transfer(dmaCh)
	    != DMA_VC4LITE_STATUS_SUCCESS) {
		printk(KERN_ERR "%s %d", __func__, __LINE__);
		return -1;
	}

	mb();
	csl_dma_poll_int(1);

	return 0;

}

static void rhea_mm_acp_workaround(void)
{
	int atm_count, index;

	atm_count = readl(KONA_AXITRACE16_VA + 0x10);
	atm_count = atm_count % 8;
	if (atm_count) {
		for (index = 0; index < (8 - atm_count); ++index) {
			rhea_mm_memc_dma_start(4, DMA_VC4LITE_BURST_LENGTH_1, 1,
					       1, 1);
		}
	}

	atm_count = readl(KONA_AXITRACE16_VA + 0x58);
	atm_count = atm_count % 16;

	if ((atm_count) && (atm_count > 8)) {
		/* we have less than 8 need to write, let's write 8 more. Make it more than 8 to write */
		for (index = 0; index < 8; ++index) {
			rhea_mm_memc_dma_start(4, DMA_VC4LITE_BURST_LENGTH_1, 1,
					       1, 1);
		}
	}

	atm_count = readl(KONA_AXITRACE16_VA + 0x58);
	atm_count = atm_count % 16;
	if (atm_count) {
		if (atm_count <= 8) {
			/* we have > 8 to write */
			for (index = 0; index < 8 - atm_count; ++index) {
				rhea_mm_memc_dma_start(0x20,
					DMA_VC4LITE_BURST_LENGTH_2,
						1, 1, 1);
			}
			for (index = 8 - atm_count; index < 8; ++index) {
				rhea_mm_memc_dma_start(4,
						DMA_VC4LITE_BURST_LENGTH_1,
						1, 1, 1);
			}
		} else {
			printk(KERN_ERR "atm <= 8 for AXI 16 write error");
		}
	}

	atm_count = readl(KONA_AXITRACE16_VA + 0x14);
	atm_count = atm_count % 8;
	if (atm_count) {
		for (index = 0; index < (8 - atm_count); ++index) {
			rhea_mm_memc_dma_start(4,
				DMA_VC4LITE_BURST_LENGTH_1, 1, 0, 1);
		}
	}

	atm_count = readl(KONA_AXITRACE16_VA + 0x5C);
	atm_count = atm_count % 16;

	if (atm_count && (atm_count > 8)) {
	/* we have less than 8 need to write, let's write 8 more.
	Make it more than 8 to write */
		for (index = 0; index < 8; ++index) {
			rhea_mm_memc_dma_start(4,
				DMA_VC4LITE_BURST_LENGTH_1, 1,
				0, 1);
		}
	}

	atm_count = readl(KONA_AXITRACE16_VA + 0x5C);
	atm_count = atm_count % 16;
	if (atm_count) {
		if (atm_count <= 8) {
			/* we have > 8 to Read */
			for (index = 0; index < 8 - atm_count; ++index) {
				rhea_mm_memc_dma_start(8,
					 DMA_VC4LITE_BURST_LENGTH_2,
					1, 0, 1);
			}
			for (index = 8 - atm_count; index < 8; ++index) {
				rhea_mm_memc_dma_start(4,
					DMA_VC4LITE_BURST_LENGTH_1,
					1, 0, 1);
			}
		} else {
			printk(KERN_ERR "atm <= 8 AXI 16 read error");
		}
	}
}

static void rhea_mm_memc_workaround(void)
{
	int atm_count, index;

	atm_count = readl(KONA_AXITRACE17_VA + 0x10);
	atm_count = atm_count % 8;
	if (atm_count) {
		for (index = 0; index < (8 - atm_count); ++index) {
			rhea_mm_memc_dma_start(4, DMA_VC4LITE_BURST_LENGTH_1, 1,
					       1, 0);
		}
	}

	atm_count = readl(KONA_AXITRACE17_VA + 0x58);
	atm_count = atm_count % 16;

	if ((atm_count) && (atm_count > 8)) {
	/* we have less than 8 need to write,
	let's write 8 more. Make it more than 8 to write */
		for (index = 0; index < 8; ++index)
			rhea_mm_memc_dma_start(4,
				DMA_VC4LITE_BURST_LENGTH_1,
					1, 1, 0);/* Chan 1, Write*/

	}

	atm_count = readl(KONA_AXITRACE17_VA + 0x58);

	atm_count = atm_count % 16;
	if (atm_count) {
		if (atm_count <= 8) {
			/* we have > 8 to write */
			for (index = 0; index < 8 - atm_count; ++index) {
				rhea_mm_memc_dma_start(0x20,
						DMA_VC4LITE_BURST_LENGTH_2,
						1, 1, 0);
			}

			for (index = 8 - atm_count; index < 8; ++index) {
				rhea_mm_memc_dma_start(4,
					DMA_VC4LITE_BURST_LENGTH_1,
					1, 1, 0);
			}
		} else {
			printk(KERN_ERR "atm <= 8 error");
		}
	}

	/*
	 * First align the read command counter
	 * Using 2 transfers per dma, the read counter increments by 3 each
	 * time. (2 transfers + 1 read command registers)
	 */
	atm_count = readl(KONA_AXITRACE17_VA + 0x14);
	atm_count = atm_count % 8;
	if (atm_count) {
		for (; atm_count % 8 != 0; atm_count += 3) {
			rhea_mm_memc_dma_start(8, DMA_VC4LITE_BURST_LENGTH_1, 1,
					       0, 0);
		}
	}

	/*
	   atm_count = readl(KONA_AXITRACE17_VA + 0x5C); //read beats
	   atm_count = atm_count % 16;

	   if (atm_count) {
	   for(; atm_count % 16 != 0; atm_count += 13) {
	   // Chan 1, Read
	   rhea_mm_memc_dma_start(28, DMA_VC4LITE_BURST_LENGTH_1, 1, 0, 0);

	   }
	   }
	 */

	return;
}

static void rhea_mm_shutdown_workaround(void)
{
	unsigned int n_wr_cmds;
	unsigned int n_rd_data;
	unsigned int n_wr_beats;
	unsigned int n_rd_cmds;

	unsigned int n_rd_cmds_mm_memc;
	unsigned int n_wr_cmds_mm_memc;
	unsigned int n_wr_beats_mm_memc;

	unsigned int old_n_wr_cmds;
	unsigned int old_n_rd_data;
	unsigned int old_n_wr_beats;
	unsigned int old_n_rd_cmds;

	unsigned int old_n_rd_cmds_mm_memc;
	unsigned int old_n_wr_cmds_mm_memc;
	unsigned int old_n_wr_beats_mm_memc;

	old_n_wr_cmds = readl(KONA_AXITRACE16_VA + 0x10);
	old_n_rd_data = readl(KONA_AXITRACE16_VA + 0x5C);

	old_n_rd_cmds = readl(KONA_AXITRACE16_VA + 0x14);
	old_n_wr_beats = readl(KONA_AXITRACE16_VA + 0x58);

	old_n_rd_cmds_mm_memc = readl(KONA_AXITRACE17_VA + 0x14);
	old_n_wr_cmds_mm_memc = readl(KONA_AXITRACE17_VA + 0x10);
	old_n_wr_beats_mm_memc = readl(KONA_AXITRACE17_VA + 0x58);

	rhea_mm_acp_workaround();
	rhea_mm_memc_workaround();

	n_wr_cmds = readl(KONA_AXITRACE16_VA + 0x10);
	n_rd_data = readl(KONA_AXITRACE16_VA + 0x5C);

	n_rd_cmds = readl(KONA_AXITRACE16_VA + 0x14);
	n_wr_beats = readl(KONA_AXITRACE16_VA + 0x58);

	n_rd_cmds_mm_memc = readl(KONA_AXITRACE17_VA + 0x14);
	n_wr_cmds_mm_memc = readl(KONA_AXITRACE17_VA + 0x10);
	n_wr_beats_mm_memc = readl(KONA_AXITRACE17_VA + 0x58);

	if (n_wr_cmds % 8 || n_rd_data % 16 || n_rd_cmds % 8 || n_wr_beats % 16
	    || n_rd_cmds_mm_memc % 8 || n_wr_cmds_mm_memc % 8
	    || n_wr_beats_mm_memc % 16) {
		printk(KERN_ERR
		       "Before shutdown workaround axi16 wcmd=0x%08x rcmds=0x%08x wbeats=0x%08x rbeats=0x%08x\n",
		       old_n_wr_cmds, old_n_rd_cmds, old_n_wr_beats,
		       old_n_rd_data);
		printk(KERN_ERR
		       "Before shutdown workaround axi17 wcmd=0x%08x rcmds=0x%08x wbeats=0x%08x\n",
		       old_n_wr_cmds_mm_memc, old_n_rd_cmds_mm_memc,
		       old_n_wr_beats_mm_memc);

		printk(KERN_ERR
		       "After shutdown workaround axi16 wcmd=0x%08x rcmds=0x%08x wbeats=0x%08x rbeats=0x%08x\n",
		       n_wr_cmds, n_rd_cmds, n_wr_beats, n_rd_data);
		printk(KERN_ERR
		       "After shutdown workaround axi17 wcmd=0x%08x rcmds=0x%08x wbeats=0x%08x\n",
		       n_wr_cmds_mm_memc, n_rd_cmds_mm_memc,
		       n_wr_beats_mm_memc);
	}
}

static int mm_pol_chg_notifier(struct notifier_block *self,
			       unsigned long event, void *data)
{
	struct pi_notify_param *p = data;

	BUG_ON(p->pi_id != PI_MGR_PI_ID_MM);
	if (event == PI_PRECHANGE) {
#ifdef CONFIG_RHEA_WA_HWJIRA_2348
			if (JIRA_WA_ENABLED(2348)) {
				if (IS_ACTIVE_POLICY(p->old_value) && IS_RETN_POLICY(p->new_value)) {
					u32 reg_val;
					mm_workaround_debug("before going to retention\n");
					scu_standby(false);
					writel(0xA5A501, KONA_MM_CLK_VA +
					       MM_CLK_MGR_REG_WR_ACCESS_OFFSET);
					reg_val = readl(KONA_MM_CLK_VA +
						MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET);
					writel(reg_val |
						MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_CLK_EN_MASK,
					       KONA_MM_CLK_VA +
					       MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET);

					rhea_acp_workaround();
					mb();

					writel(reg_val,
					       KONA_MM_CLK_VA +
					       MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET);
					scu_standby(true);
					mm_workaround_debug("After going to retention\n");
				}
			}
#endif /*CONFIG_RHEA_WA_HWJIRA_2348 */
#ifdef CONFIG_RHEA_WA_HWJIRA_2489
			if (JIRA_WA_ENABLED(2489)) {
				if (IS_SHUTDOWN_POLICY(p->new_value)) {
					u32 reg_val;

					mm_workaround_debug("before going to shut down\n");
					scu_standby(false);
					writel(0xA5A501,
					       KONA_MM_CLK_VA +
					       MM_CLK_MGR_REG_WR_ACCESS_OFFSET);
					reg_val =
					    readl(KONA_MM_CLK_VA +
						  MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET);
					writel(reg_val |
					       MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_CLK_EN_MASK,
					       KONA_MM_CLK_VA +
					       MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET);

					rhea_mm_shutdown_workaround();
					mb();

					writel(reg_val,
					       KONA_MM_CLK_VA +
					       MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET);
					scu_standby(true);

					mm_workaround_debug("After going to shut down\n");
				}
			}
#endif /*CONFIG_RHEA_WA_HWJIRA_2489 */
	} else {
#ifdef CONFIG_RHEA_WA_HWJIRA_2221
		if (JIRA_WA_ENABLED(2221)) {
			char *temp_buf;
			extern char *noncache_buf_va;
			u32 temp_val;
			int c;

			if ((!IS_ACTIVE_POLICY(p->new_value) &&
			     IS_ACTIVE_POLICY(p->old_value))) {
				writel(CSR_AXI_PORT_CTRL_PORT3_DISABLE_MASK,
				       KONA_MEMC0_NS_VA +
				       CSR_AXI_PORT_CTRL_OFFSET);
				temp_buf = noncache_buf_va;

				for (c = 0; c < 16; c++, temp_buf += 64)
					temp_val = *(volatile u32 *)temp_buf;

				writel(0x0,
				       KONA_MEMC0_NS_VA +
				       CSR_AXI_PORT_CTRL_OFFSET);
			}
		}
#endif /*CONFIG_RHEA_WA_HWJIRA_2221 */
#ifdef CONFIG_RHEA_WA_HWJIRA_2489
		if (JIRA_WA_ENABLED(2489)) {

			if (IS_SHUTDOWN_POLICY(p->old_value) &&
			    !IS_SHUTDOWN_POLICY(p->new_value)) {

				mm_workaround_debug("coming out of shutdown reset\n");
				/* Enable the AXI trace17 counters again. */
				writel(0x5551, KONA_AXITRACE17_VA + 0x0);
				writel(0x2, KONA_AXITRACE17_VA + 0xC);
			}
		}
#endif /*CONFIG_RHEA_WA_HWJIRA_2489 */
	}
	return 0;
}

static struct notifier_block mm_pol_chg_notify_blk = {
	.notifier_call = mm_pol_chg_notifier,
};

int __init mm_workarounds_init(void)
{
#if defined(CONFIG_RHEA_WA_HWJIRA_2348) || \
				defined(CONFIG_RHEA_WA_HWJIRA_2489)
/*Enable the trace16 counter*/
	writel(0x5551, KONA_AXITRACE16_VA + 0x0);
	writel(0x2, KONA_AXITRACE16_VA + 0xC);
#endif

#ifdef CONFIG_RHEA_WA_HWJIRA_2489
/*Enable the trace17 counter*/
	writel(0x5551, KONA_AXITRACE17_VA + 0x0);
	writel(0x0, KONA_AXITRACE17_VA + 0xC);
	writel(0x2, KONA_AXITRACE17_VA + 0xC);
#endif /*CONFIG_RHEA_WA_HWJIRA_2489 */

	return 0;
}

int __init mm_workarounds_late_init(void)
{
#if defined(CONFIG_RHEA_WA_HWJIRA_2348) || \
				defined(CONFIG_RHEA_WA_HWJIRA_2489)
	acp_workaround_src_buffer = dma_alloc_writecombine(NULL,
							   4 * 1024,
				&acp_workaround_src_buffer_phys,
						   GFP_KERNEL);
	acp_workaround_dest_buffer =
	    dma_alloc_writecombine(NULL, 4 * 1024,
				   &acp_workaround_dest_buffer_phys,
				   GFP_KERNEL);
#endif

#if defined(CONFIG_RHEA_WA_HWJIRA_2348) || \
				defined(CONFIG_RHEA_WA_HWJIRA_2489) || \
				defined(CONFIG_RHEA_WA_HWJIRA_2221)
	pi_mgr_register_notifier(PI_MGR_PI_ID_MM,
				 &mm_pol_chg_notify_blk,
				 PI_NOTIFY_POLICY_CHANGE);
#endif
	return 0;
}

arch_initcall(mm_workarounds_init);
late_initcall(mm_workarounds_late_init);
