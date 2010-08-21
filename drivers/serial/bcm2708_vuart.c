/*
 *  linux/drivers/serial/bc2708.c
 *
 *  Copyright (C) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This provides a "virtual" UART port that uses an area of shared memory
 * shared with a broadcom communications processor.
 */

#if defined(CONFIG_SERIAL_BCM_VUART_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/console.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <asm/io.h>

#include <mach/vcio.h>

#include "bcm2708_vuart.h"

#define DRIVER_NAME "bcm2708_vuart"

#define SERIAL_BCMVUART_MAJOR   207          // stolen - really belongs to IMX
#define MINOR_START	        24           // stolen? - really belongs to IMX
#define DEV_NAME		"ttybcv"     

#ifndef EOF
#define EOF (-1)
#endif

/* ----------------------------------------------------------------------
 *      Circular Buffer
 * -------------------------------------------------------------------- */

extern int /*char or EOF*/
cbuf_rd_poll(volatile CBUF_T *cb)
{
   int ch = EOF;
   const unsigned size_ln2 = cb->size_ln2; // never changes
   unsigned char *buf = (unsigned char *)&cb->buf[0];       // never changes
   unsigned in_count = cb->in_count;       // snapshot - only changed by them
   unsigned out_count = cb->out_count;     // snapshot - only changed by us
   
   if (in_count > out_count)
   {
      ch = buf[out_count & ((1<<size_ln2) - 1)];
      cb->out_count = out_count+1; // no write contention - only we update this
   }

   return ch;
}


// (make sure this comparison will be correct even when one pointer has
// wrapped around and the other hasn't)
#define CBUF_WR_FULL(out, in, size) \
        ((int)((unsigned)((out) + (size)) - (in)) <= 0)



extern int /*bool*/
cbuf_wr_poll(volatile CBUF_T *cb, char ch)
{
   const unsigned size_ln2 = cb->size_ln2;    // never changes
   volatile unsigned char *buf = &cb->buf[0]; // never changes
   unsigned in_count = cb->in_count;          // snapshot - only changed by us
   unsigned out_count = cb->out_count;        // snapshot - only changed by them

   // (make sure this comparison will be correct even when one pointer has
   // wrapped around and the other hasn't)
   if (!CBUF_WR_FULL(out_count, in_count, 1<<size_ln2))
   {
      buf[in_count & ((1<<size_ln2) - 1)] = ch;
      cb->in_count = in_count+1; // no write contention - only we update this
      return 1; // written
   } else
      return 0; // not written
}


static inline void
cbuf_init(volatile CBUF_T *cb)
{
   cb->in_count = 0;
   cb->out_count = 0;
   *(unsigned *)&cb->size_ln2 = CBUF_LEN_LN2;
}


static inline int /*bool*/
cbuf_empty(volatile CBUF_T *cb)
{
   unsigned in_count = cb->in_count;       // snapshot - only changed by us
   unsigned out_count = cb->out_count;     // snapshot - only changed by them

   return in_count == out_count;
}


static inline int /*bool*/
cbuf_full(volatile CBUF_T *cb)
{
   const unsigned size_ln2 = cb->size_ln2; // never changes
   unsigned in_count = cb->in_count;       // snapshot - only changed by us
   unsigned out_count = cb->out_count;     // snapshot - only changed by them

   return CBUF_WR_FULL(out_count, in_count, 1<<size_ln2);
}


/* ----------------------------------------------------------------------
 *      Virtual UART
 * -------------------------------------------------------------------- */


static inline void vuart_init(volatile VUART_T *vuart)
{
   cbuf_init(&vuart->to_host);
   cbuf_init(&vuart->from_host);
   vuart->sentinal = VUART_ID;
}

static inline int /*bool*/ vuart_tx_empty(volatile VUART_T *vuart)
{
   return cbuf_empty(&vuart->to_host);
}

