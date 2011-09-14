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

#include "lcd_clock.h" 

int brcm_enable_smi_lcd_clocks(struct pi_mgr_dfs_node** dfs_node)
{
	struct clk *smi_axi;
	struct clk *mm_dma_axi;
	struct clk *smi;

	*dfs_node = pi_mgr_dfs_add_request("smi_lcd", PI_MGR_PI_ID_MM, PI_OPP_TURBO);
	if (!*dfs_node)
	{
	    printk(KERN_ERR "Failed to add dfs request for SMI LCD\n");
	    return  -EIO;
	}

	smi_axi = clk_get (NULL, "smi_axi_clk");
	mm_dma_axi = clk_get(NULL, "mm_dma_axi_clk");
	smi = clk_get (NULL, "smi_clk");
	BUG_ON (!smi_axi || !smi || !mm_dma_axi);

	if (clk_set_rate(smi, 250000000)) {
		printk(KERN_ERR "Failed to set the SMI peri clock to 250MHZ");
		return -EIO;
	}

	if (clk_enable(smi)) {
		printk(KERN_ERR "Failed to enable the SMI peri clock");
		return -EIO;
	}

	if (clk_enable (smi_axi)) {
		printk(KERN_ERR "Failed to enable the SMI bus clock");
		return -EIO;
	}

	if (clk_enable(mm_dma_axi)) {
		printk(KERN_ERR "Failed to enable the MM DMA bus clock");
		return -EIO;
	}

	return 0;
}

int brcm_disable_smi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node)
{
	struct clk *smi_axi;
	struct clk *mm_dma_axi;
	struct clk *smi;

	smi_axi = clk_get (NULL, "smi_axi_clk");
	mm_dma_axi = clk_get (NULL, "mm_dma_axi_clk");
	smi = clk_get (NULL, "smi_clk");
	BUG_ON (!smi_axi || !smi || !mm_dma_axi);

	clk_disable(smi);
	clk_disable(smi_axi);
	clk_disable(mm_dma_axi);

	if (pi_mgr_dfs_request_remove(dfs_node))
	{
	    printk(KERN_ERR "Failed to remove dfs request for SMI LCD\n");
	    return  -EIO;
	}

	return 0;
}

int brcm_enable_dsi_lcd_clocks(struct pi_mgr_dfs_node** dfs_node)
{
	struct clk *mm_dma_axi;

	*dfs_node = pi_mgr_dfs_add_request("dsi_lcd", PI_MGR_PI_ID_MM, PI_OPP_TURBO);
	if (!*dfs_node)
	{
	    printk(KERN_ERR "Failed to add dfs request for DSI LCD\n");
	    return  -EIO;
	}

	mm_dma_axi = clk_get (NULL, "mm_dma_axi_clk");
	BUG_ON (!mm_dma_axi);

	if (clk_enable(mm_dma_axi)) {
		printk(KERN_ERR "Failed to enable the MM DMA bus clock");
		return -EIO;
	}

	return 0;
}

int brcm_disable_dsi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node)
{
	struct clk *mm_dma_axi;

	mm_dma_axi = clk_get (NULL, "mm_dma_axi_clk");
	BUG_ON (!mm_dma_axi);

	clk_disable(mm_dma_axi);

	if (pi_mgr_dfs_request_remove(dfs_node))
	{
	    printk(KERN_ERR "Failed to remove dfs request for SMI LCD\n");
	    return  -EIO;
	}

	return 0;
}
