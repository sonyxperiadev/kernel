/*****************************************************************************
*
* Island-specific clock framework
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

#include <mach/clock.h>
#include <asm/io.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_kps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_ikps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_ikps_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khubaon_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_khub_clk_mgr_reg.h>
//#include <mach/rdb/brcm_rdb_pwrmgr.h>

/* Proc clocks */
static struct proc_clock arm_clk = {
	.clk	=	{
		.name	=	"arm_clk",
		.rate	=	700*CLOCK_1M,
		.div	=	1,
		.id	=	-1,
		.ops	=	&proc_clk_ops,
	},
	.proc_clk_mgr_base = PROC_CLK_BASE_ADDR,
};

/* ARM perhiperhal clock */
DECLARE_REF_CLK         (arm_periph, ARM_PERIPH, 0, 2, name_to_clk(arm));

/* Ref clocks */
DECLARE_REF_CLK		(crystal, CRYSTAL, 			26*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(frac_1m, FRAC_1M,			1*CLOCK_1M,		1,	0);
DECLARE_REF_CLK		(ref_96m_varVDD, REF_96M_VARVDD,		96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_96m, VAR_96M,			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_96m, REF_96M,			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_500m_varVDD, VAR_500M_VARVDD,		500*CLOCK_1M,	1,	0);

DECLARE_REF_CLK		(ref_1m,	REF_1M, 		1*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_32k, REF_32K,			32*CLOCK_1K,	1,	0);
DECLARE_REF_CLK		(misc_32k, MISC_32K,			32*CLOCK_1K,	1,	0);

DECLARE_REF_CLK		(ref_312m, REF_312M,			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK		(ref_208m, REF_208M,			208*CLOCK_1M,	0,	name_to_clk(ref_312m));
DECLARE_REF_CLK		(ref_156m, REF_156M,			156*CLOCK_1M,	2,	name_to_clk(ref_312m));
DECLARE_REF_CLK		(ref_104m, REF_104M,			104*CLOCK_1M,	3,	name_to_clk(ref_312m));
DECLARE_REF_CLK		(ref_52m, REF_52M,			52*CLOCK_1M,	2,	name_to_clk(ref_104m));
DECLARE_REF_CLK		(ref_26m, REF_26M,			26*CLOCK_1M,	6,	name_to_clk(ref_156m));
DECLARE_REF_CLK		(ref_13m, REF_13M,			13*CLOCK_1M,	4,	name_to_clk(ref_52m));

DECLARE_REF_CLK		(var_312m, VAR_312M,			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK		(var_208m, VAR_208M,			208*CLOCK_1M,	0,	name_to_clk(var_312m));
DECLARE_REF_CLK		(var_156m, VAR_156M,			156*CLOCK_1M,	2,	name_to_clk(var_312m));
DECLARE_REF_CLK		(var_104m, VAR_104M,			104*CLOCK_1M,	3,	name_to_clk(var_312m));
DECLARE_REF_CLK		(var_52m, VAR_52M,			52*CLOCK_1M,	2,	name_to_clk(var_104m));
DECLARE_REF_CLK		(var_13m, VAR_13M,			13*CLOCK_1M,	4,	name_to_clk(var_52m));

//DECLARE_REF_CLK		(usbh_48m, USBH_48M,			48*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_cx40, REF_CX40,			153600*CLOCK_1K,1,	0);	// FIXME
/* CCU clock */
/*****************************************************************************
	Reference clocks
*****************************************************************************/
DECLARE_CCU_CLK(kpm_ccu, 2, KONA_MST, KPM, MASK,
	26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 208*CLOCK_1M, 312*CLOCK_1M, 312*CLOCK_1M);

DECLARE_CCU_CLK(kps_ccu, 2, KONA_SLV, IKPS, MASK,
	26*CLOCK_1M,  52*CLOCK_1M,  78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, 156*CLOCK_1M);

DECLARE_CCU_CLK(khubaon_ccu, 4, AON, KHUBAON, MASK1,
         26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
        156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_CCU_CLK2(khub_ccu, 6, HUB, KHUB, MASK1, MASK2,
	26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
	104*CLOCK_1M, 208*CLOCK_1M, 312*CLOCK_1M, CLOCK_UNUSED);

/*****************************************************************************
	Bus clocks
*****************************************************************************/
/* KPM bus clock */

DECLARE_BUS_CLK(usb_otg, USB_OTG, USB_OTG_AHB, kpm_ccu, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,  52*CLOCK_1M,
	78*CLOCK_1M, 104*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M);

DECLARE_BUS_CLK(usbh_ahb, USB_EHCI, USBH_AHB, kpm_ccu, KONA_MST, KPM,
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

/* KPS bus clock */
DECLARE_BUS_CLK(hsm_ahb, HSM, HSM_AHB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  52*CLOCK_1M,  78*CLOCK_1M,  104*CLOCK_1M,
	 156*CLOCK_1M,  156*CLOCK_1M);

DECLARE_BUS_CLK(hsm_apb, HSM, HSM_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(spum_open_apb, SPUM_OPEN_APB, SPUM_OPEN_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(spum_sec_apb, SPUM_SEC_APB, SPUM_SEC_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK_NO_GATING_SEL(apb1, APB1, APB1, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(timers_apb, TIMERS, TIMERS_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(ssp0_apb, SSP0, SSP0_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(ssp2_apb, SSP2, SSP2_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(dmac_mux_apb, DMAC_MUX, DMAC_MUX_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb4_apb, UARTB4, UARTB4_APB, kps_ccu, KONA_SLV, IKPS,
         26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
         52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb3_apb, UARTB3, UARTB3_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb2_apb, UARTB2, UARTB2_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(uartb_apb, UARTB, UARTB_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(pwm_apb, PWM, PWM_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

/* PWM has no divider - looks more likely bus clock, somehow in the peripheral clock zone  */
DECLARE_BUS_CLK(pwm, PWM, PWM, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,
	 26*CLOCK_1M,  26*CLOCK_1M);

DECLARE_BUS_CLK(bbl_apb, BBL, BBL_REG_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(bsc1_apb, BSC1, BSC1_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(bsc2_apb, BSC2, BSC2_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK_NO_GATING_SEL(apb2, APB2_REG, APB2_REG, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

/* KHUBAON bus clocks  */

DECLARE_BUS_CLK(pmu_bsc_apb, PMU_BSC, PMU_BSC_APB, khubaon_ccu, AON, KHUBAON,
         26*CLOCK_1M,  52*CLOCK_1M, 78*CLOCK_1M, 104*CLOCK_1M,
        156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(audioh_apb, AUDIOH, AUDIOH_APB, khub_ccu, HUB, KHUB,
	26*CLOCK_1M,  26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M,
	26*CLOCK_1M, 26*CLOCK_1M, 26*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(ssp3_apb, SSP3, SSP3_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,
	 26*CLOCK_1M,  26*CLOCK_1M, 26*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(ssp4_apb, SSP4, SSP4_APB, khub_ccu, HUB, KHUB,
	 26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,  26*CLOCK_1M,
	 26*CLOCK_1M,  26*CLOCK_1M, 26*CLOCK_1M, CLOCK_UNUSED);

/* audioh_156m and audioh_2p4m clocks resemple BUS clock, hence declaring as
 *  * bus clokc. */
DECLARE_BUS_CLK(audioh_156m, AUDIOH, AUDIOH_156M, khub_ccu, HUB, KHUB,
	156*CLOCK_1M,  156*CLOCK_1M, 156*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 156*CLOCK_1M, 156*CLOCK_1M, CLOCK_UNUSED);

DECLARE_BUS_CLK(audioh_2p4m, AUDIOH, AUDIOH_2P4M, khub_ccu, HUB, KHUB,
	2400*CLOCK_1K,  2400*CLOCK_1K, 2400*CLOCK_1K, 2400*CLOCK_1K,
	2400*CLOCK_1K, 2400*CLOCK_1K, 2400*CLOCK_1K, CLOCK_UNUSED);

DECLARE_BUS_CLK(gpiokp_apb,  GPIOKP , GPIOKP_APB,  khubaon_ccu, AON, KHUBAON,
                26*CLOCK_1M, 52*CLOCK_1M, CLOCK_UNUSED, 52*CLOCK_1M,
                78*CLOCK_1M, CLOCK_UNUSED);

/*****************************************************************************
	Peripheral clocks
*****************************************************************************/

/* KPM peripheral */

static struct clk *usbh_48m_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_96m),
	name_to_clk(ref_96m),
};

static struct clk_src usbh_48m_clk_src = {
	.total		=	ARRAY_SIZE(usbh_48m_clk_src_tbl),
	.sel		=	2,
	.parents	=	usbh_48m_clk_src_tbl,
};
DECLARE_PERI_CLK_FIXED_DIV(usbh_48m, USB_EHCI, USBH_48M, ref_96m, 48*CLOCK_1M, DIV_TRIG, KONA_MST, KPM);
DECLARE_BUS_CLK(usbh_12m, USB_EHCI, USBH_12M, usbh_48m, KONA_MST, KPM,
	12*CLOCK_1M, 12*CLOCK_1M, 12*CLOCK_1M, 12*CLOCK_1M,
	12*CLOCK_1M, 12*CLOCK_1M, 12*CLOCK_1M, 12*CLOCK_1M);

static struct clk *sdio_clk_src_tbl[] =
{
	name_to_clk(crystal),
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

DECLARE_PERI_CLK_NO_DIV_COUNT(bsc1, BSC1, ref_13m, 13*CLOCK_1M, DIV_TRIG, KONA_SLV, IKPS);
DECLARE_PERI_CLK_NO_DIV_COUNT(bsc2, BSC2, ref_13m, 13*CLOCK_1M, DIV_TRIG, KONA_SLV, IKPS);

static struct clk *ssp_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_104m),
	name_to_clk(ref_104m),
	name_to_clk(var_96m),
	name_to_clk(ref_96m),
};

static struct clk_src ssp0_clk_src = {
	.total		=	ARRAY_SIZE(ssp_clk_src_tbl),
	.sel		=	1,
	.parents	=	ssp_clk_src_tbl,
};
DECLARE_PERI_CLK(ssp0, SSP0, SSP0, var_104m, 52*CLOCK_1M, 2, DIV_TRIG, KONA_SLV, IKPS, 0);

static struct clk *ssp0_audio_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_312m),
	name_to_clk(ref_cx40),
};

static struct clk_src ssp0_audio_clk_src = {
	.total		=	ARRAY_SIZE(ssp0_audio_clk_src_tbl),
	.sel		=	2,
	.parents	=	ssp0_audio_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV3(ssp0_audio, SSP0, SSP0_AUDIO, SSP0_AUDIO, ref_cx40, 153600*CLOCK_1K, 1, DIV_TRIG, KONA_SLV, IKPS, 0);

static struct clk *ssp2_audio_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_312m),
	name_to_clk(ref_cx40),
};

static struct clk_src ssp2_audio_clk_src = {
	.total		=	ARRAY_SIZE(ssp2_audio_clk_src_tbl),
	.sel		=	2,
	.parents	=	ssp2_audio_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV3(ssp2_audio, SSP2, SSP2_AUDIO, SSP2_AUDIO, ref_cx40, 153600*CLOCK_1K, 1, DIV_TRIG, KONA_SLV, IKPS, 0);

static struct clk *uart_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_156m),
	name_to_clk(ref_156m),
};

static struct clk_src uartb4_clk_src = {
        .total          =       ARRAY_SIZE(uart_clk_src_tbl),
        .sel            =       1,
        .parents        =       uart_clk_src_tbl,
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
DECLARE_PERI_CLK(uartb4, UARTB4, UARTB4, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, IKPS, 8);
DECLARE_PERI_CLK(uartb3, UARTB3, UARTB3, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, IKPS, 8);
DECLARE_PERI_CLK(uartb2, UARTB2, UARTB2, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, IKPS, 8);
DECLARE_PERI_CLK(uartb, UARTB, UARTB, var_156m, 13*CLOCK_1M, 12, DIV_TRIG, KONA_SLV, IKPS, 8);

static struct clk *timers_clk_src_tbl[] =
{
	name_to_clk(ref_1m),
	name_to_clk(ref_32k),
};

static struct clk_src timers_clk_src = {
	.total		=	ARRAY_SIZE(timers_clk_src_tbl),
	.sel		=	1,
	.parents	=	timers_clk_src_tbl,
};
DECLARE_PERI_CLK_NO_DIV_COUNT(timers, TIMERS, ref_32k, 32*CLOCK_1K, DIV_TRIG, KONA_SLV, IKPS);

static struct clk *spum_clk_src_tbl[] =
{
	name_to_clk(var_312m),
	name_to_clk(ref_312m),
};

static struct clk_src spum_open_clk_src = {
	.total		=	ARRAY_SIZE(spum_clk_src_tbl),
	.sel		=	0,
	.parents	=	spum_clk_src_tbl,
};

static struct clk_src spum_sec_clk_src = {
	.total		=	ARRAY_SIZE(spum_clk_src_tbl),
	.sel		=	0,
	.parents	=	spum_clk_src_tbl,
};

DECLARE_PERI_CLK(spum_open, SPUM_OPEN, SPUM_OPEN, var_312m, 28*CLOCK_1M, 11, DIV_TRIG, KONA_SLV, IKPS, 0);
DECLARE_PERI_CLK(spum_sec, SPUM_SEC, SPUM_SEC, var_312m, 28*CLOCK_1M, 11, DIV_TRIG, KONA_SLV, IKPS, 0);

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

static struct clk *caph_srcmixer_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(ref_312m),
};

static struct clk_src caph_srcmixer_clk_src = {
	.total          =       ARRAY_SIZE(caph_srcmixer_clk_src_tbl),
	.sel            =       1,
	.parents        =       caph_srcmixer_clk_src_tbl,
};

DECLARE_PERI_CLK(caph_srcmixer, CAPH, CAPH_SRCMIXER, crystal, 26*CLOCK_1M, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

static struct clk *ssp3_audio_clk_src_tbl[] = {
	name_to_clk(crystal),
	name_to_clk(ref_312m),
	name_to_clk(ref_cx40),
};

static struct clk_src ssp3_audio_clk_src = {
	.total          =       ARRAY_SIZE(ssp3_audio_clk_src_tbl),
	.sel            =       2,
	.parents        =       ssp3_audio_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV3(ssp3_audio, SSP3, SSP3_AUDIO, SSP3_AUDIO, ref_cx40, 153600*CLOCK_1K, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

static struct clk *ssp4_audio_clk_src_tbl[] = {
	name_to_clk(crystal),
	name_to_clk(ref_312m),
	name_to_clk(ref_cx40),
};

static struct clk_src ssp4_audio_clk_src = {
	.total          =       ARRAY_SIZE(ssp4_audio_clk_src_tbl),
	.sel            =       2,
	.parents        =       ssp4_audio_clk_src_tbl,
};

DECLARE_PERI_CLK_PRE_DIV3(ssp4_audio, SSP4, SSP4_AUDIO, SSP4_AUDIO, ref_cx40, 153600*CLOCK_1K, 1, PERIPH_SEG_TRG, HUB, KHUB, 0);

static struct clk *audioh_26m_clk_src_tbl[] = {
	name_to_clk(crystal),
	name_to_clk(ref_26m),
};

static struct clk_src audioh_26m_clk_src = {
	.total          =       ARRAY_SIZE(audioh_26m_clk_src_tbl),
	.sel            =       0,
	.parents        =       audioh_26m_clk_src_tbl,
};

DECLARE_PERI_CLK_NO_DIV_COUNT2(audioh_26m, AUDIOH_26M, AUDIOH, crystal, 26*CLOCK_1M, PERIPH_SEG_TRG, HUB, KHUB);

/* table for registering clock */
struct clk_lookup island_clk_tbl[] =
{
	CLK_LK(arm),

	/* Reference clocks */
	CLK_LK(arm_periph),
	CLK_LK(crystal),
	CLK_LK(frac_1m),
	CLK_LK(ref_96m_varVDD),
	CLK_LK(var_96m),
	CLK_LK(ref_96m),
	CLK_LK(var_500m_varVDD),
	CLK_LK(ref_32k),
	CLK_LK(misc_32k),
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
	CLK_LK(ref_cx40),

	/* CCUs */
	CLK_LK(kpm_ccu),
	CLK_LK(kps_ccu),
	CLK_LK(khub_ccu),
	CLK_LK(khubaon_ccu),

	/* Bus clocks */
	CLK_LK(usb_otg),
	CLK_LK(usbh_ahb),
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
	CLK_LK(bbl_apb),
	CLK_LK(timers_apb),
	CLK_LK(uartb_apb),
	CLK_LK(uartb2_apb),
	CLK_LK(uartb3_apb),
	CLK_LK(uartb4_apb),
	CLK_LK(pmu_bsc_apb),
	CLK_LK(hsm_ahb),
	CLK_LK(hsm_apb),
	CLK_LK(spum_open_apb),
	CLK_LK(spum_sec_apb),
	CLK_LK(spum_open_apb),
	CLK_LK(spum_sec_apb),
	CLK_LK(dmac_mux_apb),
	CLK_LK(ssp0_apb),
	CLK_LK(ssp2_apb),
	CLK_LK(ssp3_apb),
	CLK_LK(ssp4_apb),
	CLK_LK(pwm_apb),
	CLK_LK(gpiokp_apb),
	CLK_LK(apb1),
	CLK_LK(apb2),

	/* Peripheral clocks */
	CLK_LK(hub),
	CLK_LK(usbh_48m),
	CLK_LK(usbh_12m),
	CLK_LK(sdio1),
	CLK_LK(sdio2),
	CLK_LK(sdio3),
	CLK_LK(sdio4),
	CLK_LK(bsc1),
	CLK_LK(bsc2),
	CLK_LK(uartb),
	CLK_LK(uartb2),
	CLK_LK(uartb3),
	CLK_LK(uartb4),
	CLK_LK(pmu_bsc),
	CLK_LK(pwm),
	CLK_LK(ssp0),
	CLK_LK(ssp0_audio),
	CLK_LK(ssp2_audio),
	CLK_LK(timers),
	CLK_LK(spum_open),
	CLK_LK(spum_sec),
	CLK_LK(caph_srcmixer),
	CLK_LK(ssp3_audio),
	CLK_LK(ssp4_audio),
	CLK_LK(audioh_apb),
	CLK_LK(audioh_156m),
	CLK_LK(audioh_2p4m),
	CLK_LK(audioh_26m),
};

int __init clock_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(island_clk_tbl); i++)
		clk_register(&island_clk_tbl[i]);

	return 0;
}

int __init clock_late_init(void)
{
#ifdef CONFIG_DEBUG_FS
	int i;
	clock_debug_init();
	for (i=0; i<ARRAY_SIZE(island_clk_tbl); i++)
		clock_debug_add_clock (island_clk_tbl[i].clk);
#endif
	return 0;
}

late_initcall(clock_late_init);

unsigned long clock_get_xtal(void)
{
	unsigned long xtal_tbl[] = { 13000000, 26000000, 19200000, 38400000};
	unsigned int reg = readl (KONA_CHIPREG_VA + CHIPREG_ISLAND_STRAP_OFFSET);

	return xtal_tbl[ (reg & CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_MASK)
		>> CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_SHIFT];
}