static inline int /*bool*/ vuart_tx_full(volatile VUART_T *vuart)
{
   return cbuf_full(&vuart->to_host);
}

static inline int /*bool*/ vuart_tx_poll(volatile VUART_T *vuart, int ch)
{
   return cbuf_wr_poll(&vuart->to_host, ch);
}

static inline void vuart_transmit_buffer(volatile VUART_T *vuart,
                                         struct uart_port *port)
{
        struct circ_buf *xmit = &port->state->xmit;
        
        while (vuart_tx_poll(vuart, xmit->buf[xmit->tail])) {
		/* sent xmit->buf[xmit->tail] to the port */
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		port->icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	}

        //CGG
        //printk(KERN_INFO DRIVER_NAME ": tx'd %d chars",
        //       port->icount.tx - txin);
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

        // not really supposed to do this here, but there is a 30s delay
        // waiting for us if we don't send everything
        while (!vuart_tx_empty(vuart))
                msleep_interruptible(1/*ms*/);
        
	//if (uart_circ_empty(xmit))
	//	bcm_vuart_stop_tx(port);
}


/* ----------------------------------------------------------------------
 *      UART Device
 * -------------------------------------------------------------------- */


struct bcm_vuart_port {
	struct uart_port	port;
        volatile VUART_T       *vuart;
};


static struct bcm_vuart_port *bcm_console_port;


/*
 * Return TIOCSER_TEMT when transmitter is not busy.
 */
static unsigned int bcm_vuart_tx_empty(struct uart_port *port)
{
	struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        unsigned empty = vuart_tx_empty(sport->vuart);
        //if (!empty) //CGG
        //   printk("F[%x,%x]", sport->vuart->to_host.in_count,
        //          sport->vuart->to_host.out_count);
	return empty? TIOCSER_TEMT: 0;
}

/*
 * Return status of the modem control lines
 */
static unsigned int bcm_vuart_get_mctrl(struct uart_port *port)
{
        // struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
	unsigned int tmp = TIOCM_DSR | TIOCM_CAR;

	tmp |= TIOCM_CTS;
	tmp |= TIOCM_RTS;

	return tmp;
}

/*
 * Set the modem control lines
 */
static void bcm_vuart_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
        // struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        // don't have modem - does nothing
}

static void bcm_vuart_stop_tx(struct uart_port *port)
{
	// struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        // we cant - do nothing
}

/*
 * Transmit accumulated buffer of characters
 */
static void bcm_vuart_start_tx(struct uart_port *port)
{
	struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;

        // wait for old transmit to complete
        // (we shouldn't really do this here, but something causes huge
        //  delays elsewhere if we don't)
        while (vuart_tx_full(sport->vuart))
                msleep_interruptible(1/*ms*/);
        
	if (!vuart_tx_full(sport->vuart))
                vuart_transmit_buffer(sport->vuart, &sport->port);
        //CGG
        //else
        //   printk("T[%x,%x]", sport->vuart->to_host.in_count,
        //          sport->vuart->to_host.out_count);
}

/*
 * Signal that further reception is unwelcome
 */
static void bcm_vuart_stop_rx(struct uart_port *port)
{
	// struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        // we can't - do nothing
}

/*
 * Enable modem status interrupts
 */
static void bcm_vuart_enable_ms(struct uart_port *port)
{
	// struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        // we don't have any interrupts - do nothing
}

/*
 * Set the "sending break" state according to break_state
 */
static void bcm_vuart_break_ctl(struct uart_port *port, int break_state)
{
	// struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        // we don't have a break signal - do nothing
}

static int bcm_vuart_startup(struct uart_port *port)
{
	// struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        // no setting up necessary
	return 0;
}

static void bcm_vuart_shutdown(struct uart_port *port)
{
	// struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
        // nothing to shut down - do nothing
}

