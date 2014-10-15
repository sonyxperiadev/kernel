/*******************************************************************************
 * Copyright 2012 Broadcom Corporation.  All rights reserved.
 *
 * @file   drivers/tty/serial/8250/8250_brcm_bt.c
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a
 * license other than the GPL, without Broadcom's express prior written consent.
 * *******************************************************************************/

#include <linux/serial_8250.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <linux/tty_flip.h>
#include "8250.h"
#include "8250_dw.h"

#ifdef CONFIG_BRCM_UART_CHANGES
static unsigned char
bt_serial8250_rx_chars(struct uart_8250_port *up, unsigned char lsr)
{
	struct uart_port *port = &up->port;
	unsigned char ch;
	int max_count = 256;
	char flag;
	int afe_status = 0;
	int n_fifo;
	int mcr;

	/* Handle port gets called from either the interrupt context
	 * _OR_ from the timeout thread context (serial8250_timeout).
	 *
	 * Now first using qos APIs prevent the shutting down of UART clocks.
	 * for RX context. For TX we can determine when to enable and disable
	 * going to and coming out of retention. But for Rx, we need to have
	 * a timer. Once a RX happens we restart the timer to expire after
	 * say 'n' milliseconds. So when the timer function gets executed
	 * we know that 'n' milliseconds has expired without any RX activity
	 * so release the UART Rx context to go to retention.
	 *
	 * Now for from the tx perspective we'll enable the system to go to
	 * retention
	 * i.e release the clocks when the tx circular buffer is empty and
	 * the TX FIFO + THR is also empty.
	 *
	 * From the Power Mgr perspective if from both the TX and RX context
	 * if we have released the clocks then this block will be put into
	 * retention. Note that the aggregation is done by the Power
	 * Management code. */
	pi_mgr_qos_request_update(&up->qos_rx_node,0);

	/* Some RX activity has happened either byte received _OR_ some RX
	 * error, whatever may be the case we have requested the PI MGR not
	 * to go to retention, so start the timer immediately. If there are
	 * no more Rx activity for another RX_SHUTOFF_DELAY_MSECS, then this
	 * timer would come and call the pi mgr API to release the RX context
	 * */
	mod_timer(&up->rx_shutoff_timer,
			jiffies + msecs_to_jiffies(RX_SHUTOFF_DELAY_MSECS));

	do {
		n_fifo = port->serial_in(port,UART_RX_FIFO_LEVEL);
		/* Jira-1744 UART Line Status Register's Data Ready bit is not
		 * updated sometime when data is received in fifo and a Rx
		 * interrupt is generated.
		 * UART RX interrupt happened but in LSR Data Ready is not set.
		 * This fix is to take the decission based on iir also. */
		if (likely((lsr & UART_LSR_DR) ||
					(up->iir & UART_IIR_TIME_OUT)))
			ch = port->serial_in(port, UART_RX);
		else
			ch = 0;

		flag = TTY_NORMAL;
		up->port.icount.rx++;

		lsr |= up->lsr_saved_flags;
		up->lsr_saved_flags = 0;

		if (unlikely(lsr & UART_LSR_BRK_ERROR_BITS)) {
			if (lsr & UART_LSR_BI) {
				lsr &= ~(UART_LSR_FE | UART_LSR_PE);
				up->port.icount.brk++;

				/* We do the SysRQ and SAK checking
				 * here because otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask. */
				if (uart_handle_break(&up->port))
					goto ignore_char;
			}
			else if (lsr & UART_LSR_PE)
				up->port.icount.parity++;
			else if (lsr & UART_LSR_FE)
				up->port.icount.frame++;
			if (lsr & UART_LSR_OE)
				up->port.icount.overrun++;

			/* Mask off conditions which should be ignored.*/
			lsr &= up->port.read_status_mask;

			if (lsr & UART_LSR_BI)
				flag = TTY_BREAK;
			else if (lsr & UART_LSR_PE)
				flag = TTY_PARITY;
			else if (lsr & UART_LSR_FE)
				flag = TTY_FRAME;
		}
		if(n_fifo == 0)
			goto ignore_char;
		uart_insert_char(&up->port, lsr, UART_LSR_OE, ch, flag);
ignore_char:
		lsr = port->serial_in(port, UART_LSR);
	} while ((lsr & (UART_LSR_DR | UART_LSR_BI)) && (max_count-- > 0));

	spin_unlock(&up->port.lock);
	tty_flip_buffer_push(&port->state->port);
	spin_lock(&up->port.lock);
	return lsr;
}

static int bt_serial8250_handle_irq(struct uart_port *port, unsigned int iir)
{
	unsigned char status;
	unsigned long flags;
	struct uart_8250_port *up =
		container_of(port, struct uart_8250_port, port);

	if (iir & UART_IIR_NO_INT)
		return 0;

	spin_lock_irqsave(&port->lock, flags);

	status = port->serial_in(port, UART_LSR);

	/* Jira-1744 UART Line Status Register's Data Ready bit is not
	* updated sometime when data is received in fifo and a R
	* interrupt is generated.
	* UART RX interrupt happened but in LSR Data Ready is not set.
	* This fix is to take the decission based on iir also. */
	if ((iir & UART_IIR_RDI) || (status & (UART_LSR_DR | UART_LSR_BI))) {
#if defined(CONFIG_HAS_WAKELOCK)
		wake_lock_timeout(&up->uart_lock,
				msecs_to_jiffies(WAKELOCK_TIMEOUT_VAL));
#endif /* CONFIG_HAS_WAKELOCK */
		status = bt_serial8250_rx_chars(up, status);
	}

	serial8250_modem_status(up);

	/* Note that from the serial8250_tx_chars we are NOT disabling the TX
	* interrupt when the circular buffer becomes empty. So we will get
	* a TX over interrupt once the FIFO is flushed. In that case, go
	* and stop the Tx and disable the clocks so that the CCU can go to
	* retention. */
	if ((status & (UART_LSR_TEMT | UART_LSR_THRE)) &&
			uart_circ_empty(&up->port.state->xmit)) {
		if (up->ier & UART_IER_THRI) {
			up->ier &= ~UART_IER_THRI;
			port->serial_out(port, UART_IER, up->ier);
		}
		pi_mgr_qos_request_update(&up->qos_tx_node, PI_MGR_QOS_DEFAULT_VALUE);
	} else if (status & UART_LSR_THRE)
		serial8250_tx_chars(up);

	spin_unlock_irqrestore(&port->lock, flags);
	return 1;
}

int bt_dw8250_handle_irq(struct uart_port *port)
{
	struct dw8250_data *d = port->private_data;
	unsigned int iir = port->serial_in(port, UART_IIR);
#ifdef CONFIG_BRCM_UART_CHANGES
	struct uart_8250_port *up =
		container_of(port, struct uart_8250_port, port);

	/* This is used for checking the Timeout interrupt in
	 * serial8250_rx_chars() */
	up->iir = iir;
#endif

	if (bt_serial8250_handle_irq(port, iir)) {
		return 1;
	} else if ((iir & UART_IIR_BUSY) == UART_IIR_BUSY) {
		/* Clear the USR and write the LCR again. */
		(void)port->serial_in(port, UART_USR);
#ifdef CONFIG_BRCM_UART_CHANGES
		/* Stop writing to LCR if the value is same. */
		if (port->serial_in(port, UART_LCR) != d->last_lcr)
			port->serial_out(port, UART_LCR, d->last_lcr);
#endif
		return 1;
	}
	return 0;
}

EXPORT_SYMBOL(bt_dw8250_handle_irq);
#endif
