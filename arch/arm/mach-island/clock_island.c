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
#include <mach/rdb/brcm_rdb_sysmap_a9.h>

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
DECLARE_REF_CLK		(ref_96m_varVDD, 	96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_96m, 			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_96m, 			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_500m_varVDD, 	500*CLOCK_1M,	1,	0);


DECLARE_REF_CLK		(ref_32k, 			32*CLOCK_1K,	1,	0);
DECLARE_REF_CLK		(misc_32k, 			32*CLOCK_1K,	1,	0);

DECLARE_REF_CLK		(ref_312m, 			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK		(ref_208m, 			208*CLOCK_1M,	0,	name_to_clk(ref_312m));
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

/* peri clocks */
static struct clk *sdio_clk_src_tbl[] =
{
	name_to_clk(crystal),
	name_to_clk(var_52m),
	name_to_clk(ref_52m),
	name_to_clk(var_96m),
	name_to_clk(ref_96m),
};

static struct clk_src sdio_clk_src = {
	.total		=	ARRAY_SIZE(sdio_clk_src_tbl),
	.parents	=	sdio_clk_src_tbl,
};

static struct peri_clock sdio1_clk = {
	.clk	=	{
		.name	=	"sdio1_clk",
		.parent	=	name_to_clk(ref_52m),
		.rate	=	26*CLOCK_1M,
		.div	=	2,
		.id	=	-1,

		.src	= 	&sdio_clk_src,
		.ops	=	&peri_clk_ops,
	},
};

static struct peri_clock sdio2_clk = {
	.clk	=	{
		.name	=	"sdio2_clk",
		.parent	=	name_to_clk(ref_52m),
		.rate	=	26*CLOCK_1M,
		.div	=	2,
		.id	=	-1,

		.src	= 	&sdio_clk_src,
		.ops	=	&peri_clk_ops,
	},
};

static struct peri_clock sdio3_clk = {
	.clk	=	{
		.name	=	"sdio3_clk",
		.parent	=	name_to_clk(ref_52m),
		.rate	=	26*CLOCK_1M,
		.div	=	2,
		.id	=	-1,

		.src	= 	&sdio_clk_src,
		.ops	=	&peri_clk_ops,
	},
};

static struct peri_clock sdio4_clk = {
	.clk	=	{
		.name	=	"sdio4_clk",
		.parent	=	name_to_clk(ref_52m),
		.rate	=	26*CLOCK_1M,
		.div	=	2,
		.id	=	-1,

		.src	= 	&sdio_clk_src,
		.ops	=	&peri_clk_ops,
	},
};

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
