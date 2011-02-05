/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/reg_wdt.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  PURPOSE:
*     This file contains definitions for the watchdog timer.
*
*  NOTES:
*
*****************************************************************************/

#if !defined(__ASM_ARCH_REG_WDT_H)
#define __ASM_ARCH_REG_WDT_H

/* ---- Include Files ---------------------------------------------------- */
#include <mach/hardware.h>

/* ---- Constants and Types ---------------------------------------------- */

/* REG_WDT_CR bits */
#define REG_WDT_CR_EN           0x8000	/* 1:Enable watchdog timer 0:disable */
#define REG_WDT_CR_SRSTEN       0x4000	/* Soft reset enable 0:no effect */
					    /* 1:a reset pulse will be generated (no pmu) */
#define REG_WDT_CR_INTEN        0x2000	/* Interrupt assertion 0:none 1:int enabled on timer expiry */
#define REG_WDT_CR_DSLPCEN      0x1000	/* Deep Sleep Count Enable */
					    /* 0: Timer clock stopped during deep-sleep mode */
					    /* 1: Timer clock keeps running during deep-sleep mode */
#define REG_WDT_CR_CLKS_MASK    0x0300	/* Clock Source */
#define REG_WDT_CR_CLKS_1HZ     0x0000	/* 1 Hz clock, Free running clock derived from 32 kHz clock */
#define REG_WDT_CR_CLKS_32KHZ   0x0100	/* 32 kHz clock, Free running */
#define REG_WDT_CR_LD_MASK      0x00ff	/* Reload value mask */
					    /* Counter Reload Value, in units, of clocks as selected */
					    /* in the CLKS field; this value will be reloaded to the watchdog */
					    /* timer every time this register is written into */

/* This value will trigger a soft reset, used for downloading and checking the pumr register. */
#define REG_WDT_RESET_VAL       (REG_WDT_CR_EN | REG_WDT_CR_SRSTEN | REG_WDT_CR_CLKS_32KHZ | 1)

#endif
