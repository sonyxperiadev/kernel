/******************************************************************************/
/*                                                                            */
/*  Copyright 2010  Broadcom Corporation                                      */
/*                                                                            */
/*    Unless you and Broadcom execute a separate written software license     */
/*    agreement governing use of this software, this software is licensed     */
/*    to you under the terms of the GNU					      */
/*    General Public License version 2 (the GPL), available at                */
/*                                                                            */
/*         http://www.broadcom.com/licenses/GPLv2.php                         */
/*                                                                            */
/*    with the following added to such license:                               */
/*                                                                            */
/*    As a special exception, the copyright holders of this software give     */
/*    you permission to  link this software with independent modules,  and to */
/*    copy and distribute the resulting executable under terms of your choice,*/
/*    provided that you also meet, for each linked independent module, the    */
/*    terms and conditions of the license of that module. An independent      */
/*    module is a module which is not derived form this software. The special */
/*    exception does not apply to andy modifications of the software	      */
/*                                                                            */
/*    Notwithstanding the above, under no circumstances may you combine this  */
/*    software in any way with any other Broadcom software provided under a   */
/*    license other than the GPL, without Broadcom's express prior written    */
/*    consent.								      */
/******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <mach/rdb/brcm_rdb_dmux.h>
#include <mach/io_map.h>
#include <mach/dma.h>
#include <mach/clock.h>
#include <plat/dmux.h>
#include <mach/memory.h>

#define DMUX_CHAN_INVALID	0xF
#define DMUX_PERI_INVALID	0x7F

/* DMUX device */
struct kona_dmux_dev {
	void __iomem *base_addr;
	struct clk *dmux_clk;
};

static struct kona_dmux_dev dmux;

/* DMAC MUX map for the Kona architecture */
struct dmac_mux_map {
	char *name;		/* Name of peripheral channel */
	enum dma_peri id;	/* peripheral ID, virtual DMA channel ID */
	enum dmac_mux mux;	/* MUX setting to alloc the peripheral */
};

