/*****************************************************************************
*
* Rhea-specific clock framework
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/math64.h>
#include <linux/delay.h>

#include <mach/clock.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_sysmap_a9.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_root_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_pwrmgr.h>
#include <linux/clk.h>
#include <asm/io.h>

/*****************************************************************************
	Reference clocks
*****************************************************************************/
DECLARE_REF_CLK (crystal,	CRYSTAL, 			26*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (dummy,		DUMMY, 				0,		1,	0);
DECLARE_REF_CLK (frac_1m,	FRAC_1M,			1*CLOCK_1M,		1,	0);
DECLARE_REF_CLK (var_96m,	VAR_96M,			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK (ref_96m,	REF_96M,			96*CLOCK_1M,	1,	0);

DECLARE_REF_CLK (ref_312m,	REF_312M,			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK (ref_208m,	REF_208M,			208*CLOCK_1M,	0,	name_to_clk(ref_312m));
DECLARE_REF_CLK (ref_156m,	REF_156M,			156*CLOCK_1M,	2,	name_to_clk(ref_312m));
DECLARE_REF_CLK (ref_104m,	REF_104M,			104*CLOCK_1M,	3,	name_to_clk(ref_312m));
DECLARE_REF_CLK (ref_52m,	REF_52M,			52*CLOCK_1M,	2,	name_to_clk(ref_104m));
DECLARE_REF_CLK (ref_13m,	REF_13M,			13*CLOCK_1M,	4,	name_to_clk(ref_52m));
DECLARE_REF_CLK (ref_26m,	REF_26M,			26*CLOCK_1M,	6,	name_to_clk(ref_156m));

DECLARE_REF_CLK (var_312m,	VAR_312M,			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK (var_208m,	VAR_208M,			208*CLOCK_1M,	0,	name_to_clk(var_312m));
DECLARE_REF_CLK (var_156m,	VAR_156M,			156*CLOCK_1M,	2,	name_to_clk(var_312m));
DECLARE_REF_CLK (var_104m,	VAR_104M,			104*CLOCK_1M,	3,	name_to_clk(var_312m));
DECLARE_REF_CLK (var_52m,	VAR_52M,			52*CLOCK_1M,	2,	name_to_clk(var_104m));
DECLARE_REF_CLK (var_13m,	VAR_13M,			13*CLOCK_1M,	4,	name_to_clk(var_52m));

DECLARE_REF_CLK	(usbh_48m,	USBH_48M,			48*CLOCK_1M,	1,	0);

DECLARE_CCU_CLK(kpm_ccu, 2, KONA_MST, KPM, MASK,
	26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 208*CLOCK_1M, 312*CLOCK_1M, 312*CLOCK_1M);

DECLARE_CCU_CLK(kps_ccu, 2, KONA_SLV, KPS, MASK,
	26*CLOCK_1M,  52*CLOCK_1M,  78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, 156*CLOCK_1M);

DECLARE_CCU_CLK(khubaon_ccu, 4, AON, KHUBAON, MASK1,
         26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
        156*CLOCK_1M, CLOCK_UNUSED);

/*****************************************************************************
	Bus clocks
*****************************************************************************/
/* KPM bus clock */
DECLARE_BUS_CLK(usb_otg, USB_OTG, USB_OTG_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio1_ahb, SDIO1, SDIO1_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio2_ahb, SDIO2, SDIO2_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio3_ahb, SDIO3, SDIO3_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(sdio4_ahb, SDIO4, SDIO4_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio1_sleep, SDIO1, SDIO1_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio2_sleep, SDIO2, SDIO2_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio3_sleep, SDIO3, SDIO3_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING_SEL(sdio4_sleep, SDIO4, SDIO4_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK(timers_apb, TIMERS, TIMERS_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb3_apb, UARTB3, UARTB3_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb2_apb, UARTB2, UARTB2_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb_apb, UARTB, UARTB_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(bsc1_apb, BSC1, BSC1_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(bsc2_apb, BSC2, BSC2_APB, kps_ccu, KONA_SLV, KPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

/* KHUBAON bus clocks  */

DECLARE_BUS_CLK(pmu_bsc_apb, PMU_BSC, PMU_BSC_APB, khubaon_ccu, AON, KHUBAON,
         26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
        156*CLOCK_1M, CLOCK_UNUSED);

/*****************************************************************************
	Peripheral clocks
*****************************************************************************/
/* KPM peripheral */
static struct clk *sdio_clk_src_tbl[] =
{
    /*parent as crystal is not working currently. SDIO clock is not getting
     * stabilised with crystal clock. Hence this temp fix.*/
/*	name_to_clk(crystal), */
	name_to_clk(dummy),
	name_to_clk(var_52m),
	name_to_clk(ref_52m),
	name_to_clk(var_96m),
	name_to_clk(ref_96m),
};

static struct clk_src sdio1_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

static struct clk_src sdio2_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

static struct clk_src sdio3_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

static struct clk_src sdio4_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.sel		=	2,
	.parents	=	sdio_clk_src_tbl,
};

DECLARE_PERI_CLK(sdio1, SDIO1, SDIO1, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio2, SDIO2, SDIO2, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio3, SDIO3, SDIO3, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio4, SDIO4, SDIO4, ref_52m, 26*CLOCK_1M, 2, DIV_TRIG, KONA_MST, KPM, 0);

/* KPS peripheral clock */
static struct clk *bsc_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_104m),
	name_to_clk(ref_104m),
	name_to_clk(var_13m),
	name_to_clk(ref_13m),
};

static struct clk_src bsc1_clk_src = {
	.total		=	ARRAY_SIZE(bsc_clk_src_tbl),
	.sel		=	3,
	.parents	=	bsc_clk_src_tbl,
};

static struct clk_src bsc2_clk_src = {
	.total		=	ARRAY_SIZE(bsc_clk_src_tbl),
	.sel		=	3,
	.parents	=	bsc_clk_src_tbl,
};

DECLARE_PERI_CLK_NO_DIV_COUNT(bsc1, BSC1, ref_13m, 13*CLOCK_1M, DIV_TRIG, KONA_SLV, KPS);
DECLARE_PERI_CLK_NO_DIV_COUNT(bsc2, BSC2, ref_13m, 13*CLOCK_1M, DIV_TRIG, KONA_SLV, KPS);

static struct clk *uart_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_156m),
	name_to_clk(ref_156m),
};

static struct clk_src uartb3_clk_src = {
	.total		=	ARRAY_SIZE(uart_clk_src_tbl),
	.sel		=	1,
	.parents	=	uart_clk_src_tbl,
};
static struct clk_src uartb2_clk_src = {
	.total		=	ARRAY_SIZE(uart_clk_src_tbl),
	.sel		=	1,
	.parents	=	uart_clk_src_tbl,
};
static struct clk_src uartb_clk_src = {
	.total		=	ARRAY_SIZE(uart_clk_src_tbl),
	.sel		=	1,
	.parents	=	uart_clk_src_tbl,
};
DECLARE_PERI_CLK(uartb3, UARTB3, UARTB3, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, KPS, 8);
DECLARE_PERI_CLK(uartb2, UARTB2, UARTB2, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, KPS, 8);
DECLARE_PERI_CLK(uartb, UARTB, UARTB, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, KPS, 8);

/* KHUBAON peripheral clock */
/* HUBAON clock -- Needs work around as the enable bit is in HUB_DIV register
 * instead of HUB_CLKGATE*/
/* APB6 clock -- Needs work around as the enable bit is in HUB_DIV register
 * instead of APB6_CLKGATE*/

static struct clk *pmu_bsc_clk_src_tbl[] =
{
        name_to_clk(crystal),
        /*pmu_bsc_var clock is not defined in the clock tree..!! this need to
         * be confirmed and then enabled */
        /*name_to_clk(pmu_bsc_var), */
        //name_to_clk(bbl_32k),
};

static struct clk_src pmu_bsc_clk_src = {
        .total          =       ARRAY_SIZE(pmu_bsc_clk_src_tbl),
        .sel            =       0,
        .parents        =       pmu_bsc_clk_src_tbl,
};

DECLARE_PERI_CLK(pmu_bsc, PMU_BSC, PMU_BSC, crystal, 32*CLOCK_1K, 1, PERIPH_SEG_TRG, AON, KHUBAON, 0);

/* KHUB peripheral clock */
static struct clk *hub_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_312m),
};

static struct clk_src hub_clk_src = {
	.total		=	ARRAY_SIZE(hub_clk_src_tbl),
	.sel		=	1,
	.parents	=	hub_clk_src_tbl,
};

DECLARE_PERI_CLK(hub, HUB, HUB, var_312m, 312*CLOCK_1M, 1, HUB_SEG_TRG, HUB, KHUB, 1);

/* table for registering clock */
static struct __init clk_lookup hana_clk_tbl[] =
{
/*Reference clocks */
	CLK_LK(crystal),
	CLK_LK(frac_1m),
	CLK_LK(var_96m),
	CLK_LK(ref_96m),

	CLK_LK(ref_312m),
	CLK_LK(ref_208m),
	CLK_LK(ref_104m),
	CLK_LK(ref_52m),
	CLK_LK(ref_13m),
	CLK_LK(ref_26m),

	CLK_LK(var_312m),
	CLK_LK(var_208m),
	CLK_LK(var_156m),
	CLK_LK(var_52m),
	CLK_LK(var_13m),

	CLK_LK(usbh_48m),
/* CCUs */
	CLK_LK(kpm_ccu),
	CLK_LK(kps_ccu),
/* Bus clocks */
	CLK_LK(sdio1_ahb),
	CLK_LK(sdio2_ahb),
	CLK_LK(sdio3_ahb),
	CLK_LK(sdio4_ahb),
	CLK_LK(sdio1_sleep),
	CLK_LK(sdio2_sleep),
	CLK_LK(sdio3_sleep),
	CLK_LK(sdio4_sleep),
	CLK_LK(bsc1_apb),
	CLK_LK(bsc2_apb),
	CLK_LK(timers_apb),
	CLK_LK(uartb3_apb),
	CLK_LK(uartb2_apb),
	CLK_LK(uartb_apb),

	CLK_LK(pmu_bsc_apb),

/* Perepheral clocks */
	/* HUB CCU */
	CLK_LK(hub),


	CLK_LK(sdio1),
	CLK_LK(sdio2),
	CLK_LK(sdio3),
	CLK_LK(sdio4),

	CLK_LK(bsc1),
	CLK_LK(bsc2),

	CLK_LK(usb_otg),
	CLK_LK(uartb3),
	CLK_LK(uartb2),
	CLK_LK(uartb),

	CLK_LK(pmu_bsc),
};

static int clock_module_temp_fixes(void)
{
   struct clk *temp_clk;
   /***************************************************
    * Temp fixes for clocks in HUBAON CCU.
    **************************************************/
    /* Enable ACI APB clock */
    temp_clk = clk_get(NULL, "aci_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    /*Set SIM clock rate to 26Mhz and enabale it */
    temp_clk = clk_get(NULL, "sim_clk");
    clk_set_rate(temp_clk, 26*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);
     /*Set SIM2 clock rate to 26Mhz and enabale it */
    temp_clk = clk_get(NULL, "sim2_clk");
    clk_set_rate(temp_clk, 26*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);
    /***************************************************
    * Temp fixes for clocks in HUB CCU.
    **************************************************/
    /*enable and then disable. so that HUB CCU enable gets called and masks
     * gating policy bits*/
    /***************************************************
    * Temp fixes for clocks in Kona master CCU.
    **************************************************/
    temp_clk = clk_get(NULL, "usb_otg_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

   /***************************************************
    * Temp fixes for clocks in Kona slave CCU.
    **************************************************/
    temp_clk = clk_get(NULL, "uartb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb2_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb2_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb3_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "uartb3_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc1_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc1_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc2_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "bsc2_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "pwm_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "pwm_apb_clk");
    clk_enable(temp_clk);
    clk_put(temp_clk);
   /***************************************************
    * Temp fixes for clocks in Kona slave CCU.
    **************************************************/
    temp_clk = clk_get(NULL, "mm_dma_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi0_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi0_lp");
    clk_set_rate(temp_clk, 100*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi1_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "csi1_lp");
    clk_set_rate(temp_clk, 100*CLOCK_1M);
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "isp_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "smi_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "smi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "v3d_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "vce_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi0_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi0_esc");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi1_axi");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi1_esc");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "mm_apb");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "spi_apb");
    clk_enable(temp_clk);
    clk_put(temp_clk);

    temp_clk = clk_get(NULL, "dsi_pll_o_dsi_pll");
    clk_enable(temp_clk);
    clk_put(temp_clk);



    return 0;

}

int __init clock_init(void)
{
    int i;

    for (i=0; i<ARRAY_SIZE(hana_clk_tbl); i++)
	clk_register(&hana_clk_tbl[i]);

     /*********************  TEMPORARY *************************************
     * Work arounds for clock module . this could be because of ASIC
     * errata or other limitations or special requirements.
     * -- To be revised based on future fixes.
     *********************************************************************/
    clock_module_temp_fixes();

    return 0;
}

int __init clock_late_init(void)
{
#ifdef CONFIG_DEBUG_FS
	int i;
	clock_debug_init();
	for (i=0; i<ARRAY_SIZE(hana_clk_tbl); i++)
		clock_debug_add_clock (hana_clk_tbl[i].clk);
#endif
	return 0;
}

late_initcall(clock_late_init);

unsigned long clock_get_xtal(void)
{
	return 26*CLOCK_1M;
}
