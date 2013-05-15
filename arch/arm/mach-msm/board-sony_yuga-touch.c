/* arch/arm/mach-msm/board-sony_yuga-touch.c
 *
 * Copyright (C) 2012 Sony Mobile Communications AB.
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

struct synaptics_funcarea clearpad_funcarea_array_0x14[] = {
	{
		{ 0, 0, 719, 1279 }, { 0, 0, 719, 1332 },
		SYN_FUNCAREA_POINTER, NULL
	},
	{ .func = SYN_FUNCAREA_END }
};

struct synaptics_funcarea clearpad_funcarea_array[] = {
	{
		{ 0, 0, 1079, 1919 }, { 0, 0, 1079, 1919 },
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
	default:
		funcarea = clearpad_funcarea_array;
		break;
	}

	return funcarea;
}

int clearpad_flip_config_get(u8 module_id, u8 rev)
{
	switch (module_id) {
	case 0x36:
		if (rev <= 0x02)
			return SYN_FLIP_Y;
		else
			return SYN_FLIP_NONE;
	default:
		return SYN_FLIP_NONE;
	}
}

struct evgen_block *clearpad_evgen_block_get(u8 module_id, u8 rev)
{
	return NULL;
}

struct synaptics_easy_wakeup_config clearpad_easy_wakeup_config = {
	.gesture_enable = false,
	.large_panel = false,
};
