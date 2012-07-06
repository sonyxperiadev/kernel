/* arch/arm/mach-msm/touch-fuji_nozomi.c
 *
 * Copyright (C) 2011 Sony Ericsson Mobile Communications AB.
 *
 * Author: Yusuke Yoshimura <Yusuke.Yoshimura@sonyericsson.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/input.h>
#include <linux/clearpad.h>

struct synaptics_button_data synaptics_back_key = {
	.code = KEY_BACK,
};

struct synaptics_button_data synaptics_home_key = {
	.code = KEY_HOME,
};

struct synaptics_button_data synaptics_menu_key = {
	.code = KEY_MENU,
};

struct synaptics_funcarea clearpad_funcarea_array[] = {
	{
		{ 0, 0, 719, 1279 }, { 0, 0, 719, 1299 },
		SYN_FUNCAREA_POINTER, NULL
	},
	{
		{ 0, 1315, 219, 1327 }, { 0, 1280, 239, 1327 },
		SYN_FUNCAREA_BUTTON, &synaptics_back_key
	},
	{
		{ 260, 1319, 459, 1327 }, { 240, 1280, 479, 1327 },
		SYN_FUNCAREA_BUTTON, &synaptics_home_key
	},
	{
		{ 500, 1315, 719, 1327 }, { 480, 1280, 719, 1327 },
		SYN_FUNCAREA_BUTTON, &synaptics_menu_key
	},
	{ .func = SYN_FUNCAREA_END }
};

struct synaptics_funcarea *clearpad_funcarea_get(u8 module_id, u8 rev)
{
	struct synaptics_funcarea *funcarea = NULL;

	pr_info("%s: module_id=0x%02x rev=0x%02x\n", __func__, module_id, rev);

	funcarea = clearpad_funcarea_array;

	return funcarea;
}
