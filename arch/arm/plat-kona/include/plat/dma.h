/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/dma.h
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
 * This file provides the interface for all the DMA clients
 */
#ifndef ASM_ARCH_DMA_H
#define ASM_ARCH_DMA_H

/*
 * DMA Configuration
 */

#define REG_DMA_CONFIG_BIG_ENDIAN           0x0002
#define REG_DMA_CONFIG_LITTLE_ENDIAN        0x0000
#define REG_DMA_CONFIG_ENABLED              0x0001

/*
 * DMA per channel control
 */

/* Enable Terminal Count interrupt  */
#define REG_DMA_CHAN_CTL_TC_INT_ENABLE          0x80000000
#define REG_DMA_CHAN_CTL_PROT_MASK              0x70000000
#define REG_DMA_CHAN_CTL_PROT_CACHEABLE         0x40000000
#define REG_DMA_CHAN_CTL_PROT_BUFFERABLE        0x20000000
#define REG_DMA_CHAN_CTL_PROT_PRIVILEDGED       0x10000000
#define REG_DMA_CHAN_CTL_DEST_INCR              0x08000000
#define REG_DMA_CHAN_CTL_SRC_INCR               0x04000000
#define REG_DMA_CHAN_CTL_DEST_WIDTH_MASK        0x00E00000	/* Width of dest */
#define REG_DMA_CHAN_CTL_DEST_WIDTH_32          0x00400000	/* 32 bit xfer width */
#define REG_DMA_CHAN_CTL_DEST_WIDTH_16          0x00200000	/* 16 bit xfer width */
#define REG_DMA_CHAN_CTL_DEST_WIDTH_8           0x00000000	/* 8 bit xfer width  */
#define REG_DMA_CHAN_CTL_SRC_WIDTH_MASK         0x001C0000
#define REG_DMA_CHAN_CTL_SRC_WIDTH_32           0x00080000	/* 32 bit xfer width */
#define REG_DMA_CHAN_CTL_SRC_WIDTH_16           0x00040000	/* 16 bit xfer width */
#define REG_DMA_CHAN_CTL_SRC_WIDTH_8            0x00000000	/* 8 bit xfer width  */
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_MASK   0x00038000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_256    0x00038000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_128    0x00030000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_64     0x00028000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_32     0x00020000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_16     0x00018000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_8      0x00010000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_4      0x00008000
#define REG_DMA_CHAN_CTL_DEST_BURST_SIZE_1      0x00000000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_MASK    0x00007000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_256     0x00007000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_128     0x00006000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_64      0x00005000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_32      0x00004000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_16      0x00003000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_8       0x00002000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_4       0x00001000
#define REG_DMA_CHAN_CTL_SRC_BURST_SIZE_1       0x00000000
#define REG_DMA_CHAN_CTL_TRANSFER_SIZE_MASK     0x00000FFF

/*
 * DMA per channel configuration
 */

#define REG_DMA_CHAN_CFG_HALT                       0x00040000
#define REG_DMA_CHAN_CFG_ACTIVE                     0x00020000
#define REG_DMA_CHAN_CFG_LOCK                       0x00010000
#define REG_DMA_CHAN_CFG_TC_INT_ENABLE              0x00008000
#define REG_DMA_CHAN_CFG_ERROR_INT_ENABLE           0x00004000
#define REG_DMA_CHAN_CFG_FLOW_CTL_MASK              0x00003800
/* Peripheral to peripheral, source peripheral controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_PRF_SRC    0x00003800
/* Peripheral to memory, source peripheral controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_MEM_SRC    0x00003000
/* Memory to peripheral, dest peripheral controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_MEM_TO_PRF_DST    0x00002800
/* Peripheral to peripheral, dest peripheral controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_PRF_DST    0x00002000
/* Peripheral to peripheral, DMA controller controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_PRF_DMA    0x00001800
/* Peripheral to memory, DMA controller controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_PRF_TO_MEM_DMA    0x00001000
/* Memory to peripheral, DMA controller controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_MEM_TO_PRF_DMA    0x00000800
/* Memory to memory, DMA controller controls flow */
#define REG_DMA_CHAN_CFG_FLOW_CTL_MEM_TO_MEM_DMA    0x00000000

#define REG_DMA_CHAN_CFG_DEST_PERIPHERAL_MASK       0x000007C0
#define REG_DMA_CHAN_CFG_DEST_PERIPHERAL_SHIFT      6
#define REG_DMA_CHAN_CFG_SRC_PERIPHERAL_MASK        0x0000003E
#define REG_DMA_CHAN_CFG_SRC_PERIPHERAL_SHIFT       1
#define REG_DMA_CHAN_CFG_ENABLE                     0x00000001

/* Port numbers */
#if defined(CONFIG_ARCH_BCM116X)

