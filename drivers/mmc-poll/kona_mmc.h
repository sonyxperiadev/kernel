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

#ifndef __POLL_KONA_MMC
#define __POLL_KONA_MMC

struct kona_mmc {
	unsigned int sysad;	// SYSADDR 
	unsigned int blkcnt_sz;	// BLOCK
	unsigned int argument;	// ARG
	unsigned int cmdreg;	// CMD

	unsigned int rspreg0;	// RESP0
	unsigned int rspreg1;	// RESP2
	unsigned int rspreg2;	// RESP4
	unsigned int rspreg3;	// RESP6
	unsigned int bdata;	// BUFDAT
	unsigned int prnsts;	// PSTATE
	unsigned int ctrl_host_pwr_blk_wak;	// CTRL 
	unsigned int ctrl1_clkcon_timeout_swrst;	// CTRL1 
	unsigned int norintsts;	// INTR
	unsigned int norintstsen;	// INTREN1 
	unsigned int norintsigen;	// INTREN2

	unsigned short acmd12errsts;	// ERRSTAT
	unsigned char res1[2];	// ERRSTAT

	unsigned int capareg;	// CAPABILITIES1

	unsigned char res2[4];	// CAPABILITIES2

	unsigned int maxcurr;	// MAX A1 0x0000_0048 -- 

	unsigned int max_a2;	// MAX A2

	unsigned int cmdentstat;	// CMDENTSTAT 
	unsigned int admaerr;	// 0x0000_0054
	unsigned int admaddr0;	// 0x0000_0058
	unsigned int empty1;	// 0x0000_005C
	unsigned int presetval1;	// 0x0000_0060
	unsigned int presetval2;	// 0x0000_0064
	unsigned int presetval3;	// 0x0000_0068
	unsigned int presetval4;	// 0x0000_006C
	unsigned int boottimeout;	// 0x0000_0070
	unsigned int dbgsel;	// 0x0000_0074
};

struct kona_mmc_p2 {
	unsigned int sbus;	// 0x0000_00E0
	unsigned char empty[0x10];	// 
	unsigned int spi_int;	// 0x0000_00F0
	unsigned char empty1[0x0C];	// 
	unsigned int hcversirq;	// 0x0000_00FC
};

struct kona_mmc_p3 {
	unsigned int corectrl;	// 0x0000_8000
	unsigned char corestat;	// 0x0000_8004
	unsigned int coreimr;	// 0x0000_8008
	unsigned int coreisr;	// 0x0000_800C
	unsigned int coreimsr;	// 0x0000_8010
	unsigned int coredbg1;	// 0x0000_8014
	unsigned int coregpo_mask;	// 0x0000_8018
};

struct mmc_host {
	struct kona_mmc *reg;
	struct kona_mmc_p2 *reg_p2;
	struct kona_mmc_p3 *reg_p3;
	unsigned int base_clock_freq;
	unsigned int version;	// SDHCI spec. version 
	unsigned int clock;	// Current clock (MHz)
};

struct kona_mmc_dev {
	int index;
	char name[64];
	unsigned int mmc_reg_base;
	unsigned int source_clk_reg;
};

int kona_mmc_init(int dev_index);

#endif /* __POLL_KONA_MMC */