/******************************************************************************
 * This array gathers info needed by DMUX logic to map peripherals.
 *
 *  name:	Peripheral name, every machine defines it's own set of channels.
 *  id:		virtual channels for Kona architecture, defined in plat/dmux.h
 *  mux:	MUX values for Kona architecture, defined in plat/dmux.h
 *
 * This array is defined for RHEA architecture, a superset.
 * For other Kona implementations like Samoa, it is assumed that
 * mach specific dma.h file will define the channel names, and
 * define 'kona_dmux_map' for that specific architecture.
 * This approach makes DMA drive API independent of arch specific details.
 *
******************************************************************************/
static const struct dmac_mux_map kona_dmux_map[] = {
	{.name = DMA_CHAN_UARTB_A, .id = KONA_DMACH_UARTB_A, .mux =
	 KONA_DMUX_UARTB_A},
	{.name = DMA_CHAN_UARTB_B, .id = KONA_DMACH_UARTB_B, .mux =
	 KONA_DMUX_UARTB_B},
	{.name = DMA_CHAN_UARTB2_A, .id = KONA_DMACH_UARTB2_A, .mux =
	 KONA_DMUX_UARTB2_A},
	{.name = DMA_CHAN_UARTB2_B, .id = KONA_DMACH_UARTB2_B, .mux =
	 KONA_DMUX_UARTB2_B},
	{.name = DMA_CHAN_UARTB3_A, .id = KONA_DMACH_UARTB3_A, .mux =
	 KONA_DMUX_UARTB3_A},
	{.name = DMA_CHAN_UARTB3_B, .id = KONA_DMACH_UARTB3_B, .mux =
	 KONA_DMUX_UARTB3_B},
	{.name = DMA_CHAN_SSP_0A_RX0, .id = KONA_DMACH_SSP_0A_RX0, .mux =
	 KONA_DMUX_SSP_0A_RX0},
	{.name = DMA_CHAN_SSP_0B_TX0, .id = KONA_DMACH_SSP_0B_TX0, .mux =
	 KONA_DMUX_SSP_0B_TX0},
	{.name = DMA_CHAN_SSP_0C_RX1, .id = KONA_DMACH_SSP_0C_RX1, .mux =
	 KONA_DMUX_SSP_0C_RX1},
	{.name = DMA_CHAN_SSP_0D_TX1, .id = KONA_DMACH_SSP_0D_TX1, .mux =
	 KONA_DMUX_SSP_0D_TX1},
	{.name = DMA_CHAN_SSP_1A_RX0, .id = KONA_DMACH_SSP_1A_RX0, .mux =
	 KONA_DMUX_SSP_1A_RX0},
	{.name = DMA_CHAN_SSP_1B_TX0, .id = KONA_DMACH_SSP_1B_TX0, .mux =
	 KONA_DMUX_SSP_1B_TX0},
	{.name = DMA_CHAN_SSP_1C_RX1, .id = KONA_DMACH_SSP_1C_RX1, .mux =
	 KONA_DMUX_SSP_1C_RX1},
	{.name = DMA_CHAN_SSP_1D_TX1, .id = KONA_DMACH_SSP_1D_TX1, .mux =
	 KONA_DMUX_SSP_1D_TX1},
	{.name = DMA_CHAN_HSIA, .id = KONA_DMACH_HSIA, .mux = KONA_DMUX_HSIA},
	{.name = DMA_CHAN_HSIB, .id = KONA_DMACH_HSIB, .mux = KONA_DMUX_HSIB},
	{.name = DMA_CHAN_HSIC, .id = KONA_DMACH_HSIC, .mux = KONA_DMUX_HSIC},
	{.name = DMA_CHAN_HSID, .id = KONA_DMACH_HSID, .mux = KONA_DMUX_HSID},
	{.name = DMA_CHAN_EANC, .id = KONA_DMACH_EANC, .mux = KONA_DMUX_EANC},
	{.name = DMA_CHAN_STEREO, .id = KONA_DMACH_STEREO, .mux =
	 KONA_DMUX_STEREO},
	{.name = DMA_CHAN_NVIN, .id = KONA_DMACH_NVIN, .mux = KONA_DMUX_NVIN},
	{.name = DMA_CHAN_VIN, .id = KONA_DMACH_VIN, .mux = KONA_DMUX_VIN},
	{.name = DMA_CHAN_VIBRA, .id = KONA_DMACH_VIBRA, .mux =
	 KONA_DMUX_VIBRA},
	{.name = DMA_CHAN_IHF_0, .id = KONA_DMACH_IHF_0, .mux =
	 KONA_DMUX_IHF_0},
	{.name = DMA_CHAN_VOUT, .id = KONA_DMACH_VOUT, .mux = KONA_DMUX_VOUT},
	{.name = DMA_CHAN_SLIMA, .id = KONA_DMACH_SLIMA, .mux =
	 KONA_DMUX_SLIMA},
	{.name = DMA_CHAN_SLIMB, .id = KONA_DMACH_SLIMB, .mux =
	 KONA_DMUX_SLIMB},
	{.name = DMA_CHAN_SLIMC, .id = KONA_DMACH_SLIMC, .mux =
	 KONA_DMUX_SLIMC},
	{.name = DMA_CHAN_SLIMD, .id = KONA_DMACH_SLIMD, .mux =
	 KONA_DMUX_SLIMD},
	{.name = DMA_CHAN_SIM_A, .id = KONA_DMACH_SIM_A, .mux =
	 KONA_DMUX_SIM_A},
	{.name = DMA_CHAN_SIM_B, .id = KONA_DMACH_SIM_B, .mux =
	 KONA_DMUX_SIM_B},
	{.name = DMA_CHAN_SIM2_A, .id = KONA_DMACH_SIM2_A, .mux =
	 KONA_DMUX_SIM2_A},
	{.name = DMA_CHAN_SIM2_B, .id = KONA_DMACH_SIM2_B, .mux =
	 KONA_DMUX_SIM2_B},
	{.name = DMA_CHAN_IHF_1, .id = KONA_DMACH_IHF_1, .mux =
	 KONA_DMUX_IHF_1},
	{.name = DMA_CHAN_SP_2A_RX0, .id = KONA_DMACH_SSP_2A_RX0, .mux =
	 KONA_DMUX_SSP_2A_RX0},
	{.name = DMA_CHAN_SSP_2B_TX0, .id = KONA_DMACH_SSP_2B_TX0, .mux =
	 KONA_DMUX_SSP_2B_TX0},
	{.name = DMA_CHAN_SSP_2C_RX1, .id = KONA_DMACH_SSP_2C_RX1, .mux =
	 KONA_DMUX_SSP_2C_RX1},
	{.name = DMA_CHAN_SSP_2D_TX1, .id = KONA_DMACH_SSP_2D_TX1, .mux =
	 KONA_DMUX_SSP_2D_TX1},
	{.name = DMA_CHAN_SPUM_SecureA, .id = KONA_DMACH_SPUM_SecureA, .mux =
	 KONA_DMUX_SPUM_SecureA},
	{.name = DMA_CHAN_SPUM_SecureB, .id = KONA_DMACH_SPUM_SecureB, .mux =
	 KONA_DMUX_SPUM_SecureB},
	{.name = DMA_CHAN_SPUM_OpenA, .id = KONA_DMACH_SPUM_OpenA, .mux =
	 KONA_DMUX_SPUM_OpenA},
	{.name = DMA_CHAN_SPUM_OpenB, .id = KONA_DMACH_SPUM_OpenB, .mux =
	 KONA_DMUX_SPUM_OpenB},
	{.name = NULL, .id = KONA_DMA_MAX_CHANNELS, .mux = KONA_DMUX_INVALID}
	/* Termination */
};

enum dma_peri dmux_name_to_id(const char *name)
{
	int i;

	for (i = 0; kona_dmux_map[i].id < KONA_DMA_MAX_CHANNELS; i++) {
		if (strncmp(kona_dmux_map[i].name, name, MAX_CHAN_NAME_LENGTH)
		    == 0) {
			return kona_dmux_map[i].id;
		}
	}
	return KONA_DMACH_INVALID;
}

