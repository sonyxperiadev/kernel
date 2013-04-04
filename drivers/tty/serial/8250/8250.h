/*
 *  Driver for 8250/16550-type serial ports
 *
 *  Based on drivers/char/serial.c, by Linus Torvalds, Theodore Ts'o.
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/serial_8250.h>

#ifdef CONFIG_BRCM_UART_CHANGES
#if defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif /* CONFIG_HAS_WAKELOCK */

#include <linux/clk.h>

#ifdef CONFIG_KONA_PI_MGR
#include <mach/pi_mgr.h>
#include <plat/pi_mgr.h>
#endif /* CONFIG_KONA_PI_MGR */

#define UART_USR                (0x1F) /* UART status register */
#define UART_TX_FIFO_LEVEL      (0x20) /* UART Transmit FIFO level register */
#define UART_RX_FIFO_LEVEL      (0x21) /* UART Receive FIFO level register */
#define UART_HALT_TX            (0x29) /* UART Halt Tx register */
#define UART_CONFIG_ID          (0x3D) /* UART Configuration ID register */
#define UART_COMPONENT_VER      (0x3E) /* UART Component Version register */
#define UART_PERIPHERAL_ID      (0x3F) /* UART Peripheral PID register */
#define UART_CONFIG             (0x40) /* UART Configuration register */
#define UART_IRCR               (0x42) /* UART IrDA Configuration register */
#define UART_UBABCSR            (0x44) /* UART UBABCSR Auto Baud Detection
					  Control and State Register */
#define UART_UBABCNTR           (0x45) /* UART UBABCNTR Auto Baud Detection
					  Control and State Register */
#define UART_IIR_TIME_OUT       (0x0C) /* Timeout indication interrupt */
#define UART_USR_TFE            (0x04) /* Tx Fifo Empty Bit */

#endif /* CONFIG_BRCM_UART_CHANGES */

struct uart_8250_port {
	struct uart_port	port;
	struct timer_list	timer;		/* "no irq" timer */
	struct list_head	list;		/* ports on this IRQ */
	unsigned short		capabilities;	/* port capabilities */
	unsigned short		bugs;		/* port bugs */
	unsigned int		tx_loadsz;	/* transmit fifo load size */
	unsigned char		acr;
	unsigned char		ier;
	unsigned char		lcr;
	unsigned char		mcr;
	unsigned char		mcr_mask;	/* mask of user bits */
	unsigned char		mcr_force;	/* mask of forced bits */
	unsigned char		cur_iotype;	/* Running I/O type */

	/*
	 * Some bits in registers are cleared on a read, so they must
	 * be saved whenever the register is read but the bits will not
	 * be immediately processed.
	 */
#define LSR_SAVE_FLAGS UART_LSR_BRK_ERROR_BITS
	unsigned char		lsr_saved_flags;
#define MSR_SAVE_FLAGS UART_MSR_ANY_DELTA
	unsigned char		msr_saved_flags;
#ifdef CONFIG_BRCM_UART_CHANGES
#if defined(CONFIG_HAS_WAKELOCK)
	struct wake_lock uart_lock;
#define WAKELOCK_TIMEOUT_VAL CONFIG_BRCM_UART_WAKELOCK_TIMEOUT
#endif
	/*
	 * Kona PM - QOS service
 	 */
	struct timer_list	rx_shutoff_timer;
#define RX_SHUTOFF_DELAY_MSECS	3000

#ifdef CONFIG_KONA_PI_MGR
	struct pi_mgr_qos_node qos_tx_node;
	struct pi_mgr_qos_node qos_rx_node;
#else
	void *qos_tx_node;
	void *qos_rx_node;
	/*
	 * Stubs - If KONA_PI_MGR is not defined let the functions be dummy
	 * otherwise the code looks ugly with too many #ifdefs
	 */
#define pi_mgr_qos_add_request(a,b,c,d) (0)
#define pi_mgr_qos_request_update(a,b)
#define PI_MGR_QOS_DEFAULT_VALUE 0
#define PI_MGR_PI_ID_ARM_SUB_SYSTEM 1
#endif /* KONA_PI_MGR not defined */
#else /* CONFIG_BRCM_UART_CHANGES not defined */
	/* Apart from Rhea platforms (i.e island for now) these calls are stubs */
	void *qos_tx_node;
	void *qos_rx_node;
#define pi_mgr_qos_add_request(a,b,c,d) (0)
#define pi_mgr_qos_request_update(a,b)
#define PI_MGR_QOS_DEFAULT_VALUE 0
#define PI_MGR_PI_ID_ARM_SUB_SYSTEM 1
#endif /* CONFIG_BRCM_UART_CHANGES */
	unsigned int iir;
};

