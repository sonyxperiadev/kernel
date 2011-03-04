/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
#include <mach/pinmux.h>
#include <mach/rdb/brcm_rdb_sysmap_a9.h>
#include <mach/rdb/brcm_rdb_padctrlreg.h>

#define	PIN_DESC(ball, alt1, alt2, alt3, alt4, alt5, alt6)	 	\
	[PN_##ball] = {							\
		.name		=	PN_##ball,			\
		.reg_offset	=	PADCTRLREG_##ball##_OFFSET,	\
		.f_tbl		=	{				\
			PF_##alt1, PF_##alt2, PF_##alt3, 		\
			PF_##alt4, PF_##alt5, PF_##alt6, 		\
		},							\
	}

/* Rhea chip-level pin description table */
static struct pin_desc pin_desc_tbl[PN_MAX] = {
	/* MMC0 */
	PIN_DESC(MMC0CK, MMC0CK, SSP2CK, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0CMD, MMC0CMD, SSP2DO, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0RST, MMC0RST, SSP2CK, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT7, MMC0DAT7, SYSCLKREQA, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT6, MMC0DAT6, SYSCLKREQB, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT5, MMC0DAT5, DCLK2, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT4, MMC0DAT4, DCLK3, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT3, MMC0DAT3, SSP2SYN, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT2, MMC0DAT2, RESERVED, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT1, MMC0DAT1, RESERVED, RESERVED, RESERVED, GPIO, RESERVED),
	PIN_DESC(MMC0DAT0, MMC0DAT0, SSP2DI, RESERVED, RESERVED, GPIO, RESERVED),

	/* SD */
	PIN_DESC(SDCK, SDCK, PTI_CLK, RESERVED, SSP1CK, GPIO, U1TXD),
	PIN_DESC(SDCMD, SDCMD, RXD, RESERVED, SSP1DO, GPIO, U1RXD),
	PIN_DESC(SDDAT3, SDDAT3, PTI_DAT3, RESERVED, SWCLKTCK, GPIO, PM_DEBUG3),
	PIN_DESC(SDDAT2, SDDAT2, PTI_DAT2, RESERVED, SWDIOTMS, GPIO, PM_DEBUG2),
	PIN_DESC(SDDAT1, SDDAT1, PTI_DAT1, RESERVED, SSP1SYN, GPIO, PM_DEBUG1),
	PIN_DESC(SDDAT0, SDDAT0, PTI_DAT0, RESERVED, SSP1DI, GPIO, PM_DEBUG0),
};

struct chip_pin_desc g_chip_pin_desc = {
	.desc_tbl	=	pin_desc_tbl,
	.base_addr	=	PAD_CTRL_BASE_ADDR,
	.mapping_size	=	0x800,
};