static void
bcm_vuart_set_termios(struct uart_port *port, struct ktermios *termios,
		      struct ktermios *old)
{
	struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
	unsigned long flags;
	unsigned int old_csize = old ? old->c_cflag & CSIZE : CS8;

	/*
	 * If we don't support modem control lines, don't allow
	 * these to be set.
	 */
	if (1) {
		termios->c_cflag &= ~(HUPCL | CRTSCTS | CMSPAR);
		termios->c_cflag |= CLOCAL;
	}

	/*
	 * We only support CS7 and CS8.
	 */
	while ((termios->c_cflag & CSIZE) != CS7 &&
	       (termios->c_cflag & CSIZE) != CS8) {
		termios->c_cflag &= ~CSIZE;
		termios->c_cflag |= old_csize;
		old_csize = CS8;
	}

	spin_lock_irqsave(&sport->port.lock, flags);

	sport->port.read_status_mask = 0;

	/*
	 * Characters to ignore
	 */
	sport->port.ignore_status_mask = 0;
        
	spin_unlock_irqrestore(&sport->port.lock, flags);
}

static const char *bcm_vuart_type(struct uart_port *port)
{
	struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;

	return sport->port.type == PORT_BCMVUART ? "BCMVUART" : NULL;
}

/*
 * Release the memory region(s) being used by 'port'.
 */
static void bcm_vuart_release_port(struct uart_port *port)
{
	struct platform_device *pdev = to_platform_device(port->dev);
	struct resource *mmres;

	mmres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(mmres->start, mmres->end - mmres->start + 1);
}

/*
 * Request the memory region(s) being used by 'port'.
 */
static int bcm_vuart_request_port(struct uart_port *port)
{
	struct platform_device *pdev = to_platform_device(port->dev);
	struct resource *mmres;
	void *ret;

	mmres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mmres)
		return -ENODEV;

	ret = request_mem_region(mmres->start, mmres->end - mmres->start + 1,
                                 "uart-bcmvuart");
	return  NULL != ret? 0: -EBUSY;
}

/*
 * Configure/autoconfigure the port.
 */
static void bcm_vuart_config_port(struct uart_port *port, int flags)
{
	struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;

	if (flags & UART_CONFIG_TYPE &&
	    bcm_vuart_request_port(&sport->port) == 0)
		sport->port.type = PORT_BCMVUART;
}

/*
 * Verify the new serial_struct (for TIOCSSERIAL).
 * The only change we allow are to the flags and type, and
 * even then only between PORT_BCMVUART and PORT_UNKNOWN
 */
static int
bcm_vuart_verify_port(struct uart_port *port, struct serial_struct *ser)
{
        // struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;
	int ret = 0;

        // Try to remove excessive waits when the transmission blocks
        ser->close_delay = 1;   /* .001 sec */
        ser->closing_wait = 10; /* .010 sec */

	if (ser->type != PORT_UNKNOWN && ser->type != PORT_BCMVUART) {
                printk(KERN_ERR DRIVER_NAME ": serial port type %d is "
                       "invalid\n", ser->type);
		ret = -EINVAL;
        }
	return ret;
}



static struct uart_ops bcm_vuart_pops = {
	.tx_empty	= bcm_vuart_tx_empty,
	.set_mctrl	= bcm_vuart_set_mctrl,
	.get_mctrl	= bcm_vuart_get_mctrl,
	.stop_tx	= bcm_vuart_stop_tx,
	.start_tx	= bcm_vuart_start_tx,
	.stop_rx	= bcm_vuart_stop_rx,
	.enable_ms	= bcm_vuart_enable_ms,
	.break_ctl	= bcm_vuart_break_ctl,
	.startup	= bcm_vuart_startup,
	.shutdown	= bcm_vuart_shutdown,
	.set_termios	= bcm_vuart_set_termios,
	.type		= bcm_vuart_type,
	.release_port	= bcm_vuart_release_port,
	.request_port	= bcm_vuart_request_port,
	.config_port	= bcm_vuart_config_port,
	.verify_port	= bcm_vuart_verify_port,
};