#define DMA_MAX_CHANNELS        		8
#define REG_DMA_PERIPHERAL_CRYPTO_OUT		0
#define REG_DMA_PERIPHERAL_CAMERA		1
#define REG_DMA_PERIPHERAL_I2S_TX		2
#define REG_DMA_PERIPHERAL_I2S_RX		3
#define REG_DMA_PERIPHERAL_SIM_RX		4
#define REG_DMA_PERIPHERAL_SIM_TX		4
#define REG_DMA_PERIPHERAL_CRC			5
#define REG_DMA_PERIPHERAL_SPI_RX		6
#define REG_DMA_PERIPHERAL_SPI_TX		7
#define REG_DMA_PERIPHERAL_UARTA_RX		8
#define REG_DMA_PERIPHERAL_UARTA_TX		9
#define REG_DMA_PERIPHERAL_UARTB_RX		10
#define REG_DMA_PERIPHERAL_UARTB_TX		11
#define REG_DMA_PERIPHERAL_RESERVED1		12
#define REG_DMA_PERIPHERAL_RESERVED2		13
#define REG_DMA_PERIPHERAL_USB_1		14
#define REG_DMA_PERIPHERAL_USB_2		15
#define REG_DMA_PERIPHERAL_HSDPA_SCATTER	16
#define REG_DMA_PERIPHERAL_HSDPA_GATHER		17
#define REG_DMA_PERIPHERAL_CRYPTO_IN		18
#define REG_DMA_PERIPHERAL_LCD			19
#define REG_DMA_PERIPHERAL_MSPRO		20
#define REG_DMA_PERIPHERAL_RESERVED3		21
#define REG_DMA_PERIPHERAL_RESERVED4		22
#define REG_DMA_PERIPHERAL_RESERVED5		23

#elif defined(CONFIG_ARCH_BCM215XX)

#define DMA_MAX_CHANNELS        		12
#define REG_DMA_PERIPHERAL_CRYPTO_OUT		0
#define REG_DMA_PERIPHERAL_SIM1_RX		1
#define REG_DMA_PERIPHERAL_I2S_TX		2
#define REG_DMA_PERIPHERAL_I2S_RX		3
#define REG_DMA_PERIPHERAL_SIM1_TX		4
#define REG_DMA_PERIPHERAL_CRC			5
#define REG_DMA_PERIPHERAL_SPI_RX		6
#define REG_DMA_PERIPHERAL_SPI_TX		7
#define REG_DMA_PERIPHERAL_UARTA_RX		8
#define REG_DMA_PERIPHERAL_UARTA_TX		9
#define REG_DMA_PERIPHERAL_UARTB_RX		10
#define REG_DMA_PERIPHERAL_UARTB_TX		11
#define REG_DMA_PERIPHERAL_DES_IN		12
#define REG_DMA_PERIPHERAL_DES_OUT		13
#define REG_DMA_PERIPHERAL_SPI2_TX		14
#define REG_DMA_PERIPHERAL_SPI2_RX		15
#define REG_DMA_PERIPHERAL_UARTC_RX		16
#define REG_DMA_PERIPHERAL_UARTC_TX		17
#define REG_DMA_PERIPHERAL_CRYPTO_IN		18
#define REG_DMA_PERIPHERAL_LCD			19
#define REG_DMA_PERIPHERAL_MSPRO		20
#define REG_DMA_PERIPHERAL_MIPI_DSI1		21
#define REG_DMA_PERIPHERAL_MIPI_DSI2		22
#define REG_DMA_PERIPHERAL_TVENC1		23
#define REG_DMA_PERIPHERAL_TVENC2		24
#define DMA_PERIPHERAL_AUDIO_IN			25
#define DMA_PERIPHERAL_AUDIO_OUT		26
#define DMA_PERIPHERAL_AUDIO_POLY		27
#define DMA_PERIPHERAL_AUDIO_BT			28
#define DMA_PERIPHERAL_MPHI			29
#define DMA_PERIPHERAL_SIM2_RX			30
#define DMA_PERIPHERAL_SIM2_TX			31

#endif

int dma_request_chan(unsigned int chan, const char *name);
int dma_request_avail_chan(unsigned int *chanp, const char *name);
void dma_free_chan(unsigned int chan);
int dma_request_irq(int chan, irqreturn_t(*handler) (void *), void *dev_id);
int dma_free_irq(int chan);
int dma_enable_irq(int chan);
int dma_disable_irq(int chan);

void dma_init_chan(int chan);
void dma_setup_chan(int chan, int srcaddr, int dstaddr, int link, int ctrl,
		    int cfg);
void dma_poll_chan(int chan);

u32 dma_get_src_addr(int chan);
u32 dma_get_dest_addr(int chan);
u32 dma_get_link_addr(int chan);

void dma_enable_chan(int chan);
void dma_disable_chan(int chan);

#endif /* ASM_ARCH_DMA_H */
