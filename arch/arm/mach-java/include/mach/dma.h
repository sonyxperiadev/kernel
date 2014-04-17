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

/* Peripheral channels definition for Rhea Platform */
#define DMA_CHAN_UARTB_A	 "UARTB_A"
#define DMA_CHAN_UARTB_B	 "UARTB_B"
#define DMA_CHAN_UARTB2_A	 "UARTB2_A"
#define DMA_CHAN_UARTB2_B	 "UARTB2_B"
#define DMA_CHAN_UARTB3_A	 "UARTB3_A"
#define DMA_CHAN_UARTB3_B	 "UARTB3_B"
#define DMA_CHAN_SSP_0A_RX0	 "SSP_0A_RX0"
#define DMA_CHAN_SSP_0B_TX0	 "SSP_0B_TX0"
#define DMA_CHAN_SSP_0C_RX1	 "SSP_0C_RX1"
#define DMA_CHAN_SSP_0D_TX1	 "SSP_0D_TX1"
#define DMA_CHAN_SSP_1A_RX0	 "SSP_1A_RX0"
#define DMA_CHAN_SSP_1B_TX0	 "SSP_1B_TX0"
#define DMA_CHAN_SSP_1C_RX1	 "SSP_1C_RX1"
#define DMA_CHAN_SSP_1D_TX1	 "SSP_1D_TX1"
#define DMA_CHAN_HSIA	 "HSIA"
#define DMA_CHAN_HSIB	 "HSIB"
#define DMA_CHAN_HSIC	 "HSIC"
#define DMA_CHAN_HSID	 "HSID"
#define DMA_CHAN_EANC	 "EANC"
#define DMA_CHAN_STEREO	 "STEREO"
#define DMA_CHAN_NVIN	 "NVIN"
#define DMA_CHAN_VIN	 "VIN"
#define DMA_CHAN_VIBRA	 "VIBRA"
#define DMA_CHAN_IHF_0	 "IHF_0"
#define DMA_CHAN_VOUT	 "VOUT"
#define DMA_CHAN_SLIMA	 "SLIMA"
#define DMA_CHAN_SLIMB	 "SLIMB"
#define DMA_CHAN_SLIMC	 "SLIMC"
#define DMA_CHAN_SLIMD	 "SLIMD"
#define DMA_CHAN_SIM_A	 "SIM_A"
#define DMA_CHAN_SIM_B	 "SIM_B"
#define DMA_CHAN_SIM2_A	 "SIM2_A"
#define DMA_CHAN_SIM2_B	 "SIM2_B"
#define DMA_CHAN_IHF_1	 "IHF_1"
#define DMA_CHAN_SP_2A_RX0	 "SP_2A_RX0"
#define DMA_CHAN_SSP_2B_TX0	 "SSP_2B_TX0"
#define DMA_CHAN_SSP_2C_RX1	 "SSP_2C_RX1"
#define DMA_CHAN_SSP_2D_TX1	 "SSP_2D_TX1"
#define DMA_CHAN_SPUM_SecureA	 "SPUM_SecureA"
#define DMA_CHAN_SPUM_SecureB	 "SPUM_SecureB"
#define DMA_CHAN_SPUM_OpenA	 "SPUM_OpenA"
#define DMA_CHAN_SPUM_OpenB	 "SPUM_OpenB"

#include <plat/dma-pl330.h>

#endif /* __MACH_DMA_H */
