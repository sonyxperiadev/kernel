/*
 * Synopsys DesignWare 8250 driver.
 *
 * Copyright 2011 Picochip, Jamie Iles.
 * Copyright 2013 Intel Corporation
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
 * Copyright 2013 Broadcom Corporation.  All rights reserved.
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
 * ***************************************************************************/

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
#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>

#include <linux/gpio.h>
#include "8250.h"

#define GPIO_PIN47 47
#define GPIO_PIN20 20
#define UART_RESUME 0
#define UART_SUSPEND 3
/* Offsets for the DesignWare specific registers */
#define DW_UART_USR	0x1f /* UART Status Register */
#define DW_UART_CPR	0xf4 /* Component Parameter Register */
#define DW_UART_UCV	0xf8 /* UART Component Version */

/* Component Parameter Register bits */
#define DW_UART_CPR_ABP_DATA_WIDTH	(3 << 0)
#define DW_UART_CPR_AFCE_MODE		(1 << 4)
#define DW_UART_CPR_THRE_MODE		(1 << 5)
#define DW_UART_CPR_SIR_MODE		(1 << 6)
#define DW_UART_CPR_SIR_LP_MODE		(1 << 7)
#define DW_UART_CPR_ADDITIONAL_FEATURES	(1 << 8)
#define DW_UART_CPR_FIFO_ACCESS		(1 << 9)
#define DW_UART_CPR_FIFO_STAT		(1 << 10)
#define DW_UART_CPR_SHADOW		(1 << 11)
#define DW_UART_CPR_ENCODED_PARMS	(1 << 12)
#define DW_UART_CPR_DMA_EXTRA		(1 << 13)
#define DW_UART_CPR_FIFO_MODE		(0xff << 16)
/* Helper for fifo size calculation */
#define DW_UART_CPR_FIFO_SIZE(a)	(((a >> 16) & 0xff) * 16)

#ifdef CONFIG_DW_BT_UART_CHANGES
extern int bt_dw8250_handle_irq(struct uart_port *p);
#endif

struct dw8250_data {
	int		last_lcr;
	int		line;
	struct clk	*clk;
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

static int dw8250_handle_irq(struct uart_port *p)
{
	struct dw8250_data *d = p->private_data;
	unsigned int iir = p->serial_in(p, UART_IIR);
#ifdef CONFIG_BRCM_UART_CHANGES
	unsigned long flags;
	struct uart_8250_port *up =
		container_of(p, struct uart_8250_port, port);

	/* This is used for checking the Timeout interrupt in
	 * serial8250_rx_chars() */
	up->iir = iir;
#endif

	if (serial8250_handle_irq(p, iir)) {
		return 1;
	} else if ((iir & UART_IIR_BUSY) == UART_IIR_BUSY) {
		/* Clear the USR and write the LCR again. */
		(void)p->serial_in(p, DW_UART_USR);
#ifdef CONFIG_BRCM_UART_CHANGES
		/* Stop writing to LCR if the value is same. */
		spin_lock_irqsave(&p->lock, flags);
		if (p->serial_in(p, UART_LCR) != d->last_lcr)
			p->serial_out(p, UART_LCR, d->last_lcr);
		spin_unlock_irqrestore(&p->lock, flags);
#else
		p->serial_out(p, UART_LCR, d->last_lcr);
#endif

		return 1;
	}

	return 0;
}

void dw8250_do_pm(struct uart_port *port, unsigned int state,
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
		if (port->handle_irq == bt_dw8250_handle_irq) {
			clk_disable(private_data->clk);
			clk_set_rate(private_data->clk, port->uartclk);
			clk_enable(private_data->clk);
			port->uartclk = clk_get_rate(private_data->clk);
		}
			/*Resume sequence*/
		pm_runtime_get_sync(port->dev);
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
			ret = gpio_direction_output(GPIO_PIN47, 1);
			if (ret)
				pr_err("UART: GPIO: direction_input failed\n");
		} else if ((private_data->power_save_enable) &&
				(port->irq == BCM_INT_ID_UART1)) {
			pinmux_set_pin_config(&uartb2_config[0]);
			ret = gpio_direction_output(GPIO_PIN20, 1);
			if (ret)
				pr_err("UART: GPIO: direction_input failed\n");
		}
		serial8250_do_pm(port, state, old_state);
		pm_runtime_put_sync_suspend(port->dev);
		/* Set the BT clock to 26Mhz if not in use.
		 * This is causing the increase in power numbers in
		 * MP3 use case */
		if (port->handle_irq == bt_dw8250_handle_irq) {
			clk_disable(private_data->clk);
			clk_set_rate(private_data->clk, 26000000);
			clk_enable(private_data->clk);
		}
		break;
	default:
		serial8250_do_pm(port, state, old_state);
		break;
	}
}