struct old_serial_port {
	unsigned int uart;
	unsigned int baud_base;
	unsigned int port;
	unsigned int irq;
	unsigned int flags;
	unsigned char hub6;
	unsigned char io_type;
	unsigned char *iomem_base;
	unsigned short iomem_reg_shift;
	unsigned long irqflags;
};

/*
 * This replaces serial_uart_config in include/linux/serial.h
 */
struct serial8250_config {
	const char	*name;
	unsigned short	fifo_size;
	unsigned short	tx_loadsz;
	unsigned char	fcr;
	unsigned int	flags;
};

#define UART_CAP_FIFO	(1 << 8)	/* UART has FIFO */
#define UART_CAP_EFR	(1 << 9)	/* UART has EFR */
#define UART_CAP_SLEEP	(1 << 10)	/* UART has IER sleep */
#define UART_CAP_AFE	(1 << 11)	/* MCR-based hw flow control */
#define UART_CAP_UUE	(1 << 12)	/* UART needs IER bit 6 set (Xscale) */
#define UART_CAP_RTOIE	(1 << 13)	/* UART needs IER bit 4 set (Xscale, Tegra) */

#define UART_BUG_QUOT	(1 << 0)	/* UART has buggy quot LSB */
#define UART_BUG_TXEN	(1 << 1)	/* UART has buggy TX IIR status */
#define UART_BUG_NOMSR	(1 << 2)	/* UART has buggy MSR status bits (Au1x00) */
#define UART_BUG_THRE	(1 << 3)	/* UART has buggy THRE reassertion */

#define PROBE_RSA	(1 << 0)
#define PROBE_ANY	(~0)

#define HIGH_BITS_OFFSET ((sizeof(long)-sizeof(int))*8)

#ifdef CONFIG_SERIAL_8250_SHARE_IRQ
#define SERIAL8250_SHARE_IRQS 1
#else
#define SERIAL8250_SHARE_IRQS 0
#endif

static inline int serial_in(struct uart_8250_port *up, int offset)
{
	return up->port.serial_in(&up->port, offset);
}

static inline void serial_out(struct uart_8250_port *up, int offset, int value)
{
	up->port.serial_out(&up->port, offset, value);
}

#if defined(__alpha__) && !defined(CONFIG_PCI)
/*
 * Digital did something really horribly wrong with the OUT1 and OUT2
 * lines on at least some ALPHA's.  The failure mode is that if either
 * is cleared, the machine locks up with endless interrupts.
 */
#define ALPHA_KLUDGE_MCR  (UART_MCR_OUT2 | UART_MCR_OUT1)
#elif defined(CONFIG_SBC8560)
/*
 * WindRiver did something similarly broken on their SBC8560 board. The
 * UART tristates its IRQ output while OUT2 is clear, but they pulled
 * the interrupt line _up_ instead of down, so if we register the IRQ
 * while the UART is in that state, we die in an IRQ storm. */
#define ALPHA_KLUDGE_MCR (UART_MCR_OUT2)
#else
#define ALPHA_KLUDGE_MCR 0
#endif
