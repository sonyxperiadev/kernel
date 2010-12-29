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
#define	CLOCK_1K	1000
#define	CLOCK_1M	(CLOCK_1K * 1000)

#define	DECLARE_PROC_CLK(clk_name, clk_rate)	\
	struct proc_clock clk_name##_clk = {		\
		.clk	=	{							\
			.name	=	"##clk_name##_clk",		\
			.rate	=	clk_rate,				\
			.div	=	1,						\
			.id		=	-1,						\
			.ops	=	&proc_clk_ops,			\
		},										\
	}

#define	DECLARE_REF_CLK(clk_name, clk_rate, clk_div, clk_parent)		\
	struct proc_clock clk_name##_clk = {				\
		.clk	=	{							\
			.name	=	"##clk_name##_clk",			\
			.parent	=	clk_parent,				\
			.rate	=	clk_rate,				\
			.div	=	clk_div,				\
			.id		=	-1,						\
			.ops	=	&ref_clk_ops,			\
		},										\
	}

/* Proc clocks */
DECLARE_PROC_CLK	(arm, 				700*CLOCK_1M);

/* Ref clocks */
DECLARE_REF_CLK		(crystal, 			26*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(frac_1m, 			1*CLOCK_1M,		1,	0);
DECLARE_REF_CLK		(ref_96m_varVDD, 	96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_96m, 			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_96m, 			96*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(var_500M_varVDD, 	500*CLOCK_1M,	1,	0);


DECLARE_REF_CLK		(ref_32k, 			32*CLOCK_1K,	1,	0);
DECLARE_REF_CLK		(misc_32k, 			32*CLOCK_1K,	1,	0);

DECLARE_REF_CLK		(ref_312M, 			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK		(ref_208M, 			208*CLOCK_1M,	0,	to_clk(&ref_312M_clk));
DECLARE_REF_CLK		(ref_104M, 			104*CLOCK_1M,	3,	to_clk(&ref_312M_clk));
DECLARE_REF_CLK		(ref_52M, 			52*CLOCK_1M,	2,	to_clk(&ref_104M_clk));
DECLARE_REF_CLK		(ref_13M, 			13*CLOCK_1M,	4,	to_clk(&ref_52M_clk));

DECLARE_REF_CLK		(var_312M, 			312*CLOCK_1M,	0,	0);
DECLARE_REF_CLK		(var_208M, 			208*CLOCK_1M,	0,	to_clk(&var_312M_clk));
DECLARE_REF_CLK		(var_156M, 			156*CLOCK_1M,	2,	to_clk(&var_312M_clk));
DECLARE_REF_CLK		(var_104M, 			104*CLOCK_1M,	3,	to_clk(&var_312M_clk));
DECLARE_REF_CLK		(var_52M, 			52*CLOCK_1M,	2,	to_clk(&var_104M_clk));
DECLARE_REF_CLK		(var_13M, 			13*CLOCK_1M,	4,	to_clk(&var_52M_clk));

DECLARE_REF_CLK		(usbh_48M, 			48*CLOCK_1M,	1,	0);
DECLARE_REF_CLK		(ref_cx40, 			153600*CLOCK_1K,1,	0);	// FIXME
