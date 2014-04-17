/*
 *  linux/include/linux/serial_8250.h
 *
 *  Copyright (C) 2004 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _LINUX_SERIAL_8250_H
#define _LINUX_SERIAL_8250_H

#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <plat/pi_mgr.h>

/*
 * This is the platform device platform_data structure
 */
struct plat_serial8250_port {
	unsigned long	iobase;		/* io base address */
	void __iomem	*membase;	/* ioremap cookie or NULL */
	resource_size_t	mapbase;	/* resource base */
	unsigned int	irq;		/* interrupt number */
	unsigned long	irqflags;	/* request_irq flags */
	unsigned int	uartclk;	/* UART clock rate */
	void            *private_data;
	unsigned char	regshift;	/* register shift */
	unsigned char	iotype;		/* UPIO_* */
	unsigned char	hub6;
	upf_t		flags;		/* UPF_* flags */
	unsigned int	type;		/* If UPF_FIXED_TYPE */
	unsigned int	(*serial_in)(struct uart_port *, int);
	void		(*serial_out)(struct uart_port *, int, int);
	void		(*set_termios)(struct uart_port *,
			               struct ktermios *new,
			               struct ktermios *old);
	int		(*handle_irq)(struct uart_port *);
	void		(*pm)(struct uart_port *, unsigned int state,
			      unsigned old);
	const unsigned char * clk_name;
	const unsigned char * port_name;
	void		(*handle_break)(struct uart_port *);
};

/*
 * Allocate 8250 platform device IDs.  Nothing is implied by
 * the numbering here, except for the legacy entry being -1.
 */
enum {
	PLAT8250_DEV_LEGACY = -1,
	PLAT8250_DEV_PLATFORM,
	PLAT8250_DEV_PLATFORM1,
	PLAT8250_DEV_PLATFORM2,
	PLAT8250_DEV_FOURPORT,
	PLAT8250_DEV_ACCENT,
	PLAT8250_DEV_BOCA,
	PLAT8250_DEV_EXAR_ST16C554,
	PLAT8250_DEV_HUB6,
	PLAT8250_DEV_AU1X00,
	PLAT8250_DEV_SM501,
};

struct uart_8250_dma;

/*
 * This should be used by drivers which want to register
 * their own 8250 ports without registering their own
 * platform device.  Using these will make your driver
 * dependent on the 8250 driver.
 */

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

	struct uart_8250_dma	*dma;

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
	unsigned int iir;
#else /* CONFIG_BRCM_UART_CHANGES not defined */
	/* Apart from Rhea platforms (i.e island for now) these calls are stubs */
	void *qos_tx_node;
	void *qos_rx_node;
#define pi_mgr_qos_add_request(a,b,c,d) (0)
#define pi_mgr_qos_request_update(a,b)
#define PI_MGR_QOS_DEFAULT_VALUE 0
#define PI_MGR_PI_ID_ARM_SUB_SYSTEM 1
#endif /* CONFIG_BRCM_UART_CHANGES */

	/* 8250 specific callbacks */
	int			(*dl_read)(struct uart_8250_port *);
	void			(*dl_write)(struct uart_8250_port *, int);
};

int serial8250_register_8250_port(struct uart_8250_port *);
void serial8250_unregister_port(int line);
void serial8250_suspend_port(int line);
void serial8250_resume_port(int line);

extern int early_serial_setup(struct uart_port *port);

extern int serial8250_find_port(struct uart_port *p);
extern int serial8250_find_port_for_earlycon(void);
extern unsigned int serial8250_early_in(struct uart_port *port, int offset);
extern void serial8250_early_out(struct uart_port *port, int offset, int value);
extern int setup_early_serial8250_console(char *cmdline);
extern void serial8250_do_set_termios(struct uart_port *port,
		struct ktermios *termios, struct ktermios *old);
extern void serial8250_do_pm(struct uart_port *port, unsigned int state,
			     unsigned int oldstate);
extern int fsl8250_handle_irq(struct uart_port *port);
int serial8250_handle_irq(struct uart_port *port, unsigned int iir);
unsigned char serial8250_rx_chars(struct uart_8250_port *up, unsigned char lsr);
void serial8250_tx_chars(struct uart_8250_port *up);
unsigned int serial8250_modem_status(struct uart_8250_port *up);

extern void serial8250_set_isa_configurator(void (*v)
					(int port, struct uart_port *up,
						unsigned short *capabilities));

#endif
