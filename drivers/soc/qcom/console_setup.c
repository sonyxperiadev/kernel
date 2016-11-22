/* drivers/soc/qcom/console_setup.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/string.h>
#include <linux/of_platform.h>
#include <asm/setup.h>

#define CONSOLE_NAME "ttyHSL"
#define CONSOLE_IX 0
#define CONSOLE_OPTIONS "115200,n8"
#define CONSOLE_ENABLE 0x01

static int need_serial_console;

static int __init setup_serial_console(char *console_flag)
{
	unsigned long val;

	if (kstrtoul(console_flag, 16, &val))
		return -EINVAL;

	if (val & CONSOLE_ENABLE)
		need_serial_console = 1;

	return 0;
}

/*
* The S1 Boot configuration TA unit can specify that the serial console
* enable flag will be passed as Kernel boot arg with tag babe09A9.
*/
early_param("oemandroidboot.babe09a9", setup_serial_console);

static void disable_serial_gpio(void)
{
	struct device_node *np_tx, *np_rx, *np_tx_mux, *np_rx_mux,
		*np_tx_def, *np_rx_def;
	struct property *pp;
	static struct property gpio_function = {
		.name = "function",
		.value = "gpio",
		.length = sizeof("gpio"),
	};
	static struct property output_low = {
		.name = "output-low",
		.value = NULL,
		.length = 0,
	};
	static struct property bias_disable = {
		.name = "bias-disable",
		.value = NULL,
		.length = 0,
	};

	np_tx = of_find_node_by_path(
			"/soc/pinctrl@01010000/msm_gpio_4");
	if (!np_tx) {
		pr_err("couldn't find msm_gpio_4 node\n");
		return;
	}

	np_rx = of_find_node_by_path(
			"/soc/pinctrl@01010000/msm_gpio_5");
	if (!np_rx) {
		pr_err("couldn't find msm_gpio_5 node\n");
		goto err0;
	}

	np_tx_mux = of_find_node_by_name(np_tx, "mux");
	if (!np_tx_mux) {
		pr_err("couldn't find msm_gpio_4 mux node\n");
		goto err1;
	}

	np_rx_mux = of_find_node_by_name(np_rx, "mux");
	if (!np_tx_mux) {
		pr_err("couldn't find msm_gpio_5 mux node\n");
		goto err2;
	}

	of_update_property(np_tx_mux, &gpio_function);
	of_update_property(np_rx_mux, &gpio_function);

	np_tx_def = of_find_node_by_name(np_tx, "config");
	if (!np_tx_def) {
		pr_err("couldn't find msm_gpio_4 config node\n");
		goto err3;
	}

	np_rx_def = of_find_node_by_name(np_rx, "config");
	if (!np_rx_def) {
		pr_err("couldn't find msm_gpio_5 config node\n");
		goto err4;
	}

	of_add_property(np_tx_def, &output_low);

	pp = of_find_property(np_rx_def, "bias-pull-up", NULL);
	if (pp) {
		of_remove_property(np_rx_def, pp);
		of_add_property(np_rx_def, &bias_disable);
	}
	of_add_property(np_rx_def, &output_low);

	of_node_put(np_rx_def);
err4:
	of_node_put(np_tx_def);
err3:
	of_node_put(np_rx_mux);
err2:
	of_node_put(np_tx_mux);
err1:
	of_node_put(np_rx);
err0:
	of_node_put(np_tx);
	return;
}

static int __init init_console_setup(void)
{
	if (need_serial_console) {
		pr_info("Adding %s%d as preferred console\n",
			CONSOLE_NAME, CONSOLE_IX);
		add_preferred_console(CONSOLE_NAME,
			CONSOLE_IX,
			CONSOLE_OPTIONS);
	} else {
		struct device_node *np;
		static struct property serial_con_status = {
			.name = "status",
			.value = "disabled",
			.length = sizeof("disabled"),
		};

		np = of_find_node_by_path("/soc/serial@075b0000");
		if (!np) {
			pr_err("couldn't find /soc/serial@075b0000 node\n");
			return -EINVAL;
		}

		pr_info("disabling %s node", np->full_name);
		of_update_property(np, &serial_con_status);
		of_node_put(np);
		disable_serial_gpio();
	}

	return 0;
}
early_initcall(init_console_setup);