/* ----------------------------------------------------------------------
 *      Console Device
 * -------------------------------------------------------------------- */


#ifdef CONFIG_SERIAL_BCM_VUART_CONSOLE

static void bcm_console_putchar(struct uart_port *port, int ch)
{
	struct bcm_vuart_port *sport = (struct bcm_vuart_port *)port;

	while (vuart_tx_full(sport->vuart))
		barrier();
        vuart_tx_poll(sport->vuart, ch);
}

/*
 * Interrupts are disabled on entering
 */
static void
bcm_console_write(struct console *co, const char *s, unsigned int count)
{
        struct bcm_vuart_port *sport = bcm_console_port;

	uart_console_write(&sport->port, s, count, bcm_console_putchar);

	/*
	 *	Finally, wait for transmitter to become empty
	 */
	while (!vuart_tx_empty(sport->vuart))
                continue;
}

static int __init
bcm_console_setup(struct console *co, char *options)
{
        int ret;
	struct bcm_vuart_port *sport;
	int baud = 921600;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	/*
	 * Check whether an invalid uart number has been specified, and
	 * if so, provide the first available port that does have
	 * console support.
	 */
	if (co->index == -1 || co->index >= 1)
		co->index = 0;
	if (co->index != 0)
		ret = -ENODEV;

        else {
                sport = bcm_console_port;
                
                if (NULL != options)
                        uart_parse_options(options,
                                           &baud, &parity, &bits, &flow);
                // else // we can't set these - do nothing

                ret = uart_set_options(&sport->port,
                                       co, baud, parity, bits, flow);
        }

        printk(KERN_INFO DRIVER_NAME ": registered console - rc %d\n", ret);
        
        return ret;
}



static struct uart_driver bcmuart_reg;

static struct console bcm_console = {
	.name		= DEV_NAME,
	.write		= &bcm_console_write,
	.device		= &uart_console_device,
	.setup		= &bcm_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &bcmuart_reg,
};

#define BCM_CONSOLE	&bcm_console

#else
#define BCM_CONSOLE	NULL
#endif

/* ----------------------------------------------------------------------
 *      Platform Device
 * -------------------------------------------------------------------- */


static struct uart_driver bcmuart_reg = {
	.owner          = THIS_MODULE,
	.driver_name    = DRIVER_NAME,
	.dev_name       = DEV_NAME,
	.major          = SERIAL_BCMVUART_MAJOR,
	.minor          = MINOR_START,
	.nr             = 1,
	.cons           = BCM_CONSOLE,
};

static int bcm_vuart_suspend(struct platform_device *dev, pm_message_t state)
{
	struct bcm_vuart_port *sport = platform_get_drvdata(dev);

	if (sport)
		uart_suspend_port(&bcmuart_reg, &sport->port);

	return 0;
}

static int bcm_vuart_resume(struct platform_device *dev)
{
	struct bcm_vuart_port *sport = platform_get_drvdata(dev);

	if (sport)
		uart_resume_port(&bcmuart_reg, &sport->port);

	return 0;
}


static int bcm_vuart_register(struct device *dev, volatile VUART_T **out_vuart)
{
        int ret = 0;
        dma_addr_t dma = 0;
        void *vuart_mem =
           // dma_alloc_writecombine(dev, PAGE_ALIGN(sizeof(VUART_T)),
           //                       &dma, GFP_KERNEL);
           dma_alloc_coherent(dev, PAGE_ALIGN(sizeof(VUART_T)),
                              &dma, GFP_KERNEL);

        if (NULL == vuart_mem) {
                printk(KERN_ERR DRIVER_NAME": unable to allocate "
                       "VUART buffers\n");
                ret = -ENOMEM;
        } else {
                *out_vuart = (VUART_T *)vuart_mem;
                vuart_init(*out_vuart);

                // send the DMA address for the circular buffers to
                // the coms processor through the VideoCore I/O mailbox
                bcm_mailbox_write(MBOX_CHAN_VUART, dma);
                printk(KERN_INFO DRIVER_NAME": registered virtual "
                       "UART @%08x through MBOX %p\n",
                       dma, __io_address(ARM_0_MAIL1_WRT));
        }
        return ret;
}


