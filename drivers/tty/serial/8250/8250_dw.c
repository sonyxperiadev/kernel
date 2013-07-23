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
#include <mach/pinmux.h>
#include <linux/pm.h>
#include <linux/gpio.h>
#include "8250.h"

#define GPIO_PIN47 47
#define GPIO_PIN20 20
#define UART_RESUME 0
#define UART_SUSPEND 3

#ifdef CONFIG_DW_BT_UART_CHANGES
extern int bt_dw8250_handle_irq(struct uart_port *p);
#endif

struct dw8250_data {
	struct clk	*clk;
	int	last_lcr;
	int	line;
	/*If power-save-enable is set Change UBRTSN
	* to save floor current during deep sleep*/
	bool power_save_enable;
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
static int dw8250_handle_irq(struct uart_port *port)
{
	struct dw8250_data *d = port->private_data;
	unsigned int iir = port->serial_in(port, UART_IIR);
#ifdef CONFIG_BRCM_UART_CHANGES
	unsigned long flags;
	struct uart_8250_port *up =
		container_of(port, struct uart_8250_port, port);

	/* This is used for checking the Timeout interrupt in
	 * serial8250_rx_chars() */
	up->iir = iir;
#endif

	if (serial8250_handle_irq(port, iir)) {
		return 1;
	} else if ((iir & UART_IIR_BUSY) == UART_IIR_BUSY) {
		/* Clear the USR and write the LCR again. */
		(void)port->serial_in(port, UART_USR);
#ifdef CONFIG_BRCM_UART_CHANGES
		/* Stop writing to LCR if the value is same. */
		spin_lock_irqsave(&port->lock, flags);
		if (port->serial_in(port, UART_LCR) != d->last_lcr)
			port->serial_out(port, UART_LCR, d->last_lcr);
		spin_unlock_irqrestore(&port->lock, flags);
#else
		port->serial_out(port, UART_LCR, d->last_lcr);
#endif

		return 1;
	}

	return 0;
}

void dw8250_pm(struct uart_port *port, unsigned int state,
			      unsigned old_state)
{
	static struct pin_config uartb3_config[2] = {
		{
		.name = PN_UBRTSN,
		.func = PF_GPIO47,
		.reg.b = {
			.drv_sth = 3,
			.input_dis = 0,
			.slew_rate_ctrl = 0,
			.pull_up = 0,
			.pull_dn = 0,
			.hys_en = 0,
			},
		},
		{
		.name = PN_UBRTSN,
		.func = PF_UB3RTSN,
		.reg.b = {
			.drv_sth = 3,
			.input_dis = 0,
			.slew_rate_ctrl = 0,
			.pull_up = 1,
			.pull_dn = 0,
			.hys_en = 0,
			},
		},
	};

	static struct pin_config uartb2_config[2] = {
		{
		.name = PN_GPIO20,
		.func = PF_GPIO20,
		.reg.b = {
			.drv_sth = 3,
			.input_dis = 0,
			.slew_rate_ctrl = 0,
			.pull_up = 0,
			.pull_dn = 0,
			.hys_en = 0,
			},
		},
		{
		.name = PN_GPIO20,
		.func = PF_UB2RTSN,
		.reg.b = {
			.drv_sth = 3,
			.input_dis = 0,
			.slew_rate_ctrl = 0,
			.pull_up = 1,
			.pull_dn = 0,
			.hys_en = 0,
			},
		},
	};

	int ret = 0;
	struct dw8250_data *private_data = port->private_data;
	pr_debug("In %s port = 0x%08X state = %d old_state = %d\n",
	       __func__, (unsigned int)port, state, old_state);

	switch (state) {
	case UART_RESUME:
			/*Resume sequence*/
		if ((private_data->power_save_enable) &&
			(port->irq == BCM_INT_ID_UART2))
			pinmux_set_pin_config(&uartb3_config[1]);
		else if ((private_data->power_save_enable) &&
			(port->irq == BCM_INT_ID_UART1))
			pinmux_set_pin_config(&uartb2_config[1]);
		serial8250_do_pm(port, state, old_state);
		break;
	case UART_SUSPEND:
			/*Suspend sequence*/
		if ((private_data->power_save_enable) &&
				(port->irq == BCM_INT_ID_UART2)) {
			pinmux_set_pin_config(&uartb3_config[0]);
			ret = gpio_direction_input(GPIO_PIN47);
			if (ret)
				pr_err("UART: GPIO: direction_input failed\n");
		} else if ((private_data->power_save_enable) &&
				(port->irq == BCM_INT_ID_UART1)) {
			pinmux_set_pin_config(&uartb2_config[0]);
			ret = gpio_direction_input(GPIO_PIN20);
			if (ret)
				pr_err("UART: GPIO: direction_input failed\n");
		}
		serial8250_do_pm(port, state, old_state);
		break;
	default:
		serial8250_do_pm(port, state, old_state);
		break;
	}
}

static int __devinit dw8250_probe(struct platform_device *pdev)
{
	struct uart_port port = {};

#ifdef CONFIG_BRCM_UART_CHANGES
	int i, irqflag = 0, ret = 0;
	struct dw8250_data *data;
	struct plat_serial8250_port *p = pdev->dev.platform_data;
	struct resource *regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct resource *irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	struct device_node *np = pdev->dev.of_node;
	u32 val;
	const char *prop;

	if (p) {
		for (i = 0; p && p->flags != 0; p++, i++) {
			data = devm_kzalloc(&pdev->dev, sizeof(*data),
				GFP_KERNEL);
			if (!data)
				return -ENOMEM;
			if (p->private_data) {
				data->power_save_enable = 1;
				if (p->irq == BCM_INT_ID_UART2) {
					ret = gpio_request(GPIO_PIN47,
						"serial_pin_47");
					if (ret) {
						pr_err("UART: GPIO %d",
							GPIO_PIN47);
						pr_err("gpio_request ");
						pr_err("failed\n");
					}
				} else if (p->irq == BCM_INT_ID_UART1) {
					ret = gpio_request(GPIO_PIN20,
						"serial_pin_20");
					if (ret) {
						pr_err("UART: GPIO %d",
							GPIO_PIN20);
						pr_err("gpio_request ");
						pr_err("failed\n");
					}
				}
			}

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
#ifdef CONFIG_DW_BT_UART_CHANGES
				if (p->port_name) {
					if (!strcmp(p->port_name, "bluetooth"))
						port.handle_irq =
							bt_dw8250_handle_irq;
				}
#endif
			}

			port.set_termios	= p->set_termios;
			port.pm                 = dw8250_pm;
			port.dev                = &pdev->dev;
			port.irqflags           |= irqflag;

			pdev->dev.platform_data = p;

#ifndef CONFIG_MACH_BCM_FPGA
			data->clk = clk_get(port.dev, p->clk_name);
			if (IS_ERR(data->clk))
				return PTR_ERR(data->clk);

			clk_disable(data->clk);
			clk_set_rate(data->clk, port.uartclk);
			clk_enable(data->clk);

			port.uartclk = clk_get_rate(data->clk);
#endif

			data->line = serial8250_register_port(&port);
			if (data->line < 0) {
				return data->line;
			}
			platform_set_drvdata(pdev, data);
		}
	} else { /* Get info from DT */

#ifdef CONFIG_OF
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
#ifdef CONFIG_BRCM_UART_CHANGES
		port.membase = ioremap(regs->start, resource_size(regs));
		port.irqflags = 0;
#endif /* CONFIG_BRCM_UART_CHANGES */
		port.handle_irq = dw8250_handle_irq;
		port.type = PORT_16550A;
		port.flags = UPF_BOOT_AUTOCONF | UPF_BUG_THRE |
			UPF_SKIP_TEST | UPF_FIXED_TYPE;
		port.dev = &pdev->dev;
		port.pm = dw8250_pm;
		port.iotype = UPIO_MEM32;
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
				dev_err(&pdev->dev, "unsupported reg-io-width\
						(%u)\n", val);
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

		/*If power-save-enable is set Change UBRTSN
		* to save floor current during deep sleep
		*/
		if (!of_property_read_u32(np, "power-save-enable", &val)) {
			data->power_save_enable = (bool) val;

			if (data->power_save_enable &&
				(port.irq == BCM_INT_ID_UART2)) {
				ret = gpio_request(GPIO_PIN47,
					"serial_pin_47");
				if (ret) {
					pr_err("UART: GPIO %d",
						GPIO_PIN47);
					pr_err("gpio_request fails\n");
				}
			} else if (data->power_save_enable &&
				port.irq == BCM_INT_ID_UART1) {
				ret = gpio_request(GPIO_PIN20,
					"serial_pin_20");
				if (ret) {
					pr_err("UART: GPIO %d",
						GPIO_PIN20);
					pr_err("gpio_request fails\n");
				}
			}
		}

#ifdef CONFIG_BRCM_UART_CHANGES
		val = of_property_read_string(np, "port-name", &prop);
		if (val == 0) {
#ifdef CONFIG_DW_BT_UART_CHANGES
			if (!strcmp(prop, "bluetooth"))
				port.handle_irq = bt_dw8250_handle_irq;
#endif
		}

#ifndef CONFIG_MACH_HAWAII_FPGA
		if (of_property_read_string(np, "clk-name", &prop)) {
			dev_err(&pdev->dev, "clk-name Not found in dt-blob\n");
			return -1;
		}

		data->clk = clk_get(port.dev, prop);
		if (IS_ERR_OR_NULL(data->clk))
			return PTR_ERR(data->clk);

		clk_disable(data->clk);
		clk_set_rate(data->clk, port.uartclk);
		clk_enable(data->clk);

		port.uartclk = clk_get_rate(data->clk);
#endif
#endif /* CONFIG_BRCM_UART_CHANGES */

		data->line = serial8250_register_port(&port);
		if (data->line < 0)
			return data->line;

		platform_set_drvdata(pdev, data);
#else /* CONFIG_OF is not defined */
		/*
		 * The control comes here, if CONFIG_OF i.e
		 * Device Tree is not used and also platform data is
		 * NULL. This is an error condition.
		 */
		pr_err("%s(): No DT and platform Data is NULL, can't proceed \r\n",
			__func__);
		return -EINVAL;
#endif /* CONFIG_OF */
	} /* if (platform_data) else DT */
#endif

