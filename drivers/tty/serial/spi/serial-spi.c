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

#include <linux/module.h>
#include <linux/termios.h>
#include <linux/tty.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/kfifo.h>
#include <linux/tty_flip.h>
#include <linux/timer.h>
#include <linux/serial.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/rfkill.h>
#include <linux/fs.h>
#include <linux/ip.h>
#include <linux/dmapool.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>

#include "serial-spi.h"

#define TTY_SPI_MORE_MASK		0x10
#define TTY_SPI_MORE_BIT		12	/* bit position in u16 */
#define TTY_SPI_CTS_BIT			13	/* bit position in u16 */
#define TTY_SPI_MODE			SPI_MODE_0
#define TTY_SPI_TTY_ID			0
#define TTY_SPI_TIMEOUT_SEC		2
#define TTY_SPI_HEADER_0		(-1)
#define TTY_SPI_HEADER_F		(-2)

#define PAD_CTRL 0x35004800

#define SERIAL_SPI_DEBUG
#ifdef SERIAL_SPI_DEBUG
#define dbg_print(fmt, arg...) \
	printk(KERN_ALERT "%s():" fmt, __func__, ##arg)
#else
#define dbg_print(fmt, arg...)   do { } while (0)
#endif

/*#define RHEA_LOOPBACK_TEST*/
#define RHEA_TTY_SPI_DEBUG


/* forward reference */
static void serial_spi_handle_mdm_rts(struct tty_spi_device *tty_spi_dev);
static void serial_spi_insert_flip_string(struct tty_spi_device *tty_spi_dev,
					  unsigned char *chars, size_t size);
static int serial_spi_reset(struct tty_spi_device *tty_spi_dev);
static int serial_spi_device_init(struct spi_master *master);

/* local variables */
static int spi_bpw = 32;	/* 8, 16 or 32 bit word length */
static struct tty_driver *tty_spi_drv;
static struct tty_spi_device *saved_tty_spi_dev;
static struct device *spi_serial_dev;
static struct lock_class_key tty_spi_key;
static int tx_down;

/* Integration Camp 12w16 -- test code to measure sequential packet delivery */
static unsigned tx_seq_num = 1;
static unsigned rx_seq_num = 1;
/**
 *	serial_spi_hangup		-	hang up an TTY SPI device
 *	@tty_spi_dev: our SPI device
 *
 *	Hang up the tty attached to the TTY SPI device if one is currently
 *	open. If not take no action
 */
static void serial_spi_ttyhangup(struct tty_spi_device *tty_spi_dev)
{
	struct tty_port *pport = &tty_spi_dev->tty_port;
	struct tty_struct *tty = tty_port_tty_get(pport);
	if (tty) {
		tty_hangup(tty);
		tty_kref_put(tty);
	}
}

/**
 *	serial_spi_timeout		-	SPI timeout
 *	@arg: our SPI device
 *
 *	The SPI has timed out: hang up the tty. Users will then see a hangup
 *	and error events.
 */
static void serial_spi_timeout(unsigned long arg)
{
	struct tty_spi_device *tty_spi_dev = (struct tty_spi_device *)arg;

	dev_warn(&tty_spi_dev->spi_dev->dev, "*** SPI Timeout ***");
	serial_spi_ttyhangup(tty_spi_dev);
	clear_bit(TTY_SPI_STATE_TIMER_PENDING, &tty_spi_dev->flags);
}

/* char/tty operations */

/**
 *	serial_spi_tiocmget	-	get modem lines
 *	@tty: our tty device
 *	@filp: file handle issuing the request
 *
 *	Map the signal state into Linux modem flags and report the value
 *	in Linux terms
 */
static int serial_spi_tiocmget(struct tty_struct *tty)
{
	unsigned int value;
	struct tty_spi_device *tty_spi_dev = tty->driver_data;

	value =
	    (test_bit(TTY_SPI_RTS, &tty_spi_dev->signal_state) ? TIOCM_RTS : 0)
	    | (test_bit(TTY_SPI_DTR, &tty_spi_dev->signal_state) ? TIOCM_DTR :
	       0) | (test_bit(TTY_SPI_CTS,
			      &tty_spi_dev->
			      signal_state) ? TIOCM_CTS : 0) |
	    (test_bit(TTY_SPI_DSR, &tty_spi_dev->signal_state) ? TIOCM_DSR : 0)
	    | (test_bit(TTY_SPI_DCD, &tty_spi_dev->signal_state) ? TIOCM_CAR :
	       0) | (test_bit(TTY_SPI_RI,
			      &tty_spi_dev->signal_state) ? TIOCM_RNG : 0);
	return value;
}

/**
 *	serial_spi_tiocmset	-	set modem bits
 *	@tty: the tty structure
 *	@set: bits to set
 *	@clear: bits to clear
 *
 *	The Modem only supports DTR and RTS. Set them accordingly
 *	and flag that an update to the modem is needed.
 *
 */
static int serial_spi_tiocmset(struct tty_struct *tty,
			       unsigned int set, unsigned int clear)
{
	struct tty_spi_device *tty_spi_dev = tty->driver_data;

	if (set & TIOCM_RTS)
		set_bit(TTY_SPI_RTS, &tty_spi_dev->signal_state);
	if (set & TIOCM_DTR)
		set_bit(TTY_SPI_DTR, &tty_spi_dev->signal_state);
	if (clear & TIOCM_RTS)
		clear_bit(TTY_SPI_RTS, &tty_spi_dev->signal_state);
	if (clear & TIOCM_DTR)
		clear_bit(TTY_SPI_DTR, &tty_spi_dev->signal_state);

	set_bit(TTY_SPI_UPDATE, &tty_spi_dev->signal_state);
	return 0;
}
/**
 *	serial_spi_open	-	called on tty open
 *	@tty: our tty device
 *	@filp: file handle being associated with the tty
 *
 *	Open the tty interface. We let the tty_port layer do all the work
 *	for us.
 *
 */
static int serial_spi_open(struct tty_struct *tty, struct file *filp)
{
	return tty_port_open(&saved_tty_spi_dev->tty_port, tty, filp);
}

/**
 *	serial_spi_close	-	called when our tty closes
 *	@tty: the tty being closed
 *	@filp: the file handle being closed
 *
 *	Perform the close of the tty. We use the tty_port layer to do all
 *	our hard work.
 */
static void serial_spi_close(struct tty_struct *tty, struct file *filp)
{
	struct tty_spi_device *tty_spi_dev = tty->driver_data;
	tty_port_close(&tty_spi_dev->tty_port, tty, filp);
}

/**
 *	serial_spi_decode_spi_header	-	decode received header
 *	@buffer: the received data
 *	@length: decoded length
 *	@more: decoded more flag
 *
 *	If header is all F it is left, the same as it was,
 *    if header is all 0 it is set to 0 otherwise it is
 *	taken from the incoming header.
 */
static int serial_spi_decode_spi_header(unsigned char *buffer, int *length,
					unsigned char *more,
					unsigned char *received_cts)
{
	unsigned char h1;
	unsigned char h2;
	unsigned char h3;
	unsigned char h4;

	h1 = *buffer;
	h2 = *(buffer + 1);
	h3 = *(buffer + 2);
	h4 = *(buffer + 3);
	*length = buffer[4] + (buffer[5] << 8);
	rx_seq_num = buffer[8] + (buffer[9] << 8) +
			(buffer[10] << 8) + (buffer[11] << 8);
	*more = 0;
	*received_cts = 0;
	dbg_print("RX PKT h1 %x h2 %x h3 %x h4 %x len %x rx_seq_num = %x\n",
			h1, h2, h3, h4, *length, rx_seq_num);
	if (h1 != 0x7F || h2 != 0x7E || h3 != 0x55 || h4 != 0xAA)
		return TTY_SPI_HEADER_F;
	return 0;
}

/**
 *	spi_setup_spi_header	-	set header fields
 *	@txbuffer: pointer to start of SPI buffer
 *	@tx_count: bytes
 *	@more: indicate if more to follow
 *
 *	Format up an SPI header for a transfer
 *
 */
static void serial_spi_setup_spi_header(unsigned char *txbuffer, int tx_count,
					unsigned char more)
{
	/* dbg_print(" tx length=%d\n",tx_count); */
	txbuffer[0] = 0x7F;
	txbuffer[1] = 0x7E;
	txbuffer[2] = 0x55;
	txbuffer[3] = 0xAA;
	txbuffer[4] = tx_count & 0xFF;
	txbuffer[5] = ((tx_count >> 8)) & 0xFF;
	txbuffer[6] = 0;
	txbuffer[7] = 0;
	tx_seq_num++;
	*(u32 *) (txbuffer + 8) = tx_seq_num;
	*(u32 *) (txbuffer + 12) = 0;
	if (tx_seq_num == 0xffffffff)
		tx_seq_num = 0;
	dbg_print("TX PACKET length %x tx_seq_num = %x\n",
		(txbuffer[4] + txbuffer[5] << 8), tx_seq_num);
}

/**
 *	serial_spi_wakeup_serial	-	SPI space made
 *	@port_data: our SPI device
 *
 *	We have emptied the FIFO enough that we want to get more data
 *	queued into it. Poke the line discipline via tty_wakeup so that
 *	it will feed us more bits
 */
static void serial_spi_wakeup_serial(struct tty_spi_device *tty_spi_dev)
{
	struct tty_struct *tty;

	tty = tty_port_tty_get(&tty_spi_dev->tty_port);
	if (!tty)
		return;
	tty_wakeup(tty);
	tty_kref_put(tty);
}

/**
 *	serial_spi_prepare_tx_buffer	-	prepare transmit frame
 *	@tty_spi_dev: our SPI device
 *
 *	The transmit buffr needs a header and various other bits of
 *	information followed by as much data as we can pull from the FIFO
 *	and transfer. This function formats up a suitable buffer in the
 *	tty_spi_dev->tx_buffer
 *
 */
static int serial_spi_prepare_tx_buffer(struct tty_spi_device *tty_spi_dev)
{
	int temp_count;
	int queue_length;
	int tx_count, i;
	unsigned char *tx_buffer;

	dbg_print("enter\n");

	tx_buffer = tty_spi_dev->tx_buffer;
	memset(tx_buffer, 0, TTY_SPI_TRANSFER_SIZE);
	dbg_print("location of txbuffer = 0x%x\n", tx_buffer);

	/* make room for required SPI header */
	tx_buffer += TTY_SPI_HEADER_OVERHEAD;
	tx_count = TTY_SPI_HEADER_OVERHEAD;
	dbg_print("16 byte header\n");

	/* clear to signal no more data if this turns out to be the
	 * last buffer sent in a sequence */
	tty_spi_dev->spi_more = 0;

/*
	queue_length = kfifo_len(&tty_spi_dev->tx_fifo);
	dbg_print("tx_buffer queue_length=%d\n",queue_length);
	if (queue_length != 0) {
*/
		/* data to mux -- see if there's room for it */
/*
		temp_count = min(queue_length, TTY_SPI_PAYLOAD_SIZE);
		dbg_print(" buffer queue or max payload size =%d\n",
				temp_count);
*/
		temp_count = kfifo_out_locked(&tty_spi_dev->tx_fifo,
/*					      tx_buffer, temp_count,*/
					      tx_buffer, TTY_SPI_FIFO_SIZE,
					      &tty_spi_dev->fifo_lock);
		kfifo_reset(&tty_spi_dev->tx_fifo);
		queue_length = kfifo_len(&tty_spi_dev->tx_fifo);
		dbg_print("tx_buffer queue_length after reset =%d\n",
					queue_length);
		dbg_print("tx_buffer queue_length=%d\n", queue_length);
		/* update buffer pointer and data count in message */
		tx_buffer += temp_count;
		tx_count += temp_count;
		dbg_print(" total tx count with header =%d\n", tx_count);
		if (temp_count == queue_length) {
			/* poke port to get more data */
			dbg_print(" fifo is empty poke it\n");
			serial_spi_wakeup_serial(tty_spi_dev);
			}
		else {	/* more data in port, use next SPI message */
			tty_spi_dev->spi_more = 1;
			dbg_print(" fifo is not empty spi_more bit\n");
		}
/*	}*/
	/* have data and info for header -- set up SPI header in buffer */
	/* spi header needs payload size, not entire buffer size */
	dbg_print("location of txbuffer after filling in = 0x%x\n",
				tx_buffer);
	dbg_print(" data to be transmitted\n");
	temp_count = tx_count;
	if (tx_count % 64)
		tx_count = ((tx_count >> 6) + 1) << 6;	/* 64 bytes alignment */
	serial_spi_setup_spi_header(tty_spi_dev->tx_buffer,
				    temp_count - TTY_SPI_HEADER_OVERHEAD,
				    tty_spi_dev->spi_more);
#if 0
	for (i = 0; i < (24/4); i++)
		dbg_print(" [%d]=0x%x 0x%x 0x%x 0x%x\n", i*4,
				*(tty_spi_dev->tx_buffer+(i*4)),
				*(tty_spi_dev->tx_buffer+(i*4)+1),
				*(tty_spi_dev->tx_buffer+(i*4)+2),
				*(tty_spi_dev->tx_buffer+(i*4)+3)
				);
#endif
	return tx_count;
}

static void serial_spi_print_gpio(struct tty_spi_device *tty_spi_dev)
{
	printk(KERN_ERR "Mdm_rdy gpio%d value =%d\n", tty_spi_dev->gpio.mdm_rdy,
	       gpio_get_value(tty_spi_dev->gpio.mdm_rdy));
	printk(KERN_ERR "Mdm_rts gpio%d value =%d\n", tty_spi_dev->gpio.mdm_rts,
	       gpio_get_value(tty_spi_dev->gpio.mdm_rts));
	printk(KERN_ERR "Ap_rdy gpio%d value =%d\n", tty_spi_dev->gpio.ap_rdy,
	       gpio_get_value(tty_spi_dev->gpio.ap_rdy));
	printk(KERN_ERR "Ap_rts gpio%d value =%d\n", tty_spi_dev->gpio.ap_rts,
	       gpio_get_value(tty_spi_dev->gpio.ap_rts));
}

/**
 *	serial_spi_write		-	line discipline write
 *	@tty: our tty device
 *	@buf: pointer to buffer to write (kernel space)
 *	@count: size of buffer
 *
 *	Write the characters we have been given into the FIFO. If the device
 *	is not active then activate it, when the SRDY line is asserted back
 *	this will commence I/O
 */
static int serial_spi_write(struct tty_struct *tty, const unsigned char *buf,
			    int count)
{
	struct tty_spi_device *tty_spi_dev = tty->driver_data;

	unsigned char *tmp_buf = (unsigned char *)buf;
	void __iomem *vaddr;
	int i;
	int tx_count;
	int fifo_queue_length;
	dbg_print("   enter\n");
	if (!test_bit(TTY_SPI_STATE_PIN_MUX_SET, &(tty_spi_dev->flags))) {
		/*hack to config pin mux. CP is hardcoding the pin Mux */
		dbg_print(" serial_spi_write  pin mux is not set\n");
		vaddr = ioremap(PAD_CTRL, 1024);

		dbg_print("Before setting......\n");
		for (i = 0; i < 9; i++)
			dbg_print(" Address %p   %x\n",
					vaddr + 0xE8 + i * 4,
					readl(vaddr + 0xE8 + i * 4));
		/*write password */
		writel(0xa5a501, vaddr + 0x7F0);
		/*unlock config register */
		writel(0, vaddr + 0x784);
		writel(0, vaddr + 0x788);
		for (i = 0; i < 9; i++)
			writel(0x443, vaddr + 0xE8 + i * 4);
		dbg_print("After setting......\n");
		for (i = 0; i < 9; i++) {
			dbg_print(" Address %p   %x\n",
					vaddr + 0xE8 + i * 4,
					readl(vaddr + 0xE8 + i * 4));
		}
		set_bit(TTY_SPI_STATE_PIN_MUX_SET, &(tty_spi_dev->flags));
#ifdef RHEA_LOOPBACK_TEST
		dbg_print("Loop back setting AP and mdm alive ......\n");
		gpio_set_value(tty_spi_dev->gpio.ap_alive, 1);
		/* simulation of modem rdy */
		gpio_set_value(tty_spi_dev->gpio.mdm_alive, 1);
		/* simulation of modem rts */
#endif
		sema_init(&tty_spi_dev->spi_semaphore, 1);

		serial_spi_print_gpio(tty_spi_dev);
	}
	dbg_print("spi_semaphore down\n");
	/*down(&tty_spi_dev->spi_semaphore);

	condition not needed,
	semaphore will be blocked
	if any Tx or Rx transactions is on-going

	if(gpio_get_value(tty_spi_dev->gpio.mdm_rts) &&
		gpio_get_value(tty_spi_dev->gpio.mdm_rdy))
*/
	{
	    dbg_print("write started\n");
		tx_down = 1;
		set_bit(TTY_SPI_TX_FC, &tty_spi_dev->signal_state);
		tx_count = kfifo_in_locked(&tty_spi_dev->tx_fifo,
					   tmp_buf, count,
					   &tty_spi_dev->fifo_lock);
		fifo_queue_length = kfifo_len(&tty_spi_dev->tx_fifo);
		dbg_print("kfifo length after new write is %x ",
				fifo_queue_length);
		gpio_set_value(tty_spi_dev->gpio.ap_rts, 1);
#ifdef RHEA_LOOPBACK_TEST
		gpio_set_value(tty_spi_dev->gpio.ap_alive, 0);
		/* For simulation of modem mdm_rdy */
#endif
		serial_spi_print_gpio(tty_spi_dev);
		dbg_print("fifo set and configure the GPIOs for transmit\n");
		/*up(&tty_spi_dev->spi_semaphore);*/
		dbg_print("   exit\n");
		return tx_count;
	}
	dbg_print(" spi_semaphore up/release failure\n");
	/*up(&tty_spi_dev->spi_semaphore);*/
	dbg_print("exit no tx count == 0\n");
	return 0;
}
static int serial_spi_read(struct tty_spi_device *tty_spi_dev)
{

	unsigned char *tx_buffer;
	dbg_print(" serial_spi_read  enter\n");
	tx_buffer = tty_spi_dev->tx_buffer;
#ifndef RHEA_LOOPBACK_TEST
	memset(tx_buffer, 0, TTY_SPI_TRANSFER_SIZE);
#endif
	return TTY_SPI_TRANSFER_SIZE;
}

/**
 *	serial_spi_chars_in_buffer	-	line discipline helper
 *	@tty: our tty device
 *
 *	Report how much data we can accept before we drop bytes. As we use
 *	a simple FIFO this is nice and easy.
 */
static int serial_spi_write_room(struct tty_struct *tty)
{
	struct tty_spi_device *tty_spi_dev = tty->driver_data;
	return TTY_SPI_FIFO_SIZE - kfifo_len(&tty_spi_dev->tx_fifo);
}

/**
 *	serial_spi_chars_in_buffer	-	line discipline helper
 *	@tty: our tty device
 *
 *	Report how many characters we have buffered. In our case this is the
 *	number of bytes sitting in our transmit FIFO.
 */
static int serial_spi_chars_in_buffer(struct tty_struct *tty)
{
	struct tty_spi_device *tty_spi_dev = tty->driver_data;

	return kfifo_len(&tty_spi_dev->tx_fifo);
}

/**
 *	spi_port_hangup
 *	@port: our tty port
 *
 *	tty port hang up. Called when tty_hangup processing is invoked either
 *	by loss of carrier, or by software (eg vhangup). Serialized against
 *	activate/shutdown by the tty layer.
 */
static void serial_spi_hangup(struct tty_struct *tty)
{
	struct tty_spi_device *tty_spi_dev = tty->driver_data;
	tty_port_hangup(&tty_spi_dev->tty_port);
}

/**
 *	tty_spi_port_activate
 *	@port: our tty port
 *
 *	tty port activate method - called for first open. Serialized
 *	with hangup and shutdown by the tty layer.
 */
static int tty_spi_port_activate(struct tty_port *port, struct tty_struct *tty)
{
	struct tty_spi_device *tty_spi_dev =
	    container_of(port, struct tty_spi_device, tty_port);

	/* clear any old data; can't do this in 'close' */
	kfifo_reset(&tty_spi_dev->tx_fifo);

	/* put port data into this tty */
	tty->driver_data = tty_spi_dev;

	/* allows flip string push from int context */
	tty->low_latency = 0;

	return 0;
}

/**
 *	tty_spi_port_shutdown
 *	@port: our tty port
 *
 *	tty port shutdown method - called for last port close. Serialized
 *	with hangup and activate by the tty layer.
 */
static void tty_spi_port_shutdown(struct tty_port *port)
{
	struct tty_spi_device *tty_spi_dev =
	    container_of(port, struct tty_spi_device, tty_port);
	clear_bit(TTY_SPI_STATE_TIMER_PENDING, &tty_spi_dev->flags);
	tasklet_kill(&tty_spi_dev->io_work_tasklet);
}

static const struct tty_port_operations tty_spi_port_ops = {
	.activate = tty_spi_port_activate,
	.shutdown = tty_spi_port_shutdown,
};

static const struct tty_operations tty_spi_serial_ops = {
	.open = serial_spi_open,
	.close = serial_spi_close,
	.write = serial_spi_write,
	.hangup = serial_spi_hangup,
	.write_room = serial_spi_write_room,
	.chars_in_buffer = serial_spi_chars_in_buffer,
	.tiocmget = serial_spi_tiocmget,
	.tiocmset = serial_spi_tiocmset,
};

/**
 *	serial_spi_insert_fip_string	-	queue received data
 *	@tty_spi_ser: our SPI device
 *	@chars: buffer we have received
 *	@size: number of chars reeived
 *
 *	Queue bytes to the tty assuming the tty side is currently open. If
 *	not the discard the data.
 */
static void serial_spi_insert_flip_string(struct tty_spi_device *tty_spi_dev,
					  unsigned char *chars, size_t size)
{
	struct tty_struct *tty = tty_port_tty_get(&tty_spi_dev->tty_port);
	dbg_print("ENTER\n");
	if (!tty)
		return;
	tty_insert_flip_string(tty, chars, size);
	dbg_print("insert string\n");
	tty_flip_buffer_push(tty);
	dbg_print("push\n");
	tty_kref_put(tty);
	dbg_print("Exit\n");
}

/**
 *	serial_spi_complete	-	SPI transfer completed
 *	@spi_dev: our SPI device
 *
 *	An SPI transfer has completed. Process any received data and kick off
 *	any further transmits we can commence.
 */
static void serial_spi_complete(void *spi_dev)
{
	struct tty_spi_device *tty_spi_dev = spi_dev;
	int length;
	int actual_length;
	unsigned char more;
	unsigned char cts;
	int local_write_pending = 0;
	int queue_length;
	int decode_result;
	int rx = 0, i;

	dbg_print("ENTER\n");
	if (!tty_spi_dev->spi_msg.status && test_bit(TTY_SPI_RX_FC,
						     &tty_spi_dev->
						     signal_state)) {
		dbg_print("RX FC is set\n");
		rx = 1;
		/* check header validity, get comm flags */
		decode_result =
		    serial_spi_decode_spi_header(tty_spi_dev->rx_buffer,
						 &length, &more, &cts);
		for (i = 0; i < 24; i++)
			dbg_print(" [%d]=0x%x\n", i,
					*(tty_spi_dev->rx_buffer+(i)));
		serial_spi_print_gpio(tty_spi_dev);

		if (decode_result == TTY_SPI_HEADER_F) {
			dbg_print(" header decode failed\n");
			goto complete_exit;
		}
		/* cts is always zero according to the decode function*/
		tty_spi_dev->spi_slave_cts = cts;
		gpio_set_value(tty_spi_dev->gpio.ap_rdy, 0);
		actual_length = min((unsigned int)length,
				    tty_spi_dev->spi_msg.actual_length);

#if 1
		serial_spi_insert_flip_string(tty_spi_dev,
					      tty_spi_dev->rx_buffer +
					      TTY_SPI_HEADER_OVERHEAD,
					      (size_t) actual_length);
#endif
		dbg_print(" data saved to tty queue\n");
#ifdef RHEA_TTY_SPI_DEBUG
		dbg_print("rx actual length: %d frame length %d\n",
			  actual_length, length);
#endif
		if (actual_length < length) {
			dbg_print("rescheduling work ######");
			schedule_work(&tty_spi_dev->io_wq);
		}
	} else {
		dbg_print("serial_spi_complete SPI transfer no RX");
	}

complete_exit:
	serial_spi_print_gpio(tty_spi_dev);
	if (tty_spi_dev->write_pending) {
		dbg_print("serial_spi_complete :: write pending");
		tty_spi_dev->write_pending = 0;
		local_write_pending = 1;
	}

	clear_bit(TTY_SPI_STATE_IO_IN_PROGRESS, &(tty_spi_dev->flags));
	dbg_print("serial_spi_complete ::read done ");
/*	kfifo_reset(&tty_spi_dev->tx_fifo);
	queue_length = kfifo_len(&tty_spi_dev->tx_fifo);

	only tx
*/
	if (rx == 0) {
		/* && tx_down) { */
		dbg_print("serial_spi_complete rx = 0 and tx_down =1");
		/*kfifo_reset(&tty_spi_dev->tx_fifo);*/
		queue_length = kfifo_len(&tty_spi_dev->tx_fifo);
		gpio_set_value(tty_spi_dev->gpio.ap_rts, 0);
		clear_bit(TTY_SPI_TX_FC, &tty_spi_dev->signal_state);
#ifdef RHEA_LOOPBACK_TEST
		gpio_set_value(tty_spi_dev->gpio.ap_alive, 1);
		/* simulation of modem rdy */
		gpio_set_value(tty_spi_dev->gpio.mdm_alive, 0);
		/* simulation of modem rts */
#endif
		/*up(&tty_spi_dev->spi_semaphore); */
		dbg_print("spi_semaphore cleared");

	} else {
	/* only rx */
		dbg_print("if its rx only so clear gpios accordingly ");
		gpio_set_value(tty_spi_dev->gpio.ap_rdy, 1);
		clear_bit(TTY_SPI_RX_FC, &tty_spi_dev->signal_state);
#ifdef RHEA_LOOPBACK_TEST
		gpio_set_value(tty_spi_dev->gpio.ap_alive, 1);
		/* simulation of modem rdy */
		gpio_set_value(tty_spi_dev->gpio.mdm_alive, 1);
		/* simulation of modem rts */
#endif
		up(&tty_spi_dev->spi_semaphore);
	}
	serial_spi_print_gpio(tty_spi_dev);
}

/**
 *	tty_spi_spio_io_wq		-	I/O work queue
 *	@data: our SPI device
 *
 *	Queue data for transmission if possible and then kick off the
 *	transfer.
 */
static void serial_spi_io_wq(struct work_struct *wq)
{
	int retval;
	struct tty_spi_device *tty_spi_dev =
	    container_of(wq, struct tty_spi_device, io_wq);
	dbg_print("Enter\n");

	if (!gpio_get_value(tty_spi_dev->gpio.mdm_rdy)
	    || !gpio_get_value(tty_spi_dev->gpio.mdm_rts)) {

		/* not sure why this unack srdy init nb is used. */
		if (tty_spi_dev->gpio.unack_srdy_int_nb > 0)
			tty_spi_dev->gpio.unack_srdy_int_nb--;

		if (test_bit(TTY_SPI_RX_FC, &tty_spi_dev->signal_state)) {
			retval = serial_spi_read(tty_spi_dev);
			down(&tty_spi_dev->spi_semaphore);
		} else {
			down(&tty_spi_dev->spi_semaphore);
			retval = serial_spi_prepare_tx_buffer(tty_spi_dev);
#ifdef RHEA_TTY_SPI_DEBUG
			dbg_print("Sending %d bytes to modem...\n",
					retval);
#endif
		}
		dbg_print(" tx buffer done or Rx got\n");
		spi_message_init(&tty_spi_dev->spi_msg);
		INIT_LIST_HEAD(&tty_spi_dev->spi_msg.queue);

		tty_spi_dev->spi_msg.context = tty_spi_dev;
		tty_spi_dev->spi_msg.complete = serial_spi_complete;

		/* set up our spi transfer */
		/* note len is BYTES, not transfers */
		tty_spi_dev->spi_xfer.len = retval;
		/*tty_spi_dev->spi_xfer.cs_change = 0;*/
		tty_spi_dev->spi_xfer.delay_usecs = 0;
		tty_spi_dev->spi_xfer.speed_hz =
		    tty_spi_dev->spi_dev->max_speed_hz;
		tty_spi_dev->spi_xfer.bits_per_word = spi_bpw;

		tty_spi_dev->spi_xfer.tx_buf = tty_spi_dev->tx_buffer;
		tty_spi_dev->spi_xfer.rx_buf = tty_spi_dev->rx_buffer;

		spi_message_add_tail(&tty_spi_dev->spi_xfer,
				     &tty_spi_dev->spi_msg);

		retval = spi_async(tty_spi_dev->spi_dev, &tty_spi_dev->spi_msg);
		dbg_print("Exit wq done\n");
	} else {
		dbg_print("No active signals detected ???!!!\n");
		tty_spi_dev->write_pending = 1;
	}
}

/**
 *	serial_spi_free_port	-	free up the tty side
 *	@tty_spi_dev: TTY SPI device going away
 *
 *	Unregister and free up a port when the device goes away
 */
static void serial_spi_free_port(struct tty_spi_device *tty_spi_dev)
{
	if (tty_spi_dev->tty_dev)
		tty_unregister_device(tty_spi_drv, tty_spi_dev->minor);
	kfifo_free(&tty_spi_dev->tx_fifo);
}

/**
 *	serial_spi_create_port	-	create a new port
 *	@tty_spi_dev: our spi device
 *
 *	Allocate and initialise the tty port that goes with this interface
 *	and add it to the tty layer so that it can be opened.
 */
static int serial_spi_create_port(struct tty_spi_device *tty_spi_dev)
{
	int ret = 0;
	struct tty_port *pport = &tty_spi_dev->tty_port;

	spin_lock_init(&tty_spi_dev->fifo_lock);
	lockdep_set_class_and_subclass(&tty_spi_dev->fifo_lock,
				       &tty_spi_key, 0);

	if (kfifo_alloc(&tty_spi_dev->tx_fifo, TTY_SPI_FIFO_SIZE, GFP_KERNEL)) {
		ret = -ENOMEM;
		dbg_print("Allocate fifo error!!!!\n");
		goto error_ret;
	}

	tty_port_init(pport);
	pport->ops = &tty_spi_port_ops;
	tty_spi_dev->minor = TTY_SPI_TTY_ID;
	tty_spi_dev->tty_dev =
	    tty_register_device(tty_spi_drv, tty_spi_dev->minor,
				spi_serial_dev);
	if (IS_ERR(tty_spi_dev->tty_dev)) {
		dev_dbg(&tty_spi_dev->spi_dev->dev,
			"%s: registering tty device failed", __func__);
		ret = PTR_ERR(tty_spi_dev->tty_dev);
		goto error_ret;
	}
	return 0;

error_ret:
	serial_spi_free_port(tty_spi_dev);

	return ret;
}

/**
 *	serial_spi_handle_mdm_rts		-	handle Modem RTS
 *	@tty_spi_dev: device handle Modem RTS
 *
 *
 */
static void serial_spi_handle_mdm_rts(struct tty_spi_device *tty_spi_dev)
{
	dbg_print(" Enter\n");
	if (!gpio_get_value(tty_spi_dev->gpio.mdm_rts)) {
		dbg_print(" Detected Falling Edge\n");
		set_bit(TTY_SPI_RX_FC, &tty_spi_dev->signal_state);
		gpio_set_value(tty_spi_dev->gpio.ap_rdy, 0);
		schedule_work(&tty_spi_dev->io_wq);
	} else {
		dbg_print("Detected Rising Edge\n");
#if 0
		if (!tx_down)
			return;
#endif
		tx_down = 0;
		/*up(&tty_spi_dev->spi_semaphore);*/
		set_bit(TTY_SPI_STATE_IO_READY, &tty_spi_dev->flags);
	}
}

/**
 *	spi_mdm_rts_interrupt	-	mdm_rts
 *	@irq: our IRQ number
 *	@dev: our tty_spi device
 *
 *	The modem RTS. Handle the rts event
 */
static irqreturn_t spi_mdm_rts_interrupt(int irq, void *dev)
{
	struct tty_spi_device *tty_spi_dev = dev;
	tty_spi_dev->gpio.unack_srdy_int_nb++;

	dbg_print("enter spi_mdm_rts_interrupt\n");
/*
	dbg_print("In modem rts ISR\n");
	dbg_print("Get MDM_RTS gpio%d value =%d\n",
			tty_spi_dev->gpio.mdm_rts,
		gpio_get_value(tty_spi_dev->gpio.mdm_rts));
	int val = gpio_get_value(tty_spi_dev->gpio.mdm_alive);
*/
	serial_spi_handle_mdm_rts(tty_spi_dev);
	return IRQ_HANDLED;
}

/**
 *	serial_spi_handle_mdm_rts		-	handle Modem RTS
 *	@tty_spi_dev: device handle Modem RTS
 *
 */
static void serial_spi_handle_mdm_rdy(struct tty_spi_device *tty_spi_dev)
{
	dbg_print(" Enter\n");

	if (!gpio_get_value(tty_spi_dev->gpio.mdm_rdy)) {
		dbg_print(" falling edge detected\n");
		/* we do set tx flow control in
		write api and shouldnt be repeated here*/
		set_bit(TTY_SPI_TX_FC, &tty_spi_dev->signal_state);
		schedule_work(&tty_spi_dev->io_wq);
	} else {
		dbg_print(" rising edge detected move to ready state\n");
		up(&tty_spi_dev->spi_semaphore);
		set_bit(TTY_SPI_STATE_IO_READY, &tty_spi_dev->flags);
	}
}

/**
 *	spi_mdm_rdy_interrupt	-	Modem RDY
 *	@irq: interrupt number
 *	@dev: our device pointer
 *
 *	The modem has enter RDY state. Check the GPIO
 *	line to see which.
 *
 */
static irqreturn_t spi_mdm_rdy_interrupt(int irq, void *dev)
{
	struct tty_spi_device *tty_spi_dev = dev;
	dbg_print("In modem rdy ISR\n");
	/*
	   dbg_print("In modem rdy ISR\n");
	   dbg_print("Get MDM_RDY gpio%d value =%d\n",
				tty_spi_dev->gpio.mdm_rdy,
				gpio_get_value(tty_spi_dev->gpio.mdm_rdy));
	 */
	serial_spi_handle_mdm_rdy(tty_spi_dev);
	return IRQ_HANDLED;
}

/**
 *	serial_spi_free_device - free device
 *	@tty_spi_dev: device to free
 *
 *	Free the SPI device
 */
static void serial_spi_free_device(struct tty_spi_device *tty_spi_dev)
{
	serial_spi_free_port(tty_spi_dev);
	/*
	   dma_free_coherent(&tty_spi_dev->spi_dev->dev,
	   TTY_SPI_TRANSFER_SIZE,
	   tty_spi_dev->tx_buffer,
	   tty_spi_dev->tx_bus);
	   dma_free_coherent(&tty_spi_dev->spi_dev->dev,
	   TTY_SPI_TRANSFER_SIZE,
	   tty_spi_dev->rx_buffer,
	   tty_spi_dev->rx_bus);
	 */
	kfree(tty_spi_dev->rx_buffer);
	kfree(tty_spi_dev->tx_buffer);
}

/**
 *	serial_spi_reset	-	reset modem
 *	@tty_spi_dev: modem to reset
 *
 *	Perform a reset on the modem
 */
static int serial_spi_reset(struct tty_spi_device *tty_spi_dev)
{
	int ret = 0;
	/*
	 * set up modem power, reset
	 *
	 * delays are required on some platforms for the modem
	 * to reset properly
	 */
	gpio_set_value(tty_spi_dev->gpio.ap_rts, 0);
	gpio_set_value(tty_spi_dev->gpio.ap_rdy, 1);
	/*
	dbg_print("Aftre Clear AP_RTS pin %d bit=%d",
			tty_spi_dev->gpio.ap_rts,
			gpio_get_value(tty_spi_dev->gpio.ap_rts));
	 */
	clear_bit(TTY_SPI_TX_FC, &tty_spi_dev->signal_state);
	clear_bit(TTY_SPI_RX_FC, &tty_spi_dev->signal_state);
	return ret;
}

/**
 *	serial_spi_spi_remove	-	SPI device was removed
 *	@spi: SPI device
 *
 *	FIXME: We should be shutting the device down here not in
 *	the module unload path.
 */

static int serial_spi_spi_remove(struct tty_spi_device *spi)
{
	struct tty_spi_device *tty_spi_dev = spi;
	/* stop activity */
	/* tasklet_kill(&tty_spi_dev->io_work_tasklet); */
	/* free irq */
	free_irq(gpio_to_irq(tty_spi_dev->gpio.mdm_alive), (void *)tty_spi_dev);
	free_irq(gpio_to_irq(tty_spi_dev->gpio.mdm_rts), (void *)tty_spi_dev);

	gpio_free(tty_spi_dev->gpio.mdm_rts);
	gpio_free(tty_spi_dev->gpio.mdm_rdy);
	gpio_free(tty_spi_dev->gpio.ap_rdy);
	gpio_free(tty_spi_dev->gpio.ap_rts);
	gpio_free(tty_spi_dev->gpio.mdm_alive);

	/* free allocations */
	serial_spi_free_device(tty_spi_dev);
	saved_tty_spi_dev = NULL;
	return 0;
}

/**
 *	serial_spi_exit	-	module exit
 *
 *	Unload the module.
 */

static void __exit serial_spi_exit(void)
{
	/* unregister */
	tty_unregister_driver(tty_spi_drv);
}

static int serial_spi_device_init(struct spi_master *master)
{
	int ret;
	/*int srdy;*/
	struct tty_spi_device *tty_spi_dev;
	struct spi_device *spi;
	struct device *d;
	int i;
	void __iomem *vaddr;
	dbg_print("Enter\n");

	if (saved_tty_spi_dev) {
		dev_dbg(&spi->dev, "ignoring subsequent detection");
		return -ENODEV;
	}
	d = bus_find_device_by_name(&spi_bus_type, NULL, "spi0.0");
	if (d) {
		spi = (struct spi_device *)d;
		dbg_print("spi_device_handler = 0x%x\n", d);
	} else {
		spi = spi_alloc_device(master);
		if (!spi) {
			dbg_print("spi allocated device failed!");
			return -ENODEV;
		}
		ret = spi_add_device(spi);
		if (ret) {
			dbg_print("add spi device failed!");
			return -ENODEV;
		}
	}
	/* initialize structure to hold our device variables */
	tty_spi_dev = kzalloc(sizeof(struct tty_spi_device), GFP_KERNEL);
		dbg_print("serial_spi_device_init tty_spi_dev  0x%x\n",
				tty_spi_dev);
	if (!tty_spi_dev) {
		dev_err(&spi->dev, "spi device allocation failed");
		return -ENOMEM;
	}
	saved_tty_spi_dev = tty_spi_dev;
	tty_spi_dev->spi_dev = spi;
	clear_bit(TTY_SPI_STATE_IO_IN_PROGRESS, &tty_spi_dev->flags);
	spin_lock_init(&tty_spi_dev->spi_lock);
	spin_lock_init(&tty_spi_dev->power_lock);
	tty_spi_dev->power_status = 0;
	init_timer(&tty_spi_dev->spi_timer);
	tty_spi_dev->spi_timer.function = serial_spi_timeout;
	tty_spi_dev->spi_timer.data = (unsigned long)tty_spi_dev;

	/* initialize spi mode, etc */
	tty_spi_dev->use_dma = 0;
	tty_spi_dev->max_hz = 20000000;
	spi->max_speed_hz = tty_spi_dev->max_hz;
	/*spi->mode = TTY_SPI_MODE | (SPI_LOOP & spi->mode);*/
	spi->mode = TTY_SPI_MODE;
	spi->bits_per_word = spi_bpw;
	ret = spi_setup(spi);
	if (ret) {
		dev_err(&spi->dev, "SPI setup wasn't successful %d", ret);
		return -ENODEV;
	}

	/* ensure SPI protocol flags are initialized to enable transfer */
	tty_spi_dev->spi_more = 0;
	tty_spi_dev->spi_slave_cts = 0;

	tty_spi_dev->rx_buffer = kmalloc(TTY_SPI_TRANSFER_SIZE, GFP_KERNEL);
	if (!tty_spi_dev->rx_buffer) {
		dev_err(&spi->dev, "SPI RX buffer allocation failed");
		ret = -ENOMEM;
		goto error_ret;
	}
	tty_spi_dev->tx_buffer = kmalloc(TTY_SPI_TRANSFER_SIZE, GFP_KERNEL);
	if (!tty_spi_dev->tx_buffer) {
		dev_err(&spi->dev, "SPI TX buffer allocation failed");
		ret = -ENOMEM;
		goto error_ret;
	}

	spi_set_drvdata(spi, tty_spi_dev);
	/*
	   tasklet_init(&tty_spi_dev->io_work_tasklet, serial_spi_io,
	   (unsigned long)tty_spi_dev);
	 */
	INIT_WORK(&tty_spi_dev->io_wq, serial_spi_io_wq);

	sema_init(&tty_spi_dev->spi_semaphore, 1);
	printk(KERN_ERR "Semaphore after init: %d\n",
	       tty_spi_dev->spi_semaphore.count);
	set_bit(TTY_SPI_STATE_PRESENT, &tty_spi_dev->flags);

	/* create our tty port */
	ret = serial_spi_create_port(tty_spi_dev);
	if (ret != 0) {
		dev_err(&spi->dev, "create default tty port failed");
		goto error_ret;
	}

	/* set pm runtime power state and register with power system */
	pm_runtime_set_active(&spi->dev);
	pm_runtime_enable(&spi->dev);

	/* Configure GPIO pins */
	tty_spi_dev->gpio.ap_rdy = GPIO_AP_RDY;
	tty_spi_dev->gpio.ap_rts = GPIO_AP_RTS;
	tty_spi_dev->gpio.mdm_rdy = GPIO_MDM_RDY;
	tty_spi_dev->gpio.mdm_rts = GPIO_MDM_RTS;
	tty_spi_dev->gpio.mdm_alive = GPIO_MDM_ALIVE;
	tty_spi_dev->gpio.ap_alive = GPIO_AP_ALIVE;

	dev_info(&spi->dev, "gpios %d, %d, %d, %d, %d",
		 tty_spi_dev->gpio.ap_rdy, tty_spi_dev->gpio.ap_rts,
		 tty_spi_dev->gpio.mdm_rdy, tty_spi_dev->gpio.mdm_rts,
		 tty_spi_dev->gpio.mdm_alive);
	/*hack to config pin mux. CP is hardcoding the pin Mux */
	vaddr = ioremap(PAD_CTRL, 1024);
	/*write password */
	writel(0xa5a501, vaddr + 0x7F0);
	/*unlock config register */
	writel(0, vaddr + 0x784);
	writel(0, vaddr + 0x788);
	for (i = 0; i < 9; i++)
		writel(0x443, vaddr + 0xE8 + i * 4);
	/* Configure gpios */
	ret = gpio_request(tty_spi_dev->gpio.ap_rdy, "mdmgpio2");
	if (ret < 0) {
		dev_err(&spi->dev, "Unable to allocate GPIO%d (RESET)",
			tty_spi_dev->gpio.ap_rdy);
		goto error_ret;
	}
	ret += gpio_direction_output(tty_spi_dev->gpio.ap_rdy, 0);
	ret += gpio_export(tty_spi_dev->gpio.ap_rdy, 1);
	if (ret) {
		dev_err(&spi->dev, "Unable to configure GPIO%d (RESET)",
			tty_spi_dev->gpio.ap_rdy);
		ret = -EBUSY;
		goto error_ret2;
	}

	ret = gpio_request(tty_spi_dev->gpio.ap_rts, "mdmgpio0");
	ret += gpio_direction_output(tty_spi_dev->gpio.ap_rts, 0);
	ret += gpio_export(tty_spi_dev->gpio.ap_rts, 1);
	if (ret) {
		dev_err(&spi->dev, "Unable to configure GPIO%d (ON)",
			tty_spi_dev->gpio.ap_rts);
		ret = -EBUSY;
		goto error_ret3;
	}

	ret = gpio_request(tty_spi_dev->gpio.mdm_rdy, "mdmgpio7");
	if (ret < 0) {
		dev_err(&spi->dev, "Unable to allocate GPIO%d (MRDY)",
			tty_spi_dev->gpio.mdm_rdy);
		goto error_ret3;
	}
	ret += gpio_export(tty_spi_dev->gpio.mdm_rdy, 1);
	ret += gpio_direction_input(tty_spi_dev->gpio.mdm_rdy);
	if (ret) {
		dev_err(&spi->dev, "Unable to configure GPIO%d (MRDY)",
			tty_spi_dev->gpio.mdm_rdy);
		ret = -EBUSY;
		goto error_ret4;
	}

	ret = gpio_request(tty_spi_dev->gpio.mdm_rts, "mdmgpio6");
	if (ret < 0) {
		dev_err(&spi->dev, "Unable to allocate GPIO%d (RTS)",
			tty_spi_dev->gpio.mdm_rts);
		ret = -EBUSY;
		goto error_ret4;
	}
	ret += gpio_export(tty_spi_dev->gpio.mdm_rts, 1);
	ret += gpio_direction_input(tty_spi_dev->gpio.mdm_rts);
	if (ret) {
		dev_err(&spi->dev, "Unable to configure GPIO%d (RTS)",
			tty_spi_dev->gpio.mdm_rts);
		ret = -EBUSY;
		goto error_ret5;
	}

	ret = gpio_request(tty_spi_dev->gpio.mdm_alive, "mdmgpio4");
	if (ret < 0) {
		dev_err(&spi->dev, "Unable to allocate GPIO%d (MDM_ALIVE)",
			tty_spi_dev->gpio.mdm_alive);
		goto error_ret5;
	}
	ret += gpio_export(tty_spi_dev->gpio.mdm_alive, 1);
#ifdef RHEA_LOOPBACK_TEST
	ret += gpio_direction_output(tty_spi_dev->gpio.mdm_alive, 1);
#else
	ret += gpio_direction_input(tty_spi_dev->gpio.mdm_alive);
#endif
	if (ret) {
		dev_err(&spi->dev, "Unable to configure GPIO%d (RESET_OUT)",
			tty_spi_dev->gpio.mdm_alive);
		ret = -EBUSY;
		goto error_ret6;
	}

	ret = gpio_request(tty_spi_dev->gpio.ap_alive, "mdmgpio8");
	if (ret < 0) {
		dev_err(&spi->dev, "Unable to allocate GPIO%d (AP_ALIVE)",
			tty_spi_dev->gpio.ap_alive);
		goto error_ret6;
	}
	ret += gpio_export(tty_spi_dev->gpio.ap_alive, 1);
	ret += gpio_direction_output(tty_spi_dev->gpio.ap_alive, 1);
	if (ret) {
		dev_err(&spi->dev, "Unable to configure GPIO%d (AP_ALIVE)",
			tty_spi_dev->gpio.mdm_alive);
		ret = -EBUSY;
		gpio_free(tty_spi_dev->gpio.ap_alive);
		goto error_ret7;
	}

	ret = request_irq(gpio_to_irq(tty_spi_dev->gpio.mdm_rdy),
			  spi_mdm_rdy_interrupt,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, DRVNAME,
			  (void *)tty_spi_dev);
	if (ret) {
		dev_err(&spi->dev, "Unable to get irq %x\n",
			gpio_to_irq(tty_spi_dev->gpio.mdm_rdy));
		goto error_ret6;
	}

	ret = serial_spi_reset(tty_spi_dev);

	ret = request_irq(gpio_to_irq(tty_spi_dev->gpio.mdm_rts),
			  spi_mdm_rts_interrupt,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, DRVNAME,
			  (void *)tty_spi_dev);
	if (ret) {
		dev_err(&spi->dev, "Unable to get irq %x",
			gpio_to_irq(tty_spi_dev->gpio.mdm_rts));
		goto error_ret7;
	}
	/* handle case that modem is already signaling SRDY */
	/* no outgoing tty open at this point, this just satisfies the
	 * modem's read and should reset communication properly
	 */
	dbg_print("Exit\n");

	return 0;

error_ret7:
	free_irq(gpio_to_irq(tty_spi_dev->gpio.mdm_rts), (void *)tty_spi_dev);
	gpio_free(tty_spi_dev->gpio.ap_alive);
error_ret6:
	free_irq(gpio_to_irq(tty_spi_dev->gpio.mdm_rdy), (void *)tty_spi_dev);
	gpio_free(tty_spi_dev->gpio.mdm_alive);
error_ret5:
	gpio_free(tty_spi_dev->gpio.mdm_rts);
error_ret4:
	gpio_free(tty_spi_dev->gpio.mdm_rdy);
error_ret3:
	gpio_free(tty_spi_dev->gpio.ap_rts);
error_ret2:
	gpio_free(tty_spi_dev->gpio.ap_rdy);
error_ret:
	/* serial_spi_free_device(tty_spi_dev); */
	serial_spi_spi_remove(tty_spi_dev);
	saved_tty_spi_dev = NULL;
	return ret;
}

/**
 *	serial_spi_init		-	module entry point
 *
 *	Initialise the SPI and tty interfaces for the ISPI driver
 *	We need to initialize upper-edge spi driver after the tty
 *	driver because otherwise the spi probe will race
 */

static int __init serial_spi_init(void)
{
	int result;
	struct spi_master *master;
	dbg_print("serial_spi_init  Enter\n");
	tty_spi_drv = alloc_tty_driver(1);
	dbg_print("serial_spi_init tty_spi_drv pointer 0x%x\n",
			tty_spi_drv);
	if (!tty_spi_drv) {
		pr_err("%s: alloc_tty_driver failed", DRVNAME);
		return -ENOMEM;
	}

	tty_spi_drv->magic = TTY_DRIVER_MAGIC;
	tty_spi_drv->owner = THIS_MODULE;
	tty_spi_drv->driver_name = DRVNAME;
	tty_spi_drv->name = TTYNAME;
	tty_spi_drv->minor_start = TTY_SPI_TTY_ID;
	tty_spi_drv->num = 1;
	tty_spi_drv->type = TTY_DRIVER_TYPE_SERIAL;
	tty_spi_drv->subtype = SERIAL_TYPE_NORMAL;
	tty_spi_drv->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;
	tty_spi_drv->init_termios = tty_std_termios;

	tty_set_operations(tty_spi_drv, &tty_spi_serial_ops);
	result = tty_register_driver(tty_spi_drv);
	if (result) {
		pr_err("%s: tty_register_driver failed(%d)", DRVNAME, result);
		put_tty_driver(tty_spi_drv);
		return result;
	}
	master = spi_busnum_to_master(0);
	dbg_print("serial_spi_init spi_master pointer 0x%x\n", master);
	/*dbg_print("serial spi master = %p\n",master);*/
	result = serial_spi_device_init(master);
	if (result) {
		pr_err("%s: spi_allocate device failed(%d)", DRVNAME, result);
		goto exit_spi;
	}
	return result;
exit_spi:
	tty_unregister_device(tty_spi_drv, TTY_SPI_TTY_ID);
	tty_unregister_driver(tty_spi_drv);
	return result;
}

/* module_init(serial_spi_init);
module_exit(serial_spi_exit); */
late_initcall(serial_spi_init);
MODULE_LICENSE("GPL");
