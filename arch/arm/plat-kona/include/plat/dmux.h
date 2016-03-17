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

#ifndef __MACH_DMUX_H
#define __MACH_DMUX_H

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

/*
 * DMAC MUX Logic settings for Rhea/Hera/Samoa
 */
enum dmac_mux {
	KONA_DMUX_UARTB_A = 8,
	KONA_DMUX_UARTB_B = 9,
	KONA_DMUX_UARTB2_A = 10,
	KONA_DMUX_UARTB2_B = 11,
	KONA_DMUX_UARTB3_A = 12,
	KONA_DMUX_UARTB3_B = 13,
	KONA_DMUX_SSP_0A_RX0 = 16,
	KONA_DMUX_SSP_0B_TX0 = 17,
	KONA_DMUX_SSP_0C_RX1 = 18,
	KONA_DMUX_SSP_0D_TX1 = 19,
	KONA_DMUX_SSP_1A_RX0 = 20,
	KONA_DMUX_SSP_1B_TX0 = 21,
	KONA_DMUX_SSP_1C_RX1 = 22,
	KONA_DMUX_SSP_1D_TX1 = 23,
	KONA_DMUX_HSIA = 32,
	KONA_DMUX_HSIB = 33,
	KONA_DMUX_HSIC = 34,
	KONA_DMUX_HSID = 35,
	KONA_DMUX_EANC = 40,
	KONA_DMUX_STEREO = 41,
	KONA_DMUX_NVIN = 42,
	KONA_DMUX_VIN = 43,
	KONA_DMUX_VIBRA = 44,
	KONA_DMUX_IHF_0 = 45,
	KONA_DMUX_VOUT = 46,
	KONA_DMUX_SLIMA = 47,
	KONA_DMUX_SLIMB = 48,
	KONA_DMUX_SLIMC = 49,
	KONA_DMUX_SLIMD = 50,
	KONA_DMUX_SIM_A = 51,
	KONA_DMUX_SIM_B = 52,
	KONA_DMUX_SIM2_A = 53,
	KONA_DMUX_SIM2_B = 54,
	KONA_DMUX_IHF_1 = 55,
	KONA_DMUX_SSP_2A_RX0 = 56,
	KONA_DMUX_SSP_2B_TX0 = 57,
	KONA_DMUX_SSP_2C_RX1 = 58,
	KONA_DMUX_SSP_2D_TX1 = 59,
	KONA_DMUX_SPUM_SecureA = 65,
	KONA_DMUX_SPUM_SecureB = 66,
	KONA_DMUX_SPUM_OpenA = 67,
	KONA_DMUX_SPUM_OpenB = 68,
	KONA_DMUX_INVALID = 0x7f,
};

enum dma_peri dmux_name_to_id(const char *name);
int dmux_id_to_name(enum dma_peri peri, char *pname);
int dmux_sema_protect(void);
int dmux_sema_unprotect(void);
int dmux_alloc_channel(u32 * pchan);
int dmux_release_channel(u32 channel);
int dmux_alloc_peripheral(u32 channel, enum dma_peri peri, u8 * peri_req_id);
int dmux_alloc_multi_peripheral(u32 channel, enum dma_peri a, enum dma_peri b,
				u8 * src_id, u8 * dst_id);
int dmux_dealloc_peripheral(u32 channel);
int dmux_init(void);
void dmux_exit(void);

#endif /* __MACH_DMUX_H */