	return 0;
}

static int dw8250_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct dw8250_data *data = platform_get_drvdata(pdev);
	serial8250_suspend_port(data->line);
	return 0;
}

#ifdef CONFIG_BRCM_UART_CHANGES
static int dw8250_resume(struct platform_device *pdev)
{
	struct dw8250_data *data = platform_get_drvdata(pdev);
	serial8250_resume_port(data->line);
	return 0;
}

static int __devexit dw8250_remove(struct platform_device *pdev)
{
	struct dw8250_data *data = platform_get_drvdata(pdev);

	serial8250_unregister_port(data->line);

	return 0;
}

void serial8250_togglerts_afe(struct uart_port *port, unsigned int flowon)
{
	/* Keeping this as a dummy function.
	 * Thsi function has a dependency in drivers/bluetooth/bcm_bzhw.c
	 * Need to discuss with BT floks and remove this function.*/
	unsigned char old_mcr;

	old_mcr = port->serial_in(port, UART_MCR);
	if (flowon) {
		/* Enable AFE */
		old_mcr |= (UART_MCR_AFE | UART_MCR_RTS);
		port->serial_out(port, UART_MCR, old_mcr);
	} else {
		/* In case of flow_off, Disable AFE and pull the RTS line high.
		 * This will make sure BT will NOT send data. */
		old_mcr &= ~(UART_MCR_AFE);
		port->serial_out(port, UART_MCR, old_mcr);
		/* Writing MCR[1] = 0, make RTS line to go high */
		old_mcr &= ~(UART_MCR_RTS);
		port->serial_out(port, UART_MCR, old_mcr);
	}
}
EXPORT_SYMBOL(serial8250_togglerts_afe);
#endif

static const struct of_device_id dw8250_match[] = {
#ifdef CONFIG_BRCM_UART_CHANGES
	{ .compatible = "bcm,uart" },
#endif
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
#ifdef CONFIG_BRCM_UART_CHANGES
	.suspend		= dw8250_suspend,
	.resume			= dw8250_resume,
#endif
	.remove			= __devexit_p(dw8250_remove),
};

module_platform_driver(dw8250_platform_driver);

MODULE_AUTHOR("Jamie Iles");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Synopsys DesignWare 8250 serial port driver");
