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

#define __DSI_USE_CLK_API__

#include "lcd_clock.h" 
#ifdef __DSI_USE_CLK_API__
#include <plat/clock.h>
#endif

#ifdef __DSI_USE_CLK_API__
struct 
{
	char *dsi_pll_ch;    
        char *dsi_axi;	     
        char *dsi_esc;
        u32  pixel_pll_sel;	     
} dsi_bus_clk[2] = 	     
{
    { 
        "dsi_pll_chnl0",
        "dsi0_axi_clk",  
        "dsi0_esc_clk",  
        DSI0_PIXEL_PLL,
    },
    { 
        "dsi_pll_chnl1",
        "dsi1_axi_clk",  
        "dsi1_esc_clk",  
        DSI1_PIXEL_PLL,
    },	
};
#endif

int brcm_enable_smi_lcd_clocks(struct pi_mgr_dfs_node *dfs_node)
{
#ifndef CONFIG_MACH_BCM_FPGA
	struct clk *smi_axi;
	struct clk *smi;

	if (pi_mgr_dfs_request_update(dfs_node, PI_OPP_TURBO))
	{
	    printk(KERN_ERR "Failed to update dfs request for SMI LCD at enable\n");
	    return  -EIO;
	}

	smi_axi = clk_get (NULL, "smi_axi_clk");
	smi = clk_get (NULL, "smi_clk");
	BUG_ON(IS_ERR_OR_NULL(smi_axi) || IS_ERR_OR_NULL(smi));

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
#endif
	return 0;
}

int brcm_disable_smi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node)
{
#ifndef CONFIG_MACH_BCM_FPGA
	struct clk *smi_axi;
	struct clk *smi;

	smi_axi = clk_get (NULL, "smi_axi_clk");
	smi = clk_get (NULL, "smi_clk");
	BUG_ON(IS_ERR_OR_NULL(smi_axi) || IS_ERR_OR_NULL(smi));

	clk_disable(smi);
	clk_disable(smi_axi);

	if (pi_mgr_dfs_request_update(dfs_node, PI_MGR_DFS_MIN_VALUE))
	{
	    printk(KERN_ERR "Failed to update dfs request for SMI LCD at disable\n");
	    return  -EIO;
	}

#endif
	return 0;
}

void en_disp_clks()
{
	if (!g_display_enabled)
		return;
	struct clk *dsi_axi;
	struct clk *dsi_esc;
	struct clk *dsi_pll;
	struct clk *dsi_pll_ch;
	dsi_axi	= clk_get(NULL, dsi_bus_clk[0].dsi_axi);
	dsi_esc	= clk_get(NULL, dsi_bus_clk[0].dsi_esc);
	BUG_ON(IS_ERR_OR_NULL(dsi_axi) || IS_ERR_OR_NULL(dsi_esc));
	if (clk_enable(dsi_axi))
		return;

	if (clk_enable(dsi_esc))
		return;

	dsi_pll     = clk_get(NULL, "dsi_pll");
	dsi_pll_ch  = clk_get(NULL, dsi_bus_clk[0].dsi_pll_ch);
	BUG_ON(IS_ERR_OR_NULL(dsi_pll) || IS_ERR_OR_NULL(dsi_pll_ch));
	if (clk_enable(dsi_pll))
		return;

	if (clk_enable(dsi_pll_ch))
		return;
}
device_initcall(en_disp_clks);

#ifdef __DSI_USE_CLK_API__
int brcm_enable_dsi_lcd_clocks(
	struct pi_mgr_dfs_node* dfs_node,
	unsigned int dsi_bus, 
        unsigned int dsi_pll_hz, 
        unsigned int dsi_pll_ch_div, 
        unsigned int esc_clk_hz )
{
#ifndef CONFIG_MACH_BCM_FPGA
	struct	clk *dsi_axi;
	struct	clk *dsi_esc;
	
	if (g_display_enabled) {
		printk(KERN_ERR "Skipping brcm_enable_dsi_lcd_clocks\n");
		return 0;
	}
	printk(KERN_INFO "brcm_enable_dsi_lcd_clocks\n");
	if (pi_mgr_dfs_request_update(dfs_node, PI_OPP_TURBO))
	{
	    printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
	    return  -EIO;
	}

	dsi_axi     = clk_get (NULL, dsi_bus_clk[dsi_bus].dsi_axi);
	dsi_esc     = clk_get (NULL, dsi_bus_clk[dsi_bus].dsi_esc);
	BUG_ON(IS_ERR_OR_NULL(dsi_axi) || IS_ERR_OR_NULL(dsi_esc));

	if (clk_enable (dsi_axi)) {
		printk(KERN_ERR "Failed to enable the DSI[%d] AXI clock\n", 
                	dsi_bus);
		return -EIO;
	}
       
	if (clk_set_rate(dsi_esc, esc_clk_hz)) {
		printk(KERN_ERR "Failed to set the DSI[%d] ESC clk to %d Hz\n", 
                	dsi_bus, esc_clk_hz);
		return -EIO;
	}
	if (clk_enable(dsi_esc)) {
		printk(KERN_ERR "Failed to enable the DSI[%d] ESC clk\n", 
                	dsi_bus );
		return -EIO;
	}
#endif

	return 0;
}

