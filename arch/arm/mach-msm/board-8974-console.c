/* arch/arm/mach-msm/board-8974-console.c
 *
 * Copyright (C) 2013 Sony Mobile Communications AB.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/console.h>
#include <linux/string.h>
#include <asm/setup.h>

#define CONSOLE_NAME "ttyHSL"
#define CONSOLE_IX 0
#define CONSOLE_OPTIONS "115200,n8"

static int __init setup_serial_console(char *console_flag)
{
	if (console_flag &&
		strnlen(console_flag, COMMAND_LINE_SIZE) >= 2 &&
		(console_flag[0] != '0' || console_flag[1] != '0'))
		add_preferred_console(CONSOLE_NAME,
			CONSOLE_IX,
			CONSOLE_OPTIONS);
	return 1;
}

/*
* The S1 Boot configuration TA unit can specify that the serial console
* enable flag will be passed as Kernel boot arg with tag babe09A9.
*/
__setup("oemandroidboot.babe09a9=", setup_serial_console);
