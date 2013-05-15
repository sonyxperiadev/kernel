/* arch/arm/mach-msm/board-sony_pollux-touch.c
 *
 * Copyright (C) 2012 - 2013 Sony Mobile Communications AB.
 *
 * Author: Yusuke Yoshimura <Yusuke.Yoshimura@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <linux/input.h>
#include <linux/clearpad.h>
#include <linux/input/evgen_helper.h>

struct synaptics_pointer_data pointer_data_0x19 = {
	.offset_x = 0,
	.offset_y = 49,
};

struct synaptics_pointer_data pointer_data_0x1A = {
	.offset_x = 0,
	.offset_y = 55,
};

struct synaptics_funcarea clearpad_funcarea_array_0x14[] = {
	{
		{ 0, 0, 719, 1279 }, { 0, 0, 719, 1332 },
		SYN_FUNCAREA_POINTER, NULL
	},
	{ .func = SYN_FUNCAREA_END }
};

struct synaptics_funcarea clearpad_funcarea_array_0x19[] = {
	{
		{ 0, 0, 719, 36 }, { 0, 0, 719, 36 },
		SYN_FUNCAREA_INSENSIBLE, NULL
	},
	{
		{ 0, 49, 719, 1328 }, { 0, 37, 719, 1332 },
		SYN_FUNCAREA_POINTER, &pointer_data_0x19
	},
	{ .func = SYN_FUNCAREA_END }
};

struct synaptics_funcarea clearpad_funcarea_array_0x1A[] = {
	{
		{ 0, 0, 719, 42 }, { 0, 0, 719, 42 },
		SYN_FUNCAREA_INSENSIBLE, NULL
	},
	{
		{ 0, 55, 719, 1334 }, { 0, 43, 719, 1336 },
		SYN_FUNCAREA_POINTER, &pointer_data_0x1A
	},
	{ .func = SYN_FUNCAREA_END }
};

struct synaptics_funcarea clearpad_funcarea_array_0x30[] = {
	{
		{ 0, 0, 1919, 1119 }, { 0, 0, 1919, 1119 },
		SYN_FUNCAREA_POINTER, NULL
	},
	{ .func = SYN_FUNCAREA_END }
};

struct synaptics_funcarea clearpad_funcarea_array[] = {
	{
		{ 0, 0, 1919, 1199 }, { 0, 0, 1919, 1199 },
		SYN_FUNCAREA_POINTER, NULL
	},
	{ .func = SYN_FUNCAREA_END }
};

struct synaptics_funcarea *clearpad_funcarea_get(u8 module_id, u8 rev)
{
	struct synaptics_funcarea *funcarea = NULL;

	pr_info("%s: module_id=0x%02x rev=0x%02x\n", __func__, module_id, rev);
	switch (module_id) {
	case 0x14:
		funcarea = clearpad_funcarea_array_0x14;
		break;
	case 0x19:
		funcarea = clearpad_funcarea_array_0x19;
		break;
	case 0x1A:
		funcarea = clearpad_funcarea_array_0x1A;
		break;
	case 0x30:
		funcarea = clearpad_funcarea_array_0x30;
		break;
	case 0x3A:
		if (rev < 0x04)
			funcarea = clearpad_funcarea_array_0x30;
		else
			funcarea = clearpad_funcarea_array;
		break;
	default:
		funcarea = clearpad_funcarea_array;
		break;
	}

	return funcarea;
}

int clearpad_flip_config_get(u8 module_id, u8 rev)
{
	return SYN_FLIP_NONE;
}

static struct evgen_record double_tap[] = {
	{
		.type = EVGEN_LOG,
		.data.log.message = "=== DOUBLE TAP ===",
	},
	{
		.type = EVGEN_KEY,
		.data.key.code = KEY_POWER,
		.data.key.down = true,
	},
	{
		.type = EVGEN_KEY,
		.data.key.code = KEY_POWER,
		.data.key.down = false,
	},
	{
		.type = EVGEN_END,
	},
};

static struct evgen_block evgen_blocks[] = {
	{
		.name = "double_tap",
		.records = double_tap,
	},
	{
		.name = NULL,
		.records = NULL,
	}
};

struct evgen_block *clearpad_evgen_block_get(u8 module_id, u8 rev)
{
	return evgen_blocks;
}

struct synaptics_easy_wakeup_config clearpad_easy_wakeup_config = {
	.gesture_enable = false,
	.large_panel = true,
	.timeout_delay = 2000,
};
