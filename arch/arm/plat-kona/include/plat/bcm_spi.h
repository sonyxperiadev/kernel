/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/bcm_spi.h
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
 * Broadcom bcm21xx SPI master controller
 */
#ifndef REG_SPI_H__
#define REG_SPI_H__
#include <mach/hardware.h>
#include <plat/syscfg.h>

#define START_STATE         ((void *)0)
#define RUNNING_STATE       ((void *)1)
#define DONE_STATE          ((void *)2)
#define ERROR_STATE         ((void *)-1)

#define QUEUE_RUNNING         0
#define QUEUE_STOPPED         1

#define SPI_SSPCR0              (0x00)	/* Offset */
#define SPI_SSPCR0_DSS(x)	(x)
#define SPI_SSPCR0_FRF(x)	(x << 3)
#define SPI_SSPCR0_FRF_MOT	(SPI_SSPCR0_FRF(0))
#define SPI_SSPCR0_FRF_TI	(SPI_SSPCR0_FRF(1))
#define SPI_SSPCR0_FRF_NAT	(SPI_SSPCR0_FRF(2))
#define SPI_SSPCR0_SPO(x)	(x << 5)
#define SPI_SSPCR0_SPH(x)	(x << 6)
#define SPI_SSPCR0_SCR(x)	(x << 7)

#define SPI_SSPCR1              (0x04)	/* Offset */
#define SPI_SSPSCR1_SOD         (0x08)	/* Slave output disable                */
#define SPI_SSPSCR1_MS          (0x04)	/* Master/Slave 1:slave 0:master        */
#define IS_MODE_SLAVE(mode)	(mode & 1)	/* mode: 1 = slave, 0 = master */
#define SPI_SSPSCR1_SSE         (0x02)	/* Interafce Enable                */
#define SPI_SSPSCR1_LBM         (0x01)	/* Loopback select: 1:loopback, 0:normal    */

#define SPI_SSPDR               (0x08)	/* Offset */

#define SPI_SSPSR               (0x0c)	/* Offset */
/* SSPSR status register    */
#define SPI_SSPSR_BSY           (0x10)	/* Busy            */
#define SPI_SSPSR_RFF           (0x08)	/* Rx FIFO 1: full     */
#define SPI_SSPSR_RNE           (0x04)	/* Rx FIFO 1: not empty    */
#define SPI_SSPSR_TNF           (0x02)	/* Tx FIFO 1: not full    */
#define SPI_SSPSR_TFE           (0x01)	/* Tx FIFO 1: not empty    */
#define SPI_SSPSR_BSY           (0x10)	/* Busy            */

#define SPI_SSPCPSR             (0x10)	/* Offset */
#define SPI_SSPCPSR_CPSDVSR(x)	(x & 0xff)

#define SPI_SSPIMSC             (0x14)	/* Offset */
#define SPI_ENABLE_ALL_INTERRUPTS	(0xf)
#define SPI_DISABLE_ALL_INTERRUPTS	(~SPI_ENABLE_ALL_INTERRUPTS)
#define SPI_SSPRIS              (0x18)	/* Offset */
#define SPI_SSPRIS_RORIM	(0x1 << 0)
#define SPI_SSPRIS_RTIM		(0x1 << 1)
#define SPI_SSPRIS_RXIM		(0x1 << 2)
#define SPI_SSPRIS_TXIM		(0x1 << 3)
#define SPI_SSPMIS              (0x1c)	/* Offset */
#define SPI_SSPICR              (0x20)	/* Offset */
#define SPI_CLEAR_ALL_INTERRUPTS (0x03)
#define SPI_SSPICR_RORIC	(0x1 << 0)
#define SPI_SSPICR_RTIC		(0x1 << 1)
#define SPI_SSPDMACR            (0x24)	/* Offset */
#define SPI_SSPDMACR_RXEN	(0x01 << 0)
#define SPI_SSPDMACR_TXEN	(0x01 << 1)
#define SPI_DISABLE_ALL_DMA	(0x0)
#define SPI_SPIPADPULL		(0x1034)	/*offset */
#define SPI_SPIFSSCR		(0x103C)	/* Offset */
#define SPI_SPIFSSCR_FSSNEW(x)	(x << 0x0)
#define SPI_SPIFSSCR_FSSSELNEW	(0x1 << 0x7)
#define ENABLE_CS		0
#define DISABLE_CS		1

#define FIFO_DEPTH		8

struct driver_data {
	u8 slot_id;
	struct platform_device *pdev;
	void __iomem *ioaddr;	/* Mapped address */
	struct clk *spi_clk;

	/* SPI framework hookup */
	struct spi_master *master;

	/* DMA setup stuff */
	int rx_channel;
	int tx_channel;
	u32 mode;
	int irq;
	int run;
	int busy;

	/* Driver message queue */
	struct workqueue_struct *workqueue;
	struct work_struct pump_messages;
	spinlock_t lock;
	struct list_head queue;

	/* Message Transfer pump */
	struct tasklet_struct pump_transfers;

	/* Current message transfer state info */
	struct spi_message *cur_msg;
	struct spi_transfer *cur_transfer;
	struct chip_data *cur_chip;
	size_t len;
	void *tx;
	void *tx_end;
	void *rx;
	void *rx_end;
	int dma_mapped;
	dma_addr_t rx_dma;
	dma_addr_t tx_dma;
	size_t rx_map_len;
	size_t tx_map_len;
	int tx_dma_needs_unmap;
	int rx_dma_needs_unmap;
	int cs_change;
	u8 enable_dma;
	u32 dummy_dma_buf ____cacheline_aligned;
	int (*syscfg_inf) (uint32_t module, uint32_t op);
	void (*write) (struct driver_data *drv_data);
	void (*read) (struct driver_data *drv_data);
	 irqreturn_t(*transfer_handler) (struct driver_data *drv_data);
	int (*cs_control) (struct driver_data *drv_data, u8 cs);
};

struct bcm21xx_spi_platform_data {
	u8 slot_id;
	u8 enable_dma;
	u8 cs_line;
	u8 mode;
	int (*cs_control) (struct driver_data *drv_data, u8 cs);
	int (*syscfg_inf) (uint32_t module, uint32_t op);
};
#endif /* REG_SPI_H__ */
