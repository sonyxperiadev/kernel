/*
 * Copyright (c) 2010 - 2012 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __POLL_KONA_MMC
#define __POLL_KONA_MMC

struct kona_mmc {
	unsigned int sysad;	/* SYSADDR */
	unsigned int blkcnt_sz;	/* BLOCK */
	unsigned int argument;	/* ARG */
	unsigned int cmdreg;	/* CMD */

	unsigned int rspreg0;	/* RESP0 */
	unsigned int rspreg1;	/* RESP2 */
	unsigned int rspreg2;	/* RESP4 */
	unsigned int rspreg3;	/* RESP6 */
	unsigned int bdata;	/* BUFDAT */
	unsigned int prnsts;	/* PSTATE */
	unsigned int ctrl_host_pwr_blk_wak;	/* CTRL */
	unsigned int ctrl1_clkcon_timeout_swrst;	/* CTRL1 */
	unsigned int norintsts;		/* INTR */
	unsigned int norintstsen;	/* INTREN1 */
	unsigned int norintsigen;	/* INTREN2 */

	unsigned short acmd12errsts;	/* ERRSTAT */
	unsigned char res1[2];	/* ERRSTAT */

	unsigned int capareg;	/* CAPABILITIES1 */

	unsigned char res2[4];	/* CAPABILITIES2 */

	unsigned int maxcurr;	/* MAX A1 0x0000_0048 --  */

	unsigned int max_a2;	/* MAX A2 */

	unsigned int cmdentstat;	/* CMDENTSTAT  */
	unsigned int admaerr;	/* 0x0000_0054 */
	unsigned int admaddr0;	/* 0x0000_0058 */
	unsigned int empty1;	/* 0x0000_005C */
	unsigned int presetval1;	/* 0x0000_0060 */
	unsigned int presetval2;	/* 0x0000_0064 */
	unsigned int presetval3;	/* 0x0000_0068 */
	unsigned int presetval4;	/* 0x0000_006C */
	unsigned int boottimeout;	/* 0x0000_0070 */
	unsigned int dbgsel;	/* 0x0000_0074 */
};

struct kona_mmc_p2 {
	unsigned int sbus;	/* 0x0000_00E0 */
	unsigned char empty[0x10];
	unsigned int spi_int;	/* 0x0000_00F0 */
	unsigned char empty1[0x0C];
	unsigned int hcversirq;	/* 0x0000_00FC */
};

struct kona_mmc_p3 {
	unsigned int corectrl;	/* 0x0000_8000 */
	unsigned char corestat;	/* 0x0000_8004 */
	unsigned int coreimr;	/* 0x0000_8008 */
	unsigned int coreisr;	/* 0x0000_800C */
	unsigned int coreimsr;	/* 0x0000_8010 */
	unsigned int coredbg1;	/* 0x0000_8014 */
	unsigned int coregpo_mask;	/* 0x0000_8018 */
};

struct mmc_host {
	struct kona_mmc *reg;
	struct kona_mmc_p2 *reg_p2;
	struct kona_mmc_p3 *reg_p3;
	unsigned int base_clock_freq;
	unsigned int version;	/* SDHCI spec. version */
	unsigned int clock;	/* Current clock (MHz) */
};

struct kona_mmc_dev {
	int index;
	char name[64];
	unsigned int mmc_reg_base;
	unsigned int source_clk_reg;
};

int kona_mmc_init(int dev_index);

#endif /* __POLL_KONA_MMC */
