/*******************************************************************************
Copyright 2011 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef _SERIAL_SPI_H
#define _SERIAL_SPI_H

#define DRVNAME				"mdmSPI"
#define TTYNAME				"ttySPI"

#define TTY_SPI_MAX_MINORS		1
#define TTY_SPI_TRANSFER_SIZE		5060/*2048*/	/*4480*/
#define TTY_SPI_FIFO_SIZE		 8192/*	4096*/

#define TTY_SPI_HEADER_OVERHEAD		16
#define TTY_RESET_TIMEOUT		msecs_to_jiffies(50)

/* device flags bitfield definitions */
#define TTY_SPI_STATE_PRESENT			0
#define TTY_SPI_STATE_IO_IN_PROGRESS	1
#define TTY_SPI_STATE_IO_READY			2
#define TTY_SPI_STATE_TIMER_PENDING		3
#define TTY_SPI_STATE_PIN_MUX_SET		4

/* flow control bitfields */
#define TTY_SPI_DCD			0
#define TTY_SPI_CTS			1
#define TTY_SPI_DSR			2
#define TTY_SPI_RI			3
#define TTY_SPI_DTR			4
#define TTY_SPI_RTS			5
#define TTY_SPI_TX_FC			6
#define TTY_SPI_RX_FC			7
#define TTY_SPI_UPDATE			8

#define TTY_SPI_PAYLOAD_SIZE		(TTY_SPI_TRANSFER_SIZE - \
						TTY_SPI_HEADER_OVERHEAD)

#define TTY_SPI_IRQ_TYPE		DETECT_EDGE_RISING
#define TTY_SPI_GPIO_TARGET		0
#define TTY_SPI_GPIO0			0x105

#define TTY_SPI_STATUS_TIMEOUT		(2000*HZ)

/* values for bits in power status byte */
#define TTY_SPI_POWER_DATA_PENDING	1
#define TTY_SPI_POWER_SRDY		2

#define MDMGPIO_OFFSET			112
#define GPIO_AP_ALIVE		(MDMGPIO_OFFSET + 8)	/* mdmgpio 8 */
#define GPIO_MDM_RTS		(MDMGPIO_OFFSET + 7)	/* mdmgpio 7 */
#define GPIO_MDM_RDY		(MDMGPIO_OFFSET + 6)	/* mdmgpio 6 */
/* Mdm Alive and AP Rdy were flipped during
integration camp- below are original values*/
#define GPIO_MDM_ALIVE		(MDMGPIO_OFFSET + 4)	/* mdmgpio 4 */
#define GPIO_AP_RDY			(MDMGPIO_OFFSET + 2)	/* mdmgpio 2 */
#define GPIO_AP_RTS			(MDMGPIO_OFFSET + 0)	/* mdmgpio 0 */

#define MODEM_NORMAL_MODE       0x38
#define MODEM_DOWNLOAD_MODE		0x7c

struct tty_spi_device {
	/* Our SPI device */
	struct spi_device *spi_dev;

	/* Port specific data */
	struct kfifo tx_fifo;
	spinlock_t fifo_lock;
	unsigned long signal_state;

	/* TTY Layer logic */
	struct tty_port tty_port;
	struct device *tty_dev;
	int minor;

	/* Low level I/O work */
	struct tasklet_struct io_work_tasklet;
	struct work_struct io_wq;
	unsigned long flags;
	dma_addr_t rx_dma;
	dma_addr_t tx_dma;

	int modem;		/* Modem type */
	int use_dma;		/* provide dma-able addrs in SPI msg */
	long max_hz;		/* max SPI frequency */

	spinlock_t spi_lock;
	int write_pending;
	spinlock_t power_lock;
	unsigned char power_status;

	unsigned char *rx_buffer;
	unsigned char *tx_buffer;
	dma_addr_t rx_bus;
	dma_addr_t tx_bus;
	unsigned char spi_more;
	unsigned char spi_slave_cts;

	struct timer_list spi_timer;

	struct spi_message spi_msg;
	struct spi_transfer spi_xfer;
	struct semaphore       spi_semaphore;
	struct {
		/* gpio lines */
		unsigned short mdm_rts;		/* Modem request gpio */
		unsigned short mdm_rdy;		/* Modem ready gpio */
		unsigned short ap_rdy;		/* AP ready gpio */
		unsigned short ap_rts;		/* AP request gpio */
		unsigned short mdm_alive;	/* modem alive gpio */
		unsigned short ap_alive;	/* AP alive gpio */
		/* state/stats */
		int unack_srdy_int_nb;
	} gpio;

};

#endif /* _SERIAL_SPI_H */
