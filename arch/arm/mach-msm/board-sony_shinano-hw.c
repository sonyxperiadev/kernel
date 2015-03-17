/* arch/arm/mach-msm/board-sony_shinano-hw.c
 *
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 *
 * Author: Kouhei Fujiya <Kouhei.X.Fujiya@sonymobile.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/string.h>
#include <linux/of.h>
#include "board-sony_shinano-hw.h"

struct hw_map {
	const char *space_no;
	enum sony_hw hw;
};

static const struct hw_map sony_hw_map[] = {
	{ "1280-4852", HW_LEO },
	{ "1280-4876", HW_LEO },
	{ "1280-4918", HW_LEO },
	{ "1282-0073", HW_LEO },
	{ "1280-4897", HW_LEO_SAMBA },
	{ "1284-1688", HW_LEO_SAMBA },
	{ "1276-9754", HW_SIRIUS },
	{ "1278-9570", HW_SIRIUS },
	{ "1278-9584", HW_SIRIUS_SAMBA },
	{ "1278-9462", HW_SIRIUS_SAMBA },
};

static int _sony_hw = HW_UNKNOWN;

int get_sony_hw(void)
{
	int ret = 0;
	struct device_node *dt_root;
	int i, j, count = 0;
	const char *dt_space_no = NULL;

	if (_sony_hw != HW_UNKNOWN)
		return _sony_hw;

	dt_root = of_find_node_by_path("/");
	count = of_property_count_strings(dt_root, "somc,space-no");

	if (count <= 0)
		return HW_UNKNOWN;

	for (i = 0; i < count; i++) {
		ret = of_property_read_string_index(dt_root,
				"somc,space-no", i, &dt_space_no);
		if (ret < 0)
			continue;

		for (j = 0; j < ARRAY_SIZE(sony_hw_map); j++) {
			if (!strcmp(dt_space_no, sony_hw_map[j].space_no)) {
				_sony_hw = sony_hw_map[j].hw;
				break;
			};
		};
	};

	return _sony_hw;
}
