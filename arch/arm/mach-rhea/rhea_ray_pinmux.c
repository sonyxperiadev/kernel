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
#include "linux/kernel.h"
#include "mach/pinmux.h"

#define	PIN_CFG(ball, f, hys, dn, up, rc, ipd, drv) 			\
	{								\
		.name		=	PN_##ball,			\
		.func		=	PF_##f,				\
		.reg.b		=	{				\
			.hys_en		=	hys,			\
			.pull_dn	=	PULL_DN_##dn,		\
			.pull_up	=	PULL_UP_##up,		\
			.slew_rate_ctrl	=	rc,			\
			.input_dis	=	ipd,			\
			.drv_sth	=	DRIVE_STRENGTH_##drv,	\
		},							\
	}

static struct pin_config board_pin_config[] = {
	PIN_CFG(MMC0CK, MMC0CK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC0CMD, MMC0CMD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0RST, MMC0RST, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT7, MMC0DAT7, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT6, MMC0DAT6, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT5, MMC0DAT5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT4, MMC0DAT4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT3, MMC0DAT3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT2, MMC0DAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT1, MMC0DAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT0, MMC0DAT0, 0, OFF, ON, 0, 0, 8MA),
};

/* board level init */
int pinmux_board_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);
	return 0;
}