#ifdef CONFIG_BRCM_UART_CHANGES
/* We can optimize this functions. Filling Some of the
 * Uart_port Sturcture members again here. */
static int dw8250_probe_platform_data(struct platform_device *pdev)
{
	struct uart_8250_port uart = {};
	struct uart_port *port = &uart.port;
	struct plat_serial8250_port *p = pdev->dev.platform_data;
	int i, irqflag = 0, ret = 0;
	struct dw8250_data *data = port->private_data;

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

			port->private_data = data;

			port->iobase             = p->iobase;
			port->membase            = p->membase;
			port->irq                = p->irq;
			port->irqflags           = p->irqflags;
			port->uartclk            = p->uartclk;
			port->regshift           = p->regshift;
			port->iotype             = p->iotype;
			port->flags              = p->flags;
			port->mapbase            = p->mapbase;
			port->hub6               = p->hub6;
			port->type               = p->type;
			port->serial_in          = dw8250_serial_in;
			port->serial_out         = dw8250_serial_out;
			if (!(p->handle_irq)) {
				/* Default 8250_dw handler */
				port->handle_irq = dw8250_handle_irq;
				if (p->port_name) {
					if (!strcmp(p->port_name, "bluetooth"))
						port->handle_irq =
							bt_dw8250_handle_irq;
				}
			}

			port->set_termios	= p->set_termios;
			port->irqflags          |= irqflag;
			port->pm                 = dw8250_do_pm;
			port->dev                = &pdev->dev;
			port->irqflags           |= irqflag;

			pdev->dev.platform_data = p;

#ifndef CONFIG_MACH_BCM_FPGA
			data->clk = clk_get(port->dev, p->clk_name);
			if (IS_ERR(data->clk))
				return PTR_ERR(data->clk);

			clk_disable(data->clk);
			clk_set_rate(data->clk, port->uartclk);
			clk_enable(data->clk);

			port->uartclk = clk_get_rate(data->clk);
#endif
			data->line = serial8250_register_8250_port(&uart);
			if (data->line < 0)
				return data->line;

			platform_set_drvdata(pdev, data);

			pm_runtime_set_active(&pdev->dev);
			pm_runtime_enable(&pdev->dev);
		}
	}
	return 0;

}
#endif

static int dw8250_probe_of(struct uart_port *p)
{
	struct device_node	*np = p->dev->of_node;
	u32			val;
#ifdef CONFIG_BRCM_UART_CHANGES
	const char *prop;
	struct dw8250_data *data = p->private_data;
	int ret;
#endif

	if (!of_property_read_u32(np, "reg-io-width", &val)) {
		switch (val) {
		case 1:
			break;
		case 4:
			p->iotype = UPIO_MEM32;
			p->serial_in = dw8250_serial_in32;
			p->serial_out = dw8250_serial_out32;
			break;
		default:
			dev_err(p->dev, "unsupported reg-io-width (%u)\n", val);
			return -EINVAL;
		}
	}

	if (!of_property_read_u32(np, "reg-shift", &val))
		p->regshift = val;

	/* clock got configured through clk api, all done */
	if (p->uartclk)
		return 0;

	/* try to find out clock frequency from DT as fallback */
	if (of_property_read_u32(np, "clock-frequency", &val)) {
		dev_err(p->dev, "clk or clock-frequency not defined\n");
		return -EINVAL;
	}
	p->uartclk = val;

#ifdef CONFIG_BRCM_UART_CHANGES
	/*If power-save-enable is set Change UBRTSN
	* to save floor current during deep sleep
	*/
	if (!of_property_read_u32(np, "power-save-enable", &val)) {
		data->power_save_enable = (bool) val;

		if (data->power_save_enable && (p->irq == BCM_INT_ID_UART2)) {
			ret = gpio_request(GPIO_PIN47, "serial_pin_47");
			if (ret) {
				pr_err("UART: GPIO %d",	GPIO_PIN47);
				pr_err("gpio_request fails\n");
			}
		} else if (data->power_save_enable &&
				(p->irq == BCM_INT_ID_UART1)) {
			ret = gpio_request(GPIO_PIN20, "serial_pin_20");
			if (ret) {
				pr_err("UART: GPIO %d",	GPIO_PIN20);
				pr_err("gpio_request fails\n");
			}
		}
	}

	val = of_property_read_string(np, "port-name", &prop);
	if (val == 0) {
#ifdef CONFIG_DW_BT_UART_CHANGES
		if (!strcmp(prop, "bluetooth"))
			p->handle_irq = bt_dw8250_handle_irq;
#endif
	}

#ifndef CONFIG_MACH_HAWAII_FPGA
	if (of_property_read_string(np, "clk-name", &prop)) {
		dev_err(p->dev, "clk-name Not found in dt-blob\n");
		return -1;
	}

	data->clk = clk_get(p->dev, prop);
	if (IS_ERR_OR_NULL(data->clk))
		return PTR_ERR(data->clk);

	clk_disable(data->clk);
	clk_set_rate(data->clk, p->uartclk);
	clk_enable(data->clk);

	p->uartclk = clk_get_rate(data->clk);
#endif
#endif /* CONFIG_BRCM_UART_CHANGES */
	return 0;
}

