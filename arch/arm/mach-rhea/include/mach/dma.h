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

#include <linux/init.h>

#ifndef __MACH_DMA_H
#define __MACH_DMA_H

/* PL330 DMA peripheral channels for Rhea/Hera/Samoa */
enum dma_peri {
	KONA_DMACH_INVALID = 0xFF,
	KONA_DMACH_UARTB_A = 0,
	KONA_DMACH_UARTB_B,
	KONA_DMACH_UARTB2_A,
	KONA_DMACH_UARTB2_B,
	KONA_DMACH_UARTB3_A,
	KONA_DMACH_UARTB3_B,
	KONA_DMACH_SSP_0A_RX0,
	KONA_DMACH_SSP_0B_TX0,
	KONA_DMACH_SSP_0C_RX1,
	KONA_DMACH_SSP_0D_TX1,
	KONA_DMACH_SSP_1A_RX0,
	KONA_DMACH_SSP_1B_TX0,
	KONA_DMACH_SSP_1C_RX1,
	KONA_DMACH_SSP_1D_TX1,
	KONA_DMACH_HSIA,
	KONA_DMACH_HSIB,
	KONA_DMACH_HSIC,
	KONA_DMACH_HSID,
	KONA_DMACH_EANC,
	KONA_DMACH_STEREO,
	KONA_DMACH_NVIN,
	KONA_DMACH_VIN,
	KONA_DMACH_VIBRA,
	KONA_DMACH_IHF_0,
	KONA_DMACH_VOUT,
	KONA_DMACH_SLIMA,
	KONA_DMACH_SLIMB,
	KONA_DMACH_SLIMC,
	KONA_DMACH_SLIMD,
	KONA_DMACH_SIM_A,
	KONA_DMACH_SIM_B,
	KONA_DMACH_SIM2_A,
	KONA_DMACH_SIM2_B,
	KONA_DMACH_IHF_1,
	KONA_DMACH_SSP_2A_RX0,
	KONA_DMACH_SSP_2B_TX0,
	KONA_DMACH_SSP_2C_RX1,
	KONA_DMACH_SSP_2D_TX1,
	KONA_DMACH_SPUM_SecureA,
	KONA_DMACH_SPUM_SecureB,
	KONA_DMACH_SPUM_OpenA,
	KONA_DMACH_SPUM_OpenB,
	KONA_DMA_MAX_CHANNELS
};

#include <plat/dma-pl330.h>

#endif /* __MACH_DMA_H */
