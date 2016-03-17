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

#include <linux/clk.h>
#include <plat/pi_mgr.h>

extern int g_display_enabled;
 
extern int brcm_enable_smi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node);

extern int brcm_disable_smi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node);

extern int brcm_enable_dsi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node,
	u32 dsi_bus, u32 dsi_pll_hz, u32 dsi_pll_ch_div, u32 esc_clk_hz );

extern int brcm_disable_dsi_lcd_clocks(struct pi_mgr_dfs_node* dfs_node,
	u32 dsi_bus);

extern int brcm_enable_dsi_pll_clocks(u32 dsi_bus, u32 dsi_pll_hz,
		u32 dsi_pll_ch_div, int dsi_pll_ch_desense_offset,
		u32 esc_clk_hz);

extern int brcm_disable_dsi_pll_clocks(u32 dsi_bus);

extern int brcm_init_lcd_clocks(u32 dsi_bus);