#ifdef CONFIG_ACPI
static int dw8250_probe_acpi(struct uart_8250_port *up)
{
	const struct acpi_device_id *id;
	struct uart_port *p = &up->port;

	id = acpi_match_device(p->dev->driver->acpi_match_table, p->dev);
	if (!id)
		return -ENODEV;

	p->iotype = UPIO_MEM32;
	p->serial_in = dw8250_serial_in32;
	p->serial_out = dw8250_serial_out32;
	p->regshift = 2;

	if (!p->uartclk)
		p->uartclk = (unsigned int)id->driver_data;

	up->dma = devm_kzalloc(p->dev, sizeof(*up->dma), GFP_KERNEL);
	if (!up->dma)
		return -ENOMEM;

	up->dma->rxconf.src_maxburst = p->fifosize / 4;
	up->dma->txconf.dst_maxburst = p->fifosize / 4;

	return 0;
}
#else
static inline int dw8250_probe_acpi(struct uart_8250_port *up)
{
	return -ENODEV;
}
#endif /* CONFIG_ACPI */

static void dw8250_setup_port(struct uart_8250_port *up)
{
	struct uart_port	*p = &up->port;
	u32			reg = readl(p->membase + DW_UART_UCV);

	/*
	 * If the Component Version Register returns zero, we know that
	 * ADDITIONAL_FEATURES are not enabled. No need to go any further.
	 */
	if (!reg)
		return;

	dev_dbg_ratelimited(p->dev, "Designware UART version %c.%c%c\n",
		(reg >> 24) & 0xff, (reg >> 16) & 0xff, (reg >> 8) & 0xff);

	reg = readl(p->membase + DW_UART_CPR);
	if (!reg)
		return;

	/* Select the type based on fifo */
	if (reg & DW_UART_CPR_FIFO_MODE) {
		p->type = PORT_16550A;
		p->flags |= UPF_FIXED_TYPE;
		p->fifosize = DW_UART_CPR_FIFO_SIZE(reg);
		up->tx_loadsz = p->fifosize;
		up->capabilities = UART_CAP_FIFO;
	}

	if (reg & DW_UART_CPR_AFCE_MODE)
		up->capabilities |= UART_CAP_AFE;
}

