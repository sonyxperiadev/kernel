/*****************************************************************************
*  Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
#ifndef _I2C_KONA_H_
#define	_I2C_KONA_H_

enum bsc_bus_speed
{
   BSC_BUS_SPEED_32K,
   BSC_BUS_SPEED_50K,
   BSC_BUS_SPEED_230K,
   BSC_BUS_SPEED_380K,
   BSC_BUS_SPEED_400K,
   BSC_BUS_SPEED_430K,
   BSC_BUS_SPEED_HS, /* high-speed */

   /* for FPGA where only the 26 MHz core clock is available */
   BSC_BUS_SPEED_100K_FPGA,
   BSC_BUS_SPEED_400K_FPGA,
   BSC_BUS_SPEED_HS_FPGA,

   /* used as internal array index so do not reorder this */
   BSC_BUS_SPEED_MAX,
};


/*
 * Board dependent configuration for the Broadcom Kona BSC (I2C) adapter
 */
struct bsc_adap_cfg
{
	/* to explicitly disable the I2C adapter */
	int disable;
	/* I2C bus speed in KHz */
	enum bsc_bus_speed speed;
	char *bsc_clk;
	char *bsc_apb_clk;
};
#endif // _I2C_KONA_H_
