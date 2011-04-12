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

#include <mach/clock.h>
#include <asm/io.h>
#include <linux/math64.h>
#include <mach/io_map.h>
#include <mach/rdb/brcm_rdb_sysmap_a9.h>
#include <mach/rdb/brcm_rdb_chipreg.h>
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_ikps_clk_mgr_reg.h>
//#include <mach/rdb/brcm_rdb_pwrmgr.h>



#define	DECLARE_REF_CLK(clk_name, clk_rate, clk_div, clk_parent)		\
	static struct proc_clock clk_name##_clk = {				\
		.clk	=	{						\
			.name	=	__stringify(clk_name##_clk),		\
			.parent	=	clk_parent,				\
			.rate	=	clk_rate,				\
			.div	=	clk_div,				\
			.id	=	-1,					\
			.ops	=	&ref_clk_ops,				\
		},								\
	}

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



/* Ref clocks */
DECLARE_REF_CLK		(crystal, 			26*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(frac_1m, 			1*CLOCK_1M,		1,	0);
DECLARE_REF_CLK		(ref_96m_varVDD, 		96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_96m, 			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_96m, 			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_500m_varVDD, 		500*CLOCK_1M,	1,	0);

DECLARE_REF_CLK		(ref_1m,	 		1*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_32k, 			32*CLOCK_1K,	1,	0);
DECLARE_REF_CLK		(misc_32k, 			32*CLOCK_1K,	1,	0);

DECLARE_REF_CLK		(ref_312m, 			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK		(ref_208m, 			208*CLOCK_1M,	0,	name_to_clk(ref_312m));
DECLARE_REF_CLK		(ref_156m, 			156*CLOCK_1M,	2,	name_to_clk(ref_312m));
DECLARE_REF_CLK		(ref_104m, 			104*CLOCK_1M,	3,	name_to_clk(ref_312m));
DECLARE_REF_CLK		(ref_52m, 			52*CLOCK_1M,	2,	name_to_clk(ref_104m));
DECLARE_REF_CLK		(ref_13m, 			13*CLOCK_1M,	4,	name_to_clk(ref_52m));

DECLARE_REF_CLK		(var_312m, 			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK		(var_208m, 			208*CLOCK_1M,	0,	name_to_clk(var_312m));
DECLARE_REF_CLK		(var_156m, 			156*CLOCK_1M,	2,	name_to_clk(var_312m));
DECLARE_REF_CLK		(var_104m, 			104*CLOCK_1M,	3,	name_to_clk(var_312m));
DECLARE_REF_CLK		(var_52m, 			52*CLOCK_1M,	2,	name_to_clk(var_104m));
DECLARE_REF_CLK		(var_13m, 			13*CLOCK_1M,	4,	name_to_clk(var_52m));

DECLARE_REF_CLK		(usbh_48m, 			48*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_cx40, 			153600*CLOCK_1K,1,	0);	// FIXME
/* CCU clock */
/*****************************************************************************
	Reference clocks
*****************************************************************************/
DECLARE_CCU_CLK(kpm_ccu, 2, KONA_MST, KPM,
	26*CLOCK_1M,  52*CLOCK_1M, 104*CLOCK_1M, 156*CLOCK_1M,
	156*CLOCK_1M, 208*CLOCK_1M, 312*CLOCK_1M, 312*CLOCK_1M);

DECLARE_CCU_CLK(kps_ccu, 2, KONA_SLV, IKPS,
	26*CLOCK_1M,  52*CLOCK_1M,  78*CLOCK_1M, 104*CLOCK_1M,
	156*CLOCK_1M, 156*CLOCK_1M);

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

DECLARE_BUS_CLK_NO_GATING(sdio1_sleep, SDIO1, SDIO1_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING(sdio2_sleep, SDIO2, SDIO2_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING(sdio3_sleep, SDIO3, SDIO3_SLEEP, kpm_ccu, KONA_MST, KPM,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K,
	32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K, 32*CLOCK_1K);

DECLARE_BUS_CLK_NO_GATING(sdio4_sleep, SDIO4, SDIO4_SLEEP, kpm_ccu, KONA_MST, KPM,
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

DECLARE_BUS_CLK_NO_GATING(apb1, APB1, APB1, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(timers_apb, TIMERS, TIMERS_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(ssp0_apb, SSP0, SSP0_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(dmac_mux_apb, DMAC_MUX, DMAC_MUX_APB, kps_ccu, KONA_SLV, IKPS,
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

DECLARE_BUS_CLK(bsc1_apb, BSC1, BSC1_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK(bsc2_apb, BSC2, BSC2_APB, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);

DECLARE_BUS_CLK_NO_GATING(apb2, APB2_REG, APB2_REG, kps_ccu, KONA_SLV, IKPS,
	 26*CLOCK_1M,  26*CLOCK_1M,  39*CLOCK_1M,  52*CLOCK_1M,
	 52*CLOCK_1M,  78*CLOCK_1M);


/*****************************************************************************
	Peripheral clocks
*****************************************************************************/
/* KPM peripheral */
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

DECLARE_PERI_CLK(sdio1, SDIO1, SDIO1, ref_52m, 26*CLOCK_1M, 2, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio2, SDIO2, SDIO2, ref_52m, 26*CLOCK_1M, 2, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio3, SDIO3, SDIO3, ref_52m, 26*CLOCK_1M, 2, KONA_MST, KPM, 0);
DECLARE_PERI_CLK(sdio4, SDIO4, SDIO4, ref_52m, 26*CLOCK_1M, 2, KONA_MST, KPM, 0);

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

DECLARE_PERI_CLK_NO_DIV(bsc1, BSC1, ref_13m, 13*CLOCK_1M, KONA_SLV, IKPS);
DECLARE_PERI_CLK_NO_DIV(bsc2, BSC2, ref_13m, 13*CLOCK_1M, KONA_SLV, IKPS);

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
DECLARE_PERI_CLK(ssp0, SSP0, SSP0, var_104m, 52*CLOCK_1M, 2, KONA_SLV, IKPS, 0);

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
DECLARE_PERI_CLK(uartb3, UARTB3, UARTB3, var_156m, 13*CLOCK_1M, 12, KONA_SLV, IKPS, 8);
DECLARE_PERI_CLK(uartb2, UARTB2, UARTB2, var_156m, 13*CLOCK_1M, 12, KONA_SLV, IKPS, 8);
DECLARE_PERI_CLK(uartb, UARTB, UARTB, var_156m, 13*CLOCK_1M, 12, KONA_SLV, IKPS, 8);

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
DECLARE_PERI_CLK_NO_DIV(timers, TIMERS, ref_32k, 32*CLOCK_1K, KONA_SLV, IKPS);

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

DECLARE_PERI_CLK(spum_open, SPUM_OPEN, SPUM_OPEN, var_312m, 28*CLOCK_1M, 11, KONA_SLV, IKPS, 0);
DECLARE_PERI_CLK(spum_sec, SPUM_SEC, SPUM_SEC, var_312m, 28*CLOCK_1M, 11, KONA_SLV, IKPS, 0);


/* table for registering clock */
struct clk_lookup island_clk_tbl[] =
{
	CLK_LK(arm),

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

	CLK_LK(var_312m),
	CLK_LK(var_208m),
	CLK_LK(var_156m),
	CLK_LK(var_52m),
	CLK_LK(var_13m),

	CLK_LK(usbh_48m),
	CLK_LK(ref_cx40),

	CLK_LK(sdio1),
	CLK_LK(sdio2),
	CLK_LK(sdio3),
	CLK_LK(sdio4),

	CLK_LK(bsc1),
	CLK_LK(bsc2),

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

	CLK_LK(kpm_ccu),
	CLK_LK(kps_ccu),

	CLK_LK(usb_otg),

	CLK_LK(hsm_ahb),
	CLK_LK(hsm_apb),
	CLK_LK(spum_open_apb),
	CLK_LK(spum_sec_apb),

	CLK_LK(uartb),
	CLK_LK(uartb2),
	CLK_LK(uartb3),
	CLK_LK(uartb_apb),
	CLK_LK(uartb2_apb),
	CLK_LK(uartb3_apb),

	CLK_LK(spum_open_apb),
	CLK_LK(spum_sec_apb),
	CLK_LK(apb1),
	CLK_LK(timers_apb),
	CLK_LK(ssp0_apb),
	CLK_LK(dmac_mux_apb),

	CLK_LK(pwm_apb),
	CLK_LK(pwm),
	CLK_LK(apb2),
	CLK_LK(ssp0),

	CLK_LK(timers),
	CLK_LK(spum_open),
	CLK_LK(spum_sec),
};

int __init clock_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(island_clk_tbl); i++)
		clkdev_add (&island_clk_tbl[i]);

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
