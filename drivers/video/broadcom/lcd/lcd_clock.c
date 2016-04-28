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
#define __PREVENT_MM_RETENTION

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
	int err;

	err = pi_mgr_dfs_request_update(dfs_node, PI_OPP_ECONOMY);
	if (err) {
		printk(KERN_ERR "Failed to update dfs request for SMI LCD at enable\n");
		return err;
	}

	smi_axi = clk_get (NULL, "smi_axi_clk");
	smi = clk_get (NULL, "smi_clk");
	BUG_ON(IS_ERR(smi_axi) || IS_ERR(smi));

	err = clk_set_rate(smi, 250000000);
	if (err) {
		printk(KERN_ERR "Failed to set the SMI peri clock to 250MHZ");
		return err;
	}

	err = clk_enable(smi);
	if (err) {
		printk(KERN_ERR "Failed to enable the SMI peri clock");
		return err;
	}

	err = clk_enable(smi_axi);
	if (err) {
		printk(KERN_ERR "Failed to enable the SMI bus clock");
		return err;
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
	BUG_ON(IS_ERR(smi_axi) || IS_ERR(smi));

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

static int dsi_axi_clk(int bus, bool on)
{
	int err;
	struct clk *dsi_axi = clk_get(NULL, dsi_bus_clk[bus].dsi_axi);

	BUG_ON(IS_ERR(dsi_axi));
	if (on) {
		err = clk_enable(dsi_axi);
		if (err) {
			pr_err("Failed to enable the DSI[%d] AXI clock\n",
			bus);
		return err;
		}
	} else {
		clk_disable(dsi_axi);
	}
	return 0;
}

static int dsi_esc_clk(int bus, u32 esc_clk_hz, bool on)
{
	int err;
	struct	clk *dsi_esc = clk_get(NULL, dsi_bus_clk[bus].dsi_esc);

	BUG_ON(IS_ERR(dsi_esc));

	if (on) {
		err = clk_set_rate(dsi_esc, esc_clk_hz);
		if (err) {
			pr_err("Failed to set the DSI[%d] ESC clk to %d Hz\n",
				bus, esc_clk_hz);
			return err;
		}
		err = clk_enable(dsi_esc);
		if (err) {
			pr_err("Failed to enable the DSI[%d] ESC clk\n",
				bus);
			return err;
		}
	} else {
		clk_disable(dsi_esc);
	}
	return 0;
}

static int en_disp_clks(void)
{
	struct clk *dsi_axi;
	struct clk *dsi_esc;
	struct clk *dsi_pll;
	struct clk *dsi_pll_ch;
	int err;

	if (!g_display_enabled)
		return -1;

	pr_err("Enabling display clocks\n");
	dsi_axi	= clk_get(NULL, dsi_bus_clk[0].dsi_axi);
	dsi_esc	= clk_get(NULL, dsi_bus_clk[0].dsi_esc);
	BUG_ON(IS_ERR(dsi_axi) || IS_ERR(dsi_esc));
	err = clk_enable(dsi_axi);
	if (err) {
		pr_err("Error while enabling DSI AXI clk\n");
		return err;
	}

	err = clk_enable(dsi_esc);
	if (err) {
		pr_err("Error while enabling DSI ESC clk\n");
		return err;
	}

	dsi_pll     = clk_get(NULL, "dsi_pll");
	dsi_pll_ch  = clk_get(NULL, dsi_bus_clk[0].dsi_pll_ch);
	BUG_ON(IS_ERR(dsi_pll) || IS_ERR(dsi_pll_ch));
	err = clk_enable(dsi_pll);
	if (err) {
		pr_err("Error while enabling pll\n");
		return err;
	}

	err = clk_enable(dsi_pll_ch);
	if (err) {
		pr_err("Error while enabling pll ch\n");
		return err;
	}
	return 0;
}
subsys_initcall(en_disp_clks);

static int dis_disp_clks(void)
{
	struct clk *dsi_axi;
	struct clk *dsi_esc;
	struct clk *dsi_pll;
	struct clk *dsi_pll_ch;

	if (!g_display_enabled)
		return -1;

	pr_err("Disabling display clocks\n");
	dsi_axi	= clk_get(NULL, dsi_bus_clk[0].dsi_axi);
	dsi_esc	= clk_get(NULL, dsi_bus_clk[0].dsi_esc);
	BUG_ON(IS_ERR(dsi_axi) || IS_ERR(dsi_esc));
	clk_disable(dsi_axi);
	clk_disable(dsi_esc);

	dsi_pll     = clk_get(NULL, "dsi_pll");
	dsi_pll_ch  = clk_get(NULL, dsi_bus_clk[0].dsi_pll_ch);
	BUG_ON(IS_ERR(dsi_pll) || IS_ERR(dsi_pll_ch));
	clk_disable(dsi_pll);
	clk_disable(dsi_pll_ch);

	g_display_enabled = 0;
	return 0;
}
late_initcall(dis_disp_clks);

/* Reset some lcd parts to inital status */
int brcm_init_lcd_clocks(u32 dsi_bus)
{
	struct clk *dsi_axi;

	dsi_axi = clk_get(NULL, dsi_bus_clk[dsi_bus].dsi_axi);
	BUG_ON(IS_ERR(dsi_axi));

	/*
	 * Sometimes DSI axi bus may get hang if
	 * TX (CSL_DSI_SendPacket) busy,
	 * doing a clk_reset to recovery from this error
	 */
	return clk_reset(dsi_axi);
}

#ifdef __DSI_USE_CLK_API__
int brcm_enable_dsi_lcd_clocks(
	struct pi_mgr_dfs_node* dfs_node,
	unsigned int dsi_bus, 
        unsigned int dsi_pll_hz, 
        unsigned int dsi_pll_ch_div, 
        unsigned int esc_clk_hz )
{
#ifndef CONFIG_MACH_BCM_FPGA
	
	if (pi_mgr_dfs_request_update(dfs_node, PI_OPP_ECONOMY)) {
		printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
		return  -EIO;
	}

	dsi_axi_clk(dsi_bus, true);
	dsi_esc_clk(dsi_bus, esc_clk_hz, true);

#endif

	return 0;
}

int brcm_enable_dsi_pll_clocks(
	unsigned int dsi_bus, 
        unsigned int dsi_pll_hz, 
        unsigned int dsi_pll_ch_div, 
	int dsi_pll_ch_desense_offset,
        unsigned int esc_clk_hz )
{
#ifndef CONFIG_MACH_BCM_FPGA
	struct	clk *dsi_pll;
	struct	clk *dsi_pll_ch;
	u32	pixel_pll_val;
        u32	dsi_pll_ch_hz;
	u32	dsi_pll_ch_hz_csl;
	int dsi_pll_desense_offset;
	int err;

	dsi_pll = clk_get(NULL, "dsi_pll");
	dsi_pll_ch = clk_get(NULL, dsi_bus_clk[dsi_bus].dsi_pll_ch);
	BUG_ON(IS_ERR(dsi_pll) || IS_ERR(dsi_pll_ch));

	printk(KERN_INFO "brcm_enable_dsi_pll_clocks\n");
	/* DSI timing is set-up in CSL/cHal using req. clock values */
	dsi_pll_ch_hz_csl = dsi_pll_hz / dsi_pll_ch_div;

#ifdef __PREVENT_MM_RETENTION
	dsi_axi_clk(dsi_bus, true);
#endif
	err = clk_set_rate(dsi_pll, dsi_pll_hz);
	if (err) {
		printk(KERN_ERR "Failed to set the DSI[%d] PLL to %d Hz\n", 
                	dsi_bus, dsi_pll_hz);
		return err;
	}
	err = clk_enable(dsi_pll);
	if (err) {
		printk(KERN_ERR "Failed to enable the DSI[%d] PLL\n", dsi_bus);
		return err;
	}
	
        dsi_pll_ch_hz = clk_get_rate(dsi_pll) / dsi_pll_ch_div;
	
	err = clk_set_rate(dsi_pll_ch, dsi_pll_ch_hz);
	if (err) {
		printk(KERN_ERR "Failed to set the DSI[%d] PLL CH to %d Hz\n", 
                	dsi_bus, dsi_pll_ch_hz);
		return err;
	}

	dsi_pll_desense_offset = dsi_pll_ch_desense_offset * dsi_pll_ch_div;
	err = pll_set_desense_offset(dsi_pll, dsi_pll_desense_offset);
	if (err) {
		pr_err("Failed to set desense offset to %dHz\n",
		dsi_pll_desense_offset);
		return err;
	}

	err = clk_enable(dsi_pll_ch);
	if (err) {
		printk(KERN_ERR "Failed to enable DSI[%d] PLL CH\n", dsi_bus);
		return err;
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

	err = mm_ccu_set_pll_select(dsi_bus_clk[dsi_bus].pixel_pll_sel,
		pixel_pll_val);
	if (err) {
		printk(KERN_ERR "Failed to set DSI[%d] PIXEL PLL Sel to %d\n", 
                	dsi_bus, pixel_pll_val);
		return err;
        }
#endif

	return 0;
}



int brcm_disable_dsi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node, u32 dsi_bus)
{
#ifndef CONFIG_MACH_BCM_FPGA
	dsi_axi_clk(dsi_bus, false);
	dsi_esc_clk(dsi_bus, 0, false);

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
	BUG_ON(IS_ERR(dsi_pll) || IS_ERR(dsi_pll_ch));

	mm_ccu_set_pll_select(dsi_bus_clk[dsi_bus].pixel_pll_sel, DSI_NO_CLOCK);
	clk_disable(dsi_pll_ch);
	clk_disable(dsi_pll);
#ifdef __PREVENT_MM_RETENTION
	dsi_axi_clk(dsi_bus, false);
#endif

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
	if (pi_mgr_dfs_request_update(dfs_node, PI_OPP_ECONOMY)) {
		printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
		return  -EIO;
	}

#endif
	return 0;
}

int brcm_disable_dsi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node, u32 dsi_bus)
{
#ifndef CONFIG_MACH_BCM_FPGA

	if (pi_mgr_dfs_request_update(dfs_node, PI_MGR_DFS_MIN_VALUE)) {
		printk(KERN_ERR "Failed to update dfs request for DSI LCD\n");
		return  -EIO;
	}
#endif
	return 0;
}

#endif  // #ifdef __DSI_USE_CLK_API__
