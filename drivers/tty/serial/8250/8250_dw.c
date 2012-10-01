/*
 * Synopsys DesignWare 8250 driver.
 *
 * Copyright 2011 Picochip, Jamie Iles.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The Synopsys DesignWare 8250 has an extra feature whereby it detects if the
 * LCR is written whilst busy.  If it is, then a busy detect interrupt is
 * raised, the LCR needs to be rewritten and the uart status register read.
 */
/*******************************************************************************
 * Copyright 2012 Broadcom Corporation.  All rights reserved.
 *
 * @file   drivers/tty/serial/8250/8250_dw.c
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/serial_8250.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "8250.h"

#ifdef CONFIG_BRCM_UART_CHANGES
extern int bt_dw8250_handle_irq(struct uart_port *p);
#endif

struct dw8250_data {
	struct clk	*clk;
	int	last_lcr;
	int	line;
};

static void dw8250_serial_out(struct uart_port *p, int offset, int value)
{
	struct dw8250_data *d = p->private_data;

	if (offset == UART_LCR)
		d->last_lcr = value;

	offset <<= p->regshift;
	writeb(value, p->membase + offset);
}

static unsigned int dw8250_serial_in(struct uart_port *p, int offset)
{
	offset <<= p->regshift;

	return readb(p->membase + offset);
}

static void dw8250_serial_out32(struct uart_port *p, int offset, int value)
{
	struct dw8250_data *d = p->private_data;

	if (offset == UART_LCR)
		d->last_lcr = value;

	offset <<= p->regshift;
	writel(value, p->membase + offset);
}

static unsigned int dw8250_serial_in32(struct uart_port *p, int offset)
{
	offset <<= p->regshift;

	return readl(p->membase + offset);
}

/* Offset for the DesignWare's UART Status Register. */
//#define UART_USR	0x1f
static int dw8250_handle_irq(struct uart_port *p)
{
	struct dw8250_data *d = p->private_data;
	unsigned int iir = p->serial_in(p, UART_IIR);


	if (serial8250_handle_irq(p, iir)) {
		return 1;
	} else if ((iir & UART_IIR_BUSY) == UART_IIR_BUSY) {
		/* Clear the USR and write the LCR again. */
		(void)p->serial_in(p, UART_USR);
		p->serial_out(p, d->last_lcr, UART_LCR);

		return 1;
	}

	return 0;
}

static int __devinit dw8250_probe(struct platform_device *pdev)
{
	struct uart_port port = {};
#ifdef CONFIG_BRCM_UART_CHANGES
	int i, irqflag = 0;
	struct dw8250_data *data;
	struct plat_serial8250_port *p = pdev->dev.platform_data;

	for (i = 0; p && p->flags != 0; p++, i++) {
		data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
		if (!data)
			return -ENOMEM;
		port.private_data = data;

		port.iobase             = p->iobase;
		port.membase            = p->membase;
		port.irq                = p->irq;
		port.irqflags           = p->irqflags;
		port.uartclk            = p->uartclk;
		port.regshift           = p->regshift;
		port.iotype             = p->iotype;
		port.flags              = p->flags;
		port.mapbase            = p->mapbase;
		port.hub6               = p->hub6;
		port.type               = p->type;
		port.serial_in          = dw8250_serial_in;
		port.serial_out         = dw8250_serial_out;
		if (!(p->handle_irq)) {
			/* Default 8250_dw handler */
			port.handle_irq = dw8250_handle_irq;
			if (p->port_name) {
				if (!strcmp(p->port_name, "bluetooth"))
					port.handle_irq = bt_dw8250_handle_irq;
			}
		}

		port.set_termios	= p->set_termios;
		port.pm                 = p->pm;
		port.dev                = &pdev->dev;
		port.irqflags           |= irqflag;

		pdev->dev.platform_data = p;

		data->clk = clk_get(port.dev, p->clk_name);
		if (IS_ERR(data->clk))
			return PTR_ERR(data->clk);

		clk_disable(data->clk);
		clk_set_rate(data->clk, port.uartclk);
		clk_enable(data->clk);

		port.uartclk = clk_get_rate(data->clk);

		data->line = serial8250_register_port(&port);
		if (data->line < 0) {
			return data->line;
		}
	}
#else
	struct resource *regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct resource *irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	struct device_node *np = pdev->dev.of_node;
	u32 val;
	struct dw8250_data *data;

	if (!regs || !irq) {
		dev_err(&pdev->dev, "no registers/irq defined\n");
		return -EINVAL;
	}

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	port.private_data = data;

	spin_lock_init(&port.lock);
	port.mapbase = regs->start;
	port.irq = irq->start;
	port.handle_irq = dw8250_handle_irq;
	port.type = PORT_8250;
	port.flags = UPF_SHARE_IRQ | UPF_BOOT_AUTOCONF | UPF_IOREMAP |
		UPF_FIXED_PORT | UPF_FIXED_TYPE;
	port.dev = &pdev->dev;

	port.iotype = UPIO_MEM;
	port.serial_in = dw8250_serial_in;
	port.serial_out = dw8250_serial_out;
	if (!of_property_read_u32(np, "reg-io-width", &val)) {
		switch (val) {
		case 1:
			break;
		case 4:
			port.iotype = UPIO_MEM32;
			port.serial_in = dw8250_serial_in32;
			port.serial_out = dw8250_serial_out32;
			break;
		default:
			dev_err(&pdev->dev, "unsupported reg-io-width (%u)\n",
				val);
			return -EINVAL;
		}
	}

	if (!of_property_read_u32(np, "reg-shift", &val))
		port.regshift = val;

	if (of_property_read_u32(np, "clock-frequency", &val)) {
		dev_err(&pdev->dev, "no clock-frequency property set\n");
		return -EINVAL;
	}
	port.uartclk = val;

	data->line = serial8250_register_port(&port);
	if (data->line < 0)
		return data->line;

	platform_set_drvdata(pdev, data);
#endif

	return 0;
}

static int __devexit dw8250_remove(struct platform_device *pdev)
{
	struct dw8250_data *data = platform_get_drvdata(pdev);

	serial8250_unregister_port(data->line);

	return 0;
}

#ifdef CONFIG_BRCM_UART_CHANGES
void serial8250_togglerts_afe(struct uart_port *port, unsigned int flowon)
{
	/* Keeping this as a dummy function.
	 * Thsi function has a dependency in drivers/bluetooth/bcm_bzhw.c
	 * Need to discuss with BT floks and remove this function.*/
}
EXPORT_SYMBOL(serial8250_togglerts_afe);
#endif

static const struct of_device_id dw8250_match[] = {
	{ .compatible = "snps,dw-apb-uart" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, dw8250_match);

static struct platform_driver dw8250_platform_driver = {
	.driver = {
#ifdef CONFIG_BRCM_UART_CHANGES
		.name		= "serial8250_dw",
#else
		.name		= "dw-apb-uart",
#endif
		.owner		= THIS_MODULE,
		.of_match_table	= dw8250_match,
	},
	.probe			= dw8250_probe,
	.remove			= __devexit_p(dw8250_remove),
};

module_platform_driver(dw8250_platform_driver);

MODULE_AUTHOR("Jamie Iles");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Synopsys DesignWare 8250 serial port driver");