int brcm_enable_dsi_pll_clocks(
	unsigned int dsi_bus, 
        unsigned int dsi_pll_hz, 
        unsigned int dsi_pll_ch_div, 
        unsigned int esc_clk_hz )
{
#ifndef CONFIG_MACH_BCM_FPGA
	struct	clk *dsi_pll;
	struct	clk *dsi_pll_ch;
	u32	pixel_pll_val;
        u32	dsi_pll_ch_hz;
	u32	dsi_pll_ch_hz_csl;
	
	if (g_display_enabled) {
		printk(KERN_ERR "skipping brcm_enable_dsi_pll_clocks\n");
		return 0;
	}
	printk(KERN_INFO "brcm_enable_dsi_pll_clocks\n");
	/* DSI timing is set-up in CSL/cHal using req. clock values */
	dsi_pll_ch_hz_csl = dsi_pll_hz / dsi_pll_ch_div;

	dsi_pll     = clk_get (NULL, "dsi_pll");
	dsi_pll_ch  = clk_get (NULL, dsi_bus_clk[dsi_bus].dsi_pll_ch);
	BUG_ON(IS_ERR_OR_NULL(dsi_pll) || IS_ERR_OR_NULL(dsi_pll_ch));
       
	if (clk_set_rate(dsi_pll, dsi_pll_hz)) {
		printk(KERN_ERR "Failed to set the DSI[%d] PLL to %d Hz\n", 
                	dsi_bus, dsi_pll_hz);
		return -EIO;
	}
	if (clk_enable(dsi_pll)) {
		printk(KERN_ERR "Failed to enable the DSI[%d] PLL\n", dsi_bus);
		return -EIO;
	}
	
        dsi_pll_ch_hz = clk_get_rate(dsi_pll) / dsi_pll_ch_div;
	
	if (clk_set_rate(dsi_pll_ch, dsi_pll_ch_hz)) {
		printk(KERN_ERR "Failed to set the DSI[%d] PLL CH to %d Hz\n", 
                	dsi_bus, dsi_pll_ch_hz);
		return -EIO;
	}
        
	if (clk_enable(dsi_pll_ch)) {
		printk(KERN_ERR "Failed to enable DSI[%d] PLL CH\n", dsi_bus);
		return -EIO;
	}

#if 0 /* RDB doesn't recommend using DDR2 clock */
       	#define DSI_CORE_MAX_HZ	 125000000
	
	if(	(dsi_pll_ch_hz_csl >> 1) <= DSI_CORE_MAX_HZ)
		pixel_pll_val = DSI_TXDDRCLK;
	else if((dsi_pll_ch_hz_csl >> 2) <= DSI_CORE_MAX_HZ)
		pixel_pll_val = DSI_TXDDRCLK2;
	else 
		pixel_pll_val = DSI_TX0_BCLKHS;
#else
	#define MAX_DATARATE_FOR_DDR (200*1000*1000)
	if (dsi_pll_ch_hz_csl > MAX_DATARATE_FOR_DDR)
		pixel_pll_val = DSI_TX0_BCLKHS;
	else
		pixel_pll_val = DSI_TXDDRCLK;
#endif

        if (mm_ccu_set_pll_select(dsi_bus_clk[dsi_bus].pixel_pll_sel,
        	pixel_pll_val)) {
                
		printk(KERN_ERR "Failed to set DSI[%d] PIXEL PLL Sel to %d\n", 
                	dsi_bus, pixel_pll_val);
		return -EIO;
        }
#endif

	return 0;
}



int brcm_disable_dsi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node, u32 dsi_bus)
{
#ifndef CONFIG_MACH_BCM_FPGA
	struct clk *dsi_axi;
	struct clk *dsi_esc;

	dsi_axi    = clk_get(NULL, dsi_bus_clk[dsi_bus].dsi_axi);
	dsi_esc    = clk_get(NULL, dsi_bus_clk[dsi_bus].dsi_esc);
	BUG_ON(IS_ERR_OR_NULL(dsi_axi) || IS_ERR_OR_NULL(dsi_esc));

	clk_disable(dsi_axi);
	clk_disable(dsi_esc);

	if (pi_mgr_dfs_request_update(dfs_node, PI_MGR_DFS_MIN_VALUE))
	{
	    printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
	    return  -EIO;
	}
#endif

	return 0;
}


int brcm_disable_dsi_pll_clocks(u32 dsi_bus)
{
#ifndef CONFIG_MACH_BCM_FPGA
	struct clk *dsi_pll;
	struct clk *dsi_pll_ch;

	dsi_pll    = clk_get (NULL, "dsi_pll");
	dsi_pll_ch = clk_get (NULL, dsi_bus_clk[dsi_bus].dsi_pll_ch);
	BUG_ON(IS_ERR_OR_NULL(dsi_pll) || IS_ERR_OR_NULL(dsi_pll_ch));

	mm_ccu_set_pll_select(dsi_bus_clk[dsi_bus].pixel_pll_sel, DSI_NO_CLOCK);
	clk_disable(dsi_pll_ch);
	clk_disable(dsi_pll);
#endif

	return 0;
}

#else  // !__DSI_USE_CLK_API__

int brcm_enable_dsi_lcd_clocks(
	struct pi_mgr_dfs_node* dfs_node,
	unsigned int dsi_bus, 
        unsigned int dsi_pll_hz, 
        unsigned int dsi_pll_ch_div, 
        unsigned int esc_clk_hz )
{

#ifndef CONFIG_MACH_BCM_FPGA
	if (pi_mgr_dfs_request_update(dfs_node, PI_OPP_TURBO))
	{
	    printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
	    return  -EIO;
	}

#endif
	return 0;
}

int brcm_disable_dsi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node, u32 dsi_bus)
{
#ifndef CONFIG_MACH_BCM_FPGA

	if (pi_mgr_dfs_request_update(dfs_node, PI_MGR_DFS_MIN_VALUE));
	{
	    printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
	    return  -EIO;
	}
#endif
	return 0;
}

#endif  // #ifdef __DSI_USE_CLK_API__