static int bcm_vuart_probe(struct platform_device *pdev)
{
	struct bcm_vuart_port *sport;
	int ret = 0;
        
	sport = kzalloc(sizeof(*sport), GFP_KERNEL);
	if (!sport) {
                printk(KERN_ERR DRIVER_NAME ": failed to allocate "
                       "port memory\n");
		return -ENOMEM;
        }
        ret = bcm_vuart_register(&pdev->dev, &sport->vuart);
	if (ret != 0) 
		goto free;

	sport->port.dev = &pdev->dev;
        sport->port.mapbase = 0; // not used
	sport->port.membase = NULL;
	sport->port.type = PORT_BCMVUART;
	sport->port.iotype = UPIO_MEM;
	// sport->port.irq - we don't have one
	sport->port.fifosize = 32;
	sport->port.ops = &bcm_vuart_pops;
	sport->port.flags = UPF_BOOT_AUTOCONF;
	sport->port.line = pdev->id < 0? 0: pdev->id;

	// sport->port.uartclk = clk_get_rate(sport->clk);

	bcm_console_port = sport;

	ret = uart_add_one_port(&bcmuart_reg, &sport->port);
	if (ret != 0) {
                printk(KERN_ERR DRIVER_NAME ": failed to add UART port "
                       "on line %d - rc %d\n", sport->port.line, ret);
		ret = -ENODEV;
		goto deinit;
        }
	platform_set_drvdata(pdev, &sport->port);

        printk(KERN_INFO DRIVER_NAME ": located device\n");
	return 0;

        
deinit:
	iounmap(sport->port.membase);
free:
	kfree(sport);

	return ret;
}

static int bcm_vuart_remove(struct platform_device *pdev)
{
	struct bcm_vuart_port *sport = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	if (sport) {
		uart_remove_one_port(&bcmuart_reg, &sport->port);
	}

	iounmap(sport->port.membase);
	kfree(sport);

	return 0;
}

static struct platform_driver bcm_vuart_driver = {
	.probe		= bcm_vuart_probe,
	.remove		= bcm_vuart_remove,

	.suspend	= bcm_vuart_suspend,
	.resume		= bcm_vuart_resume,
	.driver		= {
		.name	= "bcm2708_vuart",
		.owner	= THIS_MODULE,
	},
};

static int __init bcm_vuart_init(void)
{
	int ret;

	printk(KERN_INFO "Serial: Broadcom virtual UART driver\n");

	ret = uart_register_driver(&bcmuart_reg); // creates .state for .nr inst
	if (0 == ret) {
                ret = platform_driver_register(&bcm_vuart_driver);
                if (ret != 0) {
                        uart_unregister_driver(&bcmuart_reg);
                        printk(KERN_ERR DRIVER_NAME ": failed to register "
                               "on platform\n");
                } else
                        printk(KERN_INFO DRIVER_NAME ": registered UART\n");
                
                ret = 0;
        } else {
                printk(KERN_ERR DRIVER_NAME ": failed to register UART\n");
        }
        return ret;
}

static void __exit bcm_vuart_exit(void)
{
	platform_driver_unregister(&bcm_vuart_driver);
	uart_unregister_driver(&bcmuart_reg);
}

//module_init(bcm_vuart_init);
arch_initcall(bcm_vuart_init); // Initialize early
module_exit(bcm_vuart_exit);

MODULE_AUTHOR("Gray Girling");
MODULE_DESCRIPTION("Virtual UART for communications with a Broadcom services processor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:bcm-vuart");