int dmux_id_to_name(enum dma_peri peri, char *pname)
{
	if (peri < KONA_DMA_MAX_CHANNELS) {
		strlcpy(pname, kona_dmux_map[peri].name, MAX_CHAN_NAME_LENGTH);
		return 0;
	}
	return -1;
}

int dmux_sema_protect(void)
{
	if (readl(dmux.base_addr + DMUX_SEMAPHORE_OFFSET) == 0)
		return 0;
	else
		return -1;
}

int dmux_sema_unprotect(void)
{
	writel(1, dmux.base_addr + DMUX_SEMAPHORE_OFFSET);
	return 0;
}

int dmux_alloc_channel(u32 *pchan)
{
	u32 channel;

	channel = readl(dmux.base_addr + DMUX_CHAN_ALLOC_DEALLOC_OFFSET);
	channel &= DMUX_CHAN_ALLOC_DEALLOC_CHANNEL_AD_MASK;

	if (channel != DMUX_CHAN_INVALID) {
		*pchan = channel;
		return 0;
	}
	return -1;
}

int dmux_release_channel(u32 channel)
{
	writel(channel, dmux.base_addr + DMUX_CHAN_ALLOC_DEALLOC_OFFSET);
	return 0;
}

int dmux_alloc_peripheral(u32 channel, enum dma_peri peri, u8 *peri_req_id)
{
	u32 peri_reg_val, read_reg, time_out = 256;

	if (peri < KONA_DMA_MAX_CHANNELS) {
		peri_reg_val =
		    (kona_dmux_map[peri].mux << DMUX_PER_0_PER_A_0_SHIFT) |
		    (DMUX_PERI_INVALID << DMUX_PER_0_PER_B_0_SHIFT);
	} else {
		return -1;
	}

	writel(peri_reg_val, dmux.base_addr + DMUX_PER_0_OFFSET + channel * 4);

	/*FIXME: wait loop, timeout is taken from RTOS!!! */
	do {
		read_reg =
		    readl(dmux.base_addr + DMUX_PER_0_OFFSET + channel * 4);
		if ((read_reg & ~DMUX_PER_0_RESERVED_MASK) == peri_reg_val) {
			if (peri_req_id)
				*peri_req_id = 2 * channel;

			return 0;
		}
	} while (time_out--);

	return -1;
}

int dmux_alloc_multi_peripheral(u32 channel,
				enum dma_peri a,
				enum dma_peri b, u8 *src_id, u8 *dst_id)
{
	u32 peri_reg_val, read_reg, time_out = 256;

	if ((a < KONA_DMA_MAX_CHANNELS) && (b < KONA_DMA_MAX_CHANNELS)) {
		peri_reg_val =
		    (kona_dmux_map[a].mux << DMUX_PER_0_PER_A_0_SHIFT) |
		    (kona_dmux_map[b].mux << DMUX_PER_0_PER_B_0_SHIFT);

	} else if (a < KONA_DMA_MAX_CHANNELS) {
		peri_reg_val =
		    (kona_dmux_map[a].mux << DMUX_PER_0_PER_A_0_SHIFT) |
		    (DMUX_PERI_INVALID << DMUX_PER_0_PER_B_0_SHIFT);
	} else {
		peri_reg_val =
		    (kona_dmux_map[b].mux << DMUX_PER_0_PER_A_0_SHIFT) |
		    (DMUX_PERI_INVALID << DMUX_PER_0_PER_B_0_SHIFT);
	}

	writel(peri_reg_val, dmux.base_addr + DMUX_PER_0_OFFSET + channel * 4);

	/*FIXME: wait loop, timeout is taken from RTOS!!! */
	do {
		read_reg =
		    readl(dmux.base_addr + DMUX_PER_0_OFFSET + channel * 4);
		if ((read_reg & ~DMUX_PER_0_RESERVED_MASK) == peri_reg_val) {
			if (dst_id)
				*dst_id = 2 * channel;
			if (src_id)
				*src_id = 2 * channel + 1;
			return 0;
		}
	} while (time_out--);

	return -1;
}

int dmux_dealloc_peripheral(u32 channel)
{
	channel &= DMUX_PER_DEALLOC_CHANNEL_P_MASK;
	writel(channel, dmux.base_addr + DMUX_PER_DEALLOC_OFFSET);

	return 0;
}

int dmux_init(void)
{

	pr_info("DMAC MUX Init\n");

	/* Set DMUX base address */
	dmux.base_addr = KONA_DMUX_VA;

	/* get the clocks going */
	dmux.dmux_clk = clk_get(NULL, "dmac_mux_apb_clk");
	if (IS_ERR(dmux.dmux_clk)) {
		pr_err("Failed to get the dmac_mux_apb_clk!!!\n");
		return PTR_ERR(dmux.dmux_clk);
	}
	return clk_enable(dmux.dmux_clk);
}

void dmux_exit(void)
{
	clk_disable(dmux.dmux_clk);
	return;
}