static int dw8250_probe(struct platform_device *pdev)
{
	struct uart_8250_port uart = {};
	struct resource *regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct resource *irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	struct dw8250_data *data;
	int err = 0;

	/* If platform_data do everything in dw8250_probe_platform_data()
	 * and return */
	if (pdev->dev.platform_data) {
		err = dw8250_probe_platform_data(pdev);
		return err;
	}
	if (!regs || !irq) {
		dev_err(&pdev->dev, "no registers/irq defined\n");
		return -EINVAL;
	}

	spin_lock_init(&uart.port.lock);
	uart.port.mapbase = regs->start;
	uart.port.irq = irq->start;
	uart.port.handle_irq = dw8250_handle_irq;
#ifdef CONFIG_BRCM_UART_CHANGES
	uart.port.type = PORT_16550A;
	uart.port.flags = UPF_BOOT_AUTOCONF | UPF_BUG_THRE |
		UPF_SKIP_TEST | UPF_FIXED_TYPE;
	uart.port.irqflags = 0;
	uart.port.pm = dw8250_do_pm;
#else
	uart.port.type = PORT_8250;
	uart.port.flags = UPF_SHARE_IRQ | UPF_BOOT_AUTOCONF | UPF_FIXED_PORT;
	uart.port.pm = dw8250_do_pm;
#endif /* CONFIG_BRCM_UART_CHANGES */
	uart.port.dev = &pdev->dev;

	uart.port.membase = devm_ioremap(&pdev->dev, regs->start,
					 resource_size(regs));
	if (!uart.port.membase)
		return -ENOMEM;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->clk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(data->clk)) {
		clk_prepare_enable(data->clk);
		uart.port.uartclk = clk_get_rate(data->clk);
	}

	uart.port.iotype = UPIO_MEM;
	uart.port.serial_in = dw8250_serial_in;
	uart.port.serial_out = dw8250_serial_out;
	uart.port.private_data = data;

	dw8250_setup_port(&uart);

	/* We store the clock information in the private data.
	 * Clock name is part of Device tree. */
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	uart.port.private_data = data;

	if (pdev->dev.of_node) {
		err = dw8250_probe_of(&uart.port);
		if (err)
			return err;
	} else if (ACPI_HANDLE(&pdev->dev)) {
		err = dw8250_probe_acpi(&uart);
		if (err)
			return err;
	} else {
		return -ENODEV;
	}

	data->line = serial8250_register_8250_port(&uart);
	if (data->line < 0)
		return data->line;

	platform_set_drvdata(pdev, data);

	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	return 0;
}

static int dw8250_remove(struct platform_device *pdev)
{
	struct dw8250_data *data = platform_get_drvdata(pdev);

	pm_runtime_get_sync(&pdev->dev);

	serial8250_unregister_port(data->line);

	if (!IS_ERR(data->clk))
		clk_disable_unprepare(data->clk);

	pm_runtime_disable(&pdev->dev);
	pm_runtime_put_noidle(&pdev->dev);

	return 0;
}

#ifdef CONFIG_PM
static int dw8250_suspend(struct device *dev)
{
	struct dw8250_data *data = dev_get_drvdata(dev);

	serial8250_suspend_port(data->line);

	return 0;
}

static int dw8250_resume(struct device *dev)
{
	struct dw8250_data *data = dev_get_drvdata(dev);

	serial8250_resume_port(data->line);

	return 0;
}
#endif /* CONFIG_PM */

#ifdef CONFIG_BRCM_UART_CHANGES
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

#ifdef CONFIG_PM_RUNTIME
static int dw8250_runtime_suspend(struct device *dev)
{
	struct dw8250_data *data = dev_get_drvdata(dev);

	if (!IS_ERR(data->clk))
		clk_disable_unprepare(data->clk);

	return 0;
}

static int dw8250_runtime_resume(struct device *dev)
{
	struct dw8250_data *data = dev_get_drvdata(dev);

	if (!IS_ERR(data->clk))
		clk_prepare_enable(data->clk);

	return 0;
}
#endif

static const struct dev_pm_ops dw8250_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dw8250_suspend, dw8250_resume)
	SET_RUNTIME_PM_OPS(dw8250_runtime_suspend, dw8250_runtime_resume, NULL)
};

static const struct of_device_id dw8250_of_match[] = {
#ifdef CONFIG_BRCM_UART_CHANGES
	{ .compatible = "bcm,uart" },
#endif
	{ .compatible = "snps,dw-apb-uart" },
	{ /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, dw8250_of_match);

static const struct acpi_device_id dw8250_acpi_match[] = {
	{ "INT33C4", 0 },
	{ "INT33C5", 0 },
	{ "80860F0A", 0 },
	{ },
};
MODULE_DEVICE_TABLE(acpi, dw8250_acpi_match);

static struct platform_driver dw8250_platform_driver = {
	.driver = {
#ifdef CONFIG_BRCM_UART_CHANGES
		.name		= "serial8250_dw",
#else
		.name		= "dw-apb-uart",
#endif
		.owner		= THIS_MODULE,
		.pm		= &dw8250_pm_ops,
		.of_match_table	= dw8250_of_match,
		.acpi_match_table = ACPI_PTR(dw8250_acpi_match),
	},
	.probe			= dw8250_probe,
	.remove			= dw8250_remove,
};

module_platform_driver(dw8250_platform_driver);

MODULE_AUTHOR("Jamie Iles");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Synopsys DesignWare 8250 serial port driver");
