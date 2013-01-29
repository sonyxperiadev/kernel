/************************************************************************/
/*                                                                      */
/*  Copyright 2012  Broadcom Corporation                                */
/*                                                                      */
/* Unless you and Broadcom execute a separate written software license  */
/* agreement governing use of this software, this software is licensed  */
/* to you under the terms of the GNU General Public License version 2   */
/* (the GPL), available at						*/
/*                                                                      */
/*          http://www.broadcom.com/licenses/GPLv2.php                  */
/*                                                                      */
/*  with the following added to such license:                           */
/*                                                                      */
/*  As a special exception, the copyright holders of this software give */
/*  you permission to link this software with independent modules, and  */
/*  to copy and distribute the resulting executable under terms of your */
/*  choice, provided that you also meet, for each linked independent    */
/*  module, the terms and conditions of the license of that module. An  */
/*  independent module is a module which is not derived from this       */
/*  software. The special  exception does not apply to any              */
/*  modifications of the software.					*/
/*									*/
/*  Notwithstanding the above, under no circumstances may you combine	*/
/*  this software in any way with any other Broadcom software provided	*/
/*  under a license other than the GPL, without Broadcom's express	*/
/*  prior written consent.						*/
/*									*/
/************************************************************************/

#ifndef __DSI_TIMING_H__
#define __DSI_TIMING_H__

/*--- Counter Mode Flags */
/* record has MAX value set */
#define	DSI_C_HAS_MAX	     1
/* record MIN value is MAX of 2 values */
#define	DSI_C_MIN_MAX_OF_2   2


/*--- Counter timeBase Flags */
/* ESC2LPDT entry - must be first record */
#define	DSI_C_TIME_ESC2LPDT  0
/* counts in HS Bit Clk */
#define	DSI_C_TIME_HS	     1
/* counts in ESC CLKs */
#define	DSI_C_TIME_ESC	     2

/* D-PHY Timing Record */
struct DSI_COUNTER {
	char *name;
	UInt32 timeBase;
	UInt32 mode;
	/* esc_clk LP counters are speced using this member */
	UInt32 time_lpx;
	UInt32 time_min1_ns;
	UInt32 time_min1_ui;
	UInt32 time_min2_ns;
	UInt32 time_min2_ui;
	UInt32 time_max_ns;
	UInt32 time_max_ui;
	UInt32 counter_min;
	UInt32 counter_max;
	UInt32 counter_step;
	UInt32 counter_offs;
	/* calculated value of the register */
	UInt32 counter;
	/* dbg */
	UInt32 period;
};

#endif
